#include "project.h"
#ifdef PROJECT_VOPTS
   #include "vopts.h"
#endif
#ifdef PROJECT_VMAKE
   #include "vmake.h"
#endif

Prototype void free_mem(void *mem, int size);
Prototype char *savestr(char *str);
Prototype void *get_mem(int size);
Prototype int request(int nochoice, int txtidx, char *parm, char *option);
Prototype void set_busy(void);
Prototype void set_idle(void);
Prototype void cover_window(void);
Prototype void uncover_window(void);

/***********************************************************************************
 * Procedure: free_mem
 * Synopsis:  free_mem(mem, size);
 * Purpose:   Return allocated memory to the global pool for reuse.
 ***********************************************************************************/
void free_mem(void *mem,
              int size
             )
{
   free(mem);
}

/***********************************************************************************
 * Procedure: get_mem
 * Synopsis:  mem = get_mem(size);
 * Purpose:   Allocate zeroed memory for use.  If the memory can not be allocated,
 *            an error is indicated in the title bar and NULL is returned.
 ***********************************************************************************/
void *get_mem(int size
            )
{
   void *rslt;

   rslt = malloc(size);
   if (!rslt)
   {
      sprintf(global.title, "No Memory for %d bytes\n", size);
      if (global.window)
      {
         SetWindowTitles(global.window, global.title, NULL);
      }
   }
   else
   {
      memset(rslt, 0, size);
   }
   return(rslt);
}

/***********************************************************************************
 * Procedure: savestr
 * Synopsis:  copystr = savestr(str);
 * Purpose:   Save a copy of a string for later reference
 ***********************************************************************************/
char *savestr(char *str
             )
{
   char *p;

   if (!*str) return("");
   p = get_mem(strlen(str)+1);
   strcpy(p, str);
   return(p);
}

/***********************************************************************************
 * Procedure: request
 * Synopsis:  rc = request(nochoice, txtidx, parm, option)
 * Purpose:   Present a requester to the user
 ***********************************************************************************/
int request(int nochoice,
            int txtidx,
            char *parm,
            char *option
           )
{
   int rqrc;

#ifdef PROJECT_VMAKE
/* VOpts doesn't support REXX - under VMake want to fail with no requester */
   if (global.inrexx)
   {
      global.rexxrc = txtidx; /* rexx RC matches config file text entries  */
      return 0;               /* equivalent to user clicking "Cancel"      */
   }
#endif

   if (IntuitionBase->LibNode.lib_Version >= 36)
   {
      int i;
      ULONG iflags;
      char *args[5];
      struct EasyStruct RespES = { sizeof(struct EasyStruct), 0, NULL, "%s", "%s|%s" };

      i = 0;
      args[i++] = global.text[txtidx];
      if (parm)
      {
         RespES.es_TextFormat = "%s\n%s";
         args[i++] = parm;
      }

      if (nochoice)
      {
         RespES.es_GadgetFormat = global.text[TEXT_OK];
      }
      else
      {
         args[i++] = global.text[TEXT_OK];
         if (option)
         {
            RespES.es_GadgetFormat = "%s|%s|%s";
            args[i++] = option;
         }
         args[i++] = global.text[TEXT_CANCEL];
      }

      iflags = IDCMP_DISKINSERTED;

      rqrc = EasyRequestArgs(global.window, &RespES, &iflags, (APTR)args);
   }
   else
   {
      struct IntuiText body, body1, pos, neg;
      int width, height;

      body.FrontPen  =
      body.BackPen   = -1;
      body.DrawMode  = JAM1;
      body.LeftEdge  = 6;
      body.TopEdge   = 4;
      body.ITextFont = &global.ri.TextAttr;
      body.IText     = global.text[txtidx];
      body.NextText  = NULL;

      body1 = body;
      pos   = body;
      neg   = body;
      width = IntuiTextLength(&body);
      height = 4 * global.iheight;
      if (parm)
      {
         int ewidth;
         body.NextText = &body1;
         body1.IText   = parm;
         body1.TopEdge = global.iheight;
         height += global.iheight;
         ewidth = IntuiTextLength(&body1);
         if (ewidth > width) width = ewidth;
      }
      width += 36;  /* margins, resize gadget etc. */
      pos.IText = global.text[TEXT_OK];
      if (nochoice)
      {
         neg.IText = pos.IText;
      }
      else
      {
         neg.IText = global.text[TEXT_CANCEL];
      }

      rqrc = AutoRequest(global.window,
                         &body, &pos, &neg, 0, 0, width, height);      
   }

#ifdef PROJECT_VMAKE
/* VOpts doesn't support ARexx - want VMake to set ARexx return code */
/* unless user followed a success path from the requester            */
   if ((rqrc == 0) || nochoice)
      global.rexxrc = txtidx;
#endif

   return rqrc;
}

