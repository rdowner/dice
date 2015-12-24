/* $VER: clib/graphics_protos.h 1.0 (23.1.94) */
#ifndef GRAPHICS_PROTOS_H
#define GRAPHICS_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/graphics_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct GfxBase *GfxBase;
#include <pragmas/graphics_pragmas.h>
#endif
#endif
#endif
