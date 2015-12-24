/* $VER: clib/disk_protos.h 1.0 (23.1.94) */
#ifndef DISK_PROTOS_H
#define DISK_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/disk_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct Library *DiskBase;
#include <pragmas/disk_pragmas.h>
#endif
#endif
#endif
