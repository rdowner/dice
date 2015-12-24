
/*
 * $VER: lib/bcpl.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef _LIB_BCPL_H
#define _LIB_BCPL_H

#define BTOC(bptr, type)  ((type *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)((unsigned long)(cptr) >> 2))

#endif

