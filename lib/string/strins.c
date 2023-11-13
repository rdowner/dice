
/*
 *  STRINS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

void
HYPER(strins)(d, s)
char *d;
const char *s;
{
    int len = strlen(s);    /*  # bytes to insert   */
    char *ptr;

    /*
     *  make room
     */

    ptr = d + strlen(d);
    while (ptr >= d) {
        ptr[len] = ptr[0];
        --ptr;
    }

    /*
     *  insert string
     */

    while (*s)
        *++ptr = *s++;
}

