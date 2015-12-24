
/*
 *  STRSPN.C
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
HYPER(strspn)(str, toks)
const char *str;
const char *toks;
{
    const char *ptr;

    for (ptr = str; *ptr && strchr(toks, *ptr); ++ptr);
    return(ptr - str);
}

