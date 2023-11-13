#include "vopts.h"

Prototype int  init_gadgets(void);
Prototype void free_gadget(struct Gadget *gadget);
Prototype void free_gadlist(struct GADLIST *gadlist);
Prototype int  set_group_gadgets(void);
Prototype void set_fr_gadgets(char *frtext);
Prototype void clear_fr_gadgets(void);
Prototype struct Gadget *create_gadget(struct GADLIST *gadlist,struct G_OBJECT *object,int ulx, int uly,int width);
Prototype void setup_string_gadget(struct Gadget *gad,int base);
Prototype struct Gadget *setup_cycle_gadget(struct Gadget *gad,struct IntuiText *itext,struct G_VALUE *val);
Prototype void reset_list_object(struct G_LIST *list,int active);
Prototype int  setup_list_object(struct GADLIST *gadlist,struct Gadget *gad);
Prototype struct GADLIST *layout_gadgets(struct G_OBJECT *objlist, int group, int ulx, int uly, int width);
Prototype struct Border *build_border(int width, int height, int mode);
Prototype int  init_gad_sizes(void);
Prototype void get_cycle_sizes(struct G_OBJECT *objlist, int *titsize,  int *cycsize, int *fudgesize);
Prototype int  text_width(char *str);
Prototype int string_width(char *str, int def_size);

/***********************************************************************************
 * Procedure: init_gadgets
 * Synopsis:  rc = init_gadgets();
 * Purpose:   Set up all the base system gadgets for the window.  This includes all
 *            The user defined gadgets as well as the three buttons on the bottom
 ***********************************************************************************/
int init_gadgets()
{
   struct Gadget *gad;
   struct Border *border;
   struct IntuiText *itext;
   int ulx, uly;
   int i;

   global.gadlist = layout_gadgets(global.objects, 0,
                                    global.ri.WindowLeft,
                                    global.ri.WindowTitle,
                                    global.cycsize[0]+global.titsize[0]
                                    );
   if (global.gadlist == NULL) return(1);
   ulx = global.ri.WindowLeft;
   uly = global.height - global.ri.WindowBottom - global.iheight;

   for(i = 0; i < 3; i++)
   {
      global.button[i].class = CLASS_BUTTON;
      global.button[i].state = i; /* Note that these values correspond to */
                                   /* the BUTTON_xxx equates               */

      gad = create_gadget(NULL, global.button+i, ulx, uly, BUTTON_WIDTH);

      if (gad == NULL) return(1);
      gad->GadgetRender = build_border(BUTTON_WIDTH,
                                       global.iheight, MODE_OUT);

      itext = gad->GadgetText;
      itext->LeftEdge += (BUTTON_WIDTH - IntuiTextLength(itext))/2;

      gad->NextGadget = global.gadlist->gadgets;
      global.gadlist->gadgets = gad;
      global.gadlist->count++;

      gad->GadgetType = BOOLGADGET;

      ulx += (global.width - (global.ri.WindowLeft  +
                               global.ri.WindowRight +
                               (3*BUTTON_WIDTH)
                              )
             ) / 2 + BUTTON_WIDTH;
   }

   /* Calculate the limits for the box that holds the gadgets */
   global.boxw = global.cycsize[1] + global.titsize[1] +
                  MARGIN_LEFT + MARGIN_RIGHT;
   global.boxx = global.width - global.ri.WindowRight - global.boxw;

   global.boxy = global.ri.WindowTitle + DHBAR;
   global.boxh = uly - MARGIN_MID - DVBAR - global.boxy;

   for (i = 2; i >= 0; i -= 2)
   {
      /* We also need to attach a box around the other area.  Since we don't want to */
      /* Have to constantly create/recreate it, we can get away with putting it on   */
      /* the first button.                                                           */
      border = build_border(global.boxw + DVBAR + i*2,
                            global.boxh + DHBAR + i,
                            i ? MODE_IN : MODE_OUT);
      if (border != NULL)
      {
         border->NextBorder->LeftEdge =
         border->LeftEdge = (global.boxx - VBAR - i       ) - gad->LeftEdge;
         border->NextBorder->TopEdge =
         border->TopEdge  = (global.boxy - HBAR - (i >> 1)) - gad->TopEdge;
         border->NextBorder->NextBorder = gad->GadgetRender;
         gad->GadgetRender = border;
      }
   }

   global.boxx += MARGIN_LEFT;
   global.boxy += MARGIN_TOP-DHBAR;
   global.boxw -= (MARGIN_LEFT+MARGIN_RIGHT);
   global.boxh -= (MARGIN_TOP-DHBAR+MARGIN_BOTTOM);

   /* now setup stuff for default file requester gadgets */
   for (i = 0; i < 2; i++)
   {
      global.frbutton[i].class = CLASS_BUTTON;
      global.frbutton[i].state = BUTTON_FRSAVE + i;
      /* this is a shameless trick to skip the normal middle button */
      global.frbutton[i].title = global.button[2*i].title;

      global.frbutton[i].title =
         global.text[i == 0 ? TEXT_OK : TEXT_CANCEL];
   }
   global.frstring.base.class = CLASS_STRING;
   global.frstring.base.title = global.text[TEXT_FILEREQ];

   return(set_group_gadgets());
}
/***********************************************************************************
 * Procedure: free_gadget
 * Synopsis:  free_gadget(gadget)
 * Purpose:   Free up all memory associated with a gadget
 ***********************************************************************************/
void free_gadget(struct Gadget *gadget)
{
   struct G_OBJECT *obj;

   if (gadget)
   {
      obj = (struct G_OBJECT *)(gadget->UserData);

      obj->gadget = NULL;

      if (obj->class == CLASS_STRING)
      {
         /* We need to free the border structure also */
      }
      free_mem(gadget, sizeof(struct Gadget)+sizeof(struct IntuiText));
   }
}

