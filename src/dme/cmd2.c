/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * CMD2.C
 */

#include "defs.h"
#include <workbench/startup.h>
#include <workbench/workbench.h>

Prototype struct _ED *uninit_init (struct _ED *);
Prototype void do_remeol (void);
Prototype void do_wleft (void);
Prototype void do_wright (void);
Prototype void do_split (void);
Prototype int do_join (void);
Prototype void do_margin (void);
Prototype void do_wordwrap (void);
Prototype void do_setparcol (void);
Prototype void do_reformat (int);
Prototype void do_tabstop (void);
Prototype void do_insertmode (void);
Prototype void do_insline (void);
Prototype void do_deline (void);
Prototype void do_chfilename (void);
Prototype void do_edit (void);
Prototype void do_bsave (void);
Prototype void do_save (void);
Prototype void do_saveas (void);
Prototype void do_savetabs (void);
Prototype void do_block (void);
Prototype int blockok (void);
Prototype void do_bdelete (void);
Prototype void do_bcopy (void);
Prototype void do_bmove (void);
Prototype void do_if (void);
Prototype int do_toggle (int);
Prototype void do_tlate (void);
Prototype void do_bsource (void);
Prototype void do_scanf (void);
Prototype void do_bappend(void);
Prototype void do_append(void);
Prototype int extend (struct _ED *, int);
Prototype int makeroom (int);
Prototype void freelist (char **, int);

typedef struct DiskObject   DISKOBJ;

#define nomemory()  {memoryfail = 1;}

extern char MForceTitle;
extern void do_bmove();

ED *
uninit_init(ep)
ED *ep;
{
    FONT  *font;
    BPTR lock = DupLock((BPTR)ep->dirlock);
    WIN *win;
    int bufsiz = offsetof(struct _ED,EndConfig) - offsetof(struct _ED, BeginConfig);
    char *buf = malloc(bufsiz);

    movmem(&ep->BeginConfig, buf, bufsiz);
    win = ep->Win;
    font = ep->Font;
    ep->Font = NULL;
    text_uninit();
    text_init(Ep, NULL, NULL);
    ep = Ep;
    if (ep->Font)
        CloseFont(ep->Font);
    ep->Win = win;
    ep->Font = font;

    movmem(buf, &ep->BeginConfig, bufsiz);

    ep->Modified = 0;
    ep->Line = ep->Topline = 0;
    UnLock((BPTR)ep->dirlock);
    ep->dirlock = (long)lock;

    free(buf);
    return(ep);
}

void
do_remeol()
{
    Current[Clen = Ep->Column] = 0;
    text_sync();
    text_redisplaycurrline();
}

void
do_wleft()
{
    ED *ep = Ep;
    int i;

    for (;;) {
        i = ep->Column;
        if (i == 0)
            goto prevline;
        --i;
        while (i && Current[i] == ' ')
            --i;
        if (i == 0 && Current[0] == ' ') {
prevline:
            if (Comlinemode || ep->Line == 0) {
                i = ep->Column;
                break;
            }
            text_sync();
            --ep->Line;
            text_load();
            ep->Column = Clen;
            continue;
        }
        while (i && Current[i] != ' ')
            --i;
        if (Current[i] == ' ')
            ++i;
        break;
    }
    ep->Column = i;
    text_sync();
}

void
do_wright()
{
    ED *ep = Ep;
    int i;

    for (;;) {
        i = ep->Column;
        if (i == Clen)
            goto nextline;
        while (i != Clen && Current[i] != ' ')  /* skip past current word */
            ++i;
        while (i != Clen && Current[i] == ' ')  /* to beg. of next word   */
            ++i;
        if (i == Clen) {
nextline:
            if (Comlinemode || ep->Line == ep->Lines - 1) {
                i = ep->Column;
                break;
            }
            text_sync();
            ++ep->Line;
            text_load();
            ep->Column = i = 0;
            if (Current[0] != ' ')
                break;
            continue;
        }
        break;
    }
    ep->Column = i;
    text_sync();
}

