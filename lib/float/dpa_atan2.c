
/*
 *  FLOAT/DPA_ATAN2.C	Courtesy of Klaxon Suralis
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <math.h>

double
atan2(double y, double x)
{
    double pi =       3.14159265358979323 ;	/* from memory	     */
    double piover2  = 3.14159265358979323/2.0 ; /* / at compile time?*/
    double temp ;

    if (x==0.0) {
	return ((y >= 0.0) ? piover2 : -piover2);
    } else {
	temp = atan(y/x);
	if  (x > 0.0)
	    return (temp);	/* this is the easy way out  */
	else
	    return ((temp >= 0.0) ? (temp-pi) : (temp+pi));
    }
}

