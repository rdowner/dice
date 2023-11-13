#include "vmake.h"

Prototype int build_command(char *buf, int len, char *template, char *sel);
Prototype void do_command(char *string);
Prototype int save_current(int perm);
Prototype void reset_options(void);
Prototype int set_option(struct G_OBJECT *object,char **argv);
Prototype void exec_command(char *cmd, char *sel);
Prototype void set_workdir(void);

void unload_symbols(void);
void load_symbols(void);
void add_list(struct G_LIST *list, char *name, int private);
void add_listitem(struct G_LIST *list, char *name, int private, int dirty);

#define MAX_SYMBOL 31
#define LINE_LEN  78
#define MIN_LINE  10
#define SYM_PROJCFG "CFGNAME"

static int showcmd;

/***********************************************************************************
 * Procedure: get_cursel
 * Synopsis:  name = get_cursel();
 * Purpose:   Return the name associated with the current selection
 ***********************************************************************************/
static char *get_cursel(void)
{
   struct G_OBJECT *object;
#define object_list  ((struct G_LIST   *)object)

   /* We need to determine the currently selected file */
   for(object = global.objects; object != NULL; object = object->next)
   {
      if ((object->class == CLASS_LIST) && object_list->sel)
         return(object_list->sel->buf);
#undef object_list
   }
   return("");
}

/*****************************************************************************
 * Procedure: GetPubScrName
 * Synopsis:  bool = GetPubScrName(scr, buff)
 * Purpose:   fill the name of public a screen int a buffer, return TRUE
 *            if found
 *****************************************************************************/
int GetPubScrName( struct Screen * scr, UBYTE * namebuff )
{
   struct List          * scrlist;
   struct Node          * node;

   namebuff[0] = 0;

   if (AslBase) /* need Dos 2.0 or higher for this trick */
      if(scrlist = LockPubScreenList()) 
      {
         /* traverse the public screen node list looking for our screen */
         for ( node = scrlist->lh_Head; node->ln_Succ; node = node->ln_Succ ) 
         {
            if ( ((struct PubScreenNode *)node)->psn_Screen == scr ) 
            {
               strcpy( namebuff, node->ln_Name );
               break;
            }
         }

         UnlockPubScreenList();
      }

   return( namebuff[0] ? TRUE : FALSE );

} // GetPubScrName()

/***********************************************************************************
 * Procedure: build_command
 * Synopsis:  rc = build_command(buf, len, template)
 * Purpose:   Construct a command to be issued
 *            The template may have % strings in it where we will substitute:
 *               %s   - Current selection
 *               %f   - The current DMAKEFILE
 *               %o   - An optional selection
 *               %t   - The object module version of a file
 *               %r   - Relative pathname of file
 *               %(x) - The expansion of symbol X
 *               %%   - insert one literal % in the buffer
 ***********************************************************************************/
int build_command(char *buf, int len, char *template, char *sel)
{
   char *name;
   int nlen;
   int makeobj;
   /* note: MAX_FILENAME is greater than MAXPBUBSCREENNAME */
   char exname[MAX_FILENAME+1];

   while(*template && len)
   {
      if (*template != '%')
      {
         *buf++ = *template++;
         len--;
      }
      else
      {
         int cvalid = 1; /* Assume valid substitution */
         char req;
         makeobj = 0;
         req = toupper(template[1]);
         template += 2; /* skip over %x */
         name = sel;
         switch(req)
         {
            case 'F':
               {
                  BPTR savelock = 0;
                  if (global.homedir)
                     savelock = CurrentDir(global.homedir);
                  expand_filename(Sym_Lookup(SYM_SCRIPT), exname);
                  name = exname;
                  if (savelock)
                     CurrentDir(savelock);
                  break;
               }
            case '(':
               {
                  char symbol[MAX_SYMBOL+1];
                  int i;
                  i = 0;
                  for(i = 0; *template && (*template != ')'); template++)
                     if (i < MAX_SYMBOL) symbol[i++] = *template;
                  symbol[i] = 0;
                  name = Sym_Lookup(symbol);
                  if (*template == ')') template++;
                  if (!*name)
                  {
                     i = atoi(symbol);
                     if ((i > 0) && (i <= NUM_CONFIG))
                     {
                        name = global.text[CONFIG_BASE-1+i];
                        if (name == NULL) name = "";
                        else
                        {
                           int rc;

                           /* We want to recurse to parse out any substitutions     */
                           /* in the string.  Note that in order to prevent runaway */
                           /* recursion, we temporarily kill the config string      */
                           global.text[CONFIG_BASE-1+i] = "";
                           rc = build_command(buf, len, name, sel);
                           global.text[CONFIG_BASE-1+i] = name;

                           /* if something went wrong on the recursion, just get    */
                           /* out of here.                                          */
                           if (rc) return(i);

                           /* This is a little tricky, what we are doing is cheating*/
                           /* by letting it fill in the buffer with what is already */
                           /* there.  This is harmless (the copy is onto itself) and*/
                           /* allows us to get the length checking done for free    */
                           name = buf;
                        }
                     }
                     else
                     {
                        int max, istart;
                        switch (toupper(*symbol))
                        {
                        case 'T':
                           max = NUM_TEXT;
                           istart = 0;
                           break;
                        case 'C':
                           max = NUM_CONFIG;
                           istart = CONFIG_BASE;
                           break;
                        case 'S':
                           max = NUM_SUBRTN;
                           istart = SUBRTN_BASE;
                           break;
                        default:
                           max = 0;
                           break;
                        }
                        i = atoi(symbol+1);
                        if ((i > 0) && (i <= max))
                        {
                           name = global.text[istart+i-1];
                        }
                     }
                  }
               }
               break;

            case 'T':
               {
                  char *p;
                  if (!*name)
                  {
                     p = Sym_Lookup("EXEDIR");
                     name = Sym_Lookup("PROJECT");
                  }
                  else
                  {
                     p = Sym_Lookup("OD");
                     makeobj = 1;
                  }

                  /* we need to first copy over the output directory */
                  nlen = strlen(p);
                  if (nlen > len)
                  {
                     request(1, TEXT_BUFFOVFL, NULL, NULL);
                     return(1);
                  }
                  strcpy(buf, p);
                  buf += nlen;
                  len -= nlen;
               }
               break;

            case 'N':
               if (global.screen && GetPubScrName(global.screen, exname))
                  name = exname;
               else
                  name = "Workbench";
               break;

            case 'R':
            case 'S':
               /* If nothing is selected, and they required a selection, let  */
               /* them know and abort the command                             */
               if (!*sel)
               {
                  request(1, TEXT_NOSEL, NULL, NULL);
                  return(1);
               }

               /* Expand pathname for 'S', not for 'R' */
               if (req == 'R') break;
               /* Else fall through to prepend the path to the name */

            case 'O':
               expand_filename(sel, exname);
               name = exname;
               break;
            case '%':
               /* interpret "%%" as just "%" and stuff it into the command    */
               /* buffer.  Fall through to default processing for literals.   */
               template += 1;
            default:
               cvalid = 0; /* just copy literally, not a substitution */
         }
         if (cvalid) /* Means we're doing a substitution */
         {
            /* name is what we want to fill the current buffer with */
            nlen = strlen(name);
            if (nlen > len)
            {
               request(1, TEXT_BUFFOVFL, NULL, NULL);
               return(1);
            }
            strcpy(buf, name);
            if (makeobj)
            {
               /* If they requested a output file, we need to convert the */
               /* file to an object file name                             */
               if (nlen > 2 && (buf[nlen-2] == '.'))
                  buf[nlen-1] = 'o';
            }
            buf += nlen;
            len -= nlen;
         }
         else /* Just copy over literally */
         {
            /* just do the '%' here, main loop checks length as well */
            *buf++ = '%';
            template -= 1;  /* back up to char after '%' */
            len--;
         }
      }
   }
   *buf++ = 0;  /* Don't forget to null terminate the result */
   return(0);
}

