
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

struct ExtSprite *
HYPER ## AllocSpriteData( struct BitMap *bm, unsigned long tag1Type, ... )
{
	return AllocSpriteDataA( bm, (TagItem *)&tag1Type );
}
