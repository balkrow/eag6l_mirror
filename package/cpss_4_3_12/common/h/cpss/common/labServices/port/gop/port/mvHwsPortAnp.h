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
* @file mvHwsPortAnp.h
*
* @brief Anp unit functions.
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortAnp_H
#define __mvHwsPortAnp_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/*global variables include and macros*/
#ifdef MV_HWS_REDUCED_BUILD
extern GT_BOOL mvHwsAnpUsedForStaticPort;
#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(_var,_value) ((_var) = (_value))
#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(_var) (_var)
#else
#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.labServicesDir.mvHwsAnpSrc._var,_value)

#define PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsAnpSrc._var)
#endif

#define MV_HWS_HAWK_ANP_NUM_CNS             14 /* 8(mac400) + 6(usx)*/
#define MV_HWS_HAWK_CLUSTER_NUM_CNS         4
#define MV_HWS_HARRIER_CLUSTER_NUM_CNS      3
#define MV_HWS_ANP_CMD_TABLE_SIZE_CNS       64
#define MV_HWS_ANP_CMD_TABLE_EMPTY_CNS      0xFFFFFFFF

/**
* @struct MV_HWS_HAWK_ANP_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    GT_U32      hwSerdesIdx; /* the serdes that connected to the anp can be changed by mux at init time */
    GT_U32      outSerdesIdx; /* the serdes that connected now to the anp changed by port enable/disable */
}MV_HWS_ANP_SD_MUX_STC;

/**
* @struct MV_HWS_HAWK_ANP_MISC_PARAM_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    GT_U8      interopLinkTimer; /*  */
}MV_HWS_ANP_MISC_PARAM_STC;

/**
*  @enum CPSS_PM_PORT_ANP_TRAINING_MODE
 *
 * @brief Available Training mode used for static port over ANP
 *
 */
typedef enum
{
    MV_HWS_ANP_PORT_TRX_TRAINING_E,
    MV_HWS_ANP_PORT_RX_TRAINING_E,
    MV_HWS_ANP_PORT_NO_TRAINING_E,

    MV_HWS_ANP_PORT_TRAINING_LAST_E

} MV_HWS_ANP_PORT_TRAINING_MODE_ENT;

/**
* @struct MV_HWS_HAWK_ANP_STC
*
* @brief Defines structure for anp parameters.
*/
typedef struct
{
    MV_HWS_ANP_SD_MUX_STC anpSdMuxDb[MV_HWS_HAWK_CLUSTER_NUM_CNS][MV_HWS_HAWK_ANP_NUM_CNS];
    MV_HWS_AP_CFG *anpPortParamPtr[HWS_MAX_DEVICE_NUM];
    GT_BOOL       *anpSkipResetPtr[HWS_MAX_DEVICE_NUM];
    GT_U32        *anpCmdIntLineIdxPtr[HWS_MAX_DEVICE_NUM];
    GT_BOOL       *anpIsStaticPortOverAnpPtr[HWS_MAX_DEVICE_NUM];
    MV_HWS_ANP_MISC_PARAM_STC *anpPortMiscParamDbPtr[HWS_MAX_DEVICE_NUM];
    GT_BOOL       *anpUseCmdTablePtr[HWS_MAX_DEVICE_NUM];
    MV_HWS_ANP_PORT_TRAINING_MODE_ENT *anpStaticOverAnpTrainingMode[HWS_MAX_DEVICE_NUM];
}MV_HWS_ANP_PORT_DATA_STC;

