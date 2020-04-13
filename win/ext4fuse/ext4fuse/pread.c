#include <Windows.h>
#include <stdio.h>
#include <io.h>


int pread(unsigned int fd, char* buf, size_t count, int offset)
{
/*
	char buffer[512];
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
			distanceToMove.LowPart = offset;
			success = SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN);
			if (success)
			{
				OVERLAPPED overLap = {0};
				DWORD bytesRead = 0;
				// overLap.Offset = offset;
				count = 512;

				success = ReadFile(handle, (void*)buffer, (DWORD)count, &bytesRead, &overLap);
				if (success)
				{
					SetFilePointerEx(handle, oldPos, NULL, FILE_BEGIN);
					ret = bytesRead;
				}
			}
		}
	}
	return ret;
*/

	long cur = tell(fd);
	int ret = -1;
	int offset512 = (offset & 0xFFFFFE00);
	int offsetInto = (offset - offset512);
	size_t count512 = ((count + 511) / 512) * 512;
	char* buffer = (char *)malloc(count512);
	if (buffer)
	{
		if (_lseek(fd, offset512, SEEK_SET) == offset512) {		
			ret = read(fd, buffer, count512);
			if (ret >= count)
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

}
