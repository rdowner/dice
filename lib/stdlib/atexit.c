
/*
 *  ATEXIT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <lib/atexit.h>

int
atexit(func)
void (*func)(void);
{
    AtExit *at = malloc(sizeof(AtExit));

    if (at) {
	at->Next = _ExitBase;
	at->Func = func;
	_ExitBase = at;
	return(0);
    }
    return(-1);
}


