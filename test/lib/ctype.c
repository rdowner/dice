
main()
{
    short i;

    puts("IsCntrl");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", iscntrl(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsLower");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", islower(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsSpace");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isspace(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("ToLower");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", tolower(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsALNum");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isalnum(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsDigit");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isdigit(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsPrint");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isprint(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsUpper");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isupper(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("ToUpper");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", toupper(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsAlpha");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isalpha(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsGraph");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isgraph(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsPunct");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", ispunct(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
    puts("IsXDigit");
    for (i = -1; i < 256; ++i) {
        printf("%02x ", isxdigit(i));
        if ((i + 1) % 16 == 0)
            puts("");
    }
}
