
/*
 *  FOPEN.C             fopen freopen fdopen
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  modes:      a[b][+]     append [binary] [+ignored]
 *              r[b][+]     read [binary] [update-allow writes]
 *              w[b][+]     write [binary] [uupdate-allow reads]
 *
 *              a always appends- write only
 *              r file must exist '+' means can write too.
 *              w file always create/truncate
 *              b binary (ignored)
 *
 *              F INTERNAL, user should never specify.  Used by fdopen.
 */

#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <lib/misc.h>

FILE *
fopen(name, modes)
const char *name;
const char *modes;
{
    FILE *fi = malloc(sizeof(FILE));

    if (fi) {
        _slow_bzero(fi, sizeof(FILE));
        if (freopen(name, modes, fi) == NULL) {
            free(fi);
            fi = NULL;
        }
    }
    return(fi);
}

FILE *
fdopen(fd, modes)
int fd;
const char *modes;
{
    char buf[16];

    if (strlen(modes) < 14) {
        buf[0] = 'F';
        strcpy(buf + 1, modes);
        return(fopen((char *)fd, buf));
    }
    return(NULL);
}

FILE *
freopen(name, modes, fi)
const char *name;
const char *modes;
FILE *fi;
{
    short fdmode = 0;
    short fimode = 0;
    short fdOpen = 0;

    if (fi == stdin || fi == stdout || fi == stderr)
        fimode |= __SIF_NOFREE;

    {
        char c;
        while (c = *modes) {
            if (c == 'r') {
                fdmode |= O_RDONLY;
                fimode |= __SIF_READ;
            }
            if (c == 'w') {
                fdmode |= O_WRONLY | O_CREAT | O_TRUNC;
                fimode |= __SIF_WRITE;
            }
            if (c == '+') {
                fdmode |= O_RDWR;
                fdmode &= ~(O_RDONLY|O_WRONLY);
                fimode |= __SIF_READ | __SIF_WRITE;
            }
            if (c == 'b') {
                fdmode |= O_BINARY;
                fimode |= __SIF_BINARY;
            }
            if (c == 'a') {
                fdmode |= O_CREAT | O_APPEND;
                fimode |= __SIF_WRITE | __SIF_APPEND;
            }
            if (c == 'F')       /*  INTERNAL    */
                fdOpen = 1;
            if (c == 'C')       /*  INTERNAL    */
                fimode |= __SIF_REMOVE;
            ++modes;
        }
    }
    if (fi) {
        if (fi->sd_Flags & __SIF_OPEN) {
            __fclose(fi);
            if ( (fimode & __SIF_REMOVE) && (fi->sd_Name))free(fi->sd_Name);
            // disconnect file handle
            if ((fi->sd_Flags & __SIF_NOFREE) == 0) {
                *fi->sd_Prev = fi->sd_Next;
                if (fi->sd_Next)fi->sd_Next->sd_Prev = fi->sd_Prev;
            }
        }
        _slow_bzero(fi, sizeof(FILE));
        if (fdOpen)
            fi->sd_Fd = (int)name;
        else
            fi->sd_Fd = open(name, fdmode, 0666);

        if (fi->sd_Fd >= 0) {
            fi->sd_UC = -1;

            if ((fimode & __SIF_NOFREE) == 0) {
                fi->sd_Next = _Iod;
                fi->sd_Prev = &_Iod;
                if (fi->sd_Next)
                    fi->sd_Next->sd_Prev = &fi->sd_Next;
                _Iod = fi;
            }
            {
                long pos = lseek(fi->sd_Fd, 0, 1);

                if (pos >= 0) {
                    fimode |= __SIF_FILE;
                    fi->sd_Offset = pos;
                }
            }

            fi->sd_BufSiz = _bufsiz;
            fi->sd_Flags = __SIF_OPEN | fimode;
            fi->sd_RLeft = -1;
            fi->sd_WLeft = -1;
            if (fimode & __SIF_REMOVE)
                fi->sd_Name = name;     /*  name assumed to be malloc'd */
        } else {
            return(NULL);
        }
    }
    return(fi);
}

