/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Filename: subs.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:48 $
**      $Log: subs.c,v $
 * Revision 30.326  1995/12/24  06:09:48  dice
 * .
 *
 * Revision 30.325  1995/12/24  05:38:15  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:37:39  dice
 * fixed bug in internationalization where fi is garbage on open failure
 *
 * Revision 30.0  1994/06/10  18:04:57  dice
 * .
 *
 * Revision 1.4  1993/11/14  17:21:14  jtoebes
 * Minor cleanup, delete old code, optimize performance for memory allocations.
 * Improve output for type to string conversion.
 *
 * Revision 1.4  1993/11/14  17:21:14  jtoebes
 * Minor cleanup, delete old code, optimize performance for memory allocations.
 * Improve output for type to string conversion.
 *
 *
**/

#include "defs.h"

Prototype int32_t Align(int32_t, int32_t);
Prototype int32_t PowerOfTwo(uint32_t);
Prototype void *zalloc(int32_t);
Prototype void *talloc(int32_t);
Prototype void tclear(void);
Prototype void *zrealloc(void *, int32_t, int32_t, int32_t);
Prototype char *SymToString(Symbol *);
Prototype char *TypeToString(Type *);
Prototype char *TypeToProtoStr(Type *, short);
Prototype void eprintf(short, const char *, ...);
Prototype void veprintf(short, const char *, va_list);
Prototype void AddAuxSub(char *);
Prototype void DumpAuxSubs(void);
#ifdef NOTDEF
Prototype void MarkAreaMunged(int32_t, int32_t);
Prototype int  OffsetMunged(int32_t);
#endif

Prototype int32_t FPStrToInt(Exp *, char *, int);
Prototype char *IntToFPStr(int32_t, int32_t, int32_t *);
Prototype int FltIsZero(Exp *, char *, int);
Prototype int FltIsNegative(char *, int);
Prototype int32_t FPrefix(Exp *, char *, int, char *);
Prototype void StorToTmpFlt(Exp *, Stor *, TmpFlt *);
Prototype void TmpFltToStor(Exp *, TmpFlt *, Stor *);
Prototype void BalanceTmpFlt(TmpFlt *, TmpFlt *);
Prototype void NormalizeTmpFlt(TmpFlt *);
Prototype int  TmpFltMantDiv(uword *, short, uword);
Prototype int  TmpFltMantMul(uword *, short, uword);

Prototype int32_t Internationalize(char *str, int32_t size);
Prototype int LoadLocaleDefs(char *file);

Prototype void NoMem(void);

int32_t
Align(int32_t bytes, int32_t align)
{
    int32_t n = align - (bytes & (align - 1));
    if (n != align)
	bytes += n;
    return(bytes);
}

int32_t
PowerOfTwo(v)
uint32_t v;
{
    short i;

    for (i = 0; v; ++i) {
	if (v & 1) {
	    if (v == 1)
		return((int)i);
	    return(-1);
	}
	v >>= 1;
    }
    return(-1);
}

void *
zalloc(bytes)
int32_t bytes;
{
    static char *Buf;
    static int32_t Bytes;

    ++ZAllocs;

    bytes = (bytes + 7) & ~7;

    if (bytes <= 128)
    {
        if (bytes <= Bytes)
        {
            void *ptr = (void *)Buf;
            Buf += bytes;
            Bytes -= bytes;
            return(ptr);
        }
        else
        {
            void *ptr = malloc(CHUNKSIZE);

            ++ZChunks;

            if (!ptr)
                NoMem();

            setmem((char *)ptr, CHUNKSIZE, 0);
            Buf = (char *)ptr + bytes;
            Bytes = CHUNKSIZE - bytes;
            return(ptr);
        }
    }
    else /* bytes > 128 */
    {
	void *ptr = malloc(bytes);
	if (!ptr)
	    NoMem();
	setmem((char *)ptr, bytes, 0);
	++ZAloneChunks;
	return(ptr);
    }
}

