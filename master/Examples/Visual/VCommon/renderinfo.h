/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
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
