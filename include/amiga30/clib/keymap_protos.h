#ifndef  CLIB_KEYMAP_PROTOS_H
#define  CLIB_KEYMAP_PROTOS_H
/*
**      $VER: keymap_protos.h 36.4 (19.07.90)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
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
#endif   /* CLIB_KEYMAP_PROTOS_H */
