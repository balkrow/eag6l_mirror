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
* @file cpssDxChIpfixFw.h
*
* @brief Structures, enums and CPSS APIs declarations for Ipfix Firmware config.
*
* @version   1
*********************************************************************************
**/

#ifndef __cpssDxChIpfixFwh
#define __cpssDxChIpfixFwh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>

#define CPSS_DXCH_IPFIX_FW_FALCON_TILES_MAX_CNS     4

/**********************************************************
 *                         Enumerators
 **********************************************************/

/**
 * @enum CPSS_DXCH_IPFIX_FW_END_REASON_ENT
 *
 * @brief enumerator for active/idle timeout
 */
typedef enum {
    /** idle flow */
    CPSS_DXCH_IPFIX_FW_END_REASON_IDLE_E,

    /** long lasting flow*/
    CPSS_DXCH_IPFIX_FW_END_REASON_LONG_LASTING_E,

    CPSS_DXCH_IPFIX_FW_END_REASON_RESERVED_E,

    /** removed by application */
    CPSS_DXCH_IPFIX_FW_END_REASON_APP_REMOVED_E,
} CPSS_DXCH_IPFIX_FW_END_REASON_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT
 *
 * @brief   enumerator for config error type
 */
typedef enum {
    /** Failed due to any other error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_FAIL_E,

    /** Out of range error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_OUT_OF_RANGE_E,

    /** Bad parameter error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_BAD_PARAM_E,

    /** No resource error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NO_RESOURCE_E,

    /** initialization error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_INIT_ERROR_E,

    /** not supported error  */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NOT_SUPPORTED_E,

    /** scan cycle error */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_SCAN_CYCLE_ERROR_E,

    /** non error value */
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NONE_E
} CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT
 *
 * @brief enumerator for flow type
 */
typedef enum {
    /** all active flows */
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E,

    /** lastly active flows */
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_E,

    /** all elephant flows */
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ELEPHANTS_E,

    /** lastly active elephant flows */
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_ELEPHANTS_E
} CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT
 *
 * @brief enumerator for maximum number of flows
 */
typedef enum {
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_UNLIMITED_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_128_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_256_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_512_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_1K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_2K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_4K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_8K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_16K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_32K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_64K_E,
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_LAST_E
} CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_FW_CONFIG_TYPE_ENT
 *
 * @brief enumerator for configGet type
 */
typedef enum {
    /** get global config params */
    CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_GLOBAL_E,

    /** get port group config params */
    CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_PORT_GROUP_E,

    /** get export config params */
    CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_EXPORT_E,

    /** get elephant config params */
    CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_ELEPHANT_E,
} CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT
 *
 * @brief enumerator for IPC message opcodes
 */
typedef enum {
    /** Invalid */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_NONE_E,

    /** Entry Add */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_ADD_E           = 0x01,

    /** Entry Delete */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_E        = 0x02,

    /** Entry Delete All*/
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E    = 0x03,

    /** Global Config Set */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_SET_E   = 0x04,

    /** Data Get */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_E            = 0x05,

    /** Data Get all*/
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E        = 0x06,

    /** Entry Invalidate */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E    = 0x07,

    /** Export Completion */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E   = 0x08,

    /** config get */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E          = 0x09,

    /** Global Config Return */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E = 0x0A,

    /** config error */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E        = 0x0B,

    /** Port Group Config Set */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_SET_E   = 0x0C,

    /** Port Group Config Return */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E = 0x0D,

    /** export config set */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_SET_E   = 0x0E,

    /** export config return */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E = 0x0F,

    /** elephant config set */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_SET_E   = 0x10,

    /** elephant config return */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E = 0x11,

    /** enable set */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENABLE_SET_E             = 0x12,

    /** entry removed */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_REMOVED_E          = 0x13,

    /** elephant set / elephant invalidate */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_SET_E           = 0x14,

    /** delete all completion */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E   = 0x15,

    /** data clear / data clear all */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_E              = 0x16,

    /** data get all stop */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_STOP_E       = 0x17,

    /** data clear all completion */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_ALL_COMPLETION_E = 0x18,

    /** entry learned */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_LEARNED_E             = 0x19,

    /** max count */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_LAST_E

} CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT;

