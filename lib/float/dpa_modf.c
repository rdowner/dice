
/*
 *  DPA_MODF.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  double frac = modf(double value, double *ip);
 */

#include <math.h>

double
modf(value, ip)
double value;
double *ip;
{
    if (value >= 0.0)
        *ip = floor(value);
    else
        *ip = ceil(value);
    return(value - *ip);
}

