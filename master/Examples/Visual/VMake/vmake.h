#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include <proto/asl.h>
#include <libraries/gadtools.h>
#include <intuition/intuitionbase.h>
#include <exec/memory.h>
#include <lib/rexx.h>
#include <libraries/asl.h>
#include "renderinfo.h"
#include "defaultfonts.h"
#undef GLOBAL

#define ALT_CONFIG_FILE "DCC_Config:VMake.config"
#define CONFIG_FILE "DCC:Config/VMake.config"

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

extern struct Library        *GadToolsBase;
extern struct Library        *AslBase;
extern struct ExecBase       *SysBase;
extern struct IntuitionBase  *IntuitionBase;
extern struct GfxBase        *GfxBase;
extern struct Library        *ArpBase;

#define MAX_LIST       35   /* Number of items to display in a list        */
#define MAX_MENU       64   /* Largest number of menu items to allow       */
#define MAX_BUTTON      5
#define MAX_TITLE      64

#define MENU_END   NM_END   /* Indicates the last item in a menu           */
#define MENU_MENU NM_TITLE  /* Indicates a menu entry (with subitems)      */
#define MENU_ITEM NM_ITEM   /* Indicates a menu item                       */
#define MENU_SUB   NM_SUB   /* Indicates a submenu item                    */
#define MENU_BAR        4   /* Used for a bar between menu items           */

#define MAX_STRING     64   /* Largest allowable string                    */
#define MAX_FILENAME  255   /* Longest possible file name                  */
#define CBUF_SIZE_20 2000   /* Command buffer size for Dos 2.0 and higher  */
#define CBUF_SIZE_13  200   /* Command buffer size for Dos 1.3             */

#define MODE_OUT        0
#define MODE_IN         1

#define MARGIN_TOP      4
#define MARGIN_BOTTOM   3
#define MARGIN_MID      4

#define MARGIN_LEFT     8
#define MARGIN_RIGHT    8
#define RESIZE_WIDTH   14

#define VBAR            2   /* Pixel width of a vertical BAR               */
#define DVBAR    (2*VBAR)   /* Pixel width of two vertical BARs            */
#define HBAR            1   /* Pixel height of a horizontal BAR            */
#define DHBAR    (2*HBAR)   /* Pixel height of two Horizontal BARs         */

#define CYC_ICON_WIDTH 24

#define CHECK_WIDTH    26
#define CHECK_HEIGHT   11

#define BUTTON_WIDTH   80

#define CLASS_MASK    255   /* lower 8 bits are used for gadget class      */
#define SUBCLASS_OFF    8   /* Upper 8 bits available for sub index        */
#define SUBCLASS_BIT 1<<SUBCLASS_OFF

#define STATE_MASK    127   /* allow 7 bits to be used for state           */
#define DIRTY_BIT    1<<7   /* lives in upper bit of G_OBJ.state           */

#define CLASS_STRING    1
#define CLASS_CYCLE     2
#define CLASS_CHECK     3
#define CLASS_LIST      4
#define CLASS_BUTTON    5
#define CLASS_ADD       6   /* Not really a gadget type                    */
#define CLASS_DEL       7   /* Not really a gadget type                    */
#define CLASS_UP        8   /* Not really a gadget type                    */
#define CLASS_DOWN      9   /* Not really a gadget type                    */
#define CLASS_PROP     10   /* Not really a gadget type                    */
#define CLASS_REFRESH  14   /* Fake a refresh event                        */
#define CLASS_SELECT   15   /* Not a gadget type - do a list selection     */
#define CLASS_LTOP     16   /* Not a gadget type - select top of list      */
#define CLASS_LBOT     17   /* Not a gadget type - select bottom of list   */
#define CLASS_LUP      18   /* Not a gadget type - select item above       */
#define CLASS_LDN      19   /* Not a gadget type - select item below       */

