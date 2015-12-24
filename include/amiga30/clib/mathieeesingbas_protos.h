#ifndef  CLIB_MATHIEEESINGBAS_PROTOS_H
#define  CLIB_MATHIEEESINGBAS_PROTOS_H
/*
**	$VER: mathieeesingbas_protos.h 1.3 (07.11.90)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
LONG IEEESPFix( FLOAT parm );
FLOAT IEEESPFlt( long integer );
LONG IEEESPCmp( FLOAT leftParm, FLOAT rightParm );
LONG IEEESPTst( FLOAT parm );
FLOAT IEEESPAbs( FLOAT parm );
FLOAT IEEESPNeg( FLOAT parm );
FLOAT IEEESPAdd( FLOAT leftParm, FLOAT rightParm );
FLOAT IEEESPSub( FLOAT leftParm, FLOAT rightParm );
FLOAT IEEESPMul( FLOAT leftParm, FLOAT rightParm );
FLOAT IEEESPDiv( FLOAT dividend, FLOAT divisor );
FLOAT IEEESPFloor( FLOAT parm );
FLOAT IEEESPCeil( FLOAT parm );
#endif	 /* CLIB_MATHIEEESINGBAS_PROTOS_H */
