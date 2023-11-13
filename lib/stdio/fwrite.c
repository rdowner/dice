
/*
 *  fwrite.c
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define buf ((const char *)vbuf)

size_t
fwrite(vbuf, elmsize, elms, fi)
const void *vbuf;
size_t elmsize;
size_t elms;
FILE *fi;
{
    int n = EOF;
    int bytes;

    if (elmsize == 1)
        bytes = elms;
    else if (elms == 1)
        bytes = elmsize;
    else
        bytes = elms * elmsize;

    if (fi == NULL)
        return(-1);
    if (fi->sd_Flags & __SIF_WRITE) {
        if (bytes > fi->sd_WLeft)
            fflush(fi);             /*  also puts us into 'write' mode */

        if (bytes <= fi->sd_WLeft) {
            movmem(buf, fi->sd_WPtr, bytes);
            fi->sd_WLeft -= bytes;
            fi->sd_WPtr += bytes;
            n = bytes;

            if ((fi->sd_Flags & __SIF_IOLBF) && bytes && buf[bytes-1] == '\n')
                fflush(fi);
        } else {
            long w = bytes;

            while (w > 0) {
                n = write(fi->sd_Fd, buf, w);
                if (n <= 0)
                    break;
                fi->sd_Offset += n;
                vbuf = buf + n;
                w -= n;
            }
        }
    }
    if (n < 0)
        fi->sd_Error = EOF;
    if (fi->sd_Error)
        return(fi->sd_Error);
    if (n == bytes)
        return(elms);
    return(n / elmsize);
}

