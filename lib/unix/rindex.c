
/*
 *  UNIX/RINDEX.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stddef.h>
#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

char *
HYPER(rindex)(toks, c)
const char *toks;
int c;
{
    const char *ptr = toks + strlen(toks);

    while (ptr > toks && *ptr != (char)c)
	--ptr;
    if (*ptr == (char)c)
	return(ptr);
    return(NULL);
}

