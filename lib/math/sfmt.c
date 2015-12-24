
/*
 *  MATH/SFMT.C     replace specific cases in sfmt
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  NOTE:   Can only deal with double's at this point since we do not
 *  know what format the float's are in (FFP or IEEESING)
 */

#ifndef MATH_SFMT

#define MATH_SFMT
#include <math.h>
#include <float.h>
#include "stdio/sfmt.c"

#else
    case 'e':       /*  [-]d.dddddde[+/-]dd     */
    case 'E':       /*  [-]d.ddddddE[+/-]dd     */
    case 'f':       /*  [-]d.dddddd             */
    case 'g':       /*  if exp < -4 or exp > prec use 'e', else use 'f' */
    case 'G':       /*  if exp < -4 or exp > prec use 'E', else use 'f' */
	if ((flags & F_LONG) == 0) {
	    fprintf(stderr, "*scanf: %%e,E,f,g,G, only 'l' double format currently supported\n");
	} else {
	    short n = 0;
	    short state = 0;

	    while (i1 && n < sizeof(buf2) - 1) {
		switch(state) {
		case 0:     /*	opt -/+ */
		case 5:     /*	e-/+	*/
		    ++state;
		    if (v == '-' || v == '+')
			break;
		    continue;
		case 1:     /*	dec num */
		case 3:     /*	dec num */
		case 6:     /*	num num */
		    if (v >= '0' && v <= '9')
			break;
		    ++state;
		    continue;
		case 2:     /*	opt .	*/
		    ++state;
		    if (v == '.')
			break;
		    continue;
		case 4:     /*	e/E	*/
		    if (v == 'e' || v == 'E') {
			++state;
			break;
		    }
		    state = 7;
		    /* fall through */
		case 7:     /*	END	*/
		    goto fltdone;
		}
		buf2[n++] = v;
		++nscanned;
		--i1;
		v = (*func)(desc);
	    }
fltdone:
	    buf2[n] = 0;
	    if (n == 0)
		return(EOF);
	    if (stor)
		*(double *)stor = strtod(buf2, NULL);
	}
	break;
#endif


