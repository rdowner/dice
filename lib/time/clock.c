
/*
 *  CLOCK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Return system clock
 */

#include <time.h>
#include <clib/dos_protos.h>

clock_t
clock()
{
    unsigned long v[3];
    clock_t t;

    DateStamp((struct DateStamp *)v);
    t = v[0] * (1440 * 60 * 50) + v[1] * (60 * 50) + v[2];
    return(t);
}

