#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "multi_thread.h"
#include "eag6l.h"

int sysrdfifo;
int syswrfifo;

struct multi_thread_master *sysmon_intf_master;

static int sysmon_recv_fifo(struct multi_thread *thread)

{

        int len = 0;
	SYS_IPC_DATA req;

        len = read(MULTI_THREAD_FD (thread),&req, sizeof(SYS_IPC_DATA));

        if (len < 0)
        {
                multi_thread_add_read (sysmon_intf_master, sysmon_recv_fifo, NULL, MULTI_THREAD_FD (thread));
                return 0;
        }

	/**TODO: define request type, process**/
	

        multi_thread_add_read (sysmon_intf_master, sysmon_recv_fifo, NULL, MULTI_THREAD_FD (thread));
	return 0;
}

int sysmon_ipc_fifo_recv_init (void )
{
        umask(0000);
        (void) unlink(SYSMON_FIFO_READ);
        (void) unlink(SYSMON_FIFO_WRITE);

        if(mkfifo(SYSMON_FIFO_READ, 0666) != 0)
        {
                return 1;
        }
        if(mkfifo(SYSMON_FIFO_WRITE, 0666) != 0)
        {
                return 1;
        }

        if( (sysrdfifo = open (SYSMON_FIFO_READ,O_RDWR)) < 0)
                return 1;
        if( (syswrfifo = open (SYSMON_FIFO_WRITE,O_RDWR)) < 0)
                return 1;

        multi_thread_add_read (sysmon_intf_master, sysmon_recv_fifo, NULL, sysrdfifo);

        return 0;
}

void sysmon_intf_init (void) 
{
	sysmon_intf_master = multi_thread_make_master();
	sysmon_ipc_fifo_recv_init();
}



int _echo_to_terminal (struct multi_thread *thread)
{
	thread = thread;
        cmdOsPrintf("eag6l_intf_task run\n");
	multi_thread_add_timer (sysmon_intf_master, _echo_to_terminal, NULL, 10);
	return 0;
}

static unsigned __TASKCONV OAM_thread(void)
{
	struct multi_thread thread;
	sysmon_intf_init();

	/*add timer*/
	multi_thread_add_timer (sysmon_intf_master, _echo_to_terminal, NULL, 10);

	while (multi_thread_fetch (sysmon_intf_master, &thread))
		multi_thread_call (&thread);

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}



GT_STATUS eag6lOAMstart(void)
{
    GT_TASK   tid;

    if (cmdOsTaskCreate(
                "OAMSupport",
                6,                      /* thread priority          */
                8192,                   /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))OAM_thread,
                NULL,
                &tid) != GT_OK)
    {
        cmdOsPrintf("commander: failed to create OAM task\n");
        return GT_FAIL;
    }
    osTaskGracefulCallerRegister(tid, GT_TRUE, NULL, NULL);

    return GT_OK;
}


