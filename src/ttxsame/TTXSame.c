/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/***************************************************************************/
/*  Copyright (c) 1992 Obvious Implementation Corp. All Rights Reserved.   */
/*                     207 Livingstone Drive,                              */
/*                     Cary N.C. 27513 - USA                               */
/***************************************************************************/
#include <stdio.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>
#include <strings.h>
#include <stdlib.h>
#include <lib/rexx.h>
#include "TTXSame_Rev.h"

extern struct Library *RexxSysBase;
extern struct DosLibrary *DOSBase;

struct SAVER {
   struct SAVER *next;
   char         *fname;
};

char *RexxHostName = NULL;

#define MAX_FILENAME 1024
char buf[MAX_FILENAME+1];

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
 * Procedure: main
 * Synopsis:  rc = main(argc, argv);
 * Purpose:   Main entry point
 ***********************************************************************************/
int main(int argc, char **argv)
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

/***********************************************************************************
 * Procedure: DoRexxCommand
 * Synopsis:  rc = DoRexxCommand(msg, port, arg, pres)
 * Purpose:   Handling executing a rexx command
 ***********************************************************************************/
long DoRexxCommand(void *msg,              /* RexxMsg structure if we need it      */
                   struct MsgPort *port,   /* MsgPort structure if we need it      */
                   char *arg0,             /* arg0                                 */
                   char **pres             /* where to put our result if rc==0     */
                  )
{
   return(0);
}
