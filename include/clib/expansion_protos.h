/* $VER: clib/expansion_protos.h 1.0 (23.1.94) */
#ifndef EXPANSION_PROTOS_H
#define EXPANSION_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/expansion_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct Library *ExpansionBase;
#include <pragmas/expansion_pragmas.h>
#endif
#endif
#endif
