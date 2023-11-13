
/*
 *  READ.C
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

int
read(fd, buf, bytes)
int fd;
void *buf;
unsigned int bytes;
{
    _IOFDS *d;
    int n = -1;

    chkabort();
    if (d = __getfh(fd)) {
        if (!(d->fd_Flags & O_WRONLY)) {
            if (d->fd_Exec)
                return((*d->fd_Exec)(d->fd_Fh, IOC_READ, buf, (void *)bytes));
            n = Read(d->fd_Fh, buf, bytes);
        } else {
            errno = ENOPERM;
        }
    }
    return(n);
}

