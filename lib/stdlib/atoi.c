
/*
 *  int ATOI(str)
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdlib.h>

int
atoi(str)
const char *str;
{
    short neg = 0;
    long v = 0;

    while (*str == ' ' || *str == '\t')
	++str;
    if (*str == '-') {
	neg = 1;
	++str;
    }
    if (*str == '+')
	++str;

    /*
     *	while v < 65536 we setup the operation such that it is optimizable
     *	by DICE.  This yields a huge efficiency increase.
     */

    while (*str >= '0' && *str <= '9') {
	if (v <= (unsigned short)-1)
	    v = (unsigned short)v * 10;
	else
	    v = v * 10;
	v += *str++ - '0';
    }
    if (neg)
	v = -v;
    return(v);
}