/**
 * @enum  CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT
 *
 * @brief enumerator for number of IPFIX entries per flow
 */
typedef enum {

    /** 2 entries per flow */
    CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E,

    /** 1 entry per flow */
    CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_ONE_E

} CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT;

/**********************************************************
 *                         Structures
 **********************************************************/

/**
 * @struct CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC
 *
 * @brief IPFIX Firmware global confiuration parameters structure.
 */
typedef struct {
    /** @brief Has no effect. ReadOnly parameter */
    GT_BOOL                             ipfixEnable;

    /** @brief Enable/Disable idle flow monitoring termination by FW
     *  It can be modified only when no active IPFIX flow. */
    GT_BOOL                             idleAgingOffload;

    /** @brief Enable/Disable long-lasting flow monitoring termination by FW
     *  It can be modified only when no active IPFIX flow. */
    GT_BOOL                             longAgingOffload;

    /** @brief Enable/Disable the extension of IPFIX counters in FW SRAM
     *  Cannot be enabled if elephantDetectionOffload is enabled.
     *  It can be modified only when no active IPFIX flow. */
    GT_BOOL                             countersExtensionOffload;

    /** @brief Enable/Disable the extension of IPFIX timestamps in FW SRAM
     *  It can be modified only when no active IPFIX flow. */
    GT_BOOL                             timeStampsExtensionOffload;

    /** @brief Sets the first policer stage concerned by IPFIX.
     * If hwAutoLearnEnable is enabled, IPLR1 is not supported.
     * First lookup and first policer stages must be from the same CP stage,
     *    either ingress or egress.
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    firstPolicerStage;

    /** @brief Sets the minimum time a flow must be idle for being terminated
     *  Range: 0 to 15min, in sec units. Typical is 30sec.
     *      If timestampExtensionOffload is disabled, the range is limited to 4min
     *  Meaningless when idleAgingOffload is disabled.
     *  If set to 0, flows are marked as pending termination for Idle timeout
     *      as soon as they are learned.
     *  It means the entry will be terminated by the coming aging scan cycle*/
    GT_U32                              idleTimeout;

    /** @brief Sets the minimum time a flow must be active for being terminated
     *  Range: 0 to 15min, in sec units. Typical is 200sec.
     *      If timestampExtensionOffload is disabled, the range is limited to 4min
     *  Meaningless when longAgingOffload is disabled.
     *  If set to 0, flows are marked as pending termination for Active timeout
     *      as soon as they are learned.
     *  It means the entry will be terminated by the coming aging scan cycle */
    GT_U32                              activeTimeout;

    /** @brief Sets the rate at which the whole IPFIX table will be scanned by FW.
     *  Range: 1 to 600, in 100msec units; i.e., [100ms, 60sec]. Typical is 2.5sec. */
    GT_U32                              scanCycleInterval;

    /** @brief Sets the number of IPFIX entries that will be scanned by a
     *         single Table Periodic Scan loop, as well as the max number of Rx IPC
     *         messages that will be processed by a single Rx IPC Handler loop.
     *  Range: 1 to 64K/16K. Default 1K/256*/
    GT_U32                              scanLoopItems;

    /** @brief Sets the second policer stage concerned by IPFIX.
        If secondPolicerStage == firstPolicerStage, it means that only one
            policer stage is concerned by IPFIX.
        Second lookup and second policer stages must be from the same CP stage,
            either ingress or egress.
        Meaningless if hwAutoLearnEnable is enabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT    secondPolicerStage;

    /** @brief Sets whether the new flows that were auto-learned by HW are
     *          reported to host.
     *  Meaningless if hwAutoLearnEnable is disabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_BOOL                             reportNewFlows;

    /** @brief Enable/Disable hardware auto learn.
     *  It can be modified only when no active IPFIX flow.
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_BOOL                             hwAutoLearnEnable;


    /** @brief Max index of entries in the In-transit range.
     *  Must be set to a lower or equal index than maxFirmwareIndex
     *  Meaningless if hwAutoLearnEnable is disabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_U32                              maxInTransitIndex;

    /** @brief Max index of entries in the Firmware range.
     *  Min index of entries in the Firmware range is [maxIntransitIndex+1]
     *  Must be set to a lower or equal index than maxAppIndex
     *  Meaningless if hwAutoLearnEnable is disabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_U32                              maxFwIndex;

    /** @brief Max index of entries in the App range.
     *  Min index of entries in the App range is [maxFirmwareIndex+1]
     *  If maxAppIndex == maxFirmwareIndex, it means that no IPFIX entry is
     *      allocated to the App.
     *  Meaningless if hwAutoLearnEnable is disabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_U32                              maxAppIndex;

    /** @brief Sets the maximum (highest) index of the IPFIX range for the
     *          first policer stage
     *  Range: 0 to (64K - 1) or (16K - 1)
     *  IPFIX entry at index 0 is always kept free.
     *  Setting it to 0 means no entry is bound to IPFIX in this policer
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_U32                              firstPolicerMaxIpfixIndex;

    /** @brief Sets the maximum (highest) index of the IPFIX range for the
     *          second policer stage
     *  Range: firstPolicerMaxIpfixIndex to (64K - 1) or (16K - 1)
     *  If secondPolicerMaxIpfixIndex == firstPolicerMaxIpfixIndex, it means
     *      that no entry is allocated to IPFIX for the second policer stage
     *  Meaningless if hwAutoLearnEnable is enabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_U32                              secondPolicerMaxIpfixIndex;

    /** @brief Controls whether an IPFIX flow uses a single IPFIX entry in a
     *           single CP or two IPFIX entries indexed by the same FlowID in
     *           the two CPs of a Tile (at once).
     *  (APPLICABLE DEVICES: Falcon)
     *  When Set to one IPFIX entries per Tile is 8K otherwise 4K for all Tiles,
     *  Setting it to one is an optimization mode, which requires that the Exact
     *    Match key UDBs include the ingress/egress port number depending on the
     *    policerStage IPLR/EPLR, respectively.
     *  It can be modified only when no active IPFIX flow
    .*/
    CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT ipfixEntriesPerFlow;

} CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC
 *
 * @brief structure to hold parameters of per tile IPFIX settings
 *        (APPLICALBE_DEVICES: Falcon)
 */
