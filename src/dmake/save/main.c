
/*
 *  MAIN.C
 */

#include "defs.h"
#ifdef AMIGA
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <clib/intuition_protos.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <clib/wb_protos.h>
#include <clib/icon_protos.h>
#include <lib/misc.h>
#endif

void help(int);
void InitStuff(void);
const char *SkipAss(const char *);
#ifdef AMIGA
struct IntuiText *ITextOf(char *);
#endif

Prototype List	DoList;
Prototype short DDebug;
Prototype short NoRunOpt;
Prototype short ExitCode;
Prototype short	DoAll;
Prototype short SomeWork;

List	DoList;
short	DDebug;
short	NoRunOpt;
short	QuietOpt;
short	DoAll;
short   SomeWork;
#ifdef AMIGA
short	XSaveLockValid;
BPTR	XSaveLock;
#endif
char	*XFileName = "DMakefile";
short	FileSpecified = 0;
short	ExitCode;

void
myexit()
{
if (SomeWork)
   printf("DMAKE Done.\n");
else
   printf("All Targets up to date.\n");

#ifdef AMIGA
    if (XSaveLockValid) {
	CurrentDir(XSaveLock);
	XSaveLockValid = 0;
    }
#endif
}

#ifdef AMIGA

wbmain(wbs)
struct WBStartup *wbs;
{
    struct DiskObject *dob;
    short i;
    short j;
    short abortIt = 0;

    /*
     *	Search for options, set current directory to last valid
     *	disk object
     */

    InitStuff();

    for (i = 0; i < wbs->sm_NumArgs; ++i) {
	BPTR saveLock = CurrentDir((BPTR)wbs->sm_ArgList[i].wa_Lock);

	if (i == wbs->sm_NumArgs - 1 && FileSpecified == 0)
	    XFileName = strdup(wbs->sm_ArgList[i].wa_Name);

	if (dob = GetDiskObject(wbs->sm_ArgList[i].wa_Name)) {
	    for (j = 0; dob->do_ToolTypes[j]; ++j) {
		char *ptr = dob->do_ToolTypes[j];

		if (strnicmp(ptr, "FILE=", 5) == 0) {
		    XFileName = strdup(SkipAss(ptr));
		    FileSpecified = 1;
		} else if (strnicmp(ptr, "DRYRUN=", 7) == 0) {
		    NoRunOpt = strtol(SkipAss(ptr), NULL, 0);
		} else if (strnicmp(ptr, "TARGET=", 7) == 0) {
		    CreateDepRef(&DoList, SkipAss(ptr));
		} else if (strnicmp(ptr, "QUIET=", 6) == 0) {
		    QuietOpt = strtol(SkipAss(ptr), NULL, 0);
		} else if (strnicmp(ptr, "DEBUG=", 6) == 0) {
		    DDebug = strtol(SkipAss(ptr), NULL, 0);
		} else if (strnicmp(ptr, "CONSOLE=", 8) == 0) {
		    OpenConsole(SkipAss(ptr)); /*  lib/misc.h  */
		} else {
		    char buf[64];

		    sprintf(buf, "Bad ToolType: %s", ptr);
		    switch(AutoRequest(NULL, ITextOf(ptr), ITextOf("Ignore"), ITextOf("Abort"), 0, 0, 300, 40)) {
		    case 1:
			break;
		    case 0:
			abortIt = 1;
			break;
		    }
		}
		if (abortIt)
		    break;
	    }
	    FreeDiskObject(dob);
	}
	CurrentDir(saveLock);
	if (abortIt)
	    break;
    }
#ifdef AMIGA
    XSaveLock = CurrentDir((BPTR)wbs->sm_ArgList[wbs->sm_NumArgs-1].wa_Lock);
    XSaveLockValid = 1;
#endif

    if (abortIt == 0)
	main(1, NULL);
}

#endif

main(ac, av)
char *av[];
{
    short i;
    int r = 0;

    InitStuff();

    /*printf("ARGS= %d\n", ac);*/
    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];

	/*printf("ARG[%d]= %d:%s\n", i, strlen(av[i]), av[i]);*/

	if (*ptr != '-') {
	    CreateDepRef(&DoList, ptr);
	    continue;
	}
	ptr += 2;
	switch(ptr[-1]) {
	case 'F':   /*  fast opt    */
	    break;
	case 'f':
	    XFileName = (*ptr) ? ptr : av[++i];
	    break;
	case 'n':
	    NoRunOpt = 1;
	    break;
	case 'D':
	    (*ptr) ? ptr : av[++i];
	    var = MakeVar(name, '$');
	    AppendVar(var, value, len);
	    break;
	case 'd':
	    DDebug = (*ptr) ? atoi(ptr) : 1;
	    break;
	case 'a':
	    DoAll = 1;
	case 'q':
	    QuietOpt = 1;
	    break;
	case 'h':
	default:
	    help(1);
	}
    }
    if (QuietOpt == 0)
	puts("DMake V2.1 " __DATE__);

    if (i > ac)
	error(FATAL, "Expected argument to command line option");

    /*
     *	resolve dependancies requested by the user.  If none requested
     *	the resolve the first one
     */

    {
	DepRef *node;

	ParseFile(XFileName);

	if (GetHead(&DoList) == NULL) {
	    if (node = GetHead(&DepList))
		CreateDepRef(&DoList, ((DepNode *)node)->dn_Node.ln_Name);
	}

	while (node = RemHead(&DoList)) {
	    time_t t;
            if ((node->rn_Dep->dn_Node.ln_Type != NT_RESOLVED) &&
               (GetHead(&node->rn_Dep->dn_DepCmdList) == NULL))
            {
                error(FATAL, "Unable to find %s", node->rn_Node.ln_Name);
                break;
            }

	    if ((r = ExecuteDependency(node, &t)) < 0)
	    {
		break;
            }
	}
    }
    if (r < 0 && ExitCode < 20)
	ExitCode = 20;
    return(ExitCode);
}

void
InitStuff()
{
    static int Initialized;

    atexit(myexit);
    if (Initialized == 0) {
	Initialized = 1;
	NewList(&DoList);
	InitCommand();
	InitCmdList();
	InitVar();
	InitDep();
	InitParser();
    }
}

void
help(code)
int code;
{
    puts("DMAKE V2.1 (c)Copyright 1991 Matthew Dillon, All Rights Reserved");
    puts("DMAKE [-f file] [-n]");
    exit(code);
}

const char *
SkipAss(ptr)
const char *ptr;
{
    while (*ptr && *ptr != '=')
	++ptr;
    if (*ptr == '=') {
	for (++ptr; *ptr == ' ' || *ptr == '\t'; ++ptr)
	    ;
    }
    return(ptr);
}

#ifdef AMIGA
struct IntuiText *
ITextOf(ptr)
char *ptr;
{
    static struct IntuiText ITAry[8];
    static short ITIdx;
    struct IntuiText *it = ITAry + ITIdx;

    ITIdx = (ITIdx + 1) & 7;
    it->FrontPen = 1;
    it->BackPen  = 0;
    it->DrawMode = JAM2;
    it->LeftEdge = 2;
    it->TopEdge = 6;
    it->IText = (unsigned char *)ptr;
    return(it);
}
#endif

