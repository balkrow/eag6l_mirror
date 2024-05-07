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
* @file prvCpssDxChTam.h
*
* @brief CPSS definitions for configuring, gathering info and statistics
*        for the TAM (Telemetry Analytics and Monitoring) feature
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChTamh
#define __prvCpssDxChTamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/*********************************************************************
 * compilation time tunable resorces section
 * Restricted by overal size of memory
 */
#define PRV_CPSS_DXCH_TAM_FW_COUNTERS           1500/*128*/
#define PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS          8
#define PRV_CPSS_DXCH_TAM_FW_MICROBURSTS        8
#define PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS         8
#define PRV_CPSS_DXCH_TAM_FW_THRESHOLDS         16
/*********************************************************************/

/**
* @enum PRV_CPSS_DXCH_TAM_OP_CODE_ENT
*
* @brief OP codes for messages towards TAM feature.
*
*/
typedef enum
{
    /** init the TAM FW */
    PRV_CPSS_DXCH_TAM_OP_CODE_INIT_E = 1,

    /* Missing opcodes are used by TAM for other IPCs */
    /** Global Enable\Disable IFA. */
    PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_E = 10,

    /** Push Global Configuration to SCPU. */
    PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_SET_E,

    /** Push Port Configuration to SCPU. */
    PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_SET_E,

    /** IFA admin state get. */
    PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_GET_E,

    /** IFA Port Configuration from SCPU. */
    PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_GET_E,

    /** IFA Global Configuration from SCPU. */
    PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_GET_E,

} PRV_CPSS_DXCH_TAM_OP_CODE_ENT;

/* Message length definitions */
#define PRV_CPSS_DXCH_TAM_OP_CODE_INIT_MSG_LENGTH 1
#define PRV_CPSS_DXCH_TAM_OP_CODE_SNAPSHOT_SET_MSG_LENGTH 3
#define PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_MSG_LENGTH 2
#define PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_SET_MSG_LENGTH 5
#define PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_SET_MSG_LENGTH 1282

/* Return message length definitions */
#define PRV_CPSS_DXCH_TAM_OP_CODE_INIT_RET_MSG_LENGTH 12
#define PRV_CPSS_DXCH_TAM_OP_CODE_SNAPSHOT_SET_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_RETURN_MSG_LENGTH 2
#define PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_RETURN_MSG_LENGTH 5
#define PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_RETURN_MSG_LENGTH 6

/**
* @enum PRV_CPSS_DXCH_TAM_OP_CODE_ENT
*
* @brief Description: Return code due to messages towards TAM feature.
*
*/
typedef enum
{
    /** operation passed ok */
    PRV_CPSS_DXCH_TAM_RC_CODE_OK_E,

    /** generic failure */
    PRV_CPSS_DXCH_TAM_RC_CODE_FAIL_E,
    PRV_CPSS_DXCH_TAM_RC_CODE_NOT_INITIALIZED_E,
    PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_OP_CODE_E,

    /** wrong message parameters */
    PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_IN_PARAMS_E,

    /** wrong message size */
    PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_IN_SIZE_E

} PRV_CPSS_DXCH_TAM_RC_CODE_ENT;

/* Offset of shared memory (where the message data is expected to be written) */
/* from the CM3 SRAM end.*/
#define PRV_CPSS_DXCH_TAM_SHM_OFFSET_FROM_SRAM_END 0x1800 /*TBD also in FW*/

/* External remapping definitions */
/* FW cconfigures 4 windows to read Switch Core addresses from CM3 */
#define EXT_REMAP_BASE 0xa0100000
#define EXT_REMAP_WIN_0_BASE 0x20000000
#define EXT_REMAP_WIN_1_BASE 0x30000000
#define EXT_REMAP_WIN_2_BASE 0x24000000
#define EXT_REMAP_WIN_3_BASE 0x30100000

#define EXT_REMAP_WIN_0_MASK 0xFC000000
#define EXT_REMAP_WIN_1_MASK 0xFFF00000
#define EXT_REMAP_WIN_0_VAL 0x40000000

