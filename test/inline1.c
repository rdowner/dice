
#include <protos/exec_protos.h>

main(ac, av)
char *av[];
{
    struct Task *task;

    task = FindTask(av[1]);
    printf("task = %08lx\n", task);
    return(0);
}
