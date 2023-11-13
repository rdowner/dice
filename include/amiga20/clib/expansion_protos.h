#ifndef  CLIB_EXPANSION_PROTOS_H
#define  CLIB_EXPANSION_PROTOS_H
/*
**      $Filename: clib/expansion_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:09 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
void AddConfigDev( struct ConfigDev *configDev );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
BOOL AddBootNode( long bootPri, unsigned long flags,
        struct DeviceNode *deviceNode, struct ConfigDev *configDev );
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
void AllocBoardMem( unsigned long slotSpec );
struct ConfigDev *AllocConfigDev( void );
APTR AllocExpansionMem( unsigned long numSlots, unsigned long slotAlign );
void ConfigBoard( APTR board, struct ConfigDev *configDev );
void ConfigChain( APTR baseAddr );
struct ConfigDev *FindConfigDev( struct ConfigDev *oldConfigDev,
        long manufacturer, long product );
void FreeBoardMem( unsigned long startSlot, unsigned long slotSpec );
void FreeConfigDev( struct ConfigDev *configDev );
void FreeExpansionMem( unsigned long startSlot, unsigned long numSlots );
UBYTE ReadExpansionByte( APTR board, unsigned long offset );
void ReadExpansionRom( APTR board, struct ConfigDev *configDev );
void RemConfigDev( struct ConfigDev *configDev );
void WriteExpansionByte( APTR board, unsigned long offset,
        unsigned long byte );
void ObtainConfigBinding( void );
void ReleaseConfigBinding( void );
void SetCurrentBinding( struct CurrentBinding *currentBinding,
        unsigned long bindingSize );
ULONG GetCurrentBinding( struct CurrentBinding *currentBinding,
        unsigned long bindingSize );
struct DeviceNode *MakeDosNode( APTR parmPacket );
BOOL AddDosNode( long bootPri, unsigned long flags,
        struct DeviceNode *deviceNode );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#endif   /* CLIB_EXPANSION_PROTOS_H */
