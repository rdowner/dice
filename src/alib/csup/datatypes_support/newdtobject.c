
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

Object *NewDTObject( APTR name, Tag Tag1, ... )
{
	return NewDTObjectA(name,(TagItem *)&Tag1);
}
