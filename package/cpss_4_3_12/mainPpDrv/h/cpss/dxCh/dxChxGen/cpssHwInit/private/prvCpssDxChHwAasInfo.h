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
* @file prvCpssDxChHwAasInfo.h
*
* @brief Private definition for the AAS devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwAasInfoh
#define __prvCpssDxChHwAasInfoh

#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvDxChHwRegAddrAasDbInit function
* @endinternal
*
* @brief   init the base address manager of the Aas device.
*         prvDxChAasUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrAasDbInit(GT_VOID);

/**
* @internal prvCpssAasInitParamsSet function
* @endinternal
*
* @brief   AAS : init the very first settings in the DB of the device.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssAasInitParamsSet
(
    IN  GT_U8   devNum
);

GT_STATUS prvCpssAasNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
);
/**
* @internal prvCpssDxChSharedMemoryAasConnectClientsToMemory function
* @endinternal
*
* @brief   function to fill into the info of SHM :
*           set shared tables HW with the needed configurations,
*           according to info about the sizes of memories that the clients need.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]
*       devNum  - the device number
*       sharedTablesSizeInfoPtr  - (pointer to) the shared tables sizes info
*
* @param[out]
*   None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR    - on hardware error.
* @retval GT_BAD_PARAM   - on wrong devNum or other table parameter.
*
*/
GT_STATUS prvCpssDxChSharedMemoryAasConnectClientsToMemory(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PP_CONFIG_SHARED_TABLES_SIZE_STC  *sharedTablesSizeInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwAasInfoh */

