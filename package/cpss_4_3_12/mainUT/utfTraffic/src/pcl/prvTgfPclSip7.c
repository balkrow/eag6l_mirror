/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfPclSip7.c
*
* @brief Sip7 PCL specific features
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclSip7.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*******************************************************************************
* prvTgfSip7TcamClientsDbHandleDelete
*
* DESCRIPTION:
*       Delete  TCAM Clents DB.
*
* RETURNS: GT_OK
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfSip7TcamClientsDbHandleDelete
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        return GT_NOT_INITIALIZED;
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        rc = tgfTcamClientsConfigurationDbDelete(tcamClientsDbHandle);
        prvTgfTcamClientCommomHandlersHandleSet(0, NULL);
    }
    return rc;
}

/*******************************************************************************
* prvTgfSip7TcamClientsDbHandleCreate
*
* DESCRIPTION:
*       Create  TCAM Clents DB.
*
* @param[in] devNum   - (pointer to)Global TCAM Clients configuration.
*                       NULL means to use default Sample configuration.
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfSip7TcamClientsDbHandleCreate
(
    IN const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr
)
{
    GT_STATUS rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle == NULL)
    {
        if (appCfgPtr == (const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC*)NULL)
        {
            appCfgPtr = tgfTcamClientsSamplePclConfigurationGet();
        }
        rc = tgfTcamClientsConfigurationDbCreate(
            appCfgPtr,
            &tcamClientsDbHandle);
        prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    }
    return rc;
}

/*******************************************************************************
* prvTgfSip7TcamClientsDbHandlePrint
*
* DESCRIPTION:
*       Print  TCAM Clents DB.
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfSip7TcamClientsDbHandlePrint
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *dbPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        return GT_NOT_INITIALIZED;
    }

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        dbPtr = (const PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC*)tcamClientsDbHandle;
        rc = prvTgfTcamClientsInternalDbDump(&(dbPtr->db));
    }

    return rc;
}

/*******************************************************************************
* prvTgfSip7TcamClientsDevInit
*
* DESCRIPTION:
*
* @param[in] devNum   - device numer
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvTgfSip7TcamClientsDevInit
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() != GT_FALSE)
    {
        if (prvTgfTcamClientCommomHandlersDeviceConfiguredGet(0, devNum))
        {
            return GT_OK;
        }
    }

    rc = prvTgfSip7TcamClientsDbHandleCreate(
        (const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC*)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    rc = tgfTcamClientsDeviceConfigure(devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }

    return rc;
}

/**
* @internal prvTgfSip7TcamClientsGenericDevInit function
* @endinternal
*
* @brief   Configure device using PCL configuration built by caller.
*
* @param[in] devNum          - deviceNumber
* @param[in] appCfgPtr       - (pointer to) pcl configuration
*
* @retval GT_OK on success, other om Fail
*/
GT_STATUS prvTgfSip7TcamClientsGenericDevInit
(
    IN   GT_U8                                        devNum,
    IN   const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC  *appCfgPtr
)
{
    GT_STATUS                                      rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE                     tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        prvTgfTcamClientCommomHandlersHandleSet(0, NULL);
        /* all devices cleared from bitmap */
    }
    rc = tgfTcamClientsConfigurationDbCreate(
        appCfgPtr,
        &tcamClientsDbHandle);
    if (rc != GT_OK)
    {
        return rc;
    }
    prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    rc = tgfTcamClientsDeviceConfigure(
        devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }
    return rc;
}

/**
* @internal prvTgfSip7TcamClientsOneLookupDevInit function
* @endinternal
*
* @brief   Configure device for One Lookup PCL configuration.
*
* @param[in] devNum          - deviceNumber
* @param[in] pclLookup       - PCL lookup
* @param[in] tcamKeySize     - TCAM Key Size
* @param[in] subFieldArrPtr  - (pointer to)array of subfields included in TCAM Key
*
* @retval GT_OK on success, other om Fail
*/
GT_STATUS prvTgfSip7TcamClientsOneLookupDevInit
(
    IN   GT_U8                                    devNum,
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT          pclLookup,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT             tcamKeySize,
    IN   TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *subFieldArrPtr
)
{
    GT_STATUS                                      rc = GT_OK;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE                     tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        prvTgfTcamClientCommomHandlersHandleSet(0, NULL);
        /* all devices cleared from bitmap */
    }
    appCfgPtr = tgfTcamClientsOneLookupPclConfigurationBuild(
        pclLookup, tcamKeySize, subFieldArrPtr);
    if (appCfgPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    rc = tgfTcamClientsConfigurationDbCreate(
        appCfgPtr,
        &tcamClientsDbHandle);
    if (rc != GT_OK)
    {
        return rc;
    }
    prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    rc = tgfTcamClientsDeviceConfigure(
        devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }
    return rc;
}

/**
* @internal tgfTcamClientsIngressAndEgressPclConfigurationBuild function
* @endinternal
*
* @brief   Build One Ingress (or Midway) Lookup and Egress Lookup PCL configuration.
*
* @param[in] ipclLookup               - ingress PCL lookup
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved IPCL UDBs
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved EPCL UDBs
* @param[in] ipclTcamKeySize          - IPCL TCAM Key Size
* @param[in] epclTcamKeySize          - EPCL TCAM Key Size
* @param[in] ipclSubFieldArrPtr       - (pointer to)array of subfields included in IPCL TCAM Key
* @param[in] epclSubFieldArrPtr       - (pointer to)array of subfields included in EPCL TCAM Key
*
* @retval GT_OK on success, other om Fail
*/
GT_STATUS prvTgfSip7TcamClientsIngressAndEgressPclLookupsDevInit
(
    IN   GT_U8                                         devNum,
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT               ipclLookup,
    IN   GT_U32                                        ipclReservedUdbBmpArr[],
    IN   GT_U32                                        epclReservedUdbBmpArr[],
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  ipclTcamKeySize,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  epclTcamKeySize,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *ipclSubFieldArrPtr,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *epclSubFieldArrPtr
)
{
    GT_STATUS                                      rc = GT_OK;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE                     tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        prvTgfTcamClientCommomHandlersHandleSet(0, NULL);
        /* all devices cleared from bitmap */
    }
    appCfgPtr =  tgfTcamClientsIngressAndEgressPclConfigurationBuild(
        ipclLookup, ipclReservedUdbBmpArr, epclReservedUdbBmpArr,
        ipclTcamKeySize, epclTcamKeySize,
        ipclSubFieldArrPtr, epclSubFieldArrPtr);
    if (appCfgPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    rc = tgfTcamClientsConfigurationDbCreate(
        appCfgPtr,
        &tcamClientsDbHandle);
    if (rc != GT_OK)
    {
        return rc;
    }
    prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    rc = tgfTcamClientsDeviceConfigure(
        devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }
    return rc;
}

