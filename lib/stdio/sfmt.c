
/*
 *  SFMT.C	*scanf()
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  func format same as fgetc: func(desc)
 *  unfunc format same as ungetc: func(c,desc)
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <lib/misc.h>

#define F_SHORT     32
#define F_LONG	    64
#define F_DLONG     128

static char buf2[256];

int
_sfmt(ctl, va, func, unfunc, desc, pcnt)
unsigned char *ctl;
va_list va;
int (*func)(void *);
int (*unfunc)(int, void *);
int *pcnt;
void *desc;
{
    short flags;
    short v;
    short i1;
    int nscanned = 0;
    void *stor;

    *pcnt = 0;
    v = (*func)(desc);
    for (;;) {
	while (*ctl && *ctl != '%') {
	    if (*ctl == ' ' || *ctl == '\t') {
		if (v != ' ' && v != '\t' && v != '\n')
		    goto done;
		while (v == ' ' || v == '\t') {
		    v = (*func)(desc);
		    ++nscanned;
		}
		while (*ctl == ' ' || *ctl == '\t')
		    ++ctl;
	    } else {
		if (v != *ctl)
		    goto done;
		v = (*func)(desc);
		++nscanned;
		++ctl;
	    }
	    if (v == 0)
		goto done;
	}
	if (*ctl++ == 0)
	    goto done;
	if (*ctl == '%') {      /*  match a %   */
	    if (v != '%')
		goto done;
	    v = (*func)(desc);
	    ++nscanned;
	    ++ctl;
	    continue;
	}

	/*
	 *  %[flags][width]c
	 */

	if (*ctl == '*') {
	    stor = NULL;
	    ++ctl;
	} else {
	    stor = va_arg(va, void *);
	}
	i1 = -1;
	if (*ctl >= '0' && *ctl <= '9') {
	    char c = *ctl;
	    i1 = 0;
	    while (c >= '0' && c <= '9') {
		i1 = i1 * 10 + (c - '0');
		c = *++ctl;
	    }
	}
	flags = 0;
	for (;;) {
	    char c = *ctl;
	    if (c == 'h') {
		++ctl;
		flags |= F_SHORT;   /*	only if sizeof(int) != 4 */
		continue;
	    }
	    if (c == 'l') {
		++ctl;
		flags |= F_LONG;
		continue;
	    }
	    if (c == 'L') {
		++ctl;
		flags |= F_DLONG;
		continue;
	    }
	    break;
	}
	if (*ctl == 'n') {
	    if (stor) {
		if (flags & F_SHORT)
		    *(short *)stor = nscanned;
		else
		    *(int *)stor = nscanned;
		stor = NULL;	/* don't count %n in retval */
	    }
	} else {
	    nscanned = _sfmtone(ctl, &v, stor, func, desc, flags, i1, nscanned);
	}
	if (*ctl == '[') {
	    ++ctl;
	    if (*ctl == '^')
		++ctl;
	    if (*ctl == ']')
		++ctl;
	    while(*ctl&&(*ctl != ']')&&(*ctl != ' ')&&(*ctl != '\t')&&(*ctl != '\n'))
		++ctl;
	    if (*ctl == 0)
		--ctl;
	}
	++ctl;
	if (nscanned < 0)
	    break;
	if (stor)
	    ++*pcnt;
    }
done:
    if (v != EOF)
	(*unfunc)(v, desc);

    if (nscanned < 0 && *pcnt == 0)
	return(EOF);
    return(0);
}

