
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

ULONG SetDTAttrs( Object *o, struct Window *win, struct Requester *req,Tag Tag1, ... )
{
	return SetDTAttrsA(o,win,req,(TagItem *)&Tag1);
}
