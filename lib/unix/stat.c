
/*
 *  STAT call
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <lib/misc.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)	path
#endif


typedef struct FileInfoBlock	FileInfoBlock;

stat(name, stat)
const char *name;
struct stat *stat;
{
    __aligned FileInfoBlock fib;
    BPTR lock;
    int r = -1;

    clrmem(stat, sizeof(*stat));
    /*
     *	If lock fails find file via its parent directory.  This is for
     *	unix compatibility because you can stat an open write file in unix.
     */

    fib.fib_FileName[0] = 0;

    if ((lock = Lock(UnixToAmigaPath(name), SHARED_LOCK)) == NULL) {
	char *buf = strdup(UnixToAmigaPath(name));
	char *ptr;
	char sk = 0;

	for (ptr = buf + strlen(buf); ptr >= buf && *ptr != ':' && *ptr != '/'; --ptr);
	if (ptr < buf || *ptr == ':') {
	    ++ptr;
	    sk = *ptr;
	}
	*ptr = 0;
	lock = Lock(buf, SHARED_LOCK);
	if (sk)
	    *ptr = sk;
	else
	    ++ptr;

	if (lock == NULL) {
	    free(buf);
	    errno = ENOENT;
	    return(-1);
	}
	if (Examine(lock, &fib)) {
	    while (ExNext(lock, &fib)) {
#ifdef TEST
		printf("Compare '%s' '%s'\n", ptr + 1, fib.fib_FileName);
#endif
		if (stricmp(ptr, fib.fib_FileName) == 0) {
		    r = 0;
		    break;
		}
	    }
	}
	free(buf);
    } else {
	if (Examine(lock, &fib))
	    r = 0;
    }
    if (lock == NULL) {
	errno = ENOENT;
	return(-1);
    }
    if (r >= 0) {
	stat->st_size = fib.fib_Size;
	stat->st_ino = (long)((struct FileLock *)BADDR(lock))->fl_Key;
	stat->st_dev = (long)((struct FileLock *)BADDR(lock))->fl_Task;
	stat->st_mode = (fib.fib_DirEntryType > 0) ? S_IFDIR : S_IFREG;
	stat->st_ctime = stat->st_mtime = fib.fib_Date.ds_Days * (1440 * 60) +
					fib.fib_Date.ds_Minute * 60 +
					fib.fib_Date.ds_Tick / 50 + _TimeCompensation;
	if ((fib.fib_Protection & 8) == 0)
	    stat->st_mode |= S_IREAD;
	if ((fib.fib_Protection & 4) == 0)
	    stat->st_mode |= S_IWRITE;
	if ((fib.fib_Protection & 2) == 0)
	    stat->st_mode |= S_IEXEC;
	if (fib.fib_Protection & 0x40)
	    stat->st_mode |= S_IEXEC;
    }
    UnLock(lock);
    if (r < 0)
	errno = ENOENT;
    return(r);
}

#ifdef TEST

#include <fcntl.h>

main(ac, av)
char *av[];
{
    struct stat xstat;
    short i;

    for (i = 1; i < ac; ++i) {
	int r = stat(av[i], &xstat);
	printf("r = %d fs=%d ti=%08lx\n", r, xstat.st_size, xstat.st_ctime);
    }
    return(0);
}

#endif