/*
 * ANP counters information:
 *    anRestartCounter  - Counts the number of times AN had restart, after got to resolution.
 *                        That is, link_fail_inhibit timer expired without link, or AN completed, and then link failed.
 *    dspLockFailCounter- Counts the number of times Channel SM waits for dsp_lock and gets timeout.
 *    linkFailCounter   - Counts the number of times Channel SM waits for link and gets timeout.
 *    txTrainDuration   - Holds the duration (clock cycles) of the last tx_train.
 *    txTrainFailCounter- Counts the number of times tx_train performed and completed with fail status.
 *    txTrainTimeoutCounter - Counts the number of times tx_train performed and got timeout.
 *    txTrainOkCounter  - Counts the number of times tx_train performed and completed without fail.
 *    rxInitOk  - Counts the number of times rx_init performed and finished with rx_init_done.
 *    rxInitTimeOut  - Counts the number of times rx_init performed and finished with timeout.
 *    rxTrainFailed  - Counts the number of times rx_train performed and completed with fail status.
 *    rxTrainOk  - Counts the number of times rx_train performed and completed without fail.
 *    rxTrainTimeOut  - Counts the number of times rx_train performed and got timeout.
*/
typedef struct
{
    GT_U32 anRestartCounter;
    GT_U32 dspLockFailCounter;
    GT_U32 linkFailCounter;
    GT_U32 txTrainDuration;
    GT_U32 txTrainFailCounter;
    GT_U32 txTrainTimeoutCounter;
    GT_U32 txTrainOkCounter;
    GT_U32 rxInitOk;
    GT_U32 rxInitTimeOut;
    GT_U32 rxTrainFailed;
    GT_U32 rxTrainOk;
    GT_U32 rxTrainTimeOut;
    GT_U32 rxTrainDuration;

}MV_HWS_ANP_PORT_COUNTERS;

/*
 * ANP SM information:
 *    chSmState  - Channel state machine.
 *    txSmState  - Tx state machine.
 *    rxSmState  - Rx state machine.
*/
typedef struct
{
    GT_U32 chSmState;
    GT_U32 txSmState;
    GT_U32 rxSmState;

}MV_HWS_ANP_PORT_SM;

/*
 * ANP info struct:
 *    portConters - Anp counters.
 *    portSm      - Anp state machine.
*/
typedef struct
{
    MV_HWS_ANP_PORT_COUNTERS portConters;
    MV_HWS_ANP_PORT_SM       portSm;
}MV_HWS_ANP_INFO_STC;

