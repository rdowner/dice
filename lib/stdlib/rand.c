

/*
 *  RAND.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdlib.h>

int  _RandSeed1 = 0;
int  _RandSeed2 = 0;

static __autoinit void
rand_init()
{
    srand(1);
}

int
rand(void)
{
    _RandSeed1 = ((_RandSeed1 * 13 + 1) ^ (_RandSeed1 >> 9)) + _RandSeed2;
    _RandSeed2 = (_RandSeed2 * _RandSeed1 + 13) ^ (_RandSeed2 >> 13);
    return(_RandSeed1 & RAND_MAX);
}

void
srand(seed)
unsigned int seed;
{
    _RandSeed1 = (seed - 1) ^ 0xAB569834;
    _RandSeed2 = (seed + 1) ^ 0x56F42001;
    rand();
    rand();
    rand();
}

