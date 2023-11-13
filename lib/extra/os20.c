
/*
 *  OS20.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct Library *SysBase;

void
OS2_0()
{
    char *str = "Runs under 2.0 only\n";

    if (SysBase->lib_Version < 36) {
        Write(Output(), str, strlen(str));
        exit(1);
    }
}

