/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include 	"defs.h"
#include	"dbug_protos.h"


Prototype BOOL 	enable_menus(void);
Prototype void 	init_default_menus(void);
Prototype void 	free_menus(void);
Prototype void 	set_menu_item(int num, int type, unsigned char *str, unsigned char *cmd, unsigned char *comkey);
Prototype void 	do_scroller(void);
Prototype UWORD FindScrollerTop(UWORD total, UWORD displayable, UWORD pot);
Prototype int 	FindScrollerValues(UWORD total, UWORD displayable, UWORD top, WORD overlap, UWORD *body, UWORD *pot);
Prototype void 	setscrollbar(int flag);
Prototype void 	ActivateArrows(struct Window *win);
Prototype void 	InActivateArrows(struct Window *win);
Prototype BOOL 	ProcessMenuItem(char *args,int type);
Prototype int	FindSlot(char *args);

#define MAXMENU         128     /* Maxumum number of user menus         */
#define GADTOOLSBASE	GadToolsBase

struct Menu      *DebugMenu = NULL;
struct NewMenu newmenu[MAXMENU];
int MenuFlag = 0;
unsigned long  ScrollStart = 0xffffffff;
unsigned long  ScrollEnd = 0;
int SymbolCount;

struct InitMenu {
    int type;
    int num;
    unsigned char *str;
    unsigned char *cmd;
    unsigned char *comkey;
};

#define NM_BAR	4

struct InitMenu IMenu[] = {
 NM_TITLE, 	1, "Project", 		"",NULL,
 NM_ITEM, 	1, "New", 		"reset",NULL,
 NM_BAR,	1, "",			"",NULL,
 NM_ITEM, 	1, "Open", 		"open",NULL,
 NM_SUB, 	1, "Source", 		"open source",NULL,
 NM_SUB, 	1, "Mixed", 		"open mixed",NULL,
 NM_SUB, 	1, "Dism", 		"open dism",NULL,
 NM_SUB, 	1, "Bytes", 		"open bytes",NULL,
 NM_SUB, 	1, "Symbols", 		"open symbols",NULL,
 NM_SUB, 	1, "Sorted symbols", 	"open symlist",NULL,
 NM_SUB, 	1, "Breakpoints",	"open breakpoints",NULL,
 NM_ITEM, 	1, "Close", 		"close","C",
 NM_BAR,	1, "",			"",NULL,
 NM_ITEM, 	1, "Save Prefs", 	"saveprefs",NULL,
 NM_BAR,	1, "",			"",NULL,

 NM_ITEM, 	1, "Quit", 		"quit","Q",

 NM_TITLE,	2, "Display",		"",NULL,
 NM_ITEM,	2, "Source",	 	"source","S",
 NM_ITEM,	2, "Mixed", 		"mixed","M",
 NM_ITEM,	2, "Dism",	 	"dism","D",
 NM_ITEM,	2, "Bytes",	 	"bytes","B",
 NM_ITEM,	2, "Words", 		"words","W",
 NM_ITEM,	2, "Longs", 		"longs","L",
 NM_ITEM,	2, "Registers", 	"registers","R",
 NM_ITEM,	2, "Offsets", 		"offsets",NULL,

 NM_TITLE,	3, "Program Info",	"",NULL,
 NM_ITEM,	3, "Info on task", 	"info",NULL,
 NM_ITEM,	3, "Hunks", 		"hunks",NULL,
 NM_ITEM,	3, "Symbols", 		"symbols",NULL,
 NM_ITEM,	3, "Sorted symbols", 	"symlist",NULL,

 NM_TITLE,	4, "System Info",	"",NULL,
 NM_ITEM,	14, "Tasks",	 	"tasks","",
 NM_ITEM,	15, "Libraries", 	"libs","",
 NM_ITEM,	15, "Devices",	 	"devices","",
 NM_ITEM,	16, "ExecBase",	 	"execbase","",
 NM_ITEM,	18, "DosBase",	 	"dosbase",NULL,
 NM_ITEM,	19, "Memory",		"memlist",NULL,
 NM_ITEM,	19, "Interrupts",	"intrs",NULL,
 NM_ITEM,	19, "Ports",		"ports",NULL,
 NM_ITEM,	19, "Resource",		"resources",NULL,
 NM_ITEM,	11, "Process", 		"process",NULL,

