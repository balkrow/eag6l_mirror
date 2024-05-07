/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssGlobalNonSharedMainPpDrvInitVars.c
*
* @brief This file  Initialize global non shared variables used in  module:mainPpDrv
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSerdesCfg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSpeed.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChPhyMpdInit.h>
#endif



/*global variables macros*/

#define PRV_NON_SHARED_HW_INIT_DIR_HW_INIT_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.hwInitDir.hwInitSrc._var,_value)

#ifdef CHX_FAMILY

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChPortDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/port
 *
 */
static GT_VOID cpssGlobalNonSharedDbMainPpDrvModDxChPortDirDataSectionInit
(
    GT_VOID
)
{
    {
        PRV_CPSS_DXCH_PORT_SPEED_SET_FUN portSpeedSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1][PRV_CPSS_XG_PORT_OPTIONS_MAX_E]=
        {                                   /* PRV_CPSS_XG_PORT_XG_ONLY_E  PRV_CPSS_XG_PORT_HX_QX_ONLY_E PRV_CPSS_XG_PORT_XG_HX_QX_E PRV_CPSS_GE_PORT_GE_ONLY_E PRV_CPSS_XG_PORT_XLG_SGMII_E  PRV_CPSS_XG_PORT_CG_SGMII_E */
        /* CPSS_PP_FAMILY_CHEETAH_E     */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_CHEETAH2_E    */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_CHEETAH3_E    */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_DXCH_XCAT_E   */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  {  NULL,                       NULL,                         NULL,/*xCatSpeedSet,*/      bcat2SpeedSet,             bcat2SpeedSet,                NULL         },
        /* CPSS_PP_FAMILY_DXCH_AC5_E    */  {  NULL,                       NULL,                         NULL,/*xCatSpeedSet,*/      bcat2SpeedSet,             bcat2SpeedSet,                NULL         },
        /* CPSS_PP_FAMILY_DXCH_LION_E   */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  {  NULL,                       NULL,                         NULL,                       NULL,                      NULL,                         NULL         },
        /* CPSS_PP_FAMILY_DXCH_LION2_E  */  {  NULL,                       NULL,                         NULL,                       lion2SpeedSet,/*for CPU*/  NULL,                         lion2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_LION3_E  */  {  NULL,                       NULL,                         NULL,                       lion2SpeedSet,             NULL,                         lion2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_BOBCAT2_E*/  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_BOBCAT3_E*/  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             bcat2SpeedSet,                bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_ALDRIN_E */  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             bcat2SpeedSet,                bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_AC3X_E   */  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             bcat2SpeedSet,                bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_ALDRIN2_E*/  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             bcat2SpeedSet,                bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_FALCON_E */  {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_AC5P_E */    {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_AC5X_E */    {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_HARRIER_E */ {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_IRONMAN_E */ {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet},
        /* CPSS_PP_FAMILY_DXCH_AAS_E */     {  NULL,                       NULL,                         NULL,                       bcat2SpeedSet,             NULL,                         bcat2SpeedSet}
        };

        cpssOsMemCpy(
            PRV_NON_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portSpeedSrc.portSpeedSetFuncPtrArray),
            portSpeedSetFuncPtrArray,
            sizeof(portSpeedSetFuncPtrArray));
    }

    {
        PRV_CPSS_DXCH_PORT_SPEED_GET_FUN portSpeedGetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1]=
        {
        /* CPSS_PP_FAMILY_CHEETAH_E     */  NULL,
        /* CPSS_PP_FAMILY_CHEETAH2_E    */  NULL,
        /* CPSS_PP_FAMILY_CHEETAH3_E    */  NULL,
        /* CPSS_PP_FAMILY_DXCH_XCAT_E   */  xcatPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_AC5_E    */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_LION_E   */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  xcatPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_LION2_E  */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_LION3_E  */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_BOBCAT2_E*/  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_BOBCAT3_E*/  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_ALDRIN_E */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_AC3X_E */    lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_ALDRIN2_E */ lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_FALCON_E */  lionPortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_AC5P_E */    sip6_10_PortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_AC5X_E */    sip6_10_PortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_HARRIER_E */ sip6_10_PortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_IRONMAN_E */ sip6_10_PortSpeedGet,
        /* CPSS_PP_FAMILY_DXCH_AAS_E */     sip6_10_PortSpeedGet
        };

        cpssOsMemCpy(
            PRV_NON_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portSpeedSrc.portSpeedGetFuncPtrArray),
            portSpeedGetFuncPtrArray,
            sizeof(portSpeedGetFuncPtrArray));
    }
    {
        static const PRV_CPSS_DXCH_DEV_X_UNITDEFLIST  pattern_prv_dev_unitDefList[] =
        {
             { CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc2    [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E, &prv_paUnitDef_All_bobk   [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_ALDRIN_E,  CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_AC3X_E,    CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_BOBCAT3_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc3    [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_ALDRIN2_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin2[0]   , NULL}

            /* not relevant to sip6 devices */

            ,{ CPSS_MAX_FAMILY,               CPSS_BAD_SUB_FAMILY,               NULL                    , NULL}
        };


        cpssOsMemCpy(
            PRV_NON_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList),
            pattern_prv_dev_unitDefList,
            sizeof(pattern_prv_dev_unitDefList));

        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[0].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_bc2));
        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[1].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_bobk));
        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[2].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_aldrin));
        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[3].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_aldrin));
        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[4].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_bc3));
        PRV_NON_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList[5].unitsDescrAllPtr,
                PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc.prv_unitsDescrAll_aldrin2));


        PRV_NON_SHARED_GLOBAL_VAR_SET(
            mainPpDrvMod.portDir.portPaSrc.prv_paUnitsDrv.dev_x_unitDefList,
            PRV_NON_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portPaSrc.prv_dev_unitDefList));
    }


}

/**
 * @internal cpssGlobalNonSharedDbMainPpDrvModPhyDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non shared variables used in :
 *          module:mainPpDrv
 *          dir:phy (MPD)
 */
static GT_VOID cpssGlobalNonSharedDbMainPpDrvModPhyDirDataSectionInit
(
    GT_VOID
)
{
    prvCpssDxChPhyMpdNonSharedDbInit (&(PRV_MPD_NON_SHARED_DB));
}
#endif

/**
 * @internal cpssGlobalNonSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:cpssHwInit
 *
 */
static GT_VOID cpssGlobalNonSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_NON_SHARED_HW_INIT_DIR_HW_INIT_SRC_GLOBAL_VAR_INIT(dummy,1);
}

/**
 * @internal cpssGlobalNonSharedDbMainPpDrvModHwInitDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *
 */
static GT_VOID cpssGlobalNonSharedDbMainPpDrvModHwInitDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    cpssGlobalNonSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit();
#ifdef CHX_FAMILY
    cpssGlobalNonSharedDbMainPpDrvModDxChPortDirDataSectionInit();
    cpssGlobalNonSharedDbMainPpDrvModPhyDirDataSectionInit();
#endif
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global non shared variables used in :
 *         module:mainPpDrv
 *
 */
GT_VOID cpssGlobalNonSharedDbMainPpDrvModDataSectionInit
(
    GT_VOID
)
{
    /*add here directory  initializers*/
    cpssGlobalNonSharedDbMainPpDrvModHwInitDirDataSectionInit();
}




