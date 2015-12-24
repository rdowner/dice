
/*
 *  DC1/MACHINE.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  The more machine dependant stuff goes here
 *
 *  note that string constant is converted to label-relative by this pt.
 *
 *  REGISTERS:	There are 16 data and 16 address registers.  Only the lower
 *  8 of each is valid.  The upper 8 is used to detect when we run out of
 *  registers and precisely the number of registers we would otherwise have
 *  needed.
 */

#if 0
#define CODE_ALIGN	4
#define STACK_ALIGN	4   /*	block-by-block alignment of stack   */
#define STRUCT_ALIGN	2   /*	NOTE: must be at least 2 so structure-copies work */
#define ARG_ALIGN	4   /*	for integers only		    */
#endif

#define TMP_STACK_MAX	64

#define OPT_SIZE	2   /*	multiply optimizio  */
#define INT_SIZE	4
#define PTR_SIZE	4
#define LGBO_SIZE	16  /*	largest primitive object size (int32_t double) */

#define SWITCH_SLOP	4

#define ASM_CODE	1   /*	sections    */
#define ASM_BSS 	2
#define ASM_DATA	3
#define ASM_FARDATA	4


#define ST_IntConst	1   /*	integer constant    */
#define ST_PtrConst	2
#define ST_FltConst	3   /*	floating constant   */
#define ST_StrConst	4   /*	string	 constant   */
#define ST_Reg		5   /*	register	    */
#define ST_RelReg	6
#define ST_RelArg	7
#define ST_RelName	8   /*	(&namedlabel+off)   */
#define ST_RelLabel	9   /*	(&label+off)        */
#define ST_RegIndex	10
#define ST_Push 	15  /*	asm.c only	    */

#define SF_VAR	    0x00000001	/*  variable			*/
#define SF_NOSA     0x00000002	/*  no store-across onto me	*/
#define SF_TMP	    0x00000004	/*  temporary (not an lvalue)   */
#define SF_LEA	    0x00000008	/*  effective-address-of	*/
#define SF_UNSIGNED 0x00000010
#define SF_ASMLEVEL 0x00000020	/*  double-check who allocated this?	*/
#define SF_CODE     0x00000040	/*  label/name in code segment		*/
#define SF_FORCED   0x00000080	/*  force-alloc, must restore on free	*/

#define SF_BITFIELD 0x00000100	/*  storage is a bit field	*/
#define SF_FAR	    0x00000200	/*  force far reference 	*/
#define SF_NEAR     0x00000400	/*  force near reference	*/
#define SF_IDXWORD  0x00000800	/*  word index for ST_RegIndex	*/
#define SF_TMP2     0x00001000	/*  index is a temporary	*/

#define SF_IDXSCAL2 0x00002000	/*  ST_RegIndex, scale factors	*/
#define SF_IDXSCAL4 0x00004000
#define SF_IDXSCAL8 0x00008000
#define SF_REGARGS  0x00010000	/*  asm_call			*/
#define SF_REGARGSUSED	0x00020000  /*	asm_call		    */

typedef struct Stor {
    short   st_Type;	    /*	storage type	*/
    short   st_RegNo;	    /*	if register	*/
    int32_t    st_Size;	    /*	quantity size	*/
    int32_t    st_Flags;
    short   st_RegNo2;	    /*	ST_RegIndex (data register) */
    char    st_BOffset;     /*	bitfield offset */
    char    st_BSize;	    /*	bitfield size	*/

    union {
	int32_t	    IntConst;	/*  terminating integer constant    */
	uint32_t	    UIntConst;
	char	    *StrConst;	/*  terminating string constant     */
	char	    *FltConst;	/*  terminating float constant	    */
	int32_t	    RegValid;	/*  how much of reg is valid?	    */
	int32_t	    Label;	/*  label number (local label)      */
	Symbol	    *Name;	/*  named label 		    */
    } u;

    union {
	int32_t	    StrLen;	/*  length of unterminated string   */
	int32_t	    FltLen;	/*  length of unterminated fp string*/
	int32_t	    Offset;	/*  offset relative storage	    */
	uint32_t	    UOffset;
    } v;
} Stor;

