
/* Intuition tag call */

#include <exec/types.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

ULONG 
HYPER ## SetAttrs( APTR object, unsigned long tag1, ... )
{
	return SetAttrsA( object, (TagItem *)&tag1 );
}
