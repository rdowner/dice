
/*
 *  PERROR.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

void
perror(str)
const char *str;
{
    fprintf(stderr, "%s: (%d)%s\n", str, errno, strerror(errno));
}

