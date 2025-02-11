/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoLion2FabricForBc2.c
*
* @brief Application demo of using Lion2 device as Switch Fabric connecting
* Bobcat2 devices.
*
* @version   3
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoLion2FabricForBc2.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* macro ORs the given field to data byte */
#define PRV_U8_OR_SUBFLD_MAC(_byte, _bitHigh, _bitLow, _value, _valStartBit) \
    _byte |= (GT_U8)(((_value >> _valStartBit) & (0xFF >> (7 + _bitLow - _bitHigh))) << _bitLow)

/* PCL rules use ingress standard "UDB" Key      */
/* UDBs 0-15 assumed configured to L2+12 - L2+27 */
/* to cover all 16-byte eDSA tag of packet       */
/* rule.ruleIngrStdUdb.udb0_15 should be used as */
/* _arr macro parameter                          */

/**
* @internal prvLion2FabricUdbsConfigure function
* @endinternal
*
* @brief   The function configures UDBs 0-15 to L2+12 - L2+27
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       GT_OK on success, other on fail
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricUdbsConfigure
(
    IN    GT_U8                      devNum,
    IN    GT_BOOL                    enable
)
{
    GT_STATUS rc;
    GT_U32    i;
    GT_U32                               udbIndex;
    GT_U32                               udbOffset;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        udbOffsetType;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetTypeArr[] =
    {
        CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
        CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
        CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E,
        CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E,
        CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
        CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        CPSS_DXCH_PCL_PACKET_TYPE_UDE_E,
        CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E
    };

    GT_U32 packetTypeAmount = (sizeof(packetTypeArr) / sizeof(packetTypeArr[0]));

    udbOffsetType =
        (enable == GT_FALSE)
            ? CPSS_DXCH_PCL_OFFSET_INVALID_E
            : CPSS_DXCH_PCL_OFFSET_L2_E;

    for (i = 0; (i < packetTypeAmount); i++)
    {
        for (udbIndex = 0; (udbIndex < 16); udbIndex++)
        {
            udbOffset = (enable == GT_FALSE) ? 0 : (12 + udbIndex);

            rc = cpssDxChPclUserDefinedByteSet(
                devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
                packetTypeArr[i], CPSS_PCL_DIRECTION_INGRESS_E,
                udbIndex, udbOffsetType, (GT_U8)udbOffset);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvLion2FabricPclEnable function
* @endinternal
*
* @brief   The function enables Ingress PCL on all specified ingress ports
*
* @param[in] devNum                   - device number
* @param[in] portsBmpPtr              - (pointer to)all used ports bitmap
*                                       GT_OK on success, other on fail
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricPclEnable
(
    IN    GT_U8                      devNum,
    IN    CPSS_PORTS_BMP_STC         *portsBmpPtr
)
{
    GT_STATUS                               rc;
    GT_PORT_NUM                             portNum;
    CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC   accMode =
    {
        CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,
        CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,
        CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E,
        CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,
        CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,
        CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E
    };

    rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPclCfgTblAccessModeSet(
        devNum, &accMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (portNum = 0; (portNum < CPSS_MAX_PORTS_NUM_CNS); portNum++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum) == 0)
            continue;

        rc = cpssDxChPclPortIngressPolicyEnable(
            devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvLion2FabricPclLookupBind function
* @endinternal
*
* @brief   The function binds Ingress PCL Loopup to given PCL Id on all specified ingress ports
*
* @param[in] devNum                   - device number
* @param[in] portsBmpPtr              - (pointer to)all used ports bitmap
* @param[in] lookupNum                - Lookup number
* @param[in] pclId                    - pcl Id
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       GT_OK on success, other on fail
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricPclLookupBind
(
    IN    GT_U8                      devNum,
    IN    CPSS_PORTS_BMP_STC         *portsBmpPtr,
    IN    CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum,
    IN    GT_U32                     pclId,
    IN    GT_BOOL                    enable

)
{
    GT_STATUS                        rc;
    GT_PORT_NUM                      portNum;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC     lookupCfg;

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    /* port assignment for each port below */

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = enable;
    lookupCfg.pclId = pclId;
    lookupCfg.groupKeyTypes.nonIpKey =
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;

    for (portNum = 0; (portNum < CPSS_MAX_PORTS_NUM_CNS); portNum++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum) == 0)
            continue;

        if (enable != GT_FALSE)
        {
            rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
                devNum, portNum, CPSS_PCL_DIRECTION_INGRESS_E,
                lookupNum, 0/*subLookupNum*/,
                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        interfaceInfo.devPort.portNum = portNum;

        rc = cpssDxChPclCfgTblSet(
            devNum, &interfaceInfo,
            CPSS_PCL_DIRECTION_INGRESS_E,
            lookupNum, &lookupCfg);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvLion2FabricPvidForce function
* @endinternal
*
* @brief   The function forces VID==1 fpr all packets
*
* @param[in] devNum                   - device number
* @param[in] portsBmpPtr              - (pointer to)all used ports bitmap
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       GT_OK on success, other on fail
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricPvidForce
(
    IN    GT_U8                      devNum,
    IN    CPSS_PORTS_BMP_STC         *portsBmpPtr,
    IN    GT_BOOL                    enable
)
{
    GT_STATUS                               rc;
    GT_PORT_NUM                             portNum;

    for (portNum = 0; (portNum < CPSS_MAX_PORTS_NUM_CNS); portNum++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum) == 0)
            continue;

        rc = cpssDxChBrgVlanForcePvidEnable(
            devNum, portNum, enable);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (enable != GT_FALSE)
        {
            rc = cpssDxChBrgVlanPortVidSet(
                devNum, portNum, CPSS_DIRECTION_INGRESS_E, 1/*vlanId*/);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/* Bridge lookup(0) support */
/* CoS lookup(1) support    */

/* Fields used for parsing eDSA Tags:                           */
/*        Packet Command:             Word0[31:30]   L2+12[7:6] */
/*        eVLAN[07:00]                Word0[07:00]   L2+15[7:0] */
/*        eVLAN[11:08]                Word0[11:08]   L2+14[3:0] */
/*        eVLAN[15:12]                Word3[30:27]   L2+24[6:3] */
/*        FORWARD_useVIDX[0]          Word1[12]      L2+18[4]   */
/*        FORWARD_eVIDX[07:00]        Word1[07:00]   L2+19[7:0] */
/*        FORWARD_eVIDX[11:08]        Word1[11:08]   L2+18[3:0] */
/*        FORWARD_eVIDX[15:12]        Word3[23:20]   L2+25[7:4] */
/*        FORWARD_TrgDev[04:00]       Word1[04:00]   L2+19[4:0] */
/*        FORWARD_TrgDev[11:05]       Word3[06:00]   L2+27[6:0] */
/*        FROM_CPU_useVIDX[0]         Word0[18]      L2+13[2]   */
/*        FROM_CPU_eVIDX[04:00]       Word0[23:19]   L2+14[7:3] */
/*        FROM_CPU_eVIDX[09:05]       Word0[28:24]   L2+13[4:0] */
/*        FROM_CPU_eVIDX[11:10]       Word1[13:12]   L2+18[5:4] */
/*        FROM_CPU_eVIDX[15:12]       Word3[23:20]   L2+25[7:4] */
/*        FROM_CPU_TrgDev[04:00]      Word0[28:24]   L2+12[4:0] */
/*        FROM_CPU_TrgDev[11:05]      Word3[06:00]   L2+27[6:0] */
/*        TO_ANALYZER_useVIDX[0]      Word1[26]      L2+16[2]   */
/*        TO_ANALYZER_eVIDX[00:00]    Word3[07:07]   L2+27[7:7] */
/*        TO_ANALYZER_eVIDX[08:01]    Word3[15:08]   L2+26[7:0] */
/*        TO_ANALYZER_eVIDX[15:09]    Word3[22:16]   L2+25[6:0] */
/*        TO_ANALYZER_TrgDev[01:00]   Word1[15:14]   L2+18[7:6] */
/*        TO_ANALYZER_TrgDev[09:02]   Word1[23:16]   L2+17[7:0] */
/*        TO_ANALYZER_TrgDev[11:10]   Word1[25:24]   L2+16[1:0] */
/*        FORWARD_DP[1:0]             Word1[14:13]   L2+18[6:5] */
/*        FORWARD_TC[0:0]             Word1[15:15]   L2+18[7:7] */
/*        FORWARD_TC[2:1]             Word1[17:16]   L2+17[1:0] */
/*        FROM_CPU_TC[0:0]            Word0[17:17]   L2+13[1:1] */
/*        FROM_CPU_TC[1:1]            Word1[14:14]   L2+18[6:6] */
/*        FROM_CPU_TC[2:2]            Word1[27:27]   L2+16[3:3] */

/* FORWARD_TC and FORWARD_DP needs BC2 configuration below:           */
/* <HA_IP> Header Alteration (HA_IP)/HA Physical Port Table 2/HA Physical Port Table2 Entry */
/* bit 55 DSA QoS Mode (default qosProfile) needed - TC/DP            */
/* The BC2 backward compatible mode generates 7-bit Qos Profile index */
/* assumed as global in the system, the referred configuration causes */
/* such 7 bits: 0,0,TC[2:0],DP[1:0]                                   */


#define PRV_eDSA_FLD_PACKET_CMD_SET_MAC(_arr, _hwVal) \
    PRV_U8_OR_SUBFLD_MAC(_arr[0], 7, 6, _hwVal, 0)

#define PRV_eDSA_FLD_eVID_SET_MAC(_arr, _val)       \
    PRV_U8_OR_SUBFLD_MAC(_arr[3],  7, 0, _val, 0);  \
    PRV_U8_OR_SUBFLD_MAC(_arr[2],  3, 0, _val, 8);  \
    PRV_U8_OR_SUBFLD_MAC(_arr[12], 6, 3, _val, 12)

#define PRV_eDSA_FLD_FORWARD_useVIDX_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[6], 4, 4, _val, 0)

#define PRV_eDSA_FLD_FORWARD_eVIDX_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[7],  7, 0, _val, 0);     \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  3, 0, _val, 8);     \
    PRV_U8_OR_SUBFLD_MAC(_arr[13], 7, 4, _val, 12)

#define PRV_eDSA_FLD_FORWARD_TrgDev_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[7],  4, 0, _val, 0);      \
    PRV_U8_OR_SUBFLD_MAC(_arr[15], 6, 0, _val, 5);

#define PRV_eDSA_FLD_FROM_CPU_useVIDX_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[1], 2, 2, _val, 0)

#define PRV_eDSA_FLD_FROM_CPU_eVIDX_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[1],  7, 3, _val, 0);      \
    PRV_U8_OR_SUBFLD_MAC(_arr[0],  4, 0, _val, 5);      \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  5, 4, _val, 10);     \
    PRV_U8_OR_SUBFLD_MAC(_arr[13], 7, 4, _val, 12)

#define PRV_eDSA_FLD_FROM_CPU_TrgDev_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[0],  4, 0, _val, 0);       \
    PRV_U8_OR_SUBFLD_MAC(_arr[15], 6, 0, _val, 5);

#define PRV_eDSA_FLD_TO_ANALYZER_useVIDX_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[4], 2, 2, _val, 0)

#define PRV_eDSA_FLD_TO_ANALYZER_eVIDX_SET_MAC(_arr, _val)  \
    PRV_U8_OR_SUBFLD_MAC(_arr[15],  7, 7, _val, 0);         \
    PRV_U8_OR_SUBFLD_MAC(_arr[14],  7, 0, _val, 1);         \
    PRV_U8_OR_SUBFLD_MAC(_arr[13],  6, 0, _val, 9)

#define PRV_eDSA_FLD_TO_ANALYZER_TrgDev_SET_MAC(_arr, _val)  \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  7, 6, _val, 0);           \
    PRV_U8_OR_SUBFLD_MAC(_arr[5],  7, 0, _val, 2);           \
    PRV_U8_OR_SUBFLD_MAC(_arr[4],  1, 0, _val, 10)

#define PRV_eDSA_FLD_FORWARD_DP_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  6, 5, _val, 0);

#define PRV_eDSA_FLD_FORWARD_TC_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  7, 7, _val, 0);  \
    PRV_U8_OR_SUBFLD_MAC(_arr[5],  1, 0, _val, 1)

#define PRV_eDSA_FLD_FROM_CPU_TC_SET_MAC(_arr, _val) \
    PRV_U8_OR_SUBFLD_MAC(_arr[1],  1, 1, _val, 0);   \
    PRV_U8_OR_SUBFLD_MAC(_arr[6],  6, 6, _val, 1);   \
    PRV_U8_OR_SUBFLD_MAC(_arr[4],  3, 3, _val, 2)

/**
* @internal prvLion2FabricActionToPort function
* @endinternal
*
* @brief   The function prepares the Action Structure for redirection to port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] actionPtr                - memory to fulfill
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricActionToPort
(
    IN    GT_U8                      devNum,
    IN    GT_PORT_NUM                portNum,
    OUT   CPSS_DXCH_PCL_ACTION_STC   *actionPtr
)
{
    cpssOsMemSet(actionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
    actionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe = GT_TRUE;
    actionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    actionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    actionPtr->redirect.data.outIf.outInterface.devPort.portNum = portNum;
}

/**
* @internal prvLion2FabricActionToVidx function
* @endinternal
*
* @brief   The function prepares the Action Structure for redirection to VIDX
*
* @param[in] vidx                     - VIDX id
*
* @param[out] actionPtr                - memory to fulfill
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricActionToVidx
(
    IN    GT_U32                     vidx,
    OUT   CPSS_DXCH_PCL_ACTION_STC   *actionPtr
)
{
    cpssOsMemSet(actionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
    actionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe = GT_TRUE;
    actionPtr->redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    actionPtr->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VIDX_E;
    actionPtr->redirect.data.outIf.outInterface.vidx = vidx;
}

/**
* @internal prvLion2FabricActionQosProfileAssign function
* @endinternal
*
* @brief   The function prepares the Action Structure for Qos Profile Assignment
*
* @param[in] qosProfile               - QoS Profile
*
* @param[out] actionPtr                - memory to fulfill
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricActionQosProfileAssign
(
    IN    GT_U32                     qosProfile,
    OUT   CPSS_DXCH_PCL_ACTION_STC   *actionPtr
)
{
    cpssOsMemSet(actionPtr, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
    actionPtr->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->qos.ingress.profilePrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    actionPtr->qos.ingress.profileAssignIndex = GT_TRUE;
    actionPtr->qos.ingress.profileIndex = qosProfile;
}

/**
* @internal prvLion2FabricRulePclIdOnly function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule wit PCL id only
*
* @param[in] pclId                    - pcl Id
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRulePclIdOnly
(
    IN  GT_U32                                           pclId,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    cpssOsMemSet(maskPtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(patternPtr, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));

    maskPtr->ruleIngrStdUdb.commonIngrUdb.pclId    = 0x3FF;
    patternPtr->ruleIngrStdUdb.commonIngrUdb.pclId = pclId;
}

/**
* @internal prvLion2FabricRulePclIdAndPktCmd function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule catching PCL id and Packet Command
*
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRulePclIdAndPktCmd
(
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    prvLion2FabricRulePclIdOnly(pclId, maskPtr, patternPtr);

    udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_PACKET_CMD_SET_MAC(udbArr, 3);
    udbArr = patternPtr->ruleIngrStdUdb.udb0_15;

    switch (packetCommand)
    {
        case CPSS_DXCH_NET_DSA_CMD_TO_CPU_E:
            PRV_eDSA_FLD_PACKET_CMD_SET_MAC(udbArr, 0/*TO_CPU*/);
            break;
        case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
            PRV_eDSA_FLD_PACKET_CMD_SET_MAC(udbArr, 1/*FROM_CPU*/);
            break;
        case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
            PRV_eDSA_FLD_PACKET_CMD_SET_MAC(udbArr, 2/*TO_ANALYZER*/);
            break;
        case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
            PRV_eDSA_FLD_PACKET_CMD_SET_MAC(udbArr, 3/*FORWARD*/);
            break;
        default:
            break;
    }
}

