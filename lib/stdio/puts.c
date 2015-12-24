
/*
 *  PUTS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>

int
puts(buf)
const char *buf;
{
    fwrite(buf, 1, strlen(buf), stdout);
    putc('\n', stdout);
    return(stdout->sd_Error);
}

