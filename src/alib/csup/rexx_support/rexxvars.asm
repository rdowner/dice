* === rexxvars.asm =====================================================
*
* $Id: rexxvars.asm,v 30.8 1994/08/18 05:54:55 dice Exp dice $
*
* Copyright (c) 1988 William S. Hawes (All Rights Reserved)
*
* $Log: rexxvars.asm,v $
;; Revision 30.8  1994/08/18  05:54:55  dice
;; .
;;
;; Revision 30.0  1994/06/10  18:10:49  dice
;; .
;;
;; Revision 30.0  1994/06/10  18:10:49  dice
;; .
;;
* Revision 36.1  90/08/28  10:03:12  mks
* First "QUICK-FIX" of RexxVars.asm for the trashing of D2.
* This is a correct fix but not most optimal.
* 
* Also, made to assemble on my system.
* 
* RexxVars.asm is now under RCS control
* 
* ======================================================================
* Functions to implement the ARexx direct variable interface.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"
         INCLUDE  "rexx/errors.i"
         INCLUDE  "rexx/rexx.i"

LINKSYS MACRO           ; link to a library without having to see a _LVO
        MOVE.L  A6,-(SP)
        MOVE.L  \2,A6
        JSR     _LVO\1(A6)
        MOVE.L  (SP)+,A6
        ENDM

CALLSYS MACRO           ; call a library via A6 without having to see _LVO
        JSR     _LVO\1(A6)
        ENDM

XLIB    MACRO           ; define a library reference without the _LVO
        XREF    _LVO\1
        ENDM
;

         XREF     _AbsExecBase

         XDEF     CheckRexxMsg
         XDEF     _CheckRexxMsg
         XDEF     GetRexxVar
         XDEF     _GetRexxVar
         XDEF     SetRexxVar
         XDEF     _SetRexxVar

         ; EXEC library routines

         XLIB     CloseLibrary
         XLIB     OpenLibrary

         STRUCTURE StackFrame,0
         APTR     sf_MsgPtr            ; message pointer
         APTR     sf_Save1             ; 1st error trap
         APTR     sf_Save2             ; 2nd error trap
         LABEL    sf_SIZEOF            ; size: 12 bytes


* Checks whether a message came from a valid REXX context.
* Usage: boolean = CheckRexxMsg(rmptr);
_CheckRexxMsg
         move.l   4(sp),a0             ; message packet

* ========================     CheckRexxMsg     ========================
* Verifies that the message represents a valid REXX context.
* Registers:   A0 -- message
* Return:      D0 -- boolean
CheckRexxMsg
         movem.l  d2/a2/a6,-(sp)
         movea.l  a0,a2
         movea.l  _AbsExecBase,a6      ; EXEC library base

         ; Open the REXX Systems library

         lea      RXSLib(pc),a1        ; library name
         moveq    #0,d0                ; any version
         CALLSYS  OpenLibrary
         move.l   d0,d2                ; library opened?
         beq.s    1$                   ; no??

         ; Close the library ...

         movea.l  d0,a1
         CALLSYS  CloseLibrary

         ; Make sure the library matches the message LibBase pointer ...

         moveq    #0,d0                ; clear return
         cmp.l    rm_LibBase(a2),d2    ; matches?
         bne.s    1$                   ; no
         move.l   rm_TaskBlock(a2),d1  ; global pointer?
         beq.s    1$                   ; no

         ; Make sure the message came from REXX ...

         movea.l  a2,a0                ; message
         movea.l  d2,a6                ; REXX base
         CALLSYS  IsRexxMsg            ; D0=boolean

1$:      tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2/a2/a6
         rts

* Retrieves the value of a variable from the current storage environment.
* USAGE: error = GetRexxVar(msgptr,name,&return);
_GetRexxVar
         movem.l  4(sp),a0/a1          ; message/variable name
         bsr.s    GetRexxVar           ; D0=error A1=value
         bne.s    1$                   ; ... error
         movea.l  12(sp),a0            ; return pointer
         move.l   a1,(a0)              ; install value

1$:      rts