/**
* @internal prvLion2FabricRuleToCpu function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule catching To CPU packets
*
* @param[in] pclId                    - pcl Id
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleToCpu
(
    IN  GT_U32                                           pclId,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    prvLion2FabricRulePclIdAndPktCmd(
        pclId, CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, maskPtr, patternPtr);
}

/**
* @internal prvLion2FabricRuleToDevice function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule to one Device
*
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] hwDevId                  - hw Dev Id
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleToDevice
(
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           hwDevId,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    prvLion2FabricRulePclIdAndPktCmd(
        pclId, packetCommand, maskPtr, patternPtr);

    switch (packetCommand)
    {
        case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FROM_CPU_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FROM_CPU_TrgDev_SET_MAC(udbArr, 0xFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FROM_CPU_useVIDX_SET_MAC(udbArr, 0);
            PRV_eDSA_FLD_FROM_CPU_TrgDev_SET_MAC(udbArr, hwDevId);
            break;
        case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_TO_ANALYZER_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_TO_ANALYZER_TrgDev_SET_MAC(udbArr, 0xFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_TO_ANALYZER_useVIDX_SET_MAC(udbArr, 0);
            PRV_eDSA_FLD_TO_ANALYZER_TrgDev_SET_MAC(udbArr, hwDevId);
            break;
        case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FORWARD_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FORWARD_TrgDev_SET_MAC(udbArr, 0xFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FORWARD_useVIDX_SET_MAC(udbArr, 0);
            PRV_eDSA_FLD_FORWARD_TrgDev_SET_MAC(udbArr, hwDevId);
            break;
        default:
            break;
    }
}

/**
* @internal prvLion2FabricRuleToVidx function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule to VIDX
*
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] eVidx                    -  Id
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleToVidx
(
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           eVidx,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    prvLion2FabricRulePclIdAndPktCmd(
        pclId, packetCommand, maskPtr, patternPtr);

    switch (packetCommand)
    {
        case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FROM_CPU_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FROM_CPU_eVIDX_SET_MAC(udbArr, 0xFFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FROM_CPU_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FROM_CPU_eVIDX_SET_MAC(udbArr, eVidx);
            break;
        case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_TO_ANALYZER_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_TO_ANALYZER_eVIDX_SET_MAC(udbArr, 0xFFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_TO_ANALYZER_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_TO_ANALYZER_eVIDX_SET_MAC(udbArr, eVidx);
            break;
        case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
            udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FORWARD_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FORWARD_eVIDX_SET_MAC(udbArr, 0xFFFF);
            udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
            PRV_eDSA_FLD_FORWARD_useVIDX_SET_MAC(udbArr, 1);
            PRV_eDSA_FLD_FORWARD_eVIDX_SET_MAC(udbArr, eVidx);
            break;
        default:
            break;
    }
}

/**
* @internal prvLion2FabricRuleToVid function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Rule to VID
*
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] eVid                     -  Id
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleToVid
(
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           eVid,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    /* Flooding in current VLAN */
    prvLion2FabricRuleToVidx(
        pclId, packetCommand, 0xFFF /*eVidx*/, maskPtr, patternPtr);

    udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_eVID_SET_MAC(udbArr, 0xFFFF);
    udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_eVID_SET_MAC(udbArr, eVid);
}

