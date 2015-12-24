*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CreateStdIO ram interface
	XREF	_None
	SECTION	alib
	XDEF	_CreateStdIO
_CreateStdIO:
		move.l	a6,-(a7)
		move.l	_None,a6
		move.l	8(a7),a0
		jsr	-48(a6)
		move.l	(a7)+,a6
		rts
	END