/***********************************************************************************
 * Procedure: free_gadlist
 * Synopsis:  free_gadlist(gadlist)
 * Purpose:   Free up all memory associated with a gadget list created by
 *            layout_gadgets
 ***********************************************************************************/
void free_gadlist(struct GADLIST *gadlist)
{
   struct Gadget *gad, *nextgad;
   struct G_CYCLE *cyc;
   struct G_STRING *str;

   for(gad = gadlist->gadgets; gad && gadlist->count; gad = nextgad, gadlist->count--)
   {
      nextgad = gad->NextGadget;
      cyc = (struct G_CYCLE *)gad->UserData;

      if (cyc->base.class == CLASS_CYCLE)
      {
         /* If there is a string gadget currently in the cycle gadget, we need */
         /* to free it up                                                      */
         if ((str = cyc->curval->string) != NULL)
         {
            free_gadget(str->base.gadget);
         }
      }
      free_gadget(gad);
   }

   free_mem(gadlist, sizeof(struct GADLIST));
}

/***********************************************************************************
 * Procedure: set_group_gadgets
 * Synopsis:  set_group_gadgets();
 * Purpose:   Create the gadgetry for
 ***********************************************************************************/
int set_group_gadgets()
{
   struct G_OBJECT *next;

   if (global.grpgadlist)
   {
      /* We need to take this off the system gadget list and then free up */
      /* all the storage associated with it                               */
      set_gadlist(global.grpgadlist, 0);
      free_gadlist(global.grpgadlist);

      global.grpgadlist = NULL;
   }

   if (global.rp)
   {
      /* Wipe out the inner area of the gadget so that we can redraw it later */
      SetBPen(global.rp, 0);
      SetAPen(global.rp, 0);
      RectFill( global.rp, global.boxx, global.boxy,
                            global.boxx+global.boxw, global.boxy+global.boxh);
   }

   next = global.curgroup->base.next;
   global.curgroup->base.next = global.curgroup->objects;
   global.grpgadlist = layout_gadgets((struct G_OBJECT *)global.curgroup, 1,
                                    global.boxx, global.boxy, global.boxw);
   global.curgroup->base.next = next;

   set_gadlist(global.grpgadlist, 1);

   return(0);
}

/***********************************************************************************
 * Procedure: set_fr_gadgets
 * Synopsis:  set_fr_gadgets(text);
 * Purpose:   setup the string gadget and buttons for the default file requester
 *            to use when AmigaDos 2.0 is not present.  Text is initial string
 *            gadget text
 ***********************************************************************************/

void set_fr_gadgets(char *frtext)
{
   int ulx, uly, i;
   struct Gadget *gad;
   struct IntuiText *itext;

   /* Need to see if requester already displayed an exit if so.  */
   if (global.frgadlist != NULL) return;

   set_gadgets(0);
   SetBPen(global.rp, 0);
   SetAPen(global.rp, 0);
   RectFill( global.rp, global.ri.WindowLeft,
                        global.ri.WindowTitle,
                        global.width - global.ri.WindowRight + 4,
                        global.height - global.ri.WindowBottom);

   strncpy(global.frstring.buf, frtext, MAX_STRING);
   /* make sure string is null terminated */
   global.frstring.buf[MAX_STRING-1] = 0;
   global.frgadlist = layout_gadgets((struct G_OBJECT *)&global.frstring, 2,
                 global.ri.WindowLeft,
                 global.ri.WindowTop + (global.height - global.iheight)/2,
                 global.width);

   if (global.frgadlist == NULL) return;

   ulx = global.ri.WindowLeft;
   uly = global.height - global.ri.WindowBottom - global.iheight;

   /* Create the buttons here */
   for(i = 0; i < 2; i++)
   {
      gad = create_gadget(global.frgadlist, global.frbutton+i,
                          ulx, uly, BUTTON_WIDTH);

      if (gad == NULL) return;
      gad->GadgetRender = build_border(BUTTON_WIDTH,
                                       global.iheight, MODE_OUT);

      itext = gad->GadgetText;
      itext->LeftEdge += (BUTTON_WIDTH - IntuiTextLength(itext))/2;

      gad->GadgetType = BOOLGADGET;
      ulx = global.width - (global.ri.WindowRight + BUTTON_WIDTH);
   }
   set_gadlist(global.frgadlist, 1);

}

/***********************************************************************************
 * Procedure: clear_fr_gadgets
 * Synopsis:  clear_fr_gadgets();
 * Purpose:   remove the default requester stuff and redisplay the normal
 *            VOPTS gadgets.
 ***********************************************************************************/

void clear_fr_gadgets(void)
{
   if (global.frgadlist != NULL)
   {
      set_gadlist(global.frgadlist, 0);
      /* Free the gadlist */
      free_gadlist(global.frgadlist);
      global.frgadlist = NULL;
      SetBPen(global.rp, 0);
      SetAPen(global.rp, 0);
      RectFill( global.rp, global.ri.WindowLeft,
                        global.ri.WindowTitle,
                        global.width - global.ri.WindowRight,
                        global.height - global.ri.WindowBottom);

   }
   set_gadgets(1);

}
/***********************************************************************************
 * Procedure: create_gadget
 * Synopsis:  Gadget = create_gadget(gadlist, object, ulx, uly, width)
 * Purpose:   Create all the appropriate gadgetry and image structures for a list
 *            of gadgets.
 ***********************************************************************************/