void
do_split()              /* split line in two at cursor pos */
{
    ubyte buf[256];
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;
    char onLastLine;

    strcpy(buf, Current+ep->Column);
    Current[Clen = ep->Column] = '\0';
    text_sync();
    SetAPen(rp, ep->BGPen);
    if (Nsu == 0)
        RectFill(rp, COL(0), ROW(ep->Line-ep->Topline), Xbase+Xpixs-1, ROW(ep->Line-ep->Topline+1)-1);
    SetAPen(rp, ep->FGPen);
    text_displayseg(ep->Line - ep->Topline, 1);

    onLastLine = (ep->Line == ep->Lines-1);

    do_downadd();
    if (!onLastLine)
        do_insline();
    strcpy(Current, buf);
    Clen = strlen(Current);
    text_sync();
    text_displayseg(ep->Line - ep->Topline, 1);
    do_up();
}

do_join()
{
    int i = Clen, j;
    ED *ep = Ep;

    if (ep->Line + 1 < ep->Lines && strlen(ep->List[ep->Line+1])+i <= 253) {
        if (i && Current[i-1] != ' ')
            Current[i++] = ' ';
        strcpy(Current+i, ep->List[ep->Line+1]);
        for (j = i; Current[j] == ' '; ++j);
        for (; i >= 0 && Current[i] == ' '; --i);
        if (j > i+2)
            movmem(Current+j, Current+i+2, strlen(Current+j)+1);
        Clen = strlen(Current);
        text_sync();
        text_displayseg(ep->Line - ep->Topline, 1);
        {
            int l = text_lineno();
            do_down();
            do_deline();
            if (l != text_lineno())
                do_up();
        }
        return(1);
    }
    return(0);
}

void
do_margin()
{
    Ep->Margin = atoi(av[1]);
}

void
do_wordwrap()
{
    ED *ep = Ep;

    if (av[1][1] == 'n')
        ep->Wordwrap = 1;
    if (av[1][1] == 'f')
        ep->Wordwrap = 0;
    if (av[1][0] == 't')
        ep->Wordwrap = 1 - ep->Wordwrap;
    if (ep->Wordwrap)
        title("Wordwrap ON");
    else
        title("Wordwrap OFF");
}

void
do_setparcol()
{
    Ep->WWCol = atoi(av[1]);
}

/*
 * n == -1  :   force reformat entire paragraph
 * n ==  0  :   only until line equalizes (from text_write())
 *
 * What is a paragraph?   A paragraph ends whenever the left justification
 * gets larger, or on a blank line.
 */

