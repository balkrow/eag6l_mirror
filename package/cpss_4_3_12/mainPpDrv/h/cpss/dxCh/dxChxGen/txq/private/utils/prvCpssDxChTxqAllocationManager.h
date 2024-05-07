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
* @file prvCpssDxChTxqAllocationManager.h
*
* @brief CPSS SIP6 TXQ  dynamic resource allocations  operation functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqAllocationManager
#define __prvCpssDxChTxqAllocationManager

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_PDQ_RESOURCE_HEAD_PTR(_devNum,_tileNum,_level) \
    &(PRV_CPSS_DXCH_PP_MAC(_devNum)->port.tileConfigsPtr[_tileNum].mapping.pdqResources[_level])

#define PRV_SDQ_RESOURCE_HEAD_PTR(_devNum,_tileNum,_dp) \
    &(PRV_CPSS_DXCH_PP_MAC(_devNum)->port.tileConfigsPtr[_tileNum].mapping.sdqResources[_dp])



typedef struct PRV_CPSS_TXQ_PDQ_RESOURCE_RANGE{

    GT_U32  first;

    GT_U32  last;

    struct PRV_CPSS_TXQ_PDQ_RESOURCE_RANGE *next;

} PRV_CPSS_TXQ_RESOURCE_RANGE_STC;


GT_STATUS prvDxChTxqSip7AllocManagerAddResourceRange
(
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    GT_U32  first,
    GT_U32  last
);

GT_STATUS prvDxChTxqSip7AllocManagerRangeGet
(
    IN PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    IN  GT_U32    size,
    OUT GT_U32  *firstPtr
);

GT_STATUS prvCpssDxChTxqAllocManagerPdqHeadGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 level,
    OUT PRV_CPSS_TXQ_RESOURCE_RANGE_STC ** headPtr
);

GT_STATUS prvDxChTxqSip7AllocManagerRemoveResourceRange
(
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    IN GT_U32  first,
    IN GT_U32  last
);


GT_STATUS prvDxChTxqSip7AllocManagerFreeAllRanges
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);

GT_STATUS prvDxChTxqSip7AllocManagerInitAllRanges
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqAllocationManager */

