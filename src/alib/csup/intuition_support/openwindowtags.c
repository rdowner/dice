
/*
 *  OpenWindowTags
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

