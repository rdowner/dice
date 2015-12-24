/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
# 1 "VMake.c" 0
typedef void        *APTR;
typedef long  LONG;
typedef unsigned long ULONG;
typedef unsigned long LONGBITS;
typedef short  WORD;
typedef unsigned short UWORD;
typedef unsigned short WORDBITS;
typedef signed char BYTE;
typedef unsigned char UBYTE;
typedef unsigned char BYTEBITS;
typedef short  RPTR;
typedef unsigned char  *STRPTR;
typedef short  SHORT;
typedef unsigned short USHORT;
typedef short  COUNT;
typedef unsigned short UCOUNT;
typedef ULONG  CPTR;
typedef float  FLOAT;
typedef double  DOUBLE;
typedef short  BOOL;
typedef unsigned char TEXT;
typedef unsigned int size_t;
typedef long  BPTR;
typedef long  BSTR;

struct DiskObject {
   UWORD  do_Magic;
   UWORD  do_Version;
   UBYTE  do_Type;
   char *  do_DefaultTool;
   char **  do_ToolTypes;
   LONG  do_CurrentX;
   LONG  do_CurrentY;
   struct DrawerData * do_DrawerData;
   char *  do_ToolWindow;
   LONG  do_StackSize;
 };

struct Node {
   struct  Node *ln_Succ;
   struct  Node *ln_Pred;
   UBYTE   ln_Type;
   BYTE    ln_Pri;
   char    *ln_Name;
};

struct List {
   struct  Node *lh_Head;
   struct  Node *lh_Tail;
   struct  Node *lh_TailPred;
   UBYTE   lh_Type;
   UBYTE   l_pad;
};

struct MsgPort {
   struct  Node mp_Node;
   UBYTE   mp_Flags;
   UBYTE   mp_SigBit;
   void   *mp_SigTask;
   struct  List mp_MsgList;
};

struct Message {
   struct  Node mn_Node;
   struct  MsgPort *mn_ReplyPort;
   UWORD   mn_Length;
};

struct WBStartup {
   struct Message sm_Message;
   struct MsgPort * sm_Process;
   BPTR  sm_Segment;
   LONG  sm_NumArgs;
   char *  sm_ToolWindow;
   struct WBArg * sm_ArgList;
};

struct WBArg {
   BPTR   wa_Lock;
   BYTE  *wa_Name;
};

struct DateStamp {
   LONG   ds_Days;
   LONG   ds_Minute;
   LONG   ds_Tick;
};

struct FileInfoBlock
{
   LONG   fib_DiskKey;
   LONG   fib_DirEntryType;
   char   fib_FileName[108];
   LONG   fib_Protection;
   LONG   fib_EntryType;
   LONG   fib_Size;
   LONG   fib_NumBlocks;
   struct DateStamp fib_Date;
   char   fib_Comment[80];
   char   fib_Reserved[36];
};

LONG Examine( BPTR lock, struct FileInfoBlock *fileInfoBlock );
extern char *strdup(const char *);
extern int stricmp(const char *, const char *);
extern size_t strlen(const char *);
extern char *strcpy(char *, const char *);

int wbmain(wbs)
struct WBStartup *wbs;
{
   struct DiskObject *dob;
   int i;

   InitialFileName = strdup(wbs->sm_ArgList[i].wa_Name);
   dob = GetDiskObject(wbs->sm_ArgList[i].wa_Name);
}

int main(int argc, char **argv)
{
    char appwinbuff[256];
    char *exttext;
    int extlen;
    int argnmln;
    struct WBArg *argptr;
    BYTE *argname;
    char *p;
    int l;

    if ((argnmln = strlen(argname)) > extlen);
    if (!stricmp(argname + argnmln - extlen, exttext));
    if (Examine(argptr->wa_Lock, &vmfib));
    strcpy(appwinbuff, "/");
    if (Examine(argptr->wa_Lock, &vmfib));
    if (strlen(argptr->wa_Name) < (255-l))
        strcpy(p, argptr->wa_Name);
}
