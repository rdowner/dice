

*	   ************************************************
*	   ** Math Lib C Interface - ASCII Conversion	 **
*	   **						 **
*	   ************************************************


***********************************************************************
**								      *
**   Copyright 1984, Amiga Computer Inc.   All rights reserved.       *
**   No part of this program may be reproduced, transmitted,	      *
**   transcribed, stored in retrieval system, or translated into      *
**   any language or computer language, in any form or by any	      *
**   means, electronic, mechanical, magnetic, optical, chemical,      *
**   manual or otherwise, without the prior written permission of     *
**   Amiga Computer Incorporated, 3350 Scott Blvd, Bld #7,	      *
**   Santa Clara, CA 95051					      *
**								      *
***********************************************************************

	xdef	_afp,_fpa,_dbf,_arnd		* C entries for conversion math functions

	xref	FFPAFP,FFPFPA,FFPDBF,FFPARND



******* amiga.lib/afp **************************************************
*
*   NAME
*	afp - Convert ASCII string variable into fast floating point
*
*   SYNOPSIS
*	ffp_value = afp(string);
*
*    FUNCTION
*	Accepts the address of the ASCII string in C format that is
*	converted into an FFP floating point number.
*
*	The string is expected in this Format:
*	{S}{digits}{'.'}{digits}{'E'}{S}{digits}
*	<*******MANTISSA*******><***EXPONENT***>
*
*
*	Syntax rules:
*	Both signs are optional and are '+' or '-'. The mantissa must be
*	present. The exponent need not be present. The mantissa may lead
*	with a decimal point. The mantissa need not have a decimal point.
*	Examples: All of these values represent the number fourty-two.
*		     42 	   .042e3
*		     42.	  +.042e+03
*		    +42.	  0.000042e6
*		0000042.00	 420000e-4
*			     420000.00e-0004
*
*	Floating point range:
*	Fast floating point supports the value zero and non-zero values
*	within the following bounds -
*			18			       20
*	 9.22337177 x 10   > +number >	5.42101070 x 10
*			18			       -20
*	-9.22337177 x 10   > -number > -2.71050535 x 10
*
*	Precision:
*	This conversion results in a 24 bit precision with guaranteed
*	error less than or equal to one-half least significant bit.
*
*	INPUTS
*	string - Pointer to the ASCII string to be converted.
*
*
*	OUTPUTS
*	string - points to the character which terminated the scan
*	equ - fast floating point equivalent
*****************************************************************************

_afp:
	movem.l d3-d7,-(sp)         * Save registers used
	move.l	24(sp),a0           * Put ASCII string address in A0
	jsr	FFPAFP		    * Execute Motorola function
	bra.s	fpal3		    * Set up functional result and exit


******* amiga.lib/fpa **************************************************
*
*   NAME
*	fpa - convert fast floating point into ASCII string equivalent
*
*   SYNOPSIS
*	exp = fpa(fnum, &string[0]);
*
*   FUNCTION
*	Accepts an FFP number and the address of the ASCII string where it's
*	converted output is to be stored.  The number is converted to a NULL
*	terminated ASCII string in and stored at the address provided.
*	Additionally, the base ten (10) exponent in binary form is returned.
*
*   INPUTS
*	fnum	   - Motorola Fast Floating Point number
*	&string[0] - address for output of converted ASCII character string
*		     (16 bytes)
*
*   RESULT
*	&string[0] - converted ASCII character string
*	exp	   - integer exponent value in binary form
*
*   BUGS
*	None
*****************************************************************************
* Docs recovered 9/1/88 from Rob Peck's sun - Bryce


_fpa:
	movem.l d3-d7,-(sp)         * Save registers used
	move.l	24(sp),d7           * Put FFP number to convert in D7
	jsr	FFPFPA		    * Execute Motorola function

	move.l	42(sp),a0           * Put ASCII string address in A0
fpal1:
	moveq	#6,d1		    * Set up for 14 byte data transfer
fpal2:
	move	(sp)+,(a0)+         * Move one (1) word at a time
	dbf	d1,fpal2	    * Loop until all 14 bytes moved
fpal3:
	move.l	d7,d0		    * Get exponent as functional value
	movem.l (sp)+,d3-d7         * Restore registers used
	rts


