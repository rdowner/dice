#ifndef  CLIB_ALIB_STDIO_PROTOS_H
#define  CLIB_ALIB_STDIO_PROTOS_H
/*
**	$VER: alib_stdio_protos.h 39.1 (25.08.92)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
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
#endif	 /* CLIB_ALIB_STDIO_PROTOS_H */
