*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeletePort ram interface
        XREF    _None
        SECTION alib
        XDEF    _DeletePort
_DeletePort:
                move.l  a6,-(a7)
                move.l  _None,a6
                move.l  8(a7),a0
                jsr     -66(a6)
                move.l  (a7)+,a6
                rts
        END
