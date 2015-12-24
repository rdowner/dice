#include "vmake.h"

struct Gadget *oldgad;

Prototype void handle_hit(struct Gadget *gad,int shift, int doubleclick);
Prototype void set_cyc_state(struct G_CYCLE *cyc,struct G_VALUE *val);
Prototype void handle_list(struct G_LIST *list,struct Gadget *gad,int class,char *buf, int doubleclick);
Prototype void redraw_gadget(struct Gadget *gad);
Prototype void recalc_prop(struct G_LIST *list,UWORD *body,UWORD *sltop);
Prototype int  newpos(struct G_LIST *list,int spos,struct G_ENTRY *ent);
Prototype void set_gadlist(struct GADLIST *gadlist,int state);
Prototype void set_gadgets(int state);
Prototype int  get_work_filename(char *gfname, int spat);
Prototype void mark_clean(void);
Prototype int  test_dirty(void);

/***********************************************************************************
 * Procedure: handle_hit
 * Synopsis:  handle_hit(gadget, shift, doubleclick);
 * Purpose:   Handle the processing for the hit on a gadget.
 ***********************************************************************************/
void handle_hit(struct Gadget *gad,
                int    shift,
                int    doubleclick
               )
{
   struct G_OBJECT *obj;

   obj = (struct G_OBJECT *)gad->UserData;
   /* Ignore any hits on disabled gadgets */
   if ((gad->Flags & GADGHIGHBITS) == GADGHNONE) return;

   if (doubleclick)
   {
      if (oldgad != gad)
         doubleclick = 0;
   }
   else
      oldgad = gad;

#ifdef DBG_JGM
printf("Double click state = %d\n", doubleclick);
#endif

   switch(obj->class)
   {
      case CLASS_STRING:
         break;
      case CLASS_CYCLE:
         {
            /* We need to advance the string to the next state */
            /* When we refresh the gadget, we need to first blank it out */
            struct G_CYCLE *cyc;
            struct G_VALUE *val;

            cyc = (struct G_CYCLE *)obj;
            val = cyc->curval;

            if (shift)
            {
               for(val = cyc->values;
                   (val->next != NULL) && (val->next != cyc->curval);
                   val = val->next);
            }
            else
            {
               val = val->next;
               if (val == NULL) val = cyc->values;
            }
            set_cyc_state(cyc, val);
            obj->state |= DIRTY_BIT;
         }
         break;
      case CLASS_CHECK:
         obj->state = ((gad->Flags & SELECTED) != 0) & DIRTY_BIT;
         break;
      case CLASS_LIST:
         handle_list((struct G_LIST *)obj, gad, gad->GadgetID, NULL, doubleclick);
         break;
      case CLASS_BUTTON:
         if (!doubleclick) /* don't edit or compile twice for a double click */
            do_command(global.button[obj->state & STATE_MASK].command);
         break;
      default:
         break;
   }
}
/***********************************************************************************
 * Procedure: redraw_gadget
 * Synopsis:  redraw_gadget(gadget);
 * Purpose:   Rerender a gadget on the screen
 ***********************************************************************************/
void redraw_gadget(struct Gadget *gad
                  )
{
   SetBPen(global.rp, 0);
   SetAPen(global.rp, 0);
   RectFill( global.rp, gad->LeftEdge + VBAR, gad->TopEdge + HBAR,
                         gad->LeftEdge + gad->Width - 1 - VBAR,
                         gad->TopEdge + gad->Height - 1 - HBAR);
   RefreshGList( gad, global.window, NULL, 1);
}

/***********************************************************************************
 * Procedure: set_cyc_state
 * Synopsis:  set_cyc_state(cyc,val)
 * Purpose:   Set a cycle gadget to a give value
 ***********************************************************************************/