static char Buf1[256];
static void **TBase;
static void **TBuf;
static char *TPtr;
static int32_t TBytes;

void *
talloc(bytes)
int32_t bytes;
{
    ++TAllocs;

    bytes = (bytes + 3) & ~3;

    if (bytes <= 128)
    {
        if (bytes <= TBytes)
        {
            void *ptr = (void *)TPtr;
            TPtr += bytes;
            TBytes -= bytes;
            return(ptr);
        }
        else
        {
            void **ptr = malloc(CHUNKSIZE + sizeof(void *) + sizeof(void *));

            if (!ptr)
                NoMem();

            if (TBuf) {
		*TBuf = (void *)TBase;
		TBase = (void *)TBuf;
            }
	    *ptr = NULL;
            TBuf = ptr;
	    ++ptr;
	    *(int32_t *)ptr = CHUNKSIZE;
	    ++ptr;

            ++TChunks;
            setmem((char *)ptr, CHUNKSIZE, 0);
            TPtr = (char *)ptr + bytes;
            TBytes = CHUNKSIZE - bytes;
            return(ptr);
        }
    }
    else /* bytes > 128 */
    {
	void **ptr = malloc(bytes + sizeof(void *) + sizeof(void *));

	if (!ptr)
	    NoMem();

	*ptr = (void *)TBase;
	TBase = (void *)ptr;
	++ptr;
	*(int32_t *)ptr = bytes;
	++ptr;

	++TAloneChunks;
	setmem((char *)ptr, bytes, 0);
	return(ptr);
    }
}

void
tclear()
{
    void **ptr;

    /*TBytes = 0;*/
    while ((ptr = TBase) != NULL) {
	TBase = (void **)*ptr;
	/*setmem((char *)ptr + sizeof(ptr), ptr[1], 0x81);*/
	free(ptr);
    }
}

void *
zrealloc(ptr, objsize, oldsize, newsize)
void *ptr;
int32_t objsize;
int32_t oldsize;
int32_t newsize;
{
    void *new;

    if (oldsize > newsize)
	return(ptr);
    new = zalloc(newsize * objsize);
    if (oldsize)
	movmem((char *)ptr, (char *)new, oldsize * objsize);
    /*
	if (ptr) zfree(ptr);
     */
    return(new);
}

char *
SymToString(sym)
Symbol *sym;
{
    static char *Buf;
    static int32_t Len;

    if (sym == NULL)
	return("<unnamed>");
    if (sym->Len + 1 > Len) {
	if (Buf)
	    free(Buf);
	Buf = malloc(sym->Len + 16);
	Len = sym->Len + 16;
	if (!Buf)
	    NoMem();
    }
    movmem(sym->Name, Buf, sym->Len);
    Buf[sym->Len] = 0;
    return(Buf);
}


/*
 *  TypeToProtoStr(type)
 *
 *  Used for -mRRY option
 */

char *
TypeToProtoStr(Type *type, short i)
{
    char *ptr = Buf1;

    switch(type->Id) {
    case TID_INT:
	if (type->Flags & TF_UNSIGNED)
	    i += sprintf(ptr + i, "unsigned ");
	if (type->Flags & TF_SIGNED)
	    i += sprintf(ptr + i, "signed ");
	if (type->Flags & TF_CONST)
	    i += sprintf(ptr + i, "const ");
	if (type->Flags & TF_VOLATILE)
	    i += sprintf(ptr + i, "volatile ");
	i += sprintf(ptr + i,
	    (*type->Size == 0) ? "void" :
	    (*type->Size == 1) ? "char" :
	    (*type->Size == 2) ? "short" :
	    (*type->Size == 4) ? "int" : "iunknown"
	);
	break;
    case TID_FLT:
	i += sprintf(ptr + i,
	    (*type->Size == 4) ? "float" :
	    (*type->Size == 8) ? "double" :
	    (*type->Size == 16) ? "long double" : "funknown"
	);
	break;
    case TID_PTR:
    case TID_ARY:
	TypeToProtoStr(type->SubType, i);
	i = strlen(ptr);
	i += sprintf(ptr + i, " *");
	break;
    case TID_PROC:
	i += sprintf(ptr + i, "void");
	break;
    case TID_STRUCT:
	i += sprintf(ptr + i, "struct");
    case TID_UNION:
	if (type->Id == TID_UNION)
	    i += sprintf(ptr + i, "union");

	/*
	 *  find associated struct/union name
	 */

	{
	    Symbol *sym;

	    if ((sym = FindStructUnionTag(type)) != NULL) {
		i += sprintf(ptr + i, " %s", SymToString(sym));
	    } else {
		i += sprintf(ptr + i, " unknown ");
	    }
	}
	break;
    default:
	i += sprintf(ptr + i, "badid%d", type->Id);
	break;
    }
    return(ptr);
}

