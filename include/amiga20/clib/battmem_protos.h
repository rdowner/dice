#ifndef  CLIB_BATTMEM_PROTOS_H
#define  CLIB_BATTMEM_PROTOS_H
/*
**	$Filename: clib/battmem_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:07 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "battmem.resource" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
void ObtainBattSemaphore( void );
void ReleaseBattSemaphore( void );
ULONG ReadBattMem( APTR buffer, unsigned long offset, unsigned long length );
ULONG WriteBattMem( APTR buffer, unsigned long offset, unsigned long length );
#endif	 /* CLIB_BATTMEM_PROTOS_H */
