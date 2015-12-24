
/*
 *  FSETPOS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

int
fsetpos(fp, pos)
FILE *fp;
const fpos_t *pos;
{
    if (fseek(fp, *pos, 0) < 0)
	return(EOF);
    return(0);
}

