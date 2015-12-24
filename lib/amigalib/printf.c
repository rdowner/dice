
/*
 *  PRINTF.C	dos.library var-args interface to VPrintf()
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <exec/types.h>
#include <clib/dos_protos.h>

LONG
Printf(unsigned char *ctl, ...)
{
    va_list va;
    int n;

    va_start(va, ctl);
    n = VPrintf(ctl, va);
    va_end(va);
    return(n);
}

