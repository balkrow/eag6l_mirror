#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include <gtOs/gtEnvDep.h>
#endif
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSyncEther.h>
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include <cpss/common/port/cpssPortManager.h>
#endif
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <appDemo/userExit/userEventHandler.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
#include <stdbool.h>
#endif
#include <netinet/in.h>

#include "multi_thread.h"
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#include "sysmon.h"
#include "sys_fifo.h"
#include "eag6l.h"
#include "syslog.h"
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
#include "bp_regs.h"
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include "eag6l_fsm.h"
#endif

#undef DEBUG

#if 1/*[#52] 25G to 100G forwarding 기능 추가, balkrow, 2024-06-12*/
extern uint8_t EAG6LMacLearningnable (void);
extern uint8_t EAG6L25Gto100GFwdSet (void);
#endif

#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
int DCO_SFP_LOSS;
#endif

#if 1/*[#35] traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
extern uint8_t EAG6LVlanInit (uint8_t mode);
#else
extern uint8_t EAG6LVlanInit (void);
#endif
extern uint8_t EAG6LFecInit (void);
extern GT_VOID appDemoTraceOn_GT_OK_Set(GT_U32);
extern void initFaultFsmList (void);
#endif

#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
extern uint8_t EAG6LJumboFrameEnable (void);
#endif
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
int8_t gLocalQL = 0;
int8_t gSecSendQL = 0;
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
extern uint8_t eag6LPortlist [];
extern uint8_t eag6LPortArrSize;
extern SVC_FAULT_FSM svcPortFaultFsm[9];
int32_t svc_fault_fsm_timer(struct multi_thread *thread);

int portLfRfDetect (struct multi_thread *thread);
uint8_t gEag6LSDKInitStatus = GT_FALSE;
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
struct multi_thread *llcf_thread = NULL;
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
uint8_t esmcRxPort = 0;

uint8_t getSPortByCport(CARD_SIDE_PORT_NUM  port)
{
	uint8_t bitVal;
	switch(port)
	{
	case C_PORT1:
		bitVal = 0;
		break;
	case C_PORT2:
		bitVal = 1;
		break;
	case C_PORT3:
		bitVal = 2;
		break;
	case C_PORT4:
		bitVal = 3;
		break;
	case C_PORT5:
		bitVal = 4;
		break;
	case C_PORT6:
		bitVal = 5;
		break;
	case C_PORT7:
		bitVal = 6;
		break;
	default:
		bitVal = 7;
		break;
	}
	return bitVal;
}
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
void getPortStrByCport(CARD_SIDE_PORT_NUM port, char *port_str)
{
	switch(port)
	{
	case C_PORT1:
		sprintf(port_str, "%s", "P1");
		break;
	case C_PORT2:
		sprintf(port_str, "%s", "P2");
		break;
	case C_PORT3:
		sprintf(port_str, "%s", "P3");
		break;
	case C_PORT4:
		sprintf(port_str, "%s", "P4");
		break;
	case C_PORT5:
		sprintf(port_str, "%s", "P5");
		break;
	case C_PORT6:
		sprintf(port_str, "%s", "P6");
		break;
	case C_PORT7:
		sprintf(port_str, "%s", "P7");
		break;
	default:
		sprintf(port_str, "%s", "UNKNOWN");
		break;
	}
}
#endif

#if 0/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
static int
esmc_check_ql_tlv(T_esmc_ql_tlv *ql_tlv) {
	if(ql_tlv->type != ESMC_PDU_QL_TLV_TYPE_LEN)
		return -1;

	if(memcmp(ql_tlv->length, g_ql_tlv_length, ESMC_PDU_QL_TLV_LENGTH_LEN))
		return -1;

	/* SSM code can be zero (e.g. QL-STU has SSM code equal to 0x0 and
	 * eSSM code equal to 0xFF) */

	return 0;
}

static int
esmc_check_ext_ql_tlv(T_esmc_ext_ql_tlv *ext_ql_tlv, int *enhanced) {

	/* Received ESMC PDU does not originate from enhanced node (i.e. eSSM
	 * code is equal to 0x0), so no need for additional parsing */
	if(ext_ql_tlv->esmc_e_ssm_code == E_esmc_e_ssm_code_NONE) {
		*enhanced = 0;
		return 0;
	}

	/* Received ESMC PDU has non-zero enhanced SSM code, but need to
	 * check if other fields are valid */

	if(ext_ql_tlv->type != ESMC_PDU_EXT_QL_TLV_TYPE) {
		*enhanced = 1;
		return -1;
	}
	if(memcmp(ext_ql_tlv->length, g_ext_ql_tlv_length, ESMC_PDU_EXT_QL_TLV_LENGTH_LEN)) {
		*enhanced = 1;
		return -1;
	}

	*enhanced = 1;
	return 0;
}
#endif

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
extern GT_STATUS cpssDxChNetIfSdmaRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
);
extern PRV_CPSS_SHARED_GLOBAL_DB       *cpssSharedGlobalVarsPtr;
GT_STATUS esmcRxHandler(void);

#define EAG_HANDLER_MAX_PRIO     200

extern GT_STATUS cpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

CPSS_UNI_EV_CAUSE_ENT eagHndlrCauseArr[] =  
{
#if 1
	CPSS_PP_RX_BUFFER_QUEUE0_E,
#else	
        CPSS_PP_RX_BUFFER_QUEUE0_E, \
        CPSS_PP_RX_BUFFER_QUEUE1_E, \
        CPSS_PP_RX_BUFFER_QUEUE2_E, \
        CPSS_PP_RX_BUFFER_QUEUE3_E, \
        CPSS_PP_RX_BUFFER_QUEUE4_E, \
        CPSS_PP_RX_BUFFER_QUEUE5_E, \
        CPSS_PP_RX_BUFFER_QUEUE6_E, \
        CPSS_PP_RX_BUFFER_QUEUE7_E
#endif
};

static EAG6L_HNDLR_PARAM tParamArr;
#endif

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
extern GT_STATUS cpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
);

extern GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
);
#endif

EVENT_NOTIFY_FUNC *notifyEventToIPC = NULL;
extern GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

extern GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
);

extern GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
);

extern GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
);
#endif

int sysrdfifo;
int syswrfifo;

struct multi_thread_master *master;
#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
static uint16_t gSyncePriInf = 0xff; 
static uint16_t gSynceSecInf = 0xff; 
#endif

uint8_t eag6LLinkStatus[PORT_ID_EAG6L_MAX];
#if 1/*[#67] build err 수정, balkrow, 2024-06-27*/
uint8_t eag6LSpeedStatus[PORT_ID_EAG6L_MAX];
#else
f 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
uint8_t eag6LSpeedStatus[PORT_ID_EAG6L_MAX];
#endif
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
CPSS_DXCH_PORT_FEC_MODE_ENT FEC_MODE[PORT_ID_EAG6L_MAX];
uint8_t SPEED[PORT_ID_EAG6L_MAX];
#endif
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
uint8_t eag6LLF[PORT_ID_EAG6L_MAX];
uint8_t eag6LRF[PORT_ID_EAG6L_MAX];
#endif

uint8_t get_eag6L_dport(uint8_t lport)
{
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= (PORT_ID_EAG6L_MAX - 1)))
		return eag6LPortlist[lport - 1];
	else {
		syslog(LOG_ERR, "%s: invalid parameter lport[%d].", __func__, lport);
		return 255;/*invalid dport*/
	}
}

uint8_t get_eag6L_lport(uint8_t dport)
{
	uint8_t ii;
	
	for(ii = 0; ii < eag6LPortArrSize; ii++) {
		if(eag6LPortlist[ii] == dport)
			return (ii + 1);
	}
	return 0;
}

#if 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
uint8_t get_port_speed(uint8_t lport, uint8_t * speed)
{
	uint8_t dport;
	uint8_t ret = GT_OK;
	CPSS_PM_PORT_PARAMS_STC pparam;

#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
	/* not for 100G port */
	if(lport >= (PORT_ID_EAG6L_MAX - 1))
		return 0;
#endif
	dport = get_eag6L_dport(lport);
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	if(dport == 0xff)
		return 0;
#endif	
	memset(&pparam, 0, sizeof pparam);
	ret = cpssDxChPortManagerPortParamsGet(0, dport, &pparam);
	if(ret != GT_OK) {
		syslog(LOG_INFO, "%s: has failed[%d]", __func__, ret);
		return -1;
	}

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	if(pparam.portParamsType.regPort.speed == 
			CPSS_PORT_SPEED_25000_E) {
		if(pparam.portParamsType.regPort.ifMode == 
			CPSS_PORT_INTERFACE_MODE_KR_E)
			*speed = PORT_IF_25G_KR;
		else if(pparam.portParamsType.regPort.ifMode == 
			CPSS_PORT_INTERFACE_MODE_SR_LR_E)
			*speed = PORT_IF_25G_SR_LR;
		else
			*speed = PORT_IF_25G_KR;
	} else if(pparam.portParamsType.regPort.speed == 
			CPSS_PORT_SPEED_10000_E) {
		if(pparam.portParamsType.regPort.ifMode == 
			CPSS_PORT_INTERFACE_MODE_KR_E)
			*speed = PORT_IF_10G_KR;
		else if(pparam.portParamsType.regPort.ifMode == 
			CPSS_PORT_INTERFACE_MODE_SR_LR_E)
			*speed = PORT_IF_10G_SR_LR;
		else
			*speed = PORT_IF_10G_KR;
	}
#else
	if(pparam.portParamsType.regPort.speed == CPSS_PORT_SPEED_25000_E)
		*speed = PORT_IF_25G_KR;
	else if(pparam.portParamsType.regPort.speed == CPSS_PORT_SPEED_10000_E)
		*speed = PORT_IF_10G_KR;
#endif
	else {
		syslog(LOG_INFO, "%s: invalid speed ? [%d].", 
			__func__, pparam.portParamsType.regPort.speed);
		return -1;
	}
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
	SPEED[lport] = *speed;
#endif
	return 0;
}
#endif /*PWY_FIXME*/

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
uint8_t gEag6LIPCstate = IPC_INIT_SUCCESS; 

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);
extern GT_STATUS cpssDxChSamplePortManagerFecModeSet
(
	IN  GT_U8                                   devNum,
	IN  GT_PHYSICAL_PORT_NUM                    portNum,
	IN  CPSS_PORT_FEC_MODE_ENT                  fecMode
);
extern GT_STATUS cpssPortManagerLuaSerdesTypeGet
(
	IN  GT_SW_DEV_NUM              devNum,
	OUT CPSS_PORT_SERDES_TYPE_ENT  *serdesType
);

