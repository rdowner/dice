
/*
 *  VSPRINTF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
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

    movmem(buf, *sst, n);
    *sst += n;
    return(n2);
}

int
HYPER(vsprintf)(buf, ctl, va)
char *buf;
const char *ctl;
va_list va;
{
    char *ptr = buf;
    int error;

    error = _pfmt(ctl, va, _swrite, &ptr);
    va_end(va);
    *ptr = 0;
    return(error);      /*  count/error */
}

