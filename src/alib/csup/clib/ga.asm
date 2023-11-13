        section romhunks
        xdef    _ttskasm
_ttskasm:
*       move.w  #0x40,_dmaconw          * turn off blitter
        move.l  22(a1),a0               * get ccode ptr
        move.l  26(a1),-(sp)
        jsr     (a0)                    * call the routine
        addq.l  #4,sp
*       move.w  #0x8040,_dmaconw        * restart blitter
        moveq   #0,d0
        rts

        end
