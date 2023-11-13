
main(ac, av)
char *av[];
{
    if (ac != 3) {
        puts("printf string fieldwidth");
        exit(1);
    }
    printf("--%.*s--\n", atoi(av[2]), av[1]);
}

