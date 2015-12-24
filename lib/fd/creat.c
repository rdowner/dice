
/*
 *  CREAT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  creat(name, prot)   (prot ignored)
 */

#include <fcntl.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

int
creat(name, mode)
const char *name;
int mode;
{
    return(open(UnixToAmigaPath(name), O_CREAT | O_TRUNC | O_RDWR, mode));
}

