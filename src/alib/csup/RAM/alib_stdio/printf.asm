*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** printf ram interface
	XREF	_None
	SECTION	alib_stdio
	XDEF	_printf
_printf:
		move.l	a6,-(a7)
		move.l	_None,a6
		lea	8(a7),a0
		jsr	-24(a6)
		move.l	(a7)+,a6
		rts
	END
