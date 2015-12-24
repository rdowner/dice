*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** arnd ram interface
	XREF	_None
	SECTION	alib
	XDEF	_arnd
_arnd:
		move.l	a6,-(a7)
		move.l	_None,a6
		movem.l	8(a7),d0/d1/a0
		jsr	-150(a6)
		move.l	(a7)+,a6
		rts
	END
