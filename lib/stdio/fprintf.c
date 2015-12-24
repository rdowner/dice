
/*
 *  FPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

int
fprintf(fi, ctl, ...)
FILE *fi;
const char *ctl;
{
    int error;
    va_list va;

    va_start(va, ctl);
    error = _pfmt(ctl, va, fwrite, fi);
    va_end(va);
    return(error);      /*  count/error */
}

