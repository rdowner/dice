#ifndef  CLIB_CIA_PROTOS_H
#define  CLIB_CIA_PROTOS_H
/*
**      $VER: cia_protos.h 1.7 (19.07.90)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
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
#endif   /* CLIB_CIA_PROTOS_H */