void
do_reformat(n)
{
    char *str;
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;
    int nlok, lnsc, fnst, fnsc;
    int column = ep->Column;
    int srow   = ep->Line;
    int crow   = srow;
    int erow   = srow;
    short dins = 0;         /* relative insert lines/delete lines   */
    char moded = 0;         /* any modifications done at all?       */
    char checked = 0;       /* for cursor positioning.              */

    if (ep->Margin == 0)
        ep->Margin = 75;

    ++Nsu;
    for (;;) {
        str = (char *)ep->List[ep->Line+1];
        fnst = 0;
        fnsc = firstns(Current);
        nlok = (ep->Line + 1 < ep->Lines && fnsc >= (fnst=firstns(str)));
        if (ep->WWCol >= 0)
            fnst = fnsc = ep->WWCol;
        if (nlok && str[0] == 0)
            nlok = 0;
        lnsc = lastns(Current);
        if (lnsc < ep->Margin) {    /* space at end of line for marg-lnsc-2 letter word   */
            if (nlok == 0)        /* but no more data to joinup   */
                break;            /* done */
            if (ep->Margin - lnsc - 2 >= wordlen(str+fnst)) {
                ep->Column = 0;
                Clen = lastns(Current);
                if (Current[Clen])
                    ++Clen;
                moded = 1;
                --dins;
                if (do_join())
                    continue;
                ++dins;
                title("Error, Margin > 124");
                break;
            }
            if (n == 0)        /* if couldn't mod line, and text_write, don't update any more */
                break;
            do_down();
            erow = ep->Line;
            continue;
        }
                                /* no space, need to split      */
                                /* find start of prev word      */
        for (;;) {
            int i = lnsc;
            while (i && Current[i] != ' ')
                --i;
            lnsc = i;
            if (i >= ep->Margin) {
                while (i && Current[i] == ' ')
                    --i;
                if (i < ep->Margin)
                    break;
                lnsc = i;
                continue;
            }
            break;
        }
        if (lnsc) {             /* ok to split at word          */
            ++lnsc;
            ++dins;
            ep->Column = lnsc;
            do_split(); /* Split at point LNSC          */
            do_down();          /* must insert proper amount?   */
            {
                int indent = (nlok == 0) ? fnsc : fnst;
                if (ep->WWCol >= 0)
                    indent = ep->WWCol;
                if (!checked) {
                    checked = 1;
                    if (lnsc <= column) {   /* if split before cursor   */
                        column = column - ep->Column + indent;
                        ++crow;
                    }
                }
                if (Clen + indent < 253) {
                    movmem(Current, Current + indent, strlen(Current)+1);
                    setmem(Current, indent, ' ');
                    Clen += indent;
                }
            }
            erow = ep->Line;
            continue;
        }
        if (n == 0)
            break;
        do_down();
    }
    if (column < 0 || column > 200)
        column = 0;
    if (srow >= ep->Lines) {
        srow = ep->Lines - 1;
        goto ra;
    }
    if (dins || srow < ep->Topline || srow >= ep->Topline + Rows) {
ra:
        text_sync();
        --Nsu;
        ep->Line = crow;
        ep->Column = column;
        text_load();
        if (!text_sync())
            text_redisplay();
    } else {
        text_sync();
        --Nsu;
        ep->Line = crow;
        ep->Column = column;
        text_load();
        if (erow != srow) {
            if (!text_sync()) {
                ++erow;
                if (erow - ep->Topline > Rows)
                    erow = ep->Topline + Rows;
                SetAPen(rp, ep->BGPen);
                RectFill(rp, COL(0), ROW(srow - ep->Topline), Xbase+Xpixs-1, ROW(erow - ep->Topline)-1);
                SetAPen(rp, ep->FGPen);
                text_displayseg(srow - ep->Topline, erow - srow);
            }
        } else {
            text_sync();
            if (moded)
                text_redisplaycurrline();
        }
    }
    if (column > Clen) {
        setmem(Current+Clen, column - Clen, ' ');
        Current[column] = 0;
    }
    ep->Column = column;
}

void
do_tabstop()
{
    Ep->Tabstop = atoi(av[1]);
}

void
do_insertmode()
{
    ED *ep = Ep;

    if (av[1][0]) {
        switch(av[1][1] & 0x1F) {
        case 'n'&0x1F:
            ep->Insertmode = 1;
            break;
        case 'f'&0x1F:
            ep->Insertmode = 0;
            break;
        case 'o'&0x1F:
            ep->Insertmode = 1 - ep->Insertmode;
            break;
        }
        if (ep->Insertmode)
            title("Insert mode on");
        else
            title("Insert mode off");
    }
}

void
do_insline()
{
    ubyte *ptr;
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    ep->Modified = 1;
    text_sync();
    if (makeroom(32) && (ptr = allocb(1))) {
        bmovl(ep->List+ep->Line, ep->List+ep->Line+1,ep->Lines-ep->Line);
        ep->List[ep->Line] = ptr;
        *ptr = 0;
        ++ep->Lines;
        if (BEp == ep) {
            if (ep->Line < BSline)
                ++BSline;
            if (ep->Line <= BEline)
                ++BEline;
        }
    } else {
        nomemory();
    }
    text_load();
    if (Nsu == 0)
        ScrollRaster(rp,0,-Ysize, COL(0), ROW(ep->Line-ep->Topline), COL(Columns)-1, ROW(Rows)-1);
    text_displayseg(ep->Line - ep->Topline, 1);
}