void
veprintf(short asout, const char *str, va_list va)
{
    va_list tmp_va;

    va_copy(tmp_va, va);
    if (asout) {
	printf(";");
	vprintf(str, tmp_va);
    }
    va_copy(tmp_va, va);
    vfprintf(stderr, str, tmp_va);
    if (ErrorFi) {
	va_copy(tmp_va, va);
	vfprintf(ErrorFi, str, tmp_va);
    }
}

void
eprintf(short asout, const char *str, ...)
{
    va_list va;

    va_start(va, str);
    veprintf(asout, str, va);
    va_end(va);
}

/*
 *
 */

typedef struct NameNode {
    struct NameNode *Next;
    char *ASName;
} NameNode;

NameNode *ASBase;

void
AddAuxSub(name)
char *name;
{
    NameNode *nn;

    for (nn = ASBase; nn; nn = nn->Next) {
	if (strcmp(name, nn->ASName) == 0)
	    return;
    }
    nn = AllocStructure(NameNode);
    nn->ASName = name;
    nn->Next = ASBase;
    ASBase = nn;
}

void
DumpAuxSubs()
{
    NameNode *nn;

    for (nn = ASBase; nn; nn = nn->Next)
	printf("\txref\t__%s\n", nn->ASName);
}

/*
 *  [+/-]nnn.nnnE[+/-]nnn
 *
 *  Convert FP value to integer.  Generate prefix and exponent
 */

int32_t
FPStrToInt(exp, ptr, len)
Exp *exp;
char *ptr;
int len;
{
    char *bp = Buf1;
    int32_t x = FPrefix(exp, ptr, len, bp); /*  convert to prefix and exponent */
    int32_t v;

    if (x <= 0) 	    /*	too small   */
	return(0);
    if (x > sizeof(Buf1))   /*	too large   */
	yerror(exp->ex_LexIdx, EFATAL_FPINT_TOO_LARGE);

    ++bp;		    /*	skip sign   */
    while (*bp && x) {	    /*	skip to actual decimal pt   */
	++bp;
	--x;
    }
    while (x) { 	    /*	zero extend to actual dec pt*/
	*bp++ = '0';
	--x;
    }
    *bp = 0;
    v = atol(Buf1+1);
    if (Buf1[0] != 1)
	v = -v;

    return(v);
}

char *
IntToFPStr(v, isuns, plen)
int32_t v;
int32_t isuns;
int32_t *plen;
{
    if (isuns)
	sprintf(Buf1, "%u", v);
    else
	sprintf(Buf1, "%d", v);
    *plen = strlen(Buf1);
    return(strdup(Buf1));
}

int
FltIsZero(exp, ptr, len)
Exp *exp;
char *ptr;
int len;
{
    char *bp = Buf1;

    (void)FPrefix(exp, ptr, len, bp); /*  convert to prefix and exponent */

    ++bp;	/* skip sign   */
    while (*bp) {
	if (*bp != '0')
	    return(0);
	++bp;
    }
    return(1);
}

int
FltIsNegative(ptr, len)
char *ptr;
int len;
{
    if (len && *ptr == '-')
	return(1);
    return(0);
}