struct Gadget *create_gadget(struct GADLIST *gadlist,
                             struct G_OBJECT *object,
                             int ulx, int uly,
                             int width
                            )
{
   struct Gadget *gad;
   struct IntuiText *itext;


   /* Create a gadget structure and an associated intuitext structure to be */
   /* Layed out on the screen                                               */
   gad = (struct Gadget *)get_mem(sizeof(struct Gadget)+sizeof(struct IntuiText));
   if (gad)
   {
      itext = (struct IntuiText *)(gad+1);

      gad->LeftEdge    = ulx;
      gad->TopEdge     = uly;
      gad->Width       = width;
      gad->Height      = global.iheight;
      gad->Flags       = GADGHCOMP;
      gad->Activation  = RELVERIFY;
      gad->GadgetText  = itext;
      gad->UserData    = (APTR)object;
      gad->GadgetID    = object->class;
      object->gadget      = gad;

      /* Now we fill in the Intuitext structure */
      itext->FrontPen  = 1;
      itext->BackPen   = 0;
      itext->DrawMode  = JAM1;
      itext->LeftEdge  = 0;
      itext->TopEdge   = (global.iheight - global.ri.FontSize) / 2;
      itext->ITextFont = &global.ri.TextAttr;
      itext->IText     = object->title;
      itext->NextText  = NULL;

      if (gadlist)
      {
         gad->NextGadget = gadlist->gadgets;
         gadlist->gadgets = gad;
         gadlist->count++;
      }
   }

   return(gad);
}

/***********************************************************************************
 * Procedure: setup_string_gadget
 * Synopsis:  setup_string_gadget(gad, string, ulx, uly, width, base)
 * Purpose:   Create all the appropriate gadgetry and image structures for a list
 *            of gadgets.
 ***********************************************************************************/
void setup_string_gadget(struct Gadget *gad,
                         int base
                        )
{
   int wraps;
   int i;
   struct Border *border;
   struct StringInfo *si;
   int sluff;

   if (gad == NULL) return;

   wraps = (base == CLASS_STRING) ? 2 : 1;

   gad->LeftEdge += VBAR*wraps;
   gad->TopEdge  += DHBAR;
   gad->Width    -= DVBAR*wraps;
   gad->Height   -= 2*DHBAR;

   sluff = (gad->Height - global.ri.FontSize);
   if (sluff < 0) sluff = 0;
   gad->Height  = global.ri.FontSize;
   gad->TopEdge += sluff / 2;
   if (gad->GadgetText)
   {
      gad->GadgetText->TopEdge -= (DHBAR+sluff);
      gad->GadgetText->LeftEdge -= VBAR*wraps;
   }

   if ((base == CLASS_LIST)   ||
       (base == CLASS_STRING) ||
       (base == CLASS_CYCLE))
   {
      for(i=wraps; i > 0; i--)
      {
         /* We need to put a border around the gadget. */
         /* For some string gadgets, we only need a single border */
         /* We can tell this by the base type */
         border = build_border(gad->Width+4*i, gad->Height + sluff + 2*i,
                               (i == 1) ? MODE_IN : MODE_OUT);
         if (border != NULL)
         {
            border->NextBorder->LeftEdge = border->LeftEdge = -2*i;
            border->NextBorder->TopEdge  = border->TopEdge  = -1*i - (sluff/2);
            border->NextBorder->NextBorder = gad->GadgetRender;
            gad->GadgetRender = border;
         }
      }
   }

   /* Lastly, we need to setup up the SpecialInfo structure for the string gadget */
   /* If for some reason this fails, we will turn it into a button gadget         */
   si = (struct StringInfo *)get_mem(sizeof(struct StringInfo));
   if (si != NULL)
   {
      gad->SpecialInfo = (APTR)si;
      gad->GadgetType = STRGADGET;
      si->Buffer = ((struct G_STRING *)(gad->UserData))->buf;
      si->MaxChars = MAX_STRING;
   }
   else
      gad->GadgetType = BOOLGADGET;
}


/***********************************************************************************
 * Procedure: setup_cycle_gadget
 * Synopsis:  gadget = setup_cycle_gadget(gadget, value)
 * Purpose:   Create any subtending string gadgets and correctly position any
 *            cycle value text
 ***********************************************************************************/
struct Gadget *setup_cycle_gadget(struct Gadget *gad,
                                  struct IntuiText *itext,
                                  struct G_VALUE *val
                                 )
{
   struct Gadget *strgad;
   int ilen;

   ilen = IntuiTextLength(itext);

   if (val->string)
   {
      int x, width;

      itext->LeftEdge = CYC_ICON_WIDTH + 2;
      /* We need to create a string gadget to get the input text from */

      x = itext->LeftEdge + ilen;
      width = gad->Width - x - 2;

      /* Now, create a string gadget and border to put around the gadget */
      strgad = create_gadget(NULL, (struct G_OBJECT *)val->string,
                                       gad->LeftEdge+x, gad->TopEdge, width);
      if (strgad)
      {
         setup_string_gadget(strgad, CLASS_CYCLE);
      }
   }
   else
   {
      strgad = NULL;
      itext->LeftEdge  = (CYC_ICON_WIDTH + gad->Width - ilen) / 2;
   }
   return(strgad);
}
/***********************************************************************************
 * Procedure: reset_list_object
 * Synopsis:  void reset_list_object(list,active);
 * Purpose:   Initialize all the appropriate text pointers for a list gadget
 ***********************************************************************************/
void reset_list_object(struct G_LIST *list,
                       int active
                      )
{
   struct G_ENTRY *ent;
   int i;

   ent = list->top;

   /* Run through all the entries visible on the screen and for each */
   /* one that is displayed, mark the gadget as selectable.          */
   for(i = 0; i < MAX_LIST; i++)
   {
      list->btngad[i]->Flags  = (list->btngad[i]->Flags & ~GADGHIGHBITS) |
                                GADGHNONE;
      list->btngad[i]->Height = global.eheight;
      list->strgad[i]->GadgetRender = NULL;
      /* seems unnecessary, but Dos 1.3 likes to write blanks to unselected */
      /* string gadgets                                                     */
      ((struct StringInfo *)list->strgad[i]->SpecialInfo)->Buffer = global.defbuf;

      /* Is there an entry corresponding to this position on the screen? */
      if (ent)
      {
         /* Certainly is an entry, is it where they want a string gadget ? */
         if (i == active)
         {
            /* Yes, shrink the gadget out of existence so that it doesn't  */
            /* appear on the screen.                                       */
            list->btngad[i]->Height = 1;
            list->strgad[i]->GadgetRender = list->sborder;
         }
         else
         {
            list->btngad[i]->Flags = (list->btngad[i]->Flags & ~GADGHIGHBITS) |
                                     GADGHCOMP;
         }

         ((struct StringInfo *)
          list->strgad[i]->SpecialInfo)->Buffer = ent->buf;
         ent = (struct G_ENTRY *)ent->base.next;
      }
   }
}

