/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 */

/*
 * This file contains the TOPAZ80 default font for
 * global recognition...
 */

#include        <exec/types.h>
#include        <graphics/text.h>

#include        "DefaultFonts.h"

static char fontnam[11]="topaz.font";

struct TextAttr TOPAZ80={fontnam,8,0,FPF_ROMFONT};

struct TextAttr TOPAZ60={fontnam,9,0,FPF_ROMFONT};
