*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DoMethodA ram interface
	XREF	_None
	SECTION	alib
	XDEF	_DoMethodA
_DoMethodA:
		movem.l	a2/a6,-(a7)
		move.l	_None,a6
		move.l	12(a7),a2
		move.l	16(a7),a1
		jsr	-252(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
