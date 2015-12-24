#ifndef  CLIB_MISC_PROTOS_H
#define  CLIB_MISC_PROTOS_H
/*
**	$Filename: clib/misc_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:12 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
UBYTE *AllocMiscResource( unsigned long unitNum, UBYTE *name );
void FreeMiscResource( unsigned long unitNum );
#endif	 /* CLIB_MISC_PROTOS_H */
