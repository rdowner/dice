
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

// void 
void VFWritef( BPTR fh, STRPTR format, LONG *argarray );

void
HYPER ## FWritef( BPTR fh, STRPTR format, ... )
{
    va_list va;


    va_start(va, format);
    VFWritef(fh, format, va);
    va_end(va);
}


