
/*
 *  STRNCAT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

typedef unsigned char ubyte;

#ifndef HYPER
#define HYPER(x) x
#endif

char *
HYPER(strncat)(d, s, n)
char *d;
const char *s;
size_t n;
{
    char c;
    char *base= d;

    s;
    n;

    if (n) {
        d += strlen(d);
        while (c = *s) {
            *d = c;
            ++s;
            ++d;
            if (--n == 0)
                break;
        }
    }
    *d = 0;
    return(base);
}

