
/*
 *  REMOVE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dos_protos.h>
#include <stdio.h>
#include <errno.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

int
remove(name)
const char *name;
{
    if (DeleteFile(UnixToAmigaPath(name)))
	return(0);
    errno = ENOFILE;
    return(-1);
}

