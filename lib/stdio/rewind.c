
/*
 *  REWIND.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

void
rewind(fi)
FILE *fi;
{
    fseek(fi, 0L, 0);
}

