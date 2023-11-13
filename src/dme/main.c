/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * MAIN.C
 *
 */

#include "defs.h"
#include <exec/tasks.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <libraries/asl.h>
#include <proto/asl.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>

#if AREXX
#include <lib/rexx.h>
#endif

Prototype short Xsize, Ysize;
Prototype short XTbase, YTbase;
Prototype short Rows, Columns;
Prototype short Xbase, Ybase;
Prototype short Xpixs, Ypixs;

Prototype ubyte *av[];
Prototype struct FileRequester *FReq;
Prototype char *RexxHostName;
Prototype short IconSaveOpt;
Prototype struct WBStartup *Wbs;
Prototype struct Library *IconBase;


Prototype void ipchandler (void);
Prototype void initipc (void);
Prototype void do_ipc (void);
Prototype void do_iconify (void);
Prototype void do_iconsave(void);
Prototype void do_tomouse (void);
Prototype void iconify (void);
Prototype void uniconify (void);
Prototype void do_newwindow (void);
Prototype void do_openwindow(void);
Prototype struct Window *TOpenWindow (struct NewWindow *);
Prototype struct Window *opensharedwindow (struct NewWindow *);
Prototype void closesharedwindow (struct Window *);
Prototype int getyn (char *);
Prototype void title (char *);
Prototype void window_title (void);
Prototype void set_window_params (void);
Prototype void exiterr (char *);
Prototype int breakcheck (void);
Prototype void breakreset (void);
Prototype void do_resize (void);
Prototype int ops (char **, int);
Prototype char *geoskip(char *, int *, int *);
Prototype void GeometryToNW(char *, struct NewWindow *);

typedef struct Process      PROC;
typedef struct WBStartup    WBS;
typedef struct DiskObject   DISKOBJ;

#define IDCMPFLAGS   CLOSEWINDOW|NEWSIZE|RAWKEY|MOUSEBUTTONS|ACTIVEWINDOW|\
                     MOUSEMOVE|MENUPICK

struct NewWindow Nw = {
   0, 1, 0  , 0  , -1, -1,  /*  width, height filled in by program */
   IDCMPFLAGS,
   ACTIVATE|WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE|NOCAREREFRESH|RMBTRAP,
   NULL, NULL, (ubyte *)"   WAIT   ",
   NULL, NULL,
   32, 32, -1, -1,
   WBENCHSCREEN
};

short Sharedrefs;
short Oldtlen = 999;      /*  Old Title Length    */
struct MsgPort *Sharedport;
struct FileRequester *FReq;
static DISKOBJ *Do;
WBS     *Wbs;

short Xsize,  Ysize;            /* font character sizes        */
short Rows,  Columns;           /* character rows/cols available       */
short Xbase,  Ybase;            /* offset pixel base for display       */
short XTbase,YTbase;            /* used for text display               */
short Xpixs,  Ypixs;            /* actual # X/Y pixels available       */
short Mx, My;
short IconSaveOpt;

ubyte *av[8];
char Quitflag;
char Overide;
char MShowTitle, MForceTitle;

char    *RexxHostName = "DME";


long Mask;

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *IconBase;
struct Library *AslBase;

extern int Enable_Abort;

static char *Ffile;

int _bufsiz = 8192;

int main(int, char **);
int mybrk(void);

int
wbmain(wbs)
WBS *wbs;
{
    IconSaveOpt = 1;
    return(main(0, (char **)wbs));
}

