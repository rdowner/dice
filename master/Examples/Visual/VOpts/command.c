#include "vopts.h"

Prototype void do_command(char *string);
Prototype void read_file(int type);
Prototype void save_file(int type);
Prototype int get_filename(char *str);
Prototype void reset_options(int reshow);
Prototype void reset_group(struct G_OBJECT *object);
Prototype int match_opt(char *optstr,char **valstr,char *buf, int exact);
Prototype int set_option(struct G_OBJECT *object,char **argv, int exact);
Prototype int parse_options(char *string);
Prototype int get_option(struct G_OBJECT *object, char **buf, int *len);
Prototype int append_option(char *optstr1, char *optstr2, char **buf, int *len);

parse_options(char *string);

/***********************************************************************************
 * Procedure: do_command
 * Synopsis:  do_command(string);
 * Purpose:   Execute the command associated with a given string
 ***********************************************************************************/
void do_command(char *string)
{
   char *p;
   char buf[10];
   int len;
   int type;

   /* The following valid commands are allowed: */
   /*    READ  [ file | ? | ENV ]               */
   /*    SAVE  [ file | ? | ENV ]               */
   /*    RESET                                  */
   /*    QUIT                                   */
   /*    PARSE option                           */

   len = 0;
   while (*string == ' ') string++;

   p = string;
   while(((*p >= 'A') && (*p <= 'Z')) ||
         ((*p >= 'a') && (*p <= 'z')))
   {
      if (*p < 'Z') /* Upper case characters which occur first in ASCII */
         buf[len++] = *p;
      else
         buf[len++] = 'A' + (*p - 'a');
      if (len > 5) break;
      p++;
   }
   buf[len] = 0;
   while(*p == ' ') p++;

   if (!strcmp(buf, "READ"))
   {
      global.fileop = 0;
      type = get_filename(p);
      if (type == 0)            /* user aborted filename                */
         return;
      SetWindowTitles(global.window, global.title, global.wtitle);
      global.filetype = type;
      read_file(type);
   }
   else if (!strcmp(buf, "SAVE"))
   {
      global.fileop = 1;
      type = get_filename(p);
      if (type == 0)            /* user aborted filename                */
         return;
      if (request(0, TEXT_SAVEOK, global.filename, NULL) == 0)
         /* Chickened out...    */
         return;
      SetWindowTitles(global.window, global.title, global.wtitle);
      global.filetype = type;
      save_file(type);
   }
   else if (!strcmp(buf, "QUIT"))
   {
      global.done = 1;
   }
   else if (!strcmp(buf, "PARSE"))
   {
      char tbuf[MAX_STRING+1];
      strncpy(tbuf, p, MAX_STRING);
      set_gadgets(0);
      parse_options(tbuf);
      set_gadgets(1);
   }
   else if (!strcmp(buf, "RESET"))
   {
      reset_options(1);
   }
   else
   {
      /* We have an invalid option, ignore the command and let them know  */
      /* about the problem with the option.                               */
      request(1, TEXT_BADCMD, buf, NULL);
      return;
   }
}
/***********************************************************************************
 * Procedure: read_file
 * Synopsis:  read_file(type)
 * Purpose:   read file, extract options according to (type) and display
 ***********************************************************************************/
