
*        TTL       FAST FLOATING POINT FLOAT TO ASCII (FFPFPA)
***************************************
* (C) COPYRIGHT 1980 BY MOTOROLA INC. *
***************************************

******************************************************
*                     FFPFPA                         *
*                 FLOAT TO ASCII                     *
*                                                    *
*    INPUT:  D7 - FLOATING POINT NUMBER              *
*                                                    *
*    OUTPUT: D7 - THE BASE TEN EXPONENT IN BINARY    *
*                 FOR THE RETURNED FORMAT            *
*            SP - DECREMENTED BY 14 AND              *
*                 POINTING TO THE CONVERTED          *
*                 NUMBER IN ASCII FORMAT             *
*                                                    *
*            ALL OTHER REGISTERS UNAFFECTED          *
*                                                    *
*    CONDITION CODES:                                *
*            N - SET IF THE RESULT IS NEGATIVE       *
*            Z - SET IF THE RESULT IS ZERO           *
*            V - CLEARED                             *
*            C - CLEARED                             *
*            X - UNDEFINED                           *
*                                                    *
*   CODE SIZE: 192 BYTES   STACK WORK AREA: 42 BYTES *
*                                                    *
*                                                    *
*            {S}{'.'}{DDDDDDDD}{'E'}{S}{DD}          *
*            <     FRACTION   >< EXPONENT >          *
*                                                    *
*        WHERE  S - SIGN OF MANTISSA OR EXPONENT     *
*                   ('+' OR '-')                     *
*               D - DECIMAL DIGIT                    *
*                                                    *
*        STACK OFFSET OF RESULT  S.DDDDDDDDESDD      *
*        AFTER RETURN            00000000001111      *
*                                01234567890123      *
*                                                    *
*                                                    *
*        EXAMPLES   +.12000000E+03  120              *
*                   +.31415927E+01  PI               *
*                   +.10000000E-01  ONE-HUNDREDTH    *
*                   -.12000000E+03  MINUS 120        *
*                                                    *
*     NOTES:                                         *
*       1) THE BINARY BASE 10 EXPONENT IS RETURNED   *
*          IN D7 TO FACILITATE CONVERSIONS TO        *
*          OTHER FORMATS.                            *
*       2) EVEN THOUGH EIGHT DIGITS ARE RETURNED,THE *
*          PRECISION AVAILABLE IS ONLY 7.167 DIGITS. *
*          ROUNDING SHOULD BE PERFORMED WHEN LESS    *
*          THAN EIGHT DIGITS ARE ACTUALLY UTILIZED   *
*          IN THE MANTISSA.                          *
*       3) THE STACK IS LOWERED BY 14 BYTES BY THIS  *
*          ROUTINE.  THE RETURN ADDRESS IS REPLACED  *
*          BY A PORTION OF THE RESULTS               *
*                                                    *
*  TIME: (8MHZ NO WAIT STATES ASSUMED)               *
*        330 MICROSECONDS CONVERTING THE SAMPLE      *
*        VALUE OF 55.55 TO ASCII.                    *
*                                                    *
******************************************************
         ;PAGE
*FFPFPA   IDNT      1,1  FFP FLOAT TO ASCII

*         OPT       PCS

         XREF      FFP10TBL  ;POWER OF TEN TABLE

         XDEF      FFPFPA    ;ENTRY POINT

* STACK DEFINITION
STKOLD   EQU       48       ;PREVIOUS CALLERS STACK POINTER
STKEXP   EQU       46       ;EXPONENT
STKEXPS  EQU       45       ;EXPONENTS SIGN
STKLTRE  EQU       44       ;'E'
STKMANT  EQU       36       ;MANTISSA
STKPER   EQU       35       ;'.'
STKMANS  EQU       34       ;MANTISSA'S SIGN
STKNEWRT EQU       30       ;NEW RETURN POSITION
STKRTCC  EQU       28       ;RETURN CONDITION CODE
STKSAVE  EQU       0        ;REGISTER SAVE AREA


FFPFPA:
                 LEA       -10(SP),SP                   ;SET STACK TO NEW LOCATION
         MOVE.L    10(SP),-(SP)                 ;SAVE RETURN
         MOVE.W    #0,-(SP)                             ;SAVE FOR RETURN CODE
         TST.B     D7                                   ;TEST VALUE
*        MOVE.W    SR,-(SP)                             ;SAVE FOR RETURN CODE (NOTE: COMMENTED OUT)
         MOVEM.L   D2-D6/A0/A1,-(SP)    ;SAVE REGISTERS

* ADJUST FOR ZERO VALUE
         BNE.S     FPFNOT0                              ;BRANCH NO ZERO INPUT
         MOVEQ     #$41,D7                              ;SETUP PSUEDO INTEGER EXPONENT

* SETUP MANTISSA'S SIGN
FPFNOT0  MOVE.W    #'+.',STKMANS(SP)    ;INSERT PLUS AND DECIMAL
         MOVE.B    D7,D6                                ;COPY SIGN+EXPONENT
         BPL.S     FPFPLS                               ;BRANCH IF PLUS
         ADDQ.B    #2,STKMANS(SP)               ;CHANGE PLUS TO MINUS

