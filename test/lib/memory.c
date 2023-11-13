
/*
 *  MEMORY.C
 *
 *  test movmem, cmpmem, setmem/clrmem
 */

static unsigned char Buf[1024];

main()
{
    short i;
    unsigned char *buf = (unsigned char *)(((long)Buf + 3) & ~3);

    for (i = 0; i < 256; ++i) {
        buf[3] = 'x';
        buf[4+i] = 'x';
        setmem(buf + 4, i, i);
        if (buf[3] != 'x' || buf[4+i] != 'x')
            puts("setmem failed");
        if (i && (buf[4] != (i & 255) || buf[4+i-1] != (i & 255)))
            puts("setmem failed");
    }
    strcpy(buf, "abcdefgh");
    movmem(buf, buf + 1, 4);
    if (strcmp(buf, "aabcdfgh"))
        puts("movmem failed 1");
    strcpy(buf, "abcdefgh");
    movmem(buf+1,buf, 4);
    if (strcmp(buf, "bcdeefgh"))
        puts("movmem failed 2");
    strcpy(buf, "abcdefgh");
    strcpy(buf+512+251, "ijklmnop");
    buf[511] = 23;
    movmem(buf+4, buf+512, 255);
    if (strcmp(buf+512+255, "mnop"))
        puts("movmem failed 3");
    if (strcmp(buf+512, "efgh"))
        puts("movmem failed 4");
    if (buf[511] != 23)
        puts("movmem failed 5");
    puts("test complete");
}

