
/*
 *  STPCPY.C
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
HYPER(stpcpy)(d, s)
char *d;
const char *s;
{
    while (*d++ = *s++);
    return(d-1);
}


