#include "vmake.h"

Prototype int get_filename(char *str, char *buf, int savemode);
Prototype void commit_filename(char *buf);
Prototype void read_script(void);
Prototype int read_file(void);
Prototype int write_file(void);
Prototype void expand_filename(char *name, char *fullname);
Prototype void go_dir(BPTR dirlock);
Prototype BPTR xlockdir(char *path);

#define LINE_LEN  78
#define MIN_LINE  10

static BPTR XSaveLock;
static BOOL XSaveLockValid;

/***********************************************************************************
 * Procedure: restorecurdir
 * Synopsis:  (void)restorecurdir();  - Called at exit time if necessary
 * Purpose:   Restores the original directory from where the program started
 ***********************************************************************************/
static void restorecurdir()
{
   if (XSaveLockValid)
      UnLock(CurrentDir(XSaveLock));
   XSaveLockValid = 0;
}

/***********************************************************************************
 * Procedure: go_dir
 * Synopsis:  (void)go_dir(dirlock);
 * Purpose:   Set the current directory to one specified by the given lock
 ***********************************************************************************/
void go_dir(BPTR dirlock)
{
   BPTR lock;

   if (dirlock)
   {
      if ( (lock = DupLock(dirlock)) != NULL)
      {
         if (!XSaveLockValid)
         {
            XSaveLock = CurrentDir(lock);
            XSaveLockValid = 1;
            atexit(restorecurdir);
         }
         else
         {
            UnLock(CurrentDir(lock));
         }
      }
   }
}

BPTR xlockdir(char *path)
{
   BPTR lock;

   lock = Lock(path, SHARED_LOCK);

   /* We should check to make sure that the lock is valid */
   return(lock);
}

/***********************************************************************************
 * Procedure: get_filename
 * Synopsis:  type = get_filename(str)
 * Purpose:   Parse out a file descriptor and return a type for that name
 *            This routine does not modify any permanent global data
 ***********************************************************************************/
int get_filename(char *str, char *buf, int savemode)
{
   char *p;
   int len;

   go_dir(global.homedir);

   if (*str)
   {
      /* They actually specified a name.  All we need to do is replace the */
      /* current one.  If they specified a '?', then we need to prompt the */
      /* user with the file requester.  We try to use ASL first and then   */
      /* fall back to ARP.                                                 */
      if (*str == '?')
      {
         int n;

         if (global.inrexx)
         /* don't put up requesters if called from rexx - just fail */
         {
            global.rexxrc = TEXT_BADPROJ;  /* sort of fits the bill... */
            return 0;
         }

         if (global.freq == NULL) return(0);

         if (AslBase != NULL)
         {
            struct TagItem taglist[10];

            n = 0;
            taglist[n  ].ti_Tag  = ASL_Pattern;
            taglist[n++].ti_Data = (ULONG)global.text[CONFIG_PATTERN];

            taglist[n  ].ti_Tag  = ASL_File;
            taglist[n++].ti_Data = (ULONG)"";

            taglist[n  ].ti_Tag  = ASL_Dir;
            taglist[n++].ti_Data = (ULONG)"";

            taglist[n  ].ti_Tag  = ASL_Window;
            taglist[n++].ti_Data = (ULONG)global.window;

#ifdef ASLFR_DoSaveMode
            taglist[n  ].ti_Tag  = ASLFR_DoSaveMode;
            taglist[n++].ti_Data = savemode;
#endif

#ifdef ASLFR_DoPatterns
            taglist[n  ].ti_Tag  = ASLFR_DoPatterns;
            taglist[n++].ti_Data = TRUE;
#endif

#ifdef ASLFR_RejectIcons
            taglist[n  ].ti_Tag  = ASLFR_RejectIcons;
            taglist[n++].ti_Data = TRUE;
#endif

#ifdef ASLFR_SleepWindow
            taglist[n  ].ti_Tag  = ASLFR_SleepWindow;
            taglist[n++].ti_Data = TRUE;
#endif

            taglist[n  ].ti_Tag  = TAG_DONE;
            taglist[n++].ti_Data = 0;

            if (!AslRequest( (APTR)global.freq, taglist))
               return(0);

         }
         else
         {
            /* We must have arp.library (otherwise we wouldn't have a file     */
            /* Requester structure.)                                           */
            if (!ArpFileRequest( global.freq ))
               return(0);
         }

         /* Since the ASL requester and the ARP requester have the pointers to */
         /* the directory and the filename in the same place, we can use the   */
         /* same code to parse both of them                                    */
         n = strlen(global.freq->rf_Dir);
         len = strlen(global.freq->rf_File);
         if (len == 0)
            return (0);  /* didn't pick a name */
         if (n > MAX_FILENAME) n = MAX_FILENAME;
         p = buf;
         memcpy(p, global.freq->rf_Dir, n);
         p += n;

         if ((n > 0) && (p[-1] != ':'))
         {
            *p++ = '/';
            n++;
         }
         if ((n + len) > MAX_FILENAME) len = MAX_FILENAME-n;
         strncpy(p, global.freq->rf_File, len);
         p[len] = 0;
      }
      else
      {
         /* Copy over the file name.  Stop when we get to a '?' */
         int len;
         p = strchr(str, ';');
         if (p)
            len = p-str;
         else
            len = strlen(str);
         if (len > MAX_FILENAME) len = MAX_FILENAME;
         memcpy(buf, str, len);
         buf[len] = 0;
      }
   }
   else
   {
      /* default is to use existing global.filename */
      strcpy(buf, global.filename);
   }
   return(1);
}

