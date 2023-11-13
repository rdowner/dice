#ifndef  CLIB_CONSOLE_PROTOS_H
#define  CLIB_CONSOLE_PROTOS_H
/*
**      $Filename: clib/console_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:08 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "console.device" */
#ifndef  EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#ifndef  DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
struct InputEvent *CDInputHandler( struct InputEvent *events,
        struct Library *consoleDevice );
LONG RawKeyConvert( struct InputEvent *events, STRPTR buffer, long length,
        struct KeyMap *keyMap );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#endif   /* CLIB_CONSOLE_PROTOS_H */
