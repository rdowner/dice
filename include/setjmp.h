
/*
 * $VER: setjmp.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SETJMP_H
#define SETJMP_H

typedef long jmp_buf[16];

extern __stkargs int setjmp(jmp_buf);
extern __stkargs void longjmp(jmp_buf, int);

#endif