/***********************************************************************************
 * Procedure: parse_keyword
 * Synopsis:  ptr = parse_keyword(str, buf);
 * Purpose:   Parse a keyword into the buffer
 ***********************************************************************************/
char *parse_keyword(char *str, char *buf)
{
   int i;
   /* Gather the string into a buffer */
   i = 0;
   while(*str == ' ') str++;

   while((*str) && (*str != ' '))
   {
      buf[i++] = *str++;
      if (i > MAX_SYMBOL) return(str);
   }
   buf[i] = 0;
   while(*str == ' ') str++;
   return(str);
}

/***********************************************************************************
 * Procedure: exec_command
 * Synopsis:  exec_command(cmd, selection)
 * Purpose:   Execute the given command using appropriate substitutions
 ***********************************************************************************/
void exec_command(char *cmd, char *sel)
{

   go_dir(global.workdir);

   if (!build_command(global.cbuf, global.cbufsize, cmd, sel))
   {
      if (showcmd)
         PostLog(global.cbuf);
      IssueCommand(global.cbuf);
   }
}

/***********************************************************************************
 * Procedure: send_rexx
 * Synopsis:  send_rexx(cmd)
 * Purpose:   Send the rexx command to the given port name
 ***********************************************************************************/
void send_rexx(char *cmd)
{
   char *res;
   long ec;
   char port[MAX_SYMBOL+1];

   go_dir(global.workdir);

   if (!build_command(global.cbuf, global.cbufsize, cmd, get_cursel()))
   {
      if (showcmd)
         PostLog(global.cbuf);
      /* Parse out the port name */
      cmd = parse_keyword(global.cbuf, port);
      /* Send the command off to rexx to be processed */
      /* We will wait here until it is complete       */
      PlaceRexxCommandDirect(NULL, port, cmd, &res, &ec);
      if (res != NULL)
      {
         PostLog(res);
         free(res);
      }
   }

}

/***********************************************************************************
 * Procedure: log_message
 * Synopsis:  log_message(msg, dorexx)
 * Purpose:   Output a message to the session log
 ***********************************************************************************/
void log_message(char *msg, int dorexx)
{
   int gotmsg;

   /* set RC in case build_command() fails and doesn't say why */
   if (dorexx)
      global.rexxrc = TEXT_BADPARM;

   gotmsg = !build_command(global.cbuf, global.cbufsize, msg, get_cursel());
   if (gotmsg)
      PostLog(global.cbuf);
   else
      PostLog("-------------------");

   if (dorexx)
   {
      if (gotmsg)
      {
         /* undo previous default -- build_command() worked */
         global.rexxrc = 0;
         strcpy(global.rexxrs, global.cbuf);
         global.rexxrs[global.cbufsize] = 0;
      }
   }
}

/***********************************************************************************
 * Procedure: set_workdir
 * Synopsis:  set_workdir();
 * Purpose:   Set the working directory based on the current directory and symbol
 *            values
 ***********************************************************************************/
