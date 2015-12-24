
        section romhunks
        xref    _Write
        xref    _AbsExecBase
        xref    _LVORawDoFmt
        xref    _stdout



LF      equ     10
CR      equ     13

************************************************************************
*
*       doprnt -- format a string to an output stream
*
************************************************************************
        xdef    __doprnt
__doprnt:       ; ( stream, format, values)
                movem.l a2/a3/a4/a6,-(sp)
                move.l  5*4(sp),a4

        ;------ format the string:
                move.l  6*4(sp),a0
                move.l  7*4(sp),a1
                lea     stuffChar(pc),a2
                lea     -140(sp),sp     ; local string buffer
                move.l  sp,a3
                move.l  _AbsExecBase,a6
                jsr     _LVORawDoFmt(a6)

        ;------ find end of formatted string:
                moveq.l #-1,d0
ps_size:        tst.b   (a3)+
                dbeq    d0,ps_size
                not.l   d0
                beq.s   ps_empty

        ;------ write the formatted string:
ps_common:
                move.l  d0,-(sp)
                pea     4(sp)
                pea     (a4)
                jsr     _Write  ;(stream, &string, size)
                lea     12(sp),sp
ps_empty:
                lea     140(sp),sp
                movem.l (sp)+,a2/a3/a4/a6
                rts

;------ putchar function used by DoFmt: --------------------------------
stuffChar:
*               cmp.b   #LF,d0
*               bne.s   sc_put
*               move.b  #CR,(a3)+
sc_put:
                move.b  d0,(a3)+
                rts

        END
