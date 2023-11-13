/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
# 1 "fails.c" 0

# 1 "defs.h" 1





 

# 1 "local/defs.h" 2



# 1 "dinclude:amiga20/exec/types.h" 3












 

















   
  
 


typedef void           *APTR;        

typedef long            LONG;        
typedef unsigned long   ULONG;       
typedef unsigned long   LONGBITS;    
typedef short           WORD;        
typedef unsigned short  UWORD;       
typedef unsigned short  WORDBITS;    

typedef signed char     BYTE;        

 

typedef unsigned char   UBYTE;       
typedef unsigned char   BYTEBITS;    
typedef short           RPTR;        
typedef unsigned char  *STRPTR;      


 
typedef short           SHORT;       
typedef unsigned short  USHORT;      
typedef short           COUNT;
typedef unsigned short  UCOUNT;
typedef ULONG           CPTR;


 
typedef float           FLOAT;
typedef double          DOUBLE;
typedef short           BOOL;
typedef unsigned char   TEXT;















  
 





# 4 "local/defs.h" 2

# 4 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/execbase.h" 3












 



# 1 "dinclude:amiga20/exec/lists.h" 4












 



# 1 "dinclude:amiga20/exec/nodes.h" 5












 








 

struct Node {
    struct  Node *ln_Succ;       
    struct  Node *ln_Pred;       
    UBYTE   ln_Type;
    BYTE    ln_Pri;              
    char    *ln_Name;            
};       

 
struct MinNode {
    struct MinNode *mln_Succ;
    struct MinNode *mln_Pred;
};






 
 


























# 17 "dinclude:amiga20/exec/lists.h" 4

# 17 "dinclude:amiga20/exec/lists.h" 4




 
struct List {
   struct  Node *lh_Head;
   struct  Node *lh_Tail;
   struct  Node *lh_TailPred;
   UBYTE   lh_Type;
   UBYTE   l_pad;
};       



 
struct MinList {
   struct  MinNode *mlh_Head;
   struct  MinNode *mlh_Tail;
   struct  MinNode *mlh_TailPred;
};       













 









# 17 "dinclude:amiga20/exec/execbase.h" 3

# 17 "dinclude:amiga20/exec/execbase.h" 3




# 1 "dinclude:amiga20/exec/interrupts.h" 4












 










struct Interrupt {
    struct  Node is_Node;
    APTR    is_Data;                 
    void    (*is_Code)();            
};


struct IntVector {               
    APTR    iv_Data;
    void    (*iv_Code)();
    struct  Node *iv_Node;
};


struct SoftIntList {             
    struct List sh_List;
    UWORD  sh_Pad;
};



 





# 21 "dinclude:amiga20/exec/execbase.h" 3

# 21 "dinclude:amiga20/exec/execbase.h" 3




# 1 "dinclude:amiga20/exec/libraries.h" 4












 






 






 





 
 
struct Library {
    struct  Node lib_Node;
    UBYTE   lib_Flags;
    UBYTE   lib_pad;
    UWORD   lib_NegSize;             
    UWORD   lib_PosSize;             
    UWORD   lib_Version;             
    UWORD   lib_Revision;            
    APTR    lib_IdString;            
    ULONG   lib_Sum;                 
    UWORD   lib_OpenCnt;             
};       

 






 













# 25 "dinclude:amiga20/exec/execbase.h" 3

# 25 "dinclude:amiga20/exec/execbase.h" 3




# 1 "dinclude:amiga20/exec/tasks.h" 4












 











 
struct Task {
    struct  Node tc_Node;
    UBYTE   tc_Flags;
    UBYTE   tc_State;
    BYTE    tc_IDNestCnt;            
    BYTE    tc_TDNestCnt;            
    ULONG   tc_SigAlloc;             
    ULONG   tc_SigWait;      
    ULONG   tc_SigRecvd;             
    ULONG   tc_SigExcept;            
    UWORD   tc_TrapAlloc;            
    UWORD   tc_TrapAble;             
    APTR    tc_ExceptData;           
    APTR    tc_ExceptCode;           
    APTR    tc_TrapData;             
    APTR    tc_TrapCode;             
    APTR    tc_SPReg;                
    APTR    tc_SPLower;      
    APTR    tc_SPUpper;      
    void    (*tc_Switch)();          
    void    (*tc_Launch)();          
    struct  List tc_MemEntry;        
    APTR    tc_UserData;             
};

 














 








 
















# 29 "dinclude:amiga20/exec/execbase.h" 3

# 29 "dinclude:amiga20/exec/execbase.h" 3






 
struct ExecBase {
        struct Library LibNode;  

 

        UWORD   SoftVer;         
        WORD    LowMemChkSum;    
        ULONG   ChkBase;         
        APTR    ColdCapture;     
        APTR    CoolCapture;     
        APTR    WarmCapture;     
        APTR    SysStkUpper;     
        APTR    SysStkLower;     
        ULONG   MaxLocMem;       
        APTR    DebugEntry;      
        APTR    DebugData;       
        APTR    AlertData;       
        APTR    MaxExtMem;       

        UWORD   ChkSum;  

 

        struct  IntVector IntVects[16];

 

        struct  Task *ThisTask;  

        ULONG   IdleCount;       
        ULONG   DispCount;       
        UWORD   Quantum;         
        UWORD   Elapsed;         
        UWORD   SysFlags;        
        BYTE    IDNestCnt;       
        BYTE    TDNestCnt;       

        UWORD   AttnFlags;       

        UWORD   AttnResched;     
        APTR    ResModules;      
        APTR    TaskTrapCode;
        APTR    TaskExceptCode;
        APTR    TaskExitCode;
        ULONG   TaskSigAlloc;
        UWORD   TaskTrapAlloc;


 

        struct  List MemList;
        struct  List ResourceList;
        struct  List DeviceList;
        struct  List IntrList;
        struct  List LibList;
        struct  List PortList;
        struct  List TaskReady;
        struct  List TaskWait;

        struct  SoftIntList SoftInts[5];

 

        LONG    LastAlert[4];

        








 
        UBYTE   VBlankFrequency;         
        UBYTE   PowerSupplyFrequency;    

        struct  List SemaphoreList;

        




 
        APTR    KickMemPtr;      
        APTR    KickTagPtr;      
        APTR    KickCheckSum;    

 

        UWORD   ex_Pad0;
        ULONG   ex_Reserved0;
        APTR    ex_RamLibPrivate;
        



 
        ULONG   ex_EClockFrequency;      
        ULONG   ex_CacheControl;         
        ULONG   ex_TaskID;               

        ULONG   ex_PuddleSize;
        ULONG   ex_PoolThreshold;
        struct  MinList ex_PublicPool;

        APTR    ex_MMULock;              

        UBYTE   ex_Reserved[12];
};


 

 














 
 


 















# 5 "local/defs.h" 2

# 5 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/nodes.h" 3












 








 


 
 

 
 
 

 










 
 


























# 6 "local/defs.h" 2

# 6 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/lists.h" 3












 







 






 



 




 













 









# 7 "local/defs.h" 2

# 7 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/libraries.h" 3












 






 






 





 
 




 
 
 
 
 
 
 
 

 






 













# 8 "local/defs.h" 2

# 8 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/ports.h" 3












 














 

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



# 9 "local/defs.h" 2

# 9 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/memory.h" 3












 






 

struct  MemChunk {
    struct  MemChunk *mc_Next;   
    ULONG   mc_Bytes;            
};


 

struct  MemHeader {
    struct  Node mh_Node;
    UWORD   mh_Attributes;       
    struct  MemChunk *mh_First;  
    APTR    mh_Lower;            
    APTR    mh_Upper;            
    ULONG   mh_Free;             
};


 

struct  MemEntry {
union {
    ULONG   meu_Reqs;            
    APTR    meu_Addr;            
    } me_Un;
    ULONG   me_Length;           
};






 

 
struct  MemList {
    struct  Node ml_Node;
    UWORD   ml_NumEntries;       
    struct  MemEntry ml_ME[1];   
};




 
 


















# 10 "local/defs.h" 2

# 10 "local/defs.h" 2

# 1 "dinclude:amiga20/exec/execbase.h" 3












 





















 

 

 

 
 
 
 
 
 
 
 
 
 
 
 
 

 

 



 

 

 
 
 
 
 
 
 

 

 
 







 












 












 
 
 








 
 
 
 

 








 
 
 
 





 





 

 














 
 


 















# 11 "local/defs.h" 2

# 11 "local/defs.h" 2

# 1 "dinclude:amiga20/utility/tagitem.h" 3












 





 
 
 






 

typedef ULONG   Tag;

struct TagItem  {
    Tag         ti_Tag;
    ULONG       ti_Data;
};

 








 




 

 





# 12 "local/defs.h" 2

# 12 "local/defs.h" 2

# 1 "dinclude:amiga20/hardware/custom.h" 3












 










 


struct Custom {
    UWORD   bltddat;
    UWORD   dmaconr;
    UWORD   vposr;
    UWORD   vhposr;
    UWORD   dskdatr;
    UWORD   joy0dat;
    UWORD   joy1dat;
    UWORD   clxdat;
    UWORD   adkconr;
    UWORD   pot0dat;
    UWORD   pot1dat;
    UWORD   potinp;
    UWORD   serdatr;
    UWORD   dskbytr;
    UWORD   intenar;
    UWORD   intreqr;
    APTR    dskpt;
    UWORD   dsklen;
    UWORD   dskdat;
    UWORD   refptr;
    UWORD   vposw;
    UWORD   vhposw;
    UWORD   copcon;
    UWORD   serdat;
    UWORD   serper;
    UWORD   potgo;
    UWORD   joytest;
    UWORD   strequ;
    UWORD   strvbl;
    UWORD   strhor;
    UWORD   strlong;
    UWORD   bltcon0;
    UWORD   bltcon1;
    UWORD   bltafwm;
    UWORD   bltalwm;
    APTR    bltcpt;
    APTR    bltbpt;
    APTR    bltapt;
    APTR    bltdpt;
    UWORD   bltsize;
    UBYTE   pad2d;
    UBYTE   bltcon0l;    
    UWORD   bltsizv;
    UWORD   bltsizh;     
    UWORD   bltcmod;
    UWORD   bltbmod;
    UWORD   bltamod;
    UWORD   bltdmod;
    UWORD   pad34[4];
    UWORD   bltcdat;
    UWORD   bltbdat;
    UWORD   bltadat;
    UWORD   pad3b[3];
    UWORD deniseid;    
    UWORD   dsksync;
    ULONG   cop1lc;
    ULONG   cop2lc;
    UWORD   copjmp1;
    UWORD   copjmp2;
    UWORD   copins;
    UWORD   diwstrt;
    UWORD   diwstop;
    UWORD   ddfstrt;
    UWORD   ddfstop;
    UWORD   dmacon;
    UWORD   clxcon;
    UWORD   intena;
    UWORD   intreq;
    UWORD   adkcon;
    struct  AudChannel {
      UWORD *ac_ptr;  
      UWORD ac_len;      
      UWORD ac_per;      
      UWORD ac_vol;      
      UWORD ac_dat;      
      UWORD ac_pad[2];   
    } aud[4];
    APTR    bplpt[8];
    UWORD   bplcon0;
    UWORD   bplcon1;
    UWORD   bplcon2;
    UWORD   bplcon3;
    UWORD   bpl1mod;
    UWORD   bpl2mod;
    UWORD   bplhmod;
    UWORD   pad86[1];
    UWORD   bpldat[8];
    APTR    sprpt[8];
    struct  SpriteDef {
      UWORD pos;
      UWORD ctl;
      UWORD dataa;
      UWORD datab;
    } spr[8];
    UWORD   color[32];
    UWORD htotal;
    UWORD hsstop;
    UWORD hbstrt;
    UWORD hbstop;
    UWORD vtotal;
    UWORD vsstop;
    UWORD vbstrt;
    UWORD vbstop;
    UWORD sprhstrt;
    UWORD sprhstop;
    UWORD bplhstrt;
    UWORD bplhstop;
    UWORD hhposw;
    UWORD hhposr;
    UWORD beamcon0;
    UWORD hsstrt;
    UWORD vsstrt;
    UWORD hcenter;
    UWORD   diwhigh;     
};



 














 


 






 










# 13 "local/defs.h" 2

# 13 "local/defs.h" 2

# 1 "dinclude:amiga20/hardware/dmabits.h" 3












 

 















 
 




















# 14 "local/defs.h" 2

# 14 "local/defs.h" 2

# 1 "dinclude:amiga20/intuition/intuitionbase.h" 3












 











# 1 "dinclude:amiga20/intuition/intuition.h" 4












 







# 1 "dinclude:amiga20/graphics/gfx.h" 5












 















struct Rectangle
{
    WORD   MinX,MinY;
    WORD   MaxX,MaxY;
};

struct Rect32
{
    LONG    MinX,MinY;
    LONG    MaxX,MaxY;
};

typedef struct tPoint
{
    WORD x,y;
} Point;

typedef UBYTE *PLANEPTR;

struct BitMap
{
    UWORD   BytesPerRow;
    UWORD   Rows;
    UBYTE   Flags;
    UBYTE   Depth;
    UWORD   pad;
    PLANEPTR Planes[8];
};





# 21 "dinclude:amiga20/intuition/intuition.h" 4

# 21 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/graphics/clip.h" 5












 










# 1 "dinclude:amiga20/exec/semaphores.h" 6












 


















 

 
struct SemaphoreRequest {
    struct  MinNode sr_Link;
    struct  Task *sr_Waiter;
};

 
struct SignalSemaphore {
    struct  Node ss_Link;
    WORD    ss_NestCount;
    struct  MinList ss_WaitQueue;
    struct  SemaphoreRequest ss_MultipleLink;
    struct  Task *ss_Owner;
    WORD    ss_QueueCount;
};



 

struct Semaphore {
    struct MsgPort sm_MsgPort;
    WORD   sm_Bids;
};






# 24 "dinclude:amiga20/graphics/clip.h" 5

# 24 "dinclude:amiga20/graphics/clip.h" 5



# 1 "dinclude:amiga20/utility/hooks.h" 6












 









 
struct Hook     {
    struct MinNode      h_MinNode;
    ULONG               (*h_Entry)();    
    ULONG               (*h_SubEntry)(); 
    void                *h_Data;         
};


























































 



# 27 "dinclude:amiga20/graphics/clip.h" 5

# 27 "dinclude:amiga20/graphics/clip.h" 5


 
 



struct Layer
{
    struct  Layer *front,*back;
    struct  ClipRect    *ClipRect;   
    struct  RastPort    *rp;
    struct  Rectangle   bounds;
    UBYTE   reserved[4];
    UWORD   priority;                
    UWORD   Flags;                   
    struct  BitMap *SuperBitMap;
    struct  ClipRect *SuperClipRect;  
                                   
    APTR    Window;                
    WORD   Scroll_X,Scroll_Y;
    struct  ClipRect *cr,*cr2,*crnew;    
    struct  ClipRect *SuperSaveClipRects;  
    struct  ClipRect *_cliprects;        
    struct  Layer_Info  *LayerInfo;      
    struct  SignalSemaphore Lock;
    struct  Hook *BackFill;
    ULONG   reserved1;
    struct  Region *ClipRegion;
    struct  Region *saveClipRects;       
    WORD    Width,Height;                
    UBYTE   reserved2[18];
     
    struct  Region  *DamageList;    
 
};

struct ClipRect
{
    struct  ClipRect *Next;          
    struct  ClipRect *prev;          
    struct  Layer   *lobs;           
    struct  BitMap  *BitMap;
    struct  Rectangle   bounds;      
    struct  ClipRect *_p1,*_p2;      
    LONG    reserved;                

 

};

 



 







# 25 "dinclude:amiga20/intuition/intuition.h" 4

# 25 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/graphics/view.h" 5












 













# 1 "dinclude:amiga20/graphics/copper.h" 6












 












struct CopIns
{
    WORD   OpCode;  
    union
    {
    struct CopList *nxtlist;
    struct
    {
   union
   {
   WORD   VWaitPos;         
   WORD   DestAddr;         
   } u1;
   union
   {
   WORD   HWaitPos;         
   WORD   DestData;         
   } u2;
    } u4;
    } u3;
};
 







 
struct cprlist
{
    struct cprlist *Next;
    UWORD   *start;          
    WORD   MaxCount;        
};

struct CopList
{
    struct  CopList *Next;   
    struct  CopList *_CopList;   
    struct  ViewPort *_ViewPort;     
    struct  CopIns *CopIns;  
    struct  CopIns *CopPtr;  
    UWORD   *CopLStart;      
    UWORD   *CopSStart;      
    WORD   Count;           
    WORD   MaxCount;        
    WORD   DyOffset;        






};

struct UCopList
{
    struct UCopList *Next;
    struct CopList  *FirstCopList;  
    struct CopList  *CopList;       
};

 

struct copinit
{
    UWORD vsync_hblank[2];
    UWORD diwstart[4];
    UWORD diagstrt[4];       
    UWORD sprstrtup[(2*8*2)];
    UWORD wait14[2];
    UWORD norm_hblank[2];
    UWORD genloc[4];
    UWORD jump[(2*2)];
    UWORD wait_forever[2];
    UWORD   sprstop[4];
};



# 27 "dinclude:amiga20/graphics/view.h" 5

# 27 "dinclude:amiga20/graphics/view.h" 5




# 1 "dinclude:amiga20/graphics/gfxnodes.h" 6












 





struct  ExtendedNode    {
struct  Node    *xln_Succ;
struct  Node    *xln_Pred;
UBYTE   xln_Type;
BYTE    xln_Pri;
char    *xln_Name;
UBYTE   xln_Subsystem;
UBYTE   xln_Subtype;
LONG    xln_Library;
LONG    (*xln_Init)();
};










# 31 "dinclude:amiga20/graphics/view.h" 5

# 31 "dinclude:amiga20/graphics/view.h" 5




# 1 "dinclude:amiga20/graphics/monitor.h" 6












 













struct  MonitorSpec
{
    struct      ExtendedNode    ms_Node;
    UWORD       ms_Flags;
    LONG        ratioh;
    LONG        ratiov;
    UWORD       total_rows;
    UWORD       total_colorclocks;
    UWORD       DeniseMaxDisplayColumn;
    UWORD       BeamCon0;
    UWORD       min_row;
    struct      SpecialMonitor  *ms_Special;
    UWORD       ms_OpenCount;
    LONG        (*ms_transform)();
    LONG        (*ms_translate)();
    LONG        (*ms_scale)();
    UWORD       ms_xoffset;
    UWORD       ms_yoffset;
    struct      Rectangle       ms_LegalView;
    LONG        (*ms_maxoscan)();        
    LONG        (*ms_videoscan)();       
    UWORD       DeniseMinDisplayColumn;
    ULONG       DisplayCompatible;
    struct      List DisplayInfoDataBase;
    struct      SignalSemaphore DisplayInfoDataBaseSemaphore;
    ULONG       ms_reserved00;
    ULONG       ms_reserved01;
};

















































































struct  AnalogSignalInterval
{
    UWORD       asi_Start;
    UWORD       asi_Stop;
};

struct  SpecialMonitor
{
    struct      ExtendedNode    spm_Node;
    UWORD       spm_Flags;
    int (*do_monitor)();
    int (*reserved1)();
    int (*reserved2)();
    int (*reserved3)();
    struct      AnalogSignalInterval    hblank;
    struct      AnalogSignalInterval    vblank;
    struct      AnalogSignalInterval    hsync;
    struct      AnalogSignalInterval    vsync;
};



# 35 "dinclude:amiga20/graphics/view.h" 5

# 35 "dinclude:amiga20/graphics/view.h" 5






struct ViewPort
{
   struct   ViewPort *Next;
   struct   ColorMap  *ColorMap;     
                   
   struct   CopList  *DspIns;        
   struct   CopList  *SprIns;        
   struct   CopList  *ClrIns;        
   struct   UCopList *UCopIns;       
   WORD    DWidth,DHeight;
   WORD    DxOffset,DyOffset;
   UWORD    Modes;
   UBYTE        SpritePriorities;                
   UBYTE        ExtendedModes;
   struct   RasInfo *RasInfo;
};

struct View
{
   struct ViewPort *ViewPort;
   struct cprlist *LOFCprList;    
   struct cprlist *SHFCprList;    
   WORD DyOffset,DxOffset;    
                               
   UWORD   Modes;              
};

 
 
struct ViewExtra
{
        struct ExtendedNode n;
        struct View *View;        
        struct MonitorSpec *Monitor;  
};

 
 
struct ViewPortExtra
{
        struct ExtendedNode n;
        struct ViewPort *ViewPort;       
        struct Rectangle DisplayClip;   
};



 


















struct RasInfo   
{
   struct   RasInfo *Next;           
   struct   BitMap *BitMap;
   WORD    RxOffset,RyOffset;       
};

struct ColorMap
{
        UBYTE   Flags;
        UBYTE   Type;
        UWORD   Count;
        APTR    ColorTable;
        struct  ViewPortExtra *cm_vpe;
        UWORD   *TransparencyBits;
        UBYTE   TransparencyPlane;
        UBYTE   reserved1;
        UWORD   reserved2;
        struct  ViewPort *cm_vp;
        APTR    NormalDisplayInfo;
        APTR    CoerceDisplayInfo;
        struct  TagItem *cm_batch_items;
        ULONG   VPModeID;
};

 
 





 









# 29 "dinclude:amiga20/intuition/intuition.h" 4

# 29 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/graphics/rastport.h" 5












 









struct AreaInfo
{
    WORD   *VctrTbl;          
    WORD   *VctrPtr;          
    BYTE    *FlagTbl;          
    BYTE    *FlagPtr;          
    WORD   Count;             
    WORD   MaxCount;          
    WORD   FirstX,FirstY;     
};

struct TmpRas
{
    BYTE *RasPtr;
    LONG Size;
};

 
struct GelsInfo
{
    BYTE sprRsrvd;            
 
    UBYTE Flags;               
    struct VSprite *gelHead, *gelTail;  
     
    WORD *nextLine;
     
    WORD **lastColor;
    struct collTable *collHandler;      
    WORD leftmost, rightmost, topmost, bottommost;
   APTR firstBlissObj,lastBlissObj;     
};

struct RastPort
{
    struct  Layer *Layer;
    struct  BitMap   *BitMap;
    UWORD  *AreaPtrn;         
    struct  TmpRas *TmpRas;
    struct  AreaInfo *AreaInfo;
    struct  GelsInfo *GelsInfo;
    UBYTE   Mask;              
    BYTE    FgPen;             
    BYTE    BgPen;             
    BYTE    AOlPen;            
    BYTE    DrawMode;          
    BYTE    AreaPtSz;          
    BYTE    linpatcnt;         
    BYTE    dummy;
    UWORD  Flags;             
    UWORD  LinePtrn;          
    WORD   cp_x, cp_y;        
    UBYTE   minterms[8];
    WORD   PenWidth;
    WORD   PenHeight;
    struct  TextFont *Font;    
    UBYTE   AlgoStyle;         
    UBYTE   TxFlags;           
    UWORD   TxHeight;          
    UWORD   TxWidth;           
    UWORD   TxBaseline;        
    WORD    TxSpacing;         
    APTR    *RP_User;
    ULONG   longreserved[2];

    UWORD   wordreserved[7];   
    UBYTE   reserved[8];       

};

 





 





              




 
 
 
 



# 33 "dinclude:amiga20/intuition/intuition.h" 4

# 33 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/graphics/layers.h" 5












 
















                                         
                                         


struct Layer_Info
{
    struct Layer *top_layer;
    struct Layer *check_lp;          
    struct ClipRect *obs;            
        struct MinList  FreeClipRects;
        struct SignalSemaphore Lock;
        struct List gs_Head;                     
        LONG    longreserved;
        UWORD   Flags;
        BYTE    fatten_count;
        BYTE    LockLayersCount;
    UWORD  LayerInfo_extra_size;
    WORD        *blitbuff;
    void        *LayerInfo_extra;
};






# 37 "dinclude:amiga20/intuition/intuition.h" 4

# 37 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/graphics/text.h" 5












 













 















 













                                 
                                 
                                 
                                 

     



 
struct TextAttr {
    STRPTR  ta_Name;             
    UWORD   ta_YSize;            
    UBYTE   ta_Style;            
    UBYTE   ta_Flags;            
};

struct TTextAttr {
    STRPTR  tta_Name;            
    UWORD   tta_YSize;           
    UBYTE   tta_Style;           
    UBYTE   tta_Flags;           
    struct TagItem *tta_Tags;    
};


 

                                         




 
struct TextFont {
    struct Message tf_Message;   
                                 
    UWORD   tf_YSize;            
    UBYTE   tf_Style;            
    UBYTE   tf_Flags;            
    UWORD   tf_XSize;            
    UWORD   tf_Baseline;         
    UWORD   tf_BoldSmear;        

    UWORD   tf_Accessors;        

