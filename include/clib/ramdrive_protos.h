/* $VER: clib/ramdrive_protos.h 1.0 (23.1.94) */
#ifndef RAMDRIVE_PROTOS_H
#define RAMDRIVE_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/ramdrive_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct Library *RamdriveBase;
#include <pragmas/ramdrive_pragmas.h>
#endif
#endif
#endif
