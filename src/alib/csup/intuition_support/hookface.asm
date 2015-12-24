*  $Id: hookface.asm,v 30.0 1994/06/10 18:10:22 dice Exp $

	INCLUDE 'exec/types.i'
	INCLUDE 'utility/hooks.i'

	xdef	_CallHook
	xdef	_CallHookA
	xdef	_HookEntry

****************************************************************************
* new hook standard
* use struct Hook (with minnode at the top)
*
* *** register calling convention: ***
*	A0 - pointer to hook itself
*	A1 - pointer to parameter packed ("message")
*	A2 - Hook specific address data ("object," e.g, gadget )
*
* ***  C conventions: ***
* Note that parameters are in unusual register order: a0, a2, a1.
* This is to provide a performance boost for assembly language
* programming (the object in a2 is most frequently untouched).
* It is also no problem in "register direct" C function parameters.
*
* calling through a hook
*	CallHook( hook, object, msgid, p1, p2, ... );
*	CallHookA( hook, object, msgpkt );
*
* using a C function:	CFunction( hook, object, message );
*	hook.h_Entry = HookEntry;
*	hook.h_SubEntry = CFunction;
*
****************************************************************************

* C calling hook interface for prepared message packet
_CallHookA:
	movem.l	a2/a6,-(sp)	; protect
	move.l	12(sp),a0	; hook
	move.l	16(sp),a2	; object
	move.l	20(sp),a1	; message
	; ------ now have registers ready, invoke function
	pea.l	hreturn(pc)
	move.l	h_Entry(a0),-(sp)	; old rts-jump trick
	rts
hreturn:
	movem.l	(sp)+,a2/a6
	rts

* C calling hook interface for "varargs message packet"
_CallHook:
	movem.l	a2/a6,-(sp)	; protect
	move.l	12(sp),a0	; hook
	move.l	16(sp),a2	; object
	lea.l	20(sp),a1	; message
	; ------ now have registers ready, invoke function
	pea.l	hpreturn(pc)
	move.l	h_Entry(a0),-(sp)	; old rts-jump trick
	rts
hpreturn:
	movem.l	(sp)+,a2/a6
	rts

* entry interface for C code (large-code, stack parameters)
_HookEntry:
	move.l	a1,-(sp)
	move.l	a2,-(sp)
	move.l	a0,-(sp)
	move.l	h_SubEntry(a0),a0	; C entry point
	jsr	(a0)
	lea	12(sp),sp
	rts

	end
