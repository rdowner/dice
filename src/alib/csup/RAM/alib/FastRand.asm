*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FastRand ram interface
	XREF	_None
	SECTION	alib
	XDEF	_FastRand
_FastRand:
		move.l	a6,-(a7)
		move.l	_None,a6
		move.l	8(a7),d0
		jsr	-114(a6)
		move.l	(a7)+,a6
		rts
	END
