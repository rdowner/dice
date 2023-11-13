
/*
 *  STPCHR.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

const char *
HYPER(stpchr)(str, c)
const char *str;
char c;
{
    char cc;

    while (cc = *str) {
        if (cc == c)
            return(str);
        ++str;
    }
    if (cc == c)
        return(str);
    return(NULL);
}

