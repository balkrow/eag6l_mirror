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
* @file prvCpssGlobalSharedDb.c
*
* @brief This file provides handling of global variables DB.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifdef CHX_FAMILY
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcam.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChPhyMpdInit.h>
#endif

/*add here module initializers*/
#include <cpss/common/private/globalShared/init/prvCpssGlobalMainPpDrvInitVars.h>
#include <cpss/common/private/globalShared/init/prvCpssGlobalCommonInitVars.h>
#include <cpss/common/private/globalShared/init/prvCpssGlobalMainTmDrvInitVars.h>
#include <cpss/common/private/globalShared/init/prvCpssGlobalAskInitVars.h>


#define CPSS_SHM_FILENAME   "CPSS_SHM_DATA"
#define SHARED_DB_PREFIX     "/dev/shm/"


#define SHARED_DB_MAGIC     0xCAFEFACE
#define NON_SHARED_DB_MAGIC 0xFACECAFE
#define SHARED_MEMORY_SEM_NAME   "CPSS_SHM_INIT_SEM"

#define PRV_SHARED_HW_INFO(_devNum) PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(_devNum)->hwInfo[0]



PRV_CPSS_SHARED_GLOBAL_DB       *cpssSharedGlobalVarsPtr = NULL;
PRV_CPSS_SHARED_GLOBAL_DB        cpssSharedGlobalVars;

PRV_CPSS_NON_SHARED_GLOBAL_DB   *cpssNonSharedGlobalVarsPtr = NULL;
PRV_CPSS_NON_SHARED_GLOBAL_DB    cpssNonSharedGlobalVars ;
#ifdef SHARED_MEMORY
static GT_STATUS prvCpssDxChHwsSiliconIfCallbacksBind
(
    GT_VOID
);
GT_STATUS prvCpssDrvSysConfigPhase1BindCallbacks
(
    void
);

typedef GT_STATUS (* secondaryProcDevicelessInitFunc)(GT_VOID);
typedef GT_STATUS (* secondaryProcDeviceOrientedInitFunc)(GT_U8 devNum);

#endif
static GT_STATUS cpssGlobalSharedDbUnLock
(
    GT_VOID
);

static GT_STATUS cpssGlobalNonSharedDbDataSectionInit
(
    GT_VOID
);

#ifdef SHARED_MEMORY
extern GT_VOID prvBindDuplicatedAddrGetFunc(IN GT_U8 devNum);

extern GT_STATUS prvCpssPortManagerApIsConfigured
(
    IN GT_U8                  devNum,
    IN GT_U32                 portNum,
    OUT GT_BOOL               *apConfiguredPtr
);

extern GT_STATUS prvCpssPortManagerIsInDebugState
(
    IN  GT_U8                  devNum,
    IN  GT_U32                 portNum,
    OUT GT_BOOL               *isInDebugStatePtr
);

extern GT_STATUS prvCpssDxChPortDevInfo
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
);

extern GT_VOID hwPpPhase1DeviceSpecificIntCallbacksInit
(
    IN      GT_U8                                   devNum
);

extern GT_STATUS prvCpssHawkDedicatedEventsNonSharedConvertInfoSet
(
    IN GT_U8   devNum
);

extern GT_STATUS prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet
(
    IN GT_U8   devNum
);

extern GT_STATUS hwsAc3SerdesIfInit
(
    IN GT_U8 devNum
);

extern GT_STATUS prvCpssDrvPpIntMaskSetFptrFalconInit
(
    IN GT_U8   devNum
);

extern GT_STATUS prvCpssCommonPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);

extern GT_STATUS prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnable
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       enable
);

extern GT_STATUS prvCpssDxChTrunkFamilyInit
(
     IN CPSS_PP_FAMILY_TYPE_ENT  devFamily
);

static GT_STATUS prvCpssGlobalDbPpPhase1CallbacksInit
(
    IN      GT_U8                                   devNum
);

GT_STATUS prvPerFamilySecondaryClientInit(IN GT_U8 devNum);

static GT_STATUS prvCpssDxChHwsRegDbInit
(
    GT_U8 devNum
);

static GT_STATUS prvCpssGlobalDbTablesFormatInit
(
    IN      GT_U8                                   devNum
);

static GT_STATUS  prvCpssGlobalDbTablesAccessInit
(
    IN GT_U8    devNum
);

static GT_STATUS prvCpssGlobalDbDxChPortIfFunctionsObjInit
(
    IN GT_U8 devNum
);

static GT_STATUS prvCpssGlobalDbDxChPortMngEngineCallbacksInit
(
    IN GT_U8    devNum
);

static GT_STATUS  prvCpssGlobalDbBindDuplicatedAddrGetFunc
(
    IN GT_U8    devNum
);

static GT_STATUS prvCpssGlobalDbPortPmFuncBind
(
    IN  GT_U8                   devNum
);


static GT_STATUS prvCpssGlobalDbHwsFalconUnitBaseAddrCalcBind
(
    IN  GT_U8                   devNum
);

#endif


#define SHLIB_CHECK_STATUS(origFunc) \
{ \
    rc = origFunc; \
    if (GT_OK != rc) \
    { \
        PRV_CPSS_SHM_PRINT("Failure at: %d = %s\n",rc,#origFunc);\
        return rc; \
    } \
}

#ifdef SHARED_MEMORY

static GT_STATUS prvCpssDxChAc5HwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;
    hwsAlleycat5IfInitHwsDevFunc(devNum,&funcPtrsStc);

    return GT_OK;
}

static GT_STATUS hwsHarrierIfInitHwsDevFuncSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;
    funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
    hwsHarrierIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS hwsIronmanIfInitHwsDevFuncSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */
    HWS_EXT_FUNC_STC_PTR        extFuncPtrsStc;

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));

    extFuncPtrsStc.ppHwTraceEnable = (MV_OS_HW_TRACE_ENABLE_FUNC)cpssDrvPpHwTraceEnable;
    funcPtrsStc.extFunctionStcPtr = &extFuncPtrsStc;
    funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;

    hwsIronmanIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}


static GT_STATUS prvCpssDxChAc5xHwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;
    funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
    hwsPhoenixIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS prvCpssDxChAc5pHwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;
    funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
    hwsHawkIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}
static GT_STATUS prvCpssDxChFalconHwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsFalconIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS prvCpssDxChBc3HwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsBobcat3IfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS prvCpssDxChBc2HwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsBobcat2IfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS prvCpssDxChBobKHwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsBobKIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}


static GT_STATUS prvCpssDxChAdrinHwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsAldrinIfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

static GT_STATUS prvCpssDxChAlleycat3HwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsAlleycat3IfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}




static GT_STATUS prvCpssGlobalDbInitDuplicateAdressFunc
(
    GT_U8 devNum
)
{
    DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
    return GT_OK;
}

static GT_STATUS prvCpssGlobalDbInitDmaConvertFunc
(
    GT_U8 devNum
)
{
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum)=NULL;
    return GT_OK;
}


static GT_STATUS prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnableSecondClient
(
    IN  GT_U8         devNum
)
{
    return prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnable(devNum,GT_TRUE);
}


static GT_STATUS prvCpssDxChAdrin2HwsIfInitDevSecondClient
(
    GT_U8 devNum
)
{
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    hwsAldrin2IfInitHwsDevFunc(devNum,&funcPtrsStc);
    return GT_OK;
}

/*CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E*/
static const secondaryProcDeviceOrientedInitFunc bc2SubFamilyBobKFuncDb[]=
{
    hwsBobKPortsParamsSupModesMapSet,
    prvCpssDxChBobKHwsIfInitDevSecondClient,
    hwsBobKIfReload,
    prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnableSecondClient,
    NULL /*should be last*/
};

