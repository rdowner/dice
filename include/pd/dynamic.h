
/*
 *  DYNAMIC.H
 *
 *  dynamic.library prototypes
 */

#ifndef PD_DYNAMIC_H
#define PD_DYNAMIC_H

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef DYNAMIC_COMPILE

void *GetHyperSymbol(const char *, struct TagItem *);
void *RelsHyperSymbol(void *, struct TagItem *);
void FlushHyperLib(void);
void LockHyperLib(void);
void UnLockHyperLib(void);
void ScanHyperLib(void);
void SetHyperDebug(long,long);

void *GetHyperSymbolTags(const char *, Tag tag1, ...);
void *RelsHyperSymbolTags(void *, Tag tag1, ...);

#endif

#define DT_END	    0
#define DT_TYPE     1

#endif

