/* $VER: proto/console.h 1.0 (17.4.93) */
#ifndef CONSOLE_PROTO_H
#define CONSOLE_PROTO_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/console_protos.h>
#ifdef __SUPPORTS_PRAGMAS__
extern struct Library *ConsoleDevice;
#include <pragmas/console_pragmas.h>
#endif
#endif
