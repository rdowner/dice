#ifndef  CLIB_WB_PROTOS_H
#define  CLIB_WB_PROTOS_H
/*
**	$VER: wb_protos.h 38.4 (31.05.92)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif
#ifndef  INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

struct AppWindow *AddAppWindowA( unsigned long id, unsigned long userdata,
	struct Window *window, struct MsgPort *msgport,
	struct TagItem *taglist );
struct AppWindow *AddAppWindow( unsigned long id, unsigned long userdata,
	struct Window *window, struct MsgPort *msgport, Tag tag1, ... );

BOOL RemoveAppWindow( struct AppWindow *appWindow );

struct AppIcon *AddAppIconA( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
	struct DiskObject *diskobj, struct TagItem *taglist );
struct AppIcon *AddAppIcon( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
	struct DiskObject *diskobj, Tag tag1, ... );

BOOL RemoveAppIcon( struct AppIcon *appIcon );

struct AppMenuItem *AddAppMenuItemA( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct TagItem *taglist );
struct AppMenuItem *AddAppMenuItem( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, Tag tag1, ... );

BOOL RemoveAppMenuItem( struct AppMenuItem *appMenuItem );

/*--- functions in V39 or higher (beta release for developers only) ---*/


void WBInfo( BPTR lock, STRPTR name, struct Screen *screen );


#endif	 /* CLIB_WB_PROTOS_H */
