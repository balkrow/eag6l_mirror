#ifndef SYS_FIFO_H
#define SYS_FIFO_H

#define SYSMON_FIFO_READ "/tmp/.sysmonrdfifo"
#define SYSMON_FIFO_WRITE "/tmp/.sysmonwrfifo"


enum sysmon_cmd_fifo_type
{
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
	int						result;
	unsigned short			vid;
	unsigned int			portmap;
	unsigned int			taggedmap;
	short					state;
	char					buffer[1000];
	char					noti_msg[64];
} sysmon_fifo_msg_t;


extern void sysmon_master_fifo_init (void);
#endif /* SYS_FIFO_H */
