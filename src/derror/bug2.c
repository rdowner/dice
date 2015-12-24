/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include <stdio.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <string.h>
#include <stdlib.h>
#include <clib/exec_protos.h>
#include <lib/rexx.h>


#include "error_rev.h"

#define PORTNAME "DICE_ERROR_PARSER"

typedef struct ErrorInfo {
    struct ErrorInfo *next;
    struct ErrorInfo *prev;
    char             line[1];
} EInfo;


struct FileInfo {
    struct FileInfo *next;
    struct FileInfo *prev;
    char            *dir;
    char            *args;
    struct ErrorInfo base;
    struct ErrorInfo *cur;
};

struct FileInfo files;
struct FileInfo *curfile;

char *RexxHostName = NULL;  /* no automatic start */

#define MAX_FILENAME 1024
char buf[MAX_FILENAME+1];

short active;

/***********************************************************************************
 * Procedure: AddLine
 * Synopsis:  AddLine(FileInfo *, char *)
 * Purpose:   Adds the line to the current file information.  If there is not enough
 *            memory, it will simply ignore the line silently.
 ***********************************************************************************/
void AddLine(struct FileInfo *fi, char *line)
{
   struct ErrorInfo *ei;

   ei = malloc(sizeof(struct ErrorInfo) + strlen(line));
   if (ei)
   {
      ei->prev = fi->base.prev;
      ei->next = fi->base.prev->next;
      fi->base.prev->next = ei;
      fi->base.prev = ei;
   }
}

/***********************************************************************************
 * Procedure: NewFI
 * Synopsis:  FileInfo = NewFI(char *dir, char *args)
 * Purpose:   Creates a new file info.  If it is unable to, it returns NULL
 ***********************************************************************************/
struct FileInfo *NewFI(char *dir, char *args)
{
   struct FileInfo *fi;

   fi = malloc(sizeof(struct FileInfo));
   if (fi)
   {
      fi->dir = strdup(dir);
      if (fi->dir)
      {
          fi->args = strdup(args);
          if (fi->args)
          {
             fi->prev = files.prev;
             fi->next = files.prev->next;
             files.prev->next = fi;
             files.prev = fi;

             fi->base.prev = fi->base.next = &fi->base;
             fi->cur = NULL;
          }
          else
          {
             free(fi->dir);
             free(fi);
             fi = NULL;
          }
      }
      else
      {
         free(fi);
         fi = NULL;
      }
   }
   return(fi);
}

/***********************************************************************************
 * Procedure: FreeFI
 * Synopsis:  FreeFI(struct FileInfo *)
 * Purpose:   Removes a FileInfo structure from the list
 ***********************************************************************************/
void FreeFI(struct FileInfo *fi)
{
   struct ErrorInfo *ei;

   /* Handle any attempts to free the base FI */
   if (fi == files) return;

   /* Unlink us from the chain of file handles */
   fi->prev->next = fi->next;
   fi->next->prev = fi->prev;

   /* Update any system pointers which might look at what we are going to free */
   if (curfile == fi)
   {
      curfile = files.next;
      if (curfile == files) curfile = NULL;  // no more anyway
   }

   /* Free all the lines */
   fi->base.prev->next == NULL;  // mark our stopping point
   for(ei = fi->base.next; ei != NULL;)
   {
      struct ErrorInfo *sei;

      sei = ei;
      ei = ei->next;
      free(sei);
   }

   /* Finally, get rid of the file information */
   free(fi->dir);
   free(fi->args);
   free(fi);
}
k
/***********************************************************************************
 * Procedure: say
 * Synopsis:  (void)say(msg);
 * Purpose:   Displays the given message on the console
 ***********************************************************************************/
void say(char *msg)
{
   BPTR out;
   out = Output();
   if (out)
   {
      Write(out, msg, strlen(msg));
      Write(out, "\n", 1);
   }
}

/***********************************************************************************
 * Procedure: usage
 * Synopsis:  (void)usage();
 * Purpose:   Displays the command line usage message - does not return
 ***********************************************************************************/
