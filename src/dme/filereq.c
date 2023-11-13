/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASL interface
 *  ARP interface
 */

#include "defs.h"
#include <stdarg.h>
#include <libraries/asl.h>
#include <proto/asl.h>

Prototype void do_arpinsfile (void);
Prototype void do_arpload (void);
Prototype void do_arpsave (void);
Prototype void fixfile (char *, char *);
Prototype void splitpath (char *, char *, char *);

Prototype void do_aslload(void);
Prototype void do_aslsave(void);
Prototype void do_aslinsfile(void);
Prototype void do_aslfont(void);

/*
 *  The ASL interface uses the asl.library if it exists, else tries to
 *  use the ARP interfafce.
 */

void
do_aslload()
{
    char file[64];
    char dir[64];

    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);

    if (FReq) {                /* If we have asl.library & requester */
        if (AslRequestTags((APTR)FReq,
                            ASL_Hail,       "DME Load New File",
                            ASL_OKText,     "Load",         /* ASL_Window, window,    ASL_FuncFlags, ((win)&&(win->UserPort)) ? 0 : FILF_NEWIDCMP,*/
                            ASL_FuncFlags,  FILF_NEWIDCMP,  /* no window tag */
                            ASL_Dir,        dir,
                            ASL_File,       file,
                            TAG_DONE )) {

            BPTR newlock;

            if (newlock = Lock( FReq->rf_Dir, SHARED_LOCK)) {
                UnLock(CurrentDir(oldlock));
                Ep->dirlock = (long)newlock;
                /* fixfile(FReq->rf_File,FReq->rf_Dir); */
                av[0] = (ubyte *)"n";
                av[1] = (ubyte *) FReq->rf_File;
                do_edit();
                return;
            }
        }
        CurrentDir(oldlock);
    } else {              /* no asl.library? try ARP */
        do_arpload();
    }
}


void
do_aslsave()
{
    char file[64];
    char dir[64];
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);
    if (FReq) {
        if (AslRequestTags( (APTR)FReq,
                            ASL_Hail,       "DME Save File",
                            ASL_OKText,     "Save",
                            ASL_FuncFlags,  FILF_NEWIDCMP | FILF_SAVE,  /* no window tag! */
                            ASL_Dir,        dir,
                            ASL_File,       file,
                            TAG_DONE )) {

            CurrentDir(oldlock);
            fixfile( FReq->rf_File, FReq->rf_Dir);
            av[1] = (ubyte *)FReq->rf_File;
            do_saveas();
        } else {
            CurrentDir(oldlock);
        }
    } else {
        do_arpsave();
    }
}

void
do_aslinsfile()
{
    char file[64];
    char dir[64];
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);
    if (FReq) {
        if (AslRequestTags( (APTR)FReq,
                            ASL_Hail,       "DME Insert File",
                            ASL_OKText,     "Load",
                            ASL_FuncFlags,  FILF_NEWIDCMP,  /* no window tag */
                            ASL_Dir,        dir,
                            ASL_File,       file,
                            TAG_DONE )) {

            CurrentDir(oldlock);
            fixfile( FReq->rf_File, FReq->rf_Dir);
            av[0] = (ubyte *)"i";
            av[1] = (ubyte *)FReq->rf_File;
            do_edit();
            return;
        }
        CurrentDir(oldlock);
    } else {
        do_arpinsfile();
    }
}

/*
 *  DICE 2.06.17 won't allow FontTags[] inside do_aslfont(),
 *  I wanted to put it after *fontreq, below....
 */

struct TagItem FontTags[] = {   ASL_Hail,       (ULONG)"DME fixed width font",
                                ASL_FuncFlags,  FONF_FIXEDWIDTH,
                                ASL_MinHeight,  6,
                                ASL_MaxHeight,  24,     /* >reasonable, eh? */
                                TAG_DONE };

void
do_aslfont()
{
    if (FReq) {
        struct FontRequester *fontreq;

        if (fontreq = (struct FontRequester *)
                      AllocAslRequest(ASL_FontRequest,FontTags) ) {

            if (AslRequest(fontreq, NULL)) {

                /*  Since we have GetFont() already, just pass
                 *   the name and size from fontreq's TextAttr to it.
                 *
                 *  What follows mimics SETFONT (in cmnd3.c)
                 */

                FONT *font = (FONT *)GetFont( fontreq->fo_Attr.ta_Name,
                                             (short) fontreq->fo_Attr.ta_YSize );
                ED *ep = Ep;
                if (font) {
                    text_sync();
                    if (ep->Font)
                        CloseFont(ep->Font);
                    ep->Font = font;
                    SetFont(ep->Win->RPort, font);
                    SetRast(ep->Win->RPort, 0);
                    RefreshWindowFrame(ep->Win);
                    set_window_params();
                    text_redisplay();
                } else {
                    title("Unable to find font");
                }
            }
            FreeAslRequest(fontreq);
        }
    } else {
        title("Need ASL.LIBRARY for Font Requester!");
    }
}


void
do_arpinsfile()
{
    char file[64];
    char dir[64];
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);
    if (arpreq("Insert File", file, dir, NULL)) {
        CurrentDir(oldlock);
        fixfile(file, dir);
        av[0] = (ubyte *)"i";
        av[1] = (ubyte *)file;
        do_edit();
        return;
    }
    CurrentDir(oldlock);
}

void
do_arpload()
{
    char file[64];
    char dir[64];
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);
    if (arpreq("New File", file, dir, NULL)) {
        BPTR newlock;
        if (newlock = Lock(dir, SHARED_LOCK)) {
            UnLock(CurrentDir(oldlock));
            Ep->dirlock = (long)newlock;
            /*
            fixfile(file,dir);
            */
            av[0] = (ubyte *)"n";
            av[1] = (ubyte *)file;
            do_edit();
            return;
        }
    }
    CurrentDir(oldlock);
}

void
do_arpsave()
{
    char file[64];
    char dir[64];
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    splitpath(Ep->Name, file, dir);
    if (arpreq("Save File", file, dir, NULL)) {
        CurrentDir(oldlock);
        fixfile(file,dir);
        av[1] = (ubyte *)file;
        do_saveas();
    } else {
        CurrentDir(oldlock);
    }
}

void
fixfile(file,dir)
char *file,*dir;
{
    char *ptr;
    short len = strlen(dir);
    char hasdev = 0;

    /*
     *  do we need to add a slash to the directory spec?
     */

    if (len && dir[len-1] != '/' && dir[len-1] != ':') {
        dir[len++] = '/';
        dir[len] = 0;
    }

    /*
     *  Is file spec really a full path spec?
     */

    for (ptr = file; *ptr; ++ptr) {
        if (ptr[0] == ':')
            hasdev = 1;
    }
    if (!hasdev) {
        movmem(file,file+len,strlen(file)+1);
        movmem(dir,file,len);
    }
}

/*
 *  Search backwards for first ':' or '/' and split path there.
 *  This subroutine may appear to be coded incorrectly to a novice
 *  programmer.  It isn't [now].
 */

void
splitpath(name, file, dir)
char *name;
char *file, *dir;
{
    short i;

    for (i = strlen(name); i >= 0; --i) {       /* was (incorrectly) "i > 0" */
        if (name[i] == ':' || name[i] == '/')
            break;
    }
    ++i;
    strcpy(file, name + i);
    movmem(name, dir, i);
    dir[i] = 0;
}

