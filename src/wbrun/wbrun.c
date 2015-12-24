/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986, 1988 The Software Distillery.                */
/* |. o.| ||   All Rights Reserved. This program may not be distributed      */
/* | .  | ||   without the permission of the authors.                        */
/* | o  | ||  Alan Beale     Jim Cooper        Bruce Drake   Jay Denebeim    */
/* |  . |//   Gordon Keener  John Mainwaring   Jack Rouse    John Toebes     */
/* ======     Doug Walker                              BBS:(919)-471-6436    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <exec/types.h>
#include <exec/memory.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/icon.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <string.h>
#include "WBRun_rev.h"

int setarg(struct WBArg *, char *, BPTR);
void msg(char *, char *);
void main(int, char **);
struct MsgPort *MyCreatePort(char *name,long pri);
extern void __stdargs MemCleanup();
extern void __stdargs _main(register char *line);

/*-------------------------------------------------------------------------*/
/* Declarations for CBACK */

extern BPTR _Backstdout;         /* standard output when run in background */
long __BackGroundIO = 1;          /* Flag to tell it we want to do I/O      */
long __stack = 4000;              /* Amount of stack space our task needs   */
char *__procname = "WBRun" VERSTAG;	 /* The name of the task to create         */
long __priority = 0;              /* The priority to run us at              */

#define CSI "\0x9B"

struct DosLibrary *DOSBase;
struct Library *MathBase;
struct Library *MathTransBase;

struct Library *IconBase;
#define ICON_REV 0

#define MAXARG 32              /* maximum command line arguments */
#define QUOTE  '"'

#define isspace(c)	((c == ' ')||(c == '\t') || (c == '\n'))

/**
*
* name         _main - process command line, and call "main"
*
* synopsis     _main(line);
*              char *line;     ptr to command line that caused execution
*
* description	This function performs the standard pre-processing for
*		the main module of a C program.  It accepts a command
*		line of the form
*
*			pgmname arg1 arg2 ...
*
*		and builds a list of pointers to each argument.  The first
*		pointer is to the program name.
*
**/
void __stdargs _main(register char *line)
{
register short argc = 0;
register char **pargv;
char *argv[MAXARG];     /* arg pointers */

/*
*
* Build argument pointer list
*
*/
while (argc < MAXARG)
	{
	while (isspace(*line))	line++;
	if (*line == '\0')	break;
	pargv = &argv[argc++];
	if (*line == QUOTE)
		{
		*pargv = ++line;  /* ptr inside quoted string */
		while ((*line != '\0') && (*line != QUOTE)) line++;
		if (*line == '\0')  return;
		else		    *line++ = '\0';  /* terminate arg */
		}
	else		/* non-quoted arg */
		{
		*pargv = line;
		while ((*line != '\0') && (!isspace(*line))) line++;
		if (*line == '\0')  break;
		else 		    *line++ = '\0';  /* terminate arg */
		}
	}  /* while */

main((int)argc,argv);              /* call main function */
}