void usage(void)
{
   say("FILE/M,MACRO/K,PROJECT/K" VERSTAG);
   exit(20);
}

/***********************************************************************************
 * Procedure: mustmalloc
 * Synopsis:  mem = mustmalloc(size);
 * Purpose:   Allocates memory or Displays an outof memory message -
 *            if there is not enough memory, it does not return
 ***********************************************************************************/
void *mustmalloc(int size)
{
   void *r;

   r = malloc(size);
   if (r == NULL)
   {
      say("No Memory!\n");
      exit(20);
   }
   return(r);
}

/***********************************************************************************
 * Procedure: dottx
 * Synopsis:  result = dottx(port, cmd);
 * Purpose:   Sends a command to TurboText
 ***********************************************************************************/
char *dottx(char *port, char *cmd)
{
   char *res;
   long ec;

   if (port == NULL) port = "TURBOTEXT";
   PlaceRexxCommandDirect(NULL, port, cmd, &res, &ec);

   return(res);
}

/***********************************************************************************
 * Procedure: full_path
 * Synopsis:  path = full_path(name)
 * Purpose:   Constructs a fully expanded filename
 *            Note, we can not assume that the file exists, so it will not be possible
 *            to actually lock it.  We can assume that the directory it is part of
 *            does exist.
 ***********************************************************************************/
char *full_path(char *name)
{
   BPTR lock;
   __aligned struct FileInfoBlock fib;
   char *tail, *p;
   int pos;

   /* Step 1 - split out any directory information from the actual name */
   p = strrchr(name, '/');
   if (p == NULL) p = strrchr(name, ':');
   if (p != NULL)
   {
      /* There was some directory information involved */
      char c;
      tail = strdup(p+1);
      c = p[1];
      p[1] = 0;
      lock = Lock(name, SHARED_LOCK);
      p[1] = c;
   }
   else
   {
      /* No directory information involved, just the name relative to the */
      /* current directory                                                */
      lock = Lock("", SHARED_LOCK);
      tail = strdup(name);
   }

   /* Step 2 - we have the lock on the directory and the tail part of the name */
   /* We want to construct a fully qualified path for the directory.           */
   /* If for some reason the lock on the directory returned 0, we want to just */
   /* return the name they gave us to begin with.                              */
   if (lock == 0)
   {
      free(tail);
      return(strdup(name));
   }

   /* Step 3 - Fully qualify the directory portion into the buffer */
   if (DOSBase->dl_lib.lib_Version >= 36)
   {
      if (!NameFromLock(lock, buf, MAX_FILENAME))
      {
         /* Either the name is too long or there was something else wrong with */
         /* the file name, just return what they gave us as a start            */
         UnLock(lock);
         free(tail);
         return(strdup(name));
      }
      UnLock(lock);
      pos = 0;
   }
   else
   {
      /* Running under 1.3, we have to do this the old fashion way */

      /* Just so we don't have to do any inserts/extra copies, we will work */
      /* from the end of the buffer and insert as we go                     */
      pos = MAX_FILENAME;
      buf[--pos] = 0;
      while(lock != 0)
      {
         BPTR parent;
         int len;

         /* Examine the lock to get the name for it */
         Examine(lock, &fib);

         /* Find the parent of this directory       */
         parent = ParentDir(lock);
         UnLock(lock);
         lock = parent;

         len = strlen(fib.fib_FileName);
         pos -= 1;

         if (len > pos)
         {
            /* oops, not enough room, just return the name they gave us */
            UnLock(lock);
            free(tail);
            return(strdup(name));
         }
         buf[pos] = lock ? ':' : '/';
         pos -= len;
         memcpy(buf+pos, fib.fib_FileName, len);
      }
   }

   /* We have the path part in the buffer and the name part in the tail */
   /* All that is left is to concatenate them together correctly        */
   {
      int len;

      /* Successful, the buf holds the path for the directory.  We will need   */
      /* to add a / to the end if it doesn't end in a colon                    */
      len = strlen(buf+pos);
      if ((buf[pos+len-1] != ':') && (buf[pos+len-1] != '/'))
      {
         buf[pos+len++] = '/';
         buf[pos+len] = 0;
      }
      name = malloc(len+strlen(tail)+1);
      if (name != NULL)
      {
         strcpy(name, buf+pos);
         strcpy(name+len, tail);
      }
   }
   return(name);
}

