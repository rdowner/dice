#ifndef  CLIB_TIMER_PROTOS_H
#define  CLIB_TIMER_PROTOS_H
/*
**      $Filename: clib/timer_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:13 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "Timer.Device" */
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
void AddTime( struct timeval *dest, struct timeval *src );
void SubTime( struct timeval *dest, struct timeval *src );
LONG CmpTime( struct timeval *dest, struct timeval *src );
ULONG ReadEClock( struct EClockVal *dest );
void GetSysTime( struct timeval *dest );
#endif   /* CLIB_TIMER_PROTOS_H */
