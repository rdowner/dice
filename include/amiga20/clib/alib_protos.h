#ifndef  CLIB_ALIB_PROTOS_H
#define  CLIB_ALIB_PROTOS_H
/*
**	$Filename: clib/alib_protos.h $
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
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
#ifndef  LIBRARIES_MATHFFP_H
#include <libraries/mathffp.h>
#endif
#ifndef  LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
void BeginIO( struct IORequest *io );
struct IORequest *CreateExtIO( struct MsgPort *msg, long size );
struct MsgPort *CreatePort( UBYTE *name, long pri );
struct IOStdReq *CreateStdIO( struct MsgPort *msg );
struct Task *CreateTask( UBYTE *name, long pri, APTR initPC,
	unsigned long stackSize );
void DeleteExtIO( struct IORequest *io );
void DeletePort( struct MsgPort *io );
void DeleteStdIO( struct IOStdReq *io );
void DeleteTask( struct Task *task );
void NewList( struct List *list );
LONG NameFromAnchor( struct AnchorPath *anchor, UBYTE *buffer, long buflen );
/* in clib; from graphics.library; */
void AddTOF( struct Isrvstr *i, long (*p)(), long a );
void RemTOF( struct Isrvstr *i );
void waitbeam( long b );
/* in math_support */
FLOAT afp( BYTE *string );
void arnd( long place, long exp, BYTE *string );
FLOAT dbf( unsigned long exp, unsigned long mant );
LONG fpa( FLOAT fnum, BYTE *string );
void fpbcd( FLOAT fnum, BYTE *string );
/* in timer_support */
LONG TimeDelay( long unit, unsigned long secs, unsigned long microsecs );
LONG DoTimer( struct timeval *, long unit, long command );
/**/
/*  Amiga.lib Functions */
/**/
void ArgArrayDone( void );
UBYTE **ArgArrayInit( long arg1, UBYTE **arg2 );
LONG ArgInt( UBYTE **arg1, UBYTE *arg2, long arg3 );
UBYTE *ArgString( UBYTE **arg1, UBYTE *arg2, UBYTE *arg3 );
CxObj *HotKey( UBYTE *arg1, struct MsgPort *arg2, long arg3 );
struct InputEvent *InvertString( UBYTE *arg1, ULONG *arg2 );
/**/
/* Macros */
/**/
/* CxObj  *CxCustom(LONG(*)(),LONG)		      (A0,D0) */
/* CxObj  *CxDebug(LONG)			      (D0) */
/* CxObj  *CxFilter(BYTE *)			      (A0) */
/* CxObj  *CxSender(struct MsgPort *,LONG)	      (A0,D0) */
/* CxObj  *CxSignal(struct Task *,LONG)	      (A0,D0) */
/* CxObj  *CxTranslate(struct InputEvent *)	      (A0) */
/* CxObj  *CxTypeFilter(LONG)			      (D0) */
#endif	 /* CLIB_ALIB_PROTOS_H */
