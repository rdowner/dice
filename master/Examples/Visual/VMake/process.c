#include "vmake.h"
#include <dos/dostags.h>

Prototype int InitSession(void);
Prototype void TermSession(void);
Prototype void PostLog(char *msg);
Prototype int IssueCommand(char *cmd);
Prototype int setup_cli(void);

/******************************************************************************/
static APTR    pr_ConsoleTask;  /* Console handler process for current window */

static BPTR    Outfh;           /* Filehandle for session output              */
static BPTR    Sesfh;           /* Filehandle for session Input/Output        */
static LONG    cli_Interactive; /* Boolean; True if prompts required          */
static LONG    cli_Background;  /* Boolean; True if CLI created by RUN        */

static struct Process *SesProcess;
/******************************************************************************/

/***********************************************************************************
 * Procedure: InitSession
 * Synopsis:  rc = InitSession()FreeVec(vec);
 * Purpose:   This code returns a vector to storage
 ***********************************************************************************/
int InitSession()
{
   struct FileHandle *handle;
   struct CommandLineInterface *cli;

   if (!Sesfh)
   {
char cbuf[256];
      SesProcess = (struct Process *)FindTask(0);
      cli = BADDR(SesProcess->pr_CLI);

      /* Save the current console and background status information */
      pr_ConsoleTask = SesProcess->pr_ConsoleTask;

      cli_Interactive    = cli->cli_Interactive;
      cli_Background     = cli->cli_Background;

      /* Open up our console to do the work */
      if (!build_command(cbuf, 255, global.text[CONFIG_CONSOLE], 0))
         /* JAT's usual inverted logic... */
         Sesfh = Open(cbuf, MODE_OLDFILE);
      else
         /* error message (since we don't knowwhat build_command() produced */
         strcpy(cbuf, global.text[CONFIG_CONSOLE]);
      if (Sesfh == NULL) 
      {
         Sesfh = Open("Con:0/0/320/100/Vmake_Default/Auto", MODE_OLDFILE);
         if (Sesfh == NULL)
            /* nowhere to put an error message, give up */
            return(1);
         else
         {
            char buf[256];
            
            sprintf(buf, "Bad Console \"%s\" from file: %s\n", 
                    cbuf, Sym_Lookup(SYM_CONFIG));
            PostLog(buf);
         }
      }
      handle = (struct FileHandle *)(Sesfh << 2);
      SesProcess->pr_CIS = SesProcess->pr_COS = Sesfh;

      SesProcess->pr_ConsoleTask = (APTR)handle->fh_Type;

      if (DOSBase->dl_lib.lib_Version >= 36)
         Outfh = Open("*", MODE_NEWFILE);

      cli->cli_Interactive    = DOSTRUE;
      cli->cli_Background     = DOSFALSE;
      atexit(TermSession);
   }
   return(0);
}

/***********************************************************************************
 * Procedure: TermSession
 * Synopsis:  (void)TermSession();
 * Purpose:   Terminate usage of the Session
 ***********************************************************************************/
void TermSession()
{
   struct CommandLineInterface *cli;
   if (Sesfh)
   {
      cli = BADDR(SesProcess->pr_CLI);

      Close(Sesfh);
      if (Outfh) Close(Outfh);
      Sesfh = Outfh = 0;
      SesProcess->pr_ConsoleTask = pr_ConsoleTask;
      cli->cli_Interactive       = cli_Interactive;
      cli->cli_Background        = cli_Background;
   }
}

/***********************************************************************************
 * Procedure: PostLog
 * Synopsis:  (void)PostLog(message);
 * Purpose:   Output a message to the session log
 ***********************************************************************************/
void PostLog(char *msg)
{
   if (!InitSession())
   {
      Write(Sesfh, msg, strlen(msg));
      Write(Sesfh, "\n", 1);
   }
}


