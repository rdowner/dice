
/*
 * $VER: signal.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SIGNAL_H
#define SIGNAL_H

typedef char sig_atomic_t;
typedef void (*__sigfunc)(int);

typedef void (*sig_t)(int);

struct sigvec {
    void    (*sv_handler)();
    int     sv_mask;
    int     sv_flags;
};

#define sv_onstack sv_flags

#define SA_ONSTACK	0x0001
#define SA_RESTART	0x0002
#define SA_NOCLDSTOP	0x0004

#define SV_ONSTACK	SA_ONSTACK
#define SV_INTERRUPT	SA_RESTART

#define SIG_ERR ((__sigfunc)(-1))
#define SIG_DFL ((__sigfunc)(NULL))
#define SIG_IGN ((__sigfunc)(1))

#define SIGABRT     1
#define SIGFPE	    2
#define SIGILL	    3
#define SIGINT	    4	/*  also static init in signal/signal.c */
#define SIGSEGV     5
#define SIGTERM     6
#define SIGPIPE     7
#define SIGCLD	    8
#define SIGQUIT     9
#define SIGBUS	    10
#define SIGIOT	    11
#define SIGCONT     12
#define SIGKILL     13
#define SIGCHLD     14
#define SIGXCPU     15
#define SIGXFSZ     16
#define SIGTSTP     17
#define SIGTTIN     18
#define SIGTTOU     19
#define SIGSTOP     20
#define SIGHUP	    21

#define NSIG	    32

#define sigmask(signo)	(1 << (signo))

extern __sigfunc signal(int, __sigfunc);
extern int raise(int);

#endif

