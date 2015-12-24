#ifndef  CLIB_BATTCLOCK_PROTOS_H
#define  CLIB_BATTCLOCK_PROTOS_H
/*
**	$VER: battclock_protos.h 1.3 (03.05.90)
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
void ResetBattClock( void );
ULONG ReadBattClock( void );
void WriteBattClock( unsigned long time );
#endif	 /* CLIB_BATTCLOCK_PROTOS_H */
