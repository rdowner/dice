
/*
 *  getfnl wildcard_path
 */

main(ac, av)
char *av[];
{
    int r;
    int i;
    char **ary;
    static char Buf[8192];

    if (ac == 1) {
	puts("getfnl wildcard_path");
	exit(1);
    }
    r = getfnl(av[1], Buf, sizeof(Buf), 0);
    printf("result %d\n", r);

    if (r > 0) {
	ary = malloc(sizeof(char *) * (r + 1));
	strbpl(ary, r + 1, Buf);
	for (i = 0; i < r; ++i) {
	    printf("%-14s ", ary[i]);
	    if ((i + 1) % 5 == 0)
		puts("");
	}
	puts("");
    }
}

