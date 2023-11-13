
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

BOOL 
HYPER ## LayoutMenuItems( struct MenuItem *firstitem, APTR vi, Tag tag1, ... )
{
        return LayoutMenuItemsA(firstitem,vi,(TagItem *)&tag1);
}
