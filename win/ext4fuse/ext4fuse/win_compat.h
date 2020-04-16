#ifndef WIN_COMPAT_H_INCLUDED
#define WIN_COMPAT_H_INCLUDED
#pragma once
#include <io.h>
#include <fcntl.h>
#include <fuse.h>
#include <sys/stat.h>

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


#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec

#endif // WIN_COMPAT_H_INCLUDED
