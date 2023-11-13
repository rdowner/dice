//***********************************************************************
//*  Copyright (c) 1993, 1994 Obvious Implementation Corp.              *
//*                           All Rights Reserved.                      *
//*                     207 Livingstone Drive,                          *
//*                     Cary N.C. 27513 - USA                           *
//***********************************************************************

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
#include <lib/misc.h>

#include "DError_rev.h"

#define PORTNAME "DICE_ERROR_PARSER"  // This is what everyone else calls us by

extern __stkargs LONG SetRexxVar(struct RexxMsg *,char *,char *,ULONG);

struct ErrorInfo {
    struct ErrorInfo *next;      // Next error line in the list
    struct ErrorInfo *prev;      // Previous line in the list
    char             line[1];    // Text of the line.  Note the [1] for the NULL
};

struct FileInfo {
    struct FileInfo *next;       // Next file entry in the list
    struct FileInfo *prev;       // Previous file entry in the list
    char            *source;     // Source file that was compiled
    char            *dir;        // Directory the file was compiled in
    char            *args;       // Rexx arguments used for the compile
    struct ErrorInfo base;       // Root node for all lines
    struct ErrorInfo *cur;       // Current active error line
};

struct FileInfo files;           // Root node for all files
struct FileInfo *curfile;        // Current active error file

char *RexxHostName = NULL;       // We will create the host ourseleves

#define MAX_FILENAME 1024
char buf[MAX_FILENAME+1];
char ebuf[512];             // AREXX return string holding buffer

short run_arexx_server;

//***************************************************************************
//* Procedure: AddLine                                                      *
//* Synopsis:  AddLine(FileInfo *, char *)                                  *
//* Purpose:   Adds the line to the current file information.  If there is  *
//*            not enough memory, it will simply ignore the line silently.  *
//***************************************************************************
void AddLine(struct FileInfo *fi, char *line)
{
   struct ErrorInfo *ei;

   ei = malloc(sizeof(struct ErrorInfo) + strlen(line));
   if (ei)
   {
      ei->prev = fi->base.prev;
      ei->next = fi->base.prev->next;
      strcpy(ei->line, line);
      fi->base.prev->next = ei;
      fi->base.prev = ei;
   }
}

//******************************************************************************
//* Procedure: NewFile
//* Synopsis:  FileInfo = NewFile(char *source, char *dir, char *args)
//* Purpose:   Creates a new file info.  If it is unable to, it returns NULL
//******************************************************************************
struct FileInfo *NewFile(char *source, char *dir, char *args)
{
   struct FileInfo *fi;

   fi = malloc(sizeof(struct FileInfo));
   if (fi)
   {
      fi->source = strdup(source);
      fi->dir = strdup(dir);
      fi->args = strdup(args);
      fi->cur  = NULL;
      if (fi->source != NULL &&
          fi->dir    != NULL &&
          fi->args   != NULL)
      {
          fi->prev = files.prev;
          fi->next = files.prev->next;
          files.prev->next = fi;
          files.prev = fi;

          fi->base.prev = fi->base.next = &fi->base;
      }
      else
      {
         if (fi->source) free(fi->source);
         if (fi->dir)    free(fi->dir);
         if (fi->args)   free(fi->args);
         free(fi);
         fi = NULL;
      }
   }
   return(fi);
}

//************************************************************************
//* Procedure: FreeFile
//* Synopsis:  FreeFile(struct FileInfo *)
//* Purpose:   Removes a FileInfo structure from the list
//************************************************************************
void FreeFile(struct FileInfo *fi)
{
   struct ErrorInfo *ei;

   //
   // Handle any attempts to free the base FI
   //
   if (fi == &files) return;

   // Unlink us from the chain of file handles
   fi->prev->next = fi->next;
   fi->next->prev = fi->prev;

   //
   // Update any system pointers which might look at what we are going to free
   //
   if (curfile == fi)
   {
      curfile = NULL;  // no more anyway
   }

   //
   // Free all the lines
   //
   fi->base.prev->next = NULL;  // mark our stopping point
   for(ei = fi->base.next; ei != NULL;)
   {
      struct ErrorInfo *sei;

      sei = ei;
      ei = ei->next;
      free(sei);
   }

   //
   // Finally, get rid of the file information
   //
   free(fi->source);
   free(fi->dir);
   free(fi->args);
   free(fi);
}