/*
 *  Convert an ascii fp representation to prefix form
 *
 *	[.]nnnnnnnnnnnnnnnnnnnnnnn and exponent
 */


int32_t
FPrefix(exp, ptr, len, buf)
Exp *exp;
char *ptr;
char *buf;
int len;
{
    int32_t x = 0; 	    /*	exponent of .prefix */
    short sgn = 1;	    /*	sign of prefix	*/
    short zero= 1;
    short blen = sizeof(Buf1) - 1;

    if (len && *ptr == '+') {
	++ptr;
	--len;
    }
    if (len && *ptr == '-') {
	++ptr;
	--len;
	sgn = -1;
    }

    *buf++ = sgn;	/*  first element is sign 1 = positive	*/
    --blen;

    while (len && blen && *ptr >= '0' && *ptr <= '9') {
	if (zero && *ptr == '0') {
	    ;
	} else {
	    zero = 0;
	    *buf++ = *ptr;
	    --blen;
	    ++x;
	}
	++ptr;
	--len;
    }
    if (len && *ptr == '.') {
	++ptr;
	--len;

	while (len && blen && *ptr >= '0' && *ptr <= '9') {
	    if (zero && *ptr == '0') {
		--x;
	    } else {
		zero = 0;
		*buf++ = *ptr;
		--blen;
	    }
	    ++ptr;
	    --len;
	}
    }
    if (blen == 0)
	yerror(exp->ex_LexIdx, EFATAL_FPSTR_TOO_LONG);
    *buf = 0;

    if (len && (*ptr == 'e' || *ptr == 'E')) {
	int32_t n = 0;
	short nsgn = 1;
	++ptr;
	--len;

	if (len && *ptr == '-') {
	    nsgn = -1;
	    ++ptr;
	    --len;
	}
	if (len && *ptr == '+') {
	    ++ptr;
	    --len;
	}
	while (len) {
	    n = n * 10 + *ptr - '0';
	    ++ptr;
	    --len;
	}
	if (nsgn < 0)
	    x -= n;
	else
	    x += n;
    }
    return(x);
}

/*
 *  Convert ascii fp number into a temporary floating point binary
 *  representation.
 */

void
StorToTmpFlt(exp, s, f)
Exp *exp;
Stor *s;
TmpFlt *f;
{
    char *bp = Buf1;

    /*
     *	bp[0] == -1 if negative, else positive. bp[1..\0] holds digits (ascii)
     *	x is exponent
     */

    f->tf_Exponent = FPrefix(exp, s->st_FltConst, s->st_FltLen, bp);
    f->tf_Negative = (bp[0] == -1) ? 1 : 0;
    {
	int32_t z = 0;
	f->tf_LMantissa[0] = z;
	f->tf_LMantissa[1] = z;
	f->tf_LMantissa[2] = z;
	f->tf_LMantissa[3] = z;
    }

    ++bp;
    while (*bp) {
	int32_t n = *bp - '0';
	if (f->tf_LMantissa[0] > (uint32_t)0xFFFFFFFF / 10 - 10)
	    break;
	TmpFltMantMul(f->tf_WMantissa, 8, 10);
	--f->tf_Exponent;
	f->tf_LMantissa[3] += n;
	if (f->tf_LMantissa[3] < n) {
	    if (++f->tf_LMantissa[2] == 0) {
		if (++f->tf_LMantissa[1] == 0)
		    ++f->tf_LMantissa[0];
	    }
	}
	++bp;
    }

    /*
     *	normalize
     */

    NormalizeTmpFlt(f);
}

/*
 *  Convert a TmpFlt back to a storage structure
 *
 *  Construct ascii rep in Buf1
 */

