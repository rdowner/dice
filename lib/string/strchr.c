
/*
 *  STRCHR.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

char *
HYPER(strchr)(toks, c)
const char *toks;
int c;
{
    while (*toks) {
	if (*toks == (char)c)
	    return(toks);
	++toks;
    }
    if (c == 0)
	return(toks);
    return(NULL);
}

