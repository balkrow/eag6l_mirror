/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "FreeRTOS.h"
#include <task.h>
#include "hw.h"
#include "ic.h"
#include <portmacro.h>
#include "srvCpuServices.h"
#include <srvCpuIpc.h>

extern MV_U32 CLIUartBaud;

unsigned g_sync_tcp_port;
unsigned g_async_tcp_port;

int g_dist_wm_sim_sync_sock_fd = -1;
int g_dist_wm_sim_async_sock_fd = -1;
unsigned g_treeId = 1; /* default, need to change via command line */
unsigned g_wm_mgId = 0; /* default, need to change via command line */
int wm_pipefd[2];
pid_t peer_pid;

nvichandler simulated_irq_handlers[ICMAX_INT] = { 0 };

pthread_mutex_t wm_sock_lock[__FD_SETSIZE];

unsigned getPeerPid(void)
{
    return peer_pid;
}

void InterruptHandler(void);

/* empty exception handlers */
void InterruptHandler(void)
{
    while(1);
}

/* Invoke Interrupt callbacks */
static void vICCallHandlers( void )
{
    int i;

    for (i = 0; i < ICMAX_INT; i++)
    {
        if (simulated_irq_handlers[i] != NULL)
            simulated_irq_handlers[i]();
    }
}

/*
  Register interrupt handler in virtual table
  And in WM simulation
*/
static void vICSetHandler( long interrupt, nvichandler func )
{
    /* set simulated exception table */
    printf("%s: entry\n", __func__);
    simulated_irq_handlers[interrupt] = func;
    registerInterrupt(g_dist_wm_sim_sync_sock_fd, interrupt, 0, g_treeId);
}

int iICGetSimulationSocket(void)
{
    return g_dist_wm_sim_sync_sock_fd;
}

void iICSetSimulationtreeId(unsigned treeId)
{
    g_treeId = treeId;
}

void iICSetSimulationMGId(unsigned mgId)
{
    g_wm_mgId = mgId;
}

void iICLock(int fd)
{
    pthread_mutex_lock(&wm_sock_lock[fd]);
}

void iICUnlock(int fd)
{
    pthread_mutex_unlock(&wm_sock_lock[fd]);
}

/*
  FreeRTOS handler for registering interrupt and handler
  Standard, except for interrupt line number truncation
*/
long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long priority )
{
    long ret;

    printf("%s: entry\n", __func__);

    interrupt &= 0x1; /* we have just two lines in the simulation,
			 but ARM NVIC offsets in the CM# start from
			 offset 16 */ 
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;
    if ( func == NULL )
        return -2;

    printf("%s: midpoint\n", __func__);
 
    ret = iICDisableInt( interrupt );

    vICSetHandler(irq_vec_offset, func);
    iICPrioritySet(interrupt, priority);

    if ( enable )
        ret = iICEnableInt( interrupt );

    return ret;
}

/*
  Standard FreeRTOS interrupt setup functions,
  Without simulation specific code:
*/
inline void iICSetupInt( long interrupt, long secure, long priority,
                         long edge, long enable )
{
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return;

    if ( enable )
        iICEnableInt( interrupt );
    else
        iICDisableInt( interrupt );
}

long iICUnregisterHandler(int irq_vec_offset, long interrupt )
{
    long ret;

    ret = iICDisableInt( interrupt );
    vICSetHandler(irq_vec_offset, InterruptHandler);

    return ret;
}

/*
  Enable/Disable interrupt is translated into
  WM simulation mask/unmask interrupt message
*/
long iICEnableInt( long interrupt )
{

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    unMaskInterrupt(g_dist_wm_sim_sync_sock_fd, interrupt);

    return 0;
}

long iICDisableInt( long interrupt )
{
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    maskInterrupt(g_dist_wm_sim_sync_sock_fd, interrupt);

    return 0;
}

long iICPrioritySet( long interrupt, long priority )
{

    /* priority not implemented in WM simulation */
    return 0;
}

/*
  Called when interrupt occured.
  Writes to the WM pipe file descriptor
  to notify the main thread about the
  interrupt
*/
void iICSocketIRQ(unsigned int treeId)
{
    int ret;
    unsigned char buf[4] = { 0 };

    printf("iICSocketIRQ, treeId %d\n", treeId);
    if (treeId == g_treeId)
    {
        ret = write(wm_pipefd[1], buf, 1);
        if (ret <= 0)
            printf("%s: ret %d\n", __func__, ret);
    }
}

void iICCheckWatchdog(void)
{
    MV_U32 val, base = srvCpuGetMgBase(g_wm_mgId);

    /* 0x418 is a fake register which allows us to detect watchdog
     * without fiddling with the cause registers, which will more
     * incorrectly model the simulation */
    if (!readRegister(g_dist_wm_sim_sync_sock_fd, base + 0x418, &val))
    {
        if (val&0x1) /* Watchdog occured */
        {
            writeRegister(g_dist_wm_sim_sync_sock_fd, base + 0x418, 0); /* clear it */
            printf("WATCHDOG! Terminating Simulation Process! base=%x\n", base);
            exit(123);
	}
    }
}

