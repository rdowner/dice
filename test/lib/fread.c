
/*
 *  TEST/FREAD.C
 *
 *  T:x1 T:x2
 */

#include <stdio.h>

struct foo {
    long a;
    short b;
};

main()
{
    struct foo q[2];
    struct foo r[2];
    FILE *fi;
    FILE *fo;
    int n;

    q[0].a = 23;
    q[0].b = 47;
    q[1].a = 24;
    q[1].b = 48;

    fo = fopen("T:x1", "w");
    printf("fo = %08lx\n", fo);
    if (fo) {
	n = fwrite(q, sizeof(q[0]), 2, fo);
	printf("n = %ld\n", n);
	fclose(fo);
    }
    fi = fopen("T:x1", "r");
    printf("fi = %08lx\n", fi);
    if (fi) {
	n = fread(r, sizeof(r[0]), 2, fi);
	printf("n = %ld\n", n);
	fclose(fi);
    }
    printf("%d == %d\n", q[0].a, r[0].a);
    printf("%d == %d\n", q[0].b, r[0].b);
    printf("%d == %d\n", q[1].a, r[1].a);
    printf("%d == %d\n", q[1].b, r[1].b);

    puts("done");
}

