
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

void 
HYPER ## GT_SetGadgetAttrs( struct Gadget *gad, struct Window *win,
        struct Requester *req, Tag tag1, ... )
{
        GT_SetGadgetAttrsA(gad,win,req,(TagItem *)&tag1);
}

