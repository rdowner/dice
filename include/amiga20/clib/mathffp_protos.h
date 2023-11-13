#ifndef  CLIB_MATHFFP_PROTOS_H
#define  CLIB_MATHFFP_PROTOS_H
/*
**      $Filename: clib/mathffp_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:10 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "mathffp.library" */
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
