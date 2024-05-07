/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
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
* @file flexeAgentHwConfig.h
*
* @brief FLEXE AGENT hardware configuration API definitions
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_HW_CONFIG_H
#define FLEXE_AGENT_HW_CONFIG_H


/**
 * @enum FLEXE_GROUP_BANDWIDTH_TYPE_ENT
 *
 * @brief enumerator for group bandwidth
 */
typedef enum {
    FLEXE_GROUP_BANDWIDTH_TYPE_50G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_100G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_150G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_200G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_250G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_300G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_350G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_400G_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_200G_WITH_100G_INST_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_300G_WITH_100G_INST_E,
    FLEXE_GROUP_BANDWIDTH_TYPE_400G_WITH_100G_INST_E
} FLEXE_GROUP_BANDWIDTH_TYPE_ENT;

GT_STATUS prvFlexeAgentCoderClientConfigSet
(
    IN  GT_U8    tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *initParamsPtr
);

GT_STATUS prvFlexeAgentXcClientConfigSet
(
    IN  GT_U8    tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *paramsPtr
);

GT_STATUS flexeAgentShimClientConfigSet
(
    IN  GT_U8                        tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *paramsPtr
);

/* [TBD] comments */
GT_STATUS   flexeAgentHwInit
(
    GT_U8       tileId
);

GT_STATUS flexeAgentGroupCreate
(
    IN  GT_U8       tileId,
    IN  FLEXE_AGENT_GROUP_CONFIG_STC *paramsPtr
);

GT_STATUS flexeAgentGroupDelete
(
    IN  GT_U8       tileId,
    IN  GT_U8       instanceBmp,
    IN  GT_U8       *bondMaskArrPtr,
    IN  GT_U8       interleaveCfg
);

GT_STATUS flexeAgentCalendarSwitch
(
    IN GT_U8        tileId
);

GT_STATUS flexeAgentShimClientConfigSet
(
    IN  GT_U8                        tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *paramsPtr
);

#endif /*FLEXE_AGENT_HW_CONFIG_H*/
