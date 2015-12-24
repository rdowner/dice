*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ArgArrayInit ram interface
	XREF	_None
	SECTION	alib
	XDEF	_ArgArrayInit
_ArgArrayInit:
		move.l	a6,-(a7)
		move.l	_None,a6
		movem.l	8(a7),a0/a1
		jsr	-192(a6)
		move.l	(a7)+,a6
		rts
	END
