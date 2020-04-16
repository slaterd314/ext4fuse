#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdint.h>
#include "../../logging.h"

static int ll_pread(HANDLE hFile, void* buffer, size_t count, off_t offset);

// Note: this isn't thread-safe
// However, pread() is only called from __disk_read()
// which itself has a mutex protecting the call to pread.
static uint8_t * read_buffer = NULL;
DWORD sector_size = 512;
DWORD buffer_size = 4096;

int win_open(const char* path, int flags)
{
	// to open a raw partition, 
	// we must use FILE_SHARE_READ | FILE_SHARE_WRITE flags

	((void)flags);
	HANDLE hFile = CreateFile(path, 
		FILE_READ_DATA,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	// Get the disk geometry to determine what our buffer alignment needs to be and 
	// what out read block size must be.
	DWORD junk = 0;
	DISK_GEOMETRY_EX geom = { 0 };
	BOOL success = DeviceIoControl(hFile, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, (void*)&geom, (DWORD)sizeof(geom), &junk, (LPOVERLAPPED)NULL);
	if (!success)
	{
		return -1;
	}
	sector_size = geom.Geometry.BytesPerSector;
	buffer_size = max(sector_size, 4096);
	// make sure we allocate a multiple of the page size since we're using VirtualAlloc()
	buffer_size = ((buffer_size + 4095) / 4096) * 4096;
	
	read_buffer = (uint8_t *)VirtualAlloc(NULL, buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!read_buffer)
	{
		CloseHandle(hFile);
		return -1;
	}
	
	// Allow us to read all the way to the end of the partition.
	DeviceIoControl(hFile, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, NULL, 0, &junk, (LPOVERLAPPED)NULL);

	// Open an osfhandle so we can pass a 32-bit int back to our caller.
	int fd = _open_osfhandle((intptr_t)hFile, (_O_RDONLY | _O_BINARY));
	if (fd == -1)
	{
		CloseHandle(hFile);
	}
	return fd;
}

int pread(unsigned int fd, char* buf, size_t size, off_t offset)
{
	int ret = -1;
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		ret = 0;
		// distance into the first block at which our offset starts.
		off_t first_offset = offset % sector_size;
		// offset of the first sector we need to read.
		off_t aligned_offset = (offset - first_offset);
		// account for the alignment bytes
		size_t block_size = size + first_offset;
		// Total number of bytes we need to read for aligned sector sized reads.
		block_size = ((block_size + sector_size - 1) / sector_size) * sector_size;

		// read at least sector_size, but not more that buffer_size
		size_t read_size = max(min(block_size, buffer_size), sector_size);

		int pread_ret = ll_pread(hFile, read_buffer, read_size, aligned_offset);
		ASSERT(pread_ret == read_size);
		size_t first_size = min(size, (size_t)(read_size - first_offset));
		memcpy(buf, read_buffer + first_offset, first_size);
		buf += first_size;
		size -= first_size;
		offset += first_size;
		ret += first_size;
		// once we get here, size is an even multiple of sector_size
		while (size >= buffer_size) {
			int pread_ret = ll_pread(hFile, read_buffer, buffer_size, offset);
			ASSERT(pread_ret == buffer_size);
			memcpy(buf, read_buffer, buffer_size);
			buf += buffer_size;
			size -= buffer_size;
			offset += buffer_size;
			ret += buffer_size;
		}

		if (size > 0) {
			block_size = ((size + sector_size - 1) / sector_size) * sector_size;
			int pread_ret = ll_pread(hFile, read_buffer, block_size, offset);
			ASSERT(pread_ret == block_size);
			memcpy(buf, read_buffer, size);
			buf += size;
			offset += size;
			ret += size;
			size -= size;
		}
	}
	return ret;
}

static int ll_pread(HANDLE hFile, void* buffer, size_t count, off_t offset)
{
	LARGE_INTEGER distanceToMove;
	LARGE_INTEGER oldPos;
	int ret = -1;
	distanceToMove.QuadPart = oldPos.QuadPart = 0;
	// fetch the current position.
	BOOL success = SetFilePointerEx(hFile, distanceToMove, &oldPos, FILE_CURRENT);
	if (success)
	{
		// move to the requested position
		distanceToMove.QuadPart = offset;
		success = SetFilePointerEx(hFile, distanceToMove, NULL, FILE_BEGIN);
		if (success)
		{
			// read the data
			DWORD bytesRead = 0;
			success = ReadFile(hFile, buffer, (DWORD)count, &bytesRead, NULL);
			if (success)
			{
				// reset the file pointer
				SetFilePointerEx(hFile, oldPos, NULL, FILE_BEGIN);
				ret = bytesRead;
			}
		}
	}
	return ret;
}
