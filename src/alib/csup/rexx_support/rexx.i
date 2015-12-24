* === rexx/rexx.i ======================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Main include file for Rexx interpreter.  Includes definitions for data
* structures and flags used for parsing and execution.

         IFND     REXX_REXX_I
REXX_REXX_I       EQU   1

         IFND     REXX_STORAGE_I
         INCLUDE "rexx/storage.i"
         ENDC

* Limit constants
TEMPBUFF    EQU   1000                 ; initial size of global work buffer
MAXLEN      EQU   65535                ; maximum length of strings
MAXFNLEN    EQU   64                   ; maximum filing system name length
RDTEST      EQU   80                   ; characters to read to test a file

ENV_MAX_DIGITS EQU 14                  ; maximum numeric digits

* Character definitions
EOFCHAR     EQU   -1                   ; EOF flag
NULL        EQU   0           
NEWLINE     EQU   $0A                  ; 'newline' character
BLANK       EQU   ' '                  ; ASCII blank
DQUOTE      EQU   '"'                  ; ASCII double quote
QUOTE       EQU   $27                  ; ASCII single quote
PERIOD      EQU   '.'                  ; ASCII period
                              
LOWERBIT    EQU   5                    ; ASCII lowercase bit
ASCIIBIT    EQU   7                    ; non-ASCII bit (sign bit)

* Special character codes
SPC_COMMA   EQU   ','
SPC_SEMI    EQU   ';'
SPC_COLON   EQU   ':'
SPC_OPEN    EQU   '('
SPC_CLOSE   EQU   ')'

* String types for classification
STR_ASCII   EQU   1                    ; ASCII string
STR_HEX     EQU   2                    ; HEX digit string
STR_BINARY  EQU   3                    ; BINARY digit string

         ; The Token structure ...

         STRUCTURE Token,0
         LONG     t_Succ               ; successor token
         LONG     t_Pred
         UBYTE    t_Type               ; token type
         UBYTE    t_Flags              ; attribute flag bits
         UWORD    t_Offset             ; position or keyword code
         LONG     t_Data               ; opcode or string structure
         LABEL    t_SIZEOF             ; size: 16 bytes

* Defined fields
TOFFSET  EQU      t_Offset             ; offset in clause
TKEYCODE EQU      t_Offset             ; keyword code (2-byte)
TDATA    EQU      t_Data
TSTRING  EQU      t_Data               ; string structure (4-byte)
TOPCODE  EQU      t_Data+3             ; opcode (1-byte)

* Token type definitions ...
TTB_SYMBOL  EQU   4                    ; symbol token type bit
TTF_SYMBOL  EQU   1<<TTB_SYMBOL

T_STRING    EQU   1                    ; String Token
T_OPERATOR  EQU   2                    ; Operator Token
T_COMMA     EQU   3                    ; Special Character Token: ','
T_LABEL     EQU   4                    ; Special Character Token: ':'
T_OPEN      EQU   5                    ; Special Character Token: '('
T_CLOSE     EQU   6                    ; Special Character Token: ')'
T_END       EQU   7                    ; Special Character Token: ';'
T_TERM      EQU   8                    ; expression terminator
T_FUNCTION  EQU   9                    ; function awaiting evaluation
T_SYMFIXED  EQU   TTF_SYMBOL!12        ; fixed symbol
T_SYMVAR    EQU   TTF_SYMBOL!13        ; variable symbol
T_SYMCMPD   EQU   TTF_SYMBOL!14        ; compound symbol
T_SYMSTEM   EQU   TTF_SYMBOL!15        ; stem symbol

* TSTMASK is a bitmask indicating whether a token may carry a string object.
* If it can, then the bit corresponding to the token type is set.
TSTMASK     EQU   (1<<1)!(1<<7)!(1<<8)!(1<<28)!(1<<29)!(1<<30)!(1<<31)

* Token flag bit definitions
TFB_FUNCDEF EQU   0                    ; token defines a function?
TFB_THEN    EQU   1                    ; 'THEN' symbol?
TFB_BLANK   EQU   2                    ; blank (operator) token?
TFB_QUOTED  EQU   3                    ; quoted argument string?
TFB_EQUALS  EQU   4                    ; '=' sign?
TFB_PERIOD  EQU   5                    ; '.' symbol?
TFB_ASSIGN  EQU   6                    ; an assignment symbol?
TFB_NOSTR   EQU   7                    ; don't recycle string?

         ; Clause structure ...

         STRUCTURE Clause,0
         APTR     c_Succ               ; successor
         APTR     c_Pred               ; predecessor
         UBYTE    c_Type               ; clause classification
         UBYTE    c_Flags              ; attribute bits
         UWORD    c_Count              ; number of tokens
         APTR     c_Link               ; token pointer

         UWORD    c_SrcLine            ; source line number
         UWORD    c_Len                ; length of clause
         ULONG    c_Keys               ; instruction code
         ULONG    c_SrcPos             ; source position
         ULONG    c_NextPos            ; "next" clause position

         APTR     cTL_Head             ; first token (list header)
         APTR     cTL_Tail
         APTR     cTL_TailPred         ; last token
         APTR     c_Name               ; string structure
         LABEL    c_SIZEOF             ; size: 48 bytes

