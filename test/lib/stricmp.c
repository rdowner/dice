
main(ac, av)
char *av[];
{
    if (ac != 3) {
        puts("stricmp s1 s2");
        exit(1);
    }
    printf("result %d\n", stricmp(av[1], av[2]));
    printf("result (ni-3) %d\n", strnicmp(av[1], av[2]), 3);
    return(0);
}
