
/*
 *  DEFS.H
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <devices/prtgfx.h>

#include <clib/exec_protos.h>

#define Prototype extern

typedef struct PrtInfo  PrtInfo;
typedef struct PrinterData PrinterData;
typedef struct PrinterExtendedData PrinterExtendedData;

extern const PrinterExtendedData PEDData;

#include "Printer_Driver-protos.h"