int
main(mac, mav)
int mac;
char **mav;
{
    char nf, ni;            /*  # files on command line     */
    char notdone;           /*  for endless loop            */
    char iawm = 0;          /*  overide mouse buttons       */
    char dontwait = 0;      /*  don't wait for a message    */
    short i;
    short Code;
    PROC *proc = (PROC *)FindTask(NULL);
    BPTR origlock;

    onbreak(mybrk);
    fclose(stdin);
    /*fclose(stdout);*/     /*  assume person will run >nil:    */
    fclose(stderr);         /*  close stderr & console ref. */
    origlock = CurrentDir(DupLock((BPTR)proc->pr_CurrentDir));

    NewList((LIST *)&DBase);
    NewList((LIST *)&PBase);
    IntuitionBase = (void *)OpenLibrary("intuition.library", 0);
    GfxBase = (void *)OpenLibrary("graphics.library", 0);
    if (IntuitionBase == (void *)NULL || GfxBase == (void *)NULL)
        exiterr("cannot open intuition or graphics library");
    if (AslBase = OpenLibrary("asl.library", 0))
        FReq = AllocAslRequest(ASL_FileRequest, NULL);
    IconBase = OpenLibrary("icon.library", 0);

    init_command();

    String  = (char *)malloc(1);        /*  initialize scanf variable   */
    *String = 0;

    if (mac == 0) {             /*  WORKBENCH STARTUP   */
        Wbs = (WBS *)mav;
        if (IconBase == NULL)
            exiterr("unable to open icon library");
    }

    resethash();

    if (Wbs) {
        if (Wbs->sm_ArgList[0].wa_Lock) {
            BPTR savelock = CurrentDir((BPTR)Wbs->sm_ArgList[0].wa_Lock);
            if (Do = GetDiskObject(Wbs->sm_ArgList[0].wa_Name)) {
                ops(Do->do_ToolTypes, 1);
                FreeDiskObject(Do);
            }
            CurrentDir(savelock);
        }
        nf = Wbs->sm_NumArgs - 1;
        mac = 99;
    } else {
        nf = ops(mav+1, 0);
    }

    for (ni = 0, i = 1; i < mac; ++i) {
        char *str;
        DISKOBJ *dso;
        if (Wbs) {
            if (i > nf)
                break;
            str = Wbs->sm_ArgList[i].wa_Name;
            UnLock(CurrentDir(DupLock((BPTR)Wbs->sm_ArgList[i].wa_Lock)));
            if (dso = GetDiskObject(Wbs->sm_ArgList[i].wa_Name)) {
                ops(dso->do_ToolTypes, 1);
                FreeDiskObject(dso);
            }
        } else {
            str = mav[i];
            if (*str == '-')
                continue;
        }
        do_newwindow();
        ++ni;
        av[0] = (ubyte *)"newfile";
        av[1] = (ubyte *)str;
        do_edit();
        MForceTitle = 1;
        window_title();
    }
    if (nf == 0)                    /* no files to edit */
        do_newwindow();

#if AREXX
    init_arexx();
#endif

    mountrequest(0);
    av[0] = NULL;
    av[1] = (ubyte *)"s:.edrc";
    do_source();
    av[0] = NULL;
    av[1] = (ubyte *)((Ffile) ? Ffile : ".edrc");
    do_source();
    mountrequest(1);
    {                       /*  1.29c   */
        ED *ep;
        ED *eb = Ep;
        if (eb) {
            for (ep = (ED *)eb->Node.mln_Succ; ep->Node.mln_Succ; ep = (ED *)ep->Node.mln_Succ) {
                ep->Tabstop = eb->Tabstop;
                ep->Margin  = eb->Margin;
                ep->Insertmode = eb->Insertmode;
                ep->IgnoreCase = eb->IgnoreCase;
                ep->Wordwrap   = eb->Wordwrap;
                if (eb->Font) {
                    ep->Font = eb->Font;
                    ++eb->Font->tf_Accessors;
                }
            }
        }
    }
    title("DME V1.50 \251Copyright 1988-1994 by Matthew Dillon,  All Rights Reserved             ");
    Mask |= 1 << Ep->Win->UserPort->mp_SigBit;
#if AREXX
    Mask |= 1 << RexxSigBit;
#endif
loop:
    if (!Ep->iconmode)
        text_cursor(1);
    for (notdone = 1; !Quitflag && notdone;) {
        char mmove = 0;
        short mqual;
        long mask;

        if (!Ep->iconmode)
            window_title();
        if (dontwait) {
            --dontwait;
            mask = FindTask(NULL)->tc_SigRecvd;
        } else {
            mask = Wait(Mask);
        }

        /*
         *  NOTE: due to operation of breakcheck(), the userport signal
         *  may not be set even if there are messages pending.
         */

        {
            IMESS *im;
            while (im = (IMESS *)GetMsg(Ep->Win->UserPort)) {
                Msgchk = 1;
                Abortcommand = 0;
                Code = im->Code;
                if (im->IDCMPWindow != Ep->Win) {
                    Overide = 0;
                    if (Comlinemode)
                        escapecomlinemode();
                    text_sync();
                    MShowTitle = 0;
                    if (!Ep->iconmode)
                        window_title();
                    if (text_switch(im->IDCMPWindow) == 0) {
                        ReplyMsg((MSG *)im);
                        continue;
                    }
                }
                Mx = im->MouseX;
                My = im->MouseY;
                switch(im->Class) {
                case NEWSIZE:
                    if (!Ep->iconmode) {
                        if (Comlinemode)
                            escapecomlinemode();
                        set_window_params();
                        if (!text_sync())
                            text_redisplay();
                        text_cursor(1);
                    }
                    break;
                case MOUSEBUTTONS:
                    switch(Code) {
                    case SELECTDOWN:
                    case MENUDOWN:
                        if (Ep->iconmode || iawm) {
                            uniconify();
                            text_cursor(1);
                            break;
                        }
                        Forbid();
                        Ep->Win->Flags |= REPORTMOUSE;
                        Permit();
                        uniconify();
                        text_cursor(0);
                        keyctl(NULL, im->Code|0x80, im->Qualifier);
                        text_cursor(1);
                        break;
                    case SELECTUP:
                    case MENUUP:
                        Forbid();
                        Ep->Win->Flags &= ~REPORTMOUSE;
                        Permit();
                        break;
                    }
                    break;
                case RAWKEY:
                    if ((im->Code & 0x80) == 0) {
                        /*  Handled in command interpreter.
                        if (Ep->iconmode) {
                            uniconify();
                            break;
                        }
                        */
                        text_cursor(0);
                        keyctl(im, im->Code, im->Qualifier);
                        text_cursor(1);
                    }
                    break;
                case MENUPICK:
                    {
                        char *str = menu_cmd(im);
                        if (str) {
                            str = strcpy(malloc(strlen(str)+1), str);
                            text_cursor(0);
                            do_command(str);
                            free(str);
                            text_cursor(1);
                        }
                    }
                    break;
                case CLOSEWINDOW:
                    if (Comlinemode)
                        escapecomlinemode();
                    text_sync();
                    notdone = 0;
                    break;
                case ACTIVEWINDOW:
                    if (!Ep->iconmode)
                        iawm = 1;
                    break;
                case MOUSEMOVE:
                    mmove = 1;
                    mqual = im->Qualifier;
                    break;
                }
                if (im)
                    ReplyMsg((MSG *)im);
                if (notdone == 0 || Quitflag) {
                    dontwait = 2;
                    goto boom;
                }
            }
        }
        if (mask & (1 << RexxSigBit)) {
            if (!Ep->iconmode)
                text_cursor(0);
            ProcessRexxCommands(NULL);
            if (!Ep->iconmode)
                text_cursor(1);
        }

        iawm = 0;
        if (mmove) {
            uniconify();
            mmove = 0;
            text_cursor(0);
            keyctl(NULL, QMOVE, mqual);
            text_cursor(1);
        }
        closesharedwindow(NULL);
    }
boom:
    text_sync();
    if (Ep->Modified && !Overide) {
        uniconify();
        Overide = 1;
        title("*** File has been modified ***");
        Quitflag = 0;
        goto loop;
    }
    SetWindowTitles(Ep->Win, "", (char *)-1);
    {
        WIN *win = Ep->Win;
        text_uninit();
        closesharedwindow(win);
    }
    if (Ep) {
        Quitflag = 0;
        if (!Ep->iconmode)
            set_window_params();
        text_load();
        MShowTitle = 0;
        goto loop;
    }
    closesharedwindow(NULL);
    UnLock(CurrentDir(origlock));
    if (IconBase)
        CloseLibrary((void *)IconBase);
    if (FReq)
        FreeAslRequest(FReq);
    if (AslBase)
        CloseLibrary(AslBase);
    if (GfxBase)
        CloseLibrary((void *)GfxBase);
    if (IntuitionBase)
        CloseLibrary((void *)IntuitionBase);
    IconBase = NULL;
    GfxBase  = NULL;
    IntuitionBase = NULL;
    dealloc_hash();
    return(0);
}

