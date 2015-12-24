
/*
 *  PRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  tag'd to here allowing math library to override pfmt.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

int
printf(ctl, ...)
const char *ctl;
{
    int error;
    va_list va;

    va_start(va, ctl);
    error = _pfmt(ctl, va, fwrite, stdout);
    va_end(va);
    return(error);      /*  count/error */
}

