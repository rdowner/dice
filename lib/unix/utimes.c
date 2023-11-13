
/*
 *  UNIX/UTIMES.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <time.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

int utime(char *, time_t *);
int utimes(char *, struct timeval *);

int
utime(path, times)
char *path;
time_t *times;
{
    struct timeval ts[2];

    ts[0].tv_secs = times[0];
    ts[0].tv_micro= 0;
    ts[1].tv_secs = times[1];
    ts[1].tv_micro= 0;
    return(utimes(path, ts));
}

int
utimes(path, times)
char *path;
struct timeval *times;
{
    long v[3];
    time_t t = times[1].tv_secs;

    v[0] = (unsigned long)t / 86400;        /*  day */
    v[1] = ((unsigned long)t / 60) % 1440;
    v[2] = ((unsigned long)t % 60) * 50;

    if (SetFileDate(UnixToAmigaPath(path), (struct DateStamp *)v))
        return(0);
    return(-1);
}

