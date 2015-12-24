
void
main(ac, av)
char *av[];
{
    long v1, v2, v;

    v1 = atoi(av[1]);
    v2 = atoi(av[2]);

    printf("%d x %d = %d\n", v1, v2, v1 * v2);
    printf("%08lx x %08lx = %08lx\n", v1, v2, (unsigned)v1 * (unsigned)v2);

    printf("%d.w x %d = %d\n", (short)v1, v2, (short)v1 * v2);
    printf("%d x %d.w = %d\n", v1, (short)v2, v1 * (short)v2);
}

