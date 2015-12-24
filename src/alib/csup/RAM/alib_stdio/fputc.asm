*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fputc ram interface
	XREF	_None
	SECTION	alib_stdio
	XDEF	_fputc
_fputc:
		move.l	a6,-(a7)
		move.l	_None,a6
		movem.l	8(a7),d0/d1
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
