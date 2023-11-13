
/*
 *  FREE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <stdlib.h>

extern long *__MemList;

void
free(ptr)
void *ptr;
{
    long **scan = &__MemList;
    long *item;

    if (ptr == NULL)
        return;

    ptr = (void *)((long *)ptr - 2);

    while (item = *scan) {
        if (item == (long *)ptr) {
            *scan = *(long **)item;
            FreeMem(ptr, ((long *)ptr)[1]);
            return;
        }
        scan = (long **)item;
    }
    Wait(0);
}

