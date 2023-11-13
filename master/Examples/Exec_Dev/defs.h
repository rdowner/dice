
/*
 *  DEFS.H
 */

#define DEBUG 1

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <devices/trackdisk.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdio.h>
#define abs
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>

extern struct Library *SysBase;
extern struct DosLibrary *DosBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define Prototype extern
#define DevCall __geta4 __regargs

#define CMD_OPENUNIT    (0x7FF0 & ~TDF_EXTCOM)
#define CMD_CLOSEUNIT   (0x7FF1 & ~TDF_EXTCOM)
#define CMD_KILLPROC    (0x7FF2 & ~TDF_EXTCOM)

#define EXT_CHUNK   4096

typedef struct Library  LIB;
typedef struct Device   DEV;
typedef struct Process  PROC;
typedef struct MsgPort  PORT;
typedef struct Message  MSG;
typedef struct List     LIST;
typedef struct Node     NODE;
typedef long (*func_ptr)();

typedef struct {
    struct  Unit    U;
    UWORD   OpenCnt;
    long    Fh;         /*  file handle             */
    long    Size;       /*  current size            */
    long    Pos;        /*  current position        */
    char    Extended;   /*  file has been extended  */
    char    Reserved;
} NDUnit;

typedef struct {
    LIB     Lib;
    NDUnit  Unit[32];
} NDev;

typedef struct {
    struct  Message io_Message;
    struct  Device  *io_Device;     /* device node pointer  */
    struct  Unit    *io_Unit;       /* unit (driver private)*/
    UWORD   io_Command;             /* device command */
    UBYTE   io_Flags;
    BYTE    io_Error;               /* error or warning num */
    ULONG   io_Actual;              /* actual number of bytes transferred */
    ULONG   io_Length;              /* requested number bytes transferred*/
    APTR    io_Data;                /* points to data area */
    ULONG   io_Offset;              /* offset for block structured devices */

    long    iotd_Count;             /*  (extension)     */
    long    iotd_SecLabel;          /*  (extension)     */
} IOB;

extern char DeviceName[];
extern char IdString[];
extern void DUMmySeg(void);
extern void ADevExpunge(void);

