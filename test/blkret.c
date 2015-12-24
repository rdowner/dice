
/*
 *  BLKRET.C
 *
 *  test structural returns
 */

#include <stdio.h>

typedef struct TTest {
    char a, b, c;
} TTest;

TTest fubar(TTest, int, volatile int);
TTest xcopy(TTest);

int
main(ac, av)
char *av[];
{
    int i1 = 5;
    TTest x, y;
    int i2 = 6;

    x.a = 1;
    y.a = 0;

    y = xcopy(fubar(x, 23, 46));
    printf("result: %d %d %d\n", y.a, i1, i2);
    return(0);
}

TTest
fubar(x, y, z)
TTest x;
int y;
volatile int z;
{
    y;
    printf("expect 1 got %d %d %d\n", x.a, y, z);
    x.a = 4;
    return(x);
}

TTest
xcopy(x)
TTest x;
{
    x.a = x.a + 4;
    return(x);
}

