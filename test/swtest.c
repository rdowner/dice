
/*
 *  SWTEST.C
 */

#include <stdio.h>
#include <stdlib.h>

#define MAXCASES    100

long CaseAry[MAXCASES];

char *IntType(short, short);

main(ac, av)
char *av[];
{
    short size;
    short xsigned;
    short cases;

    for (;;) {
        size = rand() % 3;          /*  char, short, long           */
        xsigned = rand() & 1;       /*  signed, else unsigned       */
        cases = rand() % MAXCASES;  /*  number of cases, max 100    */

        printf("TEST %d cases for %s\n", cases, IntType(size, xsigned));

        TestSwitch(size, xsigned, cases);
    }
}

TestSwitch(size, xsigned, cases)
short size;
short xsigned;
short cases;
{
    short i, j;
    FILE *fi;

    for (i = 0; i < cases; ++i) {
        long n;
loop:
        n = rand();
        if (size == 0) {        /*  char    */
            if (xsigned)
                n = (char)n;
            else
                n = (unsigned char)n;
        } else if (size == 1) { /*  short   */
            if (xsigned)
                n = (short)n;
            else
                n = (unsigned short)n;
        } else {                /*  long    */
            n = n + rand() << 16;
        }
        for (j = 0; j < i; ++j) {
            if (CaseAry[j] == n)
                goto loop;
        }
        CaseAry[i] = n;
    }

    /*
     *  Generate simple program to test all cases
     */

    fi = fopen("t:x.c", "w");
    fprintf(fi, "%s TestAry[] = {\n\t", IntType(size, xsigned));
    for (i = 0; i < cases; ++i) {
        if (xsigned)
            fprintf(fi, "%d%s", CaseAry[i], (i < cases - 1) ? "," : "");
        else
            fprintf(fi, "%u%s", CaseAry[i], (i < cases - 1) ? "," : "");
    }
    fprintf(fi, "\n};\n\n");

    fprintf(fi, "%s TestIt(%s);\n", IntType(size, xsigned), IntType(size, xsigned));

    fprintf(fi, "main()\n{\n");
    fprintf(fi, "    short i;\n");
    fprintf(fi, "    for (i = 0; i < %d; ++i) {\n", cases);
    fprintf(fi, "        if (TestIt(TestAry[i]) != (%s)TestAry[i]) { \n", IntType(size, xsigned));
    fprintf(fi, "            printf(\"case %%d failed\\n\", TestAry[i]);\n");
    fprintf(fi, "            exit(2);\n");
    fprintf(fi, "        }\n");
    fprintf(fi, "    }\n");
    fprintf(fi, "    return(0);\n");
    fprintf(fi, "}\n\n");

    fprintf(fi, "%s\n", IntType(size, xsigned));
    fprintf(fi, "TestIt(v)\n");
    fprintf(fi, "%s v;\n", IntType(size, xsigned));
    fprintf(fi, "{\n");
    fprintf(fi, "    switch(v) {\n");

    for (i = 0; i < cases; ++i) {
        if (xsigned)
            fprintf(fi, "    case %d:", CaseAry[i]);
        else
            fprintf(fi, "    case %u:", CaseAry[i]);
        fprintf(fi, "return((%s)%d);\n", IntType(size, xsigned), CaseAry[i]);
    }
    fprintf(fi, "    default: printf(\"FAILED %%d\\n\", v); exit(1);\n");
    fprintf(fi, "    }\n");
    fprintf(fi, "    return(0);\n");
    fprintf(fi, "}\n\n");
    fclose(fi);

    {
        long r;

        puts("dcc t:x.c -o t:x");
        r = system("dcc t:x.c -o t:x");
        if (r) {
            printf("Unable to compile!\n");
            exit(20);
        }
        r = system("t:x");
        if (r) {
            printf("FAILURE %s\n   ", IntType(size, xsigned));
            for (i = 0; i < cases; ++i)
                printf(" %d", CaseAry[i]);
            puts("");
            exit(20);
        }
    }
}


char *
IntType(size, xsigned)
short size;
short xsigned;
{
    if (size == 0) {
        if (xsigned)
            return("char");
        else
            return("unsigned char");
    } else if (size == 1) {
        if (xsigned)
            return("short");
        else
            return("unsigned short");
    } else {
        if (xsigned)
            return("long");
        else
            return("unsigned long");
    }
}

