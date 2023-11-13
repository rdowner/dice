/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include "vmake.h"
#include "vmake_rev.h"

Prototype int CreateWindow(void);
Prototype void DestroyWindow(void);
Prototype char *SkipAss(const char *);

void myexit(void);
int main(int, char **);

struct GLOBAL global;
__aligned struct FileInfoBlock vmfib;

struct Library *GadToolsBase;
struct Library *AslBase;
struct Library *WorkbenchBase;
struct Library *ArpBase;

char    *InitialFileName;
char    *ConfigName;
short   StartFromWB;
short   BuildIt;
char    *RexxScript;
char    Buf[256];

#define TEMPLATE "PROJECT,REXX/K,CONFIG/K,BUILD/S"

ULONG oldSeconds, oldMicros; /* for double click filtering */

#define VM_PROJ        0
#define VM_RSCRIPT     VM_PROJ+1
#define VM_CONFIG      VM_RSCRIPT+1
#define VM_BUILD       VM_CONFIG+1
#define OPT_COUNT      VM_BUILD+1

long vm_opts[OPT_COUNT];

char *RexxHostName = "VMAKE" VERSTAG;

/***********************************************************************************
 * Procedure: wbmain
 * Synopsis:  rc = wbmain(WBStartup);
 * Purpose:   Performs appropriate initialization to run program from Workbench
 * Notes:     Parse tooltypes as a command.  Note that we are CD'd into the
 *            application's directory, not the tooltype's icon's dir.
 ***********************************************************************************/
int wbmain(wbs)
struct WBStartup *wbs;
{
   struct DiskObject *dob;
   short i;
   short j;
   short FileSpecified = 0;
   short abortIt = 0;

   /*
    *  Search for options, set current directory to last valid
    *  disk object.  Run commands specified by tool types in order
    */

   for (i = 0; i < wbs->sm_NumArgs; ++i)
   {
      BPTR saveLock = CurrentDir((BPTR)wbs->sm_ArgList[i].wa_Lock);

      if (i == wbs->sm_NumArgs - 1 && FileSpecified == 0)
      {
         InitialFileName = strdup(wbs->sm_ArgList[i].wa_Name);
      }

      if (dob = GetDiskObject(wbs->sm_ArgList[i].wa_Name))
      {
         if (dob->do_ToolTypes)
         {
            for (j = 0; dob->do_ToolTypes[j]; ++j)
            {
               char *ptr = dob->do_ToolTypes[j];

               if (strnicmp(ptr, "FILE=", 5) == 0)
               {
                  InitialFileName = strdup(SkipAss(ptr));
                  FileSpecified = 1;
               }
               else
               {
                  if (strnicmp(ptr, "REXX=", 5) == 0)
                  {
                     RexxScript = strdup(SkipAss(ptr));
                  }
                  else
                  {
                     if (strnicmp(ptr, "CONFIG=", 7) == 0)
                        ConfigName = strdup(SkipAss(ptr));
                     else
                     {
                        if (strnicmp(ptr, "BUILD", 5) == 0)
                           BuildIt = 1;
                        else
                        {
                           /* These strings are needed for the requester      */
                           global.text[TEXT_OK] = "OK";
                           global.text[TEXT_CANCEL] = "Cancel";
                           /* OK to use any text slot - will get overlaid     */
                           global.text[1] = "Bad ToolType";
                           /* Unfortunately the dos 1.3 version of request()  */
                           /* dies of a contagious plague...                  */
                           if ((DOSBase->dl_lib.lib_Version >= 36) &&
                               !request(0, 1, ptr, NULL))
                           {
                              abortIt = 1;
                              break;
                           }
                        }
                     }
                  }
               }
            }
         }
         FreeDiskObject(dob);
      }
      CurrentDir(saveLock);
      if (abortIt)
          break;
   }

   if (abortIt == 0)
   {
      go_dir(wbs->sm_ArgList[wbs->sm_NumArgs-1].wa_Lock);
      setup_cli();
      StartFromWB = 1;
      main(1, NULL);
   }
}

/***********************************************************************************
 * Procedure: main
 * Synopsis:  rc = main(argc, argv)
 * Purpose:   The main entry point for the program
 ***********************************************************************************/
