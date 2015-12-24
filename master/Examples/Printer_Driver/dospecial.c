
/*
 *  DOSPECIAL.C
 *
 *  David Berezowski - March/88.
 *  Modified for DICE - May/91	Matthew Dillon
 *
 *  Copyright (c) 1988  Commodore-Amiga, Inc.
 *  (c)Copyright 1991 Matthew Dillon
 */

#include "defs.h"

#define ESC 27

static short Lpi  = 6;
static short Is15 = 0;
static short LeftMargin = 1;
static short RightMargin= 80;
static char *Buffer;

Prototype __geta4 DoSpecial(UWORD *, char *, BYTE *, BYTE *, BYTE *, UBYTE *);
Prototype char *SetQuality(char *, short);
Prototype char *SetPitch(char *, short);
Prototype char *SetLPI(char *, short, char *);
Prototype char *SetMargins(char *, short, short);

__geta4 long
DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
    char *ptr = outputBuffer;
    short n;

    /*
     *	*ptr++ = '<';
     *	*ptr++ = *command / 10 + '0';
     *	*ptr++ = *command % 10 + '0';
     *	*ptr++ = '>';
     */

    switch(*command) {
    case aRIS:		    /*	Reset	    */
	*ptr++ = ESC;
	*ptr++ = '@';

	/* FALL THROUGH */
    case aRIN:		    /*	Initialize  */
	*ptr++ = 24;	    /*	Cancel line	    */
	*ptr++ = ESC;	    /*	Cancel MSB control  */
	*ptr++ = '#';
	*ptr++ = ESC;	    /*	master select everything off	*/
	*ptr++ = '!';
	*ptr++ = 0;

	Lpi = (PD->pd_Preferences.PrintSpacing & EIGHT_LPI) ? 8 : 6;

	ptr = SetQuality(ptr, PD->pd_Preferences.PrintQuality == LETTER);
	ptr = SetPitch(ptr, PD->pd_Preferences.PrintPitch);
	ptr = SetLPI(ptr, Lpi, currentVMI);
	ptr = SetMargins(ptr, PD->pd_Preferences.PrintLeftMargin,
			      PD->pd_Preferences.PrintRightMargin
			);
	break;
    case aSHORP3:   /* 18 condensed fine off		    */
    case aSHORP1:   /* 16 aSHORP1 elite off		    */
    case aSHORP0:   /* 14 aSHORP0 normal pitch		    */
	ptr = SetPitch(ptr, PICA);
	ptr = SetLPI(ptr, Lpi, currentVMI);
	break;
    case aSHORP2:   /* 15 aSHORP2 elite on	(12)        */
	ptr = SetPitch(ptr, ELITE);
	ptr = SetLPI(ptr, Lpi, currentVMI);
	break;
    case aSHORP4:   /* 17 aSHORP4 condensed fine on  (15)   */
	ptr = SetPitch(ptr, FINE);
	ptr = SetLPI(ptr, Lpi, currentVMI);
	break;
    case aSUS3:     /*	aSUS3	*/
    case aSUS1:     /*	aSUS1	*/
    case aSUS0:     /*	aSUS0	*/
	*ptr++ = ESC;
	*ptr++ = 'T';
	break;
    case aSUS2:     /*	aSUS2	superscript		*/
	*ptr++ = ESC;
	*ptr++ = 'S';
	*ptr++ = '0';
	break;
    case aSUS4:     /*	aSUS4	subscript		*/
	*ptr++ = ESC;
	*ptr++ = 'S';
	*ptr++ = '1';
	break;
    case aSLRM:     /*	Set left & right margins	*/
	ptr = SetMargins(ptr, Parms[0], Parms[1]);
	break;
    case aCAM:		    /*	Cancel Margins	    */
	n = PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
	switch(PD->pd_Preferences.PrintPitch) {
	case PICA:
	    n = 10 * n / 10;
	    break;
	case ELITE:
	    n = 12 * n / 10;
	    break;
	default:
	    n = 15 * n / 10;
	    break;
	}
	ptr = SetMargins(ptr, 1, n);
	break;
    case aVERP0:    /*	8 LPI	*/
	ptr = SetLPI(ptr, 8, currentVMI);
	break;
    case aVERP1:    /*	6 LPI	*/
	ptr = SetLPI(ptr, 6, currentVMI);
	break;
    case aSLPP:     /*	set form length */
	*ptr++ = ESC;
	*ptr++ = 'C';
	*ptr++ = Parms[0];
	break;
    case aPERF:     /*	perf skip n	*/
	*ptr++ = ESC;
	*ptr++ = 'N';
	*ptr++ = Parms[0];
	break;
    }
    return(ptr - outputBuffer);
}

char *
SetQuality(ptr, letter)
char *ptr;
short letter;
{
    *ptr++ = ESC;
    *ptr++ = 'x';
    *ptr++ = (letter) ? 1 : 0;
    return(ptr);
}

char *
SetPitch(ptr, printpitch)
char *ptr;
short printpitch;
{
    *ptr++ = ESC;

    switch(printpitch) {
    case FINE:		/*  15 pitch	*/
	Is15 = 2;
	*ptr++ = 'g';
	break;
    case ELITE: 	/*  12 pitch	*/
	Is15 = 0;
	*ptr++ = 'M';
	break;
    default:		/*  10 pitch PICA   */
	Is15 = 0;
	*ptr++ = 'P';
	break;
    }
    return(ptr);
}

char *
SetLPI(ptr, lpi, cvmi)
char *ptr;
short lpi;
char *cvmi;
{
    Lpi = lpi;
    lpi += Is15;	/*  If 15 pitch, lpi += 2		*/
    *cvmi = 180 / lpi;	/*  Current Vertical Margin Increment	*/

    if (lpi == 8) {     /*  8 doesn't divide evenly into 180    */
	*ptr++ = ESC;
	*ptr++ = '0';
    } else {
	*ptr++ = ESC;
	*ptr++ = '3';
	*ptr++ = 180 / (lpi ? lpi : 1);     /* ensure no divide by 0   */
    }
    return(ptr);
}

char *
SetMargins(ptr, left, right)
char *ptr;
short left;
short right;
{
    LeftMargin = left;
    RightMargin= right;

    *ptr++ = ESC;
    *ptr++ = 'l';
    *ptr++ = left;
    *ptr++ = ESC;
    *ptr++ = 'Q';
    *ptr++ = right;
    return(ptr);
}

