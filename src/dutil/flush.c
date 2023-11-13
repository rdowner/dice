/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  dumb memory flush
 *
 */

#ifdef AMIGA
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

IDENT("flush",".3");
DCOPYRIGHT;

int
main(int ac, char **av)
{
#ifdef AMIGA
    char *ptr;

    if (ptr = AllocMem(0x7FFFFFFF, MEMF_PUBLIC)) {
        main(0, NULL);
        FreeMem(ptr, 0x7FFFFFFF);
    }
#endif
    return(0);
}