 NM_TITLE,	13, "Breakpoints",	"",NULL,
 NM_ITEM,	19, "Breakpoints",	"breakpoints",NULL,
 NM_ITEM,	19, "Clear Breakpoint",	"clear",NULL,
 NM_ITEM,	19, "Clear All",	"clear all",NULL,
 NM_ITEM,	19, "Set Breakpoint",	"bp",NULL,

 NM_TITLE,	13, "Watchpoints",	"",NULL,
 NM_ITEM,	22, "Watch Byte", 	"watchbyte",NULL,
 NM_ITEM,	22, "Watch Word", 	"watchword",NULL,
 NM_ITEM,	22, "Watch Long", 	"watchlong",NULL,

 NM_TITLE,	21, "Misc",		"",NULL,
 NM_ITEM,	22, "Help", 		"help",NULL,
 NM_ITEM,	22, "REXX Command", 	"rexx",NULL,
 NM_ITEM,	22, "Alias", 		"alias",NULL,
 NM_ITEM,	22, "Unalias", 		"unalias",NULL,
 NM_ITEM,	22, "Fkey", 		"fkey",NULL,
 NM_ITEM,	22, "Set", 		"set",NULL,
 NM_END,	23, ""  , "", NULL
};


// Initialize default DD menus

void init_default_menus(void)
{
int i;

    if(!MenuFlag) {  // default menus not set
        MenuFlag = TRUE;
        memset((char *)newmenu,NM_END,sizeof(newmenu));
        for(i=0; i< MAXMENU; i++) {
            set_menu_item(i,IMenu[i].type,IMenu[i].str,IMenu[i].cmd,IMenu[i].comkey);
            if(IMenu[i].type == NM_END)return;
	}
    }
}


// handle argument parsing for setting a menu item
BOOL ProcessMenuItem(char *args,int type)
{
char *ptr, *string, *dp;
char xname[3][80];
int n, i = 0, quote = 1, flag = TRUE, iflag;

    memset(xname[1],0,80);	/* zero the optionals */
    memset(xname[2],0,80);


    if(string = strchr(args,' ')) {	// slot first, no sense in continuing without that
    	*string++ = NULL;
    	if((n = FindSlot(args)) < 0)return FALSE;	// no slot available
    	string = SkipBlanks(string);
    	ptr = string;
    	while (flag && (i < 3)) {
	    iflag = 1;
	    dp = xname[i++];
	    while(*ptr && iflag) {
	    	switch((int) *ptr) {
	    	    case '\"':
		    	quote *= -1;
		    	break;
	    	    case '\t':
	    	    case ' ':
		    	if(quote > 0) {
			    iflag = 0;  // not in quotes
			    break;
			}
		        // fall through
	            default:
		    	*dp++ = *ptr; //++;
		}
	        ptr++;
	    }
	    *dp = NULL;		// terminate the string
            if(! *ptr)flag = FALSE;	// done with args
	}
        if(i) {
	    set_menu_item(n, type, xname[0], xname[1], xname[2]);
            return(TRUE);
	}
    }
return FALSE;
}



int FindSlot(char *args)
{
int n;

    if((n = strtol(args,NULL,10)) >= 0 && (n < MAXMENU))return n;	// normal slot specified
    if(n == -1) { // its an append operation
        for(n=0; n < (MAXMENU-1); n++)if(newmenu[n].nm_Type == NM_END) {
	    set_menu_item(n+1,NM_END,"","","");  // add the new end marker
	    return n;
	}
    }
    ScrStatus("*** Error: can't find position");
    return -1;
}

// enable current set of menus
BOOL enable_menus(void)
{
    DBugDisp *disp;
    BOOL flag = FALSE;
    struct VisualInfo *vi;   
    struct TagItem taglist[3] = {
	{GTMN_FrontPen, NULL},
	{GTMN_FullMenu, TRUE},
	{TAG_DONE, NULL}
    };

   // clear all menus from active windows on display list
   for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
	if(disp->ds_Win)ClearMenuStrip(disp->ds_Win);
   }


   if(GADTOOLSBASE) {
       if (DebugMenu != NULL) {
	    FreeMenus(DebugMenu);
       	    DebugMenu = NULL;
       }

       if (!(DebugMenu = CreateMenusA(newmenu, taglist)))return 0;

       if ((vi = GetVisualInfoA((CurDisplay->ds_Win)->WScreen, taglist+1))) {
            if (!LayoutMenusA(DebugMenu, vi, taglist+1)) {
            	FreeVisualInfo(vi);
	    	return 0;
	    }
       }
       FreeVisualInfo(vi);
   }
   else {	// gadtools unavailable, so lets do it ourself
        if (DebugMenu != NULL) {
	    v_free_menus(DebugMenu);
       	    DebugMenu = NULL;
        }
        if(!(DebugMenu = v_create_menus(newmenu)))return 0;
	if(!v_layout_menus(DebugMenu))return 0;
   }

   // Set the menu strips for active windows
   for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
	if(disp->ds_Win)SetMenuStrip(disp->ds_Win, DebugMenu);
   }
   // and for current window (possibly not on display list yet)
   if(CurDisplay && CurDisplay->ds_Win)
   	flag = (BOOL)SetMenuStrip(CurDisplay->ds_Win, DebugMenu); 
   return flag;
}



