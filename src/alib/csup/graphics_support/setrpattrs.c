
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

void 
HYPER ## SetRPAttrs( struct RastPort *rp, unsigned long tag1Type, ... )
{
	SetRPAttrsA(rp, (TagItem *)&tag1Type );
}
