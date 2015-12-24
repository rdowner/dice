
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

ULONG DoDTMethod( Object *o, struct Window *win, struct Requester *req,unsigned long data, ... )
{
	return DoDTMethodA(o,win,req,(Msg)&data);
}
