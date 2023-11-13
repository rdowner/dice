
/*
 *  FGETPOS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

int
fgetpos(fp, pos)
FILE *fp;
fpos_t *pos;
{
    *pos = ftell(fp);
    if (*pos < 0)
        return(EOF);
    return(0);
}

