
/*
 *	SUPLIB/MEMORY.H
 */

#define setmem(ptr,len,c) memset(ptr,c,len)
#define movmem(s,d,len)   memcpy(d,s,len)
#define clrmem(d,n)	  memset(d,0,n)
#define cmpmem(s,d,n)	  bcmp(s,d,n)

