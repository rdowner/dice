
/*
 *  DAS/OPS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#define OSIZE   128
#define OMASK   (OSIZE-1)


#define OdEQU       -1
#define OdREG       -2
#define OdXREF      -3
#define OdXDEF      -4
#define OdSECTION   -5
#define OdEND       -6
#define OdDS        -7
#define OdDC        -8
#define OdPROCSTART -9
#define OdPROCEND   -10
#define OdMC68020   -11
#define OdMC68881   -12
#define OdDEBUG     -13
#define OdALIGN     -14
#define OdINCLUDE   -15


#define OpABCD      1
#define OpADD       2
#define OpADDQ      3
#define OpADDX      4
#define OpAND       5
#define OpASL       6
#define OpASR       7

#define OpBCC       8
#define OpBCS       9
#define OpBEQ       10
#define OpBGE       11
#define OpBGT       12
#define OpBHI       13
#define OpBLE       14
#define OpBLS       15
#define OpBLT       16
#define OpBMI       17
#define OpBNE       18
#define OpBPL       19
#define OpBVC       20
#define OpBVS       21
#define OpBRA       22
#define OpBSR       23

#define OpBCHG      24
#define OpBCLR      25
#define OpBSET      26
#define OpBTST      27
#define OpCHK       28
#define OpCLR       29
#define OpCMP       30
#define OpDBcc      31
#define OpDIVS      32
#define OpDIVU      33
#define OpEOR       34
#define OpEXG       35
#define OpEXT       36
#define OpILLEGAL   37
#define OpJMP       38
#define OpJSR       39
#define OpLEA       40
#define OpLINK      41
#define OpLSL       42
#define OpLSR       43
#define OpMOVE      44
#define OpMOVEC     45
#define OpMOVEM     46
#define OpMOVEP     47
#define OpMOVEQ     48
#define OpMOVES     49
#define OpMULS      50
#define OpMULU      51
#define OpNBCD      52
#define OpNEG       53
#define OpNEGX      54
#define OpNOP       55
#define OpNOT       56
#define OpOR        57
#define OpPEA       58
#define OpRESET     59
#define OpROL       60
#define OpROR       61
#define OpROXL      62
#define OpROXR      63

#define OpRTD       64
#define OpRTE       65
#define OpRTR       66
#define OpRTS       67

#define OpSBCD      68
#define OpScc       69
#define OpSTOP      70
#define OpSUB       71
#define OpSUBQ      72
#define OpSUBX      73
#define OpSWAP      74
#define OpTAS       75
#define OpTST       76
#define OpTRAP      77
#define OpTRAPV     78
#define OpUNLK      79

#define LIMIT68000  80
#define LIMIT68010  80

#define OpEXTB      80

#define OpFABS      82
#define Op

#define LIMIT68020  90

/*
 *  addressing mode determines number of extension words
 *
 *  warning, must chg array in ops.c if you change these defines.
 */

#define AB_DN       1
#define AB_AN       2
#define AB_INDAN    3
#define AB_INDPP    4
#define AB_MMIND    5
#define AB_OFFAN    6
#define AB_OFFIDX   7
#define AB_ABSW     8
#define AB_ABSL     9
#define AB_OFFPC    10
#define AB_OFFIDXPC 11
#define AB_IMM      12
#define AB_REGS     13      /*  movem       */
#define AB_BBRANCH  14
#define AB_WBRANCH  15
#define AB_CCR      16
#define AB_SR       17
#define AB_USP      18
#define AB_MULDREGS 19
#define AB_MULDREGU 20

