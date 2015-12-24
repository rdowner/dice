
/*
 *  VFPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

int
vfprintf(fi, ctl, va)
FILE *fi;
const char *ctl;
va_list va;
{
    int error;

    error = _pfmt(ctl, va, fwrite, fi);
    va_end(va);
    return(error);      /*  count/error */
}

