
/*
 *  STRNCPY.C
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
HYPER(strncpy)(d, s, n)
char *d;
const char *s;
size_t n;
{
    char c;
    char *base = d;

    while(n && (c = *s)) {
        *d = c;
        ++s;
        ++d;
        --n;
    }
    if (n)
        *d = 0;
    return(base);
}

