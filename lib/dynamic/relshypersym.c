
/*
 *  LIBTAGS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dynamic_protos.h>

void *
RelsHyperSymbolTags(ptr, tag1, ...)
void *ptr;
Tag tag1;
{
    return(RelsHyperSymbol(ptr, (struct TagItem *)&tag1));
}

