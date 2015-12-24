
/*
 * $VER: lib/wild.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef _LIB_WILD_H
#define _LIB_WILD_H

extern void *_ParseWild(const char *, short)
extern int _CompWild(const char *, void *, void *);
extern void _FreeWild(void *);

#endif
