
/*
 *  STRTOL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>

#ifndef HYPER
#define HYPER(x) x
#endif

long
HYPER(strtol)(ptr, tail, base)
const char *ptr;
char **tail;
int base;
{
    long v = 0;
    short ishex = 0;
    short c;
    short neg = 0;

    while (*ptr == ' ' || *ptr == '\t')
	++ptr;
    if (*ptr == '-') {
	neg = 1;
	++ptr;
    } else if (*ptr == '+')
	++ptr;

    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X'))
	ishex = 1;

    if (base == 0) {
	base = 10;
	if (ptr[0] == '0') {
	    base = 8;
	    if (ishex)
		base = 16;
	}
    }
    if (base == 16 && ishex)
	ptr += 2;
    for (;;) {
	c = *ptr;
	if (c >= '0' && c <= '9')
	    c -= '0';
	else if (c >= 'a' && c <= 'z')
	    c -= ('a' - 10);
	else if (c >= 'A' && c <= 'Z')
	    c -= ('A' - 10);
	else
	    break;
	if (c >= base)
	    break;
	v = v * base + c;
	++ptr;
    }
    if (tail)
	*tail = ptr;
    if (neg)
	v = -v;
    return(v);
}

