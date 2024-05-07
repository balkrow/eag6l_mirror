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
* @file prvCpssDxChHwInitSoftReset.h
*
* @brief Internal definitions for Soft Reset implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChHwInitSoftReset_H
#define __prvCpssDxChHwInitSoftReset_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT
 *
 * @brief Defines DFX type.
*/
typedef enum{
    /** @brief Processing Core DFX.
     */
    PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E = 0,

    /** @brief GOP DFX.
     */
    PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E = 1

} PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT;


/**
* @enum PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT
 *
 * @brief Defines Type of Skip Initialization Matrix.
*/
typedef enum{

    /** @brief Registers Configuration Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E = 0,

    /** @brief SRR Load Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SRR_LOAD_E = 1,

    /** @brief BIST Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_BIST_E = 2,

    /** @brief Soft Repair Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SOFT_REPAIR_E = 3,

    /** @brief RAM Init Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E = 4,

    /** @brief Regs Genx Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGS_GENX_E = 5,

    /** @brief Multi-Action Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_MULTI_ACTION_E = 6,

    /** @brief Tables Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E = 7,

    /** @brief SERDES Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E = 8,

    /** @brief EEPROM Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E = 9,

    /** @brief PCIe Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E = 10,

    /** @brief Device EEPROM Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DEVICE_EEPROM_E = 11,

    /** @brief D2D Link Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_D2D_LINK_E = 12,

    /** @brief DFX Registers Configuration Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_REGISTERS_E = 13,

    /** @brief DFX Pipe Configuration Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_PIPE_E = 14,

    /** @brief Switch Core (Core Tile in Falcon) Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_E = 15,

    /** @brief CNM (Core MNG in Falcon) Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CNM_E = 16,

    /** @brief Core D2D Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E = 17,

    /** @brief Core Raven Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E = 18,

    /** @brief GOP D2D Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E = 19,

    /** @brief GOP Main Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E = 20,

    /** @brief GOP CNM Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E = 21,

    /** @brief GOP GW Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E = 22,

    /** @brief POE Skip Initialization Matrix.
     */
    PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E = 23


} PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT;


/**
* @enum PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT
 *
*  @brief Defines the types of units that are eligible to be reset or skipped
*         during a soft reset in a specific scenario.
*/
typedef enum{

    /** @brief Switch - the collection of units that can be skipped during a soft reset,
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_SWITCH_E,

    /** @brief CPU unit - the unit can be reset during a soft reset.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_CPU_E,

    /** @brief CnM unit - the unit can be skipped during a soft reset.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_CNM_E,

    /** @brief PCIe unit - the unit can be skipped during a soft reset.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_PCIE_E,

    /** @brief POE unit - the unit can be skipped during a soft reset.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_POE_E,

    /** @brief Common units - the collection of units that can be skipped during a soft reset.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_COMMON_E,

    /** @brief Registers - manages Skip Registers configuration that can be skipped during a
     *         soft reset. This is common configuration for Switch, CNM and CPU units and need
     *         to be set depends on reset type.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_REGISTERS_E,

    /** @brief Should be last.
     */
    PRV_CPSS_DXCH_SOFT_RESET_UNIT_LAST_E

} PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT;

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
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable devices
*/
GT_STATUS prvCpssDxChHwInitSoftResetUnitSkipInit
(
    IN GT_U8                                devNum,
    IN PRV_CPSS_DXCH_SOFT_RESET_UNIT_ENT    softResetUnit,
    IN GT_BOOL                              skipEnable
);

/**
* @internal prvCpssDxChHwInitSoftResetTrigger function
* @endinternal
*
* @brief   Trigger a hardware-initiated soft reset for the specified device.
*
* @param[in] devNum                   - The device number to trigger the reset on.
* @param[in] softResetType              - The use-case type of the reset to set
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
);

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
);

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
);

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
);

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
);

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
);

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
);

/**
* @internal prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger function
* @endinternal
*
* @brief   Trigger Falcon CNM System Soft Reset.
*          .
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
);

/**
* @internal prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in all Falcon Tiles and Ravens relvant to given Skip Init Matrix.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
);

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
*                                  CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
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
GT_STATUS prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  GT_CHIPLETS_BMP                                gopBitmap,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
);

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
);

/**
* @internal prvCpssDxChWriteRegisterAllMGs function
* @endinternal
*
* @brief   duplicate addess from MG0 to all other MGs.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
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
);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__prvCpssDxChHwInitSoftReset_H*/