#ifdef PROJECT_VMAKE
const USHORT busy_data[] = {

0x0000, 0x0000,   /* Position and control words */
/*************************/            /*************************/
/*                       */            /*                       */
/*  .... .*.. .... ....  */  0x0400,   /*  .... .*** **.. ....  */  0x07C0,
/*  .... .... .... ....  */  0x0000,   /*  .... .*** **.. ....  */  0x07C0,
/*  .... ...* .... ....  */  0x0100,   /*  .... ..** *... ....  */  0x0380,
/*  .... .... .... ....  */  0x0000,   /*  .... .*** ***. ....  */  0x07E0,
/*  .... .*** **.. ....  */  0x07C0,   /*  ...* **** **** *...  */  0x1FF8,
/*  ...* **** **** ....  */  0x1FF0,   /*  ..** **** ***. **..  */  0x3FEC,
/*  ..** **** **** *...  */  0x3FF8,   /*  .*** **** **.* ***.  */  0x7FDE,
/*  ..** **** **** *...  */  0x3FF8,   /*  .*** **** *.** ***.  */  0x7FBE,
/*  .*** **** **** **..  */  0x7FFC,   /*  **** **** .*** ****  */  0xFF7F,
/*  .*** ***. **** **..  */  0x7EFC,   /*  **** **** **** ****  */  0xFFFF,
/*  .*** **** **** **..  */  0x7FFC,   /*  **** **** **** ****  */  0xFFFF,
/*  ..** **** **** *...  */  0x3FF8,   /*  .*** **** **** ***.  */  0x7FFE,
/*  ..** **** **** *...  */  0x3FF8,   /*  .*** **** **** ***.  */  0x7FFE,
/*  ...* **** **** ....  */  0x1FF0,   /*  ..** **** **** **..  */  0x3FFC,
/*  .... .*** **.. ....  */  0x07C0,   /*  ...* **** **** *...  */  0x1FF8,
/*  .... .... .... ....  */  0x0000,   /*  .... .*** ***. ....  */  0x07E0,
/*                       */            /*                       */
/*************************/            /*************************/
0x0000, 0x0000   /* Reserved for the system */
};

#define BUSY_WIDTH   16
#define BUSY_HEIGHT  16
#define BUSY_XOFFSET -6
#define BUSY_YOFFSET  0
static struct Gadget *cover;
static int cover_count;
USHORT *busy_pointer;

/***********************************************************************************
 * Procedure: set_busy
 * Synopsis:  (void)set_busy();
 * Purpose:   Mark the window as busy so no input will come into it
 ***********************************************************************************/
void set_busy(void)
{
#ifdef WA_BusyPointer
   if (IntuitionBase->LibNode.lib_Version >= 39)
   {
      struct TagItem taglist[3];

      taglist[0].ti_Tag  = WA_BusyPointer;
      taglist[0].ti_Data = TRUE;

      taglist[1].ti_Tag  = WA_PointerDelay;
      taglist[1].ti_Data = TRUE;

      taglist[2].ti_Tag  = TAG_DONE;
      taglist[2].ti_Data = TRUE;

      /* Put up the busy pointer, with pointer-delay */
      SetWindowPointer( global.window, taglist);
   }
   else
#endif
   {
      if (busy_pointer == NULL)
      {
         busy_pointer = AllocMem(sizeof(busy_data), MEMF_CHIP);
         if (busy_pointer)
         {
            memcpy(busy_pointer, busy_data, sizeof(busy_data));
            SetPointer(global.window, busy_pointer,
                 BUSY_HEIGHT, BUSY_WIDTH, BUSY_XOFFSET, BUSY_YOFFSET);
         }
      }
   }
   /* We need to create a single gadget that covers the entire screen so that */
   /* They can not click on any of the gadgets.                               */
   cover_window();
}

/***********************************************************************************
 * Procedure: set_idle
 * Synopsis:  (void)set_idle();
 * Purpose:   Remove the busy mark on the window
 ***********************************************************************************/
void set_idle(void)
{

   uncover_window();
#ifdef WA_BusyPointer
   if (IntuitionBase->LibNode.lib_Version >= 39)
   {
      struct TagItem taglist[1];
      taglist[0].ti_Tag  = TAG_DONE;
      taglist[0].ti_Data = TRUE;

      /* Put up the busy pointer, with pointer-delay */
      SetWindowPointer( global.window, taglist);
   }
   else
#endif
   {
      /* Do the 1.3 stuff here */
   }
   if (busy_pointer)
   {
      ClearPointer(global.window);
      FreeMem(busy_pointer, sizeof(busy_data));
   }

   busy_pointer = NULL;
}

/***********************************************************************************
 * Procedure: cover_window
 * Synopsis:  (void)cover_window();
 * Purpose:   cover the window with a bool gadget - deactivate other gadgets
 ***********************************************************************************/
void cover_window(void)
{
   struct Gadget *gad;

   if (cover == 0) /* is there a cover already? */
   {
      gad = (struct Gadget *)get_mem(sizeof(struct Gadget));
      if (gad)
      {
         cover = gad;
   
         gad->LeftEdge    = global.ri.WindowLeft;
         gad->TopEdge     = global.ri.WindowTitle;
         gad->Width       = global.width;
         gad->Height      = global.height - global.ri.WindowTitle;
         gad->Flags       = GADGHNONE;
         gad->Activation  = 0;
         gad->GadgetType  = BOOLGADGET;
   
         AddGadget(global.window, gad, 0);
      }
      cover_count = 0;
   }
   else /* one cover is enough - as long as we don't take it off too soon */
      cover_count++;
}

/***********************************************************************************
 * Procedure: uncover_window
 * Synopsis:  un(void)cover_window();
 * Purpose:   remove bool gadget from window - reactivate other gadgets
 ***********************************************************************************/
void uncover_window(void)
{
   if (cover_count == 0) /* is this the last request to remove it? */
   {
      if (cover)
      {
         RemoveGadget(global.window, cover);
         free_mem(cover, sizeof(struct Gadget));
         cover = NULL;
      }
   }
   else /* don't take it off till the fat lady sings */
      cover_count--;
}
#endif
