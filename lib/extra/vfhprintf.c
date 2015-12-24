
/*
 *  VFHPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <clib/dos_protos.h>
#include <lib/misc.h>

#ifndef HYPER
#define HYPER(x) x
#endif

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
HYPER(vfhprintf)(fh, ctl, va)
long fh;
const char *ctl;
va_list va;
{
    int error;

    error = _pfmt(ctl, va, _fhwrite, (void *)fh);
    va_end(va);
    return(error);      /*  count/error */
}