/***
*
* Set up a menu item to a given string and command
*
***/
void set_menu_item(int num, int type, unsigned char *str, unsigned char *cmd,
	unsigned char *comkey)
{
   DBugDisp *disp;

   if (num < 0 || num >= MAXMENU) {
	return;
   }
   // shutdown all menu items from active windows until they next do an enable
   for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
	if(disp->ds_Win)ClearMenuStrip(disp->ds_Win);
   }

   /* First we free up any menu item that might be there */
   if (newmenu[num].nm_UserData != NULL) {
	free(newmenu[num].nm_UserData);
	newmenu[num].nm_UserData = NULL;
   }
   if ((newmenu[num].nm_Label != NULL) && (newmenu[num].nm_Label != NM_BARLABEL)) {
      free(newmenu[num].nm_Label);
      newmenu[num].nm_Label = NULL;
   }

   if (newmenu[num].nm_CommKey != NULL) {
      free(newmenu[num].nm_CommKey);
      newmenu[num].nm_CommKey = NULL;
   }
   newmenu[num].nm_UserData=(void *)malloc(strlen(cmd)+8);
   newmenu[num].nm_Label= (type == 4) ? NM_BARLABEL: malloc(strlen(str)+8);

   if(comkey && *comkey) {
        newmenu[num].nm_CommKey=(void *)malloc(strlen(comkey)+8);
   }

   if((newmenu[num].nm_UserData == NULL)|| (newmenu[num].nm_Label==NULL) ||
   ((comkey && *comkey) && (newmenu[num].nm_CommKey == NULL))) {
      newmenu[num].nm_Type = 0;
      return;	/* out of memory */
   }

   strcpy(((char *)newmenu[num].nm_UserData), cmd);

   if (type == 4) type = 2;
   else strcpy(newmenu[num].nm_Label,str);

   newmenu[num].nm_Type = type;
   newmenu[num].nm_Flags = 0;

   if(comkey && *comkey)strcpy(((char *)newmenu[num].nm_CommKey), comkey);
   else newmenu[num].nm_CommKey = 0;
}

/* Free up all menu storage */
void free_menus(void)
{
   int i;

   if (DebugMenu != NULL) {
      if(GADTOOLSBASE)FreeMenus(DebugMenu);
      else v_free_menus(DebugMenu);
      DebugMenu = NULL;
   }

   for (i = 0; i < MAXMENU; i++) {
      if (newmenu[i].nm_UserData != NULL)free(newmenu[i].nm_UserData);
      if ((newmenu[i].nm_Label != NULL) && (newmenu[i].nm_Label != NM_BARLABEL))free(newmenu[i].nm_Label);
      if (newmenu[i].nm_CommKey != NULL)free(newmenu[i].nm_CommKey);

      newmenu[i].nm_UserData = NULL;
      newmenu[i].nm_Label    = NULL;
      newmenu[i].nm_CommKey  = NULL;
   }
}


extern struct Gadget ColorGadget;
extern struct Gadget DownGadget;
extern struct Gadget UpGadget;
extern struct Image ColorPropImage;
extern struct PropInfo ColorPropInfo;


/* Set size of scroll bar.  If flag is true, remove the scroll bar first */

