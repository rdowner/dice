
/*
 *  SIGNAL/SIGMASK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <signal.h>
#include <dos/dos.h>
#include <lib/unix.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

extern long _SigMask;
extern long _SigRdy;

void _SignalReadySigs(void);

sigpause(sigMask)
long sigMask;
{
    long oldmask = _SigMask;

    _SigMask = sigMask;

    for (;;) {
        if (sigcheckchld())
            _SigRdy |= 1 << SIGCHLD;

        if ((_SigRdy & ~_SigMask) == 0) {
            long mask = Wait(SIGF_SINGLE | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
            if (mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)) {
                amiga_vfork_sigall(mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D));
                _SigRdy |= 1 << SIGINT;
            }
        }
        if (_SigRdy & ~_SigMask)
            break;
    }
    _SignalReadySigs();
    _SigMask = oldmask;
    return(0);
}

sigsetmask(mask)
{
    _SigMask = mask;
    _SignalReadySigs();
}

sigblock(mask)
long mask;
{
    long old = _SigMask;

    _SigMask |= mask;
    return(old);
}

void
_SignalReadySigs()
{
    long rdyMask;
    short i;

    if (rdyMask = _SigRdy & ~_SigMask) {
        for (i = 0; i < 32; ++i) {
            if (rdyMask & (1 << i))
                raise(i);
        }
    }
}