int
mybrk()
{
    return(0);
}

void
do_iconify()
{
    text_sync();
    if (!Comlinemode)
        iconify();
}

void
do_iconsave()
{
    static short LastIconSaveOpt;

    if (av[1][0] == 'l' || av[1][0] == 'L') {
        IconSaveOpt = LastIconSaveOpt;
        return;
    }
    LastIconSaveOpt = IconSaveOpt;
    if (av[1][0] == 'y' || av[1][0] == 'Y' || stricmp(av[1], "on") == 0)
        IconSaveOpt = 1;
    else
        IconSaveOpt = 0;

}

void
do_tomouse()
{
    text_position((Mx-Xbase)/Xsize, (My-Ybase)/Ysize);
}

/*
 *  New iconify() routine by fgk.
 */

void
iconify()
{
    WIN *newwin;
    ED *ep = Ep;
    WIN *win = ep->Win;

    struct IntuiText itxt;              /* To find width of prop fonts */

    itxt.ITextFont = Ep->Win->WScreen->Font;    /* Init */
    itxt.NextText = NULL;

    if (!ep->iconmode) {
        ep->Winx      = win->LeftEdge;
        ep->Winy      = win->TopEdge;
        ep->Winwidth  = win->Width;
        ep->Winheight = win->Height;

        /*Nw.Height = win->RPort->TxHeight + 3;*/


        if(Ep->Win->WScreen->Font != NULL)
            Nw.Height = Ep->Win->WScreen->Font->ta_YSize + 3;   /* height */
        else
            Nw.Height = GfxBase->DefaultFont->tf_YSize + 3;

        /*Nw.Width  = 20 + 5*8 + strlen(ep->Name) * (win->RPort->TxWidth + win->RPort->
TxSpacing);*/

        itxt.IText = ep->Name;

        /* pretending spaces are always 8 */
        Nw.Width  = 20 + 5*8 + IntuiTextLength(&itxt);          /* width */


        Nw.LeftEdge= ep->IWinx;
        Nw.TopEdge = ep->IWiny;

        if (Nw.LeftEdge + Nw.Width > win->WScreen->Width)   /* keep in bounds */
            Nw.LeftEdge = win->WScreen->Width - Nw.Width;

        if (Nw.TopEdge + Nw.Height > win->WScreen->Height)
            Nw.TopEdge = win->WScreen->Height - Nw.Height;

        Nw.Title = ep->Wtitle;

        Nw.Flags &= ~(WINDOWSIZING|WINDOWDEPTH|ACTIVATE);

        if (ep->Modified) {     /* no CLOSE */
            Nw.Width -= 3*8;
            Nw.Flags &= ~WINDOWCLOSE;
        }

       /******* Nw.Flags |= BORDERLESS;*******/

        Nw.DetailPen = ep->BGPen;
        Nw.BlockPen  = ep->FGPen;
        if (win->Flags & WINDOWACTIVE)      /*  KTS */
            Nw.Flags |= ACTIVATE;
        sprintf(ep->Wtitle, "%s     ", ep->Name);

        if (newwin = opensharedwindow(&Nw)) {
            closesharedwindow(win);
            Nw.BlockPen = -1;
            ep->iconmode = 1;
            ep->Win = newwin;
        }

        Nw.Flags |= WINDOWSIZING|WINDOWDEPTH|WINDOWCLOSE|ACTIVATE;
        Nw.Flags &= ~BORDERLESS;
    }
}


