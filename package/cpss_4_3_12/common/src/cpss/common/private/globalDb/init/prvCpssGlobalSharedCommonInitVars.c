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
* @file prvCpssGlobalCommonInitVars.c
*
* @brief This file  Initialize global shared variables used in  module:common
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/init/prvCpssGlobalCommonInitVars.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_STATUS mvHwsAvagoSerdesDbAllocate
(
    GT_VOID
);

/*global variables macros*/
#define PRV_SHARED_INTERRUPT_DIR_DXEXMX_SRC_GLOBAL_VAR_INIT(_var,_value) \
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.genericInterrupsDir.dxexmxInterruptsSrc._var,_value)

#define PRV_SHARED_INTERRUPT_DIR_COMMON_SRC_GLOBAL_VAR_INIT(_var,_value) \
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.genericInterrupsDir.genericInterruptsSrc._var,_value)

#define PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR_INIT(_var,_value) \
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.labServicesDir.portSrc._var,_value)

/**
 * @internal cpssGlobalSharedDbCommonModDiagDirGenDiagSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *
 */
static GT_VOID cpssGlobalSharedDbCommonModDiagDirGenDiagSrcDataSectionInit
(
    GT_VOID
)
{
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    PRV_SHARED_INTERRUPT_DIR_DXEXMX_SRC_GLOBAL_VAR_INIT(pollingSleepTime, 20);
#endif
}

/**
 * @internal cpssGlobalSharedDbCommonModHwInitDirInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables of hwInit used in :
 *          module:common
 *
 */
static GT_VOID cpssGlobalSharedDbCommonModHwInitDirInit
(
    GT_VOID
)
{
    PRV_SHARED_GLOBAL_VAR_SET(
        commonMod.genericHwInitDir.dmmBlockNotFoundIndication, (GT_UINTPTR)(-1));
    PRV_SHARED_GLOBAL_VAR_SET(
        commonMod.genericHwInitDir.prvCpssPrePhase1PpInitMode,PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_DEFAULT_INIT_E);
    PRV_SHARED_GLOBAL_VAR_SET(
        commonMod.configDir.dDrvPpHwSrc.onEmulator_devNum_iterateRamAction, 0xFF);
}


/**
 * @internal
 *           cpssGlobalSharedDbCommonModLabServicesDirSiliconIfSrcInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *
 */
static GT_VOID cpssGlobalSharedDbCommonModLabServicesDirSiliconIfSrcInit
(
    GT_VOID
)
{
    cpssSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceSpecInfoGlobal = (HWS_DEVICE_INFO*)cpssOsMalloc(sizeof(HWS_DEVICE_INFO) * HWS_MAX_DEVICE_NUM);
    cpssOsMemSet(cpssSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceSpecInfoGlobal,0,sizeof(HWS_DEVICE_INFO) * HWS_MAX_DEVICE_NUM);
}

/**
 * @internal
 *           cpssGlobalSharedDbCommonModSerdesDirSerdesIfSrcInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *
 */
static GT_VOID cpssGlobalSharedDbCommonModSerdesDirSerdesIfSrcInit
(
    GT_VOID
)
{
    cpssSharedGlobalVarsPtr->commonMod.serdesDir.mvHwsSerdesIfSrc.mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrl = 1;
    cpssSharedGlobalVarsPtr->commonMod.serdesDir.mvHwsSerdesIfSrc.serdesBgEnable                                 = GT_TRUE;
    cpssSharedGlobalVarsPtr->commonMod.serdesDir.mvHwsSerdesIfSrc.serdesBgClkBypassEnable                        = GT_FALSE;
}


/**
 * @internal
 *           cpssGlobalSharedDbCommonModLabServicesDirSiliconIfSrcInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *
 */
static GT_STATUS cpssGlobalSharedDbCommonModLabServicesDirAvagoSerdesIfSrcInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = GT_OK;
#ifndef ASIC_SIMULATION
    rc = mvHwsAvagoSerdesDbAllocate();
#endif
    return rc;
}

/**
 * @internal
 *           cpssGlobalSharedDbCommonModLabServicesDirInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *          dir:labServices/port
 *
 */
static GT_STATUS cpssGlobalSharedDbCommonModLabServicesDirInit
(
    GT_VOID
)
{
    GT_STATUS rc;
     /*add here source files   initializers*/
    cpssGlobalSharedDbCommonModLabServicesDirSiliconIfSrcInit();
    cpssGlobalSharedDbCommonModSerdesDirSerdesIfSrcInit();
    rc = cpssGlobalSharedDbCommonModLabServicesDirAvagoSerdesIfSrcInit();

    return rc;

}



/**
 * @internal cpssGlobalSharedDbCommonModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global shared variables used in :
 *         module:common
 *
 */
GT_STATUS cpssGlobalSharedDbCommonModDataSectionInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    /*add here directory  initializers*/
    cpssGlobalSharedDbCommonModHwInitDirInit();
    cpssGlobalSharedDbCommonModDiagDirGenDiagSrcDataSectionInit();
    rc = cpssGlobalSharedDbCommonModLabServicesDirInit();

    return rc;
}




