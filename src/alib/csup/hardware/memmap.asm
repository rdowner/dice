

	DATA

		XDEF	_AbsExecBase
_AbsExecBase	EQU	4


		XDEF	_cartridge
_cartridge	EQU	$0f00000


		XDEF	_bootrom
_bootrom	EQU	$0f80000


		XDEF	_romstart
_romstart	EQU	$0fc0000


		XDEF	_romend
_romend		EQU	$0ffffff

	END
