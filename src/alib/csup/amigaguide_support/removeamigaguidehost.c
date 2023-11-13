
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

LONG 
HYPER ## RemoveAmigaGuideHost( APTR hh, Tag tag1, ... )
{
        return RemoveAmigaGuideHostA(hh,(TagItem *)&tag1);
}