void set_cyc_state(struct G_CYCLE *cyc,
                   struct G_VALUE *val
                  )
{
   struct G_STRING *str;
   struct Gadget *gad;

   /* See It is currently being displayed on the screen.  If so, we will have to */
   /* do some cleanup work to get it updated.                                    */
   str = cyc->curval->string;
   if ((str != NULL) && (str->base.gadget != NULL))
   {
      /* If there is a string gadget currently in the cycle gadget, we need */
      /* to free it up                                                      */
      RemoveGList( global.window, str->base.gadget, 1);
      free_gadget(str->base.gadget);
   }

   if (val == NULL) return;

   cyc->curval = val;

   /* Now, if we are currently displaying the gadget, we need to get it on the */
   /* screen.                                                                  */
   if (gad = cyc->base.gadget)
   {
      struct IntuiText *itext;
      struct Gadget *strgad;
      int oldpos;

      itext = gad->GadgetText->NextText;
      itext->IText = val->title;

      strgad = setup_cycle_gadget(gad, itext, val);

      oldpos = RemoveGList(global.window, gad, 1);
      AddGList(global.window, gad, oldpos, 1, NULL);

      if (strgad) AddGadget( global.window, strgad, 0);

      /* Wipe out the inner area of the gadget so that we can redraw it later */
      redraw_gadget(gad);
      if (strgad) redraw_gadget(strgad);
   }
}

/***********************************************************************************
 * Procedure: handle_list
 * Synopsis:  handle_list(list, gadget, int class, buf, doubleclick);
 * Purpose:   Handle the processing for the hit on a list gadget.
 ***********************************************************************************/
