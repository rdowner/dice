*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NameFromAnchor ram interface
	XREF	_None
	SECTION	alib
	XDEF	_NameFromAnchor
_NameFromAnchor:
		movem.l	d2/d3/a6,-(a7)
		move.l	_None,a6
		movem.l	16(a7),d1/d2/d3
		jsr	-90(a6)
		movem.l	(a7)+,d2/d3/a6
		rts
	END
