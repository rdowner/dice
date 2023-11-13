
/*
 *  FTELL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

long
ftell(fi)
FILE *fi;
{
    long n = fi->sd_Offset;

    if (!(fi->sd_Flags & __SIF_FILE))
        return(EOF);

    if (fi->sd_WLeft >= 0)
        return(n + (fi->sd_WPtr - fi->sd_WBuf));
    else if (fi->sd_RLeft >= 0) {
        if (fi->sd_UC >= 0)
            --n;
        return(n - fi->sd_RLeft);
    }
    return(n);
}