/**
* @internal prvLion2FabricRuleQosFwdTcDp function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Qos Rule matching forwarded
*         eDSA tag by TC and DP
* @param[in] pclId                    - pcl Id
* @param[in] tc                       - matched TC
* @param[in] dp                       - matched DP
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleQosFwdTcDp
(
    IN  GT_U32                                           pclId,
    IN  GT_U32                                           tc,
    IN  GT_U32                                           dp,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    prvLion2FabricRulePclIdAndPktCmd(
        pclId, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, maskPtr, patternPtr);

    udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_FORWARD_TC_SET_MAC(udbArr, 7);
    PRV_eDSA_FLD_FORWARD_DP_SET_MAC(udbArr, 3);
    udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_FORWARD_TC_SET_MAC(udbArr, tc);
    PRV_eDSA_FLD_FORWARD_DP_SET_MAC(udbArr, dp);
}

/**
* @internal prvLion2FabricRuleQosFromCpuTc function
* @endinternal
*
* @brief   The function prepares the Mask and pattern for Qos Rule matching From Cpu
*         eDSA tag by TC
* @param[in] pclId                    - pcl Id
* @param[in] tc                       - matched TC
*
* @param[out] maskPtr                  - rule mask.
* @param[out] patternPtr               - rule pattern.
*                                       None
*
* @note NONE
*
*/
static GT_VOID prvLion2FabricRuleQosFromCpuTc
(
    IN  GT_U32                                           pclId,
    IN  GT_U32                                           tc,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                    *patternPtr
)
{
    GT_U8 *udbArr;

    prvLion2FabricRulePclIdAndPktCmd(
        pclId, CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E, maskPtr, patternPtr);

    udbArr = maskPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_FROM_CPU_TC_SET_MAC(udbArr, 7);
    udbArr = patternPtr->ruleIngrStdUdb.udb0_15;
    PRV_eDSA_FLD_FROM_CPU_TC_SET_MAC(udbArr, tc);
}

