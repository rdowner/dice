
main(ac, av)
char *av[];
{
    long r;
    long n = 1;

    if (ac == 1) {
	puts("exec_dcc prg [args [repeat]]");
	exit(1);
    }
    if (ac == 4)
	n = atoi(av[3]);
    while (n--) {
	if (ac == 2)
	    r = exec_dcc(av[1], "");
	else
	    r = exec_dcc(av[1], av[2]);
	printf("result %d\n", r);
    }
}

