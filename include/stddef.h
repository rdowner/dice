
/*
 * $VER: stddef.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef STDDEF_H
#define STDDEF_H

#ifndef NULL
#define NULL	((void *)0L)
#endif
#ifndef offsetof
#define offsetof(sname,fname)	((long)&((sname *)0)->fname)
#endif
typedef int ptrdiff_t;
typedef unsigned int size_t;
typedef char wchar_t;

#endif