/***********************************************************************************
 * Procedure: IssueCommand
 * Synopsis:  rc = IssueCommand(cmd);
 * Purpose:   Execute a given command with the current console
 ***********************************************************************************/
int IssueCommand(char *cmd)
{
   int rc;
   struct TagItem taglist[4];

   rc = InitSession();
   if (!rc)
   {
      if (Outfh)
      {
         taglist[0].ti_Tag  = SYS_UserShell;
         taglist[0].ti_Data = 1;
         taglist[1].ti_Tag  = SYS_Input;
         taglist[1].ti_Data = (ULONG)Sesfh;
         taglist[2].ti_Tag  = SYS_Output;
         taglist[2].ti_Data = (ULONG)Outfh;
         taglist[3].ti_Tag  = TAG_DONE;

         rc = SystemTagList(cmd, taglist);
      }
      else
      {
         rc = Execute(cmd, 0L, Sesfh);
      }
   }
   return(rc);
}

/***********************************************************************************
 * Procedure: Alloc_Vec
 * Synopsis:  newvec = Alloc_Vec(size);
 * Purpose:   This code allocates a DOS vector
 * Note:      This storage is NOT automatically freed by the compiler library.
 ***********************************************************************************/
static BPTR Alloc_Vec(int size)
{
   long *new;

   size += 4;
   /* Based on the given size, allocate the right amount of memory */
   new = (long *)AllocMem(size, MEMF_PUBLIC | MEMF_CLEAR);

   if (new != NULL)
   {
      /* Remember to point one past the length longword */
      *new++ = size;
   }
   else
   {
      request(1, TEXT_NOMEM, NULL, NULL);
   }
   return(MKBADDR(new));
}

/***********************************************************************************
 * Procedure: Free_Vec
 * Synopsis:  (void)FreeVec(vec);
 * Purpose:   This code returns a vector to storage
 ***********************************************************************************/
static void Free_Vec(BPTR orig)
{
   long *in;

   in = (long *)BADDR(orig);
   in--;  /* Back up to the size information for the original allocation */

   /* Based on the given size, allocate the right amount of memory */
   FreeMem(in, *in);
}

/* This structure maps a DOS Path entry.  This is not really documented in any */
/* of the DOS include files but is generally understood                        */
/* Note that it must allocated as a dos vector (length byte in front of it)    */
struct PathEnt {
  BPTR nextent;
  BPTR lock;
};

/***********************************************************************************
 * Procedure: freecli
 * Synopsis:  (void)freecli();
 * Purpose:   This code frees up any CLI allocated structures.
 * Note:      This routine is only called as an autoexit routine when setup_cli
 *            has done some work.  Otherwise we assume that no special work had to
 *            be done to make things happen.
 ***********************************************************************************/
static void freecli()
{
   struct CommandLineInterface *cli;
   BPTR oldent;

   cli = BADDR(SesProcess->pr_CLI);

   if (!cli) return;

   /* We only need to free the 4 vectors we created and all of the */
   /* locks on the the path list.                                  */
   Free_Vec(cli->cli_SetName);
   Free_Vec(cli->cli_CommandName);
   Free_Vec(cli->cli_Prompt);
   Free_Vec(cli->cli_CommandFile);

   while((oldent = cli->cli_CommandDir) != 0)
   {
      struct PathEnt *pathent;

      /* First we need to remove us from the path list */
      pathent = BADDR(oldent);
      cli->cli_CommandDir = pathent->nextent;
      UnLock(pathent->lock);     /* Free the lock at this entry        */
      Free_Vec(oldent);          /* Free the storage for the path node */
   }
   SesProcess->pr_CLI = 0;
}

/***********************************************************************************
 * Procedure: setup_cli()
 * Synopsis:  (void)setup_cli();
 * Purpose:   This code sets up a fake CLI structure by copying the appropriate
 *            information from workbench.  It will prepare for that to be
 *            automatically be freed on program termination through freecli.
 ***********************************************************************************/