typedef struct {
    /** @brief firstPolicerStage - IPLR0, IPLR1, EPLR
     *         Sets the first policer stage concerned by IPFIX, across all Tiles
     *             concerned by IPFIX */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      firstPolicerStage;

    /** @brief secondPolicerStage - IPLR0, IPLR1, EPLR
     *         Sets the second policer stage concerned by IPFIX, across all Tiles
     *             concerned by IPFIX
     *         If secondPolicerStage == firstPolicerStage, it means that only
     *             one policer stage is concerned by IPFIX */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      secondPolicerStage;

    /** @brief Set the maximum (highest) index of the IPFIX range in Tile 0 for
     *           the first policer stage
     * Range: 0 to (4K - 1)
     *     IPFIX entry at index 0 is always kept free
     *     Setting it to 0 means no entry is bound to IPFIX in this policer for
     *         this Tile */
    GT_U32  firstPolicerMaxIpfixIndex[CPSS_DXCH_IPFIX_FW_FALCON_TILES_MAX_CNS];

    /** @brief Set the maximum (highest) index of the IPFIX range in Tile 0 for
     *           the second policer stage
     * Range: firstPolicerMaxIpfixIndex to (4K - 1)
     * If SecondPolicerMaxIpfixIndex == FirstPolicerMaxIpfixIndex, it means that
     * no entry is allocated to IPFIX in Tile for the second policer stage */
    GT_U32  secondPolicerMaxIpfixIndex[CPSS_DXCH_IPFIX_FW_FALCON_TILES_MAX_CNS];
} CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC
 *
 * @brief IPFIX Data Export confiuration parameters structure.
 */