/***********************************************************************************
 * Procedure: commit_filename
 * Synopsis:  void get_filename(str)
 * Purpose:   Set the global data to use a new file name (from get_filename())
 ***********************************************************************************/
void commit_filename(char *buf)
{
   char *p;

   /* Given a filename, we want to now set a current directory for the         */
   /* Project.  This means that we need to split the name into a directory     */
   /* and file portion.  We should also set the current directory.  Note that  */
   /* It might be the case that the dmakefile has a directory override for the */
   /* sources.  This will work out fine because we will use the full path for  */
   /* the dmakefile.                                                           */
   /* We want to set the symbols DIR and DMAKEFILE                             */
   /* Start by saving the temporary copy of the new filename.                  */
   strcpy(global.filename, buf);
   p = strrchr(global.filename, '/');

   if (p == NULL) p = strrchr(global.filename, ':');
   if (p != NULL)
   {
      char *tp;
      int c;
      BPTR dirlock;

      tp = p;
      /* we need to include the colon in a root directory name, then    */
      /* null terminate so we can get a lock on the project directory   */
      if (*tp == ':') tp += 1;
      c = *tp;
      *tp = 0;
      dirlock = xlockdir(global.filename);
      if (dirlock)
      {
         UnLock(global.homedir);
         global.homedir = dirlock;
      }

      *tp = c;
      Sym_Set(SYM_SCRIPT, p+1, NULL);
   }
   else
   {
      BPTR dirlock;
      dirlock = xlockdir("");
      if (dirlock)
      {
         UnLock(global.homedir);
         global.homedir = dirlock;
      }
      Sym_Set(SYM_SCRIPT, global.filename, NULL);
   }

   /* Now we want to update the title bar */
   if (*global.filename)
   {
      int len;
      strcpy(global.title, global.text[TEXT_PROJECT]);
      len = strlen(global.title);
      strncpy(global.title+len, Sym_Lookup(SYM_SCRIPT), MAX_TITLE-len);
      global.title[MAX_TITLE-1] = 0;
   }
   else
   {
      strcpy(global.title, global.text[TEXT_NOPROJ]);
   }

   SetWindowTitles(global.window, global.title, global.title2);
}


//***********************************************************************
//* Procedure: expand_filename
//* Synopsis:  expand_pathname(name)
//* Purpose:   Constructs a fully expanded filename
//*            Note, we can not assume that the file exists, so it will
//*            not be possible to actually lock it.  We can assume that
//*            the directory it is part of does exist.  It will just
//*            return the name if the expansion fails in any way.
//* Assumptions:  Buffer for fullname is at least large enough to hold
//*            name, and at least MAX_FILENAME+1 bytes.  This routine 
//*            will not return more than MAX_FILENAME bytes.
//***********************************************************************
void expand_filename(char *name, char *fullname)
{
   BPTR lock;
   __aligned struct FileInfoBlock fib;
   char *tail, *p;
   int pos;
   char buf[MAX_FILENAME+1];

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
      tail = p+1;
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
      tail = name;
   }

   //
   // Step 2 - we have the lock on the directory and the tail part of the name
   // We want to construct a fully qualified path for the directory.
   // If for some reason the lock on the directory returned 0, we want to just
   // return the name they gave us to begin with.
   //
   if (lock == 0)
   {
      strcpy(fullname, name);
      return;
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
         strcpy(fullname, name);
         return;
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
            strcpy(fullname, name);
            return;
         }
         buf[pos] = lock ? '/' : ':';
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

      if ((len + strlen(tail)) >= MAX_FILENAME)
      {
         //  Oops, it's to long, just use the name
         strcpy(fullname, name);
      }
      else
      {
         //  Tack the name onto the path
         strcpy(fullname, buf+pos);
         strcpy(fullname+len, tail);
      }
   }
   return;
}