#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
static int esmc_ssm_and_essm_to_ql_map(T_esmc_network_option net_opt, unsigned char ssm_code, unsigned char e_ssm_code, T_esmc_ql *ql)
{
	switch(net_opt) {
	case E_esmc_network_option_1:
		switch(ssm_code) {
		case 0x00:
			*ql = E_esmc_ql_net_opt_1_INV0;
			break;
		case 0x01:
			*ql = E_esmc_ql_net_opt_1_INV1;
			break;
		case 0x02:
			switch(e_ssm_code) {
			case E_esmc_e_ssm_code_NONE:
			case E_esmc_e_ssm_code_EEC:
				*ql = E_esmc_ql_net_opt_1_PRC;
				break;
			case E_esmc_e_ssm_code_PRTC:
				*ql = E_esmc_ql_net_opt_1_PRTC;
				break;
			case E_esmc_e_ssm_code_ePRTC:
				*ql = E_esmc_ql_net_opt_1_ePRTC;
				break;
			case E_esmc_e_ssm_code_eEEC:
				*ql = E_esmc_ql_net_opt_1_eSEC;
				break;
			case E_esmc_e_ssm_code_ePRC:
				*ql = E_esmc_ql_net_opt_1_ePRC;
				break;
			default:
				return -1;
			}
			break;
		case 0x03:
			*ql = E_esmc_ql_net_opt_1_INV3;
			break;
		case 0x04:
			*ql = E_esmc_ql_net_opt_1_SSUA;
			break;
		case 0x05:
			*ql = E_esmc_ql_net_opt_1_INV5;
			break;
		case 0x06:
			*ql = E_esmc_ql_net_opt_1_INV6;
			break;
		case 0x07:
			*ql = E_esmc_ql_net_opt_1_INV7;
			break;
		case 0x08:
			*ql = E_esmc_ql_net_opt_1_SSUB;
			break;
		case 0x09:
			*ql = E_esmc_ql_net_opt_1_INV9;
			break;
		case 0x0A:
			*ql = E_esmc_ql_net_opt_1_INV10;
			break;
		case 0x0B:
			switch(e_ssm_code) {
			case E_esmc_e_ssm_code_NONE:
			case E_esmc_e_ssm_code_EEC:
				*ql = E_esmc_ql_net_opt_1_SEC;
				break;
			case E_esmc_e_ssm_code_eEEC:
				*ql = E_esmc_ql_net_opt_1_eSEC;
				break;
			default:
				return -1;
			}
			break;
		case 0x0C:
			*ql = E_esmc_ql_net_opt_1_INV12;
			break;
		case 0x0D:
			*ql = E_esmc_ql_net_opt_1_INV13;
			break;
		case 0x0E:
			*ql = E_esmc_ql_net_opt_1_INV14;
			break;
		case 0x0F:
			*ql = E_esmc_ql_net_opt_1_DNU;
			break;
		default:
			return -1;
		}
		break;

	case E_esmc_network_option_2:
		switch(ssm_code) {
		case 0x00:
			*ql = E_esmc_ql_net_opt_2_STU;
			break;
		case 0x01:
			switch(e_ssm_code) {
			case E_esmc_e_ssm_code_NONE:
			case E_esmc_e_ssm_code_EEC:
				*ql = E_esmc_ql_net_opt_2_PRS;
				break;
			case E_esmc_e_ssm_code_PRTC:
				*ql = E_esmc_ql_net_opt_1_PRTC;
				break;
			case E_esmc_e_ssm_code_ePRTC:
				*ql = E_esmc_ql_net_opt_1_ePRTC;
				break;
			case E_esmc_e_ssm_code_ePRC:
				*ql = E_esmc_ql_net_opt_2_ePRC;
				break;
			default:
				return -1;
			}
			break;
		case 0x02:
			*ql = E_esmc_ql_net_opt_2_INV2;
			break;
		case 0x03:
			*ql = E_esmc_ql_net_opt_2_INV3;
			break;
		case 0x04:
			*ql = E_esmc_ql_net_opt_2_TNC;
			break;
		case 0x05:
			*ql = E_esmc_ql_net_opt_2_INV5;
			break;
		case 0x06:
			*ql = E_esmc_ql_net_opt_2_INV6;
			break;
		case 0x07:
			*ql = E_esmc_ql_net_opt_2_ST2;
			break;
		case 0x08:
			*ql = E_esmc_ql_net_opt_2_INV8;
			break;
		case 0x09:
			*ql = E_esmc_ql_net_opt_2_INV9;
			break;
		case 0x0A:
			switch(e_ssm_code) {
			case E_esmc_e_ssm_code_NONE:
			case E_esmc_e_ssm_code_EEC:
				*ql = E_esmc_ql_net_opt_2_ST3;
				break;
			case E_esmc_e_ssm_code_eEEC:
				*ql = E_esmc_ql_net_opt_2_eEEC;
				break;
			default:
				return -1;
			}
			break;
		case 0x0B:
			*ql = E_esmc_ql_net_opt_2_INV11;
			break;
		case 0x0C:
			*ql = E_esmc_ql_net_opt_2_SMC;
			break;
		case 0x0D:
			*ql = E_esmc_ql_net_opt_2_ST3E;
			break;
		case 0x0E:
			*ql = E_esmc_ql_net_opt_2_PROV;
			break;
		case 0x0F:
			*ql = E_esmc_ql_net_opt_2_DUS;
			break;
		default:
			return -1;
		}
		break;

	case E_esmc_network_option_3:
		switch(ssm_code) {
		case 0x00:
			*ql = E_esmc_ql_net_opt_3_UNK;
			break;
		case 0x01:
			*ql = E_esmc_ql_net_opt_3_INV1;
			break;
		case 0x02:
			*ql = E_esmc_ql_net_opt_3_INV2;
			break;
		case 0x03:
			*ql = E_esmc_ql_net_opt_3_INV3;
			break;
		case 0x04:
			*ql = E_esmc_ql_net_opt_3_INV4;
			break;
		case 0x05:
			*ql = E_esmc_ql_net_opt_3_INV5;
			break;
		case 0x06:
			*ql = E_esmc_ql_net_opt_3_INV6;
			break;
		case 0x07:
			*ql = E_esmc_ql_net_opt_3_INV7;
			break;
		case 0x08:
			*ql = E_esmc_ql_net_opt_3_INV8;
			break;
		case 0x09:
			*ql = E_esmc_ql_net_opt_3_INV9;
			break;
		case 0x0A:
			*ql = E_esmc_ql_net_opt_3_INV10;
			break;
		case 0x0B:
			*ql = E_esmc_ql_net_opt_3_SEC;
			break;
		case 0x0C:
			*ql = E_esmc_ql_net_opt_3_INV12;
			break;
		case 0x0D:
			*ql = E_esmc_ql_net_opt_3_INV13;
			break;
		case 0x0E:
			*ql = E_esmc_ql_net_opt_3_INV14;
			break;
		case 0x0F:
			*ql = E_esmc_ql_net_opt_3_INV15;
			break;
		default:
			return -1;
		}
		break;

	default:
		return -1;
	}

	return 0;
}
#endif

#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
static int send_to_sysmon_master(sysmon_fifo_msg_t * msg) {
#ifdef DEBUG
	syslog(LOG_INFO, "Send msg %x to sysmon", msg->type);
#endif
#if 1/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	return write(syswrfifo, msg, sizeof(sysmon_fifo_msg_t));
#else
	if((syswrfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0) {
		syslog(LOG_INFO, "Can't open file to send to master");
		return 1;
	}

	write(syswrfifo, msg, sizeof(sysmon_fifo_msg_t));

	close(syswrfifo);

	return 0;
#endif
}

uint8_t gCpssESMCQL(T_esmc_ql ql, uint32_t port)
{
	sysmon_fifo_msg_t msg;

	msg.type = gPortESMCQLupdate;
	msg.result = FIFO_CMD_SUCCESS; 

	/*network option 1*/
	if(ql == E_esmc_ql_net_opt_1_DNU)
		msg.mode = 0x11; 
	else if(ql == E_esmc_ql_net_opt_1_SEC)
		msg.mode = 0x12; 
	else if(ql == E_esmc_ql_net_opt_1_PRC)
		msg.mode = 0x13; 
	else if(ql == E_esmc_ql_net_opt_1_SSUA)
		msg.mode = 0x14; 
	else if(ql == E_esmc_ql_net_opt_1_SSUB)
		msg.mode = 0x15; 
	else 
		msg.mode = 0xff; 
	/*network option 2*/

	msg.portid = port; 
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
#ifdef DEBUG
	{
		char port_str[10] = {0, };
		getPortStrByCport(port, port_str);
		syslog(LOG_INFO, "port %s RX QL %x", port_str, ql);
	}
#endif
#endif
	send_to_sysmon_master(&msg);
	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16
/**
* @internal showDxChRxPktReceive function
* @endinternal
*
* @brief   Show received packet function. Enabled by command line
*
* @param[in] devNum                   - The device number in which the packet was received.
*                                      queue        - The Rx queue in which the packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs
* @param[in] packetBuffs              - The received packet buffers list
* @param[in] buffLenArr               - List of buffer lengths for packetBuffs
* @param[in] rxParamsPtr              - Rx info
*                                       None
*/
#if 0
static void showDxChRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               **packetBuffs,
    IN GT_U32                              *buffLenArr,
    IN CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
)
{
    int bytesShown = 0;
    unsigned int i;
    FILE * fp = NULL;


    fp=stdout;

    fprintf(fp,"RX dev=%d queue=%d vid=%d",devNum, queueIdx,
            rxParamsPtr->dsaParam.commonParams.vid);
    if (rxParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu = &(rxParamsPtr->dsaParam.dsaInfo.toCpu);

        fprintf(fp," hwDev=%d tagged=%d", toCpu->hwDevNum, toCpu->isTagged);
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            printf(" srcTrunk=%d",toCpu->interface.srcTrunkId);
        }
        else
        {
           fprintf(fp," portNum=%d ePort=%d",
                    toCpu->interface.portNum,
                    toCpu->interface.ePort);
        }
        printf(" cpuCode=%d",toCpu->cpuCode);
    }
    /* TODO: other DSA type */
    fprintf(fp,"\n");

    for (;numOfBuff && bytesShown < PKTDUMP_MAX_BYTES; numOfBuff--,packetBuffs++,buffLenArr++)
    {
        for (i = 0; i < *buffLenArr && bytesShown < PKTDUMP_MAX_BYTES; i++, bytesShown++)
        {
            if ((bytesShown % PKTDUMP_BYTES_PER_LINE) == 0 && bytesShown != 0)
                fprintf(fp,"\n");
            fprintf(fp," %02x",(*packetBuffs)[i]);
        }
    }
    if (bytesShown != 0)
        fprintf(fp,"\n");
}
#endif

