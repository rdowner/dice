**
**      $Id: SearchRefs.asm,v 30.0 1994/06/10 18:05:50 dice Exp $
**
**      Search loaded reference file for reference.  Search is "best match"
**      for case: an exact match will return instantly.  An inexact match
**      will scan to the end looking for a better option.
**
**          :TODO: Investigate full international case sensitivity issues.
**                 We already do 90%.
**
**              Disallow tabs in search string
**
**              Could do "fast path" search of first char.  25 in 26 would
**              hit it.  Or maybe a BCPL-ish string length.
**
**      (C) Copyright 1992, Obvious Implementations, All Rights Reserved
**

                XDEF    _searchMe
                XDEF    _convertFile
                XDEF    _nextLine

LF              EQU     10
TAB             EQU     9
FF              EQU     12

SKIPNL          MACRO   \1
LP\@            tst.b   (\1)+
                bne.s   LP\@
                ENDM

        section text,code

;
; FUNCTION
;       Return a newline separated file, line-by-line.
;
; INPUTS
;       4(SP)  -Pointer to current index
;
; RESULTS
;       D0     -Length of current line.  Zero for end
;
_nextLine:      moveq   #FF+1,d1        ;Pass anything numerically above FF without comment
                move.l  4(sp),a0
                move.l  a0,a1

nl_loop:        cmp.b   (a1)+,d1
                bcs.s   nl_loop
                move.b  -1(a1),d0
                beq.s   nl_end
                cmp.b   #FF,d0
                bne.s   nl_noff
                move.b  #LF,-1(a1)      ;Replace FF with LF
nl_noff:        cmp.b   #LF,d0
                bne.s   nl_loop

                suba.l  a0,a1
                move.l  a1,d0
                rts                     ;exit _nextLine

nl_end:         subq.l  #1,a1           ;Don't write NULL
                suba.l  a0,a1
                move.l  a1,d0
                rts                     ;exit _nextLine

;
; INPUTS
;       4(SP)   File pointer
;       8(SP)   Length.  Will stick two NULLs *after* the end
;
_convertFile:   move.l  4(sp),a0
                move.l  8(sp),d0
                moveq   #LF,d1          ;Convert newlines to nulls

cf_lp:          cmp.b   (a0)+,d1
                bne.s   cf_nolf
                clr.b   -1(a0)
cf_nolf:        subq.l  #1,d0
                bne.s   cf_lp

                clr.b   (a0)+
                clr.b   (a0)+
                rts


;
; INPUTS
;       D0 - Length of string array. Last entry *must* be LF terminated.
;       A0 - Pointer to array of strings
;       A1 - Search string
;       A2 - Address of pointer to current filename string, filled in.
;
; RESULTS
;       D0 - Pointer to proper string
;      &A2 - Last matched filename
;
; REGISTERS
;      &A2 - Last matched filename
;       A3 - Current index
;       A5 - Search string
;       D2 - scratch
;       D3 - <tab>
;       D4 - Last matched name pointer
;       D5 - Last matched Inacuracy index
;       D6 - Current filename
;       D7 - #$20 for case-match
;
;
;       char * searchMe(char *strings,char *searchfor,&filename,ULONG length);
STACKOFFSET     EQU     11*4
_searchMe:      movem.l d2-d7/a2-a6,-(sp)

;---------------Register setup
                moveq   #TAB,D3         ;<tab> for quick searches
                moveq   #0,d4           ;Matched entry
                moveq   #-1,d5          ;Infinite inaccuary index
                moveq   #0,d6           ;In case no filenames found...
                moveq   #~($20),d7         ;Setup for case search

                move.l  STACKOFFSET+4(sp),a3    ;Main index
                move.l  STACKOFFSET+8(sp),a5    ;Search string
                move.l  STACKOFFSET+12(sp),a2   ;&filename pointer
                clr.l   (a2)                    ;No filenmae yet

                move.l  a5,d0                   ;Test for NULL search string
                beq     sm_alldone

;---------------Main loop
                bra.s   sm_skipSKIPNL
sm_newname:     move.l  a3,d6           ;Set new name pointer
sm_toploop:     SKIPNL  a3
sm_skipSKIPNL:  move.b  (a3),d2
                beq.s   sm_alldone      ;Double NULL indicates end...
                cmp.b   #'~',d2         ;Check for ~ prefex
                beq.s   sm_newname

                ; Compare current string, case insensitive.  Each character
                ; of a case-insensitive match adds 1 to the inaccuracy index.
                ; A zero index ends the search, else the best is used.
                ;
                ; A simple folding search is used; while this results in
                ; some false matches, such a y-umlaut to sharffe-S, we don't
                ; really care.  Any match is better than no match, and the
                ; inaccuracy index will weed out the better choice anyway.
                ;
sm_notaname:    move.l  a5,a0           ;Search string
                move.l  a3,a1           ;Stash current index, in case of match
                moveq   #0,d1           ;Zero inaccuary index

sm_nextchar:    move.b  (a0)+,d0
                beq.s   sm_endsource    ;End of search string...
                move.b  (a3)+,d2
                cmp.b   d3,d2
                beq.s   sm_endcandid    ;End of candidate string....
                cmp.b   d2,d0
                beq.s   sm_nextchar     ;Match!
                addq.w  #1,d1           ;Increase inaccuracy
                and.b   d7,d0           ;Simple folding search
                and.b   d7,d2           ;Simple folding search
                cmp.b   d2,d0
                beq.s   sm_nextchar     ;Match!
                bra.s   sm_toploop      ;We struck out...

                ; Source string has ended.  Count inaccuacy of 2 for each
                ; remaining character in the candidate string.
                ;
sm_endsource:   cmp.b   (a3)+,d3
                beq.s   sm_endmatch
                addq.w  #2,d1           ;Increase inaccuracy
                bra.s   sm_endsource

                ; Candidate string has ended.  Count inaccuacy of 2 for each
                ; remaining character in the source string.
                ;
sm_endcandid:   addq.w  #2,d1           ;Increase inaccuracy
                tst.b   (a0)+
                bne.s   sm_endcandid

                ; We have a match.  Update pointers if better than our last
                ; match.
                ;
sm_endmatch:    cmp.l   d5,d1           ;Current inaccuary index to old
                bcc.s   sm_toploop      ;Keep searching for perfection...
                move.l  d6,(a2)         ;Got one!  Stash filename.
                move.l  a1,d4           ;Got one!  Stash pointer.
                move.l  d1,d5           ;Got one!  Stash inaccuracy index.
                bne.s   sm_toploop      ;Keep searching for perfection...


                ; All done.  At this point we have nothing, a perfect match
                ; or a partial match.  Tell the world.
                ;
sm_alldone:     move.l  d4,d0           ;Return what we found
                ;[&A2 - pointer to current filename]

                movem.l (sp)+,d2-d7/a2-a6
                rts
                end