/***********************************************************************************
 * Procedure: read_script
 * Synopsis:  read_script()
 * Purpose:   Read in a config script
 ***********************************************************************************/
void read_script()
{
   char buf[256];
   FILE *fp;
   char *val;
   int len;

   go_dir(global.homedir);

   val = Sym_Lookup("TYPE");
   if (!*val) val = "Normal";

   if ((!strchr(val, '/')) && (!strchr(val, ':')))
   {
      sprintf(buf, "DCC:CONFIG/%s.DMakefile", val);
      val = buf;
   }

   fp = fopen(val, "r");
   if (fp == NULL)
   {
      request(1, TEXT_BADFILE, val, NULL);
      return;
   }

   Sym_Set(SYM_HOLD, "", NULL);
   while((len = fread(buf, 1, 255, fp)) > 0)
   {
      buf[len] = 0;
      Sym_Set(SYM_HOLD, NULL, buf);
   }
   fclose(fp);
}

/***********************************************************************************
 * Procedure: read_file
 * Synopsis:  rc = read_file()
 * Purpose:   Read in a dmakefile
 ***********************************************************************************/
int read_file()
{
   char buf[256];

   FILE *fp;
   int cstate, len;
   char name[64];
   char *fname;

   fname = Sym_Lookup(SYM_SCRIPT);

   go_dir(global.homedir);

   fp = fopen(fname, "r");
   if (fp == NULL)
   {
      request(1, TEXT_BADFILE, fname, NULL);
      return(1);
   }
   cstate = 1;
   while(cstate && (fgets(buf, 256, fp) != NULL))
   {
      int len;
      int extend;
      char *p, *val;

      len = strlen(buf)-1;

      /* Skip any blank lines */
      if (len < 2) continue;

      /* Ignore any comment lines (note that a # at the start of a line where the */
      /* previous line had a continuation mark is not considered a comment)       */
      if (buf[0] == '#' && cstate == 1)
      {
         /* When we get to a line that says:                                      */
         /*    #### AUTOMATICALLY GENERATED - DO NOT EDIT BELOW THIS LINE         */
         /* we want to just exit the loop and not gather any more lines           */
         if (!memcmp(buf, "#### AUTOMATICALLY GENERATED", 28)) break;
         continue;
      }

      extend = 0;
      buf[len] = 0; /* Wipe out the \n */
      if (buf[len-1] == '\\')
      {
         extend = 1;
         buf[--len] = 0;
      }

      if (cstate == 2)
      {
         /* This was a continuation line, just append it to the */
         /* Current string.                                     */
         Sym_Set(name, NULL, buf);
      }
      else
      {
         /* We need to parse out the name */
         p = strchr(buf, '=');
         if (p == NULL)
         {
            /* Funny line, we should issue a warning about it and just */
            /* Ignore it for now                                       */
         }
         else
         {
            /* Parse out the name that is being assigned to */
            val = p+1;

            /* Remove any trailing blanks from the name */
            while(p > buf && p[-1] == ' ') p--;
            *p = 0;
            /* Skip over any leading blanks after the '=' */
            while(*val == ' ') val++;
            p = buf;

            /* Remove any leading blanks from the name */
            while(*p == ' ') p++;
            len = strlen(p);

            /* We now have p pointing to the nul terminated name and val pointing */
            /* to a null terminated substitution string for the variable          */
            if (len > 63)
            {
               /* The name is too long, we should issue a warning and truncate it */
               p[63] = 0;
            }
            strcpy(name, p);
            /* We might consider looking up the name to see if this is a redefinition */
            /* For now we will be silent about it.                                */
            Sym_Set(name, val, NULL);
         }
      }
      cstate = 1;
      if (extend) cstate = 2;
   }
   /* Now we also want to gather in the automatically generated stuff so that      */
   /* we don't have to read it in again.  This would only need to be dealt with    */
   /* If they actually change the type of project that we are building.  This will */
   /* also have the effect of retaining any special configuration that they have   */
   /* possibly created on another machine.                                         */
   Sym_Set(SYM_HOLD, "", NULL);
   while((len = fread(buf, 1, 255, fp)) > 0)
   {
      buf[len] = 0;
      Sym_Set(SYM_HOLD, NULL, buf);
   }
   fclose(fp);

   go_dir(global.homedir);
   UnLock(global.workdir);
   global.workdir = xlockdir(Sym_Lookup("DIR"));
   return(0);
}

