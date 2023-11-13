
/*
 *
 */

#include <math.h>

main(ac, av)
char *av[];
{
    double d1 = atof(av[1]);
    double d2 = atof(av[2]);
    float f1 = d1;
    float f2 = d2;

    if (d1 <= d2)
        puts("d1 <= d2");
    if (d1 < d2)
        puts("d1 < d2");
    if (d1 == d2)
        puts("d1 == d2");
    if (d1 != d2)
        puts("d1 != d2");
    if (d1 > d2)
        puts("d1 > d2");
    if (d1 >= d2)
        puts("d1 >= d2");

    if (d1)
        puts("d1 != 0");
    else
        puts("d1 == 0");
    if (d1 < 0)
        puts("d1 < 0");
    if (d1 <= 0)
        puts("d1 <= 0");
    if (d1 > 0)
        puts("d1 > 0");
    if (d1 >= 0)
        puts("d1 >= 0");

    if (f1 <= f2)
        puts("f1 <= f2");
    if (f1 < f2)
        puts("f1 < f2");
    if (f1 == f2)
        puts("f1 == f2");
    if (f1 != f2)
        puts("f1 != f2");
    if (f1 > f2)
        puts("f1 > f2");
    if (f1 >= f2)
        puts("f1 >= f2");

    if (f1)
        puts("f1 != 0");
    else
        puts("f1 == 0");
    if (f1 < 0)
        puts("f1 < 0");
    if (f1 <= 0)
        puts("f1 <= 0");
    if (f1 > 0)
        puts("f1 > 0");
    if (f1 >= 0)
        puts("f1 >= 0");

    return(0);
}
