#ifndef  CLIB_MATHIEEESINGTRANS_PROTOS_H
#define  CLIB_MATHIEEESINGTRANS_PROTOS_H
/*
**      $VER: mathieeesingtrans_protos.h 1.3 (07.11.90)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
FLOAT IEEESPAtan( FLOAT parm );
FLOAT IEEESPSin( FLOAT parm );
FLOAT IEEESPCos( FLOAT parm );
FLOAT IEEESPTan( FLOAT parm );
FLOAT IEEESPSincos( FLOAT *cosptr, FLOAT parm );
FLOAT IEEESPSinh( FLOAT parm );
FLOAT IEEESPCosh( FLOAT parm );
FLOAT IEEESPTanh( FLOAT parm );
FLOAT IEEESPExp( FLOAT parm );
FLOAT IEEESPLog( FLOAT parm );
FLOAT IEEESPPow( FLOAT exp, FLOAT arg );
FLOAT IEEESPSqrt( FLOAT parm );
FLOAT IEEESPTieee( FLOAT parm );
FLOAT IEEESPFieee( FLOAT parm );
FLOAT IEEESPAsin( FLOAT parm );
FLOAT IEEESPAcos( FLOAT parm );
FLOAT IEEESPLog10( FLOAT parm );
#endif   /* CLIB_MATHIEEESINGTRANS_PROTOS_H */
