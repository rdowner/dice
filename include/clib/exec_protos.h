/* $VER: clib/exec_protos.h 1.0 (23.1.94) */
#ifndef EXEC_PROTOS_H
#define EXEC_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/exec_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct ExecBase *SysBase;
#include <pragmas/exec_pragmas.h>
#endif
#endif
#endif