/*CPSS_PP_SUB_FAMILY_NONE_E*/
static const secondaryProcDeviceOrientedInitFunc bc2SubFamilyNoneFuncDb[]=
{
    prvCpssDxChBc2HwsIfInitDevSecondClient,
    hwsBobcat2IfReload,
    NULL /*should be last*/
};



static GT_STATUS prvCpssDxChBobcat2SubFamilySecondClientInit
(
    GT_U8 devNum
)
{
    const secondaryProcDeviceOrientedInitFunc *perSubFamilyInitFuncPtr;
    GT_U32 i;
    GT_STATUS rc;

    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
    {
        PRV_CPSS_SHM_PRINT("PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E\n");
        perSubFamilyInitFuncPtr = bc2SubFamilyBobKFuncDb;
    }
    else
    {
        perSubFamilyInitFuncPtr = bc2SubFamilyNoneFuncDb;
    }


    if(NULL!=perSubFamilyInitFuncPtr)
    {
        for(i=0;perSubFamilyInitFuncPtr[i];i++)
        {
            PRV_CPSS_SHM_PRINT("Start sub family %d init function %d\n",PRV_CPSS_PP_MAC(devNum)->devSubFamily,i);
            SHLIB_CHECK_STATUS(perSubFamilyInitFuncPtr[i](devNum));
            PRV_CPSS_SHM_PRINT("Finish init function %d with rc %d\n",i,rc);
        }
    }

     return GT_OK;
}


