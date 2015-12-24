
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

BOOL 
HYPER ## VideoControlTags( struct ColorMap *colorMap, unsigned long tag1Type, ... )
{
	return VideoControl(colorMap,(TagItem *)&tag1Type);
}