/************************************************************************************************************/

/**
* @internal prvTgfPclPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclPortVlanFdbSet
(
    IN GT_U16 srcVid,
    IN GT_U32 srcPort,
    IN GT_U16 dstVid,
    IN GT_U32 dstPort,
    IN TGF_MAC_ADDR macAddr
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in Passanger with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        srcVid, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    rc = prvTgfBrgVlanTagMemberAdd(
        srcVid, srcPort, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgVlanTagMemberAdd");

    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        dstVid, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    rc = prvTgfBrgVlanTagMemberAdd(
        dstVid, dstPort, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgVlanTagMemberAdd");

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        macAddr, dstVid /*vlanId*/,
        prvTgfDevNum, dstPort, GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfSPclPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclPortVlanFdbReset
(
    IN GT_U16 srcVid,
    IN GT_U16 dstVid
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Passanger */
    rc = prvTgfBrgDefVlanEntryInvalidate(srcVid);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(dstVid);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

static GT_U32 getRuleIndexBase(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;
    GT_U32  indexBase;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        indexBase = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(0); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        indexBase =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,0);
    }
    return indexBase;
}

/**
* @internal prvTgfPclTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] dstPort                  - destination port
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
GT_VOID prvTgfPclTrafficEgressVidCheck
(
    IN GT_U32  dstPort,
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = dstPort;

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVid & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);
    }

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclTrafficGetCapturedAndTrace function
* @endinternal
*
* @brief   Get Captured packet and traces it
*
* @param[in] dstPort                  - destination port
*                                       None
*/
GT_STATUS prvTgfPclTrafficGetCapturedAndTrace
(
    IN GT_U32  dstPort
)
{
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          numTriggersBmp = 0;
    TGF_VFD_INFO_STC                vfdArray[1];

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = dstPort;

    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;

    /* readd any packets got by CPU from the given port */
    tgfTrafficGeneratorPortTxEthTriggerCountersGet(
        &portInterface, 1 /*numVfd*/, vfdArray, &numTriggersBmp);
    return numTriggersBmp ? GT_OK : GT_NO_MORE;

}

/**
* @internal prvTgfSip7PclGenericTest function
* @endinternal
*
* @brief   Generic Test for SIP7 PCL
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] pclCfgProfileId          - pcl Cfg Profile Id (EM profole ID)
* @param[in] pclId                    - pcl Id
* @param[in] udbPacketTypeGroupIndex  - 0 - not IP, 1- IPV4, 2 - IPV6
* @param[in] ruleRelativeIndex        - rule index relative of the base of TCAM client group
* @param[in] subFieldConditionArrPtr  - List od per-field conditions of the rule
* @param[in] srcVid                   - source VLAN ID
* @param[in] srcPort                  - source port
* @param[in] dstVid                   - destination VLAN ID
* @param[in] dstPort                  - destination port
* @param[in] macAddr                  - MAC address to learn FDB on destination port
* @param[in] packetPtr                - send packet info
*/
GT_VOID prvTgfSip7PclGenericTest
(
    IN CPSS_PCL_DIRECTION_ENT                          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                      lookupNum,
    IN GT_U32                                          pclCfgProfileId,
    IN GT_U32                                          pclId,
    IN GT_U32                                          udbPacketTypeGroupIndex, /* 0 - not IP, 1- IPV4, 2 - IPV6 */
    IN GT_U32                                          ruleRelativeIndex,
    IN TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    *subFieldConditionArrPtr,
    IN GT_U16                                          srcVid,
    IN GT_U32                                          srcPort,
    IN GT_U16                                          dstVid,
    IN GT_U32                                          dstPort,
    IN TGF_MAC_ADDR                                    macAddr,
    IN TGF_PACKET_STC                                  *packetPtr
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    CPSS_DXCH_PCL_ACTION_STC         action;
    GT_U32                           commonKeyHeaderMask;
    GT_U32                           commonKeyHeaderPattern;
    GT_U32                           ruleIndexBase;
    GT_U32                           ruleIndex;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32                           portNum;

    prvTgfSip7TcamClientsDevInit(prvTgfDevNum);

    commonKeyHeaderMask    = 0x3FF;
    commonKeyHeaderPattern = pclId;
    switch (direction)
    {
        default:
        case CPSS_PCL_DIRECTION_INGRESS_E:
            portNum = srcPort;
            switch (lookupNum)
            {
                default:
                case CPSS_PCL_LOOKUP_0_E:
                case CPSS_PCL_LOOKUP_0_0_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                    break;
                case CPSS_PCL_LOOKUP_0_1_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E;
                    break;
                case CPSS_PCL_LOOKUP_1_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E;
                    break;
            }
            break;
        case CPSS_PCL_DIRECTION_MIDWAY_E:
            portNum = srcPort;
            pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E;
            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            portNum = dstPort;
            pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E;
            break;
    }

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = direction;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.vlan.egress.vlanCmd =
            CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.egress.vlanId = dstVid;
    }
    else
    {
        action.vlan.ingress.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.ingress.vlanId = dstVid;
    }

    ruleIndexBase = getRuleIndexBase(direction, lookupNum);
    ruleIndex = ruleRelativeIndex + ruleIndexBase;

    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));
    numOfBytesInBuff = sizeof(packetBuffer); /* maximal space to use */

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclPortVlanFdbSet(
        srcVid, srcPort, dstVid, dstPort, macAddr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclDefPortInitSip7(
        portNum, direction, lookupNum,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E, 0/*cfgIndex*/,
        GT_TRUE /*enableLookup*/, pclId, pclCfgProfileId);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = tgfTcamClientsPclRuleSet(
        prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
        pclCfgProfileId,
        udbPacketTypeGroupIndex, pclLookup,
        0 /*tcamSubkeyIndex*/,
        ruleIndex, GT_TRUE /*writeValidRule*/,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArrPtr, &action);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, srcPort,
        packetPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum, dstPort,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclTrafficEgressVidCheck(
        dstPort, dstVid, GT_TRUE/*check 16 bit*/);

    label_restore:

    prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
        ruleIndex, GT_FALSE /*valid*/);

    prvTgfPclDefPortInitSip7(
        portNum, direction, lookupNum,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E, 0/*cfgIndex*/,
        GT_FALSE /*enableLookup*/, pclId, pclCfgProfileId);

    prvTgfSip7TcamClientsDbHandleDelete();

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclPortVlanFdbReset(
        srcVid, dstVid);
}

