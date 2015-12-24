
/*
 *  TRANSFER.C
 *
 *  David Berezowski - March/88.
 *  Modified for DICE - May/91	Matthew Dillon
 *
 *  Copyright (c) 1988	Commodore-Amiga, Inc.
 *  (c)Copyright 1991 Matthew Dillon
 */

#include "defs.h"

Prototype void Transfer(PrtInfo *, UWORD, UBYTE *, UWORD *);

void
Transfer(PInfo, y, ptr, colors)
PrtInfo *PInfo;
UWORD y;	    /* row # */
UBYTE *ptr;	    /* ptr to buffer */
UWORD *colors;	    /* indexes to color buffers */
{
    static UWORD bit_table[8] = {128, 64, 32, 16, 8, 4, 2, 1};
    union colorEntry *ColorInt;
    UBYTE *yptr, *mptr, *cptr, Black, Yellow, Magenta, Cyan;
    UBYTE *dmatrix, dvalue, threshold;
    UWORD *sxptr, bit, x3, ymod;
    register UWORD x, sx, width;
    register UBYTE *bptr;

    /* pre-compute */
    /* printer non-specific, MUST DO FOR EVERY PRINTER */

    x = PInfo->pi_xpos;
    ColorInt = PInfo->pi_ColorInt;
    sxptr = PInfo->pi_ScaleX;
    width = PInfo->pi_width;

    /* printer specific */

    x3 = x * 3;
    ymod = y % PED->ped_NumRows;
    bit = bit_table[ymod & 7];
    ptr += ymod >> 3;
    bptr = ptr + colors[0];
    yptr = ptr + colors[1];
    mptr = ptr + colors[2];
    cptr = ptr + colors[3];

    /* pre-compute threshold; are we thresholding? */

    if (threshold = PInfo->pi_threshold) { /* thresholding */
	dvalue = threshold ^ 15;
	bptr += x3;

	/* for all source pixels */

	do {
	    /* pre-compute intensity values for each component */
	    Black = ColorInt->colorByte[PCMBLACK];
	    ColorInt++;

	    sx = *sxptr++;

	    /* use this pixel 'sx' times */
	    do {
		if (Black > dvalue)
		    *bptr |= bit;
		bptr += 3;
	    } while (--sx);
	} while (--width);
    } else {
	/* not thresholding, pre-compute ptr to dither matrix */

	dmatrix = PInfo->pi_dmatrix + ((y & 3) << 2);
	if (PD->pd_Preferences.PrintShade == SHADE_GREYSCALE) {
	    bptr += x3;

	    /* for all source pixels */
	    do {
		/* compute intensity val for each component */
		Black = ColorInt->colorByte[PCMBLACK];
		ColorInt++;

		sx = *sxptr++;

		/* use this pixel 'sx' times */
		do {
		    if (Black > dmatrix[x & 3])
			*bptr |= bit;
		    x++; /* done 1 more printer pixel */
		    bptr += 3;
		} while (--sx);
	    } while (--width);
	} else {			    /* color */
	    /* for all source pixels */
	    do {
		/* compute intensity val for each component */
		Black = ColorInt->colorByte[PCMBLACK];
		Yellow = ColorInt->colorByte[PCMYELLOW];
		Magenta = ColorInt->colorByte[PCMMAGENTA];
		Cyan = ColorInt->colorByte[PCMCYAN];
		ColorInt++;

		sx = *sxptr++;

		/* use this pixel 'sx' times */
		do {
		    dvalue = dmatrix[x & 3];
		    if (Black > dvalue) {
			*(bptr + x3) |= bit;
		    } else  {			/* black not rendered */
			if (Yellow > dvalue) {
			    *(yptr + x3) |= bit;
			}
			if (Magenta > dvalue) {
			    *(mptr + x3) |= bit;
			}
			if (Cyan > dvalue) {
			    *(cptr + x3) |= bit;
			}
		    }
		    ++x;	/* done 1 more printer pixel */
		    x3 += 3;
		} while (--sx);
	    } while (--width);
	}
    }
}