void setscrollbar(int flag)
{
    WINDOW *win;

    if(CurDisplay) {
	if((win = CurDisplay->ds_Win)) {
    	    if(flag)RemoveGadget(win,&ColorGadget);
    	    else {	/* first time through */
		AddGadget(win,&UpGadget,(UWORD)~0);
    		AddGadget(win,&DownGadget,(UWORD)~0);
	    }
	}
    }

    /* Set up the scroll bar */
    ColorGadget.LeftEdge = - win->BorderRight + 4;
    ColorGadget.TopEdge = win->BorderTop;
    ColorGadget.Height=win->Height - win->BorderBottom - win->BorderTop-8 - 22;
    ColorGadget.Width= 14; /* win->BorderRight-3; */
    ColorPropInfo.VertBody = 0xFFFF;	/* starts with nothing */

    AddGadget(win,&ColorGadget,(UWORD)~0);
    RefreshGadgets(&UpGadget,win,NULL);	/* refresh just the scroll bar */
}

int FindScrollerValues(UWORD total, UWORD displayable, UWORD top,
		   WORD overlap, UWORD *body, UWORD *pot)
{
// UWORD hidden;
int hidden;
UWORD oldbody = *body;
UWORD oldpot = *pot;
int temp1 = total, temp2 = displayable;

hidden = MAX(temp1-temp2, 0);

if(top > hidden)top = hidden;

(*body) = (hidden > 0) ? (UWORD)(((ULONG)(displayable - overlap)*MAXBODY)
		/ (total - overlap)) : MAXBODY;

(*pot) = (hidden > 0) ? (UWORD) (((ULONG)top * MAXPOT)/hidden) : 0;

if((oldbody == *body) && (oldpot == *pot))return(FALSE);
return(TRUE);
}


UWORD FindScrollerTop(UWORD total, UWORD displayable, UWORD pot)
{
//UWORD top, hidden;
int top, hidden;
int temp1 = total, temp2 = displayable;

hidden = MAX(temp1-temp2, 0);
top = ((((ULONG) hidden * pot) + (MAXPOT/2)) >> 16); /* range from 0 to ... */

return(top);
}


void do_scroller(void)
{
WINDOW *win;
ULONG total, top;
int rows;

    if(CurDisplay) {
	if((win = CurDisplay->ds_Win)) {
	rows = CurDisplay->ds_ScrRows;
    	    // calc new values

	    top = CurDisplay->ds_WindowTop;  // current top for almost all display modes

	    switch(CurDisplay->ds_DisplayMode) {
		case DISPLAY_DISM:
		case DISPLAY_SOURCE:
		case DISPLAY_MIXED:
		case DISPLAY_BYTES:
		case DISPLAY_WORDS:
		case DISPLAY_LONGS:

		    total = (ScrollEnd - ScrollStart) >> 5;
		    top = ((int)CurDisplay->ds_WindowTop - (int)ScrollStart) >> 5;

		    if((int)top < 0 ) top = 0;
		    break;

		case DISPLAY_SYMBOL:
		    total = SymbolCount;
		    break;

		case DISPLAY_HUNKS:
		    total = numHunks;
		    break;

		case DISPLAY_HELP:
		    total = HelpSize();
		    break;

		case DISPLAY_BREAK:
		    total = MAXBP+5;
		    top = topBP;
		    break;

		default:
		    total = SizeDLIST(&CurDisplay->ds_List);
	    }
    	    if(FindScrollerValues(total, CurDisplay->ds_ScrRows, top, 2, 
    	    &ColorPropInfo.VertBody,&ColorPropInfo.VertPot)) {
		/* values changed, update */
        	NewModifyProp(&ColorGadget,win,NULL,PROPNEWLOOK | AUTOKNOB | FREEVERT,0, 
        	ColorPropInfo.VertPot,  0, ColorPropInfo.VertBody, -1 );
	    }
	}
    }
}


__chip const UWORD upPicture[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0000,0x0180,0x0660,0x1818,0x0000,
    0x0000,0x0000,0xFFFF,
/* Plane 1 */
    0xFFFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,
};

__chip const UWORD upPictureSelect[] =
{
/* Plane 0 */
    0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFE7F,0xF99F,0xE7E7,0xFFFF,
    0xFFFF,0xFFFF,0x0000,
};

__chip const UWORD downPicture[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0000,0x1818,0x0660,0x0180,0x0000,
    0x0000,0x0000,0xFFFF,
/* Plane 1 */
    0xFFFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,
};

__chip const UWORD downPictureSelect[] =
{
/* Plane 0 */
    0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE7E7,0xF99F,0xFE7F,0xFFFF,
    0xFFFF,0xFFFF,0x0000,
};

