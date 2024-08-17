/* $VER: ../include/pragmas/battmem_pragmas.h 1.0 (9.8.97) */
#ifndef BattMemBase_PRAGMA_H
#define BattMemBase_PRAGMA_H

#pragma libcall BattMemBase ObtainBattSemaphore 6 00
#pragma libcall BattMemBase ReleaseBattSemaphore c 00
#pragma libcall BattMemBase ReadBattMem 12 10803
#pragma libcall BattMemBase WriteBattMem 18 10803

#endif
