/* $VER: proto/timer.h 1.0 (17.4.93) */
#ifndef TIMER_PROTO_H
#define TIMER_PROTO_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/timer_protos.h>
#ifdef __SUPPORTS_PRAGMAS__
extern struct Library *TimerBase;
#include <pragmas/timer_pragmas.h>
#endif
#endif
