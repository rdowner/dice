;                CSYMFMT

;                objfile "dbuglib.o"
;                addsym

;                incdir  "include:"

                include "exec/types.i"
                include "exec/tasks.i"
                include "exec/execbase.i"
                include "exec/memory.i"
                include "dos/dos.i"
                include "dos/dosextens.i"
                include "dos/doshunks.i"

;                include "lvo/exec.lvo"
;                include "lvo/dos.lvo"

        IFD BARFLY
                BOPT    h-,l-,a+,f+,O-,wo-,ws-
                SUPER
        ENDC

********************************************************************************

sBase           equ     4

EXEC            MACRO
                xref    _LVO\1
                move.l  a6,-(sp)
                move.l  sBase,a6
                jsr     _LVO\1(a6)
                move.l  (sp)+,a6
                ENDM

DOS             MACRO
                xref    _LVO\1
                move.l  a6,-(sp)
                move.l  dBase,a6
                jsr     _LVO\1(a6)
                move.l  (sp)+,a6
                ENDM

************************************************************************

* Memory Allocation

memList         dc.l    0

                XDEF    _MallocAny
_MallocAny      move.l  8(sp),d1
                bra.s   Malloc

                XDEF    _MallocChip
_MallocChip     moveq.l #MEMF_CHIP,d1
                bra.s   Malloc
                XDEF    _MallocFast
_MallocFast     moveq.l #MEMF_FAST,d1
                bra.s   Malloc
                XDEF    _MallocPublic
_MallocPublic   moveq.l #MEMF_PUBLIC,d1

Malloc          or.l    #MEMF_CLEAR,d1
                move.l  4(sp),d0
                addq.l  #8,d0                   ; allocate 8 extra bytes (link,size)
                move.l  d0,-(sp)
                EXEC    AllocMem
                move.l  (sp)+,d1
                tst.l   d0
                beq.s   MallocError
                move.l  d0,a0
                move.l  memList,(a0)+           ; link into memList
                move.l  d0,memList
                move.l  d1,(a0)+                ; store size, also
                move.l  a0,d0
                rts

MallocError     moveq   #0,d0
                rts

                XDEF    _Free
_Free           move.l  4(sp),a1
                subq.l  #8,a1
                lea     memList,a0
.find           move.l  (a0),d0                 ; link ahead
                beq.s   .exit                   ; ignore if not found
                cmp.l   d0,a1                   ; found block pointing to me?
                beq.s   .found
                move.l  d0,a0
                bra.s   .find
.found          move.l  (a1),(a0)               ; unlink from memList
                move.l  4(a1),d0
                EXEC    FreeMem
.exit           rts


                XDEF    _CleanMem
_CleanMem       move.l  memList,d0
                beq.s   .exit2                  ; if memory list is NULL
                move.l  d0,a1
                move.l  (a1),memList            ; unlink memory from list
                move.l  4(a1),d0                ; fetch the size
                EXEC    FreeMem                 ; free it
                bra.s   _CleanMem               ; do entire list
.exit2          rts

********************************************************************************

                XDEF    debugSR,debugPC,debugD0,debugA7
debugSR         dc.w    0
debugPC         dc.l    0
debugD0         ds.l    8
debugA0         ds.l    7
debugA7         dc.l    0

                XREF    _debugStack,_debugStackTop
                XREF    _thisProcess,_thisCli,_exeCommandName,_commandNameSave,_systemTrapHandler
                XREF    _programState,_programSR,_programPC,_programD0,_programA7,_programStack,_programStackTop

;
; EnterProgram
;
; Synopsis:
;       This subroutine switches from debugger context to program context.
;       Program context proceeds until an Exec Exception occurs.  When this
;       happens, the Exception # from the top of stack is saved in programState,
;       and debugger mode is entered.  EnterProgram then returns to its 'C'
;       caller.  The program state variables reflect the new values, etc.
;
                XDEF    _EnterProgram
_EnterProgram   move.l  a5,-(sp)
                lea     Super,a5
                EXEC    Supervisor
                move.l  (sp)+,a5
                rts

