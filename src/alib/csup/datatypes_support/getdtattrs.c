
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

ULONG GetDTAttrs( Object *o, Tag Tag1, ... )
{
        return GetDTAttrsA(o,(TagItem *)&Tag1);
}
