#ifndef  CLIB_POTGO_PROTOS_H
#define  CLIB_POTGO_PROTOS_H
/*
**      $VER: potgo_protos.h 36.3 (07.11.90)
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
UWORD AllocPotBits( unsigned long bits );
void FreePotBits( unsigned long bits );
void WritePotgo( unsigned long word, unsigned long mask );
#endif   /* CLIB_POTGO_PROTOS_H */
