
/*
 *  CEXEC.C
 *
 *  test exec capability
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <libraries/dos.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct MsgPort	MsgPort;
typedef struct Message	Message;

main(ac, av)
char *av[];
{
    char *resName;
    long resNode;
    MsgPort *port;
    Message *msg;
    char buf[256];
    int r;

    if (ac == 1) {
	puts("cexec program");
	exit(1);
    }
    port = CreatePort(NULL, 0);
    resNode = MakeResidentNode(port, &resName);

    printf("resNode %08lx name %s\n", resNode, resName);
    sprintf(buf, "%s %s", resName, av[1]);
    puts(buf);
    r = Execute(buf, NULL, NULL);
    printf("r = %d\n", r);
    if (r) {
	puts("waiting for message...");
	while ((msg = GetMsg(port)) == NULL) {
	    long m = Wait (SIGBREAKF_CTRL_C | (1 << port->mp_SigBit));
	    if (m & SIGBREAKF_CTRL_C) {
		puts("break-abort");
		break;
	    }
	}
	if (msg) {
	    printf("msg, return code: %d\n", msg->mn_Node.ln_Name);
	    FreeMem(msg, msg->mn_Length);
	}
    }
    puts("unloading in 2 secs");
    Delay(50*2);
    puts("unloading...");
    r = DeleteResidentNode(resNode);
    if (r < 0)
	puts("wasn't able to find node!");
    DeletePort(port);
}

