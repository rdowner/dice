
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

ULONG 
HYPER ## BestModeID( unsigned long tag1Type, ... )
{
	return BestModeIDA((TagItem *)&tag1Type);
}

