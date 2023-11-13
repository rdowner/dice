#ifndef  CLIB_REXXSYSLIB_PROTOS_H
#define  CLIB_REXXSYSLIB_PROTOS_H
/*
**      $VER: rexxsyslib_protos.h 36.3 (19.02.91)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  REXX_RXSLIB_H
#include <rexx/rxslib.h>
#endif
#ifndef  REXX_REXXIO_H
#include <rexx/rexxio.h>
#endif

UBYTE *CreateArgstring( UBYTE *string, unsigned long length );
void DeleteArgstring( UBYTE *argstring );
ULONG LengthArgstring( UBYTE *argstring );
struct RexxMsg *CreateRexxMsg( struct MsgPort *port, UBYTE *extension,
        UBYTE *host );
void DeleteRexxMsg( struct RexxMsg *packet );
void ClearRexxMsg( struct RexxMsg *msgptr, unsigned long count );
BOOL FillRexxMsg( struct RexxMsg *msgptr, unsigned long count,
        unsigned long mask );
BOOL IsRexxMsg( struct RexxMsg *msgptr );


void LockRexxBase( unsigned long resource );
void UnlockRexxBase( unsigned long resource );

#endif   /* CLIB_REXXSYSLIB_PROTOS_H */
