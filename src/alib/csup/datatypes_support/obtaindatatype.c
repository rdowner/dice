
/* Datatypes tag call */

#include <exec/types.h>
#include <clib/datatypes_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

struct DataType *ObtainDataType( unsigned long type, APTR handle, Tag Tag1, ... )
{
        return ObtainDataTypeA(type, handle, (TagItem *)&Tag1);
}