/***********************************************************************************
 * Procedure: setup_list_object
 * Synopsis:  ulx = setup_list_object(gadlist, gadget)
 * Purpose:   Create all the appropriate gadgetry and image structures for a list
 *            object
 ***********************************************************************************/
int setup_list_object(struct GADLIST *gadlist,
                      struct Gadget *gad
                     )
{
   int ulx, uly, alen, dlen, ilen;
   struct G_LIST *list;
   int i;
   char *savetitle;
   struct Gadget *slider, *dngad, *upgad, *addgad, *delgad;
   struct PropInfo *pi;

#define ARR_HEIGHT  (7+DHBAR)
#define PROP_WIDTH  (14+DVBAR)

   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
   /* A list object consists of 8 separate gadgets arranged as below*/
   /* 1: Boolean gadgets that overlay string gadgets.  When the     */
   /* 2:   string gadget is to be active, the height of the boolean */
   /* 3:   gadget is set to 1 exposing the string gadget.           */
   /* 4: A slider gadget to allow positioning in the list           */
   /* 5: An up arrow gadget for moving up one item in the list      */
   /* 6: A Down arrow gadget for moving down one item in the list   */
   /* 7: A NEW gadget for creating an entry in front of current item*/
   /* 8: A DEL gadget for removing the current item from the list.  */
   /*                                                               */
   /*                                                               */
   /*  TITLE TEXT                                                   */
   /* ............................................}+---+            */
   /* ::+---------------------------------------+}}|4  |            */
   /* ::|1                                      |}}|   |            */
   /* ::| I T E M 1                             |}}|   |            */
   /* ::+---------------------------------------+}}| _ |+-------+   */
   /* ::+---------------------------------------+}}|[_]||7      |   */
   /* ::|2                                      |}}|   || N E W |   */
   /* ::| I T E M 2                             |}}+---+|       |   */
   /* ::+---------------------------------------+}}|5^ |+-------+   */
   /* ::+---------------------------------------+}}|/ \|+-------+   */
   /* ::|3                                      |}}+---+|8      |   */
   /* ::| I T E M 3                             |}}|\ /|| D E L |   */
   /* ::+---------------------------------------+}}|6V ||       |   */
   /* :}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}+---++-------+   */
   /*                                                               */
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
   list = (struct G_LIST *)gad->UserData;

   /* We need to figure out how big the NEW and DEL gadgets will be */
   alen = ilen = text_width(global.button[BUTTON_NEW].title);
   dlen = text_width(global.button[BUTTON_DEL].title);
   if (dlen > ilen) ilen = dlen;
   ilen += 2*DHBAR;

   gad->GadgetText->TopEdge   = -global.ri.FontSize;
   gad->TopEdge     += global.ri.FontSize;
   gad->Width       -= ilen + PROP_WIDTH;
   gad->Height       = MAX_LIST*global.eheight + global.listextra;
   gad->Flags        = GADGHNONE;
   gad->GadgetRender = build_border(gad->Width,
                                    gad->Height,
                                    MODE_OUT);

   savetitle = list->base.title;
   list->base.title = "";

   uly = gad->TopEdge;
   ulx = gad->LeftEdge + gad->Width;

   /* Create the slider and two arrow gadgets.  We will steal the  */
   /* first itext structure and throw it on to the main gadget     */
   slider = create_gadget(gadlist, (struct G_OBJECT *)list,
                                  ulx+DVBAR, uly+DHBAR, PROP_WIDTH-2*DVBAR );
   if (slider == NULL) return(0);
   list->slider = slider;
   slider->Height       = global.listextra + MAX_LIST*global.eheight -
                          2*ARR_HEIGHT - 2*DHBAR;
   slider->GadgetID     = CLASS_PROP;
   slider->GadgetType   = PROPGADGET;
   pi = (struct PropInfo *)get_mem(sizeof(struct PropInfo)+
                                           sizeof(struct Image));
   if (pi == NULL) return(0);
   slider->GadgetRender = (struct Image *)(pi+1);
   slider->SpecialInfo  = (APTR)pi;
   pi->Flags = AUTOKNOB | FREEVERT | PROPBORDERLESS;
   recalc_prop(list, &pi->VertBody, &pi->VertPot);

   upgad  = create_gadget(gadlist, (struct G_OBJECT *)list,
                                  ulx, uly+slider->Height+2*DHBAR, PROP_WIDTH);
   if (upgad == NULL) return(0);
   upgad->Height       = ARR_HEIGHT;
   upgad->GadgetRender = &global.arrowborder[1];
   upgad->GadgetType   = BOOLGADGET; /* not auto default for Dos 1.3  */
   upgad->GadgetID     = CLASS_UP;
   upgad->GadgetText   = NULL;

   dngad  = create_gadget(gadlist, (struct G_OBJECT *)list,
                                  ulx, upgad->TopEdge+ARR_HEIGHT, PROP_WIDTH);
   if (dngad == NULL) return(0);
   dngad->Height       = ARR_HEIGHT;
   dngad->GadgetRender = &global.arrowborder[0];
   dngad->GadgetType   = BOOLGADGET; /* not auto default for Dos 1.3  */
   dngad->GadgetID     = CLASS_DOWN;
   dngad->GadgetText   = NULL;

   /* Create the ADD/DEL Buttons                                        */
   addgad = create_gadget(gadlist, (struct G_OBJECT *)list, ulx+PROP_WIDTH,
                                  uly + global.listextra + MAX_LIST*global.eheight-2*global.iheight, ilen);
   if (addgad == NULL) return(0);
   addgad->GadgetRender = build_border(ilen, addgad->Height, MODE_OUT);
   addgad->GadgetType   = BOOLGADGET; /* not auto default for Dos 1.3  */
   addgad->GadgetID     = CLASS_ADD;
   addgad->GadgetText->LeftEdge += (ilen-alen)>>1;
   addgad->GadgetText->IText = global.button[BUTTON_NEW].title;

   delgad = create_gadget(gadlist, (struct G_OBJECT *)list, addgad->LeftEdge,
                                       addgad->TopEdge+addgad->Height, ilen);
   if (delgad == NULL) return(0);
   list->delgad = delgad;
   delgad->GadgetRender = addgad->GadgetRender;
   delgad->GadgetType   = BOOLGADGET; /* not auto default for Dos 1.3  */
   delgad->GadgetID     = CLASS_DEL;
   if (list->first == NULL)
      delgad->Flags       |= GADGDISABLED;
   delgad->GadgetText->LeftEdge += (ilen-dlen)>>1;
   delgad->GadgetText->IText = global.button[BUTTON_DEL].title;

   for(i = 0; i < MAX_LIST; i++)
   {
      struct Gadget *tgad;

      /* Create the string and button gadgets.  Because we insert them on the */
      /* List in reverse order and we want the button gadget to appear first, */
      /* we need to create them in reverse order.                             */
      /* This ordering is assumed in the state changing code.                 */
      tgad = list->strgad[i] = create_gadget(gadlist,
                                             (struct G_OBJECT *)list,
                                             gad->LeftEdge+VBAR, uly,
                                             gad->Width-DVBAR);
      if (tgad == NULL) return(0);
      tgad->GadgetText = NULL;
      tgad->Height = global.eheight;
      setup_string_gadget(tgad, CLASS_LIST+i);
      tgad->GadgetID   = CLASS_STRING;

      tgad = list->btngad[i] = create_gadget(gadlist,
                                             (struct G_OBJECT *)list,
                                             gad->LeftEdge+VBAR, uly,
                                             gad->Width-DVBAR);
      if (tgad == NULL) return(0);
      tgad->GadgetType = BOOLGADGET; /* not auto default for Dos 1.3  */
      tgad->Height     = global.eheight;
      tgad->GadgetText = NULL;
      tgad->GadgetID   = CLASS_LIST+i;
      uly += global.eheight;
   }

   list->sborder = list->strgad[0]->GadgetRender;
   list->strgad[0]->GadgetRender = NULL;

   /* Undo the damage we did to the base object */
   list->base.title = savetitle;
   list->base.gadget = gad;

   reset_list_object(list, -1);

   return(uly+global.listextra);
}

