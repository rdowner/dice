/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include "vopts.h"
#include "vopts_rev.h"

Prototype int CreateWindow(void);
Prototype void DestroyWindow(void);
Prototype char *SkipAss(const char *);

void myexit(void);
int main(int, char **);

struct GLOBAL global;

struct Library *GadToolsBase;
struct Library *AslBase;
struct Library *ArpBase;

BPTR XSaveLock;
short XSaveLockValid;

char    *InitialFileName;
short   FileSpecified;
char    Buf[256];
short   didgadgets;

const char version[] = VERSTAG;

/*
 *  WORKBENCH STARTUP
 *
 *  Parse tooltypes as a command.  Note that we are CD'd into the
 *  application's directory, not the tooltype's icon's dir.
 */

void
myexit()
{
   if (XSaveLockValid) {
      CurrentDir(XSaveLock);
      XSaveLockValid = 0;
   }
}

int
wbmain(wbs)
struct WBStartup *wbs;
{
   DiskObject *dob;
   short i;
   short j;
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
               /* OK to use any text slot, we're about to bail out */
               strcpy(global.text[1], "Bad ToolType:");
               if (request(0, 1, ptr, NULL))
               {
                  abortIt = 1;
                  break;
               }
            }
         }
         FreeDiskObject(dob);
      }
      CurrentDir(saveLock);
      if (abortIt)
          break;
   }
   XSaveLock = CurrentDir((BPTR)wbs->sm_ArgList[wbs->sm_NumArgs-1].wa_Lock);
   XSaveLockValid = 1;
   atexit(myexit);

   if (abortIt == 0)
      main(1, NULL);
}

/*
 *  CLI STARTUP
 *
 */

int
main(argc, argv)
int argc;
char **argv;
{
   struct IntuiMessage *msg;
   int parse_fail;

   /*
    *   Open GadToolsBase/AslBase manually so we don't exit if they
    *   don't exist.
    */

    if (argc != 1) {
        InitialFileName = argv[1];
        FileSpecified = 1;
    }

   GadToolsBase = OpenLibrary("gadtools.library", 0);
   AslBase      = OpenLibrary("asl.library", 0);

   memset(&global, 0, sizeof(global));

   FillIn_RenderInfo(&global.ri, NULL);

   /* Provide a default height/width so that we will get any errors displayed well */
/*
   global.width = 320;
*/
   global.width = (3 * global.ri.ScreenWidth) / 4;
   global.height = global.ri.WindowTitle;

   global.ri.WindowTitle  += MARGIN_TOP;
   global.ri.WindowLeft   += MARGIN_LEFT;
   global.ri.WindowRight  += MARGIN_RIGHT;
   global.ri.WindowBottom += MARGIN_BOTTOM;

   if (AslBase != NULL)
      global.freq = AllocFileRequest();
   else
   {
      ArpBase = OpenLibrary("arp.library", 0);
      if (ArpBase != NULL)
         global.freq = ArpAllocFreq();
   }

   parse_fail = parse_config(ALT_CONFIG_FILE);
   if (parse_fail == 2) /* failed to open file */
      parse_fail = parse_config(CONFIG_FILE);
   if ((!parse_fail) &&
       (!init_gad_sizes()))
   {
      init_gadgets();
      didgadgets = 1;
   }
   else
   {
      didgadgets = 0;
   }

   if (!CreateWindow())
   {
      if (didgadgets)
      {
         set_gadgets(1);
         do_command("READ ENV");

         if (FileSpecified) {
            sprintf(Buf, "READ %s", InitialFileName);
            do_command(Buf);
         }
      }

      while(!global.done)
      {
         WaitPort(global.window->UserPort);
         while(msg = (struct IntuiMessage *)GetMsg(global.window->UserPort))
         {
            ULONG class;
            APTR iaddr;
            USHORT code;
            USHORT qual;
            struct XItem *xitem;

            code = msg->Code;
            class = msg->Class;
            iaddr = msg->IAddress;
            qual  = msg->Qualifier;

            ReplyMsg((struct Message *)msg);

            switch(class)
            {
               case CLOSEWINDOW:
                  global.done = 1;
                  break;
               case GADGETDOWN:
               case GADGETUP:
                  handle_hit((struct Gadget *)iaddr,
                      (qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 0);
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
#endif
                  break;
            }
         }
      }
      DestroyWindow();
   }

   CleanUp_RenderInfo(&global.ri);
   if (global.freq && (AslBase != NULL)) FreeFileRequest(global.freq);

   if (GadToolsBase  != NULL) CloseLibrary(GadToolsBase);
   if (AslBase  != NULL)      CloseLibrary(AslBase);
   if (ArpBase  != NULL)      CloseLibrary(ArpBase);
   return(0);
}

/*
 * Create a new window on the screen
 */
int CreateWindow()
{
   struct NewWindow nw;

   memset(&nw, 0, sizeof(nw));

   nw.Width      = global.width;
   nw.Height     = global.height;
   nw.LeftEdge   = (global.ri.ScreenWidth  - nw.Width ) / 2;
   nw.TopEdge    = (global.ri.ScreenHeight - nw.Height) / 2;

   if ((nw.Width < 0) || (nw.Height < 0))
      return(1);
   nw.DetailPen  = nw.BlockPen = -1;
   nw.IDCMPFlags = GADGETUP    |
                   GADGETDOWN  |
                   MENUPICK    |
                   VANILLAKEY  |
                   CLOSEWINDOW;
   nw.Flags      = WINDOWDEPTH    |
                   WINDOWCLOSE    |
                   WINDOWDRAG     |
                   SIMPLE_REFRESH |
                   ACTIVATE       |
                   NOCAREREFRESH;
   nw.Title      = global.title;
   nw.Type       = WBENCHSCREEN;

   global.window = OpenWindow(&nw);
   if (global.window == NULL) return(2);
   global.rp = global.window->RPort;
   SetWindowTitles(global.window, global.title, global.wtitle);
   enable_menu();
   return(0);
}

/*
 * Make the current window go away
 */
void DestroyWindow()
{
   free_menus(global.menu);
   CloseWindow(global.window);
}

/*
 *  Skip whitespace in tooltype assignment
 */

char *
SkipAss(ptr)
const char *ptr;
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