* Defined fields
CSECKEYS EQU      c_Keys+2
CLINK    EQU      c_Link

* Clause type codes
C_NULL      EQU   1                    ; Null Clause
C_LABEL     EQU   2                    ; Label Clause
C_ASSIGN    EQU   3                    ; Assignment Clause
C_INSTRUCT  EQU   4                    ; Instruction Clause
C_COMMAND   EQU   5                    ; Command Clause
C_ARGLIST   EQU   6                    ; argument list header
C_ERROR     EQU   $FF                  ; Error flag

* Clause attribute flag bit definitions
CFB_SYMBOL  EQU   0                    ; first token a symbol
CFB_SIMPLE  EQU   1                    ; expressions processed
CFB_NULLEXPR EQU  2                    ; expression was null
CFB_FINAL   EQU   3                    ; final scan
CFB_PARSED  EQU   4                    ; fully parsed
CFB_ANYFUNC EQU   5                    ; any functions?
CFB_CACHED  EQU   7                    ; clause cached

* The flag form of the clause attributes
CFF_SYMBOL  EQU   1<<CFB_SYMBOL
CFF_SIMPLE  EQU   1<<CFB_SIMPLE
CFF_NULLEXPR EQU  1<<CFB_NULLEXPR
CFF_FINAL   EQU   1<<CFB_FINAL
CFF_PARSED  EQU   1<<CFB_PARSED
CFF_ANYFUNC EQU   1<<CFB_ANYFUNC
CFF_CACHED  EQU   1<<CFB_CACHED

         ; The SourceSegment structure is used to maintain the Source Array,
         ; Labels Array, and Source Line Array.  The data fields may be
         ; either an integer value or a pointer to a string structure.
         ; The allocated size is (4 + sCount*4).

         STRUCTURE SourceSegment,0
         LONG     sCount               ; number of entries
         APTR     sSeg                 ; array of pointers to segments
         LABEL    s_SIZEOF             ; size: 8 bytes

SEGSHIFT EQU      16                   ; shift for segment
SEGMASK  EQU      $0000FFFF            ; mask for offset

         ; The control range structure

         STRUCTURE Range,0
         APTR     r_Succ               ; next range
         APTR     r_Pred
         UBYTE    r_Type               ; range type
         UBYTE    r_Flags              ; control flags ...
         UWORD    r_Pad
         LONG     r_Value              ; node pointer

         UWORD    r_Action             ; various uses ...
         UWORD    r_Test               ; test expression result
         LONG     r_Count              ; iteration count
         ULONG    r_SrcPos             ; source offset of activating line
         ULONG    r_NextPos            ; source position after range

         APTR     r_Segment            ; source segment
         APTR     r_Index              ; index variable or result
         APTR     r_TO                 ; limit expression result
         APTR     r_BY                 ; increment expression result
         LABEL    r_SIZEOF             ; size: 48 bytes

RCOMP    EQU      r_Action+1           ; comparison operator

* Control range type codes
NRANGE_DO      EQU   1                 ; 'DO' range
NRANGE_IF      EQU   2                 ; 'IF' range
NRANGE_SELECT  EQU   3                 ; 'SELECT' range
NRANGE_INTERP  EQU   4                 ; 'INTERPRET' instruction

* Control range flag bit definitions
NRFB_ACTIVE    EQU   0                 ; execute the range?
NRFB_INIT      EQU   1                 ; initialized yet?
NRFB_ITERATE   EQU   2                 ; iterative range?
NRFB_THEN      EQU   3                 ; 'THEN' clause?
NRFB_ELSE      EQU   4                 ; 'ELSE' or 'OTHERWISE' clause?
NRFB_DEBUG     EQU   5                 ; 'debug' mode?
NRFB_BRANCH    EQU   6                 ; branch taken?
NRFB_FINISH    EQU   7                 ; range finished?

* Control range flags
NRFF_ACTIVE    EQU   1<<NRFB_ACTIVE
NRFF_INIT      EQU   1<<NRFB_INIT
NRFF_ITERATE   EQU   1<<NRFB_ITERATE
NRFF_THEN      EQU   1<<NRFB_THEN
NRFF_ELSE      EQU   1<<NRFB_ELSE
NRFF_FINISH    EQU   1<<NRFB_FINISH

         ; The StacK structure is used to simulate stack operations,
         ; which actually use doubly-linked lists.

         STRUCTURE StacK,0
         APTR     skHead               ; first entry
         APTR     skTail
         APTR     skTailPred           ; last entry
         WORD     skPad                ; not used
         WORD     skNum                ; number of elements stacked
         LABEL    skSIZEOF             ; size: 16 bytes

STACKTOP EQU      skHead
STACKBOT EQU      skTailPred

         ENDC
