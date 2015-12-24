

*          ************************************************ 
*          **  Math Library C Compiler Interface         ** 
*          **                                            ** 
*          ************************************************ 
***********************************************************************
**                                                                    *
**   Copyright 1984, Amiga Computer Inc.   All rights reserved.       *
**   No part of this program may be reproduced, transmitted,          *
**   transcribed, stored in retrieval system, or translated into      *
**   any language or computer language, in any form or by any         *
**   means, electronic, mechanical, magnetic, optical, chemical,      *
**   manual or otherwise, without the prior written permission of     *
**   Amiga Computer Incorporated, 3350 Scott Blvd, Bld #7,            *
**   Santa Clara, CA 95051                                            *
**                                                                    *
***********************************************************************

* 3-26-86  Dale converted to use new greenhills compiler, cleaned up abit
* 11:50 04-Jun-85 LAH Broke up math library to create ROM, RAM and Link.
* 15:50 14-Apr-85 KBS Converted to V25 standard compatible library.
* 09:35 28-Jan-85 MJS moved float/string conversion to V2xstr.asm
* 14:41  1-Jan-85 MJS added float/string conversion
* 16:00 12-Dec-84 MJS first version
* 10:56 30-Dec-84 MJS version for RAM or ROM math library

		xref	 _MathBaseOffset

*------ Basic Math Function Vector Offsets --------------------------

		xref	_LVOSPFix,_LVOSPFlt,_LVOSPCmp,_LVOSPTst,_LVOSPAbs
		xref	_LVOSPNeg,_LVOSPAdd,_LVOSPSub,_LVOSPMul,_LVOSPDiv

		xdef	ffixi,fflti,ffltis				* C entries for basic math functions
		xdef	fcmpi,fabsi,fnegi
		xdef	faddi,fsubi,fmuli,fdivi
		xdef	_abs

* new ones from Dale
* added 3/26/86
* these have parameters already in registers d0,d1
* these are routines just for single precision FLOAT
		xdef	fsfixi,fsdivi,fsaddi,fsmuli,fststi

shit_do:
		add.l	_MathBaseOffset(a6),a0
		move.l	4(sp),d1		* get second parameter
		move.l	(sp),8(sp)		* setup return address
		addq.l	#8,sp			* non-standard c-interface
		jmp	(a0)

* hack stuff here
fmuli:
		lea	_LVOSPMul,a0
		bra	shit_do
faddi:
		lea	_LVOSPAdd,a0
		bra	shit_do
fsubi:
		lea	_LVOSPSub,a0
		bra	shit_do
fdivi:
		lea	_LVOSPDiv,a0
		bra	shit_do
fcmpi:
		lea	_LVOSPCmp,a0
		bra	shit_do

fsmuli:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPMul(a0)

fsaddi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPAdd(a0)

fssubi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPSub(a0)

fsdivi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPDiv(a0)

ffixi:
fsfixi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPFix(a0)

fflti:
ffltis:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPFlt(a0)

fststi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPTst(a0)

_abs:
		move.l	4(sp),d0
fabsi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPAbs(a0)

fnegi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPNeg(a0)

fscmpi:
		move.l	_MathBaseOffset(a6),a0
		jmp	_LVOSPCmp(a0)

		end

