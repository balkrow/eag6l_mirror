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
* @file prvCpssCommonModGlobalSharedDb.h
*
* @brief This file define common module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssCommonModGlobalSharedDb
#define __prvCpssCommonModGlobalSharedDb

#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortApInitIfPrv.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElementsPrv.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_CON_GEN_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,src/cpssDriver/pp/ config  directory, src file : cpssDrvPpConGenInit.c
 *
*/
typedef struct
{
    /* array of pointers to the valid driver devices
       index to this array is devNum.
       real type of array's element is pointer to PRV_CPSS_DRIVER_PP_CONFIG_STC */
    GT_VOID * prvCpssDrvPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* array including all driver management interface objects in Unit
       real type of array's element is pointer to PRV_CPSS_DRV_OBJ_STC */
    GT_VOID * prvCpssDrvPpObjConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* pointer to the driver list node.
       real type is pointer to PRV_CPSS_HW_DRIVER_LIST_NODE_STC */
    GT_VOID * cpssHwDriverListPtr;
    /* driver list access mutex */
    CPSS_OS_MUTEX                       prvCpssHwDriverListMtx;

} PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_CON_GEN_INIT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_HW_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,src/cpssDriver/pp/ hardware  directory, src file : prvCpssDrvPpHwApi.c
 *
*/
typedef struct
{
    GT_U8    onEmulator_devNum_iterateRamAction;
    /* on emulator : indication to check that the switch still accessible after every read/write from/to it*/
    GT_U32   onEmulator_checkAccessToSwitchIsOk_enable;
    /* on emulator : indication to sleep after every read/write */
    GT_U32   onEmulator_sleep;
    /* on emulator : indication to print after 'x' read/write operations */
    GT_U32   onEmulator_print_counter_max;
    /* on emulator : current counter related to onEmulator_print_counter_max */
    GT_U32   onEmulator_print_counter;

}PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_HW_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,src/cpssDriver/pp/ config  directory
 *   in common module ,src/cpssDriver/pp/ hardware  directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_CON_GEN_INIT_SRC_GLOBAL_DB dDrvPpConGenInitSrc;
    PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_DRV_PP_HW_SRC_GLOBAL_DB           dDrvPpHwSrc;
} PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,hwInit directory
*/
typedef struct
{
    /** main array of pointers to p[er-device info */
    void* prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** Init mode for PrePhase1 stage */
    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT prvCpssPrePhase1PpInitMode;

    /** flag needed by Golden model that may not have the DFX register that hold the
       core clock but also used for core clock after soft reset */
    GT_U32 simUserDefinedCoreClockMHz;

    /** This structure contains all Cpss Generic global data */
    PRV_CPSS_GEN_GLOBAL_DATA_STC sysGenGlobalInfo;

    GT_UINTPTR   dmmBlockNotFoundIndication;

    /** binary semaphore to emulate task lock without need to call OS task locking method */
    CPSS_OS_MUTEX   prvCpssLockSem;

#ifdef CPSS_API_LOCK_PROTECTION
    /** CPSS API mutual exclusion protection mutex for non device specific API */
    CPSS_OS_MUTEX   prvCpssApiLockDevicelessMtx[PRV_CPSS_MAX_FUNCTIONALITY_RESOLUTION_CNS] ;
    /** CPSS API mutual exclusion protection mutex for  device specific API */
    CPSS_OS_MUTEX   prvCpssApiLockByDevNumAndFunctionalityMtx
        [PRV_CPSS_MAX_PP_DEVICES_CNS][PRV_CPSS_MAX_DEV_FUNCTIONALITY_RESOLUTION_CNS] ;
    /** CPSS API mutual exclusion protection mutex for non device specific and unnested API  */
    CPSS_OS_MUTEX   prvCpssApiLockDevicelessMtxZeroLevel;
#endif /*CPSS_API_LOCK_PROTECTION*/

    /* check if the WM log was enabled (relevant for WM and for ASIM - CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC) */
    GT_U32  isWmLogOpen;
} PRV_CPSS_COMMON_MOD_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_COMMON_PORT_MANAGER_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,port directory, src file :
 *   cpssCommonPortManager.c