/**
* @internal prvLion2FabricRuleDefaultWrite function
* @endinternal
*
* @brief   The function writes to TCAM Default Rule for packets to be flooded
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] localVidx                - lion2 local VIDX
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleDefaultWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  GT_U32                                           localVidx
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRulePclIdOnly(
        pclId, &mask, &pattern);
    prvLion2FabricActionToVidx(
        localVidx, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleToCpuWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for packets to Cpu Device
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] portNum                  - lion2 egress port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleToCpuWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  GT_PORT_NUM                                      portNum
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleToCpu(
        pclId, &mask, &pattern);
    prvLion2FabricActionToPort(
        devNum, portNum, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleToDeviceWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for packets to one Device
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] targetDevHwId            - target device HW Dev Id
* @param[in] portNum                  - lion2 egress port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleToDeviceWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           targetDevHwId,
    IN  GT_PORT_NUM                                      portNum
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleToDevice(
        pclId, packetCommand, targetDevHwId, &mask, &pattern);
    prvLion2FabricActionToPort(
        devNum, portNum, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleToVidxWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for packets to VIDX
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] targetVidx               - target VIDX
* @param[in] localVidx                - lion2 local VIDX
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleToVidxWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           targetVidx,
    IN  GT_U32                                           localVidx
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleToVidx(
        pclId, packetCommand, targetVidx, &mask, &pattern);
    prvLion2FabricActionToVidx(
        localVidx, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleToVidWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for packets to VID
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] targetVid                - target VID
* @param[in] localVidx                - lion2 local VIDX
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleToVidWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           targetVid,
    IN  GT_U32                                           localVidx
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleToVid(
        pclId, packetCommand, targetVid, &mask, &pattern);
    prvLion2FabricActionToVidx(
        localVidx, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleQosDefaultWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for default TC/DP assignment
*
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] packetCommand            - Packet Command
* @param[in] qosProfile               - Qos Profile index
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleQosDefaultWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT                        packetCommand,
    IN  GT_U32                                           qosProfile
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRulePclIdAndPktCmd(
        pclId, packetCommand, &mask, &pattern);
    prvLion2FabricActionQosProfileAssign(
        qosProfile, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleQosFwdTcDpWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for Qos Rule matching forwarded
*         eDSA tag by TC and DP
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] tc                       - matched TC
* @param[in] dp                       - matched DP
* @param[in] qosProfile               - Qos Profile index
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleQosFwdTcDpWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  GT_U32                                           tc,
    IN  GT_U32                                           dp,
    IN  GT_U32                                           qosProfile
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleQosFwdTcDp(
        pclId, tc, dp, &mask, &pattern);
    prvLion2FabricActionQosProfileAssign(
        qosProfile, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/**
* @internal prvLion2FabricRuleQosFromCpuTcWrite function
* @endinternal
*
* @brief   The function writes to TCAM Rule for Qos Rule matching From Cpu
*         eDSA tag by TC
* @param[in] devNum                   - lion2 device number
* @param[in] ruleIndex                - TCAM rule index
* @param[in] pclId                    - pcl Id
* @param[in] tc                       - matched TC
* @param[in] qosProfile               - Qos Profile index
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRuleQosFromCpuTcWrite
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           ruleIndex,
    IN  GT_U32                                           pclId,
    IN  GT_U32                                           tc,
    IN  GT_U32                                           qosProfile
)
{
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                    pattern;
    CPSS_DXCH_PCL_ACTION_STC                         action;

    prvLion2FabricRuleQosFromCpuTc(
        pclId, tc, &mask, &pattern);
    prvLion2FabricActionQosProfileAssign(
        qosProfile, &action);

    return cpssDxChPclRuleSet(
        devNum, 0/*tcamIndex*/, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
}

