#define GadToolsBase_DECLARED
#define AslBase_DECLARED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/gadtools.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <intuition/intuitionbase.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include "renderinfo.h"
#include "defaultfonts.h"
#undef GLOBAL

typedef struct WBStartup    WBStartup;
typedef struct DiskObject   DiskObject;
typedef struct IntuiText IntuiText;

#define ALT_CONFIG_FILE "DCC_Config:VOpts.Config"
#define CONFIG_FILE "DCC:Config/VOpts.config"

extern struct Library *GadToolsBase;
extern struct Library *AslBase;
extern struct Library *ArpBase;

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

#define MAX_LIST  3        /* Number of items to display in a list        */
#define MAX_MENU   64      /* Largest number of menu items to allow       */
#define MAX_TITLE  63      /* Longest titlebar string                     */

#define MENU_END  NM_END   /* Indicates the last item in a menu           */
#define MENU_MENU NM_TITLE /* Indicates a menu entry (with subitems)      */
#define MENU_ITEM NM_ITEM  /* Indicates a menu item                       */
#define MENU_SUB  NM_SUB   /* Indicates a submenu item                    */
#define MENU_BAR  4        /* Used for a bar between menu items           */

#define MAX_STRING 64      /* Largest allowable string                    */
#define MAX_FILENAME 255   /* Longest possible file name                  */

#define MODE_OUT 0
#define MODE_IN  1

#define MARGIN_TOP      4
#define MARGIN_BOTTOM   3
#define MARGIN_MID      4

#define MARGIN_LEFT     8
#define MARGIN_RIGHT    8
#define MARGIN_SEP     20

#define VBAR            2         /* Pixel width of a vertical BAR        */
#define DVBAR           (2*VBAR)  /* Pixel width of two vertical BARs     */
#define HBAR            1         /* Pixel height of a horizontal BAR     */
#define DHBAR           (2*HBAR)  /* Pixel height of two Horizontal BARs  */

#define CYC_ICON_WIDTH 24

#define CHECK_WIDTH    26
#define CHECK_HEIGHT   11

#define BUTTON_WIDTH   80

#define CLASS_STRING 1
#define CLASS_CYCLE  2
#define CLASS_CHECK  3
#define CLASS_GROUP  4
#define CLASS_LIST   5
#define CLASS_LIST1  6     /* Not really a gadget type                     */
#define CLASS_LIST2  7     /* Not really a gadget type                     */
#define CLASS_BUTTON 8
#define CLASS_ENTRY  9
#define CLASS_ADD   10     /* Not really a gadget type                     */
#define CLASS_DEL   11     /* Not really a gadget type                     */
#define CLASS_UP    12     /* Not really a gadget type                     */
#define CLASS_DOWN  13     /* Not really a gadget type                     */
#define CLASS_PROP  14     /* Not really a gadget type                     */
#define CLASS_REFRESH 15   /* Fake a refresh event                         */

#define BUTTON_SAVE   0    /* Save button on main window                   */
#define BUTTON_USE    1    /* Use button on main window                    */
#define BUTTON_CANCEL 2    /* Cancel button on main window                 */
#define BUTTON_NEW    3    /* NEW button on any list object                */
#define BUTTON_DEL    4    /* DEL button on any list object                */
#define MAX_BUTTON 6
/* these buttons aren't in the main buttongadget array                     */
#define BUTTON_FRSAVE 6    /* Default "file requester" Save button         */
#define BUTTON_FRCNCL 7    /* Default "file requester" Cancel button       */


#define TEXT_NOMEM      0       /* Not enough memory                    */
#define TEXT_BADOPT     1       /* Invalid command line option          */
#define TEXT_BADFILE    2       /* Unable to open the file              */
#define TEXT_BADCMD     3       /* Invalid command                      */
#define TEXT_EXTRAPRG   4       /* More than one #pragma in a file      */
#define TEXT_SAVEOK     5       /* Confirm OK to save options to file   */
#define TEXT_BIGOPTS    6       /* options string too long to write     */
#define TEXT_FILEREQ    7       /* string gadget file requester         */
#define TEXT_GLOBOPTS   8       /* Editing global options               */
#define TEXT_FILEOPTS   9       /* Editing options for file             */
#define TEXT_OVWRITEOK 10       /* Operation will overwrite existing file */
#define TEXT_NORENAME  11       /* Can't rename existing file           */
#define TEXT_NOREAD    12       /* Unable to read existing file         */
#define TEXT_NOWRITE   13       /* Error writing, unable to update      */
#define TEXT_NOOVWRITE 14       /* Can't overwrite existing file        */
#define TEXT_CANCEL    15       /* Cancel button for requesters         */
#define TEXT_OK        16       /* OK button for requesters             */
#define MAX_TEXT      18

