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
#if 1/*[#52] 25G to 100G forwarding 기능 추가, balkrow, 2024-06-12*/
#include <stdbool.h>
#include "eag6l_fsm.h"
#include <gtOs/gtEnvDep.h>
#endif
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
#include "sys_fifo.h"
#endif

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

#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
extern GT_STATUS cpssDxChPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 mruSize
);

extern GT_STATUS cpssDxChBrgVlanMruProfileValueSet
(
    IN GT_U8     devNum,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
);

extern GT_STATUS cpssDxChBrgVlanMruProfileIdxSet
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
);
#endif

#if 1/*[#52] 25G to 100G forwarding 기능 추가, balkrow, 2024-06-12*/
extern GT_STATUS cpssDxChBrgFdbPortLearnStatusSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  status,
    IN CPSS_PORT_LOCK_CMD_ENT   cmd
);

extern GT_STATUS cpssDxChBrgVlanLearningStateSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  status
);

extern GT_STATUS cpssDxChBrgPrvEdgeVlanEnable
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

extern GT_STATUS cpssDxChBrgPrvEdgeVlanPortEnable
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable,
    IN GT_PORT_NUM      dstPort,
    IN GT_HW_DEV_NUM    dstHwDev,
    IN GT_BOOL          dstTrunk
);
#endif
extern APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];

uint8_t eag6L25GPortlist [] =
{
	0,
	8,
	16,
	24,
	32,
	40,
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
#warning "----- MVDEMO board ------------"
	48,
	49,
#endif
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
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
#warning "----- MVDEMO board ------------"
	48,
	49,
#endif
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

#if 1/*[#52] 25G to 100G forwarding 기능 추가, balkrow, 2024-06-12*/
uint8_t EAG6LMacLearningnable (void)
{
	uint8_t rc = 0, i;
	GT_U8 devNum = 0x0;
	GT_BOOL status = true;
	GT_U16 vlanId = 1;

	CPSS_PORT_LOCK_CMD_ENT cmd = CPSS_LOCK_FRWRD_E;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		rc += cpssDxChBrgFdbPortLearnStatusSet(devNum, eag6LPortlist[i], status, cmd);
	}
	rc += cpssDxChBrgVlanLearningStateSet(devNum, vlanId, status);
	return rc;
}

uint8_t EAG6L25Gto100GFwdSet (void)
{
	uint8_t rc = 0, i;
	GT_U8 devNum = 0x0;
	GT_BOOL enable = true, trunk = false;
	/*
	GT_HW_DEV_NUM dstHwDev = 16;	
	*/
	GT_HW_DEV_NUM dstHwDev;
	rc = cpssDxChCfgHwDevNumGet(0, &dstHwDev);
	syslog(LOG_NOTICE, "cpssDxChCfgHwDevNumGet=%x", dstHwDev);

	rc = cpssDxChBrgPrvEdgeVlanEnable(devNum, enable); 
	for(i = 0; i < eag6L25GPortArrSize; i++)
	{	
		rc += cpssDxChBrgPrvEdgeVlanPortEnable(devNum, eag6L25GPortlist[i], enable,
						 EAG6L_WDM_PORT, dstHwDev, trunk);
						 
	}
	return rc;
}

#endif

#if 1/*[#45] Jumbo frame 기능 추가, balkrow, 2024-06-10*/
uint8_t EAG6LJumboFrameEnable (void)
{
	uint8_t rc = 0, i;
	GT_U8 devNum = 0x0;
#if 1/*[228] switch TX ref clock 외부 diff clk 만을 보도록 수정, balkrow, 2024-12-16*/
	GT_U32 mruSize = 0x2800, mruIndex = 0;
	rc += cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex, mruSize);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID1, mruIndex);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID2, mruIndex);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID3, mruIndex);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID4, mruIndex);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID5, mruIndex);
	rc += cpssDxChBrgVlanMruProfileIdxSet(devNum, SW_TP_MODE_VID6, mruIndex);
