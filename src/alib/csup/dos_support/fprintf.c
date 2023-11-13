
/* AmigaDOS tag call */

#include <exec/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

extern LONG VFPrintf( BPTR fh, STRPTR format, LONG *argarray );

LONG
HYPER ## LONG FPrintf( BPTR fh, STRPTR format, ... )
{
    va_list va;
    int n;

    va_start(va, format);
    n = VFPrintf(fh, format, va);
    va_end(va);
    return(n);
}


