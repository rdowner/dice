#ifndef  CLIB_ALIB_STDIO_PROTOS_H
#define  CLIB_ALIB_STDIO_PROTOS_H

/*
**      $Id: alib_stdio_protos.h,v 30.0 1994/06/10 18:10:34 dice Exp $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

/* stdio functions that duplicate those in a typical C library */

LONG printf( STRPTR fmt, ... );
LONG sprintf( STRPTR buffer, STRPTR fmt, ... );
LONG fclose( long stream );
LONG fgetc( long stream );
LONG fprintf( long stream, STRPTR fmt, ... );
LONG fputc( long c, long stream );
LONG fputs( UBYTE *s, long stream );
LONG getchar( void );
LONG putchar( long c );
LONG puts( BYTE *s );
#endif   /* CLIB_ALIB_STDIO_PROTOS_H */
