*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** getchar ram interface
        XREF    _None
        SECTION alib_stdio
        XDEF    _getchar
_getchar:
                move.l  a6,-(a7)
                move.l  _None,a6
                jsr     -54(a6)
                move.l  (a7)+,a6
                rts
        END