#define TEXT_NEW        0
#define TEXT_DEL        1
#define TEXT_OK         2
#define TEXT_CANCEL     3
#define TEXT_CO         4   /* Check Out Button                               */
#define TEXT_SAVE       5   /* Save Project Button                            */

#define TEXT_PROJECT    6   /* Project:                                       */
#define TEXT_NOPROJECT  7   /* No Current Project                             */

#define TEXT_SCRIPTCO   8   /* Script is READ-ONLY.  Check out from RCS?      */
#define TEXT_BADFILE    9   /* Unable to open the file                        */
#define TEXT_IOERR     10   /* Error while accessing file                     */
#define TEXT_ASKCO     11   /* File is READ-ONLY.  You will not ... change it */
#define TEXT_ASKKILL   12   /* WARNING: This will REPLACE an existing project */
#define TEXT_ASKSAVE   13   /* WARNING: Project has been changed, .. be lost  */
#define TEXT_BADCMD    14   /* Invalid command                                */
#define TEXT_NOSEL     15   /* Nothing is selected to operate on              */
#define TEXT_NOMEM     16   /* Not enough memory                              */
#define TEXT_BADPROJ   17   /* Invalid project name                           */
#define TEXT_NOPROJ    18   /* No project currently open                      */
#define TEXT_BUFFOVFL  19   /* Not enough room in command buffer              */
#define TEXT_SELFAIL   20   /* Unable to select requested item                */
#define TEXT_BADPARM   21   /* Invalid or missing parm to command             */
#define TEXT_BADRMODE  22   /* Rexx mode must be "on" or "off"                */

#define NUM_TEXT       23   /* For range checking                             */

#define CONFIG_BASE    NUM_TEXT        /* No gaps after last TEXT_ entry!             */
#define CONFIG_CONSOLE CONFIG_BASE+0   /* Console to use for executing commands       */
#define CONFIG_PATTERN CONFIG_BASE+1   /* File requester pattern                      */
#define CONFIG_FILES   CONFIG_BASE+2   /* File requester pattern for file requests    */
#define CONFIG_CFGFPAT CONFIG_BASE+3   /* File requester pattern for config file      */
#define CONFIG_EXT     CONFIG_BASE+4   /* Extension to put on all projects            */
#define CONFIG_DCLICK  CONFIG_BASE+5   /* Command to execute for double click         */
#define CONFIG_EDIT    CONFIG_BASE+6   /* Issue an edit command                       */
#define CONFIG_CO      CONFIG_BASE+7   /* Command to issue to check out a file        */
#define CONFIG_BUILD   CONFIG_BASE+8   /* Executed for BUILD option on command line   */
#define CONFIG_EDPROJ  CONFIG_BASE+9   /* Edit command for project file               */

#define NUM_CONFIG     10    /* For range checking                             */

#define SUBRTN_BASE    CONFIG_BASE+NUM_CONFIG
#define NUM_SUBRTN     12   /* Space for user defined routines                */

#define MAX_TEXT       SUBRTN_BASE+NUM_SUBRTN /* number of items in global.text */

#define FILE_ENV        1
#define FILE_OPTIONS    2
#define FILE_C          3
#define FILE_DMAKEFILE  4

#define SYM_HOLD       "_FILE_"
#define SYM_SCRIPT     "_SCRIPT_"
#define SYM_REXXPORT   "_PORT_"
#define SYM_CONFIG     "_CONFIG_"
#define SYM_ORIG_CFG   "_ORIG_CFG_"
#define SYM_REXXINTER  "_REXXINTER_"

/***************************************************************************/
/*   Definition of objects used throughout VMake                           */
/***************************************************************************/
struct G_OBJECT {
   struct G_OBJECT *next;   /* Next object in the list                     */
                            /* Note you must cast to use based on the class*/
   struct G_OBJECT *prev;   /* Previous object in the list.                */
   char             class;  /* Class of the object                         */
   char             state;  /* State (meaning is class based)              */
                            /* Check: bool (is check selected?)            */
                            /* Button: button's own index                  */
                            /* List: Which (if any) string gadget selected */
   char            *title;  /* Title associated with the object.           */
   struct Gadget   *gadget; /* Intuition Gadget representing object        */
};

