
/*
 * $VER: ioctl.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef IOCTL_H
#define IOCTL_H

#define IOF_CMD     0x00FF
#define IOF_GET     0x0000
#define IOF_SET     0x1000

#define IOC_READ    1
#define IOC_WRITE   2
#define IOC_DUP     3
#define IOC_SEEK    4
#define IOC_CLOSE   5
#define IOC_CEXEC   6
#define IOC_MODES   7

#define IOC_SIO     8       /*  MS support */

#define IOC_ISATTY  9

#define IOC_SIGNAL      10  /*  signal bit for fd       */
#define IOC_POST_BREAK  11  /*  propogate ^C through fd */
#define IOC_DOMAIN      12  /*  what kind of descriptor are we? */
#define IOC_GETDESC     13  /*  get actual low level desc.      */

#define IOC_MAPADDR 14      /*  request map address (dicecache) */

#define IODOM_AMIGADOS  0   /*  AmigaDOS descriptor     */
#define IODOM_MS        1   /*  multi-stream descriptor */


//extern int ioctl(int, int, void *, void *);
extern int ioctl(int, int, void *);
#endif

