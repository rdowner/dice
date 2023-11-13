
/*
 *  RENAME.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/dos_protos.h>
#include <stdio.h>
#include <errno.h>

int
rename(s, d)
const char *s;
const char *d;
{
    if (Rename(s, d))
        return(0);
    errno = ENOFILE;
    return(-1);
}

