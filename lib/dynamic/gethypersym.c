
/*
 *  GetHyperSymbolTags()
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dynamic_protos.h>

void *
GetHyperSymbolTags(symName, tag1, ...)
const char *symName;
Tag tag1;
{
    return(GetHyperSymbol(symName, (struct TagItem *)&tag1));
}

