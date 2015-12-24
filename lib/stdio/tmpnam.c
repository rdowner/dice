
/*
 *  TMPNAM()
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <clib/exec_protos.h>
#include <stdio.h>

char *
tmpnam(buf)
char *buf;
{
    static char Buf[L_tmpnam];
    static long i;

    if (buf == NULL)
	buf = Buf;
    sprintf(buf, "T:%08lx-%ld", FindTask(NULL), i++);
    return(buf);
}

