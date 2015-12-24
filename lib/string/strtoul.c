
/*
 *  STRTOUL.C
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

unsigned long
HYPER(strtoul)(ptr, tail, base)
const char *ptr;
char **tail;
int base;
{
    return((unsigned long)strtol(ptr, tail, base));
}