/***************************** TESTS **********************************/

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             12

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0xAA

/* original VLAN Id */
#define PRV_TGF_SRC_VLAN_ID_CNS              0x10

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x11

/* lookup0 VLAN Id */
#define PRV_TGF_SRC_MID_VLAN_ID_CNS          0x12

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,   /* etherType */
    0, 0, PRV_TGF_SRC_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthertypePart =
{
    0x3456
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketEthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static TGF_PACKET_STC prvTgfPacketEthInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                 /* totalLen */
    sizeof(prvTgfPacketEthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthPartsArray                                        /* partsArray */
};

/**
* @internal prvTgfSip7PclMpclMacDaSaTest function
* @endinternal
*
* @brief   MPCL Test matching MAC_DA and MAC_SA
*
*/
GT_VOID prvTgfSip7PclMpclMacDaSaTest
(
    GT_VOID
)
{
    TGF_PACKET_L2_STC *l2Ptr = (TGF_PACKET_L2_STC *)(prvTgfPacketEthInfo.partsArray[0].partPtr);
    GT_U8 *macDa = l2Ptr->daMac;
    GT_U8 *macSa = l2Ptr->saMac;
    /* the patterns updated from packet info */
    TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    subFieldConditionArr[] =
    {
        /*{{fieldId, bitOffset, bitLength}, mask, pattern}*/
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E,  0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E, 32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,   0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,  32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };

    subFieldConditionArr[0].pattern =
        (macDa[5] & 0xFF) | ((macDa[4] & 0xFF) << 8) |
        ((macDa[3] & 0xFF) << 16) | ((macDa[2] & 0xFF) << 24);
    subFieldConditionArr[1].pattern =
        (macDa[1] & 0xFF) | ((macDa[0] & 0xFF) << 8);
    subFieldConditionArr[2].pattern =
        (macSa[5] & 0xFF) | ((macSa[4] & 0xFF) << 8) |
        ((macSa[3] & 0xFF) << 16) | ((macSa[2] & 0xFF) << 24);
    subFieldConditionArr[3].pattern =
        (macSa[1] & 0xFF) | ((macSa[0] & 0xFF) << 8);

    prvTgfSip7PclGenericTest(
        CPSS_PCL_DIRECTION_MIDWAY_E,
        CPSS_PCL_LOOKUP_0_E,
        1 /*pclCfgProfileId*/,
        0xAA/*pclId*/,
        0 /*udbPacketTypeGroupIndex*/, /* 0 - not IP, 1- IPV4, 2 - IPV6 */
        PRV_TGF_PCL_RULE_IDX_CNS /*ruleRelativeIndex*/,
        subFieldConditionArr,
        PRV_TGF_SRC_VLAN_ID_CNS /*srcVid*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] /*srcPort*/,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*dstVid*/,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] /*dstPort*/,
        macDa/*macAddr*/,
        &prvTgfPacketEthInfo/*packetPtr*/
    );
}

static TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    prvTgfSip7PclAppCfg;

