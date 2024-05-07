/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvCpssDxChIpfixFw.h
*
* @brief Private Structures, enums and CPSS APIs declarations for Ipfix Firmware config.
*
* @version   1
*********************************************************************************
**/

#ifndef __prvCpssDxChIpfixFwh
#define __prvCpssDxChIpfixFwh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>


/* Falcon */
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_FALCON_MAX_CNS             (_32K)
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_PER_TILE_FALCON_MAX_CNS     (_4K)
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS     (_4K)
/* Maximum service CPU num for FALCON */
#define PRV_CPSS_DXCH_IPFIX_FALCON_FW_SCPU_MAX_CNS          20

/* AC5X */
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5X_MAX_CNS                (_16K)
/* Maximum In-Transit IPFIX Index */
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5X_CNS      ((_16K) - 1)
/* maximum scan loop items*/
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_AC5X_MAX_CNS      (_16K)
/* Maximum number of exact match indexes */
#define PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_AC5X_MAX_CNS             (_16K)
/* Maximum service CPU num for AC5P/AC5X */
#define PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS           2

/* AC5P */
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5P_MAX_CNS                (_64K)
/* Maximum In-Transit IPFIX Index */
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5P_CNS      ((_64K) - 1)
/* maximum scan loop items*/
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_AC5P_MAX_CNS      (_64K)
/* Maximum number of exact match indexes */
#define PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_AC5P_MAX_CNS             (_64K)

/* Maximum number of Flows selected */
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(_devNum)                        \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
     PRV_CPSS_DXCH_IPFIX_FW_FLOWS_FALCON_MAX_CNS :                  \
     ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?  \
      PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5X_MAX_CNS :                            \
      PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5P_MAX_CNS))
#define PRV_CPSS_DXCH_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(_devNum)                        \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
     PRV_CPSS_DXCH_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS :                  \
     ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?  \
      PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5X_MAX_CNS :                            \
      PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5P_MAX_CNS))
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_CNS(_devNum)             \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5X_CNS :                   \
    PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5P_CNS)
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MAX_CNS(_devNum)             \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_AC5X_MAX_CNS :                   \
    PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_AC5P_MAX_CNS)
#define PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_MAX_CNS(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_AC5X_MAX_CNS :                          \
    PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_AC5P_MAX_CNS)
#define PRV_CPSS_DXCH_IPFIX_FW_SCPU_MAX_CNS(_devNum)    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
    PRV_CPSS_DXCH_IPFIX_FALCON_FW_SCPU_MAX_CNS :                             \
    PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS)

/* Maximum IPFIX Index */
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_IPFIX_INDEX_CNS(devNum)    (PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(devNum) - 1)
/* Maximum Policer IPFIX Index */
#define PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum)  (PRV_CPSS_DXCH_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1)


/* Maximum number of IPC messages that can be fetched at a time */
#define PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS   128

/* Maximum MTU size */
#define PRV_CPSS_DXCH_IPFIX_FW_DATA_PKTS_MTU_MAX_CNS        1150

/* Maximum Perioric Export interval */
#define PRV_CPSS_DXCH_IPFIX_FW_PERIODIC_EXPORT_INTERVAL_MAX_CNS 10000

/* Maximum value for SDMA queue number */
#define PRV_CPSS_DXCH_IPFIX_FW_SDMA_MAX_QUEUE_NUM_CNS       7

/* idle time out max = 15 min */
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_CNS         (15 * 60)

/* idle time out max without time stamp extension = 4 min */
#define PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_WITHOUT_EXTENSION_CNS         (4 * 60)

/* 1 unit = 100msec, min = 100 msec */
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MIN_CNS 1

/* 1 unit = 100msec, max = 60 sec */
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MAX_CNS (_1K)

/* Minimum scan loop items */
#define PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MIN_CNS 1

/* maximum mg number */
#define PRV_CPSS_DXCH_IPFIX_FW_MG_NUM_MAX_CNS 16

