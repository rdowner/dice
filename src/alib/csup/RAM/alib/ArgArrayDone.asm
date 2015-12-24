*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ArgArrayDone ram interface
	XREF	_None
	SECTION	alib
	XDEF	_ArgArrayDone
_ArgArrayDone:
		move.l	a6,-(a7)
		move.l	_None,a6
		jsr	-186(a6)
		move.l	(a7)+,a6
		rts
	END