int main(int argc, char **argv)
{
   struct IntuiMessage *msg;
   struct RDargs *rdargs;

   if (!StartFromWB)
   {
      if (DOSBase->dl_lib.lib_Version >= 36)
      {
         rdargs = ReadArgs(TEMPLATE, vm_opts, NULL);
         if (rdargs == NULL)
            PrintFault(IoErr(), NULL);
         else
         {           
            InitialFileName = (char *)vm_opts[VM_PROJ];
            RexxScript = (char *)vm_opts[VM_RSCRIPT];
            ConfigName = (char *)vm_opts[VM_CONFIG];
            BuildIt = vm_opts[VM_BUILD];
         }
      }
      else
      {
         if (argc != 1)
         {
            InitialFileName = argv[1];
         }
      }
   }

   /*
    *   Open GadToolsBase/AslBase manually so we don't exit if they
    *   don't exist.
    */

   GadToolsBase  = OpenLibrary("gadtools.library", 0);
   AslBase       = OpenLibrary("asl.library", 0);
   WorkbenchBase = OpenLibrary("workbench.library", 0);

   memset(&global, 0, sizeof(global));

   if (AslBase != NULL)
   /* Amigados version 2.0 or greater */
   {
      global.cbufsize = CBUF_SIZE_20; /* Long command lines */
      global.freq = AllocFileRequest();
   }
   else
   /* Amigados version 1.3 */
   {
      global.cbufsize = CBUF_SIZE_13; /* Short command lines */
      ArpBase = OpenLibrary("arp.library", 0);
      if (ArpBase != NULL)
         global.freq = ArpAllocFreq();
   }

   global.cbuf = malloc(global.cbufsize+1);
   global.rexxrs = malloc(global.cbufsize+1);
   if ((global.cbuf == NULL) || (global.rexxrs == NULL))
      return(20); /* not enough memory to run & no good way to say why not */

   if (ConfigName)
   {
      global.parsefail = parse_config(ConfigName);
      if (!global.parsefail)
         Sym_Set(SYM_CONFIG, ConfigName, NULL);
   }
   else
   {
      global.parsefail = parse_config(ALT_CONFIG_FILE);
      if (!global.parsefail)
         Sym_Set(SYM_CONFIG, ALT_CONFIG_FILE, NULL);
      else
      {
         if (global.parsefail == 2)
         {
            global.parsefail = parse_config(CONFIG_FILE);
            if (!global.parsefail)
               Sym_Set(SYM_CONFIG, CONFIG_FILE, NULL);
         }
      }
   }

   if (!global.parsefail)
      strcpy(global.title, global.text[TEXT_NOPROJ]);
   Sym_Set(SYM_ORIG_CFG, Sym_Lookup(SYM_CONFIG), 0);

   {
      /* We need to make the rexx port name available for them to use */
      char *pname;
      GetDiceRexxPortSlot(NULL, &pname);
      if (pname != NULL)
         Sym_Set(SYM_REXXPORT, pname, NULL);
   }
   /* REXX mode is initially non-interactive */
   global.rexxinter = 0;
   Sym_Set(SYM_REXXINTER, "OFF", NULL);

while (!global.done)
{
   /* global.screen is initialized to NULL to open on workbench screen */
   /* stay in screen/window loop till reset by JUMP */
   global.newscreen = 0;

   if(global.parsefail)
   {
      memset(global.menuitem, 0, sizeof(global.menuitem));
      global.gadlist = NULL; /* doesn't happen when we freegadlist() */
   }

   FillIn_RenderInfo(&global.ri, global.screen);

   /* Provide a default height/width so that we will get any errors displayed well */
   global.width = global.ri.ScreenWidth-50;
   global.height = global.ri.WindowTitle;

   global.ri.WindowTitle  += MARGIN_TOP;
   global.ri.WindowLeft   += MARGIN_LEFT;
   global.ri.WindowRight  += MARGIN_RIGHT + RESIZE_WIDTH;
   global.ri.WindowBottom += MARGIN_BOTTOM;

   if (!global.parsefail)
      if (!init_gad_sizes(global.ri.ScreenWidth, global.ri.ScreenHeight,0))
      {
         init_gadgets();
      }

   if (!CreateWindow())
   {
      struct AppWindow *appwindow;
      struct MsgPort *appmsgport;
      struct AppMessage *appmsg;
      long   waitflags = 1 << global.window->UserPort->mp_SigBit;

      if (WorkbenchBase && (WorkbenchBase->lib_Version >= 36))
      {
         if (appmsgport = CreateMsgPort())
            appwindow = AddAppWindowA(
                           1,     /* userID */
                           0,     /* ptr to userdata, prototyped as long */
                           global.window, appmsgport, /*obvious*/
                           NULL   /* taglist */
                        );
         if (appwindow) waitflags |= 1 << appmsgport->mp_SigBit;
      }
      else
      {
         appwindow  = NULL;
         appmsgport = NULL;
      }

      waitflags |= 1 << RexxSigBit;

      set_gadgets(1);

      if (InitialFileName && !global.parsefail)
      {
         strcpy(global.filename, InitialFileName);
         do_command("READ");
      }

      /* ensure that console is available for REXX scripts etc. -- we're    */
      /* either first time in or jumping to a new screen                    */
      TermSession();
      InitSession();

      if (RexxScript && !global.parsefail)
      {
         char buf[256];
         sprintf(buf, "ADDR REXX %s %%(_PORT_) %s",
                      RexxScript, global.filename);
         do_command(buf);
      }

      /* If there's no project loaded now, we need to ghost most menu items */
      /* and cover the window so they can't play with the gadgets.          */
      if ((global.filename[0] == 0) && !global.parsefail)
      {
         ghost_menus();
         cover_window(); 
      }
      else   /* We have a project, do they want to build it right away?     */
         if (BuildIt && !global.parsefail)
            do_command(global.text[CONFIG_BUILD]);

      /* only use these on the first pass */ 
      InitialFileName = RexxScript = NULL;
      BuildIt = 0;

      while(!global.done  && !global.newscreen)
      {
         long mask;

         global.unghost = 0; /* will get set if we excute NEW or READ */

         mask = Wait(waitflags);
         while(msg = (struct IntuiMessage *)GetMsg(global.window->UserPort))
         {
            ULONG class;
            APTR iaddr;
            USHORT code;
            USHORT qual;
            ULONG  Seconds, Micros;
            struct XItem *xitem;

            code = msg->Code;
            class = msg->Class;
            iaddr = msg->IAddress;
            qual  = msg->Qualifier;
            Seconds = msg->Seconds;
            Micros = msg->Micros;

            ReplyMsg((struct Message *)msg);

            switch(class)
            {
               case CLOSEWINDOW:
                  do_command("QUIT");
                  break;
               case GADGETDOWN:
                  /* We will only get this message from s list slider gadget  */
                  /* but should really verify ... oh well                     */
                  global.mouseprop = 1; /* streamline prop gadget handling    */
                  ReportMouse(1, global.window);
                  break;
               case GADGETUP:
               {
                  int doubleclick;
                  
/* really want a reasonable way to initialize oldSecs/Micros for first call... */
                  if (class == GADGETUP)
                  {
                     doubleclick = DoubleClick(oldSeconds, oldMicros,
                                               Seconds, Micros);
                     oldSeconds = Seconds;
                     oldMicros = Micros;
                  }
                  handle_hit((struct Gadget *)iaddr,
                      (qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 0,
                      doubleclick);
                  /* restore this after mouse dragging stops to avoid an      */
                  /* extra screen flash.                                      */
                  global.mouseprop = 0; /* do gadget hits from now on */
                  ReportMouse(0, global.window);
                  break;
               }
               case NEWSIZE:
                  set_gadgets(0);
                  free_gadlist(global.gadlist);
                  init_gad_sizes(global.window->Width,
                                 global.window->Height, 1);
                  init_gadgets();
                  SetBPen(global.rp, 0);
                  SetAPen(global.rp, 0);
/* debug - original
                  RectFill( global.rp, global.ri.WindowLeft, global.ri.WindowTitle,
                         global.window->Width  - global.ri.WindowRight + MARGIN_RIGHT - 1,
                         global.window->Height - global.ri.WindowBottom + MARGIN_BOTTOM - 1);
*/
                  RectFill( global.rp, global.ri.WindowLeft - MARGIN_LEFT,
                         global.ri.WindowTitle,
                         global.window->Width  - global.ri.WindowRight + MARGIN_RIGHT - 1,
                         global.window->Height - global.ri.WindowBottom + MARGIN_BOTTOM - 1);
                  set_gadgets(1);
                  RefreshWindowFrame(global.window);
                  break;
               case MENUPICK:
                  while (code != MENUNULL)
                  {
                     xitem = (struct XItem *)
                             ItemAddress(global.menu, code);
                     do_command(xitem->userdata);
                     code = xitem->item.NextSelect;
                  }
                  break;
               case VANILLAKEY:
#ifdef DBG_JGM
                  printf("Key:%d '%c'\n", code, code);
switch (code)
{
case 'c':
   mark_clean();
   printf("set all objects clean\n");
   break;
case 'd':
   {
   int dirty;
   dirty = test_dirty();
   printf("Dirty state is %d\n", dirty);
   break;
   }
case 'g':
   ghost_menus();
   break;
case 'u':
   unghost_menus();
   break;
}
#endif

                  break;
               case MOUSEMOVE:
                  if (global.mouseprop == 1)
                     handle_list(global.filelist, global.filelist->slider, 
                                 CLASS_PROP, NULL, 0);
                  break;
/*
default:
   printf("Unknown IntuiMessage class %d\n", class);
   break;
*/
            }
         }

         if (appwindow && global.filelist)
         /* Must be Dos 2.0 or greater and must have list structure to put  */
         /* file names into                                                 */
         {
            char appwinbuff[256];

            while (appmsg = (struct AppMessage *)GetMsg(appmsgport))
            {
               int i;
               char *exttext = global.text[CONFIG_EXT];
               int extlen = strlen(exttext);
               int argnmln;
               struct WBArg *argptr;
               BYTE *argname;
               char *projname; /* point to proj.DICE if found */

               argptr = appmsg->am_ArgList;
               for (i = 0; i < appmsg->am_NumArgs; i++)
               {
                  int aproctype = 0;
                  /* takes on values: 0  no defined processing           */
                  /*                  1  add file name to list           */
                  /*                  2  read in a project script        */
                  /*                  3  go to dir - request script name */
                  projname = NULL;
                  argname = argptr->wa_Name;
                  if (*argname) /* argument is a file, not a directory */
                  {
                     aproctype = 1; /* simple file */
                     if ((argnmln = strlen(argname)) > extlen)
                        if (!stricmp(argname + argnmln - extlen, exttext))
                        {
                           aproctype = 2; /* read project file */
                           projname = (char *)argname;
                        }
                  }
                  else /* directory - see if we can find a default project */
                  {
                     aproctype = 3; /* simple directory */
                  }

                  if (aproctype >= 2) /* see if it's OK to change directory */
                  {
                     if (save_current(1) == 0)
                     {
                        BPTR dlock;
                        go_dir(argptr->wa_Lock);
                        if ((dlock = DupLock(argptr->wa_Lock)) != NULL)
                        {
                           UnLock(global.homedir);
                           global.homedir = dlock;
                        }
                        else
                           aproctype = 0; /* READ wouldn't work... */
                     }
                     else
                        /* give up on this one */
                        aproctype = 0;
                  }

                  if (aproctype == 3) /* see if there is a default project */
                  {
                     if (Examine(argptr->wa_Lock, &vmfib))
                     {
                        BPTR dlock;
                        strcpy(appwinbuff, vmfib.fib_FileName);
                        strcat(appwinbuff, exttext);
                        if (dlock = Lock(appwinbuff, SHARED_LOCK))
                        {
                           aproctype = 2; /* default project exists */
                           projname = appwinbuff;
                           UnLock(dlock);
                        }
                     }
                  }

                  /* now we've figure out how to process the object - do it */
                  switch (aproctype)
                  {
                     case 1: /* simple filename to add to list */
                     {
                        int match = 0; /* directory match ? */
                        int pdir = 0;  /* prepend directory name? */
                        int l;
                        char *p;
                        BPTR fplock = 0, wplock =0;

                        if (SameLock(global.workdir, argptr->wa_Lock)
                               == LOCK_SAME)
                           match = 1;
                        else
                        {
                           fplock = ParentDir(argptr->wa_Lock);
                           if (SameLock(fplock, global.workdir)
                                  == LOCK_SAME)
                           {
                              match = 2;
                              pdir = 1;
                           }
                           else
                           {
                              wplock = ParentDir(global.workdir);
                              if (SameLock(wplock, argptr->wa_Lock)
                                     == LOCK_SAME)
                                 match = 3;
                              else
                              {
                                 if (SameLock(wplock, fplock)
                                        == LOCK_SAME)
                                 {
                                    match = 4;
                                    pdir = 1;
                                 }
                              }
                           }
                        }

                        *appwinbuff = '\0';
                        if (match)
                        /* Match   Filename
                             0     <absolute path>/wa_Name
                             1     wa_Name
                             2     fparent/wa_Name
                             3     /wa_Name
                             4     /fparent/wa_Name
                        */
                        {
                           if (match > 2)  /* 3 or 4 */
                              strcpy(appwinbuff, "/");
                           if (pdir)       /* parent dir name required */
                           {
                              if (Examine(argptr->wa_Lock, &vmfib))
                              {
                                 strcat(appwinbuff, vmfib.fib_FileName);
                                 /* note: fplock is valid when pdir = 1 */
                                 if (fplock == 0) /* name is root */
                                    strcat(appwinbuff, ":");
                              }
                              else
                                 match = 0;  /* no path name - forget it */
                           }
                        }
                        else
                           if (NameFromLock(argptr->wa_Lock, appwinbuff, 255))
                              pdir = 1;  /* have path in appwinbuff now */

                        /* path (if any) in appwinbuff, name in wa_Name */
                        l = strlen(appwinbuff);
                        p = appwinbuff+l;

                        if (pdir)  /* parent dir name in appwinbuff */
                        /* append "/" if approriate */
                           if (*(p-1) != ':')
                           {
                              strcpy(p, "/");
                              p++;
                           }
                        if (strlen(argptr->wa_Name) < (255-l))
                           strcpy(p, argptr->wa_Name);
                        handle_list(global.filelist, NULL, CLASS_ADD, appwinbuff, 0);

                        if (fplock)
                           UnLock(fplock);
                        if (wplock)
                           UnLock(wplock);
                        break;

                     }
                     case 2: /* project script: proj.DICE */
                     {
                        char cmdbuff[266];

                        strcpy(cmdbuff, "READ ");
                        strcat(cmdbuff, projname);
                        do_command(cmdbuff);
                        break;
                     }
                     case 3: /* go to directory and prompt for file */
                     {
                        char cmdbuff[] = "READ ?";
                        do_command(cmdbuff);
                        break;
                     }
                  }
                  argptr++;
               }
               ReplyMsg((struct Message *)appmsg);
            }            
         }

         /* See if they want us to do any rexx commands */
         if (mask & (1 << RexxSigBit))
            ProcessRexxCommands(NULL);

         /* we had to defer this because it messes up no end if you try to */
         /* change the menus while processing a menupick intuimessage      */
         if (global.unghost) /* set by Read or New commands */ 
         {
            if (global.filename[0]) /* Read or New might have failed... */
            {  /* but they didn't */
               unghost_menus();
               uncover_window();
            }
            else
            {  /* or else they did */
               ghost_menus();
               cover_window();
            }
         }
      }
      if (appwindow)  RemoveAppWindow(appwindow);
      if (appmsgport) DeleteMsgPort(appmsgport);
      set_gadgets(0);
      free_gadlist(global.gadlist);
      DestroyWindow();
   }

   CleanUp_RenderInfo(&global.ri);
/* this is where the open/close window loop ends */

}
   UnLock(global.workdir);
   UnLock(global.homedir);

   if (global.freq && (AslBase != NULL)) FreeFileRequest(global.freq);

   if (GadToolsBase  != NULL) CloseLibrary(GadToolsBase);
   if (AslBase       != NULL) CloseLibrary(AslBase);
   if (ArpBase       != NULL) CloseLibrary(ArpBase);
   if (WorkbenchBase != NULL) CloseLibrary(WorkbenchBase);
   return(0);
}

/***********************************************************************************
 * Procedure: CreateWindow
 * Synopsis:  rc = CreateWindow();
 * Purpose:   Create the window on the current screen using the current environment
 ***********************************************************************************/
int CreateWindow()
{
   struct NewWindow nw;

   struct TagItem       wintags[] = {
      { WA_PubScreen,           (ULONG)global.screen },
          /* The screen we want to be on */
      { WA_PubScreenFallBack,   (ULONG)TRUE     }, /* Or workbench if not */
      { TAG_DONE,               (ULONG)0        }
   };


   memset(&nw, 0, sizeof(nw));

   nw.Width      = global.width;
   nw.Height     = global.height;
   nw.LeftEdge   = (global.ri.ScreenWidth  - nw.Width ) / 2;
   nw.TopEdge    = (global.ri.ScreenHeight - nw.Height) / 2;
   nw.MinWidth   = 4*BUTTON_WIDTH;
   nw.MinHeight  = 110;
   nw.MaxWidth   = 0xffff;
   nw.MaxHeight  = 0xffff;

   if ((nw.Width < 0) || (nw.Height < 0))
      return(1);
   nw.DetailPen  = nw.BlockPen = -1;
if (global.parsefail)
{
   nw.IDCMPFlags = CLOSEWINDOW;
   nw.Flags      = WINDOWDEPTH    |
                   WINDOWCLOSE    |
                   WINDOWDRAG     |
                   SIMPLE_REFRESH |
                   ACTIVATE       |
                   NOCAREREFRESH;

}
else
{
   nw.IDCMPFlags = GADGETUP    |
                   GADGETDOWN  |
                   MENUPICK    |
                   MOUSEMOVE   |
                   VANILLAKEY  |
                   NEWSIZE     |
                   CLOSEWINDOW;
   nw.Flags      = WINDOWDEPTH    |
                   WINDOWSIZING   |
                   WINDOWCLOSE    |
                   WINDOWDRAG     |
                   SIMPLE_REFRESH |
                   ACTIVATE       |
                   NOCAREREFRESH;

}
   nw.Title      = global.title;
   if (AslBase != 0)  /* public screens come with Dos 2.0... */
      {
      nw.Type       = PUBLICSCREEN;

      global.window = OpenWindowTagList(&nw, wintags);
      if (global.screen)
         UnlockPubScreen(NULL, global.screen);
      }
   else /* do the Dos 1.3 version */
      {
      nw.Type       = WBENCHSCREEN;
      global.window = OpenWindow(&nw);
      }

   if (global.window == NULL) return(2);
   SetWindowTitles(global.window, global.title, global.title2);
   global.rp = global.window->RPort;
   enable_menu();
   return(0);
}

/***********************************************************************************
 * Procedure: DestroyWindow
 * Synopsis:  (void)DestroyWindow();
 * Purpose:   Make the current window go away
 ***********************************************************************************/
void DestroyWindow()
{
   free_menus(global.menu);
   CloseWindow(global.window);
global.window = 0; /* do we need this? */
}


/***********************************************************************************
 * Procedure: SkipAss
 * Synopsis:  p = SkipAss(ptr)
 * Purpose:   Skip whitespace in tooltype assignment
 ***********************************************************************************/
char * SkipAss(const char *ptr)
{
   while (*ptr && *ptr != '=')
      ptr++;
   if (*ptr == '=')
   {
      for (ptr++; *ptr == ' ' || *ptr == '\t'; ptr++)
         ;
   }
   return(ptr);
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
   if (!global.rexxinter)
      global.inrexx = 1;      /* controls some error processing         */
   global.rexxmsgs = 1;       /* always get messages & return codes     */
   global.rexxrs[0] = '\0';   /* clear any leftover value               */
   global.rexxrc = 0;         /* presumed innocent until proven guilty  */
   InitSession();             /* ensure console window will be around   */

strcpy(global.rexxrs, "Command OK");
   do_command(arg0);

   global.rexxmsgs = 0;
   global.inrexx = 0;         /* revert to non-rexx error handling      */
   if (global.rexxrc == 0)
      *pres = global.rexxrs;  /* result string (might point to "")      */
   else
      global.rexxrc += 1;     /* to match numbering in documentation    */
   return(global.rexxrc);
}