/*Due to const will be located in read only section.
Add here device oriented  callbacks.
*/
static const secondaryProcDeviceOrientedInitFunc lion2InitFuncDb[]=
{
    prvCpssGlobalDbInitDuplicateAdressFunc,
    NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc bc2InitFuncDb[]=
{
    prvCpssDxChPortDynamicPizzaArbiterWSInit,
    prvCpssDxChPortPizzaArbiterAllUnitsDrvSwOnlyInit,
    prvCpssGlobalDbInitDuplicateAdressFunc,
    prvCpssDxChBobcat2SubFamilySecondClientInit,
    NULL /*should be last*/
};



static const secondaryProcDeviceOrientedInitFunc bc3InitFuncDb[]=
{
   prvCpssDxChBc3HwsIfInitDevSecondClient,
   hwsBobcat3PortsParamsSupModesMapSet,
   hwsBobcat3IfReIoad,
   prvCpssDxChPortDynamicPizzaArbiterWSInit,
   prvCpssDxChPortPizzaArbiterAllUnitsDrvSwOnlyInit,
   prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnableSecondClient,
   prvCpssGlobalDbInitDuplicateAdressFunc,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc aldrin2InitFuncDb[]=
{
    prvCpssDxChAdrin2HwsIfInitDevSecondClient,
    hwsAldrin2PortsParamsSupModesMapSet,
    hwsAldrin2IfReload,
    prvCpssDxChPortDynamicPizzaArbiterWSInit,
    prvCpssDxChPortPizzaArbiterAllUnitsDrvSwOnlyInit,
    prvCpssGlobalDbInitDuplicateAdressFunc,
    prvCpssGlobalDbInitDmaConvertFunc,
    NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc xcat3InitFuncDb[]=
{
    hwsAc3IfReIoad,
    hwsAc3SerdesIfInit,
    prvCpssGlobalDbInitDuplicateAdressFunc,
    prvCpssDxChAlleycat3HwsIfInitDevSecondClient,
    NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc aldrinInitFuncDb[]=
{
    prvCpssDxChAdrinHwsIfInitDevSecondClient,
    hwsAldrinPortsParamsSupModesMapSet,
    hwsAldrinIfReload,
    prvCpssDxChPortDynamicPizzaArbiterWSInit,
    prvCpssDxChPortPizzaArbiterAllUnitsDrvSwOnlyInit,
    prvCpssGlobalDbInitDuplicateAdressFunc,
    prvCpssDxChHwPpErrataTailDropTablesAccessCallbackEnableSecondClient,
    NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc ac5pInitFuncDb[]=
{
   prvCpssDxChAc5pHwsIfInitDevSecondClient,
   hwsHawkIfReIoad,
   hwsHawkPortsParamsSupModesMapSet,
   prvCpssHawkNonSharedHwInfoFuncPointersSet,
   prvCpssHawkDedicatedEventsNonSharedConvertInfoSet,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc ac5xInitFuncDb[]=
{
   prvCpssDxChAc5xHwsIfInitDevSecondClient,
   hwsPhoenixIfReIoad,
   hwsPhoenixPortsParamsSupModesMapSet,
   prvCpssPhoenixNonSharedHwInfoFuncPointersSet,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc ac5InitFuncDb[]=
{
   hwsAlleycat5PortsParamsSupModesMapSet,
   hwsAlleycat5IfReIoad,
   prvCpssDxChAc5HwsIfInitDevSecondClient,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc ac3xInitFuncDb[]=
{
   prvCpssDxChPortPizzaArbiterAllUnitsDrvSwOnlyInit,
   hwsAldrinPortsParamsSupModesMapSet,
   hwsAldrinIfReload,
   prvCpssDxChPortDynamicPizzaArbiterWSInit,
   prvCpssDxChAdrinHwsIfInitDevSecondClient,
   prvCpssGlobalDbInitDuplicateAdressFunc,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc harrierInitFuncDb[]=
{
   hwsHarrierIfInitHwsDevFuncSecondClient,
   hwsHarrierIfReIoad,
   hwsHarrierPortsParamsSupModesMapSet,
   prvCpssHarrierNonSharedHwInfoFuncPointersSet,
   prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc ironmanInitFuncDb[]=
{
   hwsIronmanIfInitHwsDevFuncSecondClient,
   hwsIronmanIfReload,
   prvCpssIronmanNonSharedHwInfoFuncPointersSet,
   hwsIronmanPortsParamsSupModesMapSet,
   prvCpssDxChPhyMpdReducedInit,
   NULL /*should be last*/
};

static const secondaryProcDeviceOrientedInitFunc aasInitFuncDb[]=
{
   prvCpssAasNonSharedHwInfoFuncPointersSet,
   NULL /*should be last*/
};


static const secondaryProcDeviceOrientedInitFunc falconInitFuncDb[]=
{
   prvCpssDrvPpIntMaskSetFptrFalconInit,
   hwsFalconIfReIoad,
   hwsFalconPortElementsDbInit,
   hwsFalconPortsParamsSupModesMapSet,
   prvCpssFalconNonSharedHwInfoFuncPointersSet,
   prvCpssDxChFalconHwsIfInitDevSecondClient,
   NULL /*should be last*/
};



GT_STATUS prvCpssGlobalDbPpPhase1CallbacksInit
(
    IN      GT_U8                                   devNum
)
{
    hwPpPhase1DeviceSpecificIntCallbacksInit(devNum);
    return GT_OK;
}

GT_STATUS prvCpssGlobalDbTablesFormatInit
(
    IN      GT_U8                                   devNum
)
{
    initTablesFormatDb(devNum);
    return GT_OK;
}

static GT_STATUS  prvCpssGlobalDbTablesAccessInit
(
    IN GT_U8    devNum
)
{
    return  prvCpssDxChTablesAccessInit(devNum, GT_TRUE,GT_FALSE);
}


GT_STATUS prvCpssGlobalDbDxChPortIfFunctionsObjInit
(
    IN GT_U8 devNum
)
{
    return  prvCpssDxChPortIfFunctionsObjInit(devNum,  &(PORT_OBJ_FUNC(devNum)), PRV_CPSS_PP_MAC(devNum)->devFamily);
}

GT_STATUS prvCpssGlobalDbDxChPortMngEngineCallbacksInit
(
    IN GT_U8    devNum
)
{
    prvCpssDxChPortMngEngineCallbacksInit(devNum);
    return GT_OK;
}


GT_STATUS  prvCpssGlobalDbBindDuplicatedAddrGetFunc
(
    IN GT_U8    devNum
)
{
    /* attach the generic code with the Dx function */
    prvBindDuplicatedAddrGetFunc(devNum);
    return GT_OK;
}

GT_STATUS prvCpssGlobalDbPortPmFuncBind
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_PORT_PM_FUNC_PTRS     cpssPmFunc;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*From prvCpssDxChPortMgrInit*/
        cpssPmFunc.cpssPmIsFwFunc = prvCpssPortManagerApIsConfigured;
        cpssPmFunc.cpssPmIsInDebugStateFunc = prvCpssPortManagerIsInDebugState;

        rc = prvCpssDxChPortPmFuncBind(devNum, &cpssPmFunc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPmFuncBind failed=%d", rc);
        }
     }

    return rc;
}

GT_STATUS prvCpssGlobalDbHwsFalconUnitBaseAddrCalcBind
(
    IN  GT_U8                   devNum
)
{

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         /*From prvCpssDxChUnitBaseTableInit*/

        /* bind once : HWS with function that calc base addresses of units for : Falcon,Hawk,Phoenix */
        hwsFalconUnitBaseAddrCalcBind(prvCpssSip6HwsUnitBaseAddrCalc);
    }

    return GT_OK;
}

#endif

/**
* @internal cpssGlobalNonSharedDbPpMappingSyncSecondaryProcess function
* @endinternal
*
* @brief This function update non shared DB with unique PP mapping.
*              Should be called for secondary process.
*
* @param[in] hwInfoPtr - information regarding the devices that already mapped by operating system. In case equal 0 there is no device mapped.
* @param[in] numberOfDevices - information regarding the number of  devices that already mapped by operating system.
*                                                          In case equal 0 there is no device mapped.
*
*
*/
GT_VOID cpssGlobalNonSharedDbPpMappingSyncSecondaryProcess
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U32               numberOfDevices
)
{
   GT_U32 i,j;

   PRV_CPSS_SHM_PRINT("MappingSyncSecondaryProc:numberOfDevices %d \n",numberOfDevices);

   /* the correct device should be found*/
    for(i=0 ; i<numberOfDevices;i++)
    {
       for(j=0;j<PRV_CPSS_MAX_PP_DEVICES_CNS;j++)
       {
         if(PRV_CPSS_IS_DEV_EXISTS_MAC(j))
         {
            if((hwInfoPtr[i].hwAddr.busNo == (PRV_SHARED_HW_INFO(j).hwAddr.busNo))
                ||
                /*assume only one device on mbus*/
                ((hwInfoPtr[i].busType ==CPSS_HW_INFO_BUS_TYPE_MBUS_E)&&hwInfoPtr[i].busType== PRV_SHARED_HW_INFO(j).busType))
            {
                if(hwInfoPtr[i].hwAddr.devSel == PRV_SHARED_HW_INFO(j).hwAddr.devSel)
                {
                    if(hwInfoPtr[i].hwAddr.funcNo == PRV_SHARED_HW_INFO(j).hwAddr.funcNo)
                    {
                        PRV_CPSS_SHM_PRINT("Match found for device %d busNo 0x%08x\n",j,hwInfoPtr[i].hwAddr.busNo);

                        /*j is the correct device*/
                        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.cnm.start = hwInfoPtr[i].resource.cnm.start;
                        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.switching.start=hwInfoPtr[i].resource.switching.start;
                        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.resetAndInitController.start=hwInfoPtr[i].resource.resetAndInitController.start;
                        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.sram.start=hwInfoPtr[i].resource.sram.start;
                        PRV_CPSS_SHM_PRINT("ppMapping.cnm.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.cnm.start);
                        PRV_CPSS_SHM_PRINT("ppMapping.switching.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.switching.start);
                        PRV_CPSS_SHM_PRINT("ppMapping.resetAndInitController.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.resetAndInitController.start);
                        PRV_CPSS_SHM_PRINT("ppMapping.sram.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[j].ppMapping.sram.start);
                        if(hwInfoPtr->busType ==CPSS_HW_INFO_BUS_TYPE_MBUS_E)
                        {
                            hwInfoPtr[i].hwAddr.busNo = PRV_SHARED_HW_INFO(j).hwAddr.busNo;
                        }
                        /*found matching device for index i so break*/
                        break;
                    }
                }
            }
         }
      }
      if(j==PRV_CPSS_MAX_PP_DEVICES_CNS)
      {
       PRV_CPSS_SHM_PRINT("device index %d is not found at shared data base\n",i);
      }
    }
}
/**
* @internal cpssGlobalNonSharedDbPpMappingSyncPrimaryProcess function
* @endinternal
*
* @brief  This function add PP mapping to  non shared DB.
*               Should be called for master  process.
*               Master process will call this function when creating a driver.
* @param[in] hwInfoPtr - information regarding the device that already mapped .
* @param[in] devNum - device number
*
*
*/
GT_VOID cpssGlobalNonSharedDbPpMappingSyncPrimaryProcess
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U8                devNum
)
{

    if(hwInfoPtr->resource.cnm.start)
    {
        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.cnm.start = hwInfoPtr->resource.cnm.start;
    }

    if(hwInfoPtr->resource.switching.start)
    {
        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.switching.start=hwInfoPtr->resource.switching.start;
    }

    if(hwInfoPtr->resource.resetAndInitController.start)
    {
        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.resetAndInitController.start=hwInfoPtr->resource.resetAndInitController.start;
    }

    if(hwInfoPtr->resource.sram.start)
    {
        cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.sram.start=hwInfoPtr->resource.sram.start;
    }
    PRV_CPSS_SHM_PRINT("===devNum %d busNo 0x%08x ==\n",devNum,hwInfoPtr->hwAddr.busNo);
    PRV_CPSS_SHM_PRINT("ppMapping.cnm.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.cnm.start);
    PRV_CPSS_SHM_PRINT("ppMapping.switching.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.switching.start);
    PRV_CPSS_SHM_PRINT("ppMapping.resetAndInitController.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.resetAndInitController.start);
    PRV_CPSS_SHM_PRINT("ppMapping.sram.start : %p\n",cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[devNum].ppMapping.sram.start);
}



/**
* @internal cpssGlobalSharedDbAddProcess function
* @endinternal
*
* @brief add process to pid list(debug)
*
*/
static GT_VOID cpssGlobalSharedDbAddProcess
(
    GT_VOID
)
{
    GT_U32 index = 0;


     /*if it is not in shared library mode ,then do nothing*/
    if((cpssSharedGlobalVarsPtr!=NULL)&&
        (GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed))
    {
        cpssOsMutexLock(cpssSharedGlobalVarsPtr->procCpssDbMtx);

        for(index=0;index<CPSS_MAX_PIDS_SIZE;index++)
        {
            if(cpssSharedGlobalVarsPtr->pids[index]==0)
            {
                cpssSharedGlobalVarsPtr->pids[index]= cpssOsTaskGetPid();
                if(cpssSharedGlobalVarsPtr->clientNum<CPSS_MAX_PIDS_SIZE-1)
                {
                    /*support 255 clients*/
                    cpssSharedGlobalVarsPtr->clientNum++;
                }
                break;
            }
        }

        cpssOsMutexUnlock(cpssSharedGlobalVarsPtr->procCpssDbMtx);
    }
}
/**
* @internal cpssGlobalSharedDbRemoveProcess function
* @endinternal
*
* @brief remove process from pid list(debug)
*
*/
static GT_VOID cpssGlobalSharedDbRemoveProcess
(
    GT_VOID
)
{
    GT_U32 index = 0;
    GT_U32 myPid = cpssOsTaskGetPid();

    if((cpssSharedGlobalVarsPtr!=NULL)&&
        (GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed))
    {
        cpssOsMutexLock(cpssSharedGlobalVarsPtr->procCpssDbMtx);

        for(index=0;index<CPSS_MAX_PIDS_SIZE;index++)
        {
            if(cpssSharedGlobalVarsPtr->pids[index]==myPid)
            {
                cpssSharedGlobalVarsPtr->pids[index]=0;
                cpssSharedGlobalVarsPtr->clientNum--;
                break;
            }
        }

        cpssOsMutexUnlock(cpssSharedGlobalVarsPtr->procCpssDbMtx);
    }
}

void  cpssGlobalSharedDbCleanUp(void)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL  unlinkDone;
    if(PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.generalVars.initialization))
    {
        /*in case something will go wrong during the init*/
        rc = cpssGlobalSharedDbUnLock();
        PRV_CPSS_SHM_PRINT( "*** cpssGlobalSharedDbUnLock() called rc = %d\n",rc);
    }

    /*Try to destroy if last*/
    rc = cpssGlobalDbDestroy(GT_TRUE,GT_FALSE/*keep pointers ,other threads may still use*/,&unlinkDone);
    PRV_CPSS_SHM_PRINT( "*** cpssGlobalDbDestroy () called rc = %d ,unlinkDone %d\n",rc,unlinkDone);
}



/**
* @internal osGlobalSharedDbLock function
* @endinternal
*
* @brief  Lock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS cpssGlobalSharedDbLock
(
    GT_VOID
)
{
    CPSS_OS_SIG_SEM dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;

    /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
    {
        /* register shared memory content cleaner in case init will go wrong */
        if(GT_FALSE==cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpRegistered)
        {
            if (cpssOsTaskAtProcExit(cpssGlobalSharedDbCleanUp))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Cannot register cpssGlobalSharedDbCleanUp\n");
            }

            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpRegistered = GT_TRUE;
            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpIndex=
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.atExitIndex++;
         }

        /* Open named semaphore to guard  code from other clients. */
         rc = cpssOsNamedSemOpen(SHARED_MEMORY_SEM_NAME, &dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         rc = cpssOsNamedSemWait(dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.generalVars.initialization,GT_TRUE);
     }

     return rc;
}

/**
* @internal osGlobalSharedDbUnLock function
* @endinternal
*
* @brief  Unlock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS cpssGlobalSharedDbUnLock
(
    GT_VOID
)
{
    CPSS_OS_SIG_SEM  dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
    {

       /* Open named semaphore to guard   code from other clients. */
        rc = cpssOsNamedSemOpen(SHARED_MEMORY_SEM_NAME, &dbInitSem);
        if(rc!=GT_OK)
        {
             return rc;
        }

        rc = cpssOsNamedSemPost(dbInitSem);
        if(rc!=GT_OK)
        {
           return rc;
        }

        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.generalVars.initialization,GT_FALSE);

     }

    return rc;
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables .
 *
 */
static GT_STATUS cpssGlobalSharedDbDataSectionInit
(
    GT_VOID
)
{
    CPSS_NULL_PTR_CHECK_MAC(cpssSharedGlobalVarsPtr);

    /*cpss shared*/
    cpssSharedGlobalVarsPtr->magic = SHARED_DB_MAGIC;

    /*add here modules initializers*/
    cpssGlobalSharedDbMainPpDrvModDataSectionInit();

#if defined (INCLUDE_TM)
    cpssGlobalSharedDbMainTmDrvModDataSectionInit();
#endif
#ifdef CPSS_APP_PLATFORM
    cpssGlobalSharedDbAskModDataSectionInit();
#endif

    return cpssGlobalSharedDbCommonModDataSectionInit();
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize process interface to shared lib:
 *   Create a shared memory file, if one does not yet exist
 *   Map shared memory to process address space
 *
 */

GT_STATUS cpssGlobalDbShmemInit
(
    GT_BOOL *initDataSegmentPtr
)
{
    GT_VOID_PTR shmemPtr;
    GT_STATUS rc;
    GT_U32    size;
    GT_CHAR_PTR shmemName;


    size = sizeof(PRV_CPSS_SHARED_GLOBAL_DB);
    shmemName = CPSS_SHM_FILENAME;
    cpssSharedGlobalVarsPtr = NULL;

    rc = cpssOsGlobalDbShmemInit(size,shmemName,initDataSegmentPtr,&shmemPtr);
    if(rc==GT_OK)
    {
       cpssSharedGlobalVarsPtr =(PRV_CPSS_SHARED_GLOBAL_DB  *)shmemPtr;
    }

    return rc;
}

#ifdef SHARED_MEMORY
/**
 * @internal prvCpssSystemRecoveryCallbacksBind function
 * @endinternal
 *
 * @brief   binds system recovery callbacks
 *
 */
static GT_STATUS prvCpssSystemRecoveryCallbacksBind
(
    GT_VOID
)
{
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryParallelCompletionHandleFuncPtr = prvCpssDxChSystemRecoveryParallelCompletionHandle;

    return GT_OK;
}
#endif

/**
* @internal cpssGlobalDbInit function
* @endinternal
*
* @brief   Initialize CPSS global variables data base.
*          Global variables data base is allocated.
*          In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*          After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport - whether shared memory should be used for shared variables.
*                           in case equal GT_FALSE dynamic memory allocation is used for shared variables,
*                           otherwise shared memory is used s used for shared variables.
* @param[in] forceInit - whether to reset  shared data base to default values.
*                           in case equal GT_FALSE shared data base  is initialized only if allocated,
*                           otherwise shared data base  is initialized even if the data base is allocated.
*                           Required for PP reset.
* @param[in] hwInfoPtr - information regarding the resources that already mapped by operating system. In case equal 0 there is no device mapped.
* @param[in] numberOfDevices - information regarding the number of  resources that already mapped by operating system.
*                                                          In case equal 0 there is no device mapped.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbInit
(
    IN GT_BOOL             aslrSupport,
    IN GT_BOOL             forceInit,
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U32               numberOfDevices
)
{

   GT_STATUS rc = GT_OK;
   GT_BOOL   initDataSegment = GT_FALSE;
   GT_U32    initializerPid;
   GT_BOOL   sharedMemory = GT_FALSE;
   GT_U32    procDbMutexIndex = 0;
   CPSS_OS_FILE_TYPE_STC        fileInfo;
   GT_CHAR line[256];


#if !defined(LINUX)
    aslrSupport = GT_FALSE;
#endif

#ifdef SHARED_MEMORY
     sharedMemory = GT_TRUE;
#endif


   if(NULL == cpssSharedGlobalVarsPtr)
   {
       PRV_CPSS_SHM_PRINT("CPSS SHLIB : %s\n",sharedMemory?"Y":"N");
       if(GT_TRUE==sharedMemory)
       {
        PRV_CPSS_SHM_PRINT("CPSS ASLR  : %s\n",aslrSupport?"Y":"N");
       }
       if(GT_FALSE == aslrSupport)
       {
           cpssSharedGlobalVarsPtr = &cpssSharedGlobalVars;
           initDataSegment = GT_TRUE;
           cpssSharedGlobalVarsPtr->sharedMemoryUsed = GT_FALSE;
       }
       else
       {
           rc = cpssGlobalDbShmemInit(&initDataSegment);
           if(rc!=GT_OK)
           {
               /* Error */
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }
           if(NULL == cpssSharedGlobalVarsPtr)
           {
             /*something went wrong*/
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
           }

          PRV_CPSS_SHM_PRINT("CPSS SHMEM allocated at %p\n",cpssSharedGlobalVarsPtr);
       }


        /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/
        cpssSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;

       /*only one  process initialize data segment*/
       rc = cpssGlobalSharedDbLock();
       if(rc!=GT_OK)
       {
             return rc;
       }

       if(GT_TRUE==forceInit||((GT_TRUE == initDataSegment)&&
            (GT_FALSE == cpssSharedGlobalVarsPtr->dbInitialized)))
       {
           if(GT_TRUE==forceInit&&GT_TRUE==cpssSharedGlobalVarsPtr->dbInitialized)
           {
                if(cpssSharedGlobalVarsPtr->sharedMemoryUsed&&cpssSharedGlobalVarsPtr->procCpssDbMtx!=0)
                {
                    procDbMutexIndex = cpssSharedGlobalVarsPtr->procCpssDbMtx;
                }
           }
           /*save initialized PID*/
           initializerPid = cpssSharedGlobalVarsPtr->initializerPid;
           cpssOsMemSet(cpssSharedGlobalVarsPtr,0,sizeof(PRV_CPSS_SHARED_GLOBAL_DB));
           rc = cpssGlobalSharedDbDataSectionInit();
           if(rc!=GT_OK)
           {
               /* Error */
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }
           cpssSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;
           cpssSharedGlobalVarsPtr->dbInitialized = GT_TRUE;
           cpssSharedGlobalVarsPtr->initializerPid = initializerPid;
           cpssSharedGlobalVarsPtr->procCpssDbMtx = procDbMutexIndex;

       }
       else
       {
          /*shared memory already existed,check magic*/
          if(cpssSharedGlobalVarsPtr->magic != SHARED_DB_MAGIC)
          {
            /*something went wrong*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
          }
       }

       if(cpssNonSharedGlobalVars.nonVolatileDb.generalVars.verboseMode)
       {
           PRV_CPSS_SHM_PRINT("Dump memory map\n");
           fileInfo.type = CPSS_OS_FILE_REGULAR;
           if( (fileInfo.fd = cpssOsFopen("/proc/self/maps", "r",&fileInfo)) == 0)
           {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"  Failed to open /proc/self/maps");
           }

           while (cpssOsFgets( line, 256, fileInfo.fd ) != NULL )
           {
              cpssOsPrintf("%s\n", line);
           }

           cpssOsFclose(&fileInfo);
       }

       if(cpssSharedGlobalVarsPtr->sharedMemoryUsed&&cpssSharedGlobalVarsPtr->procCpssDbMtx==0)
       {

            /* Create the sync semaphore */
            if (cpssOsMutexCreate("procCpssDbMtx", &(cpssSharedGlobalVarsPtr->procCpssDbMtx)) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            PRV_CPSS_SHM_PRINT("processDbMtx created with id %d   \n",cpssSharedGlobalVarsPtr->procCpssDbMtx );
        }
        else
        {
            PRV_CPSS_SHM_PRINT("processDbMtx already exist with id %d   \n",cpssSharedGlobalVarsPtr->procCpssDbMtx );
        }
        cpssGlobalSharedDbAddProcess();

        if( cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            if(GT_FALSE==cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcRegistered)
            {
                rc = cpssOsTaskAtProcExit(cpssGlobalSharedDbRemoveProcess);
                if(rc!=GT_OK)
                {
                    return rc;
                }

                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcRegistered = GT_TRUE;
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcIndex=
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.atExitIndex++;
            }
        }

        rc = cpssGlobalSharedDbUnLock();
        if(rc!=GT_OK)
        {
            return rc;
        }
   }

   if(NULL == cpssNonSharedGlobalVarsPtr)
   {
       cpssNonSharedGlobalVarsPtr = &cpssNonSharedGlobalVars;

       cpssGlobalNonSharedDbDataSectionInit();

       if(numberOfDevices>0)
       {
        cpssGlobalNonSharedDbPpMappingSyncSecondaryProcess(hwInfoPtr,numberOfDevices);
       }
   }

   return rc;
}



/**
* @internal cpssGlobalDbDestroy function
* @endinternal
*
* @brief   UnInitialize CPSS global variables data base.
*          Global variables data base is deallocated.
*          In case  shared memory is used then shared memory is unlinked here.
*
* @param[in] unlink   - whether shared memory should be unlinked.
* @param[in] resetPointers   - whether to reset data base pointers
* @param[out] unlinkDonePtr   -GT_TRUE if this is the last process and unlink is performed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbDestroy
(
    IN GT_BOOL unlink,
    IN GT_BOOL resetPointers,
    OUT GT_BOOL *unlinkDonePtr
)
{
   GT_CHAR buffer[128]={'\0'};
   GT_STATUS rc;
   GT_U32 i;

   cpssOsStrCat(buffer,SHARED_DB_PREFIX);
   cpssOsStrCat(buffer,CPSS_SHM_FILENAME);


   if(unlinkDonePtr)
   {
    *unlinkDonePtr = GT_FALSE;
   }

   if(NULL != cpssSharedGlobalVarsPtr)
   {
       for(i=0;i<PRV_CPSS_MAX_PP_DEVICES_CNS;i++)
       {
           if(PRV_CPSS_IS_DEV_EXISTS_MAC(i))
           {
               PRV_CPSS_SHM_PRINT("Destroy  device %d  non shared DB\n",i);

               if(cpssNonSharedGlobalVarsPtr)
               {
                   hwsSerdesIfClose(i);
                   hwsPcsIfClose(i);
                   hwsMacIfClose(i);
#ifdef CHX_FAMILY
                   FREE_PTR_MAC(PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(i)->devFamily));
#endif
                   PRV_CPSS_SHM_PRINT("Destroy  device %d  non shared DB done.\n",i);
               }
           }
       }
        if(GT_FALSE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            cpssOsMemSet(cpssSharedGlobalVarsPtr,0,sizeof(PRV_CPSS_SHARED_GLOBAL_DB));
        }
        else
        {
            if(GT_TRUE == unlink)
            {
                if(cpssSharedGlobalVarsPtr->clientNum == 0)
                {
                    if(cpssSharedGlobalVarsPtr->procCpssDbMtx!=0)
                    {
                        cpssOsMutexDelete(cpssSharedGlobalVarsPtr->procCpssDbMtx);
                    }

                    rc = cpssOsGlobalDbShmemUnlink(buffer);
                    if(rc!=GT_OK)
                    {
                         return rc;
                    }

                    /* Remove named semaphore for clean-up */
                    rc = cpssOsNamedSemUnlink(SHARED_MEMORY_SEM_NAME);
                    if(rc!=GT_OK)
                    {
                         return rc;
                    }
                    if(unlinkDonePtr)
                    {
                     *unlinkDonePtr = GT_TRUE;
                    }
                }
            }
        }


   }

#ifdef CHX_FAMILY
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.dxChXcat3TablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.lion2TablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobcat2TablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobkTablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.falconTablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.hawkTablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.phoenixTablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.harrierTablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.ironman_L_TablesInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.directAccessBobcat2TableInfoPtr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobcat2B0TablesInfo_overrideA0Ptr);
    FREE_GLOBAL_NON_SHARED_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.directAccessBobkTableInfoPtr);
