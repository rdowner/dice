#ifndef  CLIB_MISC_PROTOS_H
#define  CLIB_MISC_PROTOS_H
/*
**	$VER: misc_protos.h 36.2 (07.11.90)
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
UBYTE *AllocMiscResource( unsigned long unitNum, UBYTE *name );
void FreeMiscResource( unsigned long unitNum );
#endif	 /* CLIB_MISC_PROTOS_H */
