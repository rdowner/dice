
/*
 * $VER: math.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef MATH_H
#define MATH_H

#ifndef ERRNO_H
#define EDOM        1           /*  repeated in errno.h */
#define ERANGE      2           /*  repeated in errno.h */
#endif
#define HUGE_VAL    1.0e300

/*
 *  Math Constants
 */

#define M_E             2.7182818284590452354
#define M_LOG2E         1.4426950408889634074
#define M_LOG10E        0.43429448190325182765
#define M_LN2           0.69314718055994530942
#define M_LN10          2.30258509299404568402
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define M_1_PI          0.31830988618379067154
#define M_2_PI          0.63661977236758134308
#define M_2_SQRTPI      1.12837916709551257390
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.70710678118654752440

                            /*  FLOAT MATH ROUTINES */
#ifdef _FFP_FLOAT
#define fexp    _ffpexp
#define ffrexp  _ffpfrexp
#define fldexp  _ffpldexp
#define flog    _ffplog
#define flog10  _ffplog10
#define fmodf   _ffpmodf
#define fcosh   _ffpcosh
#define fsinh   _ffpsinh
#define ftanh   _ffptanh
#define fceil   _ffpceil
#define ffabs   _ffpabs
#define ffloor  _ffpfloor
#define ffmod   _ffpmod
#define fpow    _ffppow
#define fsqrt   _ffpsqrt
#define facos   _ffpacos
#define fasin   _ffpasin
#define fatan   _ffpatan
#define fatan2  _ffpatan2
#define fcos    _ffpcos
#define fsin    _ffpsin
#define ftan    _ffptan
#endif

#ifdef _SP_FLOAT
#define fexp    _spexp
#define ffrexp  _spfrexp
#define fldexp  _spldexp
#define flog    _splog
#define flog10  _splog10
#define fmodf   _spmodf
#define fcosh   _spcosh
#define fsinh   _spsinh
#define ftanh   _sptanh
#define fceil   _spceil
#define ffabs   _spabs
#define ffloor  _spfloor
#define ffmod   _spmod
#define fpow    _sppow
#define fsqrt   _spsqrt
#define facos   _spacos
#define fasin   _spasin
#define fatan   _spatan
#define fatan2  _spatan2
#define fcos    _spcos
#define fsin    _spsin
#define ftan    _sptan
#endif

extern float    fexp(float);
extern float    ffrexp(float, int *);
extern float    fldexp(float, int);
extern float    flog(float);
extern float    flog10(float);
extern float    fmodf(float, float *);

extern float    fcosh(float);
extern float    fsinh(float);
extern float    ftanh(float);

extern float    fceil(float);
extern float    ffabs(float);
extern float    ffloor(float);
extern float    ffmod(float, float);

extern float    fpow(float, float);
extern float    fsqrt(float);

extern float    facos(float);
extern float    fasin(float);
extern float    fatan(float);
extern float    fatan2(float, float);
extern float    fcos(float);
extern float    fsin(float);
extern float    ftan(float);

                                /*  DOUBLE MATH ROUTINES (STANDARD) */

extern double   exp(double);
extern double   frexp(double, int *);
extern double   ldexp(double, int);
extern double   log(double);
extern double   log10(double);
extern double   modf(double, double *);

extern double   cosh(double);
extern double   sinh(double);
extern double   tanh(double);

extern double   ceil(double);
extern double   fabs(double);
extern double   floor(double);
extern double   fmod(double, double);

extern double   pow(double, double);
extern double   sqrt(double);

extern double   acos(double);
extern double   asin(double);
extern double   atan(double);
extern double   atan2(double, double);
extern double   cos(double);
extern double   sin(double);
extern double   tan(double);

#ifndef STDLIB_H
extern double atof(const char *);
extern double strtod(const char *, char **);
#endif


                                /*  LONG DOUBLE MATH ROUTINES   */

extern long double lexp(long double);
extern long double lfrexp(long double, int *);
extern long double lldexp(long double, int);
extern long double llog(long double);
extern long double llog10(long double);
extern long double lmodf(long double, long double *);

extern long double lcosh(long double);
extern long double lsinh(long double);
extern long double ltanh(long double);

extern long double lceil(long double);
extern long double lfabs(long double);
extern long double lfloor(long double);
extern long double lfmod(long double, long double);

extern long double lpow(long double, long double);
extern long double lsqrt(long double);

extern long double lacos(long double);
extern long double lasin(long double);
extern long double latan(long double);
extern long double latan2(long double, long double);
extern long double lcos(long double);
extern long double lsin(long double);
extern long double ltan(long double);

#endif

