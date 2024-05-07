/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChHwInitSoftRest.c
*       Internal function used for Soft Reset testing
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitSoftReset.h>
#include <cpss/common/config/private/prvCpssCommonRegs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* for SIP6 The prvCpssDrvHwPpApi.c functions         */
/* subsitute CPSS_HW_DRIVER_AS_SWITCHING_E instead of */
/* CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E and  */
/* CPSS_HW_DRIVER_AS_DFX_E that is the same value 8   */
/* So DFX registers can be accessed using function    */
/* prvCpssDrvHwPpPortGroupWriteRegister and other     */
/* Switching core oriented functions                  */
/* see                                                */
/* prvCpssDrvHwPpWriteRegBitMaskDrv and               */
/* prvCpssDrvHwPpDoReadOpDrv                          */

static const int prvCpssDxChHwInitSoftResetTraceEnable = 0;
#ifdef SOFT_RESET_DEBUG
int prvCpssDxChHwInitSoftResetTraceEnableSet(int enable)
{
    prvCpssDxChHwInitSoftResetTraceEnable = enable;
    return 0;
}
#endif /* SOFT_RESET_DEBUG */

/**
* @internal prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet function
* @endinternal
*
* @brief   Get address of register from Skip Initialization Marix by type.
*          For SIP6 this address of the register in DFX of Tile0 Eagle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC5; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @param[out] regAddrPtr              - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device - not supported DFX server or
*                                    specified type of Skip Initialization Marix
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *regAddrPtr
)
{
    GT_U32 regAddr;
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SRR_LOAD_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SRRSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_BIST_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.BISTSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SOFT_REPAIR_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SoftRepairSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGS_GENX_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RegxGenxSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_MULTI_ACTION_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.MultiActionSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DEVICE_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DeviceEEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_D2D_LINK_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.D2DLinkSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXRegistersSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_PIPE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.coreSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CNM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.cnmSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_RavenSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_MainSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_CNMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_GWSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.poeSkipInitializationMatrix;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert function
* @endinternal
*
* @brief   Convert SW type of Skip Initialization Domain to sequence of HW types.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] index                    - index in the type sequence.
*
* @param[out] hwSkipTypePtr           - pointer to HW Skip Type
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @retval GT_OK                    - on success,
* @retval GT_NO_MORE               - index is out of sequence length
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert
(
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT                  skipType,
    IN  GT_U32                                          index,
    OUT  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  *hwSkipTypePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(hwSkipTypePtr);

    switch (skipType)
    {
        case CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:
            switch (index)
            {
                case 0:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E;
                    break;
                case 1:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E;
                    break;
                default: return /* not error to LOG */ GT_NO_MORE;
            }
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_POE_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert function
* @endinternal
*
* @brief   Get address of register instance in given SIP6 Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] portGroupPtr         - pointer to port group address
* @param[out] regAddrPtr           - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert
(
    IN  GT_U8                                 devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT dfxType,
    IN  GT_U32                                tileIndex,
    IN  GT_U32                                gopIndex,
    IN  GT_U32                                regAddr,
    OUT GT_U32                                *portGroupPtr,
    OUT GT_U32                                *regAddrPtr
)
{
    GT_U32        dfxAddressMask   = 0x000FFFFF;
    GT_U32        dfxInGopLocaBase = 0x00700000;
    GT_U32        relativeDfxBase;
    GT_BOOL       error;

    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DEV_USER_SOFT_RESET_SUPPORTED_DEVICES_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (tileIndex > 0/* support AC5P */ && tileIndex >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (gopIndex >= FALCON_RAVENS_PER_TILE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *portGroupPtr = 0;
    switch (dfxType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E:
            /* Processing Core DFX */
            dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
            relativeDfxBase =
                (dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl
                 & (~ dfxAddressMask));
            *regAddrPtr =
                (relativeDfxBase | (regAddr & dfxAddressMask))
                + prvCpssSip6TileOffsetGet(devNum, tileIndex);
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E:
            /* GOP DFX */
            relativeDfxBase =
                (prvCpssDxChHwUnitBaseAddrGet(
                    devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + gopIndex), &error)
                 + dfxInGopLocaBase);
            *regAddrPtr =
                (relativeDfxBase | (regAddr & dfxAddressMask))
                + prvCpssSip6TileOffsetGet(devNum, tileIndex);
            if (error != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

GT_STATUS prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    IN GT_U32   value,
    IN GT_U32   mask
);
GT_STATUS prvCpssDrvHwPpSmiChipletRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask function
* @endinternal
*
* @brief   Write DFX Register bit mask to register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E == dfxType)
    {
        GT_U32 ravenId = tileIndex*FALCON_RAVENS_PER_TILE + gopIndex/*0..3*/;
        GT_U32 value;

        /* check that this Raven is accessable !*/
        rc = prvCpssDrvHwPpSmiChipletRegisterRead(devNum,ravenId,0x00300050,&value);
        if(rc != GT_OK)
        {
            /* no SMI to this chiplet */
            return GT_OK;
        }
        if(value != 0x11AB)
        {
            cpssOsPrintf("devNum[%d] ravenId[%d] is not accessable to MG 0x00300050 : read [0x%8.8x] instead of 0x11AB ... so skip it \n",
                devNum,ravenId,value);
            return GT_OK;
        }

        return prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask(devNum,ravenId,convertedRegAddr, data , mask);
    }

    return prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, portGroup, convertedRegAddr, mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegRead function
* @endinternal
*
* @brief   Read DFX register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDrvHwPpPortGroupReadRegister(
        devNum, portGroup, convertedRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead function
* @endinternal
*
* @brief   Read given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegRead(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to reset trigger register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_U32       dbRegAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dbRegAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa function
* @endinternal
*
* @brief   Run logic before triggering Falcon CNM System Soff Reset.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa
(
    IN  GT_U8                                          devNum
)
{
    GT_U32 singleHwDataWord = 0;
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32  ii;
    GT_U32  rc;
    GT_U32  index;
    GT_U32  numOfEntries;

    static const CPSS_DXCH_TABLE_ENT tablesToRead[] = {
         CPSS_DXCH_TABLE_FDB_E
        ,CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E
        ,CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E
        ,CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E
        ,CPSS_DXCH_TABLE_LAST_E/* must be last*/
    };

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_FALCON_SBM_XOR_CLIENTS_READ_WRITE_BEFORE_SW_RESET_WA_E))
    {
        /*  the 2 JIRAs :
            ST-13:      SW reset is not supported for clients using XOR memorySW reset is not supported for clients using XOR memory
            HA-3719:    SW reset is not supported for eport1 target attributes table due to XOR memory implementation
        */
        if (prvCpssDxChHwInitSoftResetTraceEnable)
        {
            cpssOsPrintf("tables read/write ERRATA SBM_XOR_CLIENTS_READ_WRITE_BEFORE_SW_RESET\n");
        }

        /* need to read single entry from all the tables and write back to the same table */
        for(ii = 0; tablesToRead[ii] != CPSS_DXCH_TABLE_LAST_E; ii++)
        {
            tableType = tablesToRead[ii];

            if (tableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E &&
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum == 0)
            {
                continue;
            }

            if (tableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E)
            {
                GT_U32 exactMatchNumOfBanks;
                PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);
                if(exactMatchNumOfBanks == 0)
                {
                    /* the device to soft reset between 'phase 1' and 'pp logical init'
                      at this time such non-coharent happens , and we not want to fail in fucntion
                      prvCpssDxChReadTableEntryField(...) */
                    continue;
                }
            }


            numOfEntries = (tableType == CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E) ? 5 : 7;
            for (index = 0; index < numOfEntries; index++)
            {
                /* Read single bit of specific index into single word */
                rc = prvCpssDxChReadTableEntryField(devNum,
                                                    tableType,
                                                    index,
                                                    0, 0, 1,
                                                    &singleHwDataWord);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* Write single word to specific index */
                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                     tableType,
                                                     index,
                                                     0, 0, 1,
                                                     singleHwDataWord);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cm3CoprocessorsDisable function
* @endinternal
*
* @brief   Disable all CM3 CPUs in Eagle CNMs
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS cm3CoprocessorsDisable
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr/*address in MG0*/
)
{
    GT_U32      ii;
    GT_U32      diffFromMg0;
    GT_STATUS   rc;

    GT_U32 numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    if(numMgs == 0)
    {
        numMgs = 1;
    }

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) &&
        !PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "Not implemented for SIP7 device(s)");
    }

    for(ii = 0 ; ii < numMgs; ii=ii+4 /* CM3 is in each forth MG */)
    {
         diffFromMg0 = (ii == 0) ? 0 : prvCpssSip6OffsetFromFirstInstanceGet(devNum,ii,PRV_CPSS_DXCH_UNIT_MG_E);
         rc = prvCpssHwPpSetRegField(devNum, regAddr + diffFromMg0, 28, 2, 0);
         if (rc != GT_OK)
         {
             return rc;
         }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChWriteRegisterAllMGs function
* @endinternal
*
* @brief   duplicate addess from MG0 to all other MGs.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssDxChWriteRegisterAllMGs
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,/*address in MG0*/
    IN GT_U32 value
)
{
    GT_U32  ii;
    GT_U32  diffFromMg0;

    GT_U32 numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    if(numMgs == 0)
    {
        numMgs = 1;
    }

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) &&
        !PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "Not implemented for SIP7 device(s)");
    }

    for(ii = 0 ; ii < numMgs; ii++)
    {
         diffFromMg0 = (ii == 0) ? 0 : prvCpssSip6OffsetFromFirstInstanceGet(devNum,ii,PRV_CPSS_DXCH_UNIT_MG_E);
         prvCpssHwPpWriteRegister(devNum,regAddr + diffFromMg0,value);
    }

    return GT_OK;
}

