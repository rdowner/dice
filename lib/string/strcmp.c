
/*
 *  STRCMP.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

typedef unsigned char ubyte;

int
HYPER(strcmp)(s, d)
const char *s;
const char *d;
{
    while (*(ubyte *)s == *(ubyte *)d) {
	if (*s == 0)
	    return(0);
	++s;
	++d;
    }
    if (*(ubyte *)s < *(ubyte *)d)
	return(-1);
    return(1);
}

