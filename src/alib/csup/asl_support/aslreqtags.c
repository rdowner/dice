
/*
 *  AslRequestTags
 */

#include <exec/types.h>
#include <clib/asl_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

BOOL
HYPER ## AslRequestTags(req, tag1, ...)
APTR req;
Tag tag1;
{
    return(AslRequest(req, (TagItem *)&tag1));
}