#define st_IntConst	u.IntConst
#define st_PtrConst	v.UOffset
#define st_UIntConst	u.UIntConst
#define st_StrConst	u.StrConst
#define st_FltConst	u.FltConst
#define st_RegValid	u.RegValid
#define st_Label	u.Label
#define st_Name 	u.Name

#define st_StrLen	v.StrLen
#define st_FltLen	v.FltLen	/*  WARNING, assumed to be int32_t */
#define st_Offset	v.Offset

/*
 *  The machine frame is part of the semantic structure (private to sem.c)
 *  While the semantic structure keeps lists of overloaded symbols the frame
 *  structure keeps storage statistics.
 */

#define FF_FPUSED   0x01    /*	frame pointer used		*/
#define FF_CALLMADE 0x02    /*	call made in block or subblock	*/

typedef struct Frame {
    char    Flags;
    char    Rsv1;
    short   Rsv2;

    int32_t    ArgsStackUsed;  /*	for top level block	*/
    int32_t    StackParent;    /*	by  parent		*/
    int32_t    StackUsed;	    /*	by  me	so far		*/
    int32_t    DownStackUsed;  /*	by sub-blocks		*/

    short   SubARegOver;    /*	# regs over for sub blocks  */
    short   SubDRegOver;
    short   CurARegOver;    /*	# regs over for this block  */
    short   CurDRegOver;
    int32_t    Reserved2;
    int32_t    Reserved3;
} Frame;

/*
 *  up to 32 machine registers
 */

#define DREGS	8
#define AREGS	8

#define RB_D0	0
#define RB_D1	1
#define RB_D2	2
#define RB_D3	3
#define RB_D4	4
#define RB_D5	5
#define RB_D6	6
#define RB_D7	7

#define RB_ADDR 16

#define RB_A0	16
#define RB_A1	17
#define RB_A2	18
#define RB_A3	19
#define RB_A4	20
#define RB_A5	21
#define RB_A6	22
#define RB_A7	23

#define RF_D0	(1 << RB_D0)
#define RF_D1	(1 << RB_D1)
#define RF_D2	(1 << RB_D2)
#define RF_D3	(1 << RB_D3)
#define RF_D4	(1 << RB_D4)
#define RF_D5	(1 << RB_D5)
#define RF_D6	(1 << RB_D6)
#define RF_D7	(1 << RB_D7)

#define RF_A0	(1 << RB_A0)
#define RF_A1	(1 << RB_A1)
#define RF_A2	(1 << RB_A2)
#define RF_A3	(1 << RB_A3)
#define RF_A4	(1 << RB_A4)
#define RF_A5	(1 << RB_A5)
#define RF_A6	(1 << RB_A6)
#define RF_A7	(1 << RB_A7)

#define RF_DREG     0x0000FFFF
#define RF_AREG     0xFFFF0000

#define RB_SP	RB_A7
#define RB_FP	RB_A5

#define RF_SP	(1 << RB_SP)
#define RF_FP	(1 << RB_FP)

#define REGSCRATCH  (RF_D0|RF_D1|RF_A0|RF_A1)
#define REGREAL     0x00FF00FF
#define REGSDPTR    (RF_A4)
#define REGSLPTR    (RF_A5)
#define REGREGCALL  (RF_D2|RF_D3)

#define MAX_STACK_TMP	16

#define RB_RES1     RB_D0   /*	8..32 bit quantities	*/
#define RB_RES2     RB_D1   /*	64 bit quantities	*/

#define COND_T		1
#define COND_F		-1

#define COND_LT 	1
#define COND_LTEQ	2
#define COND_GT 	3
#define COND_GTEQ	4
#define COND_EQ 	5
#define COND_NEQ	6

#define COND_BPL	7   /*	CAN'T BE USED WITH COMPARE  */
#define COND_BMI	8   /*	CAN'T BE USED WITH COMPARE  */

#define CF_UNS		0x10

typedef struct TmpStack {
    int32_t    ts_Offset;
    short   ts_Size;
    short   ts_Refs;
} TmpStack;

typedef struct TmpAggregate {
    struct TmpAggregate *ta_Next;
    char    *ta_Buf;
    int32_t    ta_Index;
    int32_t    ta_Bytes;
} TmpAggregate;