void
do_deline()
{
    int delline;
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    strcpy(Deline, Current);
    if (ep->Lines > 1) {
        ep->Modified = 1;
        text_sync();

        FreeMem(ep->List[ep->Line], strlen(ep->List[ep->Line])+1);
        bmovl(ep->List+ep->Line+1, ep->List+ep->Line,ep->Lines-ep->Line-1);
        if (BEp == ep) {
            if (ep->Line < BSline)
                --BSline;
            if (ep->Line <= BEline)
                --BEline;
        }
        delline = ep->Line;
        if (ep->Line >= --ep->Lines) {
            --ep->Line;
            text_load();
            if (ep->Line < ep->Topline) {
                if (Nsu == 0) {
                    ep->Topline = ep->Line - (Rows>>1);
                    if (ep->Topline < 0)
                        ep->Topline = 0;
                    text_redisplay();
                }
                return;
            }
        }
        text_load();
        if (Nsu == 0)
            ScrollRaster(rp,0,Ysize, COL(0), ROW(delline-ep->Topline), COL(Columns)-1, ROW(Rows)-1);
        text_displayseg(Rows-1, 1);
    } else {
        do_firstcolumn();
        do_remeol();
        ep->Modified = 0;
    }
}

void
do_chfilename()
{
    text_sync();
    strncpy(Ep->Name, av[1], 63);
    MForceTitle = 1;
}

void
do_edit()
{
    FILE *fi;
    BPTR oldlock;
    long lines;
    ubyte buf[256];
    ubyte *ptr;
    char failed = 1;
    ED *ep = Ep;

    text_sync();
    if (*av[0] == 'n') {        /* newfile or insfile   */
        if (ep->Modified && getyn("Delete modified Image?") == 0)
            return;
        ep = uninit_init(ep);
        strncpy(ep->Name, av[1], 63);
    } else {
        ep->Modified = 1;
    }
    lines = ep->Lines;
    oldlock = CurrentDir((BPTR)ep->dirlock);
    if (fi = fopen(av[1], "r")) {
        int len;
        char oktitle = 1;

        title("Loading...");
        while ((len = xefgets(fi, buf, 255)) >= 0) {
            failed = 0;
            if (makeroom(256) && (ptr = allocb(len+1))) {
                ep->List[ep->Lines++] = ptr;
                movmem(buf, ptr, len+1);
            } else {
                set_window_params();
                nomemory();
                oktitle = 0;
                break;
            }
        }
        set_window_params();
        if (oktitle)
            title("OK");
    } else {
        title("File Not Found");
#if AREXX
        CmdErr = 5;
#endif
    }
    if (fi)
        fclose(fi);
    CurrentDir(oldlock);
    if (ep->Lines != 1 && lines == 1 && ep->List[0][0] == 0) {
        ep->Modified = 0;
        ep->Line = 0;
        FreeMem(ep->List[0], strlen(ep->List[0])+1);
        bmovl(ep->List+1, ep->List,--ep->Lines);
    } else {
        if (!failed && lines <= ep->Lines - 1) {
            long bsline, beline;
            short bschar, bechar;
            ED *bep = BEp;

            bsline = BSline;
            beline = BEline;
            bschar = BSchar;
            bechar = BEchar;

            BEp = ep;
            BSline = lines;
            BEline = ep->Lines - 1;
            do_bmove();

            if (bep && bep != ep) {
                BSline = bsline;
                BEline = beline;
                bschar = BSchar;
                bechar = BEchar;
                BEp = bep;
            }
        }
    }
    set_window_params();
    text_load();
    text_redisplay();
}


static char blockmode;
static char *openmode = "w";

void
do_bappend()
{
    char *oldMode = openmode;

    blockmode = 1;
    openmode = "a";
    do_saveas();
    openmode = oldMode;
}

void
do_append()
{
    char *oldMode = openmode;

    openmode = "a";
    do_saveas();
    openmode = oldMode;
}

void
do_bsave()
{
    blockmode = 1;
    do_saveas();
}

void
do_save()
{
    av[1] = Ep->Name;
    do_saveas();
}

void
do_savetabs()
{
    Savetabs = (av[1][0] && av[1][1] == 'n') ? 1 : 0;
}

