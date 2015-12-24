
/*
 *  test/system.c
 *
 */

main(ac, av)
char *av[];
{
    int r;
    int i;
    int rep = 1;

    if (ac == 1) {
	puts("system cmd");
	exit(1);
    }
    if (ac == 3)
	rep = atoi(av[2]);
    for (i = 0; i < rep; ++i) {
	r = system(av[1]);
	printf("result %d\n", r);
    }
}

