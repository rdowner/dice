
/*
 *  SLEEP.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>

void
sleep(n)
int n;
{
    while (n) {
	Delay(50);
	chkabort();
	--n;
    }
}

