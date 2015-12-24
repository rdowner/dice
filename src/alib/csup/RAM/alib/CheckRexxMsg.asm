*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CheckRexxMsg ram interface
	XREF	_None
	SECTION	alib
	XDEF	_CheckRexxMsg
_CheckRexxMsg:
		move.l	a6,-(a7)
		move.l	_None,a6
		move.l	8(a7),a0
		jsr	-228(a6)
		move.l	(a7)+,a6
		rts
	END
