
/*
 *  auto-aggregate test
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>


main(ac, av)
char *av[];
{
    struct Window *win;
    struct TagItem Ti[] = {
	WA_Left, 40,
	WA_Top, 20,
	WA_Width, atoi(av[1]),
	WA_Height, atoi(av[2]),
	WA_AutoAdjust, 1,
	TAG_END
    };


    if (win = OpenWindowTagList(NULL, Ti)) {
	Delay(50 * 5);
	CloseWindow(win);
    }
    return(0);
}

