
/*
 * $VER: lib/atexit.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef _LIB_ATEXIT_H
#define _LIB_ATEXIT_H

typedef struct AtExit {
    struct  AtExit *Next;
    void (*Func)(void);
} AtExit;

extern AtExit *_ExitBase;

#endif

