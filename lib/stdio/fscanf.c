
/*
 *  FSCANF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

int
fscanf(fi, ctl, ...)
FILE *fi;
const char *ctl;
{
    int error;
    int cnt;
    va_list va;

    va_start(va, ctl);
    error = _sfmt(ctl, va, fgetc, ungetc, fi, &cnt);
    va_end(va);
    if (error)
        return(error);
    return(cnt);
}