* =========================     GetRexxVar     =========================
* Retrieves the value of a variable from the current storage environment.
* Registers:   A0 -- context
*              A1 -- variable name
* Return:      D0 -- error code
*              A1 -- value (argstring)
GetRexxVar
         movem.l  d2/d3/a2-a6,-(sp)
         movea.l  a0,a2                ; save message
         movea.l  a1,a3                ; save buffer

         ; Check for a valid context

         bsr      CheckRexxMsg         ; D0=boolean
         beq.s    GRVErr10             ; invalid context
         movea.l  rm_LibBase(a2),a6    ; REXX base

         ; Find the current storage environment

         movea.l  rm_TaskBlock(a2),a0  ; global pointer
         CALLSYS  CurrentEnv           ; D0=A0=environment
         movea.l  a0,a4                ; save it

         ; Create the stem and compound parts

         movea.l  a3,a0                ; name
         bsr      TypeString           ; D0=error D1=compound A1=stem
         bne.s    GRVOut               ; ... failure
         movea.l  a1,a2                ; save stem
         move.l   d1,d2                ; save compound

         ; Look up the value ...

         movea.l  a4,a0                ; environment
         move.l   d2,d0                ; compound string
         moveq    #0,d1                ; clear node
         CALLSYS  FetchValue           ; D0=node  D1=flag  A1=value
         moveq    #0,d0                ; all OK

         ; Check for a literal value (and return NULL)

         addq.l   #ns_Buff,a1          ; offset to string
         tst.l    d1                   ; a literal?
         beq.s    GRVOut               ; no
         suba.l   a1,a1                ; clear pointer
         bra.s    GRVOut

         ; Error conditions

GRVErr10 moveq    #10,d0               ; invalid context

GRVOut   tst.l    d0                   ; set CCR
         movem.l  (sp)+,d2/d3/a2-a6
         rts

* USAGE: error = SetRexxVar(message,name,value,length)
_SetRexxVar
         movem.l  4(sp),a0/a1
         movem.l  12(sp),d0/d1

* =========================     SetRexxVar     =========================
* Assigns a value to a variable in the current storage environment.
* Registers:   A0 -- context
*              A1 -- variable name
*              D0 -- value
*              D1 -- length
* Return:      D0 -- error code
STACKBF  SET      sf_SIZEOF
SetRexxVar
         movem.l  d2-d7/a2-a6,-(sp)
         lea      -STACKBF(sp),sp      ; stack frame
         movea.l  a0,a2                ; save message
         movea.l  a1,a3                ; save name
         movea.l  d0,a5                ; save value
         move.l   d1,d3                ; save length

         ; Install our own error trap

         lea      SRVErr3(pc),a0       ; trap location
         movea.l  sp,a1                ; stack frame
         bsr      SaveTrap

         ; Check for a valid context

         movea.l  a2,a0                ; message packet
         bsr      CheckRexxMsg         ; D0=boolean
         beq.s    SRVErr10             ; invalid context
         movea.l  rm_LibBase(a2),a6    ; REXX base

         ; Make sure the value string is not too long

         moveq    #9,d0                ; string too long
         cmpi.l   #65535,d3            ; too long?
         bgt.s    SRVOut               ; yes

         ; Find the current storage environment

         movea.l  rm_TaskBlock(a2),a0  ; global pointer
         CALLSYS  CurrentEnv           ; D0=A0=environment
         movea.l  a0,a4                ; save environment

         ; Create the stem and compound parts ...

         movea.l  a3,a0                ; variable name
         bsr      TypeString           ; D0=error D1=compound A1=stem
         bne.s    SRVOut               ; ... failure
         movea.l  a1,a2                ; save stem
         move.l   d1,d2                ; save compound

         ; Locate or create the symbol node ...

         movea.l  a4,a0                ; environment
         move.l   d2,d0                ; compound string
         CALLSYS  EnterSymbol          ; D0=A0=node A1=value
         move.l   d0,d4                ; save node

         ; Create the value string ...

         movea.l  a4,a0                ; environment
         movea.l  a5,a1                ; pointer
         move.l   d3,d0                ; length
         bsr      MakeString           ; D0=A0=string
         beq.s    SRVErr3              ; ... failure

         ; Install the value string

         movea.l  a4,a0                ; environment
         movea.l  d0,a1                ; value string
         move.l   d4,d0                ; symbol table node
         CALLSYS  SetValue             ; D0=node A1=value

         moveq    #0,d0
         bra.s    SRVOut

         ; Error conditions

SRVErr3  moveq    #3,d0                ; allocation failure
         bra.s    SRVOut

SRVErr10 moveq    #10,d0               ; invalid context

SRVOut   movea.l  sp,a0                ; stack level
         move.l   d0,-(sp)             ; push error
         bsr      RestoreTrap
         move.l   (sp)+,d0             ; pop error

         lea      STACKBF(sp),sp
         movem.l  (sp)+,d2-d7/a2-a6
         rts

* =========================     TypeString     =========================
* Classifies a symbol and returns the stem and compound parts.
* Registers:   A0 -- variable name
*              A4 -- environment
* Return:      D0 -- error code
*              D1 -- compound name
*              A1 -- stem name
TypeString
         movem.l  d2/d3/a2/a3,-(sp)
         moveq    #0,d2                ; clear error
         moveq    #0,d3

         CALLSYS  Strlen               ; D0=length
         movea.l  a0,a1
         movea.l  a4,a0                ; environment
         bsr.s    MakeString           ; D0=A0=string
         movea.l  d0,a2                ; save string
         beq.s    TSErr3               ; failure ...

         ; Now check whether it looks like a stem ...

         lea      ns_Buff(a2),a1       ; buffer area
         move.w   ns_Length(a2),d0     ; length
         move.l   a1,d1