******* amiga.lib/dbf **************************************************
*
*   NAME
*	dbf - convert FFP dual-binary number to FFP format
*
*   SYNOPSIS
*	fnum = dbf(exp, mant);
*
*   FUNCTION
*	Accepts a dual-binary format (described below) floating point
*	number and converts it to an FFP format floating point number.
*	The dual-binary format is defined as:
*
*		exp bit  16	= sign (0=>positive, 1=>negative)
*		exp bits 15-0	= binary integer representing the base
*					  ten (10) exponent
*		man		= binary integer mantissa
*
*   INPUTS
*	exp - binary integer representing sign and exponent
*	mant - binary integer representing the mantissa
*
*   RESULT
*	fnum - converted FFP floating point format number
*
*   BUGS
*	None
*****************************************************************************



_dbf:
	movem.l d3-d7,-(sp)         * Save registers used
	moveq	#0,d6		    * Insure D6 is cleared
	move.l	28(sp),d7           * D7 = mantissa
	tst.l	d7
	bpl	dbfl1
	bset	#16,d6		    * Set negative mantissa indicator
	neg.l	d7		    * Convert to positive mantissa value
dbfl1:
	move	26(sp),d6           * Set up actual exponent value
	jsr	FFPDBF		    * Execute Motorola function
	bra.s	fpal3		    * Set up functional result and exit



******* amiga.lib/arnd *************************************************
*
*  NAME
*	arnd - ASCII round of the provided floating point string
*
*   SYNOPSIS
*	arnd(place, exp, &string[0]);
*
*   FUNCTION
*	Accepts an ASCII string representing an FFP floating point
*	number, the binary representation of the exponent of said
*	floating point number and the number of places to round to.
*	A rounding process is initiated, either to the left or right
*	of the decimal place and the result placed back at the
*	input address defined by &string[0].
*
*   INPUTS
*	place - integer representing number of decimal places to round to
*	exp - integer representing exponent value of the ASCII string
*	&string[0] - address where rounded ASCII string is to be placed
*		     (16 bytes)
*
*   RESULT
*	&string[0] - rounded ASCII string
*
*   BUGS
*	None
*****************************************************************************


_arnd:
	movem.l d3-d7,-(sp)         * Save registers used

	movem.l 24(sp),d6-d7        * Set up rounding magnitude and exponent
	move.l	32(sp),a0           * Get address of ASCII string

	lea	-14(sp),a1          * Point A1 to stack area
	move.l	a1,sp		    * Point SP there too
	moveq	#6,d1		    * Set up for 7 word mem-mem move
arndl1:
	move	(a0)+,(a1)+         * Move one (1) word at a time
	dbf	d1,arndl1	    * Loop until all 7 words moved

	jsr	FFPARND 	    * Execute Motorola function

	move.l	46(sp),a0           * Get string address for result
	bra.s	fpal1		    * Move the string and get out



****i** amiga.lib/fpbcd ************************************************
*
*   NAME
*	fpbcd - convert FFP floating point number to BCD format
*
*   SYNOPSIS
*	fpbcd(fnum, &string[0]);
*
*   FUNCTION
*	Accepts a floating point number and the address where the
*	converted BCD data is to be stored.  The FFP number is
*	converted and stored at the specified address in an ASCII
*	form in accordance with the following format:
*
*		MMMM S E S B
*
*	Where:	M = Four bytes of BCD, each with two (2) digits of
*		    the mantissa (8 digits)
*		S = Sign of mantissa (0x00 = positive, 0xFF = negative)
*		E = BCD byte for two (2) digit exponent
*		S = Sign of exponent (0x00 = positive, 0xFF = negative)
*		B = One (1) byte binary two's compliment representation
*			of the exponent
*
*   INPUTS
*	fnum - floating point number
*	&string[0] - address where converted BCD data is to be placed
*
*   RESULT
*	&string[0] - converted BCD data
*****************************************************************************


;_fpbcd:
;	movem.l d3-d7,-(sp)         * Save registers used
;	move.l	24(sp),d7           * Put FFP value in D7
;	jsr	FFPFPBCD	    * Execute Motorola function
;	move.l	40(sp),a0           * Put ASCII string address in A0
;	moveq	#3,d1		    * Set up for 8 byte data transfer
;	bra.s	fpal2		    * Move the string and get out


	end
