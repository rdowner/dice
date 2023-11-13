/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MENU.C
 *
 *  Menu routines... made to take up as little space as possible, and
 *  thus uses many tricks which you should watch out for.
 */

#include "defs.h"
#include "graphics/gfxbase.h"
#include "graphics/rastport.h"

Prototype void menu_strip (struct Window *);
Prototype void menu_off (void);
Prototype void menu_on (void);
Prototype void do_menuoff (void);
Prototype void do_menuon (void);
Prototype char *menutomacro (char *);
Prototype char *menu_cmd (struct IntuiMessage *);
Prototype void fixmenu (void);
Prototype void do_menuclear (void);
Prototype void do_menuadd (void);
Prototype void do_menudelhdr (void);
Prototype int do_menudel (void);

typedef struct {
    ITEM item;
    char *com;
} XITEM;

short Menuoff;
short DoMenuoff;

MENU *Menu;

void
menu_strip(win)
WIN *win;
{
    if (!Menuoff && Menu) {
        SetMenuStrip(win,Menu);
        Forbid();
        win->Flags &= ~RMBTRAP;
        Permit();
    }
}

void
menu_off()
{
    ED *ed;
    if (Menuoff == 0) {
        for (ed = (ED *)DBase.mlh_Head; ed->Node.mln_Succ; ed = (ED *)ed->Node.mln_Succ) {
            ClearMenuStrip(ed->Win);
            Forbid();
            ed->Win->Flags |= RMBTRAP;
            Permit();
        }
    }
    ++Menuoff;
}

void
menu_on()
{
    ED *ed;
    if (Menu && Menuoff == 1) {
        fixmenu();
        for (ed = (ED *)DBase.mlh_Head; ed->Node.mln_Succ; ed = (ED *)ed->Node.mln_Succ) {
            SetMenuStrip(ed->Win,Menu);
            Forbid();
            ed->Win->Flags &= ~RMBTRAP;
            Permit();
        }
    }
    --Menuoff;
}

void
do_menuoff()
{
    menu_off();
    ++DoMenuoff;
}

void
do_menuon()
{
    if (DoMenuoff) {
        --DoMenuoff;
        menu_on();
    }
}

char *
menutomacro(str)
char *str;
{
    char header[64];
    char itembuf[64];
    short i;
    char *ptr;
    MENU *menu;
    ITEM *item;

    for (i = 0; str[i] && str[i] != '-'; ++i);
    if (str[i] == '-') {
        strncpy(header, str, i);
        header[i] = 0;
        strcpy(itembuf, str + i + 1);
        for (menu = Menu; menu; menu = menu->NextMenu) {
            if (ncstrcmp(header, menu->MenuName) == 0) {
                for (item = menu->FirstItem; item; item = item->NextItem) {
                    ptr = (char *)((ITEXT *)item->ItemFill)->IText;
                    if (ncstrcmp(itembuf, ptr) == 0) {
                        ptr = ((XITEM *)item)->com;
                        goto done;
                    }
                }
            }
        }
    }
    ptr = NULL;
done:
    return(ptr);
}

char *
menu_cmd(im)
IMESS *im;
{
    XITEM *item;

    if (item = (XITEM *)ItemAddress(Menu, im->Code))
        return(item->com);
    return(NULL);
}

/*
 *  new fixmenu by BIX:dhack handles 2.0 fonts
 */

void
fixmenu()
{
    MENU *menu;
    ITEM *item;
    ITEXT *it;
    int row, col, maxc, scr;
    struct IntuiText itxt;  /* To find width in case of proportional fonts */

    /* Initialize the few elements needed */
    itxt.ITextFont = Ep->Win->WScreen->Font;
    itxt.NextText = NULL;

    col = 0;
    for (menu = Menu; menu; menu = menu->NextMenu) {
        short offset;  /* Offset for width; used for position of item
                          relative to it's menu header at end of loop */

        /* Put this menu header into the IntuiText and figure out how
           wide it is in pixels */
        itxt.IText = menu->MenuName;
        maxc = IntuiTextLength(&itxt);    /* This is now # of pixels */

        /* Find 'average' character size */
        offset = (maxc/strlen(menu->MenuName))*2;
        menu->Width = maxc + offset;

        row = 0;
        for (item = menu->FirstItem; item; item = item->NextItem) {
            it = (ITEXT *)item->ItemFill;
            item->TopEdge = row;
            if(it != NULL) {
                it->TopEdge = 1;         /* Center in select box */
                it->LeftEdge = offset/2;
                it->DrawMode = JAM1;     /* How workbench's menus are... */
            }

            /* Figure out how wide this item is */
            itxt.IText = ((ITEXT *)item->ItemFill)->IText;
            scr = IntuiTextLength(&itxt);  /* This is now # of pixels */
            if (scr > maxc)
                maxc = scr;

            /* Figure out how tall the text is */
            if(Ep->Win->WScreen->Font != NULL)
                item->Height = Ep->Win->WScreen->Font->ta_YSize + 2;

            else /* This is something to default to, just in case */
                item->Height = GfxBase->DefaultFont->tf_YSize + 2;

            row += item->Height;
        }
        maxc += offset;

        /* Fit the menu onto the screen */
        if( (maxc+10) > Ep->Win->WScreen->Width ) {   /* Wider than screen! */
            offset = -col;
            /* Unfortunately, this won't reduce the menu's width */
            maxc = Ep->Win->WScreen->Width - 12;
        } else if( (offset = Ep->Win->WScreen->Width - (col+maxc+10)) > 0 )
            offset = 0;

        for (item = menu->FirstItem; item; item = item->NextItem) {
            item->Width = maxc;
            item->LeftEdge = offset;
        }

        menu->LeftEdge = col;
        menu->Height = row;
        col += menu->Width;
    }
}


