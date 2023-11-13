
/*
 *  FCLOSE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dos_protos.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib/misc.h>

int
fclose(fi)
FILE *fi;
{
    int error = EOF;

    if (fi && (fi->sd_Flags & __SIF_OPEN)) {
        error = __fclose(fi);
        if (fi->sd_Name)
            free(fi->sd_Name);
        if ((fi->sd_Flags & __SIF_NOFREE) == 0) {
            *fi->sd_Prev = fi->sd_Next;
            if (fi->sd_Next)
                fi->sd_Next->sd_Prev = fi->sd_Prev;
            free(fi);
        }
    }
    return(error);
}

int
__fclose(fi)
FILE *fi;
{
    int error;
    int n = fi->sd_BufSiz - fi->sd_WLeft;

    if (fi->sd_WLeft >= 0 && fi->sd_WBuf && n) {
        if (write(fi->sd_Fd, fi->sd_WBuf, n) != n)
            fi->sd_Error = EOF;
    }
    error = fi->sd_Error;
    if (fi->sd_Flags & __SIF_MYBUF) {
        if (fi->sd_RBuf) {
            free(fi->sd_RBuf);
            fi->sd_RBuf = NULL;
        }
        if (fi->sd_WBuf) {
            free(fi->sd_WBuf);
            fi->sd_WBuf = NULL;
        }
    }
    close(fi->sd_Fd);
    if (fi->sd_Flags & __SIF_REMOVE)
        DeleteFile(fi->sd_Name);
    fi->sd_Flags &= ~(__SIF_OPEN|__SIF_READ|__SIF_WRITE);
    return(error);
}

