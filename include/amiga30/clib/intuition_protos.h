#ifndef  CLIB_INTUITION_PROTOS_H
#define  CLIB_INTUITION_PROTOS_H
/*
**	$VER: intuition_protos.h 38.16 (28.08.92)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif
#ifndef  INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif
#ifndef  INTUITION_CGHOOKS_H
#include <intuition/cghooks.h>
#endif
#ifndef  INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif
/* Public functions OpenIntuition() and Intuition() are intentionally */
/* not documented. */
void OpenIntuition( void );
void Intuition( struct InputEvent *iEvent );
UWORD AddGadget( struct Window *window, struct Gadget *gadget,
	unsigned long position );
BOOL ClearDMRequest( struct Window *window );
void ClearMenuStrip( struct Window *window );
void ClearPointer( struct Window *window );
BOOL CloseScreen( struct Screen *screen );
void CloseWindow( struct Window *window );
LONG CloseWorkBench( void );
void CurrentTime( ULONG *seconds, ULONG *micros );
BOOL DisplayAlert( unsigned long alertNumber, UBYTE *string,
	unsigned long height );
void DisplayBeep( struct Screen *screen );
BOOL DoubleClick( unsigned long sSeconds, unsigned long sMicros,
	unsigned long cSeconds, unsigned long cMicros );
void DrawBorder( struct RastPort *rp, struct Border *border, long leftOffset,
	long topOffset );
void DrawImage( struct RastPort *rp, struct Image *image, long leftOffset,
	long topOffset );
void EndRequest( struct Requester *requester, struct Window *window );
struct Preferences *GetDefPrefs( struct Preferences *preferences, long size );
struct Preferences *GetPrefs( struct Preferences *preferences, long size );
void InitRequester( struct Requester *requester );
struct MenuItem *ItemAddress( struct Menu *menuStrip,
	unsigned long menuNumber );
BOOL ModifyIDCMP( struct Window *window, unsigned long flags );
void ModifyProp( struct Gadget *gadget, struct Window *window,
	struct Requester *requester, unsigned long flags,
	unsigned long horizPot, unsigned long vertPot,
	unsigned long horizBody, unsigned long vertBody );
void MoveScreen( struct Screen *screen, long dx, long dy );
void MoveWindow( struct Window *window, long dx, long dy );
void OffGadget( struct Gadget *gadget, struct Window *window,
	struct Requester *requester );
void OffMenu( struct Window *window, unsigned long menuNumber );
void OnGadget( struct Gadget *gadget, struct Window *window,
	struct Requester *requester );
void OnMenu( struct Window *window, unsigned long menuNumber );
struct Screen *OpenScreen( struct NewScreen *newScreen );
struct Window *OpenWindow( struct NewWindow *newWindow );
ULONG OpenWorkBench( void );
void PrintIText( struct RastPort *rp, struct IntuiText *iText, long left,
	long top );
void RefreshGadgets( struct Gadget *gadgets, struct Window *window,
	struct Requester *requester );
UWORD RemoveGadget( struct Window *window, struct Gadget *gadget );
/* The official calling sequence for ReportMouse is given below. */
/* Note the register order.  For the complete story, read the ReportMouse */
/* autodoc. */
void ReportMouse( long flag, struct Window *window );
void ReportMouse1( struct Window *window, long flag );
BOOL Request( struct Requester *requester, struct Window *window );
void ScreenToBack( struct Screen *screen );
void ScreenToFront( struct Screen *screen );
BOOL SetDMRequest( struct Window *window, struct Requester *requester );
BOOL SetMenuStrip( struct Window *window, struct Menu *menu );
void SetPointer( struct Window *window, UWORD *pointer, long height,
	long width, long xOffset, long yOffset );
void SetWindowTitles( struct Window *window, UBYTE *windowTitle,
	UBYTE *screenTitle );
void ShowTitle( struct Screen *screen, long showIt );
void SizeWindow( struct Window *window, long dx, long dy );
struct View *ViewAddress( void );
struct ViewPort *ViewPortAddress( struct Window *window );
void WindowToBack( struct Window *window );
void WindowToFront( struct Window *window );
BOOL WindowLimits( struct Window *window, long widthMin, long heightMin,
	unsigned long widthMax, unsigned long heightMax );
