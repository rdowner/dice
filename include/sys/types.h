
/*
 * $VER: sys/types.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#ifndef TIME_H
#include <time.h>
#endif
/*#ifndef STDLIB_H*/
/*#include <stdlib.h>*/
/*#endif*/
#ifndef STDIO_H
#include <stdio.h>
#endif
#ifndef ERRNO_H
#include <errno.h>
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE  256
#endif

typedef long fd_mask;

typedef struct fd_set {
    fd_mask fds_bits[(FD_SETSIZE+31)/32];
} fd_set;

#define FD_ISSET(n,set) ((set)->fds_bits[(n)>>5] & (1 << ((n) & 31)))
#define FD_SET(n,set)	((set)->fds_bits[(n)>>5] |= (1 << ((n) & 31)))
#define FD_CLR(n,set)	((set)->fds_bits[(n)>>5] &= ~(1 << ((n) & 31)))
#define FD_ZERO(set)	clrmem((void *)(set), sizeof(*(set)))

#define major(x)	((int)(((u_int)(x) >> 8)&0xff)) /* major number */
#define minor(x)	((int)((x)&0xff))		/* minor number */
#define makedev(x,y)	((dev_t)(((x)<<8) | (y)))	/* create dev_t */

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
typedef unsigned int u_int;

typedef char *	caddr_t;		/* core address */
typedef long	daddr_t;		/* disk address */
typedef short	dev_t;			/* device number */
typedef u_long	ino_t;			/* inode number */
typedef long	off_t;			/* file offset (should be a quad) */
typedef u_short nlink_t;		/* link count */
typedef long	swblk_t;		/* swap offset */
typedef long	segsz_t;		/* segment size */
typedef u_short uid_t;			/* user id */
typedef u_short gid_t;			/* group id */
typedef short	pid_t;			/* process id */
typedef u_short mode_t; 		/* permissions */
typedef u_long	fixpt_t;		/* fixed point number */

#define __P(args)

#endif
