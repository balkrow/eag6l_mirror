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
* @file prvTgfPclSip7.h
*
* @brief Sip7 specific PCL features
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclSip7
#define __prvTgfPclSip7

#include <trafficEngine/tgfTrafficGenerator.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <extUtils/tcamClients/tgfTcamClients.h>
#include <extUtils/tcamClients/prvTgfTcamClients.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
);

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
);

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
);

/*******************************************************************************
* prvpTgfSip7TcamClientsDevInit
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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
*                                       None
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
);

/*===== TESTS =====*/

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclSip7 */

