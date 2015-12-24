	section romhunks
	xdef _umuls
	xdef _IMulU		* integer multipy unsigned
*	unsigned short multiply
_umuls:
_IMulU:
	move.w	6(sp),d0
	mulu	10(sp),d0
	rts

	xdef _smuls
	xdef _IMulS
*	signed short multiply
_smuls:
_IMulS:
	move.w	6(sp),d0
	muls	10(sp),d0
	rts

	xdef _sdiv
	xdef _IDivS
*	signed divide
_IDivS:
_sdiv:
	move.l	4(sp),d0
	divs	10(sp),d0
	ext.l	d0
	rts

	xdef _IDivU
_IDivU:
	move.l 4(sp),d0
	divu	10(sp),d0
	swap	d0
	clr.w	d0
	swap	d0
	rts

	END
