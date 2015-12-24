
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

ULONG 
HYPER ## ExtendFontTags( struct TextFont *font, unsigned long tag1Type, ... )
{
	return ExtendFont(font, (TagItem *)&tag1Type);
}
