
#include <sys/types.h>

fd_set	fdr;

main()
{
    int n;
    char buf[256];

    puts("READY");

    for (;;) {
	FD_SET(0, &fdr);
	n = select(1, &fdr, NULL, NULL, NULL);
	printf("RESULT %d\n", n);
	n = read(0, buf, sizeof(buf));
	if (n <= 0)
	    break;
	printf("TEXT: (%d) %*.*s\n", n, n, n, buf);
    }
    puts("DONE");
    return(0);
}

