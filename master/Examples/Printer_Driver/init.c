
/*
 *  INIT.C
 */

#include "defs.h"

Prototype __geta4 long DevInit(PrinterData *);
Prototype __geta4 long DevExpunge(void);
Prototype __geta4 long DevOpen(void);
Prototype __geta4 long DevClose(void);

Prototype PrinterData *PD;
Prototype PrinterExtendedData *PED;

PrinterData *PD;
PrinterExtendedData *PED;

__geta4 long
DevInit(pd)
PrinterData *pd;
{
    PD = pd;
    PED= &PEDData;
    return(0);
}

__geta4 long
DevExpunge(void)
{
    return(0);
}

__geta4 long
DevOpen(void)
{
    return(0);
}

__geta4 long
DevClose(void)
{
    return(0);
}

