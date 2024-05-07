/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSip7LpmRamDeviceSpecific.h
*
* DESCRIPTION:
*       Declaration of the device specific functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1$
*
*******************************************************************************/

#ifndef __prvCpssDxChSip7LpmRamDeviceSpecifich
#define __prvCpssDxChSip7LpmRamDeviceSpecifich

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvCpssDxChIpLpmLpmRamDeviceSpecificBankSizeGetSip7 function
 * @endinternal
 *
 * @note   APPLICABLE DEVICES:      AAS.
 * @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *                                  Falcon; AC5P; AC5X; Harrier; Ironman.
 *
 * @brief Get hardware big bank size for Sip7 device
 *
 * @return GT_U32 - Big bank size
 *
*/
GT_U32 prvCpssDxChIpLpmLpmRamDeviceSpecificBankSizeGetSip7
(
    GT_U8 devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip7LpmRamDeviceSpecifich */