static void RxPktReceive
(
#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
 IN GT_U8                                devNum,
#endif
 IN GT_U8                               **packetBuffs,
 IN CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
 )
{
	T_esmc_ql parsed_ql = 0xff;
	CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu; 
	/*
	   T_port_ext_ql_tlv_data parsed_ext_ql_tlv_data;
	   */
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
	T_esmc_network_option net_opt = 0;
#else
	T_esmc_network_option net_opt = E_esmc_network_option_1;
#endif
	T_esmc_pdu *msg;


	if (rxParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
		toCpu = &(rxParamsPtr->dsaParam.dsaInfo.toCpu);

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
	if((esmcRxPort & (1 << getSPortByCport(toCpu->interface.portNum))) == 0)
		return;
#endif
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
	if(gLocalQL)
		net_opt = gLocalQL;
	else
		net_opt = E_esmc_network_option_1;
#endif

	msg = (T_esmc_pdu *)*packetBuffs;
	esmc_ssm_and_essm_to_ql_map(net_opt, msg->ql_tlv.ssm_code, msg->ext_ql_tlv.esmc_e_ssm_code, &parsed_ql);
#ifdef DEBUG 
	syslog(LOG_INFO,"esmc event : port %d, ssm_code %x, e_ssm_code %x, ql %x, PriInf %x, SecInf %x",
			toCpu->interface.portNum, msg->ql_tlv.ssm_code, msg->ext_ql_tlv.esmc_e_ssm_code, parsed_ql, gSyncePriInf, gSynceSecInf); 
#endif
#if 0
	if(toCpu->interface.portNum == gSyncePriInf || toCpu->interface.portNum == gSynceSecInf) 
#endif
	if(parsed_ql != 0xff) 
		gCpssESMCQL(parsed_ql, toCpu->interface.portNum);


}
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
GT_VOID processPortEvt
(
 GT_U8 devNum,
 GT_U32 uniEv,
 GT_U32 evExtData
 )
{
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
	GT_U32                              rc;
	GT_U32                              numOfBuff = BUFF_LEN;
	GT_U8                               packetBuffs[BUFF_LEN];
	GT_U32                              buffLenArr[BUFF_LEN];
	CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;
	GT_U8                               queueIdx  = 0;
#endif
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
	GT_BOOL                             lf_st, rf_st;
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	char port_str[10] = {0, };
#endif

	CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
	uint8_t portno;

	if(uniEv == CPSS_PP_PORT_PM_LINK_STATUS_CHANGED_E) 
	{
		cpssDxChPortManagerStatusGet(devNum, evExtData, &portConfigOutParams);
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
		getPortStrByCport((GT_U8)evExtData, port_str);
		syslog(LOG_INFO,"port event : devNum=%x, uniEv=%x, port=%s, portState=%s",
			devNum, uniEv, port_str, 
			portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E ? "Link Up":"Link Down");
#endif
		/* update link status cache */
		portno = get_eag6L_lport(evExtData/*dport*/);
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
		if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
		{
			eag6LLinkStatus[portno] = 1;
			eag6LRF[portno] = eag6LLF[portno] = 0;
		}
		else
		{
			eag6LLinkStatus[portno] = 0;

			/* get local fault */
			rc = cpssDxChPortXgmiiLocalFaultGet(devNum, evExtData, &lf_st);
			if(rc != GT_OK)
				syslog(LOG_INFO, "loca fault : get failed for port %s, ret[%d].",
					port_str, rc);
			else
				eag6LLF[portno] = lf_st;

			/* get remote fault */
			rc = cpssDxChPortXgmiiRemoteFaultGet(devNum, evExtData, &rf_st);
			if(rc != GT_OK)
				syslog(LOG_INFO, "loca fault : get failed for port %s, ret[%d].",
					port_str, rc);
			else
				eag6LRF[portno] = rf_st;
		}
#else
		if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
			eag6LLinkStatus[portno] = 1;
		else
			eag6LLinkStatus[portno] = 0;
#endif
	}
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
	else if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE0_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE7_E)
	{
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
		queueIdx = (GT_U8)(uniEv - CPSS_PP_RX_BUFFER_QUEUE0_E);
		rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx, &numOfBuff, 
					     (GT_U8 **)packetBuffs, buffLenArr, &rxParams);

		if(rc != GT_OK)
		{
			syslog(LOG_ERR, "cpssDxChNetIfSdmaRxPacketGet error %x", rc);
		}
		else
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
			RxPktReceive((GT_U8 **)packetBuffs, &rxParams);
#endif
#endif

#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
		cpssDxChNetIfRxBufFree(devNum, queueIdx, (GT_U8 **)packetBuffs, numOfBuff);

#endif
	}
#endif
}
#endif

