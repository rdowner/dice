
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

LONG 
HYPER ## SetAmigaGuideContext( APTR cl, unsigned long id, Tag tag1, ... )
{
        return SetAmigaGuideContextA(cl,id,(TagItem *)&tag1);
}
