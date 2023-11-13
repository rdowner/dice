*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DiM ram interface
        XREF    _None
        SECTION alib
        XDEF    _DiM
_DiM:
                movem.l a2/a6,-(a7)
                move.l  _None,a6
                move.l  12(a7),a2
                move.l  16(a7),a1
                jsr     -216(a6)
                movem.l (a7)+,a2/a6
                rts
        END
