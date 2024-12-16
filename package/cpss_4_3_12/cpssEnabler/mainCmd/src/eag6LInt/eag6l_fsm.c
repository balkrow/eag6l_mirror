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
#include "eag6l_fsm.h"
#if 1/*[#40] IPC source 정리, balkrow, 2024-06-11 */
#include "sysmon.h"
#endif

extern uint8_t eag6LPortArrSize;
extern uint8_t eag6LPortlist [];
uint8_t WdmPort = EAG6L_WDM_PORT;
#define DEBUG
#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
extern int DCO_SFP_LOSS;
#endif
#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-13*/
extern uint8_t get_eag6L_lport(uint8_t dport);
extern uint8_t eag6LLF[PORT_ID_EAG6L_MAX];
extern uint8_t eag6LRF[PORT_ID_EAG6L_MAX];

uint8_t llcf_act_lf[PORT_ID_EAG6L_MAX];
uint8_t llcf_act_rf[PORT_ID_EAG6L_MAX];
#endif

extern GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

extern GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
);

#if 1/*[#213] SFP equip/not equip ¿ LLCF ¿¿, balkrow, 2024-11-25*/
extern GT_STATUS cpssDxChPortRemoteFaultSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         send
);
#endif

#if 1/*[#214] LLCF on 상태에서 100G link down시 Localfault 전달, balkrow, 2024-11-27*/
GT_STATUS prvCpssCommonPortIfModeToHwsTranslate
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode,
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed,
    OUT MV_HWS_PORT_STANDARD            *hwsIfModePtr
);

GT_STATUS mvHwsPortSendLocalFaultSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 enable
);
#endif

#if 1/*[#40] IPC source 정리, balkrow, 2024-06-11 */
SVC_FAULT_FSM svcPortFaultFsm[PORT_ID_EAG6L_MAX];
#else
SVC_FAULT_FSM svcPortFaultFsm[EAG6L_PORT_MAX];
#endif

SVC_FAULT_ST faultFsmInit
(
 uint16_t port
)
{
	svcPortFaultFsm[port].fwdPortBmp = 0;
	svcPortFaultFsm[port].retry_cnt = 0;
	return SVC_FAULT_ST_INIT;
}

SVC_FAULT_ST checkLFSFState
(
 uint16_t port
)
{
	GT_STATUS rc = SVC_FAULT_ST_LFRF_CHECK;
	GT_BOOL islocalFault = GT_FALSE, isRemoteFault = GT_FALSE;
	GT_U8 devNum = 0;

	/*only WDM port, forwarding RF */
	if(eag6LPortlist[port] != WdmPort)
	{
		rc = SVC_FAULT_ST_INIT;
		goto next_state;
	}

	cpssDxChPortXgmiiLocalFaultGet(devNum, eag6LPortlist[port], &islocalFault);
	cpssDxChPortXgmiiRemoteFaultGet(devNum, eag6LPortlist[port], &isRemoteFault);

	syslog(LOG_INFO, "port=%d, islocalFault=%d, isRemoteFault=%d", eag6LPortlist[port], islocalFault, isRemoteFault);

	if(islocalFault)
		rc = SVC_FAULT_ST_LF;
#if 1/*[#165] DCO SFP 관련 LLCF 수정, balkrow, 2024-10-24*/ 
	else if(DCO_SFP_LOSS)
		rc = SVC_FAULT_ST_LF;
#endif
	else if(isRemoteFault)
		rc = SVC_FAULT_ST_RF;

	if(svcPortFaultFsm[port].retry_cnt > FAULT_CHECK_MAX)
	{
		svcPortFaultFsm[port].retry_cnt = 0;
		rc = SVC_FAULT_ST_NO_LFRF;
	}
	else
		svcPortFaultFsm[port].retry_cnt++;
next_state :	
	return rc;
}

