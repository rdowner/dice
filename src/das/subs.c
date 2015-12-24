/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 *
 */

#include "defs.h"
#include <stdarg.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY    0
#endif

Prototype char	*EAToString(EffAddr *);
Prototype void	syntax(long);
Prototype void	RegMaskToStr(char *, long);
Prototype void	cvtstrtolower(char *);
Prototype int	OnlyOneRegister(long);
Prototype void	cerror(short, ...);
Prototype long	Align(long, long);
Prototype long	AlignDelta(long, long);
Prototype char	*MakeTmpLabelName(void);
Prototype char	ExtWordToReg(uword);
Prototype char *ObtainErrorString(short);
Prototype void NoMemory(void);
Prototype void ExitError(short);

Prototype void eprintf(const char *ctl, ...);
Prototype void veprintf(const char *ctl, va_list va);
Prototype void eputc(char c);

Local char  *DumpRegs(char *, short, char, char);

char	*ErrorFileName = DCC "config/dice.errors";
char	*ErrorAry;
long	ErrorArySize;
char	ErrBuf[128];

char *
EAToString(ea)
EffAddr *ea;
{
    static char Buf[2][128];
    static int Bi;
    char *ptr;
    char rc = (ea->Reg1 >= RB_AREG) ? 'A' : 'D';

    Bi = !Bi;
    ptr = Buf[Bi];

    if (ea->Mode1 == 0)
	return("-");
    switch(ea->Mode1) {
    case AB_DN:
    case AB_AN:
	*ptr++ = rc;
	*ptr++ = (ea->Reg1 & 7) + '0';
	*ptr = 0;
	break;
    case AB_MMIND:
	*ptr++ = '-';
    case AB_INDAN:
    case AB_INDPP:
	*ptr++ = '(';
	*ptr++ = rc;
	*ptr++ = (ea->Reg1 & 7) + '0';
	*ptr++ = ')';
	*ptr = 0;
	if (ea->Mode1 != AB_INDPP)
	    break;
	*ptr++ = '+';
	*ptr = 0;
	break;
    case AB_IMM:
	*ptr++ = '#';
	*ptr = 0;
    case AB_ABSW:
    case AB_ABSL:
    case AB_OFFAN:
    case AB_OFFIDX:
    case AB_OFFPC:
    case AB_OFFIDXPC:
    case AB_BBRANCH:
    case AB_WBRANCH:
	if (ea->Label1) {
	    strcpy(ptr, ea->Label1->Name);
	    ptr += strlen(ptr);
	    if (ea->Offset1)
		strcpy(ptr, "+");
	}
	if (ea->Offset1) {
	    sprintf(ptr, "%d", (int)ea->Offset1);
	    ptr += strlen(ptr);
	}
	switch(ea->Mode1) {
	case AB_OFFAN:
	    sprintf(ptr, "(A%d)", ea->Reg1 - RB_AREG);
	    break;
	case AB_OFFIDX:
	    {
		short reg2 = ExtWordToReg(ea->ExtWord);
		char x2 = (reg2 >= RB_AREG) ? 'A' : 'D';
		char s2 = (ea->ExtWord & EXTF_LWORD) ? 'L' : 'W';

		sprintf(ptr, "(A%d,%c%d.%c)", ea->Reg1 - RB_AREG, x2, reg2 & 7, s2);
	    }
	    break;
	case AB_OFFPC:
	    strcpy(ptr, "(PC)");
	    break;
	case AB_OFFIDXPC:
	    {
		short reg2 = ExtWordToReg(ea->ExtWord);
		char x2 = (reg2 >= RB_AREG) ? 'A' : 'D';
		char s2 = (ea->ExtWord & EXTF_LWORD) ? 'L' : 'W';

		sprintf(ptr, "(PC,%c%d.%c)", x2, reg2 & 7, s2);
	    }
	    break;
	}
	break;
    case AB_REGS:
	RegMaskToStr(ptr, ea->ExtWord);
	break;
    case AB_CCR:
	strcpy(ptr, "CCR");
	break;
    case AB_SR:
	strcpy(ptr, "SR");
	break;
    case AB_USP:
	strcpy(ptr, "USP");
	break;
    default:
	sprintf(ptr, "?%d", ea->Mode1);
	break;
    }
    return(Buf[Bi]);
}

void
syntax(long n)
{
    cerror(EERROR_SYNTAX);
}

char *
DumpRegs(char *ptr, short rbefore, char rs, char re)
{
    char ac = (rs >= RB_AREG) ? 'A' : 'D';

    if (rbefore)
	*ptr++ = '/';
    if (rs < 0)
	return(ptr);
    if (rs == re) {
	*ptr++ = ac;
	*ptr++ = '0' + (rs & 7);
	return(ptr);
    }
    *ptr++ = ac;
    *ptr++ = '0' + (rs & 7);
    *ptr++ = '-';
    *ptr++ = ac;
    *ptr++ = '0' + (re & 7);
    return(ptr);
}

