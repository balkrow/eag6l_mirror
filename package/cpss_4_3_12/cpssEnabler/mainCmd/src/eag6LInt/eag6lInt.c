#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "multi_thread.h"
#include "eag6l.h"
#include "../../../../../quagga-1.2.4/sysmon/sys_fifo.h"

int sysrdfifo;
int syswrfifo;


struct multi_thread_master *master;


static int send_to_sysmon_master(sysmon_fifo_msg_t * msg)
{
	if((sysrdfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0)
		return 1;

	write(sysrdfifo, msg, sizeof(sysmon_fifo_msg_t));

	close(sysrdfifo);

	return 0;
}

static int sysmon_slave_system_command(sysmon_fifo_msg_t * msg)
{
    if(msg->type > sysmon_cmd_fifo_test && msg->type < sysmon_cmd_fifo_max)
    {
        switch(msg->type)
        {
			case sysmon_cmd_fifo_hello_test:
				cmdOsPrintf("sysmon_cmd_fifo_hello_test (REQ) : %s\n", msg->noti_msg);

				strcpy(msg->buffer, ">>> TEST DONE <<<");
				send_to_sysmon_master(msg);
				break;
            case sysmon_cmd_fifo_sftp_get:
                cmdOsPrintf("sysmon_cmd_fifo_sftp_get (REQ) :port[%d].\n", msg->portid);
                break;
            case sysmon_cmd_fifo_sftp_set:
                cmdOsPrintf("sysmon_cmd_fifo_sftp_se (REQ) : port[%d].\n", msg->portid);
                break;

				/*TODO*/

			default:
				break;
		}
	}

	return 0;
}

static int sysmon_slave_recv_fifo(struct multi_thread *thread)
{
	int len = 0;
	sysmon_fifo_msg_t req;

	memset(&req, 0, sizeof req);
	len = read(MULTI_THREAD_FD (thread),&req, sizeof(sysmon_fifo_msg_t));
	if (len < 0)
	{
		multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, MULTI_THREAD_FD (thread));
		return 0;
	}

	/**TODO: define request type, process**/
	sysmon_slave_system_command(&req);

	multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, MULTI_THREAD_FD (thread));
	return 0;
}

int sysmon_slave_fifo_recv_init (void )
{
	umask(0000);
	(void) unlink(SYSMON_FIFO_WRITE);

	if(mkfifo(SYSMON_FIFO_WRITE, 0666) != 0)
	{
		return 1;
	}

	if( (syswrfifo = open (SYSMON_FIFO_WRITE, O_RDWR)) < 0)
		return 1;

	multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, syswrfifo);

	return 0;
}

void sysmon_slave_init (void) 
{
	master = multi_thread_make_master();
	sysmon_slave_fifo_recv_init();
}

#if 0/*test-code*/
int _echo_to_terminal (struct multi_thread *thread)
{
	thread = thread;
    cmdOsPrintf("sysmon_slave_task run\n");
	multi_thread_add_timer (master, _echo_to_terminal, NULL, 10);
	return 0;
}
#endif/*test-code*/

static unsigned __TASKCONV OAM_thread(void)
{
	struct multi_thread thread;

	sysmon_slave_init();

#if 0/*test-code*/
	/*add timer*/
	multi_thread_add_timer (master, _echo_to_terminal, NULL, 10);
#endif/*test-code*/

	while (multi_thread_fetch (master, &thread))
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