uint8_t gCpssSDKInit(int args, ...)
{
#if 0/* NOTE : activate if manual sdk setup. */
	va_list argP;
	uint8_t result = 0;
	sysmon_fifo_msg_t *msg = NULL;
	args = args;
#else /**************************************/
	uint8_t result;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	if(gEag6LSDKInitStatus == GT_TRUE) 
	{
		result = 0;
		goto to_sysmon;
	}
#endif

	result = cpssInitSystem(38, 1, 0);
#if 1/*[#52] 25G to 100G forwarding 기능 추가, balkrow, 2024-06-12*/
	result += EAG6LMacLearningnable();
	result += EAG6L25Gto100GFwdSet();
#endif
#if 1/*[#35]traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
	/*initial tag/untag forwarding */
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	result += EAG6LVlanInit(SW_AGGREGATION_MODE);
#else
	result += EAG6LVlanInit();
#endif
#endif
#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
	result += EAG6LJumboFrameEnable();
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	if(!result)
		gEag6LSDKInitStatus = GT_TRUE; 
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	syslog(LOG_INFO, "cpssInitSystem result %x", result);
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
to_sysmon:
#endif
#endif /*manual sdk setup*/
	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	msg->result = result; 	
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssHello(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
#ifdef DEBUG
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gHello, msg->type);
#endif
	msg->result = FIFO_CMD_SUCCESS;
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
uint8_t gCpssEsmcToCPUSet(uint16_t port)
{
	CPSS_MAC_ENTRY_EXT_STC macEntry;
	GT_STATUS rc;
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	char port_str[10] = {0, };
#endif

	/* set Mac for traffic mirrored to CPU*/
	memset(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));

	macEntry.daCommand = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
	macEntry.isStatic = GT_TRUE;

	macEntry.key.key.macVlan.macAddr.arEther[0] = 0x01;
	macEntry.key.key.macVlan.macAddr.arEther[1] = 0x80;
	macEntry.key.key.macVlan.macAddr.arEther[2] = 0xC2;
	macEntry.key.key.macVlan.macAddr.arEther[3] = 0x0;
	macEntry.key.key.macVlan.macAddr.arEther[4] = 0x0;
	macEntry.key.key.macVlan.macAddr.arEther[5] = 0x02;
	macEntry.key.key.macVlan.vlanId = 1;
	macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

	macEntry.dstInterface.type = CPSS_INTERFACE_VID_E;
	macEntry.dstInterface.devPort.hwDevNum = 0;
	macEntry.dstInterface.devPort.portNum = port;
	macEntry.dstInterface.vidx = 1;
	macEntry.dstInterface.trunkId = 0;
	macEntry.dstInterface.vlanId = 1;
	macEntry.daRoute                  = GT_FALSE;
	macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
	macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
	macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
	macEntry.sourceID                 = 0;
	macEntry.userDefined              = 0;
	macEntry.daQosIndex               = 0;
	macEntry.saQosIndex               = 0;
	macEntry.daSecurityLevel          = 0;
	macEntry.saSecurityLevel          = 0;
	macEntry.appSpecificCpuCode       = GT_FALSE;
	macEntry.age                      = GT_TRUE;

	rc = cpssDxChBrgFdbMacEntrySet(0, &macEntry);

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	getPortStrByCport(port, port_str);
	syslog(LOG_NOTICE, "port %s set cpu trap function ret %x", port_str, rc);
#endif
	return rc;
}
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
uint8_t gCpssEsmcToCPUUnSet(void)
{
	CPSS_MAC_ENTRY_EXT_KEY_STC             macEntry;
	GT_STATUS rc = 0;
	GT_U8 devNum = 0;

	macEntry.key.macVlan.macAddr.arEther[0] = 0x01;
	macEntry.key.macVlan.macAddr.arEther[1] = 0x80;
	macEntry.key.macVlan.macAddr.arEther[2] = 0xC2;
	macEntry.key.macVlan.macAddr.arEther[3] = 0x0;
	macEntry.key.macVlan.macAddr.arEther[4] = 0x0;
	macEntry.key.macVlan.macAddr.arEther[5] = 0x02;
	macEntry.key.macVlan.vlanId = 1;

	rc = cpssDxChBrgFdbMacEntryDelete(devNum, &macEntry);
	return rc;
}
#endif

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
uint8_t gCpssSynceEnable(int args, ...)
{

	uint8_t ret = 0, i;
	uint8_t devNum = 0;
	va_list argP;
	sysmon_fifo_msg_t *msg;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	for(i = 0; i < eag6LPortArrSize; i++) 	
	{
		ret += cpssDxChPortRefClockSourceOverrideEnableSet(devNum,
			eag6LPortlist[i], 
			true, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
#if 0/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
		ret +=	gCpssEsmcToCPUSet(eag6LPortlist[i]);
#endif
	}
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
	ret +=	gCpssEsmcToCPUSet(eag6LPortlist[0]);
#endif

	syslog(LOG_NOTICE, "%s : synce enable ret=%x", __func__, ret);

	if(!ret)
		msg->result = FIFO_CMD_SUCCESS;
	else
		msg->result = FIFO_CMD_FAIL;

	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceDisable(int args, ...)
{
	uint8_t ret = 0, i;
	uint8_t devNum = 0;
	va_list argP;
	sysmon_fifo_msg_t *msg;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	for(i = 0; i < eag6LPortArrSize; i++) 	
	{
		ret += cpssDxChPortRefClockSourceOverrideEnableSet(devNum,
			eag6LPortlist[i], 
			false, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
	}

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	gCpssEsmcToCPUUnSet();
#endif

	syslog(LOG_NOTICE, "%s : synce disable ret=%x", __func__, ret);

	if(!ret)
		msg->result = FIFO_CMD_SUCCESS;
	else
		msg->result = FIFO_CMD_FAIL;

	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

#if 1/*[#127] SYNCE current interface <BF><BF>, balkrow, 2024-09-12*/
uint8_t gCpssSynceIfconf(int args, ...)
{
	uint8_t ret = 0;
	uint8_t devNum = 0, config;
	uint32_t clock_src, portNum;
	GT_BOOL enable = false;
	CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType;

	va_list argP;
	sysmon_fifo_msg_t *msg;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	clock_src = msg->mode;
	portNum = msg->portid;
	config = msg->portid2;
	va_end(argP);

	if(config) 
		enable = true;

	if(clock_src == PRI_SRC)
	{
		recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

		if(gSyncePriInf != 0xff && gSyncePriInf != portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E,
									false,
									gSyncePriInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									 0);
#endif

			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E, gSyncePriInf, ret);
		}
	}
	else
	{
		recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;
		if(gSynceSecInf != 0xff && gSynceSecInf != portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E,
									false,
									gSynceSecInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									0);
#endif
			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E, gSynceSecInf, ret);
		}
	}


	ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
							recoveryClkType,
							enable,
							portNum,
							0);

	syslog(LOG_NOTICE, "%s : clock src %x recoveryClkType %x portNum %x %x %x", __func__, clock_src, recoveryClkType, portNum, config, ret);
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-18 */
	ret += cpssDxChPortSyncEtherRecoveryClkDividerValueSet(devNum, portNum, 0, recoveryClkType, CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E);
	syslog(LOG_NOTICE, "%s : clock src %x portNum %x recoveryClkType %x ret %x", __func__, clock_src, portNum, recoveryClkType, ret);
#endif

	if(!ret)
	{
		msg->result = FIFO_CMD_SUCCESS;
		if(clock_src == PRI_SRC)
		{
			gSyncePriInf = portNum; 
		}
		else 
		{ 
			gSynceSecInf = portNum;
		}
	}
	else
		msg->result = FIFO_CMD_FAIL;

	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif

uint8_t gCpssSynceIfSelect(int args, ...)
{
	uint8_t ret = 0;
	uint8_t devNum = 0;
	uint32_t clock_src, portNum;
	GT_BOOL enable = true;
	CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType;

	va_list argP;
	sysmon_fifo_msg_t *msg;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	clock_src = msg->mode;
	portNum = msg->portid;
	va_end(argP);


	if(clock_src == PRI_SRC)
	{
		recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E;
		/** clear prev Interface */
		if(gSyncePriInf != 0xff && gSyncePriInf != portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E,
									false,
									gSyncePriInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									 0);
#endif

			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E, gSyncePriInf, ret);
		}

#if 0/*[#127] SYNCE current interface <BF><BF>, balkrow, 2024-09-12*/
		if(gSynceSecInf != 0xff && gSynceSecInf == portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E,
									false,
									gSynceSecInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									0);
#endif
			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E, gSynceSecInf, ret);

		}
#endif
	}
	else if(clock_src == SEC_SRC)
	{
		recoveryClkType = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E;
		/** clear prev Interface */
		if(gSynceSecInf != 0xff && gSynceSecInf != portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E,
									false,
									gSynceSecInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									0);
#endif
			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E, gSynceSecInf, ret);
		}

#if 0/*[#127] SYNCE current interface <BF><BF>, balkrow, 2024-09-12*/
		if(gSyncePriInf != 0xff && gSyncePriInf == portNum)
		{
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
									CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E,
									false,
									gSyncePriInf,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
									0);
#endif
			syslog(LOG_NOTICE, "%s : clear clock src %x portNum %x, ret=%x", __func__, 
			       CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E, gSyncePriInf, ret);

		}
#endif
	}
	else
		goto fail_return;




	ret += cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, 
					  recoveryClkType,
					  enable,
					  portNum,
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-05*/
					  0);
	syslog(LOG_NOTICE, "%s : clock src %x portNum %x recoveryClkType %x enable %x ret %x", __func__, clock_src, portNum, recoveryClkType, enable, ret);

	ret += cpssDxChPortSyncEtherRecoveryClkDividerValueSet(0, portNum, 0, recoveryClkType, CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E);
#endif

	syslog(LOG_NOTICE, "%s : clock src %x portNum %x recoveryClkType %x ret %x", __func__, clock_src, portNum, recoveryClkType, ret);
	if(!ret)
	{
		msg->result = FIFO_CMD_SUCCESS;
#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-18*/
		if(clock_src == PRI_SRC)
		{
			gSyncePriInf = portNum; 
			msg->portid = portNum;
			msg->portid2= 0xff;
		}
		else 
		{ 
			gSynceSecInf = portNum;
			msg->portid = 0xff;
			msg->portid2 = portNum;
		}
#endif
	}
	else
		msg->result = FIFO_CMD_FAIL;

	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
fail_return :
	return IPC_CMD_FAIL;
}
#endif

#if 0/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
uint8_t gCpssSynceEnable(int args, ...)
{
	uint8_t ret;
	uint8_t portno, dport;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gSynceEnable, msg->type);

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			dport, 1/*overrideEnable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		if(ret != GT_OK)
			msg->result |= (0x1 < (portno - 1));

		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(0/*devNum*/, 
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E/*recoveryClkType*/, 
			1/*enable*/, dport/*portNum*/, 0/*laneNum*/);
		if(ret != GT_OK) {
			msg->result |= (0x100 < (portno - 1));
			syslog(LOG_INFO, "cpssDxChPortSyncEtherRecoveryClkConfigSet ret[%d]", ret);
		}

		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(0/*devNum*/,
			dport, 0/*laneNum*/, 
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E);
		if(ret != GT_OK)
			msg->result |= (0x10000 < (portno - 1));
	}

	syslog(LOG_INFO, ">>> gCpssSynceEnable DONE total ret[0x%x] <<<", (unsigned int)msg->result);

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceDisable(int args, ...)
{
	uint8_t ret;
	uint8_t portno, dport;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gSynceDisable, msg->type);

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			dport, 0/*overrideDisable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		/* FIXME : just disabling is OK?? */
		if(ret != GT_OK)
			msg->result |= (0x1 < (portno - 1));
	}

	syslog(LOG_INFO, ">>> gCpssSynceDisable DONE ret[%d] <<<", ret);

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
uint8_t gCpssSynceIfSelect(int args, ...)
{
#if 0
	uint8_t ret;
	uint8_t portno, dport;
	va_list argP;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
#endif
	args = args;
	/*TODO: */
	return IPC_CMD_SUCCESS;
}
#else
uint8_t gCpssSynceIfSelect(int args, ...)
{
	uint8_t ret;
	uint8_t portno, dport;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] type[%d] pri[%d] sec[%d].", 
		__func__, gSynceIfSelect, msg->type, msg->type, msg->portid, msg->portid2);

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			dport, 0/*overrideDisable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		/* FIXME : just disabling is OK?? */
		if(ret != GT_OK)
			msg->result |= (0x1 < (portno - 1));
	}

	syslog(LOG_INFO, ">>> gCpssSynceIfSelect DONE ret[%d] <<<", ret);

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif /*[#56] registter update timer*/
#endif /*Verifying*/



