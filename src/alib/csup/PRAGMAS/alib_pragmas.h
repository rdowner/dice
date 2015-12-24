/*
**	$Id: alib_pragmas.h,v 30.0 1994/06/10 18:10:33 dice Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "amiga.lib" */
/* These prototypes are for functions in the amiga.lib link-library. */

/*  Exec support functions */

#pragma libcall None BeginIO 1e 801
#pragma libcall None CreateExtIO 24 0802
#pragma libcall None CreatePort 2a 0802
#pragma libcall None CreateStdIO 30 801
#pragma libcall None CreateTask 36 190804
#pragma libcall None DeleteExtIO 3c 801
#pragma libcall None DeletePort 42 801
#pragma libcall None DeleteStdIO 48 801
#pragma libcall None DeleteTask 4e 801
#pragma libcall None NewList 54 801
#pragma libcall None LibAllocPooled 5a 0802
#pragma libcall None LibCreatePool 60 21003
#pragma libcall None LibDeletePool 66 801
#pragma libcall None LibFreePooled 6c 09803

/* Assorted functions in amiga.lib */

#pragma libcall None FastRand 72 001
#pragma libcall None RangeRand 78 001

/* Graphics support functions in amiga.lib */

#pragma libcall None AddTOF 7e 09803
#pragma libcall None RemTOF 84 801
#pragma libcall None waitbeam 8a 001

/* math support functions in amiga.lib */

#pragma libcall None afp 90 801
#pragma libcall None arnd 96 81003
#pragma libcall None dbf 9c 1002
#pragma libcall None fpa a2 8002
#pragma libcall None fpbcd a8 8002

/* Timer support functions in amiga.lib (V36 and higher only) */

#pragma libcall None TimeDelay ae 21003
#pragma libcall None DoTimer b4 10803

/*  Commodities functions in amiga.lib (V36 and higher only) */

#pragma libcall None ArgArrayDone ba 00
#pragma libcall None ArgArrayInit c0 9802
#pragma libcall None ArgInt c6 09803
#pragma libcall None ArgString cc A9803
#pragma libcall None HotKey d2 09803
#pragma libcall None InvertString d8 9802
#pragma libcall None FreeIEvents de 801

/* Commodities Macros */

/* CxObj *CxCustom(LONG(*)(),LONG id)(A0,D0) */
/* CxObj *CxDebug(LONG id)(D0) */
/* CxObj *CxFilter(STRPTR description)(A0) */
/* CxObj *CxSender(struct MsgPort *port,LONG id)(A0,D0) */
/* CxObj *CxSignal(struct Task *task,LONG signal)(A0,D0) */
/* CxObj *CxTranslate(struct InputEvent *ie)(A0) */

/*  ARexx support functions in amiga.lib */

#pragma libcall None CheckRexxMsg e4 801
#pragma libcall None GetRexxVar ea 09803
#pragma libcall None SetRexxVar f0 109804

/*  Intuition hook and boopsi support functions in amiga.lib. */
/*  These functions do not require any particular ROM revision */
/*  to operate correctly, though they deal with concepts first introduced */
/*  in V36.  These functions would work with compatibly-implemented */
/*  hooks or objects under V34. */

#pragma libcall None CallHookA f6 9A803
#ifdef __SASC_60
#pragma tagcall None CallHook f6 9A803
#endif
#pragma libcall None DoMethodA fc 9A02
#ifdef __SASC_60
#pragma tagcall None DoMethod fc 9A02
#endif
#pragma libcall None DoSuperMethodA 102 9A803
#ifdef __SASC_60
#pragma tagcall None DoSuperMethod 102 9A803
#endif
#pragma libcall None CoerceMethodA 108 9A803
#ifdef __SASC_60
#pragma tagcall None CoerceMethod 108 9A803
#endif
/* Note that SetSuperAttrs() does not have a regargs equivalent. */
/* See amiga_lib.doc for the trivial invocation of the regargs version. */
#ifdef __SASC_60
#pragma tagcall None SetSuperAttrs 108 9A803
#endif

/*  Network-support functions in amiga.lib. */
/*  ACrypt() first appeared in later V39 versions of amiga.lib, but */
/*  operates correctly under V37 and up. */

#pragma libcall None ACrypt 10e A9803
