/* $VER: proto/exec.h 1.0 (17.4.93) */
#ifndef EXEC_PROTO_H
#define EXEC_PROTO_H 1
#include <pragmas/config.h>
#include <exec/types.h>
#include <clib/exec_protos.h>
#ifdef __SUPPORTS_PRAGMAS__
extern struct ExecBase *SysBase;
#include <pragmas/exec_pragmas.h>
#endif
#endif
