
/*
 *  MATH/PFMT.C     replace specific cases in pfmt
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  NOTE:   Can only deal with double's at this point since we do not
 *  know what format the float's are in (FFP or IEEESING)
 *
 *  FROM STDIO/PFMT.C:
 *	char *prefix1;	    set to "-" if negative, else leave alone
 *	char *postfix1;     set to "E+/-NN" if applicable
 *	char *value;	    set to value content
 *	long len;	    set to length of value
 *	char buf[];	    scratch array
 *	short trail_zero;   trailing zeros, set to number
 *
 *	do not build more then 32 chars starting at the end of buf,
 *	trailing zeros are handled by setting trail_zero to the proper
 *	value
 *
 */

#ifndef MATH_PFMT

#define MATH_PFMT   1
#include <math.h>
#include <float.h>
#include "stdio/pfmt.c"

#elif MATH_PFMT == 1
#define MATH_PFMT   2
    case 'e':       /*  [-]d.dddddde[+/-]dd     */
    case 'E':       /*  [-]d.ddddddE[+/-]dd     */
    case 'f':       /*  [-]d.dddddd             */
    case 'g':       /*  if exp < -4 or exp > prec use 'e', else use 'f' */
    case 'G':       /*  if exp < -4 or exp > prec use 'E', else use 'f' */
	{
	    static char prefix[32];
	    short xp;
	    short _pfmt_round(char *, short, short);
	    short gtrunc;

	    if ((c == 'g' || c == 'G') && !(flags & F_HASH))
		gtrunc = 1;
	    else
		gtrunc = 0;

	    if (i2 < 0) 	/*  default precision	    */
		i2 = 6;

	    if (flags & F_DLONG) {
		va_arg(va, long double);
		i = xp = 0;
	    } else {
		double d;

		d = va_arg(va, double);
		if (d < 0.0) {
		    d = -d;
		    prefix1 = "-";
		}

		/*
		 *  construct prefix buffer by extracting one digit at
		 *  a time
		 */

		if (d >= 1.0)
		    xp = (int)log10(d);
		else
		    xp = (int)log10(d) - 1;

		d = d * pow(10.0, (double)-xp) + (DBL_EPSILON * 5);

		for (i = 0; i < DBL_DIG; ++i) {
		    prefix[i] = (int)d;
		    d = (d - (double)prefix[i]) * 10.0;
		}
	    }

	    /*
	     *	prefix	- prefix array (might have leading zeros)
	     *	i	- number of digits in array
	     *	xp	- exponent of first digit
	     *
	     *	handle %g and %G
	     */

	    if (c == 'g' || c == 'G') {
		if (xp < -4 || xp >= i2)
		    c = (c == 'g') ? 'e' : 'E';
		else
		    c = 'f';
	    }

	    /*
	     *	handle %e and %E
	     */

	    value = buf + sizeof(buf) - MATH_BUFSIZE;

	    if (c == 'e' || c == 'E') {
		if ((i = _pfmt_round(prefix, i2 + 1, i)) < 0) {
		    i = -i;
		    ++xp;
		}
		if (i == 0)
		    xp = 0;
		value[0] = prefix[0] + '0';
		if (i2 > 0 && (gtrunc == 0 || i > 1)) {
		    value[1] = '.';
		    for (len = 0; len < i2 && len < (MATH_BUFSIZE - 8); ++len) {
			if (len + 1 >= i) {
			    if (gtrunc == 0)
				trail_zero = i2 - len;
			    break;
			}
			value[len+2] = prefix[len+1] + '0';
		    }

		    /*
		     *	put in at least one '0' after the decimal!
		     *	(don't have to check gtrunc because prefix
		     *	array has at least 2 entries)
		     */

		    if (trail_zero == 0 && len == 0) {
			value[2] = '0';
			++len;
		    }
		    ++len;
		}
		++len;
		{
		    short yp = (xp < 0) ? -xp : xp;

		    postfix1 = value + len;
		    postfix1[0] = c;
		    postfix1[1] = (xp < 0) ? '-' : '+';
		    postfix1[2] = yp / 100 + '0';
		    postfix1[3] = yp / 10 % 10 + '0';
		    postfix1[4] = yp % 10 + '0';
		    postfix1[5] = 0;
		}
	    } else {
		short pi;

		if ((i = _pfmt_round(prefix, i2 + 1 + xp, i)) < 0) {
		    i = -i;
		    ++xp;
		}
		for (len = pi = 0; len < MATH_BUFSIZE && xp >= 0; ++len) {
		    if (pi < i) {
			value[len] = prefix[pi] + '0';
			++pi;
		    } else {
			value[len] = '0';
		    }
		    --xp;
		}
		if (len == 0)
		    value[len++] = '0';

		/* XXX */

		/*
		 *  pre-zeros
		 */

		if (len < MATH_BUFSIZE && i2 > 0 && (gtrunc == 0 || pi < i)) {
		    short prec = 0;

		    value[len++] = '.';

		    while (len < MATH_BUFSIZE && -(prec + 1) > xp && prec < i2) {
			value[len++] = '0';
			++prec;
		    }
		    while (len < MATH_BUFSIZE && prec < i2) {
			if (pi < i)
			    value[len++] = prefix[pi++] + '0';
			else if (gtrunc == 0)
			    value[len++] = '0';
			++prec;
		    }
		}
	    }
	}
	break;

#elif MATH_PFMT == 2

/*
 *  round the prefix array starting at index ri, the array's maximum size
 *  is max.  Returns a new maximum size (which is negative if a carry
 *  over occured).  This new maximum is set to not include our rounding
 *  value of 5 and also takes into account trailing zeros.
 */

short
_pfmt_round(prefix, ri, max)
char *prefix;
short ri;
short max;
{
    short carry;
    char *ptr;

    if (ri < max) {	/*  handle rounding + the log boundry case	    */
	max = ri;
	if (max < 0)
	    max = 0;
	carry = 5;
    } else {		/*  handle log boundry case (entries that are 10)   */
	if (max)
	    ri = --max;
	else
	    ri = -1;
	carry = 0;
    }
    for (ptr = prefix + ri; ri >= 0; --ri, --ptr) {
	*ptr += carry;
	if (*ptr >= 10) {
	    *ptr -= 10;
	    carry = 1;
	} else {
	    carry = 0;
	}
	if (*ptr == 0 && ri == max - 1)
	    --max;
    }
    if (carry) {
	movmem(prefix, prefix + 1, max);
	++max;
	prefix[0] = 1;
	return(-max);
    }
    return(max);
}

#endif

