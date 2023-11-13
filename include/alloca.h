
/*
 * $VER: alloca.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef ALLOCA_H
#define ALLOCA_H

extern __regargs void *_dice_alloca(long);

#define alloca          _dice_alloca

#endif

