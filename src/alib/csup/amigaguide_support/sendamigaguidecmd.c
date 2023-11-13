
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

LONG 
HYPER ## SendAmigaGuideCmd( APTR cl, STRPTR cmd, Tag tag1, ... )
{
        return SendAmigaGuideCmdA(cl,cmd,(TagItem *)&tag1);
}
