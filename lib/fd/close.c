
/*
 *  CLOSE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ioctl.h>
#include <errno.h>
#include <lib/misc.h>

int __InUnixFork;

void
__closeall(void)
{
    short fd;

    for (fd = 0; fd < _IoFDLimit; ++fd)
        close(fd);
}

int
close(fd)
int fd;
{
    _IOFDS *d;
    int r = -1;

    if (d = __getfh(fd)) {
        if (__InUnixFork)
            return(0);

        if (d->fd_FileName)
            free(d->fd_FileName);
        if (d->fd_Exec) {                       /*  special */
            r = (*d->fd_Exec)(d->fd_Fh, IOC_CLOSE, NULL, NULL);
        } else {
            r = 0;
            if ((d->fd_Flags & O_NOCLOSE) == 0)
                Close(d->fd_Fh);
        }
        d->fd_Flags = 0;
        d->fd_Exec  = NULL;
        d->fd_Fh    = NULL;
        d->fd_FileName = NULL;
    }
    return(r);
}

