
/*
 * $VER: sys/dir.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SYS_DIR_H
#define SYS_DIR_H

#define MAXPATHLEN  1024
#define MAXNAMLEN   256

typedef struct DIR {
    long    am_Private;
} DIR;

struct direct {
    char    *d_name;
    short   d_namlen;
    short   d_reserved;
    long    d_ino;
};

struct dirent {
    char    *d_name;
    short   d_namlen;
    short   d_reserved;
    long    d_ino;
};


DIR *opendir(const char *);
struct direct *readdir(DIR *);
int rewinddir(DIR *);
int closedir(DIR *);

#endif