/**
* @internal falconMgManualResetRegisters function
* @endinternal
*
* @brief   Falcon : manualy reset some MG registers that are important for logic
*           before the device start reset.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] resetCm3              - GT_TRUE - reset CM3 CPUs, GT_FALSE - don't reset CM3 CPUs
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS   falconMgManualResetRegisters
(
    IN GT_U8  devNum,
    IN GT_BOOL resetCm3
)
{
    GT_U32  regAddr,mgBaseAddr;
    GT_U32  data;
    GT_U32  queue;

    mgBaseAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_generalControl & 0xFFFF0000;

    /* register that are not in the CPSS DB of registers */
    regAddr = mgBaseAddr + 0x00000070;/* user defined register 0 */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0);

    regAddr = mgBaseAddr + 0x00000074;/* user defined register 1 */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0);

    /*Global Interrupts Summary Mask*/
    regAddr = mgBaseAddr + 0x00000034;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_generalControl;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0x000f0f0f);

    if (resetCm3)
    {
        /* CM3 CPU in MG units get reset during Soft Reset incuding reset of RAM for all devices but not Falcon.
           Force CM3 to be disabled. */
        regAddr = mgBaseAddr + 0x00000500;/* Confi Processor Global Configuration register */
        cm3CoprocessorsDisable(devNum,regAddr);
    }

    /* set resource error mode to HW default.
       This is needed to disable SDMA properly in case "Retry" mode was configured */
    for (queue = 0; queue < 8; queue++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxSdmaResourceErrorCountAndMode[queue];
        /* set register to default value that stops SDMA on resource error */
        data = 0x400;
        prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxQCmdReg;
    /* disable all 8 RX SDMA queues in all MGs */
    data = 0xFF00;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg;
    /* disable all 8 TX SDMA queues in all MGs.
       Use data same as for RX SDMA disable. */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    /* wait to make sure that SDMA disabled */
    cpssOsTimerWkAfter(1);

#if 0     /* must not reset it so the prvCpssDxChCfgInitAuqEnableDisable will know to init it properly ! */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
    writeRegister_all_MGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
    writeRegister_all_MGs(devNum,regAddr,1);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
    writeRegister_all_MGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
    writeRegister_all_MGs(devNum,regAddr,1);
#endif /*0*/


    return GT_OK;
}