/***********************************************************************************
 * Procedure: write_file
 * Synopsis:  write_file()
 * Purpose:   Write out a dmake file based on the current options
 ***********************************************************************************/
int write_file()
{
   FILE *fp;
   char *place;
   char *name, *val;
   char *fname;

   go_dir(global.homedir);

   fname = Sym_Lookup(SYM_SCRIPT);

   name = Sym_Lookup("PROJECT");
   if (!*name) name = "*"; /* Force an invalid project name */

   while(*name)
   {
      if (strchr("#?\\/*: []{}^$&|()\"'", *name))
      {
         request(1, TEXT_BADPROJ, Sym_Lookup("PROJECT"), NULL);
         return(1);
      }
      name++;
   }

   if(!*fname)
   {
      request(1, TEXT_NOPROJ, NULL , NULL);
      return(1);
   }

   while ((fp = fopen(fname, "w")) == NULL)
   {
      struct stat stat_buf;

      if ( (stat(fname, &stat_buf) >= 0) &&
           (!(stat_buf.st_mode & S_IWRITE)))
      {
         /* The file exists, but is read-only.  See if they want us */
         /* to check it out for them to work on                     */
         if (!request(0, TEXT_SCRIPTCO, fname, NULL)) return;

         /* We need to check the file out for them */
         exec_command(global.text[CONFIG_CO], fname);
      }
      else
      {
         request(0, TEXT_BADFILE, fname, NULL);
         return(1);
      }

   }

   place = NULL;
   while(place = Sym_Next(place, &name, &val))
   {
      int nlen, vlen;

      nlen = strlen(name);
      vlen = strlen(val);

      if (nlen + vlen < LINE_LEN)
      {
         if (fprintf(fp, "%s= %s\n", name, val) <= 0) goto ioerr;
      }
      else
      {
         if (fputs(name, fp) || fputs("= ", fp)) goto ioerr;
         nlen += 2;
         while(nlen + vlen > LINE_LEN)
         {
            /* Attempt to break the line at a space - DMAKE doesn't require   */
            /* that the line end in a space, but it is easier for a person to */
            /* edit it when it works out that way.                            */
            nlen = LINE_LEN-nlen;
            while((nlen > MIN_LINE) && val[nlen-1] != ' ') nlen--;

            /* If we couldn't find a space on the line, just break it at the */
            /* target column anyways.                                        */
            if (nlen == MIN_LINE) nlen = LINE_LEN-nlen;

            if (fwrite(val, nlen, 1, fp) != 1) goto ioerr;
            if (fputs("\\\n", fp)) goto ioerr;
            val += nlen;
            vlen -= nlen;
            nlen = 0;
         }
         if (fputs(val, fp) || (fputc('\n', fp) == EOF)) goto ioerr;
      }
   }
   /* Add the separator line                                                */
   if (fputs("\n#### AUTOMATICALLY GENERATED - DO NOT EDIT BELOW THIS LINE\n", fp))
      goto ioerr;
   if (fputs(Sym_Lookup(SYM_HOLD), fp)) goto ioerr;
   fclose(fp);

   /* Now we need to create an icon for it */

   {
      struct DiskObject *dobj = NULL;

      if (global.oldproject)
         /* We read the project, so it may have a valid existing icon       */
         dobj = GetDiskObject(fname);
      if (dobj != NULL)
      {
         int len;
         len = strlen(dobj->do_DefaultTool);
         if ((len >= 5) && (!stricmp(dobj->do_DefaultTool+len-5, "vmake")))
            ;
         else
         {
            /* Don't trust it, make a new one                               */
            FreeDiskObject(dobj);
            dobj = NULL;
         }
      }
      if (dobj == NULL)
      {
         dobj = GetDiskObject("DCC:Config/Default_Project");
         if (dobj != NULL)
            PutDiskObject(fname,dobj);
      }
      if (dobj != NULL)
         FreeDiskObject(dobj);
   }
   return(0);

ioerr:
   request(1, TEXT_IOERR, fname, NULL);
   fclose(fp);
   return(1);
}

