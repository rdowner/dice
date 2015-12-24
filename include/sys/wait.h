
/*
 * $VER: sys/wait.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef SYS_WAIT_H
#define SYS_WAIT_H

#ifndef SYS_RESOURCE_H
#include <sys/resource.h>
#endif

union wait {
    struct {
	char wu_termsig;
	char wu_stopval;
	char wu_flags;
	char wu_status;
	long wu_retcode;
	char wu_stopsig;
	char wu_reserved;
    } u;
};

#define WF_EXITED   0x0001	/*  DICE private    */

#define w_termsig u.wu_termsig
#define w_stopval u.wu_stopval
#define w_retcode u.wu_retcode
#define w_status  u.wu_status
#define w_stopsig u.wu_stopsig
#define w_coredump u.wu_termsig     /*	0   */

#define WNOHANG     0x0001
#define WUNTRACED   0x0002

#define WIFSTOPPED(reason)  0
#define WIFEXITED(reason)   ((reason).u.wu_flags & WF_EXITED)
#define WIFSIGNALED(reason) 0

int wait(union wait *);
int wait3(union wait *, int, struct rusage *);

#endif
