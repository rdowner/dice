
/*
 *  UNIX/SYMLINK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

extern struct DosLibrary *DOSBase;

int
symlink(fileName, linkName)
char *fileName;
char *linkName;
{
    int r = -1;

    if (DOSBase->dl_lib.lib_Version >= 37) {
        if (MakeLink(UnixToAmigaPath(linkName), (long)UnixToAmigaPath(fileName), 1))
            r = 0;
    }
    return(r);
}