typedef struct{
    /** @brief Maximum number of L2 payload bytes in an IPFIX data packet */
    GT_U32          dataPktMtu;

    /** @brief eDSA tag to be inserted by FW in IPFIX data packets sent to host */
    GT_U32          txDsaTag[4];

    /** @brief SDMA queue number used by IPFIX Assistant FW to send IPFIX data
     *         packets to host */
    GT_U32          localQueueNum;

    /** @brief MG Number*/
    GT_U32          mgNum;

    /** @brief Enable/Disable the periodic IPFIX data export, once every
     *         periodicExportInterval number of scanCycleInterval
     *  When disabled, IPFIX data is exported to host only on demand */
    GT_BOOL         periodicExportEnable;

    /** @brief Interval between two periodic IPFIX data export cycles, expressed
               in number of scanCycleInterval units
        Range: 1 to 10,000 */
    GT_U32          periodicExportInterval;

    /** @brief When enabled, the Last CPU or Drop Code field (8-bits) from the
     *         IPFIX Table is exported to host at flow termination or at
     *         data export cycles */
    GT_BOOL         exportLastCpuCode;

    /** @brief When enabled, the Last Packet Command field (3-bits) from the
     *         IPFIX Table is exported to host at flow termination or at
     *         data export cycles*/
    GT_BOOL         exportLastPktCmd;

    /** @brief Enable/disable mode. When enabled, on every export cycle,
     *           FW clears the data counters (not the timestamps) in IPFIX Table
     *           and in SRAM Extension Table
     *  APPLICABLE DEVICES: Falcon
     */
    GT_BOOL     deltaMode;
} CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC
 *
 * @brief Elephant detection confiuration parameters structure.
 */
typedef struct{
    /** @brief Enable the Elephant Detection & Report, which occurs when scanning
     *         all active IPFIX flows once every scanCycleInterval
     *  Cannot be enabled if countersExtensionOffload is enabled. */
    GT_BOOL     elephantDetectionOffload;

    /** @brief When disabled, an elephant flow that returns to be a mouse is not
     *         reported as such */
    GT_BOOL     reportMiceEnable;

    /** @brief Enable/Disable elephantSet/Invalidate messages are extended by 4B
     *         to report the EM index of elephant/mice flows.
     *  Meaningless if hwAutoLearnEnable is disabled
     *  (APPLICABLE DEVICES: AC5X, AC5P)
     */
    GT_BOOL     reportEmIndexEnable;

    /** @brief 6B packet count threshold used for elephant detection */
    GT_U32      pktCntThreshold[2];

    /** @brief 6B byte count threshold used for elephant detection */
    GT_U32      byteCntThreshold[2];

    /** @brief 1B cross counter high threshold used for elephant detection */
    GT_U32      crossCntThresholdHigh;

    /** @brief 1B cross counter low threshold used to detect an elephant flow
     *         that returned to be a mouse */
    GT_U32      crossCntThresholdLow;

    /** @brief 2B. Minimum requested duration for a flow before starting to be
     *         checked by the elephant detection algorithm. (4ms units) */
    GT_U32      startThreshold;
} CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC;


/**
 * @struct CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC
 *
 * @brief IPFIX Entry parameters
 */
