
/*
 *  EXIT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib/atexit.h>
#include <lib/misc.h>
#include <libraries/dosextens.h>

typedef struct Process Process;

extern long *__MemList;
AtExit *_ExitBase = NULL;

void
exit(code)
int code;
{
    {
	AtExit *eb;
	for (eb = _ExitBase; eb; eb = eb->Next)
	    (*eb->Func)();
    }

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    /*
     *	stdio
     */

    while (_Iod)
	fclose(_Iod);

    /*
     *	file descriptors
     */

    __closeall();

    /*
     *	low level exit (handles allocated memory & workbench)
     */

    _exit(code);
}

