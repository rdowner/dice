
#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <stdio.h>

typedef struct CommandLineInterface CLI;

main(ac, av)
char *av[];
{
    struct Process *proc = FindTask(NULL);

    if (ac > 1) {
        FILE *fi = fopen("ram:z", "w");
        long fh;
        long fh2;
        fprintf(fi, "ram:why\n");
        fclose(fi);
        if (fh = Open("ram:z", 1005)) {
            fclose(stderr);
            fh2 = Open("*", 1006);
            puts("ok");
            printf("execute: %d\n", Execute(av[1], fh, fh2));
            Close(fh);
            Close(fh2);
        }
    }

    CLI *cli = (CLI *)((long)proc->pr_CLI << 2);
    if (cli)
        printf("cli return code: %d\n", cli->cli_ReturnCode);
}