void
do_saveas()
{
    BPTR oldlock;
    FILE *fi;
    long i;
    short j, k;
    ubyte *ptr, *bp;
    long xs, xe;
    ubyte buf[256];
    char bm;
    ED *ep;

    bm = blockmode;
    if (blockmode && blockok()) {
        xs = BSline;
        xe = BEline + 1;
        ep = BEp;
    } else {
        xs = 0;
        xe = Ep->Lines;
        ep = Ep;
    }
    blockmode = 0;
    text_sync();
    oldlock = CurrentDir((BPTR)Ep->dirlock);
    if (IconSaveOpt && IconBase) {      /* Write out .info file */
        DISKOBJ sdo, *d;
        clrmem(&sdo, sizeof(sdo));
        if ((d = GetDiskObject(av[1])) == NULL) {
            if (Wbs == NULL || getpathto(Wbs->sm_ArgList[0].wa_Lock, Wbs->sm_ArgList[0].wa_Name, buf) == 0)
                strcpy(buf, "DME");
            sdo.do_Magic = WB_DISKMAGIC;
            sdo.do_Version = WB_DISKVERSION;
            makemygadget(&sdo.do_Gadget);
            sdo.do_Type = WBPROJECT;
            sdo.do_DefaultTool = (char *)buf;
            sdo.do_ToolTypes = NULL;
            sdo.do_CurrentX = NO_ICON_POSITION;
            sdo.do_CurrentY = NO_ICON_POSITION;
            sdo.do_DrawerData = NULL;
            sdo.do_ToolWindow = NULL;
            sdo.do_StackSize = 8192;
            PutDiskObject(av[1], &sdo);
        } else {
            FreeDiskObject(d);
        }
    }
    if (fi = fopen(av[1], openmode)) {
        if (openmode[0] == 'a')
            title("Appending...");
        else
            title("Saving...");
        for (i = xs; i < xe; ++i) {
            ptr = ep->List[i];
            if (Savetabs) {
                for (bp = buf, j = 0; *ptr; ++ptr, ++bp, j = (j+1)&7) {
                    *bp = *ptr;
                    if (j == 7 && *bp == ' ' && *(bp-1) == ' ') {
                        k = j;
                        while (k-- >= 0 && *bp == ' ')
                            --bp;
                        *++bp = 9;
                    } else {
#ifdef NOTDEF
                        if (*bp < 32 || *bp == '\"' || *bp == '\'' || *bp == '\`' || *bp == '(')
                            break;
#else
                        if (*bp < 32 || *bp == '\"' || *bp == '\'' || *bp == '\`')
                            break;
#endif
                    }
                }
                strcpy(bp, ptr);
                ptr = buf;
            }
            fputs(ptr, fi);
            fputc('\n', fi);
        }
        if (fclose(fi)) {
err:        Abortcommand = 1;
            title("WRITE FAILED!");
        } else {
            ep->Modified &= bm;
            title("OK");
        }
    } else {
        title("Unable to open write file");
        Abortcommand = 1;
    }
    CurrentDir(oldlock);
}

void
do_block()          /* block, unblock   */
{
    text_sync();

    switch(av[0][0]) {
    case 'b':
        if (BSline < 0) {
bstart:
            BEp = Ep;
            BSline = Ep->Line;
            title("Block Begin");
        } else {
            if (BEline > -1) {
                title("Block Already Marked");
                break;
            }
            if (BEp != Ep)
                goto bstart;
            title("Block End");
            BEline = Ep->Line;
            if (BSline > BEline) {
                BEline = BSline;
                BSline = Ep->Line;
            }
            text_redrawblock(1);
        }
        break;
    case 'u':
        text_redrawblock(0);
        title ("Block Unmarked");
        break;
    }
}


blockok()
{
    if (BEp && BSline >= 0 && BSline <= BEline && BEline < BEp->Lines)
        return(1);
    BEp = NULL;
    BSline = BEline = -1;
    title("Block Not Specified");
    return(0);
}