#ifdef NOTDEF       /*  old iconify routine */
void
iconify()
{
    WIN *newwin;
    ED *ep = Ep;
    WIN *win = ep->Win;

    if (!ep->iconmode) {
        ep->Winx      = win->LeftEdge;
        ep->Winy      = win->TopEdge;
        ep->Winwidth  = win->Width;
        ep->Winheight = win->Height;
        Nw.Height = win->RPort->TxHeight + 3;
        Nw.Width  = 20 + 5*8 + strlen(ep->Name) * (win->RPort->TxWidth + win->RPort->TxSpacing);
        if (ep->Modified)
            Nw.Width -= 3*8;
        Nw.LeftEdge= ep->IWinx;
        Nw.TopEdge = ep->IWiny;
        if (Nw.LeftEdge + Nw.Width > win->WScreen->Width)
            Nw.LeftEdge = win->WScreen->Width - Nw.Width;
        if (Nw.TopEdge + Nw.Height > win->WScreen->Height)
            Nw.TopEdge = win->WScreen->Height - Nw.Height;
        Nw.Title = ep->Wtitle;
        Nw.Flags &= ~(WINDOWSIZING|WINDOWDEPTH|ACTIVATE);
        if (ep->Modified)
            Nw.Flags &= ~WINDOWCLOSE;
        Nw.Flags |= BORDERLESS;
        Nw.DetailPen = ep->BGPen;
        Nw.BlockPen  = ep->FGPen;
        if (win->Flags & WINDOWACTIVE)      /*  KTS */
            Nw.Flags |= ACTIVATE;
        sprintf(ep->Wtitle, "%s     ", ep->Name);
        if (newwin = opensharedwindow(&Nw)) {
            closesharedwindow(win);
            Nw.BlockPen = -1;
            ep->iconmode = 1;
            ep->Win = newwin;
        }
        Nw.Flags |= WINDOWSIZING|WINDOWDEPTH|WINDOWCLOSE|ACTIVATE;
        Nw.Flags &= ~BORDERLESS;
    }
}
#endif