/***********************************************************************************
 * Procedure: layout_gadgets
 * Synopsis:  gadlist = layout_gadgets(objlist, group, ulx, yly, width);
 * Purpose:   Create all the appropriate gadgetry and image structures for a list
 *            of gadgets.
 ***********************************************************************************/
struct GADLIST *layout_gadgets(struct G_OBJECT *objlist,
                               int group,
                               int ulx, int uly,
                               int width
                              )
{
   struct Gadget *gad;
   struct IntuiText *itext, *itext1;
   int ilen;
   struct G_OBJECT *obj;
   struct GADLIST *gadlist;

   gadlist = (struct GADLIST *)get_mem(sizeof(struct GADLIST));
   if (gadlist == NULL) return(NULL);

   /* First we need to lay out the gadgets on the left side of the window */
   for(obj = objlist; obj != NULL; obj = obj->next)
   {
      /* Create a gadget structure and an associated intuitext structure to be */
      /* Layed out on the screen                                               */
      gad = create_gadget(gadlist, obj, ulx, uly, width);

      if (gad == NULL) return(gadlist);

      itext = (struct IntuiText *)(gad+1);

      uly += gad->Height;

      ilen = IntuiTextLength(itext);
      switch(obj->class)
      {
         case CLASS_STRING:
            gad->LeftEdge   += global.titsize[group];
            gad->Width       = global.cycsize[group];
            itext->LeftEdge  = -ilen;
            setup_string_gadget(gad, CLASS_STRING);
            break;

         case CLASS_GROUP:
            gad->GadgetRender = global.cycborder[group];
            gad->GadgetType   = BOOLGADGET;
            gad->Width        = global.cycsize[group];
            gad->LeftEdge    += global.titsize[group] / 2;
            itext->LeftEdge   = (CYC_ICON_WIDTH+gad->Width-ilen)/2;
            uly  += MARGIN_TOP;
            break;

         case CLASS_CYCLE:
            gad->GadgetRender = global.cycborder[group];
            gad->GadgetType = BOOLGADGET;
            gad->LeftEdge   += global.titsize[group];
            gad->Width       = global.cycsize[group];
            itext->LeftEdge  = -ilen;
            itext1 = (struct IntuiText *)get_mem(sizeof(struct IntuiText));
            if (itext1 == NULL) return(gadlist);
            itext1->FrontPen  = 1;
            itext1->BackPen   = 0;
            itext1->DrawMode  = JAM1;
            itext1->LeftEdge  = 0;
            itext1->TopEdge   = 3;
            itext1->ITextFont = &global.ri.TextAttr;
            itext1->IText     = ((struct G_CYCLE *)obj)->curval->title;
            itext1->NextText  = NULL;
            itext1->LeftEdge  = (CYC_ICON_WIDTH+gad->Width-IntuiTextLength(itext1))/2;

            itext->NextText   = itext1;
            itext1->IText     = ((struct G_CYCLE *)obj)->curval->title;
            break;

         case CLASS_CHECK:
            gad->GadgetType   = BOOLGADGET;
            gad->Width        = CHECK_WIDTH;
            gad->Height       = CHECK_HEIGHT;
            gad->Flags        = GADGHIMAGE;
            if (obj->state)   gad->Flags |= SELECTED;
            gad->Activation  |= TOGGLESELECT;
            gad->GadgetRender = global.checkborder[0];
            gad->SelectRender = global.checkborder[1];
            itext->LeftEdge  += CHECK_WIDTH + 4;
            break;

         case CLASS_LIST:
            uly = setup_list_object(gadlist, gad);
            if (uly <= 0) return(gadlist);
            break;

         default:
            gad->GadgetType = BOOLGADGET;
            break;
      }
   }
   return(gadlist);
}