_sfmtone(ctl, pv, stor, func, desc, flags, i1, nscanned)
char *ctl;	/*  conversion specifier    */
short *pv;	/*  last read value	    */
void *stor;	/*  pointer into storage    */
int (*func)(void *);  /*  fgetc type function	  */
void *desc;	/*  stdio/custom descriptor */
short flags;	/*  conversion flags	    */
short i1;	/*  maximum field width     */
short nscanned;
{
    short v = *pv;
    int len = 0;
    char buf[32];
    char *ptr = buf;
    char c = *ctl;

    if (c != 'c') {     /*  %c is a special case */
	while (v == ' ' || v == '\t' || v == '\n') {
	    v = (*func)(desc);
	    ++nscanned;
	}
    }

    if (v == 0 || v == EOF)
	return(EOF);

    switch(c) {
    case 'c':
	if (i1 == -1)
	    i1 = 1;
	while (v != EOF && v != '\n' && i1) {
	    if (stor)
		*(char *)stor = v;
	    v = (*func)(desc);
	    ++nscanned;
	    if (stor)
		stor = (void *)((char *)stor + 1);
	    --i1;
	}
	break;
    case 'p':   /*  0x format   */
	if (i1 < 0)
	    i1 = 8;
    case 'x':
    case 'X':
    case 'o':
    case 'i':
    case 'd':
    case 'u':
	{
	    long n = 0;
	    short neg = 0;
	    short base = 10;
	    short b1 = i1;

	    if (i1 && v == '-') {
		neg = 1;
		v = (*func)(desc);
		++nscanned;
		--i1;
	    }
	    if (c == 'o')
		base = 8;
	    if (c == 'x' || c == 'X' || c == 'p') {
		base = 16;
		if (i1 && v == '0') {
		    v = (*func)(desc);
		    --i1;
		    ++nscanned;
		    if (i1 && v == 'x' || v == 'X') {
			--i1;
			v = (*func)(desc);
			++nscanned;
		    }
		}
	    }
	    if (i1 && c == 'i' && v == '0') {
		base = 8;
		v = (*func)(desc);
		--i1;
		++nscanned;
		if (i1 && v == 'x' || v == 'X') {
		    base = 16;
		    v = (*func)(desc);
		    --i1;
		    ++nscanned;
		}
	    }
	    while (i1) {
		short vv;

		if (v >= '0' && v <= '9')
		    vv = v - '0';
		else if (v >= 'a' && v <= 'z')
		    vv = v - ('a' - 10);
		else if (v >= 'A' && v <= 'Z')
		    vv = v - ('A' - 10);
		else
		    break;
		if (vv >= base)
		    break;
		n = n * base + vv;
		v = (*func)(desc);
		++nscanned;
		--i1;
	    }
	    if (b1 == i1)
		return(EOF);
	    if (neg)
		n = -n;
	    if (stor) {
		if (flags & F_SHORT)
		    *(short *)stor = n;
		else
		    *(int *)stor = n;
	    }
	}
	break;
#ifdef MATH_SFMT
#include <math/sfmt.c>
#else
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
	/*fprintf(stderr, "*scanf: %%e,E,f,g,G use -lm (mathlib)\n");*/
	break;
#endif
    case 's':
	while (i1 && v != ' ' && v != '\t' && v != '\n' && v != EOF) {
	    if (stor)
		*(char *)stor = v;
	    v = (*func)(desc);
	    ++nscanned;
	    if (stor)
		stor = (void *)((char *)stor + 1);
	    --i1;
	}
	if (stor)
	    *(char *)stor = 0;
	break;
    case '[':
	{
	    char vax;
	    short i;

	    ++ctl;
	    if (*ctl == '^') {
		++ctl;
		setmem(buf2, 256, 1);
		vax = 0;
	    } else {
		setmem(buf2, 256, 0);
		vax = 1;
	    }
	    if (*ctl == ']') {
		++ctl;
		buf2[']'] = vax;
	    }
	    while ((i = (unsigned char)*ctl)&&(i != ']')&&(i != '\t')&&(i != '\n')) {
		buf2[i] = vax;
		++ctl;
	    }
	}
	while (i1 && v != EOF && buf2[v]) {
	    if (stor)
		*(char *)stor = v;
	    v = (*func)(desc);
	    ++nscanned;
	    if (stor)
		stor = (void *)((char *)stor + 1);
	    --i1;
	}
	if (stor)
	    *(char *)stor = 0;
	break;
    default:
	return(-2);
    }
    *pv = v;
    return(nscanned);
}

