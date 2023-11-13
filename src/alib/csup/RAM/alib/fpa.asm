*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** fpa ram interface
        XREF    _None
        SECTION alib
        XDEF    _fpa
_fpa:
                move.l  a6,-(a7)
                move.l  _None,a6
                movem.l 8(a7),d0/a0
                jsr     -162(a6)
                move.l  (a7)+,a6
                rts
        END
