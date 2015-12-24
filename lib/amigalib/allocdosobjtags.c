
/*
 *  AllocDosObjectTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

APTR
HYPER ## AllocDosObjectTags(type, tag1, ...)
unsigned long type;
Tag tag1;
{
    return(AllocDosObject(type, (TagItem *)&tag1));
}

