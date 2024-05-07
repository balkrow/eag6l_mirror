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
* @file mvHwsMtiCpuPcsRev2If.c
*
* @brief MTI CPU PCS interface API
*
* @version   1
********************************************************************************
*/
#ifndef MICRO_INIT
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#else
#include <cpssCommon/cpssPresteraDefs.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMacCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcsCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtipCpuExtUnits.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
static char* mvHwsMtiCpuPcsRev2TypeGetFunc(void)
{
  return "MTI_CPU_PCS_REV2";
}

/**
* @internal mvHwsMtiCpuPcsRev2LoopBack function
* @endinternal
*
* @brief   Set PCS loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_CPU_PCS_UNITS_FIELDS_E pcsField;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = CPU_PCS_UNITS_CONTROL1_PORT_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, data, NULL));

    extField = MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}


/**
* @internal mvHwsMtiCpuPcsRev2Reset function
* @endinternal
*
* @brief   Set MTICpu PCS RESET/UNRESET or FULL_RESET
*          action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      pcsNum;
    MV_HWS_CPU_PCS_UNITS_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsNum = curPortParams.portPcsNumber;

    pcsField = CPU_PCS_UNITS_CONTROL1_PORT_RESET_E;

    if (POWER_DOWN == action || RESET == action)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_CPU_PCS_UNIT, pcsField, 0, NULL));
    }
    else if (UNRESET == action)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_CPU_PCS_UNIT, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2LoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_CPU_PCS_UNITS_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = CPU_PCS_UNITS_CONTROL1_PORT_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;

    return GT_OK;
}

/**
 * @internal mvHwsMtiCpuPcsRxResetGet function
 * @endinternal
 *
 * @brief   Get the PCS sd_rx_reset state.
 *
 * @param devNum
 * @param portGroup
 * @param phyPortNum
 * @param portMode
 * @param state
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsMtiCpuPcsRxResetGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_RESET            *state
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E pcsField;

    if(state == NULL)
    {
        return GT_BAD_PTR;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = MTIP_CPU_EXT_UNITS_RESET_SD_RX_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, &data, NULL));

    *state = (data != 0) ? UNRESET : RESET;

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPCS100.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
* @param[in] portMode                 - port mode
*
* @param[out] lockPtr                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E extField;


    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    extField = MTIP_CPU_EXT_UNITS_STATUS_LINK_STATUS_E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, extField, &data, NULL));

    *lockPtr = (data & 0x1);

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2Mode function
* @endinternal
*
* @brief   Set the MtiCpuPcs mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr

)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_CPU_PCS_UNITS_FIELDS_E pcsField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 markerInterval, noRsfec;
    MV_HWS_PORT_FEC_MODE    portFecMode;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    /* I: m_RAL.PORT_VENDOR_PCS_MODE.Hi_ber25.set(miiType == _25GMII); */
    pcsField = CPU_PCS_UNITS_VENDOR_PCS_MODE_HI_BER25_E;
    /**
     * When 1 PCS implements 25G Hi-Ber (2ms, 97 errors). When 0,
     * Hi-Ber implements according to Ena_clause49
     */
    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, (_25GMII_E == miiType), NULL));

    /* II: m_RAL.PORT_VENDOR_PCS_MODE.Ena_clause49.set(1); */
    pcsField = CPU_PCS_UNITS_VENDOR_PCS_MODE_ST_ENA_CLAUSE49_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, 1, NULL));

    /* III: m_RAL.PORT_VENDOR_PCS_MODE.Disable_mld.set(noRsfec); */
    pcsField = CPU_PCS_UNITS_VENDOR_PCS_MODE_DISABLE_MLD_E;
    noRsfec = (portFecMode != RS_FEC)? 1: 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, noRsfec, NULL));

    /* IV: if (MarkerInterval != 0) m_RAL.PORT_VL_INTVL.alignment_markers_interval.set(MarkerInterval); */
    CHECK_STATUS(mvHwsMarkerIntervalCalc(devNum, portMode, curPortParams.portFecMode, &markerInterval));
    if (markerInterval != 0){
        pcsField = CPU_PCS_UNITS_VL_INTVL_ALIGNMENT_MARKERS_INTERVAL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_PCS_UNIT, pcsField, markerInterval, NULL));
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiCpuPcsRev2CheckGearBox function
* @endinternal
*
* @brief   check if gear box is locked on MtiCpuPcs.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] lockPtr                 - pointer lock value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2CheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    GT_U32                              pcsField;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_UREG_DATA                        data;
    MV_HWS_UNITS_ID                     unitId;

    if(lockPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for RS_FEC modes 'block_lock' field is not relevant */
    if(curPortParams.portFecMode == RS_FEC)
    {
        pcsField = CPU_PCS_RSFEC_UNITS_RSFEC_STATUS_AMPS_LOCK_E;
        unitId = MTI_CPU_RSFEC_UNIT;
    }
    else
    {
        pcsField = CPU_PCS_UNITS_BASER_STATUS1_BLOCK_LOCK_E;
        unitId = MTI_CPU_PCS_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, curPortParams.portPcsNumber, unitId, pcsField, &data, NULL));

    *lockPtr = (data != 0);

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2FecConfigGet function
* @endinternal
*
* @brief   Return the FEC mode  status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    return  mvHwsExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiCpuLowSpeedPcsRxReset function
* @endinternal
*
* @brief   CPU EXT PCS SD RX RESET/UNRESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_UREG_DATA data;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    switch (action) {
    case RESET:
        data = 0;
        break;
    case UNRESET:
        data = 1;
        break;
    default:
        return GT_OK;
    }

    /* Reset/Unreset PMA Channel RX */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_EXT_UNIT, MTIP_CPU_EXT_UNITS_RESET_SD_RX_RESET__E, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2SendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsRev2SendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E extField;

    portGroup = portGroup;
    portMode = portMode;

    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 1;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }

    extField = MTIP_CPU_EXT_UNITS_CONTROL_TX_REM_FAULT_E;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_CPU_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsRev2IfInit function
