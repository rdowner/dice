#ifndef  CLIB_MATHIEEESINGBAS_PROTOS_H
#define  CLIB_MATHIEEESINGBAS_PROTOS_H
/*
**	$Filename: clib/mathieeesingbas_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:11 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "mathieeesingbas.library" */
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
