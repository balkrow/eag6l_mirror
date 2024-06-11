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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "multi_thread.h"
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#include "sysmon.h"
#include "sys_fifo.h"
#include "eag6l.h"
#include "syslog.h"
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include "eag6l_fsm.h"
#endif

#if 1/*[#35] traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
extern uint8_t EAG6LVlanInit (void);
extern uint8_t EAG6LFecInit (void);
extern GT_VOID appDemoTraceOn_GT_OK_Set(GT_U32);
extern void initFaultFsmList (void);
#endif

#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
extern uint8_t EAG6LJumboFrameEnable (void);
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

uint8_t eag6LLinkStatus[PORT_ID_EAG6L_MAX];

#if 1/*PWY_FIXME*/
uint8_t eag6LPortlist2[] = 
{
	0,		/*port1-25G*/
	8,		/*port2-25G*/
	16,		/*port3-25G*/
	24,		/*port4-25G*/
	32,		/*port5-25G*/
	40,		/*port6-25G*/
	48,		/*port7-25G*/
	49,		/*port8-25G*/
	50,		/*port9-100G-offset-0*/
	51,		/*port9-100G-offset-1*/
	52,		/*port9-100G-offset-2*/
	53,		/*port9-100G-offset-3*/
};
uint8_t eag6LPortArrSize2 = sizeof(eag6LPortlist2) / sizeof(uint8_t);

uint8_t get_eag6L_dport(uint8_t lport, uint8_t offset)
{
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= PORT_ID_EAG6L_PORT8))
		return eag6LPortlist2[lport - 1];
	else if((lport == PORT_ID_EAG6L_PORT9) && (offset < 4))
		return eag6LPortlist2[lport - 1 + offset];
	else {
		syslog(LOG_ERR, "%s: invalid parameter lport[%d] offset[%d].", 
			__func__, lport, offset);
		return 255;/*invalid dport*/
	}
}
#endif /*PWY_FIXME*/

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#undef DEBUG
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



#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
GT_VOID processPortEvt
(
 GT_U8 devNum,
 GT_U32 uniEv,
 GT_U32 evExtData
 )
{
	CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;

	if(uniEv == CPSS_PP_PORT_PM_LINK_STATUS_CHANGED_E) 
	{
		cpssDxChPortManagerStatusGet(devNum, evExtData, &portConfigOutParams);
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/

		syslog(LOG_INFO,"port event : devNum=%x, uniEv=%x, evExtData=%x, portState=%s",
			devNum, uniEv, evExtData, 
			portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E ? "Link Up":"Link Down");
#endif
	}
}
#endif

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

uint8_t gCpssSDKInit(int args, ...)
{
	uint8_t result;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

	result = cpssInitSystem(38, 1, 0);
#if 1/*[#35]traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
	/*initial tag/untag forwarding */
	result = EAG6LVlanInit();
#endif
#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
	result = EAG6LJumboFrameEnable();
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	if(!result)
		gEag6LSDKInitStatus = GT_TRUE; 
#endif

#ifdef DEBUG
	syslog(LOG_INFO, "cpssInitSystem result %x", result);
#endif
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
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gHello, msg->type);
	msg->result = FIFO_CMD_SUCCESS;
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
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
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
					dport, 1/*overrideEnable*/, 
					CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
			if(ret != GT_OK)
				msg->result |= (0x1 < (portno - 1));
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			for(portno = 0; portno < 4; portno++) {
				dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, portno);
				ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
						dport, 1/*overrideEnable*/, 
						CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
				if(ret != GT_OK)
					msg->result |= (0x1 < (portno - 1));
			}
		}
	}

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(0/*devNum*/, 
					CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E/*recoveryClkType*/, 
					1/*enable*/, dport/*portNum*/, 0/*laneNum*/);
			if(ret != GT_OK) {
				msg->result |= (0x100 < (portno - 1));
				syslog(LOG_INFO, "cpssDxChPortSyncEtherRecoveryClkConfigSet ret[%d]", ret);
			}
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			for(portno = 0; portno < 4; portno++) {
				dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, portno);
				ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(0/*devNum*/, 
						CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E/*recoveryClkType*/, 
						1/*enable*/, dport/*portNum*/, 0/*laneNum*/);
				if(ret != GT_OK)
					msg->result |= (0x1 < (portno - 1));
			}
		}
	}

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(0/*devNum*/,
					dport, 0/*laneNum*/, 
					CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,
					CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E);
			if(ret != GT_OK)
				msg->result |= (0x10000 < (portno - 1));
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			for(portno = 0; portno < 4; portno++) {
				dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, portno);
				ret = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(0/*devNum*/,
						dport, 0/*laneNum*/, 
						CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,
						CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E);
				if(ret != GT_OK)
					msg->result |= (0x1 < (portno - 1));
			}
		}
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
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
					dport, 0/*overrideDisable*/, 
					CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
			/* FIXME : just disabling is OK?? */
			if(ret != GT_OK)
				msg->result |= (0x1 < (portno - 1));
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			for(portno = 0; portno < 4; portno++) {
				dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, portno);
				ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
						dport, 0/*overrideDisable*/, 
						CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
				if(ret != GT_OK)
					msg->result |= (0x1 < (portno - 1));
			}
		}
	}

	syslog(LOG_INFO, ">>> gCpssSynceDisable DONE ret[%d] <<<", ret);

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}

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
		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);
			ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
					dport, 0/*overrideDisable*/, 
					CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
			/* FIXME : just disabling is OK?? */
			if(ret != GT_OK)
				msg->result |= (0x1 < (portno - 1));
		} else if(portno == PORT_ID_EAG6L_PORT9) {
			for(portno = 0; portno < 4; portno++) {
				dport = get_eag6L_dport(PORT_ID_EAG6L_PORT9, portno);
				ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
						dport, 0/*overrideDisable*/, 
						CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
				if(ret != GT_OK)
					msg->result |= (0x1 < (portno - 1));
			}
		}
	}

	syslog(LOG_INFO, ">>> gCpssSynceIfSelect DONE ret[%d] <<<", ret);

	/* reply the result */
	send_to_sysmon_master(msg);
	return IPC_CMD_SUCCESS;
}
#endif



