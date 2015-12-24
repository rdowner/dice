
/*
 *  FIFO.H
 *
 *  PUBLIC FIFO STRUCTURES AND DEFINES
 */

#ifndef LIBRARIES_FIFO_H
#define LIBRARIES_FIFO_H

#define FIFONAME    "fifo.library"

#define FIFOF_READ	  0x00000100L	  /*  intend to read from fifo	  */
#define FIFOF_WRITE	  0x00000200L	  /*  intend to write to fifo	  */
#define FIFOF_RESERVED	  0xFFFF0000L	  /*  reserved for internal use   */
#define FIFOF_NORMAL	  0x00000400L	  /*  request blocking/sig support*/
#define FIFOF_NBIO	  0x00000800L	  /*  non-blocking IO		  */

#define FIFOF_KEEPIFD	  0x00002000L	  /*  keep fifo alive if data pending */
#define FIFOF_EOF	  0x00004000L	  /*  EOF on close		      */
#define FIFOF_RREQUIRED   0x00008000L	  /*  reader required to exist	  */

#define FREQ_RPEND	1
#define FREQ_WAVAIL	2
#define FREQ_ABORT	3

typedef void *FifoHan;			  /*  returned by OpenFifo()  */

#ifndef IN_LIBRARY

FifoHan OpenFifo(char *, long, long);
void CloseFifo(FifoHan, long);
long ReadFifo(FifoHan, char **, long);
long WriteFifo(FifoHan, char *, long);
void RequestFifo(FifoHan, struct Message *, long);
long BufSizeFifo(FifoHan);

#endif

#endif

