
/*
 *  FREAD.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  fread does not return EOF unless an error occurs.  fread returns
 *  0 or < elms when the end of file is reached.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/misc.h>

#define buf ((char *)vbuf)

size_t
fread(vbuf, elmsize, elms, fi)
void *vbuf;
size_t elmsize;
size_t elms;
FILE *fi;
{
    int bytes;
    int origBytes;

    if (elmsize == 1)
        bytes = elms;
    else if (elms == 1)
        bytes = elmsize;
    else
        bytes = elms * elmsize;

    origBytes = bytes;

    if (fi && (fi->sd_Flags & __SIF_READ) && !fi->sd_Error) {
        int n = 0;      /*  actual read */

        if (bytes == 0 || (fi->sd_Flags & __SIF_EOF))
            return(0);

        if (fi->sd_UC >= 0) {       /*  ungotten character */
            *buf = fi->sd_UC;
            vbuf = buf + 1;
            fi->sd_UC = -1;
            --bytes;
            ++n;
        }
        if (fi->sd_RLeft > 0) {
            int nn = (fi->sd_RLeft > bytes) ? bytes : fi->sd_RLeft;

            movmem(fi->sd_RPtr, buf, nn);
            fi->sd_RPtr += nn;
            fi->sd_RLeft -= nn;
            vbuf = buf + nn;
            bytes -= nn;
            n += nn;
        }
        if (bytes) {
            if (bytes < fi->sd_BufSiz) {
                if (_filbuf(fi))        /*  on no more data return n    */
                    goto skip;
            }
            if (fi->sd_RLeft <= 0) {    /*  unbuffered read or too-large read */
                int nn;

                nn = read(fi->sd_Fd, buf, bytes);
                if (nn < 0) {
                    if (n == 0)
                        n = -1;
                } else {
                    if (nn == 0 && n == 0) {
                        if (fi->sd_Flags & __SIF_EOF)
                            n = -1;
                        fi->sd_Flags |= __SIF_EOF;
                    }
                    fi->sd_Offset += nn;
                    n += nn;
                }
            } else {
                if (fi->sd_RLeft < bytes)
                    bytes = fi->sd_RLeft;
                movmem(fi->sd_RPtr, buf, bytes);
                fi->sd_RPtr += bytes;
                fi->sd_RLeft -= bytes;
                n += bytes;
            }
        }
skip:
        if (n < 0) {
            fi->sd_Error = EOF;
            fi->sd_Flags |= __SIF_EOF;  // make sure this flag is set too
        }
        if (fi->sd_Error)
            return(fi->sd_Error);
        if (n == origBytes)
            return(elms);
        if (n == 0)
            return(0);
        return(n / elmsize);
    }
    return(0);
}

