
/*
 *  X.C
 *
 *  If you do not have the 2.0 includes you will need to remove the
 *  #include <clib/#?> stuff and change IDCMP_* and WFLG_* to the
 *  1.3 names (refer intuition/intuition.h)
 */

#include <stdio.h>
#include <stdlib.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

typedef struct Screen Screen;
typedef struct Window Window;
typedef struct IntuiMessage IntuiMessage;

Screen *Scr;
Window *Win;

struct NewScreen Ns = {
    0, 0, 640, -1, 4,
    -1, -1,
    LACE|HIRES,
    CUSTOMSCREEN,
    NULL,
    "Pixel Twiddle"
};

struct NewWindow Nw = {
    0, 0, 0, 0,
    -1, -1,

    IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_CLOSEWINDOW,

    WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_SIZEGADGET|
    WFLG_SIMPLE_REFRESH|WFLG_REPORTMOUSE|
    WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_NOCAREREFRESH,

    NULL, NULL, "Pixel Twiddle",
    NULL,
    NULL,
    16, 32, -1, -1,
    CUSTOMSCREEN
};

void myexit(void);

main(ac, av)
int ac;
char *av[];
{
    short notDone = 1;
    short down = 0;
    char pen = 1;

    atexit(myexit);

    if (Scr = OpenScreen(&Ns)) {
	Nw.TopEdge= 2;	/*  leave 2 pixel screen title so can drag screen */
	Nw.Height = Scr->Height - Nw.TopEdge;
	Nw.Width  = Scr->Width;

	Nw.Screen = Scr;

	if (Win = OpenWindow(&Nw)) {
	    while (notDone) {
		IntuiMessage *imsg;

		WaitPort(Win->UserPort);
		while (imsg = (IntuiMessage *)GetMsg(Win->UserPort)) {
		    switch(imsg->Class) {
		    case IDCMP_CLOSEWINDOW:
			notDone = 0;
			break;
		    case IDCMP_MOUSEBUTTONS:
			switch(imsg->Code) {
			case SELECTDOWN:
			    Move(Win->RPort, imsg->MouseX, imsg->MouseY);
			    down = 1;
			    break;
			case SELECTUP:
			    Draw(Win->RPort, imsg->MouseX, imsg->MouseY);
			    down = 0;
			    break;
			}
			break;
		    case IDCMP_MOUSEMOVE:
			if (down)
			    Draw(Win->RPort, imsg->MouseX, imsg->MouseY);
			break;
		    }
		    ReplyMsg(&imsg->ExecMessage);
		    SetAPen(Win->RPort, pen++);
		}
	    }
	}
    }
    return(0);
}

void
myexit(void)
{
    if (Win) {
	CloseWindow(Win);
	Win = NULL;
    }
    if (Scr) {
	CloseScreen(Scr);
	Scr = NULL;
    }
}

