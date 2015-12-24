
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

void 
HYPER ## DrawBevelBox( struct RastPort *rport, long left, long top, long width,
	long height, Tag tag1 )
{
	DrawBevelBoxA(rport,left,top,width,height,(TagItem *)&tag1);
}