#define AF_DN       (1 << AB_DN)
#define AF_AN       (1 << AB_AN)
#define AF_INDAN    (1 << AB_INDAN)
#define AF_INDPP    (1 << AB_INDPP)
#define AF_MMIND    (1 << AB_MMIND)
#define AF_OFFAN    (1 << AB_OFFAN)
#define AF_OFFIDX   (1 << AB_OFFIDX)
#define AF_ABSW     (1 << AB_ABSW)
#define AF_ABSL     (1 << AB_ABSL)
#define AF_OFFPC    (1 << AB_OFFPC)
#define AF_OFFIDXPC (1 << AB_OFFIDXPC)
#define AF_IMM      (1 << AB_IMM)
#define AF_REGS     (1 << AB_REGS)
#define AF_BBRANCH  (1 << AB_BBRANCH)
#define AF_WBRANCH  (1 << AB_WBRANCH)
#define AF_CCR      (1 << AB_CCR)
#define AF_SR       (1 << AB_SR)
#define AF_USP      (1 << AB_USP)
#define AF_MULDREGS (1 << AB_MULDREGS)
#define AF_MULDREGU (1 << AB_MULDREGU)

#define AF_ALL      (AF_DN|AF_AN|AF_INDAN|AF_INDPP|AF_MMIND|AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL|AF_OFFPC|AF_OFFIDXPC|AF_IMM)
#define AF_ALLNA    (AF_DN      |AF_INDAN|AF_INDPP|AF_MMIND|AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL|AF_OFFPC|AF_OFFIDXPC|AF_IMM)
#define AF_ALT      (AF_DN|AF_AN|AF_INDAN|AF_INDPP|AF_MMIND|AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL)
#define AF_ALTNA    (AF_DN      |AF_INDAN|AF_INDPP|AF_MMIND|AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL)
#define AF_ALTM     (            AF_INDAN|AF_INDPP|AF_MMIND|AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL)
#define AF_CTL      (            AF_INDAN                  |AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL|AF_OFFPC|AF_OFFIDXPC)
#define AF_CTLNPC   (            AF_INDAN                  |AF_OFFAN|AF_OFFIDX|AF_ABSW|AF_ABSL|         AF_OFFIDXPC)

#define S_B     2   /*  1 << 1 */
#define S_W     4   /*  1 << 2 */
#define S_L     16  /*  1 << 4 */
#define S_WL    (S_W|S_L)
#define S_BW    (S_B|S_W)
#define S_ALL   (S_B|S_W|S_L)

#define ISF_ABSW        0x01
#define ISF_BYTEB       0x02
#define ISF_ODDREL      0x04    /*  data-rel outer displacement */
#define ISF_BDDREL      0x08    /*  data-rel outer displacement */
#define ISF_INSTBYTE    0x10    /*  byte inst (used for branch) */
/*#define ISF_LABEL2  0x04      extended label - 020 full format    */
/*#define ISF_OFFSET2 0x08      extended offset - 020 full format   */

#define EXTF_LWORD  0x0800
#define EXTF_AREG   0x8000
#define EXTF_FULL   0x0100    /*  MC68020 full format */
#define EXTF_NOIDX  0x0040
#define EXTF_NOBREG 0x0080

#define EXTF_BDNULL 0x0010
#define EXTF_BDWORD 0x0020
#define EXTF_BDLONG 0x0030
#define EXTF_BDMASK 0x0030

#define EXTF_ODNULL 0x0001
#define EXTF_ODWORD 0x0002
#define EXTF_ODLONG 0x0003
#define EXTF_ODMASK 0x0003

#define EXTF_MEMIND 0x0004


#define IMM07   1
#define IMM18   2
#define IMM256  3
#define IMMBR   4
#define IMMB    5   /*  like #imm normal byte, but independant of inst size */
#define IMM0F   6   /*  low 4 bits of instruction, #0-15                    */

typedef struct OpCod {
    char    *OpName;
    short   Id;
    uword   Template;
    char    B_Rs;
    char    B_EAs;
    char    B_Rd;
    char    B_EAd;
    char    B_Siz;
    char    Sizes;
    int32_t    SModes;
    int32_t    DModes;
    short   Special;

    struct OpCod   *HNext;
    struct OpCod   *SibNext;
} OpCod;

extern short ExtWords[];        /*  ExtWords[ABmode]    */
extern short RegCode[];
extern short EACode[];
extern short PCRel[];