void
uniconify()
{
    ED *ep = Ep;
    WIN *win = ep->Win;
    WIN *newwin;
    RP *rp;

    if (ep->iconmode) {
        ep->IWinx = win->LeftEdge;
        ep->IWiny = win->TopEdge;
        Nw.LeftEdge = ep->Winx;
        Nw.TopEdge  = ep->Winy;
        Nw.Width    = ep->Winwidth;
        Nw.Height   = ep->Winheight;
        Nw.Title    = ep->Wtitle;
        Nw.DetailPen = ep->BGPen;
        Nw.BlockPen  = ep->FGPen;

        if (newwin = opensharedwindow(&Nw)) {
            closesharedwindow(win);
            win= ep->Win = newwin;
            rp = win->RPort;

            menu_strip(win);
            if (ep->Font)
                SetFont(rp, ep->Font);
            set_window_params();
            if (!text_sync())
                text_redisplay();
            /** text_cursor(1); **/
            MShowTitle = 0;
            window_title();
            ep->iconmode = 0;
        }
    }
}


void
do_newwindow()
{
    WIN *win;

    if (Ep)
        text_sync();
    Nw.Title = (ubyte *)"    OK    ";

    if (text_init(Ep, NULL, &Nw)) {
        if (win = opensharedwindow(&Nw)) {
            menu_strip(win);
            Ep->Win = win;
            set_window_params();
            text_load();
        } else {
            text_uninit();
        }
    }
}

/*
 *  openwindow with geometry specification.  Negative number specify
 *  relative-right / relative-left (leftedge & topedge), or relative-width /
 *  relative height (width & height).
 *
 *      <leftedge><topedge><width><height>
 *
 *  Example:    +10+10-20-20    Open window centered on screen 10 pixels
 *                              from the border on all sides.
 */

