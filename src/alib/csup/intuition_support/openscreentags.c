
/*
 *  OpenScreenTags
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	TagItem;
typedef struct Screen	Screen;

Screen *
HYPER ## OpenScreenTags(ns, tag1, ...)
struct NewScreen *ns;
Tag tag1;
{
    return(OpenScreenTagList(ns, (TagItem *)&tag1));
}

