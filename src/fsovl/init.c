/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * INIT.C - initialize DOS handler functions
 */

#include "defs.h"

Prototype void Initialize(char *name);
Prototype void UnInitialize(void);
Prototype MsgPort *FindDosDevice(char *name, short len, BPTR *plock);

Prototype DosList *Dl;
Prototype MsgPort *PktPort;
Prototype MsgPort *AuxPort;
Prototype long    PktPortMask;
Prototype struct DosLibrary *DOSBase;

void MkDevice(char *devName);
void DelDevice(void);

DosList *Dl;
MsgPort *PktPort;
MsgPort *AuxPort;
long    PktPortMask;

void
Initialize(char *name)
{
    Process *proc = FindTask(NULL);

    /*
     *  Initialize port
     */

    PktPort = CreatePort(NULL, 0);
    AuxPort = CreatePort(NULL, 0);
    PktPortMask = 1 << PktPort->mp_SigBit;

    /*
     *  create DOS node
     */

    MkDevice(name);
}

void
UnInitialize()
{
    DelDevice();
    if (PktPort) {
        DeletePort(PktPort);
        PktPort = NULL;
    }
    if (AuxPort) {
        DeletePort(AuxPort);
        AuxPort = NULL;
    }
}

/*
 * note: devName is a CPTR to a BSTR
 */

void
MkDevice(devName)
char *devName;
{
    DosList *dl;
    RootNode *root;
    DosInfo *info;

    Dl = dl = DosAlloc(sizeof(struct DosList));
    dl->dol_Type = DLT_DEVICE;
    dl->dol_Task = PktPort;
    dl->dol_Name = CTOB(devName);

    Forbid();
    root  = (struct RootNode *)DOSBase->dl_Root;
    info  = (struct DosInfo  *)BADDR(root->rn_Info);
    dl->dol_Next = info->di_DevInfo;
    info->di_DevInfo = CTOB(dl);
    Permit();
}

void
DelDevice()
{
    DosList *dl;
    DosInfo *info;
    RootNode *root;
    DosList *dls;
    BPTR    *bpp;

    if (dl = Dl) {
        Forbid();
        root  = (struct RootNode *)DOSBase->dl_Root;
        info  = (struct DosInfo  *)BTOC(root->rn_Info);

        for (bpp = &info->di_DevInfo; dls = BTOC(*bpp); bpp = &dls->dol_Next) {
            if (dls == dl)
                break;
        }
        if (dls == dl) {
            *bpp = dls->dol_Next;
        } else {
            ;
        }
        Permit();
        DosFree(dl);
        Dl = NULL;
    }
}

MsgPort *
FindDosDevice(char *name, short len, BPTR *plock)
{
    DosInfo *info;
    RootNode *root;
    DosList *dls;
    BPTR    *bpp;

    *plock = 0;

    Forbid();
    root  = (struct RootNode *)DOSBase->dl_Root;
    info  = (struct DosInfo  *)BTOC(root->rn_Info);

    for (bpp = &info->di_DevInfo; dls = BTOC(*bpp); bpp = &dls->dol_Next) {
        ubyte *ptr = BTOC(dls->dol_Name);
        if (len == *ptr && strnicmp(name, ptr + 1, len) == 0) {
            if (dls->dol_Type == DLT_DEVICE) {
                break;
            } else if (dls->dol_Type == DLT_VOLUME) {
                break;
            } else if (dls->dol_Type == DLT_DIRECTORY) {
                *plock = DupLockPacket(dls->dol_Lock);
                break;
            }
        }
    }
    Permit();
    if (dls)
        return(dls->dol_Task);
    return(NULL);
}

