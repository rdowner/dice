

/*
 *  GETFH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

_IOFDS *
__getfh(ifd)
int ifd;
{
    _IOFDS *d;
    short fd = ifd;

    if(ifd == (int)stdin)d = &_IoStaticFD[0];
    else if (ifd == (int)stdout)d = &_IoStaticFD[1];
    else if (ifd == (int)stderr)d = &_IoStaticFD[2];
    else if ((unsigned)fd >= _IoFDLimit) {
	errno = EBADF;
	return(NULL);
    }
    else d = _IoFD + fd;

    if ((d->fd_Flags & O_ISOPEN) == 0) {
	errno = EBADF;
	return(NULL);
    }
    return(d);
}