#endif

   if(GT_TRUE==resetPointers)
   {
        cpssSharedGlobalVarsPtr = NULL;
        cpssNonSharedGlobalVarsPtr = NULL;
   }

   return GT_OK;

}


/**
* @internal prvCpssGlobalDbDump function
* @endinternal
*
* @brief   Dump shared global DB attributes
*
* @retval GT_OK                    - anycase
*/
GT_STATUS prvCpssGlobalDbDump
(
    GT_VOID
)
{
    GT_U32 i,myPid;
    myPid = cpssOsTaskGetPid();

    cpssOsPrintf("Shared globals DB:\n");
    cpssOsPrintf("==================\n");

    if(cpssSharedGlobalVarsPtr)
    {
        cpssOsPrintf("Addr        :%p\n",cpssSharedGlobalVarsPtr);
        cpssOsPrintf("Magic       :0x%x\n",cpssSharedGlobalVarsPtr->magic);
        cpssOsPrintf("Master  Pid :0x%x\n",cpssSharedGlobalVarsPtr->initializerPid);
        cpssOsPrintf("Allocated by me:%d\n",(myPid==cpssSharedGlobalVarsPtr->initializerPid)?1:0);
        cpssOsPrintf("clientNum   :%d\n",cpssSharedGlobalVarsPtr->clientNum);
        cpssOsPrintf("SHMEM used  :%d\n",cpssSharedGlobalVarsPtr->sharedMemoryUsed);

        if(cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            cpssOsPrintf("Used by processes\n");
            for(i=0;i<CPSS_MAX_PIDS_SIZE;i++)
            {
                if(cpssSharedGlobalVarsPtr->pids[i]!=0)
                {
                  cpssOsPrintf("%d pid %d [0x%x] \n",i,cpssSharedGlobalVarsPtr->pids[i],cpssSharedGlobalVarsPtr->pids[i]);
                }
            }
        }
    }
    else
    {
        cpssOsPrintf("Not initialized\n");
    }




    cpssOsPrintf("Non-shared globals DB:\n");
    cpssOsPrintf("======================\n");

    if(cpssNonSharedGlobalVarsPtr)
    {
       cpssOsPrintf("Addr            :%p\n",cpssNonSharedGlobalVarsPtr);
       cpssOsPrintf("Magic           :0x%x\n",cpssNonSharedGlobalVarsPtr->magic);
    }
    else
    {
       cpssOsPrintf("Not initialized\n");
    }

    return GT_OK;
}
/**
* @internal prvCpssGlobalDbExistGet function
* @endinternal
*
* @brief   Check if  gllobal variables data base was allocated.
*
* @retval GT_TRUE                    global DB exist
* @retval GT_FAIL                  - global DB does not exist
*/
GT_BOOL prvCpssGlobalDbExistGet
(
    GT_VOID
)
{
    return cpssSharedGlobalVarsPtr!=NULL;
}

