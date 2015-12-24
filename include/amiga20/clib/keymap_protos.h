#ifndef  CLIB_KEYMAP_PROTOS_H
#define  CLIB_KEYMAP_PROTOS_H
/*
**	$Filename: clib/keymap_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:10 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "keymap.library" */
#ifndef  DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
void SetKeyMapDefault( struct KeyMap *keyMap );
struct KeyMap *AskKeyMapDefault( void );
WORD MapRawKey( struct InputEvent *event, STRPTR buffer, long length,
	struct KeyMap *keyMap );
LONG MapANSI( STRPTR string, long count, STRPTR buffer, long length,
	struct KeyMap *keyMap );
#endif	 /* CLIB_KEYMAP_PROTOS_H */