* START SEARCH FOR MAGNITUDE IN BASE 10 POWER TABLE
FPFPLS   ADD.B     D6,D6     ;SIGN OUT OF PICTURE
         MOVE.B    #$80,D7   ;SET ROUDING FACTOR FOR SEARCH
         EOR.B     D7,D6     ;CONVERT EXPONENT TO BINARY
         EXT.W     D6        ;EXPONENT TO WORD
         ASR.W     #1,D6     ;BACK FROM SIGN EXTRACTMENT
         MOVEQ     #1,D3     ;START BASE TEN COMPUTATION
         LEA       FFP10TBL,A0 ;START AT TEN TO THE ZERO
         CMP.W     (A0),D6   ;COMPARE TO TABLE
         BLT.S     FPFMIN    ;BRANCH MINUS EXPONENT
         BGT.S     FPFPLU    ;BRANCH PLUS EXPONENT
FPFEQE   CMP.L     2(A0),D7  ;EQUAL SO COMPARE MANTISSA'S
         BCC.S     FPFFND    ;BRANCH IF INPUT GREATER OR EQUAL THAN TABLE
FPFBCK   ADDQ.W    #6,A0     ;TO NEXT LOWER ENTRY IN TABLE
         SUBQ.W    #1,D3     ;DECREMENT BASE 10 EXPONENT
         BRA.S     FPFFND    ;BRANCH POWER OF TEN FOUND

* EXPONENT IS HIGHER THAN TABLE
FPFPLU   LEA       -6(A0),A0 ;TO NEXT HIGHER ENTRY
         ADDQ.W    #1,D3     ;INCREMENT POWER OF TEN
         CMP.W     (A0),D6   ;TEST NEW MAGNITUDE
         BGT.S     FPFPLU    ;LOOP IF STILL GREATER
         BEQ.S     FPFEQE    ;BRANCH EQUAL EXPONENT
         BRA.S     FPFBCK    ;BACK TO LOWER AND FOUND

* EXPONENT IS LOWER THAN TABLE
FPFMIN   LEA       6(A0),A0  ;TO NEXT LOWER ENTRY
         SUBQ.W    #1,D3     ;DECREMENT POWER OF TEN Y ONE
         CMP.W     (A0),D6   ;TEST NEW MAGNITUDE
         BLT.S     FPFMIN    ;LOOP IF STILL LESS THAN
         BEQ.S     FPFEQE    ;BRANCH EQUAL EXPONENT

* CONVERT THE EXPONENT TO ASCII
FPFFND   MOVE.L    #'E+00',STKLTRE(SP) ;SETUP EXPONENT PATTERN
         MOVE.W    D3,D2     ;? EXPONENT POSITIVE
         BPL.S     FPFPEX    ;BRANCH IF SO
         NEG.W     D2        ;ABSOLUTIZE
         ADDQ.B    #2,STKEXPS(SP) ;TURN TO MINUS SIGN
FPFPEX   CMP.W     #10,D2    ;? TEN OR GREATER
         BCS.S     FPFGEN    ;BRANCH IF NOT
         ADDQ.B    #1,STKEXP(SP) ;CHANGE ZERO TO ONE
         SUB.W     #10,D2    ;ADJUST TO DECIMAL
FPFGEN   OR.B      D2,STKEXP+1(SP) ;FILL IN LOW DIGIT

* GENERATE THE MANTISSA IN ASCII A0->TABLE  D7=BINARY MANTISSA
* D5 - MANTISSA FROM TABLE       D6.W = BINARY EXPONENT
* D4 - SHIFT AND DIGIT BUILDER   D2 = DBRA MANTISSA DIGIT COUNT
* A1->MANTISSA STACK POSITION
         MOVEQ     #7,D2     ;COUNT FOR EIGHT DIGITS
         LEA       STKMANT(SP),A1 ;POINT TO MANTISSA START
         TST.L     D7        ;? ZERO TO CONVERT
         BPL.S     FPFZRO    ;BRANCH IF SO TO NOT ROUND
         TST.B     5(A0)     ;? 24 BIT PRECISE IN TABLE
         BNE.S     FPFNXI    ;BRANCH IF NO TRAILING ZEROES
FPFZRO   CLR.B     D7        ;CLEAR ADJUSTMENT FOR .5 LSB PRECISION
FPFNXI   MOVE.W    D6,D4     ;COPY BINARY EXPONENT
         SUB.W     (A0)+,D4  ;FIND NORMALIZATION FACTOR
         MOVE.L    (A0)+,D5  ;LOAD MANTISSA FROM TABLE
         LSR.L     D4,D5     ;ADJUST TO SAME EXPONENT
         MOVEQ     #9,D4     ;START AT NINE AND COUNT DOWN
FPFINC   SUB.L     D5,D7     ;SUBTRACT FOR ANOTHER COUNT
         DBCS      D4,FPFINC ;DECREMENT AND BRANCH IF OVER
         BCS.S     FPFNIM    ;BRANCH NO IMPRECISION
         CLR.B     D4        ;CORRECT RARE UNDERFLOW DUE TO TABLE IMPRECISION
FPFNIM   ADD.L     D5,D7     ;MAKE UP FOR OVER SUBTRACTION
         SUB.B     #9,D4     ;CORRECT VALUE
         NEG.B     D4        ;TO BETWEEN 0 AND 9 BINARY
         OR.B      #'0',D4   ;CONVERT TO ASCII
         MOVE.B    D4,(A1)+  ;INSERT INTO ASCII MANTISSA PATTERN
         DBF       D2,FPFNXI ;BRANCH IF MORE DIGITS TO GO

* RETURN WITH BASE TEN EXPONENT BINARY IN D7
         MOVE.W    D3,D7     ;TO D7
         EXT.L     D7        ;TO FULL WORD
         MOVEM.L   (SP)+,D2-D6/A0/A1 ;RESTORE REGISTERS
*        RTR                 ;RETURN WITH PROPER CONDITION CODE
         ADDQ.L    #2,SP
         RTS

         END
