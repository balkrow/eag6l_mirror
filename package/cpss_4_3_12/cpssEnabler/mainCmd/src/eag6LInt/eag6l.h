#define SYSMON_FIFO_READ "/tmp/.sysmonrdfifo"
#define SYSMON_FIFO_WRITE "/tmp/.sysmonwrfifo"

typedef struct sysmon_priv_data {
} SYS_IPC_DATA;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
typedef uint8_t (*cCPSSToSysmonFuncs) (int args, ...);
#endif

#if 0/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
typedef struct portFaultstr {
	uint16_t port;
	uint16_t flag; 
	uint32_t link; 
} PortFaultList;
#endif
