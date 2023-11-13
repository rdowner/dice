
/*
 *  CREATETASK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  in-memory:  [memList][stack][Task]
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

typedef struct MemList MemList;
typedef struct Task    Task;

Task *
CreateTask(name, pri, initPC, stackSize)
unsigned char *name;
long pri;
APTR initPC;
unsigned long stackSize;
{
    MemList *memList;
    Task    *task = NULL;
    long    totalSize;

    stackSize = (stackSize + 3) & ~3;
    totalSize = sizeof(MemList) + sizeof(Task) + stackSize;

    if (memList = AllocMem(totalSize, MEMF_PUBLIC|MEMF_CLEAR)) {
        memList->ml_NumEntries = 1;
        memList->ml_ME[0].me_Addr = (APTR)(memList + 1);
        memList->ml_ME[0].me_Length = totalSize - sizeof(MemList);

        task = (struct task *)memList + sizeof(MemList) + stackSize;
        task->tc_Node.ln_Pri  = pri;
        task->tc_Node.ln_Type = NT_TASK;
        task->tc_Node.ln_Name = name;
        task->tc_SPLower = (char *)memList + sizeof(MemList);
        task->tc_SPUpper = (char *)task->tc_SPLower + stackSize;
        task->tc_SPReg   = task->tc_SPUpper;
        NewList(&task->tc_MemEntry);
        AddTail(&task->tc_MemEntry, &memList->ml_Node);
        if (AddTask(task, initPC, NULL) == 0) {
            FreeEntry(memList);
            task = NULL;
        }
    }
    return(task);
}


