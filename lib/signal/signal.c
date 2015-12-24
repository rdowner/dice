/*
 *  SIGNAL.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <clib/exec_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

static void SigIgn(int);
static void SigBye(int);

extern void (*_SigIntFunc)(int);    /*  see amiga/chkabort.c    */

/*
 *  Note: SIGINT is special
 */

static void (*Sigs[NSIG])(int) = {
    SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn,
    SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn,
    SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn,
    SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn, SigIgn
};

static char SigType[NSIG] = { 0,0,0,0,0,0,0,0, /* An array of SIG_DFL trust me */
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0};

long _SigMask;
long _SigRdy;

__sigfunc
signal(signo, func)
int signo;
__sigfunc func;
{
    __sigfunc old;

    if (signo < 0 || signo > 31 || func == SIG_ERR) {
	errno = EINVAL;
	return(SIG_ERR);
    }
    switch(SigType[signo]) {
    case SIG_DFL:
	old = SIG_DFL;
	break;
    case SIG_IGN:
	old = SIG_IGN;
	break;
    default:
	old = (__sigfunc)Sigs[signo];
	break;
    }

    switch(func) {
    case SIG_DFL:
	SigType[signo] = (char)SIG_DFL;
	if (signo == SIGINT) {
	    _SigIntFunc = NULL;
	    Sigs[signo] = SigBye;
	} else {
	    Sigs[signo] = SigIgn;
	}
	if (_SigRdy & (1 << signo))
	    raise(signo);
	break;
    case SIG_IGN:
	SigType[signo] = (char)SIG_IGN;
	if (signo == SIGINT)
	    _SigIntFunc = SigIgn;
	Sigs[signo] = SigIgn;
	_SigRdy &= ~(1 << signo);
	break;
    default:
	SigType[signo] = 99;
	if (signo == SIGINT)
	    _SigIntFunc = func;
	Sigs[signo] = func;
	if (_SigRdy & (1 << signo))
	    raise(signo);
	break;
    }
    return(old);
}

int
raise(signo)
int signo;
{
    __sigfunc func;

    if (_SigMask & (1 << signo)) {
	_SigRdy |= 1 << signo;
	return(0);
    }
    _SigRdy &= ~(1 << signo);

    if (signo == SIGINT) {
	SetSignal(SIGBREAKF_CTRL_C, SIGBREAKF_CTRL_C);  /*  cause break signal */
	chkabort();
	return(0);
    }

    if (signo < 0 || signo > 31) {
	errno = EINVAL;
	return(-1);
    }
    func = Sigs[signo];
    signal(signo, SIG_DFL);

    (*func)(signo);
    return(0);
}

static void
SigIgn(signo)
{
}

static void
SigBye(signo)
{
    exit(EXIT_FAILURE);
}