    UBYTE   tf_LoChar;           
    UBYTE   tf_HiChar;           
    APTR    tf_CharData;         

    UWORD   tf_Modulo;           
    APTR    tf_CharLoc;          
                                 
    APTR    tf_CharSpace;        
    APTR    tf_CharKern;         
};

 


 



struct TextFontExtension {       
    UWORD   tfe_MatchWord;               
    UBYTE   tfe_Flags0;                  
    UBYTE   tfe_Flags1;                  
    struct TextFont *tfe_BackPtr;        
    struct MsgPort *tfe_OrigReplyPort;   
    struct TagItem *tfe_Tags;            
    UWORD  *tfe_OFontPatchS;             
    UWORD  *tfe_OFontPatchK;             
     
};

 
 



                                 





 
struct ColorFontColors {
    UWORD   cfc_Reserved;        
    UWORD   cfc_Count;           
    UWORD  *cfc_ColorTable;      
};

 
struct ColorTextFont {
    struct TextFont ctf_TF;
    UWORD   ctf_Flags;           
    UBYTE   ctf_Depth;           
    UBYTE   ctf_FgColor;         
    UBYTE   ctf_Low;             
    UBYTE   ctf_High;            
    UBYTE   ctf_PlanePick;       
    UBYTE   ctf_PlaneOnOff;      
    struct ColorFontColors *ctf_ColorFontColors;  
    APTR    ctf_CharData[8];     
};

 
struct TextExtent {
    UWORD   te_Width;            
    UWORD   te_Height;           
    struct Rectangle te_Extent;  
};



# 41 "dinclude:amiga20/intuition/intuition.h" 4

# 41 "dinclude:amiga20/intuition/intuition.h" 4








# 1 "dinclude:amiga20/devices/inputevent.h" 5












 



# 1 "dinclude:amiga20/devices/timer.h" 6












 


# 1 "dinclude:amiga20/exec/types.h" 7












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 16 "dinclude:amiga20/devices/timer.h" 6

# 16 "dinclude:amiga20/devices/timer.h" 6

# 1 "dinclude:amiga20/exec/io.h" 7












 






struct IORequest {
    struct  Message io_Message;
    struct  Device  *io_Device;      
    struct  Unit    *io_Unit;        
    UWORD   io_Command;      
    UBYTE   io_Flags;
    BYTE    io_Error;                
};

struct IOStdReq {
    struct  Message io_Message;
    struct  Device  *io_Device;      
    struct  Unit    *io_Unit;        
    UWORD   io_Command;      
    UBYTE   io_Flags;
    BYTE    io_Error;                
    ULONG   io_Actual;               
    ULONG   io_Length;               
    APTR    io_Data;                 
    ULONG   io_Offset;               
};

 



 


















# 17 "dinclude:amiga20/devices/timer.h" 6

# 17 "dinclude:amiga20/devices/timer.h" 6

 








struct timeval {
    ULONG tv_secs;
    ULONG tv_micro;
};

struct EClockVal {
    ULONG ev_hi;
    ULONG ev_lo;
};

struct timerequest {
    struct IORequest tr_node;
    struct timeval tr_time;
};

 






# 17 "dinclude:amiga20/devices/inputevent.h" 5

# 17 "dinclude:amiga20/devices/inputevent.h" 5


 

 
 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 


 



 
 
 

 

 











 
struct IEPointerPixel   {
    struct Screen       *iepp_Screen;    
    struct {                             
        WORD    X;
        WORD    Y;
    }                   iepp_Position;
};












 
struct IEPointerTablet  {
    struct {
        UWORD   X;
        UWORD   Y;
    }                   iept_Range;      
    struct {
        UWORD   X;
        UWORD   Y;
    }                   iept_Value;      

    WORD                iept_Pressure;   
};



 
 






 










 





 




 
 
 

 




 


































 

struct InputEvent {
    struct  InputEvent *ie_NextEvent;    
    UBYTE   ie_Class;                    
    UBYTE   ie_SubClass;                 
    UWORD   ie_Code;                     
    UWORD   ie_Qualifier;                
    union {
        struct {
            WORD    ie_x;                
            WORD    ie_y;
        } ie_xy;
        APTR    ie_addr;                 
        struct {
            UBYTE   ie_prev1DownCode;    
            UBYTE   ie_prev1DownQual;    
            UBYTE   ie_prev2DownCode;    
            UBYTE   ie_prev2DownQual;    
        } ie_dead;
    } ie_position;
    struct timeval ie_TimeStamp;         
};











# 49 "dinclude:amiga20/intuition/intuition.h" 4

# 49 "dinclude:amiga20/intuition/intuition.h" 4








 

 
 
 
struct Menu
{
    struct Menu *NextMenu;       
    WORD LeftEdge, TopEdge;      
    WORD Width, Height;  
    UWORD Flags;                 
    BYTE *MenuName;              
    struct MenuItem *FirstItem;  

     
    WORD JazzX, JazzY, BeatX, BeatY;
};


 


 







 
 
 
struct MenuItem
{
    struct MenuItem *NextItem;   
    WORD LeftEdge, TopEdge;      
    WORD Width, Height;          
    UWORD Flags;                 

    LONG MutualExclude;          

    APTR ItemFill;               

    

 
    APTR SelectFill;             

    BYTE Command;                

    struct MenuItem *SubItem;    

    

 
    UWORD NextSelect;
};


 






 






 


 








 
 
 
struct Requester
{
    struct Requester *OlderRequest;
    WORD LeftEdge, TopEdge;              
    WORD Width, Height;                  
    WORD RelLeft, RelTop;                

    struct Gadget *ReqGadget;            
    struct Border *ReqBorder;            
    struct IntuiText *ReqText;           
    UWORD Flags;                         

     
    UBYTE BackFill;
     
    struct Layer *ReqLayer;

    UBYTE ReqPad1[32];

    




 
    struct BitMap *ImageBMap;    
    struct Window *RWindow;      

    struct Image  *ReqImage;     

    UBYTE ReqPad2[32];
};


 

                          


 

         

         

         

 

        

 

         


 










 
 
 
struct Gadget
{
    struct Gadget *NextGadget;   

    WORD LeftEdge, TopEdge;      
    WORD Width, Height;          

    UWORD Flags;                 

    UWORD Activation;            

    UWORD GadgetType;            

    


 
    APTR GadgetRender;

    

 
    APTR SelectRender;

    struct IntuiText *GadgetText;    

    









 
    LONG MutualExclude;   

    

 
    APTR SpecialInfo;

    UWORD GadgetID;      
    APTR UserData;       
};


 
 


















 










 








 






 









 


 



 





 





 









 





 









 






                                  


 










 

 



 





 








 










 

 
 
 


 
struct BoolInfo
{
    UWORD  Flags;        
    UWORD  *Mask;       




 
    ULONG  Reserved;     
};




 


 
 
 


 
struct PropInfo
{
    UWORD Flags;         

    






 
    UWORD HorizPot;      
    UWORD VertPot;       

    















 
    UWORD HorizBody;             
    UWORD VertBody;              

     
    UWORD CWidth;        
    UWORD CHeight;       
    UWORD HPotRes, VPotRes;      
    UWORD LeftBorder;            
    UWORD TopBorder;             
};


 





 















 
 
 


 
struct StringInfo
{
     
    UBYTE *Buffer;       
    UBYTE *UndoBuffer;   
    WORD BufferPos;      
    WORD MaxChars;       
    WORD DispPos;        

     
    WORD UndoPos;        
    WORD NumChars;       
    WORD DispCount;      
    WORD CLeft, CTop;    

    




 
     
    struct StringExtend *Extension;

    



 
    LONG LongInt;

    



 
    struct KeyMap *AltKeyMap;
};

 
 
 



 
struct IntuiText
{
    UBYTE FrontPen, BackPen;     
    UBYTE DrawMode;              
    WORD LeftEdge;               
    WORD TopEdge;                
    struct TextAttr *ITextFont;  
    UBYTE *IText;                
    struct IntuiText *NextText;  
};






 
 
 








 
struct Border
{
    WORD LeftEdge, TopEdge;      
    UBYTE FrontPen, BackPen;     
    UBYTE DrawMode;              
    BYTE Count;                  
    WORD *XY;                    
    struct Border *NextBorder;   
};






 
 
 


 
struct Image
{
    WORD LeftEdge;               
    WORD TopEdge;                
    WORD Width;                  
    WORD Height;
    WORD Depth;                  
    UWORD *ImageData;            

    






























 
    UBYTE PlanePick, PlaneOnOff;

    


 
    struct Image *NextImage;
};






 
 
 
struct IntuiMessage
{
    struct Message ExecMessage;

    

 
    ULONG Class;

     
    UWORD Code;

     
    UWORD Qualifier;

    

 
    APTR IAddress;

    




 
    WORD MouseX, MouseY;

    

 
    ULONG Seconds, Micros;

    

 
    struct Window *IDCMPWindow;

     
    struct IntuiMessage *SpecialLink;
};


 


 























 

 

 


 





 



 
 






 




 




 
struct IBox {
    WORD Left;
    WORD Top;
    WORD Width;
    WORD Height;
    };



 
 
 
struct Window
{
    struct Window *NextWindow;           

    WORD LeftEdge, TopEdge;              
    WORD Width, Height;                  

    WORD MouseY, MouseX;                 

    WORD MinWidth, MinHeight;            
    UWORD MaxWidth, MaxHeight;           

    ULONG Flags;                         

    struct Menu *MenuStrip;              

    UBYTE *Title;                        

    struct Requester *FirstRequest;      

    struct Requester *DMRequest;         

    WORD ReqCount;                       

    struct Screen *WScreen;              
    struct RastPort *RPort;              

    








 
    BYTE BorderLeft, BorderTop, BorderRight, BorderBottom;
    struct RastPort *BorderRPort;


    



 
    struct Gadget *FirstGadget;

     
    struct Window *Parent, *Descendant;

    

 
    UWORD *Pointer;      
    BYTE PtrHeight;      
    BYTE PtrWidth;       
    BYTE XOffset, YOffset;       

     
    ULONG IDCMPFlags;    
    struct MsgPort *UserPort, *WindowPort;
    struct IntuiMessage *MessageKey;

    UBYTE DetailPen, BlockPen;   

    


 
    struct Image *CheckMark;

    UBYTE *ScreenTitle;  

    




 
    WORD GZZMouseX;
    WORD GZZMouseY;
    

 
    WORD GZZWidth;
    WORD GZZHeight;

    UBYTE *ExtData;

    BYTE *UserData;      

    

 
    struct Layer *WLayer;

    

 
    struct TextFont *IFont;

    


 
    ULONG       MoreFlags;

     
};


 








 
 

















 




 



 





 

                                         

 




 


 




 
 
 



 
struct NewWindow
{
    WORD LeftEdge, TopEdge;              
    WORD Width, Height;                  

    UBYTE DetailPen, BlockPen;           

    ULONG IDCMPFlags;                    

    ULONG Flags;                         

    



 
    struct Gadget *FirstGadget;

    


 
    struct Image *CheckMark;

    UBYTE *Title;                          

    



 
    struct Screen *Screen;

    


 
    struct BitMap *BitMap;

    










 
    WORD MinWidth, MinHeight;        
    UWORD MaxWidth, MaxHeight;        

    



 
    UWORD Type;

};







 
struct ExtNewWindow
{
    WORD LeftEdge, TopEdge;
    WORD Width, Height;

    UBYTE DetailPen, BlockPen;
    ULONG IDCMPFlags;
    ULONG Flags;
    struct Gadget *FirstGadget;

    struct Image *CheckMark;

    UBYTE *Title;
    struct Screen *Screen;
    struct BitMap *BitMap;

    WORD MinWidth, MinHeight;
    UWORD MaxWidth, MaxHeight;

    






 
    UWORD Type;

    







 
    struct TagItem      *Extension;
};






 



 







                         




                        

 



                         





 



                        





 


                        


 

                        




 

                        



 

                         

                        





 

                        





 

                        

 

                        

 

                         

    

 












                         

                         



     

                        

 


                         

 

                        

 





# 1 "dinclude:amiga20/intuition/screens.h" 5












 































 

 
 
 



 



 



struct DrawInfo
{
    UWORD       dri_Version;     
    UWORD       dri_NumPens;     
    UWORD       *dri_Pens;       

    struct TextFont     *dri_Font;       
    UWORD       dri_Depth;       

    struct {       
        UWORD   X;
        UWORD   Y;
    }           dri_Resolution;

    ULONG       dri_Flags;               
    ULONG       dri_Reserved[7];         
};



 













 
 
 
struct Screen
{
    struct Screen *NextScreen;           
    struct Window *FirstWindow;          

    WORD LeftEdge, TopEdge;              
    WORD Width, Height;                  

    WORD MouseY, MouseX;                 

    UWORD Flags;                         

    UBYTE *Title;                        
    UBYTE *DefaultTitle;                 

     
    





 
    BYTE BarHeight, BarVBorder, BarHBorder, MenuVBorder, MenuHBorder;
    BYTE WBorTop, WBorLeft, WBorRight, WBorBottom;

    struct TextAttr *Font;               

     
    struct ViewPort ViewPort;            
    struct RastPort RastPort;            
    struct BitMap BitMap;                
    struct Layer_Info LayerInfo;         

    

 
    struct Gadget *FirstGadget;

    UBYTE DetailPen, BlockPen;           

    

 
    UWORD SaveColor0;

     
    struct Layer *BarLayer;

    UBYTE *ExtData;

    UBYTE *UserData;     

     
};


 


 

 



















 










 



 




 




                         

                         

                         


                         

                        


 

                         

                         

                        


 

                         

                        

 

                        


 


                        

 

                        

 

                        

 

                        









 

                         

 

                         

                         

                         

                         

                        

 

                        



 



 







 







 
 
 


 
struct NewScreen
{
    WORD LeftEdge, TopEdge, Width, Height, Depth;   

    UBYTE DetailPen, BlockPen;   

    UWORD ViewModes;             

    UWORD Type;                  

    struct TextAttr *Font;       

    UBYTE *DefaultTitle;         

    struct Gadget *Gadgets;      

    




 
    struct BitMap *CustomBitMap;
};











 
struct ExtNewScreen
{
    WORD LeftEdge, TopEdge, Width, Height, Depth;
    UBYTE DetailPen, BlockPen;
    UWORD ViewModes;
    UWORD Type;
    struct TextAttr *Font;
    UBYTE *DefaultTitle;
    struct Gadget *Gadgets;
    struct BitMap *CustomBitMap;

    struct TagItem      *Extension;
                                

 
};

 






 









 

struct PubScreenNode    {
    struct Node         psn_Node;        
    struct Screen       *psn_Screen;
    UWORD               psn_Flags;       
    WORD                psn_Size;        
    WORD                psn_VisitorCount;  
    struct Task         *psn_SigTask;    
    UBYTE               psn_SigBit;      
};





 




 


# 1 "dinclude:amiga20/intuition/iobsolete.h" 6













 




















 






 




 



















 





















 






















 






























 


































 





















































 


























 



















 





 




















































 


































# 404 "dinclude:amiga20/intuition/screens.h" 5

# 404 "dinclude:amiga20/intuition/screens.h" 5




# 1218 "dinclude:amiga20/intuition/intuition.h" 4

# 1218 "dinclude:amiga20/intuition/intuition.h" 4




# 1 "dinclude:amiga20/intuition/preferences.h" 5












 









 
 
 

 












 



struct Preferences
{
     
    BYTE FontHeight;                     

     
    UBYTE PrinterPort;                   

     
    UWORD BaudRate;                      

     
    struct timeval KeyRptSpeed;          
    struct timeval KeyRptDelay;          
    struct timeval DoubleClick;          

     
    UWORD PointerMatrix[(1 + 16 + 1) * 2];       
    BYTE XOffset;                        
    BYTE YOffset;                        
    UWORD color17;                       
    UWORD color18;                       
    UWORD color19;                       
    UWORD PointerTicks;                  

     
    UWORD color0;                        
    UWORD color1;                        
    UWORD color2;                        
    UWORD color3;                        

     
    BYTE ViewXOffset;                    
    BYTE ViewYOffset;                    
    WORD ViewInitX, ViewInitY;           

    BOOL EnableCLI;                      

     
    UWORD PrinterType;                   
    UBYTE PrinterFilename[30]; 

     
    UWORD PrintPitch;                    
    UWORD PrintQuality;                  
    UWORD PrintSpacing;                  
    UWORD PrintLeftMargin;               
    UWORD PrintRightMargin;              
    UWORD PrintImage;                    
    UWORD PrintAspect;                   
    UWORD PrintShade;                    
    WORD PrintThreshold;                 

     
    UWORD PaperSize;                     
    UWORD PaperLength;                   
    UWORD PaperType;                     

     
     
    UBYTE   SerRWBits;    
                          
    UBYTE   SerStopBuf;   
                          
    UBYTE   SerParShk;    
                          
    UBYTE   LaceWB;       

    UBYTE   WorkName[30];   

    BYTE    RowSizeChange;       
    BYTE    ColumnSizeChange;

    UWORD    PrintFlags;         
    UWORD    PrintMaxWidth;      
    UWORD    PrintMaxHeight;     
    UBYTE    PrintDensity;       
    UBYTE    PrintXOffset;       

    UWORD    wb_Width;           
    UWORD    wb_Height;          
    UBYTE    wb_Depth;           

    UBYTE    ext_size;           
                             
};


 



 



 



 









 



 




 



 



 



 



 




 






 











 



 







 











 






 




 






















 







# 1222 "dinclude:amiga20/intuition/intuition.h" 4

# 1222 "dinclude:amiga20/intuition/intuition.h" 4


 
 
 



 
struct Remember
{
    struct Remember *NextRemember;
    ULONG RememberSize;
    UBYTE *Memory;
};


 
 
struct ColorSpec {
    WORD        ColorIndex;      
    UWORD       Red;             
    UWORD       Green;           
    UWORD       Blue;            
};

 
 
 
struct EasyStruct {
    ULONG       es_StructSize;   
    ULONG       es_Flags;        
    UBYTE       *es_Title;       
    UBYTE       *es_TextFormat;  
    UBYTE       *es_GadgetFormat;  
};



 
 
 

 


















 






 







 









 









 









 


























 






# 25 "dinclude:amiga20/intuition/intuitionbase.h" 3

# 25 "dinclude:amiga20/intuition/intuitionbase.h" 3









 






 














 
 
 






 

 
struct IntuitionBase
{
    struct Library LibNode;

    struct View ViewLord;

    struct Window *ActiveWindow;
    struct Screen *ActiveScreen;

    

 
    struct Screen *FirstScreen;  

    ULONG Flags;         
    WORD        MouseY, MouseX;
                         

    ULONG Seconds;       
    ULONG Micros;        

    


 
};




# 15 "local/defs.h" 2

# 15 "local/defs.h" 2

# 1 "dinclude:amiga20/intuition/gadgetclass.h" 3












 















 

 































     

         

         


        



 


          


        

 

 

         


        

 


        


 

 



         








 


 















 





         




        


 



 






 





 










 

 
struct gpHitTest {
    ULONG               MethodID;
    struct GadgetInfo   *gpht_GInfo;
    struct {
        WORD    X;
        WORD    Y;
    }                   gpht_Mouse;
};
 


 
struct gpRender {
    ULONG               MethodID;
    struct GadgetInfo   *gpr_GInfo;      
    struct RastPort     *gpr_RPort;      
    LONG                gpr_Redraw;      
};

 




 
struct gpInput {
    ULONG               MethodID;
    struct GadgetInfo   *gpi_GInfo;
    struct InputEvent   *gpi_IEvent;
    LONG                *gpi_Termination;
    struct {
        WORD    X;
        WORD    Y;
    }                   gpi_Mouse;
};

 



 









 



 
struct gpGoInactive {
    ULONG               MethodID;
    struct GadgetInfo   *gpgi_GInfo;

     
    ULONG               gpgi_Abort;     


 
};


 






# 16 "local/defs.h" 2

# 16 "local/defs.h" 2

# 1 "dinclude:amiga20/libraries/gadtools.h" 3












 

 













 


 











 






 


 

 


 

 















 






 

 



 

 

struct NewGadget
    {
    WORD ng_LeftEdge, ng_TopEdge;        
    WORD ng_Width, ng_Height;            
    UBYTE *ng_GadgetText;                
    struct TextAttr *ng_TextAttr;        
    UWORD ng_GadgetID;                   
    ULONG ng_Flags;                      
    APTR ng_VisualInfo;                  
    APTR ng_UserData;                    
    };




 









 

 

struct NewMenu
    {
    UBYTE nm_Type;               
    STRPTR nm_Label;             
    STRPTR nm_CommKey;           
    UWORD nm_Flags;              
    LONG nm_MutualExclude;       
    APTR nm_UserData;            
    };

 









 







 




 








 




 




 


 





 

 







































 













































 








 

 

 





 



# 17 "local/defs.h" 2

# 17 "local/defs.h" 2

# 1 "dinclude:amiga20/graphics/gfxbase.h" 3












 











struct GfxBase
{
    struct Library  LibNode;
    struct View *ActiView;
    struct copinit *copinit;     
    long    *cia;                
    long    *blitter;            
    UWORD   *LOFlist;
    UWORD   *SHFlist;
    struct bltnode *blthd,*blttl;
    struct bltnode *bsblthd,*bsblttl;
    struct Interrupt vbsrv,timsrv,bltsrv;
    struct List     TextFonts;
    struct  TextFont *DefaultFont;
    UWORD  Modes;                        
    BYTE VBlank;
    BYTE Debug;
    WORD BeamSync;
    WORD system_bplcon0;         
    UBYTE   SpriteReserved;
    UBYTE   bytereserved;
    UWORD   Flags;
    WORD   BlitLock;
        WORD    BlitNest;

        struct  List    BlitWaitQ;
        struct  Task    *BlitOwner;
        struct  List    TOF_WaitQ;
        UWORD   DisplayFlags;            
                                         
        struct SimpleSprite **SimpleSprites;
        UWORD   MaxDisplayRow;           
        UWORD   MaxDisplayColumn;        
        UWORD   NormalDisplayRows;
        UWORD   NormalDisplayColumns;
         
        UWORD   NormalDPMX;              
        UWORD   NormalDPMY;              
        struct  SignalSemaphore *LastChanceMemory;
        UWORD   *LCMptr;
        UWORD   MicrosPerLine;           
        UWORD   MinDisplayColumn;
        UBYTE   ChipRevBits0;
        UBYTE   crb_reserved[5];
        UWORD   monitor_id;              
        ULONG   hedley[8];
        ULONG   hedley_sprites[8] ;      
        ULONG   hedley_sprites1[8] ;             
        WORD    hedley_count;
        UWORD   hedley_flags;
        WORD    hedley_tmp;
        LONG    *hash_table;
        UWORD   current_tot_rows;
        UWORD   current_tot_cclks;
        UBYTE   hedley_hint;
        UBYTE   hedley_hint2;
        ULONG   nreserved[4];
        LONG    *a2024_sync_raster;
        WORD   control_delta_pal;
        WORD   control_delta_ntsc;
        struct  MonitorSpec *current_monitor;
        struct  List MonitorList;
        struct  MonitorSpec *default_monitor;
        struct  SignalSemaphore *MonitorListSemaphore;
        void    *DisplayInfoDataBase;
        struct  SignalSemaphore *ActiViewCprSemaphore;
        ULONG   *UtilityBase;            
        ULONG   *ExecBase;               
        };








 












# 18 "local/defs.h" 2

# 18 "local/defs.h" 2

# 1 "dinclude:amiga20/graphics/view.h" 3












 






























 
 
 
 
 
 



 







 
 
 
 
 


 
 



 
 


 
 



 
 




 


















 

 

 




















 
 





 









# 19 "local/defs.h" 2

# 19 "local/defs.h" 2

# 1 "dinclude:amiga20/graphics/displayinfo.h" 3












 

















 

typedef APTR DisplayInfoHandle;

 






struct QueryHeader
{
        ULONG   StructID;        
        ULONG   DisplayID;       
        ULONG   SkipID;          
        ULONG   Length;          
};

struct DisplayInfo
{
        struct  QueryHeader Header;
        UWORD   NotAvailable;    
        ULONG   PropertyFlags;   
        Point   Resolution;      
        UWORD   PixelSpeed;      
        UWORD   NumStdSprites;   
        UWORD   PaletteRange;    
        Point   SpriteResolution;  
        UBYTE   pad[4];
        ULONG   reserved[2];     
};

 





 





















struct DimensionInfo
{
        struct  QueryHeader Header;
        UWORD   MaxDepth;              
        UWORD   MinRasterWidth;        
        UWORD   MinRasterHeight;       
        UWORD   MaxRasterWidth;        
        UWORD   MaxRasterHeight;       
        struct  Rectangle   Nominal;   
        struct  Rectangle   MaxOScan;  
        struct  Rectangle VideoOScan;  
        struct  Rectangle   TxtOScan;  
        struct  Rectangle   StdOScan;  
        UBYTE   pad[14];
        ULONG   reserved[2];           
};

