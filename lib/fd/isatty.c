
/*
 *  ISATTY.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dos_protos.h>
#include <stdio.h>
#include <fcntl.h>
#include <ioctl.h>
#include <errno.h>

int
isatty(fd)
int fd;
{
    _IOFDS *d;

    if (d = __getfh(fd)) {
        if (d->fd_Exec)
            return((*d->fd_Exec)(d->fd_Fh, IOC_ISATTY, NULL, NULL));
        if (IsInteractive(d->fd_Fh))
            return(1);
        return(0);
    }
    return(-1);
}