/**
* @internal mvHwsAnpClose function
* @endinternal
*
* @brief  ANP unit close
*
* @param[in] devNum                - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpClose
(
    IN GT_U8              devNum
);

/**
* @internal mvHwsAnpInit function
* @endinternal
*
* @brief  ANP unit init
*
* @param[in] devNum                - system device number
* @param[in] skipWritingToHW       - skip writing to HW
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
);

/**
* @internal mvHwsAnpSerdesSdwMuxSet function
* @endinternal
*
* @brief  ANP unit enable SerDes control
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - system SerDes number
* @param[in] anpEnable - true  - SerDes is controlled by ANP
*                      - false - SerDes is controlled by regs
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
);

/**
* @internal mvHwsAnpPortEnable function
* @endinternal
*
* @brief  ANP unit port enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - system port number
* @param[in] portMode              - configured port mode
* @param[in] apEnable              - apEnable flag
* @param[in] enable                - True = enable/False =
*       disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
);

/**
* @internal mvHwsAnpPortReset function
* @endinternal
*
* @brief   reset anp machine
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] reset              - reset parameter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortReset
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_BOOL              reset
);

/**
* @internal mvHwsAnpPortStart function
* @endinternal
*
* @brief   Port anp start (set capabilities and start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStart
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_AP_CFG       *apCfgPtr
);

/**
* @internal mvHwsAnpCfgPcsDone function
* @endinternal
*
* @brief   config psc done after pcs configuration.
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpCfgPcsDone
(
    IN GT_U8                    devNum,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode
);

/**
* @internal mvHwsAnpPortStopAn function
* @endinternal
*
* @brief   Port anp stop autoneg
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortStopAn
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortInteropGet function
* @endinternal
*
* @brief   Return ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
);

/**
* @internal mvHwsAnpPortLinkTimerSet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  timeInMsec
);

/**
* @internal mvHwsAnpPortLinkTimerGet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAnpPortLinkTimerGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *timeInMsecPtr
);

/**
* @internal mvHwsAnpPortStatusGet function
* @endinternal
*
* @brief   Returns the AP port resolution information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatusPtr                 - AP status parameters
*
* @param[out] apStatus                 - AP/HCD results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatusGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_STATUS  *apStatusPtr
);

/**
* @internal mvHwsAnpPortResolutionBitSet function
* @endinternal
*
* @brief   set resolution bit per port mode
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] anResBit                 - resolution bit shift
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortResolutionBitSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  anResBit
);

/**
* @internal mvHwsPortAnpConfigGet function
* @endinternal
*
* @brief   Returns the ANP port configuration.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @param[out] apCfgPtr             - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAnpConfigGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_CFG       *apCfgPtr
);

/**
* @internal mvHwsAnpPortStatsGet function
* @endinternal
*
* @brief   Returns the ANP port statistics information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] apStats              - AP statistics parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsGet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_AP_PORT_STATS *apStats
);

/**
* @internal mvHwsAnpPortStatsReset function
* @endinternal
*
* @brief   Reset ANP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - AP port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortStatsReset
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortInteropSet function
* @endinternal
*
* @brief   Set ANP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] apInteropPtr             - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInteropSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_AP_PORT_INTROP  *apInteropPtr
);

/**
* @internal mvHwsAnpPortInfoGet function
* @endinternal
*
* @brief   Get ANP port information
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[out] anpInfoPtr           - anp info
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInfoGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    OUT MV_HWS_ANP_INFO_STC      *anpInfoPtr
);

/**
* @internal mvHwsAnpCmdTableClear function
* @endinternal
*
* @brief   clear anp cmd table
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTableClear
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsAnpPortSkipResetSet function
* @endinternal
*
* @brief   set skip reset value (prevent from delete port to
*          reset ANP)
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortSkipResetSet (
    GT_U8                       devNum,
    GT_U32                      portNum,
    GT_BOOL                     skip
);

/**
* @internal mvHwsAnpPortParallelDetectInit function
* @endinternal
*
* @brief   Init sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectInit
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpPortParallelDetectReset function
* @endinternal
*
* @brief   Reset sequence for parallel detect
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectReset
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpParallelDetectWaitForPWRUPprog function
* @endinternal
*
* @brief   Wait for CH FSM to reach PWRUP prog state
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectWaitForPWRUPprog
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum
);

/**
* @internal mvHwsAnpPortParallelDetectAutoNegSet function
* @endinternal
*
* @brief   Set AN37
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] autoNegEnabled        - enable/disable AN37
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAutoNegSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  autoNegEnabled
);

/**
* @internal mvHwsAnpPortParallelDetectOWLinkStatus function
* @endinternal
*
* @brief enable to override link_status from PCS value
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] owLinkStatus          - true/false if override is
*       enabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectOWLinkStatus
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  owLinkStatus
);

/**
* @internal mvHwsAnpPortParallelDetectAN37Seq function
* @endinternal
*
* @brief   Check peer side AN37 status seq.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] startSeq              - start or end seq
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortParallelDetectAN37Seq
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  startSeq
);

/**
 * @internal mvHwsAnpSetStaticPort
 * @endinternal
 *
 * @brief
 *
 * @param devNum
 * @param portNum
 * @param isStaticOverAnp
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAnpSetStaticPort
(
    IN GT_U8        devNum,
    IN GT_U32       portNum,
    IN GT_BOOL      isStaticOverAnp
);

/**
 * @internal mvHwsAnpGetStaticPort
 * @endinternal
 *
 * @brief
 *
 *
 * @param devNum
 * @param portNum
 * @param isANPStaticPtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAnpGetStaticPort
(
    IN  GT_U8        devNum,
    IN  GT_U32       portNum,
    OUT GT_BOOL      *isANPStaticPtr
);

/**
 * @internal mvHwsAnpFieldSet
 * @endinternal
 *
 * @brief
 *
 *
 * @param devNum
 * @param hawkField
 * @param harrierField
 * @param phoenixField
 *
 * @return GT_U32
 */
GT_U32 mvHwsAnpFieldSet
(
    IN GT_U8    devNum,
    IN GT_U32   hawkField,
    IN GT_U32   harrierField,
    IN GT_U32   phoenixField
);

/**
 * @internal mvHwsAnpPortCountersGet
 * @endinternal
 *
 * @brief
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param anpCounters
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAnpPortCountersGet
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum,
    MV_HWS_ANP_PORT_COUNTERS *anpCounters
);

/**
 * @internal mvHwsAnpCmdRegisterWrite
 * @endinternal
 *
 * @brief
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param serdesSpeed
 * @param numOfLanes
 * @param progState
 * @param apMode
 * @param addr
 * @param data
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAnpCmdRegisterWrite
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    MV_HWS_SERDES_TYPE   serdesType,
    GT_U32               numOfLanes, /*1/2/4/8/FF - R1/R2/R4/R8/ALL*/
    GT_U32               progState,
    GT_U32               apMode, /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               addr,
    GT_U32               data,
    GT_U32               mask
);

