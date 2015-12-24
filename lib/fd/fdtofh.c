
/*
 *  FDTOFH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ioctl.h>
#include <errno.h>

void *
fdtofh(fd)
int fd;
{
    _IOFDS *d;

    if (d = __getfh(fd)) {
	if (d->fd_Exec)
	    return(NULL);
	return((void *)d->fd_Fh);
    }
    return(NULL);
}

