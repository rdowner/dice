
main()
{
    static char b1[] = { "abcda" };
    static char b2[] = { "abcda" };
    static char b3[] = { "xbada" };
    static char b4[] = { "xbadb" };

    if (cmpmem(b1, b2, 4))
        puts("cmpmem failed 1");
    if (cmpmem(b2, b3, 4) >= 0)
        puts("cmpmem failed 2");
    if (cmpmem(b3, b2, 4) <= 0)
        puts("cmpmem failed 3");

    if (cmpmem(b3, b4, 5) >= 0)
        puts("cmpmem failed 4");
    if (cmpmem(b4, b3, 5) <= 0)
        puts("cmpmem failed 5");
    if (cmpmem(b1, b2, 5))
        puts("cmpmem failed 6");
}