void set_workdir(void)
{
   if (test_dirty())
   {
      load_symbols();
   }

   go_dir(global.homedir);
   UnLock(global.workdir);
   global.workdir = xlockdir(Sym_Lookup("DIR"));
   go_dir(global.workdir);
}

/***********************************************************************************
 * Procedure: save_current
 * Synopsis:  rc = save_current();
 * Purpose:   Save the current file (if it has changed)
 ***********************************************************************************/
int save_current(int perm)
{
   if (test_dirty())
   {
      int rc;

      rc = request(0, TEXT_ASKSAVE,
                   Sym_Lookup(SYM_SCRIPT), global.text[TEXT_SAVE]);

      if (!rc) return(1);

      if (rc == 2)
      {
         load_symbols();
         if (write_file()) return(1);
      }
      mark_clean();
   }
   return(0);
}

/***********************************************************************************
 * Procedure: lookup_keyword
 * Synopsis:  kw = lookup_keyword(&p, keystr);
 * Purpose:   Lookup the keyword given and return the position of the next token
 *            (If the keyword was found).  If the keyword wasn't found, -1 is returned
 ***********************************************************************************/
int lookup_keyword(char **bufp, char *keystr)
{
   int i;
   char *p;
   char buf[MAX_SYMBOL+1];

   p = parse_keyword(*bufp, buf);

   /* Look for the keyword in the table they provided us */
   for(i = 0; *keystr; keystr += strlen(keystr)+1, i++)
   {
      if (!stricmp(buf, keystr))
      {
         /* We got it.  Return the end of the string for them */
         *bufp = p;
         return(i);
      }
   }

   /* We didn't find the keyword, so let them know */
   return(-1);
}

char cmdlist[] =
#define KEY_TRACE     0
           "TRACE\0" /* Debug the program in the current mode         */
#define KEY_EDIT      1
           "EDIT\0"  /* Invoke the editor on the current file         */
#define KEY_EXEC      2
           "EXEC\0"  /* Execute the given command                     */
#define KEY_NEW       3
           "NEW\0"   /* RESET, Prompt for a new file name             */
#define KEY_QUIT      4
           "QUIT\0"  /* Exit                                          */
#define KEY_READ      5
           "READ\0"  /* Read a file                                   */
#define KEY_SAVE      6
           "SAVE\0"  /* Save file                                     */
#define KEY_FRONT     7
           "FRONT\0" /* Pop Window to the front                       */
#define KEY_BACK      8
           "BACK\0"  /* Send Window to the back                       */
#define KEY_JUMP      9
           "JUMP\0"  /* Jump window to a given screen                 */
#define KEY_LOG      10
           "LOG\0"   /* Output a log message to the screen            */
#define KEY_SCAN     11
           "SCAN\0"  /* Scan the directory for a file pattern list    */
#define KEY_ADDR     12
           "ADDR\0"  /* Send a rexx command to a given port           */
#define KEY_SET      13
           "SET\0"   /* Set a symbol to a given value                 */
#define KEY_CONFIG   14
           "CONFIG\0"/* Change a config string                        */
#define KEY_CALL     15
           "CALL\0"  /* Execute a config string                       */
#define KEY_SELECT   16
           "SELECT\0"/* Select an element in the file list            */
#define KEY_ADD      17
           "ADD\0"   /* Add an element to the list                    */
#define KEY_DEL      18
           "DEL\0"   /* Delete the current item from the list         */
#define KEY_LTOP     19
           "LTOP\0"  /* Select top item in list                       */
#define KEY_LBOT     20
           "LBOT\0"  /* Select bottom item in list                    */
#define KEY_LUP      21
           "LUP\0"   /* Select next item in list                      */
#define KEY_LDN      22
           "LDN\0"   /* Select previous item in list                  */
#define KEY_RECFG    23
           "RECFG\0" /* Read in a config file                         */
#define KEY_REXXI    24
           "REXXI\0" /* Set REXX mode to interactive                  */
;

/***********************************************************************************
 * Procedure: scan_dir
 * Synopsis:  (void)scan_dir();
 * Purpose:   Scan the working directory for all potential files
 ***********************************************************************************/
void scan_dir(char *pat)
{
   FILE *fp;
   char buf[32];

   if (!global.filelist) return;

   if (!*pat) pat = global.text[CONFIG_FILES];

   exec_command("LIST >RAM:TEMPFILE \"%r\" LFORMAT=\"%%s%%s\"", pat);
   if ((fp = fopen("RAM:TEMPFILE", "r")) != NULL)
   {
      while(fgets(buf, 64, fp) != NULL)
      {
         int len;
         len = strlen(buf) - 1;  /* Get rid of the \n */
         while(len && (buf[len-1] == ' ')) len--;

         buf[len] = 0; /* Make sure we null terminate it */
         add_listitem(global.filelist, buf, 0, 1);
      }
      fclose(fp);
   }
}

/***********************************************************************************
 * Procedure: do_command
 * Synopsis:  do_command(string);
 * Purpose:   Execute the command associated with a given string
 ***********************************************************************************/
void do_command(char *string)
{
   char *p;
   int  cmdnum;
   int  postlog;
   char *callstr;
   int  lclass;

   /* EDIT                - Invoke the editor on the current file         */
   /* EXEC cmd            - Execute the given command                     */
   /* NEW     [ file | ?] - RESET, Prompt for a new file name             */
   /* QUIT                - Exit                                          */
   /* READ    [ file | ?] - Read a file                                   */
   /* RUN                 - Execute program in current mode               */
   /* SAVE    [ file | ?] - Save file                                     */
   /* FRONT               - Pop Window to the front                       */
   /* BACK                - Send Window to the back                       */
   /* JUMP [screen]       - Jump Window to a given screen                 */
   /* LOG  [string]       - Output a long message to the screen           */

   set_workdir();
   set_busy();
   callstr = NULL;
   postlog = 0;

   while (*string)
   {
      int c;

      cmdnum = lookup_keyword(&string, cmdlist);

      /* Skip over the name to the rest of the command */
      p = string;

      /* Locate any trailing semi-colon so we can have multiple commands */
      while(*string && (*string++ != ';'))
         ;
      if ((c = string[-1]) == ';')
         string[-1] = 0;


      switch(cmdnum)
      {
         case KEY_TRACE: /*             - Toggle the trace state */
            showcmd = !showcmd;
            break;
         case KEY_EXEC:  /* cmd         - Execute a given command               */
            InitSession();
            if (save_current(0)) break;
            exec_command(p, get_cursel());
            break;
         case KEY_NEW:   /* [ file | ?] - RESET, Prompt for a new file name     */
            {
               char *ext;
               int  len, elen;
               char buf[MAX_FILENAME+1];

               global.rexxrc = TEXT_BADPROJ; /* unless another error occurs */
               if (save_current(1)) break;
               if (!get_filename(p, buf, FALSE)) break;

               /* Now we need to see if the project already exists and warn them */
               /* that they will be overwriting it.                              */
               {
                  BPTR lock;
                  lock = Lock(buf, ACCESS_READ);
                  if (lock != NULL)
                  {
                     UnLock(lock);
                     if (!request(0, TEXT_ASKKILL, buf, NULL))
                     {
                        break;
                     }
                  }
               }

               /* no errors, no cancellations */
               global.rexxrc = 0;
               commit_filename(buf);
               set_gadgets(0);
               reset_options();
               /* indicate that a new icon will be required                */
               global.oldproject = 0;
               /* really trying to load a project, need to fix up ghosting */
               global.unghost = 1;

               /* If they didn't put an extension on the end of it, we want to     */
               /* automatically do that for them.  We should also seed the default */
               /* project name for them.                                           */
               p = Sym_Lookup(SYM_SCRIPT);
               ext = global.text[CONFIG_EXT];
               elen = strlen(ext);
               len = strlen(p) - elen;

               /* If the name is shorter than the extension OR it doesn't end with */
               /* the extension, then we will put the extension on for them        */
               if ((len < 0) || stricmp(p+len, ext))
               {
                  Sym_Set("PROJECT", p, NULL);
                  Sym_Set(SYM_SCRIPT, NULL, ext);
                  strcpy(global.filename, Sym_Lookup(SYM_SCRIPT));
               }
               else
               {
                  p[len] = 0;
                  Sym_Set("PROJECT", p, NULL);
                  p[len] = *ext;
               }
               Sym_Set("TYPE", "Normal", NULL);
               Sym_Set("CFLAGS", "-R -// -f -2.0 -d1", NULL);

               unload_symbols();
               set_gadgets(1);
            }
            break;
         case KEY_QUIT:  /*             - Exit                                  */
            if (!save_current(1))
               global.done = 1;
            break;
         case KEY_EDIT:  /*             - Invoke the editor on the current file */
            global.rexxrc = TEXT_NOSEL; /* default error for interactive REXX */
            if (save_current(0)) break;
            {
               struct stat stat_buf;
               char *sel;

               /* If the file is read-only, we want to see if we can check it out  */
               p = global.text[CONFIG_CO];
               sel = get_cursel();
               if (sel && (stat(sel, &stat_buf) >= 0))
               {
                  if (!(stat_buf.st_mode & S_IWRITE))
                  {
                     int rc;

                     /* The file exists, but is read-only.  See if they want us */
                     /* to check it out for them to work on                     */
                     rc = request(0, TEXT_ASKCO, sel, global.text[TEXT_CO]);
                     if (rc == 0) break;  /* Don't do the edit */
                     if (rc == 2)
                     {
                        exec_command(global.text[CONFIG_CO], sel);
                     }
                  }
               }
               /* not aborting, can't be any errors... */
               global.rexxrc = 0;
               /* if EDPROJ config string is filled, we can do special      */
               /* handling for project file.                                */
               if ((global.text[CONFIG_EDPROJ] == 0) || 
                   (stricmp(sel, Sym_Lookup(SYM_SCRIPT))))
               {
                  exec_command(global.text[CONFIG_EDIT], sel);
                  break;
               }
               else
               {
                  exec_command(global.text[CONFIG_EDPROJ], sel);
                 if (sel && (stat(sel, &stat_buf) < 0))
                 {
                    /* Don't try to read if it doesn't exist */
                    break;
                 }
               }
            }
            /* Fall through to READ to use modified project file             */
            p = ""; /* READ will default to default to current project       */
         case KEY_READ:  /* [ file | ?] - Read a file                        */
            {
               char *symcfg, *symnew;
               char rbuff[200];
               char buf[MAX_FILENAME+1];
               struct stat stat_buf;

               global.rexxrc = TEXT_BADPROJ; /* just in case... */
               if (save_current(1)) break;
               if (build_command(rbuff, 200, p, get_cursel())) break;

               p = rbuff;
               if (!get_filename(p, buf, FALSE)) break;
               if (*buf && (stat(buf, &stat_buf) < 0))
               {
                  request(1, TEXT_BADFILE, buf, NULL);
                  break;
               }
               commit_filename(buf);
               set_gadgets(0);
               reset_options();
               if (read_file())
               {
                  /* For some reason we couldn't read the file,       */
                  /* just reset all the options to be a null value.   */
                  reset_options();
                  commit_filename("");
               }
               else
               {
                  global.rexxrc = 0;     /* Everything's hunky dory now */
                  /* we're really trying to do it, lock all the gadgets */
                  global.unghost = 1;
               }
               global.oldproject = 1;

               /* now see if we need to change the config file          */
               symcfg = Sym_Lookup(SYM_CONFIG);
               symnew = Sym_Lookup(SYM_PROJCFG);
               /* if the project doesn't specify a config file          */
               if (!symnew || (symnew[0] == '\0'))
                  /* revert to original default                         */
                  symnew = Sym_Lookup(SYM_ORIG_CFG);
               /* if configuration file name has changed                */
               if (stricmp(symcfg, symnew))
               {
                  Sym_Set(SYM_CONFIG, symnew, 0);
                  global.newscreen = 1; /* configuration redefines window */
                  global.parsefail = parse_config(symnew);
                  if (global.parsefail)
                  {
                     global.rexxrc = TEXT_BADPROJ;
                     break;   /* error display in title bar             */
                  }
               }

               unload_symbols();
               /* If we're about to redisplay the window, defer         */
               /* repainting the gadgets until then                     */
               if (!global.newscreen)
                  set_gadgets(1);
               break;
            }
         case KEY_SAVE:  /* [ file | ?] - Save file                     */
            {
               int  changed;
               char *ext;
               int  len, elen;
               char svproj[MAX_FILENAME+1];
               char newproj[MAX_FILENAME+1];

               global.rexxrc = TEXT_BADPROJ; /* in case of unnamed perils */
               /* save project name in case they change it */
               strncpy(svproj, Sym_Lookup(SYM_SCRIPT), MAX_FILENAME);
               svproj[MAX_FILENAME] = '\0';
               if (!get_filename(p, newproj, TRUE)) 
                  break;
               /* Now see if they have chosen a new name that already exists */
               /* and warn them that they will be overwriting it.            */
               changed = stricmp(svproj, newproj);
               if (changed)
               {
                  BPTR lock;

                  lock = Lock(newproj, ACCESS_READ);
                  if (lock != NULL)
                  {
                     UnLock(lock);
                     if (!request(0, TEXT_ASKKILL, newproj, NULL))
                     {
                        break;
                     }
                  }
                  /* indicate that a new icon will be required               */
                  global.oldproject = 0;
               }

               /* looks like they want to go through with this               */
               global.rexxrc = 0;  /* didn't fail, don't lie to rexx         */

               /* If they didn't put an extension on the end of it, we want  */
               /* to automatically do that for them.                         */
               ext = global.text[CONFIG_EXT];
               len = strlen(newproj);
               elen = strlen(ext);

               /* If the name is shorter than the extension OR it doesn't end with */
               /* the extension, then we will put the extension on for them        */
               if ((len < elen) || stricmp(newproj+len-elen, ext))
               {
                  strncat(newproj, ext, MAX_FILENAME-len);
                  newproj[MAX_FILENAME] = 0;
               }
               commit_filename(newproj);

               (void)test_dirty();
               load_symbols();
               if (!write_file())
               {
                  /* JAT's usual negative logic -- means it worked           */
                  if (changed)
                  {
                     /* Select the old project.                              */
                     handle_list(global.filelist, NULL, CLASS_SELECT, svproj, 0);
                     if (global.filelist->sel != NULL) /* we got it */
                     {
                        /* If it was in the list, delete it                  */
                        handle_list(global.filelist, NULL, CLASS_DEL, NULL, 0);
                     }
                     /* Now put the new project at the end or the list       */
                     set_gadgets(0);
                     add_listitem(global.filelist, newproj, 1, 0);
                     set_gadgets(1);
                  }
                  /* Now the current state is saved in the new project file. */
                  mark_clean();
               }

               break;
            }
         case KEY_ADDR:  /* port cmd    - Send an arexx command to a given port */
            InitSession();
            if (save_current(0)) break;
            send_rexx(p);
            break;
         case KEY_FRONT: /*             - Pop Window to the front               */
            WindowToFront(global.window);
            ScreenToFront(global.window->WScreen);
            ActivateWindow(global.window);
            break;
         case KEY_BACK:  /*             - Send Window to the back               */
            WindowToBack(global.window);
            break;
         case KEY_JUMP:  /* [screen]    - Jump Window to a given screen         */
            /* This should only be done for 2.0..... */
            if (AslBase)
            {
               char buf[MAXPUBSCREENNAME+1];
               if (*p == 0)
               {
                  if (NextPubScreen(global.window->WScreen, buf))
                     p = buf;
               }
               if (p)
                  global.screen = LockPubScreen(p);
               else
                  global.screen = 0;
               /* force main event loop to close & reopen the window */
               global.newscreen = 1;
               break;
            }
            request(1, TEXT_BADCMD, "--- JUMP Dos 1.3---", NULL);
            break;
         case KEY_RECFG:
            if (save_current(0) == 0) /* didn't change their mind? */
            {
               char cfname[MAX_FILENAME+1];
               char *cfp = p;

               if (*cfp == 0)
                  if (get_work_filename(cfname, CONFIG_CFGFPAT))
                     cfp = cfname;

               if (*cfp != 0) /* got a name from somewhere */
               {
                  Sym_Set(SYM_CONFIG, cfp, 0);
                  global.newscreen = 1; /* configuration redefines window */
                  set_gadgets(0);
                  load_symbols();
                  global.parsefail = parse_config(cfp);
                  if (!global.parsefail)
                  {
                     unload_symbols();
                     Sym_Set(SYM_PROJCFG, cfp, 0);
                     global.dirtysym = DOSTRUE;
                  }
               }
            }
            break;
         case KEY_LOG:   /* message     - Output message to the log             */
            log_message("\n---------------------------", 0);
            log_message(p, global.rexxmsgs);
            postlog = 1;
            break;
         case KEY_SCAN:  /*             - Scan for a list of files              */
            set_gadgets(0);
            scan_dir(p);
            set_gadgets(1);
            break;
         case KEY_SET:
            {
               char buf[MAX_SYMBOL+1];

               p = parse_keyword(p, buf);
               if (buf[0] == 0)
               {
                  request(1, TEXT_BADPARM, NULL, NULL);
                  break;                  
               }
               Sym_Set(buf, p, NULL);
               if (buf[0] != '_')
                  global.dirtysym = DOSTRUE; /* to flag save_current()  */
            }
            break;
         case KEY_CONFIG:
            {
               char buf[MAX_SYMBOL+1];

               p = parse_keyword(p, buf);

               cmdnum = atoi(buf);
               if ((cmdnum > 0) && (cmdnum <= NUM_CONFIG))
               {
                  if (global.text[CONFIG_BASE-1+cmdnum])
                     free(global.text[CONFIG_BASE-1+cmdnum]);
                  global.text[CONFIG_BASE-1+cmdnum] = strdup(p);
               }
               else
                  request(1, TEXT_BADPARM, NULL, NULL);
            }
            break;
         case KEY_SELECT:
            /* Select the given name. */
            handle_list(global.filelist, NULL, CLASS_SELECT, p, 0);
            if (global.filelist->sel == NULL) /* we didn't select anything */
               if (*p != '\0') /* and we weren't just trying to deselect   */
                  request(1, TEXT_SELFAIL, p, NULL);
            break;
         case KEY_LTOP:
            lclass = CLASS_LTOP;
            goto listmove;
         case KEY_LBOT:
            lclass = CLASS_LBOT;
            goto listmove;
         case KEY_LUP:
            lclass = CLASS_LUP;
            goto listmove;
         case KEY_LDN:
            lclass = CLASS_LDN;
      listmove:
            handle_list(global.filelist, NULL, lclass, p, 0);
            if (global.inrexx) /* don't nag but do rexx error codes        */ 
               if (global.filelist->sel == NULL) /* didn't select anything */
                  request(1, TEXT_SELFAIL, p, NULL);
            break;
         case KEY_ADD:
            /* add an item to the file list, can specify name with command */
            handle_list(global.filelist, NULL, CLASS_ADD, p, 0);
            break;
         case KEY_DEL:
            /* Delete the current selection from the file list             */
            if (global.inrexx) /* error msg helpful in rexx, else irritating */
            { 
               if (global.filelist->sel == NULL) /* can't delete nothing   */
               {
                  request(1, TEXT_NOSEL, NULL, NULL);
                  break; /* so don't call delete routine                   */
               }
               if (*p != '\0') /* DEl doesn't take parameters              */
               {
                  request(1, TEXT_BADPARM, NULL, NULL);
                  break; /* so don't call delete routine                   */
               }
            }
            handle_list(global.filelist, NULL, CLASS_DEL, p, 0);
            break;
         case KEY_REXXI:
            /* set REXX interactive mode on or off                         */
            if (!stricmp(p, "on"))
            {
               /* Interactive, allows requesters */
               global.rexxinter = 1;
               Sym_Set(SYM_REXXINTER, "ON", NULL);
            }
            else
            {
               if (!stricmp(p, "off"))
               {
                  /* set REXX mode to fail instead of displaying requesters */
                  global.rexxinter = 0;
                  Sym_Set(SYM_REXXINTER, "OFF", NULL);
               }
               else 
               {
                  request(1, TEXT_BADRMODE, p, NULL);
               }
            }
            break;
         case KEY_CALL:
            if (callstr == NULL)
            {
               char buf[MAX_SYMBOL+1];

               p = parse_keyword(p, buf);

               cmdnum = atoi(buf);
               if ((cmdnum > 0)           &&
                   (cmdnum <= NUM_SUBRTN) &&
                   ((p = global.text[cmdnum+SUBRTN_BASE-1]) != NULL) &&
                   *p)
               {
                  string[-1] = c;
                  callstr = string;
                  string = p;
                  continue;
               }
               break;
            }
         default:
            /* We have an invalid option, ignore the command and let them know  */
            /* about the problem with the option.                               */
            request(1, TEXT_BADCMD, p, NULL);
            /* We also don't want to process any more commands...               */
            string[-1] = c;
            goto doneall;
      }
      string[-1] = c;
      if ((*string == '\0') && (callstr != NULL))
      {
         string = callstr;
         callstr = NULL;
      }
   }

doneall:
   if (postlog)
   {
      log_message("---------------------------", 0);
      if (!global.inrexx)
         ActivateWindow(global.window);
   }
   set_idle();
}