#ifdef SHARED_MEMORY

GT_STATUS prvPerFamilySecondaryClientInit(IN GT_U8 devNum)
{
    GT_STATUS rc = GT_OK;
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portTypeOptions;/* options available for port */
    GT_U32      i;
    GT_U32      indexInTable;


    const secondaryProcDeviceOrientedInitFunc * perFamilyInitFuncDb[]=
    {
         xcat3InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_XCAT3_E*/
         ac5InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_AC5_E*/
         NULL ,/*CPSS_PP_FAMILY_DXCH_LION_E*/
         NULL ,/*CPSS_PP_FAMILY_DXCH_XCAT2_E*/
         lion2InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_LION2_E*/
         NULL ,/*CPSS_PP_FAMILY_DXCH_LION3_E*/
         bc2InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_BOBCAT2_E*/
         bc3InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_BOBCAT3_E*/
         aldrinInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_ALDRIN_E*/
         ac3xInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_AC3X_E*/
         aldrin2InitFuncDb ,/*CPSS_PP_FAMILY_DXCH_ALDRIN2_E*/
         falconInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_FALCON_E*/
         ac5pInitFuncDb,/*CPSS_PP_FAMILY_DXCH_AC5P_E*/
         ac5xInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_AC5X_E*/
         harrierInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_HARRIER_E*/
         ironmanInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_IRONMAN_E*/
         aasInitFuncDb ,/*CPSS_PP_FAMILY_DXCH_AAS_E*/
    };

    PRV_CPSS_SHM_LOG_ENTER

    /* initialize info about those PP's family */

    if((PRV_CPSS_PP_MAC(devNum)->devFamily)<CPSS_PP_FAMILY_DXCH_XCAT3_E||
        (PRV_CPSS_PP_MAC(devNum)->devFamily)>CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
       PRV_CPSS_SHM_PRINT("Device %d (Family %d) not handled at prvPerFamilySecondaryClientInit\n",
        devNum,PRV_CPSS_PP_MAC(devNum)->devFamily);

       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    if(PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily) == NULL)
    {
        /* this family was not initialized yet */
        PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily)=
            cpssOsMalloc(sizeof(PRV_CPSS_FAMILY_INFO_STC));

        if (PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily) == NULL)
        {
            CPSS_LOG_INFORMATION_MAC("hwPpPhase1Part1(..) : prvCpssFamilyInfoArray[devPtr->devFamily] == NULL : GT_OUT_OF_CPU_MEM");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily),
                        0,sizeof(PRV_CPSS_FAMILY_INFO_STC));

       PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily)->numReferences = 1;

        /* initialize specific family trunk info */
        prvCpssDxChTrunkFamilyInit(PRV_CPSS_PP_MAC(devNum)->devFamily);
    }
    else
    {
        PRV_CPSS_NON_SHARED_FAMILY_INFO(PRV_CPSS_PP_MAC(devNum)->devFamily)->numReferences++;
    }

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;


    for(portTypeOptions = PRV_CPSS_XG_PORT_XG_ONLY_E; portTypeOptions < PRV_CPSS_XG_PORT_OPTIONS_MAX_E; portTypeOptions++)
    {
        PORT_OBJ_FUNC(devNum).setPortSpeed[portTypeOptions] =
            PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedSetFuncPtrArray)[PRV_CPSS_PP_MAC(devNum)->devFamily- CPSS_PP_FAMILY_START_DXCH_E - 1][portTypeOptions];
    }

    PORT_OBJ_FUNC(devNum).getPortSpeed =
        PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedGetFuncPtrArray)[PRV_CPSS_PP_MAC(devNum)->devFamily- CPSS_PP_FAMILY_START_DXCH_E - 1];

    indexInTable = PRV_CPSS_PP_MAC(devNum)->devFamily-CPSS_PP_FAMILY_DXCH_XCAT3_E;

    if(NULL!=perFamilyInitFuncDb[indexInTable])
    {
        for(i=0;perFamilyInitFuncDb[indexInTable][i];i++)
        {
            PRV_CPSS_SHM_PRINT("Start index %d family %d init function %d\n",indexInTable,PRV_CPSS_PP_MAC(devNum)->devFamily,i);
            SHLIB_CHECK_STATUS(perFamilyInitFuncDb[indexInTable][i](devNum));
            PRV_CPSS_SHM_PRINT("Finish init function %d with rc %d\n",i,rc);
        }
    }

    PRV_CPSS_SHM_LOG_EXIT

    return rc;
}


