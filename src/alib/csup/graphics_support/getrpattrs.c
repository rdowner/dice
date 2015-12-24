
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

void 
HYPER ## GetRPAttrs( struct RastPort *rp, unsigned long tag1Type, ... )
{
	GetRPAttrsA(rp, (TagItem *)&tag1Type );
}
