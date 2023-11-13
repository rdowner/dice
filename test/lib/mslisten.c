
/*
 *  MSLISTEN.C
 *
 */

#include <stdio.h>
#include <ms/fcntl.h>

main()
{
    FILE *fi;
    char buf[256];

    printf("MSListen running, calling openport\n");
    fi = openport(O_RDWR, 512);
    printf("open-port fi %08lx\n", fi);
    if (fi) {
        puts("reading til EOF");
        while (fgets(buf, 256, fi)) {
            printf("* ");
            fputs(buf, stdout);
            fflush(stdout);
            fprintf(fi, "accepted\n");
            fflush(fi);
        }
        puts("closing");
        fclose(fi);
        puts("closed");
    }
}