void read_file(int type)
{
   char buf[256];

   if (type == FILE_ENV)
   {
      strncpy(buf, getenv("DCCOPTS"), 255);
      reset_options(0);
      parse_options(buf);
   }
   else
   {
      FILE *fp;
      int cstate;

      fp = fopen(global.filename, "r");
      if (fp == NULL)
      {
         request(1, TEXT_BADFILE, global.filename, NULL);
         return;
      }

      reset_options(0);
      cstate = 1;
      while(cstate && (fgets(buf, 256, fp) != NULL))
      {
         int len;
         int extend;

         len = strlen(buf)-1;
         if (len < 2) continue;

         buf[len] = 0; /* Wipe out the \n */
         extend = 0;
         if (buf[len-1] == '\\')
         {
            extend = 1;
            buf[--len] = 0;
         }

         switch(type)
         {
            case FILE_C:
               /* Make sure it says #pragma DCCOPTS */
               if (!memcmp(buf, "#pragma DCCOPTS ", 16))
               {
                  /* We actually got it */
                  if (cstate == 3)
                  {
                     request(1, TEXT_EXTRAPRG, NULL, NULL);
                     cstate = 0;
                  }
                  else
                  {
                     if (parse_options(buf+16))
                        cstate = 0;
                     else
                        cstate = 2;
                  }
               }
               else
               {
                  if (cstate == 2) cstate = 3;
               }
               break;
            case FILE_OPTIONS:
               cstate = !parse_options(buf);
               break;
            case FILE_DMAKEFILE:
               if ((!memcmp(buf, "CFLAGS=", len=7)) ||
                   (len = 0, cstate == 2))
               {
                  cstate = 0;
                  if (extend) cstate = 2;
                  if (parse_options(buf+len))
                     cstate = 0;
               }
               break;
         }
      }
   global.nameoffile = 1; /* indicate that global.filename has been read in */
   fclose(fp);
   }
   set_gadgets(1);
}

/***********************************************************************************
 * Procedure: save_file
 * Synopsis:  save_file(type)
 * Purpose:   Write user selected options to file, formatted for type
 ***********************************************************************************/
