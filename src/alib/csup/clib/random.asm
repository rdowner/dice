*******************************************************************************
*                                                                             *
* ranged random function -      generates random integer within a range       *
*                                                                             *
*       input:  range passed on top of stack (32 bit int) 1 <= range < 65536  *
*                                                                             *
*       output: random number returned on d0 (16 bit int) 0 <= range < range  *
*                                                                             *
*       effects:d0                                                            *
*                                                                             *
*******************************************************************************
        section romhunks
_RangeRan:
_RangeRand:
        movem.l d5/d4,-(sp)     * save registers
        move.w  14(sp),d5       * range in d5
        move.w  d5,d4           * range-1 in d4 for counting # of randomizes
        subq.w  #1,d4           *       log2(range-1) times
        move.l  _RangeSeed,d0   * seed in d0
*
L1:
*                               * randomizing loop:
        add.l   d0,d0           * shift seed left (add dn,dn is faster)
        bhi.s   L2              * carry? (using bhi instead of bcc unlocks 0s)
        eori.l  #$1D872B41,d0   * magic number for 32 bit seed
L2:
        lsr.w   #1,d4
        bne.s   L1
*
        move.l  d0,_RangeSeed   * save new seed
        tst.w   d5              * test for range == 0
        bne.s   L3
        swap    d0              * range == 0; return bits 0-15 of seed in d0
        bra.s   L4
L3:
        mulu    d5,d0           * range > 0; multiply bits 0-15 of seed (fract)
*        X range and return int part of product
L4:
        clr.w   d0
        swap    d0
        movem.l (sp)+,d5/d4     * restore registers
        rts
        xdef    _RangeRan,_RangeRand
*******************************************************************************
*                                                                             *
* fast random function -        generates fairly random integer quickly       *
*                                                                             *
*       input:  random seed passed on top of stack (32 bit int)               *
*                                                                             *
*       output: new random seed returned on d0 (32 bit int)                   *
*                                                                             *
*       effects:d0                                                            *
*                                                                             *
*******************************************************************************
_FastRand:
        move.l  4(sp),d0        * seed in d5
        add.l   d0,d0           * shift seed left (add dn,dn is faster)
        bhi.s   L5              * carry? (using bhi instead of bcc unlocks 0s)
        eori.l  #$1D872B41,d0   * magic number for 32 bit seed
L5:
        rts
        xdef    _FastRand
        data
        xdef    _RangeSeed
_RangeSeed:
        dc.l    0               * random number seed

        end