/*--- start of next generation of names -------------------------------------*/
struct Preferences *SetPrefs( struct Preferences *preferences, long size,
	long inform );
/*--- start of next next generation of names --------------------------------*/
LONG IntuiTextLength( struct IntuiText *iText );
BOOL WBenchToBack( void );
BOOL WBenchToFront( void );
/*--- start of next next next generation of names ---------------------------*/
BOOL AutoRequest( struct Window *window, struct IntuiText *body,
	struct IntuiText *posText, struct IntuiText *negText,
	unsigned long pFlag, unsigned long nFlag, unsigned long width,
	unsigned long height );
void BeginRefresh( struct Window *window );
struct Window *BuildSysRequest( struct Window *window, struct IntuiText *body,
	struct IntuiText *posText, struct IntuiText *negText,
	unsigned long flags, unsigned long width, unsigned long height );
void EndRefresh( struct Window *window, long complete );
void FreeSysRequest( struct Window *window );
/* The return codes for MakeScreen(), RemakeDisplay(), and RethinkDisplay() */
/* are only valid under V39 and greater.  Do not examine them when running */
/* on pre-V39 systems! */
LONG MakeScreen( struct Screen *screen );
LONG RemakeDisplay( void );
LONG RethinkDisplay( void );
/*--- start of next next next next generation of names ----------------------*/
APTR AllocRemember( struct Remember **rememberKey, unsigned long size,
	unsigned long flags );
/* Public function AlohaWorkbench() is intentionally not documented */
void AlohaWorkbench( long wbport );
void FreeRemember( struct Remember **rememberKey, long reallyForget );
/*--- start of 15 Nov 85 names ------------------------*/
ULONG LockIBase( unsigned long dontknow );
void UnlockIBase( unsigned long ibLock );
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
LONG GetScreenData( APTR buffer, unsigned long size, unsigned long type,
	struct Screen *screen );
void RefreshGList( struct Gadget *gadgets, struct Window *window,
	struct Requester *requester, long numGad );
UWORD AddGList( struct Window *window, struct Gadget *gadget,
	unsigned long position, long numGad, struct Requester *requester );
UWORD RemoveGList( struct Window *remPtr, struct Gadget *gadget,
	long numGad );
void ActivateWindow( struct Window *window );
void RefreshWindowFrame( struct Window *window );
BOOL ActivateGadget( struct Gadget *gadgets, struct Window *window,
	struct Requester *requester );
void NewModifyProp( struct Gadget *gadget, struct Window *window,
	struct Requester *requester, unsigned long flags,
	unsigned long horizPot, unsigned long vertPot,
	unsigned long horizBody, unsigned long vertBody, long numGad );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
LONG QueryOverscan( unsigned long displayID, struct Rectangle *rect,
	long oScanType );
void MoveWindowInFrontOf( struct Window *window,
	struct Window *behindWindow );
void ChangeWindowBox( struct Window *window, long left, long top, long width,
	long height );
struct Hook *SetEditHook( struct Hook *hook );
LONG SetMouseQueue( struct Window *window, unsigned long queueLength );
void ZipWindow( struct Window *window );
/*--- public screens ---*/
struct Screen *LockPubScreen( UBYTE *name );
void UnlockPubScreen( UBYTE *name, struct Screen *screen );
struct List *LockPubScreenList( void );
void UnlockPubScreenList( void );
UBYTE *NextPubScreen( struct Screen *screen, UBYTE *namebuf );
void SetDefaultPubScreen( UBYTE *name );
UWORD SetPubScreenModes( unsigned long modes );
UWORD PubScreenStatus( struct Screen *screen, unsigned long statusFlags );

struct RastPort *ObtainGIRPort( struct GadgetInfo *gInfo );
void ReleaseGIRPort( struct RastPort *rp );
void GadgetMouse( struct Gadget *gadget, struct GadgetInfo *gInfo,
	WORD *mousePoint );
void GetDefaultPubScreen( UBYTE *nameBuffer );
LONG EasyRequestArgs( struct Window *window, struct EasyStruct *easyStruct,
	ULONG *idcmpPtr, APTR args );