/**
* @internal prvTgfSip7PclMpclMacDaSaTest function
* @endinternal
*
* @brief   IPCL,MPCL,EPCL Tests matching MAC_DA and MAC_SA and UDB58-59
*
*/
GT_VOID prvTgfSip7PclUdbAppFieldTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *sampleCfgPtr;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC fieldsInfo;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT            udbs58_59_fieldId;
    TGF_PACKET_L2_STC *l2Ptr = (TGF_PACKET_L2_STC *)(prvTgfPacketEthInfo.partsArray[0].partPtr);
    GT_U8 *macDa = l2Ptr->daMac;
    GT_U8 *macSa = l2Ptr->saMac;
    GT_U16 ethType = prvTgfPacketEthertypePart.etherType;
    /* the patterns updated from packet info */
    static TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    ingressSubFieldConditionArr[] =
    {
        /*{{fieldId, bitOffset, bitLength}, mask, pattern}*/
        {{0,                                              0, 16}, 0x0000FFFF, 0}, /* type filled in run time */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E,  0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E, 32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,   0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,  32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };

    /* the patterns updated from packet info */
    static TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    egressSubFieldConditionArr[] =
    {
        /*{{fieldId, bitOffset, bitLength}, mask, pattern}*/
        {{0,                                              0, 16}, 0x0000FFFF, 0}, /* type filled in run time */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E,   0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E,  32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,   0, 32}, 0xFFFFFFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,  32, 16}, 0x0000FFFF, 0},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };

    /* Superkey fields */
    static TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC ingressSuperkeyFieldsUseArr[] =
    {
        {{0                                                   , 0, 16}, {7, 7, 7, 7}}, /* field type dynamic */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E, 0,  8}, {7, 7, 7, 7}},
        /* SIP and DIP always included in suprkey */
        /* DST_MAC included in IPCL and MPCL suprkey */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, {7, 7, 7, 7}},
        /* end of list stamp */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, {0,0,0,0}}
    };

    static TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC egressSuperkeyFieldsUseArr[] =
    {
        {{0                                                   , 0, 16}, 7}, /* field type dynamic */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E, 0,  8}, 7},
        /* SIP and DIP always included in suprkey */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 7},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E, 0, 48}, 7},
        /* end of list stamp */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
    };

    /* Muxing lines fields*/
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC ingressUdbSubFieldsUseArr[] =
    {
        {{0                                                   , 0, 16}, 0x3F}, /* field type dynamic */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E, 0,  8}, 0x3F},
        /* SIP and DIP always included in suprkey */
        /* DST_MAC included in IPCL and MPCL suprkey */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 0x3F},
        /* end of list stamp */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0x3F}
    };

    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC egressUdbSubFieldsUseArr[] =
    {
        {{0                                                   , 0, 16}, 0x3F}, /* field type dynamic */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E, 0,  8}, 0x3F},
        /* SIP and DIP always included in suprkey */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 0x3F},
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E, 0, 48}, 0x3F},
        /* end of list stamp */
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
    };

    static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC muxingTableSubfieldsCfgArr[] =
    {
        /* subKeyMuxTableLineIndex, pktTypeGroupIndex, pclLookup,  */
        /* muxWordWithCommonHeaderIndexesBmp, muxedSubfieldEntryArrPtr */
        {1, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 1, ingressUdbSubFieldsUseArr},
        {2, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 1, ingressUdbSubFieldsUseArr},
        {3, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,  1, egressUdbSubFieldsUseArr},
        {4, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,  1, egressUdbSubFieldsUseArr},
        {5, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,  1, ingressUdbSubFieldsUseArr},
        {6, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,  1, ingressUdbSubFieldsUseArr},
        /*END-OF-LIST Stamp*/
        {0xFF, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 0, NULL}
    };

    sampleCfgPtr = tgfTcamClientsSamplePclConfigurationGet();
    if (sampleCfgPtr == NULL)
    {
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(0, (sampleCfgPtr == NULL));
        return;
    }

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }
    udbs58_59_fieldId =
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E;
    ingressSuperkeyFieldsUseArr[0].subField.fieldId  = udbs58_59_fieldId;
    egressSuperkeyFieldsUseArr[0].subField.fieldId   = udbs58_59_fieldId;
    ingressSubFieldConditionArr[0].subField.fieldId  = udbs58_59_fieldId;
    egressSubFieldConditionArr[0].subField.fieldId   = udbs58_59_fieldId;

    fieldsInfo.fieldType = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_APP_E;
    /* for UDB_APP fields anchor not relevant */
    fieldsInfo.udbAnchor = CPSS_DXCH_PCL_OFFSET_INVALID_E;
    /* for UDB_APP fields offset and length measured in bytes */
    fieldsInfo.bitOffset = 58;
    fieldsInfo.bitLength = 2;

    rc = tgfTcamClientsPclFieldRegister(
        udbs58_59_fieldId, &fieldsInfo);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    cpssOsMemCpy(
        &prvTgfSip7PclAppCfg, sampleCfgPtr, sizeof(prvTgfSip7PclAppCfg));

    /* UDBs sets for using by system for dynamic UDB_CFG field allocation */
    /* UDB 12-13 used by Rule Header (in Sample Configuration)            */
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E][0]  = 0xFFFFCFFF; /*IPCL0 indexes 0-11, 14-31*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E][1]  = 0x03FFFFFF; /*IPCL0 indexes 32-57*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E][0]  = 0xFFFFCFFF; /*IPCL1 indexes 0-11, 14-31*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E][1]  = 0x03FFFFFF; /*IPCL1 indexes 32-57*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E][0]  = 0xFFFFCFFF; /*IPCL2 indexes 0-11, 14-31*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E][1]  = 0x03FFFFFF; /*IPCL2 indexes 32-57*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E][0]   = 0xFFFFCFFF; /*MPCL indexes 10-11, 14-31*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        udbIndexesBmpArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E][1]   = 0x03FFFFFF; /*MPCL indexes 32-57*/
    prvTgfSip7PclAppCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.
        ingressSubFieldsUseArrPtr = ingressSuperkeyFieldsUseArr;

    prvTgfSip7PclAppCfg.egressSuperKeyFieldsSet.udbIndexesBmp[0]  = 0xFFFFCFFF;/*EPCL indexes 0-11, 14-31*/
    prvTgfSip7PclAppCfg.egressSuperKeyFieldsSet.udbIndexesBmp[1]  = 0x03FFFFFF;/*EPCL indexes 32-57*/
    prvTgfSip7PclAppCfg.egressSuperKeyFieldsSet.egressSubFieldsUseArrPtr = egressSuperkeyFieldsUseArr;

    prvTgfSip7PclAppCfg.muxingTableSubfieldsCfgArrPtr = muxingTableSubfieldsCfgArr;

    /* Application registered field UDBs configuration */

    rc = cpssDxChPclUserDefinedByteSet(
        prvTgfDevNum, 0/*ruleFormat*/,
        CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        58 /*udbIndex*/, CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E, 0/*offset*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    rc = cpssDxChPclUserDefinedByteSet(
        prvTgfDevNum, 0/*ruleFormat*/,
        CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_INGRESS_E,
        59 /*udbIndex*/, CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E, 1/*offset*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    rc = cpssDxChPclUserDefinedByteSet(
        prvTgfDevNum, 0/*ruleFormat*/,
        CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_EGRESS_E,
        58 /*udbIndex*/, CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E, 0/*offset*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    rc = cpssDxChPclUserDefinedByteSet(
        prvTgfDevNum, 0/*ruleFormat*/,
        CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_PCL_DIRECTION_EGRESS_E,
        59 /*udbIndex*/, CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E, 1/*offset*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);


    rc = prvTgfSip7TcamClientsDbHandleCreate(&prvTgfSip7PclAppCfg);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    ingressSubFieldConditionArr[0].pattern = ethType;
    ingressSubFieldConditionArr[1].pattern =
        (macDa[5] & 0xFF) | ((macDa[4] & 0xFF) << 8) |
        ((macDa[3] & 0xFF) << 16) | ((macDa[2] & 0xFF) << 24);
    ingressSubFieldConditionArr[2].pattern =
        (macDa[1] & 0xFF) | ((macDa[0] & 0xFF) << 8);
    ingressSubFieldConditionArr[3].pattern =
        (macSa[5] & 0xFF) | ((macSa[4] & 0xFF) << 8) |
        ((macSa[3] & 0xFF) << 16) | ((macSa[2] & 0xFF) << 24);
    ingressSubFieldConditionArr[4].pattern =
        (macSa[1] & 0xFF) | ((macSa[0] & 0xFF) << 8);

    egressSubFieldConditionArr[0].pattern = ethType;
    egressSubFieldConditionArr[1].pattern =
        (macDa[5] & 0xFF) | ((macDa[4] & 0xFF) << 8) |
        ((macDa[3] & 0xFF) << 16) | ((macDa[2] & 0xFF) << 24);
    egressSubFieldConditionArr[2].pattern =
        (macDa[1] & 0xFF) | ((macDa[0] & 0xFF) << 8);
    egressSubFieldConditionArr[3].pattern =
        (macSa[5] & 0xFF) | ((macSa[4] & 0xFF) << 8) |
        ((macSa[3] & 0xFF) << 16) | ((macSa[2] & 0xFF) << 24);
    egressSubFieldConditionArr[4].pattern =
        (macSa[1] & 0xFF) | ((macSa[0] & 0xFF) << 8);


    rc = prvTgfSip7TcamClientsDbHandleCreate(&prvTgfSip7PclAppCfg);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    prvTgfSip7PclGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1 /*pclCfgProfileId*/,
        0xAA/*pclId*/,
        0 /*udbPacketTypeGroupIndex*/, /* 0 - not IP, 1- IPV4, 2 - IPV6 */
        PRV_TGF_PCL_RULE_IDX_CNS /*ruleRelativeIndex*/,
        ingressSubFieldConditionArr,
        PRV_TGF_SRC_VLAN_ID_CNS /*srcVid*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] /*srcPort*/,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*dstVid*/,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] /*dstPort*/,
        macDa/*macAddr*/,
        &prvTgfPacketEthInfo/*packetPtr*/
    );

    rc = prvTgfSip7TcamClientsDbHandleCreate(&prvTgfSip7PclAppCfg);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    prvTgfSip7PclGenericTest(
        CPSS_PCL_DIRECTION_MIDWAY_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1 /*pclCfgProfileId*/,
        0xAA/*pclId*/,
        0 /*udbPacketTypeGroupIndex*/, /* 0 - not IP, 1- IPV4, 2 - IPV6 */
        PRV_TGF_PCL_RULE_IDX_CNS /*ruleRelativeIndex*/,
        ingressSubFieldConditionArr,
        PRV_TGF_SRC_VLAN_ID_CNS /*srcVid*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] /*srcPort*/,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*dstVid*/,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] /*dstPort*/,
        macDa/*macAddr*/,
        &prvTgfPacketEthInfo/*packetPtr*/
    );

    rc = prvTgfSip7TcamClientsDbHandleCreate(&prvTgfSip7PclAppCfg);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    prvTgfSip7PclGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1 /*pclCfgProfileId*/,
        0xAA/*pclId*/,
        0 /*udbPacketTypeGroupIndex*/, /* 0 - not IP, 1- IPV4, 2 - IPV6 */
        PRV_TGF_PCL_RULE_IDX_CNS /*ruleRelativeIndex*/,
        egressSubFieldConditionArr,
        PRV_TGF_SRC_VLAN_ID_CNS /*srcVid*/,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] /*srcPort*/,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*dstVid*/,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] /*dstPort*/,
        macDa/*macAddr*/,
        &prvTgfPacketEthInfo/*packetPtr*/
    );
}