/*
 *  This sets the appropriate AREXX stem variables for the return
 *     FILE:   The name of the file to edit
 *     DIR:    The directory that the file is relative to
 *     LINE:   The line number of the file to go to
 *     COL:    The column number in the file
 *     ERRNO:  The error number
 *     STRING: The error message to be printed out
 */

void SetStem(void *rxmsg, char *stem, EInfo *einfo)
{
   char stembuf[40];
   int len;
   char *p;

   /* DC1: "fails.c" L:1 C:1 W:68 expected semicolon */

   strncat(stembuf, stem, 32);
   stembuf[32] = 0;
   len = strlen(stembuf);

   p = stembuf+len;

   strcpy(p, ".FILE");
   SetRexxVar(rxmsg, stembuf, "FILENAME", 8);
   strcpy(p, ".DIR");
   SetRexxVar(rxmsg, stembuf, "DIRECTORY/DIR", 13);
   strcpy(p, ".LINE");
   SetRexxVar(rxmsg, stembuf, "12", 2);
   strcpy(p, ".COL");
   SetRexxVar(rxmsg, stembuf, "4", 1);
   strcpy(p, ".ERRNO");
   SetRexxVar(rxmsg, stembuf, "42", 2);
   strcpy(p, ".STRING");
   SetRexxVar(rxmsg, stembuf, "Complete Error Message", 22);
}

/*
 *  We handle the following Rexx Commands:
 *     Quit
 *     Next <stem>
 *     First <stem>
 *     Prev <stem>
 *     Clear
 *     TTXSame <file>
 *     Load <error> <dir> <sourcefile> <R0> <R1> <R2> <R3> <R4> <R5> <R6> <R7> <R8> <R9>
 *
 *   Where <stem> Is any valid rexx variable name
 */

long
DoRexxCommand(msg, port, arg0, pres)
void *msg;              /*  RexxMsg structure if we need it     */
struct MsgPort *port;   /*  MsgPort structure if we need it     */
char *arg0;     /*  arg0                                */
char **pres;    /*  where to put our result if rc==0    */
{
    int rc;
    char cmd[9];
    int i;
    char *p, *t;

    printf("MatchCheck %s vs %s\n", arg0, "pass");

    strncpy(cmd, arg0, 8);
    cmd[8] = 0;   // Ensure that the string is properly terminated
    p = strchr(cmd, ' ');
    if (p) *p = 0;

    p = arg0+strlen(cmd);
    while (*p == ' ') p++;

    if (!stricmp(cmd, "QUIT"))
    {
        active = 0;
        rc = 0;
    }
    else if (!stricmp(cmd, "NEXT"))
    {
       SetStem(msg, p, NULL);
       *pres = "NEXT Command Complete";
       rc = 0;
    }
    else if (!stricmp(cmd, "FIRST"))
    {
       SetStem(msg, p, NULL);
       *pres = "FIRST Command Complete";
       rc = 0;
    }
    else if (!stricmp(cmd, "PREV"))
    {
       SetStem(msg, p, NULL);
       *pres = "PREV Command Complete";
       rc = 0;
    }
    else if (!stricmp(cmd, "CLEAR"))
    {
       *pres = "All Errors Cleared";
       rc = 0;
    }
    else if (!stricmp(cmd, "TTXSAME"))
    {
       *pres = "TTXSAME not activated";
       rc = 0;
    }
    else if (!stricmp(cmd, "LOAD"))
    {
        SetStem(msg, "BASE", NULL);
        *pres = "success!";
        rc = 0;
    }
    else
    {
        *pres = "Command Unknown";
        rc = 5;
    }
    return(rc);
}


#if 0
Pass  :  DC1, DAS, DCPP, DLINK
File  :  Char *
Dir   :
Line  :  Short
Column:  Short
Type  :  W, E, F
Number:  Char
String:  Char *