void save_file(int type)
{
#define MAXOPTSTR 512

   char buf[MAXOPTSTR];
   char *pbuf=buf;
   struct G_GROUP *group;
   int  ok, len = MAXOPTSTR - 1;

   *buf = 0;    /* just in case no options are set                      */

   if (ok = get_option(global.objects, &pbuf, &len)) /* didn't over run buffer...*/
      for (group = global.groups;
           ok  && (group != NULL);
           group = (struct G_GROUP *)group->base.next)
         {
         if (!((type == FILE_ENV) && group->local))
            ok = get_option(group->objects, &pbuf, &len);
         }
   if (!ok)
      {
      ok = request(0, TEXT_BIGOPTS, NULL, NULL);
      if (!ok) return;
      }
#ifdef JGM_DBG
printf("Options: %s\n", buf);
#endif

   if (*buf == 0) return;       /* let's not try anything fancy         */

   if (type == FILE_ENV)
   {
      setenv("DCCOPTS", buf);
   }
   else
   {

      FILE *fpout, *fpin;

      if (global.nameoffile != 1)
      {
         /* didn't already read this file - does it exist? */
         fpin = fopen(global.filename, "r");
         /* yes I know lock is more efficient ... */
         if (fpin != NULL)
         {
            fclose(fpin); /* it don't have nothing we want */
            ok = request(0, TEXT_OVWRITEOK, global.filename, NULL);
            if (!ok) return;
         }
      }

      switch (type)
      {
         case FILE_OPTIONS:
         {
            char *p1 = buf, *p2;

            fpout = fopen(global.filename, "w");
            if (fpout == NULL)
            {
               request(1, TEXT_BADFILE, global.filename, NULL);
               return;
            }

            do
            {
               p2 = strchr(p1+1, '-');
               if (p2 != NULL)
                  p2[-1] = '\0';
               fputs(p1, fpout);
               fputc('\n', fpout);
               p1 = p2;
            }
            while (p1 != NULL);
            fclose(fpout);
            break;
         }
         case FILE_DMAKEFILE:
         {
            int werr = 1;       /* init to no error                     */
            char newname[MAX_FILENAME+11];
            char copybuf[256];

            strcpy(newname, global.filename);
            strcat(newname, ".vopt-temp");
            if ((fpin = fopen(newname, "r")) != NULL)
            {
               fclose(fpin);
               remove(newname);
            }
            if (rename(global.filename, newname) < 0)   /* error        */
            {
               request(1, TEXT_NORENAME, global.filename, NULL);
               return;
            }
            if ((fpin = fopen(newname, "r")) == NULL)
            {
               rename(newname, global.filename);
               request(1, TEXT_NOREAD, global.filename, NULL);
               return;
            }
            if ((fpout = fopen(global.filename, "w")) == NULL)
            {
               fclose(fpin);
               rename(newname, global.filename);
               request(1, TEXT_BADFILE, global.filename, NULL);
               return;
            }

            /* note that both fprintf() and fputs() return <0 for error */
            while ((fgets(copybuf, 256, fpin) != NULL) && (werr >= 0))
            {
               if (!memcmp(copybuf, "CFLAGS=", 7))
                  werr = fprintf(fpout, "%s%s\n", "CFLAGS= ", buf);
               else
                  werr = fputs(copybuf, fpout);
            }
            fclose(fpout);
            fclose(fpin);
            if (werr < 0)       /* error writing file - don't rename    */
            {
               request(1, TEXT_NOWRITE, newname, NULL);
               remove(global.filename);
               rename(newname, global.filename);
               return;
            }
            remove(newname); /*delete backup copy of file */
            break;
         }
         case FILE_C:
         {
            int werr = 1;   /* init to no error                 */
            int cstate = 0; /* -1 = #pragma DCCOPTS
                                0 = ordinary line
                                1 = comment continuation
                            */
            char newname[MAX_FILENAME+11];
            char copybuf[256];

            strcpy(newname, global.filename);
            strcat(newname, ".vopt-temp");
            if ((fpin = fopen(newname, "r")) != NULL)
            {
               fclose(fpin);
               remove(newname);
            }
            if (rename(global.filename, newname) < 0)   /* error        */
            {
               request(1, TEXT_NORENAME, global.filename, NULL);
               return;
            }
            if ((fpin = fopen(newname, "r")) == NULL)
            {
               rename(newname, global.filename);
               request(1, TEXT_NOOVWRITE, global.filename, NULL);
               return;
            }
            if ((fpout = fopen(global.filename, "w")) == NULL)
            {
               fclose(fpin);
               rename(newname, global.filename);
               request(1, TEXT_BADFILE, global.filename, NULL);
               return;
            }

            werr = fprintf(fpout, "%s%s\n", "#pragma DCCOPTS ", buf);
            while ((fgets(copybuf, 256, fpin) != NULL) && (werr >= 0))
            {
               if (cstate == 0)
               {
                  if (!memcmp(copybuf, "#pragma DCCOPTS ", 16))
                     cstate = -1;
                  else
                  ;
               }
               /* else check for close comment                          */

               if (cstate >= 0)
                  werr = fputs(copybuf, fpout);
               else
                  cstate = 0;   /* next line is ordinary - not comment  */
            }

            fclose(fpout);
            fclose(fpin);
            if (werr < 0)       /* error writing file - don't rename    */
            {
               request(1, TEXT_NOOVWRITE, newname, NULL);
               remove(global.filename);
               rename(newname, global.filename);
               return;
            }
            remove(newname); /*delete backup copy of file */
            break;
         }
         default:
         printf("unimplemented file type %d\n", type);
      }
   }
}

/***********************************************************************************
 * Procedure: get_filename
 * Synopsis:  type = get_filename(str)
 * Purpose:   Parse out a file descriptor and return a type for that name
 ***********************************************************************************/
