
/*
 *  DC1/TOKENS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  The higher the precedence the closer the operator bonds.  Unary operators
 *  have higher precedence than binary operators.  postfix operators have higher
 *  precedence than unary operators.
 *
 *  only binary operators need be specified in the ExpPrec array.
 *
 *  NOTE!!  Update Greg the C++ guy if you change this!
 */

#define EXPPREC {   0,	0,  0,	90, 0,	0,  130,20, 80,  50, 20,  0,   0,  130, 20, 120,    \
		    0,	20, 10, 120,0,	20, 0,	0,  130, 20, 100, 110, 20, 100, 20, 90,     \
		    100,100,110,20, 30, 31, 0,	0,  70,  20, 60,  50,  20, 0,	0,  0,	    \
		    0,0 								    \
		}

/*
 *  TOKF_PRIVATE is used in semantic level handling and also with
 *  preprocessor compiled tokens.  Specifically, 0x80nn through 0x87nn
 *  translates to 0x00nn through 0xn0nn
 */

#define TOKF_PRIVATE	0x8000
#define TOKF_CLASS	0xFF00

/*
 *  preprocessor compiled token sequence
 */

#define TokBegSeq	(TOKF_PRIVATE|0x0000)
#define TokEndSeq	(TOKF_PRIVATE|0x0001)

/*
 *  expression elements
 */

#define TokNot		0x0002
#define TokNotEq	0x0003
#define TokPercent	0x0006
#define TokPercentEq	0x0007
#define TokAnd		0x0008
#define TokAndAnd	0x0009
#define TokAndEq	0x000A
#define TokLParen	0x000B
#define TokRParen	0x000C
#define TokStar 	0x000D
#define TokStarEq	0x000E
#define TokPl		0x000F
#define TokPlPl 	0x0010
#define TokPlEq 	0x0011
#define TokComma	0x0012
#define TokMi		0x0013
#define TokMiMi 	0x0014
#define TokMiEq 	0x0015
#define TokStrInd	0x0016
#define TokStrElm	0x0017
#define TokDiv		0x0018
#define TokDivEq	0x0019
#define TokLt		0x001A
#define TokLtLt 	0x001B
#define TokLtLtEq	0x001C
#define TokLtEq 	0x001D
#define TokEq		0x001E
#define TokEqEq 	0x001F
#define TokGt		0x0020
#define TokGtEq 	0x0021
#define TokGtGt 	0x0022
#define TokGtGtEq	0x0023
#define TokQuestion	0x0024
#define TokColon	0x0025
#define TokLBracket	0x0026
#define TokRBracket	0x0027
#define TokCarat	0x0028
#define TokCaratEq	0x0029
#define TokOr		0x002A
#define TokOrOr 	0x002B
#define TokOrEq 	0x002C
#define TokTilde	0x002D
#define TokLBrace	0x002E
#define TokRBrace	0x002F
#define TokSemi 	0x0030
#define TokSizeof	0x0031
#define TokDotDotDot	0x0032
#define TokTypeof	0x0033

					/*  EXPRESSION TERMINATORS  */
#define TokIntConst	0x1001
#define TokStrConst	0x1002
#define TokFltConst	0x1003
#define TokVarId	0x1004
#define TokLabelId	((short)(0x1006|TOKF_PRIVATE))
#define TokEnumConst	0x1007

					/*  SYMBOL		    */
#define TokId		0x2000

#define TokTypeQual	0x3000		/*  special		    */
#define TokRegQual	0x3001

#define TokBreak	0x4001
#define TokCase 	0x4002
#define TokContinue	0x4003
#define TokDefault	0x4004
#define TokDo		0x4005
#define TokElse 	0x4006
#define TokFor		0x4007
#define TokGoto 	0x4008
#define TokIf		0x4009
#define TokReturn	0x400A
#define TokStruct	0x400B
#define TokSwitch	0x400C
#define TokUnion	0x400D
#define TokWhile	0x400E
#define TokTypeDef	0x400F
#define TokEnum 	0x4010
#define TokBreakPoint	0x4011


					/*  TYPES	*/
#define TokTypeId	0x5001

/*
 *  0x60nn is reserved
 */

#define TokCppRef1_Byte  0x86
#define TokCppRef2_Byte  0x87

/*
 *  Dummy tokens
 */

#define TokBlock	0x7000
#define TokExp		0x7001
#define TokDecl 	0x7002
#define TokExpAssBlock	0x7003
#define TokAddr 	0x7004
#define TokVarRef	0x7005
#define TokPreInc	0x7006
#define TokPreDec	0x7007
#define TokPosInc	0x7008
#define TokPosDec	0x7009
#define TokCall 	0x700A
#define TokBFExt	0x700B
#define TokCast 	0x700C

