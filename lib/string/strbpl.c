
/*
 *  STRBPL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

int
HYPER(strbpl)(av, max, sary)
char **av;
int max;
const char *sary;
{
    int i;

    for (i = 0; i < max; ++i) {
        if (*sary == 0) {
            *av++ = NULL;
            return(i);
        }
        *av++ = sary;
        while (*sary)
            ++sary;
        ++sary;
    }
    return(-1);
}

