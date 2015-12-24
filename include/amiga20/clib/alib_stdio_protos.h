#ifndef  CLIB_ALIB_STDIO_PROTOS_H
#define  CLIB_ALIB_STDIO_PROTOS_H
/*
**	$Filename: clib/alib_stdio_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:06 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
/* functions that duplicate those in a typical C library */
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
