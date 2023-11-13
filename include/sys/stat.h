
/*
 * $VER: sys/stat.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SYS_STAT_H
#define SYS_STAT_H

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef LIBRARIES_DOS_H
#include <libraries/dos.h>
#endif

#define S_IFMT      0xF0000
#define S_IFREG     0x10000
#define S_IFDIR     0x20000
#define S_IFLNK     0x30000
#define S_IFCHR     0x40000
#define S_IFBLK     0x50000

#define S_ISUID     0x08000
#define S_ISGID     0x04000
#define S_ISVTX     0x02000

#define S_IREAD     000400
#define S_IWRITE    000200
#define S_IEXEC     000100

struct stat {
    long    st_mode;
    long    st_size;
    long    st_blksize;     /*  not used, compat    */
    long    st_blocks;
    long    st_ctime;
    long    st_mtime;
    long    st_atime;       /*  not used, compat    */
    long    st_dev;
    short   st_rdev;        /*  not used, compat    */
    long    st_ino;
    short   st_uid;         /*  not used, compat    */
    short   st_gid;         /*  not used, compat    */
    short   st_nlink;       /*  not used, compat    */
};

#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)

extern int stat(const char *, struct stat *);
extern int lstat(const char *, struct stat *);
extern int fstat(int, struct stat *);
extern int readlink(char *, char *, int);

/*
 *  dummy unix compat
 */

#define makedev(maj,min)    (((maj) << 8) | (min))
#define major(rdev)     (unsigned char)((rdev) >> 8)
#define minor(rdev)     (unsigned char)(rdev)

#endif

