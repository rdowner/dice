
/*
 *  x prog args
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>

main(ac, av)
char *av[];
{
    union wait istat;
    long pid;
    int fds[2];
    short j;

    for (j = 1; j < ac; ++j) {
	if (av[j][0] == '|') {
	    av[j] = NULL;
	    break;
	}
    }

    if (j == ac) {
	startproc(av + 1, -1, -1);
    } else {
	pipe(fds);
	startproc(av + 1, -1, fds[1]);
	startproc(av + j + 1, fds[0], -1);
    }
    while ((pid = wait(&istat)) != -1)
	printf("PID %d code %d\n", pid, istat.w_retcode);
    return(0);
}

int
startproc(av, infd, outfd)
char **av;
int infd;
int outfd;
{
    int pid;

    if ((pid = amiga_vfork()) == 0) {
	printf("child\n");
	if (infd >= 0)
	    amiga_dup2(infd, 0);
	if (outfd >= 0)
	    amiga_dup2(outfd, 1);
	amiga_execvp(av[0], av);
    } else if (pid < 0) {
	puts("vfork failed");
    }
    return(0);
}

