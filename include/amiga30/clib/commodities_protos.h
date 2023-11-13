#ifndef  CLIB_COMMODITIES_PROTOS_H
#define  CLIB_COMMODITIES_PROTOS_H
/*
**      $VER: commodities_protos.h 38.3 (24.10.91)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
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

/*  OBJECT UTILITIES */

CxObj *CreateCxObj( unsigned long type, long arg1, long arg2 );
CxObj *CxBroker( struct NewBroker *nb, LONG *error );
LONG ActivateCxObj( CxObj *co, long true );
void DeleteCxObj( CxObj *co );
void DeleteCxObjAll( CxObj *co );
ULONG CxObjType( CxObj *co );
LONG CxObjError( CxObj *co );
void ClearCxObjError( CxObj *co );
LONG SetCxObjPri( CxObj *co, long pri );

/*  OBJECT ATTACHMENT */

void AttachCxObj( CxObj *headObj, CxObj *co );
void EnqueueCxObj( CxObj *headObj, CxObj *co );
void InsertCxObj( CxObj *headObj, CxObj *co, CxObj *pred );
void RemoveCxObj( CxObj *co );

/*  TYPE SPECIFIC */

void SetTranslate( CxObj *translator, struct InputEvent *events );
void SetFilter( CxObj *filter, STRPTR text );
void SetFilterIX( CxObj *filter, IX *ix );
LONG ParseIX( STRPTR description, IX *ix );

/*  COMMON MESSAGE */

ULONG CxMsgType( CxMsg *cxm );
APTR CxMsgData( CxMsg *cxm );
LONG CxMsgID( CxMsg *cxm );

/*  MESSAGE ROUTING */

void DivertCxMsg( CxMsg *cxm, CxObj *headObj, CxObj *returnObj );
void RouteCxMsg( CxMsg *cxm, CxObj *co );
void DisposeCxMsg( CxMsg *cxm );

/*  INPUT EVENT HANDLING */

BOOL InvertKeyMap( unsigned long ansiCode, struct InputEvent *event,
        struct KeyMap *km );
void AddIEvents( struct InputEvent *events );
/*--- functions in V38 or higher (distributed as Release 2.1) ---*/
/*  MORE INPUT EVENT HANDLING */
BOOL MatchIX( struct InputEvent *event, IX *ix );
#endif   /* CLIB_COMMODITIES_PROTOS_H */
