*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** waitbeam ram interface
	XREF	_None
	SECTION	alib
	XDEF	_waitbeam
_waitbeam:
		move.l	a6,-(a7)
		move.l	_None,a6
		move.l	8(a7),d0
		jsr	-138(a6)
		move.l	(a7)+,a6
		rts
	END
