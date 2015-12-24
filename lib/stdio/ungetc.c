
/*
 *  UNGETC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

int
ungetc(c, fi)
int c;
FILE *fi;
{
    if (fi->sd_UC < 0) {
	fi->sd_UC = c;
	return(c);
    }
    return(EOF);
}