void handle_list(struct G_LIST *list,
                 struct Gadget *gad,
                 int class,
                 char *buf,
                 int doubleclick
                )
{
   struct G_ENTRY *ent, *prevent;
   struct PropInfo *pi;
   int expect;
   int docmnd;
   int pos;
   int prevpos;
   int i;
   int oldpos[MAX_LIST];

   static int oldpropidx;

   expect = docmnd = 0;
   pos = -1; /* used to indicate no valid selection */
   if (list->sel) /* we should expect a valid selection */
   {
      expect = 1;
      pos = list->base.state & STATE_MASK;
   }
   prevpos = pos;

   switch(class & CLASS_MASK) /* Isolate class from subclass */
   {
      case CLASS_ADD:
      {
         int nameinbuf = 0;
         /* Note that gad parameter may not be set on entry. In this case */
         /* if buf parm is set, it has name of file to add, otherwise     */
         /* request a name.  No way to enter a name for Dos 1.3 yet.      */
         if (doubleclick) /* Don't put up requester twice */
            return;
         ent = get_mem(sizeof(struct G_ENTRY));
         if (ent == NULL) 
         {
            request(1, TEXT_NOMEM, NULL, NULL);
            return;
         }

         if (buf)
            if (*buf != '\0')
            {
               strcpy(ent->buf, buf);
               nameinbuf = 1;
            }
         if (!nameinbuf)
         {
            if (get_work_filename(ent->buf, CONFIG_FILES) == 0)
            {
               /* User aborted file request, we need to give up too */
               free_mem(ent, sizeof(struct G_ENTRY));
               global.rexxrc = TEXT_CANCEL; /* It's what happened...   */
               return; /* don't mess around with gadgets */
            }
         }

         /* we're committed - make sure we select it, whereever we put it */
         expect = 1;
         /* now link the new name into the list */
         if (list->first == NULL)
         {
            int oldpos;
            oldpos = RemoveGList(global.window, list->delgad, 1);
            list->delgad->Flags   &= ~GADGDISABLED;
            AddGList(global.window, list->delgad, oldpos, 1, NULL);
            redraw_gadget(list->delgad);
            list->top = list->first = ent;
            pos = 0; /* select the entry we just added */
         }
         else
         {
            if (pos == -1)  /* if nothing selected, add at top of */
               pos = 0;     /* display - and select it            */
            prevent = list->top;
            for (i = 0; (i < pos) && (prevent != NULL); i++)
               prevent = (struct G_ENTRY *)prevent->base.next;
            /* Insert in front of prevent */
            ent->base.next = (struct G_OBJECT *)prevent;
            if (prevent)
            {
               ent->base.prev = prevent->base.prev;
               prevent->base.prev = (struct G_OBJECT *)ent;
            }
            if (ent->base.prev)
            {
               if (list->top == prevent) list->top = ent;
               ent->base.prev->next = (struct G_OBJECT *)ent;
            }
            else
            {
               list->top = list->first = ent;
            }
            expect = 1;
         }
         /* the list has been changed */
         list->base.state |= DIRTY_BIT;
         break;
      }
      case CLASS_DEL:
         if (doubleclick) /* don't delete two objects by mistake... */
            return;
         if (pos == -1)   /* nothing selected - don't pick one at random...*/
            return;
         ent = list->top;
         for (i = 0; (i < pos) && (ent != NULL); i++)
            ent = (struct G_ENTRY *)ent->base.next;
         /* We want to delete ent */
         if (ent == NULL) return;
         if (ent->base.next)
            ent->base.next->prev = ent->base.prev;
         if (ent->base.prev)
            ent->base.prev->next = ent->base.next;
         else
            list->first = (struct G_ENTRY *)ent->base.next;

         /* Update the base pointer so that we are looking at the right space */
         if (list->top == ent)
         {
            list->top = (struct G_ENTRY *)ent->base.next;
            if (list->top == NULL)
               list->top = (struct G_ENTRY *)ent->base.prev;
         }
         free_mem(ent, sizeof(struct G_ENTRY));
         /* the list has been changed */
         list->base.state |= DIRTY_BIT;
         /* and note that we fall through to set state of del gadget */
         goto dodelgad;
         /* except that we want to leave the next item selected - it */
         /* will have moved up to the display position just vacated. */ 
      case CLASS_REFRESH:
         expect = 0;

dodelgad:
         {
            int oldpos;

            oldpos = RemoveGList(global.window, list->delgad, 1);
            if (list->first == NULL)
               list->delgad->Flags   |=  GADGDISABLED;
            else
               list->delgad->Flags   &= ~GADGDISABLED;
            AddGList(global.window, list->delgad, oldpos, 1, NULL);
            redraw_gadget(list->delgad);
         }
         RefreshGList( list->delgad, global.window, NULL, 5);
         break;
      case CLASS_UP:
         expect = 0; /* trying to move deselects */
         if ((list->top != NULL) && (list->top->base.prev != NULL))
         {
            list->top = (struct G_ENTRY *)list->top->base.prev;
         }
         break;
      case CLASS_DOWN:
         {
            struct G_ENTRY *ent;
            expect = 0; /* trying to move deselects */
            if ((list->top != NULL) && (list->top->base.next != NULL))
            {
               list->top = (struct G_ENTRY *)list->top->base.next;
            }
            ent = list->first;
            i = list->maxent - global.listsize;
            while (ent && i)
            {
               if ((ent = (struct G_ENTRY *)ent->base.next) == list->top) 
                  break;
               i--;
            }
            if ((i == 0) && (ent != NULL))
               list->top = ent;
         }
         break;
      case CLASS_PROP:
         {
            int maxtop;
            expect = 0; /* trying to move deselects */
            pi = (struct PropInfo *)gad->SpecialInfo;
            maxtop = list->maxent - global.listsize;
            i = (pi->VertPot*(2*maxtop))/MAXBODY;
            i = (i + 1)/ 2; /* rounding for more natural button behavior */
            if (i > maxtop) i = maxtop;
            if ((global.mouseprop) && (i == oldpropidx))
            {
               /* don't want to refresh the list unless the view has     */
               /* changed.  Have to refresh the prop gadget or it decays */
               /* all over the border.                                   */
               UWORD body, sltop;
               recalc_prop(list, &body, &sltop);
               NewModifyProp(list->slider, global.window, NULL,
                             AUTOKNOB | FREEVERT | PROPBORDERLESS,
                             0, sltop, MAXBODY, body, 1L);
               return;
            }
            else
               oldpropidx = i;   
            list->top = list->first;
            while((i > 0) && (list->top != NULL))
            {
               list->top = (struct G_ENTRY *)list->top->base.next;
               i--;
            }
            if (list->top == NULL) list->top = list->first;
         }
         break;
      case CLASS_LIST:
         pos = class >> SUBCLASS_OFF;
         expect = 1;
         if (doubleclick)
            docmnd = 1; /* user double clicked on a list entry         */
         break;
      case CLASS_SELECT:
         {
            int spos;
            int simplename;
            struct G_ENTRY *ent;

            expect = 0; /* signal nothing to select                    */
            pos = -1; /* nothing selected                              */
            if (buf == NULL) /* no parameter - nothing to do           */
               break;
            if (*buf == '\0') /* empty string not a valid list entry   */
               break;
            /* set pathname true if path separators present, false (or */
            /* NULL) otherwise                                         */
            simplename = (strpbrk(buf, ":/") == NULL);
            ent = list->first;
            spos = 0;
            while (ent)
            {
               char *fnp, *tp;
               fnp = ent->buf;
               if (simplename)
               /* want to match simple name - ignore any path in list entry */
               {
                  tp = strrchr(ent->buf, '/');
                  if (tp == NULL) /* no slash, try for a colon         */
                     tp = strrchr(ent->buf, ':');
                  if (tp != NULL) /* strip off the whole path          */
                     fnp = tp + 1;
               }
               if (stricmp(fnp, buf))
               {
                  /* does not match, step through list                 */
                  ent = (struct G_ENTRY *)ent->base.next;
                  spos++;
               }
               else
                  break;
            }
            if (ent == NULL) /* didn't find a match in the list        */
               break;
            /* we're selected something, now figure out where it goes  */
            expect = 1;
            pos = newpos(list, spos, ent);
         }
         break;
      case CLASS_LTOP:
      case CLASS_LBOT:
         int spos;
         struct G_ENTRY *ent;

         ent = list->first; /* tricky: not needed when going to bottom */
         if ((class & CLASS_MASK) == CLASS_LTOP)
            spos = 0;
         else
            spos = list->maxent-1;
         expect = 1;
         pos = newpos(list, spos, ent);
         break;
      case CLASS_LUP:
      case CLASS_LDN:
         int spos;
         struct G_ENTRY *ent, *sent;

         sent = list->sel;
         if ((class & CLASS_MASK) == CLASS_LUP)
            sent = (struct G_ENTRY *)sent->base.prev;
         else
            sent = (struct G_ENTRY *)sent->base.next;
         spos = 0;
         ent = list->first;
         while (ent)
         {
            if (ent == sent)
               break;
            ent = (struct G_ENTRY *)ent->base.next;
            spos++;
         }
         expect = 1;
         pos = newpos(list, spos, ent);
         break;
      default:
         printf("Unexpected list gadget class 0x%x\n", class);
         break;
   }

   list->sel = NULL;  /* turn off selection & see if we reselect */

   gad = list->base.gadget;

   if (expect)
   {
      /* Make sure that we are actually in a position to enable the */
      /* requested string gadget.  If not, just ignore the request  */
      /* Start from the top of the display and see if there's an    */
      /* entry at the requested position.                           */
      ent = list->top;
      for(i = pos; ent && (i > 0); i--)
         ent = (struct G_ENTRY *)ent->base.next;

      if (ent)
      {
         list->sel = ent;
         }
      else
      {
         pos = -1;
         /* clear out the selected string gadget */
         list->base.state = list->base.state & DIRTY_BIT;
      }
   }
   else
   {
      pos = -1;
   }
   /* point to selected entry gadget if any */
   list->base.state = (list->base.state & DIRTY_BIT) + (pos & STATE_MASK);

   /* handle refreshing of LIST hit differently to reduce screen flash   */
   /* and improve response to double click (remove gadget for less time) */
   if ((class & CLASS_MASK) != CLASS_LIST)
   {
      for(i = global.listsize-1; i >= 0; i--)
      {
         oldpos[i] = RemoveGList(global.window, list->btngad[i], 2);
      }
      reset_list_object(list, pos);

      /* Wipe out the inner area of the gadget so that we can redraw it later */
      redraw_gadget(gad);

      for(i = 0; i < global.listsize; i++)
      {
         AddGList(global.window, list->btngad[i], oldpos[i], 2, NULL);
         RefreshGList( list->btngad[i], global.window, NULL, 2);
      }
   }
   else
   {
      if ((prevpos != -1) && (prevpos != pos))
      /* redraw old button gadget to wipe out the border */
      {
         int oldpos;
         struct Gadget *bgad = list->btngad[prevpos];
         oldpos = RemoveGList(global.window, list->btngad[prevpos], 2);
         list->strgad[prevpos]->GadgetRender = NULL;
         /* standard redraw() misses the edges of the border *sigh*       */
         /* this sizing is a bit slimy and ad hoc but it seems to work... */
         SetBPen(global.rp, 0);
         SetAPen(global.rp, 0);
         RectFill( global.rp, bgad->LeftEdge, bgad->TopEdge + 1,
                   bgad->LeftEdge + bgad->Width - 1,
                   bgad->TopEdge + bgad->Height - 2 + global.listextra);
         AddGList(global.window, list->btngad[prevpos], oldpos, 2, NULL);
         RefreshGList( list->btngad[prevpos], global.window, NULL, 2);     
      }
      if (pos != -1)
      /* draw border for newly selected list gadget */
      {
         int oldpos;
         oldpos = RemoveGList(global.window, list->btngad[pos], 2);
         list->strgad[pos]->GadgetRender = list->sborder;
         AddGList(global.window, list->btngad[pos], oldpos, 2, NULL);
         RefreshGList( list->btngad[pos], global.window, NULL, 2);
      }
   }

/* remove to prevent typing into gadget
   if (pos >= 0)
      ActivateGadget(list->strgad[pos], global.window, NULL);
*/

   {
      UWORD body, sltop;
      recalc_prop(list, &body, &sltop);
      NewModifyProp(list->slider, global.window, NULL,
                    AUTOKNOB | FREEVERT | PROPBORDERLESS,
                    0, sltop, MAXBODY, body, 1L);
   }

   if (docmnd && list->sel) /* double clicked on a valid entry */
      do_command(global.text[CONFIG_DCLICK]);
}
/***********************************************************************************
 * Procedure: recalc_prop
 * Synopsis:  recalc_prop(list)
 * Purpose:   Calculate the appropriate BODY and TOP values for a List Prop gadget
 ***********************************************************************************/
