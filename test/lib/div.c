
main(ac, av)
char *av[];
{
    int i = atoi(av[1]);
    int j = atoi(av[2]);
    int k;
    long t1, t2;
    long n;


    printf("%08lx / %08lx = %08lx\n", i, j, i / j);
    printf("u%08lx / u%08lx = u%08lx\n", (unsigned)i, (unsigned)j, (unsigned)i / (unsigned)j);

    printf("%08lx %% %08lx = %08lx\n", i, j, i % j);
    printf("u%08lx %% u%08lx = u%08lx\n", (unsigned)i, (unsigned)j, (unsigned)i % (unsigned)j);
    /*
    printf("%ld / %ld = %ld\n", i, j, i / j);
    printf("%lu / %lu = %lu\n", (unsigned)i, (unsigned)j, (unsigned)i / (unsigned)j);

    printf("%ld %% %ld = %ld\n", i, j, i % j);
    printf("%lu %% %lu = %lu\n", (unsigned)i, (unsigned)j, (unsigned)i % (unsigned)j);

    time(&t1);
    for (n = 0; n < 100000; ++n)
	k = i / j;
    time(&t2);
    printf("time for 100000 divisions %d\n", t2 - t1);
    */

}

