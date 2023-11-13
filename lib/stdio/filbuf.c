
/*
 *  _FILBUF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int
_filbuf(fi)
FILE *fi;
{
    int n;
    int error = 0;

    chkabort();
    if (fi && (fi->sd_Flags & __SIF_OPEN) && fi->sd_RLeft <= 0) {
        /*
         *  if stdin flush stdout
         */
        if (fi == stdin)
            fflush(stdout);
        /*
         *  if a normal file flush pending write data
         */
        if (fi->sd_WLeft >= 0 && (fi->sd_Flags & __SIF_FILE)) {
            fflush(fi);
            fi->sd_WLeft = -1;
            fi->sd_RLeft = 0;
        }
        if (fi->sd_BufSiz && fi->sd_RBuf == NULL) {
            fi->sd_RBuf = malloc(fi->sd_BufSiz);
            fi->sd_Flags |= __SIF_MYBUF;
            if (fi->sd_RBuf == NULL)     /*  if couldn't malloc, make unbuf'd */
                fi->sd_BufSiz = 0;
        }
        if (fi->sd_BufSiz) {            /*  else we might be unbuffered!     */
            n = read(fi->sd_Fd, fi->sd_RBuf, fi->sd_BufSiz);
            if (n < 0) {
                if (errno != EAGAIN) {
                    fi->sd_Error = EOF;
                    error = EOF;
                }
            } else {
                fi->sd_RLeft = n;
                fi->sd_Offset += n;
                if (n == 0) {
                    error = EOF;
                    fi->sd_Flags |= __SIF_EOF;
                } else {
                    fi->sd_Flags &= ~__SIF_EOF;
                }
            }
        } else {
            fi->sd_RLeft = 0;   /*  unbuffered */
        }
        fi->sd_RPtr = fi->sd_RBuf;
    }
    return(error);
}