/*
  This thread handles asynchronuous messages as
  well as interrupts messages by select()-ing
  socket file descriptor and pipe file descriptor
*/
void iICWaitIRQThread(void *unused)
{
    fd_set rfds;
    char buf[4];
    int retval;
    /*sigset_t set;

    xPortMustCallOneTimeStartupExternalPthreads();*/

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(g_dist_wm_sim_async_sock_fd, &rfds);
        FD_SET(wm_pipefd[0], &rfds);

        /*sigfillset( &set );*/
        retval = select(1, &rfds, NULL, NULL, NULL/*, &set*/);
        if (retval)
        {
            if (FD_ISSET(g_dist_wm_sim_async_sock_fd, &rfds))
            { /* got message from ASync. socket, process it: */
                printf("Async sock rx msg\n");
                iICLock(g_dist_wm_sim_async_sock_fd);
                parse_wm_msg(g_dist_wm_sim_async_sock_fd); /* interrupt? */
                iICUnlock(g_dist_wm_sim_async_sock_fd);
            }
            if (FD_ISSET(wm_pipefd[0], &rfds))
            { /* got interrupt on file pipe : */
                retval = read(wm_pipefd[0], buf, 1);
                if (retval <= 0)
                {
                    /*printf("%s: read ret is %d\n", __func__, retval);*/
                    usleep(10000);
                    continue;
                }
                printf("IRQ on tree %d\n", g_treeId);
                vICCallHandlers(); /* invoke IRQ handles */
            }
            iICCheckWatchdog(); /* check if this is watchdog interrupt */
        }
    }

    return /*unused*/;
}

/*
  Provide a simplified soft symbolic link based on the
  base SRAM filename + TCP port used to the name
  derived from the process id. This allows to access
  the file from vuart according to the tcp port, which
  is constant between runs, instead of the pid which
  changes between runs:
*/
void softlink_shm(char *base_filename, char *file_to_link_to)
{
    char sfname[128];

    sprintf(sfname, "/tmp/%s-%u", base_filename, g_sync_tcp_port);
    unlink(sfname);
    symlink(file_to_link_to, sfname);
}

/*
  Initialize the simulation - create OS objects and threads,
  initialize TCP sockets. Must be called first before other
  functions.
*/
void vICSimInit(unsigned sync_tcp_port, unsigned async_tcp_port )
{
    /*pthread_t t;*/
    int ret, i;

    printf("%s: start\n", __func__);
    for (i = 0; i < __FD_SETSIZE; i++)
    {
        pthread_mutex_init(&wm_sock_lock[i], NULL);
    }

    g_sync_tcp_port = sync_tcp_port;
    g_async_tcp_port = async_tcp_port;

    g_dist_wm_sim_sync_sock_fd = connectToServer(sync_tcp_port);
    g_dist_wm_sim_async_sock_fd = connectToServer(async_tcp_port);
    ret = pipe(wm_pipefd);
    if (ret <= 0)
        printf("%s: pipe ret is %d\n", __func__, ret);

    /*pthread_create(&t, NULL, iICWaitIRQThread, NULL);*/
    xTaskCreate(iICWaitIRQThread, "IRQ", 32768,
                NULL, tskIDLE_PRIORITY + 2, NULL);

}

/*
  Demo IRQ handler which parses the tree in a naive manner
  and indicate by prinout the triggering interrupt.
  Used as a base for interrupt handler, as an example
  to understanding the code and for debugging it.
  Can be excluded from installation using precompiler directive.
*/
static void demo_handler(void)
{
    unsigned glbl_sum_reg, cause_reg;
    unsigned glbl_sum_reg_val, cause_reg_val;

    switch (g_treeId)
    { /* Find the global summary register and cause register by interrupt tree id: */
        case 1:
        glbl_sum_reg = 0x610;
        cause_reg = 0x144;
        break;

        case 2:
        glbl_sum_reg = 0x670;
        cause_reg = 0x10c;
        break;
    }

    glbl_sum_reg_val = srvCpuRegRead(sim_mg_base + glbl_sum_reg);
    printf("global summary reg %x cause reg %x glbl val %x\n", glbl_sum_reg, cause_reg, glbl_sum_reg_val); /* read global interrupt summary */

    if (glbl_sum_reg_val & 0x40000) /* Misc. MG1 interrupt summary? */
    {
        cause_reg_val = srvCpuRegRead(sim_mg_base + cause_reg); /* read MG1 cause register */
        printf("glbl sum reg %x cause reg %x cause val %x\n", glbl_sum_reg, cause_reg, cause_reg_val);

        /* print which interrupt happened according to the cause register: */
        if (cause_reg_val & 2)
            printf("Host to Service CPU Doorbell Interrupt!\n");

        if (cause_reg_val & 0x80)
        {
            printf("SC to SC Doorbell0 Interrupt\n");
	    ipcMessageResponse(0);
	}

        if (cause_reg_val & 0x100)
        {
            printf("SC to SC Doorbell1 Interrupt\n");
	    ipcMessageResponse(1);
	}

        if (cause_reg_val & 0x200)
        {
            printf("SC to SC Doorbell2 Interrupt\n");
	    ipcMessageResponse(2);
	}

        if (cause_reg_val & 0x400)
        {
            printf("SC to SC Doorbell3 Interrupt\n");
	    ipcMessageResponse(3);
	}


    }
}

/*
  FreeRTOS callback to initialize interrupts
  Initialize UART speed, O/S objects, and sends
  messages to set debug level and get peer
  process ID number:
*/
void vICInit( void )
{
    CLIUartBaud = 115200;
    xPortInitObjects();
    sendMsgDbgLvl(g_dist_wm_sim_async_sock_fd, 4 /* dbg irq*/);
    readPeerPid(g_dist_wm_sim_sync_sock_fd); /* usually we receive unrelated message on first read */
    peer_pid = readPeerPid(g_dist_wm_sim_sync_sock_fd);
#ifndef DONT_INSTALL_DEMO_IRQ_HANDLER
    iICRegisterHandler(1, 1, demo_handler,
                       1, 1 );
#endif
}

