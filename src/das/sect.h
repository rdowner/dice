
/*
 *  DAS/SECT.H
 *
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdint.h>

#define SECTBLKSIZE     8192    /*  allocation size */

/*
 *  Relocation and section information.  The section is broken up into
 *  relocation lists for byte, word, and int32_t relocations for local and
 *  external symbols.  Each list itself is kept sorted by the SECTION HUNK THE
 *  LOCAL OR EXTERNAL LABEL BELONGS TO.
 *
 *  labels exported from this section are placed on the export list.  Labels
 *  imported into this section are listed via the Reloc lists.
 */

typedef struct Reloc {
    struct Reloc *RNext;
    Label   *Label;             /*  label we are referencing                */
    int32_t    Offset;          /*  offset in section of item to be relocd  */
} Reloc;

typedef struct DBlock {
    struct DBlock *Next;
    int32_t    Size;
    int32_t    Max;
    void    *Data;
} DBlock;

#define RELOC_ABSOLUTE  0x0000
#define RELOC_DATAREL   0x0100
#define RELOC_PCREL     0x0200

#define SECT_DUMMY  0
#define SECT_CODE   1
#define SECT_DATA   2
#define SECT_BSS    3
#define SECT_ABS    4
#define SECT_COMMON 5

typedef struct Sect {
    struct Sect *Next;
    short   Hunk;           /*  starts at 0         */
    short   Type;           /*  code, data, bss, abs*/
    int32_t    HunkMask;
    char    *Name;          /*  section name        */
    int32_t    Addr;        /*  current addr in sect*/

    int32_t    ObjLen;      /*  size of generated code/data         */
    int32_t    DebugLen;            /*  debug entries                       */
    int32_t    DebugIdx;
    DBlock  *Block;         /*  linked list of generated code/data. NULL if bss */
    DBlock  *LastBlock;     /*  for appending...                    */

    Reloc   *RelocAry[10];
    Label   *XDefLab;       /*  exported                */
    struct DebugNode   *DebugAry;
} Sect;

#define RA_EXT  5

#define r_ByteReloc         RelocAry[0]
#define r_WordRelocPc       RelocAry[1]
#define r_LongReloc         RelocAry[2]
#define r_WordDataReloc     RelocAry[3]
#define r_LongRelocPc       RelocAry[4]
#define r_ExtByteReloc      RelocAry[5]
#define r_ExtWordRelocPc    RelocAry[6]
#define r_ExtLongReloc      RelocAry[7]
#define r_ExtWordDataReloc  RelocAry[8]
#define r_ExtLongRelocPc    RelocAry[9]

extern Sect    *CurSection;     /*  current section     */
extern Sect    *SectBase;       /*  list of sections    */
