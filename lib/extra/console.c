
/*
 *  CONSOLE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  OpenConsole()   - set this process's console as far as we can do such
 *		      things.
 */

#define DOSBase_DECLARED

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/alerts.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <lib/misc.h>

#define BTOC(bptr)  ((void *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)(((long)cptr) >> 2))

#define DOS_TRUE    (-1)
#define DOS_FALSE   (0)

#ifdef DEBUG
#define dbprintf(x) fhprintf x
#else
#define dbprintf(x)
#endif

typedef struct FileHandle   FileHandle;
typedef struct Process	    Process;
typedef struct List	    List;
typedef struct MsgPort	    MsgPort;
typedef struct Message	    Message;
typedef struct CommandLineInterface CLI;
typedef struct Task	    Task;

static BPTR  CustomCIS;
static BPTR  CustomCOS;
static BPTR  SaveCIS;
static BPTR  SaveCOS;
static MsgPort *SaveConsoleTask;

extern struct DosLibrary *DOSBase;

__autoexit static
void
opencon_exit(void)
{
    Process *proc = (Process *)FindTask(NULL);

    if (CustomCIS || CustomCOS)
	proc->pr_ConsoleTask = SaveConsoleTask;
    if (CustomCIS) {
	proc->pr_CIS = SaveCIS;
	Close(CustomCIS);
	CustomCIS = 0;
    }
    if (CustomCOS) {
	Write(CustomCOS, "**END**\n", 8);
	proc->pr_COS = SaveCOS;
	Close(CustomCOS);
	CustomCOS = 0;
    }
}

BOOL
OpenConsole(str)
const char *str;
{
    Process *proc = (Process *)FindTask(NULL);
    FileHandle *fh;
    BOOL r = FALSE;

    opencon_exit();
    if (CustomCIS = Open(str, 1005)) {
	fh = BTOC(CustomCIS);
	if (fh->fh_Type) {
	    r = TRUE;

	    SaveConsoleTask = proc->pr_ConsoleTask;
	    SaveCOS = proc->pr_COS;
	    SaveCIS = proc->pr_CIS;
	    proc->pr_ConsoleTask = fh->fh_Type;
	    proc->pr_COS = CustomCOS = Open("*", 1005);
	    proc->pr_CIS = CustomCIS;
	    freopen("*", "r", stdin);
	    freopen("*", "w", stdout);
	    freopen("*", "w", stderr);
	    /*proc->pr_ConsoleTask = SaveConsoleTask;*/
	    stdout->sd_Flags |= __SIF_IOLBF;
	    stderr->sd_Flags |= __SIF_IOLBF;
	} else {
	    Close(CustomCIS);
	    CustomCIS = 0;
	}
    }
    return(r);
}