void recalc_prop(struct G_LIST *list,
                 UWORD *body, UWORD *sltop
               )
{
   int count, top, maxtop;
   struct G_ENTRY *ent;

   count = top = 0;
   ent = list->first;

   while(ent != NULL)
   {
      if (ent == list->top) top = count;
      count++;
      ent = (struct G_ENTRY *)ent->base.next;
   }

   list->maxent = count;

   *body  = MAXBODY;

   *sltop = 0;
   if (count > global.listsize)
   {
      *body  = (MAXBODY * global.listsize) / count;
      maxtop = count - global.listsize;
      if (top > maxtop) top = maxtop;
      *sltop = (MAXBODY * top) / maxtop;
   }

}
/***********************************************************************************
 * Procedure: newpos
 * Synopsis:  newpos(list, spos, ent)
 * Purpose:   set display position for a new list selection
 *            caller knows both entry & index - why recalculate?
 ***********************************************************************************/
int  newpos(struct G_LIST *list,int spos,struct G_ENTRY *ent)
{
   int j, pos, dtop, maxtop;
   struct G_ENTRY *dent;

   maxtop = list->maxent - global.listsize;

   /* Now find the index of the top of the visible display      */
   dent = list->first;
   dtop = 0;
   while (dent)
   {
      if (dent == list->top)
      /* We've reached the start of the visible display         */
         break;
      else
      /* keep counting                                          */
      {
         dent = (struct G_ENTRY *)dent->base.next;
         dtop++;
      }
   }
   if ((spos >= dtop) && ((spos - dtop) < global.listsize))
   /* selection is within range of display                      */
   {
      pos = spos - dtop;
   }
   else if (spos <= maxtop)
   /* can move display to start with selected item              */
   {
      list->top = ent;
      pos = 0;
   }
   else
   /* find the lowest possible entry for the top of the display */
   {
      for (j = dtop; j < maxtop; j++)
         list->top = (struct G_ENTRY *)list->top->base.next;
      pos = spos - maxtop;
   }

   return pos;            
}
/***********************************************************************************
 * Procedure: set_gadlist
 * Synopsis:  set_gadlist(gadlist,on/off)
 * Purpose:   Enable or Disable all the gadgets in a list
 ***********************************************************************************/
