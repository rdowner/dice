
/*
 *  SystemTags
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

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

LONG
HYPER ## SystemTags(cmd, tag1, ...)
UBYTE *cmd;
unsigned long tag1;
{
    return(SystemTagList(cmd, (TagItem *)&tag1));
}

