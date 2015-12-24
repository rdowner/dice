
/* Workbench tag call */

#include <exec/types.h>
#include <clib/wb_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

struct AppWindow *
HYPER ## AddAppWindow( unsigned long id, unsigned long userdata,
	struct Window *window, struct MsgPort *msgport, Tag tag1, ... )
{
	return AddAppWindowA( id, userdata, window, msgport, (TagItem *)&tag1);
}