int setup_cli()
{
   struct CommandLineInterface *cli, *wbcli;
   struct Process *wbproc;
   struct PathEnt *pathent, *wbent;

   SesProcess = (struct Process *)FindTask(0);

   /* Allocate a CLI structure if we need one                        */
   if (SesProcess->pr_CLI) return;

   cli = get_mem(sizeof(struct CommandLineInterface));
   if (cli == NULL) return(1);

   atexit(freecli);

   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
   /* Next we need to go through and copy the workbench path over to      */
   /* Our process.  The only things we need to duplicate are:             */
   /*                                                                     */
   /* We need to copy over the BPTR link list for this one.               */
   /*    pr_CLI->cli_CommandDir   Head of the path locklist               */
   /*                                                                     */
   /* These are buffers which we need to allocate space for and clone     */
   /*    pr_CLI->cli_SetName      Name of current directory               */
   /*    pr_CLI->cli_CommandName  Name of current command                 */
   /*    pr_CLI->cli_Prompt       Current prompt (set by PROMPT)          */
   /*    pr_CLI->cli_CommandFile  Name of EXECUTE command file            */
   /*                                                                     */
   /* These fields are just copied over verbatim                          */
   /*    pr_CLI->cli_FailLevel    Fail level (set by FAILAT)              */
   /*    pr_CLI->cli_Interactive  Boolean; True if prompts required       */
   /*    pr_CLI->cli_Background   Boolean; True if CLI created by RUN     */
   /*    pr_CLI->cli_DefaultStack Stack size to be obtained in long words */
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
   SesProcess->pr_CLI = MKBADDR(cli);

   /* Now we need to find the workbench CLI */
   if ( ( (wbproc = (struct Process *)FindTask("Workbench")) == NULL) &&
        ( (wbproc = (struct Process *)FindTask("Fastbench")) == NULL) )
   {
      /* For some reason workbench and FastBench are not in the system */
      /* We need to punt and not let them really do any work           */
      return(2);
   }
   wbcli = BADDR(wbproc->pr_CLI);

   /* Say what??  We have a workbench but it doesn't have a CLI???   */
   if (wbcli == NULL) return(3);

   /* Do all of the staight copies from the Workbench CLI */
   cli->cli_FailLevel    = 20;
   cli->cli_Interactive  = DOSFALSE;
   cli->cli_Background   = DOSFALSE;
   cli->cli_DefaultStack = 2048; /* Longwords */

   /* Next we get all of the cloned vector buffers */
   cli->cli_SetName      = Alloc_Vec( 80);  /* These numbers are truely magic  */
   cli->cli_CommandName  = Alloc_Vec(104);  /* and CAN NOT be changed for true */
   cli->cli_Prompt       = Alloc_Vec( 60);  /* compatibility with 1.3.         */
   cli->cli_CommandFile  = Alloc_Vec( 40);  /* Don't even try to change them   */

   /* Lastly we need to copy over all of the paths from workbench */
   if (wbcli->cli_CommandDir)
   {
      /* Start out the process by copying over the first vector */
      cli->cli_CommandDir = Alloc_Vec(8);
      wbent   = BADDR(wbcli->cli_CommandDir);
      pathent = BADDR(cli->cli_CommandDir);

      /* Now loop through copying over the vector and then replacing everything */
      /* in place.  This looks a little strange because we are actually counting */
      /* on creating a structure with a pointer to the other linked list, but  */
      while(wbent->nextent)
      {
         pathent->nextent = Alloc_Vec(8);
         pathent->lock = DupLock(wbent->lock);
         if (!pathent->lock)
         {
            pathent->nextent = 0;  /* Make sure we terminate the list here */
            return(4);
         }
         wbent   = BADDR(wbent->nextent);
         pathent = BADDR(pathent->nextent);
      }
      pathent->lock = DupLock(wbent->lock);
      if (!pathent->lock) return(5);
   }
}
