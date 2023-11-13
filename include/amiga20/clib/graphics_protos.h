#ifndef  CLIB_GRAPHICS_PROTOS_H
#define  CLIB_GRAPHICS_PROTOS_H
/*
**      $Filename: clib/graphics_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:09 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "graphics.library" */
#ifndef  GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif
#ifndef  GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif
#ifndef  GRAPHICS_GELS_H
#include <graphics/gels.h>
#endif
#ifndef  GRAPHICS_RASTPORT_H
#include <graphics/rastport.h>
#endif
#ifndef  GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif
#ifndef  GRAPHICS_COPPER_H
#include <graphics/copper.h>
#endif
#ifndef  GRAPHICS_CLIP_H
#include <graphics/clip.h>
#endif
#ifndef  GRAPHICS_REGIONS_H
#include <graphics/regions.h>
#endif
#ifndef  GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif
#ifndef  GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif
#ifndef  HARDWARE_BLIT_H
#include <hardware/blit.h>
#endif
/*------ BitMap primitives ------*/
LONG BltBitMap( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct BitMap *destBitMap, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm, unsigned long mask,
        PLANEPTR tempA );
void BltTemplate( PLANEPTR source, long xSrc, long srcMod,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize );
/*------ Text routines ------*/
void ClearEOL( struct RastPort *rp );
void ClearScreen( struct RastPort *rp );
WORD TextLength( struct RastPort *rp, STRPTR string, unsigned long count );
LONG Text( struct RastPort *rp, STRPTR string, unsigned long count );
LONG SetFont( struct RastPort *rp, struct TextFont *textFont );
struct TextFont *OpenFont( struct TextAttr *textAttr );
void CloseFont( struct TextFont *textFont );
ULONG AskSoftStyle( struct RastPort *rp );
ULONG SetSoftStyle( struct RastPort *rp, unsigned long style,
        unsigned long enable );
/*------        Gels routines ------*/
void AddBob( struct Bob *bob, struct RastPort *rp );
void AddVSprite( struct VSprite *vSprite, struct RastPort *rp );
void DoCollision( struct RastPort *rp );
void DrawGList( struct RastPort *rp, struct ViewPort *vp );
void InitGels( struct VSprite *head, struct VSprite *tail,
        struct GelsInfo *gelsInfo );
void InitMasks( struct VSprite *vSprite );
void RemIBob( struct Bob *bob, struct RastPort *rp, struct ViewPort *vp );
void RemVSprite( struct VSprite *vSprite );
void SetCollision( unsigned long num,
        void (*routine)(struct VSprite *vSprite, APTR),
        struct GelsInfo *gelsInfo );
void SortGList( struct RastPort *rp );
void AddAnimOb( struct AnimOb *anOb, struct AnimOb **anKey,
        struct RastPort *rp );
void Animate( struct AnimOb **anKey, struct RastPort *rp );
BOOL GetGBuffers( struct AnimOb *anOb, struct RastPort *rp, long flag );
void InitGMasks( struct AnimOb *anOb );
/*------        General graphics routines ------*/
void DrawEllipse( struct RastPort *rp, long xCenter, long yCenter, long a,
        long b );
LONG AreaEllipse( struct RastPort *rp, long xCenter, long yCenter, long a,
        long b );
void LoadRGB4( struct ViewPort *vp, UWORD *colors, long count );
void InitRastPort( struct RastPort *rp );
void InitVPort( struct ViewPort *vp );
void MrgCop( struct View *view );
void MakeVPort( struct View *view, struct ViewPort *vp );
void LoadView( struct View *view );
void WaitBlit( void );
void SetRast( struct RastPort *rp, unsigned long pen );
void Move( struct RastPort *rp, long x, long y );
void Draw( struct RastPort *rp, long x, long y );
LONG AreaMove( struct RastPort *rp, long x, long y );
LONG AreaDraw( struct RastPort *rp, long x, long y );
LONG AreaEnd( struct RastPort *rp );
void WaitTOF( void );
void QBlit( struct bltnode *blit );
void InitArea( struct AreaInfo *areaInfo, APTR vectorBuffer,
        long maxVectors );
void SetRGB4( struct ViewPort *vp, long index, unsigned long red,
        unsigned long green, unsigned long blue );
void QBSBlit( struct bltnode *blit );
void BltClear( PLANEPTR memBlock, unsigned long byteCount,
        unsigned long flags );
void RectFill( struct RastPort *rp, long xMin, long yMin, long xMax,
        long yMax );
void BltPattern( struct RastPort *rp, PLANEPTR mask, long xMin, long yMin,
        long xMax, long yMax, unsigned long maskBPR );
ULONG ReadPixel( struct RastPort *rp, long x, long y );
LONG WritePixel( struct RastPort *rp, long x, long y );
BOOL Flood( struct RastPort *rp, unsigned long mode, long x, long y );
void PolyDraw( struct RastPort *rp, long count, WORD *polyTable );
void SetAPen( struct RastPort *rp, unsigned long pen );
void SetBPen( struct RastPort *rp, unsigned long pen );
void SetDrMd( struct RastPort *rp, unsigned long drawMode );
void InitView( struct View *view );
void CBump( struct UCopList *copList );
void CMove( struct UCopList *copList, APTR destination, long data );
void CWait( struct UCopList *copList, long v, long h );
LONG VBeamPos( void );
void InitBitMap( struct BitMap *bitMap, long depth, long width, long height );
void ScrollRaster( struct RastPort *rp, long dx, long dy, long xMin, long yMin,
        long xMax, long yMax );
