
/*
 *  DATA.C
 *
 *  David Berezowski - March/88.
 *  Modified for DICE - May/91	Matthew Dillon
 *
 *  Copyright (c) 1988  Commodore-Amiga, Inc.
 *  (c)Copyright 1991 Matthew Dillon
 */

#include "defs.h"

Prototype const char *CommandTable[];

const char *CommandTable[] ={
	"\377",         /* 00 aRIS reset                        */
	"\377",         /* 01 aRIN initialize                   */
	"\377",         /* 02 aIND linefeed                     */
	"\012\015",     /* 03 aNEL CRLF                         */
	"\377",         /* 04 aRI reverse LF            (NI)    */

			/* 05 aSGR0 normal char set		*/
	"\0335\033-\376\033F",
	"\0334",        /* 06 aSGR3 italics on                  */
	"\0335",        /* 07 aSGR23 italics off                */
	"\033-\001",    /* 08 aSGR4 underline on                */
	"\033-\376",    /* 09 aSGR24 underline off              */
	"\033E",        /* 10 aSGR1 boldface on                 */
	"\033F",        /* 11 aSGR22 boldface off               */
	"\377",         /* 12 aSFC set foreground color (NI)    */
	"\377",         /* 13 aSBC set background color (NI)    */
	"\377",         /* 14 aSHORP0 normal pitch              */
	"\377",         /* 15 aSHORP2 elite on                  */
	"\377",         /* 16 aSHORP1 elite off                 */
	"\377",         /* 17 aSHORP4 condensed fine on         */
	"\377",         /* 18 aSHORP3 condensed fine off        */
	"\033W\001",    /* 19 aSHORP6 enlarge on                */
	"\033W\376",    /* 20 aSHORP5 enlarge off               */

	"\033q\002",    /* 21 aDEN6 shadow print on             */
	"\033q\376",    /* 22 aDEN5 shadow print off            */
	"\033G",        /* 23 aDEN4 double strike on            */
	"\033H",        /* 24 aDEN3 double strike off           */
	"\033x\001",    /* 25 aDEN2 NLQ on                      */
	"\033x\376",    /* 26 aDEN1 NLQ off                     */

	"\377",         /* 27 aSUS2 superscript on              */
	"\377",         /* 28 aSUS1 superscript off             */
	"\377",         /* 29 aSUS4 subscript on                */
	"\377",         /* 30 aSUS3 subscript off               */
	"\377",         /* 31 aSUS0 normalize the line          */
	"\377",         /* 32 aPLU partial line up      (NI)    */
	"\377",         /* 33 aPLD partial line down    (NI)    */

	"\033R\376",    /* 34 aFNT0 Typeface 0                  */
	"\033R\001",    /* 35 aFNT1 Typeface 1                  */
	"\033R\002",    /* 36 aFNT2 Typeface 2                  */
	"\033R\003",    /* 37 aFNT3 Typeface 3                  */
	"\033R\004",    /* 38 aFNT4 Typeface 4                  */
	"\033R\005",    /* 39 aFNT5 Typeface 5                  */
	"\033R\006",    /* 40 aFNT6 Typeface 6                  */
	"\033R\007",    /* 41 aFNT7 Typeface 7                  */
	"\033R\010",    /* 42 aFNT8 Typeface 8                  */
	"\033R\011",    /* 43 aFNT9 Typeface 9                  */
	"\033R\012",    /* 44 aFNT10 Typeface 10                */

	"\033p1",       /* 45 aPROP2 proportional on            */
	"\033p0",       /* 46 aPROP1 proportional off           */
	"\377",         /* 47 aPROP0 proportional clear    (NI) */
	"\377",         /* 48 aTSS set proportional offset (NI) */
	"\033a\376",    /* 49 aJFY5 auto left justify           */
	"\033a\002",    /* 50 aJFY7 auto right justify          */
	"\033a\003",    /* 51 aJFY6 auto full jusitfy           */
	"\033a\376",    /* 52 aJFY0 auto jusity off             */
	"\377",         /* 53 aJFY3 letter space           (NI) */
	"\033a\001",    /* 54 aJFY1 word fill                   */
	"\377",         /* 55 aVERP0 1/8" line spacing          */
	"\377",         /* 56 aVERP1 1/6" line spacing          */
	"\377",         /* 57 aSLPP set form length             */
	"\377",         /* 58 aPERF perf skip n (n > 0)         */
	"\033O",        /* 59 aPERF0 perf skip off              */

	"\377",         /* 60 aLMS set left margin         (NI) */
	"\377",         /* 61 aRMS set right margin        (NI) */
	"\377",         /* 62 aTMS set top margin          (NI) */
	"\377",         /* 63 aBMS set bottom margin       (NI) */
	"\377",         /* 64 aSTBM set T&B margins        (NI) */
	"\377",         /* 65 aSLRM set L&R margins             */
	"\377",         /* 66 aCAM clear margins                */

	"\377",         /* 67 aHTS set horiz tab                */
	"\377",         /* 68 aVTS set vert tab                 */
	"\377",         /* 69 aTBC0 clear horiz tab             */
	"\033D\376",    /* 70 aTBC3 clear all horiz tabs        */
	"\377",         /* 71 aTBC1 clear vert tab              */
	"\377",         /* 72 aTBC4 clear all vert tabs         */
	"\033D\376",    /* 73 aTBCALL clear all h & v tabs      */
			/* 74 aTBSALL set default tabs		*/
"\033D\010\020\030\040\050\060\070\100\110\120\130\140\150\160\170\200\376",

	"\377",         /* 75 aEXTEND extended commands         */
	"\377",         /* 76 aRAW next 'n' chars are raw       */
};

