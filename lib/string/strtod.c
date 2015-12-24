
/*
 *  STRTOD.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>
#include <math.h>

double
strtod(str, tp)
const char *str;
char **tp;
{
    char    prefix[32]; /*  prefix (precision)	*/
    char    c;
    short   xp = -1;	/*  exponent	*/
    short   i;
    short   j;
    short   neg = 0;

    while (*str == ' ' || *str == 9)
	++str;
    switch(*str) {
    case '+':
	++str;
	break;
    case '-':
	neg = 1;
	++str;
	break;
    }

    /*
     *	skip any leading zeros, generate numerical prefix
     */

    while (*str == '0')
	++str;
    for (i = 0; (c = *str) >= '0' && c <= '9'; ++str) {
	if (i < sizeof(prefix)) {
	    prefix[i] = c - '0';
	    ++i;
	}
	++xp;
    }

    /*
     *	decimal point, continue generating prefix, exponent no longer grows
     *	skip leading zeros
     */

    if (*str == '.') {
	while (i == 0 && *str == '0') {
	    --xp;
	    ++str;
	}
	for (++str; (c = *str) >= '0' && c <= '9'; ++str) {
	    if (i < sizeof(prefix)) {
		prefix[i] = c - '0';
		++i;
	    }
	}
    }

    /*
     *	exponent
     */

    if (*str == 'e' || *str == 'E') {
	long n = 0;
	short xpneg = 0;

	++str;
	switch(*str) {
	case '+':
	    ++str;
	    break;
	case '-':
	    xpneg = 1;
	    ++str;
	    break;
	}
	while ((c = *str) >= '0' && c <= '9') {
	    n = n * 10 + c - '0';
	    ++str;
	}
	if (xpneg)
	    n = -n;
	xp += n;
    }

    if (tp)
	*tp = str;

    /*
     *	Generate number
     */

    {
	double d = 0.0;
	for (j = 0; j < i; ++j) {
	    if (prefix[j])
		d = d + (double)prefix[j] * pow(10.0, (double)xp);
	    --xp;
	}
	if (neg)
	    d = -d;
	return(d);
    }
}