#endif

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		 rc += cpssDxChPortMruSet(devNum, eag6LPortlist[i], mruSize);
	}
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

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
uint8_t EAG6LVlanInit (uint8_t mode)
{
	uint8_t rc = 0;
	int8_t i;
	uint8_t devNum = 0x0;
	GT_BOOL isDefaultProfile = GT_TRUE;
	uint32_t profile = 0x1, tpidBmp = 0;

	/* init for 25g ports. */
	for(i = 0; i < eag6L25GPortArrSize; i++)
	{
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6L25GPortlist[i], 
				CPSS_VLAN_ETHERTYPE0_E, isDefaultProfile, profile);
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6L25GPortlist[i], 
				CPSS_VLAN_ETHERTYPE1_E, isDefaultProfile, profile);
	}

	/* init for 100g port for aggregation/transparent switch mode. */
	if(mode == SW_AGGREGATION_MODE) {
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, EAG6L_WDM_PORT, 
			CPSS_VLAN_ETHERTYPE0_E, isDefaultProfile, profile);
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, EAG6L_WDM_PORT,
			CPSS_VLAN_ETHERTYPE1_E, isDefaultProfile, profile);
	} else { /* SW_TRANSPARENT_MODE */
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, EAG6L_WDM_PORT, 
			CPSS_VLAN_ETHERTYPE0_E, isDefaultProfile, 0x0/*reset*/);
		rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, EAG6L_WDM_PORT,
			CPSS_VLAN_ETHERTYPE1_E, isDefaultProfile, 0x0/*reset*/);
	}

	/* init vlan ingress tpid profile. */
	rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, 
			CPSS_VLAN_ETHERTYPE0_E, tpidBmp);
	rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, 
			CPSS_VLAN_ETHERTYPE1_E, tpidBmp);
	return  rc;
}
#else /********************************************************************/
uint8_t EAG6LVlanInit (void)
{
	uint8_t rc = 0;
	int8_t i;
	uint8_t devNum = 0x0;
	GT_BOOL isDefaultProfile = GT_TRUE;
	uint32_t profile = 0x1, tpidBmp = 0;

	for(i = 0; i < eag6LPortArrSize; i++)
	{
		rc += cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6LPortlist[i] , CPSS_VLAN_ETHERTYPE0_E, isDefaultProfile, profile);
		rc += cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, eag6LPortlist[i] , CPSS_VLAN_ETHERTYPE1_E, isDefaultProfile, profile);
		rc += cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, CPSS_VLAN_ETHERTYPE0_E, tpidBmp);
		rc += cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, CPSS_VLAN_ETHERTYPE1_E, tpidBmp);
	}
	return  rc;
}
#endif /* [#142] */

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

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
uint8_t EAG6LEmptyVlanAdd (GT_U16 vlanId)
{
	GT_STATUS   rc = GT_OK;
 	GT_U8       devNum = 0;
    GT_BOOL     status = true;
	CPSS_PORTS_BMP_STC  portsMembers;
	CPSS_PORTS_BMP_STC  portsTagging;
	CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */

	CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
	CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

	osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
	osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

	cpssVlanInfo.unregIpmEVidx          = 0xFFF;
	cpssVlanInfo.floodVidx              = 0xFFF;
	cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
	cpssVlanInfo.autoLearnDisable       = true;
	cpssVlanInfo.naMsgToCpuEn           = true;

	/* add vlan. */
	rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers, &portsTagging, 
			&cpssVlanInfo, &portsTaggingCmd);
	if (rc != GT_OK) {
		syslog(LOG_NOTICE, "cpssDxChBrgVlanEntryWrite=%x", rc);
		return rc;
	}

	/* enable learning for vlan. */
	rc = cpssDxChBrgVlanLearningStateSet(devNum, vlanId, status);
	if (rc != GT_OK) {
		syslog(LOG_NOTICE, "cpssDxChBrgVlanLearningStateSet=%x", rc);
		return rc;
	}

	return rc;
}

uint8_t EAG6LVlanRemove (GT_U16 vlanId)
{
	GT_STATUS   rc = GT_OK;
	GT_U8       devNum = 0;
    GT_BOOL     isValid;
	CPSS_PORTS_BMP_STC  portsMembers;
	CPSS_PORTS_BMP_STC  portsTagging;
	CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */

	/* get target vlan. */
	rc = cpssDxChBrgVlanEntryRead(devNum, vlanId, &portsMembers, &portsTagging, 
			&cpssVlanInfo, &isValid, &portsTaggingCmd);
	if (rc == GT_OK) {
		/* remove vlan. */
		rc = cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);
		if (rc != GT_OK) {
			syslog(LOG_NOTICE, "cpssDxChBrgVlanEntryInvalidate=%x", rc);
			return rc;
		}
	}
	return rc;
}

