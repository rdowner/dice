
/*
 *  GETWD.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *
getwd(buf)
char *buf;
{
    char *path;

    if (path = getcwd(buf, 256))
	return(path);
    strcpy(buf, strerror(errno));
    return(NULL);
}