/***********************************************************************************
 * Procedure: build_border
 * Synopsis:  Border = build_border(width, height, mode)
 * Purpose:   Build an return a set of drawing vectors that represent a button of
 *            width and height.  Mode can be either MODE_OUT or MODE_IN
 ***********************************************************************************/
struct Border *build_border(int width,
                            int height,
                            int mode
                           )
{
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
   /*                                                                             */
   /* Select the colors based on the rendering mode.  For an out box, we will use */
   /* the lighter color on the upper and left sides and the darker color on the   */
   /* other two sides.  In an in box, the choice is reversed.                     */
   /*   (x,y)                                                5                    */
   /*   1****************************************************.2                   */
   /*    **4                                               3..                    */
   /*    **                                                 ..                    */
   /*    **                                                 ..                    */
   /*    **                                                 ..                    */
   /*    **                                                 ..                    */
   /*    **3 5                                             4..                    */
   /*   2*....................................................1                   */
   /*                                                       (x+width, y+height)   */
   /* 0.       0,    0    10.   wd,   ht                                          */
   /* 2.       0,   ht    12.   wd,    0                                          */
   /* 4.       1, ht-1    14. wd-1,    1                                          */
   /* 6.       1,    0    16. wd-1,   ht                                          */
   /* 8.    wd-1,    0    18.    1,   ht                                          */
   /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

   struct Border *border, *border1;
   short *vecs;
   int i;

   static short inits[] = { 0, 0,   0, 0,   1,-1,   1, 0,  -1, 0,
                            0, 0,   0, 0,  -1, 1,  -1, 0,   1, 0};

#define VEC_COUNT 5
#define SINGLEUNIT (sizeof(struct Border)+sizeof(inits))

   width--;
   height--;
   border = (struct Border *)get_mem(2*SINGLEUNIT);
   if (border == NULL) return(NULL);

   border1 = border+1;
   vecs = (short *)(border+2);
   border->NextBorder = border1;
   border->DrawMode   = JAM1;
   border->Count      = VEC_COUNT;
   border->XY         = vecs;

   border1->DrawMode  = JAM1;
   border1->Count     = VEC_COUNT;
   border1->XY        = vecs + (2*VEC_COUNT);

   memcpy((char *)vecs, (char *)inits, sizeof(inits));
   for(i = 8; i < 17; i += 2) vecs[i] += width;
   vecs[3]  += height;
   vecs[5]  += height;
   vecs[11] += height;
   vecs[17] += height;
   vecs[19] += height;

   if (mode == MODE_OUT)
   {
      border1->FrontPen = global.ri.Shadow;
      border->FrontPen = global.ri.Highlight;
   }
   else
   {
      border1->FrontPen = global.ri.Highlight;
      border->FrontPen  = global.ri.Shadow;
   }
   return(border);
}

/***********************************************************************************
 * Procedure: init_gad_sizes
 * Synopsis:  rc = init_gad_sizes();
 * Purpose:   Calculate and initialize all the imagery structures
 ***********************************************************************************/
