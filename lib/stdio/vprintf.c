
/*
 *  VPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

int
vprintf(ctl, va)
const char *ctl;
va_list va;
{
    int error;

    error = _pfmt(ctl, va, fwrite, stdout);
    return(error);      /*  count/error     */
}

