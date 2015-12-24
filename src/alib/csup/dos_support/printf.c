
/*
 *  PRINTF.C	dos.library var-args interface to VPrintf()
 */

#include <stdio.h>
#include <stdarg.h>
#include <exec/types.h>

/* #include <clib/dos_protos.h> */

extern LONG VPrintf( STRPTR format, APTR argarray );

LONG
Printf(unsigned char *ctl, long arg1, ...)
{
    va_list va;
    int n;

    va_start(va, ctl);
    n = VPrintf(ctl, va);
    va_end(va);
    return(n);
}

