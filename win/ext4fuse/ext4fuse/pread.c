#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include <stddef.h>
#include <fcntl.h>

int win_open(const char* path, int flags)
{
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
	DWORD junk = 0;
	DeviceIoControl(hFile, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, NULL, 0, &junk, (LPOVERLAPPED)NULL);

	int fd = _open_osfhandle((intptr_t)hFile, (_O_RDONLY | _O_BINARY));
	if (fd == -1)
	{
		CloseHandle(hFile);
	}
	return fd;
}

int pread(unsigned int fd, char* buf, size_t count, off_t offset)
{
/*
	__int64 cur = _telli64(fd);
	int ret = -1;
	if (_lseeki64(fd, offset, SEEK_SET) == offset) {
		ret = read(fd, buf, count);
	}
	if (cur >= 0)
	{
		// reset the file pointer
		_lseeki64(fd, cur, SEEK_SET);
	}
	return ret;
*/

	int ret = -1;
	HANDLE handle = (HANDLE)_get_osfhandle(fd);

	if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER distanceToMove;
		LARGE_INTEGER oldPos;
		distanceToMove.QuadPart = oldPos.QuadPart = 0;
		// fetch the current position.
		BOOL success = SetFilePointerEx(handle, distanceToMove, &oldPos, FILE_CURRENT);
		if (success)
		{
			distanceToMove.QuadPart = offset;
			success = SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN);
			if (success)
			{
				//OVERLAPPED overLap = {0};
				//overLap.Offset = distanceToMove.LowPart;
				//overLap.OffsetHigh = distanceToMove.HighPart;
				DWORD bytesRead = 0;
				// success = ReadFile(handle, buf, (DWORD)count, &bytesRead, &overLap);
				void* buffer = VirtualAlloc(NULL, (DWORD)count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
				success = ReadFile(handle, buffer, (DWORD)count, &bytesRead, NULL);
				if (success)
				{
					memcpy(buf, buffer, count);
					SetFilePointerEx(handle, oldPos, NULL, FILE_BEGIN);
					ret = bytesRead;
				}
				VirtualFree(buffer, 0, MEM_RELEASE);
			}
		}
	}
	return ret;
/*
	static int c = -1;
	++c;
	long cur = tell(fd);
	int ret = -1;
	int offset512 = (offset & 0xFFFFFFFFFFFFFE00);
	int offsetInto = (offset - offset512);
	size_t count512 = ((count + 511) / 512) * 512;
	char* buffer = (char *)malloc(count512);
	if (buffer)
	{
		if (_lseek(fd, offset512, SEEK_SET) == offset512) {		
			ret = read(fd, buffer, count512);
			if (ret == count512)
			{
				memcpy(buf, buffer + offsetInto, count);
				ret = count;
			}
		}
		free(buffer);
	}
	if (cur >= 0)
	{
		// reset the file pointer
		_lseek(fd, cur, SEEK_SET);
	}
	return ret;
	*/
}
