
/*
 *  fseek file
 */

#include <stdio.h>

static char Buf[8192];

main(ac, av)
char *av[];
{
    FILE *fi;
    char buf[128];
    long i1;
    long i2;
    long r;

    if (ac == 1) {
        puts("fseek file");
        exit(1);
    }
    fi = fopen(av[1], "r+");
    if (fi == NULL) {
        puts("open failed");
        exit(1);
    }
    for (;;) {
        printf("Command -");
        fflush(stdout);
        if (gets(buf) == NULL)
            break;

        switch(buf[0]) {
        case 'r':
            i1 = atoi(buf + 1);
            if (i1 >= 0 && i1 <= sizeof(Buf)) {
                r = fread(Buf, 1, i1, fi);
                printf("r = %d\n", r);
                dump(Buf, r);
            }
            break;
        case 'w':
            i1 = atoi(buf + 1);
            if (i1 >= 0 && i1 <= sizeof(Buf)) {
                setmem(Buf, i1, buf[strlen(buf)-1]);
                r = fwrite(Buf, 1, i1, fi);
                printf("r = %d\n", r);
            }
            break;
        case 's':
            i1 = atoi(buf + 1);
            strtok(buf + 1, " \t");
            i2 = atoi(strtok(NULL, " \t"));
            r = fseek(fi, i1, i2);
            printf("%d = fseek(fi, %d, %d)\n", r, i1, i2);
            break;
        case 't':
            printf("pos = %d\n", ftell(fi));
            break;
        case 'u':
            printf("result = %d\n", ungetc('x', fi));
            break;
        case 'q':
            exit(0);
        default:
            printf("r bytes         -read\n"
                   "w bytes c       -write\n"
                   "s off how       -seek\n"
                   "t               -tell\n"
                   "u               -ungetc\n"
                   "q               -quit\n"
                   "h               -help\n"
            );
            break;
        }
    }
}

dump(ptr, bytes)
unsigned char *ptr;
long bytes;
{
    printf("<");
    while (bytes > 0) {
        printf("%c", *ptr);
        --bytes;
        ++ptr;
    }
    printf(">\n");
}

