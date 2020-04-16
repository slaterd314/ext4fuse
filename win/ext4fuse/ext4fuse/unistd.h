#ifndef UNISTD_H
#define UNISTD_H
#include <stddef.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fuse.h>

typedef ptrdiff_t ssize_t;

ssize_t pread(int fd, void *buf, size_t count, off_t offset);

#define open win_open
int win_open(const char* path, int flags);


#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000)	/* fifo */
#ifndef _POSIX_SOURCE
#define	S_ISLNK(m)	((m & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	((m & 0170000) == 0140000)	/* socket */
#endif

#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif

#endif // UNISTD_H
