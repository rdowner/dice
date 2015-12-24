/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 * $Id: DefaultFonts.c,v 30.8 1994/08/18 05:53:12 dice Exp dice $
 *
 * $Source: /home/dice/com/src/vcommon/RCS/DefaultFonts.c,v $
 *
 * $Date: 1994/08/18 05:53:12 $
 *
 * $Revision: 30.8 $
 *
 * $Log: DefaultFonts.c,v $
 * Revision 30.8  1994/08/18  05:53:12  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:09:38  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:09:38  dice
 * .
 *
 * Revision 1.1  1992/07/18  23:06:34  jtoebes
 * Initial revision
 *
 * Revision 1.1  92/04/21  06:21:31  J_Toebes
 * Initial revision
 * 
 * Revision 1.1  90/05/20  12:15:58  mks
 * Initial revision
 * 
 */

/*
 * This file contains the TOPAZ80 default font for
 * global recognition...
 */

#include	<exec/types.h>
#include	<graphics/text.h>

#include	"DefaultFonts.h"

static char fontnam[11]="topaz.font";

struct TextAttr TOPAZ80={fontnam,8,0,FPF_ROMFONT};

struct TextAttr TOPAZ60={fontnam,9,0,FPF_ROMFONT};