*/
typedef struct
{
    GT_BOOL isPortMgrEnable[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_BOOL stageInit[CPSS_MAX_PORTS_NUM_CNS];/* prevent for PM log to print in loops the init stage, now its only print one time for a change */
    GT_BOOL cdrPrintStatusLock[CPSS_MAX_PORTS_NUM_CNS];/* prevent for PM log to print in loops the cdr lock status, now its only print one time for a change */
    GT_U32  globalTimeStampSec;
    GT_U32  globalTimeStampNSec;
    GT_U32  globalTimeStampSecEnd;
    GT_U32  globalTimeStampNSecEnd;
} PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_COMMON_PORT_MANAGER_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,port directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_COMMON_PORT_MANAGER_SRC_GLOBAL_DB   commonPortManagerSrc;
} PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_ANP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory (port/common), src
 *   file : mvHwsPortAnp.c
*/
typedef struct
{
    GT_BOOL                     mvHwsAnpUseCmdTable;
    GT_U32                      mvHwsAnpTimerDisable;
    MV_HWS_ANP_PORT_DATA_STC    mvHwsAnpPortDB;
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_ANP_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_PORT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsIpcApis.c, mvHwsPortInitIf.c
*/
typedef struct
{
    /* IPC queue IDs pool */
    GT_UINTPTR                  hwsIpcChannelHandlers[HWS_MAX_DEVICE_NUM];
    GT_U32                      hwsIpcQueueIdPool[HWS_MAX_DEVICE_NUM][MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM];

    MV_HWS_GLOBALS              *apGlobalsCfgPtr;
    MV_HWS_GLOBALS              apGlobalsCfgSnap;
    GT_BOOL                     apEngineInitOnDev[HWS_MAX_DEVICE_NUM][32]; /* check with CPSS real num of devs */
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_PORT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_COMMON_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsPortModeElements.c
*/
typedef struct
{
    /* Array of Devices - each device is an array of ports */
    MV_HWS_DEVICE_PORTS_ELEMENTS hwsDevicesPortsElementsArrayGlobal[HWS_MAX_DEVICE_NUM];
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_COMMON_SRC_GLOBAL_DB;

typedef struct
{
    /* An array of RSFEC counters to store the value accumulated by resetting the counters by reading */
    MV_HWS_RSFEC_COUNTERS_STC addedRsfecResultGlobal[CPSS_MAX_PORTS_NUM_CNS];
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_RSFEC_CW_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SILICON_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsSiliconIf.c
*/
typedef struct
{
    /* Contains related data for specific device */
    HWS_DEVICE_INFO* hwsDeviceSpecInfoGlobal;
    /* store base address and unit index per unit per device type */
    HWS_UNIT_INFO hwsDeviceSpecUnitInfo[LAST_SIL_TYPE][LAST_UNIT];

    MV_OS_MICRO_DELAY_FUNC          hwsOsMicroDelayGlobalPtr;
    MV_SIL_ILKN_REG_DB_GET          hwsIlknRegDbGetFuncGlobalPtr;
    MV_OS_HW_TRACE_ENABLE_FUNC      hwsPpHwTraceEnableGlobalPtr;
    /* TBD - need to update function signature to get device type */

} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SILICON_IF_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SERVICE_CPU_FW_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsServiceCpuFw.c
*/
typedef struct
{

    GT_U32 mvHwsServiceCpuEnabled[HWS_MAX_DEVICE_NUM];
    GT_U32 mvHwsServiceCpuDbaEnabled;

} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SERVICE_CPU_FW_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_AVAGO_SERDES_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvAvagoSerdesIf.c
*/
typedef struct
{
    GT_VOID * aaplSerdesDb;
}
PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_AVAGO_SERDES_IF_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,labServices directory, src file :
 *   mvHwsIpcApis.c
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_PORT_SRC_GLOBAL_DB          portSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SILICON_IF_SRC_GLOBAL_DB    mvHwsSiliconIfSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_COMMON_SRC_GLOBAL_DB        commonSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_ANP_SRC_GLOBAL_DB           mvHwsAnpSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_SERVICE_CPU_FW_SRC_GLOBAL_DB mvHwsServiceCpuFwSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_AVAGO_SERDES_IF_SRC_GLOBAL_DB mvAvagoSerdesIfSrc;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_RSFEC_CW_SRC_GLOBAL_DB      rsfecSrc;
} PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_SERDES_IF_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory, src file :
 *   mvHwsSerdesIf.c
*/
typedef struct
{
    GT_BOOL                     serdesExternalFirmware;
    HWS_MUTEX                   serdesAccessMutex;
    GT_U8                       mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrl;
    GT_BOOL                     serdesBgEnable; /* enable/disable band-gap WA */
    GT_BOOL                     serdesBgClkBypassEnable; /* improvement for band-gap WA */
} PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_SERDES_IF_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module ,serdes directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_SERDES_IF_SRC_GLOBAL_DB mvHwsSerdesIfSrc;
} PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_GLOBAL_DB;


/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in driver module, interrupt generic directory
*/
typedef struct
{
    /** The control block of an interrupt queue.
     *  actual type of pointer is PRV_CPSS_DRV_EV_REQ_Q_CTRL_STC */
    GT_VOID *uniEvQArr;

    /** The size of the uniEvQArr array above */
    GT_U32   uniEvQArrSize;

    PRV_CPSS_DRV_COMMON_INT_SERVICE_ROUTINE_STC         prvCpssDrvComIntSvcRtnDb;
    GT_BOOL                                             afterDbReleaseIntVecAttached_initialized;
    GT_U32                                              afterDbReleaseIntVecAttached[PRV_CPSS_MAX_PP_DEVICES_CNS];
#ifdef CHX_FAMILY
    /** statistical counters for Ethernet port debugging */
    PRV_CPSS_GEN_NETIF_MII_STAT_STC ethPortDbg[PRV_CPSS_GEN_NETIF_MAX_RX_QUEUES_NUM_CNS];
#endif

} PRV_CPSS_DRV_COMMON_MOD_INT_GENERIC_DIR_SHARED_GLOBAL_DB;

/* value of this constant must be equal to PRV_CPSS_DXCH_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS */
#define PRV_CPSS_GLOBAL_DB_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS 13

/* value of this constant must be equal to PRV_CPSS_DXCH_MACSEC_REGID_ARRAY_SIZE_CNS */
#define PRV_CPSS_GLOBAL_DB_AC5P_MACSEC_REGID_ARRAY_SIZE_CNS 13

/* value of this constant must be equal to PRV_CPSS_DXCH_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS */
#define PRV_CPSS_GLOBAL_DB_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS 7

/* value of this constant must be equal to PRV_CPSS_DXCH_AAS_MACSEC_REGID_ARRAY_SIZE_CNS */
#define PRV_CPSS_GLOBAL_DB_AAS_MACSEC_REGID_ARRAY_SIZE_CNS 13

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_DXEXMX_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in driver module, interrupt DxExMx directory
*/
typedef struct
{
    /** indication that the aldrin2IntrScanArr was initalized and ready for use */
    /** NOTE : needed in aldrin2 due to the 'PTP fake' manipultion (look for 'fake' key word)*/
    GT_BOOL aldrin2initDone;
    /** indication that the bobcat3IntrScanArr was initalized and ready for use */
    /** NOTE : needed in BC3 due to the 'PTP fake' manipultion (look for 'fake' key word)*/
    GT_BOOL bobcat3initDone;
    /** Device interrupt init done */
    GT_BOOL falconInitDone[3];/*1 tile,2 tiles,4 tiles*/
    GT_BOOL hawkInitDone;
    GT_BOOL harrierInitDone;
    GT_BOOL ironmanInitDone;
    GT_BOOL aasInitDone[2];/*1 tile , 2 tiles*/
    GT_BOOL phoenixInitDone;
    GT_BOOL bc2InitDone;
    GT_BOOL aldrinInitDone;
    GT_BOOL ac5InitDone;
    GT_BOOL xcat3InitDone;
    GT_BOOL lion2InitDone;
    GT_BOOL caelumInitDone;
    GT_BOOL cetusInitDone;
    /** the sleep time for polling mode - value in 'milliseconds' */
    GT_U32 pollingSleepTime; /* = 20 */
    /** buffers to print interrupt tree elements */
    GT_CHAR readRegFuncAddr[20];
    GT_CHAR writeRegFuncAddr[20];
    PRV_CPSS_DRV_INT_TASK_PARAMS_STC intArr[64];
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_LAST_E];

    /** array to store macsec registers IDs which will be init during Harrier macsec init */
    GT_U32 macSecRegIdNotAccessibleBeforeMacSecInitHarrier[PRV_CPSS_GLOBAL_DB_HARRIER_MACSEC_REGID_ARRAY_SIZE_CNS];

    /** array to store macsec registers IDs which will be init during AC5P (Hawk) macsec init */
    GT_U32 macSecRegIdNotAccessibleBeforeMacSecInitAc5p[PRV_CPSS_GLOBAL_DB_AC5P_MACSEC_REGID_ARRAY_SIZE_CNS];

    /** array to store macsec registers ids which will be init during AC5X (Phoenix) macsec init */
    GT_U32 macSecRegIdNotAccessibleBeforeMacSecInitAc5x[PRV_CPSS_GLOBAL_DB_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS];

    /** array to store macsec registers ids which will be init during Ironman macsec init */
    GT_U32 macSecRegIdNotAccessibleBeforeMacSecInitIronMan[PRV_CPSS_GLOBAL_DB_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS];

    /** array to store macsec registers IDs which will be init during AAS macsec init */
    GT_U32 macSecRegIdNotAccessibleBeforeMacSecInitAas[PRV_CPSS_GLOBAL_DB_AAS_MACSEC_REGID_ARRAY_SIZE_CNS];

} PRV_CPSS_DRV_COMMON_MOD_INT_DXEXMX_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_DRV_COMMON_MOD_INT_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in driver module, interrupt directory
*/
typedef struct
{
    PRV_CPSS_DRV_COMMON_MOD_INT_GENERIC_DIR_SHARED_GLOBAL_DB genericInterruptsSrc;
    PRV_CPSS_DRV_COMMON_MOD_INT_DXEXMX_DIR_SHARED_GLOBAL_DB dxexmxInterruptsSrc;
} PRV_CPSS_DRV_COMMON_MOD_INT_DIR_SHARED_GLOBAL_DB;


typedef struct
{
      GT_VOID* optionalFwListHead;
      GT_CHAR prvSrvCpuUsrPath[PRV_CPSS_GENERIC_SRV_CPU_USR_PATH_LEN_CNS];
} PRV_CPSS_COMMON_CPSS_SRVCPU_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, systemRecovery  directory , source file prvCpssCommonSystemRecoveryParallel.c
*/
typedef struct
{
    PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STC              systemRecoveryManagersDb;
} PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in common module, systemRecovery directory
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SYS_RECOVERY_SRC_SHARED_GLOBAL_DB genSystemRecoverySrc;
} PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module
*/
typedef struct
{
    PRV_CPSS_COMMON_MOD_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB           genericHwInitDir;
    PRV_CPSS_DRV_COMMON_MOD_INT_DIR_SHARED_GLOBAL_DB                genericInterrupsDir;
    PRV_CPSS_COMMON_CPSS_SRVCPU_DIR_SHARED_GLOBAL_DB                srvcpuDir;
    PRV_CPSS_COMMON_MOD_SERDES_DIR_SHARED_GLOBAL_DB                 serdesDir;
    PRV_CPSS_COMMON_MOD_CONFIG_DIR_SHARED_GLOBAL_DB                 configDir;
    PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB           labServicesDir;
    PRV_CPSS_COMMON_MOD_PORT_DIR_SHARED_GLOBAL_DB                   portDir;
    PRV_CPSS_COMMON_MOD_SYS_RECOVERY_DIR_SHARED_GLOBAL_DB           systemRecoveryDir;
} PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB;

#endif /* __prvCpssCommonModGlobalSharedDb */
