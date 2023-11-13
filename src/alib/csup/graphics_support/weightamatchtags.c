
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

WORD 
HYPER ## WeighTAMatchTags( struct TextAttr *reqTextAttr, 
        struct TextAttr *targetTextAttr, unsigned long tag1Type, ... )
{
        return WeighTAMatch(reqTextAttr,targetTextAttr, (TagItem *)&tag1Type);
}