GT_STATUS prvCpssDxChHwsRegDbInit
(
    GT_U8 devNum
)
{
   GT_STATUS rc = GT_OK;

   if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
   {
       rc =  hwsPhoenixRegDbInit(devNum);
   }
   else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
   {
       rc =  hwsHawkRegDbInit(devNum);
   }
   else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
   {
       rc =  hwsHarrierRegDbInit(devNum);
   }
   else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
   {
       rc =  hwsIronmanRegDbInit(devNum);
   }
   else
   {
        PRV_CPSS_SHM_PRINT("Device %d does not use HWS regDb\n",devNum);
   }

   return rc;
}

static GT_STATUS prvCpssDxChHwsSiliconIfCallbacksBind
(
    GT_VOID
)
{


     /*copy from prvCpssDxChPortIfCfgInit*/
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemCopyFuncGlobalPtr =
         (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemSetFuncGlobalPtr =
         cpssOsMemSet;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsTimerWkFuncGlobalPtr =
         cpssOsTimerWkAfter;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsFreeFuncGlobalPtr =
         cpssOsFree;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMallocFuncGlobalPtr=
         cpssOsMalloc;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsStrCatFuncGlobalPtr =
         cpssOsStrCat;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsTimerGetFuncGlobalPtr =
         cpssOsTimeRT;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterSetFuncGlobalPtr =
         (MV_REG_ACCESS_SET)genRegisterSet;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterGetFuncGlobalPtr =
         (MV_REG_ACCESS_GET)genRegisterGet;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegGetFuncGlobalPtr=
            (MV_SERDES_REG_ACCESS_GET)prvCpssGenPortGroupSerdesReadRegBitMask;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceTemperatureGetFuncGlobalPtr =
         (MV_DEVICE_TEMPERATURE_GET)prvCpssGenDeviceTemperatureGet;
     cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegSetFuncGlobalPtr =
            ( MV_SERDES_REG_ACCESS_SET)prvCpssGenPortGroupSerdesWriteRegBitMask;
      cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsExactDelayGlobalPtr =
                prvCpssCommonPortTraceDelay;
      cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegSetFuncGlobalPtr =
            cpssDrvHwPpResetAndInitControllerWriteReg;
      cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegGetFuncGlobalPtr =
            cpssDrvHwPpResetAndInitControllerReadReg;
      cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldSetFuncGlobalPtr =
           cpssDrvHwPpResetAndInitControllerSetRegField;
      cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldGetFuncGlobalPtr =
           cpssDrvHwPpResetAndInitControllerGetRegField;

      return GT_OK;

}

static GT_STATUS prvCpssGlobalDbInitDevicelessDb
(
    GT_VOID
)
{
   GT_STATUS rc;
   GT_U32 i;

   /*Due to const will be located in read only section.
   Add here deviceless callbacks.
   */
   static const secondaryProcDevicelessInitFunc devicelessInitFuncDb[]=
   {
       prvCpssDxChHwsSiliconIfCallbacksBind,
       prvCpssDrvSysConfigPhase1BindCallbacks,
       prvCpssSystemRecoveryCallbacksBind,
       NULL /*should be last*/
   };

   /*Due to const will be located in read only section.
   Add here deviceless callbacks names.
   */

   static GT_CHAR_PTR const devicelessInitFuncDbNames[]=
   {
       "prvCpssDxChHwsSiliconIfCallbacksBind",
       "prvCpssDrvSysConfigPhase1BindCallbacks",
       "prvCpssSystemRecoveryCallbacksBind",
       NULL /*should be last*/
   };

   for(i=0;devicelessInitFuncDb[i];i++)
   {
     PRV_CPSS_SHM_PRINT("Start devicelessInitFuncDb[%d] %s\n",i,devicelessInitFuncDbNames[i]);
     rc = devicelessInitFuncDb[i]();
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"%s failed=%d",devicelessInitFuncDbNames[i], rc);
     }
     PRV_CPSS_SHM_PRINT("Finish devicelessInitFuncDb[%d] %s\n",i,devicelessInitFuncDbNames[i]);
   }

   return GT_OK;
}