int get_filename(char *str)
{
   char *p;
   int len;

   if (*str)
   {
      /* They actually specified a name.  All we need to do is replace the */
      /* current one.  If they specified a '?', then we need to prompt the */
      /* user with the file requester.  We will assume the ASL one for now */
      /* but should have a reasonable fall back one.                       */
      if (!strcmp(str, "?"))
      {

         if (global.freq) /* could be ASL or Arp */
         {
            int n;
            if (AslBase != NULL)
            {
/*
               struct TagItem taglist[2];
*/
               struct TagItem taglist[5];
#if 0
               taglist[0].ti_Tag = ASL_Hail;
               taglist[0].ti_Data = (ULONG)(v+1);
#endif
/* new */
               /* Pattern to match against project files */
               taglist[0].ti_Tag  = ASL_Pattern;
               taglist[0].ti_Data = (ULONG)"(#?FILE|#?DM#AK#?|#?.DICE)";

               taglist[1].ti_Tag  = ASL_File;
               taglist[1].ti_Data = (ULONG)"";

               taglist[2].ti_Tag  = ASL_Dir;
               taglist[2].ti_Data = (ULONG)"";


               taglist[3].ti_Tag = TAG_DONE;
   
               if (!AslRequest( (APTR)global.freq, taglist))
                  return(0);
            }
            else /* must be Arp since we have a requester... */
            {
               if (!ArpFileRequest(global.freq))
                  return (0);
            }   
            n = strlen(global.freq->rf_Dir);
            if (n > 255) n = 255;
            p = global.filename;
            memcpy(p, global.freq->rf_Dir, n);
            p += n;

            if ((n > 0) && (p[-1] != ':') && (p[-1] != '/'))
            {
               *p++ = '/';
               n++;
            }
            len = strlen(global.freq->rf_File);
            if ((n + len) > 255) len = 255-n;
            strncpy(p, global.freq->rf_File, len);
            p[len] = 0;

            global.nameoffile = 0; /* new name - possibly unsafe to write */
         }
         else
         {
            /* We don't have ASL.LIBRARY, try something else instead */
            set_fr_gadgets(global.filename);
            /* If user hits save button, we'll be back with the name */
            return 0;

         }
      }
      else
      {
         strncpy(global.filename, str, MAX_FILENAME);
      }
   }

   if (!stricmp(global.filename, "ENV"))
   {
      strcpy(global.title, global.text[TEXT_GLOBOPTS]);
      return(FILE_ENV);
   }

   len = strlen(global.filename);
   p = global.filename + strlen(global.filename);

   strcpy(global.title, global.text[TEXT_FILEOPTS]);
   strncat(global.title, global.filename, MAX_TITLE);
   global.title[MAX_TITLE] = 0;

   if (len < 2)
      return(FILE_OPTIONS);

   if ((len >= 7) && (!stricmp(p-7, "DCCOPTS")))
      return(FILE_OPTIONS);

   if ((len >= 9) && (!stricmp(p-9, "DMAKEFILE")))
      return(FILE_DMAKEFILE);

   if ((len >= 5) && (!stricmp(p-5, ".DICE")))
      return(FILE_DMAKEFILE);

   if (!stricmp(p-2, ".H"))
      return(FILE_C);

   if (!stricmp(p-2, ".C"))
      return(FILE_C);

   return(FILE_OPTIONS);
}

/***********************************************************************************
 * Procedure: reset_options
 * Synopsis:  reset_options(reshow)
 * Purpose:   Reset all the options to the default values
 ***********************************************************************************/
void reset_options(int reshow)
{
   struct G_GROUP *group;

   set_gadgets(0);

   reset_group(global.objects);
   for(group = global.groups; group != NULL;
       group = (struct G_GROUP *)group->base.next)
   {
      reset_group(group->objects);
   }
   if (reshow) set_gadgets(1);
}

/***********************************************************************************
 * Procedure: reset_group
 * Synopsis:  reset_group(object)
 * Purpose:   Reset all the options in a group to the default values
 ***********************************************************************************/