* @endinternal
*
* @brief   Init MtiCpuPcs configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiCpuPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_CPU_PCS])
    {
        funcPtrArray[MTI_CPU_PCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_CPU_PCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_CPU_PCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_CPU_PCS]->pcsResetFunc        = mvHwsMtiCpuPcsRev2Reset;
    funcPtrArray[MTI_CPU_PCS]->pcsRxResetFunc      = mvHwsMtiCpuPcsRxReset;
    funcPtrArray[MTI_CPU_PCS]->pcsRxResetGetFunc   = mvHwsMtiCpuPcsRxResetGet;
    funcPtrArray[MTI_CPU_PCS]->pcsModeCfgFunc      = mvHwsMtiCpuPcsRev2Mode;
    funcPtrArray[MTI_CPU_PCS]->pcsLbCfgFunc        = mvHwsMtiCpuPcsRev2LoopBack;
    funcPtrArray[MTI_CPU_PCS]->pcsLbCfgGetFunc     = mvHwsMtiCpuPcsRev2LoopBackGet;
    funcPtrArray[MTI_CPU_PCS]->pcsTypeGetFunc      = mvHwsMtiCpuPcsRev2TypeGetFunc;
    funcPtrArray[MTI_CPU_PCS]->pcsCheckGearBoxFunc = mvHwsMtiCpuPcsRev2CheckGearBox;
    funcPtrArray[MTI_CPU_PCS]->pcsFecCfgGetFunc    = mvHwsMtiCpuPcsRev2FecConfigGet;
    funcPtrArray[MTI_CPU_PCS]->pcsAlignLockGetFunc = mvHwsMtiCpuPcsRev2AlignLockGet;
    funcPtrArray[MTI_CPU_PCS]->pcsSendFaultSetFunc = mvHwsMtiCpuPcsRev2SendFaultSet;

    return GT_OK;
}