char *ExtendedCharTable[] = {
	" ",                                    /* NBSP*/
	"\033R\007[\033R\\0",                   /* i */
	"c\010|",                               /* c| */
	"\033R\003#\033R\\0",                   /* L- */
	"\033R\005$\033R\\0",                   /* o */
	"\033R\010\\\\\033R\\0",                /* Y- */
	"|",                                    /* | */
	"\033R\002@\033R\\0",                   /* SS */

	"\033R\001~\033R\\0",                   /* " */
	"c",                                    /* copyright */
	"\033S\\0a\010_\033T",                  /* a_ */
	"<",                                    /* << */
	"~",                                    /* - */
	"-",                                    /* SHY */
	"r",                                    /* registered trademark */
	"-",                                    /* - */

	"\033R\001[\033R\\0",                   /* degrees */
	"+\010_",                               /* +_ */
	"\033S\\0002\033T",                     /* 2 */
	"\033S\\0003\033T",                     /* 3 */
	"'",                                    /* ' */
	"u",                                    /* u */
	"P",                                    /* reverse P */
	"\033S\\000.\033T",                     /* . */

	",",                                    /* , */
	"\033S\\0001\033T",                     /* 1 */
	"\033R\001[\033R\\0\010-",              /* o_ */
	">",                                    /* >> */
	"\033S\\0001\033T\010-\010\033S\0014\033T",     /* 1/4 */
	"\033S\\0001\033T\010-\010\033S\0012\033T",     /* 1/2 */
	"\033S\\0003\033T\010-\010\033S\0014\033T",     /* 3/4 */
	"\033R\007]\033R\\0",                   /* upside down ? */

	"A\010`",                               /* `A */
	"A\010'",                               /* 'A */
	"A\010^",                               /* ^A */
	"A\010~",                               /* ~A */
	"\033R\002[\033R\\0",                   /* "A */
	"\033R\004]\033R\\0",                   /* oA */
	"\033R\004[\033R\\0",                   /* AE */
	"C\010,",                               /* C, */

	"E\010`",                               /* `E */
	"\033R\011@\033R\\0",                   /* 'E */
	"E\010^",                               /* ^E */
	"E\010\033R\001~\033R\\0",              /* "E */
	"I\010`",                               /* `I */
	"I\010`",                               /* 'I */
	"I\010^",                               /* ^I */
	"I\010\033R\001~\033R\\0",              /* "I */

	"D\010-",                               /* -D */
	"\033R\007\\\\\033R\\0",                /* ~N */
	"O\010`",                               /* `O */
	"O\010'",                               /* 'O */
	"O\010^",                               /* ^O */
	"O\010~",                               /* ~O */
	"\033R\002\\\\\033R\\0",                /* "O */
	"x",                                    /* x */

	"\033R\004\\\\\033R\\0",                /* 0 */
	"U\010`",                               /* `U */
	"U\010'",                               /* 'U */
	"U\010^",                               /* ^U */
	"\033R\002]\033R\\0",                   /* "U */
	"Y\010'",                               /* 'Y */
	"T",                                    /* Thorn */
	"\033R\002~\033R\\0",                   /* B */

	"\033R\001@\033R\\0",                   /* `a */
	"a\010'",                               /* 'a */
	"a\010^",                               /* ^a */
	"a\010~",                               /* ~a */
	"\033R\002{\033R\\0",                   /* "a */
	"\033R\004}\033R\\0",                   /* oa */
	"\033R\004{\033R\\0",                   /* ae */
	"\033R\001\\\\\033R\\0",                /* c, */

	"\033R\001}\033R\\0",                   /* `e */
	"\033R\001{\033R\\0",                   /* 'e */
	"e\010^",                               /* ^e */
	"e\010\033R\001~\033R\\0",              /* "e */
	"\033R\006~\033R\\0",                   /* `i */
	"i\010'",                               /* 'i */
	"i\010^",                               /* ^i */
	"i\010\033R\001~\033R\\0",              /* "i */

	"d",                                    /* d */
	"\033R\007|\033R\\0",                   /* ~n */
	"\033R\006|\033R\\0",                   /* `o */
	"o\010'",                               /* 'o */
	"o\010^",                               /* ^o */
	"o\010~",                               /* ~o */
	"\033R\002|\033R\\0",                   /* "o */
	":\010-"                                /* :- */

	"\033R\004|\033R\\0",                   /* o/ */
	"\033R\001|\033R\\0",                   /* `u */
	"u\010'",                               /* 'u */
	"u\010^",                               /* ^u */
	"\033R\002}\033R\\0",                   /* "u */
	"y\010'",                               /* 'y */
	"t",                                    /* thorn */
	"y\010\033R\001~\033R\\0"               /* "y */
};

