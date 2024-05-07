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
* @file prvTgfIpfixFw.h
*
* @brief IPFIX Manager functional testing private API declarations
*
* @version   1
********************************************************************************
*/

#define PRV_TGF_IPFIX_FW_AC5X_FULL_CAPACITY_FLOWS_NUM   15000
#define PRV_TGF_IPFIX_FW_FALCON_FULL_CAPACITY_FLOWS_NUM 4096

#define PRV_TGF_IPFIX_SCPU_NUM_CNS(_devNum)                                         \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 16 : 0)

#define PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(_devNum)                   \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ?  GT_FALSE: GT_TRUE)

#define PRV_TGF_IPFIX_ADD_DELETE_FLOWS_NUM_CNS(_devNum)                   \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ?  100: 100)

#define PRV_TGF_IPFIX_FULL_CAPACITY_FLOWS_NUM_CNS(_devNum)                   \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
    PRV_TGF_IPFIX_FW_FALCON_FULL_CAPACITY_FLOWS_NUM : PRV_TGF_IPFIX_FW_AC5X_FULL_CAPACITY_FLOWS_NUM)

#define IPFIX_FW_WAIT_TIME_FOR_IPC_MSG_RETURN 200
#define IPFIX_FW_WAIT_TIME_IPFIX_ENTRY_RELOCATION 200
#define IPFIX_FW_WAIT_TIME_BEFORE_CLEAN       2000
#define IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND 2000
#define IPFIX_FW_WAIT_TIME_AFTER_DELETE_SEND  2000
#define IPFIX_FW_WAIT_TIME_BEFORE_DEBUG_DUMP  7000
#define IPFIX_FW_WAIT_FOR_AGED_OUT            12000
#define IPFIX_FW_WAIT_FOR_IDLE_ACTIVE_AGED_OUT 50000
#define IPFIX_FW_WAIT_FOR_MAX_AGED_OUT        300000
#define IPFIX_FW_WAIT_FOR_MAX_EXTENDED_AGED_OUT 1200000
#define IPFIX_FW_TRAFFIC_SEND_INTERVAL ((PRV_APP_IPFIX_FW_ENTRY_IDLE_TIMEOUT_CNS/2)*1000)
#define PRV_TGF_IPFIX_FW_IDLE_TIMEOUT_IN_SEC_CNS (8)
#define PRV_TGF_IPFIX_FW_MAX_IDLE_TIMEOUT_IN_SEC_CNS (240)
#define PRV_TGF_IPFIX_FW_MAX_EXTENDED_IDLE_TIMEOUT_IN_SEC_CNS (900)
#define PRV_TGF_IPFIX_FW_ACTIVE_TIMEOUT_IN_SEC_CNS (20)
#define PRV_TGF_IPFIX_FW_MAX_ACTIVE_TIMEOUT_IN_SEC_CNS (240)
#define PRV_TGF_IPFIX_FW_MAX_EXTENDED_ACTIVE_TIMEOUT_IN_SEC_CNS (900)
#define PRV_TGF_IPFIX_FW_IDLE_TIMEOUT_IN_IDLE_ACTIVE_SEC_CNS (100)
#define PRV_TGF_IPFIX_FW_ACTIVE_TIMEOUT_IN_IDLE_ACTIVE_SEC_CNS (50)

typedef enum
{
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_IDLE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_IDLE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_BASIC_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_AND_AGING_BASIC_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_ACTIVE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_ACTIVE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_ACTIVE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_ACTIVE_TIMEOUT_E,
    PRV_TGF_IPFIX_FW_TEST_TYPE_PERIODIC_DATA_EXPORT_E
} PRV_TGF_IPFIX_FW_TEST_TYPE_ENT;


/*************************** Enumerators ***************************/

typedef enum
{
    PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_UDP_E,
    PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_TCP_E,
    PRV_TGF_IPFIX_FW_PKT_TYPE_IPV6_UDP_E,
    PRV_TGF_IPFIX_FW_PKT_TYPE_IPV6_TCP_E,
    PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_UDP_JUMBO_E,
} PRV_TGF_IPFIX_FW_PKT_TYPE_ENT;

/*************************** Function Declarations *****************/

GT_VOID prvTgfIpfixFwVlanConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfIpfixFwDataPktsCheck function
 * @endinternal
 *
 * @brief Check data packet
 *
 * @retval GT_VOID
 */
GT_VOID prvTgfIpfixFwDataPktsCheck
(
    GT_VOID
);

GT_VOID prvTgfIpfixFwTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount,
    IN GT_BOOL  captureEnable
);

GT_VOID prvTgfIpfixFwFlowsLearnFullCapacityTrafficGenerate
(
    IN  GT_U32      maxFlows,
    OUT GT_U32      *collisonCountPtr
);

GT_VOID prvTgfIpfixFwElephantFlowsTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount,
    IN GT_BOOL  isJumbo
);

GT_VOID prvTgfIpfixFwAllL4TrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount
);

GT_VOID prvTgfIpfixFwConfigRestore
(
    GT_VOID
);

GT_VOID prvTgfIpfixFwNewFlowsLearn
(
    IN  GT_BOOL withFw,
    IN  GT_BOOL checkDataPkts,
    IN  GT_BOOL hwAutoLearn
);

/**
 * @internal prvTgfIpfixFwConfigSet function
 * @endinternal
 *
 * @brief Set IPFIX configuration
 *
 * @param[in] devNum   - device number
 * @param[in] sCpuNum  - service CPU number
 * @param[in] testType - test type
 *
 * @return GT_OK - on success
 */
GT_STATUS prvTgfIpfixFwConfigSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN  PRV_TGF_IPFIX_FW_TEST_TYPE_ENT testType
);

/**
 * @internal    prvTgfIpfixFwMain function
 * @endinternal
 *
 * @brief   Perform all initializations and start IPFIX
 *          Manager tasks
 *
 * @param[in] devNum   -   device number
 * @param[in] sCpuNum  -   service CPU Number
 * @param[in] testType -   test type
 *
 * @retval GT_OK - on success
 */
GT_STATUS prvTgfIpfixFwMain
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN  PRV_TGF_IPFIX_FW_TEST_TYPE_ENT testType,
    IN  GT_BOOL hwAutoLearnEnable
);
