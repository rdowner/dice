
/* AmigaDOS tag call */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

// LONG DoPkt( struct MsgPort *port, long action, long arg1, long arg2, long arg3,
//      long arg4, long arg5 );

LONG
HYPER ## DoPkt2( struct MsgPort *port, long action, long arg1, long arg2 )
{
    return DoPkt(port,action,arg1,arg2,0,0,0);
}
