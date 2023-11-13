
/*
 *  Simple program to flash the LED
 *
 */

#include <exec/types.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

main(ac, av)
int ac;
char *av[];
{
    struct Task *task = FindTask(NULL);
    short n = 500;

    /*
     *  actual loop
     */

    puts("\nWatch the Power LED and Press ^C to quit!\n");

    while ((task->tc_SigRecvd & SIGBREAKF_CTRL_C) == 0) {
        short i;
        short j;

        for (i = 0; i < n; ++i) {
            for (j = 0; j < i; ++j)
                *(char *)0xBFE001 |= 0x02;
            for (j = i; j < n; ++j)
                *(char *)0xBFE001 &= ~0x02;
        }
    }
    return(0);
}

