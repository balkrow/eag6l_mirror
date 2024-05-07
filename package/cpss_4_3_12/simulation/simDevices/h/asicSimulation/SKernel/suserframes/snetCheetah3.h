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
* @file snetCheetah3.h
*
* @brief This is a external API definition for CH3.
*
* @version   4
********************************************************************************
*/
#ifndef __snetCheetah3h
#define __snetCheetah3h

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/skernel.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    GT_U32  startBit;
    GT_U32  lastBit;
}HW_FIELD;


typedef enum{
    SNET_CHT3_MLL_FIELDS_LAST_0_E                                                        ,
    SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_0_E                                             ,
    SNET_CHT3_MLL_FIELDS_MLL_VID_0_E                                                     ,
    SNET_CHT3_MLL_FIELDS_USE_VIDX_0_E                                                    ,
    SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_0_E                                             ,
    SNET_CHT3_MLL_FIELDS_TRG_PORT_0_E                                                    ,
    SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_0_E                                                ,
    SNET_CHT3_MLL_FIELDS_TRG_DEV_0_E                                                     ,
    SNET_CHT3_MLL_FIELDS_VIDX_0_E                                                        ,
    SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_0_E                                            ,
    SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_0_E                                              ,
    SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_0_E                                                 ,
    SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_0_HOP_LIMIT_THRESHOLD_0_E                         ,
    SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_0_E                                             ,
    SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_0_E                                              ,

    SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_1_E                                              ,

    SNET_CHT3_MLL_FIELDS_LAST_1_E                                                        ,
    SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_1_E                                             ,
    SNET_CHT3_MLL_FIELDS_MLL_VID_1_E                                                     ,
    SNET_CHT3_MLL_FIELDS_USE_VIDX_1_E                                                    ,
    SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_1_E                                             ,
    SNET_CHT3_MLL_FIELDS_TRG_PORT_1_E                                                    ,
    SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_1_E                                                ,
    SNET_CHT3_MLL_FIELDS_TRG_DEV_1_E                                                     ,
    SNET_CHT3_MLL_FIELDS_VIDX_1_E                                                        ,
    SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_1_E                                            ,
    SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_1_E                                              ,
    SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_1_E                                                 ,
    SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_1_HOP_LIMIT_THRESHOLD_1_E                         ,
    SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_1_E                                             ,

    SNET_CHT3_MLL_FIELDS_NEXT_MLL_PTR_E                                                  ,

/* Added for WLAN bridging -- start */
    SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_0_E                                          ,
    SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_0_E                                          ,
    SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_0_E                                        ,
    SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_0_E                                        ,

    SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_1_E                                          ,
    SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_1_E                                          ,
    SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_1_E                                        ,
    SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_1_E                                        ,
/* Added for WLAN bridging -- end */

    SNET_CHT3_MLL_FIELDS_LAST_E
}SNET_CHT3_MLL_FIELDS_ENT;

extern const HW_FIELD snetCh3MllEntryFormat[];


typedef struct {
    GT_U32  rpf_fail_cmd;
    GT_U32 ttlThres;
    GT_U32 excludeSrcVlan;
    GT_U32 last;
    SNET_DST_INTERFACE_STC lll;
    GT_U32  vid;
    GT_U32  isTunnelStart;
    struct{
        GT_U32  tunnelStartType;
        GT_U32  tunnelStartPtr;
    }tsInfo;/* tunnel start info */

/* Added for WLAN bridging -- start */
    GT_BIT      unregBcFiltering;
    GT_BIT      unregMcFiltering;
    GT_BIT      unknownUcFiltering;
    GT_BIT      vlanEgressTagMode;
/* Added for WLAN bridging -- end */

}SNET_CHT3_SINGLE_MLL_STC;

/* Multicast Link List */
typedef struct {
    SNET_CHT3_SINGLE_MLL_STC      first_mll;
    SNET_CHT3_SINGLE_MLL_STC      second_mll;
    GT_U32              nextPtr;
}SNET_CHT3_DOUBLE_MLL_STC;

/**
* @internal checkSumCalc function
* @endinternal
*
* @brief   Perform ones-complement sum , and ones-complement on the final sum-word.
*         The function can be used to make checksum for various protocols.
* @param[in] pMsg                     - pointer to IP header.
* @param[in] lMsg                     - IP header length.
*
* @note 1. If there's a field CHECKSUM within the input-buffer
*       it supposed to be zero before calling this function.
*       2. The input buffer is supposed to be in network byte-order.
*
*/
GT_U16 checkSumCalc
(
    IN GT_U8 *pMsg,
    IN GT_U16 lMsg
);

/**
* @internal snetCht3ReadMllEntry function
* @endinternal
*
* @brief   Read Multicast Link List entry
*
* @param[out] mllPtr                   - pointer to the MLL entry.
*/
void snetCht3ReadMllEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  mllIndex,
    OUT SNET_CHT3_DOUBLE_MLL_STC *mllPtr
);

/**
* @internal snetCht3IngressL3IpReplication function
* @endinternal
*
* @brief   Ingress L3 Ipv4/6 replication
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
GT_VOID snetCht3IngressL3IpReplication
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);

/**
* @internal snetCht3mllCounters function
* @endinternal
*
* @brief   Update MLL counters
*/
GT_VOID snetCht3mllCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL      isL2Mll
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetah3h */