LONG EasyRequest( struct Window *window, struct EasyStruct *easyStruct,
	ULONG *idcmpPtr, ... );
struct Window *BuildEasyRequestArgs( struct Window *window,
	struct EasyStruct *easyStruct, unsigned long idcmp, APTR args );
struct Window *BuildEasyRequest( struct Window *window,
	struct EasyStruct *easyStruct, unsigned long idcmp, ... );
LONG SysReqHandler( struct Window *window, ULONG *idcmpPtr, long waitInput );
struct Window *OpenWindowTagList( struct NewWindow *newWindow,
	struct TagItem *tagList );
struct Window *OpenWindowTags( struct NewWindow *newWindow,
	unsigned long tag1Type, ... );
struct Screen *OpenScreenTagList( struct NewScreen *newScreen,
	struct TagItem *tagList );
struct Screen *OpenScreenTags( struct NewScreen *newScreen,
	unsigned long tag1Type, ... );

/*	new Image functions */
void DrawImageState( struct RastPort *rp, struct Image *image, long leftOffset,
	long topOffset, unsigned long state, struct DrawInfo *drawInfo );
BOOL PointInImage( unsigned long point, struct Image *image );
void EraseImage( struct RastPort *rp, struct Image *image, long leftOffset,
	long topOffset );

APTR NewObjectA( struct IClass *classPtr, UBYTE *classID,
	struct TagItem *tagList );
APTR NewObject( struct IClass *classPtr, UBYTE *classID, unsigned long tag1,
	... );

void DisposeObject( APTR object );
ULONG SetAttrsA( APTR object, struct TagItem *tagList );
ULONG SetAttrs( APTR object, unsigned long tag1, ... );

ULONG GetAttr( unsigned long attrID, APTR object, ULONG *storagePtr );

/*	special set attribute call for gadgets */
ULONG SetGadgetAttrsA( struct Gadget *gadget, struct Window *window,
	struct Requester *requester, struct TagItem *tagList );
ULONG SetGadgetAttrs( struct Gadget *gadget, struct Window *window,
	struct Requester *requester, unsigned long tag1, ... );

/*	for class implementors only */
APTR NextObject( APTR objectPtrPtr );
struct IClass *MakeClass( UBYTE *classID, UBYTE *superClassID,
	struct IClass *superClassPtr, unsigned long instanceSize,
	unsigned long flags );
void AddClass( struct IClass *classPtr );


struct DrawInfo *GetScreenDrawInfo( struct Screen *screen );
void FreeScreenDrawInfo( struct Screen *screen, struct DrawInfo *drawInfo );

BOOL ResetMenuStrip( struct Window *window, struct Menu *menu );
void RemoveClass( struct IClass *classPtr );
BOOL FreeClass( struct IClass *classPtr );
/*--- functions in V39 or higher (beta release for developers only) ---*/
struct ScreenBuffer *AllocScreenBuffer( struct Screen *sc, struct BitMap *bm,
	unsigned long flags );
void FreeScreenBuffer( struct Screen *sc, struct ScreenBuffer *sb );
ULONG ChangeScreenBuffer( struct Screen *sc, struct ScreenBuffer *sb );
void ScreenDepth( struct Screen *screen, unsigned long flags, APTR reserved );
void ScreenPosition( struct Screen *screen, unsigned long flags, long x1,
	long y1, long x2, long y2 );
void ScrollWindowRaster( struct Window *win, long dx, long dy, long xMin,
	long yMin, long xMax, long yMax );
void LendMenus( struct Window *fromwindow, struct Window *towindow );
ULONG DoGadgetMethodA( struct Gadget *gad, struct Window *win,
	struct Requester *req, Msg message );
ULONG DoGadgetMethod( struct Gadget *gad, struct Window *win,
	struct Requester *req, unsigned long MethodID, ... );
void SetWindowPointerA( struct Window *win, struct TagItem *taglist );
void SetWindowPointer( struct Window *win, unsigned long tag1, ... );
BOOL TimedDisplayAlert( unsigned long alertNumber, UBYTE *string,
	unsigned long height, unsigned long time );
void HelpControl( struct Window *win, unsigned long flags );
#endif	 /* CLIB_INTUITION_PROTOS_H */
