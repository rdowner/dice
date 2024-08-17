/* $VER: clib/dos_protos.h 1.0 (23.1.94) */
#ifndef DOS_PROTOS_H
#define DOS_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/dos_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct DosLibrary *DOSBase;
#include <pragmas/dos_pragmas.h>
#endif
#endif
#endif