1$:      cmpi.b   #'.',(a1)+           ; a period?
         dbeq     d0,1$                ; loop back
         bne.s    2$                   ; not compound

         exg      d1,a1                ; begin=>A1 , end=>D1
         sub.l    a1,d1                ; stem length
         move.l   a2,d3                ; compound part

         ; Create the stem string ...

         movea.l  a4,a0                ; environment
         move.l   d1,d0                ; length
         bsr.s    MakeString           ; D0=A0=string
         movea.l  d0,a2                ; save it
         beq.s    TSErr3               ; failure

         ; Check for a valid stem ...

2$:      lea      ns_Buff(a2),a0       ; string pointer
         CALLSYS  IsSymbol             ; D0=code D1=length
         cmp.w    ns_Length(a2),d1     ; full length?
         beq.s    TSOut                ; yes

         moveq    #40,d2               ; variable expected
         bra.s    TSOut

TSErr3   moveq    #3,d2                ; allocation failure

TSOut    tst.l    d2                   ; error?
         beq.s    1$                   ; no ... all OK

         ; Release intermediate strings ...

         movea.l  a4,a0                ; environment
         movea.l  a2,a1                ; stem part
         bsr.s    FreeString           ; release it

         movea.l  a4,a0                ; environment
         movea.l  d3,a1                ; compound part
         bsr.s    FreeString           ; release it

1$:      movea.l  a2,a1                ; stem return
         move.l   d3,d1                ; compound return
         move.l   d2,d0                ; set CCR
         movem.l  (sp)+,d2/d3/a2/a3
         rts

* =========================     MakeString     =========================
* Allocates and initializes a string structure.
* Registers:   A0 -- environment
*              A1 -- string
*              D0 -- length
* Return:      D0 -- string structure
*              A0 -- same
MakeString
         movem.l  d0/a1,-(sp)          ; push length/pointer
         addq.l   #ns_Buff,d0          ; add offset
         addq.l   #1,d0                ; plus null byte
         CALLSYS  GetSpace             ; D0=A0=structure
         movem.l  (sp)+,d0/a1          ; pop length/pointer
         beq.s    1$                   ; failure ...

         move.l   a0,-(sp)             ; push pointer
         clr.l    (a0)
         move.w   d0,ns_Length(a0)
         move.b   #NSF_STRING,ns_Flags(a0)
         clr.b    ns_Buff(a0,d0.l)     ; null byte
         addq.l   #ns_Buff,a0          ; offset to buffer
         CALLSYS  StrcpyN              ; D0=hash
         movea.l  (sp)+,a0             ; pop pointer
         move.b   d0,ns_Hash(a0)       ; install hash byte

1$:      move.l   a0,d0                ; set CCR
         rts

* ========================     FreeString     ==========================
* Releases a string structure, if it's not owned at the time.
* Registers:   A0 -- environment
*              A1 -- string structure
FreeString
         move.l   a1,d1
         beq.s    1$
         moveq    #NSF_KEEP,d0         ; ownership bits
         and.b    ns_Flags(a1),d0      ; owned?
         bne.s    1$                   ; yes

         move.w   ns_Length(a1),d0     ; string length
         addq.l   #ns_Buff,d0          ; add offset
         addq.l   #1,d0                ; plus null byte
         CALLSYS  FreeSpace

1$:      rts

* ==========================     SaveTrap     ==========================
* Saves the global error code
* Registers:   A0 -- trap location
*              A1 -- stack level
*              A2 -- message packet
SaveTrap
         move.l   a3,-(sp)
         movea.l  rm_TaskBlock(a2),a3  ; global context

         move.l   a2,sf_MsgPtr(a1)
         movem.l  rt_ErrTrap(a3),d0/d1 ; old trap
         movem.l  a0/a1,rt_ErrTrap(a3) ; new trap
         movem.l  d0/d1,sf_Save1(a1)   ; save old trap

         movea.l  (sp)+,a3
         rts

* =========================     RestoreTrap     ========================
* Restores the global error trap values for the REXX context.
* Registers:   A0 -- stack frame
RestoreTrap
         movea.l  sf_MsgPtr(a0),a1
         movea.l  rm_TaskBlock(a1),a1  ; global context

         movem.l  sf_Save1(a0),d0/d1   ; saved trap values
         movem.l  d0/d1,rt_ErrTrap(a1) ; restore values
         rts

         ; String data

RXSLib   RXSLIBNAME                    ; library name
         CNOP     0,2

         END