typedef struct {
    /** @brief Index in the EM table
     *  APPLICABLE_DEVICES: AC5X, AC5P.
    .*/
    GT_U32      emIndex;

    /** @brief first time stamp (Value of the 15-bits long) prepended with 0's) which
     *           was extracted from the TO CPU eDSA tag of the first packet
     *  APPLICABLE_DEVICES: Falcon.
     */
    GT_U16      firstTs;

    /** @brief True/False. If set to false, it means the first field shall be copied
     *           from TOD by FW at the time it processes the command
    *  APPLICABLE_DEVICES: Falcon.
    */
    GT_BOOL     firstTsValid;
} CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC
 *
 * @brief Structure to read opcode of IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT      opcode;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_ADD_STC
 *
 * @brief Structure for entryAdd IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT   opcode;
    GT_U32                                  flowId;
    CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC     entryParams;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_ADD_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ENABLE_SET_STC
 *
 * @brief structure for enableSet IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    GT_BOOL                                ipfixEnable;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ENABLE_SET_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_FLOWID_STC
 *
 * @brief structure for IPC message with only flowId
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    GT_U32                                 flowId;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_FLOWID_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_DATA_GET_ALL_STC
 *
 * @brief structure for dataGetAll IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT       flowType;
    GT_U32                                 maxNumOfFlows;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_DATA_GET_ALL_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_GET_STC
 *
 * @brief structure for configGet IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_ENT configtype;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_GET_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC
 *
 * @brief structure for ExportCompletion IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    GT_U32                                 numOfExported;
    GT_U32                                 lastExportedFlowId;
    GT_U32                                 lastPktNum;
    GT_U32                                 lastTod[2];
} CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC
 *
 * @brief structure for EntryInvalidate IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT   opcode;
    CPSS_DXCH_IPFIX_FW_END_REASON_ENT  endReason;
    GT_U32                                  flowId;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC
 *
 * @brief structure for EntryRemoved IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT   opcode;
    CPSS_DXCH_IPFIX_FW_END_REASON_ENT  endReason;
    GT_U32                                  flowId;
    GT_U32                                  firstTs;
    GT_U32                                  lastTs;
    GT_U32                                  packetCount[2];
    GT_U32                                  dropCount[2];
    GT_U32                                  byteCount[2];
    CPSS_NET_RX_CPU_CODE_ENT                lastCpuCode;
    CPSS_PACKET_CMD_ENT                     lastPktCmd;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC
 *
 * @brief structure for GlobalConfigSet/GlobalConfigReturn IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC   globalCfg;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC
 *
 * @brief structure for PortGroupConfigSet/PortGroupConfigReturn IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT    opcode;
    CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC portGroupCfg;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC
 *
 * @brief structure for ExportConfigSet/ExportConfigReturn IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC   exportCfg;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC
 *
 * @brief structure for ElephantConfigSet/ElephantConfigReturn IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC elephantCfg;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC
 *
 * @brief structure for ElephantSet/Invalidate IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    GT_BOOL                                state;
    GT_U32                                 flowId;
    GT_U32                                 emIndex;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_DATA_CLEAR_COMPLETION_STC
 *
 * @brief structure for ElephantSet/ElephantInvalidate IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;
    GT_U32                                 numOfDeleted;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_DELETE_ALL_COMPLETION_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_STC
 *
 * @brief structure for config error IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT      opcode;
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT   errType;
} CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_ERROR_STC;

/**
 * @struct CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC
 *
 * @brief structure for entry learned IPC message
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT   opcode;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT      keySize;
    GT_U32      flowId;
    GT_U32      emIndex;
    GT_U32      flowKey[12];
} CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC;

/**********************************************************
 *                         Unions
 **********************************************************/

/**
 * @union CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT
 *
 * @brief union of IPC messages
 */
