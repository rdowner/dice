
/*
 *  PROFILE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <lib/profile.h>
#include <devices/timer.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/execbase.h>

/*
 *  force inline calls under DICE to streamline profiling.  Not required
 *  for prevention of profiling routines getting profiled because this
 *  is handled with __noprof
 */

#define  __DICE_INLINE

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <string.h>

typedef struct CommandLineInterface CLI;
typedef struct timerequest Iot;

ProfSym *_ProfCache = 0;
ProfSym *_ProfList = 0;
ProfSym *_ProfParent = NULL;
Iot     _ProfIot;
long    _ProfTimeStamp;
short   _ProfId = 1;        /*  id 0 reserved for root  */
short   _Prof2_0;           /*  running under 2.0       */

__regargs __noprof void ReadClock13(struct EClockVal *);

__regargs __noprof void
_CProfInit(ps, sysbase)
ProfSym *ps;
struct ExecBase *sysbase;
{
    ps->ps_Link = _ProfList;
    ps->ps_Id = _ProfId++;
    _ProfList = ps;

    if (sysbase->LibNode.lib_Version >= 36)
        _Prof2_0 = 1;
}

/*
 *  Profiler call, keeps track of the amount of time a routine takes to
 *  run (separately for each caller of the routine).  Attempts to remove
 *  overhead related to the profiler call itself.
 *
 *  loops and longjumps.  loops are detected when the parent is equal to
 *  the child while longjumps are detected when an element is not equal
 *  to its parent on exit.
 */

#define EVSHIFT(ev)  (((ev).ev_hi << 28) | ((unsigned long)(ev).ev_lo >> 4))
#define ECBASE(n)   (n >> 4)

/*#define GetEClock(evp) (_Prof2_0 ? (ReadEClock(evp), (evp)->ev_lo = EVSHIFT(*evp)) : ReadClock13(evp))*/
#define GetEClock(evp) if (_Prof2_0) {ReadEClock(evp);(evp)->ev_lo = EVSHIFT(*evp);} else {ReadClock13(evp);}
#define GetEClockBase(evp)  (_Prof2_0 ? ECBASE(ReadEClock(evp)) : 62500)

static short InProf;

__regargs __noprof void
_CProfExec(id)
void *id;
{
    ProfSym *ps;
    short begMode;
    struct EClockVal ev1;

    if (InProf)
        return;
    ++InProf;

    GetEClock(&ev1);

    /*
     *  find program symbol by Id
     */

    begMode = 0;
    {
        ProfSym **pps = &_ProfList;
        short cnt = 0;

        for (ps = *pps; ps; ps = *pps) {
            if (id == ps->ps_BegId) {
                begMode = 1;
                break;
            }
            if (id == ps->ps_EndId)
                break;
            pps = &ps->ps_Link;
            ++cnt;
        }
        if (ps && cnt > 8) {        /*  cache   */
            *pps = ps->ps_Link;
            ps->ps_Link = _ProfList;
            _ProfList = ps;
        }
    }

    if (begMode) {
        ProfSym *ps2;

        /*
         *  begin new entry, close out parent entry then find new entry
         *  point
         */

        if (ps2 = _ProfParent)
            ps2->ps_AccumTime += ev1.ev_lo - ps2->ps_TimeStamp;

        if (ps->ps_NumCalls == 0) {
            ps->ps_Parent = ps2;
        } else if (ps2 = ps) {
            while (ps->ps_Parent != _ProfParent) {
                if (ps->ps_SibLink) {
                    ps = ps->ps_SibLink;
                    continue;
                }
                if (ps->ps_SibLink = AllocMem(sizeof(ProfSym), MEMF_PUBLIC|MEMF_CLEAR)) {
                    ps = ps->ps_SibLink;
                    ps->ps_Size = sizeof(ProfSym);
                    ps->ps_Id = _ProfId++;
                    ps->ps_Parent = _ProfParent;
                    ps->ps_BegId  = ps2->ps_BegId;
                    ps->ps_EndId  = ps2->ps_EndId;
                    break;
                }
            }
        }

        /*
         *  if new entry point found (it had better be!) then set start
         *  point for entry point.
         */

        if (ps) {
            ++ps->ps_NumCalls;
            _ProfParent = ps;
            ps->ps_AccumTime = 0;
            GetEClock(&ev1);
            ps->ps_TimeStamp = ev1.ev_lo;
        }
    } else if (ps = _ProfParent) {
        ProfSym *ps2;

        /*
         *  close out current entry and restart parent entry
         */

        while (ps && ps->ps_EndId != id) {
            ps->ps_AccumTime += ev1.ev_lo - ps->ps_TimeStamp;
            ps->ps_TotalTime += ps->ps_AccumTime;
            if (ps->ps_Parent)
                ps->ps_Parent->ps_AccumTime += ps->ps_AccumTime;
            ps->ps_AccumTime = 0;
            ps = ps->ps_Parent;
        }
        if (ps) {
            ps->ps_AccumTime += ev1.ev_lo - ps->ps_TimeStamp;
            ps->ps_TotalTime += ps->ps_AccumTime;

            if (_ProfParent = ps2 = ps->ps_Parent) {
                ps2->ps_AccumTime += ps->ps_AccumTime;
                GetEClock(&ev1);
                ps2->ps_TimeStamp = ev1.ev_lo;
            }
            ps->ps_AccumTime = 0;
        } else {
            _ProfParent = ps;
        }
    }
    --InProf;
}

