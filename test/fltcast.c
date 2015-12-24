
#include <stdio.h>
#include <stdlib.h>

main(ac, av)
char *av[];
{
    float a = atof(av[1]);
    double b = atof(av[1]);
    float c = (float)b;
    double d = (double)a;

    printf("%lf %lf %lf %lf\n", a, b, c, d);

    return(0);
}