void
RegMaskToStr(ptr, mask)
char *ptr;
long mask;
{
    char *base = ptr;
    short regno = -1;
    short lastreg = -1;
    short i;

    printf("mask %04lx\n", mask);
    for (i = 0; i < 16; ++i) {
	if (i == 8) {
	    ptr = DumpRegs(ptr, ptr != base, regno, lastreg);
	    regno = lastreg = -1;
	}

	if (mask & (1 << i)) {
	    if (regno < 0) {
		regno = lastreg = i;
	    } else {
		if (lastreg == i - 1) {
		    lastreg = i;
		} else {
		    ptr = DumpRegs(ptr, ptr != base, regno, lastreg);
		    regno = lastreg = i;
		}
	    }
	}
    }
    ptr = DumpRegs(ptr, ptr != base, regno, lastreg);
    *ptr = 0;
}

void
cvtstrtolower(str)
char *str;
{
    while (*str) {
	if (*str >= 'A' && *str <= 'Z')
	    *str |= 0x20;
	++str;
    }
}


/*
 *  Returns the regno specified in the mask or -1 if more than one register
 *  or no registers are specified.
 */

int
OnlyOneRegister(long mask)
{
    short i;

    for (i = 0; i < 16; ++i) {
	if (mask & (1 << i)) {
	    mask &= ~(1 << i);
	    if (mask == 0)
		return((int)i);
	    break;
	}
    }
    return(-1);
}

void
cerror(short errorId, ...)
{
    static const char *MsgBase[] = {
	"Error", "SoftError", "Warning", "Note", "Fatal", "FatalSoftError"
    };
    va_list va;

    if ((errorId & EF_MASK) != EF_VERBOSE) {
    	char *nm;
    	long line;

    	nm = AsmFileName;
    	line = CurFile->fn_LineNo;

    	if (DebugLineNo && (SrcFileName != NULL))
    	{
    	   nm = SrcFileName;
    	   line = DebugLineNo;
    	}
	eprintf("DAS: \"%s\" L:%d %.*s%.*s:%d ", nm, line,
            ((ErrorOpt == 2) ? 4 : 0), "C:0 ",
	    ((ErrorOpt == 2) ? 1 : (int)strlen(MsgBase[errorId >> 12])),
	    MsgBase[errorId >> 12],
	    errorId & 0x0FFF
	);
    }

    va_start(va, errorId);
    veprintf(ObtainErrorString(errorId & 0x0FFF), va);
    va_end(va);
    eputc('\n');

    if ((errorId >> 12) >= __FATALPT__) {
	ExitError(20);
    }
    switch(errorId & EF_MASK) {
    case EF_WARN:
    case EF_SOFTWARN:
	if (ExitCode < 5)
	    ExitCode = 5;
	break;
    case EF_ERROR:
	if (ExitCode < 20)
	    ExitCode = 20;
	break;
    }
}

void
ExitError(short code)
{
    if (Fo)
	fclose(Fo);
    if (FoName)
	remove(FoName);
    if (ExitCode < code)
	ExitCode = code;
    exit(ExitCode);
}

long
Align(offset, align)
long offset;
long align;
{
    long n;

    n = offset & (align - 1);
    if (n)
	n = align - n;
    return(offset + n);
}

long
AlignDelta(offset, align)
long offset;
long align;
{
    long n = offset & (align - 1);
    if (n)
	n = align - n;
    return(n);
}

char *
MakeTmpLabelName()
{
    static long id;
    char *buf = malloc(16);

    if (buf == NULL)
	NoMemory();
    sprintf(buf, "&$%ld", id++);
    return(buf);
}

void
NoMemory()
{
    eprintf("NO MEMORY!\n");
    ExitError(25);
}

void
eprintf(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    veprintf(ctl, va);
    va_end(va);
}

void
veprintf(const char *ctl, va_list va)
{
    vfprintf(stderr, ctl, va);
    if (ErrorFi)
	vfprintf(ErrorFi, ctl, va);
}

void
eputc(char c)
{
    fputc(c, stderr);
    if (ErrorFi)
	fputc(c, ErrorFi);
}

char
ExtWordToReg(uword extword)
{
    if (extword & EXTF_AREG)
	return(((extword >> 12) & 7) + RB_AREG);
    return((extword >> 12) & 7);
}

char *
ObtainErrorString(short errNum)
{
    short i;

    if (ErrorAry == NULL) {
	int fd;
	short siz;

	if ((fd = open(ErrorFileName, O_RDONLY|O_BINARY)) < 0) {
	    sprintf(ErrBuf, "(can't open %s!)", ErrorFileName);
	    return(ErrBuf);
	}
	siz = lseek(fd, 0L, 2);
	lseek(fd, 0L, 0);
	ErrorAry = malloc(siz + 1);
	read(fd, ErrorAry, siz);
	close(fd);
	{
	    char *ptr;
	    for (ptr = strchr(ErrorAry, '\n'); ptr; ptr = strchr(ptr + 1, '\n'))
		*ptr = 0;
	}
	ErrorAry[siz] = 0;
	ErrorArySize = siz;
    }
    for (i = 0; i < ErrorArySize; i += strlen(ErrorAry + i) + 1) {
	char *ptr;
	if (ErrorAry[i] == 'A' && ErrorAry[i+1] == 'S' && strtol(ErrorAry + i + 3, &ptr, 10) == errNum)
	    return(ptr + 1);
    }
    sprintf(ErrBuf, "(no entry in %s for error)", ErrorFileName);
    return(ErrBuf);
}



