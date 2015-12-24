/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  OPS.C
 *
 */

#ifndef DISOBJ_C

#include "defs.h"

Prototype int	ophash(char *);
Prototype void	InitOps(void);
Prototype OpCod *GetOpByName(char *);
Prototype short GetInstSize(MachCtx *);

#endif


/*
 *  The # of extension words is modified by various other parameters extracted from the EffAddr.ExtWord
 *
 *  -1 indicates immediate mode special
 *  -2 indicates index mode special
 */


short ExtWords[32] = {
			  0,  0, 0, 0, 0, 0,  1, -2, 1,2,1,-2,-1,  1,-1, 1, 0, 0, 0,  0,0,0,0,0,0,0,0,0,0,0,0,0,
		     };

short RegCode[32]  = {	 -1, -1,-1,-1,-1,-1, -1,-1, 0,1,2,3, 4, -1,-1,-1,-1,-1,-1, };
short EACode[32]   = {	 -1,  0, 1, 2, 3, 4,  5, 6, 7,7,7,7, 7, -1,-1,-1,-1,-1,-1, };
short PCRel[32]    = {	  0,  0, 0, 0, 0, 0,  0, 0, 0,0,1,1, 0,  0, 1, 1, 0, 0, 0, };


#ifndef DISOBJ_C

OpCod	*OHash[OSIZE];

#endif

/*
 *  OpCodes.  Ordering within a group (e.g. OpADD) determines which one is
 *	      accepted first.. specifically, when Siz = -1 the opcode can
 *	      be accepted without a size specification tag so be careful
 *	      about order.  E.G. word branches before byte branches, etc...
 *
 *	      warning: branch ordering in array assumed to be
 *	      word, then byte (see passc.c branch opt)
 *
 *  Can't make this const because it is modified!
 *
 *  NOTE:   branches must be ordered WBRANCH/BBRANCH (see passc.c)
 */