DC1: "fails.c" L:1 C:1 W:68 expected semicolon
DC1: "fails.c" L:2 C:1 W:68 expected semicolon
DC1: "fails.c" L:4 C:1 W:68 expected semicolon
DC1: "fails.c" L:6 C:1 W:68 expected semicolon
DC1: "fails.c" L:7 C:10 W:68 expected semicolon
DC1: "fails.c" L:7 C:1 W:68 expected semicolon
DC1: "fails.c" L:8 C:1 W:68 expected semicolon
DC1: "fails.c" L:10 C:1 W:68 expected semicolon


SetRexxVar(pending->rxmsg,stem,file_name, strlen(file_name));


Configuration Options:
   Edit Command

#
#	Quick-Fix Error Handling.  Pops up your editor pointing to any
#	warnings or errors.  Enable with the -R option to DCC.
#
#		port=  never_happen	;Run REXX script
#		port=  REXX		;Talk directly to REXX
#		port=  your application ;Talk directly to "your application"


cmd=
port=
rexxcmd=
linkcmd=
linkport=
linkrexxcmd=

#
#
#		%e  	Error temporary file name
#		%f  	Fully qualified source file name
#		%d  	Directory containing the source
#		%n  	Filepart of source
#		%c	Current Directory (where DCC was run from)
#		%0-%9	argument from -R0/-R9 DCC option (e.g. -R0 fubar)
#
#	Note: A space is not allowed between the keyword and the =!
#
#cmd=   TTX %e MACRO "DCC:Rexx/ParseError.ttx %f %c %0"
port= DICE_ERROR_PARSER
cmd= Load %e %d %f %0 %1 %2 %3 %4 %5 %6 %7 %8 %9



#include "TTXSame_Rev.h"

extern struct Library *RexxSysBase;
extern struct DosLibrary *DOSBase;

struct SAVER {
   struct SAVER *next;
   char         *fname;
};

char *RexxHostName = NULL;



/***********************************************************************************
 * Procedure: main
 * Synopsis:  rc = main(argc, argv);
 * Purpose:   Main entry point
 ***********************************************************************************/