void set_gadlist(struct GADLIST *gadlist,
                 int state
                )
{
   struct Gadget *gad;
   struct G_CYCLE *cyc;
   int count;

   if ((global.window == NULL) ||
       (gadlist == NULL))
      return;

   if (state)
   {
      AddGList( global.window, gadlist->gadgets, 30000, gadlist->count, NULL);
   }
   else
   {
      RemoveGList(global.window, gadlist->gadgets, gadlist->count);
   }

   for(gad = gadlist->gadgets, count=gadlist->count;
       gad && count;
       gad = gad->NextGadget, count--)
   {
      cyc = (struct G_CYCLE *)gad->UserData;

      if ((cyc->base.class & CLASS_MASK) == CLASS_CYCLE)
      {
         set_cyc_state(cyc, state ? cyc->curval : NULL);
      }
      else if (state)
      {
         if ((cyc->base.class & CLASS_MASK) == CLASS_LIST)
         {
            if (gad == cyc->base.gadget)
               handle_list((struct G_LIST *)cyc, NULL, CLASS_REFRESH, NULL, 0);
         }
         else
         {
             RefreshGList(gad, global.window, NULL, 1);
         }
      }
   }
}

/***********************************************************************************
 * Procedure: set_gadgets
 * Synopsis:  set_gadgets(on/off)
 * Purpose:   Enable or Disable all gadgets
 ***********************************************************************************/
