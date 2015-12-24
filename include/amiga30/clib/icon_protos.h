#ifndef  CLIB_ICON_PROTOS_H
#define  CLIB_ICON_PROTOS_H
/*
**	$VER: icon_protos.h 38.1 (24.06.91)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif
LONG GetIcon( UBYTE *name, struct DiskObject *icon,
	struct FreeList *freelist );
BOOL PutIcon( UBYTE *name, struct DiskObject *icon );
void FreeFreeList( struct FreeList *freelist );
BOOL AddFreeList( struct FreeList *freelist, APTR mem, unsigned long size );
struct DiskObject *GetDiskObject( UBYTE *name );
BOOL PutDiskObject( UBYTE *name, struct DiskObject *diskobj );
void FreeDiskObject( struct DiskObject *diskobj );
UBYTE *FindToolType( UBYTE **toolTypeArray, UBYTE *typeName );
BOOL MatchToolValue( UBYTE *typeString, UBYTE *value );
UBYTE *BumpRevision( UBYTE *newname, UBYTE *oldname );
struct DiskObject *GetDefDiskObject( long type );
BOOL PutDefDiskObject( struct DiskObject *diskObject );
struct DiskObject *GetDiskObjectNew( UBYTE *name );
BOOL DeleteDiskObject( UBYTE *name );
#endif	 /* CLIB_ICON_PROTOS_H */