/**
* @internal prvTgfSip7PclCfgEntryIndexSetTest function
* @endinternal
*
* @brief   Test for Action setting of Configuration entry index of the next lookup.
*
*/
GT_VOID prvTgfSip7PclCfgEntryIndexSetTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    CPSS_DXCH_PCL_ACTION_STC         action;
    GT_U32                           commonKeyHeaderMask;
    GT_U32                           commonKeyHeaderPattern;
    GT_U32                           ruleIndexBase;
    GT_U32                           ruleIndex;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32                           portNum;
    GT_U32                           lookupIdx;
    GT_U32                           udbPacketTypeGroupIndex = 0; /* 0 - not IP, 1- IPV4, 2 - IPV6 */
    GT_U16                           srcVid = PRV_TGF_SRC_VLAN_ID_CNS;
    GT_U32                           srcPort =  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U16                           dstVid = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    GT_U32                           dstPort = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    TGF_PACKET_STC                   *packetPtr = &prvTgfPacketEthInfo;
    GT_U8                            *macAddr = ((TGF_PACKET_L2_STC *)(packetPtr->partsArray[0].partPtr))->daMac;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;

    /* empty field list */
    TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    subFieldConditionArr[] =
    {
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };
    struct
    {
        CPSS_PCL_DIRECTION_ENT     direction;
        CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum;
        GT_U32                     pclId;
        GT_U32                     pclCfgProfileId;
        GT_U32                     cfgTabIndex;
        GT_U32                    relRuleIndex;
    } cfgPerLookupArr[] =
    {
        {
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_0_E,
            1, 1, 0xFFFFFFFF, 6
        },
        {
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_1_E,
            2, 1, 2, 12
        },
        /* IPCL lookup2 ommited */
        {
            CPSS_PCL_DIRECTION_MIDWAY_E,
            CPSS_PCL_LOOKUP_0_0_E,
            4, 1, 4, 18
        },
        {
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_0_E,
            5, 1, 5, 24
        }
    };
    GT_U32 cfgPerLookupArrSize = sizeof(cfgPerLookupArr) / sizeof(cfgPerLookupArr[0]);

    prvTgfSip7TcamClientsDevInit(prvTgfDevNum);

    for (lookupIdx = 0; (lookupIdx < cfgPerLookupArrSize); lookupIdx++)
    {
        portNum =
            (cfgPerLookupArr[lookupIdx].direction != CPSS_PCL_DIRECTION_EGRESS_E)
                ? srcPort : dstPort;
        accessMode = (lookupIdx == 0)
            ? PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E
            : PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_DONT_CARE_E;
        rc = prvTgfPclDefPortInitSip7(
            portNum, cfgPerLookupArr[lookupIdx].direction,
            cfgPerLookupArr[lookupIdx].lookupNum, accessMode,
            cfgPerLookupArr[lookupIdx].cfgTabIndex/*cfgIndex*/,
            GT_TRUE /*enableLookup*/, cfgPerLookupArr[lookupIdx].pclId,
            cfgPerLookupArr[lookupIdx].pclCfgProfileId);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
        if (rc != GT_OK) goto label_restore;
    }

    for (lookupIdx = 0; (lookupIdx < cfgPerLookupArrSize); lookupIdx++)
    {
        /* action */
        cpssOsMemSet(&action, 0, sizeof(action));
        action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        action.egressPolicy = cfgPerLookupArr[lookupIdx].direction;
        pclLookup = tgfTcamClientsPclDirLookupToClientLookup(
            cfgPerLookupArr[lookupIdx].direction,
            cfgPerLookupArr[lookupIdx].lookupNum);
        if ((lookupIdx + 1) < cfgPerLookupArrSize)
        {
            action.lookupConfig.ipclConfigIndex = cfgPerLookupArr[lookupIdx + 1].cfgTabIndex;
            switch (pclLookup)
            {
                case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E:
                    action.lookupConfig.pcl0_1OverrideConfigIndex =
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                    break;
                case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E:
                    action.lookupConfig.mpclOverrideConfigIndex =
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                    break;
                case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
                    action.lookupConfig.epclOverrideConfigIndex =
                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
                    break;
                /* not tested */
                case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E:
                case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
                default:
                    break;
            }
        }
        else
        {
            if (action.egressPolicy == CPSS_PCL_DIRECTION_EGRESS_E)
            {
                action.vlan.egress.vlanCmd =
                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
                action.vlan.egress.vlanId = dstVid;
            }
            else
            {
                action.vlan.ingress.modifyVlan =
                    CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
                action.vlan.ingress.vlanId = dstVid;
            }
        }

        /* rule */
        commonKeyHeaderMask    = 0x3FF;
        commonKeyHeaderPattern = cfgPerLookupArr[lookupIdx].pclId;
        ruleIndexBase = getRuleIndexBase(
            cfgPerLookupArr[lookupIdx].direction, cfgPerLookupArr[lookupIdx].lookupNum);
        ruleIndex = ruleIndexBase + cfgPerLookupArr[lookupIdx].relRuleIndex;
        rc = tgfTcamClientsPclRuleSet(
            prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
            cfgPerLookupArr[lookupIdx].pclCfgProfileId,
            udbPacketTypeGroupIndex, pclLookup,
            0 /*tcamSubkeyIndex*/,
            ruleIndex, GT_TRUE /*writeValidRule*/,
            &commonKeyHeaderMask, &commonKeyHeaderPattern,
            subFieldConditionArr, &action);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
        if (rc != GT_OK) goto label_restore;
    }

    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));
    numOfBytesInBuff = sizeof(packetBuffer); /* maximal space to use */

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclPortVlanFdbSet(
        srcVid, srcPort, dstVid, dstPort, macAddr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, srcPort,
        packetPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum, dstPort,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclTrafficEgressVidCheck(
        dstPort, dstVid, GT_TRUE/*check 16 bit*/);

    label_restore:

    for (lookupIdx = 0; (lookupIdx < cfgPerLookupArrSize); lookupIdx++)
    {
        ruleIndexBase = getRuleIndexBase(
            cfgPerLookupArr[lookupIdx].direction, cfgPerLookupArr[lookupIdx].lookupNum);
        ruleIndex = ruleIndexBase + cfgPerLookupArr[lookupIdx].relRuleIndex;
        prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
            ruleIndex, GT_FALSE /*valid*/);
    }

    for (lookupIdx = 0; (lookupIdx < cfgPerLookupArrSize); lookupIdx++)
    {
        portNum =
            (cfgPerLookupArr[lookupIdx].direction != CPSS_PCL_DIRECTION_EGRESS_E)
                ? srcPort : dstPort;
        accessMode = (lookupIdx == 0)
            ? PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E
            : PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_DONT_CARE_E;
        pclLookup = tgfTcamClientsPclDirLookupToClientLookup(
            cfgPerLookupArr[lookupIdx].direction,
            cfgPerLookupArr[lookupIdx].lookupNum);
        rc = prvTgfPclDefPortInitSip7(
            portNum, cfgPerLookupArr[lookupIdx].direction,
            cfgPerLookupArr[lookupIdx].lookupNum, accessMode,
            cfgPerLookupArr[lookupIdx].cfgTabIndex/*cfgIndex*/,
            GT_FALSE /*enableLookup*/, 0/*pclId*/, 0/*pclCfgProfileId*/);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    }

    prvTgfSip7TcamClientsDbHandleDelete();

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclPortVlanFdbReset(
        srcVid, dstVid);
}