//***********************************************************************
//* Procedure: say
//* Synopsis:  (void)say(msg);
//* Purpose:   Displays the given message on the console
//***********************************************************************
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

//***********************************************************************
//* Procedure: usage
//* Synopsis:  (void)usage();
//* Purpose:   Displays the command line usage message - does not return
//***********************************************************************
void usage(void)
{
   say("FILE/M,MACRO/K,PROJECT/K,REXXSTARTUP/S" VERSTAG);
   exit(20);
}

//***********************************************************************
//* Procedure: dottx
//* Synopsis:  result = dottx(port, cmd);
//* Purpose:   Sends a command to TurboText
//***********************************************************************
char *dottx(char *port, char *cmd)
{
   char *res;
   long ec;

   if (port == NULL) port = "TURBOTEXT";
   PlaceRexxCommandDirect(NULL, port, cmd, &res, &ec);

   return(res);
}

//***********************************************************************
//* Procedure: full_path
//* Synopsis:  path = full_path(name)
//* Purpose:   Constructs a fully expanded filename
//*            Note, we can not assume that the file exists, so it will
//*            not be possible to actually lock it.  We can assume that
//*            the directory it is part of does exist.  Note that it can
//*            return NULL if there is no memory available.
//***********************************************************************
char *full_path(char *name)
{
   BPTR lock;
   __aligned struct FileInfoBlock fib;
   char *tail, *p;
   int pos;

   //
   // Step 1 - split out any directory information from the actual name
   //
   p = strrchr(name, '/');
   if (p == NULL) p = strrchr(name, ':');
   if (p != NULL)
   {
      //
      // There was some directory information involved
      //
      char c;
      tail = strdup(p+1);
      c = p[1];
      p[1] = 0;
      lock = Lock(name, SHARED_LOCK);
      p[1] = c;
   }
   else
   {
      //
      // No directory information involved, just the name relative to the
      // current directory
      //
      lock = Lock("", SHARED_LOCK);
      tail = strdup(name);
   }

   //
   // Step 2 - we have the lock on the directory and the tail part of the name
   // We want to construct a fully qualified path for the directory.
   // If for some reason the lock on the directory returned 0, we want to just
   // return the name they gave us to begin with.
   //
   if (lock == 0)
   {
      free(tail);
      return(strdup(name));
   }

   //
   // Step 3 - Fully qualify the directory portion into the buffer
   //
   if (DOSBase->dl_lib.lib_Version >= 36)
   {
      if (!NameFromLock(lock, buf, MAX_FILENAME))
      {
         //
         // Either the name is too long or there was something else wrong with
         // the file name, just return what they gave us as a start
         //
         UnLock(lock);
         free(tail);
         return(strdup(name));
      }
      UnLock(lock);
      pos = 0;
   }
   else
   {
      // Running under 1.3, we have to do this the old fashion way

      //
      // Just so we don't have to do any inserts/extra copies, we will work
      // from the end of the buffer and insert as we go
      //
      pos = MAX_FILENAME-1;  // Leave room for a '/' on the end sometimes
      buf[--pos] = 0;
      while(lock != 0)
      {
         BPTR parent;
         int len;

         //
         // Examine the lock to get the name for it
         //
         Examine(lock, &fib);

         //
         // Find the parent of this directory
         //
         parent = ParentDir(lock);
         UnLock(lock);
         lock = parent;

         len = strlen(fib.fib_FileName);
         pos -= 1;

         if (len > pos)
         {
            //
            // oops, not enough room, just return the name they gave us
            //
            UnLock(lock);
            free(tail);
            return(strdup(name));
         }
         buf[pos] = lock ? ':' : '/';
         pos -= len;
         memcpy(buf+pos, fib.fib_FileName, len);
      }
   }

   //
   // We have the path part in the buffer and the name part in the tail
   // All that is left is to concatenate them together correctly
   //
   {
      int len;

      //
      // Successful, the buf holds the path for the directory.  We will need
      // to add a / to the end if it doesn't end in a colon
      //
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


//***********************************************************************
//* Procedure: SetStem
//* Synopsis:  rc = SetStem(msg, stemstr)
//* Purpose:   Set the appropriate AREXX stem variables based on the
//*            current error message
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//*
//*  This sets the appropriate AREXX stem variables for the return
//*     FILE:   The name of the file to edit
//*     DIR:    The directory that the file is relative to
//*     LINE:   The line number of the file to go to
//*     ARGS:   The REXX arguments associated with the compile command
//*     COL:    The column number in the file
//*     ERRNO:  The error number
//*     STRING: The error message to be printed out
//*     TEXT:   The complete text of the original line
//*     FPATH:  The full pathname of the file
//*
//***********************************************************************
int SetStem(void *rxmsg, char *stem)
{
   char stembuf[40];
   char *p;
   char *str, *t;

   strncpy(stembuf, stem, 32);
   stembuf[32] = 0;
   p = stembuf+strlen(stembuf);

   if (curfile == NULL)      return(5);
   if (curfile->cur == NULL) return(5);

   strcpy(p, ".DIR");
   SetRexxVar(rxmsg, stembuf, curfile->dir, strlen(curfile->dir));

   strcpy(p, ".ARGS");
   SetRexxVar(rxmsg, stembuf, curfile->args, strlen(curfile->args));

   str = curfile->cur->line;   // DC1: "fails.c" L:1 C:1 W:68 expected semicolon

   strcpy(p, ".TEXT");
   SetRexxVar(rxmsg, stembuf, str, strlen(str));

   str = strchr(str, '\"');      // Locate the initial quote for the filename
   if (str == NULL) return(5);   // Skip out if we don't find it.
   str++;                        // Skip over the initial quote
   t = strchr(str, '\"');        // Now look for the terminting quote
   if (t == NULL) return(5);     // and skip out if it is not there
   strcpy(p, ".FILE");
   SetRexxVar(rxmsg, stembuf, str, t-str);

   //
   // Get the full file name also.  For this we need to set the current
   // directory and then use the fullpath routine
   //
   chdir(curfile->dir);
   {
      char c;
      char *path;

      c = *t;                    // Remember the char that terminated the name
      *t = '\0';                 // and replace it with a null
      path = full_path(str);     // so that we can get the full path of the file
      *t = c;                    // Put the original character back
      strcpy(p, ".FPATH");
      if (path)
      {
         SetRexxVar(rxmsg, stembuf, path, strlen(path));
         free(path);
      }
      else
      {
         //
         // We couldn't get a full path, just punt and use what we started with
         //
         SetRexxVar(rxmsg, stembuf, str, t-str);
      }
   }

   str = t + 1;                  // This should either be a NULL or the space

   //
   // Parse out the L:<number> to get the line number
   //
   while (*str == ' ') str++;
   t = str;
   if (str[0] == 'L' && str[1] == ':')
   {
      str += 2;                  // Skip over the 'L:'
      t = str;
      while(*t >= '0' && *t <= '9') t++;
   }
   strcpy(p, ".LINE");
   SetRexxVar(rxmsg, stembuf, str, t-str);
   str = t;

   //
   // Parse out the C:<number> to get the column number
   //
   while (*str == ' ') str++;
   t = str;
   if (str[0] == 'C' && str[1] == ':')
   {
      str += 2;                  // Skip over the 'C:'
      t = str;
      while(*t >= '0' && *t <= '9') t++;
   }
   strcpy(p, ".COL");
   SetRexxVar(rxmsg, stembuf, str, t-str);
   str = t;

   //
   // Save away the error message to be displayed
   //
   while (*str == ' ') str++;
   strcpy(p, ".STRING");
   SetRexxVar(rxmsg, stembuf, str, strlen(str));

   //
   // Lastly look for the remaining colon to get the error number
   //
   while(*str && *str != ':') str++;

   t = str;
   if (str[0] == ':')
   {
      str++;
      t = str;
      while(*t >= '0' && *t <= '9') t++;

   }
   strcpy(p, ".ERRNO");
   SetRexxVar(rxmsg, stembuf, str, t-str);

   return(0);
}

//***********************************************************************
//* Procedure: do_next
//* Synopsis:  do_next()
//* Purpose:   Advance the current pointers to the next entry
//*            If nothing is active, we wrap around to the beginning again
//***********************************************************************
void do_next(void)
{

   if (curfile != NULL &&                    // Do we have any file active?
       curfile->cur->next != &curfile->base) // With at least one more line?
   {
       //
       // Normal case, Everything is fine, just advance to the next line
       //
       curfile->cur = curfile->cur->next;
   }
   else
   {
      //
      // If there is no line active, go to the first one
      // This can happen when they go past the end of the error messages
      // or when they start out for the first time
      //
      if (curfile == NULL)
         curfile = &files;

      //
      // Now advance to the next file
      //
      curfile = curfile->next;

      if (curfile == &files)   // But make sure we actually have one
      {
         curfile = NULL;       // oops, no files at all, let them know
      }
      else
      {
         //
         // Mark the first line as active.  Note that we can assume
         // there there is always at least one line in a given file
         // because of the way that do_load() works.
         //
         curfile->cur = curfile->base.next;
      }
   }
}

//***********************************************************************
//* Procedure: do_prev
//* Synopsis:  do_prev()
//* Purpose:   Advance the current pointers to the previous entry.
//*            If we are at the beginning, don't do anything.
//***********************************************************************
void do_prev(void)
{
   //
   // If there is no line active, go to the first one
   // This can happen when they go past the end of the error messages
   // or when they start out for the first time
   //
   if (curfile)        // Do we have any line active?
   {
      //
      // Make sure we actually have a prev in the current source file
      //
      if (curfile->cur->prev == &curfile->base)
      {
         //
         // No, we need to advance to the next file
         //
         curfile = curfile->prev;
         if (curfile == &files)  // OOps, are we out of files?
         {
            curfile = NULL;
         }
         else
         {
            curfile->cur = curfile->base.prev;
         }
      }
      else
      {
         //
         // Everything is fine, just backup to the prev line
         //
         curfile->cur = curfile->cur->prev;
      }
   }
}

//************************************************************************
//* Procedure: do_load
//* Synopsis:  lines = do_load(file, dir, source, args);
//* Purpose:   Load a file into the error parsing.  A string constaining
//*            the lines whcih corrspond to the source is returned.  This
//*            string is not to be freed.
//***********************************************************************
char *do_load(char *efile, char *dir, char *source, char *args)
{
   char *result_append;
   FILE *fh;
   struct FileInfo *fi;
   int result_space;

   fh = fopen(efile, "r");
   if (fh == NULL)
      return(NULL);

   result_append = ebuf;
   result_space = sizeof(ebuf)-2;

   // See if the file is already in our list of things that are active
   for (fi = files.next; fi != &files; fi = fi->next)
   {
      if (!stricmp(fi->source, source) &&
          !stricmp(fi->dir,    dir))
      {
          // We found the existing file.  Just get rid of it
          FreeFile(fi);
          break;
      }
   }

   fi = NULL;
   while(fgets(buf, MAX_FILENAME, fh) != NULL)
   {
      int len;
      char *p, *line_text, *file_text;

      len = strlen(buf);
      if (len && (buf[len-1] == '\n'))
         buf[len-1] = '\0';

      //
      // We need to do some sanity checking here on the line to make sure
      // that it is really an error.  If not, we should just skip the line
      // and go to the next one.  We only have to do a walk through on the
      // line to see that it conforms to some basic sanity.  We will assume
      // That pathological cases don't have to be parsed here because the
      // SetStem routine does more rigerous parsing.
      //

      // DC1: "fails.c" L:1 C:1 W:68 expected semicolon
      if ((file_text = strchr(buf, '"'))  && // Find the first quote
          (p = strchr(file_text+1, '"'))  && // And the closing quote
          (line_text = strchr(p,   'L'))  && // Note, we are remembering the L: place
          (line_text[1] == ':')           && // It should be followed by a :
          (p = strchr(line_text+2, ':'))  && // Look for the : in C:
          (p = strchr(p+1, ':')) )           // Look for the : after the error type
      {
         if (fi == NULL)
         {
            // Add the source file to the list of files
            fi = NewFile(source, dir, args);

            // Of course if we don't have memory we want to leave gracefully
            if (fi == NULL) return(NULL);

            curfile = fi;  // Set it up so we start at the current file
         }

         //
         // See if we also need to squirrel away the error line number
         // for the result string.  We need to do this in the case where
         // the source file of this message is the one that we are loading for
         //
         if (!memcmp(file_text+1, source, strlen(source)))
         {
            // Yes, it appears to be what we are looking for.
            line_text += 2;                 // Skip over the L:
            while (result_space &&          // Make sure there is room in the buffer
                   (*line_text >= '0') &&
                   (*line_text <= '9'))     // and copy the numbers which follow it
            {
               *result_append++ = *line_text++;
               result_space--;
            }
            if (result_space)
            {
               *result_append++ = ' ';
               result_space--;
            }
         }

         AddLine(fi, buf);
      }
   }

   *result_append = 0;    // Null terminate the result
   if (curfile != NULL)
   {
      //
      // Mark the first line as active.  Note that we can assume
      // there there is always at least one line in a given file
      // because of the way that do_load() works.
      //
      curfile->cur = curfile->base.next;
   }
   fclose(fh);
   return(ebuf);
}


//***********************************************************************
//* Procedure: do_ttx
//* Synopsis:  do_ttx(file, macro, project)
//* Purpose:   Invoke turbotext on a given file
//*
//***********************************************************************
char *do_ttx(char *file, char *project, char *macro)
{
   char *document_list, *portname;

   portname = NULL;

   //
   // Make sure that TurboText is actually running
   //
   {
#define FIND_ITERATIONS 5
      short Port_Find;

      for(Port_Find = FIND_ITERATIONS; Port_Find > 0; Port_Find--)
      {
          if (FindPort("TURBOTEXT") != NULL) break;
          if (Port_Find == FIND_ITERATIONS)  // First time through
             Execute("Turbotext:TTX BACKGROUND NOWINDOW", 0L, 0L);
          Delay(50);
      }
      //
      // Make sure that we were acutally able to start turbotext before
      // going forward
      //
      if (Port_Find == 0)
         return(NULL);
   }

   //
   // Send the command off to rexx to be processed
   // We will wait here until it is complete
   document_list = dottx(NULL, "GETDOCUMENTS");
   if (document_list != NULL)
   {
      char *fname;

      //
      // Now we need to go through and figure out all the files that are there
      // Turbotext will return us a string in the form
      //   "file1" TURBOTEXT1 "file2" TURBOTEXT2
      // Unfortunately there are some pathological cases (such as having filenames
      // with quotes in them and files having the string TURBOTEXT in them which
      // make it hard to parse 100% accurately.
      //
      for(fname = document_list; *fname;)
      {
         char *file_path, *t;
         int length;

         //
         // We expect the string to be well formed.  Fall out on any
         // pathological cases.
         //
         if (*fname != '"')
         {
            say("TURBOTEXT Sync Error");
            free(document_list);
            return("");
         }
         fname++;
         t = fname;
         length = strlen(t);
         while ((length > 11) && memcmp(t, "\" TURBOTEXT", 11))
         {
            char *quote_pos;

            //
            // We need to find the Quote which is immediately before
            // the TURBOTEXT string
            //
            quote_pos = strchr(t+1, '"');
            if (quote_pos == NULL)
               break;
            length -= (quote_pos-t);
            t = quote_pos;
         }

         //
         // At this point, t should be pointing at the " in the name
         //
         *t = 0;

         //
         // Now we want to get the portname for the file
         //
         t += 2;                 // Skip the '" '
         portname = t;
         while(*t && (*t != ' ')) t++;
         if (*t) *t++ = 0;       // Null terminate it if necessary

         file_path = dottx(portname, "GetFilePath");

         //
         // If this is the file that we are interested in, drop out of the
         // loop and go to work on it in the file
         //
         if ((file_path != NULL) && !stricmp(file, file_path))
         {
            free(file_path);
            portname = strdup(portname);
            break;
         }

         if (file_path != NULL) free(file_path);

         //
         // Advance to let the next stuff work
         //
         fname = t;
         while(*fname == ' ') fname++;
         portname = NULL;
      }
      free(document_list);
   }


   //
   // If we got a match, the port will tell us who to talk to
   //
   if (portname == NULL)
   {
      //
      // No match here, we need to open up the file
      //
      char *open_cmd;

      open_cmd = malloc(strlen(file) + 15);  // strlen("OPENDOC NAME ")
      if (open_cmd)
      {
         strcpy(open_cmd, "OPENDOC NAME ");
         strcat(open_cmd, file);
         portname = dottx(NULL, open_cmd);
         free(open_cmd);
      }
   }
   else
   {
      //
      // The file is already open, we just need to bring it forward
      //
      char *pr;
      pr = dottx(portname, "WINDOW2FRONT");
      if (pr != NULL) free(pr);
      pr = dottx(portname, "SCREEN2FRONT");
      if (pr != NULL) free(pr);
   }

   if (portname != NULL)
   {
      char *p;

      p = dottx(portname, "ACTIVATEWINDOW");
      if (p != NULL) free(p);

      //
      // We now have the document open, portname is the port to talk to
      //
      if (project != NULL)
      {
         char *call_cmd;
         //
         // ExecARexxString call setclip(TTX_TURBOTEXT12,myport.1)
         // 000000000111111111122222222223333           3        3
         // 123456789012345678901234567890123           4        5
         //
         call_cmd = malloc(strlen(project)+strlen(portname)+36);
         if (call_cmd)
         {
            strcpy(call_cmd, "ExecARexxString call setclip(TTX_");
            strcat(call_cmd, portname);
            strcat(call_cmd, ",");
            strcat(call_cmd, project);
            strcat(call_cmd, ")");
            p = dottx(portname, call_cmd);
            if (p != NULL) free(p);
            free(call_cmd);
         }
      }

      //
      // Next we want to issue the macro command to make it run
      //
      if (macro != NULL)
      {
         char *p;
         p = dottx(portname, macro);
         if (p != NULL) free(p);
      }
   }

   //
   // Since this will be returned to rexx, we need to ensure that this is
   // somewhat permanent storage.  Unfortunately, we don't have a way to
   // free the memory easily (we lose our context once the message has been
   // sent back to arexx)
   //
   if (portname)
   {
      strcpy(ebuf, portname);
      free(portname);
      portname = ebuf;
   }

   return(portname);
}

//***********************************************************************
//* Procedure: DoRexxCommand
//* Synopsis:  DoRexxCommand(msg, port, arg, &result)
//* Purpose:   Handle a incomming AREXX command
//*
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//*
//*  We handle the following Rexx Commands:
//*     QUIT
//*     NEXT    <stem>
//*     FIRST   <stem>
//*     PREV    <stem>
//*     CURRENT <stem>
//*     CLEAR
//*     TTXSAME "<file>" "<project>" "<macro>"
//*     LOAD    "<error>" "<dir>" "<sourcefile>" <args>
//*
//*   Where <stem> Is any valid rexx variable name
//*
//***********************************************************************


long
DoRexxCommand(msg, port, arg0, pres)
void *msg;              //  RexxMsg structure if we need it
struct MsgPort *port;   //  MsgPort structure if we need it
char *arg0;             //  arg0
char **pres;            //  where to put our result if rc==0
{
    char cmd[9];
    char *p;

    strncpy(cmd, arg0, 8);
    cmd[8] = 0;         // Ensure that the string is properly terminated
    p = strchr(cmd, ' ');
    if (p) *p = 0;

    p = arg0+strlen(cmd);
    while (*p == ' ') p++;

    //*******************************************************
    //*  QUIT - Terminate the error parser                  *
    //*******************************************************
    if (!stricmp(cmd, "QUIT"))
    {
        run_arexx_server = 0;
        return(0);
    }

    //*******************************************************
    //*  CLEAR - Clear out all errors stored                *
    //*******************************************************
    if (!stricmp(cmd, "CLEAR"))
    {
       while(files.next != &files)
          FreeFile(files.next);
       return(0);
    }

    //*******************************************************
    //*  TTXSAME - Invoke TTX on a file                     *
    //*******************************************************
    if (!stricmp(cmd, "TTXSAME"))
    {
       int ac;
       char *hold;
       char *av[3];
       int rc = 0;

       av[0] = av[1] = av[2] = 0;
       hold = strdup(p);

       if (hold == NULL)
       {
          return(10);    // No memory
       }

       ac = _parseargs1(hold, strlen(hold));
       if (ac)
       {
          if (ac > 3)
             ac = 3;
          _parseargs2(hold, av, ac);
          av[0] = full_path(av[0]);
          *pres = do_ttx(av[0], av[1], av[2]);
          free(av[0]);
          if (*pres == NULL)
             rc = 5;
       }
       free(hold);
       return(rc);
    }

    //*******************************************************
    //*  LOAD - Load errors into the parser                 *
    //*******************************************************
    if (!stricmp(cmd, "LOAD"))
    {
       int ac;
       char *hold;
#define LOAD_PARMS 4
       char *av[LOAD_PARMS];
       int rc = 0;

       av[0] = av[1] = av[2] = av[3] = 0;
       hold = strdup(p);

       if (hold == NULL)
       {
          return(10);    // No memory
       }

       ac = _parseargs1(hold, strlen(hold));
       if (ac)
       {
          if (ac > LOAD_PARMS)
             ac = LOAD_PARMS;
          _parseargs2(hold, av, ac);
          *pres = do_load(av[0], av[1], av[2], av[3]);
          if (*pres == NULL)
             rc = 5;
       }
       free(hold);
       return(rc);
    }

    //*******************************************************
    //*  NEXT - Advance to the next error                   *
    //*******************************************************
    if (!stricmp(cmd, "NEXT"))
    {
       do_next();
    }
    //*******************************************************
    //*  FIRST - Go to the first error                      *
    //*******************************************************
    else if (!stricmp(cmd, "FIRST"))
    {
       curfile = NULL;
       do_next();
    }
    //*******************************************************
    //*  PREV - Move to the previous error                  *
    //*******************************************************
    else if (!stricmp(cmd, "PREV"))
    {
       do_prev();
    }
    //*******************************************************
    //*  CURRENT - Return information on the current error  *
    //*******************************************************
    else if (stricmp(cmd, "CURRENT"))
    {
        // Unrecognized command, let them know about it
        *pres = "Command Unknown";
        return(5);
    }
    return(SetStem(msg, p));
}


//***********************************************************************
//* Procedure: main
//* Synopsis:  rc = main(argc, argv);
//* Purpose:   Main entry point
//***********************************************************************
int main(int argc, char **argv)
{
   struct SAVER {
      struct SAVER *next;
      char         *fname;
   };

   int i;
   char *macro;
   char *project;
   struct SAVER base, *nsaver;

   run_arexx_server  = 0;
   files.prev = &files;
   files.next = &files;
   curfile = NULL;

   macro   = NULL;
   project = NULL;
   nsaver = &base;
   base.next = NULL;

   //
   // DICE automatically opens rexxsyslib.library for us as long
   // as we reference the base variable (via extern) and not
   // declare it.  lib/rexx.h does this for us.
   //
   // However, unlike other autoinits, if DICE is unable to open
   // the library it does not abort the program, hence the following.
   //
   if (RexxSysBase == NULL)
   {
      say("Unable to open rexxsyslib.library !");
      exit(20);
   }

   if ((argc == 2) && (!strcmp(argv[1], "?"))) usage();

   // Template for command:
   //   FILE/M,MACRO/K,PROJECT/K,REXXSTARTUP/S" VERSTAG);
   for (i = 1; i < argc; i++)
   {
      if (!stricmp(argv[i], "MACRO"))
      {
         if (++i >= argc) usage();
         if (macro != NULL) usage();
         macro = malloc(strlen(argv[i]) + 16); // ExecArexxMacro
         if (macro)
         {
            strcpy(macro, "ExecArexxMacro ");
            strcat(macro, argv[i]);
         }
      }
      else if (!stricmp(argv[i], "PROJECT"))
      {
         if (++i >= argc) usage();
         project = argv[i];
      }
      else if (!stricmp(argv[i], "REXXSTARTUP"))
      {
         run_arexx_server = 1;
      }
      else
      {
         if (!stricmp(argv[i], "FILE"))
         {
            if (++i >= argc) usage();
         }
         nsaver->next = malloc(sizeof(struct SAVER));
         if (nsaver->next)
         {
            nsaver = nsaver->next;
            nsaver->fname = full_path(argv[i]);
            nsaver->next  = NULL;
         }
      }
   }

   if (run_arexx_server)
   {
      short r = CreateGlobalDiceRexxPort(NULL, PORTNAME);
      if (r < 0)
      {
         say("AREXX Port " PORTNAME " exists.  DError is already running\n");
         exit(20);
      }
   }
   else
   {
      CreateDiceRexxPort(NULL, NULL);
   }

   // We have parsed our parameters, now we need to load each of the files
   // specified into the editor.  In the process we will have to check for
   // Any files in the editor to ensure that they are not already loaded
   while((nsaver = base.next) != NULL)
   {
      char *portname;

      portname = do_ttx(nsaver->fname, project, macro);
      if (portname == NULL)
         say("Unable to edit file");

      // Take the entry off the list because we have already processed it
      base.next = nsaver->next;
      free(nsaver->fname);
      free(nsaver);
   }

   if (run_arexx_server)
   {
      //
      // Our main loop executes received commands
      //
      while (run_arexx_server)
      {
         long mask = Wait(SIGBREAKF_CTRL_C | (1 << RexxSigBit));

         if (mask & SIGBREAKF_CTRL_C)
            break;

         if (mask & (1 << RexxSigBit))
             ProcessRexxCommands(NULL);
      }
      say("*** DError terminating\n");
   }
   return(0);
}