void
TmpFltToStor(exp, f, s)
Exp *exp;
TmpFlt *f;
Stor *s;
{
    char *bp = Buf1 + sizeof(Buf1) - 16;
    short i;

    sprintf(bp, "E%d", f->tf_Exponent);

    for (i = 0; i < 40; ++i) {	    /*	cvt to 40 digit value	*/
	*--bp = TmpFltMantDiv(f->tf_WMantissa, 8, 10) + '0';
    }
    if (f->tf_Negative)
	*--bp = '-';

    dbprintf(("result = %s\n", bp));
    {
	short len = strlen(bp);
	s->st_FltConst = zalloc(len + 1);
	s->st_FltLen   = len;
	strcpy(s->st_FltConst, bp);
    }
}

/*
 *  Balance two fp numbers so exponents match (used add/sub)
 */

void
BalanceTmpFlt(f1, f2)
TmpFlt *f1;
TmpFlt *f2;
{
    if ((f1->tf_LMantissa[0] | f1->tf_LMantissa[1] | f1->tf_LMantissa[2] | f1->tf_LMantissa[3]) == 0) {
	f1->tf_Exponent = f2->tf_Exponent;
	return;
    }
    if ((f2->tf_LMantissa[0] | f2->tf_LMantissa[1] | f2->tf_LMantissa[2] | f2->tf_LMantissa[3]) == 0) {
	f2->tf_Exponent = f1->tf_Exponent;
	return;
    }
    if (f1->tf_Exponent < f2->tf_Exponent - 40) {
	int32_t z = 0;
	f1->tf_Exponent = f2->tf_Exponent;
	f1->tf_LMantissa[0] = z;
	f1->tf_LMantissa[1] = z;
	f1->tf_LMantissa[2] = z;
	f1->tf_LMantissa[3] = z;
	return;
    }
    if (f2->tf_Exponent < f1->tf_Exponent - 40) {
	int32_t z = 0;
	f2->tf_Exponent = f1->tf_Exponent;
	f2->tf_LMantissa[0] = z;
	f2->tf_LMantissa[1] = z;
	f2->tf_LMantissa[2] = z;
	f2->tf_LMantissa[3] = z;
	return;
    }

    while (f1->tf_Exponent < f2->tf_Exponent) {   /*  10E1, 10E2 -> 1E2, 10E2  */
	TmpFltMantDiv(f1->tf_WMantissa, 8, 10);
	++f1->tf_Exponent;
    }
    while (f1->tf_Exponent > f2->tf_Exponent) {
	TmpFltMantDiv(f2->tf_WMantissa, 8, 10);
	++f2->tf_Exponent;
    }
}

void
NormalizeTmpFlt(f)
TmpFlt *f;
{
    if ((f->tf_LMantissa[0] | f->tf_LMantissa[1] | f->tf_LMantissa[2] | f->tf_LMantissa[3]) == 0) {
	f->tf_Exponent = 0;
	f->tf_Negative = 0;
	return;
    }
    while (f->tf_LMantissa[0] < 0x7FFFFFFF / 10) {
	TmpFltMantMul(f->tf_WMantissa, 8, 10);
	--f->tf_Exponent;
    }
}

int
TmpFltMantDiv(uword *wp, short n, uword v)
{
    short i;
    uint32_t c;
    uint32_t r = 0;

    for (i = 0; i < n; ++i) {
	r <<= 16;
	c = (wp[i] + r) / v;
	r = (wp[i] + r) % v;
	wp[i] = c;
    }
    return(r);
}

int
TmpFltMantMul(uword *wp, short n, uword v)
{
    short i;
    uint32_t c = 0;

    for (i = n - 1; i >= 0; --i) {
	c += wp[i] * v;
	wp[i] = c;
	c >>= 16;
    }
    return(c);
}

void
NoMem()
{
    eprintf(0, "NO MEMORY!\n");
    ExitError(25);
    /*zerror(EFATAL_NO_MEMORY);*/
}

#ifdef NOTDEF

/*
 *  Munged area routines for error tracking
 */

typedef struct MunNode {
    struct MunNode *mn_Next;
    struct MunNode *mn_Prev;
    int32_t    mn_OffBeg;
    int32_t    mn_OffEnd;
} MunNode;

