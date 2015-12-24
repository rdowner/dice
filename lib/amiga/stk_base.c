
/*
 *  STK_BASE.C
 *
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__near char *_stk_base = NULL;
long _cur_fudge = 0;
long _cur_chunk = 0;

extern long _stack_fudge;
extern long _stack_chunk;
extern long _ExitSP;

/*
 *  Calculate base of stack and add fudge factor.  This code is brought
 *  in if _stk_base is referenced, either by stack checking code or
 *  by setjmp()/longjmp() code, which is why it isn't bundled with cstack.c
 *
 *  Under 1.3, there is no meaningful stack information, so we make a good
 *  guess based on the stack pointer on program entry.
 */

static __autoinit void
cstack_init()
{
    struct Task *task = FindTask(NULL);
    long fudge = _stack_fudge;
    void dummy;

    if (&dummy < (void *)task->tc_SPLower || &dummy > (void *)task->tc_SPUpper) {
	struct CommandLineInterface *cli = (void *)BADDR(((struct Process *)task)->pr_CLI);
	_stk_base = (char *)(_ExitSP - (long)cli->cli_DefaultStack * 4 + fudge + 128);
    } else {
	_stk_base = (char *)task->tc_SPLower + fudge;
    }
    _cur_chunk = 0;
    _cur_fudge = fudge;
}

__stkargs void
_stack_panic(str)
const char *str;
{
    long fh;
    if (fh = Output()) {
	Write(fh, "stack_panic: ", 13);
	Write(fh, str, strlen(str));
	Write(fh, "\n", 1);
    }
    abort();
}