void WaitBOVP( struct ViewPort *vp );
WORD GetSprite( struct SimpleSprite *sprite, long num );
void FreeSprite( long num );
void ChangeSprite( struct ViewPort *vp, struct SimpleSprite *sprite,
        PLANEPTR newData );
void MoveSprite( struct ViewPort *vp, struct SimpleSprite *sprite, long x,
        long y );
void LockLayerRom( struct Layer *layer );
void UnlockLayerRom( struct Layer *layer );
void SyncSBitMap( struct Layer *layer );
void CopySBitMap( struct Layer *layer );
void OwnBlitter( void );
void DisownBlitter( void );
struct TmpRas *InitTmpRas( struct TmpRas *tmpRas, PLANEPTR buffer,
        long size );
void AskFont( struct RastPort *rp, struct TextAttr *textAttr );
void AddFont( struct TextFont *textFont );
void RemFont( struct TextFont *textFont );
PLANEPTR AllocRaster( unsigned long width, unsigned long height );
void FreeRaster( PLANEPTR p, unsigned long width, unsigned long height );
void AndRectRegion( struct Region *region, struct Rectangle *rectangle );
BOOL OrRectRegion( struct Region *region, struct Rectangle *rectangle );
struct Region *NewRegion( void );
BOOL ClearRectRegion( struct Region *region, struct Rectangle *rectangle );
void ClearRegion( struct Region *region );
void DisposeRegion( struct Region *region );
void FreeVPortCopLists( struct ViewPort *vp );
void FreeCopList( struct CopList *copList );
void ClipBlit( struct RastPort *srcRP, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm );
BOOL XorRectRegion( struct Region *region, struct Rectangle *rectangle );
void FreeCprList( struct cprlist *cprList );
struct ColorMap *GetColorMap( long entries );
void FreeColorMap( struct ColorMap *colorMap );
ULONG GetRGB4( struct ColorMap *colorMap, long entry );
void ScrollVPort( struct ViewPort *vp );
struct CopList *UCopperListInit( struct UCopList *uCopList, long n );
void FreeGBuffers( struct AnimOb *anOb, struct RastPort *rp, long flag );
void BltBitMapRastPort( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm );
BOOL OrRegionRegion( struct Region *srcRegion, struct Region *destRegion );
BOOL XorRegionRegion( struct Region *srcRegion, struct Region *destRegion );
BOOL AndRegionRegion( struct Region *srcRegion, struct Region *destRegion );
void SetRGB4CM( struct ColorMap *colorMap, long index, unsigned long red,
        unsigned long green, unsigned long blue );
void BltMaskBitMapRastPort( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm, PLANEPTR bltMask );
BOOL AttemptLockLayerRom( struct Layer *layer );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
APTR GfxNew( unsigned long gfxNodeType );
void GfxFree( APTR gfxNodePtr );
void GfxAssociate( APTR associateNode, APTR gfxNodePtr );
void BitMapScale( struct BitScaleArgs *bitScaleArgs );
UWORD ScalerDiv( unsigned long factor, unsigned long numerator,
        unsigned long denominator );
WORD TextExtent( struct RastPort *rp, STRPTR string, long count,
        struct TextExtent *textExtent );
ULONG TextFit( struct RastPort *rp, STRPTR string, unsigned long strLen,
        struct TextExtent *textExtent, struct TextExtent *constrainingExtent,
        long strDirection, unsigned long constrainingBitWidth,
        unsigned long constrainingBitHeight );
APTR GfxLookUp( APTR associateNode );
BOOL VideoControl( struct ColorMap *colorMap, struct TagItem *tagarray );
struct MonitorSpec *OpenMonitor( STRPTR monitorName,
        unsigned long displayID );
BOOL CloseMonitor( struct MonitorSpec *monitorSpec );
DisplayInfoHandle FindDisplayInfo( unsigned long displayID );
ULONG NextDisplayInfo( unsigned long displayID );
ULONG GetDisplayInfoData( DisplayInfoHandle handle, UBYTE *buf,
        unsigned long size, unsigned long tagID, unsigned long displayID );
void FontExtent( struct TextFont *font, struct TextExtent *fontExtent );
LONG ReadPixelLine8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long width, UBYTE *array,
        struct RastPort *tempRP );
LONG WritePixelLine8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long width, UBYTE *array,
        struct RastPort *tempRP );
LONG ReadPixelArray8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long xstop, unsigned long ystop,
        UBYTE *array, struct RastPort *temprp );
LONG WritePixelArray8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long xstop, unsigned long ystop,
        UBYTE *array, struct RastPort *temprp );
LONG GetVPModeID( struct ViewPort *vp );
LONG ModeNotAvailable( unsigned long modeID );
WORD WeighTAMatch( struct TextAttr *reqTextAttr,
        struct TextAttr *targetTextAttr, struct TagItem *targetTags );
void EraseRect( struct RastPort *rp, long xMin, long yMin, long xMax,
        long yMax );
ULONG ExtendFont( struct TextFont *font, struct TagItem *fontTags );
void StripFont( struct TextFont *font );
#endif   /* CLIB_GRAPHICS_PROTOS_H */