struct Image UpImage =  {
    -1, 0,              /* Left, Top */
    16, 11, 2,		/* width, depth, height */
    upPicture,
    3, 0,                /* PlanePick, PlaneOnOff */
    NULL
};

struct Image DownImage = {
    -1, 0,              /* Left, Top */
    16, 11, 2,		/* width, height, depth */
    downPicture,
    3, 0,                /* PlanePick, PlaneOnOff */
    NULL
};

struct Image UpImageSelect =  {
    -1, 0,              /* Left, Top */
    16, 11, 2,		/* width, depth, height */
    upPictureSelect,
    3, 0,                /* PlanePick, PlaneOnOff */
    NULL
};

struct Image DownImageSelect = {
    -1, 0,              /* Left, Top */
    16, 11, 2,		/* width, height, depth */
    downPictureSelect,
    3, 0,                /* PlanePick, PlaneOnOff */
    NULL
};

struct Gadget UpGadget = {
        NULL,		/* NextGadget */
        -15, -31,          /* gadget hit box LeftEdge, TopEdge */
        16, 11,          /* gadget hit box Width, Height */
        GADGHCOMP|GADGIMAGE|GRELRIGHT|GRELBOTTOM|RIGHTBORDER, /* Flags */
        RELVERIFY|GACT_IMMEDIATE,      /* Activation */
        BOOLGADGET,     /* GadgetType */
        &UpImageSelect,	/* GadgetRender */
        NULL,           /* no SelectRender */
        NULL,           /* no text for this gadget */
        NULL,           /* no MutualExclude */
        NULL,           /* no SpecialInfo */
        ID_UP,              /* gadget ID */
        NULL           /* no general purpose data */
};

struct Gadget DownGadget = {
	NULL,		/* NextGadget */
        -15, -20,       /* gadget hit box LeftEdge, TopEdge */
        16, 11,          /* gadget hit box Width, Height */
        GADGHCOMP|GADGIMAGE|GRELRIGHT|GRELBOTTOM|RIGHTBORDER, /* Flags */
        RELVERIFY|GACT_IMMEDIATE,      /* Activation */
        BOOLGADGET,     /* GadgetType */
        &DownImageSelect,       /* GadgetRender */
        NULL,           /* no SelectRender */
        NULL,           /* no text for this gadget */
        NULL,           /* no MutualExclude */
        NULL,           /* no SpecialInfo */
        ID_DOWN,	/* gadget ID */
        NULL           /* no general purpose data */
}; 


/* scroller gadgets */

#define COLOR_KNOB_BODY		0x1111

struct Image ColorPropImage;

struct PropInfo ColorPropInfo = {
	PROPNEWLOOK | AUTOKNOB | FREEVERT,		/* flags */
	0,				/* horiz pot */
	0xFFFF,				/* vert pot */
	0,				/* horiz body */
	0,				/* vert body */
	0, 0,				/* cwidth, cheight */
	0, 0, 				/* HpotRes, VPotRes */
	0, 0				/* Left border, Top border */
};

struct Gadget ColorGadget = {
	NULL,			/* Next gadget */
	-10,			/* leftedge */
	4,		 	/* top edge */
	10,			/* width */
	80,			/* height */
	GADGHCOMP|GADGIMAGE|GRELRIGHT|RIGHTBORDER,	/* flags */
	FOLLOWMOUSE|GACT_IMMEDIATE,	/* activation */
	PROPGADGET,		/* gadget type */
	&ColorPropImage,	/* gadget render */
	NULL,			/* select render */
	NULL,			/* gadget text */
	NULL,			/* custom gadget hook */
	&ColorPropInfo,		/* special info */
	ID_SCROLL,		/* gadget ID */
	NULL			/* user data */
};

void ActivateArrows(struct Window *win)
{
int pos;

    pos = RemoveGadget(win,&UpGadget);
    UpGadget.GadgetRender = &UpImageSelect;
    AddGadget(win,&UpGadget,pos);

    pos = RemoveGadget(win,&DownGadget);
    DownGadget.GadgetRender = &DownImageSelect;
    AddGadget(win,&DownGadget,pos);

// refresh gadgets in do_scroller

    setscrollbar(1);
    do_scroller();
}

