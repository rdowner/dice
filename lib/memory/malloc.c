
/*
 *  MALLOC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern long *__MemList;

void *
malloc(bytes)
size_t bytes;
{
    long *ptr;

    if (bytes == 0)
	return(NULL);

    ptr = AllocMem(bytes + 8, MEMF_PUBLIC);
    if (ptr) {
	ptr[0] = (long)__MemList;
	__MemList = ptr;
	ptr[1] = bytes + 8;
	ptr += 2;
    } else {
	errno = ENOMEM;
    }
    return((void *)ptr);
}

