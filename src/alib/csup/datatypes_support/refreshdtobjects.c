
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

void RefreshDTObjects( Object *o, struct Window *win, struct Requester *req,Tag Tag1, ... )
{
        return RefreshDTObjectA(o,win,req,(TagItem *)&Tag1);
}
