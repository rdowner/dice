
/*
 *  OpenWindowTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem  TagItem;
typedef struct Window   Window;

Window *
HYPER ## OpenWindowTags(nw, tag1, ...)
struct NewWindow *nw;
Tag tag1;
{
    return(OpenWindowTagList(nw, (TagItem *)&tag1));
}

