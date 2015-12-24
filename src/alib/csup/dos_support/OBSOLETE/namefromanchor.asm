	include "exec/types.i"
	include "exec/memory.i"
	include "dos/dos.i"
	include "dos/dosextens.i"
	include "dos/dosasl.i"

	XREF	_DOSBase

*LONG
*NameFromAnchor (anchor, buffer, buflen)
*	struct AnchorPath *anchor;
*	char *buffer;
*	LONG buflen;
*{
*	struct AChain *chain;
*
*	/* override if bit is set */
*	if (DOSBase->dl_Root->rn_Flags & RNF_PRIVATE1)
*		return DosNameFromAnchor(anchor,buffer,buflen);
*
*	if (buflen == 0)
*	{
*		SetIoErr(ERROR_LINE_TOO_LONG);
*		return FALSE;
*	}
*
*	*buffer = '\0';
*
*	for (chain = anchor->ap_Base;
*	     chain != NULL;
*	     chain = chain->an_Child)
*	{
*		/* Must check if the node's a pattern node! */
*		if (!AddPart(buffer,
*			     chain->an_Flags & DDF_PatternBit ?
*					&(chain->an_Info.fib_FileName[0]) :
*					&chain->an_String[0]),
*			     buflen)
*		{
*			return FALSE;	/* IoErr already set */
*		}
*	}
*
*	return DOSTRUE;
*}

* The LVO is private!!  Comes after MatchPattern in the library

	XREF	_LVOMatchPattern
	XREF	_LVOSetIoErr
	XREF	_LVOAddPart

_LVODosNameFromAnchor EQU _LVOMatchPattern-6

	XDEF	_NameFromAnchor

_NameFromAnchor:
	link	a5,#0
	movem.l	d2-d3/a2-a4/a6,-(a7)
	movem.l	8(a5),D1/D2/D3		; get args - 4 for rtn addr, 4 for lnk

	move.l	_DOSBase,a6
	move.l	dl_Root(a6),a0
	btst.b	#RNB_PRIVATE1,rn_Flags+3(a0)
	 beq.s	not_overridden

	;-- overridden by dos lib routine
	jsr	_LVODosNameFromAnchor(a6)
	bra.s	exit

not_overridden:
	tst.l	d3
	bne.s	bufflen_not_0

	;-- bufflen == 0
	moveq	#ERROR_LINE_TOO_LONG,d1
	jsr	_LVOSetIoErr(a6)
error:
	moveq	#0,d0
	bra.s	exit

bufflen_not_0:
	move.l	d2,a3		; buffer
	clr.b	(a3)
	move.l	d1,a2		; anchor

	move.l	ap_Base(a2),a4	; chain

for_loop:
	move.l	a4,d0
	beq.s	for_done

	move.l	a3,d1				; buffer
	btst.b	#DDB_PatternBit,an_Flags(a4)
	beq.s	1$
	lea	an_Info+fib_FileName(a4),a0	;chain->an_Info.fib_FileName
	bra.s	2$
1$	lea	an_String(a4),a0		;chain->an_String
2$	move.l	a0,d2
	;-- d3 already buflen

	jsr	_LVOAddPart(a6)
	tst.l	d0
	beq.s	error

	;-- loop
	move.l	an_Child(a4),a4
	bra.s	for_loop

	;-- success
for_done:
	moveq	#DOSTRUE,d0
exit:
	movem.l	(a7)+,d2-d3/a2-a4/a6
	unlk	a5
	rts
	

	END
