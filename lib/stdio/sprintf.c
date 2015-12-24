
/*
 *  SPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

#ifndef HYPER
#define HYPER(x) x
#endif

static unsigned int
_swrite(buf, n1, n2, sst)
char *buf;
size_t n1;
size_t n2;
const char **sst;
{
    size_t n;

    if (n1 == 1)
	n = n2;
    else if (n2 == 1)
	n = n1;
    else
	n = n1 * n2;

    _slow_bcopy(buf, *sst, n);
    *sst += n;
    return(n2);
}

int
HYPER(sprintf)(buf, ctl, ...)
char *buf;
const char *ctl;
{
    char *ptr = buf;
    int error;
    va_list va;

    va_start(va, ctl);
    error = _pfmt(ctl, va, _swrite, &ptr);
    va_end(va);
    *ptr = 0;
    return(error);      /*  count/error */
}