#define FILE_ENV       1
#define FILE_OPTIONS   2
#define FILE_C         3
#define FILE_DMAKEFILE 4

struct G_OBJECT {
   struct G_OBJECT *next;  /* Next object in the list                      */
                           /* Note you must cast to use based on the class */
   struct G_OBJECT *prev;  /* Previous object in the list.                 */
   char             class; /* Class of the object                          */
   char             state; /* State (meaning is class based)               */
   char            *title; /* Title associated with the object.            */
   struct Gadget   *gadget;/* Intuition Gadget representing object         */
};

struct G_STRING {
   struct G_OBJECT  base;
   char            *option;
   char             buf[MAX_STRING];
};

struct G_VALUE {
   struct G_VALUE  *next;   /* Next cycle in the list                   */
   char            *title;  /* Title to print for this value in a cycle */
   char            *option; /* The option for this choice in the cycle  */
   struct G_STRING *string; /* For those cycles supporting a string     */
};

struct G_CYCLE {
   struct G_OBJECT  base;   /* Base object information                  */
   struct G_VALUE  *values; /* List of all possible values              */
   struct G_VALUE  *curval; /* Current value                            */
};

struct G_CHECK {
   struct G_OBJECT  base;   /* Base object information                  */
   char            *option0;/* Non-selected option value                */
   char            *option1;/* Selected option value                    */
};

struct G_ENTRY {
   struct G_OBJECT  base;   /* Base object information                  */
   char             buf[MAX_STRING]; /* Value for the current entry     */
};

struct G_LIST {
   struct G_OBJECT  base;   /* Base object information                  */
                            /* State shows which item is selected       */
   char            *option; /* Option value for all entries             */
   struct G_ENTRY  *first;  /* First option in the list                 */
   struct G_ENTRY  *top;    /* Top option being displayed               */
   char              string;/* Any string gadget for the entry          */
   char              maxent;/* Number of entries in the list            */
   struct Gadget   *delgad; /* The delete gadget                        */
   struct Gadget   *slider; /* The proportional gadget                  */
   struct Gadget   *strgad[MAX_LIST];  /* String gadgets for the list   */
   struct Gadget   *btngad[MAX_LIST];  /* Button gadgets for the list   */
   struct Border   *sborder;/* Border for any active strings            */
};

struct G_GROUP {
   struct G_OBJECT  base;
   struct G_OBJECT *objects;/* List of all objects in the current group */
   int              local;  /* 0 global, 1 exclude for env:             */
};

struct GADLIST {
   struct Gadget *gadgets;
   int            count;
};

/**********************************************************/
struct GLOBAL {
   struct RenderInfo  ri;
   struct Window     *window;
   FILE              *fp;
   int                line;
   int                state;
   int                done;
   int                width;
   int                height;
   int                iheight;
   int                eheight;  /* Height of an entry in a list */
   int                maxsize;
   int                stxtwid;  /* min width for string or list text entry */
   int                lgadwid;  /* width of list gaget slider and buttons  */
   int                boxx, boxy, boxw, boxh;
   struct NewMenu     menuitem[MAX_MENU+1];
   char               title[MAX_TITLE+1];
   char               wtitle[MAX_TITLE+1];
   struct G_OBJECT    button[MAX_BUTTON];
   struct G_OBJECT    *objects;            /* Base object list */
   struct G_GROUP     *groups;             /* Group list       */
   struct G_GROUP     *curgroup;
   struct GADLIST     *gadlist;
   struct GADLIST     *grpgadlist;
   struct RastPort    *rp;
   struct Border      *cycborder[2];
   struct Border      *checkborder[2];
   struct Border       arrowborder[2];
   struct Menu        *menu;
   struct GADLIST     *frgadlist;  /* stuff for default "file requester" */
   struct G_STRING    frstring;
   struct G_OBJECT    frbutton[2];
   int                cycsize[3];
   int                titsize[3];
   char               filename[MAX_FILENAME+1];
   int                filetype;
   struct FileRequester *freq;
   char               *text[MAX_TEXT];
   int                listextra;
   int                fileop; /* is default file request 0:read or 1:write? */
   int                nameoffile; /* 0 unless global.filename was read in */
   char               defbuf[MAX_STRING]; /* default buffer for list str gads */
};

extern struct GLOBAL global;

/***************************************************************************/
/* Stuff to make ARP work.                                                 */
/***************************************************************************/
#pragma libcall ArpBase ArpFileRequest 126 801
#pragma libcall ArpBase ArpAllocFreq   294 000

extern struct FileRequester *ArpAllocFreq(void);
extern char *ArpFileRequest(struct FileRequester *);

#define Prototype extern
#include "VOpts-protos.h"