void reset_group(struct G_OBJECT *object)
{
#define object_check ((struct G_CHECK *)object)
#define object_list  ((struct G_LIST *)object)
#define object_str   ((struct G_STRING *)object)
#define object_cycle ((struct G_CYCLE *)object)

   while(object != NULL)
   {
      switch(object->class)
      {
         case CLASS_STRING:
            object_str->buf[0] = 0;
            break;
         case CLASS_CYCLE:
            {
               struct G_VALUE *val;

               val = object_cycle->curval = object_cycle->values;
               while(val != NULL)
               {
                  if (val->string)
                     val->string->buf[0] = 0;
                  val = val->next;
               }
            }
            break;
         case CLASS_LIST:
            {
               struct G_ENTRY *ent;
               ent = object_list->first;
               object_list->first = object_list->top = NULL;
               object_list->string = object_list->maxent = 0;
               while(ent != NULL)
               {
                  struct G_ENTRY *nextent;

                  nextent = (struct G_ENTRY *)ent->base.next;
                  free_mem(ent, sizeof(struct G_ENTRY));
                  ent = nextent;
               }
            }
            break;
         case CLASS_CHECK:
            object->state = 0;
            if (!object_check->option1[0]) object->state = 1;
            break;
         default:
            object->state = 0;
            break;
      }
      object = object->next;
   }
#undef object_list
#undef object_str
#undef object_cycle
#undef object_check
}
/***********************************************************************************
 * Procedure: match_opt
 * Synopsis:  rc = match_opt(optstr, argv, buf, exact)
 * Purpose:   Compare a string to see if it matches a given option
 *            If a successful match, return the number of argv entries used.
 *            If exact is specified, do not look for extension strings.
 ***********************************************************************************/
int match_opt(char *optstr,
              char **argv,
              char *buf,
              int exact
             )
{
   char *p;
   int len, olen;

   p = strchr(optstr, '%');
   if ((p == NULL) || (buf == NULL))
   {
      /* We have just a plain option.  For it to match, we must have */
      /* a complete match on the characters.                         */
      return(!strcmp(optstr, *argv));
   }

   if (exact) return 0; /* not looking for options strings this pass    */

   /* Not a plain one.  There are a couple of possibilities:       */
   /*  -opt%s       No space allowed after the option              */
   /*  -opt %s      Space after option is optional                 */
   /*  -opt  %s     Space after option is mandatory.               */
   olen = len = p - optstr;
   while(len && (optstr[len-1] == ' ')) len--;
   if (memcmp(optstr, *argv, len))
      return(0);

   /* We know that at least we match the start of the option.  Now we */
   /* Have to get the buffer filled in.                               */
   if ((*argv)[len]) /* We know that the parameter is with this option */
   {
      /* If the space is mandatory, we obviously didn't make a match  */
      if ((olen - len) > 1) return(0);
      strncpy(buf, argv[0]+len, MAX_STRING);
      return(1);
   }

   /* Parameter is with the next option */
   /* If the space is not allowed, we didn't get a match */
   if ((olen == len) || (argv[1] == NULL)) return(0);
   strncpy(buf, argv[1], MAX_STRING);
   return(2);
}

/***********************************************************************************
 * Procedure: append_option
 * Synopsis:  rc = append_option(optstr, buffer, length)
 * Purpose:   append option to string buffer, update count and pointer
 ***********************************************************************************/
int append_option(char *optstr1, char *optstr2, char **buf, int *len)
{
   int l;
   char *p, *optr;

   optr = optstr1;      /* unless we change it to fetch a string        */
   p = strchr(optstr1, '%');
   if ((p != NULL) && (optstr2 != NULL)) /* seems we have a string      */
   {
      l = p - optstr1;
      if (l >= *len) return 0;  /*no room to save it                    */
      if (optstr1[l-1] == ' ')
         l -= 1;
      memcpy(*buf, optstr1, l);
      *buf += l;
      *len -= l;
      **buf = '\0';     /* just in case next step fails...              */
      optr = optstr2;
   }

   /* now copy whatever is left and put a blank on the end              */
   l = strlen(optr);
   if (l >= *len) return 0;     /* no room to save it                   */
   strcpy(*buf, optr);
   *buf += l;
   strcpy(*buf, " ");
   *buf += 1;
   *len -= (l + 1);

   return 1;
}