SVC_FAULT_ST portEventLFstate
(
 uint16_t port
)
{
	int8_t i;
	GT_STATUS rc = SVC_FAULT_ST_FWD_LF, ret;
	GT_U8 devNum = 0;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		if(eag6LPortlist[i] == eag6LPortlist[port]) 
			continue;

		if(!(svcPortFaultFsm[i].fwdPortBmp & (1 << i)))
		{
#if 1
			 CPSS_PORT_MANAGER_STC                  portEventStc;
			 portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E;
			 ret = cpssDxChPortManagerEventSet(devNum, eag6LPortlist[i], &portEventStc);
#else
			CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
			cpssDxChPortManagerStatusGet(devNum, eag6LPortlist[i], &portConfigOutParams);
			ret = cpssDxChPortRemoteFaultSet(devNum, eag6LPortlist[i], 
						   portConfigOutParams.ifMode,
						   portConfigOutParams.speed,
						   GT_TRUE);
#endif
			if(ret == GT_OK)
				svcPortFaultFsm[i].fwdPortBmp |= 1 << i;
#ifdef DEBUG
			syslog(LOG_INFO, "port=%d, ret=%d, fwdPortBmp=%x", eag6LPortlist[i], 
			       ret, svcPortFaultFsm[i].fwdPortBmp);
#endif
		}
	}

	return rc;
}

SVC_FAULT_ST portEventRFstate
(
 uint16_t port
)
{
	int8_t i;
	GT_STATUS rc = SVC_FAULT_ST_FWD_RF, ret;
	GT_U8 devNum = 0;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		if(eag6LPortlist[i] == eag6LPortlist[port]) 
			continue;

		if(!(svcPortFaultFsm[i].fwdPortBmp & (1 << i)))
		{
#if 1
			 CPSS_PORT_MANAGER_STC                  portEventStc;
			 portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E;
			 ret = cpssDxChPortManagerEventSet(devNum, eag6LPortlist[i], &portEventStc);
#else
			CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
			cpssDxChPortManagerStatusGet(devNum, eag6LPortlist[i], &portConfigOutParams);
			cpssDxChPortRemoteFaultSet(devNum, eag6LPortlist[i], 
						   portConfigOutParams.ifMode,
						   portConfigOutParams.speed,
						   GT_TRUE);
#endif
			if(ret == GT_OK)
				svcPortFaultFsm[i].fwdPortBmp |= 1 << i;
#ifdef DEBUG
			syslog(LOG_INFO, "port=%d, ret=%d, fwdPortBmp=%x", eag6LPortlist[i], 
			       ret, svcPortFaultFsm[i].fwdPortBmp);
#endif
		}
	}

	return rc;
}

#if 1/*[#214] LLCF on 상태에서 100G link down시 Localfault 전달, balkrow, 2024-11-27*/
int32_t llcf_process(int8_t port, int8_t evt)
{
	int32_t ret = 0;
#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-13*/
	GT_U8	devNum = 0, portNum = PORT_ID_EAG6L_PORT7;
#endif
	GT_U32  portGroup = 0;
	MV_HWS_PORT_STANDARD  portMode;

#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-13*/
	int8_t lport;
	lport = get_eag6L_lport(port);
#endif
	if(evt)
	{
		CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;

		cpssDxChPortManagerStatusGet(devNum, port, &portConfigOutParams);
		prvCpssCommonPortIfModeToHwsTranslate(devNum, portConfigOutParams.ifMode, portConfigOutParams.speed, &portMode);
#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-13*/
		syslog(LOG_INFO, "port %d lf %x rf %x", portNum, eag6LLF[portNum], eag6LRF[portNum]);

		if(eag6LLF[portNum])
		{
			ret = mvHwsPortSendLocalFaultSet(devNum, portGroup, port, portMode, GT_TRUE);
			llcf_act_lf[lport] = 1;
		}
		else if(eag6LRF[portNum])
		{
			ret = cpssDxChPortRemoteFaultSet(devNum, port, 
						   portConfigOutParams.ifMode,
						   portConfigOutParams.speed,
						   GT_TRUE);
			llcf_act_rf[lport] = 1;
		}
#endif
	}
	else
	{
		CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
		cpssDxChPortManagerStatusGet(devNum, port, &portConfigOutParams);
		prvCpssCommonPortIfModeToHwsTranslate(devNum, portConfigOutParams.ifMode, portConfigOutParams.speed, &portMode);

#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-13*/
		if(llcf_act_lf[lport] == 1)
		{
			ret = mvHwsPortSendLocalFaultSet(devNum, portGroup, port, portMode, GT_FALSE);
			llcf_act_lf[lport] = 0;
		}
		else if(llcf_act_rf[lport] == 1)
		{
			ret = cpssDxChPortRemoteFaultSet(devNum, port, 
						   portConfigOutParams.ifMode,
						   portConfigOutParams.speed,
						   GT_FALSE);
			llcf_act_rf[lport] = 0;
		}
#else
		ret = mvHwsPortSendLocalFaultSet(devNum, portGroup, port, portMode, GT_FALSE);
#endif
	}
	return ret;
}
#endif

