
#include <math.h>

main(ac, av)
char *av[];
{
    double xx = (ac == 1) ? 1234.5678 : atof(av[1]);

    fpconst(xx);
    return(0);
}

fpconst(xx)
double xx;
{
    float f1 = 1.2345678901234567890123456 + 8.7654321098765432109876544;
    float f2 = 23.54;
    float f3 = 23.54 + 10.23;
    double d1 = 1.2345678901234567890123456789 + 8.7654321098765432109876543211;
    double d2 = 23.54;
    double d3 = 23.54 + 10.23;
    short i;

    puts("TESTING FLOATING CONSTANTS");

    printf("%4.8f\t(float const) ~= 10\n", f1);
    printf("%4.8f\t(float const) ~= 23.54\n", f2);
    printf("%4.8f\t(float const) ~= 33.77\n", f3);
    printf("%4.8lf\t(double const) ~= 10\n", d1);
    printf("%4.8lf\t(double const) ~= 23.54\n", d2);
    printf("%4.8lf\t(double const) ~= 33.77\n", d3);

    puts("TESTING (FLOAT)");

    f1 = 1.25;
    f2 = 3.75;
    f3 = 0.25;

    printf("1.25 + 3.75 = 5.00      (%4.8f)\n", f1 + f2);
    printf("1.25 - 3.75 =-2.50      (%4.8f)\n", f1 - f2);
    printf("1.25 * 3.75 = 4.6875    (%4.8f)\n", f1 * f2);
    printf("1.25 / 3.75 = 0.3333    (%4.8f)\n", f1 / f2);
    printf("-1.25       = -1.25     (%4.8f)\n", -f1);
    printf("abs(-1.25)  = 1.25      (%4.8f)\n", ffabs(-f1));
    printf("acos(0.25)  = 1.3181    (%4.8f)\n", facos(f3));
    printf("asin(0.25)  = 0.2527    (%4.8f)\n", fasin(f3));
    printf("atan(0.25)  = 0.2450    (%4.8f)\n", fatan(f3));
    printf("cos(0.25)   = 0.968     (%4.8f)\n", fcos(f3));
    printf("sin(1.25)   = 0.949     (%4.8f)\n", fsin(f1));
    printf("tan(1.25)   = 3.0096    (%4.8f)\n", ftan(f1));
    printf("exp(0.25)   = 1.2840    (%4.8f)\n", fexp(f3));
    printf("ln(3.75)    = 1.3218    (%4.8f)\n", flog(f2));
    printf("log10(3.75) = 0.5740    (%4.8f)\n", flog10(f2));
    printf("1.25^3.75   = 2.3089    (%4.8f)\n", fpow(f1, f2));
    printf("sqrt(3.75)  = 1.9365    (%4.8f)\n", fsqrt(f2));

    puts("TESTING (DOUBLE)");

    d1 = 1.25;
    d2 = 3.75;
    d3 = 0.25;

    printf("1.25 + 3.75 = 5.00      (%4.8f)\n", d1 + d2);
    printf("1.25 - 3.75 =-2.50      (%4.8f)\n", d1 - d2);
    printf("1.25 * 3.75 = 4.6875    (%4.8f)\n", d1 * d2);
    printf("1.25 / 3.75 = 0.3333    (%4.8f)\n", d1 / d2);
    printf("-1.25       = -1.25     (%4.8f)\n", -d1);
    printf("abs(-1.25)  = 1.25      (%4.8f)\n", abs(-d1));
    printf("acos(0.25)  = 1.3181    (%4.8f)\n", acos(d3));
    printf("asin(0.25)  = 0.2527    (%4.8f)\n", asin(d3));
    printf("atan(0.25)  = 0.2450    (%4.8f)\n", atan(d3));
    printf("cos(0.25)   = 0.968     (%4.8f)\n", cos(d3));
    printf("sin(1.25)   = 0.949     (%4.8f)\n", sin(d1));
    printf("tan(1.25)   = 3.0096    (%4.8f)\n", tan(d1));
    printf("exp(0.25)   = 1.2840    (%4.8f)\n", exp(d3));
    printf("ln(3.75)    = 1.3218    (%4.8f)\n", log(d2));
    printf("log10(3.75) = 0.5740    (%4.8f)\n", log10(d2));
    printf("1.25^3.75   = 2.3089    (%4.8f)\n", pow(d1, d2));
    printf("sqrt(3.75)  = 1.9365    (%4.8f)\n", sqrt(d2));

    d1 = xx;
    d3 = 1.0E99;
    printf("CONVERSION TEST ON %4.8lf\n", d1);
    for (i = 0; i < 100; ++i) {
        f1 = d1;
        d1 = f1;
        d2 = xx - d1;
        if (d2 == d3)
            break;
        printf("%4d %1.16le\n", i, d2);
        d3 = d2;
    }
}

