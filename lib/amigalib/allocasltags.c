
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

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

APTR
HYPER ## AllocAslRequestTags(type, tag1, ...)
unsigned long type;
Tag tag1;
{
    return(AllocAslRequest(type, (TagItem *)&tag1));
}

