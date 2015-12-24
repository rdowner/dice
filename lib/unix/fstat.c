
/*
 *  FSTAT call (only works properly under 2.0)
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <lib/misc.h>

extern struct DosLibrary *DOSBase;

typedef struct FileInfoBlock	FileInfoBlock;

fstat(fd, xstat)
int fd;
struct stat *xstat;
{
    __aligned FileInfoBlock fib;
    int r = -1;
    _IOFDS *d;

    clrmem(xstat, sizeof(*xstat));
    if (d = __getfh(fd)) {	/*  2.0 */
	if (DOSBase->dl_lib.lib_Version >= 36) {
	    if (ExamineFH(d->fd_Fh, (FileInfoBlock *) &fib)) {
		xstat->st_size = fib.fib_Size;
		xstat->st_ino =  fib.fib_DiskKey;
		xstat->st_dev = (long)((struct FileHandle *)BADDR(d->fd_Fh))->fh_Type;
		xstat->st_mode = (fib.fib_DirEntryType > 0) ? S_IFDIR : S_IFREG;
		xstat->st_ctime = xstat->st_mtime = fib.fib_Date.ds_Days * (1440 * 60) +
						fib.fib_Date.ds_Minute * 60 +
						fib.fib_Date.ds_Tick / 50 + _TimeCompensation;
		if ((fib.fib_Protection & 8) == 0)
		    xstat->st_mode |= S_IREAD;
		if ((fib.fib_Protection & 4) == 0)
		    xstat->st_mode |= S_IWRITE;
		if ((fib.fib_Protection & 2) == 0)
		    xstat->st_mode |= S_IEXEC;
		if (fib.fib_Protection & 0x40)
		    xstat->st_mode |= S_IEXEC;

		r = 0;
	    }
	    else {
		// if packet not supported it means we are talking to
	        // a handler, so we have to fake it from flags
		if(IoErr() == ERROR_ACTION_NOT_KNOWN ) {
		    if(((FILE *)fd)->sd_Flags & __SIF_READ)xstat->st_mode |= S_IREAD;
		    if(((FILE *)fd)->sd_Flags & __SIF_WRITE)xstat->st_mode |= S_IWRITE;
		    r = 0;
		}
	    }
	} else {		/*  1.3 */
	    r = stat(d->fd_FileName, xstat);
	}
	/*
	 *  extended size will not show up in examine if we have written
	 *  the active handle.
	 */
	{
	    long pos = Seek(d->fd_Fh, 0L, 0);
	    long siz;
	    Seek(d->fd_Fh, 0L, 1);
	    siz = Seek(d->fd_Fh, pos, -1);
	    if (xstat->st_size < siz)
		xstat->st_size = siz;
	}
    }
    if (r < 0)errno = ENOENT;
    return(r);
}

#ifdef TEST

main(ac, av)
char *av[];
{
    struct stat xstat;
    short i;
    int fd;

    for (i = 1; i < ac; ++i) {
	int fd = open(av[i], O_RDONLY);
	int r = fstat(fd, &xstat);
	printf("fd = %d r = %d fs=%d ti=%08lx\n", fd, r, xstat.st_size, xstat.st_ctime);
    }
    return(0);
}

#endif