#define FALCON_EXT_REMAP_WIN_0_MASK 0xFFFE0000
#define FALCON_EXT_REMAP_WIN_0_VAL 0x18880000


#define EXT_REMAP_WIN_2_MASK 0xFC000000
#define EXT_REMAP_WIN_3_MASK 0xFFFF0000

#define EXT_REMAP_WIN_1_VAL 0x55000000
#define EXT_REMAP_WIN_2_VAL 0x18000000
#define EXT_REMAP_WIN_3_VAL 0x99000000


#define PRV_CPSS_DXCH_TAM_FW_BINS               4
#define PRV_CPSS_DXCH_TAM_FW_BIN_LEVELS         PRV_CPSS_DXCH_TAM_FW_BINS - 1

#define PRV_CPSS_DXCH_TAM_FW_CM3_CORE_CLOCK     600 /*TBD aldrin*/

#define PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS   16
#define PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS   19

/**
* @enum PRV_CPSS_DXCH_TAM_TRACKING_MODE_ENT
*
* @brief Snapshot Tracking modes
*/
typedef enum
{
    /* Includes all tracking modes */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_ALL_E,
    /** Current value tracking mode */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E,
    /** Peak value tracking mode */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_PEAK_E,
    /** Minimum value tracking mode */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_MINIMUM_E,
    /** Average value tracking mode */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_AVERAGE_E,
    /** Histogram tracking mode */
    PRV_CPSS_DXCH_TAM_TRACKING_MODE_HISTOGRAM_E
}PRV_CPSS_DXCH_TAM_TRACKING_MODE_ENT;

/**
* @struct PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC
*
* @brief Counter Statistics data base structure.
*        note: the statistics should be aligned with GT_U32
*/
typedef struct
{
    GT_U32 last;
    GT_U32 min;
    GT_U32 max;
    GT_U32 count;
    GT_U64 sum;

} PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC;
#define PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_SIZE_IN_WORDS      (sizeof(PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC) >> 2)

/**
* @struct PRV_CPSS_DXCH_TAM_COUNTER_DB_STC
*
* @brief Tam counter data base structure.
*/
typedef struct
{
    GT_U32                                      regAddr;
    PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC statData[2]; /** we have 2 stat buffers. work on 0 or 1 defined by statBufNum (in PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC) */
    GT_U32                                      nextCounter; /** pointer to the next counter in the counters list. value 0 means end of list. */
    GT_U32                                      cntrData;    /** if this place will be needed. can divide the nextCounter address by 4 like in snapshot db */
} PRV_CPSS_DXCH_TAM_COUNTER_DB_STC;
#define PRV_CPSS_DXCH_TAM_COUNTER_STATDATA_IN_WORDS     1

/**
* @struct PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC;
*
* @brief Tam snapshot data base structure.
*/
typedef struct
{
    GT_U16    firstCounter;      /** the first counter address, divided by 4, in counters list (PRV_CPSS_DXCH_TAM_COUNTER_DB_STC). value 0 means free snapshot. */
    GT_U8     statBufNum;        /** mention if we work on statistic data buffer 0 or 1. used for statData[] in struct PRV_CPSS_DXCH_TAM_COUNTER_DB_STC */
    GT_U8     trackingMode;      /** Snapshot Tracking Mode */

} PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC;

typedef enum
{
    PRV_CPSS_DXCH_TAM_MICROBURST_TIME_STATUS_WAIT_TO_A_E = 0, /** we wait that the counter will exceed the high watermark. so now we not inside the microburst */
    PRV_CPSS_DXCH_TAM_MICROBURST_TIME_STATUS_WAIT_TO_B_E = 1  /** the high watermark exceeded. so we inside the microburst.
                                                                  now wait that the counter will exceed low watermark */

} PRV_CPSS_DXCH_TAM_MICROBURST_TIME_STATUS_ENT;

