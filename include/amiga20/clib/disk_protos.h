#ifndef  CLIB_DISK_PROTOS_H
#define  CLIB_DISK_PROTOS_H
/*
**      $Filename: clib/disk_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:14 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "disk.resource" */
#ifndef  RESOURCES_DISK_H
#include <resources/disk.h>
#endif
BOOL AllocUnit( long unitNum );
void FreeUnit( long unitNum );
struct DiskResourceUnit *GetUnit( struct DiskResourceUnit *unitPointer );
void GiveUnit( void );
LONG GetUnitID( long unitNum );
/*------ new for V37 ------*/
LONG ReadUnitID( long unitNum );
#endif   /* CLIB_DISK_PROTOS_H */