void
do_bdelete()
{
    long n;
    ED *bep = BEp;
    WIN *savewin = Ep->Win;

    if (blockok()) {
        text_switch(bep->Win);
        n = BEline - BSline + 1;
        if (bep->Line >= BSline && bep->Line <= BEline)
            bep->Line = BSline;
        if (bep->Line > BEline)
            bep->Line -= n;
        freelist(bep->List + BSline, BEline - BSline + 1);
        bmovl(bep->List+BEline+1,bep->List+BSline,(bep->Lines-BEline-1));
        bep->Lines -= n;
        bep->Modified = 1;
        if (bep->Line >= bep->Lines)
            bep->Line = bep->Lines - 1;
        if (bep->Line < 0)
            bep->Line = 0;
        if (bep->Lines == 0) {
            ++bep->Lines;
            bep->List[0] = allocb(1);
            bep->List[0][0] = 0;
        }
        text_load();
        BEp = NULL;
        BSline = BEline = -1;
        if (!text_sync())
            text_redisplay();
        text_switch(savewin);
    }
}

void
do_bcopy()
{
    ubyte **list;
    long lines, i;
    ED *ep = Ep;

    text_sync();
    if (!blockok())
        return;
    if (ep == BEp && ep->Line > BSline && ep->Line <= BEline) {
        title("Cannot Move into self");
        return;
    }
    lines = BEline - BSline + 1;
    if (extend(ep, lines)) {
        if (list = (ubyte **)allocl(lines)) {
            bmovl(BEp->List+BSline,list,lines);
            bmovl(ep->List+ep->Line, ep->List+ep->Line+lines, ep->Lines-ep->Line);
            for (i = 0; i < lines; ++i) {
                ubyte *str = allocb(strlen(list[i])+1);
                if (!str) {
                    nomemory();
                    FreeMem(list, lines * sizeof(char *));
                    freelist(ep->List + Ep->Line, i);
                    bmovl(ep->List+ep->Line+lines, ep->List+ep->Line, ep->Lines-ep->Line);
                    return;
                }
                strcpy(str, list[i]);
                ep->List[ep->Line+i] = str;
            }
            FreeMem(list, lines * sizeof(char *));
        }
    }
    if (ep == BEp && ep->Line <= BSline) {
        BSline += lines;
        BEline += lines;
    }
    ep->Modified = 1;
    ep->Lines += lines;
    text_load();
    if (!text_sync())
        text_redisplay();
}


void
do_bmove()
{
    long lines;
    ubyte **list;
    ED *ep = Ep;

    text_sync();
    if (!blockok())
        return;
    if (BEp == ep && ep->Line >= BSline && ep->Line <= BEline) {
        title("Cannot Move into self");
        return;
    }
    lines = BEline - BSline + 1;
    if (!(list = (ubyte **)allocl(lines))) {
        nomemory();
        return;
    }
    BEp->Modified = ep->Modified = 1;
    bmovl(BEp->List + BSline, list, lines);
    if (ep == BEp) {
        if (ep->Line > BSline) {
            bmovl(ep->List+BEline+1, ep->List+BSline, ep->Line-BEline-1);
            bmovl(list, ep->List + ep->Line - lines, lines);
        } else {
            bmovl(ep->List+ep->Line, ep->List+ep->Line+lines, BSline-ep->Line);
            bmovl(list, ep->List + ep->Line, lines);
        }
    } else {
        WIN *savewin = ep->Win;
        if (extend(ep, lines)) {
            bmovl(BEp->List+BEline+1, BEp->List+BSline, BEp->Lines-BEline-1);
            bmovl(ep->List+ep->Line, ep->List+ep->Line+lines, ep->Lines-ep->Line);
            bmovl(list, ep->List+ep->Line, lines);
            ep->Lines += lines;
            BEp->Lines -= lines;
            if (BEp->Line >= BSline && BEp->Line <= BEline)
                BEp->Line = BSline - 1;
            if (BEp->Line > BEline)
                BEp->Line -= lines;
            if (BEp->Line < 0)
                BEp->Line = 0;
            BSline = BEline = -1;
            if (BEp->Lines == 0) {
                ubyte *ptr = allocb(1);
                BEp->List[0] = ptr;
                *ptr = 0;
                ++BEp->Lines;
            }
            text_load();
            text_switch(BEp->Win);
            BEp = NULL;
            ep = Ep;
            if (!ep->iconmode) {
                if (!text_sync())
                    text_redisplay();
            }
            text_switch(savewin);
            ep = Ep;
        }
    }
    BSline = BEline = -1;
    BEp = NULL;
    FreeMem(list, lines * sizeof(char *));
    ep->Modified = 1;
    text_load();
    if (!text_sync())
        text_redisplay();
}


