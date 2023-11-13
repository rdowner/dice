// dinclude:local/defs.h

#include        <exec/types.h>
#include        <exec/execbase.h>
#include        <exec/nodes.h>
#include        <exec/lists.h>
#include        <exec/libraries.h>
#include        <exec/ports.h>
#include        <exec/memory.h>
#include        <exec/execbase.h>
#include        <utility/tagitem.h>
#include        <hardware/custom.h>
#include        <hardware/dmabits.h>
#include        <intuition/intuitionbase.h>
#include        <intuition/gadgetclass.h>
#include        <libraries/gadtools.h>
#include        <graphics/gfxbase.h>
#include        <graphics/view.h>
#include        <graphics/displayinfo.h>
#include        <graphics/gfxmacros.h>
#include        <devices/console.h>
#include        <devices/conunit.h>
#include        <devices/serial.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>
#include        <dos/filehandler.h>
#include        <dos/doshunks.h>
#include        <workbench/workbench.h>
#include        <workbench/icon.h>
#include        <clib/macros.h>

#include        <stdlib.h>
#include        <stdio.h>
#include        <stdarg.h>
#include        <fcntl.h>
#include        <string.h>
#include        <lib/rexx.h>
#include        <lib/bcpl.h>
#include        <lib/misc.h>

#include        <clib/exec_protos.h>
#include        <clib/dos_protos.h>
#include        <clib/intuition_protos.h>
#include        <clib/diskfont_protos.h>
#include        <clib/graphics_protos.h>
#include        <clib/gadtools_protos.h>
#include        <clib/asl_protos.h>
#include        <clib/alib_protos.h>
#include        <clib/icon_protos.h>
#include        <clib/wb_protos.h>


#define O_READ  (O_RDONLY)
#define O_WRITE (O_WRONLY|O_CREAT|O_TRUNC)


typedef struct TagItem          TAGS;

// dos.library
typedef BPTR                    LOCK;
typedef struct FileInfoBlock    FIB;
typedef struct Process          APROCESS;
typedef struct CommandLineInterface CLI;

// exec.library
typedef struct ExecBase         EBASE;
typedef struct Node             NODE;
typedef struct List             LIST;
typedef struct Library          LIBRARY;
typedef struct Message          MSG;
typedef struct MsgPort          MPORT;
typedef struct Task             TASK;

// intuition.library
typedef struct IntuitionBase    IBASE;
typedef struct VisualInfo       VINFO;
typedef struct Gadget           GADGET;
typedef struct NewGadget        NEWGAD;
typedef struct Screen           SCREEN;
typedef struct Window           WINDOW;
typedef struct IntuiMessage     IMSG;
typedef struct StringInfo       STRINGINFO;

// graphics.library
typedef struct GfxBase          GBASE;
typedef struct View             VIEW;
typedef struct ViewPort         VPORT;
typedef struct RastPort         RPORT;
typedef struct BitMap           BITMAP;
typedef struct RasInfo          RINFO;
typedef struct TextAttr         TATTR;
typedef struct Rectangle        RECT;
typedef struct TextFont         FONT;

// devices
typedef struct IORequest        IOREQ;
typedef struct IOStdReq         STDREQ;
typedef struct IOExtSer         SERREQ;

// workbench.library
typedef struct WBStartup        WBMSG;

// asl.library
typedef struct FontRequester    FONTREQ;
typedef struct FileRequester    FILEREQ;

// typedef unsigned short          USHORT;

extern __far struct Custom      custom;
extern GBASE                    *GfxBase;

// For makeproto utility (use it!!!)
#define Prototype               extern
#define Local                   static