/*================================================================================*/
/* APP DB */
/* there is independant BC2 device numeration used in API */
/* it is defferenrt from CPSS devNum and hwDevNum         */
/* These device numbers are only indexes in APP Device DB */
/* They applied only to BC2 connected to one Lion2 device */
/* Bitmap of such indexes is convinient for specifying    */
/* VID or VIDX device members                             */

#define PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS    0x3FE
#define PRV_LION2_FABRIC_LOOKUP1_PCL_ID_CNS    0x3FD
#define PRV_LION2_FABRIC_START_RULE_INDEX_CNS  0
#define PRV_LION2_FABRIC_START_VIDX_CNS        1

/* calculate QoS profile index for specified TC, DP */
#define PRV_QOS_PROFILE_INDEX_GET_MAC(_qosProfileBase, _tc, _dp) \
    ((_qosProfileBase) + ((_tc) * (CPSS_DP_RED_E + 1)) + (_dp))



typedef struct
{
    CPSS_PORTS_BMP_STC              portsBmp;
    GT_U32                          freeRuleIndex;
    GT_U32                          freeVidxNum;
    GT_U32                          lookup0DefaultRuleIndex;
    GT_U32                          lookup0DefaultRuleVidx;
    GT_U32                          qosProfileBase;
} PRV_LION2_FABRIC_DB_STC, *PRV_LION2_FABRIC_DB_PTR;

static GT_BOOL prvLion2FabricDbInitDone = GT_FALSE;
static PRV_LION2_FABRIC_DB_PTR prvLion2FabricDbPtrArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

#define PRV_LION2_FABRIC_CHECK_DEV_ID_MAC(_dev)                          \
    if (prvLion2FabricDbInitDone == GT_FALSE) return GT_NOT_INITIALIZED; \
    if (_dev >= PRV_CPSS_MAX_PP_DEVICES_CNS) return GT_BAD_PARAM;        \
    if (prvLion2FabricDbPtrArr[_dev] == NULL) return GT_BAD_PARAM;       \

