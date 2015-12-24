
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
HYPER ## AllocDosObjectTags(type, tag1, ...)
unsigned long type;
Tag tag1;
{
    return(AllocDosObject(type, (TagItem *)&tag1));
}