void
do_openwindow()
{
    WIN *win;

    if (Ep)
        text_sync();
    Nw.Title = (ubyte *)"    OK    ";

    if (text_init(Ep, NULL, &Nw)) {
        GeometryToNW(av[1], &Nw);
        if (win = opensharedwindow(&Nw)) {
            menu_strip(win);
            Ep->Win = win;
            set_window_params();
            text_load();
        } else {
            text_uninit();
        }
    }
}


WIN *
TOpenWindow(nw)
struct NewWindow *nw;
{
    WIN *win;

    while ((win = OpenWindow(nw)) == NULL) {
        if (nw->Width < 50 || nw->Height < 50)
            break;
        nw->Width -= 10;
        nw->Height-= 10;
    }
    return(win);
}


WIN *
opensharedwindow(nw)
struct NewWindow *nw;
{
    WIN *win;

    if (Sharedport)
        nw->IDCMPFlags = NULL;
    else
        nw->IDCMPFlags = IDCMPFLAGS;
    win = TOpenWindow(nw);
    if (win) {
        long xend = win->Width - win->BorderRight - 1;
        long yend = win->Height- win->BorderBottom - 1;
        if (Sharedport) {
            win->UserPort = Sharedport;
            ModifyIDCMP(win, IDCMPFLAGS);
        } else {
            Sharedport = win->UserPort;
        }
        ++Sharedrefs;
        if (xend > win->BorderLeft && yend > win->BorderTop) {
            SetAPen(win->RPort, nw->DetailPen);
            RectFill(win->RPort, win->BorderLeft, win->BorderTop, xend, yend);
            SetAPen(win->RPort, nw->BlockPen);
        }
    }
    return(win);
}

void
closesharedwindow(win)
WIN *win;
{
    static WIN *wunlink;
    char notoktoclosenow = 0;

    if (win) {
        SetWindowTitles(win, "", (char *)-1);
        ClearMenuStrip(win);
        Forbid();
        win->UserPort = NULL;
        ModifyIDCMP(win, GADGETUP);     /* NEVER occurs */

        notoktoclosenow = 1;

        Permit();
        if (notoktoclosenow) {
            win->UserData = (char *)wunlink;
            wunlink = win;
        } else {
            CloseWindow(win);
        }
        --Sharedrefs;
    } else {
        if (Sharedrefs == 0 && Sharedport) {
            DeletePort(Sharedport);
            Sharedport = NULL;
        }
        for (win = wunlink; win; win = wunlink) {
            wunlink = (WIN *)win->UserData;
            CloseWindow(win);
        }
        wunlink = NULL;
    }
}


getyn(text)
char *text;
{
    int result;
    ITEXT *body, *pos, *neg;

    body = (ITEXT *)AllocMem(sizeof(ITEXT), 0);
    pos  = (ITEXT *)AllocMem(sizeof(ITEXT), 0);
    neg  = (ITEXT *)AllocMem(sizeof(ITEXT), 0);
    clrmem(body, sizeof(ITEXT));
    clrmem(pos , sizeof(ITEXT));
    clrmem(neg , sizeof(ITEXT));
    body->BackPen = pos->BackPen = neg->BackPen = 1;
    body->DrawMode= pos->DrawMode= neg->DrawMode= AUTODRAWMODE;
    body->LeftEdge = 10;
    body->TopEdge  = 12;
    body->IText    = (ubyte *)text;
    pos->LeftEdge = AUTOLEFTEDGE;
    pos->TopEdge = AUTOTOPEDGE;
    pos->IText = (ubyte *)"OK";
    neg->LeftEdge = AUTOLEFTEDGE;
    neg->TopEdge = AUTOTOPEDGE;
    neg->IText = (ubyte *)"CANCEL";
    result = AutoRequest(Ep->Win,body,pos,neg,0,0,320,58);
    FreeMem(body, sizeof(ITEXT));
    FreeMem(pos , sizeof(ITEXT));
    FreeMem(neg , sizeof(ITEXT));
    return(result);
}

