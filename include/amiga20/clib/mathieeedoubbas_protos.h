#ifndef  CLIB_MATHIEEEDOUBBAS_PROTOS_H
#define  CLIB_MATHIEEEDOUBBAS_PROTOS_H
/*
**	$Filename: clib/mathieeedoubbas_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:11 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "mathieeedoubbas.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
LONG IEEEDPFix( DOUBLE parm );
DOUBLE IEEEDPFlt( long integer );
LONG IEEEDPCmp( DOUBLE leftParm, DOUBLE rightParm );
LONG IEEEDPTst( DOUBLE parm );
DOUBLE IEEEDPAbs( DOUBLE parm );
DOUBLE IEEEDPNeg( DOUBLE parm );
DOUBLE IEEEDPAdd( DOUBLE leftParm, DOUBLE rightParm );
DOUBLE IEEEDPSub( DOUBLE leftParm, DOUBLE rightParm );
DOUBLE IEEEDPMul( DOUBLE factor1, DOUBLE factor2 );
DOUBLE IEEEDPDiv( DOUBLE dividend, DOUBLE divisor );
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
DOUBLE IEEEDPFloor( DOUBLE parm );
DOUBLE IEEEDPCeil( DOUBLE parm );
#endif	 /* CLIB_MATHIEEEDOUBBAS_PROTOS_H */
