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
 */

#include   <exec/types.h>
#include   <exec/memory.h>
#include   <graphics/view.h>
#include   <graphics/text.h>
#include   <intuition/intuition.h>
#include   <intuition/screens.h>

#include   <proto/exec.h>
#include   <proto/intuition.h>
#include   <proto/graphics.h>

#include   "RenderInfo.h"
#include   "DefaultFonts.h"

/*
 * These define the amount of Red, Green, and Blue scaling used
 * to help take into account the different visual impact of those
 * colours on the screen.
 */
#define BLUE_SCALE      2
#define GREEN_SCALE     6
#define RED_SCALE       3

/*
 * This returns the colour difference hamming value...
 */
SHORT ColourDifference(UWORD rgb0, UWORD rgb1)
{
   register   SHORT   level;
   register   SHORT   tmp;

   tmp=(rgb0 & 15) - (rgb1 & 15);
   level=tmp*tmp*BLUE_SCALE;
   tmp=((rgb0>>4) & 15) - ((rgb1>>4) & 15);
   level+=tmp*tmp*GREEN_SCALE;
   tmp=((rgb0>>8) & 15) - ((rgb1>>8) & 15);
   level+=tmp*tmp*RED_SCALE;
   return(level);
}
#define MAX_COLOURS     16

/*
 * For new programs, this also opens fonts...
 */
void FillIn_RenderInfo(struct RenderInfo *ri, struct Screen *TheScreen)
{
register   SHORT      numcolours;
register   SHORT      loop;
register   SHORT      loop1;
register   SHORT      backpen;
register   SHORT      tmp;
           SHORT      colours[16];
           SHORT      colourlevels[16];
           SHORT      pens[16];
   struct  Screen     screen;

   if (!TheScreen) GetScreenData((APTR)(TheScreen=&screen),
                                 sizeof(struct Screen),WBENCHSCREEN,NULL);

   ri->WindowTop    = TheScreen->WBorTop;
   ri->WindowLeft   = TheScreen->WBorLeft;
   ri->WindowRight  = TheScreen->WBorRight;
   ri->WindowBottom = TheScreen->WBorBottom;
   ri->WindowTitle  = TheScreen->BarHeight-TheScreen->BarVBorder+TheScreen->WBorTop;

   ri->ScreenWidth  = TheScreen->Width;
   ri->ScreenHeight = TheScreen->Height;

   ri->FontSize     = TheScreen->Font->ta_YSize;

   if (!(ri->TheFont=OpenFont(TheScreen->Font)))
      ri->TheFont = OpenFont(&TOPAZ80);

   if (ri->TheFont)
   {
      ri->TextAttr.ta_Name=ri->TheFont->tf_Message.mn_Node.ln_Name;
      ri->TextAttr.ta_YSize=ri->TheFont->tf_YSize;
      ri->TextAttr.ta_Style=ri->TheFont->tf_Style;
      ri->TextAttr.ta_Flags=ri->TheFont->tf_Flags;
   }
   else ri->TextAttr=TOPAZ80;

   numcolours=1 << (TheScreen->RastPort.BitMap->Depth);
   if (numcolours>16) numcolours=16;

   if (numcolours<3)
   {       /* Some silly person is running with 2 colours... */
      ri->BackPen=0;
      ri->Highlight=1;
      ri->Shadow=1;
      ri->TextPen=1;
   }
   else
   {
      for (loop=0;loop<numcolours;loop++)
      {
         colours[loop]=GetRGB4(TheScreen->ViewPort.ColorMap,(LONG)loop);
         colourlevels[loop]=ColorLevel(colours[loop]);
         pens[loop]=loop;
      }

      /* Sort darkest to brightest... */
      for (loop=0;loop<(numcolours-1);loop++)
         for (loop1=loop+1;loop1<numcolours;loop1++)
         {
            if (colourlevels[loop]>colourlevels[loop1])
            {
               tmp=colourlevels[loop];
               colourlevels[loop]=colourlevels[loop1];
               colourlevels[loop1]=tmp;

               tmp=colours[loop];
               colours[loop]=colours[loop1];
               colours[loop1]=tmp;

               tmp=pens[loop];
               pens[loop]=pens[loop1];
               pens[loop1]=tmp;
            }
          }

      /* Now, pick the pens... HightLight... */
      loop=numcolours-1;
      while (!(ri->Highlight=pens[loop--]));

      /* and Shadow... */
      loop=0;
      while (!(ri->Shadow=pens[loop++]));

      /* The BackGround pen... */
      if (!pens[loop]) loop++;
      ri->BackPen=pens[backpen=loop];

      loop1=0;
      for (loop=0;loop<numcolours;loop++)
      {
         tmp=ColourDifference(colours[loop],colours[backpen]);
         if (tmp > loop1)
         {
            loop1 = tmp;
            ri->TextPen = pens[loop];
         }
      }
   }
}

/*
 * Close the font and free the memory...
 */
VOID CleanUp_RenderInfo(struct RenderInfo *ri)
{
   if (ri && ri->TheFont) CloseFont(ri->TheFont);
}