void set_gadgets(int state)
{
   static int gadliststate;

   if (state == gadliststate)
      /* adding or removing twice in a row causes problems */
      return;
   else
   {
      gadliststate = state;
      set_gadlist(global.gadlist,    state);
   }
}

/***********************************************************************************
 * Procedure: get_work_filename
 * Synopsis:  type = get_work_filename(gfname, spat)
 * Purpose:   Parse out a file descriptor and return a type for that name
 *            gfname points to a buffer to receive the name.
 *            spat is the index of a search pattern entry in global.text[]
 ***********************************************************************************/
int get_work_filename(char *gfname, int spat)
{
   char *p;
   int len;
   int n;

   if (global.inrexx)
   {
      /* whoever called us should have had a filename parameter */
      global.rexxrc = TEXT_BADPARM;
      return 0;
   }

   if (global.freq == NULL) return(0);

   set_workdir();
   set_busy();

   if (AslBase != NULL)
   {
      struct TagItem taglist[9];

      n = 0;

      taglist[n  ].ti_Tag  = ASL_Pattern;
      taglist[n++].ti_Data = (ULONG)global.text[spat];

      taglist[n  ].ti_Tag  = ASL_File;
      taglist[n++].ti_Data = (ULONG)"";

      taglist[n  ].ti_Tag  = ASL_Dir;
      taglist[n++].ti_Data = (ULONG)"";

      taglist[n  ].ti_Tag  = ASL_Window;
      taglist[n++].ti_Data = (ULONG)global.window;

#ifdef ASLFR_DoSaveMode
      taglist[n  ].ti_Tag  = ASLFR_DoSaveMode;
      taglist[n++].ti_Data = FALSE;
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
      {
         set_idle();
         return(0);
      }
   }
   else
   {
       /* We must have arp.library (otherwise we wouldn't have a file     */
       /* Requester structure.)                                           */
       global.freq->rf_Dir[0] = 0;
       global.freq->rf_File[0] = 0;
       if (!ArpFileRequest( global.freq ))
       {
          set_idle();
          return(0);
       }
   }
   set_idle();

   n = strlen(global.freq->rf_Dir);
   if (n > 255) n = 255;
   p = gfname;
   memcpy(p, global.freq->rf_Dir, n);
   p += n;

   if ((n > 0) && (p[-1] != ':'))
   {
      *p++ = '/';
      n++;
   }
   len = strlen(global.freq->rf_File);
   if (len == 0) return 0; /* null name string equivalent to cancelling */
   if ((n + len) > 255) len = 255-n;
   strncpy(p, global.freq->rf_File, len);
   p[len] = 0;

   if (!stricmp(gfname, "ENV"))
      return(FILE_ENV);

   len = strlen(gfname);
   p = gfname + strlen(gfname);

   if (len < 2)
      return(FILE_OPTIONS);

   if ((len >= 7) && (!stricmp(p-7, "DCCOPTS")))
      return(FILE_OPTIONS);

   if ((len >= 9) && (!stricmp(p-9, "DMAKEFILE")))
      return(FILE_DMAKEFILE);

   if (!stricmp(p-2, ".H"))
      return(FILE_C);

   if (!stricmp(p-2, ".C"))
      return(FILE_C);

   return(FILE_OPTIONS);
}


