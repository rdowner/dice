*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** TimeDelay ram interface
        XREF    _None
        SECTION alib
        XDEF    _TimeDelay
_TimeDelay:
                movem.l d2/a6,-(a7)
                move.l  _None,a6
                movem.l 12(a7),d0/d1/d2
                jsr     -174(a6)
                movem.l (a7)+,d2/a6
                rts
        END
