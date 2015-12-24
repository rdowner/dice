
/*
 *  TEST/MSCONN.C
 */

#include <stdio.h>
#include <ms/fcntl.h>
#include <ms/nextin.h>

main(ac, av)
char *av[];
{
    FILE *fi;
    char buf[256];
    struct next *next;

    if (ac != 3) {
	puts("conn 0 port");
	exit(1);
    }

    printf("MSListen running, calling openprocess\n");
    fi = openprocess(av[2], O_RDWR | O_NDELAY, 512, 0, 0);
    printf("open-process fi %08lx\n", fi);
    if (fi) {
	for (;;) {
	    next = nextinput(1, 0);
	    if (next->status) {
		int n;

		if (next->status != S_DATA)
		    printf("status %d\n", next->status);
		n = read(fileno(fi), buf, 256);
		if (n < 0) {
		    puts("EOF");
		    break;
		}
		write(1, buf, n);
	    }
	    if (WaitForChar(Input(), 60)) {
		if (gets(buf)) {
		    fputs(buf, fi);
		    fputc('\n', fi);
		    fflush(fi);
		} else {
		    break;
		}
	    }
	}
	puts("closing");
	fclose(fi);
	puts("closed");
    }
}

