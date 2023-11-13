
/*
 *  NewLoadSegTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

typedef struct TagItem   TagItem;

#ifndef HYPER
#define HYPER
#endif

BPTR
HYPER ## NewLoadSegTags(file, tag1, ...)
UBYTE *file;
Tag tag1;
{
    return(NewLoadSeg(file, (TagItem *)&tag1));
}

