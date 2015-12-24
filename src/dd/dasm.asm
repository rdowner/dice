;		 CSYMFMT

;		 objfile "dasm.o"
;		 addsym

	IFD BARFLY
		BOPT	h-,l-,a+,f+,O-,wo-,ws-
		SUPER
	ENDC



; move #$2700,SR
; 91c8 suba.l a0,a0

******************** ********************************************************
******************** ********************************************************
******************** ********************************************************
***									  ***
***			DASM  : A 680x0 Disassembler			  ***
***			Programmed by Mike Schwartz			  ***
***			(c)1988, Midnight Oil Software, Inc.		  ***
***			680x0 Enhancements by Brett Bourbin		  ***
***			(c)1992, Selgus Limited.			  ***
***									  ***
******************** ********************************************************
******************** ********************************************************
******************** ********************************************************

****
**** This disassembler provides symbolic capabilities.	It expects whatever
**** program that calls it to provide a 'C' callable subroutine to do the
**** symbol lookup.  It should be of the form:
****	name = LookupValue(value);
****	char	*name;	value;
**** Obviously, the routine looks for value in a symbol table and returns
**** a pointer to a null-terminated character string that contains the
**** name associated with the symbol.  If no symbolic debugging information
**** is desired, the caller must provide a routine along the following:
****	long	LookupValue() { return 0L; }
****

		XREF		_LookupValue

;
; For development purposes, defining MAKESYMBOLS causes MANX to put
; symbols in the library.
;
MAKESYMBOLS	EQU	1

SYMBOL		MACRO
		IFD	MAKESYMBOLS
		XDEF	\1
		ENDC
		ENDM

;
; size = Disassemble(memptr, pc, outbuf)
; short 	size;		size of instruction in bytes
; UWORD 	*memptr;	address in memory to disassemble
; ULONG 	pc;		program counter where instruction resides
; char		*outbuf;	buffer to store disassembled string
;
; Synopsis:
;	This routine disassembles one instruction from the supplied
;	buffer (memptr) and formats it into ascii suitable for printing
;	in the caller's buffer (outbuf).  The instruction is assumed to
;	be located at the address specified by the program counter (pc)
;	parameter.
;
; These registers always contain these values:
;	a6 = output buffer index
;	a5 = ptr to 'C' parameters
;	a0 = ptr to instruction, postwords...
;	d7 = program counter
;	d6 = instruction size (in bytes)
;	d4 = EA Register (0-7)
;	d3 = EA Mode (0-7)
;	d2 = instruction size (0 = byte, 4 = word, 8 = long)
;	d1 = instruction word


* SCRATCH0	EQUR		D0
* INSTRUCTION	EQUR		D1
* OPSIZE		EQUR		D2
* EAMODE		EQUR		D3
* EAREGISTER	EQUR		D4
* SCRATCH1	EQUR		D5
* BYTESIZE	EQUR		D6
* PROGRAMCOUNTER	EQUR		D7
* MEMPTR		EQUR		A0
* PTR		EQUR		A1
* PARAMS		EQUR		A5
* OUTBUF		EQUR		A6

		SYMBOL		OpCodeMap
OpCodeMap	dc.l		Group0
		dc.l		Group1
		dc.l		Group2
		dc.l		Group3
		dc.l		Group4
		dc.l		Group5
		dc.l		Group6
		dc.l		Group7
		dc.l		Group8
		dc.l		Group9
		dc.l		Group10
		dc.l		Group11
		dc.l		Group12
		dc.l		Group13
		dc.l		Group14
		dc.l		Group15

dregs		REG		d2-d7/a3-a6

		XDEF		_Disassemble
_Disassemble	link		A5,#0
		movem.l 	dregs,-(sp)

		move.l		8(A5),A0	; ptr to instruction
		move.l		12(A5),D7	; program counter
		move.l		16(A5),A6	; output buffer
		moveq.l 	#0,D6		; instruction size (in bytes)

		bsr		FetchWord		; instruction word
		move.w		D0,D1	;


;		bsr		DTab			; put a tab in outbuf

		lea		OpCodeMap,A1		; lookup table address
		move.w		D1,D0
		asr.w		#8,D0		; get OpCode (bits 12-15)
		asr.w		#4,D0
		and.w		#$000f,D0	; for table lookup
		asl.w		#2,D0		; 4 bytes per table entry
		move.l		0(A1,D0.w),A1	; fetch decode routine address
		jsr		(A1)		       ; and call the routine

		move.l		D6,D0		; return value
		movem.l 	(sp)+,dregs
		unlk		A5
		rts

***** Utility Routines

;
; FetchWord
;
; Synopsis:
;	This routine fetches the next word from memory into D0.w, and
;	increments the instruction size in D6 by 2.
;
		SYMBOL		FetchWord
FetchWord	addq		#2,D6		; instruction is 2 bytes bigger
		move.w		(A0)+,D0
		rts

;
; FetchLong
;
; Synopsis:
;	This routine fetches the next long from memory into D0.l and
;	adds 4 to the instruction size in D6.
;
		SYMBOL		FetchLong
FetchLong	addq		#4,D6
		move.l		(A0)+,D0
		rts

;
; JumpTable
; ScaledJumpTable
;
; Synopsis:
;	saves code by doing jump table lookup and jump
;
; assumes A1 = jump table address, D0 = jump table entry #
;				 or D0 = jump table entry # * 4
;
		SYMBOL		JumpTable
JumpTable	asl.w		#2,D0
		SYMBOL		ScaledJumpTable
ScaledJumpTable:
		move.l		0(A1,D0.w),-(sp)
		rts

;
; DString
;
; Synopsis:
;	Puts the string pointed to by A1 on the end of the output buffer.
;
		SYMBOL		DString
DString 	move.b		(A1)+,(A6)+
		bne.s		DString
		subq		#1,A6		; point at null terminator
		rts

;
; DNull
;
; Synopsis:
;	Puts a NULL in outbuf.
;
; NOTE: Does not increment A6 (outbuf ptr) !!!
;
		SYMBOL		DNull
DNull		clr.b		(A6)
		rts

;
; DChar
;
; Synopsis:
;	Puts a character from D0.b in output buffer.
;
		SYMBOL		DChar
DChar		move.b		D0,(A6)+
		bra.s		DNull

		SYMBOL		DTab
DTab		move.l		d0,-(sp)
10$		move.b		#' ',(A6)+
		move.l		A6,d0
		sub.l		16(A5),d0
		and.b		#7,d0
		bne.s		10$
		move.l		(sp)+,d0
		bra.s		DNull

;
; DHex
;
; Synopsis:
;	Converts the hex digit in D0 (bits 0-3) to a hex ascii character
;	and puts it in outbuf.
;
		SYMBOL		DHex
DHex		and.w		#$000f,D0
		lea		hex_string,A1
		move.b		0(A1,D0.w),(A6)+
		bra.s		DNull

		SYMBOL		DComma
DComma		move.b		#',',(A6)+
		bra.s		DNull

		SYMBOL		DA
DA		move.b		#'A',(A6)+
		bra.s		DNull

		SYMBOL		DB
DB		move.b		#'B',(A6)+
		bra.s		DNull

		SYMBOL		DD
DD		move.b		#'D',(A6)+
		bra.s		DNull

		SYMBOL		DS
DS		move.b		#'S',(A6)+
		bra.s		DNull

		SYMBOL		DZ
DZ		move.b		#'Z',(A6)+  ; output a Z
		bra.s		DNull

		SYMBOL		DOpen
DOpen		move.b		#'(',(A6)+
		bra.s		DNull

		SYMBOL		DStar
DStar		move.b		#'*',(A6)+  ; output a star (times sign)
		bra.s		DNull

		SYMBOL		DPlus
DPlus		move.b		#'+',(A6)+
		bra.s		DNull

		SYMBOL		DMinus
DMinus		move.b		#'-',(A6)+
		bra.s		DNull

		SYMBOL		DClose
DClose		move.b		#')',(A6)+
		bra.s		DNull

		SYMBOL		DImmediate
DImmediate	move.b		#'#',(A6)+
		bra.s		DNull

		SYMBOL		DDollar
DDollar 	move.b		#'$',(A6)+
		bra.s		DNull

		SYMBOL		DSlash
DSlash		move.b		#'/',(A6)+
		bra		DNull

		SYMBOL		DColon
DColon: 	move.b		#':',(A6)+  ; output a colon
		bra		DNull

		SYMBOL		DOpenCurly
DOpenCurly:	move.b		#'{',(A6)+  ; output a open curly bracket
		bra		DNull

		SYMBOL		DCloseCurly
DCloseCurly:	move.b		#'}',(A6)+  ; output a close curly bracket
		bra		DNull

		SYMBOL		DOpenBracket
DOpenBracket:	move.b		#'[',(A6)+  ; output a open bracket
		bra		DNull

		SYMBOL		DCloseBracket
DCloseBracket:	move.b		#']',(A6)+  ; output a close bracket
		bra		DNull

;
; DByte
;
; Synopsis:
;	Formats the byte in D0.b as an ascii hex number, preceded by a $.
;
		SYMBOL		DByte
DByte		bsr		DDollar
		SYMBOL		DByte1
DByte1		move.w		D0,-(sp)
		asr.w		#4,D0
		bsr		DHex
		move.w		(sp)+,D0
		bra		DHex

;
; DWord
;
; Synopsis:
;	Formats the word in D0.w as an ascii hex number, preceded by a $.
;
		SYMBOL		DWord
DWord		bsr		DDollar
		SYMBOL		DWord1
DWord1		move.w		D0,-(sp)
		asr.w		#8,D0
		bsr.s		DByte1		; high byte
		move.w		(sp)+,D0
		bra.s		DByte1

;
; DOffset
;
; Synopsis:
;	Formats the word in D0.w as an ascii hex number, preceded by a $.
;	Format as an offset;  negative numbers preceeded by a negative sign
		SYMBOL		DOffset