static GT_STATUS prvCpssGlobalDbInitDeviceOrientedDb
(
    GT_U32 devNum
)
{
   GT_STATUS rc;
   GT_U32 i;

   /*Due to const will be located in read only section.
   Add here device oriented  callbacks.
   */
   static const secondaryProcDeviceOrientedInitFunc deviceOrientedInitFuncDb[]=
   {
       prvCpssGlobalDbTablesFormatInit,
       prvCpssGlobalDbTablesAccessInit,
       prvCpssGlobalDbPpPhase1CallbacksInit,
       prvCpssGlobalDbDxChPortIfFunctionsObjInit,
       prvPerFamilySecondaryClientInit,
       prvCpssDxChHwsRegDbInit,
       prvCpssGlobalDbDxChPortMngEngineCallbacksInit,
       prvCpssDxChPortSerdesFunctionsObjInit,
       prvCpssGlobalDbBindDuplicatedAddrGetFunc,
       prvCpssGlobalDbPortPmFuncBind,
       prvCpssGlobalDbHwsFalconUnitBaseAddrCalcBind,
       initTablesDbSip6,
       NULL /*should be last*/
   };

   /*Due to const will be located in read only section.
   Add here device oriented callbacks names.
   */

   static GT_CHAR_PTR const deviceOrientedInitFuncDbNames[]=
   {
       "prvCpssGlobalDbTablesFormatInit",
       "prvCpssGlobalDbTablesAccessInit",
       "prvCpssGlobalDbPpPhase1CallbacksInit",
       "prvCpssGlobalDbDxChPortIfFunctionsObjInit",
       "prvPerFamilySecondaryClientInit",
       "prvCpssDxChHwsRegDbInit",
       "prvCpssGlobalDbDxChPortMngEngineCallbacksInit",
       "prvCpssDxChPortSerdesFunctionsObjInit",
       "prvCpssGlobalDbBindDuplicatedAddrGetFunc",
       "prvCpssGlobalDbPortPmFuncBind",
       "prvCpssGlobalDbHwsFalconUnitBaseAddrCalcBind",
       "initTablesDbSip6",
       NULL /*should be last*/
   };


   for(i=0;deviceOrientedInitFuncDb[i];i++)
   {
     PRV_CPSS_SHM_PRINT("Start deviceOrientedInitFuncDb[%d] %s\n",i,deviceOrientedInitFuncDbNames[i]);
     rc = deviceOrientedInitFuncDb[i](devNum);
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"%s failed=%d",deviceOrientedInitFuncDbNames[i], rc);
     }
     PRV_CPSS_SHM_PRINT("Finish deviceOrientedInitFuncDb[%d] %s\n",i,deviceOrientedInitFuncDbNames[i]);
   }

   return GT_OK;
}


