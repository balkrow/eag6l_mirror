/*******************************************************************************
*              (c), Copyright 2000, HFR Ltd.				       *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file eag6l_vlan.c
*
* 
* 
*
* 
********************************************************************************
*/
#include "syslog.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#define DEBUG

extern GT_VOID * osMemSet
(
    IN GT_VOID * start,
    IN int    symbol,
    IN GT_U32 size
);

extern GT_STATUS cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
);

extern GT_STATUS appDemoDevIdxGet
(
 IN  GT_U8   devNum,
 OUT GT_U32  *devIdxPtr
);

extern APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];

uint8_t eag6L25GPortlist [] =
{
	0,
	8,
	16,
	24,
	32,
	40,
	48,
	49,
};
uint8_t eag6L25GPortArrSize = sizeof(eag6L25GPortlist) / sizeof(uint8_t);

uint8_t eag6LPortlist [] =
{
	0,
	8,
	16,
	24,
	32,
	40,
	48,
	49,
	50
};
uint8_t eag6LPortArrSize = sizeof(eag6LPortlist) / sizeof(uint8_t);
uint16_t eag6L100Gport = 50;

#if 1
uint8_t EAG6LCpeVlanAdd
(
 uint8_t devNum,
 uint8_t portNum,
 uint16_t vid
)
{
	GT_STATUS   rc = GT_OK;
#if 0
	GT_U32              port;           /* current port number      */
#endif
	CPSS_PORTS_BMP_STC  portsMembers;
	CPSS_PORTS_BMP_STC  portsTagging;
	CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
	CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;
	GT_U32                      devIdx; /* index to appDemoPpConfigList */
#if 0
	portNum = portNum;
#endif

	rc = appDemoDevIdxGet(devNum, &devIdx);
	if ( GT_OK != rc )
	{
		return rc;
	}

	rc = cpssSystemRecoveryStateGet(&system_recovery);
	if (rc != GT_OK)
	{
		return rc;
	}

	osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
	/* default IP MC VIDX */
	cpssVlanInfo.unregIpmEVidx = 0xFFF;


	cpssVlanInfo.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregIpv6McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregIpv4McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.floodVidx              = 0xFFF;

	cpssVlanInfo.mirrToRxAnalyzerIndex = 0;
	cpssVlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
	cpssVlanInfo.mirrToTxAnalyzerIndex = 0;
	cpssVlanInfo.fidValue = vid;
	cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
	cpssVlanInfo.ipv4McBcMirrToAnalyzerIndex = 0;
	cpssVlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;
	cpssVlanInfo.ipv6McMirrToAnalyzerIndex = 0;

	CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
	CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
	osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
#if 1/*test by balkrow*/
	/* set all ports as VLAN members */
	if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
	{
#if 1
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, portNum);
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 50);
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 51);
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 52);
		CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, 53);
		rc = cpssDxChBrgVlanPortVidSet(devNum, portNum, CPSS_DIRECTION_INGRESS_E,vid);
		rc = cpssDxChBrgVlanPortVidSet(devNum, 50, CPSS_DIRECTION_INGRESS_E,vid);
		rc = cpssDxChBrgVlanPortVidSet(devNum, 51, CPSS_DIRECTION_INGRESS_E,vid);
		rc = cpssDxChBrgVlanPortVidSet(devNum, 52, CPSS_DIRECTION_INGRESS_E,vid);
		rc = cpssDxChBrgVlanPortVidSet(devNum, 53, CPSS_DIRECTION_INGRESS_E,vid);
		CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
		if(rc != GT_OK)
			return rc;
		portsTaggingCmd.portsCmd[portNum] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
#else
		for (port = 0; port < (appDemoPpConfigList[devIdx].maxPortNumber); port++)
		{
			CPSS_ENABLER_PORT_SKIP_CHECK(devNum,port);

			/* set the port as member of vlan */
			CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);

			/* Set port pvid */
			rc = cpssDxChBrgVlanPortVidSet(devNum, port, CPSS_DIRECTION_INGRESS_E,vid);
			CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortVidSet", rc);
			if(rc != GT_OK)
				return rc;

			portsTaggingCmd.portsCmd[port] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
		}
