
/*
 *  STRCAT.C
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
HYPER(strcat)(d, s)
char *d;
const char *s;
{
    char *base= d;

    while (*d)
        ++d;
    while (*d++ = *s++);
    return(base);
}

