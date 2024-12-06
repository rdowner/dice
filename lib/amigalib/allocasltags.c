
/*
 *  AllocAslRequestTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <clib/asl_protos.h>
#include <stdarg.h>

#ifndef HYPER
#define HYPER
#endif

APTR
HYPER ## AllocAslRequestTags(ULONG type, ...)
{
    va_list va;
    va_start(va, type);
    APTR ret = AllocAslRequest(type, (CONST struct TagItem *)va);
    va_end(va);
    return ret;
}

