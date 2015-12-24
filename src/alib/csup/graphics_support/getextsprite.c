
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

LONG 
HYPER ## GetExtSprite( struct ExtSprite *ss, unsigned long tag1Type, ... )
{
	return GetExtSpriteA( ss, (TagItem *)&tag1Type );
}