/**
* @internal prvLion2FabricQosProfilesCfg function
* @endinternal
*
* @brief   The function configures QOS Profiles
*
* @param[in] devNum                   - device number
* @param[in] qosProfileBase           - qos Profile Base Index
* @param[in] enable                   - GT_TRUE for Configure, GT_FALSE for Clean Up
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricQosProfilesCfg
(
    IN    GT_U8                      devNum,
    IN    GT_U32                     qosProfileBase,
    IN    GT_BOOL                    enable
)
{
    GT_STATUS                  rc;
    GT_U32                     tc;
    CPSS_DP_LEVEL_ENT          dp;
    CPSS_DXCH_COS_PROFILE_STC  cosProfile;

    cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));

    for (tc = 0; (tc < 8); tc++)
    {
        if (enable != GT_FALSE)
        {
            cosProfile.trafficClass = tc;
        }
        for (dp = 0; (dp <= CPSS_DP_RED_E); dp++)
        {
            if (enable != GT_FALSE)
            {
                cosProfile.dropPrecedence = dp;
            }
            rc = cpssDxChCosProfileEntrySet(
                devNum,
                PRV_QOS_PROFILE_INDEX_GET_MAC(qosProfileBase, tc, dp),
                &cosProfile);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvLion2FabricAddNewPortsHandle function
* @endinternal
*
* @brief   The function hanles new Lion2 port using
*         updates VIDX for default Lookup0 rule.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] addedPortsBmpPtr         - (pointer to) bitmap of new added Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricAddNewPortsHandle
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  CPSS_PORTS_BMP_STC    *addedPortsBmpPtr
)
{
    GT_STATUS             rc;
    GT_U32                i;
    GT_U32                addedNewPorts = 0;
    CPSS_PORTS_BMP_STC    newPortsBmp;
    CPSS_PORTS_BMP_STC    *bmpPtr =
        &(prvLion2FabricDbPtrArr[lion2FabricDevNum]->portsBmp);

    for (i = 0; (i < CPSS_MAX_PORTS_BMP_NUM_CNS); i++)
    {
        newPortsBmp.ports[i] =
            ((~ (bmpPtr->ports[i])) & addedPortsBmpPtr->ports[i]);
        if (newPortsBmp.ports[i] != 0)
        {
            addedNewPorts = 1;
        }
    }

    if (addedNewPorts == 0)
    {
        return GT_OK;
    }

    for (i = 0; (i < CPSS_MAX_PORTS_BMP_NUM_CNS); i++)
    {
        bmpPtr->ports[i] |= addedPortsBmpPtr->ports[i];
    }

    rc = cpssDxChBrgMcEntryWrite(
        lion2FabricDevNum,
        (GT_U16)prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleVidx,
        bmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPvidForce(
        lion2FabricDevNum, &newPortsBmp, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPclEnable(
        lion2FabricDevNum, &newPortsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPclLookupBind(
        lion2FabricDevNum, &newPortsBmp,
        CPSS_PCL_LOOKUP_0_0_E,
        PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPclLookupBind(
        lion2FabricDevNum, &newPortsBmp,
        CPSS_PCL_LOOKUP_0_1_E,
        PRV_LION2_FABRIC_LOOKUP1_PCL_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvLion2FabricRemovePortsHandle function
* @endinternal
*
* @brief   The function hanles removing Lion2 ports from using
*         by Lion2Fabric.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] removedPortsBmpPtr       - (pointer to) bitmap of new added Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
static GT_STATUS prvLion2FabricRemovePortsHandle
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  CPSS_PORTS_BMP_STC    *removedPortsBmpPtr
)
{
    GT_STATUS             rc;
    GT_U32                i;
    GT_U32                removedPorts = 0;
    CPSS_PORTS_BMP_STC    removedPortsBmp;
    CPSS_PORTS_BMP_STC    *bmpPtr =
        &(prvLion2FabricDbPtrArr[lion2FabricDevNum]->portsBmp);

    for (i = 0; (i < CPSS_MAX_PORTS_BMP_NUM_CNS); i++)
    {
        removedPortsBmp.ports[i] =
            (bmpPtr->ports[i] & removedPortsBmpPtr->ports[i]);
        if (removedPortsBmp.ports[i] != 0)
        {
            removedPorts = 1;
        }
    }

    if (removedPorts == 0)
    {
        return GT_OK;
    }

    for (i = 0; (i < CPSS_MAX_PORTS_BMP_NUM_CNS); i++)
    {
        bmpPtr->ports[i] &= (~ (removedPortsBmpPtr->ports[i]));
    }

    rc = cpssDxChBrgMcEntryWrite(
        lion2FabricDevNum,
        (GT_U16)prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleVidx,
        bmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPvidForce(
        lion2FabricDevNum, &removedPortsBmp, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPclLookupBind(
        lion2FabricDevNum, &removedPortsBmp,
        CPSS_PCL_LOOKUP_0_0_E,
        PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricPclLookupBind(
        lion2FabricDevNum, &removedPortsBmp,
        CPSS_PCL_LOOKUP_0_1_E,
        PRV_LION2_FABRIC_LOOKUP1_PCL_ID_CNS, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* API */