/***********************************************************************************
 * Procedure: reset_options
 * Synopsis:  reset_options()
 * Purpose:   Reset all the options to the default values
 ***********************************************************************************/
void reset_options(void)
{
#define object_list  ((struct G_LIST *)object)
#define object_str   ((struct G_STRING *)object)
#define object_cycle ((struct G_CYCLE *)object)
   struct G_OBJECT *object;
   char *p;

   /* First make sure that all the right symbols are in the symbol table in the */
   /* correct order.  Since the symbol table routines guarentee that it will    */
   /* keep the order of the entries once they are there, we only need to touch  */
   /* the order once to keep it correct for the dmakefile.                      */
   Sym_Clear();

   p = "PROJECT\0"
       "DIR\0"
       "SRCS\0"
       "HDRS\0"
       "EXTRAS\0"
       "PDEFAULT\0"
       "EXEDIR\0"
       "OD\0"
       "PRECOMP\0"
       "TYPE\0"
       "RUN\0"
       "CLIARGS\0"
       "CFLAGS\0";

   while(*p)
   {
      Sym_Set(p, NULL, "");
      p += strlen(p)+1;
   }

   object = global.objects;

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
               object_list->sel = NULL;
               object_list->maxent = 0;

               while(ent != NULL)
               {
                  struct G_ENTRY *nextent;

                  nextent = (struct G_ENTRY *)ent->base.next;
                  free_mem(ent, sizeof(struct G_ENTRY));
                  ent = nextent;
               }
            }
            break;
         case CLASS_BUTTON:
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
   mark_clean();

   UnLock(global.workdir);
   global.workdir = 0;
}

