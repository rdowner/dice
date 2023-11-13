*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CreateTask ram interface
        XREF    _None
        SECTION alib
        XDEF    _CreateTask
_CreateTask:
                move.l  a6,-(a7)
                move.l  _None,a6
                move.l  8(a7),a0
                movem.l 12(a7),d0/a1
                move.l  20(a7),d1
                jsr     -54(a6)
                move.l  (a7)+,a6
                rts
        END
