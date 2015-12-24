
main(ac, av)
char *av[];
{
    long r;

    r = system(av[1]);
    printf("result = %d\n", r);
    return(0);
}
