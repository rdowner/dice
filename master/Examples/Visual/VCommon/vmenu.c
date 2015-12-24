#include "project.h"
#ifdef PROJECT_VOPTS
   #include "vopts.h"
#endif
#ifdef PROJECT_VMAKE
   #include "vmake.h"
#endif
#include <proto/gadtools.h>

Prototype void enable_menu(void);
Prototype struct Menu *create_menus(struct NewMenu *menulist);
Prototype int layout_menus(struct Menu *menus);
Prototype void free_menus(struct Menu *menus);
Prototype void ghost_menus(void);
Prototype void unghost_menus(void);

/***********************************************************************************
 * Procedure: enable_menu
 * Synopsis:  enable_menu();
 * Purpose:   Attach any system defined menus to the current window.
 ***********************************************************************************/
void enable_menu()
{
   int i, level, newlev, count[4];
   struct NewMenu *newmenu;

   if (global.window == NULL) return;

   /* Let us go through and validate the menu they have constructed */
   level = 0;
   newmenu = global.menuitem;
   for (i = 0; i < MAX_MENU; i++)
   {
      newlev = newmenu[i].nm_Type;
      if (newlev > level)
      {
         level++;
         /* Make sure we are going down only one level */
         if (newmenu[i].nm_Type != level || level > MENU_SUB) return;
         count[level] = 0; /* Initialize our count */
      }
      else if (newlev == level)
      {
         /* Make sure we don't have two TITLES in a row */
         if (level == MENU_MENU) return;
         count[level]++;
      }
      else
      {
         /* We are going up a level.  Validate the counts on the way up */
         if (newlev < MENU_END) return;
         while (level > newlev)
         {
            if (count[level] >= "!\x20\x40\x20"[level])
               return;
            else
               level--;
         }
         /* When we hit the end of the menus, break */
         if (level == 0) break;
         count[level]++;
      }

   }
   /* Lastly ensure that they had an END marker */
   if (level) return;

   if (!(global.menu = create_menus(newmenu)))
      return;

   if (!layout_menus(global.menu))
      return;

   SetMenuStrip(global.window, global.menu);
}

/***********************************************************************************
 * Procedure: create_menus
 * Synopsis:  Menu = create_menus(NewMenu);
 * Purpose:   Create the appropriate menu structures for a given NewMenu list
 ***********************************************************************************/
struct Menu *create_menus(struct NewMenu *menulist
                         )
{
   int i;
   struct Menu   *retmenu;
   struct XMenu  *thismenu;
   struct XItem  *thisitem;
   static struct Image sep_image = {5, 1, 1000, 2,0, NULL, 0,0, NULL};


   if (GadToolsBase != NULL)
   {
      struct TagItem taglist[2];

      taglist[0].ti_Tag = GTMN_FrontPen;
      taglist[0].ti_Data = 0;
      taglist[1].ti_Tag = TAG_DONE;

      return(CreateMenusA(menulist, taglist));
   }

   /* We don't have GadTools around to do the work for us, so just create the */
   /* Menu structures directly.                                               */

   retmenu = NULL;  /* Default to giving them NOTHING */

   for (i = 0; i < MAX_MENU; i++)
   {
      switch (menulist[i].nm_Type)
      {
         case NM_TITLE:
            {
               struct XMenu  *newmenu;

               newmenu = (struct XMenu *)get_mem(sizeof(struct XMenu));
               if (newmenu == NULL)
               {
                  free_menus(retmenu);
                  return(NULL);
               }

               /* Link the menu structure onto the list of other menus */
               if (retmenu == NULL)
                  retmenu = &newmenu->menu;
               else
                  thismenu->menu.NextMenu = &newmenu->menu;

               thismenu = newmenu;
            }

            thismenu->menu.Flags    = MENUENABLED;
            thismenu->menu.MenuName = (BYTE *)menulist[i].nm_Label;
            thismenu->userdata      = menulist[i].nm_UserData;
            thismenu->image         = sep_image;
            break;

        case NM_ITEM:
            if (thismenu == NULL) return(NULL);

            {
               struct XItem  *newitem;

               newitem = (struct XItem *)get_mem(sizeof(struct XItem));
               if (newitem == NULL)
               {
                  free_menus(retmenu);
                  return(NULL);
               }
               if (thismenu->menu.FirstItem == NULL)
                  thismenu->menu.FirstItem = &newitem->item;
               else
                  thisitem->item.NextItem  = &newitem->item;
               thisitem = newitem;
            }

            if (menulist[i].nm_Label == NM_BARLABEL)
            {
               thisitem->item.Flags        = 0;
               thisitem->item.ItemFill     = &thismenu->image;
            }
            else
            {
               /* fill in the intuitext with some reasonable defaults */

               thisitem->itext.FrontPen    = 0;
               thisitem->itext.DrawMode    = JAM1;
               thisitem->itext.LeftEdge    = 5;
               thisitem->itext.TopEdge     = 1;
               thisitem->itext.ITextFont   = &global.ri.TextAttr;
               thisitem->itext.IText       = menulist[i].nm_Label;

               thisitem->item.Flags        = ITEMTEXT|ITEMENABLED|HIGHCOMP;
               thisitem->item.ItemFill     = (APTR)&thisitem->itext;

               if (menulist[i].nm_CommKey)
               {
                  thisitem->item.Command   = *menulist[i].nm_CommKey;
                  thisitem->item.Flags    |= COMMSEQ;
               }
            }
            thisitem->userdata = menulist[i].nm_UserData;
            break;

         case NM_END:
             return(retmenu);
      }
   }
   return(retmenu);
}

