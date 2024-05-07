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
* @file mvHwsIronmanPortAnp.h
*
* @brief Anp unit functions.
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsIronmanPortAnp_H
#define __mvHwsIronmanPortAnp_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>

/**
* @internal mvHwsIronmanAnpInit function
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
GT_STATUS mvHwsIronmanAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
);

/**
* @internal mvHwsIronmanAnpSerdesSdwMuxSet function
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
GT_STATUS mvHwsIronmanAnpSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
);

/**
* @internal mvHwsIronmanAnpPortEnable function
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
GT_STATUS mvHwsIronmanAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
);

/**
* @internal mvHwsIronmanAnpPortReset function
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
GT_STATUS mvHwsIronmanAnpPortReset
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_BOOL              reset
);

/**
* @internal mvHwsIronmanAnpPortStart function
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
GT_STATUS mvHwsIronmanAnpPortStart
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_U32               capability,
    GT_U32               options
);

GT_STATUS mvHwsIronmanAnpSetDefaultLinkTimer
(
    GT_U8  devNum,
    GT_U32 port,
    GT_U32 capability
);

GT_STATUS mvHwsIronmanAnpSetIntropLinkTimer
(
    GT_U8  devNum,
    GT_U32 port,
    GT_U32 capability,
    GT_U32 pdTimer,
    GT_U32 apTimer

);

/**
* @internal mvHwsIronmanAnpPortStopAn function
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
GT_STATUS mvHwsIronmanAnpPortStopAn
(
    GT_U8                devNum,
    GT_U32               phyPortNum
);

/**
* @internal mvHwsIronmanAnpPortLinkTimerSet function
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
GT_STATUS mvHwsIronmanAnpPortLinkTimerSet
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
GT_STATUS mvHwsIronmanAnpPortLinkTimerGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *timeInMsecPtr
);

#if 0
GT_STATUS mvHwsAnpNextPortShiftGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_U8                   laneIdx,
    OUT GT_U32                  *nextPortShift

);
#endif

/**
* @internal mvHwsAnpPortSignalDetectMask function
* @endinternal
*
* @brief   Mask signal detect.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] startSeq              - start or end seq
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* • For link up the following settings should be made:
* (sq_detected_lpf_ow) Port<%n> Control4[6] to 1
* (sq_detected_lpf_ow_val) Port<%n> Control4[7] to 0 (default)
*
* • For link down the following settings should be made:*
* (sq_detected_lpf_ow) Port<%n> Control4[6] to 0
*
*/
GT_STATUS mvHwsIronmanAnpPortSignalDetectHwMask
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  mask
);

typedef enum
{
    MV_HWS_ANP_AN_OPER_DISABLE        = 0x0,
    MV_HWS_ANP_AN_OPER_ENABLE         = 0x1,
    MV_HWS_ANP_AN_OPER_RESTART        = 0x2,

}MV_HWS_ANP_AN_OPER;

typedef enum
{
    MV_HWS_ANP_TX_ONLY                = 0x0,
    MV_HWS_ANP_RX_ONLY                = 0x1,
    MV_HWS_ANP_TXRX                   = 0x2,

}MV_HWS_ANP_TXRX_MODE;

/**
* @internal mvHwsIronmanAnpPortPcsSdTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset PCS sd TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsSdTxRxReset
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             reset
);

/**
* @internal mvHwsIronmanAnpPortPcsSdTxRxClockGate function
* @endinternal
*
* @brief   Gate/Un-gate PCS sd TX/RX clock
*          TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsSdTxRxClockGate
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             gate
);

/**
* @internal mvHwsIronmanAnpPortAnArbStateGet function
* @endinternal
*
* @brief   get AN ARB SM
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnArbStateGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_AP_SM_STATE      *anSmState
);

/**
* @internal mvHwsIronmanAnpPortAnTxRxClockGate function
* @endinternal
*
* @brief   Gate/Un-gate AN TX/RX clock
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnTxRxClockGate
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 gate,
    MV_HWS_ANP_TXRX_MODE    txrx
);

/**
* @internal mvHwsIronmanIronmanAnpPortAnTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset AN TX/RX
*          TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnTxRxReset
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 reset,
    MV_HWS_ANP_TXRX_MODE    txrx

);

/**
* @internal mvHwsIronmanAnpPortPcsTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset PCS TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsTxRxReset
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             reset
);

/**
* @internal mvHwsIronmanAnpPortAnPcsMuxSet function
* @endinternal
*
* @brief   Set AN/PCS mux
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnPcsMuxSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             pcs
);

/**
* @internal mvHwsIronmanAnpPortANPLinkStatusSet function
* @endinternal
*
* @brief   Overwrite link_status value in ANP (assume
*          IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_E
*          is set on init seq
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortANPLinkStatusSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             linkUp
);

/**
* @internal mvHwsIronmanAnpPortAnSet function
* @endinternal
*
* @brief   Enable/Disable/Restart AN
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_ANP_AN_OPER  oper
);


/**
* @internal mvHwsIronmanAnpPortHCDGet function
* @endinternal
*
* @brief   return HCD results
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

GT_STATUS mvHwsIronmanAnpPortHCDGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    GT_U32                  *apHcdStatus
);

GT_STATUS mvHwsIronmanAnpPortRegDump
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortAnp_H */

