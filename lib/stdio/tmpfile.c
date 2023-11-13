
/*
 *  tmpfile()       - create a temporary file that is deleted on
 *                    close
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *
tmpfile()
{
    char *name;
    FILE *fi;

    if (name = strdup(tmpnam(NULL))) {
        if (fi = fopen(name, "wb+C"))
            return(fi);
        free(name);
    }
    return(NULL);
}