/**
* @internal appDemoLion2FabricInit function
* @endinternal
*
* @brief   The function initializes DB and writes PCL rule for TO_CPU packets
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] cpuLinkPortNum           - number of Lion2 port linked to BC2 devices with CPU port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricInit
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_PORT_NUM        cpuLinkPortNum
)
{
    GT_STATUS                  rc;
    GT_U32                     tcamRulesNum;
    GT_U32                     qosProfilesNum;
    CPSS_PORTS_BMP_STC         addedPortsBmp;

    if (lion2FabricDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }

    if (prvLion2FabricDbInitDone == GT_FALSE)
    {
        cpssOsMemSet(prvLion2FabricDbPtrArr, 0, sizeof(prvLion2FabricDbPtrArr));
        prvLion2FabricDbInitDone = GT_TRUE;
    }

    if (prvLion2FabricDbPtrArr[lion2FabricDevNum] != NULL)
    {
        return GT_BAD_STATE;
    }

    prvLion2FabricDbPtrArr[lion2FabricDevNum] =
        (PRV_LION2_FABRIC_DB_PTR)cpssOsMalloc(sizeof(PRV_LION2_FABRIC_DB_STC));
    if (prvLion2FabricDbPtrArr[lion2FabricDevNum] == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    rc = cpssDxChCfgTableNumEntriesGet(
        lion2FabricDevNum,
        CPSS_DXCH_CFG_TABLE_PCL_ACTION_E,
        &tcamRulesNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(prvLion2FabricDbPtrArr[lion2FabricDevNum], 0, sizeof(PRV_LION2_FABRIC_DB_STC));
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex = PRV_LION2_FABRIC_START_RULE_INDEX_CNS;
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleVidx   = PRV_LION2_FABRIC_START_VIDX_CNS;
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum = (PRV_LION2_FABRIC_START_VIDX_CNS + 1);
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleIndex  = (tcamRulesNum - 1);

    rc = prvLion2FabricUdbsConfigure(lion2FabricDevNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricRuleToCpuWrite(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
        PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS, cpuLinkPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;

    cpssOsMemSet(&addedPortsBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
    CPSS_PORTS_BMP_PORT_SET_MAC((&addedPortsBmp), cpuLinkPortNum);

    rc = prvLion2FabricAddNewPortsHandle(
        lion2FabricDevNum, &addedPortsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvLion2FabricRuleDefaultWrite(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleIndex,
        PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleVidx);

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCfgTableNumEntriesGet(
        lion2FabricDevNum,
        CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E,
        &qosProfilesNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* 24 qos profiles needed fo TC anDP assigments */
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->qosProfileBase = qosProfilesNum - 24;

    rc = prvLion2FabricQosProfilesCfg(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->qosProfileBase,
        GT_TRUE /*enable*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2FabricForwardAddBc2Device function
* @endinternal
*
* @brief   The function writes PCL rules for UC packets with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be forwarded to the given target device.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] hwDevId                  - HW id of BC2 target device
* @param[in] linkPortNum              - number of Lion2 port linked to BC2 target device
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricForwardAddBc2Device
(
    IN  GT_U8           lion2FabricDevNum,
    IN  GT_U32          hwDevId,
    IN  GT_PORT_NUM     linkPortNum
)
{
    GT_STATUS                       rc;
    GT_U32                          i;
    CPSS_PORTS_BMP_STC              addedPortsBmp;
    /* list of supported packet commands in eDSA Tag */
    CPSS_DXCH_NET_DSA_CMD_ENT       packetCmdArr[] =
    {
        CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E    ,
        CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E ,
        CPSS_DXCH_NET_DSA_CMD_FORWARD_E
    };
    GT_U32                          packetCmdNum =
        (sizeof(packetCmdArr) / sizeof(packetCmdArr[0]));

    PRV_LION2_FABRIC_CHECK_DEV_ID_MAC(lion2FabricDevNum);

    /* write to TCAM Rule for eac supported packet command eDSA Tag */
    for (i = 0; (i < packetCmdNum); i++)
    {
        rc = prvLion2FabricRuleToDeviceWrite(
            lion2FabricDevNum,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
            PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS,
            packetCmdArr[i], hwDevId, linkPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;
    }

    /* Configure new Lion2 port for PCL lookup0_0 */
    cpssOsMemSet(&addedPortsBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
    CPSS_PORTS_BMP_PORT_SET_MAC((&addedPortsBmp), linkPortNum);

    rc = prvLion2FabricAddNewPortsHandle(
        lion2FabricDevNum, &addedPortsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2FabricForwardAddBc2Vidx function
* @endinternal
*
* @brief   The function writes PCL rules for MC packets with the given target VIDX
*         with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2Vidx                  - target VIDX (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricForwardAddBc2Vidx
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2Vidx,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          i;
    /* list of supported packet commands in eDSA Tag */
    CPSS_DXCH_NET_DSA_CMD_ENT       packetCmdArr[] =
    {
        CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E    ,
        CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E ,
        CPSS_DXCH_NET_DSA_CMD_FORWARD_E
    };
    GT_U32                          packetCmdNum =
        (sizeof(packetCmdArr) / sizeof(packetCmdArr[0]));

    PRV_LION2_FABRIC_CHECK_DEV_ID_MAC(lion2FabricDevNum);

    /* create VIDX in Lion2 for target ports bitmap */
    rc = cpssDxChBrgMcEntryWrite(
        lion2FabricDevNum,
        (GT_U16)prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum,
        targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write to TCAM Rule for eac supported packet command eDSA Tag */
    for (i = 0; (i < packetCmdNum); i++)
    {
        rc = prvLion2FabricRuleToVidxWrite(
            lion2FabricDevNum,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
            PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS,
            packetCmdArr[i], bc2Vidx,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;
    }

    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum ++;

    /* Configure all new Lion2 ports for PCL lookup0_0 */
    rc = prvLion2FabricAddNewPortsHandle(
        lion2FabricDevNum, targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2FabricForwardAddBc2Vlan function
* @endinternal
*
* @brief   The function writes PCL rules for BC packets with the given target VID
*         with commands FORWARD, FROM_CPU
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2VlanId                - target eVID (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricForwardAddBc2Vlan
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2VlanId,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          i;
    /* list of supported packet commands in eDSA Tag */
    CPSS_DXCH_NET_DSA_CMD_ENT       packetCmdArr[] =
    {
        CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E    ,
        CPSS_DXCH_NET_DSA_CMD_FORWARD_E
    };
    GT_U32                          packetCmdNum =
        (sizeof(packetCmdArr) / sizeof(packetCmdArr[0]));

    PRV_LION2_FABRIC_CHECK_DEV_ID_MAC(lion2FabricDevNum);

    /* create VIDX in Lion2 for target ports bitmap */
    rc = cpssDxChBrgMcEntryWrite(
        lion2FabricDevNum,
        (GT_U16)prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum,
        targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write to TCAM Rule for eac supported packet command eDSA Tag */
    for (i = 0; (i < packetCmdNum); i++)
    {
        rc = prvLion2FabricRuleToVidWrite(
            lion2FabricDevNum,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
            PRV_LION2_FABRIC_LOOKUP0_PCL_ID_CNS,
            packetCmdArr[i], bc2VlanId,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;
    }

    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum ++;

    /* Configure all new Lion2 ports for PCL lookup0_0 */
    rc = prvLion2FabricAddNewPortsHandle(
        lion2FabricDevNum, targetPortsBmpPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2FabricCleanUp function
* @endinternal
*
* @brief   The function restores Lion2 device from all Lion2Fabric configurations.
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricCleanUp
(
    IN GT_U8              lion2FabricDevNum
)
{
    GT_STATUS             rc, totalRc;
    CPSS_PORTS_BMP_STC    removedPortsBmp;
    CPSS_PORTS_BMP_STC    emptyPortsBmp;
    GT_U32                i;

    PRV_LION2_FABRIC_CHECK_DEV_ID_MAC(lion2FabricDevNum);

    /* restore all port related configuration */

    cpssOsMemCpy(
        &removedPortsBmp,
        &(prvLion2FabricDbPtrArr[lion2FabricDevNum]->portsBmp),
        sizeof(CPSS_PORTS_BMP_STC));

    totalRc = GT_OK;

    rc = prvLion2FabricRemovePortsHandle(
        lion2FabricDevNum, &removedPortsBmp);
    if ((rc != GT_OK) && (totalRc == GT_OK))
    {
        totalRc = rc;
    }

    /* invalidate all PCL rules */
    for (i = PRV_LION2_FABRIC_START_RULE_INDEX_CNS;
         (i < prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex);
          i++)
    {
        rc = cpssDxChPclRuleInvalidate(
            lion2FabricDevNum, 0/*tcamIndex*/, CPSS_PCL_RULE_SIZE_STD_E, i);
        if ((rc != GT_OK) && (totalRc == GT_OK))
        {
            totalRc = rc;
        }
    }

    /* invalidate PCL default rule */
    i = prvLion2FabricDbPtrArr[lion2FabricDevNum]->lookup0DefaultRuleIndex;
    rc = cpssDxChPclRuleInvalidate(
        lion2FabricDevNum, 0/*tcamIndex*/, CPSS_PCL_RULE_SIZE_STD_E, i);
    if ((rc != GT_OK) && (totalRc == GT_OK))
    {
        totalRc = rc;
    }

    /* clear all Lion2 VIDXes */
    cpssOsMemSet(&emptyPortsBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
    for (i = PRV_LION2_FABRIC_START_VIDX_CNS;
         (i < prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeVidxNum);
          i++)
    {
        rc = cpssDxChBrgMcEntryWrite(
            lion2FabricDevNum, (GT_U16)i, &emptyPortsBmp);
        if ((rc != GT_OK) && (totalRc == GT_OK))
        {
            totalRc = rc;
        }
    }

    /* invalidate UDBs */
    rc = prvLion2FabricUdbsConfigure(lion2FabricDevNum, GT_FALSE);
    if ((rc != GT_OK) && (totalRc == GT_OK))
    {
        totalRc = rc;
    }

    /* clean Up qos Profiles */
    rc = prvLion2FabricQosProfilesCfg(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->qosProfileBase,
        GT_FALSE /*enable*/);
    if ((rc != GT_OK) && (totalRc == GT_OK))
    {
        totalRc = rc;
    }

    /* free allocated memory */
    cpssOsFree(prvLion2FabricDbPtrArr[lion2FabricDevNum]);
    prvLion2FabricDbPtrArr[lion2FabricDevNum] = NULL;

    return totalRc;
}

/**
* @internal appDemoLion2FabricConfigureQos function
* @endinternal
*
* @brief   The function configure Lion2Fabric for TC/DP support inside Lion2 device.
*         The eDSA Tag should remain unchanged.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] toCpuTc                  - TC for all TO_CPU packets
* @param[in] toCpuDp                  - DP for all TO_CPU packets
* @param[in] toAnalyzerTc             - TC for all TO_ANALYZER packets
* @param[in] toAnalyzerDp             - DP for all TO_ANALYZER packets
* @param[in] fromCpuDp                - DP for all FROM_CPU packets
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS appDemoLion2FabricConfigureQos
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_U32             toCpuTc,
    IN CPSS_DP_LEVEL_ENT  toCpuDp,
    IN GT_U32             toAnalyzerTc,
    IN CPSS_DP_LEVEL_ENT  toAnalyzerDp,
    IN CPSS_DP_LEVEL_ENT  fromCpuDp
)
{
    GT_STATUS  rc;
    GT_U32     pclId = PRV_LION2_FABRIC_LOOKUP1_PCL_ID_CNS;
    GT_U32     qosProfileBase =
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->qosProfileBase;
    GT_U32     tc;
    GT_U32     dp;

    /* Rules for Forwarded Packets ((8 * 3) rules)              */
    for (tc = 0; (tc < 8); tc++)
    {
        for (dp = CPSS_DP_GREEN_E; (dp <= CPSS_DP_RED_E); dp++)
        {

            rc = prvLion2FabricRuleQosFwdTcDpWrite(
                lion2FabricDevNum,
                prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
                pclId, tc, dp,
                PRV_QOS_PROFILE_INDEX_GET_MAC(qosProfileBase, tc, dp));
            if (rc != GT_OK)
            {
                return rc;
            }
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;
        }
    }

    /* Rules for From Cpu packets */
    for (tc = 0; (tc < 8); tc++)
    {
        rc = prvLion2FabricRuleQosFromCpuTcWrite(
            lion2FabricDevNum,
            prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
            pclId, tc,
            PRV_QOS_PROFILE_INDEX_GET_MAC(qosProfileBase, tc, fromCpuDp));
        if (rc != GT_OK)
        {
            return rc;
        }
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;
    }

    /* Rule For To Cpu packets */
    rc = prvLion2FabricRuleQosDefaultWrite(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
        pclId, CPSS_DXCH_NET_DSA_CMD_TO_CPU_E,
        PRV_QOS_PROFILE_INDEX_GET_MAC(qosProfileBase, toCpuTc, toCpuDp));
    if (rc != GT_OK)
    {
        return rc;
    }
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;

    /* Rule For To Analyzer packets */
    rc = prvLion2FabricRuleQosDefaultWrite(
        lion2FabricDevNum,
        prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex,
        pclId, CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E,
        PRV_QOS_PROFILE_INDEX_GET_MAC(qosProfileBase, toAnalyzerTc, toAnalyzerDp));
    if (rc != GT_OK)
    {
        return rc;
    }
    prvLion2FabricDbPtrArr[lion2FabricDevNum]->freeRuleIndex ++;

    return GT_OK;
}




