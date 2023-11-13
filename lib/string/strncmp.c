
/*
 *  STRNCMP.C
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
HYPER(strncmp)(s, d, n)
const char *s;
const char *d;
size_t n;
{
    char c;

    n;  /*  gets n into a register */
    n;
    d;
    d;

    if (n == 0)
        return(0);

    while ((c = *s) == *d) {
        if (c == 0 || --n == 0)
            return(0);
        ++s;
        ++d;
    }
    if ((ubyte)c < (ubyte)*d)
        return(-1);
    return(1);
}

