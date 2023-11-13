
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

ULONG PrintDTObject( Object *o, struct Window *w, struct Requester *r,unsigned long data, ... )
{
        return PrintDTObjectA(o,w,r,(struct dtPrint *)&data);
}