/***********************************************************************************
 * Procedure: unload_symbols
 * Synopsis:  unload_symbols();
 * Purpose:   Unload the symbols into the objects
 ***********************************************************************************/
void unload_symbols()
{
   struct G_OBJECT *object, *savelist;
   char *p;

#define object_list  ((struct G_LIST   *)object)
#define object_str   ((struct G_STRING *)object)
#define object_check ((struct G_CHECK  *)object)
#define object_cycle ((struct G_CYCLE  *)object)

   for(object = global.objects; object != NULL; object = object->next)
   {
      switch(object->class)
      {
         case CLASS_STRING:
            strcpy(object_str->buf, Sym_Lookup(object_str->option));
            break;
         case CLASS_CYCLE:
            {
               char buf[MAX_SYMBOL+1];
               struct G_VALUE *val;

               for(val = object_cycle->values;
                   val != NULL;
                   val = (struct G_VALUE *)val->next)
               {
                  char *arg;
                  /* Gather the current name into a buffer */
                  p = parse_keyword(val->option, buf);
                  arg = Sym_Lookup(buf);
                  if (val->string)
                  {
                     /* If the next character is a % sign, we have an exact */
                     /* match that we can just fill in.   Otherwise it is   */
                     /* the name of a string that we should lookup for the  */
                     /* final symbol to fill it in with                     */
                     if (*p != '%')
                     {
                        /* Parse out the next token - this is the secondary */
                        /* Mode.  This must match the value of the primary  */
                        /* option if we are to take it                      */
                        parse_keyword(p, buf);
                        if (strcmp(buf, arg)) continue;  /* Sorry, try the next one */

                        /* We have a match, look up the substitution value  */
                        /* for the string buffer                            */
                        arg = Sym_Lookup(buf);
                     }
                     /* Copy the string into the object cycle buffer */
                     strcpy(val->string->buf, arg);
                     object_cycle->curval = val;
                     break;
                  }
                  else
                  {
                     /* Not a string entry.  It must match exactly */
                     if (!strcmp(p, arg))
                     {
                        object_cycle->curval = val;
                        break;
                     }
                  }
               }
            }
            break;
         case CLASS_LIST:
            if (!stricmp(object_list->option, "Files"))
            {
               savelist = object;
               /* Populate it with all the strings from the list */
               add_list(object_list, Sym_Lookup("SRCS"),      0);
               add_list(object_list, Sym_Lookup("HDRS"),      0);
               add_list(object_list, Sym_Lookup("EXTRAS"),    0);
               add_list(object_list, Sym_Lookup(SYM_SCRIPT),  1);
            }
            else
            {
               add_list(object_list, Sym_Lookup(object_list->option), 0);
            }
            break;
      }
   }

   mark_clean();
#undef object_check
#undef object_list
#undef object_str
#undef object_cycle
}