/***********************************************************************************
 * Procedure: layout_menus
 * Synopsis:  rc = layout_menus(Menu);
 * Purpose:   Lays out a menu given the current global information
 ***********************************************************************************/
int layout_menus(struct Menu *menus)
{
   struct XMenu     *menu;
   struct XItem     *item;
   int              xpos;

   if (GadToolsBase != NULL)
   {
      struct VisualInfo *vi;
      struct TagItem taglist;
      int rc;

      taglist.ti_Tag = TAG_DONE;

      rc = 0;
      if ((vi = GetVisualInfoA(global.window->WScreen, &taglist)))
      {
         rc = LayoutMenusA(global.menu, vi, &taglist);
         FreeVisualInfo(vi);
      }
      return(rc);
   }

   xpos = VBAR;

   for (menu = (struct XMenu *)menus; menu;
        menu = (struct XMenu *)menu->menu.NextMenu)
   {
      int width, ypos;

      /* set the x position of the menu to the next available position.
       * Set the menu width based on the size of the text.
       */

      menu->menu.LeftEdge = xpos;
      menu->menu.Width    = text_width(menu->menu.MenuName) + DVBAR;

      /* set the initial item y-position to just below the title bar */

      ypos = 0;

      /* walk through the list of items for the first pass. The object
       * is to find the width of the largest item. We will also set
       * the y-position of each item on this pass.
       */

      width = menu->menu.Width;    /* make items at least as big as menu */

      for (item = (struct XItem *)menu->menu.FirstItem;
           item;
           item = (struct XItem *)item->item.NextItem)
      {
         int  iwidth;

         /* set the y-position of the item to the next available. Also,
          * put the item just to the left of the menu. You can make
          * this number zero if you prefer.
          */

         item->item.TopEdge = ypos;
         item->item.LeftEdge = -VBAR;

         /* if it's a text item, see how big it is. */

         if (item->item.Flags & ITEMTEXT)
         {
            struct IntuiText *itext;

            itext = (struct IntuiText *)item->item.ItemFill;

            /* the height is just the font height + 2. The width is
             * based on the string pixel width.
             */

            item->item.Height = global.ri.FontSize + 2;
            iwidth = text_width(itext->IText) + 10;

            /* if it has a command-key sequence, then add in the space needed
             * for the command key and the command symbol. Note that in
             * lo-res, we should actually use LOWCOMMWIDTH, but I'm
             * feeling lazy today.
             */

            if (item->item.Flags & COMMSEQ)
            {
               char buf[2];

               buf[0] = item->item.Command;
               buf[1] = 0;
               iwidth += text_width(buf) + COMMWIDTH + 8;
            }
         }
         else
         {
            /* separator bars are always the same height */
            item->item.Height = 4;
            iwidth = 0; /* nasty things happen in Dos 1.3 if not initialized */
         }

         /* update the y-position variable. Make sure the items don't
          * run off the bottom.
          */

         ypos  += item->item.Height;
         if (ypos >= (global.ri.ScreenHeight - DHBAR))
            return(0);

          /* now see if this item is the largest one. */
          if (iwidth > width) width = iwidth;
      }

      /* on our second pass, we set the widths of all the items to the
       * width of the largest item so that it looks good. In addition,
       * we are going to adjust menus that might run off the right edge.
       */

      for (item = (struct XItem *)menu->menu.FirstItem;
          item;
          item = (struct XItem *)item->item.NextItem)
      {
         item->item.Width = width;
         if ((xpos + width) >= (global.ri.ScreenWidth - 2))
            item->item.LeftEdge =
                   global.ri.ScreenWidth - 2 - xpos - width;
      }

      menu->image.Width = width - 7;

      /* now update the x position variable, make sure that the menu
       * header is not off the edge of the screen, and add a little space
       * between the headers. Feel free to make this number whatever
       * you want. If you are feeling really ambitious, you could
       * make this number adaptable.
       */
      xpos += menu->menu.Width;
      if (xpos > global.ri.ScreenWidth) return(NULL);
      xpos += 8;
   }
   return(TRUE);
}

