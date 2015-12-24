
/*
 *  DEFS.H
 *
 *  (c)Copyright 1990, All Rights Reserved
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
#ifdef LATTICE			/*  only req'd for Lattice resident */
#include <proto/exec.h>
#include <proto/dos.h>
#define __aligned
#else
#include <lib/misc.h>
#include <lib/unix.h>
#endif

#ifndef INCLUDE_VERSION 	/*  2.0 compilation compat check */
#define INCLUDE_VERSION     0
#else
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#endif

#else

#define DLIBPRE "/usr/local/dice/"
#define DLIBPOS "dlib/"

#include <suplib/lists.h>
#include <suplib/string.h>
#include <suplib/stdlib.h>
#include <suplib/memory.h>
#include <sys/stat.h>

#define __aligned

#endif

#ifdef DEBUG
#define dbprintf(x) if (DDebug) printf x
#else
#define dbprintf(x)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#ifdef AMIGA
#include <lib/version.h>
#endif

#define Prototype   extern
#define Local

#define BTOC(bptr,ctype)    ((ctype *)((long)bptr << 2))
#define CTOB(ptr)           ((long)(ptr) >> 2)

typedef unsigned char	ubyte;
typedef unsigned short	uword;
typedef unsigned long	ulong;
typedef struct CommandLineInterface	CLI;
typedef struct Process	     Process;
typedef struct List	     List;
typedef struct FileInfoBlock FIB;
typedef struct Node	     Node;

#define DICE_C	    0
#define LATTICE_C   1
#define AZTEC_C     2



#include "DCC-protos.h"

