/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  Misc. support routines
 */

#include "defs.h"
#ifdef AMIGA
#include <exec/libraries.h>

Prototype int Running2_04(void);
Prototype int align(int);

extern struct Library *SysBase;

Running2_04()
{
    if (SysBase->lib_Version >= 37)
        return(1);
    return(0);
}

#endif

int
align(n)
int n;
{
    if (n & 3)
        return(4 - (n & 3));
    return(0);
}

