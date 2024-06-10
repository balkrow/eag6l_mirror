#ifndef SYS_FIFO_H
#define SYS_FIFO_H

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#include <stdarg.h>
#endif

#define SYSMON_FIFO_READ "/tmp/.sysmonrdfifo"
#define SYSMON_FIFO_WRITE "/tmp/.sysmonwrfifo"

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#define FIFO_CMD_SUCCESS 0
#define FIFO_CMD_FAIL 1
#define IPC_CMD_SUCCESS 0
#define IPC_CMD_FAIL 1
#define IPC_INIT_SUCCESS 0
#define IPC_INIT_FAIL 1
#define SVC_FSM_INIT_MAX_RETRY 5
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#define IPC_FUNC_ON 1
#define IPC_FUNC_OFF 0
#endif

typedef uint8_t (*cSysmonToCPSSFuncs) (int args, ...);
#endif

enum sysmon_cmd_fifo_type
{
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	gSDKInit,
	gHello,
	gSynceEnable,
	gSynceDisable,
	gSynceIfSelect,
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	gLLCFSet,
#endif
	gPortPM,
#endif
};

enum sysmon_cmd_result_type
{
	sysmon_cmd_result_success,
	sysmon_cmd_result_fail,
};

struct port_pm_counter
{
	unsigned long tx_frame;
	unsigned long tx_byte;
	unsigned long rx_frame;
	unsigned long rx_byte;
	unsigned long rx_fcs;
	unsigned long fcs_ok;
	unsigned long fcs_nok;
};

typedef struct sysmon_cmd_fifo 
{
	enum sysmon_cmd_fifo_type	type;
	unsigned int			portid;
	unsigned int			portid2;
	long					result;
	unsigned short			vid;
	unsigned int			portmap;
	unsigned int			taggedmap;
	short				state;
	struct port_pm_counter	pm;
	char				buffer[1000];
	char				noti_msg[64];
} sysmon_fifo_msg_t;


extern void sysmon_master_fifo_init (void);
#endif /* SYS_FIFO_H */
