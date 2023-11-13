
/*
 *  CHDIR.C
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

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

typedef struct FileInfoBlock FIB;

static BPTR OrigDir;

static void
chdir_exit()
{
    if (OrigDir) {
        UnLock(CurrentDir(OrigDir));
        OrigDir = NULL;
    }
}

int
chdir(path)
const char *path;
{
    int r = -1;
    BPTR lock;

    if (lock = Lock(UnixToAmigaPath(path), SHARED_LOCK)) {
        FIB *fib;

        if (fib = malloc(sizeof(FIB))) {
            if (Examine(lock, fib) && fib->fib_DirEntryType > 0) {
                r = 0;
                lock = CurrentDir(lock);
                if (OrigDir == NULL) {
                    OrigDir = lock;
                    lock = NULL;
                    atexit(chdir_exit);
                }
            }
            free(fib);
        }
        if (lock)
            UnLock(lock);
    }
    return(r);
}


