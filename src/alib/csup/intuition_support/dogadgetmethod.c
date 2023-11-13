
/* Intuition tag call */

#include <exec/types.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

ULONG 
HYPER ## DoGadgetMethod( struct Gadget *gad, struct Window *win,struct Requester *req, unsigned long MethodID, ... )
{
        return DoGadgetMethodA(gad, win, req, (Msg)&MethodID );
}
