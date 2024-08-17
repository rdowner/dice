/* $VER: clib/timer_protos.h 1.0 (23.1.94) */
#ifndef TIMER_PROTOS_H
#define TIMER_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/timer_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct Library *TimerBase;
#include <pragmas/timer_pragmas.h>
#endif
#endif
#endif
