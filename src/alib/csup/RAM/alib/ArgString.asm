*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ArgString ram interface
	XREF	_None
	SECTION	alib
	XDEF	_ArgString
_ArgString:
		movem.l	a2/a6,-(a7)
		move.l	_None,a6
		movem.l	12(a7),a0/a1/a2
		jsr	-204(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