/***********************************************************************************
 * Procedure: get_option
 * Synopsis:  rc = get_option(object, buffer, length)
 * Purpose:   make a string of all options that are set
 ***********************************************************************************/
int get_option(struct G_OBJECT *object,
               char **buf,
               int *len
              )
{
#define object_list  ((struct G_LIST   *)object)
#define object_str   ((struct G_STRING *)object)
#define object_check ((struct G_CHECK  *)object)
#define object_cycle ((struct G_CYCLE  *)object)
   while (object != NULL)
   {
      switch (object->class)
      {
         case CLASS_CYCLE:
         {
            struct G_VALUE *val;

            val = object_cycle->curval;
            if (val->option[0] == 0) break;     /* option not set       */
            if
            (
                append_option(val->option,
                              val->string != NULL ? val->string->buf : NULL,
                              buf, len)
                == 0
            )
               return 0;        /* no room to save it                   */
            break;
         }
         case CLASS_CHECK:
         {
            char *p;
            p = object->state ? object_check->option1 : object_check->option0;
            if (*p)             /* do we gots a string?                 */
               if (append_option(p, NULL, buf, len) == 0)
                  return 0;     /* shucks, nowhere to put it            */
            break;
         }
         case CLASS_STRING:
         {
            if (object_str->buf[0] == 0) break; /* empty string         */
            if
            (
                append_option(object_str->option,
                              object_str->buf,
                              buf, len)
                == 0
            )
               return 0;        /* no room, buf is full...              */
            break;
         }
         case CLASS_LIST:
         {
            struct G_ENTRY *ent;

            ent = object_list->first;
            while (ent != NULL) /* empty list will fall through         */
            {
               if
               (append_option(object_list->option, ent->buf, buf, len) == 0)
                  return 0;     /* seem familiar?                       */
               ent = (struct G_ENTRY *)ent->base.next;
            }
            break;
         }
      }
      object = object->next;
   }
   return 1;
#undef object_check
#undef object_list
#undef object_str
#undef object_cycle
}

/***********************************************************************************
 * Procedure: set_option
 * Synopsis:  rc = set_option(object, optstr, exact)
 * Purpose:   Set an option based on a option string, allowing parms if not exact
 ***********************************************************************************/
int set_option(struct G_OBJECT *object,
               char **argv,
               int exact
              )
{
   int used;
#define object_list  ((struct G_LIST   *)object)
#define object_str   ((struct G_STRING *)object)
#define object_check ((struct G_CHECK  *)object)
#define object_cycle ((struct G_CYCLE  *)object)

   used = 0;

   while((used == 0) && (object != NULL))
   {
      switch(object->class)
      {
         case CLASS_STRING:
            used = match_opt(object_str->option, argv, object_str->buf, exact);
            break;
         case CLASS_CYCLE:
            {
               struct G_VALUE *val;

               for(val = object_cycle->values;
                   val != NULL;
                   val = (struct G_VALUE *)val->next)
               {
                  used = match_opt(val->option, argv,
                                   val->string ? val->string->buf : NULL,
                                   exact);
                  if (used)
                  {
                     object_cycle->curval = val;
                     break;
                  }
               }
            }
            break;
         case CLASS_CHECK:
            used = match_opt(object_check->option0, argv, NULL, exact);
            if (used)
            {
               object->state = 0;
            }
            else
            {
               used = match_opt(object_check->option1, argv, NULL, exact);
               if (used)
               {
                  object->state = 1;
               }
            }
            break;
         case CLASS_LIST:
            {
               char buf[MAX_STRING+1];

               used = match_opt(object_list->option, argv, buf, exact);
               if (used)
               {
                  struct G_ENTRY *ent, *prevent;

                  ent = get_mem(sizeof(struct G_ENTRY));
                  if (ent)
                  {
                     strcpy(ent->buf, buf);
                     if ((prevent = object_list->top) == NULL)
                        object_list->top = object_list->first = ent;
                     else
                     {
                        while(prevent->base.next)
                           prevent = (struct G_ENTRY *)prevent->base.next;
                        prevent->base.next = (struct G_OBJECT *)ent;
                        ent->base.prev = (struct G_OBJECT *)prevent;
                     }
                  }
               }
            }
            break;
         default:
            object->state = 0;
            break;
      }
      object = object->next;
   }

   return(used);
#undef object_check
#undef object_list
#undef object_str
#undef object_cycle
}

