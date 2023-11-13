
/*
 *  DYNALOAD.H
 */

#define DERR_NOSDMEM    1
#define DERR_SYMNOTFND  2
#define DERR_OBJNOTFND  3
#define DERR_OBJINVALID 4
#define DERR_EXENOTFND  5
#define DERR_EXENOSYM   6
#define DERR_NOSEGLIST  7
#define DERR_EXEINVALID 8
#define DERR_NOMEM      9
#define DERR_FUNCNOFND  10
#define DERR_BADPARAM   11
#define DERR_BADRELOC   12
#define DERR_UNSUPRELOC 13
#define DERR_NODSPTR    14

#define DC_CacheRef     (TAG_USER+0)
#define DC_Executable   (TAG_USER+1)
#define DC_ExecDir      (TAG_USER+2)
#define DC_ExecName     (TAG_USER+3)
#define DC_SearchPath   (TAG_USER+4)
#define DC_StickyCache  (TAG_USER+5)
#define DC_BaseOffset   (TAG_USER+6)
#define DC_BasePtr      (TAG_USER+7)
#define DC_SegList      (TAG_USER+8)
#define DC_ErrorCode    (TAG_USER+9)
#define DC_ObjName      (TAG_USER+10)
#define DC_ErrorVecLen  (TAG_USER+11)
#define DC_ErrorVec     (TAG_USER+12)

#ifndef DYNALOAD

typedef long DynaDesc;

DynaDesc CreateDynaCache(struct TagItem *);
DynaDesc CreateDynaCacheTags(Tag tag1, ...);
long DeleteDynaCache(DynaDesc);
void *DynaLoad(DynaDesc, char *, struct TagItem *);
void *DynaLoadObj(DynaDesc, char *, char *);
void *DynaLoadTags(DynaDesc, char *, Tag tag1, ...);
void FreeDynaErrorVec(char **, long);
void DynaUnLoad(DynaDesc, void *);

#endif
