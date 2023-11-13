*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteExtIO ram interface
        XREF    _None
        SECTION alib
        XDEF    _DeleteExtIO
_DeleteExtIO:
                move.l  a6,-(a7)
                move.l  _None,a6
                move.l  8(a7),a0
                jsr     -60(a6)
                move.l  (a7)+,a6
                rts
        END
