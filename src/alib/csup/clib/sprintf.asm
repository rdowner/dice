
        section romhunks

        xref    _AbsExecBase
        xref    _LVORawDoFmt


************************************************************************
*
*       sprintf -- format a string into a string buffer
*
************************************************************************
        xdef    _sprintf
_sprintf:       ; ( string, format, {values} )
                movem.l a2/a3/a4/a6,-(sp)
                move.l  5*4(sp),a3

        ;------ format the string:
                move.l  6*4(sp),a0
                lea     7*4(sp),a1
                lea     stuffChar(pc),a2
                move.l  _AbsExecBase,a6
                jsr     _LVORawDoFmt(a6)

                movem.l (sp)+,a2/a3/a4/a6
                rts

;------ putchar function used by DoFmt: --------------------------------
stuffChar:
                move.b  d0,(a3)+
                rts

        END
