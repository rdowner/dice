
/*
 *  AslRequestTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <clib/asl_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

BOOL
HYPER ## AslRequestTags(req, tag1, ...)
APTR req;
Tag tag1;
{
    return(AslRequest(req, (TagItem *)&tag1));
}