struct MonitorInfo
{
        struct  QueryHeader Header;
        struct  MonitorSpec  *Mspc;    
        Point   ViewPosition;          
        Point   ViewResolution;        
        struct  Rectangle ViewPositionRange;   
        UWORD   TotalRows;             
        UWORD   TotalColorClocks;      
        UWORD   MinRow;        
        WORD    Compatibility;         
        UBYTE   pad[36];
        ULONG   reserved[2];           
};

 







struct NameInfo
{
        struct  QueryHeader Header;
        UBYTE   Name[32];
        ULONG   reserved[2];           
};

 



 







 
 
 
























 


























 






 





# 20 "local/defs.h" 2

# 20 "local/defs.h" 2

# 1 "dinclude:amiga20/graphics/gfxmacros.h" 3












 


































# 21 "local/defs.h" 2

# 21 "local/defs.h" 2

# 1 "dinclude:amiga20/devices/console.h" 3












 









 





 












 




















 
 



















 



 




 



 






# 22 "local/defs.h" 2

# 22 "local/defs.h" 2

# 1 "dinclude:amiga20/devices/conunit.h" 3












 















# 1 "dinclude:amiga20/devices/keymap.h" 4













 








struct   KeyMap {
    UBYTE   *km_LoKeyMapTypes;
    ULONG   *km_LoKeyMap;
    UBYTE   *km_LoCapsable;
    UBYTE   *km_LoRepeatable;
    UBYTE   *km_HiKeyMapTypes;
    ULONG   *km_HiKeyMap;
    UBYTE   *km_HiCapsable;
    UBYTE   *km_HiRepeatable;
};

struct  KeyMapNode {
    struct Node kn_Node;         
    struct KeyMap kn_KeyMap;
};

 
struct  KeyMapResource {
    struct Node kr_Node;
    struct List kr_List;         
};

 





















 










# 29 "dinclude:amiga20/devices/conunit.h" 3

# 29 "dinclude:amiga20/devices/conunit.h" 3






 



 




 










struct  ConUnit {
    struct  MsgPort cu_MP;
     
    struct  Window *cu_Window;   
    WORD    cu_XCP;              
    WORD    cu_YCP;
    WORD    cu_XMax;             
    WORD    cu_YMax;
    WORD    cu_XRSize;           
    WORD    cu_YRSize;
    WORD    cu_XROrigin;         
    WORD    cu_YROrigin;
    WORD    cu_XRExtant;         
    WORD    cu_YRExtant;
    WORD    cu_XMinShrink;       
    WORD    cu_YMinShrink;
    WORD    cu_XCCP;             
    WORD    cu_YCCP;

     
     
    struct  KeyMap cu_KeyMapStruct;
     
    UWORD   cu_TabStops[80];  

     
    BYTE    cu_Mask;
    BYTE    cu_FgPen;
    BYTE    cu_BgPen;
    BYTE    cu_AOLPen;
    BYTE    cu_DrawMode;
    BYTE    cu_Obsolete1;        
    APTR    cu_Obsolete2;        
    UBYTE   cu_Minterms[8];      
    struct  TextFont *cu_Font;
    UBYTE   cu_AlgoStyle;
    UBYTE   cu_TxFlags;
    UWORD   cu_TxHeight;
    UWORD   cu_TxWidth;
    UWORD   cu_TxBaseline;
    WORD    cu_TxSpacing;

     
    UBYTE   cu_Modes[(((20+1)+1)+7)/8];  
    UBYTE   cu_RawEvents[(0x15+8)/8];
};



# 23 "local/defs.h" 2

# 23 "local/defs.h" 2

# 1 "dinclude:amiga20/devices/serial.h" 3












 





                    
                    

 struct  IOTArray {
        ULONG TermArray0;
        ULONG TermArray1;
};





 

 

 
 

 struct  IOExtSer {
        struct   IOStdReq IOSer;





















 
   ULONG   io_CtlChar;     
   ULONG   io_RBufLen;     
   ULONG   io_ExtFlags;    
   ULONG   io_Baud;        
   ULONG   io_BrkTime;     
   struct  IOTArray io_TermArray;  
   UBYTE   io_ReadLen;     
   UBYTE   io_WriteLen;    
   UBYTE   io_StopBits;    
   UBYTE   io_SerFlags;    
   UWORD   io_Status;
};
   


















 























 













                                 

























 


























# 24 "local/defs.h" 2

# 24 "local/defs.h" 2

# 1 "dinclude:amiga20/dos/dos.h" 3












 








 




 







 












 





struct DateStamp {
   LONG  ds_Days;              
   LONG  ds_Minute;            
   LONG  ds_Tick;              
};  



 
struct FileInfoBlock {
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

 

 















 
 
 



 
typedef long  BPTR;                  
typedef long  BSTR;                  

 



 


 



 

 
struct InfoData {
   LONG   id_NumSoftErrors;      
   LONG   id_UnitNumber;         
   LONG   id_DiskState;          
   LONG   id_NumBlocks;          
   LONG   id_NumBlocksUsed;      
   LONG   id_BytesPerBlock;
   LONG   id_DiskType;           
   BPTR   id_VolumeNode;         
   LONG   id_InUse;              
};  

 
         




         








 





































 









 

 
 





 





 
 





 




 



 



 






 










# 25 "local/defs.h" 2

# 25 "local/defs.h" 2

# 1 "dinclude:amiga20/dos/dosextens.h" 3












 





















 
 
 

struct Process {
    struct  Task    pr_Task;
    struct  MsgPort pr_MsgPort;  
    WORD    pr_Pad;              
    BPTR    pr_SegList;          
    LONG    pr_StackSize;        
    APTR    pr_GlobVec;          
    LONG    pr_TaskNum;          
    BPTR    pr_StackBase;        
    LONG    pr_Result2;          
    BPTR    pr_CurrentDir;       
    BPTR    pr_CIS;              
    BPTR    pr_COS;              
    APTR    pr_ConsoleTask;      
    APTR    pr_FileSystemTask;   
    BPTR    pr_CLI;              
    APTR    pr_ReturnAddr;       
    APTR    pr_PktWait;          
    APTR    pr_WindowPtr;        

     
    BPTR    pr_HomeDir;          
    LONG    pr_Flags;            
    void    (*pr_ExitCode)();    
    LONG    pr_ExitData;         
    UBYTE   *pr_Arguments;       
    struct MinList pr_LocalVars;  
    ULONG   pr_ShellPrivate;     
    BPTR    pr_CES;              
};   



 
















 

struct FileHandle {
   struct Message *fh_Link;       
   struct MsgPort *fh_Port;       
   struct MsgPort *fh_Type;      
 
   LONG fh_Buf;
   LONG fh_Pos;
   LONG fh_End;
   LONG fh_Funcs;

   LONG fh_Func2;
   LONG fh_Func3;
   LONG fh_Args;

   LONG fh_Arg2;
};  

 

struct DosPacket {
   struct Message *dp_Link;       
   struct MsgPort *dp_Port;       
                                  
   LONG dp_Type;                 

 
   LONG dp_Res1;                 


 
   LONG dp_Res2;                 
 
 




   LONG dp_Arg1;
   LONG dp_Arg2;
   LONG dp_Arg3;
   LONG dp_Arg4;
   LONG dp_Arg5;
   LONG dp_Arg6;
   LONG dp_Arg7;
};  



 

struct StandardPacket {
   struct Message   sp_Msg;
   struct DosPacket sp_Pkt;
};  

 












































 




 
 




 














 
struct ErrorString {
        LONG  *estr_Nums;
        UBYTE *estr_Strings;
};




 

struct DosLibrary {
    struct Library dl_lib;
    struct RootNode *dl_Root;  
    APTR    dl_GV;             
    LONG    dl_A2;             
    LONG    dl_A5;
    LONG    dl_A6;
    struct ErrorString *dl_Errors;   
    struct timerequest *dl_TimeReq;  
    struct Library     *dl_UtilityBase;  
};   

 

struct RootNode {
    BPTR    rn_TaskArray;            

 
    BPTR    rn_ConsoleSegment;  
    struct  DateStamp rn_Time;  
    LONG    rn_RestartSeg;      
    BPTR    rn_Info;            
    BPTR    rn_FileHandlerSegment;  
    struct MinList rn_CliList;  
                                
    struct MsgPort *rn_BootProc;  
    BPTR    rn_ShellSegment;    
    LONG    rn_Flags;           
};   






 
struct CliProcList {
        struct MinNode cpl_Node;
        LONG cpl_First;       
        struct MsgPort **cpl_Array;
                             


 
};

struct DosInfo {
    BPTR    di_McName;          

    BPTR    di_DevInfo;         
    BPTR    di_Devices;         
    BPTR    di_Handlers;        
    APTR    di_NetHand;         
    struct  SignalSemaphore di_DevLock;     
    struct  SignalSemaphore di_EntryLock;   
    struct  SignalSemaphore di_DeleteLock;  
};   

 
 

struct Segment {
        BPTR seg_Next;
        LONG seg_UC;
        BPTR seg_Seg;
        UBYTE seg_Name[4];       
};







 

struct CommandLineInterface {
    LONG   cli_Result2;         
    BSTR   cli_SetName;         
    BPTR   cli_CommandDir;      
    LONG   cli_ReturnCode;      
    BSTR   cli_CommandName;     
    LONG   cli_FailLevel;       
    BSTR   cli_Prompt;          
    BPTR   cli_StandardInput;   
    BPTR   cli_CurrentInput;    
    BSTR   cli_CommandFile;     
    LONG   cli_Interactive;     
    LONG   cli_Background;      
    BPTR   cli_CurrentOutput;   
    LONG   cli_DefaultStack;    
    BPTR   cli_StandardOutput;  
    BPTR   cli_Module;          
};   






 

 

struct DeviceList {
    BPTR                dl_Next;         
    LONG                dl_Type;         
    struct MsgPort *    dl_Task;         
    BPTR                dl_Lock;         
    struct DateStamp    dl_VolumeDate;   
    BPTR                dl_LockList;     
    LONG                dl_DiskType;     
    LONG                dl_unused;
    BSTR                dl_Name;         
};

 

struct        DevInfo {
    BPTR  dvi_Next;
    LONG  dvi_Type;
    APTR  dvi_Task;
    BPTR  dvi_Lock;
    BSTR  dvi_Handler;
    LONG  dvi_StackSize;
    LONG  dvi_Priority;
    LONG  dvi_Startup;
    BPTR  dvi_SegList;
    BPTR  dvi_GlobVec;
    BSTR  dvi_Name;
};

 

struct DosList {
    BPTR                dol_Next;         
    LONG                dol_Type;         
    struct MsgPort     *dol_Task;         
    BPTR                dol_Lock;
    union {
        struct {
        BSTR    dol_Handler;     
        LONG    dol_StackSize;   
        LONG    dol_Priority;    
        ULONG   dol_Startup;     
        BPTR    dol_SegList;     
        BPTR    dol_GlobVec;    

 
        } dol_handler;

        struct {
        struct DateStamp        dol_VolumeDate;   
        BPTR                    dol_LockList;     
        LONG                    dol_DiskType;     
        } dol_volume;

        struct {
        UBYTE   *dol_AssignName;      
        struct AssignList *dol_List;  
        } dol_assign;

    } dol_misc;

    BSTR                dol_Name;         
    };

 

struct AssignList {
        struct AssignList *al_Next;
        BPTR               al_Lock;
};

 







 
struct DevProc {
        struct MsgPort *dvp_Port;
        BPTR            dvp_Lock;
        ULONG           dvp_Flags;
        struct DosList *dvp_DevNode;     
};

 





 











 





 


 
struct FileLock {
    BPTR                fl_Link;         
    LONG                fl_Key;          
    LONG                fl_Access;       
    struct MsgPort *    fl_Task;         
    BPTR                fl_Volume;       
};

 






 



 
 




 
 
 
 
 









# 26 "local/defs.h" 2

# 26 "local/defs.h" 2

# 1 "dinclude:amiga20/dos/filehandler.h" 3












 













 

struct DosEnvec {
    ULONG de_TableSize;       
    ULONG de_SizeBlock;       
    ULONG de_SecOrg;          
    ULONG de_Surfaces;        
    ULONG de_SectorPerBlock;  
    ULONG de_BlocksPerTrack;  
    ULONG de_Reserved;        
    ULONG de_PreAlloc;        
    ULONG de_Interleave;      
    ULONG de_LowCyl;          
    ULONG de_HighCyl;         
    ULONG de_NumBuffers;      
    ULONG de_BufMemType;      
    ULONG de_MaxTransfer;     
    ULONG de_Mask;            
    LONG  de_BootPri;         
    ULONG de_DosType;        

 
    ULONG de_Baud;            
    ULONG de_Control;         
    ULONG de_BootBlocks;      

};

 





























 
struct FileSysStartupMsg {
    ULONG       fssm_Unit;       
    BSTR        fssm_Device;     
    BPTR        fssm_Environ;    
    ULONG       fssm_Flags;      
};







 

struct DeviceNode {
    BPTR        dn_Next;         
    ULONG       dn_Type;         
    struct MsgPort *dn_Task;    

 
    BPTR        dn_Lock;         
    BSTR        dn_Handler;      
    ULONG       dn_StackSize;    
    LONG        dn_Priority;     
    BPTR        dn_Startup;      
    BPTR        dn_SegList;     
 
    BPTR        dn_GlobalVec;   






 
    BSTR        dn_Name;         
};



# 27 "local/defs.h" 2

# 27 "local/defs.h" 2

# 1 "dinclude:amiga20/dos/doshunks.h" 3












 

 
























 














# 28 "local/defs.h" 2

# 28 "local/defs.h" 2

# 1 "dinclude:amiga20/workbench/workbench.h" 3












 






























struct OldDrawerData {  
    struct NewWindow    dd_NewWindow;    
    LONG                dd_CurrentX;     
    LONG                dd_CurrentY;     
};
 


struct DrawerData {
    struct NewWindow    dd_NewWindow;    
    LONG                dd_CurrentX;     
    LONG                dd_CurrentY;     
    ULONG               dd_Flags;        
    UWORD               dd_ViewModes;    
};
 


struct DiskObject {
    UWORD               do_Magic;  
    UWORD               do_Version;  
    struct Gadget       do_Gadget;       
    UBYTE               do_Type;
    char *              do_DefaultTool;
    char **             do_ToolTypes;
    LONG                do_CurrentX;
    LONG                do_CurrentY;
    struct DrawerData * do_DrawerData;
    char *              do_ToolWindow;   
    LONG                do_StackSize;    

};




 


struct FreeList {
    WORD                fl_NumFree;
    struct List         fl_MemList;
};



 



















 




 


 




 


struct AppMessage {
    struct Message am_Message;   
    UWORD am_Type;               
    ULONG am_UserData;           
    ULONG am_ID;                 
    LONG am_NumArgs;             
    struct WBArg *am_ArgList;    
    UWORD am_Version;            
    UWORD am_Class;              
    WORD am_MouseX;              
    WORD am_MouseY;              
    ULONG am_Seconds;            
    ULONG am_Micros;             
    ULONG am_Reserved[8];        
};




 
struct  AppWindow       { void *aw_PRIVATE;  };
struct  AppIcon         { void *ai_PRIVATE;  };
struct          AppMenuItem     { void *ami_PRIVATE; };



# 29 "local/defs.h" 2

# 29 "local/defs.h" 2

# 1 "dinclude:amiga20/workbench/icon.h" 3












 





# 30 "local/defs.h" 2

# 30 "local/defs.h" 2

# 1 "dinclude:amiga20/clib/macros.h" 3












 







# 31 "local/defs.h" 2

# 31 "local/defs.h" 2


# 1 "dinclude:stdlib.h" 3





 






# 1 "dinclude:stddef.h" 4





 










typedef int ptrdiff_t;
typedef unsigned int size_t;
typedef char wchar_t;




# 13 "dinclude:stdlib.h" 3

# 13 "dinclude:stdlib.h" 3


typedef struct div_t {
    int     quot;
    int     rem;
} div_t;

typedef struct ldiv_t {
    long    quot;
    long    rem;
} ldiv_t;










 



extern void abort(void);





extern int atexit(void (*)(void));

extern double atof(const char *);
extern double strtod(const char *, char **);

extern int atoi(const char *);
extern long atol(const char *);
extern void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
extern void *calloc(size_t, size_t);
extern div_t div(int, int);
extern void exit(int);
extern void free(void *);
extern char *getenv(const char *);
extern int  setenv(const char *, const char *);
extern long labs(long);
extern ldiv_t ldiv(long, long);
extern void *malloc(size_t);
extern int mblen(const char *, size_t);
extern size_t mbstowcs(wchar_t *, const char *, size_t);
extern int mbtowc(wchar_t *, const char *, size_t);
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
extern int rand(void);
extern void srand(unsigned int);
extern void *realloc(void *, size_t);
extern long strtol(const char *,char **, int);
extern unsigned long strtoul(const char *, char **, int);
extern int system(const char *);
extern int system13(const char *);   
extern size_t wcstombs(wchar_t *, const char *, size_t);
extern int wctomb(char *, wchar_t);



 

extern int getfnl(const char *, char *, size_t, int);
extern int mkdir(const char *);
extern int expand_args(int, const char **, int *, char ***);
extern int (*onbreak(int (*)()))(void);
extern __stkargs void _exit(int);
extern void chkabort(void);
extern void stack_abort(void);
extern void sleep(int);

extern int _SystemBoolTag;
extern int _SystemBoolTagValue;




# 33 "local/defs.h" 2

# 33 "local/defs.h" 2

# 1 "dinclude:stdio.h" 3





 









typedef void *va_list;


typedef long    fpos_t;

typedef struct FILE {
    struct FILE *sd_Next;
    struct FILE **sd_Prev;
    unsigned char   *sd_RBuf;
    unsigned char   *sd_RPtr;
    unsigned char   *sd_WBuf;
    unsigned char   *sd_WPtr;
    long    sd_BufSiz;
    int     sd_RLeft;
    int     sd_WLeft;
    int     sd_Flags;
    int     sd_Error;        
    fpos_t  sd_Offset;       
    int     sd_Fd;
    short   sd_UC;           
    char    *sd_Name;        
} FILE;

typedef struct _IOFDS {
    long    fd_Fh;
    long    fd_Flags;
    long    (*fd_Exec)(long, int, void *, void *);    
    char    *fd_FileName;    
} _IOFDS;

extern FILE _Iob[3];                 
extern FILE *_Iod;                   

extern _IOFDS _IoStaticFD[3];        
extern _IOFDS *_IoFD;                
extern short _IoFDLimit;
extern int _bufsiz;                  





































extern void perror(const char *);

extern int    fclose(FILE *);
extern int    fflush(FILE *);
extern FILE * fopen(const char *, const char *);
extern FILE * freopen(const char *, const char *, FILE *);
extern void   setbuf(FILE *, char *);
extern int    setvbuf(FILE *, char *, int, size_t);

extern int    remove(const char *);
extern int    rename(const char *, const char *);
extern FILE * tmpfile(void);
extern char * tmpnam(char *);

extern int    fgetpos(FILE *, fpos_t *);
extern int    fseek(FILE *, long, int);
extern int    fsetpos(FILE *, const fpos_t *);
extern long   ftell(FILE *);
extern void   rewind(FILE *);

extern int    fgetc(FILE *);
extern char * fgets(char *, int, FILE *);
extern int    fputc(unsigned char, FILE *);
extern int    fputs(const char *, FILE *);











 








extern char * gets(char *);
extern int    puts(const char *);
extern int    ungetc(int, FILE *);

extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);

extern int    fprintf(FILE *, const char *, ...);
extern int    fscanf(FILE *, const char *, ...);
extern int    printf(const char *, ...);
extern int    scanf(const char *, ...);
extern int    sprintf(char *, const char *, ...);
extern int    sscanf(char *, const char *, ...);

extern int    vfprintf(FILE *, const char *, va_list);
extern int    vprintf(const char *, va_list);
extern int    vsprintf(char *, const char *, va_list);



 

extern FILE * fdopen(int, const char *);
extern int    chdir(const char *);
extern char * getcwd(char *, int);
extern int    unlink(const char *);



 

extern int    fhprintf(long, const char *, ...);
extern int    vfhprintf(long, const char *, va_list);




# 34 "local/defs.h" 2

# 34 "local/defs.h" 2

# 1 "dinclude:stdarg.h" 3





 














# 35 "local/defs.h" 2

# 35 "local/defs.h" 2

# 1 "dinclude:fcntl.h" 3





 
















 








 













extern int write(int, const void *, unsigned int);
extern int read(int, void *, unsigned int);
extern int close(int);
extern int open(const char *, int, ...);
extern int creat(const char *, int);
extern long lseek(int, long, int);

extern void *fdtofh(int);



extern _IOFDS *__getfh(int);         
extern _IOFDS *_MakeFD(int *);       




 

extern int fcntl(int, int, int);
extern int isatty(int);
extern int access(const char *, int);








# 36 "local/defs.h" 2

# 36 "local/defs.h" 2

# 1 "dinclude:string.h" 3





 









extern int memcmp(const void *, const void *, size_t);
extern int strcmp(const char *, const char *);
extern int strcoll(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern size_t strxfrm(char *, const char *, size_t);

extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, size_t);

extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern char *strcpy(char *, const char *);
extern char *stpcpy(char *, const char *);
extern char *strncpy(char *, const char *, size_t);
extern void *memset(void *, int, size_t);
extern char *strerror(int);
extern size_t strlen(const char *);

extern void *memchr(const void *, int, size_t);
extern char *strchr(const char *, int);
extern size_t strcspn(const char *, const char *);
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern size_t strspn(const char *, const char *);
extern char *strstr(const char *, const char *);
extern char *strtok(char *, const char *);

extern double strtod(const char *, char **);
extern long   strtol(const char *, char **, int);
extern unsigned long strtoul(const char *, char **, int);



 

extern void *movmem(const void *, const void *, size_t);
extern void *setmem(void *, size_t, int);
extern void *cmpmem(const void *, const void *, size_t);
extern void *clrmem(void *, size_t);
extern void *bzero(void *, size_t);
extern void *bcopy(const void *, void *, size_t);
extern void *bcmp(const void *, const void *, size_t);
extern char *strupper(char *);



 

extern int stricmp(const char *, const char *);
extern int strnicmp(const char *, const char *, int);
extern int strbpl(char **, int, const char *);
extern void strins(char *, const char *);
extern char *strdup(const char *);
extern char *stpbrk(const char *, const char *);
extern const char *stpchr(const char *, char);





# 37 "local/defs.h" 2

# 37 "local/defs.h" 2

# 1 "dinclude:lib/rexx.h" 3







 











long PlaceRexxCommand(struct MsgPort *, char *, char **, long *);
long PlaceRexxCommandDirect(struct MsgPort *, char *, char *, char **, long *);
void ProcessRexxCommands(struct MsgPort *);
short CreateGlobalDiceRexxPort(struct MsgPort *, char *);
short CreateDiceRexxPort(struct MsgPort *, char *);
void DeleteDiceRexxPort(struct MsgPort *);
int GetDiceRexxPortSlot(struct MsgPort *, char **);

extern struct Library *RexxSysBase;  
















 

long DoRexxCommand(void *rexxmsg, struct MsgPort *port, char *arg0, char **resstr);

extern char *RexxHostName;
extern short RexxSigBit;



# 38 "local/defs.h" 2

# 38 "local/defs.h" 2

# 1 "dinclude:lib/bcpl.h" 3





 










# 39 "local/defs.h" 2

# 39 "local/defs.h" 2

# 1 "dinclude:lib/misc.h" 3







 





# 1 "dinclude:stdarg.h" 4





 














# 14 "dinclude:lib/misc.h" 3

# 14 "dinclude:lib/misc.h" 3



# 1 "dinclude:stdio.h" 4





 

























 
 

 
 





 
 


 
 

 
 

 








































































 




























 








 







# 17 "dinclude:lib/misc.h" 3

# 17 "dinclude:lib/misc.h" 3


long rega4(void);
void __closeall(void);
void _finitdesc(FILE *, int, int);
int _parseargs1(char *, int);
void _parseargs2(char *, char **, int);
long _SearchResident(char *);
long _SearchPath(char *);
__stkargs long _ExecSeg(long, char *, long, void *);
int exec_dcc(char *, char *);
int _pfmt(char *, va_list, unsigned int (*)(char *, size_t, size_t, void *), void *);
int _pfmtone(char, va_list *, unsigned int (*)(char *, size_t, size_t, void *), void *, short, short, short, int);
int _sfmt(unsigned char *, va_list, int (*)(void *), int (*)(int, void *), void *, int *);
int _sfmtone(char *, short *, void *, int (*)(void *), void *, short, short, short);
int __fclose(FILE *);
int _filbuf(FILE *);
void _MakeCacheFD(_IOFDS *, void *, long);
long _CacheFDIoctl(long, int, void *, void *);
short OpenConsole(const char *);