GT_STATUS cpssGlobalNonSharedDbInit
(
    GT_VOID
)
{
    GT_U32 i;
    GT_STATUS rc;
    CPSS_PP_FAMILY_TYPE_ENT        devFamily;      /* device family */
    GT_BOOL                        processInitialized = GT_FALSE;

    for(i=0;i<PRV_CPSS_MAX_PP_DEVICES_CNS;i++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(i))
        {

            devFamily = PRV_CPSS_PP_MAC(i)->devFamily - CPSS_PP_FAMILY_START_DXCH_E - 1;
            PRV_CPSS_SHM_PRINT("Found device %d [family %d] .Initializing data base \n",i,devFamily);

            if(cpssNonSharedGlobalVarsPtr)
            {
                if(GT_FALSE ==processInitialized)
                {
                  SHLIB_CHECK_STATUS(prvCpssGlobalDbInitDevicelessDb());
                  processInitialized = GT_TRUE;
                }
                cpssOsMemSet(&(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i])),0,sizeof(PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA));

                SHLIB_CHECK_STATUS(prvCpssGlobalDbInitDeviceOrientedDb(i));
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDeviceNumberByHwInfoGet function
* @endinternal
*
* @brief  This function find  device number  by hwInfo.
*         Should be called for secondary   process in order to understand which CPSS device number was mapped to
*         specific hwInfo.
*
* @param[in]  hwInfoPtr - information regarding the device that already mapped .
* @param[out] devNumPtr - (pointer to) device number
*
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - such device was not found
*/
GT_STATUS prvCpssDeviceNumberByHwInfoGet
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    OUT GT_U8                *devNumPtr
)
{
    GT_U32 j;
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(devNumPtr);

    for(j=0;j<PRV_CPSS_MAX_PP_DEVICES_CNS;j++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(j))
        {
          if((hwInfoPtr->hwAddr.busNo == (PRV_SHARED_HW_INFO(j).hwAddr.busNo))
              ||
              /*assume only one device on mbus*/
              ((hwInfoPtr->busType ==CPSS_HW_INFO_BUS_TYPE_MBUS_E)&&hwInfoPtr->busType== PRV_SHARED_HW_INFO(j).busType))
          {
              if(hwInfoPtr->hwAddr.devSel == PRV_SHARED_HW_INFO(j).hwAddr.devSel)
              {
                  if(hwInfoPtr->hwAddr.funcNo == PRV_SHARED_HW_INFO(j).hwAddr.funcNo)
                  {
                      /*found matching device so break*/
                      *devNumPtr = j;
                      break;
                  }
              }
          }
        }
    }
    if(j==PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"Device matching requested [busNo 0x%x ][devSel 0x%x ] [funcNo 0x%x] not found.",
            hwInfoPtr->hwAddr.busNo, hwInfoPtr->hwAddr.devSel,hwInfoPtr->hwAddr.funcNo);
    }

    return rc;
 }



#endif

GT_U32  prvCpssBmpIsZero
(
    GT_VOID * portsBmpPtr
)
{
    /* BMP of ports with no ports members */
    CPSS_PORTS_BMP_STC cpssAllZeroPortsBmp = {{0}};

    cpssOsMemSet(&cpssAllZeroPortsBmp,0,sizeof(CPSS_PORTS_BMP_STC));

    return ((0 ==cpssOsMemCmp(portsBmpPtr,&cpssAllZeroPortsBmp,sizeof(CPSS_PORTS_BMP_STC))) ? 1 : 0);
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables .
 *
 */
GT_STATUS cpssGlobalNonSharedDbDataSectionInit
(
    GT_VOID
)
{
    /*cpss non-shared reset (w/o nonVolatileDb )*/
    cpssOsMemSet(&cpssNonSharedGlobalVarsPtr->commonMod,0,sizeof(PRV_CPSS_COMMON_MOD_NON_SHARED_GLOBAL_DB));
    cpssOsMemSet(&cpssNonSharedGlobalVarsPtr->mainPpDrvMod,0,sizeof(PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB));
    cpssOsMemSet(cpssNonSharedGlobalVarsPtr->nonSharedDeviceSpecificDb,0,
        sizeof(PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA)*PRV_CPSS_MAX_PP_DEVICES_CNS);

    /*initialize function pointers here*/
    cpssGlobalNonSharedDbExtDrvFuncInit();
    cpssGlobalNonSharedDbOsFuncInit();

    /*add here modules initializers*/

    PRV_CPSS_SHM_PRINT("Start initializing non-shared database\n");
    cpssGlobalNonSharedDbMainPpDrvModDataSectionInit();
    cpssGlobalNonSharedDbCommonModDataSectionInit();

#ifdef SHARED_MEMORY
    /*This code is relevant only for non-first clients in shared library mode.
            Due to ASLR the function pointers should be rebinded/*/
    cpssGlobalNonSharedDbInit();

#endif
    PRV_CPSS_SHM_PRINT("Done initializing non-shared database\n");

    return GT_OK;
}

#ifdef SHARED_MEMORY

/**
 * @internal prvCpssCallbackFuncIdGet function
 * @endinternal
 *
 * @brief  Get the pointer to compare function
 *
 * @note   APPLICABLE DEVICES:      All devices
 *
 *
 * @param[in] compareFunc     -compare function pointer
 *
 * @retval ID of compare function.In case not found 0 is returned
 */
GT_U32  prvCpssCallbackFuncIdGet
(
    GT_VOID *  func
)
{
    GT_U32    ind,unit;
    const GT_VOID * tmp;

    for(unit=GT_CALLBACK_LIB_ID_FIRST_E;unit<GT_CALLBACK_LIB_ID_LAST_E;unit++)
    {
        ind =0;
        if((GT_INTFUNCPTR *)cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[unit])
        {
           do
           {
             tmp =((GT_INTFUNCPTR *)cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[unit])[ind];

             if(tmp==func)
             {
               return ((unit<<PRV_UNIT_CB_FUNC_OFFSET)|ind);
             }
             ind++;
            }while(tmp);
        }
    }

    return 0;
}
#endif


GT_STATUS prvCpssGlobalDbCnmVirtualAdressGet
(
    GT_U8    device,
    GT_U32  *cnmVirtualAddrLowPtr,
    GT_U32  *cnmVirtualAddrHighPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(cnmVirtualAddrLowPtr&&cnmVirtualAddrHighPtr)
    {
        *cnmVirtualAddrLowPtr = (((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.cnm.start)& 0x00000000ffffffffL);
#if __WORDSIZE == 64
        *cnmVirtualAddrHighPtr = ((((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.cnm.start)>>32)& 0x00000000ffffffffL);
#else
        *cnmVirtualAddrHighPtr = 0;
#endif

        rc = GT_OK;
    }

    return rc;
}

GT_STATUS prvCpssGlobalDbSwitchVirtualAdressGet
(
    GT_U8    device,
    GT_U32  *switchVirtualAddrLowPtr,
    GT_U32  *switchVirtualAddrHighPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(switchVirtualAddrLowPtr&&switchVirtualAddrHighPtr)
    {
        *switchVirtualAddrLowPtr = (((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.switching.start)& 0x00000000ffffffffL);
#if __WORDSIZE == 64
        *switchVirtualAddrHighPtr = ((((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.switching.start)>>32)& 0x00000000ffffffffL);
#else
        *switchVirtualAddrHighPtr = 0;
#endif

        rc = GT_OK;
    }

    return rc;
}


GT_STATUS prvCpssGlobalDbDfxVirtualAdressGet
(
    GT_U8    device,
    GT_U32  *dfxVirtualAddrLowPtr,
    GT_U32  *dfxVirtualAddrHighPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(dfxVirtualAddrLowPtr&&dfxVirtualAddrHighPtr)
    {
        *dfxVirtualAddrLowPtr = (((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.resetAndInitController.start)& 0x00000000ffffffffL);
#if __WORDSIZE == 64
        *dfxVirtualAddrHighPtr = ((((uintptr_t)cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[device].ppMapping.resetAndInitController.start)>>32)& 0x00000000ffffffffL);
#else
        *dfxVirtualAddrHighPtr = 0;
#endif

        rc = GT_OK;
    }

    return rc;
}


GT_STATUS prvCpssGlobalDbHwInfoGet
(
    GT_U8    device,
    GT_U32  *busNoPtr,
    GT_U32  *devSelPtr,
    GT_U32  *funcNoPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(busNoPtr&&devSelPtr&&funcNoPtr)
    {
        *busNoPtr =PRV_SHARED_HW_INFO(device).hwAddr.busNo;
        *devSelPtr =PRV_SHARED_HW_INFO(device).hwAddr.devSel;
        *funcNoPtr =PRV_SHARED_HW_INFO(device).hwAddr.funcNo;
         rc = GT_OK;
    }

    return rc;
}