int ttxmain(int argc, char **argv)
{
   char *res;
   int i;
   char *macro;
   char *project;
   struct SAVER base, *nsaver;

   project = NULL;
   macro   = NULL;

   if (RexxSysBase == NULL)
   {
      say("Unable to open rexxsyslib.library!");
      exit(20);
   }

   CreateDiceRexxPort(NULL, NULL);

   nsaver = &base;
   base.next = NULL;

   if ((argc == 2) && (!strcmp(argv[1], "?"))) usage();

   /* Template for command: */
   /* FILE/M,MACRO/S,PROJECT/S */
   for (i = 1; i < argc; i++)
   {
      if (!stricmp(argv[i], "MACRO"))
      {
         if (++i >= argc) usage();
         if (macro != NULL) usage();
         macro = mustmalloc(strlen(argv[i]) + 16); /* ExecArexxMacro */
         strcpy(macro, "ExecArexxMacro ");
         strcat(macro, argv[i]);
      }
      else if (!stricmp(argv[i], "PROJECT"))
      {
         if (++i >= argc) usage();
         project = argv[i];
      }
      else
      {
         if (!stricmp(argv[i], "FILE"))
         {
            if (++i >= argc) usage();
         }
         nsaver->next = mustmalloc(sizeof(struct SAVER));
         nsaver = nsaver->next;
         nsaver->fname = full_path(argv[i]);
         nsaver->next  = NULL;
      }
   }

   /* We have parsed our parameters, now we need to load each of the files */
   /* specified into the editor.  In the process we will have to check for */
   /* Any files in the editor to ensure that they are not already loaded   */
   while((nsaver = base.next) != NULL)
   {
      /* Send the command off to rexx to be processed */
      /* We will wait here until it is complete       */
      res = dottx(NULL, "GETDOCUMENTS");
      if (res != NULL)
      {
         char *p, *t, *fname, *pname;
         int len;
         /* Now we need to go through and figure out all the files that are there */
         fname = res;
         pname = NULL;
         while(*fname)
         {
            if (*fname != '"')
            {
               say("TURBOTEXT Sync Error");
               return(20);
            }
            fname++;
            t = fname;
            len = strlen(t);
            while ((len > 11) && memcmp(t, "\" TURBOTEXT", 11)) /* Bug with no parm */
            {
               char *s;
               s = strchr(t+1, '"');
               if (s == NULL) break;
               len -= (s-t);
               t = s;
            }
            /* At this point, t should be pointing at the " in the name */
            *t = 0;
            /* Now we want to get the portname for the file */
            t += 2;
            pname = t;
            while(*t && (*t != ' ')) t++;
            if (*t) *t++ = 0;

            p = dottx(pname, "GetFilePath");

            /* If this is the file that we are interested in, drop out of the */
            /* loop and go to work on it in the file                          */
            if ((p != NULL) && !stricmp(nsaver->fname, p))
               break;

            /* Advance to let the next stuff work */
            fname = t;
            while(*fname == ' ') fname++;
            pname = NULL;
         }

         /* If we got a match, the port will tell us who to talk to */
         if (pname == NULL)
         {
            char *ocmd;
            ocmd = mustmalloc(strlen(nsaver->fname) + 15);  /* OPENDOC NAME */
            strcpy(ocmd, "OPENDOC NAME ");
            strcat(ocmd, nsaver->fname);
            pname = dottx(NULL, ocmd);
            free(ocmd);
         }
         else
         {
            char *p;
            p = dottx(pname, "WINDOW2FRONT");
            if (p != NULL) free(p);
            p = dottx(pname, "SCREEN2FRONT");
            if (p != NULL) free(p);
         }
         {
            char *p;
            p = dottx(pname, "ACTIVATEWINDOW");
            if (p != NULL) free(p);
         }

         /* We now have the document open, pname is the port to talk to */
         if (project != NULL)
         {
            char *ocmd;
            /* ExecARexxString call setclip(TTX_TURBOTEXT12,myport.1)   */
            /* 123456789012345678901234567890123           4        5   */
            ocmd = mustmalloc(strlen(project)+strlen(pname)+36);
            strcpy(ocmd, "ExecARexxString call setclip(TTX_");
            strcat(ocmd, pname);
            strcat(ocmd, ",");
            strcat(ocmd, project);
            strcat(ocmd, ")");
            p = dottx(pname, ocmd);
            if (p != NULL) free(p);
            free(ocmd);
         }

         /* Next we want to issue the macro command to make it run */
         if (macro != NULL)
         {
            char *p;
            p = dottx(pname, macro);
            if (p != NULL) free(p);
         }
         free(res);
      }

      /* Take the entry off the list because we have already processed it */
      base.next = nsaver->next;
      free(nsaver->fname);
      free(nsaver);
   }
   return(0);
}
#endif


/***********************************************************************************
 * Procedure: main
 * Synopsis:  rc = main(argc, argv);
 * Purpose:   Main entry point
 ***********************************************************************************/
int main(ac, av)
char *av[];
{

   /*
    *  DICE automatically opens rexxsyslib.library for us as long
    *  as we reference the base variable (via extern) and not
    *  declare it.  lib/rexx.h does this for us.
    *
    *  However, unlike other autoinits, if DICE is unable to open
    *  the library it does not abort the program, hence the following.
    */
   if (RexxSysBase == NULL)
   {
      say("Unable to open rexxsyslib.library !");
      exit(20);
   }

   short r = CreateGlobalDiceRexxPort(NULL, PORTNAME);
   if (r < 0)
   {
      say("AREXX Port " PORTNAME " exists.  DError is already running\n");
      exit(20);
   }

   say("Creating port " PORTNAME);

   active = 1;

   /*
    *  Our main loop executes received commands
    */

   while (active)
   {
      long mask = Wait(SIGBREAKF_CTRL_C | (1 << RexxSigBit));

      if (mask & SIGBREAKF_CTRL_C)
         break;

      if (mask & (1 << RexxSigBit))
          ProcessRexxCommands(NULL);
   }
   say("DError terminating\n");
   return(0);
}
