
/*
 *  CACHE.C
 *
 *  Implement DICECACHE.LIBRARY access
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <lib/misc.h>
#include <clib/dicecache_protos.h>
#include <clib/exec_protos.h>

typedef struct CacheFh {
    CacheNode	*cfh_Cn;
    long	cfh_Size;
    long	cfh_Pos;
} CacheFh;

void
_MakeCacheFD(d, cn, size)
_IOFDS *d;
void *cn;
long size;
{
    CacheFh *cfh = malloc(sizeof(CacheFh));

    if (cfh)
    {
        d->fd_Fh = (long)cfh;
        d->fd_Exec = _CacheFDIoctl;
        cfh->cfh_Cn = cn;
        cfh->cfh_Size = size;
        cfh->cfh_Pos = 0;
    }
}

_CacheFDIoctl(fh, cmd, arg1, arg2)
long fh;
int cmd;
void *arg1;
void *arg2;
{
    CacheFh *cfh = (CacheFh *)fh;
    long r = 0;

    /*
     *	most common operation
     */

    if (cmd == IOC_READ) {
	void *ptr;
	long n;

	if (ptr = DiceCacheSeek(cfh->cfh_Cn, cfh->cfh_Pos, &n)) {
	    if (n > (long)arg2)
		n = (long)arg2;
	    CopyMem(ptr, arg1, n);
	    cfh->cfh_Pos += n;
	} else {
	    n = -1;
	}
	return(n);
    }

    switch(cmd) {
    case IOC_CLOSE:
	DiceCacheClose(cfh->cfh_Cn);
	free(cfh);
    case IOC_SEEK:
	r = (long)arg1;

	switch((long)arg2) {
	case 1:
	    r += cfh->cfh_Pos;
	    break;
	case 2:
	    r = cfh->cfh_Size - r;
	    break;
	}
	if (r < 0 || r > cfh->cfh_Size)
	    r = -1;
	else
	    cfh->cfh_Pos = r;
	break;
    default:
	r = -1;
	errno = EINVAL;
	break;
    }
    return(r);
}

