
/*
 *  DCC/DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#ifdef AMIGA

#define DLIBPRE ""
#define DLIBPOS "dlib:"

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <lib/misc.h>
#include <lib/unix.h>

#ifndef INCLUDE_VERSION 	/*  2.0 compilation compat check */
#define INCLUDE_VERSION     0
#else
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#endif

#else

#define DLIBPRE		INSTDIR
#define DLIBPOS		"dlib/"

#include <suplib/lists.h>
#include <suplib/string.h>
#include <suplib/stdlib.h>
#include <suplib/memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __aligned
#undef __aligned
#endif
#define __aligned

#endif

#ifdef DEBUG
#define dbprintf(x) if (DDebug) printf x
#else
#define dbprintf(x)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#include <include/lib/version.h>	/* DICE specific include */

#define Prototype   extern
#define Local

#define BTOC(bptr,ctype)    ((ctype *)((int32_t)bptr << 2))
#define CTOB(ptr)           ((int32_t)(ptr) >> 2)

typedef unsigned char	ubyte;
typedef unsigned short	uword;
typedef struct CommandLineInterface	CLI;
typedef struct Process	     Process;
typedef struct List	     List;
typedef struct FileInfoBlock FIB;
typedef struct Node	     Node;

#define DICE_C	    0
#define LATTICE_C   1
#define AZTEC_C     2

#include "dcc-protos.h"

