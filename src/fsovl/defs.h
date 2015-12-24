
/*
 *  DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lists.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <lib/version.h>

#define Prototype extern

#define DOS_TRUE    -1
#define DOS_FALSE   0

#define GEF_DIRTY	0x00000001
#define GEF_COMPRESSED	0x00000002
#define GEF_CACHED	0x00000004
#define GEF_DIRECTORY	0x00000008

#define BTOC(bptr)  ((void *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)(((long)cptr) >> 2))

#ifdef FSDEBUG
#define dbprintf(x) if (DDebug) printf x
#else
#define dbprintf(x)
#endif

typedef unsigned char ubyte;
typedef unsigned short uword;
#ifndef linux
typedef unsigned long ulong;
#endif

typedef struct DosList	    DosList;
typedef struct RootNode	    RootNode;
typedef struct DosInfo	    DosInfo;
typedef struct DosPacket    DosPacket;
typedef struct StandardPacket StdPacket;
typedef struct FileHandle   FileHandle;
typedef struct DeviceNode   DeviceNode;
typedef struct Process	    Process;
typedef struct Node	    Node;
typedef struct List	    List;
typedef struct MsgPort	    MsgPort;
typedef struct Message	    Message;
typedef struct FileLock	    FileLock;
typedef struct FileInfoBlock FileInfoBlock;

/*
 * GEntry - control node for a file or directory in the heirarchy.
 *
 * MUST BE LONGWORD ALIGNED (BSTR requirements in AllocGEntry)
 */

typedef struct GEntry {
    Node    ge_Node;		// node in dir list, file/dir name, type
    short   ge_Pad;
    long    ge_Lock;		// lock in overlayed filesystem
    uword   ge_FHRefs;		// file handle refs
    uword   ge_LCRefs;		// lock refs, including parent dir
    List    ge_List;		// if directory, list of sub-files
    ubyte   *ge_SoftLink;	// if softlink, pointer to relative link path
    ubyte   *ge_Buf;		// file buffer
    long    ge_Bytes;		// uncompressed file size (loaded or not)
    long    ge_Max;		// maximum allocated so far
    ulong   ge_Flags;		// various flags
    struct GEntry *ge_Parent;	// parent lock, NULL if root
} GEntry;

#define GETYPE_FILE	1
#define GETYPE_DIR	2

/*
 * GHandle - control node representing an overlayed file handle
 */

typedef struct GHandle {
    // Node    gh_Node;		// list of handles
    long    gh_Pos;		// current position
    long    gh_Mode;
    GEntry  *gh_GEntry;		// underlaying control node
} GHandle;

#include <fsovl-protos.h>

