*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** callHookPkt ram interface
	XREF	_None
	SECTION	alib
	XDEF	_callHookPkt
_callHookPkt:
		movem.l	a2/a6,-(a7)
		move.l	_None,a6
		movem.l	12(a7),a0/a2
		move.l	20(a7),a1
		jsr	-210(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
