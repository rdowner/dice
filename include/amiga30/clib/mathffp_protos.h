#ifndef  CLIB_MATHFFP_PROTOS_H
#define  CLIB_MATHFFP_PROTOS_H
/*
**      $VER: mathffp_protos.h 1.4 (03.05.90)
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
LONG SPFix( FLOAT parm );
FLOAT SPFlt( long integer );
LONG SPCmp( FLOAT leftParm, FLOAT rightParm );
LONG SPTst( FLOAT parm );
FLOAT SPAbs( FLOAT parm );
FLOAT SPNeg( FLOAT parm );
FLOAT SPAdd( FLOAT leftParm, FLOAT rightParm );
FLOAT SPSub( FLOAT leftParm, FLOAT rightParm );
FLOAT SPMul( FLOAT leftParm, FLOAT rightParm );
FLOAT SPDiv( FLOAT leftParm, FLOAT rightParm );
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
FLOAT SPFloor( FLOAT parm );
FLOAT SPCeil( FLOAT parm );
#endif   /* CLIB_MATHFFP_PROTOS_H */