/***********************************************************************************
 * Procedure: load_symbols
 * Synopsis:  load_symbols();
 * Purpose:   load the symbols from the current objects
 ***********************************************************************************/
void load_symbols()
{
   struct G_OBJECT *object;
   char *p, *arg;

#define object_list  ((struct G_LIST   *)object)
#define object_str   ((struct G_STRING *)object)
#define object_check ((struct G_CHECK  *)object)
#define object_cycle ((struct G_CYCLE  *)object)

   for(object = global.objects; object != NULL; object = object->next)
   {
      /* Don't even bother with this object if it hasn't changed */
      if (!object->state & DIRTY_BIT) continue;

      switch(object->class)
      {
         case CLASS_STRING:
            Sym_Set(object_str->option, object_str->buf, NULL);
            break;

         case CLASS_CYCLE:
            {
               struct G_VALUE *val;
               char buf[MAX_SYMBOL+1];
               char sec[MAX_SYMBOL+1];

               val = object_cycle->curval;
               p = parse_keyword(val->option, buf);

               if (val->string)
               {
                  /* If the next token is a '%', we just use the string buffer */
                  /* as the result value                                       */
                  if (*p == '%')
                  {
                     p = val->string->buf;
                  }
                  else
                  {
                     /* Special case of a cascade definition */
                     /* We need to set the primary definition to the secondary */
                     /* name and the secondary definition to the string        */
                     parse_keyword(p, sec);
                     Sym_Set(sec, val->string->buf, NULL);
                     p = sec;
                  }
               }
               Sym_Set(buf, p, NULL);

               /* Handle the one special case that we know about:  */
               /*   TYPE <whatever> causes us to read in a new script */
               if (!strcmp(buf, "TYPE"))
                  read_script();
            }
            break;

         case CLASS_LIST:
            {
               int firsttime;
               struct G_ENTRY *ent;
               Sym_Set("EXTRAS", "", NULL);
               Sym_Set("SRCS",   "", NULL);
               Sym_Set("HDRS",   "", NULL);
               /* Files                                                                           */
               /*  - List of C and O files.  Always relative to the current directory             */
               /*  - Generates the SRCS and HDRS directories by splitting out based on extension. */
               /*  - Things ending in .C or .A go into SRCS                                       */
               /*  - Things ending in .H go into HDRS                                             */
               /*  - All others go into FILES_ext where ext is the extension of the file          */
               for(firsttime = 1, ent = object_list->first; ent != NULL;
                   ent = (struct G_ENTRY *)ent->base.next, firsttime = 0)
               {
                  arg = "EXTRAS";
                  p = strrchr(ent->buf, '.');
                  if (p != NULL)
                  {
                     if ((!stricmp(p, ".c")) || (!stricmp(p, ".a"))) arg = "SRCS";
                     else if (!stricmp(p, ".h"))
                     {
                        char *t;
                        t = strrchr(ent->buf, '/');
                        if (t == NULL) t = strrchr(ent->buf, ':');
                        if (t == NULL) t = ent->buf;
                        arg = "HDRS";
                        if (p > t)
                        {
                           *p = 0;
                           if (!stricmp(t, Sym_Lookup("PROJECT")))
                           {
                              *p = '.';
                              Sym_Set("PDEFAULT", ent->buf, NULL);
                           }
                           *p = '.';
                        }
                     }
                  }
                  /* Make sure that we don't put the script in there more than */
                  /* once......  It is already present as the DMAKEFILE one    */
                  if (!(ent->base.state & STATE_MASK))
                  {
                     if (stricmp(object_list->option, "Files"))
                        arg = object_list->option;

                     if (*Sym_Lookup(arg)) Sym_Set(arg, NULL, " ");

                     /* If the filename has spaces, we need to put it in quotes */
                     if (strchr(ent->buf, ' '))
                     {
                        Sym_Set(arg, NULL, "\'");
                        Sym_Set(arg, NULL, ent->buf);
                        Sym_Set(arg, NULL, "\'");
                     }
                     else
                     {
                        Sym_Set(arg, NULL, ent->buf);
                     }
                  }
               }
            }
            break;
      }
   }

#undef object_check
#undef object_list
#undef object_str
#undef object_cycle

   /* A couple of symbols MUST be a directory with the appropriate */
   /* Directory separator character on the end of it               */
   /*   DIR                                                        */
   /*   EXEDIR                                                     */
   /*   OD                                                         */
   p = "DIR\0"
       "EXEDIR\0"
       "OD\0";
   for (;*p; p += strlen(p)+1)
   {
      char *val;
      int len;

      val = Sym_Lookup(p);

      len = strlen(val);
      if (len && (val[len-1] != ':') && (val[len-1] != '/'))
         Sym_Set(p, NULL, "/");
   }
}