int init_gad_sizes()
{
   struct G_GROUP     *groups;
   int i;
   int width, height, fudgesize;
   struct Border *border;
   /* * * * * * * * * * * * * * * * * * * * * * * * * * */
   /*                                                   */
   /*              11111111                11111111     */
   /*    012345678901234567      012345678901234567     */
   /*    =================#      =================# 0   */
   /*    ||......23......##      ||..18......54..## 1   */
   /*    ||......##......##      ||..##......##..## 2   */
   /*    ||.....####.....##      ||...##....##...## 3   */
   /*    ||....##76##....##      ||....##76##....## 4   */
   /*    ||...##....##...##      ||.....####.....## 5   */
   /*    ||..##......##..##      ||......##......## 6   */
   /*    ||..18......54..##      ||......23......## 7   */
   /*    |#################      |################# 8   */
   /*                                                   */
   /* * * * * * * * * * * * * * * * * * * * * * * * * * */
   static short Up_Vectors[] = { 4,6,  8,2,  9,2, 13,6, 12,6,  9,3,  8,3,  5,6 };
   static short Dn_Vectors[] = { 4,2,  8,6,  9,6, 13,2, 12,2,  9,5,  8,5,  5,2 };

   /* * * * * * * * * * * * * * * * * * */
   /*              11111111112222       */
   /*    012345678901234567890134       */
   /*    ========================== 0   */
   /*    ||.....7.....8..........   1   */
   /*    ||.....#######......#|..   2   */
   /*    ||...5##.....##d....#|..   3   */
   /*    ||....##.....##.....#|..   4   */
   /*    ||....##..a######b..#|..   5   */
   /*    ||....##....####....#|..   6   */
   /*    ||....##....9##c....#|..   7   */
   /*    ||....##............#|..   8   */
   /*    ||...4##6...0##1....#|..   9   */
   /*    ||.....#######......#|..   10  */
   /*    ||.....3.....2..........   11  */
   /*    |########################  12  */
   /*                                   */
   /* * * * * * * * * * * * * * * * * * */
   static short Cycle_Vectors[] = { 13, 9,  14, 9,  13,10,   7,10,   6, 9,
                                     6, 3,   7, 9,   7, 2,  13, 2,  13, 7,
                                    11, 5,  16, 5,  14, 7,  14, 3, };
   static short Cycle_Line1[]   = { 20, 2,  20,11};
   static short Cycle_Line2[]   = { 21, 2,  21,11};

   static short Check_Vectors[] = { 19, 2,  17, 2,  11, 8,   8, 5,   7, 5,
                                    10, 8,   9, 5,  12, 8,  18, 2 };

   /* Make two passes attempting to layout the items.  The first time we will use */
   /* Whatever the prevailing font is.  If this doesn't work, we will switch to a */
   /* Simple TOPAZ80 and try again.  Only if that fails will we give up.          */
   for(i = 2; i >= 0; i--)
   {
      int dwid,nwid;
      global.cycsize[0] = global.cycsize[1] = 0;
      global.titsize[0] = global.titsize[1] = 0;
      global.stxtwid = text_width("0");
      nwid = text_width(global.button[BUTTON_NEW].title);
      dwid = text_width(global.button[BUTTON_DEL].title);
      global.lgadwid = (dwid > nwid ? dwid : nwid) + PROP_WIDTH + 3*DVBAR;

      fudgesize = 0;
      get_cycle_sizes(global.objects, global.titsize, global.cycsize, &fudgesize);
      if ((global.cycsize[0] + global.titsize[0]) < fudgesize)
         global.cycsize[0] = fudgesize - global.titsize[0];

      fudgesize = 0;
      for(groups = global.groups; groups != NULL;
          groups = (struct G_GROUP *)groups->base.next)
      {
         get_cycle_sizes(groups->objects, global.titsize+1, global.cycsize+1,
                         &fudgesize);
         if (global.cycsize[1] < (text_width(groups->base.title)
                                    + CYC_ICON_WIDTH + 2*DVBAR))
            global.cycsize[1] = text_width(groups->base.title)
                                    + CYC_ICON_WIDTH + 2*DVBAR;
      }
      if ((global.cycsize[1] + global.titsize[1]) < fudgesize)
         global.cycsize[1] = fudgesize - global.titsize[1];      

      global.iheight = global.ri.TextAttr.ta_YSize+DHBAR;
      if (global.iheight < global.ri.FontSize)
         global.iheight = global.ri.FontSize;
      if (i)
      {
         global.iheight += 4; /* Account for decent margins on the rendering */
         global.eheight = global.ri.FontSize + 2*DHBAR;
         global.listextra = 0;
      }
      else
      {
         global.iheight += 2; /* Account for decent margins on the rendering */
         global.eheight = global.ri.FontSize + DHBAR + HBAR;
         global.listextra = 1;
      }

      /* We have the totals for everything, let's figure out how */
      /* big the final thing is going to be.                     */
      height = ((global.maxsize+1)*global.iheight) +
               2*global.listextra +
               global.ri.WindowTitle +
               global.ri.WindowBottom + MARGIN_MID;
      width  = global.cycsize[0]+global.cycsize[1] +
               global.titsize[0]+global.titsize[1] +
               global.ri.WindowLeft +
               global.ri.WindowRight +
               MARGIN_SEP + MARGIN_LEFT + MARGIN_RIGHT;

      if ((height <= global.ri.ScreenHeight) &&
          (width  <= global.ri.ScreenWidth)) break;

      if (i==0)
      {
         /* We couldn't fit what they wanted on the screen, so error out with */
         /* A message indicating the problem.  We really shouldn't see this   */
         /* happen much, but at least account for it.                         */
         sprintf(global.title, "Bad Layout W:%d>%d H:%d>%d",
                                width, global.ri.ScreenWidth,
                                height, global.ri.ScreenHeight);
         return(1);
      }
      /* Set up so we try with Topaz 80 as our default font the second time around */
      global.ri.TextAttr = TOPAZ80;
   }

   /* Now we have calculated sizes for the objects, layout the border structures */
   for(i = 0; i < 2; i++)
   {
      /* Create a border for the cycle gadgets */
      global.cycborder[i] = build_border(global.cycsize[i],
                                          global.iheight, MODE_OUT);
      /* We also need to put in the vectors for the cycle picture */
      border = (struct Border *)get_mem(4*sizeof(struct Border));
      if (border == NULL) return(1);

      border[0].NextBorder = border+1;
      border[1].NextBorder = border+2;
      border[2].NextBorder = global.cycborder[i];
      global.cycborder[i] = border;

      border[0].DrawMode = JAM1;
      border[0].Count  = sizeof(Cycle_Vectors)/(2*sizeof(short));
      border[0].XY = Cycle_Vectors;
      border[0].FrontPen = 1;

      border[1].DrawMode = JAM1;
      border[1].Count  = sizeof(Cycle_Line1)/(2*sizeof(short));
      border[1].XY = Cycle_Line1;
      border[1].FrontPen = global.ri.Shadow;

      border[2].DrawMode = JAM1;
      border[2].Count  = sizeof(Cycle_Line2)/(2*sizeof(short));
      border[2].XY = Cycle_Line2;
      border[2].FrontPen = global.ri.Highlight;

      global.checkborder[i] = border+3;
      border[3].NextBorder = build_border(CHECK_WIDTH, CHECK_HEIGHT, MODE_OUT);
      border[3].DrawMode = JAM1;
      border[3].Count = sizeof(Check_Vectors)/(2*sizeof(short));
      border[3].XY = Check_Vectors;
      border[3].FrontPen = i;
   }

   /* Adjust the height of the cycle vectors to the current height */
   /* See the picture above for these values.  Yes, they are hard  */
   /* coded magic values, but equates for them don't make a lot of */
   /* sense in a situation like this.                              */
   for(i = 0; i < 0x0d; i++)
   {
      int extra;

      extra = global.iheight - 13;
      if (i > 8)
      {
         extra >>= 1;
      }
      else if ((i > 6) || (i == 5))
      {
         extra = 0;
      }

      Cycle_Vectors[1+i*2] += extra;
   }

   Cycle_Line2[3] = Cycle_Line1[3] = (global.iheight - 1) - DHBAR;

   {
      int bheight;
      bheight = (MAX_LIST*global.eheight) - (2*ARR_HEIGHT) + global.listextra;
      border = build_border(PROP_WIDTH, bheight, MODE_OUT);
      if (border == NULL) return(1);

      border->TopEdge -= bheight;
      border->NextBorder->TopEdge -= bheight;
   }

   global.arrowborder[0].NextBorder = build_border(PROP_WIDTH, ARR_HEIGHT, MODE_OUT);
   global.arrowborder[1].NextBorder = border;
   border->NextBorder->NextBorder = global.arrowborder[0].NextBorder;

   global.arrowborder[0].DrawMode = JAM1;
   global.arrowborder[0].Count  = sizeof(Dn_Vectors)/(2*sizeof(short));
   global.arrowborder[0].XY = Dn_Vectors;
   global.arrowborder[0].FrontPen = 1;

   global.arrowborder[1].DrawMode = JAM1;
   global.arrowborder[1].Count  = sizeof(Up_Vectors)/(2*sizeof(short));
   global.arrowborder[1].XY = Up_Vectors;
   global.arrowborder[1].FrontPen = 1;

   /* Everything worked out fine, let us set the height of the window */
   global.width = width;
   global.height = height;

   /* Now sizes for special string gadgets                      */
   global.titsize[2] = text_width(global.text[TEXT_FILEREQ]);
   global.cycsize[2] = global.width - global.ri.WindowLeft
                        - global.ri.WindowRight - 100;

   return(0);
}