;
; This routine is entered in supervisor mode.  This means the debugger's SR (word) and
; PC (LONG - rts instruction at end of EnterDebugger) are on the top of the stack.
; But this is the system stack and not the debugger's - that's in USP (IMPORTANT).
;
; Normally, _LVOSupervisor initiated routines exit via RTE directly back to the
; caller.  In this case, we will pop the debugger's SR and PC from the stack and
; save them.  We will also save the d0-d7/a0-a6 registers and USP (debugger's A7).
; We then push the program's SR and PC on the stack, load its register set from
; RAM variables, and RTE.  The RTE then starts executing the PROGRAM - this means
; that the task will be used by the target program until the debugger somehow gains
; control again.
;
; So far, the methodology of "swapping contexts" from debugger to program state has
; been described.  While in program state, the program might do various OS operations
; that would make the debugger not-transparent.  Since the program and debugger are
; sharing the same task, the debugger needs its own stack so the program won't see/trash
; values that would crash the debugger.  As well, there are various CLI and TASK structure
; fields that are set up by the OS for the debugger so the debugger needs to set those
; up for the program's needs.  Thus a full context switch involves swapping SR, PC, registers,
; AND OS structure fields.
;
; Notes:
;       Debugger must be cautious about using resources that might be required by the
;       program - signal bits are an obvious gotcha.
;
;       Because we are only diddling the SR and PC on the top of the stack, we should
;       not have to care about what CPU model we are on.  Leave the advanced stack
;       frames alone and let the processor do the right thing when we RTE.
;
Super           ori.w   #$700,sr                ; disable interrupts during supervisor mode
        ; first save debugger state
                move.w  (sp)+,debugSR
                move.l  (sp)+,debugPC
                movem.l d0-d7/a0-a6,debugD0
                move.l  usp,a0
                move.l  a0,debugA7              ; all registers are trashable from here down
        ; then setup program OS state
                lea     _exeCommandName,a2
                move.l  a2,d0
                lsr.l   #2,d0
                move.l  _thisCli,a2
                move.l  d0,cli_CommandName(a2)  ; So argv[0] works :)
                move.l  _thisProcess,a2
                move.l  _programStack,TC_SPLOWER(a2)     ; set task's TC_stack fields
                add.l   _programStackTop,d0
                addq.l  #2,d0
                move.l  d0,TC_SPUPPER(a2)       ; upper/lower so stack bounds checking code works

        ; now install trap handler so exception gets MY attention!!!
        ; this may break with debugged programs that install their own (and don't follow the rules) ...
                lea     TrapHandler,a1
                move.l  a1,TC_TRAPCODE(a2)      ; stuff trap code handler
        ; now setup program registers and supervisor-style stack
                move.l  _programA7,a1
                move.l  a1,usp
                movem.l _programD0,d0-d7/a0-a6   ; all registers are program's now, beware!
                move.l  _programPC,-(sp)         ; program counter
                move.w  _programSR,-(sp)         ; status register
                rte                             ; EXECUTE PROGRAM

;
; The program has executed until an exception occured.  The Exec exception handlers cause the
; exception # to be pushed on the stack, followed by the standard SR, PC supervisor stack.
; Cleverly, the STATE_ equates in the main program just happen to be the same as the
; exception #'s :)
;
; What we want to do is to go back into debugger context so the debugger can report on
; the occurance of the exception.  This involves saving the exception #, SR, and PC
; from the stack in the programState, programSR, and programPC variables.  The program's
; register set is saved to RAM, as well as the pertinent OS structure fields.  The debugger's
; register set and environment is restored and the RTE returns to the rts instruction at
; the end of EnterProgram().
;
; The debugger can cause the program to generate an exception - this is the key to making things
; like stepping and breakpoints possible.  An elegant solution for stepping is for the debugger
; to set bit 15 of programSR.  Each time EnterProgram is called, it will execute one instruction
; and then generate a STATE_TRACE exception.  Tracing will cease when the debugger clears bit
; 15 of programSR.  Breakpoints are implemented by replacing the instruction at the desired
; address with an ILLEGAL ($4afc) opcode.  When EnterProgram is called, the program will execute
; until the ILLEGAL instruction is encountered.  Then the STATE_ILLEGAL exception will occur
; and the debugger will regain control.
;
; Other tricks:
;       What happens when the program runs until it terminates?  This is a special case where
;       no exception would be generated!  The trick is for the debugger to push the address
;       of an illegal instruction on the program's stack before initially executing the
;       program.  All programs exit to the CLI by RTS!  Thus when the program does RTS,
;       it executes ILLEGAL right away and the debugger gains control.  The exitcode of
;       the program will be in programD0 and the PC will point at the ILLEGAL.
;
TrapHandler     move.l  (sp)+,_programState

        ; Save program state
                move.w  (sp)+,_programSR
                move.l  (sp)+,_programPC
                movem.l d0-d7/a0-a6,_programD0
                move.l  usp,a1
                move.l  a1,_programA7                    ; registers are scratch from here down

        ; Restore debugger state
                move.l  debugA7,a1
                move.l  a1,usp
                move.l  _thisProcess,a2
                move.l  _debugStack,TC_SPLOWER(a2)
                move.l  _debugStackTop,TC_SPUPPER(a2)
                move.l  _systemTrapHandler,TC_TRAPCODE(a2)
                move.l  _thisCli,a2
                move.l  _commandNameSave,cli_CommandName(a2)
                move.l  debugPC,-(sp)
                move.w  debugSR,-(sp)
                movem.l debugD0,d0-d7/a0-a6
                rte


************************************************************************

                XDEF    _TargetExit
_TargetExit     dc.w    $4afc                   ; ILLEGAL instruction

************************************************************************

                END
