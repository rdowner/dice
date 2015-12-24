/* $VER: clib/console_protos.h 1.0 (23.1.94) */
#ifndef CONSOLE_PROTOS_H
#define CONSOLE_PROTOS_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/console_protos.h>  /* Note this is in the Amiga directory */
#if __SUPPORTS_PRAGMAS__
#ifdef __DICE_INLINE
extern struct Library *ConsoleBase;
#include <pragmas/console_pragmas.h>
#endif
#endif
#endif
