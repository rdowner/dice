
/* Workbench tag call */

#include <exec/types.h>
#include <clib/wb_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

struct AppMenuItem *
HYPER ## AddAppMenuItem( unsigned long id, unsigned long userdata,
        UBYTE *text, struct MsgPort *msgport, Tag tag1, ... )
{
        return AddAppMenuItemA(id,userdata,text,msgport,(TagItem *)&tag1);
}