#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
uint8_t gCpssLLCFSet(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	msg->result = FIFO_CMD_SUCCESS; 	

	syslog(LOG_INFO, "LLCFSet enable[%d] <<<", msg->portid);

	if(msg->portid == IPC_FUNC_ON)
	{
		llcf_thread = multi_thread_add_timer (master, svc_fault_fsm_timer, NULL, 1);
		if(!llcf_thread)
			msg->result = FIFO_CMD_FAIL; 	
	}
	else
	{
		if(llcf_thread)
			multi_thread_cancel(llcf_thread);
	}

	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#40] Port config for rate/ESMC/alarm, dustin, 2024-05-30 */
uint8_t gCpssPortSetRate(int args, ...)
{
	uint8_t ret = GT_OK, retry;
	uint8_t portno, dport, ifmode;
	uint16_t speed;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
	CPSS_PORT_MANAGER_STC pmgr;
	CPSS_PM_PORT_PARAMS_STC pparam;
	CPSS_PORT_MANAGER_STATUS_STC pstage;
	CPSS_PORT_SERDES_TYPE_ENT serdes;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] port[%d] speed[%d] mode[%d].", 
	       __func__, gPortSetRate, msg->type, msg->portid, msg->speed, msg->mode);

	/* set speed/mode */
	switch(msg->speed) {
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	case PORT_IF_10G_KR:
		speed = CPSS_PORT_SPEED_10000_E;
		ifmode = CPSS_PORT_INTERFACE_MODE_KR_E;
		break;
	case PORT_IF_25G_KR:
		speed = CPSS_PORT_SPEED_25000_E;
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
		ifmode = CPSS_PORT_INTERFACE_MODE_KR_E;
#endif
		break;
#endif
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	case PORT_IF_10G_SR_LR:
		speed = CPSS_PORT_SPEED_10000_E;
		ifmode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
		break;
	case PORT_IF_25G_SR_LR:
		speed = CPSS_PORT_SPEED_25000_E;
		ifmode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
		break;
#endif
	default:
		syslog(LOG_INFO, "%s: invalid speed[%d].", __func__, msg->speed);
		return GT_ERROR;
	}

	ret = cpssDxChPortManagerPortParamsStructInit(
						      CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &pparam);
	if(ret != GT_OK) {
		syslog(LOG_INFO, "cpssDxChPortManagerPortParamsStructInit ret[%d]", ret);
		goto _gCpssPortSetRate_exit;
	}

	/* FIXME : call port config api. */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno != msg->portid)	continue;

		dport = get_eag6L_dport(portno);

		pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
		ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerEventSet(1) ret[%d]", ret);
			goto _gCpssPortSetRate_exit;
		}

		ret = cpssPortManagerLuaSerdesTypeGet(0, &serdes);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssPortManagerLuaSerdesTypeGet ret[%d]", ret);
			goto _gCpssPortSetRate_exit;
		}

		pparam.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
		pparam.magic = 0x1A2BC3D4;
		pparam.portParamsType.regPort.laneParams[0].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[0].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[0].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[1].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[1].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[1].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[2].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[2].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[2].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[3].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[3].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[3].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[4].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[4].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[4].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[5].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[5].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[5].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[6].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[6].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[6].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[7].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[7].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[7].rxParams.type = serdes;
		pparam.portParamsType.regPort.portAttributes.validAttrsBitMask = 0x20;
		pparam.portParamsType.regPort.portAttributes.preemptionParams.type = CPSS_PM_MAC_PREEMPTION_DISABLED_E;
		pparam.portParamsType.regPort.portAttributes.fecMode = FEC_OFF;
		pparam.portParamsType.regPort.speed = speed;
		pparam.portParamsType.regPort.ifMode = ifmode;

		ret = cpssDxChPortManagerPortParamsSet(0, dport, &pparam);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerPortParamsSet ret[%d]", ret);
			goto _gCpssPortSetRate_exit;
		}

#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
		{
			ret = cpssDxChSamplePortManagerFecModeSet(0, dport, CPSS_PORT_RS_FEC_MODE_ENABLED_E);
			if(ret != GT_OK) {
				syslog(LOG_INFO, "cpssDxChPortFecModeSet ret[%d]", ret);
				goto _gCpssPortSetRate_exit;
			}
			/* save fec mode & speed */
			FEC_MODE[portno] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
			SPEED[portno] = msg->speed;
		}
#else /*********************************************************/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		if((msg->speed == PORT_IF_25G_KR))
#else
		if((msg->speed == PORT_IF_25G_KR) || (msg->speed == PORT_IF_25G_CR))
#endif
		{
			ret = cpssDxChSamplePortManagerFecModeSet(0, dport, CPSS_PORT_RS_FEC_MODE_ENABLED_E);
			if(ret != GT_OK) {
				syslog(LOG_INFO, "cpssDxChPortFecModeSet ret[%d]", ret);
				goto _gCpssPortSetRate_exit;
			}
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
			/* save fec mode & speed */
			FEC_MODE[portno] = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
			SPEED[portno] = msg->speed;
#endif
		}
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
		else {
			FEC_MODE[portno] = CPSS_PORT_FEC_MODE_DISABLED_E;
			SPEED[portno] = msg->speed;
		}
#endif
#endif /* [#152] */

		ret = cpssDxChPortManagerStatusGet(0, dport, &pstage);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerStatusGet ret[%d]", ret);
			goto _gCpssPortSetRate_exit;
		}

		pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
		if(pstage.portUnderOperDisable || 
		   (pstage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)) {
			if(pstage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
				pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
			else
				pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_ENABLE_E;
		}

		ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		retry = 0;
		while(ret != GT_OK) {
			if(++retry > 3) {
				syslog(LOG_INFO, "cpssDxChPortManagerEventSet(2) ret[%d]", ret);
				goto _gCpssPortSetRate_exit;
			}
			usleep(10000);
			ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		}
	}

#if 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
	/* get current speed */
	if(msg->portid != (PORT_ID_EAG6L_MAX - 1))
		get_port_speed(msg->portid, &eag6LSpeedStatus[msg->portid]);
#endif
_gCpssPortSetRate_exit:
	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortESMCenable(int args, ...)
{
	uint8_t ret = GT_OK;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	syslog(LOG_INFO, "called %s args=%d", __func__, args);
#endif

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] port[%d] eanble[%d].", 
	       __func__, gPortESMCenable, msg->type, msg->portid, msg->mode);

	if(msg->mode) 
		esmcRxPort |= 1 << (msg->portid - 1); 
	else
		esmcRxPort &= ~(1 << (msg->portid - 1)); 
#endif

	syslog(LOG_INFO, ">>> gCpssPortESMCenable DONE ret[%x] <<<", esmcRxPort);

	msg->result = ret;

    /* reply the result */
    send_to_sysmon_master(msg);
    return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortAlarm(int args, ...)
{
    uint8_t portno;
#if 0/* just reply with link status table(updated by Marvell link scan event) */
    uint8_t dport;
    uint8_t ret = GT_OK;
	GT_BOOL link, enable;
	CPSS_PORT_MANAGER_STATUS_STC pm_sts;
#endif
#if 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
	static uint8_t __speed_check__ = 0;
	uint8_t speed;
#endif
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
    syslog(LOG_INFO, "called %s args=%d", __func__, args);
#endif

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

#ifdef DEBUG
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gPortAlarm, msg->type);
#endif

#if 1/* just reply with link status table(updated by Marvell link scan event) */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		msg->port_sts[portno].link = eag6LLinkStatus[portno];
#if 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
		if(__speed_check__++ < PORT_ID_EAG6L_MAX) {
			if(get_port_speed(portno, &speed) == 0) {
				eag6LSpeedStatus[portno] = speed;
				msg->port_sts[portno].speed = speed;
			}
		} else {
			msg->port_sts[portno].speed = eag6LSpeedStatus[portno];
		}
#endif
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
		msg->port_sts[portno].local_fault  = eag6LLF[portno];
		msg->port_sts[portno].remote_fault = eag6LRF[portno];
#endif
	}
#else/***********************************************************/
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortLinkStatusGet(0, dport, &link);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChPortLinkStatusGet: port[%d] ret[%d]", 
					portno, ret);
			ret = cpssDxChPortManagerEnableGet(0, &enable);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChPortManagerEnableGet: port[%d] ret[%d]", 
					portno, ret);
			if(enable) {
				ret = cpssDxChPortManagerStatusGet(0, dport, &pm_sts);
				if(ret != GT_OK)
					syslog(LOG_INFO, "cpssDxChPortManagerStatusGet: port[%d] ret[%d]", 
						portno, ret);
			}

			msg->port_sts[portno].link = link;
			if(enable) {
				if(pm_sts.failure)
					msg->port_sts[portno].alarm = ALM_LOCAL_FAULT;
				if(pm_sts.remoteFaultConfig)
					msg->port_sts[portno].alarm = ALM_REMOTE_FAULT;
			} else 
				msg->port_sts[portno].alarm = 0;
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, 0);
		}

    	syslog(LOG_INFO, ">>> gCpssPortAlarm  DONE port[%d] link[%d] enable[%d] alarm[%d] <<<", portno, msg->port_sts[portno].link, enable, msg->port_sts[portno].alarm);
	}
#endif

	msg->result = IPC_CMD_SUCCESS;

    /* reply the result */
    send_to_sysmon_master(msg);
    return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
uint8_t gCpssPortPMGet(int args, ...)
{
	uint8_t portno, dport;
	uint8_t ret = GT_OK;
	CPSS_PORT_MAC_COUNTER_SET_STC pmc;
#if 0/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	CPSS_DXCH_PORT_FEC_MODE_ENT fecmode;
#endif
	CPSS_RSFEC_COUNTERS_STC rs_cnt;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
	struct temp_64 {
		u32 val[2];
	};

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
#ifdef DEBUG
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gPortPMGet, msg->type);
#endif

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
		/* only for link up ports. */
		if(! eag6LLinkStatus[portno])	continue;
#endif
		memset(&pmc, 0, sizeof pmc);
		dport = get_eag6L_dport(portno);
		ret = cpssDxChPortMacCountersOnPortGet(0, dport, &pmc);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortMacCountersOnPortGet: port[%d] ret[%d]", portno, ret);

