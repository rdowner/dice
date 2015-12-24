
/* Intuition tag call */

#include <exec/types.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

APTR 
HYPER ## NewObject( struct IClass *classPtr, UBYTE *classID, unsigned long tag1, ... ) 
{
	return NewObjectA(classPtr, classID, (TagItem *)&tag1 );
}
