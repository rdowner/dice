        section romhunks
*
*       68000 SUPPORT LIBRARY
*       COPYRIGHT 1984 GREEN HILLS SOFTWARE
*       MAY 28, 1984
*       32 REMAINDER,DIVIDE, AND MULTIPLY PACKAGE
*
*       THIS PACKAGE IS DIVIDED INTO TWO PARTS.
*       ALL REMAINDERS AND DIVIDES ARE DONE WITH ENTRY
*       POINTS TO A 32 BIT DIVIDE/REMAINDER ROUTINE.
*       THIS PACKAGE USES a0,a1,d0,d1,d2.
*
*       UNSIGNED AND SIGNED 32 BIT MULTIPLIES ARE IDENTICAL,
*       AND ARE PERFORMED BY THE SAME ROUTINE. THIS ROUTINE
*       USES d0,d1,d2, AND a0.
*
*       32 BIT REMAINDER AND DIVIDE ROUTINE
*       PARAMETERS IN d1 AND d2
*       THIS PACKAGE HAS BEEN TESTED ON 5000 RANDOM NUMBER
*       PAIRS AGAINST THE VAX. THE NUMBER GENERATION SCHEME
*       GENERATED NUMBERS OF SIZE 1-32 BITS RANDOMLY, AND SHOULD
*       THEREFORE HAVE EXERCISED MOST CODE PATHS.
*
*       COMMENTS AND SUGGESTIONS FOR IMPROVEMENTS ARE WELCOME.
POSDIV:
        cmpi.l  #$0FFFF,d2      *DIVISOR MORE THAN 16 BITS OF SIGNIF?
        bgt.s   LONGDIV         *YES, GOTO ITERATION DIVIDE
        move.w  d1,a1           *STANDARD 32/16 DIVIDE WITHOUT LOSS
        clr.w   d1
        swap    d1
        divu    d2,d1
        move.l  d1,d0
        swap    d1
        move.w  a1,d0
        divu    d2,d0
        move.w  d0,d1
        clr.w   d0
        swap    d0
        rts
LONGDIV:
        move.l  d1,d0
        clr.w   d0
        swap    d0
        swap    d1
        clr.w   d1
        move.l  d2,a1
        moveq   #$0F,d2
LABEL1: add.l   d1,d1           *BEGIN DIVIDE BY ITERATION
        addx.l  d0,d0
        cmp.l   d0,a1
        bgt.s   LDEX
        sub.l   a1,d0
        addq.w  #1,d1
LDEX:   dbf     d2,LABEL1
        rts
*
* lmul, ulmul
* 32 BIT MULTIPLY ROUTINES
* 
        xdef    lmult
        xdef    ulmult
lmult:
ulmult:
        move.l  d2,-(sp)        *SAVE d2 REGISTER
        move.l  d0,d2
        mulu    d1,d2
        move.l  d2,a0
        move.l  d0,d2
        swap    d2
        mulu    d1,d2
        swap    d1
        mulu    d1,d0
        add.l   d2,d0
        swap    d0
        clr.w   d0
        add.l   d0,a0
        move.l  a0,d0
        move.l  (sp)+,d2
        rts
*
*       ulmodt
*       UNSIGNED 32 BIT REMAINDER 
        xdef    ulmodt
ulmodt: move.l  d2,-(sp)                *SAVE d2
        move.l  d1,d2   *SETUP PARAMETERS
        move.l  d0,d1
        bsr.s   POSDIV          *CALL UNSIGNED DIV/REM ROUTINE
        move.l  (sp)+,d2                *RESTORE d2
        rts
*
*       uldiv
*       UNSIGNED 32 BIT DIVIDE
        xdef    uldivt
uldivt:
        move.l  d2,-(sp)                *SAVE d2 AND d3
        move.l  d1,d2
        move.l  d0,d1   *LOAD UP PARAMETERS
        bsr     POSDIV
        move.l  d1,d0
        move.l  (sp)+,d2
        rts
*
*       lrem
*       SIGNED 32 BIT REMAINDER
*       IF DIVISOR IS NEGATIVE, RESULT IS NEGATIVE.
        xdef    lmodt
lmodt:
        move.l  d2,-(sp)        *SAVE d2
        move.l  d1,d2
        bge.s   lrem1
        neg.l   d2
lrem1:
        move.l  d0,d1
        moveq   #0,d0           *NEGATION FLAG OFF
        tst.l   d1
        bge.s   lrem2
        neg.l   d1
        not.l   d0              *NEGATION FLAG ON
lrem2:  move.l  d0,a0           *MOVE FLAG TO A SAFE PLACE
        bsr     POSDIV
        move.w  a0,d2
        beq.s   lremDONE
        neg.l   d0
lremDONE:       move.l (sp)+,d2         *RESTORE d2 
        rts
*
*       ldivt   
*       SIGNED 32 BIT DIVIDE
*       IF DIVIDEND XOR DIVISOR IS NEGATIVE,
*       RESULT MUST BE NEGATIVE.
        xdef    ldivt
ldivt:
        move.l  d2,-(sp)
        move.l  d0,a0
        moveq   #0,d0
        move.l  d1,d2
        bge.s   ldiv1
        neg.l   d2              *NEGATE OPERAND
        not.l   d0
ldiv1:  move.l  a0,d1
        bge.s   ldiv2
        neg.l   d1              *NEGATE OPERAND
        not.l   d0
ldiv2:  move.l  d0,a0           *TUCK AWAY NEGATION FLAG
        bsr     POSDIV
        move.l  a0,d2
        beq.s   ldivRET
        neg.l   d1
ldivRET:        move.l  d1,d0
        move.l  (sp)+,d2
        rts

        end

