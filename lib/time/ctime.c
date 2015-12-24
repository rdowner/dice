
/*
 *  time/ctime.c
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <time.h>

char *
ctime(tp)
const time_t *tp;
{
    return(asctime(localtime(tp)));
}