__stkargs _slow_bcopy(void *, void *, long);
__stkargs _slow_bzero(void *, long);
__stkargs _slow_bset(void *, long, int);


void _SetWildStack(long);
void *_ParseWild(const char *, short);
int _CompWild(const char *, void *, void *);
void _FreeWild(void *);


extern long _TimeCompensation;
extern int _DiceCacheEnable;



# 40 "local/defs.h" 2

# 40 "local/defs.h" 2


# 1 "dinclude:clib/exec_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/exec_protos.h" 3

# 5 "dinclude:clib/exec_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/exec_protos.h" 3

# 6 "dinclude:clib/exec_protos.h" 3

# 1 "dinclude:amiga20/clib/exec_protos.h" 4












 



 
ULONG Supervisor( unsigned long (*userFunction)() );
 
 
void InitCode( unsigned long startClass, unsigned long version );
void InitStruct( APTR initTable, APTR memory, unsigned long size );
struct Library *MakeLibrary( APTR funcInit, APTR structInit,
        unsigned long (*libInit)(), unsigned long dataSize,
        unsigned long segList );
void MakeFunctions( APTR target, APTR functionArray,
        unsigned long funcDispBase );
struct Resident *FindResident( UBYTE *name );
void InitResident( struct Resident *resident, unsigned long segList );
 
void Alert( unsigned long alertNum );
void Debug( unsigned long flags );
 
void Disable( void );
void Enable( void );
void Forbid( void );
void Permit( void );
ULONG SetSR( unsigned long newSR, unsigned long mask );
APTR SuperState( void );
void UserState( APTR sysStack );
struct Interrupt *SetIntVector( long intNumber, struct Interrupt *interrupt );
void AddIntServer( long intNumber, struct Interrupt *interrupt );
void RemIntServer( long intNumber, struct Interrupt *interrupt );
void Cause( struct Interrupt *interrupt );
 
APTR Allocate( struct MemHeader *freeList, unsigned long byteSize );
void Deallocate( struct MemHeader *freeList, APTR memoryBlock,
        unsigned long byteSize );
APTR AllocMem( unsigned long byteSize, unsigned long requirements );
APTR AllocAbs( unsigned long byteSize, APTR location );
void FreeMem( APTR memoryBlock, unsigned long byteSize );
ULONG AvailMem( unsigned long requirements );
struct MemList *AllocEntry( struct MemList *entry );
void FreeEntry( struct MemList *entry );
 
void Insert( struct List *list, struct Node *node, struct Node *pred );
void AddHead( struct List *list, struct Node *node );
void AddTail( struct List *list, struct Node *node );
void Remove( struct Node *node );
struct Node *RemHead( struct List *list );
struct Node *RemTail( struct List *list );
void Enqueue( struct List *list, struct Node *node );
struct Node *FindName( struct List *list, UBYTE *name );
 
APTR AddTask( struct Task *task, APTR initPC, APTR finalPC );
void RemTask( struct Task *task );
struct Task *FindTask( UBYTE *name );
BYTE SetTaskPri( struct Task *task, long priority );
ULONG SetSignal( unsigned long newSignals, unsigned long signalSet );
ULONG SetExcept( unsigned long newSignals, unsigned long signalSet );
ULONG Wait( unsigned long signalSet );
void Signal( struct Task *task, unsigned long signalSet );
BYTE AllocSignal( long signalNum );
void FreeSignal( long signalNum );
LONG AllocTrap( long trapNum );
void FreeTrap( long trapNum );
 
void AddPort( struct MsgPort *port );
void RemPort( struct MsgPort *port );
void PutMsg( struct MsgPort *port, struct Message *message );
struct Message *GetMsg( struct MsgPort *port );
void ReplyMsg( struct Message *message );
struct Message *WaitPort( struct MsgPort *port );
struct MsgPort *FindPort( UBYTE *name );
 
void AddLibrary( struct Library *library );
void RemLibrary( struct Library *library );
struct Library *OldOpenLibrary( UBYTE *libName );
void CloseLibrary( struct Library *library );
APTR SetFunction( struct Library *library, long funcOffset,
        unsigned long (*newFunction)() );
void SumLibrary( struct Library *library );
 
void AddDevice( struct Device *device );
void RemDevice( struct Device *device );
BYTE OpenDevice( UBYTE *devName, unsigned long unit,
        struct IORequest *ioRequest, unsigned long flags );
void CloseDevice( struct IORequest *ioRequest );
BYTE DoIO( struct IORequest *ioRequest );
void SendIO( struct IORequest *ioRequest );
BOOL CheckIO( struct IORequest *ioRequest );
BYTE WaitIO( struct IORequest *ioRequest );
void AbortIO( struct IORequest *ioRequest );
 
void AddResource( APTR resource );
void RemResource( APTR resource );
APTR OpenResource( UBYTE *resName );
 
 
void RawDoFmt( UBYTE *formatString, APTR dataStream, void (*putChProc)(),
        APTR putChData );
ULONG GetCC( void );
ULONG TypeOfMem( APTR address );
ULONG Procure( struct Semaphore *semaport, struct Message *bidMsg );
void Vacate( struct Semaphore *semaport );
struct Library *OpenLibrary( UBYTE *libName, unsigned long version );
 
 
void InitSemaphore( struct SignalSemaphore *sigSem );
void ObtainSemaphore( struct SignalSemaphore *sigSem );
void ReleaseSemaphore( struct SignalSemaphore *sigSem );
ULONG AttemptSemaphore( struct SignalSemaphore *sigSem );
void ObtainSemaphoreList( struct List *sigSem );
void ReleaseSemaphoreList( struct List *sigSem );
struct SignalSemaphore *FindSemaphore( UBYTE *sigSem );
void AddSemaphore( struct SignalSemaphore *sigSem );
void RemSemaphore( struct SignalSemaphore *sigSem );
 
ULONG SumKickData( void );
 
ULONG AddMemList( unsigned long size, unsigned long attributes,
        unsigned long pri, APTR base, UBYTE *name );
void CopyMem( APTR source, APTR dest, unsigned long size );
void CopyMemQuick( APTR source, APTR dest, unsigned long size );
 
 
void CacheClearU( void );
void CacheClearE( APTR address, unsigned long length, unsigned long caches );
ULONG CacheControl( unsigned long cacheBits, unsigned long cacheMask );
 
APTR CreateIORequest( struct MsgPort *port, unsigned long size );
void DeleteIORequest( APTR iorequest );
struct MsgPort *CreateMsgPort( void );
void DeleteMsgPort( struct MsgPort *port );
void ObtainSemaphoreShared( struct SignalSemaphore *sigSem );
 
APTR AllocVec( unsigned long byteSize, unsigned long requirements );
void FreeVec( APTR memoryBlock );
APTR CreatePrivatePool( unsigned long requirements, unsigned long puddleSize,
        unsigned long puddleThresh );
void DeletePrivatePool( APTR poolHeader );
APTR AllocPooled( unsigned long memSize, APTR poolHeader );
void FreePooled( APTR memory, APTR poolHeader );
 
void ColdReboot( void );
void StackSwap( APTR newSize, APTR newSP, APTR newStack );
 
void ChildFree( APTR tid );
void ChildOrphan( APTR tid );
void ChildStatus( APTR tid );
void ChildWait( APTR tid );
 


# 7 "dinclude:clib/exec_protos.h" 3

# 7 "dinclude:clib/exec_protos.h" 3








# 42 "local/defs.h" 2

# 42 "local/defs.h" 2

# 1 "dinclude:clib/dos_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/dos_protos.h" 3

# 5 "dinclude:clib/dos_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/dos_protos.h" 3

# 6 "dinclude:clib/dos_protos.h" 3

# 1 "dinclude:amiga20/clib/dos_protos.h" 4












 
 








# 1 "dinclude:amiga20/dos/record.h" 5














 





 





 

struct RecordLock {
        BPTR    rec_FH;          
        ULONG   rec_Offset;      
        ULONG   rec_Length;      
        ULONG   rec_Mode;        
};



# 23 "dinclude:amiga20/clib/dos_protos.h" 4

# 23 "dinclude:amiga20/clib/dos_protos.h" 4



# 1 "dinclude:amiga20/dos/rdargs.h" 5














 













































 

struct CSource {
        UBYTE   *CS_Buffer;
        LONG    CS_Length;
        LONG    CS_CurChr;
};



























 

struct RDArgs {
        struct  CSource RDA_Source;      
        LONG    RDA_DAList;              
        UBYTE   *RDA_Buffer;             
        LONG    RDA_BufSiz;              
        UBYTE   *RDA_ExtHelp;            
        LONG    RDA_Flags;               
};











 






 




# 26 "dinclude:amiga20/clib/dos_protos.h" 4

# 26 "dinclude:amiga20/clib/dos_protos.h" 4



# 1 "dinclude:amiga20/dos/dosasl.h" 5














 




































 

struct AnchorPath {
        struct AChain   *ap_Base;        

        struct AChain   *ap_Last;        

        LONG    ap_BreakBits;    
        LONG    ap_FoundBreak;   
        BYTE    ap_Flags;        
        BYTE    ap_Reserved;
        WORD    ap_Strlen;       

        struct  FileInfoBlock ap_Info;
        UBYTE   ap_Buf[1];       
         
};







                                 
                                 
                                 



                                 
                                 














struct AChain {
        struct AChain *an_Child;
        struct AChain *an_Parent;
        BPTR    an_Lock;
        struct FileInfoBlock an_Info;
        BYTE    an_Flags;
        UBYTE   an_String[1];    
};
















 














 








 







# 29 "dinclude:amiga20/clib/dos_protos.h" 4

# 29 "dinclude:amiga20/clib/dos_protos.h" 4



# 1 "dinclude:amiga20/dos/var.h" 5














 






 
 
 
 

struct LocalVar {
        struct Node lv_Node;
        UWORD   lv_Flags;
        UBYTE   *lv_Value;
        ULONG   lv_Len;
};




 

 


 



 
 
 









# 32 "dinclude:amiga20/clib/dos_protos.h" 4

# 32 "dinclude:amiga20/clib/dos_protos.h" 4



# 1 "dinclude:amiga20/dos/notify.h" 5














 















 
 


 



 

struct NotifyMessage {
    struct Message nm_ExecMessage;
    ULONG  nm_Class;
    UWORD  nm_Code;
    struct NotifyRequest *nm_NReq;       
    ULONG  nm_DoNotTouch;                
    ULONG  nm_DoNotTouch2;               
};

 
 

struct NotifyRequest {
        UBYTE *nr_Name;
        UBYTE *nr_FullName;              
        ULONG nr_UserData;               
        ULONG nr_Flags;

        union {

            struct {
                struct MsgPort *nr_Port;         
            } nr_Msg;

            struct {
                struct Task *nr_Task;            
                UBYTE nr_SignalNum;              
                UBYTE nr_pad[3];
            } nr_Signal;
        } nr_stuff;

        ULONG nr_Reserved[4];            

         
        ULONG nr_MsgCount;               
        struct MsgPort *nr_Handler;      
};

 





 


 







 




# 35 "dinclude:amiga20/clib/dos_protos.h" 4

# 35 "dinclude:amiga20/clib/dos_protos.h" 4



# 1 "dinclude:amiga20/dos/datetime.h" 5














 








 

 
struct DateTime {
        struct DateStamp dat_Stamp;      
        UBYTE   dat_Format;              
        UBYTE   dat_Flags;               
        UBYTE   *dat_StrDay;             
        UBYTE   *dat_StrDate;            
        UBYTE   *dat_StrTime;            
};

 


 








 









# 38 "dinclude:amiga20/clib/dos_protos.h" 4

# 38 "dinclude:amiga20/clib/dos_protos.h" 4

