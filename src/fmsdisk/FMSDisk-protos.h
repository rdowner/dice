
/* MACHINE GENERATED */


/* tag.a                */


/* fms.c                */

Prototype DevCall NDev *DevInit(register __a0 APTR);
Prototype DevCall NDev *DevOpen(register __d0 long, register __a1 IOB *, register __d1 long);
Prototype APTR DevExpunge(void);
Prototype APTR DevClose(register __a1 IOB *);
Prototype APTR DevReserved(void);
Prototype void DevBeginIO(register __a1 IOB *iob);
Prototype void DevAbortIO(register __a1 IOB *iob);
Prototype void SynchroMsg(UWORD, NDUnit *);
Prototype __geta4 void CoProc(void);
Prototype void ExtendSize(NDUnit *, long);
Prototype void FlushCache(void);
Prototype void GetUnitName(int, char *);
