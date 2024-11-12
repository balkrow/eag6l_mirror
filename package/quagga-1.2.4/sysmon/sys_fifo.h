#ifndef SYS_FIFO_H
#define SYS_FIFO_H

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#include <stdarg.h>
#endif
#include "sysmon.h"

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
typedef uint8_t (*cSysmonReplyFuncs) (int args, ...);
#endif

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
#define SW_AGGREGATION_MODE  1
#define SW_TRANSPARENT_MODE  2

/* NOTE : vid range must be confirmed by occ. */
#define SW_TP_MODE_VID1		4001
#define SW_TP_MODE_VID2		4002
#define SW_TP_MODE_VID3		4003
#define SW_TP_MODE_VID4		4004
#define SW_TP_MODE_VID5		4005
#define SW_TP_MODE_VID6		4006
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
	gPortSetRate,
	gPortESMCenable,
	gPortAlarm,
#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
	gPortPMGet,
	gPortPMClear,
#endif
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	gPortPMFECClear,
#endif
	/* NOTE : new type MUST be inserted above gPortESMCQLupdate. */
#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-17*/
	gPortESMCQLupdate,
#endif
#endif
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
	gPortSendQL,
	gPortLocalQL,
#endif
#if 1/*[#127] SYNCE current interface <BF><BF>, balkrow, 2024-09-11*/
	gPortSyncePortConf,
#endif
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
	gPortFECEnable,
#endif /* [#152] */
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	gSwitchModeSet,
#endif
#if 1/*[#189] LLCF 동작시 100G 포트가 LOS 시 25G port를 Tx off 하도록 수정, balkrow, 2024-11-11*/
	gPortForceLinkDown,
#endif

};

enum sysmon_cmd_result_type
{
	sysmon_cmd_result_success,
	sysmon_cmd_result_fail,
};

typedef struct sysmon_cmd_fifo 
{
	enum sysmon_cmd_fifo_type   type;
	u16                         portid;
	u16                         portid2;
	u32                         speed;
	u16                         mode;
	i32                         result;
	i16                         state;
	port_status_t               port_sts[PORT_ID_EAG6L_MAX];
	port_pm_counter_t           pm[PORT_ID_EAG6L_MAX];
	char                        noti_msg[64];
} sysmon_fifo_msg_t;


extern void sysmon_master_fifo_init (void);
#endif /* SYS_FIFO_H */