SVC_FAULT_ST portEventRFCLRstate
(
 uint16_t port
)
{
	int8_t i;
	GT_STATUS rc = SVC_FAULT_ST_INIT,ret;
	GT_U8 devNum = 0;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		if(eag6LPortlist[i] == eag6LPortlist[port]) 
			continue;

		if(svcPortFaultFsm[i].fwdPortBmp & (1 << i))
		{
#if 1
			 CPSS_PORT_MANAGER_STC                  portEventStc;
			 portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E;
			 ret = cpssDxChPortManagerEventSet(devNum, eag6LPortlist[i], &portEventStc);
#else
			CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
			cpssDxChPortManagerStatusGet(devNum, eag6LPortlist[i], &portConfigOutParams);
			ret = cpssDxChPortRemoteFaultSet(devNum, eag6LPortlist[i], 
						   portConfigOutParams.ifMode,
						   portConfigOutParams.speed,
						   GT_FALSE);
			
#endif
			if(ret == GT_OK)
				svcPortFaultFsm[i].fwdPortBmp &= ~(1 << i);
#ifdef DEBUG
			syslog(LOG_INFO, "port=%d, ret=%d, fwdPortBmp=%x", eag6LPortlist[i], 
			       ret, svcPortFaultFsm[i].fwdPortBmp);
#endif

		}
	}

	return rc;
}

SVC_FAULT_ST portEventNoLFRFstate
(
 uint16_t port
)
{
	port = port;
	return SVC_FAULT_ST_NO_LFRF;
}


SVC_FAULT_ST portEventFwdLFstate
(
 uint16_t port
)
{
	port = port;
	return SVC_FAULT_ST_FWD_LF;
}

SVC_FAULT_ST portEventFwdRFstate
(
 uint16_t port
)
{

	port = port;
	return SVC_FAULT_ST_FWD_RF;
}

void initFaultFsmList (void)
{
	int8_t i;
	for(i = 0; i < eag6LPortArrSize; i++)
	{
		svcPortFaultFsm[i].state = SVC_FAULT_ST_INIT;
		svcPortFaultFsm[i].evt = SVC_FAULT_EVT_LINK_UP;
		svcPortFaultFsm[i].fwdPortBmp = 0;
		svcPortFaultFsm[i].retry_cnt = 0;

		svcPortFaultFsm[i].cb[SVC_FAULT_ST_INIT] = faultFsmInit;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_LFRF_CHECK] = checkLFSFState;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_LF] = portEventLFstate;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_FWD_LF] = portEventFwdLFstate;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_RF] = portEventRFstate;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_FWD_RF] = portEventFwdRFstate;
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_CLR_LF] = portEventRFCLRstate; 
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_CLR_RF] = portEventRFCLRstate; 
		svcPortFaultFsm[i].cb[SVC_FAULT_ST_NO_LFRF] = portEventNoLFRFstate; 
	}
}
