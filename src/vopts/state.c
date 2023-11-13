/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include "vopts.h"

Prototype void handle_hit(struct Gadget *gad,int shift);
Prototype void set_cyc_state(struct G_CYCLE *cyc,struct G_VALUE *val);
Prototype void handle_list(struct G_LIST *list,struct Gadget *gad,int class);
Prototype void redraw_gadget(struct Gadget *gad);
Prototype void recalc_prop(struct G_LIST *list,UWORD *body,UWORD *sltop);
Prototype void set_gadlist(struct GADLIST *gadlist,int state);
Prototype void set_gadgets(int state);

/***********************************************************************************
 * Procedure: handle_hit
 * Synopsis:  handle_hit(gadget);
 * Purpose:   Handle the processing for the hit on a gadget.
 ***********************************************************************************/
void handle_hit(struct Gadget *gad,
                int    shift
               )
{
   struct G_OBJECT *obj;

   obj = (struct G_OBJECT *)gad->UserData;
   /* Ignore any hits on disabled gadgets */
   if ((gad->Flags & GADGHIGHBITS) == GADGHNONE) return;

   switch(gad->GadgetID)
   {
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
         }
         break;
      case CLASS_CHECK:
         obj->state = (gad->Flags & SELECTED) != 0;
         break;
      case CLASS_STRING:
         {
         struct StringInfo *si;
         si = (struct StringInfo *)gad->SpecialInfo;
         if ((si->Buffer[0]) || (obj->class != CLASS_LIST)) break;
         }
      case CLASS_DEL:
      case CLASS_LIST:
      case CLASS_LIST1:
      case CLASS_LIST2:
      case CLASS_ADD:
      case CLASS_UP:
      case CLASS_DOWN:
      case CLASS_PROP:
         handle_list((struct G_LIST *)obj, gad, gad->GadgetID);
         break;
      case CLASS_GROUP:
         /* We need to advance to the next valid group */
         {
         int scope;
         switch (global.filetype)
         {
            case FILE_OPTIONS:
            case FILE_C:
            case FILE_DMAKEFILE:
               scope = 2;   /* note that global.filetype cannot == 2 */
               break;
            default:
               /* FILE_ENV, uninitialized or whatever */
               scope = 1;   /* to skip over non global options       */
         }
         do
         {
            if (shift)
            {
               global.curgroup = (struct G_GROUP *)global.curgroup->base.prev;
               if (global.curgroup == NULL)
               {
                  global.curgroup = global.groups;
                  while(global.curgroup->base.next != NULL)
                     global.curgroup = (struct G_GROUP *)global.curgroup->base.next;
               }
            }
            else
            {
               global.curgroup = (struct G_GROUP *)global.curgroup->base.next;
               if (global.curgroup == NULL) global.curgroup = global.groups;
            }
         /* accept anything for options, c or dmakefile,  */
         /* but only local for env                        */
         } while (global.curgroup->local == scope);
         set_group_gadgets();
         break;
         }
      case CLASS_BUTTON:
         switch(obj->state)
         {
            case BUTTON_SAVE:
               /* Save the defaults permanently */
               do_command("SAVE");
               global.done = 1;
               break;
            case BUTTON_USE:
               /* Use as the current defaults */
               do_command("SAVE ENV");
               global.done = 1;
               break;
            case BUTTON_CANCEL:
               /* Just quit and don't do anything */
               global.done = 1;
               break;
            case BUTTON_FRSAVE:
            {
               static char command[MAX_STRING+5];

               /* remove default file requester   */
               clear_fr_gadgets();
               /*copy string to global.filename and continue file operation */
               strcpy(global.filename, global.frstring.buf);

#ifdef JGM_DBG
printf("global.fileop = %ld\n", global.fileop);
#endif
               if (global.fileop == 0)   /* READ in progress */
                  strcpy(command, "READ ");
               else
               {
                  if (global.fileop == 1)
                     strcpy(command, "SAVE ");
                  else
                     return;
               }
#ifdef JGM_DBG
printf(">>%s<<\n", command);
#endif
               strcat(command, global.frstring.buf);
#ifdef JGM_DBG
printf(">>%s<<\n", command);
#endif
               global.nameoffile = 0; /* name may now be unsafe to write */
               do_command(command);
               break;
            }
            case BUTTON_FRCNCL:
               /* clear file requester and do nothing */
               clear_fr_gadgets();
               break;
         }
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
 * Synopsis:  handle_list(list, gadget, int class);
 * Purpose:   Handle the processing for the hit on a list gadget.
 ***********************************************************************************/
void handle_list(struct G_LIST *list,
                 struct Gadget *gad,
                 int class
                )
{
   struct G_ENTRY *ent, *prevent;
   struct PropInfo *pi;
   int expect;
   int pos;
   int i;
   int oldpos[MAX_LIST];

   expect = 0;
   if (list->string)
   {
      expect = 1;
   }

   pos = list->base.state;

   switch(class)
   {
      case CLASS_LIST:
      case CLASS_LIST1:
      case CLASS_LIST2:
         pos = class - CLASS_LIST;
         expect = 1;
         break;
      case CLASS_ADD:
         ent = get_mem(sizeof(struct G_ENTRY));
         if (ent == NULL) return;
         if (list->first == NULL)
         {
            int oldpos;
            oldpos = RemoveGList(global.window, list->delgad, 1);
            list->delgad->Flags   &= ~GADGDISABLED;
            AddGList(global.window, list->delgad, oldpos, 1, NULL);
            redraw_gadget(list->delgad);
         }
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
         break;
      case CLASS_STRING:
         {
         struct StringInfo *si;
         si = (struct StringInfo *)gad->SpecialInfo;
         if (si->Buffer[0]) break;
         }
      case CLASS_DEL:
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
      case CLASS_REFRESH:
         {
            int oldpos;

            expect = 0;
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
         if ((list->top != NULL) && (list->top->base.prev != NULL))
         {
            list->top = (struct G_ENTRY *)list->top->base.prev;
         }
         break;
      case CLASS_DOWN:
         if ((list->top != NULL) && (list->top->base.next != NULL))
         {
            list->top = (struct G_ENTRY *)list->top->base.next;
         }
         break;
      case CLASS_PROP:
         pi = (struct PropInfo *)gad->SpecialInfo;
         list->top = list->first;
         i = (pi->VertPot*list->maxent)/MAXBODY;
         if (i >= list->maxent) i = list->maxent - 1;
         while((i > 0) && (list->top != NULL))
         {
            list->top = (struct G_ENTRY *)list->top->base.next;
            i--;
         }
         if (list->top == NULL) list->top = list->first;
         break;
   }

   list->base.state = pos;

   gad = list->base.gadget;
   list->string = 0;

   if (expect)
   {
      /* Make sure that we are actually in a position to enable the */
      /* requested string gadget.  If not, just ignore the request  */
      ent = list->top;
      for(i = pos; ent && (i > 0); i--)
         ent = (struct G_ENTRY *)ent->base.next;

      if (ent)
         list->string = 1;
      else
      {
         pos = -1;
         list->base.state = 0;
      }
   }
   else
   {
      pos = -1;
   }

   for(i = MAX_LIST-1; i >= 0; i--)
   {
      oldpos[i] = RemoveGList(global.window, list->btngad[i], 2);
   }
   reset_list_object(list, pos);

   /* Wipe out the inner area of the gadget so that we can redraw it later */
   redraw_gadget(gad);

   for(i = 0; i < MAX_LIST; i++)
   {
      AddGList(global.window, list->btngad[i], oldpos[i], 2, NULL);
      RefreshGList( list->btngad[i], global.window, NULL, 2);
   }
   if (pos >= 0)
      ActivateGadget(list->strgad[pos], global.window, NULL);

   {
      UWORD body, sltop;
      recalc_prop(list, &body, &sltop);

#ifdef NewModifyProp
#undef NewModifyProp
#endif
      NewModifyProp(list->slider, global.window, NULL,
                    AUTOKNOB | FREEVERT | PROPBORDERLESS,
                    0, sltop, MAXBODY, body, 1L);
   }
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
   int count, top;
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
   if (count > MAX_LIST)
      *body  = (MAXBODY * MAX_LIST) / count;

   if (count)
      *sltop = (MAXBODY * top     ) / count;
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
#ifdef AddGList
#undef AddGList
#endif
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

      if (cyc->base.class == CLASS_CYCLE)
      {
         set_cyc_state(cyc, state ? cyc->curval : NULL);
      }
      else if (state)
      {
         if (cyc->base.class == CLASS_LIST)
         {
            if (gad == cyc->base.gadget)
               handle_list((struct G_LIST *)cyc, NULL, CLASS_REFRESH);
         }
         else
         {
            if (cyc->base.class == CLASS_CHECK)
            {
               if (cyc->base.state)
               {
                  gad->Flags |= SELECTED;
               }
               else
               {
                  gad->Flags &= ~SELECTED;
               }
            }
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
   set_gadlist(global.gadlist,    state);
   set_gadlist(global.grpgadlist, state);
}
