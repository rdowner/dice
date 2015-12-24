
/*
 *  AllocAslRequestTags
 */

#include <exec/types.h>
#include <clib/asl_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

APTR
HYPER ## AllocAslRequestTags(type, tag1, ...)
unsigned long type;
Tag tag1;
{
    return(AllocAslRequest(type, (TagItem *)&tag1));
}