DOffset		tst.w		D0
		bpl.s		1$
		bsr		DMinus
		neg.w		D0
1$:		bsr		DDollar
		move.w		D0,-(sp)
		asr.w		#8,D0
		bsr.s		DByte1		; high byte
		move.w		(sp)+,D0
		bra.s		DByte1

;
; DLong
;
; Synopsis:
;	Formats the LONG in D0.L as an ascii hex number, preceded by a $.
;
		SYMBOL		DLong	;
DLong		movem.l 	d1-d7/a0-a6,-(sp)
		move.l		d0,-(sp)		; save d0...
		move.l		d0,-(sp)		; push for 'C' call
		jsr		_LookupValue
		addq		#4,sp
		tst.l		d0
		beq.s		10$			; no symbol
		tst.l		(sp)+			; drop old D0
		movem.l 	(sp)+,d1-d7/a0-a6
		move.l		d0,A1
		bra		DString 		; display symbolic
		; not symbolic, print hex number...
10$		move.l		(sp)+,d0
		movem.l 	(sp)+,d1-d7/a0-a6

		move.l		D0,-(sp)
		bsr		DDollar
		swap		D0		; do high word first
		bsr.s		DWord1
20$		move.l		(sp)+,D0
		bra.s		DWord1

;
; EffectiveAddress
;
; Synopsis:
;	This routine takes an Effective Address and formats the
;	assembler language notation and puts it in outbuf.  This routine
;	determines the size of the instruction (byte, word, long) from
;	bits 2 & 3 of the D2 register, the mode from bits 0, 1, & 2 of the
;	D3 register, and the register number from bits 0, 1, & 2 of the D4
;	register.  If an Illegal Mode/Register combination occurs, this
;	routine pops the return address and jumps to invalid_instruction,
;	and for that reason this routine must be CALLed and never jumped to.
;

		SYMBOL		ModeTable			;
ModeTable							; Mode...
		dc.l		DataRegisterDirect		; 000
		dc.l		AddressRegisterDirect		; 001
		dc.l		Indirect			; 010
		dc.l		PostIncrement			; 011
		dc.l		PreDecrement			; 100
		dc.l		IndirectDisplacement		; 101
		dc.l		IndirectIndex			; 110
		dc.l		SpecialMode			; 111

		SYMBOL		SpecialModeTable		;
SpecialModeTable						; Register...
		dc.l		AbsoluteShort			; 000
		dc.l		AbsoluteLong			; 001
		dc.l		ProgramCounterDisplacement	; 010
		dc.l		ProgramCounterIndex		; 011
		dc.l		ImmediateData			; 100
		dc.l		SpecialIllegal			; 101
		dc.l		SpecialIllegal			; 110
		dc.l		SpecialIllegal			; 111

		SYMBOL		DestinationEA
DestinationEA	move.w		D1,D3
		asr.w		#6,D3
		and.w		#7,D3

		move.w		D1,D4
		rol		#7,D4
		and.w		#7,D4
		rts

		SYMBOL		EA
EA		move.w		D1,D3
		asr.w		#3,D3
		and.w		#7,D3		; D3 = MODE

		move.w		D1,D4
		and.w		#7,D4		; D4 = Register
		rts

		SYMBOL		EffectiveAddress
EffectiveAddress
		move.w		D3,D0	; Mode
		lea		ModeTable,A1		; jump table address
		bra		JumpTable

		SYMBOL		DataRegisterDirect
DataRegisterDirect
		bsr		DD
		move.b		D4,D0
		bra		DHex

		SYMBOL		AddressRegisterDirect
AddressRegisterDirect
		bsr		DA
		move.b		D4,D0
		bra		DHex

		SYMBOL		Indirect
Indirect	bsr		DOpen
		bsr.s		AddressRegisterDirect
		bra		DClose

		SYMBOL		PostIncrement
PostIncrement	bsr		Indirect
		bra		DPlus

		SYMBOL		PreDecrement
PreDecrement	bsr		DMinus
		bra.s		Indirect

		SYMBOL		IndirectDisplacement
		XREF		_programA4
IndirectDisplacement
		moveq		#0,d0
		bsr		FetchWord		; fetch postword
		cmp.b		#4,d4
		bne.s		.notA4
		move.w		d0,.temp
		ext.l		d0
		add.l		_programA4,d0
		movem.l 	d1-d7/a0-a6,-(sp)
		move.l		d0,-(sp)
		bsr		_LookupValue
		addq.l		#4,sp
		movem.l 	(sp)+,d1-d7/a0-a6
		tst.l		d0
		beq.s		.notA4x
		move.l		d0,a1
		bsr		DString
		bra.s		Indirect
.notA4x 	move.w		.temp,d0
.notA4		bsr		DOffset			; display it (changed from DWord to get neg sign)
		bra.s		Indirect
.temp		dc.w		0

		SYMBOL		IndirectIndex
IndirectIndex:
		bsr		FetchWord		; fetch postword
		move.w		D0,D5			; preserve it
		btst		#8,D5			; check for enhanced indirect modes
		bne.s		EnhancedIndirect
		bsr		DOpen			; copy a open paren to start
		ext.w		D0			; extend
		beq.s		10$			; skip if zero
		bsr		DOffset			; otherwise show it (was DWord)
		bsr		DComma			; copy a comma
10$:
		bsr		AddressRegisterDirect
		bclr		#6,D5		; clear the index suppress bit
		bsr		CommaIndexRegister	; copy a comma then the index register
		bra		DClose



		SYMBOL		EnhancedIndirect
