
/*
 *  TEST/STKJUMP.C
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf x;

extern long _stack_base;

myexit()
{
    longjmp(x, 2);
}

main(ac, av)
char *av[];
{
    int r;

    onbreak(myexit);

    if (r = setjmp(x)) {
        Execute("avail", 0, 0);
        printf("exit code %d\n", r);
        exit(10);
    }
    Execute("avail", 0, 0);
    subroutine(atoi(av[1]));
    return(0);
}

subroutine(n)
{
    char buf[256];

    if (n == 0)
        return;
    printf("Base = %08lx\n", _stack_base);
    subroutine(n - 1);
}

