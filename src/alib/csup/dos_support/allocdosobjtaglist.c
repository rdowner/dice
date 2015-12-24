
/*
 *  AllocDosObjectTags
 */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;


APTR 
HYPER ## AllocDosObjectTagList( unsigned long type, struct TagItem *tags )
{
    return AllocDosObject(type, tags);
}

