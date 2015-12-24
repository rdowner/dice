
/*
 *  IOCTL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ioctl.h>

long
ioctl(fd, req, arg1)
int fd;
int req;
void *arg1;
{
    _IOFDS *d;

    if (d = __getfh(fd)) {
	switch(req) {
	case IOC_MODES|IOF_GET:
	    if (arg1)
		*(int *)arg1 = d->fd_Flags & ~O_INTERNAL;
	    break;
	case IOC_MODES|IOF_SET:
	    d->fd_Flags = (d->fd_Flags & O_INTERNAL) | (*(int *)arg1 & ~O_INTERNAL);
	    break;
	case IOC_GETDESC:
	    return((long)d->fd_Fh);
	}

	if (d->fd_Exec)
	    return((*d->fd_Exec)(d->fd_Fh, req, arg1, NULL));

	/*
	 *  normal file descriptor
	 */

	switch(req) {
	case IOC_CEXEC|IOF_GET:
	    if (arg1)
		*(int *)arg1 = (d->fd_Flags & O_CEXEC) ? 1 : 0;
	    return(0);
	case IOC_CEXEC|IOF_SET:
	    if (*(int *)arg1)
		d->fd_Flags |= O_CEXEC;
	    else
		d->fd_Flags &= ~O_CEXEC;
	    return(0);
	case IOC_MODES|IOF_GET:
	    return(0);
	case IOC_MODES|IOF_SET:
	    return(0);
	case IOC_DOMAIN:
	    return(IODOM_AMIGADOS);
	}
	errno = EINVAL;
    }
    return(-1);
}

