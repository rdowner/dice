
/*
 *  DC1/IEEE.H
 *
 *  IEEE*() calls
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#define float	long

extern __stkargs double IEEEDPFlt(long);
extern __stkargs double IEEEDPAdd(double, double);
extern __stkargs double IEEEDPMul(double, double);
extern __stkargs double IEEEDPDiv(double, double);
extern __stkargs double IEEEDPPow(double, double);
extern __stkargs double IEEEDPNeg(double);

extern __stkargs float	IEEESPFlt(long);
extern __stkargs float	IEEESPAdd(float, float);
extern __stkargs float	IEEESPMul(float, float);
extern __stkargs float	IEEESPDiv(float, float);
extern __stkargs float	IEEESPPow(float, float);
extern __stkargs float	IEEESPNeg(float);

