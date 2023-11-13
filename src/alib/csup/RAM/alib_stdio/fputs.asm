*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fputs ram interface
        XREF    _None
        SECTION alib_stdio
        XDEF    _fputs
_fputs:
                move.l  a6,-(a7)
                move.l  _None,a6
                move.l  8(a7),a0
                move.l  12(a7),d0
                jsr     -48(a6)
                move.l  (a7)+,a6
                rts
        END