#endif

		/* Write default VLAN entry */
		rc = cpssDxChBrgVlanEntryWrite(devNum, vid,
					       &portsMembers,
					       &portsTagging,
					       &cpssVlanInfo,
					       &portsTaggingCmd);

		CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanEntryWrite", rc);
		if (rc != GT_OK)
			return rc;
	}
	else
	{
		if( system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E &&
		    appDemoSysConfig.forceAutoLearn == GT_FALSE /*controlled learning*/)
		{
			/* the MI set the vlan as 'auto-learn' and 'no msg to cpu' */
			/* so we need to modify only those 2 params and not other values in the vlan */
			rc = cpssDxChBrgVlanNaToCpuEnable( devNum , vid , GT_TRUE);
			if (rc != GT_OK)
				return rc;
			rc = cpssDxChBrgVlanLearningStateSet( devNum ,vid , GT_FALSE);
			if (rc != GT_OK)
				return rc;
		}
	}
#else
	CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, portNum);
	ret = cpssDxChBrgVlanMemberAdd(devNum, vid, portNum, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);

	CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, eag6L100Gport);
	ret = cpssDxChBrgVlanMemberAdd(devNum, vid, eag6L100Gport, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);

        portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        ret = cpssDxChBrgVlanEntryWrite(0, vid,
                                       &portsMembers,
                                       &portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);
#ifdef DEBUG
	syslog(LOG_NOTICE, "cpssDxChBrgVlanEntryWrite=%x", ret);
#endif
#endif
	return rc;
}
#endif
uint8_t EAG6LFecInit (void)
{
	uint8_t rc = 0, i;
	uint8_t devNum = 0x0;

	for(i = 0; i < eag6L25GPortArrSize; i++)
	{
		rc = cpssDxChPortFecModeSet(devNum, eag6L25GPortlist[i], CPSS_PORT_RS_FEC_MODE_ENABLED_E);
	}
	return rc;
}

uint8_t EAG6LVlanInit (void)
{
	uint8_t rc = 0;
	int8_t i;
	uint8_t devNum = 0x0;
	GT_BOOL isDefaultProfile = GT_TRUE;
	uint32_t profile = 0x1, tpidBmp = 0;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6LPortlist[i] , CPSS_VLAN_ETHERTYPE0_E, isDefaultProfile, profile);
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6LPortlist[i] , CPSS_VLAN_ETHERTYPE1_E, isDefaultProfile, profile);
		rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, CPSS_VLAN_ETHERTYPE0_E, tpidBmp);
		rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, CPSS_VLAN_ETHERTYPE1_E, tpidBmp);
	}
	return  rc;
}

#if 0
uint32_t EAG6LVlanTestCaseCfg 
( 
 uint8_t devNum,
 uint8_t portNum
) 
{
	uint8_t i, vid = 10;		
	uint32_t ret;
	portNum = portNum;

	CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType = CPSS_PORT_ACCEPT_FRAME_ALL_E;
	ret = cpssDxChBrgVlanPortAccFrameTypeSet(devNum, 0, frameType);
	ret = cpssDxChBrgVlanPortAccFrameTypeSet(devNum, 8, frameType);

	for(i = 0; i < eag6l25GPortArrSize; i++)
	{
		EAG6LCpeVlanAdd(devNum, eag6L25GPortlist[i], vid++);
	}
#ifdef DEBUG
	syslog(LOG_NOTICE, "cpssDxChBrgVlanPortAccFrameTypeSet=%x", ret);
#endif

	return ret;
}
#endif
/**
 * * @internal ut_vlan_config_case1 function
 * * @endinternal
 * *
 * * @brief   Implements unit test case1 for vlan config
 * *
 * * @param[in] dev                      -   device number
 * *
 * * @retval GT_OK                    - on success
 * * @retval GT_FAIL                  - otherwise
 * 
 * */
#if 0
uint8_t ut_vlan_config_case1
(
)
{
}
#endif
