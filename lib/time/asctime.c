
/*
 *  ASCTIME.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <time.h>

char *
asctime(tm)
const struct tm *tm;
{
    static char TBuf[32];

    strftime(TBuf, sizeof(TBuf), "%c\n", tm);
    return(TBuf);
}