/**
* @internal prvTgfSip7PclActionStopTestStep function
* @endinternal
*
* @brief   Test for Action Stop testing lookup.
*
*/
GT_VOID prvTgfSip7PclActionStopTestStep
(
    IN CPSS_PCL_DIRECTION_ENT     direction0,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum0,
    IN GT_BOOL                    actionStop,
    IN CPSS_PCL_DIRECTION_ENT     direction1,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum1
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    CPSS_DXCH_PCL_ACTION_STC         action;
    GT_U32                           commonKeyHeaderMask;
    GT_U32                           commonKeyHeaderPattern;
    GT_U32                           ruleIndexBase;
    GT_U32                           ruleIndex;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32                           portNum;
    GT_U32                           udbPacketTypeGroupIndex = 0; /* 0 - not IP, 1- IPV4, 2 - IPV6 */
    GT_U16                           srcVid = PRV_TGF_SRC_VLAN_ID_CNS;
    GT_U32                           srcPort =  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U16                           dstVid = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    GT_U16                           midVid = PRV_TGF_SRC_MID_VLAN_ID_CNS;
    GT_U32                           dstPort = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    TGF_PACKET_STC                   *packetPtr = &prvTgfPacketEthInfo;
    GT_U8                            *macAddr = ((TGF_PACKET_L2_STC *)(packetPtr->partsArray[0].partPtr))->daMac;
    /* empty field list */
    TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    subFieldConditionArr[] =
    {
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };

    prvTgfSip7TcamClientsDevInit(prvTgfDevNum);

    portNum =
        (direction0 != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction0,
        lookupNum0, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_TRUE /*enableLookup*/, 1/*pclId*/, 1/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    portNum =
        (direction1 != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction1,
        lookupNum1, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_TRUE /*enableLookup*/, 2/*pclId*/, 1/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* action */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = direction0;
    pclLookup = tgfTcamClientsPclDirLookupToClientLookup(
        direction0, lookupNum0);
    action.actionStop = actionStop;
    if (action.egressPolicy == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.vlan.egress.vlanCmd =
            CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.egress.vlanId = midVid;
    }
    else
    {
        action.vlan.ingress.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.ingress.vlanId = midVid;
    }

    /* rule */
    commonKeyHeaderMask    = 0x3FF;
    commonKeyHeaderPattern = 1;
    ruleIndexBase = getRuleIndexBase(direction0, lookupNum0);
    ruleIndex = ruleIndexBase + 6;
    rc = tgfTcamClientsPclRuleSet(
        prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
        1/*pclCfgProfileId*/,
        udbPacketTypeGroupIndex, pclLookup,
        0 /*tcamSubkeyIndex*/,
        ruleIndex, GT_TRUE /*writeValidRule*/,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArr, &action);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* action */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = direction1;
    pclLookup = tgfTcamClientsPclDirLookupToClientLookup(
        direction1, lookupNum1);
    if (action.egressPolicy == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.vlan.egress.vlanCmd =
            CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.egress.vlanId = dstVid;
    }
    else
    {
        action.vlan.ingress.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.ingress.vlanId = dstVid;
    }

    /* rule */
    commonKeyHeaderMask    = 0x3FF;
    commonKeyHeaderPattern = 2;
    ruleIndexBase = getRuleIndexBase(direction1, lookupNum1);
    ruleIndex = ruleIndexBase + 12;
    rc = tgfTcamClientsPclRuleSet(
        prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
        1/*pclCfgProfileId*/,
        udbPacketTypeGroupIndex, pclLookup,
        0 /*tcamSubkeyIndex*/,
        ruleIndex, GT_TRUE /*writeValidRule*/,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArr, &action);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));
    numOfBytesInBuff = sizeof(packetBuffer); /* maximal space to use */

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclPortVlanFdbSet(
        srcVid, srcPort, dstVid, dstPort, macAddr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclPortVlanFdbSet(
        srcVid, srcPort, midVid, dstPort, macAddr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, srcPort,
        packetPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum, dstPort,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    /* AUTODOC: Check that TAG0 VID is modified */
    if (actionStop == GT_FALSE)
    {
        prvTgfPclTrafficEgressVidCheck(
            dstPort, dstVid, GT_TRUE/*check 16 bit*/);
    }
    else
    {
        prvTgfPclTrafficEgressVidCheck(
            dstPort, midVid, GT_TRUE/*check 16 bit*/);
    }

    label_restore:

    ruleIndexBase = getRuleIndexBase(direction0, lookupNum0);
    ruleIndex = ruleIndexBase + 6;
    prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
        ruleIndex, GT_FALSE /*valid*/);

    ruleIndexBase = getRuleIndexBase(direction1, lookupNum1);
    ruleIndex = ruleIndexBase + 12;
    prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
        ruleIndex, GT_FALSE /*valid*/);

    portNum =
        (direction0 != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction0,
        lookupNum0, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_FALSE /*enableLookup*/, 0/*pclId*/, 0/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    portNum =
        (direction1 != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction1,
        lookupNum1, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_FALSE /*enableLookup*/, 0/*pclId*/, 0/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    prvTgfSip7TcamClientsDbHandleDelete();

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclPortVlanFdbReset(srcVid, dstVid);
    prvTgfPclPortVlanFdbReset(srcVid, midVid);
}

/**
* @internal prvTgfSip7PclActionCancelDropStep function
* @endinternal
*
* @brief   Test for Action Cancel Drop of parallel lookup testing.
*
*/
GT_VOID prvTgfSip7PclActionCancelDropStep
(
    IN CPSS_PCL_DIRECTION_ENT     direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum,
    IN GT_BOOL                    hardOrSoftDrop,
    IN GT_BOOL                    cancelDropEnable,
    IN GT_U32                     hitWithCancelDrop,
    IN GT_U32                     hitWithDrop
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    CPSS_DXCH_PCL_ACTION_STC         action;
    GT_U32                           commonKeyHeaderMask;
    GT_U32                           commonKeyHeaderPattern;
    GT_U32                           ruleIndex;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    GT_U32                           portNum;
    GT_U32                           udbPacketTypeGroupIndex = 0; /* 0 - not IP, 1- IPV4, 2 - IPV6 */
    GT_U16                           srcVid = PRV_TGF_SRC_VLAN_ID_CNS;
    GT_U32                           srcPort =  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                           dstPort = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    TGF_PACKET_STC                   *packetPtr = &prvTgfPacketEthInfo;
    GT_U8                            *macAddr = ((TGF_PACKET_L2_STC *)(packetPtr->partsArray[0].partPtr))->daMac;
    CPSS_DXCH_TCAM_CLIENT_ENT        tcamClient;
    GT_U32                           tcamClientGroup;
    GT_BOOL                          tcamClientEnable;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    saveFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    CPSS_DROP_MODE_TYPE_ENT          cancelDropType;
    /* empty field list */
    TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    subFieldConditionArr[] =
    {
        {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0, 0}
    };

    prvTgfSip7TcamClientsDevInit(prvTgfDevNum);

    pclLookup = tgfTcamClientsPclDirLookupToClientLookup(
        direction, lookupNum);
    switch (pclLookup)
    {
        default:
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E:
            tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E:
            tcamClient = CPSS_DXCH_TCAM_IPCL_1_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E:
            tcamClient = CPSS_DXCH_TCAM_IPCL_2_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
            tcamClient = CPSS_DXCH_TCAM_MPCL_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
            tcamClient = CPSS_DXCH_TCAM_EPCL_E;
            break;
    }
    cancelDropType =
        (hardOrSoftDrop == GT_FALSE)
            ? CPSS_DROP_MODE_SOFT_E
            : CPSS_DROP_MODE_HARD_E;

    rc = cpssDxChTcamPortGroupClientGroupGet(
        prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        tcamClient, &tcamClientGroup, &tcamClientEnable);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (tcamClientEnable == GT_FALSE)
    {
        /* map to client group 0 */
        tcamClientGroup = 0;
        tcamClientEnable = GT_TRUE;
        rc = cpssDxChTcamPortGroupClientGroupSet(
            prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            tcamClient, tcamClientGroup, tcamClientEnable);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
        if (rc != GT_OK)
        {
            prvTgfSip7TcamClientsDbHandleDelete();
            return;
        }
    }

    rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(
        prvTgfDevNum, 0/*floorIndex*/, saveFloorInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK)
    {
        prvTgfSip7TcamClientsDbHandleDelete();
        return;
    }

    cpssOsMemSet(floorInfoArr, 0, sizeof(floorInfoArr));
    floorInfoArr[0].group = tcamClientGroup;
    floorInfoArr[0].hitNum = hitWithCancelDrop;
    floorInfoArr[1].group = tcamClientGroup;
    floorInfoArr[1].hitNum = hitWithDrop;
    rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(
        prvTgfDevNum, 0/*floorIndex*/, floorInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = cpssDxChPclParallelLookupDropCancelEnableSet(
        prvTgfDevNum, direction, lookupNum,
        cancelDropType, hitWithCancelDrop, hitWithDrop, cancelDropEnable);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    portNum =
        (direction != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction,
        lookupNum, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_TRUE /*enableLookup*/, 1/*pclId*/, 1/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* action with Cancel Drop */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy = direction;
    action.cancelDrop = GT_TRUE;

    /* rule */
    commonKeyHeaderMask    = 0x3FF;
    commonKeyHeaderPattern = 1;
    ruleIndex = 0;
    rc = tgfTcamClientsPclRuleSet(
        prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
        1/*pclCfgProfileId*/,
        udbPacketTypeGroupIndex, pclLookup,
        0 /*tcamSubkeyIndex*/,
        ruleIndex, GT_TRUE /*writeValidRule*/,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArr, &action);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* action with  Drop */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy = direction;
    action.pktCmd =
        (hardOrSoftDrop == GT_FALSE)
            ? CPSS_PACKET_CMD_DROP_SOFT_E
            : CPSS_PACKET_CMD_DROP_HARD_E;
    action.mirror.cpuCode = CPSS_NET_USER_DEFINED_0_E;

    /* rule */
    commonKeyHeaderMask    = 0x3FF;
    commonKeyHeaderPattern = 1;
    ruleIndex = 6;
    rc = tgfTcamClientsPclRuleSet(
        prvTgfDevNum, prvTgfTcamClientCommomHandlersHandleGet(0),
        1/*pclCfgProfileId*/,
        udbPacketTypeGroupIndex, pclLookup,
        0 /*tcamSubkeyIndex*/,
        ruleIndex, GT_TRUE /*writeValidRule*/,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArr, &action);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));
    numOfBytesInBuff = sizeof(packetBuffer); /* maximal space to use */

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclPortVlanFdbSet(
        srcVid, srcPort, srcVid/*dstVid*/, dstPort, macAddr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum, srcPort,
        packetPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum, dstPort,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200 /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    rc = prvTgfPclTrafficGetCapturedAndTrace(dstPort);
    if (cancelDropEnable == GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_NO_MORE, rc);
    }
    else
    {
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    }

    /*=====================================================*/
    label_restore:

    ruleIndex = 0;
    prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
        ruleIndex, GT_FALSE /*valid*/);

    ruleIndex = 6;
    prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_60_BYTES_E /*not relevant for SIP5 and above*/,
        ruleIndex, GT_FALSE /*valid*/);

    portNum =
        (direction != CPSS_PCL_DIRECTION_EGRESS_E)
            ? srcPort : dstPort;
    rc = prvTgfPclDefPortInitSip7(
        portNum, direction,
        lookupNum, PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0/*cfgIndex*/, GT_FALSE /*enableLookup*/, 0/*pclId*/, 0/*pclCfgProfileId*/);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    rc = cpssDxChPclParallelLookupDropCancelEnableSet(
        prvTgfDevNum, direction, lookupNum,
        cancelDropType, hitWithCancelDrop, hitWithDrop, GT_FALSE);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(
        prvTgfDevNum, 0/*floorIndex*/, saveFloorInfoArr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);

    prvTgfSip7TcamClientsDbHandleDelete();

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclPortVlanFdbReset(srcVid, srcVid/*dstVid*/);
}
