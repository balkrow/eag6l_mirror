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
 * @file prvCpssDxChMacSecDriver.c
 *
 * @brief Holds internal DDK API, which need to be called from CPSS.
 *
 * @version   1
 ********************************************************************************
 */
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecUtils.h>
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>

/* Driver Header files */
#include <Kit/EIP164/incl/eip164_secy.h>

/**
* @internal prvCpssDxChMacSecStatisticsSaClear function
* @endinternal
*
* @brief   Reset SA statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] direction             - select Egress or Ingress MACSec transformer
* @param[in] saHandle              - SA handle of the SA for which the statistics need to be cleared.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS prvCpssDxChMacSecStatisticsSaClear
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle
)
{
    GT_STATUS               rc;
    GT_U32                  macSecUnitId;
    GT_U32                  saIndex;
    SecY_SAHandle_t         secySaHandle;

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Now get SA and SC indexes */
    rc = SecY_SAIndex_Get(secySaHandle, &saIndex, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(direction == CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E)
    {
        rc = EIP164_SecY_SA_Stat_I_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[macSecUnitId]->IOArea,
                saIndex);
    }
    else
    {
        rc = EIP164_SecY_SA_Stat_E_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[macSecUnitId]->IOArea,
                saIndex);
    }
    if(rc != EIP164_NO_ERROR)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Statistics reset failed for saIndex(%d)", saIndex);
    }
    return GT_OK;
}