struct G_STRING {
   struct G_OBJECT  base;
   char            *option;
   char             buf[MAX_STRING];
   char             clean_buf[MAX_STRING];    /* to detect DIRTY string */
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

struct G_BUTTON {
   struct G_OBJECT  base;   /* Base object information                  */
   char            *command;/* Command for button to execute            */
};

struct G_CHECK {
   struct G_OBJECT  base;   /* Base object information                  */
   char            *option0;/* Non-selected option value                */
   char            *option1;/* Selected option value                    */
};

struct G_ENTRY {
   struct G_OBJECT  base;   /* Base object information                  */
                            /* State information indicates that the entry */
                            /* is a automatically generated entry that  */
                            /* does not need to be distributed          */
   char             buf[MAX_STRING]; /* Value for the current entry     */
};

struct G_LIST {
   struct G_OBJECT  base;   /* Base object information                  */
                            /* State shows which item is selected       */
   char            *option; /* Option value for all entries             */
   struct G_ENTRY  *first;  /* First option in the list                 */
   struct G_ENTRY  *top;    /* Top option being displayed               */
   char            spare1;  /* Was 'string' bool, replaced by 'sel' ptr */
   char              maxent;/* Number of entries in the list            */
   struct Gadget   *delgad; /* The delete gadget                        */
   struct Gadget   *slider; /* The proportional gadget                  */
   struct Gadget   *strgad[MAX_LIST];  /* String gadgets for the list   */
   struct Gadget   *btngad[MAX_LIST];  /* Button gadgets for the list   */
   struct Border   *sborder;/* Border for any active strings            */
   struct G_ENTRY  *sel;    /* Selected list entry, shown with border   */
};

struct GADLIST {
   struct Gadget *gadgets;
   int            count;
};

/***************************************************************************/
/***************************************************************************/
struct GLOBAL {
   struct RenderInfo  ri;
   struct Window     *window;
   FILE              *fp;
   struct Screen     *screen;
   int                line;
   int                state;
   int                done;
   int                newscreen;
   int                unghost;
   int                width;
   int                height;
   int                iheight;
   int                eheight;  /* Height of an entry in a list */
   int                boxx, boxy, boxw, boxh;
   struct NewMenu     menuitem[MAX_MENU+1];
   char               title[MAX_TITLE];
   char               title2[MAX_TITLE];
   struct G_BUTTON    button[MAX_BUTTON];
   struct GADLIST     *gadlist;
   struct G_OBJECT    *objects;            /* Base object list */
   struct G_LIST      *filelist;
   struct RastPort    *rp;
   struct Border      *cycborder;
   struct Border      *checkborder[2];
   struct Border       arrowborder[2];
   struct Menu        *menu;
   int                cycsize;
   int                titsize;
   int                listsize; /* Number of string gads in list gad */
   char               filename[MAX_FILENAME+1];
   struct FileRequester *freq;
   char               *text[MAX_TEXT];
   int                listextra;
   char               defbuf[MAX_STRING];
   BPTR               homedir;     /* Initial home directory */
   BPTR               workdir;     /* Work directory for all source */
   int                rexxinter; /* rexx is in interactive mode */
   int                inrexx;   /* currently processing a command from rexx */
   int                rexxmsgs; /* interactive rexx, messages & RCs only */
   int                rexxrc;   /* rexx return result code */
   char               *rexxrs;  /* rexx result string */
   int                parsefail; /* failed to read a config file */
   int                dirtysym; /* true after SET command changes a symbol */
   int                oldproject; /* true after READ, false after NEW */
   int                mouseprop;  /* process prop gadget for mouse move */
   int                cbufsize;  /* size of allocated command buffer */
   char               *cbuf;     /* buffer for building commands */
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
#include "VMake-protos.h"