MunNode *MunBase;
MunNode **MunTail = &MunBase;

void
MarkAreaMunged(s, e)
int32_t s, e;
{
    MunNode *mn = zalloc(sizeof(MunNode));

    mn->mn_Next = NULL;
    if (MunTail != &MunBase)
	mn->mn_Prev = (MunNode *)MunTail;
    mn->mn_OffBeg = s;
    mn->mn_OffEnd = e;
    *MunTail = mn;
    MunTail = &mn->mn_Next;
}

int
OffsetMunged(i)
int32_t i;
{
    static MunNode *MunCache;
    MunNode *mn;

    for (mn = MunCache; mn && i < mn->mn_OffBeg; mn = mn->mn_Prev) {
	;
    }

    if (mn == NULL)
	mn = MunBase;

    while (mn && i >= mn->mn_OffEnd)
	mn = mn->mn_Next;

    MunCache = mn;

    if (mn && (i >= mn->mn_OffBeg && i < mn->mn_OffEnd))
	return(1);
    return(0);
}

#endif

#ifdef LATTICE

int
cmpmem(ubyte *s1, ubyte *s2, int32_t n)
{
    while (n) {
	if (*s1 < *s2)
	    return(-1);
	if (*s1 > *s2)
	    return(1);
	--n;
	++s1;
	++s2;
    }
    return(0);
}

#endif

#ifdef COMMERCIAL

typedef struct INatNode {
    struct INatNode *in_Next;
    int32_t	in_Id;
    short	in_Len;
    char	in_Str[4];
} INatNode;

INatNode *InBase;

/*
 * Looks up a string in the preloaded catalog.  The string must match exactly.
 * The <size> argument includes a terminating \0.
 */

int32_t
Internationalize(char *str, int32_t size)
{
    int32_t iidx;
    INatNode *in;

    if (str[size-1] == 0)
	--size;

    if (str[0]) {
	/* fprintf(stderr, "string %s\n", str); */
	for (iidx = 0, in = InBase; in; in = in->in_Next, ++iidx) {
	    /* fprintf(
		stderr, "sizes: %d %d %s\n", size, in->in_Len, in->in_Str); */
	    if (size == in->in_Len && strcmp(str, in->in_Str) == 0)
		return(in->in_Id);
	}
    }
    return(-1);
}

/*
 * Internationalization file used to convert strings into indexes.  The file
 * contains lines of the form:
 *
 * n:String
 *
 * For example:  '4:This is a test'.  Any line not beginning with a number
 * is ignored.  Numbers containing initial 0's, as in: '0004:This is a test'
 * will be properly interpreted in base 10.  The maximum string length is 4K.
 * ';' or '#' are the accepted comment characters
 */

int
LoadLocaleDefs(char *file)
{
    FILE *fi = NULL;
    int r = 0;
    char *ptr;
    char *buf = malloc(4096);

    if (buf && (fi = fopen(file, "r"))) {
    	INatNode **pin = &InBase;
    	INatNode *in;

	r = 1;
	while (fgets(buf, 4096, fi)) {
	    int32_t id = strtol(buf, &ptr, 10);
	    short len;

	    if (*ptr != ':')	/* ignore improperly formatted lines */
		continue;
	    ++ptr;
	    len = strlen(ptr);
	    if (len && ptr[len-1] == '\n')
		--len;
	    ptr[len] = 0;

	    /* note: space for terminating \0 already handled by in_Str field
	     * in structure
	     */

	    in = zalloc(sizeof(INatNode) + len);
	    in->in_Len = len;
	    in->in_Id = id;
	    strcpy(in->in_Str, ptr);
	    /* fprintf(
		stderr,
		"ADD %d %d '%s'\n", in->in_Len,in->in_Id, in->in_Str); */
	    *pin = in;
	    pin = &in->in_Next;
	}
    }
    if (fi)
	fclose(fi);
    if (buf)
	free(buf);
    return(r);
}

#endif

void
filler_subs_c(void)
{
    Assert(1);
}