void InActivateArrows(struct Window *win)
{
int pos;

    pos = RemoveGadget(win,&UpGadget);
    UpGadget.GadgetRender = &UpImage;
    AddGadget(win,&UpGadget,pos);

    pos = RemoveGadget(win,&DownGadget);
    DownGadget.GadgetRender = &DownImage;
    AddGadget(win,&DownGadget,pos);

    RefreshGadgets(&UpGadget,win,NULL);	/* refresh just the scroll bar */
}

Prototype struct Menu *v_create_menus(struct NewMenu *menulist);
Prototype int v_layout_menus(struct Menu *menus);
Prototype void v_free_menus(struct Menu *menus);
Prototype int text_width(char *str);
Local void *malloc_clear(ULONG bytes);

struct TextAttr TOPAZ80={"topaz.font",8,0,FPF_ROMFONT};

void *malloc_clear(ULONG bytes) {
void *ptr = malloc(bytes);

if(ptr)memset(ptr,0,bytes);

return ptr;
}

struct XMenu {
   struct Menu      menu;
   APTR             userdata;
   struct Image     image;
};

struct XItem {
   struct MenuItem  item;
   APTR             userdata;
   struct IntuiText itext;
};


#define VBAR            2         /* Pixel width of a vertical BAR        */
#define DVBAR           (2*VBAR)  /* Pixel width of two vertical BARs     */
#define HBAR            1         /* Pixel height of a horizontal BAR     */
#define DHBAR           (2*HBAR)  /* Pixel height of two Horizontal BARs  */

/***********************************************************************************
 * Procedure: create_menus
 * Synopsis:  Menu = create_menus(NewMenu);
 * Purpose:   Create the appropriate menu structures for a given NewMenu list
 ***********************************************************************************/

struct Menu *v_create_menus(struct NewMenu *menulist)
{
   int i;
   struct Menu   *retmenu;
   struct XMenu  *thismenu;
   struct XItem  *thisitem;
   static struct Image sep_image = {5, 1, 1000, 2,0, NULL, 0,0, NULL};

   /* We don't have GadTools around to do the work for us, so just create the */
   /* Menu structures directly.                                               */

