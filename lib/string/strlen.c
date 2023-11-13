
/*
 *  STRLEN.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

size_t
HYPER(strlen)(s)
const char *s;
{
    const char *b = s;

    while (*s)
        ++s;
    return(s - b);
}

