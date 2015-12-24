
/*
 * $VER: lib/requestfh.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef LIB_REQUESTFH_H
#define LIB_REQUESTFH_H

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

int RequestFH(BPTR, struct Message *, long);

#define ACTION_REQUEST	5000

#define FREQ_NONE	0
#define FREQ_RPEND	1
#define FREQ_WAVAIL	2
#define FREQ_ABORT	3
#define FREQ_SCHANGE	4   /*	async open  */
#define FREQ_ROUTEREQ	5   /*	route request 'R' dummy channel */
#define FREQ_NBIOR1	0x10000
#define FREQ_NBIOR0	0x20000
#define FREQ_NBIOW1	0x40000
#define FREQ_NBIOW0	0x80000

#endif


