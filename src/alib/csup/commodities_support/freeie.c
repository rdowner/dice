

#include <devices/inputevent.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>


extern struct Library * __far SysBase;


VOID FreeIEvents(struct InputEvent *ie)
{
struct InputEvent *next;

    while (ie)
    {
        next = ie->ie_NextEvent;
        FreeMem(ie,sizeof(struct InputEvent));
        ie = next;
    }
}
