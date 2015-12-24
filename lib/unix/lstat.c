
/*
 *  LSTAT call
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#define AmigaToUnixPath(path)   path
#endif

typedef struct FileInfoBlock	FileInfoBlock;
typedef struct Process		Process;
typedef struct DevProc		DevProc;

extern struct DosLibrary *DOSBase;

lstat(name, st)
const char *name;
struct stat *st;
{
    char buf[256];

    if (readlink(name, buf, sizeof(buf)) >= 0) {
	clrmem(st, sizeof(*st));

	st->st_mode = S_IFLNK | S_IREAD | S_IWRITE;
	st->st_ctime = st->st_mtime = time(NULL);
	return(0);
    } else {
	return(stat(UnixToAmigaPath(name), st));
    }
}

int
readlink(path, name, max)
char *path;
char *name;
int max;
{
    int r = -1;

    if (DOSBase->dl_lib.lib_Version >= 37) {
	DevProc *dp = NULL;
	DevProc *dp2;
	short failsafe = 20;

	while (dp2 = GetDeviceProc(UnixToAmigaPath(path), dp)) {
	    dp = dp2;
	    if (ReadLink(dp->dvp_Port, dp->dvp_Lock, UnixToAmigaPath(path), name, max)) {
		strcpy(name, AmigaToUnixPath(name));
		r = strlen(name);
		break;
	    }
	    if (--failsafe == 0)
		break;
	    if ((dp->dvp_Flags & DVPF_ASSIGN) == 0)
		break;
	}
	FreeDeviceProc(dp);
    }
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
	int r = lstat(av[i], &xstat);
	printf("mode r = %d fs=%d ti=%08lx mode=%08lx\n", r, xstat.st_size, xstat.st_ctime, xstat.st_mode);
    }
    return(0);
}

#endif

