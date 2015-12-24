
/*
 *  stack_abort.c
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <stdio.h>
#include <stdlib.h>

void
stack_abort(void)
{
    fprintf(stderr, "run-time stack failure\n");
    abort();
}