#if 1 /* [#155] Fixing for PM get fail if FEC is OFF, dustin, 2024-10-18 */
		if(FEC_MODE[portno] == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) {
			memset(&rs_cnt, 0, sizeof rs_cnt);
			ret = cpssDxChRsFecCounterGet(0, dport, &rs_cnt);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChRsFecCounterGet: port[%d] ret[%d]", portno, ret);
		}
#else /*******************************************************************/
#if 0/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
#if 1/* [#74] Fixing for preventing too many callings to get FEC mode, dustin, 2024-07-09 */
		fecmode = FEC_MODE[portno];
		if(fecmode != CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) {
			ret = cpssDxChPortFecModeGet (0, dport, &fecmode);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChPortFecModeGet: port[%d] ret[%d]", portno, ret);
		}
		FEC_MODE[portno] = fecmode;
#else
		ret = cpssDxChPortFecModeGet (0, dport, &fecmode);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortFecModeGet: port[%d] ret[%d]", portno, ret);
#endif

		if(fecmode == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) {
			memset(&rs_cnt, 0, sizeof rs_cnt);
			ret = cpssDxChRsFecCounterGet(0, dport, &rs_cnt);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChRsFecCounterGet: port[%d] ret[%d]", portno, ret);
		}
#endif
#endif /* [#155] */

		/* copy data to msg->pm */
#if 1/*[#63] Fixing rx/tx frame counter of PM counters, dustin, 2024-06-20 */
	 /* Marvell aldrin3s does NOT support goodPktsSent/goodPktsRcv counters. */
		msg->pm[portno].tx_byte  =  ((u64)pmc.goodOctetsSent.l[1] << 32) | 
			                         (u64)pmc.goodOctetsSent.l[0];
		msg->pm[portno].rx_byte  =  ((u64)pmc.goodOctetsRcv.l[1] << 32) | 
			                         (u64)pmc.goodOctetsRcv.l[0];
		msg->pm[portno].tx_frame =  ((u64)pmc.mcPktsSent.l[1] << 32) | 
			                         (u64)pmc.mcPktsSent.l[0];
		msg->pm[portno].tx_frame += ((u64)pmc.brdcPktsSent.l[1] << 32) | 
			                         (u64)pmc.brdcPktsSent.l[0];
		msg->pm[portno].tx_frame += ((u64)pmc.ucPktsSent.l[1] << 32) | 
			                         (u64)pmc.ucPktsSent.l[0];
		msg->pm[portno].rx_frame =  ((u64)pmc.mcPktsRcv.l[1] << 32) | 
			                         (u64)pmc.mcPktsRcv.l[0];
		msg->pm[portno].rx_frame += ((u64)pmc.brdcPktsRcv.l[1] << 32) | 
			                         (u64)pmc.brdcPktsRcv.l[0];
		msg->pm[portno].rx_frame += ((u64)pmc.ucPktsRcv.l[1] << 32) | 
			                         (u64)pmc.ucPktsRcv.l[0];
		msg->pm[portno].rx_fcs   =  ((u64)pmc.badCrc.l[1] << 32) | 
			                         (u64)pmc.badCrc.l[0];
#else
		msg->pm[portno].tx_frame = ((u64)pmc.goodOctetsSent.l[1] << 32) |
			                        (u64)pmc.goodOctetsSent.l[0];
		msg->pm[portno].rx_frame = ((u64)pmc.goodOctetsRcv.l[1] << 32) |
			                        (u64)pmc.goodOctetsRcv.l[0];
		msg->pm[portno].tx_byte  = ((u64)pmc.goodPktsSent.l[1] << 32) |
			                        (u64)pmc.goodPktsSent.l[0];
		msg->pm[portno].rx_byte  = ((u64)pmc.goodPktsRcv.l[1] << 32) |
			                        (u64)pmc.goodPktsRcv.l[0];
		msg->pm[portno].rx_fcs   = ((u64)pmc.badCrc.l[1] << 32) |
			                        (u64)pmc.badCrc.l[0];
#endif

#if 1 /* [#155] Fixing for PM get fail if FEC is OFF, dustin, 2024-10-18 */
		if(FEC_MODE[portno] == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) {
			msg->pm[portno].fcs_ok   = ((u64)rs_cnt.correctedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.correctedFecCodeword.l[0];
			msg->pm[portno].fcs_nok  = ((u64)rs_cnt.uncorrectedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.uncorrectedFecCodeword.l[0];
		} else {
			msg->pm[portno].fcs_ok   = 0;
			msg->pm[portno].fcs_nok  = 0;
		}
#else /*******************************************************************/
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
		if(eag6LSpeedStatus[portno] == PORT_IF_25G_KR) {
			memset(&rs_cnt, 0, sizeof rs_cnt);
			ret = cpssDxChRsFecCounterGet(0, dport, &rs_cnt);

			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChRsFecCounterGet: port[%d] ret[%d]", portno, ret);
			msg->pm[portno].fcs_ok   = ((u64)rs_cnt.correctedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.correctedFecCodeword.l[0];
			msg->pm[portno].fcs_nok  = ((u64)rs_cnt.uncorrectedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.uncorrectedFecCodeword.l[0];
		} else {
			msg->pm[portno].fcs_ok   = 0;
			msg->pm[portno].fcs_nok  = 0;
		}
#else
		if(fecmode == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E) {
			msg->pm[portno].fcs_ok   = ((u64)rs_cnt.correctedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.correctedFecCodeword.l[0];
			msg->pm[portno].fcs_nok  = ((u64)rs_cnt.uncorrectedFecCodeword.l[1] << 32) |
				                        (u64)rs_cnt.uncorrectedFecCodeword.l[0];
		} else {
			msg->pm[portno].fcs_ok   = 0;
			msg->pm[portno].fcs_nok  = 0;
		}
#endif
#endif /* [#155] */

#ifdef DEBUG
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
syslog(LOG_INFO, ">>> gCpssPortPMGet : port[%d] ret[%d]", portno, ret);
syslog(LOG_INFO, ">>> gCpssPortPMGet tx_frame[%u/%u > %llu] rx_frame[%u/%u > %llu]", pmc.goodPktsSent.l[0], pmc.goodPktsSent.l[1], msg->pm[portno].tx_frame, pmc.goodPktsRcv.l[0], pmc.goodPktsRcv.l[1], msg->pm[portno].rx_frame);
syslog(LOG_INFO, ">>> gCpssPortPMGet tx_bytes[%u/%u > %llu] rx_bytes[%u/%u > %llu]", pmc.goodOctetsSent.l[0], pmc.goodOctetsSent.l[1], msg->pm[portno].tx_byte, pmc.goodOctetsRcv.l[0], pmc.goodOctetsRcv.l[1], msg->pm[portno].rx_byte);
syslog(LOG_INFO, ">>> gCpssPortPMGet rx_fcs[%llu]", msg->pm[portno].rx_fcs);
syslog(LOG_INFO, ">>> gCpssPortPMGet fcs_ok[%u/%u > %llu] fcs_nok[%u/%u > %llu]", rs_cnt.correctedFecCodeword.l[0], rs_cnt.correctedFecCodeword.l[1], msg->pm[portno].fcs_ok, rs_cnt.uncorrectedFecCodeword.l[0], rs_cnt.uncorrectedFecCodeword.l[1], msg->pm[portno].fcs_nok);
#else /*! armv8*/
syslog(LOG_INFO, ">>> gCpssPortPMGet : port[%d] ret[%d]", portno, ret);
syslog(LOG_INFO, ">>> gCpssPortPMGet tx_frame[%u/%u > %llu] rx_frame[%u/%u > %llu]", pmc.goodPktsSent.l[0], pmc.goodPktsSent.l[1], msg->pm[portno].tx_frame, pmc.goodPktsRcv.l[0], pmc.goodPktsRcv.l[1], msg->pm[portno].rx_frame);
syslog(LOG_INFO, ">>> gCpssPortPMGet tx_bytes[%u/%u > %llu] rx_bytes[%u/%u > %llu]", pmc.goodOctetsSent.l[0], pmc.goodOctetsSent.l[1], msg->pm[portno].tx_byte, pmc.goodOctetsRcv.l[0], pmc.goodOctetsRcv.l[1], msg->pm[portno].rx_byte);
syslog(LOG_INFO, ">>> gCpssPortPMGet rx_fcs[%llu]", msg->pm[portno].rx_fcs);
syslog(LOG_INFO, ">>> gCpssPortPMGet fcs_ok[%u/%u > %llu] fcs_nok[%u/%u > %llu]", rs_cnt.correctedFecCodeword.l[0], rs_cnt.correctedFecCodeword.l[1], msg->pm[portno].fcs_ok, rs_cnt.uncorrectedFecCodeword.l[0], rs_cnt.uncorrectedFecCodeword.l[1], msg->pm[portno].fcs_nok);
#endif /*End*/
#endif/*DEBUG*/
	}

	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortPMClear(int args, ...)
{
    uint8_t portno, dport;
    uint8_t ret = GT_OK;
	GT_BOOL enable;
	CPSS_PORT_MAC_COUNTER_SET_STC pmc;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gPortPMClear, msg->type);

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		dport = get_eag6L_dport(portno);
		/* first, check if clear-on-read enabled. */
		ret = cpssDxChPortMacCountersClearOnReadGet(0, dport, &enable);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadGet: port[%d] ret[%d]", portno, ret);

		/* enable if clear on read is not enabled. */
		if(! enable) {
			ret = cpssDxChPortMacCountersClearOnReadSet(0, dport, GT_TRUE);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadSet: port[%d] ret[%d]", portno, ret);
		}

		/* read to clear counters. */
		ret = cpssDxChPortMacCountersOnPortGet(0, dport, &pmc);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortMacCountersOnPortGet: port[%d] ret[%d]", portno, ret);

		/* recover if clear on read is not enabled. */
		if(! enable) {
			ret = cpssDxChPortMacCountersClearOnReadSet(0, dport, GT_FALSE);
			if(ret != GT_OK)
				syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadSet: port[%d] ret[%d]", portno, ret);
		}
	}

	syslog(LOG_INFO, ">>> gCpssPortPMClear DONE <<<");

	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
uint8_t gCpssPortPMFECClear(int args, ...)
{
    uint8_t portno, dport;
    uint8_t ret = GT_OK;
	GT_BOOL enable;
	CPSS_RSFEC_COUNTERS_STC rs_cnt;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] for port[%d].", 
		__func__, gPortPMFECClear, msg->type, msg->portid);

	portno = msg->portid;
	dport = get_eag6L_dport(portno);

	/* first, check if clear-on-read enabled. */
	ret = cpssDxChPortMacCountersClearOnReadGet(0, dport, &enable);
	if(ret != GT_OK)
		syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadGet: port[%d] ret[%d]", portno, ret);

	/* enable if clear on read is not enabled. */
	if(! enable) {
		ret = cpssDxChPortMacCountersClearOnReadSet(0, dport, GT_TRUE);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadSet: port[%d] ret[%d]", portno, ret);
	}

	/* read to clear counters. */
	if(eag6LSpeedStatus[portno] == PORT_IF_25G_KR) {
		memset(&rs_cnt, 0, sizeof rs_cnt);
		ret = cpssDxChRsFecCounterGet(0, dport, &rs_cnt);

		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChRsFecCounterGet: port[%d] ret[%d]", portno, ret);
	} else

	/* recover if clear on read is not enabled. */
	if(! enable) {
		ret = cpssDxChPortMacCountersClearOnReadSet(0, dport, GT_FALSE);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChPortMacCountersClearOnReadSet: port[%d] ret[%d]", portno, ret);
	}

	syslog(LOG_INFO, ">>> gCpssPortPMFECClear DONE <<<");
	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
uint8_t gCpssNone(int args, ...)
{
	args = args;
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortSendQL(int args, ...)
{
	uint8_t ret = GT_OK;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	syslog(LOG_INFO, "called %s args=%d", __func__, args);
#endif

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] val[%d].", 
	       __func__, gPortSendQL, msg->type, msg->portid);

	syslog(LOG_INFO, ">>> gCpssPortSendQL DONE ret[%x] <<<", ret);

	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssLocalQL(int args, ...)
{
	uint8_t ret = GT_OK;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	syslog(LOG_INFO, "called %s args=%d", __func__, args);
#endif

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] val[%d].", 
	       __func__, gPortLocalQL, msg->type, msg->portid);

	if(((msg->portid >> 4) & 0xf) == 0x1)
		gLocalQL = E_esmc_network_option_1;
	else if(((msg->portid >> 4) & 0xf) == 0x2)
		gLocalQL = E_esmc_network_option_2;
	else
		ret = GT_FAIL;

	syslog(LOG_INFO, ">>> gCpssSecSendQL[%x] DONE ret[%x] <<<", gLocalQL, ret);

	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
	return IPC_CMD_SUCCESS;
}
#endif