uint8_t EAG6LTransparentVlanAdd (GT_U16 vlanId, GT_U32 portNum)
{
	GT_STATUS   rc = GT_OK;
 	GT_U8       devNum = 0;
    GT_BOOL     isValid;
	CPSS_PORTS_BMP_STC  portsMembers;
	CPSS_PORTS_BMP_STC  portsTagging;
	CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */

	rc = cpssDxChBrgVlanEntryRead(devNum, vlanId, &portsMembers, &portsTagging, 
			&cpssVlanInfo, &isValid, &portsTaggingCmd);
	if (rc != GT_OK) {
		syslog(LOG_NOTICE, "cpssDxChBrgVlanEntryRead=%x", rc);
		return rc;
	}


	CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, portNum);
	CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging, portNum);

	cpssVlanInfo.unregIpmEVidx          = 0xFFF;
	cpssVlanInfo.floodVidx              = 0xFFF;
	cpssVlanInfo.fdbLookupKeyMode       = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;
	cpssVlanInfo.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.floodVidxMode          = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
	cpssVlanInfo.portIsolationMode      = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
	cpssVlanInfo.unregIpmEVidxMode      = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;
	cpssVlanInfo.ipv4IpmBrgMode         = CPSS_BRG_IPM_SGV_E;
	cpssVlanInfo.ipv6IpmBrgMode         = CPSS_BRG_IPM_SGV_E;
	cpssVlanInfo.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unknownMacSaCmd        = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregIpv4McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.unregIpv6McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
	cpssVlanInfo.ipv6SiteIdMode         = CPSS_IP_SITE_ID_INTERNAL_E;
	cpssVlanInfo.ipCtrlToCpuEn          = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
	cpssVlanInfo.mirrToTxAnalyzerMode   = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;
	cpssVlanInfo.naMsgToCpuEn           = true;
	cpssVlanInfo.fidValue               = vlanId;

	portsTaggingCmd.portsCmd[portNum]   = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
	if(portNum == 50)
		portsTaggingCmd.portsCmd[50]    = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;

	/* add vlan. */
	rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers, &portsTagging, 
			&cpssVlanInfo, &portsTaggingCmd);
	if (rc != GT_OK) {
		syslog(LOG_NOTICE, "cpssDxChBrgVlanEntryWrite=%x", rc);
		return rc;
	}

	if(portNum != 50) {
		/* set port pvid. */
		rc = cpssDxChBrgVlanPortVidSet(devNum, portNum, CPSS_DIRECTION_INGRESS_E, vlanId);
		if (rc != GT_OK) {
			syslog(LOG_NOTICE, "cpssDxChBrgVlanPortVidSet=%x", rc);
			return rc;
		}
	}
	return rc;
}

uint8_t EAG6LSwitchModeSet(uint8_t enable)
{
	uint8_t result = 0;
	uint8_t ii;

	syslog(LOG_NOTICE, "EAG6LSwitchModeSet = %s", 
		enable ? "Transparent" : "Aggregation");
	if(enable) {
		result += EAG6LVlanInit(SW_TRANSPARENT_MODE);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID1);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID1, 0/*port1*/);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID2);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID2, 8/*port2*/);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID3);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID3, 16/*port3*/);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID4);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID4, 24/*port4*/);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID5);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID5, 32/*port5*/);

		result += EAG6LEmptyVlanAdd(SW_TP_MODE_VID6);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID6, 40/*port6*/);

		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID1, 50/*port7-100g*/);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID2, 50/*port7-100g*/);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID3, 50/*port7-100g*/);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID4, 50/*port7-100g*/);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID5, 50/*port7-100g*/);
		result += EAG6LTransparentVlanAdd(SW_TP_MODE_VID6, 50/*port7-100g*/);
	} else {
		result += EAG6LVlanRemove(SW_TP_MODE_VID1);
		result += EAG6LVlanRemove(SW_TP_MODE_VID2);
		result += EAG6LVlanRemove(SW_TP_MODE_VID3);
		result += EAG6LVlanRemove(SW_TP_MODE_VID4);
		result += EAG6LVlanRemove(SW_TP_MODE_VID5);
		result += EAG6LVlanRemove(SW_TP_MODE_VID6);

		/* reset port pvid. */
		for(ii = 0; ii < eag6L25GPortArrSize; ii++) {
			result += cpssDxChBrgVlanPortVidSet(0/*devNum*/, eag6L25GPortlist[ii], 
					CPSS_DIRECTION_INGRESS_E, 1/*vlanId*/);
		}

		result += EAG6L25Gto100GFwdSet();
		result += EAG6LVlanInit(SW_AGGREGATION_MODE);
	}
	return result;
}
#endif /* [#142] */

