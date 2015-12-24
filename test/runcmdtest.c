
/*
 *  Tests bugs in RunCommand() dealing with a RunCommand()'d program
 *  calling RunCommand() itself.
 *
 *
 *
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char Buf[256];
char Name[256];

int
brk()
{
    return(0);
}

main(ac, av)
char *av[];
{
    short n;
    short j;
    long r;
    BPTR seg;

    onbreak(brk);   /*	disable break	*/

    if (ac == 1)
	n = 5;
    else
	n = strtol(av[1], NULL, 0);

    for (j = 0; j < n; ++j) {
	printf("%s %d", av[0], n);
	fflush(stdout);

	sprintf(Buf, "%d\n\r", n - 1);
	if (seg = LoadSeg(av[0])) {
	    if (GetProgramName(Name, sizeof(Name))) {
		if (SetProgramName(Name)) {
		    puts("");
		    r = RunCommand(seg, 8192, Buf, strlen(Buf) - 1);
		    SetProgramName(Name);
		    printf("r = %d", r);
		} else {
		    printf(" setprgname failed");
		}
	    } else {
		printf(" getprgname failed");
	    }
	    UnLoadSeg(seg);
	} else {
	    printf(" ldseg failed");
	}
	puts("");
    }
    return(n + 100);
}

