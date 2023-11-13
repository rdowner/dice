*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DoMethod ram interface
        XREF    _None
        SECTION alib
        XDEF    _DoMethod
_DoMethod:
                movem.l a2/a6,-(a7)
                move.l  _None,a6
                lea     12(a7),a1
                move.l  (a1)+,a2
                jsr     -252(a6)
                movem.l (a7)+,a2/a6
                rts
        END