/***********************************************************************************
 * Procedure: parse_options
 * Synopsis:  parse_options(string);
 * Purpose:   Parse an option and set the appropriate options in the global options
 *            data.  This subroutine destroys the string buffer.
 ***********************************************************************************/
#define MAX_BUF 100

int parse_options(char *string)
{
#define MAX_ARGS 80

   /* First we need to create an ARGV type array */
   /* We will assume no more than 80 arguments in a line */
   char *argv[MAX_ARGS];
   int argc;
   int state;
/*
 * Character Classes:
 *   SQ    '           Single Quote
 *   DQ    "           Double Quote
 *   BL    Space, Tab  White Space
 *   NL    Null        End of the string
 *   OT    Other       Anything else
 *        SQ   DQ   BL   NL   OT
 *    0    1    2    0    x    3
 *    1   >0   +1   +1   >x   +1
 *    2   +2   >0   +2   >x   +2
 *    3   +3   +3   >0   >x   +3
 */
#define SQ 0
#define DQ 1
#define BL 2
#define NL 3
#define OT 4
#define EXI 15
#define STATE_MASK 0x0f
#define ACTION_SHIFT  4
#define ACT_SKP 1
#define ACT_OUT 2
#define ACT_BEG 3
#define SKP (ACT_SKP << ACTION_SHIFT)
#define OUT (ACT_OUT << ACTION_SHIFT)
#define BEG (ACT_BEG << ACTION_SHIFT)

   static char class_tab[][5] = {
      { SKP|1, SKP|2,     0,       EXI, BEG|3 },
      { OUT|0,     1,     1,   OUT|EXI,     1 },
      {     2, OUT|0,     2,   OUT|EXI,     2 },
      {     3,     3, OUT|0,   OUT|EXI,     3 }
   };

   argc = 0;
   state = 0;
   while(state != EXI)
   {
      int class;
      int look;

      switch(*string)
      {
         case '\'': class = SQ; break;
         case '"' : class = DQ; break;
         case ' ' :
         case '\t': class = BL; break;
         case    0: class = NL; break;
         default:   class = OT; break;
      }
      look = class_tab[state][class];
      state = look & STATE_MASK;
      switch(look >> ACTION_SHIFT)
      {
         case ACT_SKP:
            argv[argc++] = string+1;
            break;
         case ACT_OUT:
            *string = 0;
            break;
         case ACT_BEG:
            argv[argc++] = string;
            break;
      }
      string++;
   }

   if (argc == 0) return(0);

   argv[argc] = NULL;

   {
      int i, used, exact;
      struct G_GROUP *group;

      for(i = 0; i < argc; i += used)
      {

         used = 0;      /* we've used what we used before       */
         /* do exact matches first to find things like -I0      */
         for (exact = 1; (exact >= 0) && (used == 0); exact -= 1)
         {
            used = set_option(global.objects, argv+i, exact);

            for(group = global.groups; (used == 0) && (group != NULL);
                group = (struct G_GROUP *)group->base.next)
            {
               used = set_option(group->objects, argv+i, exact);
            }
         }
         if ((!used) && (exact <= 0))
         {
            /* The option wasn't found, tell them in some way */
            if (!request(0, TEXT_BADOPT, argv[i], NULL))
               return(1);
            else
               used = 1; /* throw away the token and continue */
         }

      }
   }
   return(0);
}
