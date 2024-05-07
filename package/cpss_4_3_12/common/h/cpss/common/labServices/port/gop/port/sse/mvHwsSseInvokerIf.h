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
* @file mvHwsSseInvokerIf.h
*
* @brief   Functions definition for Sub Sequence Invoker
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsSseInvokerIf_H
#define __mvHwsSseInvokerIf_H

#include <cpss/common/labServices/port/gop/port/sse/mvHwsSseCommon.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ARGS                (10)

typedef GT_STATUS (*MV_HWS_SSE_INVOKER_SERDES_WRITE_REG_FUNC_PTR)
(
    IN  GT_U8       devNum,
    IN  GT_UOPT     portGroup,
    IN  GT_UOPT     serdesNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

typedef GT_STATUS (*MV_HWS_SSE_INVOKER_SERDES_READ_REG_FUNC_PTR)
(
    IN  GT_U8       devNum,
    IN  GT_UOPT     portGroup,
    IN  GT_UOPT     serdesNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32      *data
);


typedef struct
{
    MV_HWS_SSE_INVOKER_SERDES_READ_REG_FUNC_PTR     readRegFuncPtr;
    MV_HWS_SSE_INVOKER_SERDES_WRITE_REG_FUNC_PTR    writeRegFuncPtr;
}MV_HWS_SSE_INVOKER_SERDES_RD_WR_CALLBACKS;

/**************************** APIs decleration***********************************
*/

/**
* @internal  mvHwsSseInvokerInit  function
* @endinternal
*
* @brief  Initialize Sub Seqence Invoker
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS mvHwsSseInvokerInit
(
    IN GT_VOID
);

/**
* @internal  mvHwsSseInvokerDestroy  function
* @endinternal
*
* @brief  Free Sub Seqence Invoker internal resources
*
*/
GT_VOID mvHwsSseInvokerDestroy
(
    IN GT_VOID
);

/**
* @internal  mvHwsSseInvokerRegistration  function
* @endinternal
*
* @brief  provide registration for read and write register
*         functions that will be used during invokation
*
* @param[in] readRegFuncPtr           - read register callback
*
* @param[in] writeRegFuncPtr          - write register callback
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS mvHwsSseInvokerRegistration
(
    IN SSE_LUT_TYPE  levelType,
    IN GT_VOID_PTR   rdWrCbsObj
);

/**
* @internal  mvHwsSseInvokerScriptLoadedStatus function
* @endinternal
*
* @brief  Deletes key attribute object from a LUT of a specific
*         type (i.e. SERDES\PCS\MAC).
*
*
* @param[in] scriptStatus           - Indicates whether
*                                     the script file was loaded
*                                     successfully or not
*
* @retval None
*/
GT_VOID mvHwsSseInvokerScriptLoadedStatus
(
    IN  GT_BOOL scriptStatus
);

/**
* @internal  mvHwsSseInvokerEnablePrints function
* @endinternal
*
* @brief  Enables \ disables invoker module prints
*
*
* @param[in] enablePrints           - Indicates whether the
*                                     prints should be enabled
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsSseInvokerEnablePrints
(
    IN  GT_BOOL enablePrints
);

/**
* @internal  mvHwsSseInvokerSerdesResetSeq function
* @endinternal
*
* @brief  Run sequence before or after clearing serdes registers
*
* @param[in] orderType                - Specifies the order type
*                                       (i.e. pre or post)
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] serdesType               - serdes type
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsSseInvokerSerdesResetSeq
(
    IN SSE_ORDER_TYPE       orderType,
    IN GT_U8                devNum,
    IN GT_UOPT              portGroup,
    IN GT_UOPT              serdesNum,
    IN GT_BOOL              analogReset,
    IN GT_BOOL              digitalReset,
    IN GT_BOOL              syncEReset
);

/**
* @internal  mvHwsSseInvokerSerdesPowerUpArrayCtrlSeq function
* @endinternal
*
* @brief  Run sequence before or after physical port
*         initialization
*
* @param[in] orderType                - Specifies the order type
*                                       (i.e. pre or post)
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      serdesType - serdes type
* @param[in] numOfSer                 - number of SERDESes to configure
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] powerUp                  - true for power UP
* @param[in] serdesConfigPtr          - serdesConfigPtr
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - 10BIT mode (enable/disable)
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsSseInvokerSerdesPowerUpArrayCtrlSeq
(
    IN SSE_ORDER_TYPE               orderType,
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       numOfSer,
    IN GT_U32                       *serdesArr,
    IN GT_BOOL                      powerUp,
    IN MV_HWS_SERDES_CONFIG_STC     *serdesConfigPtr
);

/**
* @internal  mvHwsSseInvokerSerdesResetSeq function
* @endinternal
*
* @brief  Run sequence before or after controlling both TX & Rx
*         training starting.
*
* @param[in] orderType                - Specifies the order type
*                                       (i.e. pre or post)
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] rxTraining               - Rx Training (true/false)
*                                       txTraining - Tx Training
*                                       (true/false)
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsSseInvokerRxAutoTuneStartSeq
(
    IN SSE_ORDER_TYPE       orderType,
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               serdesNum,
    IN MV_HWS_SERDES_TYPE   serdesType,
    IN GT_BOOL              rxTraining
);


GT_STATUS mvHwsSseInvokerSerdesOperation
(
   IN SSE_ORDER_TYPE                orderType,
   IN GT_U8                         devNum,
   IN GT_U32                        portGroup,
   IN GT_U32                        serdesNum,
   IN MV_HWS_PORT_STANDARD          portMode,
   IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
   IN GT_U32                        *dataPtr,
   OUT GT_U32                       *resultPtr
);

/******************************************************************************
*                       Sanity Test section                                   *
*******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* __mvHwsSseInvokerIf_H */


