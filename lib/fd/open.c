
/*
 *  OPEN.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <lib/misc.h>
#include <clib/dicecache_protos.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)	path
#endif

extern struct DosLibrary *DOSBase;
extern struct Library *DiceCacheBase;

_IOFDS *
_MakeFD(pfd)
int *pfd;
{
    int fd;
    _IOFDS *d;

    /*
     *	extend limit if required
     */

    for (fd = 0, d = _IoFD; fd < _IoFDLimit; ++fd) {
	if ((d->fd_Flags & O_ISOPEN) == 0)
	    break;
	++d;
    }
    if (fd == _IoFDLimit) {
	short newLimit = fd + 5;
	_IOFDS *fds = malloc(sizeof(_IOFDS) * newLimit);
	if (fds == NULL) {
	    errno = ENOMEM;
	    return(NULL);
	}
	_slow_bzero(fds, sizeof(_IOFDS) * newLimit);
	_slow_bcopy(_IoFD, fds, sizeof(_IOFDS) * fd);
	if (_IoFD != _IoStaticFD)
	    free(_IoFD);
	_IoFD = fds;
	_IoFDLimit = newLimit;
	d = fds + fd;
    }
    if (d)
	_slow_bzero(d, sizeof(*d));
    *pfd = fd;
    return(d);
}


int
open(name, modes, ...)
const char *name;
int modes;
{
    int fd;
    _IOFDS *d = _MakeFD(&fd);

    if (d == NULL)
	return(-1);

    /*
     *	If the cache is enabled attempt to open from the cache
     */

    if (_DiceCacheEnable && (modes & (O_RDWR|O_WRONLY|O_APPEND|O_CREAT|O_TRUNC)) == O_RDONLY) {
	void *cd;
	long siz = 0;

	if (DiceCacheBase && (cd = DiceCacheOpen(name, "r", &siz))) {
	    _MakeCacheFD(d, cd, siz);
	    d->fd_Flags = modes | O_ISOPEN;
	    return(fd);
	}

	/*
	 *  check error code.  If PS_LOCK_FAILED then it was a valid suffix
	 *  but the file does not exist.  There is no need for US to try
	 *  to lock the file *again*.  Since the suffix matched it isn't
	 *  a device and we don't have to try to Open().  In otherwords,
	 *  we save doing an extra Lock AND Open if DiceCache is enabled.
	 */

	if (siz == PS_LOCK_FAILED) {
	    errno = ENOFILE;
	    return(-1);
	}
    }

    /*
     *	If we can't lock then be careful to call Open only once.  It
     *	may be a new file that does not yet exist or a device file
     */

    {
	long lock = Lock(UnixToAmigaPath(name), SHARED_LOCK);
	if (lock == NULL) {
#ifdef NOTDEF
	    if (DOSBase->lib_Version >= 36) {
		if (IsFileSystem(UnixToAmigaPath(name))) {
		    errno = ENOFILE;
		    return(-1);
		}
	    }
	    /*
	    if (IoErr() == ERROR_DEVICE_NOT_MOUNTED) {
		errno = ENOFILE;
		return(-1);
	    }
	    */
#endif
	    /*
	     *	file does not exist, open modes 1006 if create request, else
	     *	try to open modes 1005 (could be a non-file device)
	     */

	    if (modes & O_CREAT)
		d->fd_Fh = Open(UnixToAmigaPath(name), 1006);
	    else
		d->fd_Fh = Open(UnixToAmigaPath(name), 1005);

	    if (d->fd_Fh == NULL) {
		errno = ENOFILE;
		return(-1);
	    }
	    if (modes & O_APPEND)
		Seek(d->fd_Fh, 0L, 1);
	    d->fd_Flags = modes | O_ISOPEN;
	    d->fd_FileName = strdup(name);
	    return(fd);
	}
	UnLock(lock);
    }

    /*
     *	Assume it is a normal file.. we can open/reopen it any
     *	number of times.  Also, the file exists so...
     */

    if ((modes & O_EXCL) && (modes & O_CREAT)) {
	errno = EEXIST;
	return(-1);
    }
    d->fd_Fh = Open(UnixToAmigaPath(name), ((modes & O_TRUNC) ? 1006 : 1005));

    if (d->fd_Fh == NULL && (modes & O_CREAT)) {
	if (modes & O_EXCL) {
	    errno = ENOFILE;
	    return(-1);
	}
	d->fd_Fh = Open(UnixToAmigaPath(name), 1006);
    }

    if (d->fd_Fh) {
	if (modes & O_APPEND)
	    Seek(d->fd_Fh, 0L, 1);
	d->fd_Flags = modes | O_ISOPEN;
	d->fd_FileName = strdup(name);
	return(fd);
    }
    return(-1);
}