BPTR Open( UBYTE *name, long accessMode );
LONG Close( BPTR file );
LONG Read( BPTR file, APTR buffer, long length );
LONG Write( BPTR file, APTR buffer, long length );
BPTR Input( void );
BPTR Output( void );
LONG Seek( BPTR file, long position, long offset );
LONG DeleteFile( UBYTE *name );
LONG Rename( UBYTE *oldName, UBYTE *newName );
BPTR Lock( UBYTE *name, long type );
void UnLock( BPTR lock );
BPTR DupLock( BPTR lock );
LONG Examine( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG ExNext( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG Info( BPTR lock, struct InfoData *parameterBlock );
BPTR CreateDir( UBYTE *name );
BPTR CurrentDir( BPTR lock );
LONG IoErr( void );
struct MsgPort *CreateProc( UBYTE *name, long pri, BPTR segList,
        long stackSize );
void Exit( long returnCode );
BPTR LoadSeg( UBYTE *name );
void UnLoadSeg( BPTR seglist );
struct MsgPort *DeviceProc( UBYTE *name );
LONG SetComment( UBYTE *name, UBYTE *comment );
LONG SetProtection( UBYTE *name, long protect );
struct DateStamp *DateStamp( struct DateStamp *date );
void Delay( long timeout );
LONG WaitForChar( BPTR file, long timeout );
BPTR ParentDir( BPTR lock );
LONG IsInteractive( BPTR file );
LONG Execute( UBYTE *string, BPTR file, BPTR file2 );
 
 
APTR AllocDosObject( unsigned long type, struct TagItem *tags );
APTR AllocDosObjectTagList( unsigned long type, struct TagItem *tags );
APTR AllocDosObjectTags( unsigned long type, unsigned long tag1type, ... );
void FreeDosObject( unsigned long type, APTR ptr );
 
LONG DoPkt( struct MsgPort *port, long action, long arg1, long arg2, long arg3,
        long arg4, long arg5 );
LONG DoPkt0( struct MsgPort *port, long action );
LONG DoPkt1( struct MsgPort *port, long action, long arg1 );
LONG DoPkt2( struct MsgPort *port, long action, long arg1, long arg2 );
LONG DoPkt3( struct MsgPort *port, long action, long arg1, long arg2,
        long arg3 );
LONG DoPkt4( struct MsgPort *port, long action, long arg1, long arg2,
        long arg3, long arg4 );
void SendPkt( struct DosPacket *dp, struct MsgPort *port,
        struct MsgPort *replyport );
struct DosPacket *WaitPkt( void );
void ReplyPkt( struct DosPacket *dp, long res1, long res2 );
void AbortPkt( struct MsgPort *port, struct DosPacket *pkt );
 
BOOL LockRecord( BPTR fh, unsigned long offset, unsigned long length,
        unsigned long mode, unsigned long timeout );
BOOL LockRecords( struct RecordLock *recArray, unsigned long timeout );
BOOL UnLockRecord( BPTR fh, unsigned long offset, unsigned long length );
BOOL UnLockRecords( struct RecordLock *recArray );
 
BPTR SelectInput( BPTR fh );
BPTR SelectOutput( BPTR fh );
LONG FGetC( BPTR fh );
void FPutC( BPTR fh, unsigned long ch );
LONG UnGetC( BPTR fh, long character );
LONG FRead( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
LONG FWrite( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
UBYTE *FGets( BPTR fh, UBYTE *buf, unsigned long buflen );
LONG FPuts( BPTR fh, UBYTE *str );
void VFWritef( BPTR fh, UBYTE *format, LONG *argarray );
void FWritef( BPTR fh, UBYTE *format, long arg1, ... );
LONG VFPrintf( BPTR fh, UBYTE *format, LONG *argarray );
LONG FPrintf( BPTR fh, UBYTE *format, long arg1, ... );
void Flush( BPTR fh );
LONG SetVBuf( BPTR fh, UBYTE *buff, long type, long size );
 
BPTR DupLockFromFH( BPTR fh );
BPTR OpenFromLock( BPTR lock );
BPTR ParentOfFH( BPTR fh );
BOOL ExamineFH( BPTR fh, struct FileInfoBlock *fib );
LONG SetFileDate( UBYTE *name, struct DateStamp *date );
LONG NameFromLock( BPTR lock, UBYTE *buffer, long len );
LONG NameFromFH( BPTR fh, UBYTE *buffer, long len );
WORD SplitName( UBYTE *name, unsigned long seperator, UBYTE *buf, long oldpos,
        long size );
LONG SameLock( BPTR lock1, BPTR lock2 );
LONG SetMode( BPTR fh, long mode );
LONG ExAll( BPTR lock, struct ExAllData *buffer, long size, long data,
        struct ExAllControl *control );
LONG ReadLink( struct MsgPort *port, BPTR lock, UBYTE *path, UBYTE *buffer,
        unsigned long size );
LONG MakeLink( UBYTE *name, long dest, long soft );
LONG ChangeMode( long type, BPTR fh, long newmode );
LONG SetFileSize( BPTR fh, long pos, long mode );
 
LONG SetIoErr( long result );
BOOL Fault( long code, UBYTE *header, UBYTE *buffer, long len );
BOOL PrintFault( long code, UBYTE *header );
LONG ErrorReport( long code, long type, unsigned long arg1,
        struct MsgPort *device );
 
struct CommandLineInterface *Cli( void );
struct Process *CreateNewProc( struct TagItem *tags );
struct Process *CreateNewProcTagList( struct TagItem *tags );
struct Process *CreateNewProcTags( unsigned long tag1type, ... );
LONG RunCommand( BPTR seg, long stack, UBYTE *paramptr, long paramlen );
struct MsgPort *GetConsoleTask( void );
struct MsgPort *SetConsoleTask( struct MsgPort *task );
struct MsgPort *GetFileSysTask( void );
struct MsgPort *SetFileSysTask( struct MsgPort *task );
UBYTE *GetArgStr( void );
BOOL SetArgStr( UBYTE *string );
struct Process *FindCliProc( unsigned long num );
ULONG MaxCli( void );
BOOL SetCurrentDirName( UBYTE *name );
BOOL GetCurrentDirName( UBYTE *buf, long len );
BOOL SetProgramName( UBYTE *name );
BOOL GetProgramName( UBYTE *buf, long len );
BOOL SetPrompt( UBYTE *name );
BOOL GetPrompt( UBYTE *buf, long len );
BPTR SetProgramDir( BPTR lock );
BPTR GetProgramDir( void );
 
LONG SystemTagList( UBYTE *command, struct TagItem *tags );
LONG System( UBYTE *command, struct TagItem *tags );
LONG SystemTags( UBYTE *command, unsigned long tag1type, ... );
LONG AssignLock( UBYTE *name, BPTR lock );
BOOL AssignLate( UBYTE *name, UBYTE *path );
BOOL AssignPath( UBYTE *name, UBYTE *path );
BOOL AssignAdd( UBYTE *name, BPTR lock );
LONG RemAssignList( UBYTE *name, BPTR lock );
struct DevProc *GetDeviceProc( UBYTE *name, struct DevProc *dp );
void FreeDeviceProc( struct DevProc *dp );
struct DosList *LockDosList( unsigned long flags );
void UnLockDosList( unsigned long flags );
struct DosList *AttemptLockDosList( unsigned long flags );
BOOL RemDosEntry( struct DosList *dlist );
struct DosList *AddDosEntry( struct DosList *dlist );
struct DosList *FindDosEntry( struct DosList *dlist, UBYTE *name,
        unsigned long flags );
struct DosList *NextDosEntry( struct DosList *dlist, unsigned long flags );
struct DosList *MakeDosEntry( UBYTE *name, long type );
void FreeDosEntry( struct DosList *dlist );
BOOL IsFileSystem( UBYTE *name );
 
BOOL Format( UBYTE *filesystem, UBYTE *volumename, unsigned long dostype );
LONG Relabel( UBYTE *drive, UBYTE *newname );
LONG Inhibit( UBYTE *name, long onoff );
LONG AddBuffers( UBYTE *name, long number );
 
LONG CompareDates( struct DateStamp *date1, struct DateStamp *date2 );
LONG DateToStr( struct DateTime *datetime );
LONG StrToDate( struct DateTime *datetime );
 
BPTR InternalLoadSeg( BPTR fh, BPTR table, LONG *funcarray, LONG *stack );
void InternalUnLoadSeg( BPTR seglist, void (*freefunc)() );
BPTR NewLoadSeg( UBYTE *file, struct TagItem *tags );
BPTR NewLoadSegTagList( UBYTE *file, struct TagItem *tags );
BPTR NewLoadSegTags( UBYTE *file, unsigned long tag1type, ... );
LONG AddSegment( UBYTE *name, BPTR seg, long system );
struct Segment *FindSegment( UBYTE *name, struct Segment *seg, long system );
LONG RemSegment( struct Segment *seg );
 
LONG CheckSignal( long mask );
struct RDArgs *ReadArgs( UBYTE *template, LONG *array, struct RDArgs *args );
LONG FindArg( UBYTE *keyword, UBYTE *template );
LONG ReadItem( UBYTE *name, long maxchars, struct CSource *cSource );
LONG StrToLong( UBYTE *string, LONG *value );
LONG MatchFirst( UBYTE *pat, struct AnchorPath *anchor );
LONG MatchNext( struct AnchorPath *anchor );
void MatchEnd( struct AnchorPath *anchor );
BOOL ParsePattern( UBYTE *pat, UBYTE *buf, long buflen );
BOOL MatchPattern( UBYTE *pat, UBYTE *str );
 
void FreeArgs( struct RDArgs *args );
UBYTE *FilePart( UBYTE *path );
UBYTE *PathPart( UBYTE *path );
BOOL AddPart( UBYTE *dirname, UBYTE *filename, unsigned long size );
 
BOOL StartNotify( struct NotifyRequest *notify );
void EndNotify( struct NotifyRequest *notify );
 
BOOL SetVar( UBYTE *name, UBYTE *buffer, long size, long flags );
LONG GetVar( UBYTE *name, UBYTE *buffer, long size, long flags );
LONG DeleteVar( UBYTE *name, unsigned long flags );
struct LocalVar *FindVar( UBYTE *name, unsigned long type );
LONG CliInit( struct DosPacket *dp );
LONG CliInitNewcli( struct DosPacket *dp );
LONG CliInitRun( struct DosPacket *dp );
LONG WriteChars( UBYTE *buf, unsigned long buflen );
LONG PutStr( UBYTE *str );
LONG VPrintf( UBYTE *format, LONG *argarray );
LONG Printf( UBYTE *format, long arg1, ... );
 
BOOL ParsePatternNoCase( UBYTE *pat, UBYTE *buf, long buflen );
BOOL MatchPatternNoCase( UBYTE *pat, UBYTE *str );
 
BOOL SameDevice( BPTR lock1, BPTR lock2 );
 
 
 


# 7 "dinclude:clib/dos_protos.h" 3

# 7 "dinclude:clib/dos_protos.h" 3








# 43 "local/defs.h" 2

# 43 "local/defs.h" 2

# 1 "dinclude:clib/intuition_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/intuition_protos.h" 3

# 5 "dinclude:clib/intuition_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/intuition_protos.h" 3

# 6 "dinclude:clib/intuition_protos.h" 3

# 1 "dinclude:amiga20/clib/intuition_protos.h" 4












 
 



 
 
void OpenIntuition( void );
void Intuition( struct InputEvent *iEvent );
UWORD AddGadget( struct Window *window, struct Gadget *gadget,
        unsigned long position );
BOOL ClearDMRequest( struct Window *window );
void ClearMenuStrip( struct Window *window );
void ClearPointer( struct Window *window );
BOOL CloseScreen( struct Screen *screen );
void CloseWindow( struct Window *window );
LONG CloseWorkBench( void );
void CurrentTime( ULONG *seconds, ULONG *micros );
BOOL DisplayAlert( unsigned long alertNumber, UBYTE *string,
        unsigned long height );
void DisplayBeep( struct Screen *screen );
BOOL DoubleClick( unsigned long sSeconds, unsigned long sMicros,
        unsigned long cSeconds, unsigned long cMicros );
void DrawBorder( struct RastPort *rp, struct Border *border, long leftOffset,
        long topOffset );
void DrawImage( struct RastPort *rp, struct Image *image, long leftOffset,
        long topOffset );
void EndRequest( struct Requester *requester, struct Window *window );
struct Preferences *GetDefPrefs( struct Preferences *preferences, long size );
struct Preferences *GetPrefs( struct Preferences *preferences, long size );
void InitRequester( struct Requester *requester );
struct MenuItem *ItemAddress( struct Menu *menuStrip,
        unsigned long menuNumber );
BOOL ModifyIDCMP( struct Window *window, unsigned long flags );
void ModifyProp( struct Gadget *gadget, struct Window *window,
        struct Requester *requester, unsigned long flags,
        unsigned long horizPot, unsigned long vertPot,
        unsigned long horizBody, unsigned long vertBody );
void MoveScreen( struct Screen *screen, long dx, long dy );
void MoveWindow( struct Window *window, long dx, long dy );
void OffGadget( struct Gadget *gadget, struct Window *window,
        struct Requester *requester );
void OffMenu( struct Window *window, unsigned long menuNumber );
void OnGadget( struct Gadget *gadget, struct Window *window,
        struct Requester *requester );
void OnMenu( struct Window *window, unsigned long menuNumber );
struct Screen *OpenScreen( struct NewScreen *newScreen );
struct Window *OpenWindow( struct NewWindow *newWindow );
BOOL OpenWorkBench( void );
void PrintIText( struct RastPort *rp, struct IntuiText *iText, long left,
        long top );
void RefreshGadgets( struct Gadget *gadgets, struct Window *window,
        struct Requester *requester );
UWORD RemoveGadget( struct Window *window, struct Gadget *gadget );
 
 
 
void ReportMouse( long flag, struct Window *window );
void ReportMouse1( struct Window *window, long flag );
BOOL Request( struct Requester *requester, struct Window *window );
void ScreenToBack( struct Screen *screen );
void ScreenToFront( struct Screen *screen );
BOOL SetDMRequest( struct Window *window, struct Requester *requester );
BOOL SetMenuStrip( struct Window *window, struct Menu *menu );
void SetPointer( struct Window *window, UWORD *pointer, long height,
        long width, long xOffset, long yOffset );
void SetWindowTitles( struct Window *window, UBYTE *windowTitle,
        UBYTE *screenTitle );
void ShowTitle( struct Screen *screen, long showIt );
void SizeWindow( struct Window *window, long dx, long dy );
struct View *ViewAddress( void );
struct ViewPort *ViewPortAddress( struct Window *window );
void WindowToBack( struct Window *window );
void WindowToFront( struct Window *window );
BOOL WindowLimits( struct Window *window, long widthMin, long heightMin,
        unsigned long widthMax, unsigned long heightMax );
 
struct Preferences *SetPrefs( struct Preferences *preferences, long size,
        long inform );
 
LONG IntuiTextLength( struct IntuiText *iText );
BOOL WBenchToBack( void );
BOOL WBenchToFront( void );
 
BOOL AutoRequest( struct Window *window, struct IntuiText *body,
        struct IntuiText *posText, struct IntuiText *negText,
        unsigned long pFlag, unsigned long nFlag, unsigned long width,
        unsigned long height );
void BeginRefresh( struct Window *window );
struct Window *BuildSysRequest( struct Window *window, struct IntuiText *body,
        struct IntuiText *posText, struct IntuiText *negText,
        unsigned long flags, unsigned long width, unsigned long height );
void EndRefresh( struct Window *window, long complete );
void FreeSysRequest( struct Window *window );
void MakeScreen( struct Screen *screen );
void RemakeDisplay( void );
void RethinkDisplay( void );
 
APTR AllocRemember( struct Remember **rememberKey, unsigned long size,
        unsigned long flags );
 
void AlohaWorkbench( long wbport );
void FreeRemember( struct Remember **rememberKey, long reallyForget );
 
ULONG LockIBase( unsigned long dontknow );
void UnlockIBase( unsigned long ibLock );
 
LONG GetScreenData( APTR buffer, unsigned long size, unsigned long type,
        struct Screen *screen );
void RefreshGList( struct Gadget *gadgets, struct Window *window,
        struct Requester *requester, long numGad );
UWORD AddGList( struct Window *window, struct Gadget *gadget,
        unsigned long position, long numGad, struct Requester *requester );
UWORD RemoveGList( struct Window *remPtr, struct Gadget *gadget,
        long numGad );
LONG ActivateWindow( struct Window *window );
void RefreshWindowFrame( struct Window *window );
BOOL ActivateGadget( struct Gadget *gadgets, struct Window *window,
        struct Requester *requester );
void NewModifyProp( struct Gadget *gadget, struct Window *window,
        struct Requester *requester, unsigned long flags,
        unsigned long horizPot, unsigned long vertPot,
        unsigned long horizBody, unsigned long vertBody, long numGad );
 
LONG QueryOverscan( unsigned long displayID, struct Rectangle *rect,
        long oScanType );
void MoveWindowInFrontOf( struct Window *window,
        struct Window *behindWindow );
void ChangeWindowBox( struct Window *window, long left, long top, long width,
        long height );
struct Hook *SetEditHook( struct Hook *hook );
LONG SetMouseQueue( struct Window *window, unsigned long queueLength );
void ZipWindow( struct Window *window );
 
struct Screen *LockPubScreen( UBYTE *name );
void UnlockPubScreen( UBYTE *name, struct Screen *screen );
struct List *LockPubScreenList( void );
void UnlockPubScreenList( void );
UBYTE *NextPubScreen( struct Screen *screen, UBYTE *namebuf );
void SetDefaultPubScreen( UBYTE *name );
UWORD SetPubScreenModes( unsigned long modes );
UWORD PubScreenStatus( struct Screen *screen, unsigned long statusFlags );
 
struct RastPort *ObtainGIRPort( struct GadgetInfo *gInfo );
void ReleaseGIRPort( struct RastPort *rp );
void GadgetMouse( struct Gadget *gadget, struct GadgetInfo *gInfo,
        WORD *mousePoint );
 
void GetDefaultPubScreen( UBYTE *nameBuffer );
LONG EasyRequestArgs( struct Window *window, struct EasyStruct *easyStruct,
        ULONG *idcmpPtr, APTR args );
LONG EasyRequest( struct Window *window, struct EasyStruct *easyStruct,
        ULONG *idcmpPtr, APTR arg1, ... );
struct Window *BuildEasyRequestArgs( struct Window *window,
        struct EasyStruct *easyStruct, unsigned long idcmp, APTR args );
struct Window *BuildEasyRequest( struct Window *window,
        struct EasyStruct *easyStruct, unsigned long idcmp, APTR arg1, ... );
LONG SysReqHandler( struct Window *window, ULONG *idcmpPtr, long waitInput );
struct Window *OpenWindowTagList( struct NewWindow *newWindow,
        struct TagItem *tagList );
struct Window *OpenWindowTags( struct NewWindow *newWindow,
        unsigned long tag1Type, ... );
struct Screen *OpenScreenTagList( struct NewScreen *newScreen,
        struct TagItem *tagList );
struct Screen *OpenScreenTags( struct NewScreen *newScreen,
        unsigned long tag1Type, ... );
 
 
void DrawImageState( struct RastPort *rp, struct Image *image, long leftOffset,
        long topOffset, unsigned long state, struct DrawInfo *drawInfo );
BOOL PointInImage( unsigned long point, struct Image *image );
void EraseImage( struct RastPort *rp, struct Image *image, long leftOffset,
        long topOffset );
 
APTR NewObjectA( struct IClass *class, UBYTE *classID,
        struct TagItem *tagList );
APTR NewObject( struct IClass *class, UBYTE *classID, unsigned long tag1,
        ... );
 
void DisposeObject( APTR object );
ULONG SetAttrsA( APTR object, struct TagItem *tagList );
ULONG SetAttrs( APTR object, unsigned long tag1, ... );
 
ULONG GetAttr( unsigned long attrID, APTR object, ULONG *storagePtr );
 
 
ULONG SetGadgetAttrsA( struct Gadget *gadget, struct Window *window,
        struct Requester *requester, struct TagItem *tagList );
ULONG SetGadgetAttrs( struct Gadget *gadget, struct Window *window,
        struct Requester *requester, unsigned long tag1, ... );
 
 
APTR NextObject( APTR objectPtrPtr );
struct IClass *MakeClass( UBYTE *classID, UBYTE *superClassID,
        struct IClass *superClassPtr, unsigned long instanceSize,
        unsigned long flags );
void AddClass( struct IClass *class );
 
 
struct DrawInfo *GetScreenDrawInfo( struct Screen *screen );
void FreeScreenDrawInfo( struct Screen *screen, struct DrawInfo *drawInfo );
 
BOOL ResetMenuStrip( struct Window *window, struct Menu *menu );
void RemoveClass( struct IClass *classPtr );
BOOL FreeClass( struct IClass *classPtr );


# 7 "dinclude:clib/intuition_protos.h" 3

# 7 "dinclude:clib/intuition_protos.h" 3








# 44 "local/defs.h" 2

# 44 "local/defs.h" 2

# 1 "dinclude:clib/diskfont_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/diskfont_protos.h" 3

# 5 "dinclude:clib/diskfont_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/diskfont_protos.h" 3

# 6 "dinclude:clib/diskfont_protos.h" 3

# 1 "dinclude:amiga20/clib/diskfont_protos.h" 4












 
 





# 1 "dinclude:amiga20/libraries/diskfont.h" 5












 
















struct FontContents {
    char    fc_FileName[256];
    UWORD   fc_YSize;
    UBYTE   fc_Style;
    UBYTE   fc_Flags;
};

struct TFontContents {
    char    tfc_FileName[256-2];
    UWORD   tfc_TagCount;        
    



 
    UWORD   tfc_YSize;
    UBYTE   tfc_Style;
    UBYTE   tfc_Flags;
};





struct FontContentsHeader {
    UWORD   fch_FileID;          
    UWORD   fch_NumEntries;      
     
};





struct DiskFontHeader {
     
     
     
     
     
     
     
    struct Node dfh_DF;          
    UWORD   dfh_FileID;          
    UWORD   dfh_Revision;        
    LONG    dfh_Segment;         
    char    dfh_Name[32];  
    struct TextFont dfh_TF;      
};

 
 













struct AvailFonts {
    UWORD   af_Type;             
    struct TextAttr af_Attr;     
};

struct TAvailFonts {
    UWORD   taf_Type;            
    struct TTextAttr taf_Attr;   
};

struct AvailFontsHeader {
    UWORD   afh_NumEntries;       
     
};



# 20 "dinclude:amiga20/clib/diskfont_protos.h" 4

# 20 "dinclude:amiga20/clib/diskfont_protos.h" 4

struct TextFont *OpenDiskFont( struct TextAttr *textAttr );
LONG AvailFonts( STRPTR buffer, long bufBytes, long flags );
 
struct FontContentsHeader *NewFontContents( BPTR fontsLock, STRPTR fontName );
void DisposeFontContents( struct FontContentsHeader *fontContentsHeader );
 
struct DiskFontHeader *NewScaledDiskFont( struct TextFont *sourceFont,
        struct TextAttr *destTextAttr );


# 7 "dinclude:clib/diskfont_protos.h" 3

# 7 "dinclude:clib/diskfont_protos.h" 3








# 45 "local/defs.h" 2

# 45 "local/defs.h" 2

# 1 "dinclude:clib/graphics_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/graphics_protos.h" 3

# 5 "dinclude:clib/graphics_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/graphics_protos.h" 3

# 6 "dinclude:clib/graphics_protos.h" 3

# 1 "dinclude:amiga20/clib/graphics_protos.h" 4












 
 








# 1 "dinclude:amiga20/graphics/gels.h" 5












 





 
 





 





 
 



 







 










 















 

struct VSprite
{
 
 
    struct VSprite   *NextVSprite;
    struct VSprite   *PrevVSprite;




 
    struct VSprite   *DrawPath;      
    struct VSprite   *ClearPath;     



 
    WORD OldY, OldX;           

 
    WORD Flags;        


 


 
    WORD Y, X;                 

    WORD Height;
    WORD Width;        
    WORD Depth;        

    WORD MeMask;               
    WORD HitMask;              

    WORD *ImageData;           



 
    WORD *BorderLine;          
    WORD *CollMask;            

 
    WORD *SprColors;

    struct Bob *VSBob;        
 











 
    BYTE PlanePick;
    BYTE PlaneOnOff;

    WORD VUserExt;       
};

struct Bob
 
{
 

 
    WORD Flags;  

 
    WORD *SaveBuffer;    

 
    WORD *ImageShadow;



 
    struct Bob *Before;  
    struct Bob *After;   

    struct VSprite   *BobVSprite;    

    struct AnimComp  *BobComp;       

    struct DBufPacket *DBuffer;      

    WORD BUserExt;           
};

struct AnimComp
{
 

 
    WORD Flags;              




 
    WORD Timer;

 
 
    WORD TimeSet;

 
    struct AnimComp  *NextComp;
    struct AnimComp  *PrevComp;

 
    struct AnimComp  *NextSeq;
    struct AnimComp  *PrevSeq;

    WORD (*AnimCRoutine)();  

    WORD YTrans;      
    WORD XTrans;      

    struct AnimOb    *HeadOb;

    struct Bob       *AnimBob;
};

struct AnimOb
{
 
    struct AnimOb    *NextOb, *PrevOb;

 
    LONG Clock;

    WORD AnOldY, AnOldX;             

 
    WORD AnY, AnX;                   

 
    WORD YVel, XVel;                 
    WORD YAccel, XAccel;             

    WORD RingYTrans, RingXTrans;     

    WORD (*AnimORoutine)();         
 

    struct AnimComp  *HeadComp;      

    WORD AUserExt;           
};



 
struct DBufPacket
{
    WORD BufY, BufX;                 
    struct VSprite   *BufPath;       

 
 
    WORD *BufBuffer;
};



 



 




 





 

 
struct collTable
{
    int (*collPtrs[16])();
};



# 23 "dinclude:amiga20/clib/graphics_protos.h" 4

# 23 "dinclude:amiga20/clib/graphics_protos.h" 4















# 1 "dinclude:amiga20/graphics/regions.h" 5












 









struct RegionRectangle
{
    struct RegionRectangle *Next,*Prev;
    struct Rectangle bounds;
};

struct Region
{
    struct Rectangle bounds;
    struct RegionRectangle *RegionRectangle;
};



# 38 "dinclude:amiga20/clib/graphics_protos.h" 4

# 38 "dinclude:amiga20/clib/graphics_protos.h" 4



# 1 "dinclude:amiga20/graphics/sprite.h" 5












 







struct SimpleSprite
{
    UWORD *posctldata;
    UWORD height;
    UWORD   x,y;     
    UWORD   num;
};



# 41 "dinclude:amiga20/clib/graphics_protos.h" 4

# 41 "dinclude:amiga20/clib/graphics_protos.h" 4






# 1 "dinclude:amiga20/hardware/blit.h" 5












 






 
 








 










 
























 






















 
struct bltnode
{
    struct  bltnode *n;
    int     (*function)();
    char    stat;
    short   blitsize;
    short   beamsync;
    int     (*cleanup)();
};

 





# 47 "dinclude:amiga20/clib/graphics_protos.h" 4

# 47 "dinclude:amiga20/clib/graphics_protos.h" 4

 
LONG BltBitMap( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct BitMap *destBitMap, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm, unsigned long mask,
        PLANEPTR tempA );
void BltTemplate( PLANEPTR source, long xSrc, long srcMod,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize );
 
void ClearEOL( struct RastPort *rp );
void ClearScreen( struct RastPort *rp );
WORD TextLength( struct RastPort *rp, STRPTR string, unsigned long count );
LONG Text( struct RastPort *rp, STRPTR string, unsigned long count );
LONG SetFont( struct RastPort *rp, struct TextFont *textFont );
struct TextFont *OpenFont( struct TextAttr *textAttr );
void CloseFont( struct TextFont *textFont );
ULONG AskSoftStyle( struct RastPort *rp );
ULONG SetSoftStyle( struct RastPort *rp, unsigned long style,
        unsigned long enable );
 
void AddBob( struct Bob *bob, struct RastPort *rp );
void AddVSprite( struct VSprite *vSprite, struct RastPort *rp );
void DoCollision( struct RastPort *rp );
void DrawGList( struct RastPort *rp, struct ViewPort *vp );
void InitGels( struct VSprite *head, struct VSprite *tail,
        struct GelsInfo *gelsInfo );
void InitMasks( struct VSprite *vSprite );
void RemIBob( struct Bob *bob, struct RastPort *rp, struct ViewPort *vp );
void RemVSprite( struct VSprite *vSprite );
void SetCollision( unsigned long num,
        void (*routine)(struct VSprite *vSprite, APTR),
        struct GelsInfo *gelsInfo );
void SortGList( struct RastPort *rp );
void AddAnimOb( struct AnimOb *anOb, struct AnimOb **anKey,
        struct RastPort *rp );
void Animate( struct AnimOb **anKey, struct RastPort *rp );
BOOL GetGBuffers( struct AnimOb *anOb, struct RastPort *rp, long flag );
void InitGMasks( struct AnimOb *anOb );
 
void DrawEllipse( struct RastPort *rp, long xCenter, long yCenter, long a,
        long b );
LONG AreaEllipse( struct RastPort *rp, long xCenter, long yCenter, long a,
        long b );
void LoadRGB4( struct ViewPort *vp, UWORD *colors, long count );
void InitRastPort( struct RastPort *rp );
void InitVPort( struct ViewPort *vp );
void MrgCop( struct View *view );
void MakeVPort( struct View *view, struct ViewPort *vp );
void LoadView( struct View *view );
void WaitBlit( void );
void SetRast( struct RastPort *rp, unsigned long pen );
void Move( struct RastPort *rp, long x, long y );
void Draw( struct RastPort *rp, long x, long y );
LONG AreaMove( struct RastPort *rp, long x, long y );
LONG AreaDraw( struct RastPort *rp, long x, long y );
LONG AreaEnd( struct RastPort *rp );
void WaitTOF( void );
void QBlit( struct bltnode *blit );
void InitArea( struct AreaInfo *areaInfo, APTR vectorBuffer,
        long maxVectors );
void SetRGB4( struct ViewPort *vp, long index, unsigned long red,
        unsigned long green, unsigned long blue );
void QBSBlit( struct bltnode *blit );
void BltClear( PLANEPTR memBlock, unsigned long byteCount,
        unsigned long flags );
void RectFill( struct RastPort *rp, long xMin, long yMin, long xMax,
        long yMax );
void BltPattern( struct RastPort *rp, PLANEPTR mask, long xMin, long yMin,
        long xMax, long yMax, unsigned long maskBPR );
ULONG ReadPixel( struct RastPort *rp, long x, long y );
LONG WritePixel( struct RastPort *rp, long x, long y );
BOOL Flood( struct RastPort *rp, unsigned long mode, long x, long y );
void PolyDraw( struct RastPort *rp, long count, WORD *polyTable );
void SetAPen( struct RastPort *rp, unsigned long pen );
void SetBPen( struct RastPort *rp, unsigned long pen );
void SetDrMd( struct RastPort *rp, unsigned long drawMode );
void InitView( struct View *view );
void CBump( struct UCopList *copList );
void CMove( struct UCopList *copList, APTR destination, long data );
void CWait( struct UCopList *copList, long v, long h );
LONG VBeamPos( void );
void InitBitMap( struct BitMap *bitMap, long depth, long width, long height );
void ScrollRaster( struct RastPort *rp, long dx, long dy, long xMin, long yMin,
        long xMax, long yMax );
void WaitBOVP( struct ViewPort *vp );
WORD GetSprite( struct SimpleSprite *sprite, long num );
void FreeSprite( long num );
void ChangeSprite( struct ViewPort *vp, struct SimpleSprite *sprite,
        PLANEPTR newData );
void MoveSprite( struct ViewPort *vp, struct SimpleSprite *sprite, long x,
        long y );
void LockLayerRom( struct Layer *layer );
void UnlockLayerRom( struct Layer *layer );
void SyncSBitMap( struct Layer *layer );
void CopySBitMap( struct Layer *layer );
void OwnBlitter( void );
void DisownBlitter( void );
struct TmpRas *InitTmpRas( struct TmpRas *tmpRas, PLANEPTR buffer,
        long size );
void AskFont( struct RastPort *rp, struct TextAttr *textAttr );
void AddFont( struct TextFont *textFont );
void RemFont( struct TextFont *textFont );
PLANEPTR AllocRaster( unsigned long width, unsigned long height );
void FreeRaster( PLANEPTR p, unsigned long width, unsigned long height );
void AndRectRegion( struct Region *region, struct Rectangle *rectangle );
BOOL OrRectRegion( struct Region *region, struct Rectangle *rectangle );
struct Region *NewRegion( void );
BOOL ClearRectRegion( struct Region *region, struct Rectangle *rectangle );
void ClearRegion( struct Region *region );
void DisposeRegion( struct Region *region );
void FreeVPortCopLists( struct ViewPort *vp );
void FreeCopList( struct CopList *copList );
void ClipBlit( struct RastPort *srcRP, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm );
BOOL XorRectRegion( struct Region *region, struct Rectangle *rectangle );
void FreeCprList( struct cprlist *cprList );
struct ColorMap *GetColorMap( long entries );
void FreeColorMap( struct ColorMap *colorMap );
ULONG GetRGB4( struct ColorMap *colorMap, long entry );
void ScrollVPort( struct ViewPort *vp );
struct CopList *UCopperListInit( struct UCopList *uCopList, long n );
void FreeGBuffers( struct AnimOb *anOb, struct RastPort *rp, long flag );
void BltBitMapRastPort( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm );
BOOL OrRegionRegion( struct Region *srcRegion, struct Region *destRegion );
BOOL XorRegionRegion( struct Region *srcRegion, struct Region *destRegion );
BOOL AndRegionRegion( struct Region *srcRegion, struct Region *destRegion );
void SetRGB4CM( struct ColorMap *colorMap, long index, unsigned long red,
        unsigned long green, unsigned long blue );
void BltMaskBitMapRastPort( struct BitMap *srcBitMap, long xSrc, long ySrc,
        struct RastPort *destRP, long xDest, long yDest, long xSize,
        long ySize, unsigned long minterm, PLANEPTR bltMask );
BOOL AttemptLockLayerRom( struct Layer *layer );
 
APTR GfxNew( unsigned long gfxNodeType );
void GfxFree( APTR gfxNodePtr );
void GfxAssociate( APTR associateNode, APTR gfxNodePtr );
void BitMapScale( struct BitScaleArgs *bitScaleArgs );
UWORD ScalerDiv( unsigned long factor, unsigned long numerator,
        unsigned long denominator );
WORD TextExtent( struct RastPort *rp, STRPTR string, long count,
        struct TextExtent *textExtent );
ULONG TextFit( struct RastPort *rp, STRPTR string, unsigned long strLen,
        struct TextExtent *textExtent, struct TextExtent *constrainingExtent,
        long strDirection, unsigned long constrainingBitWidth,
        unsigned long constrainingBitHeight );
APTR GfxLookUp( APTR associateNode );
BOOL VideoControl( struct ColorMap *colorMap, struct TagItem *tagarray );
struct MonitorSpec *OpenMonitor( STRPTR monitorName,
        unsigned long displayID );
BOOL CloseMonitor( struct MonitorSpec *monitorSpec );
DisplayInfoHandle FindDisplayInfo( unsigned long displayID );
ULONG NextDisplayInfo( unsigned long displayID );
ULONG GetDisplayInfoData( DisplayInfoHandle handle, UBYTE *buf,
        unsigned long size, unsigned long tagID, unsigned long displayID );
void FontExtent( struct TextFont *font, struct TextExtent *fontExtent );
LONG ReadPixelLine8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long width, UBYTE *array,
        struct RastPort *tempRP );
LONG WritePixelLine8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long width, UBYTE *array,
        struct RastPort *tempRP );
LONG ReadPixelArray8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long xstop, unsigned long ystop,
        UBYTE *array, struct RastPort *temprp );
LONG WritePixelArray8( struct RastPort *rp, unsigned long xstart,
        unsigned long ystart, unsigned long xstop, unsigned long ystop,
        UBYTE *array, struct RastPort *temprp );
LONG GetVPModeID( struct ViewPort *vp );
LONG ModeNotAvailable( unsigned long modeID );
WORD WeighTAMatch( struct TextAttr *reqTextAttr,
        struct TextAttr *targetTextAttr, struct TagItem *targetTags );
void EraseRect( struct RastPort *rp, long xMin, long yMin, long xMax,
        long yMax );
ULONG ExtendFont( struct TextFont *font, struct TagItem *fontTags );
void StripFont( struct TextFont *font );


# 7 "dinclude:clib/graphics_protos.h" 3

# 7 "dinclude:clib/graphics_protos.h" 3








# 46 "local/defs.h" 2

# 46 "local/defs.h" 2

# 1 "dinclude:clib/gadtools_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/gadtools_protos.h" 3

# 5 "dinclude:clib/gadtools_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/gadtools_protos.h" 3

# 6 "dinclude:clib/gadtools_protos.h" 3

# 1 "dinclude:amiga20/clib/gadtools_protos.h" 4












 
 












 
 
 
 
struct Gadget *CreateGadgetA( unsigned long kind, struct Gadget *gad,
        struct NewGadget *ng, struct TagItem *taglist );
struct Gadget *CreateGadget( unsigned long kind, struct Gadget *gad,
        struct NewGadget *ng, Tag tag1, ... );
void FreeGadgets( struct Gadget *gad );
void GT_SetGadgetAttrsA( struct Gadget *gad, struct Window *win,
        struct Requester *req, struct TagItem *taglist );
void GT_SetGadgetAttrs( struct Gadget *gad, struct Window *win,
        struct Requester *req, Tag tag1, ... );
 
 
 
struct Menu *CreateMenusA( struct NewMenu *newmenu, struct TagItem *taglist );
struct Menu *CreateMenus( struct NewMenu *newmenu, Tag tag1, ... );
void FreeMenus( struct Menu *menu );
BOOL LayoutMenuItemsA( struct MenuItem *firstitem, APTR vi,
        struct TagItem *taglist );
BOOL LayoutMenuItems( struct MenuItem *firstitem, APTR vi, Tag tag1, ... );
BOOL LayoutMenusA( struct Menu *firstmenu, APTR vi, struct TagItem *taglist );
BOOL LayoutMenus( struct Menu *firstmenu, APTR vi, Tag tag1, ... );
 
 
 
struct IntuiMessage *GT_GetIMsg( struct MsgPort *iport );
void GT_ReplyIMsg( struct IntuiMessage *imsg );
void GT_RefreshWindow( struct Window *win, struct Requester *req );
void GT_BeginRefresh( struct Window *win );
void GT_EndRefresh( struct Window *win, long complete );
struct IntuiMessage *GT_FilterIMsg( struct IntuiMessage *imsg );
struct IntuiMessage *GT_PostFilterIMsg( struct IntuiMessage *imsg );
struct Gadget *CreateContext( struct Gadget **glistptr );
 
 
 
void DrawBevelBoxA( struct RastPort *rport, long left, long top, long width,
        long height, struct TagItem *taglist );
void DrawBevelBox( struct RastPort *rport, long left, long top, long width,
        long height, Tag tag1, ... );
 
 
 
APTR GetVisualInfoA( struct Screen *screen, struct TagItem *taglist );
APTR GetVisualInfo( struct Screen *screen, Tag tag1, ... );
void FreeVisualInfo( APTR vi );
 
 
 


# 7 "dinclude:clib/gadtools_protos.h" 3

# 7 "dinclude:clib/gadtools_protos.h" 3








# 47 "local/defs.h" 2

# 47 "local/defs.h" 2

# 1 "dinclude:clib/asl_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/asl_protos.h" 3

# 5 "dinclude:clib/asl_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/asl_protos.h" 3

# 6 "dinclude:clib/asl_protos.h" 3

# 1 "dinclude:amiga20/clib/asl_protos.h" 4












 
 








# 1 "dinclude:amiga20/libraries/asl.h" 5













 























# 1 "dinclude:amiga20/workbench/startup.h" 6












 











# 1 "dinclude:amiga20/libraries/dos.h" 7












 







# 25 "dinclude:amiga20/workbench/startup.h" 6

# 25 "dinclude:amiga20/workbench/startup.h" 6


struct WBStartup {
    struct Message      sm_Message;      
    struct MsgPort *    sm_Process;      
    BPTR                sm_Segment;      
    LONG                sm_NumArgs;      
    char *              sm_ToolWindow;   
    struct WBArg *      sm_ArgList;      
};

struct WBArg {
    BPTR                wa_Lock;         
    BYTE *              wa_Name;         
};



# 38 "dinclude:amiga20/libraries/asl.h" 5

# 38 "dinclude:amiga20/libraries/asl.h" 5










 















 
struct FileRequester    {
        APTR    rf_Reserved1;
        BYTE    *rf_File;        
        BYTE    *rf_Dir;         
        CPTR    rf_Reserved2;
        UBYTE   rf_Reserved3;
        UBYTE   rf_Reserved4;
        APTR    rf_Reserved5;
        WORD    rf_LeftEdge,rf_TopEdge;  
        WORD    rf_Width,rf_Height;    
        WORD    rf_Reserved6;
        LONG    rf_NumArgs;      
        struct WBArg *rf_ArgList;
        APTR    rf_UserData;     
        APTR    rf_Reserved7;
        APTR    rf_Reserved8;
        BYTE    *rf_Pat;         
        };                       






















 


                            

                            
                            
                            



                            














 














 
struct FontRequester    {
        APTR    fo_Reserved1[2];
        struct TextAttr fo_Attr;         
        UBYTE   fo_FrontPen;             
        UBYTE   fo_BackPen;
        UBYTE   fo_DrawMode;
        APTR    fo_UserData;
        };

 
 
 








                                 
                                 
                                 

                                 











 
 
 
 



 
 
 











 



 


















 



# 23 "dinclude:amiga20/clib/asl_protos.h" 4

# 23 "dinclude:amiga20/clib/asl_protos.h" 4

 
 
struct FileRequester *AllocFileRequest( void );
void FreeFileRequest( struct FileRequester *fileReq );
BOOL RequestFile( struct FileRequester *fileReq );
APTR AllocAslRequest( unsigned long type, struct TagItem *tagList );
APTR AllocAslRequestTags( unsigned long type, Tag Tag1, ... );
void FreeAslRequest( APTR request );
BOOL AslRequest( APTR request, struct TagItem *tagList );
BOOL AslRequestTags( APTR request, Tag Tag1, ... );


# 7 "dinclude:clib/asl_protos.h" 3

# 7 "dinclude:clib/asl_protos.h" 3








# 48 "local/defs.h" 2

# 48 "local/defs.h" 2

# 1 "dinclude:clib/alib_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/alib_protos.h" 3

# 5 "dinclude:clib/alib_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/alib_protos.h" 3

# 6 "dinclude:clib/alib_protos.h" 3

# 1 "dinclude:amiga20/clib/alib_protos.h" 4












 








# 1 "dinclude:amiga20/libraries/mathffp.h" 5












 










































int      SPFix();                          
float    SPFlt();
int      SPCmp();
int      SPTst();
float    SPAbs();
float    SPFloor();
float    SPCeil();



float    SPNeg();
float    SPAdd();
float    SPSub();
float    SPMul();
float    SPDiv();

float    SPAsin(),  SPAcos(),  SPAtan();   
float    SPSin(),   SPCos(),   SPTan(),   SPSincos();
float    SPSinh(),  SPCosh(),  SPTanh();
float    SPExp(),   SPLog(),   SPLog10(), SPPow();
float    SPSqrt(),  SPFieee();

float    afp(),    dbf();                  



# 22 "dinclude:amiga20/clib/alib_protos.h" 4

# 22 "dinclude:amiga20/clib/alib_protos.h" 4



# 1 "dinclude:amiga20/libraries/commodities.h" 5













 







 










 

 




 







struct NewBroker {
   BYTE     nb_Version;   
   BYTE     *nb_Name;
   BYTE     *nb_Title;
   BYTE     *nb_Descr;
   SHORT    nb_Unique;
   SHORT    nb_Flags;
   BYTE     nb_Pri;
    
   struct MsgPort   *nb_Port;
   WORD     nb_ReservedChannel;   
};

 




 




 

 

typedef LONG   CxObj;
typedef LONG   CxMsg;


 
typedef LONG   (*PFL)();

 
 
 











 
 
 

 











 







 


 









                               

 




















 

 
 
 






 
 
 



struct InputXpression {
   UBYTE   ix_Version;      
   UBYTE   ix_Class;        

   UWORD   ix_Code;         

   UWORD   ix_CodeMask;     
                            
                                                    

   UWORD   ix_Qualifier;    

   UWORD   ix_QualMask;     
                            
                                                    

   UWORD   ix_QualSame;     
   };

typedef struct InputXpression IX;

 




 






 




# 25 "dinclude:amiga20/clib/alib_protos.h" 4

# 25 "dinclude:amiga20/clib/alib_protos.h" 4

 
void BeginIO( struct IORequest *io );
struct IORequest *CreateExtIO( struct MsgPort *msg, long size );
struct MsgPort *CreatePort( UBYTE *name, long pri );
struct IOStdReq *CreateStdIO( struct MsgPort *msg );
struct Task *CreateTask( UBYTE *name, long pri, APTR initPC,
        unsigned long stackSize );
void DeleteExtIO( struct IORequest *io );
void DeletePort( struct MsgPort *io );
void DeleteStdIO( struct IOStdReq *io );
void DeleteTask( struct Task *task );
void NewList( struct List *list );
LONG NameFromAnchor( struct AnchorPath *anchor, UBYTE *buffer, long buflen );
 
void AddTOF( struct Isrvstr *i, long (*p)(), long a );
void RemTOF( struct Isrvstr *i );
void waitbeam( long b );
 
FLOAT afp( BYTE *string );
void arnd( long place, long SPExp, BYTE *string );
FLOAT dbf( unsigned long SPExp, unsigned long mant );
LONG fpa( FLOAT fnum, BYTE *string );
void fpbcd( FLOAT fnum, BYTE *string );
 
LONG TimeDelay( long unit, unsigned long secs, unsigned long microsecs );
LONG DoTimer( struct timeval *, long unit, long command );
 
 
 
void ArgArrayDone( void );
UBYTE **ArgArrayInit( long arg1, UBYTE **arg2 );
LONG ArgInt( UBYTE **arg1, UBYTE *arg2, long arg3 );
UBYTE *ArgString( UBYTE **arg1, UBYTE *arg2, UBYTE *arg3 );
CxObj *HotKey( UBYTE *arg1, struct MsgPort *arg2, long arg3 );
struct InputEvent *InvertString( UBYTE *arg1, ULONG *arg2 );
 
 
 
 
 
 
 
 
 
 


# 7 "dinclude:clib/alib_protos.h" 3

# 7 "dinclude:clib/alib_protos.h" 3


# 49 "local/defs.h" 2

# 49 "local/defs.h" 2

# 1 "dinclude:clib/icon_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/icon_protos.h" 3

# 5 "dinclude:clib/icon_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/icon_protos.h" 3

# 6 "dinclude:clib/icon_protos.h" 3

# 1 "dinclude:amiga20/clib/icon_protos.h" 4












 
 
 






 
LONG GetIcon( UBYTE *name, struct DiskObject *icon,
        struct FreeList *freelist );
BOOL PutIcon( UBYTE *name, struct DiskObject *icon );
void FreeFreeList( struct FreeList *freelist );
BOOL AddFreeList( struct FreeList *freelist, APTR mem, unsigned long size );
struct DiskObject *GetDiskObject( UBYTE *name );
BOOL PutDiskObject( UBYTE *name, struct DiskObject *diskobj );
void FreeDiskObject( struct DiskObject *diskobj );
UBYTE *FindToolType( UBYTE **toolTypeArray, UBYTE *typeName );
BOOL MatchToolValue( UBYTE *typeString, UBYTE *value );
UBYTE *BumpRevision( UBYTE *newname, UBYTE *oldname );
struct DiskObject *GetDefDiskObject( long type );
BOOL PutDefDiskObject( struct DiskObject *diskObject );
struct DiskObject *GetDiskObjectNew( UBYTE *name );
BOOL DeleteDiskObject( UBYTE *name );


# 7 "dinclude:clib/icon_protos.h" 3

# 7 "dinclude:clib/icon_protos.h" 3








# 50 "local/defs.h" 2

# 50 "local/defs.h" 2

# 1 "dinclude:clib/wb_protos.h" 3
 



# 1 "dinclude:amiga20/pragmas/config.h" 4
 







# 5 "dinclude:clib/wb_protos.h" 3

# 5 "dinclude:clib/wb_protos.h" 3

# 1 "dinclude:amiga20/exec/types.h" 4












 

















 
 
 


 

 
 
 
 
 
 

 

 

 
 
 
 


 
 
 





 



















 
 





# 6 "dinclude:clib/wb_protos.h" 3

# 6 "dinclude:clib/wb_protos.h" 3

# 1 "dinclude:amiga20/clib/wb_protos.h" 4












 
 
 












 
 
 
 
 
struct AppWindow *AddAppWindowA( unsigned long id, unsigned long userdata,
        struct Window *window, struct MsgPort *msgport,
        struct TagItem *taglist );
struct AppWindow *AddAppWindow( unsigned long id, unsigned long userdata,
        struct Window *window, struct MsgPort *msgport, Tag tag1, ... );
 
BOOL RemoveAppWindow( struct AppWindow *appWindow );
 
struct AppIcon *AddAppIconA( unsigned long id, unsigned long userdata,
        UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
        struct DiskObject *diskobj, struct TagItem *taglist );
struct AppIcon *AddAppIcon( unsigned long id, unsigned long userdata,
        UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
        struct DiskObject *diskobj, Tag tag1, ... );
 
BOOL RemoveAppIcon( struct AppIcon *appIcon );
 
struct AppMenuItem *AddAppMenuItemA( unsigned long id, unsigned long userdata,
        UBYTE *text, struct MsgPort *msgport, struct TagItem *taglist );
struct AppMenuItem *AddAppMenuItem( unsigned long id, unsigned long userdata,
        UBYTE *text, struct MsgPort *msgport, Tag tag1, ... );
 
BOOL RemoveAppMenuItem( struct AppMenuItem *appMenuItem );
 
 


# 7 "dinclude:clib/wb_protos.h" 3

# 7 "dinclude:clib/wb_protos.h" 3








# 51 "local/defs.h" 2

# 51 "local/defs.h" 2






typedef struct TagItem          TAGS;


typedef BPTR                    LOCK;
typedef struct FileInfoBlock    FIB;
typedef struct Process          APROCESS;
typedef struct CommandLineInterface CLI;


typedef struct ExecBase         EBASE;
typedef struct Node             NODE;
typedef struct List             LIST;
typedef struct Library          LIBRARY;
typedef struct Message          MSG;
typedef struct MsgPort          MPORT;
typedef struct Task             TASK;


typedef struct IntuitionBase    IBASE;
typedef struct VisualInfo       VINFO;
typedef struct Gadget           GADGET;
typedef struct NewGadget        NEWGAD;
typedef struct Screen           SCREEN;
typedef struct Window           WINDOW;
typedef struct IntuiMessage     IMSG;
typedef struct StringInfo       STRINGINFO;


typedef struct GfxBase          GBASE;
typedef struct View             VIEW;
typedef struct ViewPort         VPORT;
typedef struct RastPort         RPORT;
typedef struct BitMap           BITMAP;
typedef struct RasInfo          RINFO;
typedef struct TextAttr         TATTR;
typedef struct Rectangle        RECT;
typedef struct TextFont         FONT;


typedef struct IORequest        IOREQ;
typedef struct IOStdReq         STDREQ;
typedef struct IOExtSer         SERREQ;


typedef struct WBStartup        WBMSG;


typedef struct FontRequester    FONTREQ;
typedef struct FileRequester    FILEREQ;



extern __far struct Custom      custom;
extern GBASE                    *GfxBase;






# 8 "defs.h" 1

# 8 "defs.h" 1








typedef struct DLIST {
        NODE    node;
        char    name[2];
} DLIST;



typedef struct SYMLIST {
        ULONG *symbolname;
        ULONG address;
} SYMLIST;




enum DTYPES {
        DTYPE_DOSBASE,
        DTYPE_PROCESS,
        DTYPE_INFO,
        DTYPE_EXECBASE,
        DTYPE_RESOURCES,
        DTYPE_INTRS,
        DTYPE_PORTS,
        DTYPE_TASKS,
        DTYPE_LIBS,
        DTYPE_DEVICES,
        DTYPE_MEMLIST,
        DTYPE_REXXLIST,
        DTYPE_SYMLIST,
        MAX_DTYPE,
};

typedef struct DBugDisp {
        NODE    ds_Node;
        WINDOW  *ds_Win;                
        STDREQ  ds_CReadReq;            
        STDREQ  ds_CWriteReq;           

        UBYTE   ds_CReadIP;             
        UBYTE   ds_CWriteIP;            
        UBYTE   ds_OpenError;
        UBYTE   ds_CsiState;            

        UBYTE   ds_DoneFlag;
        UBYTE   ds_DisplayOffsets;
        UBYTE   ds_PromptLen;
        UBYTE   ds_PromptStart;

        ULONG   ds_Flags;


        UBYTE   ds_CInChar;             
        UBYTE   ds_COutBuf[255];
        WORD    ds_COutIndex;

        UWORD   ds_DisplayMode;         
        UWORD   ds_PreferedMode;        

        UWORD   ds_ScrRows;             
        UWORD   ds_ScrCols;

        UWORD   ds_ScrTop;
        UWORD   ds_ScrColNo;            

        UWORD   ds_ScrRowNo;

        UBYTE   ds_RegFlag;             
        UBYTE   ds_RegTouched;          

        UBYTE   *ds_ScrAry;

        
        
        

        ULONG   ds_WindowTop;           
        ULONG   ds_WindowTopLine;       
        ULONG   ds_WindowBot;           
        ULONG   ds_WindowBotLine;       

        ULONG   ds_LastRefreshMode;     
        ULONG   ds_LastRefreshTop;      

        LIST    ds_List;                
        char    ds_windowTitle[128];
} DBugDisp;






typedef struct SOURCE {
        ULONG   lineNumber;                     
        ULONG   address;                        
} SOURCE;



typedef struct DEBUG    {
        struct DEBUG    *link;                  
        struct SOURCE   *table;                 
        struct SOURCE   *tableEnd;              
        char            sourceName[128];        
        char            *source;                
        ULONG           addrBegin;              
        ULONG           addrEnd;
} DEBUG;





typedef struct HUNK {
        ULONG   type;                           
        ULONG   size;                           
        ULONG   *memptr;                        
        ULONG   *actual;                        
        ULONG   hSize;                          
        APTR    hunk;                           
        APTR    reloc32;                        
        APTR    symbols;                        
        DEBUG   *debug;                         
} HUNK;


typedef struct BP {
        UWORD   state;                          
        UWORD   count;                          
        UWORD   value;                          
        UWORD   *address;                       
} BP;









typedef struct WP {
        ULONG           type;                   
        char            expression[128];        
} WP;












typedef struct DPREFS   {
        UWORD   top,left,width,height;
        UWORD DefaultMode;
        UBYTE DefaultOffset;
        alias[96];
} DPREFS;








extern IBASE                    *IntuitionBase;
extern GBASE                    *GfxBase;
extern EBASE                    *SysBase;
extern struct DosLibrary        *DOSBase;
extern struct Library           *GadToolsBase;

extern char *RexxHostName;

extern char     *args;
extern ULONG    argSize;
extern TASK     *thisTask;
extern APROCESS *thisProcess;
extern CLI      *thisCli;

extern char     targetName[];
extern ULONG    *exeFile;
extern ULONG    exeSize;
extern ULONG    numHunks, firstHunk, lastHunk;
extern HUNK     *hunkArray;


extern USHORT oldrow, oldcol;

        



















extern UBYTE    *programStack;
extern ULONG    programStackSize;

extern ULONG    programState;
extern UWORD    programSR;
extern ULONG    programPC;
extern ULONG    programD0;
extern ULONG    programD1;
extern ULONG    programD2;
extern ULONG    programD3;
extern ULONG    programD4;
extern ULONG    programD5;
extern ULONG    programD6;
extern ULONG    programD7;
extern ULONG    programA0;
extern ULONG    programA1;
extern ULONG    programA2;
extern ULONG    programA3;
extern ULONG    programA4;
extern ULONG    programA5;
extern ULONG    programA6;
extern ULONG    programA7;

extern ULONG    lastState;
extern UWORD    lastSR;
extern ULONG    lastPC;
extern ULONG    lastD0;
extern ULONG    lastD1;
extern ULONG    lastD2;
extern ULONG    lastD3;
extern ULONG    lastD4;
extern ULONG    lastD5;
extern ULONG    lastD6;
extern ULONG    lastD7;
extern ULONG    lastA0;
extern ULONG    lastA1;
extern ULONG    lastA2;
extern ULONG    lastA3;
extern ULONG    lastA4;
extern ULONG    lastA5;
extern ULONG    lastA6;
extern ULONG    lastA7;

enum DISPLAY_MODES {
        DISPLAY_DISM,
        DISPLAY_SOURCE,
        DISPLAY_MIXED,
        DISPLAY_BYTES,
        DISPLAY_WORDS,
        DISPLAY_LONGS,
        DISPLAY_HUNKS,
        DISPLAY_SYMBOL,
        DISPLAY_HELP,
        DISPLAY_BREAK,
        DISPLAY_DOSBASE,
        DISPLAY_PROCESS,
        DISPLAY_INFO,
        DISPLAY_EXECBASE,
        DISPLAY_RESOURCES,
        DISPLAY_INTRS,
        DISPLAY_PORTS,
        DISPLAY_TASKS,
        DISPLAY_LIBS,
        DISPLAY_DEVICES,
        DISPLAY_MEMLIST,
        DISPLAY_REXXLIST,
        DISPLAY_SYMLIST,
        MAX_MODES,
};





extern DPREFS   dprefs;

extern char     commandLine[];
extern UWORD    commandCol, commandEnd;

extern WP       wpTable[32];            

extern BP       bpTable[32];            
extern BP       bpTemp;                         
extern WORD     topBP;                          

extern DBugDisp *CurDisplay;                    

extern unsigned long  ScrollStart, ScrollEnd;
extern int SymbolCount;



extern __stkargs UBYTE  *MallocPublic(ULONG size);
extern __stkargs UBYTE  *MallocFast(ULONG size);
extern __stkargs UBYTE  *MallocChip(ULONG size);
extern __stkargs UBYTE  *MallocAny(ULONG size, ULONG type);
extern __stkargs void   Free(APTR ptr);
extern __stkargs void   CleanMem(void);
extern __stkargs ULONG  Disassemble(ULONG src, ULONG addr, char *buf);
extern __stkargs void   EnterProgram(void);
extern __stkargs void   TargetExit(void);





# 2 "fails.c" 0

# 2 "fails.c" 0

# 1 "dbug_protos.h" 1

 


 


 

extern void     FreeDLIST(LIST *list);
extern BOOL     AddListNODE(LIST *list, UBYTE type, char *data);
extern BOOL     DownList(void);
extern BOOL     UpList(void);
extern BOOL     PageDownList(void);
extern BOOL     PageUpList(void);
extern WORD     RefreshList(WORD maxLines, BOOL fullRefresh, LONG top);
extern BOOL     SprintfDLIST(LIST *list, UBYTE type, char *fmt, ...);
extern void     ListDosBase(DBugDisp *dp);
extern void     ListProcess(DBugDisp *dp);
extern void     ListInfo(DBugDisp *dp);
extern void     ListExecBase(DBugDisp *dp);
extern void     BuildDLIST(LIST *dst, LIST *src, ULONG type);
extern void     ListResources(DBugDisp *dp);
extern void     ListIntrs(DBugDisp *dp);
extern void     ListPorts(DBugDisp *dp);
extern void     ListTasks(DBugDisp *dp);
extern void     ListLibs(DBugDisp *dp);
extern void     ListDevices(DBugDisp *dp);
extern void     ListMemList(DBugDisp *dp);
extern int      SizeDLIST(LIST *list);
extern BOOL     ListSymbols(DBugDisp *dp);

 

extern void             Newline(void);
extern void             PrintAddress(ULONG addr);
extern void             OffsetAddressBuf(ULONG addr, char *buf);
extern void             InitCommand(void);
extern void             InitModes(void);
extern void             RefreshCommand(int fullRefresh);
extern void             RefreshPrompt(BOOL fullRefresh);
extern void             RefreshWindow(int fullRefresh);
extern void             RefreshAllWindows(int fullRefresh);
extern void             SetDisplayMode(WORD, BOOL);
extern void             ReadPrefs(void);
extern void             WritePrefs(void);
extern void             abort(void);
extern BOOL             ParseArgToken(char *buf);
extern ULONG    OnOffToggle(char *arg, ULONG val, ULONG mask);
extern int              main(int ac, char *av[]);
extern LONG             CalcDisplayLines(void);
extern void             SetModeSave(WORD mode);
extern ULONG            ValidMemCheck(ULONG address);
extern __stkargs void kprintf(unsigned char *arg, ...);
extern UBYTE    LineBuf[128];
extern UBYTE    DirBuf[128];
extern WORD             ForceFullRefresh;
extern UBYTE    RexxReplyString[256];
extern UBYTE    DefaultPubName[128];
extern char             lastCommand[128];
extern int              RStepFlag;
extern char             rexxhostname[16];

 

extern long             ParseExp(char *, short *, long);
extern void             PushOp(short, short, short);
extern int              TopOfOpStack(void);
extern int              SecondOffOpStack(void);
extern void             PushAtom(long, short);

 

extern void             RefreshFKeys(BOOL fullRefresh);
extern BOOL             FunctionKey(UWORD num);
extern BOOL             HelpKey(void);
extern BOOL             CmdFKey(char *args);
extern BOOL             CmdRexx(char *args);
extern BOOL             CmdSavePrefs(char *args);
extern BOOL             CmdInfo(char *args);
extern BOOL             CmdExecBase(char *args);
extern BOOL             CmdResources(char *args);
extern BOOL             CmdIntrs(char *args);
extern BOOL             CmdPorts(char *args);
extern BOOL             CmdSymList(char *args);
extern BOOL             CmdTasks(char *args);
extern BOOL             CmdLibs(char *args);
extern BOOL             CmdDevices(char *args);
extern BOOL             CmdMemList(char *args);
extern BOOL             CmdDoBsBase(char *args);
extern BOOL             CmdProcess(char *args);
extern BOOL             CmdOffsets(char *args);
extern BOOL             CmdDown(char *args);
extern BOOL             CmdUp(char *args);
extern BOOL             CmdPageDown(char *args);
extern BOOL             CmdPageUp(char *args);
extern BOOL             CmdSource(char *args);
extern BOOL             CmdDism(char *args);
extern BOOL             CmdMixed(char *args);
extern BOOL             CmdBytes(char *args);
extern BOOL             CmdWords(char *args);
extern BOOL             CmdLongs(char *args);
extern BOOL             CmdBreakpoint(char *args);
extern BOOL             CmdBP(char *args);
extern BOOL             CmdClear(char *args);
extern BOOL             CmdHunks(char *args);
extern BOOL             CmdSymbol(char *args);
extern BOOL             CmdSet(char *args);
extern BOOL             RStepTarget(void);
extern BOOL             StepTarget(void);
extern BOOL             StepTargetRange(ULONG addressLow, ULONG addressHi);
extern BOOL             RStepTargetRange(ULONG addressLow, ULONG addressHi);
extern BOOL             CmdStep(char *args);
extern BOOL             CmdRStep(char *args);
extern BOOL             OverTarget(void);
extern BOOL             OverTargetRange(ULONG addressLow, ULONG addressHi);
extern BOOL             CmdOver(char *args);
extern BOOL             CmdReset(char *args);
extern BOOL             GoTarget(void);
extern BOOL             CmdGo(char *args);
extern BOOL             CmdEval(char *args);
extern BOOL             CmdOpen(char *args);
extern BOOL             CmdClose(char *args);
extern BOOL             CmdQuit(char *args);
extern BOOL             CmdHelp(char *args) ;
extern BOOL             CmdWatchBytes(char *args);
extern BOOL             CmdWatchWords(char *args);
extern BOOL             CmdWatchLongs(char *args);
extern BOOL             CmdWatchClear(char *args);
extern BOOL             CmdRefresh(char *args);
extern BOOL             CmdDosBase(char *args);
extern WORD             RefreshHelp(WORD maxLines, BOOL fullRefresh, LONG);
extern BOOL             DownHelp(void);
extern BOOL             UpHelp(void);
extern int              HelpSize(void);
extern char             *SkipBlanks(char *s);
extern BOOL             DoCommand(char *command);
extern BOOL             CmdAlias(char *args);
extern BOOL             CmdUnAlias(char *args);
extern BOOL             CmdRegs(char *args);
extern BOOL             CmdJump(char *args);
extern BOOL             CmdBar(char *args);
extern BOOL             CmdEnd(char *args);
extern BOOL             CmdItem(char *args);
extern BOOL             CmdTitle(char *args);
extern BOOL             CmdMenus(char *args);
extern BOOL             CmdSubItem(char *args);
extern BOOL             CmdChangeWindow(char *args);
extern BOOL             CmdAgain(char *args);
extern void             init_function_keys(void);
extern char     fkeys[10][16];
extern void kprintline(char *string);
extern void kprintlen(char *string, int len);

 

extern WORD             RefreshHunks(WORD maxLines, BOOL fullRefresh, LONG index);
extern WORD             RefreshSymbols(WORD maxLines, BOOL fullRefresh, LONG index);
extern LONG             SymbolIndexOfAddr(ULONG val);
extern BOOL             DownHunks(void);
extern BOOL             UpHunks(void);
extern BOOL             DownSymbol(void);
extern BOOL             PageDownSymbol(void);
extern BOOL             UpSymbol(void);
extern BOOL             PageUpSymbol(void);
extern BOOL             DBugLoadSeg(char *filename);
extern void             AllocateStack(void);
extern void             ResetTarget(void);
extern DEBUG    *FindDebug(ULONG address);
extern DEBUG    *FindNearestDebug(ULONG address);
extern SOURCE   *FindSource(DEBUG *debug, ULONG address);
extern char             *FindSourceLine(DEBUG *debug, SOURCE *source);
extern __stkargs char *LookupValue(ULONG value);
extern ULONG    *NearestSymbol(ULONG value);
extern char             *NearestValue(ULONG value);
extern BOOL             LookupSymbol(char *symbol, ULONG *value);
extern BOOL             LookupSymLen(char *symbol, UWORD len, ULONG *value);
extern int              CountSymbols(void);
extern void             CopySymbols(SYMLIST *symlist);
extern char             *addscore(char *string);
extern char             *addat(char *string);

 

extern WORD             RefreshBreakpoints(WORD maxLines, BOOL fullRefresh);
extern BOOL             UpBreak(void);
extern BOOL             DownBreak(void);
extern void             InitBreakpoints(void);
extern void             InstallBreakpoints(void);
extern void             CheckBreakpoints(void);
extern BOOL             IsBreakpoint(ULONG address);
extern BOOL             SetBreakpoint(ULONG address, UWORD count, UWORD type);
extern BOOL             ClearBreakpoint(ULONG address);
extern void             SetTempBreakpoint(ULONG address);
extern void             SetAllBreakpoints(void);
extern WORD topBP;

 

extern WORD             RefreshBytes(WORD maxLines, BOOL fullRefresh, ULONG addr);
extern WORD             RefreshWords(WORD maxLines, BOOL fullRefresh, ULONG addr);
extern WORD             RefreshLongs(WORD maxLines, BOOL fullRefresh, ULONG addr);
extern BOOL             UpBytes(void);
extern BOOL             UpWords(void);
extern BOOL             UpLongs(void);
extern BOOL             PageUpHex(void);
extern BOOL             DownBytes(void);
extern BOOL             DownWords(void);
extern BOOL             DownLongs(void);
extern BOOL             PageDownHex(void);

 

extern BOOL             DownMixed(void);
extern BOOL             UpMixed(void);
extern BOOL             PageDownMixed(void);
extern BOOL             PageUpMixed(void);
extern BOOL             DownSource(void);
extern BOOL             UpSource(void);
extern BOOL             PageDownSource(void);
extern BOOL             PageUpSource(void);
extern WORD             RefreshMixed(WORD maxLines, BOOL fullRefresh, ULONG, ULONG, WORD);
extern WORD             PrevMixedLine(ULONG *addr, ULONG *line, LONG *info);
extern WORD             NextMixedLine(ULONG *addr, ULONG *line, LONG *info);
extern WORD             CurrentMixedLine(ULONG *addr, ULONG *line, LONG *info);

 

extern BOOL             DownDism(void);
extern BOOL             UpDism(void);
extern BOOL             PageDownDism(void);
extern BOOL             PageUpDism(void);
extern WORD             RefreshDism(short maxLines, BOOL fullRefresh);
extern ULONG    PreviousInstruction(ULONG addr);

 

extern char             *StateText(ULONG staten);
extern WORD             RefreshRegisters(WORD maxLines, BOOL fullRefresh);
extern long             *RegisterAddress(char *name);

 

extern WORD             RefreshWatchpoints(WORD maxLines, BOOL fullRefresh);
extern void             InitWatchpoints(void);
extern BOOL             IsWatchpoint(char *expression);
extern BOOL             SetWatchpoint(char *expression, ULONG type);
extern BOOL             ClearWatchpoint(char *expression);
extern BOOL             ShowWatchTitle(BOOL fullRefresh, WORD *count, WORD *maxLines);

 

extern BOOL     enable_menus(void);
extern void     init_default_menus(void);
extern void     free_menus(void);
extern void     set_menu_item(int num, int type, unsigned char *str, unsigned char *cmd, unsigned char *comkey);
extern void     do_scroller(void);
extern UWORD FindScrollerTop(UWORD total, UWORD displayable, UWORD pot);
extern int      FindScrollerValues(UWORD total, UWORD displayable, UWORD top, WORD overlap, UWORD *body, UWORD *pot);
extern void     setscrollbar(int flag);
extern void     ActivateArrows(struct Window *win);
extern void     InActivateArrows(struct Window *win);
extern BOOL     ProcessMenuItem(char *args,int type);
extern int      FindSlot(char *args);
extern struct Menu *v_create_menus(struct NewMenu *menulist);
extern int v_layout_menus(struct Menu *menus);
extern void v_free_menus(struct Menu *menus);
extern int text_width(char *str);

 

extern BOOL             RXGetPC(char *args);
extern BOOL             RXGetLine(char *args);
extern BOOL             RXGetInfo(char *args);
extern BOOL             RXGetDismLine(char *args);
extern BOOL             RXGetEval(char *args);
extern BOOL             RXGetBytes(char *args);
extern BOOL             RXGetWords(char *args);
extern BOOL             RXGetLongs(char *args);
extern BOOL             RXGetCommand(char *args);
extern BOOL             RXStartList(char *args);
extern BOOL             RXEndList(char *args);
extern BOOL             RXPutList(char *args);

 

extern void             InitConsole(void);
extern LONG             ScrMainBodyRange(void *ptop, void *pbot);
extern LONG             ScrColumns(WORD overhead, WORD itemWidth, WORD limit);
extern void             ScrFlush(void);
extern void             ScrPutNewline(void);
extern void             ScrPutChar(UBYTE c);
extern void             ScrWrite(char *s, ULONG len);
extern void             ScrPuts(char *s);
extern void             ScrPutsCtl(char *s);
extern WORD             ScrPrintf(const char *fmt, ...);
extern void             ScrStatus(const char *fmt, ...);
extern void             ScrHome(void);
extern void             ScrCursoff(void);
extern void             ScrCurson(void);
extern void             ScrClr(void);
extern void             ScrRowCol(UWORD row, UWORD col);
extern void             ScrEOL(void);
extern void             ScrHighlight(void);
extern void             ScrDull(void);
extern void             ScrPlain(void);
extern void             ScrInverse(void);
extern void             ScrItalics(void);
extern void             ScrUnderline(void);
extern void             ScrScrollup(void);
extern void             ScrScrolldown(void);
extern DBugDisp         *ScrOpen(BOOL first, BOOL refresh, char *pubname);
extern void             ScrScrollClr(void);
extern void             GetWindowSize(void);
extern void             EnterDebugger(void);
extern void             CloseDisplay(struct DBugDisp *);
extern void             RequestCloseDisplay(struct DBugDisp *, int );
extern BOOL             CheckCloseDisplay(void);
extern void             ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y);
extern void             SetTitle(char *title, DBugDisp *disp);
extern void             draw_fkey_boxes(void);
extern void             drawdoublebox(void);
extern struct DBugDisp  *CurDisplay;
extern LIST     DisplayList;

