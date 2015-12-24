*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** putchar ram interface
	XREF	_None
	SECTION	alib_stdio
	XDEF	_putchar
_putchar:
		move.l	a6,-(a7)
		move.l	_None,a6
		move.l	8(a7),d0
		jsr	-60(a6)
		move.l	(a7)+,a6
		rts
	END
