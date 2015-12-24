
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

APTR 
HYPER ## GetVisualInfo( struct Screen *screen, Tag tag1, ... )
{
	return GetVisualInfoA(screen,(TagItem *)&tag1);
}
