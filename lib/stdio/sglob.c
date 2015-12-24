
/*
 *  SGLOB.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>

FILE _Iob[3];
FILE *_Iod;
_IOFDS _IoStaticFD[3];
_IOFDS *_IoFD = _IoStaticFD;
short _IoFDLimit = 3;



