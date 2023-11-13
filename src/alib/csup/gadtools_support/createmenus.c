
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

struct Menu *
HYPER ## CreateMenus( struct NewMenu *newmenu, Tag tag1, ... )
{
        return CreateMenusA(newmenu,(TagItem *)&tag1);
}

