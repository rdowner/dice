*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** sprintf ram interface
        XREF    _None
        SECTION alib_stdio
        XDEF    _sprintf
_sprintf:
                move.l  a6,-(a7)
                move.l  _None,a6
                lea     8(a7),a1
                move.l  (a1)+,a0
                jsr     -24(a6)
                move.l  (a7)+,a6
                rts
        END