/***********************************************************************************
 * Procedure: mark_clean
 * Synopsis:  mark_clean()
 * Purpose:   reset DIRTY bit for all string objects
 ***********************************************************************************/
void mark_clean(void)
{
   struct G_OBJECT *obj;
   struct G_STRING *strobj;
   #define cycobj ((struct G_CYCLE *)obj)

   obj = global.objects;
   while (obj)
   {
      obj->state &= STATE_MASK; /* reset dirt bit for this object */
      if (obj->class == CLASS_STRING)
         strobj = (struct G_STRING *)obj;
      else
      /* see if it's a cycle which might have a string sub-object */
      {
         if (obj->class == CLASS_CYCLE)
            /* strobj may end up NULL.  That's fine               */
            strobj = cycobj->curval->string;
         else
            strobj = NULL;
      }
      if (strobj)
      {
         /* checkpoint present string - safely                    */
	 strncpy(strobj->clean_buf, strobj->buf, MAX_STRING);
      }
      obj = obj->next;
   }
   global.dirtysym = 0; /* flags externally defined symbols       */
   #define cycobj
}
/***********************************************************************************
 * Procedure: test_dirty()
 * Synopsis:  bool = test_dirty()
 * Purpose:   set dirty bits for string objects and return true if any dirty
 *            objects are found
 ***********************************************************************************/
int test_dirty(void)
{
   struct G_OBJECT *obj;
   struct G_STRING *strobj;
   int dirty = 0;

   obj = global.objects;
   while (obj)
   /* we want to look at everything, not exit on the first dirty object    */
   /* because this routine also flags any dirty string (sub)objects        */
   {
      if (obj->class == CLASS_STRING)
         strobj = (struct G_STRING *)obj;
      else
      {
         /* if it's a cycle, and we haven't changed the pointer, and this  */
         /* entry in the cycle has a string gadget, see if the string has  */
         /* changed.                                                       */
         if ((obj->class == CLASS_CYCLE) && !(obj->state & DIRTY_BIT))
            strobj = ((struct G_CYCLE *)obj)->curval->string;
         else
            strobj = NULL;
      }
      if (strobj)
         if (strcmp(strobj->buf, strobj->clean_buf) != 0)
            /* not the same as the snapshot last time mark_clean() went by */
            obj->state |= DIRTY_BIT; /* easy for anyone else to see now    */

      /* whether string object or not, see if it's dirty */
      if (obj->state & DIRTY_BIT)
         dirty = DOSTRUE;
      obj = obj->next;
   }

   if (global.dirtysym) /* has user set any private symbols?               */
      dirty = DOSTRUE;

   return dirty;
}