/***********************************************************************************
 * Procedure: add_list
 * Synopsis:  add_list(list, name)
 * Purpose:   Add all the filenames in the named symbol table into the list object
 ***********************************************************************************/
void add_list(struct G_LIST *list,
              char *name,
              int private)
{
   char buf[MAX_STRING+1];
   int pos, c, state;

   pos = 0;
   state = 0;

   do
   {
      c = *name++;
      if (((state == 1) && (c == '\'')) || (pos && (c == ' ' || c == 0)))
      {
         buf[pos] = 0;
         add_listitem(list, buf, private, 0);
         state = 0;
         pos = 0;
      }
      else
      {
         if (c == '\'')     state = 1;
         else if (state || (c != ' ')) buf[pos++] = c;
         if (pos == MAX_STRING) pos = MAX_STRING-1;
      }
   } while(c);
}

/***********************************************************************************
 * Procedure: add_listitem
 * Synopsis:  add_listitem(list, name, private, dirty)
 * Purpose:   Add the given name into the current list
 ***********************************************************************************/
void add_listitem(struct G_LIST *list, char *name, int private, int dirty)
{
   struct G_ENTRY *ent, *prevent;

   ent = get_mem(sizeof(struct G_ENTRY));
   if (ent == NULL) return;

   strcpy(ent->buf, name);
   ent->base.state = private;
   if (dirty)
      list->base.state |= DIRTY_BIT;

   /* Figure out where to add the name */

   /* link the new name into the list */
   if (list->first == NULL)
   {
      list->top = list->first = ent;
   }
   else
   {
      int i;
      for (i = 0, prevent = list->first; prevent->base.next;
           prevent = (struct G_ENTRY *)prevent->base.next, i++)
      {
         /* Make sure it isn't already in the list */
         if (!stricmp(prevent->buf, name))
         {
            free_mem(ent, sizeof(struct G_ENTRY));
            return;
         }
      }
      prevent->base.next = (struct G_OBJECT *)ent;
      ent->base.prev = (struct G_OBJECT *)prevent;
   }
}