OpCod	Codes[] = {

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"ABCD", OpABCD, 0xC100, 0,  -1, 9,  -1, -1, S_B,    AF_DN,      AF_DN           },
    {	"ABCD", OpABCD, 0xC108, 0,  -1, 9,  -1, -1, S_B,    AF_MMIND,   AF_MMIND        },
    {	"ADD",  OpADD,  0xD000, 0,   3, 9,  -1,  6, S_ALL,  AF_ALL,     AF_DN           },
    {	"ADD",  OpADD,  0xD100, 9,  -1, 0,   3,  6, S_ALL,  AF_DN,      AF_ALTM         },
    {	"ADDA", OpADD,  0xD0C0, 0,   3, 9,  -1, -1, S_W,    AF_ALL,     AF_AN           },
    {	"ADDA", OpADD,  0xD1C0, 0,   3, 9,  -1, -1, S_L,    AF_ALL,     AF_AN           },
    {	"ADDI", OpADD,  0x0600, -1, -1, 0,   3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"ADDQ", OpADDQ, 0x5000, -1, -1, 0,   3,  6, S_ALL,  AF_IMM,     AF_ALT  ,  IMM18},
    {	"ADDX", OpADDX, 0xD100, 0,  -1, 9,  -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ADDX", OpADDX, 0xD108, 0,  -1, 9,  -1,  6, S_ALL,  AF_MMIND,   AF_MMIND        },
    {	"AND",  OpAND,  0xC000, 0,   3, 9,  -1,  6, S_ALL,  AF_ALLNA,   AF_DN           },
    {	"AND",  OpAND,  0xC100, 9,  -1, 0,   3,  6, S_ALL,  AF_DN,      AF_ALTM         },
    {	"ANDI", OpAND,  0x0200, -1, -1, 0,   3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"ANDI", OpAND,  0x023C, -1, -1, -1, -1, -1, S_B,    AF_IMM,     AF_CCR          },
    {	"ANDI", OpAND,  0x027C, -1, -1, -1, -1, -1, S_W,    AF_IMM,     AF_SR           },
    {	"ASL",  OpASL,  0xE120, 9,  -1, 0,  -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ASL",  OpASL,  0xE100, -1, -1, 0,  -1,  6, S_ALL,  AF_IMM,     AF_DN   ,  IMM18},
    {	"ASL",  OpASL,  0xE1C0, -1, -1, 0,   3, -1, S_W,    0,          AF_ALTM         },
    {	"ASR",  OpASR,  0xE020, 9,  -1, 0,  -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ASR",  OpASR,  0xE000, -1, -1, 0,  -1,  6, S_ALL,  AF_IMM,     AF_DN   ,  IMM18},
    {	"ASR",  OpASR,  0xE0C0, -1, -1, 0,   3, -1, S_W,    0,          AF_ALTM         },

    {	"BCC",  OpBCC,  0x6400, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BCC",  OpBCC,  0x6400, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BCS",  OpBCS,  0x6500, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BCS",  OpBCS,  0x6500, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BEQ",  OpBEQ,  0x6700, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BEQ",  OpBEQ,  0x6700, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BGE",  OpBGE,  0x6C00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BGE",  OpBGE,  0x6C00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BGT",  OpBGT,  0x6E00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BGT",  OpBGT,  0x6E00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BHI",  OpBHI,  0x6200, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BHI",  OpBHI,  0x6200, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BLE",  OpBLE,  0x6F00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BLE",  OpBLE,  0x6F00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BLS",  OpBLS,  0x6300, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BLS",  OpBLS,  0x6300, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BLT",  OpBLT,  0x6D00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BLT",  OpBLT,  0x6D00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BMI",  OpBMI,  0x6B00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BMI",  OpBMI,  0x6B00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BNE",  OpBNE,  0x6600, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BNE",  OpBNE,  0x6600, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BPL",  OpBPL,  0x6A00, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BPL",  OpBPL,  0x6A00, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BVC",  OpBVC,  0x6800, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BVC",  OpBVC,  0x6800, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },
    {	"BVS",  OpBVS,  0x6900, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0               },
    {	"BVS",  OpBVS,  0x6900, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0               },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"BCHG", OpBCHG, 0x0140,  9, -1,  0,  3, -1, S_B,    AF_DN,      AF_ALTM         },
    {	"BCHG", OpBCHG, 0x0140,  9, -1,  0,  3, -1, S_L,    AF_DN,      AF_DN           },
    {	"BCHG", OpBCHG, 0x0840, -1, -1,  0,  3, -1, S_B,    AF_IMM,     AF_ALTM , IMMB  },
    {	"BCHG", OpBCHG, 0x0840, -1, -1,  0,  3, -1, S_L,    AF_IMM,     AF_DN   , IMMB  },
    {	"BCLR", OpBCLR, 0x0180,  9, -1,  0,  3, -1, S_B,    AF_DN,      AF_ALTM         },
    {	"BCLR", OpBCLR, 0x0180,  9, -1,  0,  3, -1, S_L,    AF_DN,      AF_DN           },
    {	"BCLR", OpBCLR, 0x0880, -1, -1,  0,  3, -1, S_B,    AF_IMM,     AF_ALTM,  IMMB  },
    {	"BCLR", OpBCLR, 0x0880, -1, -1,  0,  3, -1, S_L,    AF_IMM,     AF_DN  ,  IMMB  },

    {	"BRA",  OpBRA,  0x6000, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0              },
    {	"BRA",  OpBRA,  0x6000, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0              },

    {	"BSET", OpBSET, 0x01C0,  9, -1,  0,  3, -1, S_B,    AF_DN,      AF_ALTM         },
    {	"BSET", OpBSET, 0x01C0,  9, -1,  0,  3, -1, S_L,    AF_DN,      AF_DN           },
    {	"BSET", OpBSET, 0x08C0, -1, -1,  0,  3, -1, S_B,    AF_IMM,     AF_ALTM, IMMB   },
    {	"BSET", OpBSET, 0x08C0, -1, -1,  0,  3, -1, S_L,    AF_IMM,     AF_DN  , IMMB   },
    {	"BTST", OpBTST, 0x0100,  9, -1,  0,  3, -1, S_B,    AF_DN,      AF_ALTM         },
    {	"BTST", OpBTST, 0x0100,  9, -1,  0,  3, -1, S_L,    AF_DN,      AF_DN           },
    {	"BTST", OpBTST, 0x0800, -1, -1,  0,  3, -1, S_B,    AF_IMM,     AF_ALTM, IMMB   },
    {	"BTST", OpBTST, 0x0800, -1, -1,  0,  3, -1, S_L,    AF_IMM,     AF_DN  , IMMB   },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"CHK",  OpCHK,  0x4180,  0,  3,  9, -1, -1, S_W,    AF_ALLNA,   AF_DN           },
    {	"CLR",  OpCLR,  0x4200, -1, -1,  0,  3,  6, S_ALL,  0,          AF_ALTNA        },
    {	"CMP",  OpCMP,  0xB000,  0,  3,  9, -1,  6, S_ALL,  AF_ALLNA,   AF_DN           },
    {	"CMP",  OpCMP,  0xB000,  0,  3,  9, -1,  6, S_WL,   AF_AN,      AF_DN           },
    {	"CMPA", OpCMP,  0xB0C0,  0,  3,  9, -1, -1, S_W,    AF_ALL,     AF_AN           },
    {	"CMPA", OpCMP,  0xB1C0,  0,  3,  9, -1, -1, S_L,    AF_ALL,     AF_AN           },
    {	"CMPI", OpCMP,  0x0C00, -1, -1,  0,  3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"CMPM", OpCMP,  0xB108,  0, -1,  9, -1,  6, S_ALL,  AF_INDPP,   AF_INDPP        },

    {	"DBCC", OpDBcc, 0x54C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBCS", OpDBcc, 0x55C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBEQ", OpDBcc, 0x57C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBF" , OpDBcc, 0x51C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBRA" , OpDBcc, 0x51C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBGE", OpDBcc, 0x5CC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBGT", OpDBcc, 0x5EC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBHI", OpDBcc, 0x52C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBLE", OpDBcc, 0x5FC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBLS", OpDBcc, 0x53C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBLT", OpDBcc, 0x5DC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBMI", OpDBcc, 0x5BC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBNE", OpDBcc, 0x56C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBPL", OpDBcc, 0x5AC8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBT" , OpDBcc, 0x50C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBVC", OpDBcc, 0x58C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },
    {	"DBVS", OpDBcc, 0x59C8,  0, -1, -1, -1, -1, S_W,    AF_DN,      AF_WBRANCH       },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"DIVS", OpDIVS, 0x81C0,  0,  3,  9, -1, -1, S_W,    AF_ALLNA,   AF_DN           },
    {	"DIVS", OpDIVS, 0x4C40,  0,  3, -1, -1, -1, S_L,    AF_ALLNA,   AF_DN|AF_MULDREGS},  /*  020 */
    {	"DIVU", OpDIVU, 0x80C0,  0,  3,  9, -1, -1, S_W,    AF_ALLNA,   AF_DN           },
    {	"DIVU", OpDIVU, 0x4C40,  0,  3, -1, -1, -1, S_L,    AF_ALLNA,   AF_DN|AF_MULDREGU},  /*  020 */
			  

    {	"EOR",  OpEOR,  0xB100,  9, -1,  0,  3,  6, S_ALL,  AF_DN,      AF_ALTNA        },
    {	"EORI", OpEOR,  0x0A00, -1, -1,  0,  3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"EORI", OpEOR,  0x0A3C, -1, -1, -1, -1, -1, S_B,    AF_IMM,     AF_CCR          },
    {	"EORI", OpEOR,  0x0A7C, -1, -1, -1, -1, -1, S_W,    AF_IMM,     AF_SR           },
    {	"EXG",  OpEXG,  0xC140,  9, -1,  0, -1, -1, S_L,    AF_DN,      AF_DN           },
    {	"EXG",  OpEXG,  0xC148,  9, -1,  0, -1, -1, S_L,    AF_AN,      AF_AN           },
    {	"EXG",  OpEXG,  0xC188,  9, -1,  0, -1, -1, S_L,    AF_DN,      AF_AN           },
    {	"EXG",  OpEXG,  0xC188,  0, -1,  9, -1, -1, S_L,    AF_AN,      AF_DN           },
    {	"EXT",  OpEXT,  0x4880, -1, -1,  0, -1, -1, S_W,    0,          AF_DN           },
    {	"EXT",  OpEXT,  0x48C0, -1, -1,  0, -1, -1, S_L,    0,          AF_DN           },
    {	"EXTB", OpEXTB, 0x49C0, -1, -1,  0, -1, -1, S_L,    0,          AF_DN           },
    {	"ILLEGAL", OpILLEGAL,

    /*
     *	note: BSR/BSR/JSR ordering must remain as it is (passc.c optimizations)
     */
			0x4AFC, -1, -1, -1, -1, -1, 0,	    0,		0		},
    {	"JMP",  OpJMP,  0x4EC0,  0,  3, -1, -1, -1, 0,      AF_CTL,     0               },
    {	"BSR",  OpBSR,  0x6100, -1, -1, -1, -1, -1, S_W,    AF_WBRANCH,  0              },
    {	"BSR",  OpBSR,  0x6100, -1, -1, -1, -1, -1, S_B,    AF_BBRANCH,  0              },
    {	"JSR",  OpJSR,  0x4E80,  0,  3, -1, -1, -1, 0,      AF_CTL,     0               },

    {	"LEA",  OpLEA,  0x41C0,  0,  3,  9, -1, -1, S_L,    AF_CTL,     AF_AN           },
    {	"LINK", OpLINK, 0x4E50,  0, -1, -1, -1, -1, S_W,    AF_AN,      AF_IMM          },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"LSL",  OpLSL,  0xE128,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"LSL",  OpLSL,  0xE108, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"LSL",  OpLSL,  0xE3C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },
    {	"LSR",  OpLSR,  0xE028,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"LSR",  OpLSR,  0xE008, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"LSR",  OpLSR,  0xE2C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },

    {	"MOVE", OpMOVE, 0x2000,  0,  3,  9,  6, -1, S_L,    AF_ALLNA,   AF_ALTNA        },
    {	"MOVE", OpMOVE, 0x3000,  0,  3,  9,  6, -1, S_W,    AF_ALLNA,   AF_ALTNA        },
    {	"MOVE", OpMOVE, 0x1000,  0,  3,  9,  6, -1, S_B,    AF_ALLNA,   AF_ALTNA        },
    {	"MOVE", OpMOVE, 0x2000,  0,  3,  9,  6, -1, S_L,    AF_AN,      AF_ALTNA        },
    {	"MOVE", OpMOVE, 0x3000,  0,  3,  9,  6, -1, S_W,    AF_AN,      AF_ALTNA        },

  /* 68010
   *{	"MOVE", OpMOVE, 0x42C0, -1, -1,  0,  3, -1, S_W,    AF_CCR,     AF_ALTNA        },
   */

    {	"MOVE", OpMOVE, 0x44C0,  0,  3, -1, -1, -1, S_W,    AF_ALLNA,   AF_CCR          },
    {	"MOVE", OpMOVE, 0x46C0,  0,  3, -1, -1, -1, S_W,    AF_ALLNA,   AF_SR           },
    {	"MOVE", OpMOVE, 0x40C0, -1, -1,  0,  3, -1, S_W,    AF_SR,      AF_ALTNA        },
    {	"MOVEA",OpMOVE, 0x3040,  0,  3,  9, -1, -1, S_W,    AF_ALL,     AF_AN           },
    {	"MOVEA",OpMOVE, 0x2040,  0,  3,  9, -1, -1, S_L,    AF_ALL,     AF_AN           },
    {	"MOVEA",OpMOVE, 0x4E68, -1, -1,  0, -1, -1, S_L,    AF_USP,     AF_AN           },
    {	"MOVEA",OpMOVE, 0x4E60,  0, -1, -1, -1, -1, S_L,    AF_AN,      AF_USP          },
    /* MOVEC 68010  */
    {	"MOVEM",OpMOVEM,0x48C0, -1, -1,  0,  3, -1, S_L,    AF_REGS,    AF_CTL|AF_MMIND },
    {	"MOVEM",OpMOVEM,0x4880, -1, -1,  0,  3, -1, S_W,    AF_REGS,    AF_CTL|AF_MMIND },
    {	"MOVEM",OpMOVEM,0x4CC0,  0,  3, -1, -1, -1, S_L,    AF_CTL|AF_INDPP,    AF_REGS },
    {	"MOVEM",OpMOVEM,0x4C80,  0,  3, -1, -1, -1, S_W,    AF_CTL|AF_INDPP,    AF_REGS },
    {	"MOVEP",OpMOVEP,0x0188,  9, -1,  0, -1, -1, S_W,    AF_DN,      AF_OFFAN        },
    {	"MOVEP",OpMOVEP,0x01C8,  9, -1,  0, -1, -1, S_L,    AF_DN,      AF_OFFAN        },
    {	"MOVEP",OpMOVEP,0x0108,  0, -1,  9, -1, -1, S_W,    AF_OFFAN,   AF_DN           },
    {	"MOVEP",OpMOVEP,0x0148,  0, -1,  9, -1, -1, S_L,    AF_OFFAN,   AF_DN           },
    /*	MOVES	68010	*/
    {	"MOVEQ",OpMOVEQ,0x7000, -1, -1,  9, -1, -1, S_L,    AF_IMM,     AF_DN   , IMM256},

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"MULS", OpMULS, 0xC1C0,  0,  3,  9, -1, -1, S_W,    AF_ALLNA,   AF_DN           },
    {	"MULS", OpMULS, 0x4C00,  0,  3, -1, -1, -1, S_L,    AF_ALLNA,   AF_DN|AF_MULDREGS},  /*  020 */
    {	"MULU", OpMULU, 0xC0C0,  0,  3,  9, -1, -1, S_W,    AF_ALLNA,   AF_DN           },
    {	"MULU", OpMULU, 0x4C00,  0,  3, -1, -1, -1, S_L,    AF_ALLNA,   AF_DN|AF_MULDREGU},  /*  020 */
    {	"NBCD", OpNBCD, 0x4800, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"NEG",  OpNEG,  0x4400, -1, -1,  0,  3,  6, S_ALL,  0,          AF_ALTNA        },
    {	"NEGX", OpNEGX, 0x4000, -1, -1,  0,  3,  6, S_ALL,  0,          AF_ALTNA        },
    {	"NOP",  OpNOP,  0x4E71, -1, -1, -1, -1, -1, 0,      0,          0               },
    {	"NOT",  OpNOT,  0x4600, -1, -1,  0,  3,  6, S_ALL,  0,          AF_ALTNA        },

    {	"OR",   OpOR,   0x8000,  0,  3,  9, -1,  6, S_ALL,  AF_ALLNA,   AF_DN           },
    {	"OR",   OpOR,   0x8100,  9, -1,  0,  3,  6, S_ALL,  AF_DN,      AF_ALTNA        },
    {	"ORI",  OpOR,   0x0000, -1, -1,  0,  3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"ORI",  OpOR,   0x003C, -1, -1, -1, -1, -1, S_B,    AF_IMM,     AF_CCR          },
    {	"ORI",  OpOR,   0x007C, -1, -1, -1, -1, -1, S_W,    AF_IMM,     AF_SR           },
    {	"PEA",  OpPEA,  0x4840,  0,  3, -1, -1, -1, S_L,    AF_CTL,     0               },
    {	"RESET",OpRESET,0x4E70, -1, -1, -1, -1, -1, 0,      0,          0               },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"ROL",  OpROL,  0xE138,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ROL",  OpROL,  0xE118, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"ROL",  OpROL,  0xE7C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },
    {	"ROR",  OpROR,  0xE038,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ROR",  OpROR,  0xE018, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"ROR",  OpROR,  0xE6C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },

    {	"ROXL", OpROXL, 0xE130,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ROXL", OpROXL, 0xE110, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"ROXL", OpROXL, 0xE5C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },
    {	"ROXR", OpROXR, 0xE030,  9, -1,  0, -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"ROXR", OpROXR, 0xE010, -1, -1,  0, -1,  6, S_ALL,  AF_IMM,     AF_DN   , IMM18 },
    {	"ROXR", OpROXR, 0xE4C0, -1, -1,  0,  3, -1, S_W,    0,          AF_ALTM         },

    /*	RTD 68010   */
    {	"RTE",  OpRTE,  0x4E73, -1, -1, -1, -1, -1, 0,      0,          0               },
    {	"RTR",  OpRTR,  0x4E77, -1, -1, -1, -1, -1, 0,      0,          0               },
    {	"RTS",  OpRTS,  0x4E75, -1, -1, -1, -1, -1, 0,      0,          0               },
    {	"SBCD", OpSBCD, 0x8100,  0, -1,  9, -1, -1, S_B,    AF_DN,      AF_DN           },
    {	"SBCD", OpSBCD, 0x8108,  0, -1,  9, -1, -1, S_B,    AF_MMIND,   AF_MMIND        },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    /* SCC note, even though they are in the same group since all have the
     * same note if one isn't accepted none of the others will be either
     */

    {	"SCC",  OpScc,  0x54C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SCS",  OpScc,  0x55C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SEQ",  OpScc,  0x57C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SF",   OpScc,  0x51C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SGE",  OpScc,  0x5CC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SGT",  OpScc,  0x5EC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SHI",  OpScc,  0x52C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SLE",  OpScc,  0x5FC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SLS",  OpScc,  0x53C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SLT",  OpScc,  0x5DC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SMI",  OpScc,  0x5BC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SNE",  OpScc,  0x56C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SPL",  OpScc,  0x5AC0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"ST" ,  OpScc,  0x50C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SVC",  OpScc,  0x58C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },
    {	"SVS",  OpScc,  0x59C0, -1, -1,  0,  3, -1, S_B,    0,          AF_ALTNA        },

    /*	    opcode	 Temp	Rs  EAs Rd  EAd Siz Sizes  SModes	DModes	  Spec# */

    {	"STOP", OpSTOP, 0x4E72, -1, -1, -1, -1, -1, S_W,    AF_IMM,     0               },

    {	"SUB",  OpSUB,  0x9000, 0,   3, 9,  -1,  6, S_ALL,  AF_ALL,     AF_DN           },
    {	"SUB",  OpSUB,  0x9100, 9,  -1, 0,   3,  6, S_ALL,  AF_DN,      AF_ALTM         },
    {	"SUBA", OpSUB,  0x90C0, 0,   3, 9,  -1, -1, S_W,    AF_ALL,     AF_AN           },
    {	"SUBA", OpSUB,  0x91C0, 0,   3, 9,  -1, -1, S_L,    AF_ALL,     AF_AN           },
    {	"SUBI", OpSUB,  0x0400, -1, -1, 0,   3,  6, S_ALL,  AF_IMM,     AF_ALTNA        },
    {	"SUBQ", OpSUBQ, 0x5100, -1, -1, 0,   3,  6, S_ALL,  AF_IMM,     AF_ALT  ,  IMM18},
    {	"SUBX", OpSUBX, 0x9100, 0,  -1, 9,  -1,  6, S_ALL,  AF_DN,      AF_DN           },
    {	"SUBX", OpSUBX, 0x9108, 0,  -1, 9,  -1,  6, S_ALL,  AF_MMIND,   AF_MMIND        },

    {	"SWAP", OpSWAP, 0x4840, -1, -1, 0,  -1, -1, S_W,    0,          AF_DN           },
    {	"TAS",  OpTAS,  0x4AC0, -1, -1, 0,   3, -1, S_B,    0,          AF_ALTNA        },
    {	"TRAP", OpTRAP, 0x4E40, -1, -1, -1, -1, -1, 0,      AF_IMM,     0       , IMM0F },
    {	"TRAPV",OpTRAPV,0x4E76, -1, -1, -1, -1, -1, 0,      0,          0               },
    {	"TST",  OpTST,  0x4A00,  0,  3, -1, -1,  6, S_ALL,  AF_ALTNA,   0               },
    {	"UNLK", OpUNLK, 0x4E58, -1, -1, 0,  -1, -1, 0,      0,          AF_AN           },

    {	"EQU",  OdEQU   },
    {	"REG",  OdREG   },
    {	"XREF", OdXREF  },
    {	"XDEF", OdXDEF  },
    {	"SECTION",
		OdSECTION },
    {	"DS",   OdDS    },
    {	"DC",   OdDC    },
    {	"END",  OdEND   },
    {	"INCLUDE", OdINCLUDE },
    {	"PROCSTART", OdPROCSTART },
    {	"PROCEND"  , OdPROCEND   },
    {	"MC68020"  , OdMC68020   },
    {	"MC68881"  , OdMC68881   },
    {	"DEBUG"    , OdDEBUG     },
    {	"ALIGN"	   , OdALIGN	 },
    {	NULL }

};

#ifndef DISOBJ_C

int
ophash(str)
char *str;
{
    int32_t hv = 0x1B465D8;

    while (*str) {
	hv = (hv >> 23) ^ (hv << 5) ^ (ubyte)*str;
	++str;
    }
    return(hv & OMASK);
}

void
InitOps()
{
    OpCod *oc;
    OpCod *ocn;

    for (oc = Codes; oc->OpName; ++oc) {
	int32_t hv = ophash(oc->OpName);

	oc->HNext = OHash[hv];
	OHash[hv] = oc;

	for (ocn = oc + 1; ocn->OpName && strcmp(oc->OpName, ocn->OpName) == 0; ++oc, ++ocn) {
	    oc->SibNext = ocn;
	}

	if (ocn->OpName && ocn->Id == oc->Id)
	    oc->SibNext = ocn;
    }
}

OpCod *
GetOpByName(name)
char *name;
{
    OpCod *oc;
    int32_t hv = ophash(name);

    for (oc = OHash[hv]; oc && strcmp(name, oc->OpName) != 0; oc = oc->HNext);
    return(oc);
}

/*
 *  callable after passb, returns size in bytes
 */

short
GetInstSize(mc)
MachCtx *mc;
{
    short i = ExtWords[(short)mc->Oper1.Mode1];
    short j = ExtWords[(short)mc->Oper2.Mode1];
    short opsize = mc->OpSize;	    /*	used only if IMM mode */

    if (opsize == 0) {
	OpCod *oc = mc->OpCode;
	if (oc->Sizes & S_B)
	    opsize = 1;
	if (oc->Sizes & S_W)
	    opsize = 2;
	if (oc->Sizes & S_L)
	    opsize = 4;
    }

    if (mc->OpCode == NULL)
        return(2);

    if (i == -1) {          /*  imm/branch  */
	if (mc->OpCode->Special == IMMB)
	    i = 1;
	else if (mc->OpCode->Special || (mc->OpCode->SModes & AF_BBRANCH))
	    i = 0;
	else
	    i = (opsize + 1) >> 1;  /* 1 for bw, 2 for int32_t */
    } else if (i == -2) {   /*  index mode  */
	i = 1;
	if (mc->Oper1.ExtWord & EXTF_FULL) {
	    switch(mc->Oper1.ExtWord & EXTF_BDMASK) {
	    case EXTF_BDLONG:
		++i;
	    case EXTF_BDWORD:
		++i;
		break;
	    }
	    switch(mc->Oper1.ExtWord & EXTF_ODMASK) {
	    case EXTF_ODLONG:
		++i;
	    case EXTF_ODWORD:
		++i;
		break;
	    }
	}
    }
    if (j == -1) {          /*  imm/branch  */
	if (mc->OpCode->Special == IMMB)
	    j = 1;
	else if (mc->OpCode->Special)
	    j = 0;
	else
	    j = (opsize + 1) >> 1;  /* 1 for bw, 2 for int32_t */
    } else if (j == -2) {   /*  index mode  */
	j = 1;
	if (mc->Oper2.ExtWord & EXTF_FULL) {
	    switch(mc->Oper2.ExtWord & EXTF_BDMASK) {
	    case EXTF_BDLONG:
		++j;
	    case EXTF_BDWORD:
		++j;
		break;
	    }
	    switch(mc->Oper2.ExtWord & EXTF_ODMASK) {
	    case EXTF_ODLONG:
		++j;
	    case EXTF_ODWORD:
		++j;
		break;
	    }
	}
    }
    if (mc->OpCode->DModes & (AF_MULDREGS|AF_MULDREGU))	/* XXX MULx.L, DIVx.L */
	++j;
    dbprintf(0, ("line %ld opcode %s size (%dw,%dw)%d\n", CurFile->fn_LineNo, mc->OpCode->OpName, i, j, (1 + i + j) * 2));
    return((short)((1 + i + j) << 1));
}

/*
 *  NOTE:   For #IMM and byte-branch modes this call only works if the
 *	    extension occurs directly after the instruction.
 *
 *	    For indexing modes this call works for either src or dst
 *
 *	    returns number of extension words, location and type
 *	    of relocation.
 */

#ifdef NOTDEF

short
GetSpecDataSize(mc, oc, ea, poff)
MachCtx *mc;
OpCod *oc;
EffAddr *ea;
int32_t *poff;
{
    short opsize;
    *poff = 0;

    if (ea->Mode1 == AB_BBRANCH) {
	*poff = -1;	    /*	relocate back into instruction */
	return(1);          /*  size is byte                   */
    }
    if (ea->Mode1 == AB_OFFIDX || ea->Mode1 == AB_OFFIDXPC) {
	if (ea->ExtWord & EXTF_FULL) {
	    int r = 1;

	    switch(ea->ExtWord & EXTF_ODMASK) {
	    case EXTF_ODLONG:
		++r;
	    case EXTF_ODWORD:
		++r;
		break;
	    }

	    switch(ea->ExtWord & EXTF_BDMASK) {
	    case EXTF_BDLONG:
		++r;
	    case EXTF_BDWORD:
		++r;
		break;
	    }
	    return(r);
	}
	return(1);
    }
    if (oc->Special) {
	if (oc->Special == IMMB) {
	    *poff = 1;	    /*	relocate half way through next word */
	    return(1);      /*  size is byte                    */
	}
	if (oc->Special == IMM256) {
	    *poff = -1;     /*	as in moveq, item back in inst. */
	    return(1);
	}
	return(0);          /*  not relocatable     */
    }
    opsize = mc->OpSize;    /*	opsize unspecified and #imm mode..  */
    if (opsize == 0) {
	if (oc->Sizes & S_B)
	    opsize = 1;
	if (oc->Sizes & S_W)
	    opsize = 2;
	if (oc->Sizes & S_L)
	    opsize = 4;
    }
    switch(opsize) {
    case 1:
	*poff = 1;	    /*	offset one from word	*/
	return(1);          /*  byte relocation */
    case 2:
	return(2);          /*  word relocation */
    case 4:
	return(4);          /*  int32_t relocation */
    }
    cerror(ESOFTWARN_BAD_OP_SIZE, mc->OpSize);
}
#endif


#endif
