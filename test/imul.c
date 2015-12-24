
/*
 *
 */

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef unsigned long ulong;

#define TF_VOLATILE  0x01
#define TF_REGISTER  0x02
#define TF_UNSIGNED  0x04

void Operation(short, short, short, short, short, short);
long Truncate(short, short, long);
char *TypeNameOf(short, short);

main()
{
    short i, j, k;

    for (i = 0; i < 3 * 8; ++i) {
	for (j = 0; j < 3 * 8; ++j) {
	    for (k = 0; k < 3 * 8; ++k) {
		Operation(1 << (i % 3), i / 3, 1 << (j % 3), j / 3, 1 << (k % 3), k / 3);
	    }
	}
    }
}

void
Operation(xsize, xflags, ysize, yflags, zsize, zflags)
short xsize, xflags;
short ysize, yflags;
short zsize, zflags;
{
    long x, y, z;
    long r;
    FILE *fi;

    x = Truncate(xsize, xflags, (rand() & 0x1FFFFF) - 0x0FFFFF);
    y = Truncate(ysize, yflags, (rand() & 0x1FFFFF) - 0x0FFFFF);
    z = Truncate(zsize, zflags, x * y);
    printf("(%10s)%-5d * ", TypeNameOf(xsize, xflags), x);
    printf("(%10s)%-5d = ", TypeNameOf(ysize, yflags), y);
    printf("(%10s)%-5d\t", TypeNameOf(zsize, zflags), z, x * y);
    fflush(stdout);

    /*
     *	generate file
     */

    if (fi = fopen("t:im.c", "w")) {
	fprintf(fi, "main() {\n");
	fprintf(fi, "    %s x = %d;\n", TypeNameOf(xsize, xflags), x);
	fprintf(fi, "    %s y = %d;\n", TypeNameOf(ysize, yflags), y);
	fprintf(fi, "    %s z;\n", TypeNameOf(zsize, zflags));
	fprintf(fi, "\n");
	fprintf(fi, "    z = x * y;\n");
	fprintf(fi, "    printf(\"%%d\\n\", z);\n");
	fprintf(fi, "    exit(0);\n");
	fprintf(fi, "}\n");
	fclose(fi);
	printf("test ");
	fflush(stdout);
	if (system("dcc t:im.c -o t:im") == 0) {
	    system("t:im >t:im.dat");
	    if (fi = fopen("t:im.dat", "r")) {
		char buf[256];

		r = 0;
		if (fgets(buf, sizeof(buf), fi)) {
		    sscanf(buf, "%d", &r);
		}
		fclose(fi);
	    }
	    if (r != z)
		printf("**** MISMATCH R=%d ****\n", r);
	    else
		printf("OK\n");
	} else {
	    printf("COMPILER ERROR\n");
	}
    }
}

char *
TypeNameOf(size, flags)
short size;
short flags;
{
    static char Buf[64];

    Buf[0] = 0;
    if (flags & TF_VOLATILE)
	strcat(Buf, " volatile");
    if (flags & TF_REGISTER)
	strcat(Buf, " register");
    if (flags & TF_UNSIGNED)
	strcat(Buf, " unsigned");
    switch(size) {
    case 1:
	strcat(Buf, " char");
	break;
    case 2:
	strcat(Buf, " short");
	break;
    case 4:
	strcat(Buf, " long");
	break;
    }
    if (Buf[0] == ' ')
	movmem(Buf + 1, Buf, strlen(Buf));
    return(Buf);
}

long
Truncate(size, flags, value)
short size;
short flags;
long  value;
{
    if (flags & TF_UNSIGNED) {
	switch(size) {
	case 1:
	    return((ubyte)value);
	case 2:
	    return((uword)value);
	case 4:
	    return((ulong)value);
	}
    } else {
	switch(size) {
	case 1:
	    return((char)value);
	case 2:
	    return((short)value);
	case 4:
	    return((long)value);
	}
    }
    return(0);
}

