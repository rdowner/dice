#ifndef  CLIB_TRANSLATOR_PROTOS_H
#define  CLIB_TRANSLATOR_PROTOS_H
/*
**      $Filename: clib/translator_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:13 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "translator.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
LONG Translate( STRPTR inputString, long inputLength, STRPTR outputBuffer,
        long bufferSize );
#endif   /* CLIB_TRANSLATOR_PROTOS_H */
