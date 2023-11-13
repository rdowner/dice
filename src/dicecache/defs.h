
/*
 *  DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>            /*  BPTR        */
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <string.h>
#include <stdio.h>              /*  fhprintf    */
#include <stdarg.h>
#include <stdlib.h>
#include <lists.h>
#define _DICECACHE_INTERNAL_SKIP
#include <dicecache_protos.h>

#define HSIZE   32
#define HMASK   (HSIZE-1)
#define Prototype   extern
#define BTOC(val)   ((void *)((long)(val) << 2))

#ifdef DEBUG
#define dbprintf(x) if (DDebug) xprintf x
#else
#define dbprintf(x)
#endif

#define LibCall __geta4 __regargs

typedef struct Message  Message;
typedef struct Library  Library;
typedef struct SignalSemaphore SignalSemaphore;
typedef struct FileInfoBlock   FileInfoBlock;
typedef struct FileLock FileLock;
typedef struct List     List;
typedef struct Node     Node;


typedef struct CacheNode {
    Node    cn_Node;        /*  w/name of file  */
    short   cn_Flags;       /*  longword align  */
    BPTR    cn_Lock;        /*  currentdir lock */
    char    *cn_Data;       /*  file data       */
    long    cn_Bytes;       /*  file bytes      */
    long    cn_Refs;
    __aligned FileInfoBlock cn_Fib;
} CacheNode;

#define CNF_VALID   0x0001  /*  open successfull    */
#define CNF_UNFIND  0x0002  /*  referenced and out of date, skip in search */

#include <dicecache-protos.h>

extern const char LibName[];
extern const char LibId[];

