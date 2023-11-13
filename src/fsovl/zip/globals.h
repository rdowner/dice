
/*
 *  GLOBALS.H   - GLOBALS FOR ZIP AND UNZIP
 *
 *  The union structure is designed so we can share zip/unzip buffer memory
 */

#ifndef WSIZE                   /* max 32K, power of 2  */
#define WSIZE 32768
#endif

#ifdef SMALL_MEM
#define HASH_BITS       13
#define LIT_BUFSIZE     0x2000
#else
#ifdef MEDIUM_MEM
#define HASH_BITS       14
#define LIT_BUFSIZE     0x4000
#else
#define HASH_BITS       15
#define LIT_BUFSIZE     0x8000
#endif
#endif

#define DIST_BUFSIZE    LIT_BUFSIZE

#define HASH_SIZE       (1 << HASH_BITS)
#define HASH_MASK       (HASH_SIZE-1)
#define WMASK           (WSIZE-1)

#define MAX_BITS        13
#define HSIZE           (1 << MAX_BITS)

typedef unsigned char byte;
typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

typedef ush Pos;

union Globals {
    struct {
        uch window[2L*WSIZE];
        Pos prev[WSIZE];
        Pos head[HASH_SIZE];
        uch l_buf[LIT_BUFSIZE];
        ush d_buf[DIST_BUFSIZE];
        char file_outbuf[1024];
    } zip;
    struct {
        short prefix_of[HSIZE+2];
        byte suffix_of[HSIZE+2];
        byte stack[HSIZE+2];
        byte slide[WSIZE];
    } unzip;
};

extern __far union Globals Globals;

#define window          Globals.zip.window
#define prev            Globals.zip.prev
#define head            Globals.zip.head
#define l_buf           Globals.zip.l_buf
#define d_buf           Globals.zip.d_buf
#define file_outbuf     Globals.zip.file_outbuf

#define prefix_of       Globals.unzip.prefix_of
#define suffix_of       Globals.unzip.suffix_of
#define stack           Globals.unzip.stack
#define slide           Globals.unzip.slide

