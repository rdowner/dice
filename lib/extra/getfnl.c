
/*
 *  GETFNL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <lib/misc.h>

typedef struct FileInfoBlock FIB;

int
getfnl(pat, buf, bufSize, attr)
const char *pat;
char *buf;
size_t bufSize;
int attr;               /*  0 = files, 1 = files & dirs */
{
    FIB *fib = malloc(sizeof(FIB));
    int r = -1;
    void *wildNode = NULL;

    errno = 0;

    _SetWildStack(2048);

    --bufSize;      /*  final \0 at end */

    if (fib) {
        const char *ptr;
        BPTR lock = NULL;

        for (ptr = pat + strlen(pat); ptr >= pat; --ptr) {
            if (*ptr == '/' || *ptr == ':')
                break;
        }
        ++ptr;      /*  points to just after the last / or :,   */
                    /*  or to the beginning if no / or :        */
        /*
         *  can't modify a const string !
         */
        wildNode = _ParseWild(ptr, strlen(ptr));

        {
            short len = ptr - pat;
            char *hdr = malloc(len + 1);

            if (hdr)
            {
                strncpy(hdr, pat, len);
                hdr[len] = 0;

                lock = Lock(hdr, SHARED_LOCK);
                free(hdr);
            }
        }

        if (lock) {
            if (Examine(lock, fib)) {
                r = 0;
                while (ExNext(lock, fib)) {
                    short len;
                    short prelen = ptr - pat;

                    if (attr == 0 && fib->fib_DirEntryType > 0)
                        continue;
                    if (_CompWild(fib->fib_FileName, wildNode, NULL) < 0)
                        continue;
                    if (errno)
                        break;
                    len = strlen(fib->fib_FileName) + prelen + 1;
                    if (len > bufSize) {
                        r = -1;
                        break;
                    }
                    strncpy(buf, pat, prelen);
                    strcpy(buf + prelen, fib->fib_FileName);
                    buf += len;
                    bufSize -= len;
                    ++r;
                }
            }
            UnLock(lock);
        }
        free(fib);
    }
    _FreeWild(wildNode);
    if (bufSize > 0)
        *buf = 0;
    if (errno)
        r = -1;
    return(r);
}

