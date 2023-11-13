
main(ac, av)
char *av[];
{
    short i;

    for (i = 1; i < ac; ++i) {
        int r = chdir(av[i]);
        if (r < 0)
            printf("Can't chdir %s\n", av[i]);
        Execute("cd", 0, 0);
    }
    return(0);
}
