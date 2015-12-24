
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

LONG 
HYPER ## ObtainBestPen( struct ColorMap *cm, unsigned long r, unsigned long g,
	unsigned long b, unsigned long tag1Type , ... )
{
	return ObtainBestPenA(cm, r, g, b, (TagItem *)&tag1Type);
}
