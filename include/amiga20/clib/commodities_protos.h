#ifndef  CLIB_COMMODITIES_PROTOS_H
#define  CLIB_COMMODITIES_PROTOS_H
/*
**	$Filename: clib/commodities_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:07 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "commodities.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_NODES_H
#include <exec/nodes.h>
#endif
#ifndef  LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif
#ifndef  DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
/**/
/*  OBJECT UTILITIES */
/**/
CxObj *CreateCxObj( unsigned long type, long arg1, long arg2 );
CxObj *CxBroker( struct NewBroker *nb, LONG *error );
LONG ActivateCxObj( CxObj *co, long true );
void DeleteCxObj( CxObj *co );
void DeleteCxObjAll( CxObj *co );
ULONG CxObjType( CxObj *co );
LONG CxObjError( CxObj *co );
void ClearCxObjError( CxObj *co );
void SetCxObjPri( CxObj *co, long pri );
/**/
/*  OBJECT ATTACHMENT */
/**/
void AttachCxObj( CxObj *headobj, CxObj *co );
void EnqueueCxObj( CxObj *headobj, CxObj *co );
void InsertCxObj( CxObj *headobj, CxObj *co, CxObj *pred );
void RemoveCxObj( CxObj *co );
/**/
/*  TYPE SPECIFIC */
/**/
void SetTranslate( CxObj *translator, IX *ie );
void SetFilter( CxObj *filter, IX *text );
void SetFilterIX( CxObj *filter, IX *ix );
LONG ParseIX( UBYTE *descr, IX *ix );
/**/
/*  COMMON MESSAGE */
/**/
ULONG CxMsgType( CxMsg *cxm );
UBYTE *CxMsgData( CxMsg *cxm );
LONG CxMsgID( CxMsg *cxm );
/**/
/*  MESSAGE ROUTING */
/**/
void DivertCxMsg( CxMsg *cxm, CxObj *headobj, CxObj *ret );
void RouteCxMsg( CxMsg *cxm, CxObj *co );
void DisposeCxMsg( CxMsg *cxm );
/**/
/*  INPUT EVENT HANDLING */
/**/
ULONG InvertKeyMap( unsigned long ansicode, struct InputEvent *ie,
	struct KeyMap *km );
void AddIEvents( struct InputEvent *ie );
/**/
/* FOR USE ONLY BY CONTROLLER PROGRAM */
/**/
#endif	 /* CLIB_COMMODITIES_PROTOS_H */
