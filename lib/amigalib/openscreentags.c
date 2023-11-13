
/*
 *  OpenScreenTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem  TagItem;
typedef struct Screen   Screen;

Screen *
HYPER ## OpenScreenTags(ns, tag1, ...)
struct NewScreen *ns;
Tag tag1;
{
    return(OpenScreenTagList(ns, (TagItem *)&tag1));
}