#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
uint8_t gCpssLLCFSet(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;

	va_start(argP, args);
	msg.type = va_arg(argP, uint32_t);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
	msg.result = FIFO_CMD_SUCCESS; 	

	syslog(LOG_INFO, "LLCFSet enable[%d] <<<", msg.portid);

	if(msg.portid == IPC_FUNC_ON)
	{
		llcf_thread = multi_thread_add_timer (master, svc_fault_fsm_timer, NULL, 1);
		if(!llcf_thread)
			msg.result = FIFO_CMD_FAIL; 	
	}
	else
	{
		multi_thread_cancel(llcf_thread);
	}

	send_to_sysmon_master(&msg);
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
		case PORT_IF_10G_XGMII:
			speed = CPSS_PORT_SPEED_10000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_XGMII_E;
			break;
		case PORT_IF_10G_RXAUI:
			speed = CPSS_PORT_SPEED_10000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_RXAUI_E;
			break;
		case PORT_IF_10G_KR:
			speed = CPSS_PORT_SPEED_10000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_KR_E;
			break;
		case PORT_IF_25G_KR:
			speed = CPSS_PORT_SPEED_25000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_KR_C_E;
			break;
		case PORT_IF_25G_CR:
			speed = CPSS_PORT_SPEED_25000_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_CR_C_E;
			break;
		case PORT_IF_100G_KR4:
			speed = CPSS_PORT_SPEED_100G_E;
			ifmode = CPSS_PORT_INTERFACE_MODE_KR4_E;
			break;
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

		if(portno < PORT_ID_EAG6L_PORT9) {
			dport = get_eag6L_dport(portno, 0);

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

 			if((msg->speed == PORT_IF_25G_KR) || (msg->speed == PORT_IF_25G_CR)) {
				ret = cpssDxChSamplePortManagerFecModeSet(0, dport, CPSS_PORT_RS_FEC_MODE_ENABLED_E);
				if(ret != GT_OK) {
					syslog(LOG_INFO, "cpssDxChPortFecModeSet ret[%d]", ret);
					goto _gCpssPortSetRate_exit;
				}
			}

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
	}
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
	syslog(LOG_INFO, "%s (REQ): type[%d/%d] port[%d] state[%d].", 
		__func__, gPortESMCenable, msg->type, msg->portid, msg->state);

	/* FIXME : call sdk api to en/disable per-port ESMC state. */

    syslog(LOG_INFO, ">>> gCpssPortESMCenable DONE ret[%d] <<<", ret);

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
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
    syslog(LOG_INFO, "called %s args=%d", __func__, args);
#endif

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
	syslog(LOG_INFO, "%s (REQ): type[%d/%d].", __func__, gPortAlarm, msg->type);

#if 1/* just reply with link status table(updated by Marvell link scan event) */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		msg->port_sts[portno].link = eag6LLinkStatus[portno];
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
  
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
#if 1/*[#??] Enhancing calling method, dustin, 2024-05-30 */
	gCpssToSysmonFuncs[req.type](1, &req);
#else
	switch(req.type) 
    {
        case gSDKInit:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;

        case gHello:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceEnable:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceDisable:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceIfSelect:
			gCpssToSysmonFuncs[req.type](3, req.type, req.portid, req.portid2);
            break;
        case gPortPM:
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	case gLLCFSet:
			gCpssToSysmonFuncs[req.type](2, req.type, req.portid);
#endif
            break;
        case gPortSetRate:
			gCpssToSysmonFuncs[req.type](4, req.type, req.portid, req.speed, req.mode);
            break;
        case gPortESMCenable:
			gCpssToSysmonFuncs[req.type](3, req.type, req.portid, req.state);
            break;
        case gPortAlarm:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
		default:
            syslog(LOG_INFO,"%s : default? (REQ) : type[%d]", __func__, req.type);
			gCpssToSysmonFuncs[req.type](1, req.type);
	}
#endif
#endif
#else
	/**TODO: define request type, process**/
	sysmon_slave_system_command(&req);
#endif
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


