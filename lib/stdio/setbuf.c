
/*
 *  SETBUF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

void
setbuf(fi, buf)
FILE *fi;
char *buf;
{
    if (buf)
        setvbuf(fi, buf, _IOFBF, _bufsiz);
    else
        setvbuf(fi, NULL, _IONBF, 0);
}

