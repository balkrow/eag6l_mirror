/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefMEFUtils.h
*
* DESCRIPTION:
*  This files provide infra functions to support all MEF related files.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#ifndef __csRefMEFUtils_h
#define __csRefMEFUtils_h

#include <cpss/generic/cpssTypes.h>
#include "../../infrastructure/csRefServices/stringToNetUtils.h"
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>


#define PCL_ID_CNS               0x10 /* Same PCL-Id for all */
#define PCL_RULE_FORMAT_CNS   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
#define PCL_RULE_SIZE_CNS     1 /* rule size 10 Bytes */

/* CNC           */
#define PCL_LOOKUP_0_PARALLEL_0_CNC_BLOCK_NUM 3
#define PCL_LOOKUP_0_PARALLEL_1_CNC_BLOCK_NUM 4

#define QOS_TRUST_MODE_TABLE_INDEX_CNS    0
#define QOS_TRUST_MODE_GREEN_PROFILE_CNS  100
#define QOS_TRUST_MODE_YELLOW_PROFILE_CNS 108

#define CPSS_CALL(function)\
rc = (function);\
if(GT_OK != rc)                                                                \
{ \
  osPrintf("Function %s,Line %d, fail:rc=%d!\n",__FUNCNAME__,__LINE__,rc);  \
  return rc;    \
}


/**
* @internal csRefMefUtilQosTrustModeColorConfig function
* @endinternal
*
* @brief   This function sets the qos configuration globally and per the ingress port.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
* @param[in] ingressPort      - the ingress port the apply the qos configurations.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilQosTrustModeColorConfig
(
  IN GT_U8        devNum,
  IN GT_PORT_NUM  ingressPort
);


/**
* @internal csRefMefUtilPclInit function
* @endinternal
*
* @brief   This function sets the global IPCL conigurations.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPclInit
(
  IN GT_U8  devNumsss
);


/**
* @internal csRefMefUtilPClIngressPortInit function
* @endinternal
*
* @brief   This function sets the IPCL conigurations for a specific ingress port.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum   - device number.
* @param[in] portNum  - the ingress port the apply the IPCL configurations.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPClIngressPortInit
(
  IN GT_U8        devNum,
  IN GT_PORT_NUM  portNum
);


/**
* @internal csRefMefUtilPClRuleSet function
* @endinternal
*
* @brief   This function sets the ipcl rule parameters according to the user input.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device number.
* @param[in] daMacDa      - dmac address that the IPCL will catch for incoming packets.
* @param[in] pclEntryIndex      - the pcl index for this rule.
* @param[in] egressPortNum      - the egress port to redirect the packet to, in case of a match.
* @param[in] meterId      - the metering index to be assigned in the action, in case of a match.
* @param[in] pktCmd       - the packet command to be assigned in the action, in case of a match.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPClRuleSet
(
  IN GT_U8               devNum,
  IN GT_ETHERADDR       *daMacDa,          /* Packet's MAC-DA . */
  IN GT_U32              pclEntryIndex,
  IN GT_PORT_NUM         egressPortNum,    /* Egress port */
  IN GT_U32              meterId,
  IN CPSS_PACKET_CMD_ENT pktCmd
);


/**
* @internal csRefMefUtilQosHwPolicerInit function
* @endinternal
*
* @brief   This function sets the policer global configuration in the device. 
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device number.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilQosHwPolicerInit
(
  IN GT_U8   devNum
);

#endif