# 3 "fails.c" 0

# 3 "fails.c" 0

# 1 "dinclude:ctype.h" 1





 










extern  const unsigned char __CArya[257];
extern  const unsigned char __CAryb[257];
extern  const unsigned char __CUToL[257];
extern  const unsigned char __CLToU[257];
















extern int isalnum(int);
extern int isalpha(int);
extern int iscntrl(int);
extern int isdigit(int);
extern int isgraph(int);
extern int islower(int);
extern int isspace(int);
extern int isupper(int);

extern int ispunct(int);
extern int isxdigit(int);
extern int isprint(int);

extern int tolower(int);
extern int toupper(int);




















# 4 "fails.c" 0

# 4 "fails.c" 0

# 1 "dd_rev.h" 1







# 5 "fails.c" 0

# 5 "fails.c" 0




extern void             InitConsole(void);
extern LONG             ScrMainBodyRange(void *ptop, void *pbot);
extern LONG             ScrColumns(WORD overhead, WORD itemWidth, WORD limit);
extern void             ScrFlush(void);
extern void             ScrPutNewline(void);
extern void             ScrPutChar(UBYTE c);
extern void             ScrWrite(char *s, ULONG len);
extern void             ScrPuts(char *s);
extern void             ScrPutsCtl(char *s);
static unsigned int     _swrite(char *buf, size_t n1, size_t n2, const char **sst);
extern WORD             ScrPrintf(const char *fmt, ...);
extern void             ScrStatus(const char *fmt, ...);
static void             QueueRead(void);
static void             HandleConsoleInput(struct DBugDisp *disp, WORD c);
extern void             ScrHome(void);
extern void             ScrCursoff(void);
extern void             ScrCurson(void);
extern void             ScrClr(void);
extern void             ScrRowCol(UWORD row, UWORD col);
extern void             ScrEOL(void);
extern void             ScrHighlight(void);
extern void             ScrDull(void);
extern void             ScrPlain(void);
extern void             ScrInverse(void);
extern void             ScrItalics(void);
extern void             ScrUnderline(void);
extern void             ScrScrollup(void);
extern void             ScrScrolldown(void);
__autoexit static void  ScrClose(void);
extern DBugDisp         *ScrOpen(BOOL first, BOOL refresh, char *pubname);
extern void             ScrScrollClr(void);
extern void             GetWindowSize(void);
extern void             EnterDebugger(void);

extern void             CloseDisplay(struct DBugDisp *);
extern void             RequestCloseDisplay(struct DBugDisp *, int );
extern BOOL             CheckCloseDisplay(void);
extern void             ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y);
extern void             SetTitle(char *title, DBugDisp *disp);

extern void             draw_fkey_boxes(void);
extern void             drawdoublebox(void);
static    void          drawbox(int xcol,int yrow, int xlen, int ylen);
static     void         do_putc(DBugDisp *disp, UBYTE c);



extern struct DBugDisp  *CurDisplay;
extern LIST     DisplayList;







extern struct Menu      *DebugMenu;

extern struct Gadget ColorGadget;
extern struct Gadget DownGadget;
extern struct Gadget UpGadget;
extern struct Image ColorPropImage;
extern struct PropInfo ColorPropInfo;

DBugDisp    *CurDisplay;    
LIST        DisplayList;    
MPORT       DBugPort;       
WORD        CheckClose;
char        ValidClipChar[256];
static int  count = 0;



static int  PageFlag = 0;
static int  Ticked = 0;

USHORT  oldrow = 0xFFFF;         
USHORT  oldcol = 0xFFFF;

static ULONG seconds=0;
static ULONG micros=0;



void    InitConsole(void) {

        WORD i;


        NewList(&DisplayList);

        DBugPort.mp_Flags  = 0;
        DBugPort.mp_SigBit = 4;
        DBugPort.mp_SigTask = FindTask(0L);
        NewList(&DBugPort.mp_MsgList);

        SetSignal(0L,(1L<<4));

        for (i = '0'; i <= '9'; ++i)ValidClipChar[i] = 1;
        for (i = 'a'; i <= 'z'; ++i)ValidClipChar[i] = 1;
        for (i = 'A'; i <= 'Z'; ++i)ValidClipChar[i] = 1;
        ValidClipChar['+'] = 1;
        ValidClipChar['_'] = 1;
        ValidClipChar['@'] = 1;
        ValidClipChar['$'] = 1;
        ValidClipChar['.'] = 1;

        ValidClipChar['-'] = 1;
}





void    ScrFlush(void) {
        DBugDisp *disp;

        if ((disp = CurDisplay) && disp->ds_COutIndex) {
                disp->ds_CWriteReq.io_Command = 3;
                disp->ds_CWriteReq.io_Data = (APTR)disp->ds_COutBuf;
                disp->ds_CWriteReq.io_Length = disp->ds_COutIndex;
                DoIO((IOREQ *)&disp->ds_CWriteReq);
                disp->ds_COutIndex = 0;
        }
}

void    ScrPutChar(UBYTE c) {
        DBugDisp *disp;

        if (disp = CurDisplay) {
                disp->ds_COutBuf[disp->ds_COutIndex++] = c;
                if (disp->ds_COutIndex == sizeof(disp->ds_COutBuf))ScrFlush();
                if (c == '\t') {
                    int i = 8 - (disp->ds_ScrColNo & 7);
                    while(i--)do_putc(disp,' ');
                }
                else do_putc(disp,c);
        }
}

void do_putc(DBugDisp *disp,UBYTE c)
{
    if (disp->ds_ScrColNo < disp->ds_ScrCols && disp->ds_ScrRowNo < disp->ds_ScrRows && disp->ds_ScrAry)
        disp->ds_ScrAry[disp->ds_ScrColNo++ + disp->ds_ScrCols * disp->ds_ScrRowNo] = c;
}


void    ScrPutCharCtl(UBYTE c) {
        DBugDisp *disp;

        if (disp = CurDisplay) {
                disp->ds_COutBuf[disp->ds_COutIndex++] = c;
                if (disp->ds_COutIndex == sizeof(disp->ds_COutBuf))
                        ScrFlush();
        }
}


void    ScrWrite(char *s, ULONG len) {

        while (len > 0) {
                ScrPutChar(*s++);
                --len;
        }
}

void    ScrPuts(char *s) {

        while (*s)ScrPutChar(*s++);
}

void    ScrPutsCtl(char *s) {

        while (*s)ScrPutCharCtl(*s++);

}



static unsigned int _swrite(char *buf, size_t n1, size_t n2, const char **sst) {
        size_t n;

        if (n1 == 1)
                n = n2;
        else if (n2 == 1)
                n = n1;
            else
                n = n1 * n2;

        _slow_bcopy(buf, *sst, n);
        *sst += n;
        return(n2);
}

WORD    ScrPrintf(const char *fmt, ...) {
        char            buf[128];
        char            *ptr = &buf[0];
        va_list         va;
        WORD            n;

        (va = (void*)((char *)&fmt + sizeof(fmt)));
        n = (WORD)_pfmt(fmt, va, _swrite, &ptr);
        *ptr = 0;
        ScrPuts(buf);

        ;
        n = strlen(buf);
        return n;
}

void    ScrStatus(const char *fmt, ...) {
        static char     buf[128];
        char            *ptr = &buf[0];
        va_list         va;

        ScrPlain();
        ScrDull();

        (va = (void*)((char *)&fmt + sizeof(fmt)));
        _pfmt(fmt, va, _swrite, &ptr);
        *ptr = 0;

        ScrRowCol(CurDisplay->ds_ScrRows-2, 1);

        ScrPuts(" < Status: ");

        ScrInverse();
        ScrPuts(buf);
        ScrPlain();
        ScrDull();

        ScrPuts(" > ");


        CurDisplay->ds_PromptStart = strlen(buf)+14;
        RefreshPrompt(1);
        strcpy(RexxReplyString,buf);    
        ;
}



static void     QueueRead(void) {
        DBugDisp *disp;

        if (disp = CurDisplay) {
                if (disp->ds_CReadIP == 0) {
                        disp->ds_CReadReq.io_Command = 2;
                        disp->ds_CReadReq.io_Data = (APTR)&disp->ds_CInChar;
                        disp->ds_CReadReq.io_Length = 1;
                        SendIO((IOREQ *)&disp->ds_CReadReq);
                        disp->ds_CReadIP = 1;
                }
        }
}




void    ScrPutNewline(void) {
        DBugDisp *disp;

        ScrPutCharCtl('\n');

        if (disp = CurDisplay) {
                disp->ds_ScrColNo = 0;
                ++disp->ds_ScrRowNo;
        }
}

void    ScrHome(void) {
        DBugDisp *disp;

        ScrPutsCtl("\x9b0;0\x48");
        if (disp = CurDisplay) {
                disp->ds_ScrColNo = 0;
                disp->ds_ScrRowNo = 0;
        }
}

void    ScrCursoff(void) {
        ScrPutsCtl("\x9b0 \x70");
}

void    ScrCurson(void) {
        ScrPutsCtl("\x9b \x70");
        ScrFlush();
}

void    ScrClr(void) {
        DBugDisp *disp;

        ScrPutCharCtl(12);
        if (disp = CurDisplay) {
                disp->ds_ScrColNo = 0;
                disp->ds_ScrRowNo = 0;
                if (disp->ds_ScrAry)
                        clrmem(disp->ds_ScrAry, disp->ds_ScrRows * disp->ds_ScrCols);
        }
}

void    ScrRowCol(UWORD row, UWORD col) {
        DBugDisp *disp;
        char buf[32];

        sprintf(buf, "\x9b%d;%d\x48", row, col);
        ScrPutsCtl(buf);

        if (disp = CurDisplay) {
                disp->ds_ScrColNo = col - 1;
                disp->ds_ScrRowNo = row - 1;
        }
}


void    ScrEOL(void) {
        DBugDisp *disp;

        ScrPutsCtl("\x9b\x4b");
        if ((disp = CurDisplay) && disp->ds_ScrAry) {
                if (disp->ds_ScrRowNo < disp->ds_ScrRows && disp->ds_ScrColNo < disp->ds_ScrCols)
                        clrmem(disp->ds_ScrAry + disp->ds_ScrRowNo * disp->ds_ScrCols + disp->ds_ScrColNo, disp->ds_ScrCols - disp->ds_ScrColNo);
        }
}

void    ScrHighlight(void) {
        if (IntuitionBase->ActiveScreen->BitMap.Depth == 1)
                ScrPutsCtl("\x9b7\x6d");
        else ScrPutsCtl("\x9b32\x6d");
}

void    ScrDull(void) {
        if (IntuitionBase->ActiveScreen->BitMap.Depth == 1)
                ScrPutsCtl("\x9b3\x6d");
        else    ScrPutsCtl("\x9b33\x6d");
}

void    ScrPlain(void) {
        ScrPutsCtl("\x9b0\x6d");
}

void    ScrInverse(void) {
        ScrPutsCtl("\x9b7\x6d");
}

void    ScrItalics(void) {
        ScrPutsCtl("\x9b3\x6d");
}

void    ScrUnderline(void) {
        ScrPutsCtl("\x9b4\x6d");
}




