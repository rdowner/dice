
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

BOOL 
HYPER ## LayoutMenus( struct Menu *firstmenu, APTR vi, Tag tag1, ... )
{
        return LayoutMenusA(firstmenu,vi,(TagItem *)&tag1);
}
