
/*
 * $VER: float.h 1.0 (17.4.93)
 *              XXX fixme IEEE-X.  Other bounds
 *              are somewhat loose (fp,dp)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef FLOAT_H
#define FLOAT_H

/*
 *  IEEE 32 bit format
 */

#define FLT_DIG         6
#define FLT_EPSILON     1.0E-6
#define FLT_MANT_DIG    20
#define FLT_MAX         1E37
#define FLT_MAX_10_EXP  37
#define FLT_MAX_EXP     23
#define FLT_MIN         -1E37
#define FLT_MIN_10_EXP  -37
#define FLT_MIN_EXP     -23

#define FLT_RADIX       2
#define FLT_ROUNDS      -1

/*
 *  IEEE 64 bit format. S.E[11].M[52]
 */

#define DBL_DIG         15
#define DBL_EPSILON     1.0E-15
#define DBL_MANT_DIG    52
#define DBL_MAX         1E307
#define DBL_MAX_10_EXP  307
#define DBL_MAX_EXP     1022
#define DBL_MIN         1E-307
#define DBL_MIN_10_EXP  -307
#define DBL_MIN_EXP     -1022

/*
 *  ??? 128 bit format. S.E[15].M[112]
 */

#define LDBL_DIG        33
#define LDBL_EPSILON    1E-33
#define LDBL_MANT_DIG   112
#define LDBL_MAX        1E4932
#define LDBL_MAX_10_EXP 4932
#define LDBL_MAX_EXP    32767
#define LDBL_MIN        1E-4932
#define LDBL_MIN_10_EXP -4932
#define LDBL_MIN_EXP    -32767

#endif

