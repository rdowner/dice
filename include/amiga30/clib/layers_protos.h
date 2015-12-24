#ifndef  CLIB_LAYERS_PROTOS_H
#define  CLIB_LAYERS_PROTOS_H
/*
**	$VER: layers_protos.h 38.7 (26.03.92)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  GRAPHICS_LAYERS_H
#include <graphics/layers.h>
#endif
#ifndef  GRAPHICS_CLIP_H
#include <graphics/clip.h>
#endif
#ifndef  GRAPHICS_RASTPORT_H
#include <graphics/rastport.h>
#endif
#ifndef  GRAPHICS_REGIONS_H
#include <graphics/regions.h>
#endif
void InitLayers( struct Layer_Info *li );
struct Layer *CreateUpfrontLayer( struct Layer_Info *li, struct BitMap *bm,
	long x0, long y0, long x1, long y1, long flags, struct BitMap *bm2 );
struct Layer *CreateBehindLayer( struct Layer_Info *li, struct BitMap *bm,
	long x0, long y0, long x1, long y1, long flags, struct BitMap *bm2 );
LONG UpfrontLayer( long dummy, struct Layer *layer );
LONG BehindLayer( long dummy, struct Layer *layer );
LONG MoveLayer( long dummy, struct Layer *layer, long dx, long dy );
LONG SizeLayer( long dummy, struct Layer *layer, long dx, long dy );
void ScrollLayer( long dummy, struct Layer *layer, long dx, long dy );
LONG BeginUpdate( struct Layer *l );
void EndUpdate( struct Layer *layer, unsigned long flag );
LONG DeleteLayer( long dummy, struct Layer *layer );
void LockLayer( long dummy, struct Layer *layer );
void UnlockLayer( struct Layer *layer );
void LockLayers( struct Layer_Info *li );
void UnlockLayers( struct Layer_Info *li );
void LockLayerInfo( struct Layer_Info *li );
void SwapBitsRastPortClipRect( struct RastPort *rp, struct ClipRect *cr );
struct Layer *WhichLayer( struct Layer_Info *li, long x, long y );
void UnlockLayerInfo( struct Layer_Info *li );
struct Layer_Info *NewLayerInfo( void );
void DisposeLayerInfo( struct Layer_Info *li );
LONG FattenLayerInfo( struct Layer_Info *li );
void ThinLayerInfo( struct Layer_Info *li );
LONG MoveLayerInFrontOf( struct Layer *layer_to_move,
	struct Layer *other_layer );
struct Region *InstallClipRegion( struct Layer *layer,
	struct Region *region );
LONG MoveSizeLayer( struct Layer *layer, long dx, long dy, long dw, long dh );
struct Layer *CreateUpfrontHookLayer( struct Layer_Info *li, struct BitMap *bm,
	long x0, long y0, long x1, long y1, long flags, struct Hook *hook,
	struct BitMap *bm2 );
struct Layer *CreateBehindHookLayer( struct Layer_Info *li, struct BitMap *bm,
	long x0, long y0, long x1, long y1, long flags, struct Hook *hook,
	struct BitMap *bm2 );
struct Hook *InstallLayerHook( struct Layer *layer, struct Hook *hook );
/*--- functions in V39 or higher (beta release for developers only) ---*/
struct Hook *InstallLayerInfoHook( struct Layer_Info *li, struct Hook *hook );
void SortLayerCR( struct Layer *layer, long dx, long dy );
void DoHookClipRects( struct Hook *hook, struct RastPort *rport,
	struct Rectangle *rect );
#endif	 /* CLIB_LAYERS_PROTOS_H */
