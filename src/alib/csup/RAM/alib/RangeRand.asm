*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** RangeRand ram interface
        XREF    _None
        SECTION alib
        XDEF    _RangeRand
_RangeRand:
                move.l  a6,-(a7)
                move.l  _None,a6
                move.l  8(a7),d0
                jsr     -120(a6)
                move.l  (a7)+,a6
                rts
        END
