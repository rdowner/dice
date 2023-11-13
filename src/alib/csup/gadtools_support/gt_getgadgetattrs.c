
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

LONG
HYPER ## GT_GetGadgetAttrs( struct Gadget *gad, struct Window *win,
        struct Requester *req, Tag tag1, ... )
{
        return GT_GetGadgetAttrsA(gad,win,req,(TagItem *)&tag1);
}

