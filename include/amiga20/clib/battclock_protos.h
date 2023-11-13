#ifndef  CLIB_BATTCLOCK_PROTOS_H
#define  CLIB_BATTCLOCK_PROTOS_H
/*
**      $Filename: clib/battclock_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:07 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "battclock.resource" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
void ResetBattClock( void );
ULONG ReadBattClock( void );
void WriteBattClock( unsigned long time );
#endif   /* CLIB_BATTCLOCK_PROTOS_H */
