
/*
 *  STRDUP.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>
#include <stdlib.h>

char *
strdup(s)
const char *s;
{
    int len = strlen(s);
    char *d;

    if (d = malloc(len + 1))
	strcpy(d, s);
    return(d);
}