void    ScrScrollup(void) {
        DBugDisp *disp;
        WINDOW   *win;

        if ((disp = CurDisplay) && (win = disp->ds_Win)) {
                RPORT    *rp = win->RPort;
                UWORD   top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
                UWORD   bottom = top + ScrMainBodyRange(0L, 0L) * rp->TxHeight;
                ULONG   topln;
                ULONG   botln;

                ScrollRaster(rp, 0, rp->TxHeight, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
                ScrMainBodyRange(&topln, &botln);
                --topln;
                if (disp->ds_ScrAry && botln < disp->ds_ScrRows) {
                        
                        movmem(disp->ds_ScrAry + (UWORD)(topln + 1) * disp->ds_ScrCols,
                                disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols,
                                (UWORD)(botln - topln - 1) * disp->ds_ScrCols
                        );
                        clrmem(disp->ds_ScrAry + (UWORD)(botln - 1) * disp->ds_ScrCols, disp->ds_ScrCols);
                }
                ScrRowCol(botln, 1);
        }
}



void    ScrScrolldown(void) {
        DBugDisp *disp;
        WINDOW   *win;

        if ((disp = CurDisplay) && (win = disp->ds_Win)) {
                RPORT    *rp = win->RPort;
                UWORD   top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
                UWORD   bottom = top + ScrMainBodyRange(0L,0L) * rp->TxHeight;
                ULONG   topln;
                ULONG   botln;

                ScrollRaster(rp, 0, -rp->TxHeight, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
                ScrMainBodyRange(&topln, &botln);
                ScrRowCol(topln, 1);
                --topln;
                if (disp->ds_ScrAry && botln < disp->ds_ScrRows) {
                        movmem(disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols,
                                disp->ds_ScrAry + (UWORD)(topln + 1) * disp->ds_ScrCols,
                                (UWORD)(botln - topln - 1) * disp->ds_ScrCols
                        );
                        clrmem(disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols, disp->ds_ScrCols);
                }

        }
}



void    ScrScrollClr(void) {
        DBugDisp *disp;
        WINDOW   *win;

        if ((disp = CurDisplay) && (win = disp->ds_Win)) {
                RPORT    *rp = win->RPort;
                UWORD   top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
                UWORD   bottom = top + ScrMainBodyRange(0L, 0L) * rp->TxHeight;
                ULONG   topln;
                ULONG   botln;

                SetAPen(rp,0);
                RectFill(rp, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
                ScrMainBodyRange(&topln, &botln);
                if (disp->ds_ScrAry && (topln < botln) && (topln < disp->ds_ScrRows)) {
                        clrmem(disp->ds_ScrAry + (UWORD)(topln) * disp->ds_ScrCols,
                                (botln - topln) * disp->ds_ScrCols);
                }
                ScrRowCol(botln, 1);
        }
}




LONG    ScrMainBodyRange(void *ptop, void *pbot) {

        if (ptop) {
            
            *(long *)ptop = CurDisplay->ds_ScrTop + 1;
        }
        if (pbot)
            *(long *)pbot = CurDisplay->ds_ScrRows - 4;    
        return (CurDisplay->ds_ScrRows - CurDisplay->ds_ScrTop - 4);      
}

LONG    ScrColumns(WORD overhead, WORD itemWidth, WORD limit) {
        LONG cols = (CurDisplay->ds_ScrCols - overhead) / itemWidth;
        LONG n;

        if (cols < 0)
            cols = 0;
        if (cols > limit)                
            cols = limit;

        for (n = 1; n <= cols; n <<= 1)   
            ;
        return (n >> 1);
}



__autoexit static void  ScrClose(void) {
        DBugDisp *disp;

        while ((disp = (DBugDisp *)DisplayList.lh_Head) != (DBugDisp *)&DisplayList.lh_Tail) {
            if(!disp) {
                break;
            }
            CloseDisplay(disp);
        }
       free_menus();
}

BOOL    CheckCloseDisplay(void) {
        DBugDisp        *disp;
        BOOL            retval = 1;
        BOOL            flag = 1;

    CheckClose = 0;


    while(flag) {
        flag = 0;
        for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
            if (!disp->ds_DoneFlag) {
                
                retval = 0;
            }
            else {
                CloseDisplay(disp);
                flag = 1;
                break;  
            }
        }
    }
    return retval;
}


void    CloseDisplay(DBugDisp *disp) {

        if(!disp)return;
        Remove(&disp->ds_Node);

        if (disp->ds_CReadIP) {
                AbortIO((IOREQ *)&disp->ds_CReadReq);
                WaitIO((IOREQ *)&disp->ds_CReadReq);
                disp->ds_CReadIP = 0;
        }
        if (disp->ds_CWriteIP) {
                AbortIO((IOREQ *)&disp->ds_CWriteReq);
                WaitIO((IOREQ *)&disp->ds_CWriteReq);
                disp->ds_CWriteIP = 0;
        }
        if (disp->ds_CReadReq.io_Device) {
                CloseDevice((IOREQ *)&disp->ds_CReadReq);
                disp->ds_CReadReq.io_Device = 0L;
        }
        if (disp->ds_ScrAry) {
                Free(disp->ds_ScrAry);
                disp->ds_ScrAry = 0L;
        }
        if (disp->ds_Win) {
                
                
                

                RequestCloseDisplay(disp,1);
                ClearMenuStrip(disp->ds_Win);
                CloseWindow(disp->ds_Win);
                disp->ds_Win = 0L;
        }

        if(CurDisplay == disp) {
                CurDisplay = 0L;
        }

        FreeMem(disp, sizeof(DBugDisp));
}

char    version[] = "\0$VER: dd 1.18 (17.4.93)" " Copyright 1992, O.I.C., Redistribution and use under DICE-LICENSE\n";


static struct ExtNewWindow nw = {
        0,0,             
        640,200,         
        0,1,             
        0,               
        0x00000001|0x00000002|0x00000004|0x00000008|0x00000000|0x00001000,
        0L,              
        0L,              
        0L,              
        0L,              
        0L,              
        64,64,           
        -1,-1,           
        0x0001,  
        0L
};

DBugDisp *ScrOpen(BOOL first, BOOL refresh, char *pubname) {
        DBugDisp *disp;
        DBugDisp *old = CurDisplay;
        BOOL ok = 0;

        if (disp = AllocMem(sizeof(DBugDisp), (1L<<0)|(1L<<16))) {
                NewList((LIST *)&disp->ds_List);
                nw.LeftEdge = dprefs.left;
                nw.TopEdge = dprefs.top;
                nw.Width = dprefs.width;
                nw.Height = dprefs.height;

                if (old) {
                        disp->ds_DisplayOffsets = old->ds_DisplayOffsets;
                        disp->ds_DisplayMode = old->ds_DisplayMode;
                        disp->ds_PreferedMode = old->ds_PreferedMode;
                        disp->ds_WindowTop    = old->ds_WindowTop;
                        disp->ds_WindowTopLine= old->ds_WindowTopLine;
                        disp->ds_WindowBot    = old->ds_WindowBot;
                        disp->ds_WindowBotLine= old->ds_WindowBotLine;
                }
                else {
                        
                        disp->ds_DisplayMode = dprefs.DefaultMode;
                        disp->ds_PreferedMode = dprefs.DefaultMode;
                        disp->ds_DisplayOffsets = dprefs.DefaultOffset;
                        disp->ds_LastRefreshMode = DISPLAY_BYTES;
                }

                if(pubname && *pubname && (((struct Library *)SysBase)->lib_Version >= 36)) {
                    struct Screen *pub_screen;
                    struct TagItem wtags[3];

                        nw.Flags |= 0x00040000;
                        nw.Extension = wtags;
                        wtags[0].ti_Tag = (((1L<<31) + 99) + 0x15);
                        wtags[0].ti_Data = (ULONG)pubname;

                        wtags[1].ti_Tag = (((1L<<31) + 99) + 0x17);
                        wtags[1].ti_Data = 1;
                        wtags[2].ti_Tag = (0L);
                        wtags[2].ti_Data = 0;

                        if(pub_screen = LockPubScreen(pubname)) {
                            disp->ds_Win = OpenWindow((struct NewWindow *)&nw);
                            UnlockPubScreen(pubname,pub_screen);
                        }
                }
                else {
                    memset(DefaultPubName,0,128);       
                    nw.Flags &= ~0x00040000;    
                    disp->ds_Win = OpenWindow((struct NewWindow *)&nw);
                }

                if(disp->ds_Win) {
                        PageFlag = 0;
                        SetTitle(0L,disp);      
                        disp->ds_Win->UserPort = &DBugPort;

                        ModifyIDCMP(disp->ds_Win,0x00000002|0x00000008|0x00000200|0x00000004|0x00000100|0x00040000|0x00080000);

                        disp->ds_CReadReq.io_Message.mn_ReplyPort = &DBugPort;
                        disp->ds_CReadReq.io_Data = (APTR)disp->ds_Win;
                        disp->ds_CReadReq.io_Length = sizeof(WINDOW);
                        if (OpenDevice("console.device", 0, (IOREQ *)&disp->ds_CReadReq, 0) == 0) {
                                disp->ds_CWriteReq = disp->ds_CReadReq;

                                CurDisplay = disp;
                                QueueRead();
                                ScrPutsCtl("\x9b\x3f\x37\x6c");
                                ScrFlush();
                                GetWindowSize();
                                ok = 1;

                                setscrollbar(0);        
                                ModifyIDCMP(disp->ds_Win,0x00000002|0x00000008|0x00000200|0x00000004|0x00000100|0x00040000|0x00080000|0x00000010|0x00000040|0x00000020|0x00400000);

                                
                                init_default_menus();
                                enable_menus();

                                if (refresh)RefreshWindow(-1);

                                
                                
                                
                                

                                if (first == 0) CurDisplay = old;
                        }
                }
        }
        if (disp) {
            AddTail(&DisplayList, &disp->ds_Node);
        }
        if (ok == 0) {
            if (disp)CloseDisplay(disp);
            if (first) {
                exit(25);
            }
        }
    return disp;
}



void    GetWindowSize(void) {
        DBugDisp *disp;
        WINDOW   *win;

        if ((disp = CurDisplay) && (win = disp->ds_Win)) {
                UWORD rows = (win->Height - win->BorderTop - win->BorderBottom) / win->RPort->TxHeight;
                UWORD cols = (win->Width - win->BorderLeft - win->BorderRight) / win->RPort->TxWidth;
                if (rows != disp->ds_ScrRows || cols != disp->ds_ScrCols) {
                        disp->ds_ScrRows = rows;
                        disp->ds_ScrCols = cols;
                        if (disp->ds_ScrAry)
                                Free(disp->ds_ScrAry);
                        if (disp->ds_ScrAry = MallocPublic(rows * cols))
                                clrmem(disp->ds_ScrAry, rows * cols);
                }
        }
}
























__geta4 void    EnterDebugger(void) {
        struct  MenuItem *item;

    if (CurDisplay && CurDisplay->ds_DisplayMode > DISPLAY_MIXED)
        SetDisplayMode(CurDisplay->ds_PreferedMode, 0);

    RefreshWindow(-1);
    ScrStatus("Ready");
    RefreshCommand(1);
    ScrCurson();

    SetSignal(0L,(1L<<4));      
    while (DisplayList.lh_Head != (NODE *)&DisplayList.lh_Tail) {
        ULONG   mask;
        MSG     *msg;

        if (CheckClose && CheckCloseDisplay()) {
            break;
        }



        mask = Wait( (1<<DBugPort.mp_SigBit) | (1<<RexxSigBit) );
        if (mask & (1<<DBugPort.mp_SigBit)) {
            while (msg = GetMsg(&DBugPort)) {
                DBugDisp        *disp;

                
                

                for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
                    if (msg == (MSG *)&disp->ds_CReadReq) {
                        disp->ds_CReadIP = 0;
                        CurDisplay = disp;
                        HandleConsoleInput(disp, disp->ds_CInChar);
                        CurDisplay = disp;
                        QueueRead();
                        msg = 0L;
                        break;
                    }
                }
                if (msg == 0L)continue;

                
                

                CurDisplay = 0L;
                for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
                    if (((IMSG *)msg)->IDCMPWindow == disp->ds_Win) {
                        CurDisplay = disp;
                        break;
                    }
                }
                if (CurDisplay) {
                    switch (((IMSG*)msg)->Class) {
                        case 0x00000200:
                        RequestCloseDisplay(CurDisplay,0);
                        break;

                        case 0x00000002:
                            GetWindowSize();
                            {
                                WINDOW  *window = CurDisplay->ds_Win;
                                dprefs.left = window->LeftEdge; dprefs.top = window->TopEdge;
                                dprefs.width = window->Width; dprefs.height = window->Height;
                            }
                            ScrClr();
                            setscrollbar(1);    
                            RefreshWindow(-1);
                            ScrStatus("Window Resized");
                            RefreshCommand(1);
                            ScrFlush();
                            break;

                        case 0x00000008:







                            switch(((IMSG *)msg)->Code) {

                                USHORT newrow, newcol, nc, nr;
                                case (0x68 | 0x80):
                                break;
                                case (0x68):

                                    newcol = (((IMSG *)msg)->MouseX);
                                    nc = newcol >> 3;
                                    newrow = (((IMSG *)msg)->MouseY);
                                    nr = newrow >> 3;

                                    
                                    if((oldcol == nc) && (oldrow == nr)) {
                                         
                                         
                                        char *string="bp\r";
                                        while (*string) {
                                            HandleConsoleInput(disp,*string++);
                                        }
                                        oldcol = oldrow = 0xFFFF;
                                    }
                                    else {
                                        oldcol = nc;
                                        oldrow =  nr;

                                        
                                        

                                        ProcessDataAtCoord(disp, newcol, newrow);
                                    }
                                    


                                break;
                            }
                            break;

                        case 0x00000004:
                            BeginRefresh(disp->ds_Win);
                            EndRefresh(disp->ds_Win, 1);
                            break;

                        case 0x00080000:
                            InActivateArrows(disp->ds_Win);
                            break;

                        case 0x00040000: {



                            PageFlag = 0;
                            ActivateArrows(disp->ds_Win);
                            RefreshCommand(1);
                            break;
                        }

                        case 0x00400000:
                            if(PageFlag) {      
                                Ticked = 1;
                                ScrCursoff();
                                if( PageFlag == 1)DoCommand("UP");
                                else DoCommand("DOWN");
                                RefreshPrompt(1);
                                RefreshCommand(1);
                                ScrCurson();
                            }
                            break;

                        case 0x00000040: {
                                unsigned int id;
                                APTR address = ((IMSG *)msg)->IAddress;
                                if( (id = ((struct Gadget *)address)->GadgetID) != 0) {
                                    PageFlag = 0;
                                    if(Ticked) {
                                        Ticked = 0;
                                        break;
                                    }
                                    ScrCursoff();
                                    if( id == 1)DoCommand("UP");
                                    else DoCommand("DOWN");
                                    RefreshPrompt(1);
                                    RefreshCommand(1);
                                    ScrCurson();
                                }
                                break;
                        }

                        case 0x00000010:
                            if(++count&1) {
                                break;  
                            }           

                        case 0x00000020: {
                            ULONG current, last;
                                unsigned int id;
                                APTR address = ((IMSG *)msg)->IAddress;

                            if( (id = ((struct Gadget *)address)->GadgetID) != 0) {
                                PageFlag = (id == 1) ? 1 : 2;
                                Ticked = 0;
                                break;
                            }
                            switch (CurDisplay->ds_DisplayMode) {
                                case DISPLAY_DISM:
                                case DISPLAY_SOURCE:
                                case DISPLAY_MIXED:
                                case DISPLAY_BYTES:
                                case DISPLAY_WORDS:
                                case DISPLAY_LONGS:
                                    last = (ScrollEnd - ScrollStart) >> 5;
                                    break;

                                case DISPLAY_HELP:
                                    last = HelpSize();
                                    break;

                                case DISPLAY_SYMBOL:
                                    last = SymbolCount;
                                    break;

                                case DISPLAY_HUNKS:
                                    last = numHunks;
                                    break;

                                case DISPLAY_BREAK:
                                    last = 32+5;
                                    break;

                                    default:
                                    last = SizeDLIST(&CurDisplay->ds_List);
                            }
                            current = FindScrollerTop(last, CurDisplay->ds_ScrRows, ColorPropInfo.VertPot);
                            
                            switch(CurDisplay->ds_DisplayMode) {
                                case DISPLAY_DISM:
                                case DISPLAY_SOURCE:
                                case DISPLAY_MIXED:
                                case DISPLAY_BYTES:
                                case DISPLAY_WORDS:
                                case DISPLAY_LONGS:
                                    if((ScrollStart + (current << 5)) != CurDisplay->ds_WindowTop) {
                                        CurDisplay->ds_WindowTop = ScrollStart + (current << 5);
                                        CurDisplay->ds_WindowTopLine = 0;
                                        RefreshWindow(1);
                                    }
                                    break;
                                case DISPLAY_BREAK:
                                    if(topBP != current) {
                                        topBP = current;
                                        RefreshWindow(1);
                                    }
                                    break;
                                default:;
                                    if(CurDisplay->ds_WindowTop != current) {
                                        CurDisplay->ds_WindowTop = current;
                                        CurDisplay->ds_WindowTopLine = 0;
                                        RefreshWindow(1);
                                    }
                            }
                            break;
                        }


                        case 0x00000100: {
                            USHORT code = ((IMSG *)msg)->Code;
                            char *string;

                            if (!code || (code == 0xFFFF))break;
                                while (code && (code != 0xFFFF)) {  
                                    item = ItemAddress(DebugMenu,(LONG) code);
                                    string = (* ( (APTR *)(item+1) ) );
                                    while (*string) {
                                        HandleConsoleInput(disp,*string++);
                                    }
                                    HandleConsoleInput(disp,'\r');
                                    code = item->NextSelect;
                                }
                            break;
                        }

                        default:

                        break;
                    }
                }
                if(msg)ReplyMsg(msg);
            }
        }

        if (mask & (1<<RexxSigBit)) {
            ProcessRexxCommands(0L);
        }
    }
}

void    HandleConsoleInput(DBugDisp *disp, WORD c) {
        switch(c) {
        case -1:
                break;
        case 3:
                RequestCloseDisplay(disp,1);
                break;
        case 8:
                if (commandCol) {
                        UWORD   col;

                        if (commandEnd == commandCol) commandEnd--;
                        commandCol--;
                        for (col=commandCol; col<commandEnd; col++) {
                                commandLine[col] = commandLine[col+1];
                        }
                        ScrCursoff();
                        RefreshCommand(1);
                        ScrCurson();
                }
                break;
        case 13:
                commandLine[commandEnd] = '\0';
                ScrCursoff();
                commandEnd = 0;         
                DoCommand(commandLine);
                ScrCurson();
                InitCommand();          
                ScrCursoff();
                RefreshCommand(1);
                ScrCurson();
                break;
        case 18:
                ScrCursoff();
                RefreshWindow(0);
                ScrCurson();
        case 24:
                InitCommand();
                ScrCursoff();
                RefreshCommand(1);
                ScrCurson();
                break;
        case 0x9b:
                disp->ds_CsiState = 1;
                break;
        default:
                switch (disp->ds_CsiState) {
                case 0:
                        if (c >= ' ') {
                                commandLine[commandCol] = c;
                                if (commandCol == commandEnd) commandEnd++;
                                commandCol++;
                                ScrCursoff();
                                RefreshCommand(1);
                                ScrCurson();
                        }
                        break;
                case 1:
                        ScrCursoff();

                        switch (c) {
                        case ' ': disp->ds_CsiState = 3; break;
                        case '0': disp->ds_CsiState = 2; FunctionKey(0); break;
                        case '1': disp->ds_CsiState = 2; FunctionKey(1); break;
                        case '2': disp->ds_CsiState = 2; FunctionKey(2); break;
                        case '3': disp->ds_CsiState = 2; FunctionKey(3); break;
                        case '4': disp->ds_CsiState = 2; FunctionKey(4); break;
                        case '5': disp->ds_CsiState = 2; FunctionKey(5); break;
                        case '6': disp->ds_CsiState = 2; FunctionKey(6); break;
                        case '7': disp->ds_CsiState = 2; FunctionKey(7); break;
                        case '8': disp->ds_CsiState = 2; FunctionKey(8); break;
                        case '9': disp->ds_CsiState = 2; FunctionKey(9); break;
                        case '?': disp->ds_CsiState = 2; HelpKey(); break;
                        case 'A': DoCommand("up"); break;
                        case 'B': DoCommand("down"); break;
                        case 'C': DoCommand("right"); break;
                        case 'D': DoCommand("left"); break;
                        case 'T': DoCommand("pageup"); break;
                        case 'S': DoCommand("pagedown"); break;
                        }

                        if (disp->ds_CsiState == 1) disp->ds_CsiState = 0;
                        RefreshPrompt(1);
                        RefreshCommand(1);
                        ScrCurson();
                        break;
                case 2:
                        disp->ds_CsiState = 0;
                        break;  
                case 3:
                        ScrCursoff();
                        switch (c) {
                                case 'A': DoCommand("shift-left"); break;
                                case '@': DoCommand("shift-right"); break;
                        }
                        disp->ds_CsiState = 0;
                        ScrCurson();
                        break;
                }
        }
}












void    RequestCloseDisplay(DBugDisp *disp, int force) {
        struct DBugDisp *ndisp;
        int count = 0;
        struct IntuiText body = {0,1,0,0,5,0L,"Last DD window.  Really quit ?",0L};
        struct IntuiText ok = {0,1,0,0,0,0L,"OK",0L};
        struct IntuiText cancel = {0,1,0,0,0,0L,"CANCEL",0L};

        if (disp && disp->ds_DoneFlag == 0) {
            disp->ds_DoneFlag = 1;

            
            if(!force) {
                
                
                for (ndisp = (DBugDisp *)DisplayList.lh_Head; ndisp->ds_Node.ln_Succ; ndisp = (DBugDisp *)ndisp->ds_Node.ln_Succ) {
                    if (!ndisp->ds_DoneFlag)count++;
                }
                if(count == 0) {        
                    if(!AutoRequest(disp->ds_Win,&body,&ok,&cancel,0L,0L,300,80)) {
                        disp->ds_DoneFlag = 0;  
                        return;
                    }
                }
            }

                CheckClose = 1;
                if (disp->ds_Win) {
                        Forbid();
                        ModifyIDCMP(disp->ds_Win, 0x00001000);
                        disp->ds_Win->UserPort = 0L;
                        ModifyIDCMP(disp->ds_Win, 0);
                        Permit();
                }
        }
}

void    ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y) {
        WINDOW *win = disp->ds_Win;
        WORD row = (y - win->BorderTop) / win->RPort->TxHeight;
        WORD col = (x - win->BorderLeft) / win->RPort->TxWidth;
        UBYTE *ptr = &disp->ds_ScrAry[row * disp->ds_ScrCols + col];
        WORD len;
        WORD i;

        if (ValidClipChar[*ptr] == 0) {
                switch (*ptr) {
                case '*':
                case '-':
                case '/':
                        HandleConsoleInput(disp, *ptr);
                        break;
                case '<':
                        HandleConsoleInput(disp, 'x'&0x1F);
                        break;
                case '>':
                        HandleConsoleInput(disp, 13);
                        break;
                case ' ':
                case 0:
                        
                        
                        
                        

                        if (row == disp->ds_ScrRows - 3 && col > disp->ds_PromptLen + 2) {
                                HandleConsoleInput(disp, 13);
                        }
                        break;
                }
                return;
        }
        while (col >= 0 && ValidClipChar[*ptr]) {
                --col;
                --ptr;
        }
        ++col;
        ++ptr;
        for (len = 0; ValidClipChar[ptr[len]] && col + len < disp->ds_ScrCols; ++len)
                ;
        {
                char *jp;
                int nflag = 0;

                if(*ptr == '-')nflag = 1;
                if (ptr[nflag] == '$' || (len == 8 && (strtol(ptr, &jp, 16), (jp - ptr == 8)))) {
                        if(nflag) {
                            HandleConsoleInput(disp, '-');
                            ++ptr;
                            --len;
                        }

                        HandleConsoleInput(disp, '0');
                        HandleConsoleInput(disp, 'x');
                        ++ptr;
                        --len;
                }
        }
        if (len == 2 && ptr[0] == '+' && ptr[1] == '+')--len;

        

        if (len == 1 && ptr[0] >= '1' && ptr[0] <= '9' && ptr[1] == ':') {
                FunctionKey(ptr[0] - '1');
                return;
        }
        
        if(len == 2 && ptr[0] == '1' && ptr[1] == '0' && ptr[2] == ':') {
                FunctionKey(9);
                return;
        }

        for (i = 0; i < len; ++i)HandleConsoleInput(disp, ptr[i]);

        if((len == 2) && (ptr[2] == ':')) {     
                HandleConsoleInput(disp, ':');
        }

        for(i=0; i<= 9; i++) {
            if(!strncmp(ptr,fkeys[i],len)) {
                HandleConsoleInput(disp, '\r');
                return;
            }
        }
        HandleConsoleInput(disp, ' ');
}

char    *ModeNames[10] = {
        "DISM MODE",
        "SOURCE MODE",
        "MIXED MODE",
        "BYTES (HEX)",
        "WORDS (HEX)",
        "LONGS (HEX)",
        "HUNKS",
        "SYMBOLS",
        "ONLINE HELP",
        "BREAKPOINTS"
};




 

void SetTitle(char *title, DBugDisp *disp)
{
    int type;
    DBugDisp *ldisp = disp;

    if(!ldisp)ldisp=CurDisplay;
    type = ldisp->ds_DisplayMode;

    strcpy(ldisp->ds_windowTitle, " (" "dd 1.18" " PORT: ");
    strcat(ldisp->ds_windowTitle,rexxhostname);
    strcat(ldisp->ds_windowTitle,") ");

    if((type < 10) && !title) {
        strcat(ldisp->ds_windowTitle,ModeNames[type]);
    }
    else {
        strcat(ldisp->ds_windowTitle,title);
    }
    strcat(ldisp->ds_windowTitle," ");

    strcat(ldisp->ds_windowTitle, targetName);
    if(argSize > 1)strncat(ldisp->ds_windowTitle, args, argSize-1);
    SetWindowTitles(ldisp->ds_Win,ldisp->ds_windowTitle,0L);
}

void draw_fkey_boxes(void)
{
int i,xcol = 2,yrow = CurDisplay->ds_ScrRows, xlen;

    for(i=0; i < 10; i++) {
        xlen = strlen(fkeys[i]) + ((i == 9) ? 3 : 2);
        drawbox(xcol,yrow,xlen, -1);
        xcol += (xlen+1);
    }
}



void drawbox(int xcol,int yrow, int xcollen, int yrowlen)
{
        WINDOW   *win;
        RPORT    *rp;
        int x,y,xlen,ylen, xmax, ymax;

    if(win = CurDisplay->ds_Win) {
        rp = win->RPort;

        y = win->BorderTop + ((yrow)  * rp->TxHeight);
        x = xcol * rp->TxWidth - 4;
        xlen = xcollen * rp->TxWidth + 1;
        ylen = rp->TxHeight + 3;

        if(x > (xmax = CurDisplay->ds_ScrCols * rp->TxWidth - 1))return; 
        xlen = (x+xlen) <= xmax ? xlen : xmax-x;

        SetAPen(rp,2);  
        Move(rp,x,y);
        Draw(rp,x,y-ylen);
        Draw(rp,x+xlen,y-ylen);

        SetAPen(rp,1);  
        Draw(rp,x+xlen,y);
        if(y > (ymax = win->Height - 3))return; 
        Draw(rp,x,y);
    }
}



void drawdoublebox(void)
{
        WINDOW   *win;
        RPORT    *rp;
        int x, y, xlen, ylen, i, j;

    if(win = CurDisplay->ds_Win) {
        rp = win->RPort;
        x = win->BorderLeft; 
        y = win->BorderTop + (CurDisplay->ds_ScrRows-2) * rp->TxHeight + 2;

        xlen = win->Width - win->BorderRight - win->BorderLeft - 2;
        ylen = rp->TxHeight + 6;

        for(i=2, j=0; i>0; i--, j++) {
            SetAPen(rp,i);


            Move(rp,x+j,y-ylen+j);
            Draw(rp,x+xlen+j,y-ylen+j);

            Move(rp,x+xlen+j,y+j);
            Draw(rp,x+j,y+j);
        }
    }
}