void main(argc, argv)
register int argc;
char **argv;
{
register struct WBStartup *WBStartup;
register struct DiskObject *diskobj;
char *torun, nambuf[160];
register int stacksize, i;
struct Process *ourtask;
struct MsgPort *replyport;
register BPTR olddir, lock;

/* initialize so our cleanup routine runs fine */
WBStartup = NULL;
IconBase  = NULL;
diskobj   = NULL;
replyport = NULL;
olddir    = NULL;

/* running this from workbench is ridiculous so just quit */
if (argc == 0) return;

/* issue the copyright notice - they can supress it by doing a WBRUN >NIL: */
msg( CSI "33m"
     "WBRun II"
     CSI "0m"
     " by John Toebes - Copyright © 1988 The Software Distillery\n",
     "235 Trillingham Ln, Cary NC 27513   BBS:(919)-471-6436");

/* not enough parameters? - give them the usage */
if (argc < 2)
   {
   msg(
"Usage: WBRun icon(s)\nWhere icon(s) is to run as if selected from Workbench\n",
"Note: Use WBRun >NIL: icon(s)   to hide copyright message");
   goto done;
   }

/* open the libraries we will need */
if ((IconBase = OpenLibrary(ICONNAME, ICON_REV)) == NULL)
   goto done;

/* find ourselves - if this doesn't work it deserves to die a horrible */
/* death so I will not even look at our result */
ourtask = (struct Process *)FindTask(NULL);

/* allocate storage for all the arguments on the list */
if (((WBStartup = (struct WBStartup *)
                  AllocMem(sizeof(struct WBStartup), MEMF_CLEAR)) == NULL) ||
    ((WBStartup->sm_ArgList = (struct WBArg *)
                     AllocMem(sizeof(struct WBArg)*argc, MEMF_CLEAR)) == NULL) ||
    ((replyport = MyCreatePort("PhoneyWorkbench", 0)) == NULL))
   goto done;

/* initialize the remainder of the startup fields */
WBStartup->sm_Message.mn_ReplyPort = replyport;
WBStartup->sm_NumArgs = argc-1;

/* run through all the arguments getting locks and names for them */
for (i=1; i<argc; i++)
   if (setarg(&WBStartup->sm_ArgList[i-1], argv[i],ourtask->pr_CurrentDir))
         goto done;

/*-- Load the code that is desired ---*/
/* to do this, first find the program to be run */
olddir = CurrentDir( WBStartup->sm_ArgList[0].wa_Lock );

if ((diskobj = (struct DiskObject *)
               GetDiskObject( WBStartup->sm_ArgList[0].wa_Name )) == NULL)
   {
   msg("Can't get icon for ", WBStartup->sm_ArgList[0].wa_Name);
   goto done;
   }

torun = argv[1];

/* once we have the object, look at the tool type */
/* for TOOLS we run the program (argv[0] in our case) */
/* For projects it has the name of the default tool to run with it */
/* all others are to be blown away */
if (diskobj->do_Type == WBPROJECT)
   {
   /* for a project, we need to insert the tool icon as the first tool */
   /* move everything over one */
   for (i=1; i<argc; i++)
      memcpy((char *)&WBStartup->sm_ArgList[i-1],
             (char *)&WBStartup->sm_ArgList[i],
              sizeof(struct WBArg));
   strcpy(nambuf, diskobj->do_DefaultTool);
   torun = nambuf;

   if (setarg(&WBStartup->sm_ArgList[0], torun, olddir))
      goto done;

   FreeDiskObject(diskobj);

   if ((diskobj = (struct DiskObject *)
                  GetDiskObject( WBStartup->sm_ArgList[0].wa_Name )) == NULL)
      {
      msg("Can't get icon for ", WBStartup->sm_ArgList[0].wa_Name);
      goto done;
      }

   WBStartup->sm_NumArgs++;
   }

/* at this point if it is not a TOOL we are scrod */
if (diskobj->do_Type != WBTOOL)
   {
   msg("Icon is not runnable for ", torun);
   goto done;
   }

if ((stacksize = diskobj->do_StackSize) < 4000)
   stacksize = 4000;

/* so lets load the segment for the program to run */
if ((WBStartup->sm_Segment = LoadSeg(torun)) == NULL)
   {
   msg("Can't open tool ", torun);
   goto done;
   }

/* also we will need to get it going as a process */
if ((WBStartup->sm_Process = (struct MsgPort *)
           CreateProc(torun, 0, WBStartup->sm_Segment, stacksize)) == NULL)
   {
   msg("Can't create process for ", torun);
   goto done;
   }

/* we are off and running, pass it the message to get running */
WBStartup->sm_ToolWindow = diskobj->do_ToolWindow;

PutMsg((struct MsgPort *)WBStartup->sm_Process, (struct Message *)WBStartup);

/* when he is done with what he needs, we can blow everything away */
WaitPort(replyport);

/* everything is now complete so clean up and go away */
done:
if (_Backstdout) Close(_Backstdout);
_Backstdout = NULL;

if (replyport != NULL) DeletePort(replyport);
replyport = NULL;

if (diskobj != NULL) FreeDiskObject(diskobj);
diskobj = NULL;

if (olddir != NULL) CurrentDir(olddir);

if (WBStartup != NULL)
   {
   if (WBStartup->sm_Segment != NULL)
      UnLoadSeg(WBStartup->sm_Segment);
   if (WBStartup->sm_ArgList != NULL)
      {
      /* run through and free any locks we may have */
      for (i=0; i<argc; i++)
         {
         lock = WBStartup->sm_ArgList[i].wa_Lock;
         if ((lock != NULL) && (lock != ourtask->pr_CurrentDir))
            UnLock(lock);
         }
      FreeMem((char *)(WBStartup->sm_ArgList),sizeof(struct WBArg)*argc);
      }
   FreeMem((char *)WBStartup, sizeof(struct WBStartup));
   }
WBStartup = NULL;

if (IconBase != NULL) CloseLibrary(IconBase);
IconBase = NULL;

}

/***********************************************************/
/* given a null terminated name, fill in a WBArg structure */
/* with a lock and a name relative to that lock            */
/***********************************************************/
int setarg(WBArg, name, curdir)
register struct WBArg *WBArg;
char *name;
BPTR curdir;
{
register char *p, *lastc;
register unsigned char c;

if (name == NULL || !*name) return(1);  /* bad name to use */

/* first find the last colon or slash in the name */
lastc = NULL;
for (p = name; *p; p++)
   if (*p == ':' || *p == '/')
      lastc = p+1;

/* was there a path delimiter at all ? */
if (lastc == NULL)
   {
   /* no, use the default lock and full name */
   WBArg->wa_Lock = curdir;
   WBArg->wa_Name = name;
   }
else
   {
   /* must get a lock on the right directory and use part of the name */
   if (!*lastc) return(1); /* only a drawer specified */
   WBArg->wa_Name = lastc;

   /* when setting a directory, we need to include the delimiter */
   c = *lastc;
   *lastc = NULL;
   if ((WBArg->wa_Lock = Lock(name, ACCESS_READ)) == NULL)
      {
      msg("Can't open directory:", name);
      return(1);  /* couldn't find the director */
      }
   *lastc++ = c;
   }

/* it worked so let them continue on */
return(0);
}

void msg(str1, str2)
char *str1, *str2;
{
if (_Backstdout)
   {
   Write(_Backstdout,str1,strlen(str1));
   Write(_Backstdout,str2,strlen(str2));
   Write(_Backstdout,"\n", 1);
   }
}

void __stdargs MemCleanup(void) {
}

struct MsgPort *MyCreatePort(char *name,long pri)
{
register UBYTE sigbit;
register struct MsgPort *port;

if ((sigbit = AllocSignal(-1)) == -1)
   return(NULL);

if ((port = (struct MsgPort *)
            AllocMem(sizeof(struct MsgPort), MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
   {
   FreeSignal(sigbit);
   return(NULL);
   }

port->mp_Node.ln_Name = name;
port->mp_Node.ln_Pri = pri;
port->mp_Node.ln_Type = NT_MSGPORT;
port->mp_Flags = PA_SIGNAL;
port->mp_SigBit = sigbit;
port->mp_SigTask = FindTask(0);

AddPort(port);

return(port);
}

void DeletePort(port)
register struct MsgPort *port;
{
RemPort(port);
FreeSignal(port->mp_SigBit);
FreeMem((char *)port, sizeof(struct MsgPort));
}