#define PRV_CPSS_DXCH_IPFIX_FW_IPC_MSG_MAX_SIZE_IN_BYTES_CNS    60
#define PRV_CPSS_DXCH_IPFIX_FW_IPC_BUFF_MAX_SIZE_IN_BYTES_CNS   (60 * PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS)

#define HW_SEMA_REG_BASE_SWITCH_MG 0x800
#define MAX_HW_SEMA                128
/* For CM3 the cpuId value will be 4 */
#define HW_SEMA_CM3_ID              4
#define HW_SEMA_HCPU_ID             0
#define HW_SEMA_IPFIX_EM           15

#define PRV_IPFIX_FW_BITS_SET(offset, size, var) \
    prvCpssDxChIpfixFwBitsSet(offset, size, (GT_U32)(ipcMsgPtr->var), ipcData)
#define PRV_IPFIX_FW_BITS_GET(offset, size, var, type) \
    ipcMsgPtr->var = (type) prvCpssDxChIpfixFwBitsGet(ipcDataPtr, offset, size)

typedef enum
{
    PRV_APP_IPFIX_FW_PKT_TYPE_IPV4_UDP_E,
    PRV_APP_IPFIX_FW_PKT_TYPE_IPV4_TCP_E,
    PRV_APP_IPFIX_FW_PKT_TYPE_IPV6_UDP_E,
    PRV_APP_IPFIX_FW_PKT_TYPE_IPV6_TCP_E,
} PRV_APP_IPFIX_FW_PKT_TYPE_ENT;

/**
 * @internal prvCpssDxChIpfixFwIpcMessageSend function
 * @endinternal
 *
 * @param[in] devNum   - device number
 * @param[in] devNum   - service cpu number
 * @param[in] ipcMsg   - IPC messae
 *
 * @retval    GT_OK     - on success
 * @retval    GT_FAIL   - on ipc error
 */
GT_STATUS prvCpssDxChIpfixFwIpcMessageSend
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT *ipcMsg
);

/**
 * @internal prvCpssDxChIpfixFwIpcMsgFetchAndParse function
 * @endinternal
 *
 * @brief  Fetch and parse IPC messages
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P.
 *
 * @param[in]  devNum              - device number
 * @param[in]  sCpuNum             - service CPU number
 * @param[in]  numOfMsgs           - number of messages to be fetched.
 * @param[out] numOfMsgsFetchedPtr - (pointer to) number of messages fetched.
 * @param[out] pendingMsgBuffSizePtr - (pointer to) Size in Bytes of the number of remaining messages
 *                                   in Tx IPC queue, which are pending to be fetched
 * @param[out] ipcMsgArrPtr        - (pointer to) fetched messages array.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 */
GT_STATUS prvCpssDxChIpfixFwIpcMsgFetchAndParse
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum,
    IN  GT_U32    numOfMsgs,
    OUT GT_U32    *numOfMsgsFetchedPtr,
    OUT GT_U32    *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   *ipcMsgArrPtr
);

/**
* @internal ipfixEmSemaLock function
* @endinternal
*
* @brief   Aquiring the HW semaphore lock for EM access.

* @param[in] devNum               - device number
* @param[in] semaNum              - semaphore number
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_BOOL ipfixEmSemaLock
(
    IN  GT_U8   devNum,
    IN  GT_U32  semaNum
);

/**
* @internal ipfixEmSemaLUnlock function
* @endinternal
*
* @brief   Releasing the HW semaphore lock for EM access.

* @param[in] devNum               - device number
* @param[in] semaNum              - semaphore number
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_BOOL ipfixEmSemaUnlock
(
    IN  GT_U8   devNum,
    IN  GT_U32  semaNum
);

#ifdef __cplusplus
}
#endif /* __prvCplusplus */

#endif /* __prvCpssDxChIpfixFwh */
