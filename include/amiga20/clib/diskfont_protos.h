#ifndef  CLIB_DISKFONT_PROTOS_H
#define  CLIB_DISKFONT_PROTOS_H
/*
**      $Filename: clib/diskfont_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:08 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "diskfont.library" */
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  LIBRARIES_DISKFONT_H
#include <libraries/diskfont.h>
#endif
struct TextFont *OpenDiskFont( struct TextAttr *textAttr );
LONG AvailFonts( STRPTR buffer, long bufBytes, long flags );
/*--- functions in V34 or higher (distributed as Release 1.3) ---*/
struct FontContentsHeader *NewFontContents( BPTR fontsLock, STRPTR fontName );
void DisposeFontContents( struct FontContentsHeader *fontContentsHeader );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
struct DiskFontHeader *NewScaledDiskFont( struct TextFont *sourceFont,
        struct TextAttr *destTextAttr );
#endif   /* CLIB_DISKFONT_PROTOS_H */
