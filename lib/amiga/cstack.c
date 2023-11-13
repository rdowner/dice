
/*
 *  CSTACK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Dynamic Stack Allocation.  If this library module is brought it
 *  then dynamic stacking is in effect.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>

extern long _cur_fudge;
extern long _cur_chunk;
extern char *_stk_base;
extern long _stack_fudge;
extern long _stack_chunk;

extern __stkargs void _stk_free(void);
extern __stkargs void _stack_panic(const char *);

/*
 *  _CStackAlloc(&ctlframe)
 */

__stkargs
void *
_CStackAlloc(psf, a5space)
long *psf;
long a5space;
{
    char *new;
    char *tmp;
    long bytes = _stack_chunk;  // minimum stack chunking size
    long copybytes = psf[1] +   // arguments passed by original caller
                     4 +        // return vector by original caller
                     psf[0] +   // registers saved by callee
                     4;         // fp pushed by callee
    if (bytes < copybytes + a5space + 256)
        bytes = copybytes + a5space + 256;

    //printf("bytes=%d copybytes=%d a5space=%d\n", bytes, copybytes, a5space);

    /*
     *  allocate new stack
     */

    while ((new = malloc(bytes + _stack_fudge)) == NULL)
        stack_abort();

    //printf("newstack %08lx-%08lx\n", new, bytes + _stack_fudge);

    /*
     *  alloc space off the top of the new space to copy the original
     *  procedural arguments, return vector, callee's saved registers,
     *  and A5.
     */

    tmp = new + bytes + _stack_fudge;
    tmp -= copybytes;   // caller args, rts vec, callee regs, callee fp
    movmem((char *)psf + 8, tmp, copybytes);

    /*
     *  setup the return vector on the new stack to point to stk_free,
     *  setup the return FP to point to the real return FP on the original
     *  stack.  The real return FP on the original stack is moved to the
     *  top of the saved registers on the original stack.
     */

    extern void main();
    //printf("returnvector %08lx main %08lx (%08lx)\n",
    //    *(long *)(tmp + 4 + psf[0]),
    //    main,
    //    *(long *)(tmp + 4 + psf[0]) - (long)main,
    //);

    *(long *)(tmp + 4 + psf[0]) = (long)_stk_free;
    *(long *)(tmp) = (long)((char *)psf + 8 + psf[0]);
    *(long *)((char *)psf + 8 + psf[0]) = psf[2];

    /*
     *  save old stack information
     */

    ((long *)new)[0] = 0;       // currently unused field.
    ((long *)new)[1] = (long)_stk_base;
    ((long *)new)[2] = _cur_fudge;
    ((long *)new)[3] = bytes;

    _cur_fudge = _stack_fudge;
    _cur_chunk = _stack_chunk;
    _stk_base = new + _cur_fudge;
    //printf("return %08lx\n", tmp);
    return(tmp);
}

/*
 *  CStackFree is called to free up the current stack frame.  We have
 *  already been moved back to the old stack.
 */

__stkargs
void
_CStackFree()
{
    char *old = _stk_base - _cur_fudge;
    void x;

    if ((char *)&x >= old && (char *)&x < old + _cur_fudge + _cur_chunk)
        _stack_panic("badstkptr");
    if (_cur_chunk == 0)
        _stack_panic("chksz0");

    //printf("restore frame %08lx\n", ((long *)old)[1]);

    _stk_base = (char *)((long *)old)[1];
    _cur_fudge  = ((long *)old)[2];
    _cur_chunk  = ((long *)old)[3];

    free(old);
}