/***********************************************************************************
 * Procedure: get_cycle_sizes
 * Synopsis:  get_cycle_sizes(objlist, &titsize, &cycsize)
 * Purpose:   Calculate the base sizes for different types of objects.
 *            We need to calculate the width for CYCLE titles and size for cycles
 *            Everything else has a fixed size to be used anyway.
 ***********************************************************************************/
void get_cycle_sizes(struct G_OBJECT *objlist,
                    int *titsize,
                    int *cycsize,
                    int *fudgesize)
{
   int wid;
   struct G_OBJECT *obj;
   struct G_CYCLE *cyc;
   struct G_VALUE *val;

/*
   *titsize -= DVBAR;
   *cycsize -= CYC_ICON_WIDTH+DVBAR;
*/

   /* First we need to lay out the gadgets on the left side of the window */
   for(obj = objlist; obj != NULL; obj = obj->next)
   {
      switch(obj->class)
      {
         case CLASS_CYCLE:
            /* go through and get all the cycle values */
            cyc = (struct G_CYCLE *)obj;
            set_cyc_state(cyc, cyc->values);
            for(val = cyc->values; val != NULL; val = val->next)
            {
               wid = text_width(val->title) + CYC_ICON_WIDTH+DVBAR;
               if (val->string)
               {
                  int num;
                  num = string_width(val->option, 3);
                  wid += (global.stxtwid * num) + DVBAR;
               }
               if (wid > *cycsize) *cycsize = wid;
            }
            wid = text_width(obj->title) + DVBAR;
            if (wid > *titsize) *titsize = wid;
            break;
         case CLASS_STRING:
            wid = text_width(obj->title);
            if (wid > *titsize) *titsize = wid;
            /* provide a reasonable default string entry area */
            wid = string_width(((struct G_STRING *)obj)->option, 10);
            wid = wid * global.stxtwid + 2*DVBAR;
            if (wid > *cycsize) *cycsize = wid;
            break;
         case CLASS_LIST:
            wid = text_width(obj->title);
            if (wid > *fudgesize) *fudgesize = wid;
            wid = string_width(((struct G_LIST *)obj)->option, 10);
            wid = wid * global.stxtwid + global.lgadwid;
            if (wid > *fudgesize) *fudgesize = wid;
            break;
         case CLASS_CHECK:
            wid = text_width(obj->title) + CHECK_WIDTH + 2*DVBAR;
            if (wid > *fudgesize) *fudgesize = wid;
            break;
      }
   }
/*
   *titsize += DVBAR;
   *cycsize += CYC_ICON_WIDTH+DVBAR;
*/
}

/***********************************************************************************
 * Procedure: text_width
 * Synopsis:  len = text_width(str)
 * Purpose:   Return the rendered width of a given string
 ***********************************************************************************/
int text_width(char *str
              )
{
   struct IntuiText itext;

   itext.FrontPen  = 1;
   itext.BackPen   = 0;
   itext.DrawMode  = JAM1;
   itext.LeftEdge  = 0;
   itext.TopEdge   = 1;
   itext.ITextFont = &global.ri.TextAttr;
   itext.NextText  = NULL;
   itext.IText     = str;

   return(IntuiTextLength(&itext));
}

/**********************************************************************************
 * Procedure: string_width
 * Synopsis:  len = string_width(str)
 * Purpose:   Return the size of a string specification %ns, default n = 3
 *********************************************************************************/
int string_width(char *str, int def_width)
{
   char *p;
   int num;

   p = strchr(str, '%');
   num = 0;
   if (p)
   {
      p++;
      while ((*p >= '0') && (*p <= '9'))
         num = (num * 10) + *p++ - '0';
   }
   
   if (num <= 0) num = def_width;
   return (num + 1);
}
