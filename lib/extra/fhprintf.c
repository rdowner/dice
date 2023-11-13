
/*
 *  FHPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  int r = fhprintf(DOSHANDLE, ctl, ...)
 */

#include <stdio.h>
#include <stdarg.h>
#include <clib/dos_protos.h>
#include <lib/misc.h>

#ifndef HYPER
#define HYPER(x) x
#endif
/*
 *  last arg must be pointer to maintain compatibility with pfmt()
 *  when registered args are used.
 */

static unsigned int
_fhwrite(buf, elmsize, elms, fh)
char *buf;
size_t elmsize;
size_t elms;
void *fh;
{
    int r = Write((BPTR)fh, buf, elmsize * elms);
    if (r >= 0)
        r = r / elmsize;
    return(r);
}

int
HYPER(fhprintf)(fh, ctl, ...)
long fh;            /*  equiv to BPTR   */
const char *ctl;
{
    int error;
    va_list va;

    va_start(va, ctl);
    error = _pfmt(ctl, va, _fhwrite, (void *)fh);
    va_end(va);
    return(error);
}

