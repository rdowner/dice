#ifndef  CLIB_ALIB_PROTOS_H
#define  CLIB_ALIB_PROTOS_H
/*
**      $VER: alib_protos.h 1.5 (25.08.92)
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
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
#ifndef  LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef  INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif
#ifndef  GRAPHICS_GRAPHINT_H
#include <graphics/graphint.h>
#endif

/*  Exec support functions */

void BeginIO( struct IORequest *ioReq );
struct IORequest *CreateExtIO( struct MsgPort *port, long ioSize );
struct MsgPort *CreatePort( STRPTR name, long pri );
struct IOStdReq *CreateStdIO( struct MsgPort *port );
struct Task *CreateTask( STRPTR name, long pri, APTR initPC,
        unsigned long stackSize );
void DeleteExtIO( struct IORequest *ioReq );
void DeletePort( struct MsgPort *ioReq );
void DeleteStdIO( struct IOStdReq *ioReq );
void DeleteTask( struct Task *task );
void NewList( struct List *list );

/* Assorted functions in amiga.lib */

ULONG FastRand( unsigned long seed );
UWORD RangeRand( unsigned long maxValue );

/* Graphics support functions in amiga.lib */

void AddTOF( struct Isrvstr *i, long (*p)(), long a );
void RemTOF( struct Isrvstr *i );
void waitbeam( long b );

/* math support functions in amiga.lib */

FLOAT afp( BYTE *string );
void arnd( long place, long exp, BYTE *string );
FLOAT dbf( unsigned long exp, unsigned long mant );
LONG fpa( FLOAT fnum, BYTE *string );
void fpbcd( FLOAT fnum, BYTE *string );

/* Timer support functions in amiga.lib (V36 and higher only) */

LONG TimeDelay( long unit, unsigned long secs, unsigned long microsecs );
LONG DoTimer( struct timeval *, long unit, long command );

/*  Commodities functions in amiga.lib (V36 and higher only) */

void ArgArrayDone( void );
UBYTE **ArgArrayInit( long argc, UBYTE **argv );
LONG ArgInt( UBYTE **tt, STRPTR entry, long defaultval );
STRPTR ArgString( UBYTE **tt, STRPTR entry, STRPTR defaulstring );
CxObj *HotKey( STRPTR description, struct MsgPort *port, long id );
struct InputEvent *InvertString( STRPTR str, struct KeyMap *km );
void FreeIEvents( struct InputEvent *events );

/* Commodities Macros */

/* CxObj *CxCustom(LONG(*)(),LONG id)(A0,D0) */
/* CxObj *CxDebug(LONG id)(D0) */
/* CxObj *CxFilter(STRPTR description)(A0) */
/* CxObj *CxSender(struct MsgPort *port,LONG id)(A0,D0) */
/* CxObj *CxSignal(struct Task *task,LONG signal)(A0,D0) */
/* CxObj *CxTranslate(struct InputEvent *ie)(A0) */

/*  ARexx support functions in amiga.lib */

BOOL CheckRexxMsg( struct Message *rexxmsg );
LONG GetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE **result );
LONG SetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE *value,
        long length );

/*  Intuition hook and boopsi support functions in amiga.lib  */
/*  These functions require ROM V36 and higher only. */

ULONG CallHookA( struct Hook *hookPtr, Object *obj, APTR message );
ULONG CallHook( struct Hook *hookPtr, Object *obj, ... );
ULONG DoMethodA( Object *obj, Msg message );
ULONG DoMethod( Object *obj, unsigned long MethodID, ... );
ULONG DoSuperMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG DoSuperMethod( struct IClass *cl, Object *obj, unsigned long MethodID,
        ... );
ULONG CoerceMethodA( struct IClass *cl, Object *obj, Msg message );
ULONG CoerceMethod( struct IClass *cl, Object *obj, unsigned long MethodID,
        ... );
ULONG SetSuperAttrs( struct IClass *cl, Object *obj, unsigned long Tag1,
        ... );
#endif   /* CLIB_ALIB_PROTOS_H */