#ifdef NOTDEF
void
fixmenu()
{
    MENU *menu;
    ITEM *item;
    ITEXT *it;
    int row, col, maxc, scr;

    col = 0;
    for (menu = Menu; menu; menu = menu->NextMenu) {
        short hdr_width;
        maxc = strlen(menu->MenuName);
        hdr_width = maxc * 8 + 16;
        menu->Width = hdr_width;
        row = 0;
        for (item = menu->FirstItem; item; item = item->NextItem) {
            it = (ITEXT *)item->ItemFill;
            item->TopEdge = row;
            scr = strlen(((ITEXT *)item->ItemFill)->IText);
            if (scr > maxc)
                maxc = scr;
            item->Height = 10;
            row += item->Height;
        }
        maxc = (maxc * 8) + 16;
        for (item = menu->FirstItem; item; item = item->NextItem)
            item->Width = maxc;
        menu->LeftEdge = col;
        menu->Height = row;
        col += hdr_width;
    }
}
#endif

/*
 *  menuclear
 *  menuadd     header  item    command
 *  menudel     header  item
 *  menudelhdr  header
 */

void
do_menuclear()
{
    menu_off();
    while (Menu) {
        av[1] = (ubyte *)Menu->MenuName;
        do_menudelhdr();
    }
    menu_on();
}

void
do_menuadd()
{
    MENU *menu, **mpr;
    ITEM *item, **ipr;
    ITEXT *it;

    menu_off();
    mpr = &Menu;
    for (menu = *mpr; menu; menu = *mpr) {
        if (strcmp(av[1], menu->MenuName) == 0) {
            ipr = &menu->FirstItem;
            for (item = *ipr; item; item = *ipr) {
                if (strcmp(av[2], ((ITEXT *)item->ItemFill)->IText) == 0)
                    goto newname;
                ipr = &item->NextItem;
            }
            goto newitem;
        }
        mpr = &menu->NextMenu;
    }
newmenu:    /*  create new menu */
    menu = malloc(sizeof(MENU));
    clrmem(menu, sizeof(MENU));
    menu->NextMenu = *mpr;
    *mpr = menu;
    menu->Flags = MENUENABLED;
    menu->MenuName = malloc(strlen(av[1])+1);
    strcpy(menu->MenuName, av[1]);
    ipr = &menu->FirstItem;
    *ipr = NULL;
newitem:    /*  create new item */
    it = malloc(sizeof(ITEXT));
    clrmem(it, sizeof(ITEXT));
    it->BackPen = 1;
    it->DrawMode = JAM2;
    it->IText = malloc(strlen(av[2])+1);
    strcpy(it->IText, av[2]);
    item = malloc(sizeof(XITEM));
    clrmem(item, sizeof(XITEM));
    item->NextItem = *ipr;
    *ipr = item;
    item->ItemFill = (APTR)it;
    item->Flags = ITEMTEXT|ITEMENABLED|HIGHCOMP;
newname:    /*  create new name */
    if (((XITEM *)item)->com)
        free(((XITEM *)item)->com);
    ((XITEM *)item)->com = malloc(strlen(av[3])+1);
    strcpy(((XITEM *)item)->com, av[3]);
    menu_on();
}

void
do_menudelhdr()
{
    MENU *menu;
    MENU **mpr;

    menu_off();
    mpr = &Menu;
    for (menu = *mpr; menu; menu = *mpr) {
        if (strcmp(av[1], menu->MenuName) == 0) {
            if (menu->FirstItem) {
                while (menu->FirstItem) {
                    av[2] = ((ITEXT *)menu->FirstItem->ItemFill)->IText;
                    if (do_menudel())
                        break;
                }
                break;
            }
            *mpr = menu->NextMenu;
            free(menu->MenuName);
            free(menu);
            break;
        }
        mpr = &menu->NextMenu;
    }
    menu_on();
}

int
do_menudel()
{
    MENU *menu;
    ITEM *item, **ipr;
    ITEXT *it;
    short ret = 0;

    menu_off();
    for (menu = Menu; menu; menu = menu->NextMenu) {
        if (strcmp(av[1], menu->MenuName) == 0) {
            ipr = &menu->FirstItem;
            for (item = *ipr; item; item = *ipr) {
                it = (ITEXT *)item->ItemFill;
                if (strcmp(av[2], it->IText) == 0) {
                    *ipr = item->NextItem;
                    free(it->IText);
                    free(it);
                    free(((XITEM *)item)->com);
                    free(item);
                    if (!menu->FirstItem) {
                        do_menudelhdr();
                        ret = 1;
                    }
                    menu_on();
                    return((int)ret);
                }
                ipr = &item->NextItem;
            }
        }
    }
    menu_on();
    return((int)ret);
}

