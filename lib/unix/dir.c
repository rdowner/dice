
/*
 *  OPENDIR.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

typedef struct FileInfoBlock FileInfoBlock;

typedef struct {
    struct DHan     *dh_Next;
    FileInfoBlock   dh_Fib;
    BPTR	    dh_Lock;
    struct direct   dh_Direct;
} DHan;

DHan *DHBase = NULL;

static __autoexit void
dir_exit()
{
    while (DHBase)
	closedir((DIR *)DHBase);
}


DIR *
opendir(path)
const char *path;
{
    DHan *dh = NULL;
    long lock;

    if (lock = Lock(UnixToAmigaPath(path), SHARED_LOCK)) {
	if (dh = malloc(sizeof(DHan))) {
	    dh->dh_Lock = lock;
	    if (rewinddir((DIR *)dh) < 0) {
		closedir((DIR  *)dh);
		dh = NULL;
	    }
	    dh->dh_Next = (struct DHan *)DHBase;
	    DHBase= dh;
	} else {
	    UnLock(lock);
	}
    }
    return(dh);
}

struct direct *
readdir(di)
DIR *di;
{
    DHan *dh = (DHan *)di;

    if (ExNext(dh->dh_Lock, &dh->dh_Fib)) {
	dh->dh_Direct.d_name = dh->dh_Fib.fib_FileName;
	dh->dh_Direct.d_namlen = strlen(dh->dh_Fib.fib_FileName);
	return(&dh->dh_Direct);
    }
    return(NULL);
}

int
rewinddir(di)
DIR *di;
{
    DHan *dh = (DHan *)di;
    int r = -1;

    if (Examine(dh->dh_Lock, &dh->dh_Fib)) {
	if (dh->dh_Fib.fib_DirEntryType > 0)
	    r = 0;
    }
    return(r);
}

int
closedir(di)
DIR *di;
{
    DHan *dh;
    DHan **dhp;
    int r = -1;

    for (dhp = &DHBase; dh = *dhp; dhp = (DHan**)&dh->dh_Next)
     {
	if (dh == (DHan *)di) {
	    *dhp = (DHan *)dh->dh_Next;
	    if (dh->dh_Lock)
		UnLock(dh->dh_Lock);
	    free(dh);
	    r = 0;
	    break;
	}
    }
    return(r);
}

#ifdef TEST

int
main(ac, av)
char *av[];
{
    DIR *dh;

    if (dh = opendir(av[1])) {
	struct direct *en;
	while (en = readdir(dh)) {
	    printf("READ: %s\n", en->d_name);
	}
	closedir(dh);
    }
    return(0);
}

#endif
