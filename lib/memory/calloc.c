
/*
 *  CALLOC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  calloc(elmsize, elms)
 */

#include <stdlib.h>
#include <string.h>

void *
calloc(elmSize, elms)
size_t elmSize;
size_t elms;
{
    void *ptr;
    long bytes = elmSize * elms;

    if (ptr = malloc(bytes))
	clrmem(ptr, bytes);
    return(ptr);
}

