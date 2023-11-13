
/* Intuition tag call */

#include <exec/types.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

ULONG 
HYPER ## SetGadgetAttrs( struct Gadget *gadget, struct Window *window,struct Requester *requester, unsigned long tag1, ... )
{
        return SetGadgetAttrsA( gadget, window,requester, (TagItem *)&tag1 );
}
