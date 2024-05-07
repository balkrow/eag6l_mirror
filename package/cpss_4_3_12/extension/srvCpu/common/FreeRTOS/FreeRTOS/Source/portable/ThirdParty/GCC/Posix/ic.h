#ifndef __IC_POSIX_H__
#define __IC_POSIX_H__

typedef unsigned int UINT32;

#include <common.h>
#include <new_message.h>

#define ICMAX_INT 2
#define NVIC_GLOBAL_IRQ(x) 0

/* Interrupt enable/disable */
#define IRQ_ENABLE	( 1 )
#define IRQ_DISABLE	( 0 )

#define DEFAULT_TCP_PORT_SYNC 14141
#define DEFAULT_TCP_PORT_ASYNC 41414

typedef void ( *nvichandler )( void );

MV_32 connectToServer(unsigned serverPort);
void flushSocketBuffer(MV_32 sockfd);
void parse_other_response(int sockfd, MSG_HDR *hdrHost);
void parse_wm_msg(int sockfd);
MV_32 registerInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber, MV_U32 signalID, MV_U32 treeId);
MV_32 unMaskInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber);
MV_32 maskInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber);
MV_32 sendMsgDbgLvl(MV_32 sockfd, MV_U32 dbgLvl);
MV_32 readPeerPid(MV_32 sockfd);

void iICSetSimulationtreeId(unsigned treeId);
void iICSetSimulationMGId(unsigned mgId);
void iICLock(int fd);
void iICUnlock(int fd);
long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long config );
unsigned getPeerPid(void);
void softlink_shm(char *base_filename, char *file_to_link_to);

void iICSetupInt( long interrupt, long secure, long priority,
                  long edge, long enable );
long iICUnregisterHandler(int irq_vec_offset, long interrupt );

void vICSimInit(unsigned sync_tcp_port, unsigned async_tcp_port );

void vICInit( void );
long iICEnableInt( long interrupt);
long iICDisableInt( long interrupt );
long iICPrioritySet( long interrupt, long priority );
void iICSocketIRQ(unsigned int treeId);
void iICCheckWatchdog(void);
void iICWaitIRQThread(void *unused);

void cm3_to_msys_doorbell(void);

#endif
