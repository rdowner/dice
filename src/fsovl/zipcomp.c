/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * ZIPCOMP.C - ZIP COMPRESSION
 */

#include "defs.h"

#define BITS  14        /* compression size */

Prototype int Compress(BPTR fh, char *buf, long bytes);
Prototype void DeCompress(BPTR fh, char *buf, long bytes);

static long ErrorCode;
static long Fh;
static ubyte *MemBuf;
static long  MemBytes;

static ubyte Buf[4096];
static long BufIdx;
static long BufLen;


/*    WritePacket(fh, buf, bytes); */
/*    ReadPacket(fh, buf, bytes);  */

int 
Compress(BPTR fh, char *buf, long bytes)
{
    int r;

    MemBytes = bytes;
    MemBuf = buf;
    BufIdx = 0;
    BufLen = 0;
    ErrorCode = 0;
    Fh = fh;
    r = ZipCompress();
    WritePacket(fh, Buf, BufIdx);
    return(r);
}

unsigned
zfwrite(void *buf, long blkSize, long numBlks, void *fo)
{
    long n = blkSize * numBlks;

    while (n) {
        long r;

        if ((r = sizeof(Buf) - BufIdx) == 0) {
            WritePacket(Fh, Buf, sizeof(Buf));
            BufIdx = 0;
            r = sizeof(Buf);
        }
        if (n < r)
            r = n;
        movmem(buf, Buf + BufIdx, r);
        buf = (void *)((char *)buf + r);
        n -= r;
        BufIdx += r;
    }
    return(numBlks);
}

long
zfread(char *buf, long bytes)
{
    long n = (bytes > MemBytes) ? MemBytes : bytes;

    movmem(MemBuf, buf, n);
    MemBytes -= n;
    MemBuf += n;
    return(n);
}

void 
DeCompress(BPTR fh, char *buf, long bytes)
{
    MemBytes = bytes;
    MemBuf = buf;
    Fh = fh;
    BufIdx = 0;
    BufLen = 0;
    ErrorCode = 0;
    ZipDeCompress(MemBuf, MemBytes);
}

int
ReadByte(short *v)
{
    if (BufIdx == BufLen) {
        BufIdx = 0;
        BufLen = ReadPacket(Fh, Buf, sizeof(Buf));
        if (BufLen == 0)
            return(0);
    }
    *v = Buf[BufIdx++];
    return(8);
}

long
readbuf(ubyte *buf, long n)
{
    long ttl = 0;

    while (n) {
        long r = BufLen - BufIdx;

        if (r == 0) {
            BufIdx = 0;
            BufLen = ReadPacket(Fh, Buf, sizeof(Buf));
            if (BufLen == 0)
                break;
            r = BufLen;
        }
        if (n < r)
            r = n;
        movmem(Buf + BufIdx, buf, r);
        buf += r;
        n -= r;
        BufIdx += r;
        ttl += r;
    }
    return(ttl);
}