   retmenu = NULL;  /* Default to giving them NOTHING */
   for (i = 0; i < MAXMENU; i++) {
      switch (menulist[i].nm_Type) {
         case NM_TITLE:
            {
               struct XMenu  *newmenu;

               newmenu = (struct XMenu *)malloc_clear(sizeof(struct XMenu));
               if (newmenu == NULL) {
                  v_free_menus(retmenu);
                  return(NULL);
               }

               /* Link the menu structure onto the list of other menus */
               if (retmenu == NULL)retmenu = &newmenu->menu;
               else thismenu->menu.NextMenu = &newmenu->menu;

               thismenu = newmenu;
            }

            thismenu->menu.Flags    = MENUENABLED;
            thismenu->menu.MenuName = menulist[i].nm_Label;
            thismenu->userdata      = menulist[i].nm_UserData;
            thismenu->image         = sep_image;
            break;

        case NM_ITEM:
        case NM_SUB:
            if (thismenu == NULL) return(NULL);
            {
               struct XItem  *newitem;

               newitem = (struct XItem *)malloc_clear(sizeof(struct XItem));
               if (newitem == NULL) {
                  v_free_menus(retmenu);
                  return(NULL);
               }
               if (thismenu->menu.FirstItem == NULL)
		    thismenu->menu.FirstItem = &newitem->item;
               else {
		    for(thisitem = (struct XItem *)thismenu->menu.FirstItem; thisitem->item.NextItem; thisitem = (struct XItem *)thisitem->item.NextItem)
			;
                    thisitem->item.NextItem  = &newitem->item;
	       }
	       thisitem = newitem;
	    }

            if (menulist[i].nm_Label == NM_BARLABEL) {
               thisitem->item.Flags        = 0;
               thisitem->item.ItemFill     = &thismenu->image;
            }
            else {
               /* fill in the intuitext with some reasonable defaults */
               thisitem->itext.FrontPen    = 0;
               thisitem->itext.DrawMode    = JAM1;
               thisitem->itext.LeftEdge    = 5;
               thisitem->itext.TopEdge     = 1;
               thisitem->itext.ITextFont   = &TOPAZ80;


               thisitem->itext.IText       = menulist[i].nm_Label;

               thisitem->item.Flags        = ITEMTEXT|ITEMENABLED|HIGHCOMP;
               thisitem->item.ItemFill     = (APTR)&thisitem->itext;
               if (menulist[i].nm_CommKey) {
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
int v_layout_menus(struct Menu *menus)
{
   struct XMenu     *menu;
   struct XItem     *item;
   int              xpos;
   int swidth, sheight, stxheight;

   xpos = VBAR;

   // get screen width and height for later

   if(CurDisplay && CurDisplay->ds_Win) {
	swidth = CurDisplay->ds_Win->WScreen->Width;
	sheight = CurDisplay->ds_Win->WScreen->Height;
	stxheight = CurDisplay->ds_Win->RPort->TxHeight;
   }
   else {
	swidth = 640;
	sheight = 200;
	stxheight = 8;
   }
   for (menu = (struct XMenu *)menus; menu; menu = (struct XMenu *)menu->menu.NextMenu) {
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
      for (item = (struct XItem *)menu->menu.FirstItem; item; item = (struct XItem *)item->item.NextItem) {
         int  iwidth = 0;

         /* set the y-position of the item to the next available. Also,
          * put the item just to the left of the menu. You can make
          * this number zero if you prefer.
          */

         item->item.TopEdge = ypos;
         item->item.LeftEdge = -VBAR;

         /* if it's a text item, see how big it is. */

         if (item->item.Flags & ITEMTEXT) {
            struct IntuiText *itext;

            itext = (struct IntuiText *)item->item.ItemFill;

            /* the height is just the font height + 2. The width is
             * based on the string pixel width.
             */
            item->item.Height = 2 + stxheight;
            iwidth = text_width(itext->IText) + 10;
            /* if it has a command-key sequence, then add in the space needed
             * for the command key and the command symbol. Note that in
             * lo-res, we should actually use LOWCOMMWIDTH, but I'm
             * feeling lazy today.
             */

            if (item->item.Flags & COMMSEQ) {
               char buf[2];

               buf[0] = item->item.Command;
               buf[1] = 0;
               iwidth += text_width(buf) + COMMWIDTH + 8;
            }
	 }
         else {
            /* separator bars are always the same height */
            item->item.Height = 4;
         }

         /* update the y-position variable. Make sure the items don't
          * run off the bottom.
          */

         ypos  += item->item.Height;
	 if (ypos >= (sheight - DHBAR))return 0;

          /* now see if this item is the largest one. */
          if (iwidth > width) width = iwidth;
      }

      /* on our second pass, we set the widths of all the items to the
       * width of the largest item so that it looks good. In addition,
       * we are going to adjust menus that might run off the right edge.
       */

      for (item = (struct XItem *)menu->menu.FirstItem; item; item = (struct XItem *)item->item.NextItem) {
         item->item.Width = width;
         if ((xpos + width) >= (swidth - 2))item->item.LeftEdge = swidth - 2 - xpos - width;
      }

      menu->image.Width = width - 7;

      /* now update the x position variable, make sure that the menu
       * header is not off the edge of the screen, and add a little space
       * between the headers. Feel free to make this number whatever
       * you want. If you are feeling really ambitious, you could
       * make this number adaptable.
       */
      xpos += menu->menu.Width;
      if (xpos > swidth) return(NULL);
      xpos += 8;
   }
   return(TRUE);
}

/***********************************************************************************
 * Procedure: v_free_menus
 * Synopsis:  v_free_menus(Menu);
 * Purpose:   Return any storage allocated for a given Menu structure.
 ***********************************************************************************/
void v_free_menus(struct Menu *menus)
{
   struct XMenu *menu, *nextmenu;
   struct XItem *item, *nextitem;

      for (menu = (struct XMenu *)menus; menu; menu = nextmenu) {
         nextmenu = (struct XMenu *)menu->menu.NextMenu;
         for (item = (struct XItem *)menu->menu.FirstItem; item; item = nextitem) {
            nextitem = (struct XItem *)item->item.NextItem;
            free(item);
         }
         free(menu);
      }
}

/***********************************************************************************
 * Procedure: text_width
 * Synopsis:  len = text_width(str)
 * Purpose:   Return the rendered width of a given string
 ***********************************************************************************/
int text_width(char *str)
{
   struct IntuiText itext;

   itext.FrontPen  = 1;
   itext.BackPen   = 0;
   itext.DrawMode  = JAM1;
   itext.LeftEdge  = 0;
   itext.TopEdge   = 1;
   itext.ITextFont = &TOPAZ80;	// replace with actual font
   itext.NextText  = NULL;
   itext.IText     = str;

   return(IntuiTextLength(&itext));
}