#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
uint8_t gCpssPortFECEnable(int args, ...)
{
	uint8_t ret = GT_OK, retry;
	uint8_t portno, dport, ifmode;
	uint16_t speed;
	va_list argP;
	bool enable;
	sysmon_fifo_msg_t *msg = NULL;
	CPSS_PORT_MANAGER_STC pmgr;
	CPSS_PM_PORT_PARAMS_STC pparam;
	CPSS_PORT_MANAGER_STATUS_STC pstage;
	CPSS_PORT_SERDES_TYPE_ENT serdes;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] for port[%d].", 
		__func__, gPortFECEnable, msg->type, msg->portid);

	portno = msg->portid;
	dport = get_eag6L_dport(portno);
	enable = msg->state;

	/* set speed/mode */
	switch(SPEED[portno]) {
		case PORT_IF_10G_KR:
			speed = CPSS_PORT_SPEED_10000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_KR_E;
			break;
		case PORT_IF_25G_KR:
			speed = CPSS_PORT_SPEED_25000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_KR_E;
			break;
		case PORT_IF_10G_SR_LR:
			speed = CPSS_PORT_SPEED_10000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
			break;
		case PORT_IF_25G_SR_LR:
			speed = CPSS_PORT_SPEED_25000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
			break;
		default:
			syslog(LOG_INFO, "%s: invalid speed[%d].", __func__, SPEED[portno]);
			return GT_ERROR;
	}

	/* for 100G case. */
	if(portno == PORT_ID_EAG6L_PORT7) {
		speed  = CPSS_PORT_SPEED_100G_E;
		ifmode = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
	}

	ret = cpssDxChPortManagerPortParamsStructInit(
						      CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &pparam);
	if(ret != GT_OK) {
		syslog(LOG_INFO, "cpssDxChPortManagerPortParamsStructInit ret[%d]", ret);
		goto _gCpssPortFECEnable_exit;
	}

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno != msg->portid)	continue;

		dport = get_eag6L_dport(portno);

		pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
		ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerEventSet(1) ret[%d]", ret);
			goto _gCpssPortFECEnable_exit;
		}

		ret = cpssPortManagerLuaSerdesTypeGet(0, &serdes);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssPortManagerLuaSerdesTypeGet ret[%d]", ret);
			goto _gCpssPortFECEnable_exit;
		}

		pparam.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
		pparam.magic = 0x1A2BC3D4;
		pparam.portParamsType.regPort.laneParams[0].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[0].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[0].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[1].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[1].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[1].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[2].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[2].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[2].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[3].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[3].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[3].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[4].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[4].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[4].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[5].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[5].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[5].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[6].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[6].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[6].rxParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[7].validLaneParamsBitMask = 0x0;
		pparam.portParamsType.regPort.laneParams[7].txParams.type = serdes;
		pparam.portParamsType.regPort.laneParams[7].rxParams.type = serdes;
		pparam.portParamsType.regPort.portAttributes.validAttrsBitMask = 0x20;
		pparam.portParamsType.regPort.portAttributes.preemptionParams.type = CPSS_PM_MAC_PREEMPTION_DISABLED_E;
		pparam.portParamsType.regPort.portAttributes.fecMode = FEC_OFF;
		pparam.portParamsType.regPort.speed = speed;
		pparam.portParamsType.regPort.ifMode = ifmode;

		ret = cpssDxChPortManagerPortParamsSet(0, dport, &pparam);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerPortParamsSet ret[%d]", ret);
			goto _gCpssPortFECEnable_exit;
		}

		ret = cpssDxChSamplePortManagerFecModeSet(0, dport, 
				enable ?  CPSS_PORT_RS_FEC_MODE_ENABLED_E : 
				          CPSS_PORT_FEC_MODE_DISABLED_E);
		if(ret != GT_OK)
			syslog(LOG_INFO, "cpssDxChSamplePortManagerFecModeSet: port[%d] enable[%d] ret[%d]", portno, enable, ret);
		/* save fec mode & speed */
		else
			FEC_MODE[portno] = enable ? 
				CPSS_PORT_RS_FEC_MODE_ENABLED_E : CPSS_PORT_FEC_MODE_DISABLED_E;

		ret = cpssDxChPortManagerStatusGet(0, dport, &pstage);
		if(ret != GT_OK) {
			syslog(LOG_INFO, "cpssDxChPortManagerStatusGet ret[%d]", ret);
			goto _gCpssPortFECEnable_exit;
		}

		pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
		if(pstage.portUnderOperDisable || 
		   (pstage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)) {
			if(pstage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
				pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
			else
				pmgr.portEvent = CPSS_PORT_MANAGER_EVENT_ENABLE_E;
		}

		ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		retry = 0;
		while(ret != GT_OK) {
			if(++retry > 3) {
				syslog(LOG_INFO, "cpssDxChPortManagerEventSet(2) ret[%d]", ret);
				goto _gCpssPortFECEnable_exit;
			}
			usleep(10000);
			ret = cpssDxChPortManagerEventSet(0, dport, &pmgr);
		}
	}

	syslog(LOG_INFO, ">>> gCpssPortFECEnable DONE~!! <<<");
_gCpssPortFECEnable_exit:
	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif /* [#152] */

#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
uint8_t gCpssDcoSFPState(int args, ...)
{
	uint8_t ret = GT_OK;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	DCO_SFP_LOSS = msg->state;
	syslog(LOG_INFO, "DCO_SFP_LOSS %d", DCO_SFP_LOSS);
	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
uint8_t gCpssSwitchModeSet(int args, ...)
{
extern uint8_t EAG6LSwitchModeSet(uint8_t enable);

	uint8_t ret = GT_OK;
	va_list argP;
	bool enable;
	sysmon_fifo_msg_t *msg = NULL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] state[%d].", 
		__func__, gSwitchModeSet, msg->type, msg->state);

	enable = msg->state;

	ret = EAG6LSwitchModeSet(enable ? SW_TRANSPARENT_MODE : SW_AGGREGATION_MODE);

	syslog(LOG_INFO, ">>> gCpssSwitchModeSet enable[%d] DONE~!! <<<", enable);
	msg->result = ret;

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif /* [#142] */

cCPSSToSysmonFuncs gCpssToSysmonFuncs[] =
{
	gCpssSDKInit,
	gCpssHello,
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gCpssSynceEnable,
	gCpssSynceDisable,
	gCpssSynceIfSelect,
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	gCpssLLCFSet,
#endif
	gCpssPortSetRate,
	gCpssPortESMCenable,
	gCpssPortAlarm,
#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
	gCpssPortPMGet,
	gCpssPortPMClear,
#endif
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	gCpssPortPMFECClear,
#endif
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
	gCpssNone,
	gCpssPortSendQL,
	gCpssLocalQL,
#endif
#if 1/*[#127] SYNCE current interface <BF><BF>, balkrow, 2024-09-12*/
	gCpssSynceIfconf,
#endif
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
	gCpssPortFECEnable,
#endif
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	gCpssSwitchModeSet,
#endif
#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
	gCpssDcoSFPState,
#endif
};

