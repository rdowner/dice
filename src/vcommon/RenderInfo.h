/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 * $Id: RenderInfo.h,v 30.8 1994/08/18 05:53:14 dice Exp dice $
 *
 * $Source: /home/dice/com/src/vcommon/RCS/RenderInfo.h,v $
 *
 * $Date: 1994/08/18 05:53:14 $
 *
 * $Revision: 30.8 $
 *
 * $Log: RenderInfo.h,v $
 * Revision 30.8  1994/08/18  05:53:14  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:09:38  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:09:38  dice
 * .
 *
 * Revision 1.1  1992/07/18  23:10:13  jtoebes
 * Initial revision
 *
 * Revision 1.2  92/05/04  22:30:48  jtoebes
 * First mostly working version.  Still needs list gadgets and menu support
 * 
 * Revision 1.1  92/04/21  06:22:12  J_Toebes
 * Initial revision
 *
 * Revision 1.2  90/05/20  12:18:36  mks
 * New functions to allocate and free RenderInfo structure.
 * These should be used insted of the old FillIn_RenderInfo...
 * Now has a TextAttr in the RenderInfo
 * Now has the screen Width/Height in RenderInfo
 * Now can be passed a screen pointer
 *
 * Revision 1.1  90/05/09  21:45:05  mks
 * Source now fully under RCS...
 *
 */

/*
 * This file contains the definition of the rendering information
 * for elements on the screen.  This information is used to generate
 * the correct pen colours for items on the screen...
 *
 * Note, that to call this function you MUST have Intuition and Graphics
 * libraries open...
 */

#ifndef MKS_RENDERINFO_H
#define MKS_RENDERINFO_H

#include        <exec/types.h>
#include        <graphics/text.h>
#include        <intuition/screens.h>

struct RenderInfo
{
        UBYTE           Highlight;      /* Standard Highlight   */
        UBYTE           Shadow;         /* Standard Shadow      */
        UBYTE           TextPen;        /* Requester Text Pen   */
        UBYTE           BackPen;        /* Requester Back Fill  */

        UBYTE           WindowTop;      /* Top border of window */
        UBYTE           WindowLeft;     /* Left border          */
        UBYTE           WindowRight;    /* Right border         */
        UBYTE           WindowBottom;   /* Bottom border        */

        UBYTE           WindowTitle;    /* Window title size    */      /* includes border */
        UBYTE           junk_pad;

        SHORT           ScreenWidth;    /* Width of the screen */
        SHORT           ScreenHeight;   /* Height of the screen */

        USHORT          FontSize;       /* Font size for string gadgets */

struct  TextFont        *TheFont;       /* Font TextFont */
struct  TextAttr        TextAttr;       /* Font TextAttr */
};

/*
 * Calculate a rough brightness hamming value...
 */
#define ColorLevel(rgb) ColourDifference(rgb, 0)

VOID CleanUp_RenderInfo(struct RenderInfo *);

VOID FillIn_RenderInfo(struct RenderInfo *, struct Screen *);

#endif  /* MKS_RENDERINFO_H */
