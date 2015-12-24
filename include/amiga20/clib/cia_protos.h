#ifndef  CLIB_CIA_PROTOS_H
#define  CLIB_CIA_PROTOS_H
/*
**	$Filename: clib/cia_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:07 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "CiaA.Resource" and "CiaB.Resource" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif
#ifndef  EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
struct Interrupt *AddICRVector( struct Library *resource, long iCRBit,
	struct Interrupt *interrupt );
void RemICRVector( struct Library *resource, long iCRBit,
	struct Interrupt *interrupt );
WORD AbleICR( struct Library *resource, long mask );
WORD SetICR( struct Library *resource, long mask );
#endif	 /* CLIB_CIA_PROTOS_H */