/**
* @internal prvCpssDxChManualMgSdmaReset function
* @endinternal
*
* @brief   This function is doing sdma reset.
*
* @note   APPLICABLE DEVICES:      Falcon;AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChManualMgSdmaReset
(
    IN GT_U8 devNum
)
{
    GT_U32  regAddr;
    GT_U32  data;
    GT_STATUS rc;

    /* make sure that Soft Reset done */
    cpssOsTimerWkAfter(10);

    /* toggle SDMA SW Reset bit Extended Global Control registers of all MGs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.extendedGlobalControl;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set bit 6 to Reset SDMAs */
    data |= BIT_6;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    /* reset bit 6 to place SDMAs in normal state */
    data &= (~BIT_6);
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);


    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger function
* @endinternal
*
* @brief   Trigger Falcon CNM System Soft Reset.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger
(
    IN  GT_U8                                          devNum
)
{
    GT_U32    unitBase; /* base address of unit */
    GT_BOOL   error;    /* error indication     */
    GT_STATUS rc;       /* return code          */
    GT_U32    regAddr;  /* address of register  */
    GT_U32    dbRegAddr;    /* address of register from DB        */
    GT_U32    triggerValue; /* value of bit to trigger Soft Reset */
    GT_U32    tileIndex; /* index of tile           */
    GT_U32    gopIndex;  /* index of GOP device     */
    GT_U32    mask;      /* mask for register write */
    GT_U32    data;      /* data of register        */
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr; /* pointer to registers DB */
    GT_BOOL   isPexSkiped = GT_FALSE; /* indication for Skip PCI_e reset */
    GT_U32    ravenId;     /* index of raven device */
    GT_U32    resetSrcBit; /* bit related to source of soft reset */
    GT_U32    pci_e_dfxPipe; /* DFX Pipe of PCI_e unit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* use MNG source of Soft Reset - bit# 1 in "skip" registers */
    resetSrcBit = 1;
    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AAS_E:
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:

        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum,CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,&isPexSkiped);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* when the PEX skipped then also the MG is skipped */
        /* there are some registers that we need to manually 'restore' them */
        /* and we can do it even before the reset it self */
        if(isPexSkiped)
        {
            rc = falconMgManualResetRegisters(devNum, GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        break;

        /* AC5X and IronMan has internal CPUs and CPU subsystem (CNM). need to
           configure multiple skips to avoid reset of CNM and CPUs */
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /* enable SKIP init for CPU. Ironman CNM skip manages CPU skip also. */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.CPUSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        else
        {
            /* need to skip DDR reset for Ironman */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DDRSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        /* enable SKIP init for DFX */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXRegistersSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* enable SKIP init for CNM */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.cnmSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Need to set skip CNM RAMs related pipe#4 (AC5X), pipe#1 (Ironman) */
        pci_e_dfxPipe = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) ? 1 : 4;

        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, pci_e_dfxPipe, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Enable DFX Pipe skip for "MG" Reset source */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 23+resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            rc = cpssDxChHwPpSoftResetSkipParamGet(devNum,CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,&isPexSkiped);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Avoid toggle of device_enabled_and_init_done */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.metalFixRegister;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 26, 1, 1);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        else
        {
            /* IronMan has dedicated skip for device_enabled_and_init_don */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceEnabledAndInitDoneSkipInitMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        /* Skip MPP reset to avoid CPU subsystem disruption */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.mppSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* when the CNM skipped then also the MG is skipped.
           There are some registers that we need to manually 'restore' them
           and we can do it even before the reset it self.
           Don't reset CM3 CPUs to avoid POE one to be reset. */
        rc = falconMgManualResetRegisters(devNum, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        break;

        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:

        /* AC5P, Harrier */
        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum,CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,&isPexSkiped);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* need to avoid PCI_e unit RAMs initialization during Soft Reset. */
        if(isPexSkiped)
        {
            pci_e_dfxPipe = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ? 1 : 5;

            /* Need to set skip CNM RAMs related DFX pipe  */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, pci_e_dfxPipe, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Enable DFX Pipe skip for "MG" Reset source */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 23+resetSrcBit, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* Run WA logic before system reset */
    rc = prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa failed rc[%]", rc);
        goto cleanUp;
    }

    triggerValue   = 0;
    dbRegAddr      = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;
    for (tileIndex = 0;
          (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        /* Eagle DFX use External Soft Reset MNG signal for Soft Reset treatment.
           Need to enable this signal by the configuration in Server Reset Control register:
           bit 4  - External Soft Reset Mask mng - set to 0
           bit 12 - External Soft Reset Edge Select mng - set to triggerValue */
        mask           = ((1 << 12) | (1 << 4));
        data           = (triggerValue << 12);
        rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
            devNum, PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E, tileIndex,
            0/*gopIndex*/, dbRegAddr, mask, data);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC(
                "prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask failed rc[%]", rc);
            goto cleanUp;
        }

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
        {
            /* no Raven */
            continue;
        }

        for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
        {
            if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
            {
                ravenId = 3 - gopIndex;
            }
            else
            {
                ravenId = gopIndex;
            }

            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

            /* GOP (Raven) DFX use External Soft Reset SRN signal for Soft Reset treatment.
               Need to enable this signal by the configuration in Server Reset Control register:
               bit 3  - External Soft Reset Mask srn - set to 0
               bit 11 - External Soft Reset Edge Select srn - set to triggerValue */
            mask           = ((1 << 11) | (1 << 3));
            data           = (triggerValue << 11);
            rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
                devNum, PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E, tileIndex,
                gopIndex, dbRegAddr, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC(
                    "prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask failed rc[%]", rc);
                goto cleanUp;
            }
        }
    }

    error = GT_FALSE;
    unitBase = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_CNM_RFU_E, &error);
    if (error != GT_FALSE)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChHwUnitBaseAddrGet failed rc[%]", rc);
        goto cleanUp;
    }

    /* configure state of device to be after HW reset */
    rc = prvCpssDxChDeviceDidResetConfig(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*regAddr - System Soft Reset*/
    regAddr = ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
               || PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)) ? 0x14 : 0x44;
    regAddr += unitBase;

    /* 0-th CNM unit connected to all tiles */
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, ((~ triggerValue) & 1));
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDrvHwPpPortGroupWriteRegBitMask failed rc[%]", rc);
        goto cleanUp;
    }
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, (triggerValue & 1));
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDrvHwPpPortGroupWriteRegBitMask failed rc[%]", rc);
        goto cleanUp;
    }

    /* for AC5X fix jira : CPSS-17256 : cpssDxChNetIfInit failed with GT_TIMEOUT*/
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&
       isPexSkiped)
    {
        /* do SDMA reset  */
        rc = prvCpssDxChManualMgSdmaReset(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

cleanUp:

    return rc;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in all Falcon Tiles and Ravens relvant to given Skip Init Matrix.
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 perGop;
    GT_BOOL                                 perTile;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;
    GT_U32                                  ravenId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DEV_USER_SOFT_RESET_SUPPORTED_DEVICES_MAC(devNum);

    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CNM_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E:
            perGop   = GT_FALSE;
            perTile  = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            perGop   = GT_TRUE;
            perTile  = GT_FALSE;
            break;
        default:
            perGop   = GT_TRUE;
            perTile  = GT_TRUE;
            break;
    }

    for (tileIndex = 0; (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        if (perTile != GT_FALSE)
        {
            dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E;
            gopIndex = 0;
            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
        {
            /* no Raven */
            continue;
        }
        if (perGop != GT_FALSE)
        {
            for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
            {
                if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
                {
                    ravenId = 3 - gopIndex;
                }
                else
                {
                    ravenId = gopIndex;
                }

                PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

                dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
                rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                    devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Ravens Set.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] gopBitmap             - bitmap of GOP global indexes
*                                    CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*                                    Skip types relevant to Main Die only ignored. GT_OK returned.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  GT_CHIPLETS_BMP                                gopBitmap,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;
    GT_U32                                  gopGlobalIndex;
    GT_U32                                  ravenId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (gopBitmap == CPSS_CHIPLETS_UNAWARE_MODE_CNS)
    {
        gopBitmap = 0xFFFFFFFF; /* rize all bits */
    }
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CNM_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            return GT_OK; /* ignored for GOP, relevant to Tile */
        default: break;
    }

    dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
    for (tileIndex = 0; (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
        {
            gopGlobalIndex =
                ((PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * tileIndex)
                 + gopIndex);
            if ((gopBitmap & (1 << gopGlobalIndex)) == 0) continue;

            if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
            {
                ravenId = 3 - gopIndex;
            }
            else
            {
                ravenId = gopIndex;
            }

            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance to given Skip Init Matrix.
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] skipEnable            - enable or disable skip of the unit before soft reset
*                                    GT_TRUE    - the unit is skipped before soft reset
*                                    GT_FALSE   - the unit is not skipped before soft reset
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_BOOL                                        skipEnable
)
{
    GT_U32      data;
    GT_U32      mask = BIT_1;

    /* 0 - Skip is On, 1 - Skip is Off */
    data = (skipEnable) ? 0 : BIT_1;
    /* bit1 - MNG reset source */
    return prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask(devNum, skipType, mask, data);
}

/**
* @internal prvCpssDrvHwInitSoftResetUnitDfxPipesBmpGet function
* @endinternal
*
* @brief    The function retrieves the bitmaps of pipes that should be
*           reset or skipped during a soft reset operation for specific units.
*
* @note     APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman; Caspian.
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum                - Device number
* @param[in]  softResetUnit         - Soft reset unit type
* @param[out] pipesBmpPtr           - (pointer to) the pipes bitmap
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on invalid device number or soft reset unit type
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device family
*
*/
static GT_STATUS prvCpssDrvHwInitSoftResetUnitDfxPipesBmpGet
(
    IN  GT_U8  devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT  softResetUnit,
    OUT GT_U32 *pipesBmpPtr
)
{
    GT_STATUS rc;
    GT_U32 pipesBmp = 0;
    GT_U32 pipesAllBmp = 0;

    /* The SWITCH or the CNM units are only applicable */
    if (softResetUnit == PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E ||
        softResetUnit == PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E)
    {
        /* Sets the CNM DFX pipes bitmap that need to be reset or skipped */
        switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
                pipesBmp = 0x10;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
                pipesBmp = 0x6;
                break;
            case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                pipesBmp = 0x20;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5X_E:
                pipesBmp = 0x10;
                break;
            case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                pipesBmp = 0x2;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5_E:
                pipesBmp = 0x2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }

        if (softResetUnit == PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E)
        {
            rc = prvCpssDxChDfxPipesBmpGet(devNum, &pipesAllBmp);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Gets SWITCH DFX pipes by clearing the bits corresponding to CNM DFX pipes from the shared DFX pipes bitmap */
            pipesBmp = pipesAllBmp & ~(pipesBmp);
        }
    }

    *pipesBmpPtr = pipesBmp;

    return GT_OK;
}

/**
* @internal prvCpssDrvHwInitSoftResetUnitDfxPipesSkipInit function
* @endinternal
*
* @brief   Enable or disable skipping of DFX pipe initialization for the specified soft reset unit.
*          This function sets the skip initialization bit for the relevant DFX pipes based on the provided pipes bitmap.
*
* @note     APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman; Caspian.
* @note     NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.

* @param[in]  devNum           - Device number
* @param[in]  softResetUnit    - Soft reset unit type
* @param[in]  skipEnable       - GT_TRUE to enable skipping, GT_FALSE to disable
*
* @retval GT_OK                - on success
* @retval GT_BAD_PARAM         - on invalid device number or soft reset unit type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device family
* @retval GT_FAIL              - on failure
*
**/
static GT_STATUS prvCpssDrvHwInitSoftResetUnitDfxPipesSkipInit
(
    IN  GT_U8                               devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT   softResetUnit,
    IN  GT_BOOL                             skipEnable
)
{
    GT_STATUS                                   rc;
    GT_U32                                      regAddr;
    GT_U32                                      regData;
    GT_U32                                      enable;
    GT_U32                                      pipesBmp;
    GT_U32                                      pipeId;
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    /* Get the register address for setting the skip initialization bit */
    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;

    enable = (skipEnable == GT_TRUE) ? 0 : 1;

    if (softResetUnit == PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E)
    {
        /* Get the DFX pipes bitmap for the SWICTH soft reset unit */
        rc = prvCpssDrvHwInitSoftResetUnitDfxPipesBmpGet(devNum, PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E, &pipesBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Check if all SWITCH unit pipes bits are skiped */
        if ((regData & pipesBmp) == 0)
        {
            /* If SWITCH pipes are skipped, CNM pipes must also be skipped */
            enable = 0;
        }

        /* Get the DFX pipes bitmap for the CNM soft reset unit */
        rc = prvCpssDrvHwInitSoftResetUnitDfxPipesBmpGet(devNum, PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E, &pipesBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else    /* PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E */
    {
        /* Get the DFX pipes bitmap for the SWITCH soft reset unit */
        rc = prvCpssDrvHwInitSoftResetUnitDfxPipesBmpGet(devNum, PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E, &pipesBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Iterate through each DFX pipe and set the skip initialization bit */
    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if ((pipesBmp & 1) == 0)
        {
            continue; /* Skip non-relevant pipes */
        }

        /* Set the skip initialization bit for the current pipe */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, pipeId, 1, enable);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetUnitSkipInit function
* @endinternal
*
* @brief   Enable or disbale skip initialization of the unit before soft reset.
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman; Caspian.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] softResetUnit         - soft reset unit
* @param[in] skipEnable            - enable or disable skip of the unit before soft reset
*                                    GT_TRUE    - the unit is skipped before soft reset
*                                    GT_FALSE   - the unit is not skipped before soft reset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable devices
*/
GT_STATUS prvCpssDxChHwInitSoftResetUnitSkipInit
(
    IN GT_U8                                devNum,
    IN PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT    softResetUnit,
    IN GT_BOOL                              skipEnable
)
{
    GT_STATUS                                   rc;      /* return code */
    GT_U32                                      regAddr; /* address of register */
    GT_U32                                      offset;  /* offset of bit in register */
    GT_U32                                      enable;  /* enable/disable state of the feature */
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    switch (softResetUnit)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E:
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Falcon specific units */
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_D2D_LINK_E, skipEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E, skipEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E, skipEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* mask/unmask Tables Start Init signal for case of CNM reset but skip Switch */
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_AC5X_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl44;
                    offset = 17;
                    break;
                case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl21;
                    offset = 3;
                    break;
                case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl53;
                    offset = 7;
                    break;
                case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl12;
                    offset = 29;
                    break;
                default:
                    regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    offset = 0; /* avoid warnings */
                    break;
            }

            if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                /* bit should be 0 when Switch is skipped and should be 1 when not skipped */
                enable = (skipEnable == GT_TRUE) ? 0 : 1;
                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, offset, 1, enable);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            /* Enable or disable skipping of DFX pipe initialization for the SWITCH soft reset unit */
            rc = prvCpssDrvHwInitSoftResetUnitDfxPipesSkipInit(devNum, PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E:
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CNM_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
            {
                /* SKIP init for CNM and CPU */
                enable = (skipEnable == GT_TRUE) ? 0 : 1;

                /* CPU reset is not separated from the CNM reset.
                   The decision to skip or not skip the CPU reset
                   should be exactly the same as the CNM reset */
                regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.CPUSkipInitializationMatrix;
                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 1, 1, enable);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            /* Enable or disable skipping of DFX pipe initialization for the CNM soft reset unit */
            rc = prvCpssDrvHwInitSoftResetUnitDfxPipesSkipInit(devNum, PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_PCIE_E:
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_POE_E:
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_AC5X_E:
                case CPSS_PP_FAMILY_DXCH_AC5_E:
                case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                    rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E, skipEnable);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    break;
                default:
                    break;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_COMMON_E:
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Falcon specific units */
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DEVICE_EEPROM_E, skipEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_REGISTERS_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_AC5X_E:
                case CPSS_PP_FAMILY_DXCH_AC5_E:
                case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.mppSkipInitializationMatrix;
                    /* SKIP init for MPP */
                    enable = (skipEnable == GT_TRUE) ? 0 : 1;
                    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 1, 1, enable);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
                        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
                    {
                        /* Avoid toggle of device_enabled_and_init_done */
                        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.metalFixRegister;
                        enable = (skipEnable == GT_TRUE) ? 1 : 0;
                        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 26, 1, enable);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* IronMan has dedicated skip for device_enabled_and_init_don */
                        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceEnabledAndInitDoneSkipInitMatrix;
                        enable = (skipEnable == GT_TRUE) ? 0 : 1;
                        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 1, 1, enable);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_CPU_E:
            switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DDRSkipInitializationMatrix;
                    enable = (skipEnable == GT_TRUE) ? 0 : 1;
                    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 1, 1, 0);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;
                default:
                    break;
            }
            break;

        case PRV_CPSS_DXCH_SOFT_RESET_UNIT_REGISTERS_E:
            rc = prvCpssDxChHwInitSoftResetSubUnitSkipInitMatrixSet(devNum, PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E, skipEnable);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetBeforeTrigger function
* @endinternal
*
* @brief  Perform necessary actions before triggering a hardware-initiated soft reset.
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman; Caspian.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - Device number
* @param[in] softResetType              - The use-case type of the reset to set
*                                           (see comments in CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or softResetType
*
*/
static GT_STATUS prvCpssDxChHwInitSoftResetBeforeTrigger
(
    IN  GT_U8                               devNum,
    IN  CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT   softResetType
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;                    /* Address of the device reset control register */
    GT_BOOL     systemResetWa = GT_FALSE;   /* Disable system reset WA */
    GT_BOOL     manualMgReset = GT_FALSE;   /* Disable MG manual reset */
    GT_U32      enable = 0;                 /* Enable DFX Pipe skip for "MG" Reset source */
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;

    if (softResetType == CPSS_HW_PP_RESET_ALL_SKIP_NONE_E)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Enable system reset WA */
            systemResetWa = GT_TRUE;
        }
        /* Set default value in DFX Pipe skip for "MG" Reset source */
        enable = 1;
    }
    else if (softResetType == CPSS_HW_PP_RESET_SWITCH_SKIP_PCIE_CNM_E)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Enable system reset WA */
            systemResetWa = GT_TRUE;
            /* Enable manual MG reset */
            manualMgReset = GT_TRUE;

            /* configure state of device to be after HW reset */
            rc = prvCpssDxChDeviceDidResetConfig(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (systemResetWa == GT_TRUE)
    {
        /* Run system reset WA */
        rc = prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (manualMgReset == GT_TRUE)
    {
        /* There are some registers that we need to manually 'restore' them
           and we can do it even before the reset itself */
        rc = falconMgManualResetRegisters(devNum, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Enable/disable DFX Pipe skip for "MG" Reset source */
    return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 24, 1, enable);
}

/**
* @internal prvCpssDxChHwInitSoftResetTrigger function
* @endinternal
*
* @brief   Trigger a hardware-initiated soft reset for the specified device.
*
* @note   APPLICABLE DEVICES:      AC5; Falcon; AC5P; AC5X; Harrier; Ironman; Caspian.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The device number to trigger the reset on.
* @param[in] softResetType            - The use-case type of the reset to set
*                                           (see comments in CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT)
*
* @retval GT_OK                       - on success.
* @retval GT_FAIL                     - on failure.
* @retval GT_BAD_PARAM                - on invalid parameter.
*
*/
GT_STATUS prvCpssDxChHwInitSoftResetTrigger
(
    IN  GT_U8   devNum,
    IN  CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT   softResetType
)
{
    GT_STATUS       rc;
    GT_U32          triggerValue;       /* Value used to trigger the reset */
    GT_U32          dbRegAddr;          /* Address of the device reset control register */
    GT_U32          tileIndex;          /* Index for iterating through tiles */
    GT_U32          mask, data;         /* Bitmask and data used for register writes */
    GT_BOOL         error;              /* Flag indicating an error condition */
    GT_U32          unitBase;           /* Base address of a specific hardware unit */
    GT_U32          regAddr;            /* Address of the reset register */
    GT_U32          gopIndex;           /* Index for iterating through GOPs (Ravens) */
    GT_U32          ravenId;            /* ID of the Raven component */


    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr; /* pointer to registers DB */

    /* Perform necessary actions before trigering soft reset */
    rc = prvCpssDxChHwInitSoftResetBeforeTrigger(devNum, softResetType);
    if (rc != GT_OK)
    {
        return rc;
    }

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    dbRegAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    triggerValue = 0;
    for (tileIndex = 0; (tileIndex == 0/* support single tile devices */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
         tileIndex++)
    {
        /* DFX use External Soft Reset MNG signal for Soft Reset treatment.
           Need to enable this signal by the configuration in Server Reset Control register:
           bit 4  - External Soft Reset Mask mng - set to 0
           bit 12 - External Soft Reset Edge Select mng - set to triggerValue */
        mask = ((1 << 12) | (1 << 4));
        data = (triggerValue << 12);
        rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(devNum,
                                                              PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E,
                                                              tileIndex, 0/*gopIndex*/, dbRegAddr, mask, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
        {
            /* No Ravens */
            break;
        }

        for (gopIndex = 0; gopIndex < FALCON_RAVENS_PER_TILE; gopIndex++)
        {
            if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
            {
                ravenId = 3 - gopIndex;
            }
            else
            {
                ravenId = gopIndex;
            }

            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

            /* GOP (Raven) DFX use External Soft Reset SRN signal for Soft Reset treatment.
               Need to enable this signal by the configuration in Server Reset Control register:
               bit 3  - External Soft Reset Mask srn - set to 0
               bit 11 - External Soft Reset Edge Select srn - set to triggerValue */
            mask           = ((1 << 11) | (1 << 3));
            data           = (triggerValue << 11);
            rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(devNum,
                                                                  PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E, tileIndex,
                                                                  gopIndex, dbRegAddr, mask, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    error = GT_FALSE;
    unitBase = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_CNM_RFU_E, &error);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*regAddr - System Soft Reset*/
    regAddr = ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)  ||
                PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)) ? 0x14 : 0x44;
    regAddr += unitBase;

    /* 0-th CNM unit connected to all tiles */
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, ((~ triggerValue) & 1));
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, (triggerValue & 1));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (softResetType == CPSS_HW_PP_RESET_SWITCH_SKIP_PCIE_CNM_E)
        {
            /* Do SDMA reset  */
            rc = prvCpssDxChManualMgSdmaReset(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

