
/*
 *  DENSITY.C
 *
 *  David Berezowski - March/88.
 *  Modified for DICE - May/91	Matthew Dillon
 *
 *  Copyright (c) 1988  Commodore-Amiga, Inc.
 *  (c)Copyright 1991 Matthew Dillon
 */

#include "defs.h"

Prototype short SetDensity(ULONG);

short
SetDensity(density_code)
ULONG density_code;
{
    /* SPECIAL_DENSITY	   0   1   2	3    4	  5    6    7 */

    static int XDPI[8] = {90, 90, 120, 180, 360, 360, 360, 360};
    static char codes[8] = {38, 38, 33, 39, 40, 40, 40, 40};

    PED->ped_MaxColumns = PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
    density_code /= SPECIAL_DENSITY1;

    /* default is 80 chars (8.0 in.), W_TRACTOR is 136 chars (13.6 in.) */

    PED->ped_MaxXDots = (XDPI[density_code] * PED->ped_MaxColumns) / 10;
    PED->ped_XDotsInch = XDPI[density_code];
    return(codes[density_code]);
}

