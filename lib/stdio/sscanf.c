
/*
 *  SSCANF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <lib/misc.h>

static int
_sgetc(sst)
unsigned char **sst;
{
    unsigned char *ptr = *sst;
    if (*ptr) {
        *sst = ptr + 1;
        return(*ptr);
    }
    return(EOF);
}

static int
_sungetc(c, sst)
int c;
unsigned char **sst;
{
    --*sst;
}

int
sscanf(buf, ctl, ...)
char *buf;
const char *ctl;
{
    char *ptr = buf;
    int error;
    int cnt;
    va_list va;

    va_start(va, ctl);
    error = _sfmt(ctl, va, _sgetc, _sungetc, &ptr, &cnt);
    va_end(va);
    if (error)
        return(error);
    return(cnt);
}

