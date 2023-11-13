
main(xac, xav)
int   xac;
char *xav[];
{
    int   ac;
    char **av;
    int error = expand_args(xac, xav, &ac, &av);
    int   i;

    printf("error = %d\n", error);
    for (i = 0; i < ac; ++i) {
        printf("Arg %d = %s\n", i, av[i]);
    }
    return(0);
}

