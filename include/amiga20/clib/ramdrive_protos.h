#ifndef  CLIB_RAMDRIVE_PROTOS_H
#define  CLIB_RAMDRIVE_PROTOS_H
/*
**	$Filename: clib/ramdrive_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:12 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "ramdrive.device" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
/*--- functions in V34 or higher (distributed as Release 1.3) ---*/
STRPTR KillRAD0( void );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
STRPTR KillRAD( unsigned long unit );
#endif	 /* CLIB_RAMDRIVE_PROTOS_H */
