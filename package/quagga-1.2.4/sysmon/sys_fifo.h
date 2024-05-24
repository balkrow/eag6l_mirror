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

typedef uint8_t (*cSysmonToCPSSFuncs) (int args, ...);
#endif

enum sysmon_cmd_fifo_type
{
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	gSDKInit,
	gHello,
#else
	sysmon_cmd_fifo_sdk_init,
	sysmon_cmd_fifo_test,
	sysmon_cmd_fifo_hello_test,
	sysmon_cmd_fifo_sftp_get,
	sysmon_cmd_fifo_sftp_set,
	sysmon_cmd_fifo_synce_enable,
	sysmon_cmd_fifo_synce_disable,
	sysmon_cmd_fifo_synce_get,
	sysmon_cmd_fifo_synce_reset,
	sysmon_cmd_fifo_synce_source_get,
	sysmon_cmd_fifo_synce_source_set,
	sysmon_cmd_fifo_fec_enable,
	sysmon_cmd_fifo_fec_disable,
	sysmon_cmd_fifo_fec_get,
	sysmon_cmd_fifo_fec_set,
	sysmon_cmd_fifo_port_get,
	sysmon_cmd_fifo_port_set,
	sysmon_cmd_fifo_vlan_create,
	sysmon_cmd_fifo_vlan_destory,
	sysmon_cmd_fifo_vlan_get,
	sysmon_cmd_fifo_vlan_port_get,
	sysmon_cmd_fifo_vlan_port_set,
	sysmon_cmd_fifo_max,
#endif
};

enum sysmon_cmd_result_type
{
	sysmon_cmd_result_success,
	sysmon_cmd_result_fail,
};

typedef struct sysmon_cmd_fifo 
{
	enum sysmon_cmd_fifo_type	type;
	unsigned int			portid;
	int				result;
	unsigned short			vid;
	unsigned int			portmap;
	unsigned int			taggedmap;
	short				state;
	char				buffer[1000];
	char				noti_msg[64];
} sysmon_fifo_msg_t;


extern void sysmon_master_fifo_init (void);
#endif /* SYS_FIFO_H */
