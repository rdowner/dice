
/*
 *  fputs.c
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>

int
fputs(ptr, fi)
const char *ptr;
FILE *fi;
{
    fwrite(ptr, strlen(ptr), 1, fi);
    return(fi->sd_Error);
}

