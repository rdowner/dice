
/*
 *  DC1/SYM.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

typedef struct Symbol {
    struct Symbol *Next;    /*  hash table link                 */
    void    *SemBase;       /*  for semantics junk              */
    const char   *Name;     /*  pointer to unterminated symbol  */
    int32_t    Hv;          /*  hash value                      */
    short   Len;            /*  length of symbol                */
    short   LexId;          /*  currently overloaded id         */
    void    *Data;          /*  currently overloaded data       */
} Symbol;

typedef struct SemInfo {
    struct SemInfo *MasterNext;
    struct SemInfo *Next;
    struct SemInfo **Prev;
    Symbol  *Sym;
    short   LexId;
    short   SemLevel;
    void    *Data;
} SemInfo;

typedef struct StrNode {
    struct StrNode *sn_Next;
    struct StrNode *sn_Prev;
    char    *sn_Ptr;
    int32_t    sn_Len;
} StrNode;

/*
 * SemScan*() functions
 */
#define SSCAN_NAME      0x0001
#define SSCAN_TOP       0x0002
#define SSCAN_ARG       0x0004
#define SSCAN_ERROR     0x0008
#define SSCAN_ALIGN     0x0010