void
title(buf)
char *buf;
{
    SetWindowTitles(Ep->Win, buf, (char *)-1);
    Oldtlen = 999;
    MShowTitle = 3;
}

void
window_title()
{
    int len, maxlen;

    if (memoryfail) {
        title(" -- NO MEMORY -- ");
        memoryfail = 0;
        text_redisplay();
    }
    if (MForceTitle) {
        MShowTitle = 0;
        MForceTitle = 0;
    }
    if (MShowTitle) {
        --MShowTitle;
        return;
    }
    {
        char *mod;
        FONT *oldfont;
        ED *ep = Ep;
        WIN *win = ep->Win;
        RP *rp = win->RPort;

        mod = (ep->Modified) ? " (modified)" : "          ";
        sprintf(ep->Wtitle, "%3ld/%-3ld %3ld %s%s  ",
            text_lineno(),
            text_lines(),
            text_colno()+1,
            text_name(),
            mod
        );
        if (!text_imode())
            strcat(ep->Wtitle, "Ovr ");
        len = strlen(ep->Wtitle);
        if (len < Columns && Columns < 128) {
            setmem(ep->Wtitle+len, Columns - len + 1, ' ');
            ep->Wtitle[Columns + 1] = 0;
        }

        /*
         *  Update title
         */

        if (IntuitionBase->LibNode.lib_Version >= 36) {
            SetWindowTitles(win, ep->Wtitle, (char *)-1);
        } else {
            oldfont = win->RPort->Font;
            SetFont(rp, win->WScreen->RastPort.Font);

            win->Title = ep->Wtitle;
            SetAPen(rp, ep->FGPen);
            SetBPen(rp, ep->TPen);
            Move(rp, 30, rp->Font->tf_Baseline+1);
            maxlen = (win->Width-96)/rp->Font->tf_XSize;
            if (maxlen < 0)
                maxlen = 0;
            if (len > maxlen)
                len = Oldtlen = maxlen;
            if (Oldtlen > maxlen)
                Oldtlen = maxlen;
            Text(rp, ep->Wtitle, len);      /*  No flash                    */
            while (Oldtlen - len >= (int)sizeof(Space)) {
                Text(rp, Space, sizeof(Space));
                Oldtlen -= sizeof(Space);
            }
            if (Oldtlen - len > 0)
                Text(rp, Space, Oldtlen - len);
            Oldtlen = len;                  /*  Oldtlen might have been <   */
            SetAPen(rp, ep->FGPen);
            SetBPen(rp, ep->BGPen);
            SetFont(rp, oldfont);
        }
    }
}

void
set_window_params()
{
    ED *ep = Ep;
    WIN *win = ep->Win;
    RP  *rp = win->RPort;

    Xsize = rp->Font->tf_XSize;
    Ysize = rp->Font->tf_YSize;
    Xbase = win->BorderLeft;
    Ybase = win->BorderTop;
    Xpixs   = win->Width - win->BorderRight - Xbase;
    Ypixs   = win->Height- win->BorderBottom- Ybase;
    Columns = Xpixs / Xsize;
    Rows    = Ypixs / Ysize;
    if (Columns <= 0)
        Columns = 1;
    if (Rows <= 0)
        Rows = 1;
    XTbase  =  Xbase;
    YTbase  =  Ybase + rp->Font->tf_Baseline;
    SetAPen(rp, ep->FGPen);
    SetBPen(rp, ep->BGPen);
}

void
exiterr(str)
char *str;
{
    if (Output()) {
        Write(Output(),str,strlen(str));
        Write(Output(),"\n",1);
    }
    exit(1);
}


/*
 *  Check break by scanning pending messages in the I stream for a ^C.
 *  Msgchk forces a check, else the check is only made if the signal is
 *  set in the I stream (the signal is reset).
 */