/***********************************************************************************
 * Procedure: free_menus
 * Synopsis:  free_menus(Menu);
 * Purpose:   Return any storage allocated for a given Menu structure.
 ***********************************************************************************/
void free_menus(struct Menu *menus)
{
   struct XMenu *menu, *nextmenu;
   struct XItem *item, *nextitem;

   if (global.window)
      ClearMenuStrip(global.window);

   if (!menus) return;

   if (GadToolsBase != NULL)
   {
      FreeMenus(menus);
   }

   else
   {
      for (menu = (struct XMenu *)menus; menu; menu = nextmenu)
      {
         nextmenu = (struct XMenu *)menu->menu.NextMenu;
         for (item = (struct XItem *)menu->menu.FirstItem;
              item;
              item = nextitem)
         {
            nextitem = (struct XItem *)item->item.NextItem;
            free_mem(item, sizeof(struct XItem));
         }
         free_mem(menu, sizeof(struct XMenu));
      }
   }
   global.menu = NULL;
}


/***********************************************************************************
 * Procedure: ghost_menus
 * Synopsis:  ghost_menus();
 * Purpose:   disable all menu entries except Open and New  
 ***********************************************************************************/
void ghost_menus(void)
{
   int i;
   char *udp;

   free_menus(global.menu);

   for (i = 0; i < MAX_MENU; i++)
   {

      if (global.menuitem[i].nm_Type == NM_ITEM) /* only ghost entries */
      {
         /* to allow flexible menu configuration, base our checks on   */
         /* what the menu entry is programmed to do.  We allow READ,   */
         /* NEW or QUIT when no project is loaded, ghost anything else.*/
         udp = global.menuitem[i].nm_UserData;
         if (udp) /* Menu bars don't have user data */
            if ((strnicmp(udp, "read", 4) != 0)
               && (strnicmp(udp, "new",  3) != 0)
               && (strnicmp(udp, "quit", 4) != 0))
               global.menuitem[i].nm_Flags |= NM_ITEMDISABLED;
      }
   }

   enable_menu();
}
 

/***********************************************************************************
 * Procedure: unghost_menus
 * Synopsis:  unghost_menus();
 * Purpose:   reenable all menu entries (undo effect of ghost_menus())  
 ***********************************************************************************/
void unghost_menus(void)
{
   int i;

   free_menus(global.menu);

   for (i = 0; i < MAX_MENU; i++)
   {
      global.menuitem[i].nm_Flags &= ~NM_ITEMDISABLED;
   }

   enable_menu();
}