/*
 * IF condition trueaction, IFELSE condition trueaction falseaction
 *
 *  condition:  !condition NOT the specified condition.
 *              #          toggle number is SET
 *              top        top of file (on first line)
 *              bot        end of file (on last line)
 *              left       start of line (leftmost column)
 *              right      end of line (nothing but spaces under and to the right)
 *              modified   text has been modified
 *              insert     currently in insert mode
 *              y[<=>]#    cursor is (any OR combo of <,>,=) row #  (line numbers start at 1)
 *              x[<=>]#    cursor is (<,>,<=,>=,<>) column #        (columns start at 1)
 *                          <> means 'not equal'
 *
 *              cl         char under cursor is lower case
 *              cu         char under cursor is upper case
 *              ca         char under cursor is alpha
 *              cn         char under cursor is numeric
 *              cb         char within selected block
 *              c[<=>]#    char under cursor is (combo of <,>,and =) #
 */

void
do_if()
{
    char haselse = (av[0][2] == 'e');
    char iswhile = (av[0][0] == 'w');
    char istrue, notop = 0;
    char c, cx, cc;
    ubyte *buf1, *buf2;
    ubyte *ptr;
    ED *ep = Ep;
    int i, cxn, cn;

    buf1 = (ubyte *)malloc(256);
    buf2 = (ubyte *)malloc(256);
    if (buf1 == NULL || buf2 == NULL) {
        if (buf1) free(buf1);
        if (buf2) free(buf2);
        title("No Memory!");
        return;
    }
    breakreset();
    ptr = av[1];
    if (*ptr == '!') {
        notop = 1;
        ++ptr;
    }
    c = ptr[0];
    cn= atoi(ptr);
    cx= ptr[1];
    cxn=atoi(ptr+1);
    strcpy(buf1, av[2]);

loop:
    istrue = 0;
    i = 0;
    switch(c) {
    case 'x':
        i = ep->Column + 1;
    case 'y':
        if (!i)
            i = ep->Line + 1;
conditional:
        {
            int j, n;
            char any = 0;

            for (j = 1; ptr[j] && (ptr[j]<'0'||ptr[j]>'9'); ++j);
            n = atoi(ptr+j);
            for (j = 1; ptr[j]; ++j) {
                switch(ptr[j]) {
                case '<':
                    any = 1;
                    if (i < n)
                        istrue = 1;
                    break;
                case '=':
                    any = 1;
                    if (i == n)
                        istrue = 1;
                    break;
                case '>':
                    any = 1;
                    if (i > n)
                        istrue = 1;
                    break;
                }
            }
            if (!any && i == n)  /* default is equivalence   */
                istrue = 1;
        }
        break;
    case 't':
        istrue = ep->Line == 0;
        break;
    case 'b':
        istrue = ep->Line == ep->Lines-1;
        break;
    case 'l':
        istrue = ep->Column == 0;
        break;
    case 'r':
        istrue = ep->Column == Clen;
        break;
    case 'm':
        text_sync();
        istrue = ep->Modified != 0;
        break;
    case 'e':
        istrue = Comlinemode != 0;
        break;
    case 'i':
        istrue = ep->Insertmode != 0;
        break;
    case 'c':
        cc = Current[ep->Column];
        switch(cx) {
        case 'b':
            istrue = BEp == ep && ep->Line >= BSline && ep->Line <= BEline;
            break;
        case 'l':
            istrue = cc >= 'a' && cc <= 'z';
            break;
        case 'u':
            istrue = cc >= 'A' && cc <= 'Z';
            break;
        case 'a':
            istrue = (cc>='a'&&cc<='z')||(cc>='A'&&cc<='Z')||(cc>='0'&&cc<='9');
            break;
        case 'n':
            istrue = (cc >= '0' && cc <= '9');
            break;
        default:                /* c[<=>]#  */
            i = Current[ep->Column];
            goto conditional;
            break;
        }
        break;
    default:
        if (c >= '0' && c <= '9')
            istrue = do_toggle(cn) != 0;
        else
            title("bad conditional");
        break;
    }
    istrue ^= notop;
    if (istrue) {
        strcpy(buf2, buf1);     /* could be executed multiple times */
        if (do_command(buf2) == 0)
            goto done;
        if (iswhile) {
            if (breakcheck())
                Abortcommand = 1;
            else
                goto loop;
        }
    } else {
        if (haselse) {          /* only executed once */
            strcpy(buf2, av[3]);
            do_command(buf2);
        }
    }
done:
    free(buf1);
    free(buf2);
}