const uint32_t funcsListLen = sizeof(gCpssToSysmonFuncs) / sizeof(cCPSSToSysmonFuncs);

void eag6l_ipc_init(void) {

	umask(0000);
	unlink(SYSMON_FIFO_WRITE);
	unlink(SYSMON_FIFO_READ);

	if(mkfifo(SYSMON_FIFO_WRITE, 0666) != 0)
	{
		gEag6LIPCstate = IPC_INIT_FAIL;
#ifdef DEBUG
#endif
	}
	else
	{
		if((sysrdfifo = open(SYSMON_FIFO_WRITE/*from-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}

	if(mkfifo(SYSMON_FIFO_READ, 0666) != 0)
	{
		gEag6LIPCstate = IPC_INIT_FAIL;
#ifdef DEBUG
#endif
	}
#if 1/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	else
	{
		if((syswrfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}
#endif/*PWY_FIXME*/
}
#endif

int sysmon_slave_recv_fifo(struct multi_thread *thread)
{
	int len = 0;
	sysmon_fifo_msg_t req;

	memset(&req, 0, sizeof req);
	len = read(MULTI_THREAD_FD (thread),&req, sizeof(sysmon_fifo_msg_t));
	if (len < 0)
	{
		syslog(LOG_ERR, "%s : Receive len %x error", __func__, len);
		goto next_time;
	}

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	if(req.type > funcsListLen || req.type < 0) 
	{
		syslog(LOG_ERR, "%s : Receive msg %x error", __func__, req.type);
		goto next_time;
	}
#endif
  
	gCpssToSysmonFuncs[req.type](1, &req);

next_time:
	multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, MULTI_THREAD_FD (thread));
	return 0;
}

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
int sysmon_slave_fifo_recv_init (void )
{
	if(gEag6LIPCstate == IPC_INIT_SUCCESS)
		multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, sysrdfifo);
	return 0;
}
#else
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
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
SVC_FAULT_ST fault_st_transition
(
 SVC_FAULT_ST state,
 SVC_FAULT_EVT evt
) 
{
	SVC_FAULT_ST rc = SVC_FAULT_ST_INIT;
	switch (state) {
	case SVC_FAULT_ST_INIT:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LFRF_CHECK;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;

		break;
	case SVC_FAULT_ST_LFRF_CHECK:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LFRF_CHECK;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_FWD_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_FWD_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_LF;
		break;
	case SVC_FAULT_ST_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_FWD_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_FWD_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_RF;
		break;
	case SVC_FAULT_ST_CLR_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_LF;
		break;
	case SVC_FAULT_ST_CLR_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_RF;
		break;
	case SVC_FAULT_ST_NO_LFRF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_NO_LFRF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	default:
		break;
	}
	return rc;
}

int32_t svc_fault_fsm_timer(struct multi_thread *thread)
{
	uint8_t i;
	SVC_FAULT_ST state, next_state;
	SVC_FAULT_ST evt;
	CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
	GT_U8 devNum = 0;

	thread = thread;

	if(gEag6LSDKInitStatus == GT_FALSE)
		goto next_timer;

	for(i = 0; i < eag6LPortArrSize; i++)
	{

		cpssDxChPortManagerStatusGet(devNum, eag6LPortlist[i], &portConfigOutParams);

		evt = (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E) ? 
			SVC_FAULT_EVT_LINK_UP : SVC_FAULT_EVT_LINK_DOWN;

#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
		if(DCO_SFP_LOSS)
			evt = SVC_FAULT_EVT_LINK_DOWN;
#endif

		state = fault_st_transition(svcPortFaultFsm[i].state, evt);

		next_state = svcPortFaultFsm[i].cb[state](i);
#ifdef DEBUG
		syslog(LOG_INFO, "port=%d, state=%d, evt=%d", eag6LPortlist[i], state, evt);
#endif
		svcPortFaultFsm[i].state = next_state;
		svcPortFaultFsm[i].evt = evt;
	}
next_timer:	
	multi_thread_add_timer_msec(master, svc_fault_fsm_timer, NULL, 200);
	return 0;
}

#endif

void sysmon_slave_init (void) 
{

	master = multi_thread_make_master();
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	eag6l_ipc_init();
#endif
	sysmon_slave_fifo_recv_init();
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	initFaultFsmList ();
#endif
}

static unsigned __TASKCONV OAM_thread(void)
{
	struct multi_thread thread;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	openlog("EAG6LIPC", LOG_PID | LOG_CONS| LOG_NDELAY | LOG_PID, LOG_USER);
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	notifyEventToIPC = processPortEvt;
#endif
	sysmon_slave_init();

	syslog(LOG_NOTICE, "Start EAG6L IPC Thread");
	while (multi_thread_fetch (master, &thread))
		multi_thread_call (&thread);

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
static GT_STATUS prvUniEvMaskAllSet
(
    CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    GT_U32                      arrLength,
    CPSS_EVENT_MASK_SET_ENT     operation
)
{
        GT_STATUS rc = GT_OK;
        GT_U32    i;                          /* Iterator                     */
        GT_U8     dev = 0;                    /* Device iterator              */

        /* unmask the interrupt */
        for(i = 0; i < arrLength; i++)
        {
                if(cpssUniEventArr[i] <= CPSS_PP_UNI_EV_MAX_E)
                {
			rc = cpssEventDeviceMaskSet(dev, cpssUniEventArr[i], operation);
			if(rc != GT_OK)
			{
				goto exit_cleanly_lbl;
			}
                }
                else
                {
                        rc = GT_FAIL;
                        goto exit_cleanly_lbl;
                }
        }

exit_cleanly_lbl:
/*    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);
        osTaskUnLock();*/

        return rc;
} 

static unsigned __TASKCONV rxTxEventsHndlr
(
GT_VOID * param
)
{
        GT_STATUS       rc;                                        /* return code         */
        GT_U32          i;                                                /* iterator            */
        GT_UINTPTR      evHndl;                                /* event handler       */
        GT_U32          evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];        /* event bitmap array  */
        GT_U32          evBitmap;                                                                        /* event bitmap 32 bit */
        GT_U32          evExtData;                                                                        /* event extended data */
        GT_U8           devNum;                                                                                /* device number       */
        GT_U32          uniEv;                                                                                /* unified event cause */
        GT_U32          evCauseIdx;                                                                        /* event index         */
        EAG6L_HNDLR_PARAM      *hndlrParamPtr;                                                                /* bind event array    */

        hndlrParamPtr = (EAG6L_HNDLR_PARAM*)param;
        evHndl        = hndlrParamPtr->evHndl;


        while(1)
        {
                rc = cpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
#if 0
                syslog(LOG_INFO, "cpssEventRecv: recv %d\n", rc);
#endif
                if(GT_OK != rc)
                {
                        /* If seems like this result is not unusual... */
                        /* DBG_LOG(("CpssEventSelect: err\n", 1, 2, 3, 4, 5, 6));*/
                        continue;
                }

                for(evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
                {
                        if(evBitmapArr[evCauseIdx] == 0)
                        {
                                continue;
                        }

                        evBitmap = evBitmapArr[evCauseIdx];

                        for(i = 0; evBitmap; evBitmap >>= 1, i++)
                        {
                                if((evBitmap & 1) == 0)
                                {
                                        continue;
                                }
                                uniEv = (evCauseIdx << 5) + i;

                                if((rc=cpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                                {
                                        syslog(LOG_INFO, "cpssEventRecv: %08x <dev=%d, uniEv=%d, extData=0x%0x>\n",
                                                         (GT_U32)hndlrParamPtr->hndlrIndex, devNum, uniEv, evExtData);


                                        /* Treat packet transmit */
					/*
                                        rc = rxTxEnPpEvTreat(devNum, uniEv, evExtData);
					*/

                                } else
                                {
                                        syslog(LOG_ERR, "cpssEventRecv: error %d\n", rc);
                                }
                        }
                }
        }
    return 0;
}

GT_STATUS esmcRxHandler(void)
{

    GT_STATUS           rc;                                 /* The returned code            */
    GT_TASK             eventHandlerTid; /* The task Tid                 */
    GT_U32 evHndlrCauseArrSize = sizeof(eagHndlrCauseArr)/sizeof(eagHndlrCauseArr[0]);

    tParamArr.hndlrIndex = 1;		
    rc = cpssEventBind(eagHndlrCauseArr, evHndlrCauseArrSize, &tParamArr.evHndl);
    syslog(LOG_ERR, "cpssEventBind: rc %d\n", rc);
    rc += prvUniEvMaskAllSet(eagHndlrCauseArr, evHndlrCauseArrSize, CPSS_EVENT_UNMASK_E);
    syslog(LOG_ERR, "prvUniEvMaskAllSet: rc %d\n", rc);

    rc += osTaskCreate("esmcTxRxHndl",
		      EAG_HANDLER_MAX_PRIO,
		      _32KB,
		      rxTxEventsHndlr,
		      &tParamArr,
		      &eventHandlerTid);

    if(rc != GT_OK)
    {
	syslog(LOG_ERR, "failed to create esmcTxRxHndl task, rc %d\n", rc);
	return GT_FAIL;
    }
    return GT_OK;
}
#endif



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
        cmdOsPrintf("OAMSupport: failed to create OAM task\n");
        return GT_FAIL;
    }
    osTaskGracefulCallerRegister(tid, GT_TRUE, NULL, NULL);
    return GT_OK;
}


