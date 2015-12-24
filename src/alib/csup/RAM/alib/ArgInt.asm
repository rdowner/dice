*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ArgInt ram interface
	XREF	_None
	SECTION	alib
	XDEF	_ArgInt
_ArgInt:
		move.l	a6,-(a7)
		move.l	_None,a6
		movem.l	8(a7),a0/a1
		move.l	16(a7),d0
		jsr	-198(a6)
		move.l	(a7)+,a6
		rts
	END