__autoexit __noprof void
_CProfExit()
{
    long fh;
    ProfSym *ps;
    ProfSym *ps2;
    ProfHdr ph;
    struct EClockVal ev1;
    char profBuf[64];

    ++InProf;
    profBuf[0] = 'a';
    profBuf[1] = 0;

    GetEClock(&ev1);

    for (ps = _ProfParent; ps; ps = ps->ps_Parent) {
        ps->ps_AccumTime += ev1.ev_lo - ps->ps_TimeStamp;
        ps->ps_TotalTime += ps->ps_AccumTime;
        if (ps->ps_Parent)
            ps->ps_Parent->ps_AccumTime += ps->ps_AccumTime;
        ps->ps_AccumTime = 0;
    }

    {
        CLI *cli = (CLI *)BADDR(((struct Process *)FindTask(NULL))->pr_CLI);
        if (cli) {
            unsigned char *ptr = (char *)BADDR(cli->cli_CommandName);
            short i, j;
            for (i = *ptr; i > 0; --i) {
                if (ptr[i] == ':' || ptr[i] == '/')
                    break;
            }
            for (++i, j = 0; i <= *ptr && j < sizeof(profBuf) - 8; ++i, ++j)
                profBuf[j] = ptr[i];
            profBuf[j] = 0;
        }
    }
    strcat(profBuf, ".dprof");

    ps = _ProfList;
    _ProfList = NULL;

    fh = Open(profBuf, 1006);

    ph.ph_NumIds  = _ProfId;
    ph.ph_Magic   = PROF_MAGIC;
    ph.ph_TimeBase= GetEClockBase(&ev1);
    Write(fh, &ph, sizeof(ph));

    for (; ps; ps = ps->ps_Link) {
        ProfSym *psx;

        for (ps2 = ps; ps2; ps2 = psx) {
            psx = ps2->ps_SibLink;

            ps2->ps_SibLink = NULL;
            if (ps2->ps_Parent)
                ps2->ps_Parent = (void *)ps2->ps_Parent->ps_Id;
            if (fh) {
                if (Write(fh, ps2, ps2->ps_Size) != ps2->ps_Size) {
                    Close(fh);
                    fh = 0;
                }
            }
            ps2->ps_Parent = NULL;

            if (ps2 != ps)
                FreeMem(ps2, ps2->ps_Size);
        }
    }
    if (_ProfIot.tr_node.io_Device) {
        CloseDevice(&_ProfIot.tr_node);
        _ProfIot.tr_node.io_Device = NULL;
    }
    if (fh)
        Close(fh);
}

/*
 *  Under 1.3 we have to use GetSysTime(), which is only accurate to
 *  the vertical blank (about 1/60 second).  To reduce calculations and
 *  allow for roll-over, however, we use microsecond timing with shifts.
 *
 *  This will glitch every 65536 seconds.
 */

__regargs __noprof void
ReadClock13(ev)
struct EClockVal *ev;
{
    if (_ProfIot.tr_node.io_Device == NULL) {
        OpenDevice("timer.device", UNIT_MICROHZ, &_ProfIot.tr_node, 0);
        _ProfIot.tr_node.io_Command = TR_GETSYSTIME;
    }
    DoIO(&_ProfIot.tr_node);
    ev->ev_lo = (unsigned short)_ProfIot.tr_time.tv_secs * (unsigned short)62500 + (_ProfIot.tr_time.tv_micro >> 4);
}