/**
* @struct PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC;
*
* @brief Tam microburst data base structure.
*/
typedef struct
{
    GT_U16                                      counter;        /** pointer to counter address, devided by 4, that we calculate the microburst on it. value 0 means free microburst. */
    GT_U8                                       statBufNum;     /** mention if we work on buffer 0 or 1. used for statData[] in current struct */
    GT_U8                                       snapshotId;
    GT_U32                                      highWatermark;  /** A */
    GT_U32                                      lowWatermark;   /** B */
    GT_U32                                      timeStart;
    PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC statData[2];    /** we have 2 stat buffers. work on 0 or 1 defined by statBufNum (in current struct) */
    GT_U16                                      calcHistogram;  /** 1-the timeStatus is A and histogram need to be calculated. 0-histogram already calculated. */
    GT_U16                                      timeStatus;     /** see PRV_CPSS_DXCH_TAM_MICROBURST_TIME_STATUS_WAIT_TO_A_E */
    /*GT_U32                                      histogramPtr;*/

} PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC;

typedef enum
{
    PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_COUNTER_E = 0,
    PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_MICROBURST_E = 1

} PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_ENT;

/**
* @struct PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC
*
* @brief Counter histogarm data base structure.
*/
typedef struct
{
    GT_U16                               counter;                                   /** pointer to counter address, devided by 4, that we calculate the histogram on it */
    GT_U8                                statBufNum;                                /** mention if we work on buffer 0 or 1. used for binVal[] in current struct */
    GT_U8                                type;                                      /** mention if the histogram calculated on PRV_CPSS_DXCH_TAM_COUNTER_DB_STC or PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC */
    GT_U32                               binLevel[PRV_CPSS_DXCH_TAM_FW_BIN_LEVELS];
    GT_U64                               binVal[2][PRV_CPSS_DXCH_TAM_FW_BINS];      /** if (0x00000000 < counter value < binLevel[0]) => the counter value falls into binVal[][0]
                                                                                     ** if (binLevel[0] < counter value < binLevel[1]) => the counter value falls into binVal[][1]
                                                                                     ** if (binLevel[1] < counter value < binLevel[2]) => the counter value falls into binVal[][2]
                                                                                     ** if (binLevel[2] < counter value < 0xffffffff) => the counter value falls into binVal[][3] */
    GT_U32                               snapshotPtr;                               /** pointer to snapshot address, relevant just if type==PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_COUNTER_E */
    /*GT_U16                             dummy;*/                                   /* if this place will be needed. can divide the snapshot address by 4 like in others dbs */
} PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC;

/**
* @struct PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC
*
* @brief Counter threshold data base structure.
*/
typedef struct
{
    GT_U16 counter;      /** pointer to counter, that we calculate the threshold on it, divided by 4 */
    GT_U16 snapshotPtr;  /** pointer to snapshot address, divided by 4 */
    GT_U32 level;        /** if the counter exceeds this value the notification is occures */
    /* GT_U32 trackingMode; for now just "current" mode is supported so dont need this parameter */

} PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC;

/**
* @struct CPSS_DXCH_TAM_PID_STC
*
* @brief TAM Version parameters
*/
#define PID_LEN 12
typedef struct
{
    GT_CHAR productName[PID_LEN];         /** Package Name */
    GT_CHAR ppName[PID_LEN];              /** Packet Processor Name */
    GT_CHAR releaseVersionLabel[PID_LEN]; /** Release Version Label */
    GT_CHAR apiVersion[PID_LEN];          /** API Version */
}PRV_CPSS_DXCH_TAM_PID_STC;

/**
* @internal prvCpssDxChTamMessageSend function
* @endinternal
*
* @brief   Create the message toward the TAM feature and send it.
*
* @note   APPLICABLE DEVICES:      Aldrin2, Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in,out] messageDataPtr    - (pointer to) message content to send.
* @param[in,out] messageLengthPtr  - (pointer to) message content length.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS prvCpssDxChTamMessageSend
(
    IN      GT_U8   devNum,
    INOUT   GT_U32  *messageDataPtr,
    INOUT   GT_U32  *messageLengthPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __prvCpssDxChTamh */

