#ifndef  CLIB_POTGO_PROTOS_H
#define  CLIB_POTGO_PROTOS_H
/*
**      $Filename: clib/potgo_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:12 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "potgo.resource" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
UWORD AllocPotBits( unsigned long bits );
void FreePotBits( unsigned long bits );
void WritePotgo( unsigned long word, unsigned long mask );
#endif   /* CLIB_POTGO_PROTOS_H */
