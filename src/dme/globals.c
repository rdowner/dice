/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * GLOBALS.C
 */

#include "defs.h"

Prototype MLIST     DBase;
Prototype ED        *Ep;
Prototype char      Overide;
Prototype char      Savetabs;
Prototype char      memoryfail, Nsu, Msgchk;
Prototype ubyte     CtlC;
Prototype ubyte     Current[256];
Prototype ubyte     Deline[256];
Prototype ubyte     Space[32];
Prototype short     Clen;
Prototype char      Abortcommand, MShowTitle;
Prototype char      Comlinemode;
Prototype char      *Partial;
Prototype char      *String;

ED *Ep;                 /* Current Window               */
MLIST DBase;            /* Doubly linked list of Files  */

char    Nsu;            /* Used in formatter to disable screen updates  */
char    Msgchk;         /* Force message queue check for break          */
ubyte   CtlC;           /* Keycode for 'c'                              */
char    Savetabs;       /* SaveTabs mode?                               */
char    memoryfail;     /* out of memory flag                           */
ubyte   Deline[256];    /* last deleted line                            */
ubyte   Current[256];   /* Current Line buffer and length               */
ubyte   Space[32] = { 32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
                    32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32 };
short   Clen;
char    *Partial;       /* Partial command line when executing ESCIMM   */
char    *String;        /* String Capture variable                      */

char    Comlinemode;
char    Abortcommand;

long    BSline = -1;
long    BEline = -1;
short   BSchar;
short   BEchar;

ED      *BEp;