typedef union {

    /** dummy structure to get opcode of any IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC           opcodeGet;

    /** EntryAdd IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_ADD_STC        entryAdd;

    /** EntryDelete IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_FLOWID_STC           entryDelete;

    /** EntryDeleteAll IPC message*/
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC           entryDeleteAll;

    /** Global ConfigSet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC    globalConfigSet;

    /** export ConfigSet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC    exportConfigSet;

    /** elephant ConfigSet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC  elephantConfigSet;

    /** DataGet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_FLOWID_STC           dataGet;

    /** DataGetAll IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_DATA_GET_ALL_STC     dataGetAll;

    /** DataGetAllStop IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC           dataGetAllStop;

    /** ConfigGet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_GET_STC       configGet;

    /** enableSet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ENABLE_SET_STC       enableSet;

    /** dataClear / dataClearAll IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_FLOWID_STC           dataClear;

    /** Port Group ConfigSet IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC portGroupConfigSet;

} CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT;


/**
 * @union CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT
 *
 * @brief union of IPC messages
 */
typedef union {

    /** dummy structure to get opcode of any IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC               opcodeGet;

    /** GlobalConfigReturn IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC        globalConfigReturn;

    /** ExportCompletion IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC    exportCompletion;

    /** ExportConfigReturn IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC        exportConfigReturn;

    /** ElephantConfigReturn IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC      elephantConfigReturn;

    /** ElephantSet / ElephantInvalidate IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC         elephantSet;

    /** EntryRemoved IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC        entryRemoved;

    /** EntryInvalidate IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC     entryInvalidate;

    /** DeleteAllCompletion IPC Message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_DELETE_ALL_COMPLETION_STC  deleteAllCompletion;

    /** DataClearAllCompletion IPC Message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC               dataClearAllCompletion;

    /** configError IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_ERROR_STC         configError;

    /** entryLearned IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC         entryLearned;

    /** PortGroupConfigReturn IPC message */
    CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC    portGroupConfigReturn;

} CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT;


/**********************************************************
 *                  Function Declarations
 **********************************************************/

/**
 * @internal cpssDxChIpfixFwInit function
 * @endinternal
 *
 * @brief Load/Initialize FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   cpssDxChIpfixFwInit
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
);

/**
 * @internal cpssDxChIpfixFwRestore function
 * @endinternal
 *
 * @brief Remove FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   cpssDxChIpfixFwRestore
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
);

/**
 * @internal cpssDxChIpfixFwEnableSet function
 * @endinternal
 *
 * @brief Start/stop the full running of IPFIX Assistant FW; i.e., modify its
 *        admin state between enabled/disabled
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum,
    IN  GT_BOOL      ipfixEnable
);

/**
 * @internal cpssDxChIpfixFwGlobalConfigSet function
 * @endinternal
 *
 * @brief  Set the global IPFIX settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    *globalCfgPtr
);

/**
 * @internal cpssDxChIpfixFwGlobalConfigGet function
 * @endinternal
 *
 * @brief  Request that all global IPFIX settings be returned via
 *         a GlobalConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwGlobalConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
);

/**
 * @internal cpssDxChIpfixFwPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Set per tile IPFIX settings
 *         It is used to bind one or two policers to IPFIX. Per Tile, a
 *           consecutive range of FlowIDs is allocated to IPFIX, from
 *           index #1 upward. The range is split between the two policers
 *         After calling this API, it is required to call the PortGroupConfigGet
 *           API followed by IcpMsgFetch until a PortGroupConfigReturn (or a
 *           ConfigError) IPC message is fetched, and check that the configuration
 *           has been properly recorded by FW.
 *         This API can be invoked only when no active IPFIX flow.
 *
 * @note  APPLICABLE_DEVICES: Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwPortGroupConfigSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC *portGroupCfgPtr
);

/**
 * @internal cpssDxChIpfixFwPortGroupConfigGet function
 * @endinternal
 *
 * @brief  Request that all per tile IPFIX settings be returned via
 *         a PortGroupConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwPortGroupConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
);

/**
 * @internal cpssDxChIpfixFwEntryAdd function
 * @endinternal
 *
 * @brief   Manually learn a new IPFIX flow, specifying the EM entry index
 *          at which the flow was already learned into EM Table
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum  - device number
 * @param[in]  sCpuNum - service CPU number
 * @param[in]  flowId  - flow id. When hwAutoLearnEnable is enabled,
 *                       range must be maxFirmwareIndex+1 and maxIpfixIndex (included).
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 * @param[in]  entryParamsPtr - (pointer to) IPFIX entry input parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryAdd
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId,
    IN  CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC  *entryParamsPtr
);

/**
 * @internal cpssDxChIpfixFwEntryDelete function
 * @endinternal
 *
 * @brief   Request to delete an IPFIX entry
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum   - device number
 * @param[in]  sCpuNum  - service CPU number
 * @param[in]  flowId   - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryDelete
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
);

/**
 * @internal cpssDxChIpfixFwEntryDeleteAll function
 * @endinternal
 *
 * @brief   Request to delete ALL IPFIX entries in the device
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum          - device number
 * @param[in]  sCpuNum         - service CPU number
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryDeleteAll
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum
);

/**
 * @internal cpssDxChIpfixFwDataGet function
 * @endinternal
 *
 * @brief   Request to export the IPFIX data for a single entry.
 *          After calling this API, it is required to call the cpssDxChNetIfSdmaRxPacketGet
 *          API until the IPFIX data packet returned by FW is fetched.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum         - device number
 * @param[in]  sCpuNum        - service CPU number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId         - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGet
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
);

/**
 * @internal cpssDxChIpfixFwDataGetAll function
 * @endinternal
 *
 * @brief  Request to export the IPFIX data for a group of entries either once or periodically.
 *         It cancels the effect of any DataGetAllStop API invoked before.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum           - device number
 * @param[in]  sCpuNum          - service CPU number
 * @param[in]  flowType         - flow type. Type of entries that will be
 *                                exported by FW either once or periodically.
 * @param[in]  maxNumOfFlows    - maximum number of entries exported by FW.
 *                                When set to 0, there is no export limit.
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGetAll
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT    flowType,
    IN  CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT    maxNumOfFlows
);

/**
 * @internal cpssDxChIpfixFwIpcMsgFetch function
 * @endinternal
 *
 * @brief  Fetch, format and return several pending Tx IPC messages
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum              - device number
 * @param[in]  sCpuNum             - service CPU number
 * @param[in]  numOfMsgs           - number of messages to be fetched.
 * @param[out] numOfMsgsFetchedPtr - (pointer to) number of messages fetched.
 * @param[out] pendingMsgBuffSizePtr - (pointer to) Size in Bytes of the number of remaining messages
 *                                   in Tx IPC queue, which are pending to be fetched
 * @param[out] ipcMsgArrPtr        - (pointer to) fetched messages array.
 *                                   Application needs to provide enough memory
 *                                   i.e. (numOfMsgs * CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT)
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwIpcMsgFetch
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum,
    IN  GT_U32    numOfMsgs,
    OUT GT_U32    *numOfMsgsFetchedPtr,
    OUT GT_U32    *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   *ipcMsgArrPtr
);

/**
 * @internal cpssDxChIpfixFwExportConfigSet function
 * @endInternal
 *
 * @brief Set the IPFIX Data Export settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  exportCfgPtr - (pointer to) data export configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwExportConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC  *exportCfgPtr
);

/**
 * @internal cpssDxChIpfixFwExportConfigGet function
 * @endInternal
 *
 * @brief Request that IPFIX Data Export settings be returned via an ExportConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwExportConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
);

/**
 * @internal cpssDxChIpfixFwElephantConfigSet function
 * @endInternal
 *
 * @brief Set the Elephant Detection anf Report settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  elephantCfgPtr - (pointer to) elephant detection settings structure.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwElephantConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  *elephantCfgPtr
);

/**
 * @internal cpssDxChIpfixFwElephantConfigGet function
 * @endInternal
 *
 * @brief Request that Elephant Detection settings be returned via an ElephantConfigReturn
 *       IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwElephantConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
);

/**
 * @internal cpssDxChIpfixFwDataGetAllStop function
 * @endInternal
 *
 * @brief Request to stop the periodic IPFIX Data Export
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGetAllStop
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
);

/**
 * @internal cpssDxChIpfixFwDataClear function
 * @endInternal
 *
 * @brief Clear IPFIX counters of an entry, both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  flowId       - flow identifier.
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataClear
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId
);

/**
 * @internal cpssDxChIpfixFwDataClearAll function
 * @endInternal
 *
 * @brief Clear all IPFIX counters of all IPFIX entries that were allocated to IPFIX
 *        (whether they are active or not), both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataClearAll
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpfixFwh */
