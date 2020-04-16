#ifndef UNISTD_H
#define UNISTD_H
#include <stddef.h>
#include <sys/types.h>
#include <fuse.h>

typedef ptrdiff_t ssize_t;

ssize_t pread(int fd, void *buf, size_t count, off_t offset);

#define open win_open
int win_open(const char* path, int flags);


#endif // UNISTD_H