/*
 * TOGGLE #, SETTOGGLE #, RESETTOGGLE #
 */

do_toggle(n)
{
    static char tg[MAXTOGGLE];
    int i;

    if (n >= 0) {
        if (n >= MAXTOGGLE)
            return(0);
        return((int)tg[n]);
    }
    i = atoi(av[1]);
    if (i >= 0 && i < MAXTOGGLE) {
        switch(av[0][0]) {
        case 't':
            tg[i] = !tg[i];
            break;
        case 's':
            tg[i] = 1;
            break;
        case 'r':
            tg[i] = 0;
            break;
        }
    }
}


void
do_tlate()
{
    ubyte *ptr = av[1];
    ED *ep = Ep;
    char c = Current[ep->Column];

    if (c == 0)
        c = ' ';
    if (ptr[0] == '+')
        c += atoi(ptr+1);
    else
    if (ptr[0] == '-')
        c -= atoi(ptr+1);
    else
        c = atoi(ptr);
    if (c) {
        if (Current[ep->Column] == 0) {
            Clen = ep->Column + 1;
            Current[Clen] = 0;
        }
        Current[ep->Column] = c;
        if (Nsu == 0) {
            movetocursor();
            setpen(ep->Line);
            Text(ep->Win->RPort, Current+ep->Column, 1);
        }
    }
}

/*
 *  BSOURCE
 *
 *  note that since the start and end lines are loaded immediately and the
 *  block unblock'd before execution starts, you can theoretically have
 *  another BSOURCE as part of this BSOURCE (but be carefull!).
 */

void
do_bsource()
{
    ubyte buf[256];
    int i, sl, se;

    if (blockok()) {
        sl = BSline;
        se = BEline + 1;
        for (i = sl; BEp && i < se && i < BEp->Lines; ++i) {
            text_sync();        /* make sure we are using latest text */
            strcpy(buf, BEp->List[i]);
            if (do_command(buf) == 0)
                break;
        }
        text_redrawblock(0);
    }
}

/*
 *  SCANF controlstring
 *
 *  The C scanf routine.  Only one variable, a string, is allowed in the
 *  control string.
 */

void
do_scanf()
{
    char buf[256];

    buf[0] = 0;
    sscanf(Current+Ep->Column,av[1],buf,buf,buf,buf,buf,buf,buf);
    if (String)
        free(String);
    String = (char *)malloc(strlen(buf)+1);
    strcpy(String,buf);
    title(String);
}

void
movetocursor()
{
    ED *ep = Ep;
    Move(ep->Win->RPort, XTbase+(ep->Column-ep->Topcolumn)*Xsize, YTbase+(ep->Line-ep->Topline)*Ysize);
}

extend(ep, lines)
ED *ep;
{
    long extra = ep->Maxlines - ep->Lines;
    ubyte **list;

    if (lines > extra) {
        lines += ep->Lines;
        if (list = (ubyte **)allocl(lines)) {
            bmovl(ep->List, list, ep->Lines);
            FreeMem(ep->List, sizeof(char *) * ep->Maxlines);
            ep->Maxlines = lines;
            ep->List = list;
            return(1);
        }
        nomemory();
        return(0);
    }
    return(1);
}

makeroom(n)
{
    ED *ep = Ep;
    if (ep->Lines >= ep->Maxlines)
        return(extend(ep, n));
    return(1);
}

void
freelist(list, n)
char **list;
{
    while (n) {
        FreeMem(list[0], strlen(list[0])+1);
        ++list;
        --n;
    }
}

