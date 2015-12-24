*
* Stubs to use standard C code (stack args) to call the Pool
* code...
*
			section	CPools
*
			XREF	_SysBase
			XREF	_AsmFreePooled
			XREF	_AsmAllocPooled
			XREF	_AsmDeletePool
			XREF	_AsmCreatePool
*
_LibFreePooled:		XDEF	_LibFreePooled
			move.l	a6,-(a7)
			move.l	_SysBase,a6
			movem.l	8(a7),a0/a1
			move.l	16(a7),d0
			jsr	_AsmFreePooled
			move.l	(a7)+,a6
			rts
*
_LibAllocPooled:	XDEF	_LibAllocPooled
			move.l	a6,-(a7)
			move.l	_SysBase,a6
			move.l	8(a7),a0
			move.l	12(a7),d0
			jsr	_AsmAllocPooled
			move.l	(a7)+,a6
			rts
*
_LibDeletePool:		XDEF	_LibDeletePool
			move.l	a6,-(a7)
			move.l	_SysBase,a6
			move.l	8(a7),a0
			jsr	_AsmDeletePool
			move.l	(a7)+,a6
			rts
*
_LibCreatePool:		XDEF	_LibCreatePool
			movem.l	d2/a6,-(a7)
			move.l	_SysBase,a6
			movem.l	12(a7),d0/d1/d2
			jsr	_AsmCreatePool
			movem.l	(a7)+,d2/a6
			rts
*
			END