breakcheck()
{
    IMESS *im;
    WIN *win = Ep->Win;
    struct List *list = &win->UserPort->mp_MsgList;

    if (Msgchk || (SetSignal(0,0) & (1<<win->UserPort->mp_SigBit))) {
        Msgchk = 0;
        SetSignal(0,1<<win->UserPort->mp_SigBit);

        im = (IMESS *)list->lh_Head;
        Forbid();
        for (; im != (IMESS *)&list->lh_Tail; im = (IMESS *)im->ExecMessage.mn_Node.ln_Succ) {
            if (im->Class == RAWKEY && (im->Qualifier & 0xFB) == 0x08 &&
                im->Code == CtlC) {

                Permit();
                SetSignal(SIGBREAKF_CTRL_C,SIGBREAKF_CTRL_C);
                return(1);
            }
        }
        Permit();
    }
    return(0);
}

void
breakreset()
{
    SetSignal(0, SIGBREAKF_CTRL_C);
}

/*
 *  resize cols rows
 */

void
do_resize()
{
    WIN *win = Ep->Win;
    int cols = atoi(av[1]);
    int rows = atoi(av[2]);
    short width = (cols*win->RPort->Font->tf_XSize) + win->BorderLeft + win->BorderRight;
    short height= (rows*win->RPort->Font->tf_YSize) + win->BorderTop + win->BorderBottom;

    if (width < 16 || height < 16 ||
    width > win->WScreen->Width - win->LeftEdge ||
    height > win->WScreen->Height - win->TopEdge) {
        title ("window too big (try moving to upper left corner and retrying)");
        return;
    }
    SizeWindow(win, width - win->Width, height - win->Height);
    Delay(50*2);    /* wait 2 seconds */
}

ops(av, iswb)
char **av;
{
    short nonops;
    short i;
    long val;
    char *str;

    if (av == NULL)
        return(0);
    for (i = nonops = 0; str = av[i]; ++i) {
        if (iswb) {
            if (strncmp(str, "ARG", 3) == 0) {
                while (*str && *str != '-')
                    ++str;
            }
        }
        if (*str == '-') {
            val = atoi(str+2);
            switch(str[1]) {
            case 'f':
                Ffile = str+2;
                break;
            case 'b':
                /*SizeOveride = 1;*/
                break;
            case 't':
                /*Nwtopedge = val;*/
                break;
            case 'l':
                /*Nwleftedge= val;*/
                break;
            case 'w':
                /*SizeOveride = 1;*/
                /*Nwwidth   = val;*/
                break;
            case 'h':
                /*SizeOveride = 1;*/
                /*Nwheight  = val;*/
                break;
            }
        } else {
            ++nonops;
        }
    }
    return((int)nonops);
}

/*
 *  Convert geometry to nw params.
 */

char *
geoskip(ptr, pval, psgn)
char *ptr;
int *pval;
int *psgn;
{
    if (*ptr == '-')
        *psgn = -1;
    else
        *psgn = 1;
    if (*ptr == '-' || *ptr == '+')
        ++ptr;
    *pval = atoi(ptr);
    while (*ptr >= '0' && *ptr <= '9')
        ++ptr;
    return(ptr);
}

void
GeometryToNW(geo, nw)
char *geo;
struct NewWindow *nw;
{
    int n;
    int sign;
    struct Screen scr;

    GetScreenData(&scr, sizeof(scr), WBENCHSCREEN, NULL);

    if (*geo) {
        geo = geoskip(geo, &n, &sign);
        if (sign > 0)
            nw->LeftEdge = n;
        else
            nw->LeftEdge = scr.Width - n;
    }
    if (*geo) {
        geo = geoskip(geo, &n, &sign);
        if (sign > 0)
            nw->TopEdge = n;
        else
            nw->TopEdge = scr.Height - n;
    }
    if (*geo) {
        geo = geoskip(geo, &n, &sign);
        if (sign > 0)
            nw->Width = n;
        else
            nw->Width = scr.Width - nw->LeftEdge - n;
    }
    if (*geo) {
        geo = geoskip(geo, &n, &sign);
        if (sign > 0)
            nw->Height = n;
        else
            nw->Height = scr.Height - nw->TopEdge - n;
    }
}