EnhancedIndirect:
		move		D5,D0			; get the outer displacment size
		and		#3,D0
		beq.s		RegIndirectIndex	; register indirect index
		lea		parenbracket_string(PC),A1
		bsr		DString 		; copy a ([
		bsr		BaseDisplacement	; copy the base displacement

		btst		#7,D5			; check if base is suppressed
		beq.s		10$
		bsr		DZ			; base is suppressed so copy a Z
10$:		btst		#3,D3			; check if PC or address
		bne.s		15$			; working on a PC relative instruction
		bsr		AddressRegisterDirect	; copy the address register
		bra.s		18$
15$:		lea		pc_string(PC),A1	; copy PC register string
		bsr		DString

18$:		btst		#2,D5			; check if pre or post indexed
		bne.s		20$			; pre-indexed
		bsr		DCloseBracket		; copy a close bracket

20$:		bsr		CommaIndexRegister	; copy a comma then the index register
		btst		#2,D5			; check if pre or post indexed
		beq.s		30$			; post-indexed
		bsr		DCloseBracket		; copy a close bracket

30$:		move		D5,D0
		and		#3,D0			; get the outer displacement size
		bne.s		40$			; valid displacement
		addq.l		#4,SP			; pop subroutine call
		bra		invalid_instruction	; invalid size used

40$:		subq		#1,D0			; shift it down one bit
		beq.s		60$			; no displacement field
		bsr		DComma			; copy a comma
		subq		#1,D0			; shift it down one more bit
		bne.s		50$			; LONGWORD base displacement
		bsr		FetchWord		; get the next WORD
		bsr		DWord			; copy the WORD
		bra.s		60$
50$:		bsr		FetchLong		; get the next LONGWORD
		bsr		DLong			; copy the LONGWORD

60$:		bra		DClose			; copy a close paren


		SYMBOL		RegIndirectIndex
RegIndirectIndex:
		bsr		DOpen			; copy a open paren
		bsr		BaseDisplacement	; copy the base displacement

		btst		#7,D5		; check if base is suppressed
		beq.s		10$
		bsr		DZ			; base is suppressed so copy a Z
10$:		btst		#3,D3		; check if PC or address
		bne.s		20$			; working on a PC relative instruction
		bsr		AddressRegisterDirect	; copy the address register
		bra.s		30$
20$:		lea		pc_string(PC),A1	; copy PC register string
		bsr		DString

30$:		bsr		CommaIndexRegister	; copy a comma then the index register
		bra		DClose			; copy a close paren


		SYMBOL		SpecialMode
SpecialMode	move.w		D4,D0	; Register
		lea		SpecialModeTable,A1	; jump table address
		bra		JumpTable




		SYMBOL		AbsoluteShort
AbsoluteShort	moveq		#0,D0
		bsr		FetchWord		; fetch PostWord
		move.l		d0,-(sp)		; save d0...
		movem.l 	d1-d7/a0-a6,-(sp)

		ext.l		d0			; sign extend for lookup
		move.l		d0,-(sp)		; push for 'C' call
		jsr		_LookupValue
		addq		#4,sp
		tst.l		d0
		beq.s		10$			; no symbol

		movem.l 	(sp)+,d1-d7/a0-a6
		tst.l		(sp)+			; drop old D0

		bsr		DOpen
		move.l		d0,A1
		bsr		DString 		; display symbolic
		bsr		DClose
		lea		w_string,A1
		bra		DString

		; not symbolic, print hex number...
10$		movem.l 	(sp)+,d1-d7/a0-a6
		move.l		(sp)+,d0

		bsr		DWord			; and show address
		lea		w_string,A1
		bra		DString

		SYMBOL		AbsoluteLong
AbsoluteLong	bsr		FetchLong		; fetch Post Longword
		bra		DLong
;		lea		l_string,A1
;		bra		DString

		SYMBOL		ProgramCounterDisplacement
ProgramCounterDisplacement
		bsr		FetchWord		; Fetch PostWord
		ext.l		D0		; sign extend it
		add.l		D7,D0 ; add Program Counter
		addq.l		#2,D0		; so LookupValue works
		bsr		DLong
		lea		pcr_string,A1
		bra		DString

		SYMBOL		ProgramCounterIndex
ProgramCounterIndex
		bsr		FetchWord		; Fetch PostWord
		move.w		D0,D5	; preserve it
		btst		#8,D5		; check for enhanced indirect modes
		bne		EnhancedIndirect
		ext.w		D0		; sign extend offset
		ext.l		D0		; into a long offset
		add.l		D7,D0 ; add in PC
		bsr		DLong

		bclr		#6,D5		; clear the index suppress bit
		bsr		OpenIndexRegister	; copy a paren then the index register
		bra		DClose			; )


		SYMBOL		ImmediateData
ImmediateData	bsr		DImmediate
		cmp.b		#8,D2		; long?
		bne.s		10$
		bsr		FetchLong
		bra		DLong
10$		bsr		FetchWord
		cmp.b		#4,D2		; word?
		bne		DByte
		bra		DWord

		SYMBOL		SpecialIllegal
SpecialIllegal	lea		4(sp),sp
		bra		invalid_instruction

;
; IndexRegister
; CommaIndexRegister
; OpenIndexRegister
;
; Synopsis:
;	Output a index register stored in bits 12-14 of D5 in the form
;	Xn.SIZE*SCALE, if the index has not be suppressed.
;
		SYMBOL		OpenIndexRegister
OpenIndexRegister:
		move.b		#'(',D0         ; copy a open paren
		bra.s		IndexRegister
		SYMBOL		CommaIndexRegister
CommaIndexRegister:
		move.b		#',',D0         ; copy a comma
		SYMBOL		IndexRegister
IndexRegister:
		btst		#6,D5		; check if index is suppressed
		bne.s		60$

		bsr		DChar			; copy the starting character
		move		D5,D0	; get postword again
		btst		#15,D0		; Data or address reg
		beq.s		20$
		bsr		DA
		bra.s		30$
20$:		bsr		DD

30$:		move.w		D5,D0	; get postword again
		rol		#4,D0		; get top 4 bits
		and		#7,D0		; isolate register #
		bsr		DHex			; show it

		lea		l_string,A1
		move.w		D5,D0	; get postword final time
		btst		#11,D0
		bne		40$
		lea		w_string,A1
40$:		bsr		DString
		bsr		scale910		; get the scaling
		beq.s		60$			; no scaling used (or 68000)
		bsr		DStar			; copy a star
		move.b		ScaleTable(PC,D0),D0
		bsr		DHex			; copy the ASCII scale value
60$:		rts

		SYMBOL		ScaleTable
ScaleTable:	DC.B		0,2,4,8 		; register scaling values

;
; BaseDisplacement
;
; Synopsis:
;	Output a base displacement for the enhanced addressing modes with
;	the size in bits 4-5 of D5. If a invalid size is found, the
;	call this routine and the EA routine are poped from the stack and
;	control is turned over to invalid_instruction.
;
		SYMBOL		BaseDisplacement
BaseDisplacement:
		move		D5,D0
		lsr		#5,D0		; get the base displacment size
		and		#3,D0
		bne.s		5$			; valid displacement
		addq.l		#8,SP			; remove 2 subroutine calls
		bra		invalid_instruction	; invalid size used

5$:		subq		#1,D0		; shift it down one bit
		beq.s		30$			; no displacement field
		subq		#1,D0		; shift it down one more bit
		bne.s		10$			; LONGWORD base displacement
		bsr		FetchWord		; get the next WORD
		bsr		DWord			; copy the WORD
		bra.s		20$
10$:		bsr		FetchLong		; get the next LONGWORD
		bsr		DLong			; copy the LONGWORD
20$:		bsr		DComma			; copy a comma
30$:		rts



***** Utility routines for instruction decoding routines

;
; Size67
; Size910
;
; Synopsis:
;	Extracts the size of the instruction (byte, word, long) from
;	bits 6 & 7 of the instruction word in D1.  If bits 6 and 7 are
;	both set, then this routine pops the return address and jumps
;	to invalid_instruction.  If a legal value, the ".b", ".w", or
;	".l" string is appended to outbuf, as well as a TAB.  This
;	Routine sets D2 to the appropriate size value for effective
;	address routines to look at.
;
; NOTE:
;	Because this routine pops the return address, you must ALWAYS
;	call it and not jump to it!!!
;

		SYMBOL		size67_table
size67_table	dc.l		b_string,w_string,l_string,0

		SYMBOL		Size910
Size910:	move		D1,D2		; get the size bits
		rol		#7,D2		; rotate upper 7 bits
		and		#3,D2		; keep only bits 9&10
		subq		#1,D2		; adjust to bits 6&7 size range
		bra.s		DSize		; copy the size extension


		SYMBOL		Size67
Size67		move.w		D1,D2
		asr.w		#4,D2

		SYMBOL		DSize
DSize		lea		size67_table,A1
		and.w		#$000c,D2		; bits 6&7 * 4 (bytes/long)
		move.l		0(A1,D2.w),D0 ; fetch string address
		bne.s		10$			; it was legal size
		lea		4(sp),sp		; drop return address
		bra		invalid_instruction
10$		move.l		D0,A1		; address of string (".b", etc.)
		bsr		DString
		bra		DTab


;
; scale910
;
; Synopsis:
;	This subroutine extracts a scaling value for the register from bits 9-10
;	of D5 and returns it in D0.
;
		SYMBOL		scale910
scale910:	move		D5,D0	; get the extension WORD
		rol		#7,D0		; move the bits down
		and		#3,D0		; keep only the scale
		rts

;
; dreg911
;
; Synopsis:
;	This subroutine extracts a register number from bits 9-11 of
;	D1 and formats the register as D0-D7.
;
; reg911
;
; Synopsis:
;	This routine extracts register number as in dreg911, but only formats
;	the register number as 0-7.
;

		SYMBOL		dreg02
dreg02		bsr		DD
		bra.s		reg02

		SYMBOL		areg02
areg02		bsr		DA

		SYMBOL		reg02
reg02		move.w		D1,D0
		and.w		#7,D0
		bra		DHex


		SYMBOL		dreg911
dreg911 	bsr		DD
		bra.s		reg911

		SYMBOL		areg911
areg911 	bsr		DA

		SYMBOL		reg911
reg911		move.w		D1,D0
		rol		#7,D0
		and.w		#7,D0
		bra		DHex


;
; Scratch1Dreg1214
; dreg1214
; areg1214
; reg1214
;
; Synopsis:
;	This subroutine extracts a register number from bits 12-14 of
;	D0 and formats the register as 0-7.
;

		SYMBOL		Scratch1Dreg1214
Scratch1Dreg1214:
		move		D5,D0	; get the extension WORD

		SYMBOL		dreg1214
dreg1214:	bsr		DD			; copy a D first
		bra.s		reg1214

		SYMBOL		areg1214
areg1214:	bsr		DA			; copy a A first

		SYMBOL		reg1214
reg1214:	rol		#4,D0		; get the register number
		and		#7,D0		; keep only the number
		bra		DHex			; copy the ASCII character

		SYMBOL		Dreg
Dreg:		bsr		DD			; copy a D first
		and		#7,D0		; keep only the number
		bra		DHex			; copy the ASCII character



		SYMBOL		invalid_instruction
invalid_instruction
		move.l		16(A5),A6	; destroy previous string...
;;;		bsr		DTab
		lea		dcw_string,A1
		bsr		DString
		bsr		DTab
		move.w		D1,D0
		bsr		DWord
		bsr		DTab
		lea		invalid_string,A1
		bsr		DString
		moveq		#2,D2
		rts

		SYMBOL		immediate_instruction
immediate_instruction
		bsr		DString
		cmp.b		#$3c,D1
		beq		immediate_ccr_instruction
		cmp.b		#$7c,D1
		beq		immediate_sr_instruction
		bsr		Size67		; extract the size info
		SYMBOL		immediate_operand
immediate_operand:
		bsr		ImmediateData
		bsr		DComma
		bsr		EA		; Effective Address D1 bits 0-5
		bsr		EffectiveAddress
		rts

		SYMBOL		immediate_ccr_instruction
immediate_ccr_instruction
		bsr	DTab
		moveq		#0,D2		; size = BYTE
		bsr		ImmediateData
		bsr		DComma
		lea		ccr_string,A1
		bra		DString

		SYMBOL		immediate_sr_instruction
immediate_sr_instruction
		bsr	DTab
		moveq		#4,D2		; size = WORD
		bsr		ImmediateData
		bsr		DComma
		lea		sr_string,A1
		bra		DString



***** Bit Manipulation/MOVEP/Immediate Instructions

		SYMBOL		ori_instruction
ori_instruction
		lea		ori_string,A1
		bra		immediate_instruction

		SYMBOL		andi_instruction
andi_instruction
		lea		andi_string,A1
		bra		immediate_instruction

		SYMBOL		subi_instruction
subi_instruction
		lea		subi_string,A1
		bra		immediate_instruction

		SYMBOL		addi_instruction
addi_instruction
		bsr		test67			; check for CALLM
		beq.s		callm_instruction
		lea		addi_string,A1
		bra		immediate_instruction

		SYMBOL		callm_instruction
callm_instruction:
		lea		callm_string(PC),A1    ; copy the instruction
		bsr		DString
		bsr		DTab			; move to the next column
		moveq		#0,D2		; size = BYTE
		bra		immediate_operand	; copy the immediate operand

		SYMBOL		moves_instruction
moves_instruction:
		lea		moves_string(PC),A1	; copy the instruction
		bsr		DString
		bsr		Size67			; copy the size extension

		bsr		EA			; get the <EA>
		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		btst		#11,D0		; check with direction
		beq.s		10$			; MOVES <EA>,Rn
		bsr		dareg_operand		; copy the register
		bsr		DComma			; copy a comma
		bsr		EffectiveAddress	; copy the effective address
		rts

10$:		bsr		EffectiveAddress	; copy the effective address
		bsr		DComma			; copy a comma
		move		D5,D0
		bra		dareg_operand		; copy the register

		SYMBOL		eori_instruction
eori_instruction
		bsr		test67			; check for CAS[2]
		beq.s		cas_instruction
		lea		eori_string,A1
		bra		immediate_instruction

		SYMBOL		cmpi_instruction
cmpi_instruction
		bsr		test67			; check for CAS[2] instruction
		beq.s		cas_instruction
		lea		cmpi_string,A1
		bra		immediate_instruction

		SYMBOL		rtm_instruction
rtm_instruction:
		btst		#11,D1		; check for MOVES instruction
		bne.s		moves_instruction
		lea		rtm_string(PC),A1      ; copy the instruction
		bsr		DString
		bsr		DTab			; move to the next column
		btst		#3,D1		; check if Data or Address
		beq		dreg02			; copy Data register
		bra		areg02			; copy Address register


		SYMBOL		cas_instruction
cas_instruction:
		bsr		test67			; check for RTM/MOVES
		bne.s		rtm_instruction

		move		D1,D0	; get the <EA>
		and		#$3f,D0
		cmp		#$3c,D0 	; check for 111100 mode
		beq.s		cas2_instruction
		lea		cas_string(PC),A1	; copy the instruction
		bsr		DString
		bsr		Size910 		; copy the size extension
		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		bsr		Dreg			; copy register Dc

		bsr		DComma			; copy a comma
		move		D5,D0
		lsr		#6,D0		; copy register Du
		bsr		Dreg
		bsr		DComma			; copy a comma

		bsr		EA			; get the <EA>
		bsr		EffectiveAddress	; copy the effective address
		rts

		SYMBOL		cas2_instruction
cas2_instruction:
		btst		#11,D1	; check for CHK2 instruction
		beq		chk2_instruction

		lea		cas2_string(PC),A1	; copy the instruction
		bsr		DString
		bsr		Size910 		; copy the size extension
		bsr		FetchLong		; get next 2 extension WORDs
		swap		D0		; swap the extension WORDs
		move.l		D0,D5

		bsr		Dreg			; copy register Dc1
		bsr		DColon			; copy a colon
		swap		D0		; get second extension WORD
		bsr		Dreg			; copy register Dc2
		bsr		DComma			; copy a comma

		move.l		D5,D0
		lsr		#6,D0
		bsr		Dreg			; copy register Du1
		bsr		DColon			; copy a colon
		swap		D0		; get second extension WORD
		lsr		#6,D0
		bsr		Dreg			; copy register Du2
		bsr		DComma			; copy a comma

		move.l		D5,D0
		bsr.s		DDAreg1214		; copy a register in form (Rn)
		swap		D0		; get second extension WORD

		SYMBOL		DDAreg1214
DDAreg1214:	bsr		DOpen			; copy a open paren
		btst		#15,D0		; check if Data or Address
		beq.s		10$
		bsr		areg1214		; copy Address register
		bra.s		20$
10$:		bsr		dreg1214		; copy Data register
20$:		bra		DClose			; copy a close paren


		SYMBOL		chk2_instruction
chk2_instruction:
		lea		chk2_string(PC),A1     ; copy the instruction
		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		btst		#11,D0		; check for CMP2 instruction
		bne.s		eareg_instruction

;
; CMP2 instruction
;
		lea		cmp2_string(PC),A1     ; copy the instruction

		SYMBOL		eareg_instruction
eareg_instruction:
		bsr		DString
		bsr		Size910 		; copy the size extension
		bsr		DTab			; move to the next column
		bsr		EA			; get the <EA>
		bsr		EffectiveAddress	; copy the effective address
		bsr		DComma			; copy a comma
		SYMBOL		dareg_operand
dareg_operand:
		move		D5,D0	; get the extension WORD
		btst		#15,D0		; check if Data or Address
		beq		dreg1214		; copy Data register
		bra		areg1214		; copy Address register


		SYMBOL		movep_instruction
movep_instruction
		move.w		D1,D0	; Instruction Word
		and.w		#$38,D0
		cmp.w		#$38,D0
		beq		dynamic_bit

		lea		movep_string,A1
		bsr		DString
		lea		l_string,A1
		btst		#6,D1		; w/l bit
		bne.s		10$
		lea		w_string,A1
10$		bsr		DString
		bsr		DTab
		btst		#7,D1		; direction bit
		bne.s		20$
		; memory to register...
		move.w		D1,D4
		and.w		#7,D4		; register
		bsr		IndirectIndex

		bsr		DComma

		move.w		D1,D4
		rol		#7,D4
		and.w		#7,D4
		bra		DataRegisterDirect

		; register to memory
20$		move.w		D1,D4
		rol		#7,D4
		and.w		#7,D4
		bsr		DataRegisterDirect
		bsr		DComma
		move.w		D1,D4
		and.w		#7,D4
		bra		IndirectDisplacement


		SYMBOL		bit_type
bit_type
		bsr		DB
		move.w		D1,D0
		asr.w		#4,D0
		and.w		#$000c,D0
		lea		bit_type_table,A1
		move.l		0(A1,D0.w),A1
		bsr		DString
		bra		DTab


		SYMBOL		dynamic_bit
dynamic_bit
		bsr		bit_type
		move.w		D1,D0
		asr.w		#8,D4
		asr.w		#1,D4
		and.w		#7,D4
		bsr		DataRegisterDirect
		bsr		DComma
		bsr		EA		; MUST BE BSR THEN RTS:
		bsr		EffectiveAddress
		rts				; DON'T DELETE ME


		SYMBOL		static_bit
static_bit
		bsr		bit_type
		move.w		D1,D0	; instruction
		and.w		#$38,D0 	; isolate mode bits
		beq.s		10$			; is not data register

		bsr		FetchWord
;		cmp.b		#7,D0		; check count
;		bgt		invalid_instruction
		bra.s		20$
10$		bsr		FetchWord
20$		bsr		DImmediate
		bsr		DByte
		bsr		DComma
		bsr		EA
		bsr		EffectiveAddress
		rts


		SYMBOL		bit_type_table
bit_type_table
		dc.l		tst_string,chg_string,clr_string,set_string

		SYMBOL		OpTable0
		;					; bits 8-11
OpTable0	dc.l		ori_instruction 	; 0000
		dc.l		movep_instruction	; 0001
		dc.l		andi_instruction	; 0002
		dc.l		movep_instruction	; 0003
		dc.l		subi_instruction	; 0004
		dc.l		movep_instruction	; 0005
		dc.l		addi_instruction	; 0006
		dc.l		movep_instruction	; 0007
		dc.l		static_bit		; 0008
		dc.l		movep_instruction	; 0009
		dc.l		eori_instruction	; 0010
		dc.l		movep_instruction	; 0011
		dc.l		cmpi_instruction	; 0012
		dc.l		movep_instruction	; 0013
		dc.l		cas_instruction 	; 0014
		dc.l		movep_instruction	; 0015


		SYMBOL		Group0
Group0		move.w		D1,D0	; Instruction Word
		asr.w		#8,D0
		and.w		#$000f,D0	; isolate bits 8-11
		lea		OpTable0,A1
		bra		JumpTable

**** MOVE (BYTE,WORD,LONG)


		SYMBOL		Group1
Group1
		moveq		#0,D2
		bra.s		move_instruction

		SYMBOL		Group2
Group2
		moveq		#8,D2
		bra.s		move_instruction

		SYMBOL		Group3
Group3
		moveq		#4,D2


		SYMBOL		move_instruction
move_instruction
		lea		move_string,A1
		bsr		DString
		bsr		DSize
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bsr		DestinationEA
		bsr		EffectiveAddress
		rts



**** Miscelaneous


		SYMBOL		DMove
DMove
		lea		move_string,A1
		bra		DString


		SYMBOL		test67
test67
		move.w		D1,D0
		and.b		#$c0,D0
		cmp.b		#$c0,D0
		rts


		SYMBOL		SizeEADString
SizeEADString
		bsr		DString

		SYMBOL		SizeEA
SizeEA
		bsr		Size67

		SYMBOL		JustEA
JustEA
		bsr		EA
		bsr		EffectiveAddress
		rts


		SYMBOL		negx_instruction
negx_instruction
		bsr		test67
		beq.s		from_sr_instruction
		lea		negx_string,A1
		bra.s		SizeEADString

		SYMBOL		from_sr_instruction
from_sr_instruction
		bsr		DMove
		bsr		DTab
		lea		sr_string,A1
		bsr		DString
		bsr		DComma
		moveq		#0,D2		; size = byte
		bra.s		JustEA

		SYMBOL		clr_instruction
clr_instruction:
		bsr		test67			; check for MOVE CCR,<EA>
		beq.s		from_ccr_instruction
		lea		clr_string,A1
		bra.s		SizeEADString

		SYMBOL		neg_instruction
neg_instruction
		bsr		test67
		beq.s		to_ccr_instruction
		lea		neg_string,A1
		bra.s		SizeEADString

		SYMBOL		from_ccr_instruction
from_ccr_instruction:
		bsr		DMove			; copy MOVE instruction
		bsr		DTab			; move the the next column
		lea		ccr_string(PC),A1      ; copy CCR string
		bsr		DString
		bsr		DComma			; copy a comma
		moveq		#0,D2		; size = BYTE
		bra.s		JustEA			; copy the <EA>

		SYMBOL		to_ccr_instruction
to_ccr_instruction
		bsr		DMove
		bsr		DTab
		moveq		#4,D2		; size = word
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		lea		ccr_string,A1
		bra		DString

		SYMBOL		not_instruction
not_instruction
		bsr		test67
		beq.s		to_sr_instruction
		lea		not_string,A1
		bra		SizeEADString

		SYMBOL		to_sr_instruction
to_sr_instruction
		bsr		DMove
		bsr		DTab
		moveq		#4,D2		; size = byte
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		lea		sr_string,A1
		bra		DString

		SYMBOL		nbcd_instruction
nbcd_instruction
		move.w		D1,D0	; instruction word
		and.w		#$f8,D0 	; strip off bits
		cmp.b		#$40,D0
		beq		swap_instruction
		cmp.b		#$80,D0
		beq		extw_instruction
		cmp.b		#$c0,D0
		beq		extl_instruction

		btst		#7,D1
		bne		movem_instruction
		btst		#6,D1
		bne.s		pea_instruction

		move		D1,D0	; check for LONG LINK
		and.b		#$38,D0 	; keep MODE bits
		cmp.b		#8,D0		; check for LINK
		beq.s		linkl_instruction

		lea		nbcd_string,A1
		bsr		DString
		bsr		DTab
		moveq		#0,D2		; size = byte
		bra		JustEA

		SYMBOL		linkl_instruction
linkl_instruction:
		lea		link_string(PC),A1     ; copy the LINK instruction
		bsr		DString
		bsr		DTab			; move to the next column
		bsr		DA			; copy a A
		move		D1,D0
		and		#7,D0		; get the register number
		bsr		DHex			; copy the ASCII number
		bsr		DComma			; copy a comma
		bsr		DImmediate		; copy a immediate character
		bsr		FetchLong		; get the LONGWORD offset
		bra		DLong			; copy the LONGWORD

		SYMBOL		link_instruction
link_instruction
		lea		link_string,A1
		bsr		DString
		bsr		DTab
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DComma
		bsr		DImmediate
		bsr		FetchWord
		bra		DWord

		SYMBOL		pea_instruction
pea_instruction
		lea		pea_string,A1
		bsr		DString
		bsr		DTab
		moveq		#8,D2		; size = long
		bra		JustEA

		SYMBOL		swap_instruction
swap_instruction
		btst		#3,D1		; check for BKPT instruction
		beq.s		bkpt_instruction
		lea		swap_string,A1
		bsr		DString
		bsr		DTab
		move.w		D1,D0	; instruction word
		and.w		#7,D0		; isolate register bits
		bsr		DD
		bra		DHex

		SYMBOL		bkpt_instruction
bkpt_instruction:
		lea		bkpt_string(PC),A1     ; copy the instruction name
		bsr		DString
		bsr		DTab			; move the the next column
		move		D1,D0
		and		#7,D0		; keep only the vector number
		bsr		DImmediate		; copy immediate character
		bra		DByte			; copy a ASCII BYTE

		SYMBOL		extw_instruction
extw_instruction
		lea		extw_string,A1
		bra.s		ext_instruction

		SYMBOL		extl_instruction
extl_instruction
		btst		#8,D1		; check if its a EXTB.L
		bne.s		extbl_instruction
		lea		extl_string,A1
		bra.s		ext_instruction

		SYMBOL		extbl_instruction
extbl_instruction:
		lea		extbl_string(PC),A1    ; copy EXTB.L instruction

		SYMBOL		ext_instruction
ext_instruction
		bsr		DString
		bsr		DTab
		bsr		DD
		move.w		D1,D0
		and.w		#7,D0
		bra		DHex

		SYMBOL		tst_instruction
tst_instruction
		bsr		test67
		beq.s		tas_instruction
		lea		tst_string,A1
		bra		SizeEADString

		SYMBOL		tas_instruction
tas_instruction
		move.w		D1,D0	; instruction word
		and.w		#$3f,D0
		cmp.w		#$3c,D0
		beq.s		illegal_instruction
		lea		tas_string,A1
		bsr		DString
		bsr		DTab
		moveq		#0,D2		; size = 0
		bra		JustEA

		SYMBOL		illegal_instruction
illegal_instruction
		lea		illegal_string,A1
		bra		DString

		SYMBOL		divl_table
divl_table:	DC.L		divul_string,divull_string,divsl_string,divsll_string

		SYMBOL		mull_table
mull_table:	DC.L		mulul_string,mulull_string,mulsl_string,mulsll_string

		SYMBOL		divl_instructions
divl_instructions:
		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		lsr		#8,D0		; get bits 10&11 * 4
		and		#$c,D0
		move.l		divl_table(PC,D0),A1
		bra.s		math_instruction

		SYMBOL		lmath_instructions
lmath_instructions:
		btst		#6,D1		; check for LONG DIV instructions
		bne.s		divl_instructions
;
; LONG MULU/MULS instructions
;
		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		lsr		#8,D0		; get bits 10&11 * 4
		and		#$c,D0
		move.l		mull_table(PC,D0),A1
		SYMBOL		math_instruction
math_instruction:
		bsr		DString 		; copy the instruction
		bsr		DTab			; move to the next column

		bsr		EA			; get the <EA>
		bsr		EffectiveAddress	; copy the effective address
		bsr		DComma			; copy a comma

		move		D5,D0
		and		#7,D0		; get Dr/Dh
		rol		#4,D5		; rotate upper 4 bits
		and		#7,D5		; get Dq/Dl
		cmp		D0,D5	; check if they are the same register
		beq.s		10$

		bsr		Dreg			; copy Dr/Dh register
		bsr		DColon			; copy a colon
10$:		move		D5,D0
		bra		Dreg			; copy Dq/Dl register

		SYMBOL		movem_instruction
movem_instruction
		btst		#7,D1		; check for LONG form math
		beq.s		lmath_instructions

		lea		movem_string,A1
		bsr		DString

		lea		l_string,A1
		moveq		#8,D2		; size = long
		btst		#6,D1		; check size bit
		bne.s		10$
		lea		w_string,A1
		moveq		#4,D2		; size = word
10$		bsr		DString
		bsr		DTab
		btst		#10,D1	; direction bit
		beq.s		to_memory
		; memory to registers
	bsr	FetchWord
	move.l	D0,-(sp)
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
	move.l	(sp)+,D0
		bra.s		register_mask

		SYMBOL		to_memory
to_memory
	bsr	FetchWord
; flip mask
	move.l	d1,-(sp)
	move.w	D0,D5
	moveq	#16-1,d1
11$
	lsr.w	#1,D5
	addx.w	D0,D0
	dbra	d1,11$
	move.l	(sp)+,d1

		bsr		register_mask
		bsr		DComma
		bsr		EA
		bsr		EffectiveAddress
		rts

********************* (WARNING: icky logic ahead)
		SYMBOL		register_mask
register_mask
* D0 = mask
	move.l	d1,-(sp)

	swap	D0
	move.w	#1,D0
	swap	D0
	moveq	#-1,D5		; D5 = bit counter
1$
	addq.w	#1,D5
11$
	cmp.w	#15,D5
	bgt	90$
	btst	D5,D0	; test mask bit
	beq	1$
; do a reg
	moveq	#'D',d1
	btst	#3,D5		; test Address or Data reg
	beq	2$
	moveq	#'A',d1
2$
	move.b	d1,(A6)+	    ; write reg type
	move.b	D5,d1		; write reg number
	andi.b	#7,d1			;
	ori.b	#'0',d1                 ;
	move.b	d1,(A6)+	    ;
	cmp.w	#15,D5
	beq	90$
; range?
	move.w	D5,d1		; check adjacent bit
	addq.w	#1,d1
	btst	d1,D0
	bne	5$
	move.b	#'/',(A6)+          ; write separator
	bra	1$
5$ ; do range
	move.b	#'-',(A6)+          ; write separator
3$
	addq.w	#1,D5
	btst	D5,D0
	bne	3$
	subq.w	#1,D5
	cmp.w	#15,D5
	ble	11$
	moveq	#15,D5
	bra	11$
90$
	cmp.b	#'/',-1(A6)
	bne	99$
	sub.l	#1,A6		; backup over last "/"
99$
	sf	(A6)
	move.l	(sp)+,d1
	rts

 IFNE 0
		SYMBOL		rmask_table
rmask_table
		dc.l		a7_string,a6_string,a5_string,a4_string
		dc.l		a3_string,a2_string,a1_string,a0_string
		dc.l		d7_string,d6_string,d5_string,d4_string
		dc.l		d3_string,d2_string,d1_string,d0_string

		SYMBOL		register_mask
register_mask
		bsr		FetchWord		; get register mask
		movem.l 	D3-D7/A5,-(sp)		; save regs
		move.w		D0,d3		; save it
		lea		rmask_table,a5

		move.w		D1,D0	; instruction word
		and.b		#$38,D0

		moveq		#0,d6			; bit #
		moveq		#1,d5			; bit # increment

		cmp.b		#$20,D0
		beq.s		10$

		moveq		#15,d6			; bit #
		moveq		#-1,d5			; bit # increment

10$		moveq		#15,D7			; bit counter
		moveq		#0,d4			; flag: !0 = put / first
20$		btst		d6,d3			; bit in mask set?
		beq.s		40$			; if not
		tst.b		d4
		beq.s		30$			; don't put / first
		bsr		DSlash
30$		st		d4
		move.w		d6,D0
		asl.w		#2,D0
		move.l		(A5,D0.w),A1
		bsr		DString
40$		add.w		d5,d6			; add increment to bit #
		dbra		D7,20$			; do all bits
		movem.l 	(sp)+,D3-D7/A5
		rts
	ENDC

		SYMBOL		trap_instructions
trap_instructions
		btst		#7,D1
		bne.s		jsr_instruction
		move.w		D1,D0	; instruction word
		asr.w		#3,D0
		and.w		#7,D0
		lea		TrapTable,A1
		bra		JumpTable

		SYMBOL		jsr_instruction
jsr_instruction
		btst		#6,D1
		bne.s		jmp_instruction
		lea		jsr_string,A1
		bsr		DString
		bsr		DTab
		moveq		#8,D2		; size = long
		bra		JustEA

		SYMBOL		jmp_instruction
jmp_instruction
		lea		jmp_string,A1
		bsr		DString
		bsr		DTab
		moveq		#8,D2
		bra		JustEA

		SYMBOL		chk_instruction
chk_instruction
		btst		#6,D1
		bne.s		lea_instruction
		lea		chk_string,A1
		bsr		DString
		moveq		#4,D2		; size = word
		btst		#7,D1		; check the size
		bne.s		10$			; true WORD size
		moveq		#8,D2		; size = LONG
		lea		l_string(PC),A1        ; copy a .L for the instruction
		bsr		DString
10$:
		bsr		DTab
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bra		dreg911

		SYMBOL		lea_instruction
lea_instruction
		lea		lea_string,A1
		bsr		DString
		bsr		DTab
		moveq		#4,D2
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bsr		DA
		bra		reg911

		SYMBOL		trap_instruction
trap_instruction
		lea		trap_string,A1
		bsr		DString
		bsr		DTab
		move.w		D1,D0
		and.w		#15,D0
		bra		DByte

		SYMBOL		unlk_instruction
unlk_instruction
		lea		unlk_string,A1
		bsr		DString
		bsr		DTab
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bra		DHex

		SYMBOL		to_usp_instruction
to_usp_instruction
		bsr		DMove
		lea		l_string,A1
		bsr		DString
		bsr		DTab
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DComma
		lea		usp_string,A1
		bra		DString

		SYMBOL		from_usp_instruction
from_usp_instruction
		bsr		DMove
		lea		l_string,A1
		bsr		DString
		bsr		DTab
		lea		usp_string,A1
		bsr		DString
		bsr		DComma
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bra		DHex

		SYMBOL		reset_instructions
reset_instructions
		lea		ResetTable,A1
		move.w		D1,D0
		and.w		#7,D0
		bra		JumpTable

		SYMBOL		reset_instruction
reset_instruction
		lea		reset_string,A1
		bra		DString

		SYMBOL		nop_instruction
nop_instruction
		lea		nop_string,A1
		bra		DString

		SYMBOL		stop_instruction
stop_instruction
		lea		stop_string,A1
		bsr		DString
		bsr		DTab
		bsr		FetchWord
		bsr		DImmediate
		bra		DWord

		SYMBOL		rte_instruction
rte_instruction
		lea		rte_string,A1
		bra		DString

		SYMBOL		rts_instruction
rts_instruction
		lea		rts_string,A1
		bra		DString

		SYMBOL		rtd_instruction
rtd_instruction:
		lea		rtd_string(PC),A1      ; copy the instruction
		bsr		DString
		moveq		#4,D2		; size = WORD
		bra		ImmediateData		; copy the immediate data

		SYMBOL		trapv_instruction
trapv_instruction
		lea		trapv_string,A1
		bra		DString

		SYMBOL		rtr_instruction
rtr_instruction
		lea		rtr_string,A1
		bra		DString

		SYMBOL		MovecTable
MovecTable:	DC.L		spc_string,dfc_string,cacr_string,tc_string
		DC.L		itt0_string,itt1_string,dtt0_string,dtt1_string
		DC.L		usp_string,vbr_string,caar_string,msp_string
		DC.L		isp_string,mmusr_string,urp_string,srp_string

		SYMBOL		movec_instruction
movec_instruction:
		lea		movec_string(PC),A1    ; copy the instruction
		bsr		DString
		bsr		DTab			; move to the next column

		bsr		FetchWord		; get the extension WORD
		move		D0,D5
		and		#7,D0		; keep control register low bits
		btst		#11,D5		; check if high bit was set
		beq.s		10$
		addq		#8,D0		; skip over first 8 entries
10$:		lsl		#2,D0		; get index * 4
		move.l		MovecTable(PC,D0),A1

		btst		#0,D1		; check which direction
		beq.s		20$			; MOVEC Rc,Rn
		bsr		dareg_operand		; copy the register
		bsr		DComma			; copy a comma
		bsr		DString 		; copy the control register

20$:		bsr		DString 		; copy the control register
		bsr		DComma			; copy a comma
		bra		dareg_operand		; copy the register


		SYMBOL		MiscTable
MiscTable	dc.l		negx_instruction	; 000
		dc.l		clr_instruction 	; 001
		dc.l		neg_instruction 	; 010
		dc.l		not_instruction 	; 011
		dc.l		nbcd_instruction	; 100
		dc.l		tst_instruction 	; 101
		dc.l		movem_instruction	; 110
		dc.l		trap_instructions	; 111

		SYMBOL		TrapTable
TrapTable	dc.l		trap_instruction	; 000
		dc.l		trap_instruction	; 001
		dc.l		link_instruction	; 010
		dc.l		unlk_instruction	; 011
		dc.l		to_usp_instruction	; 100
		dc.l		from_usp_instruction	; 101
		dc.l		reset_instructions	; 110
		dc.l		movec_instruction	; 111

		SYMBOL		ResetTable
ResetTable	dc.l		reset_instruction	; 000
		dc.l		nop_instruction 	; 001
		dc.l		stop_instruction	; 010
		dc.l		rte_instruction 	; 011
		dc.l		rtd_instruction 	; 100
		dc.l		rts_instruction 	; 101
		dc.l		trapv_instruction	; 110
		dc.l		rtr_instruction 	; 111

		SYMBOL		Group4
Group4
		btst		#8,D1
		bne		chk_instruction

		lea		MiscTable,A1
		move.w		D1,D0
		rol		#7,D0
		and.w		#7,D0
		bra		JumpTable


**** add quick, subtract quick, set conditionally, decrement instructions

		SYMBOL		ConditionTable
ConditionTable
		dc.b		"T F HILSCCCLNEEQVCVSPLMIGELTGTLE"
		SYMBOL		BConditionTable
BConditionTable
		dc.b		"RA??HILSHSLONEEQVCVSPLMIGELTGTLE"

		SYMBOL		DBCondition
DBCondition
		lea		BConditionTable,A1
		bra.s		DC_entry

		SYMBOL		DCondition
DCondition
		lea		ConditionTable,A1
		SYMBOL		DC_entry
DC_entry	move.w		D1,D0
		asr.w		#8,D0
		and.w		#$000f,D0
		add.w		D0,D0		; 2 bytes/entry
		move.b		0(A1,D0.w),(A6)+
		cmp.b		#' ',1(A1,D0.w)
		beq.s		99$
		move.b		1(A1,D0.w),(A6)+
99$		clr.b		(A6)
		rts


		SYMBOL		Group5
Group5
		bsr		test67
		beq.s		set_instruction
		btst		#8,D1
		bne.s		subq_instruction

		SYMBOL		addq_instruction
addq_instruction
		lea		addq_string,A1

		SYMBOL		addq_finish
addq_finish
		bsr		DString
		bsr		Size67
		bsr		DImmediate
		move.w		D1,D0		; instruction word
		rol		#7,D0
		and.w		#7,D0
		bne.s		10$
		moveq		#8,D0
10$		bsr		DByte
		bsr		DComma
		bsr		EA
		bsr		EffectiveAddress
		rts

		SYMBOL		subq_instruction
subq_instruction
		lea		subq_string,A1
		bra.s		addq_finish

		SYMBOL		set_instruction
set_instruction
		move.w		D1,D0
		and.w		#$38,D0
		cmp.w		#8,D0
		beq.s		dbxx_instruction
		cmp		#$38,D0 	; check for TRAPcc instruction
		bne.s		10$
		move		D1,D0
		and		#7,D0		; get the register/opmode
		cmp		#1,D0		; check if it is a Scc one
		bgt.s		trapxx_instruction

10$:		bsr		DS
		bsr		DCondition
		bsr		DTab
		bsr		EA
		bsr		EffectiveAddress
		rts

		SYMBOL		dbxx_instruction
dbxx_instruction
		lea		db_string,A1
		bsr		DString
		bsr		DCondition
	cmp.b	#'F',-1(A6)
	bne.s	10$
	move.b	#'R',-1(A6)
	move.b	#'A',(A6)+
	sf	(A6)
10$		bsr		DTab
		bsr		DD
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DComma
		bsr		FetchWord
		ext.l		D0
		add.l		D7,D0
		add.l		#2,D0
		bra		DLong

		SYMBOL		trapxx_instruction
trapxx_instruction:
		lea		trap_string(PC),A1	; copy the instruction
		bsr		DString
		bsr		DCondition		; copy the condition codes
		move		D1,D0
		and		#7,D0		; get the operand size
		cmp		#4,D0		; check for any operand
		beq.s		10$			; no operand for this instruction
		lea		w_string(PC),A1 ; get a ".W" size extension
		cmp		#3,D0		; check for LONG operand
		beq.s		20$
		cmp		#2,D0		; make sure not invalid
		bne		invalid_instruction

		bsr		DString 		; copy the ".W" extension
		bsr		DTab			; move to the next column
		moveq		#4,D2		; size = WORD
		bra		ImmediateData		; copy the immediate data
10$:		rts

20$:		lea		l_string(PC),A1 ; copy a ".L" size extension
		bsr		DString
		bsr		DTab			; move to the next column
		moveq		#8,D2		; size = LONG
		bra		ImmediateData		; copy the immediate data


**** Conditional Branch Instructions


		SYMBOL		Group6
Group6
		move.w		D1,D0
		and.w		#$0f00,D0
		cmp.w		#$0100,D0
		beq.s		bsr_instruction

		SYMBOL		bxx_instruction
bxx_instruction
		bsr		DB
		bsr		DBCondition

		SYMBOL		bxx_finish
bxx_finish
		move.b		D1,D0
		beq.s		10$			; size = WORD
		cmp.b		#$ff,D0 	; check for LONG instruction
		bne.s		20$			; size = BYTE

		lea		l_string(PC),A1        ; copy a .L for the instruction
		bsr		DString
		bsr		FetchLong		; get a LONGWORD
		bra.s		40$
10$:		bsr		FetchWord		; get a WORD
		bra.s		30$
20$:		ext		D0		; make into a WORD
		lea		s_string(PC),A1        ; copy a .S for the instruction
		bsr		DString
30$:		ext.l		D0		; make into a LONGWORD
40$:		bsr		DTab			; move to the next column
		add.l		D7,D0
		addq		#2,D0
		bra		DLong

		SYMBOL		bsr_instruction
bsr_instruction
		lea		bsr_string,A1
		bsr		DString
		bra.s		bxx_finish

**** MOVE QUICK instruction


		SYMBOL		Group7
Group7
		btst		#8,D1
		bne		invalid_instruction

		lea		moveq_string,A1
		bsr		DString
		bsr		DTab
		bsr		DImmediate
		move.b		D1,D0
		bsr		DByte
		bsr		DComma
		bra		dreg911

**** OR, DIVIDE, SUBTRACT DECIMAL instructions


; 831e should be or.b d1,(a6)+ is sbcd -(a6),-(a1)
; *** This is now fixed [BSB 18-June-92]

		SYMBOL		OrTable
OrTable:
		dc.l		or_instruction		; bits 8-6 000
		dc.l		or_instruction		;	   001
		dc.l		or_instruction		;	   010
		dc.l		divu_instruction	;	   011
		dc.l		sbcd_instruction	;	   100
		dc.l		pack_instruction	;	   101
		dc.l		unpk_instruction	;	   110
		dc.l		divs_instruction	;	   111

		SYMBOL		Group8
Group8
		move.w		D1,D0
		and.w		#$01c0,D0
		lsr		#4,D0		; adjust table index
		lea		OrTable(PC),A1		; get Group8 table
		bra		ScaledJumpTable 	; call Group8 routine

		SYMBOL		or_instruction
or_instruction
		lea		or_string,A1

		SYMBOL		or_finish
or_finish
		bsr		DString
		bsr		Size67
		btst		#8,D1
		bne.s		10$			; dn,ea
		; ea,dn
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bra		dreg911

10$		bsr		dreg911
		bsr		DComma
		bsr		EA
		bsr		EffectiveAddress
		rts


		SYMBOL		divu_instruction
divu_instruction
		lea		divu_string,A1

		SYMBOL		divu_finish
divu_finish
		bsr		DString
		bsr		DTab
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bra		dreg911


		SYMBOL		divs_instruction
divs_instruction
		lea		divs_string,A1
		bra.s		divu_finish

		SYMBOL		sbcd_instruction
sbcd_instruction
		move		D1,D0	; get the instruction
		and		#$0030,D0		; keep bits 5-4
		bne.s		or_instruction		; must be OR.B instruction

		lea		sbcd_string,A1
		bsr		DString
		bsr		DTab

		SYMBOL		sbcd_finish
sbcd_finish
		btst		#3,D1
		beq.s		both_registers	; if register to register

		SYMBOL		both_predecrements
both_predecrements:
		bsr		DMinus
		bsr		DOpen
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DClose

		bsr		DComma

		bsr		DMinus
		bsr		DOpen
		bsr		DA
		bsr		reg911
		bra		DClose

		SYMBOL		both_registers
both_registers:
		bsr		DD
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DComma
		bra		dreg911


		SYMBOL		unpk_instruction
unpk_instruction:
		lea		unpk_string(PC),A1	; copy the command
		bra.s		pack_finish


		SYMBOL		pack_instruction
pack_instruction:
		lea		pack_string(PC),A1	; copy the command

		SYMBOL		pack_finish
pack_finish:
		move		D1,D0	; get the instruction
		and		#$0030,D0		; keep bits 5-4
		bne		or_instruction		; must be OR.B instruction

		bsr		DString
		bsr		DTab			; move to the next column
		btst		#3,D1		; check if registers
		beq.s		10$			; register to register
		bsr		both_predecrements	; -(An),-(An)
		bra.s		20$
10$:
		bsr		both_registers		; Dn,Dn
20$:
		bsr		DComma			; output a comma
		moveq		#4,D2		; immediate WORD size
		bra		ImmediateData		; output adjustment


**** SUB, SUBX instructions


		SYMBOL		Group9
Group9: 	move		D1,D0	; check for SUBA instruction
		and		#$1c0,D0		; keep opmode bits
		cmp		#$0c0,D0		; check for first SUBA
		beq.s		suba_instruction
		cmp		#$1c0,D0		; check for second SUBA
		beq.s		suba_instruction

		move.w		D1,D0
		and.w		#$130,D0
		cmp.w		#$0100,D0
		beq.s		subx_instruction

		SYMBOL		sub_instruction
sub_instruction
		lea		sub_string,A1
		bra		or_finish

		SYMBOL		suba_instruction
suba_instruction:
		lea		suba_string(PC),A1	; copy the instruction
		bra		address_operand

		SYMBOL		subx_instruction
subx_instruction
		lea		subx_string,A1
		bsr		DString
		bsr		Size67
		bra		sbcd_finish

**** LINE A instructions


		SYMBOL		Group10
Group10
		moveq		#2,D6
		lea		dcw_string,A1
		bsr		DString
		bsr		DTab
		moveq		#0,D0
		move.w		D1,D0
		bsr		DWord
		bsr		DTab
		lea		aline_string,A1
		bra		DString


**** CMP, CMPM, EOR instructions


		SYMBOL		GROUP11_TABLE
GROUP11_TABLE	dc.l		cmp_instruction
		dc.l		cmp_instruction
		dc.l		cmp_instruction
		dc.l		cmpa_instruction
		dc.l		cmpm_instruction
		dc.l		cmpm_instruction
		dc.l		cmpm_instruction
		dc.l		cmpa_instruction

; b501 should be eor.b d2,d1 is cmpm.b (a1)+,(a2)+
; *** This is now fixed [BSB 18-June-92]

		SYMBOL		Group11
Group11
		move.w		D1,D0
		asr.w		#6,D0
		and.w		#7,D0
		lea		GROUP11_TABLE(pc),A1
		bra		JumpTable

		SYMBOL		cmpa_instruction
cmpa_instruction
		move		D1,D0	; get the instruction
		and		#$01c0,D0		; keep the opmode
		cmp		#$0100,D0		; check for EOR.B
		beq		eor_instruction

		lea		cmpa_string,A1
		bsr		DString
		move.w		#%1111111001111111,D0 ; bit67 = 01
		btst		#8,D1
		beq.s		10$
		move.w		#%1111111010111111,D0 ; bit67 = 10
10$		and.w		D0,D1
		bsr		Size67
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bra		areg911

		SYMBOL		cmpm_instruction
cmpm_instruction
		move.w		D1,D0
		and.w		#$38,D0
		cmp.w		#$08,D0
		bne.s		eor_instruction

		lea		cmpm_string,A1
		bsr		DString
		bsr		Size67
		bsr		DOpen
		bsr		DA
		move.w		D1,D0
		and.w		#7,D0
		bsr		DHex
		bsr		DClose
		bsr		DPlus
		bsr		DComma
		bsr		DOpen
		bsr		DA
		bsr		reg911
		bsr		DClose
		bra		DPlus

		SYMBOL		cmp_instruction
cmp_instruction
		lea		cmp_string,A1
		bsr		DString
		bsr		Size67
		bsr		EA
		bsr		EffectiveAddress
		bsr		DComma
		bra		dreg911

		SYMBOL		eor_instruction
eor_instruction
		lea		eor_string,A1
		bra		or_finish

**** AND, MULTIPLY, EXG, ABCD instructions


		SYMBOL		Group12
Group12
		move.w		D1,D0
		and.w		#$01c0,D0
		cmp.w		#$01c0,D0
		beq.s		muls_instruction
		cmp.w		#$00c0,D0
		beq.s		mulu_instruction

		move.w		D1,D0
		and.w		#$01f8,D0
		cmp.w		#$0188,D0
		beq.s		exgm_instruction
		cmp.w		#$0148,D0
		beq.s		exga_instruction
		cmp.w		#$140,D0
		beq.s		exgd_instruction

		and.w		#$01f0,D0
		cmp.w		#$0100,D0
		beq.s		abcd_instruction


		SYMBOL		and_instruction
and_instruction
		lea		and_string,A1
		bra		or_finish

		SYMBOL		muls_instruction
muls_instruction
		lea		muls_string,A1
		bra		divu_finish

		SYMBOL		mulu_instruction
mulu_instruction
		lea		mulu_string,A1
		bra		divu_finish

		SYMBOL		dexg
dexg		lea		exg_string,A1
		bsr		DString
		bra		DTab


		SYMBOL		exgm_instruction
exgm_instruction
		bsr		dexg
		bsr		dreg911
		bsr		DComma
		bra		areg02

		SYMBOL		exga_instruction
exga_instruction
		bsr		dexg
		bsr		areg911
		bsr		DComma
		bra		areg02

		SYMBOL		exgd_instruction
exgd_instruction
		bsr		dexg
		bsr		dreg911
		bsr		DComma
		bra		dreg02

		SYMBOL		abcd_instruction
abcd_instruction
		lea		abcd_string,A1
		bra		sbcd_finish


**** ADD, ADDX instructions


		SYMBOL		Group13
Group13
		move.w		D1,D0
		and.w		#$1c0,D0
		cmp.w		#$c0,D0
		beq.s		adda_instruction
		cmp.w		#$1c0,D0
		beq.s		adda_instruction
		; determine addx or add...
		move.w		D1,D0
		and.w		#$0130,D0
		cmp.w		#$0100,D0
		beq.s		addx_instruction

		SYMBOL		add_instruction
add_instruction
		lea		add_string,A1
		bra		or_finish

		SYMBOL		adda_instruction
adda_instruction
		lea		adda_string,A1

		SYMBOL		address_operand
address_operand:
		bsr		DString
		lea		w_string,A1
		move.l		#4,D2
		btst		#8,D1
		beq.s		10$
		lea		l_string,A1
		move.l		#8,D2
10$		bsr		DString
		bsr		DTab
		bsr		EA		; MUST BE BSR THEN RTS:
		bsr		EffectiveAddress
		bsr		DComma
		bsr		DA
		move.w		D1,D0
		asr.w		#8,D0
		asr.w		#1,D0
		and.w		#7,D0
		bra		DHex

		SYMBOL		addx_instruction
addx_instruction
		lea		addx_string,A1
		bsr		DString
		bsr		Size67
		bra		sbcd_finish

**** Shift/Rotate/Bit Field instructions

		SYMBOL		ShiftTable
ShiftTable	dc.l		asr_string,lsr_string,roxr_string,ror_string
		dc.l		asl_string,lsl_string,roxl_string,rol_string

		SYMBOL		dshift
dshift
		lea		ShiftTable,A1
		btst		#8,D1
		beq.s		10$
		addq.w		#4,D0
10$		asl.w		#2,D0
		move.l		0(A1,D0.w),A1
		bra		DString
;		bra		DTab


		SYMBOL		Group14
Group14
		bsr		test67
		beq.s		memory_field

		SYMBOL		register_shift
register_shift
		move.w		D1,D0
		asr.w		#3,D0
		and.w		#3,D0
		bsr		dshift
		bsr		Size67
;		bsr		DTab
		btst		#5,D1
		beq.s		10$
		bsr		dreg911
		bsr		DComma
		bra		dreg02
10$		bsr		DImmediate
		move.w		D1,D0
		rol		#7,d0
		and.w		#7,d0
		bne.s		20$
		moveq		#8,d0
20$		bsr		DHex
		bsr		DComma
		bra		dreg02

		SYMBOL		BitfieldTable
BitfieldTable:	DC.L		bftst_string,bfextu_string,bfchg_string,bfexts_string
		DC.L		bfclr_string,bfffo_string,bfset_string,bfins_string

		SYMBOL		memory_field
memory_field:
		move.w		D1,D0
		asr.w		#8,D0
		btst		#3,D0		; check for bit field instructions
		beq		memory_shift		; must be memory shift instruction

;
; Bit field insructions
;
		and		#7,D0		; keep instruction index
		lsl		#2,D0		; make LONGWORD index
		lea		BitfieldTable(PC,D0),A1
		move.l		(A1),A1 	      ; get the string pointer
		bsr		DString 		; copy the string
		bsr		DTab			; tab to the next column

		bsr		FetchWord		; get the second extension WORD
		move		D0,D5
		bsr		EA			; set the EA bits
		move		D1,D0
		lsr		#8,D0		; keep the bit field instruction
		and		#7,D0
		btst		#0,D0		; check type of <EA> needed
		beq.s		10$			; only a <EA>{} is needed
		cmp		#7,D0		; check if it is a BFINS
		bne.s		20$			; no, needs <EA>{},Dn

		bsr		Scratch1Dreg1214	; copy Dn,<EA>{OFFSET:WIDTH}
		bsr		DComma

;
; Output a <EA>{offset:width} for a instruction
;
10$:		bsr		EffectiveAddress
		bsr		DOpenCurly		; copy a open curly bracket
		move		D5,D0
		lsr		#6,D0		; get the offset
		and		#$3f,D0 	; keep only the offset
		bsr		DByte			; copy the bit field offset
		bsr		DColon			; copy a colon
		move		D5,D0
		and		#$3f,D0 	; keep only the width
		bsr		DByte			; copy the bit field width
		bra		DCloseCurly		; copy a close curly bracket

20$:		bsr.s		10$			; copy the <EA>{OFFSET:WIDTH}
		bsr		DComma			; copy a comma
		bra		Scratch1Dreg1214	; copy the data register


		SYMBOL		memory_shift
memory_shift:
		asr.w		#1,D0
		and.w		#3,D0
		bsr		dshift
		lea		b_string,A1
		bsr		DString
;		bsr		DTab
		bsr		EA
		bsr		EffectiveAddress
		rts

**** LINE F/Coprocessor Interface instructions


		SYMBOL		Group15
Group15
		moveq		#2,D6
		lea		dcw_string,A1
		bsr		DString
		bsr		DTab
		moveq		#0,D0
		move.w		D1,D0
		bsr		DWord
		bsr		DTab
		lea		fline_string,A1
		bra		DString

**** Strings

string		MACRO
		SYMBOL		\1
\1		dc.b		\2,0
		ENDM

		SYMBOL		invalid_string
invalid_string	dc.b		";",$20,"***INVALID***",0

		SYMBOL		aline_string
aline_string	dc.b		";",$20,"***A-LINE***",0

		SYMBOL		fline_string
fline_string	dc.b		";",$20,"***F-LINE***",0

		string		hex_string,"0123456789ABCDEF"

		string		d0_string,"D0"
		string		d1_string,"D1"
		string		d2_string,"D2"
		string		d3_string,"D3"
		string		d4_string,"D4"
		string		d5_string,"D5"
		string		d6_string,"D6"
		string		d7_string,"D7"
		string		a0_string,"A0"
		string		a1_string,"A1"
		string		a2_string,"A2"
		string		a3_string,"A3"
		string		a4_string,"A4"
		string		a5_string,"A5"
		string		a6_string,"A6"
		string		a7_string,"A7"

		string		dcw_string,"DC.W"

		string		chg_string,"CHG"
		string		set_string,"SET"

		string		parenbracket_string,"(["

		string		pcr_string,"(PC)"
		string		pc_string,"PC"
		string		ccr_string,"CCR"
		string		sr_string,"SR"
		string		usp_string,"USP"

		string		spc_string,"SFC"
		string		dfc_string,"DFC"
		string		vbr_string,"VBR"
		string		cacr_string,"CACR"
		string		caar_string,"CAAR"
		string		isp_string,"ISP"
		string		tc_string,"TC"
		string		itt0_string,"ITT0"
		string		itt1_string,"ITT1"
		string		dtt0_string,"DTT0"
		string		dtt1_string,"DTT1"
		string		msp_string,"MSP"
		string		mmusr_string,"MMUSR"
		string		urp_string,"URP"
		string		srp_string,"SRP"

		string		b_string,".B"
		string		w_string,".W"
		string		l_string,".L"
		string		s_string,".S"

		string		abcd_string,"ABCD"
		string		add_string,"ADD"
		string		adda_string,"ADDA"
		string		addi_string,"ADDI"
		string		addq_string,"ADDQ"
		string		addx_string,"ADDX"
		string		and_string,"AND"
		string		andi_string,"ANDI"
		string		asl_string,"ASL"
		string		asr_string,"ASR"
		string		bfchg_string,"BFCHG"
		string		bfclr_string,"BFCLR"
		string		bfexts_string,"BFEXTS"
		string		bfextu_string,"BFEXTU"
		string		bfffo_string,"BFFFO"
		string		bfins_string,"BFINS"
		string		bfset_string,"BFSET"
		string		bftst_string,"BFTST"
		string		bgnd_string,"BGND"
		string		bkpt_string,"BKPT"
		string		bsr_string,"BSR"
		string		callm_string,"CALLM"
		string		cas_string,"CAS"
		string		cas2_string,"CAS2"
		string		chk_string,"CHK"
		string		chk2_string,"CHK2"
		string		cinv_string,"CINV"
		string		clr_string,"CLR"
		string		cmp_string,"CMP"
		string		cmpa_string,"CMPA"
		string		cmpi_string,"CMPI"
		string		cmpm_string,"CMPM"
		string		cmp2_string,"CMP2"
		string		cpush_string,"CPUSH"
		string		db_string,"DB"
		string		divs_string,"DIVS"
		string		divsl_string,"DIVS.L"
		string		divsll_string,"DIVSL.L"
		string		divu_string,"DIVU"
		string		divul_string,"DIVU.L"
		string		divull_string,"DIVUL.L"
		string		eor_string,"EOR"
		string		eori_string,"EORI"
		string		exg_string,"EXG"
		string		extbl_string,"EXTB.L"
		string		extw_string,"EXT.W"
		string		extl_string,"EXT.L"
		string		illegal_string,"ILLEGAL"
		string		jmp_string,"JMP"
		string		jsr_string,"JSR"
		string		lea_string,"LEA"
		string		link_string,"LINK"
		string		lsl_string,"LSL"
		string		lsr_string,"LSR"
		string		move_string,"MOVE"
		string		move16_string,"MOVE16"
		string		movec_string,"MOVEC"
		string		movem_string,"MOVEM"
		string		movep_string,"MOVEP"
		string		moveq_string,"MOVEQ"
		string		moves_string,"MOVES"
		string		muls_string,"MULS"
		string		mulsl_string,"MULS.L"
		string		mulsll_string,"MULSL.L"
		string		mulu_string,"MULU"
		string		mulul_string,"MULU.L"
		string		mulull_string,"MULUL.L"
		string		nbcd_string,"NBCD"
		string		neg_string,"NEG"
		string		negx_string,"NEGX"
		string		nop_string,"NOP"
		string		not_string,"NOT"
		string		or_string,"OR"
		string		ori_string,"ORI"
		string		pack_string,"PACK"
		string		pea_string,"PEA"
		string		pflush_string,"PFLUSH"
		string		pflusha_string,"PFLUSHA"
		string		pload_string,"PLOAD"
		string		pmove_string,"PMOVE"
		string		ptest_string,"PTEST"
		string		reset_string,"RESET"
		string		rol_string,"ROL"
		string		ror_string,"ROR"
		string		roxl_string,"ROXL"
		string		roxr_string,"ROXR"
		string		rtd_string,"RTD"
		string		rte_string,"RTE"
		string		rtm_string,"RTM"
		string		rtr_string,"RTR"
		string		rts_string,"RTS"
		string		sbcd_string,"SBCD"
		string		stop_string,"STOP"
		string		sub_string,"SUB"
		string		suba_string,"SUBA"
		string		subi_string,"SUBI"
		string		subq_string,"SUBQ"
		string		subx_string,"SUBX"
		string		swap_string,"SWAP"
		string		tas_string,"TAS"
		string		trap_string,"TRAP"
		string		trapv_string,"TRAPV"
		string		tst_string,"TST"
		string		unlk_string,"UNLK"
		string		unpk_string,"UNPK"

		END
