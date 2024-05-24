#define SYSMON_FIFO_READ "/tmp/.sysmonrdfifo"
#define SYSMON_FIFO_WRITE "/tmp/.sysmonwrfifo"

typedef struct sysmon_priv_data {
} SYS_IPC_DATA;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
typedef uint8_t (*cCPSSToSysmonFuncs) (int args, ...);
#endif
