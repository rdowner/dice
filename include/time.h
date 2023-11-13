
/*
 * $VER: time.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef TIME_H
#define TIME_H

#ifndef STDDEF_H
#include <stddef.h>
#endif

typedef unsigned long clock_t;
typedef unsigned long time_t;

struct tm {
    int tm_sec;     /*  0-59    */
    int tm_min;     /*  0-59    */
    int tm_hour;    /*  0-23    */
    int tm_mday;    /*  1-31    */
    int tm_mon;     /*  0-11    */
    int tm_year;    /*  n+1900  */
    int tm_wday;    /*  (sun)0-6*/
    int tm_yday;    /*  0-366   */
    int tm_isdst;   /*  daylight svings time flag */
};

#define CLK_TCK     50
#define CLOCKS_PER_SEC  CLK_TCK

extern char *asctime(const struct tm *);
extern clock_t clock(void);
extern char *ctime(const time_t *);
extern double difftime(time_t, time_t);
extern struct tm *gmtime(const time_t *);
extern struct tm *localtime(const time_t *);
extern time_t mktime(struct tm *);
extern size_t strftime(char *, size_t, const char *, const struct tm *);
extern time_t time(time_t *);

/*
 *  non-standard
 */

extern struct tm *localtime_tm(const time_t *, struct tm *);

#endif