GT_STATUS mvHwsAnpCmdRegisterWriteExt
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    MV_HWS_PORT_STANDARD portMode,
    GT_U32               progState,
    GT_U32               apMode, /* 0-non ap, 1- ap, ff - dont care */
    GT_U32               addr,
    GT_U32               data,
    GT_U32               mask
);

GT_STATUS mvHwsAnpCmdInterfaceSetProg
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    GT_BOOL              enable,
    GT_U32               progState,  /*in disable we delete all progs*/
    GT_U8                progSetMode
);

/**
* @internal mvHwsAnpCmdRxOverride function
* @endinternal
*
* @brief  write all rx parametrs to anp command table
*
* @param[in] devNum        - system device number
* @param[in] phyPortNum    - port number
* @param[in] serdesSpeed   - serdes speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdRxOverrideParam
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    MV_HWS_PORT_STANDARD portMode,
    GT_U32               paramType,  /*parameter type*/
    GT_U32               valMinMax,  /*0 -init val, 1-min val, 2- max val*/
    GT_U32               paramValue /*param value*/

);

/**
* @internal mvHwsAnpCmdTxOverride function
* @endinternal
*
* @brief  write all tx parametrs to anp command table
*
* @param[in] devNum        - system device number
* @param[in] phyPortNum    - port number
* @param[in] serdesSpeed   - serdes speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpCmdTxOverrideParams
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    MV_HWS_SERDES_SPEED  serdesSpeed,
    MV_HWS_PORT_STANDARD portMode,
    GT_U8 minPre2,
    GT_U8 maxPre2,
    GT_U8 minPre,
    GT_U8 maxPre,
    GT_U8 minMain,
    GT_U8 maxMain,
    GT_U8 minPost,
    GT_U8 maxPost
);

GT_STATUS mvHwsPortAnpSerdesRxConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *rxConfigPtr
);
GT_STATUS mvHwsPortAnpSerdesTxConfigSet
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *txConfigPtr
);

GT_STATUS mvHwsAnpUseCmdTableSet
(
    IN GT_U8        devNum,
    IN GT_U32       portNum,
    IN GT_BOOL      useCmdTable
);

GT_BOOL mvHwsAnpGetUseCmdTable
(
    IN GT_U8        devNum,
    IN GT_U32       portNum
);

GT_STATUS mvHwsAnpNextPortShiftGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_U8                   laneIdx,
    OUT GT_U32                  *nextPortShift

);

/**
 * @internal mvHwsAnpSetStaticPortTrainingMode
 * @endinternal
 *
 * @brief
 *
 *
 * @param devNum
 * @param portNum
 * @param training mode
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAnpSetStaticPortTrainingMode
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  portNum,
    IN  MV_HWS_ANP_PORT_TRAINING_MODE_ENT       trainingMode
);

/**
* @internal mvHwsAnpPortTxPostAdaptSet function
* @endinternal
*
* @brief   Enable/Disable Tx post-tap adaptation bit in SerDes,
*          using the ANP command-interface
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] portMode              - configured port mode
* @param[in] enable                - enable/disable tx adapt bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortTxPostAdaptSet
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum,
    MV_HWS_PORT_STANDARD     portMode,
    GT_BOOL                  enable
);

GT_STATUS mvHwsAnpPortDebugModeSet
(
    GT_U8                devNum,
    GT_U32               phyPortNum,
    GT_BOOL              enable,
    GT_BOOL              allPorts

);

/**
* @internal mvHwsAnpPortInfoDump function
* @endinternal
*
* @brief   Get ANP port and ANP registers information to desire output: terminal/printf, cpssLog, cpssOsLog.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] output                - desired output type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortInfoDump
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      phyPortNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT              output
);

/**
* @internal mvHwsAnpPortDump function
* @endinternal
*
* @brief  Print ANP port information desire output
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] buffer                - buffer of output information
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortDump
(
    IN  GT_U8                                      devNum,
    IN  GT_U32                                     phyPortNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT             output
);

/**
* @internal mvHwsAnpPortRegDump function
* @endinternal
*
* @brief   Print ANP registers information to desire output
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port number
* @param[in] output                - output type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAnpPortRegDump
(
    IN  GT_U8                                      devNum,
    IN  GT_U32                                     phyPortNum,
    IN  MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT             output
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortAnp_H */
