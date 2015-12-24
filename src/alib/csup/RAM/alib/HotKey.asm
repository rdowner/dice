*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** HotKey ram interface
	XREF	_None
	SECTION	alib
	XDEF	_HotKey
_HotKey:
		move.l	a6,-(a7)
		move.l	_None,a6
		movem.l	8(a7),a0/a1
		move.l	16(a7),d0
		jsr	-210(a6)
		move.l	(a7)+,a6
		rts
	END
