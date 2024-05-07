/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *\
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssAskModGlobalSharedDb.h
*
* @brief This file define common/cpssAppPlatform module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssAskModGlobalSharedDb
#define __prvCpssAskModGlobalSharedDb

#ifdef CPSS_APP_PLATFORM_REFERENCE

#include <cpssAppPlatformSysConfig.h>


#define PRV_SHARED_ASK_GLOBAL_COMMON_SRC_VAR_GET(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssAppUtilsCommonSrc._var)


/* CPSS App Platform CNS */
#define CPSS_APP_PLATFORM_MAX_PP_CNS 32
#define MAX_MAPPED_PORTS_AC5P_CNS    128
#define APP_PLATFORM_MAX_MAPPED_PORTS_CNS    1024
#define MAX_MAPPED_PORTS_AC5X_CNS    128
/* TBD - remove MAX limit. change to dynamic allocation */
#define CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS 11




/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_APP_UTILS_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory.
*/
typedef struct
{
    GT_U32  useSinglePipeSingleDp;
    GT_BOOL appRefPortMgrMode;

} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_APP_UTILS_SRC_GLOBAL_DB;

/** @struct PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC
 *
 *   @brief Positions of LED bit indications in stream.
 *
 */
typedef struct {
    /** The first bit in the LED stream indication to be driven in current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart;
    /** The last bit in the LED stream indication to be driven in the current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd;
} PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC;


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_UTILS_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory.
*/
typedef struct
{
    GT_U32 applicationPlatformTcamIpcl0RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamIpcl1RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamIpcl2RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamEpclRuleBaseIndexOffset ;
    /* number of rules that can be used per PLC clients */
    GT_U32 applicationPlatformTcamIpcl0MaxNum;
    GT_U32 applicationPlatformTcamIpcl1MaxNum;
    GT_U32 applicationPlatformTcamIpcl2MaxNum;
    GT_U32 applicationPlatformTcamEpclMaxNum ;

    /* base offset for TTI client in TCAM */
    GT_U32 applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
    GT_U32 applicationPlatformTcamTtiHit3RuleBaseIndexOffset;
    /* number of rules that can be used per lookup of TTI */
    GT_U32 applicationPlatformTcamTtiHit0MaxNum ;
    GT_U32 applicationPlatformTcamTtiHit1MaxNum ;
    GT_U32 applicationPlatformTcamTtiHit2MaxNum ;
    GT_U32 applicationPlatformTcamTtiHit3MaxNum ;

    GT_U32  save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
    GT_U32  save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
    GT_U32  save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
    GT_U32  save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset;

    GT_U32  save_applicationPlatformTcamTtiHit0MaxNum;
    GT_U32  save_applicationPlatformTcamTtiHit1MaxNum;
    GT_U32  save_applicationPlatformTcamTtiHit2MaxNum;
    GT_U32  save_applicationPlatformTcamTtiHit3MaxNum;

    GT_U32 save_applicationPlatformTcamIpcl0RuleBaseIndexOffset;
    GT_U32 save_applicationPlatformTcamIpcl1RuleBaseIndexOffset;
    GT_U32 save_applicationPlatformTcamIpcl2RuleBaseIndexOffset;
    GT_U32 save_applicationPlatformTcamEpclRuleBaseIndexOffset ;

    GT_U32 save_applicationPlatformTcamIpcl0MaxNum;
    GT_U32 save_applicationPlatformTcamIpcl1MaxNum;
    GT_U32 save_applicationPlatformTcamIpcl2MaxNum;
    GT_U32 save_applicationPlatformTcamEpclMaxNum;

    GT_BOOL                            appPlatformLpmRamConfigInfoSet;
    CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC  appPlatformLpmRamConfigInfo[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS];
    GT_U32                             appPlatformLpmRamConfigInfoNumOfElements;
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  cpssAppPlatformPpPhase2Db[CPSS_APP_PLATFORM_MAX_PP_CNS];
    GT_BOOL                            lpmDbInitialized;     /* keeps if LPM DB was created */
    GT_BOOL                            applicationPlatformTtiTcamUseOffset ;
    GT_BOOL                            applicationPlatformPclTcamUseIndexConversion ;
    GT_VOID_PTR                        lpmDbSL;
    PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC sip6_led_indications_ezb[CPSS_LED_UNITS_MAX_NUM_CNS];
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_UTILS_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_CONFIG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory.
*/
typedef struct
{
    CPSS_DXCH_PORT_MAP_STC  *cpssAppPlatformPortMapPtr;
    GT_U32 cpssAppPlatformPortMapSize;
    /* next info not saved in CPSS, Saving locally, for fast retrieval */
    GT_U32 cpssCapMaxPortNum[CPSS_APP_PLATFORM_MAX_PP_CNS];
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_CONFIG_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PCI_CONFIG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatformPciConfig.c source file.
*/
typedef struct
{
    GT_BOOL initWasDone;
    GT_STATUS    firstStatus;
    GT_U32 deviceIdx;    /* device id index */
    GT_U32 instance;     /* device instance */
    GT_U32 numOfDevices; /* total number of devices in the system */
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PCI_CONFIG_SRC_GLOBAL_DB;


typedef struct _CAP_PROFILE_DB_STC {
    GT_CHAR*   ppProfileStr[CPSS_APP_PLATFORM_MAX_PP_CNS];
    GT_CHAR*   runtimeProfileStr;
} CAP_PROFILE_DB_STC;


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PCI_CONFIG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatformProfileMgr.c source file.
*/
typedef struct
{
    /**** DSs *****/
   GT_U32                         cap_profile_count;
   GT_U32                         last_used_profile;

   GT_CHAR*                       cap_profile_name_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] ;
   GT_CHAR*                       cap_profile_descr_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS];
   CPSS_APP_PLATFORM_PROFILE_STC* cap_profile_list_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS];

    /**** DSs for testing/Validation *****/
   GT_U32                         cap_test_profile_count;
   GT_CHAR*                       cap_test_profile_name_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] ;
   GT_CHAR*                       cap_test_profile_descr_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS];
   CPSS_APP_PLATFORM_PROFILE_STC* cap_test_profile_list_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS];
   CAP_PROFILE_DB_STC cap_profile_db ;
   CAP_PROFILE_DB_STC cap_test_profile_db;

} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PROFILE_MGR_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_SIP_6_PLATFORM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatformSip6.c source file.
*/
typedef struct
{
    /* Belly-to-Belly board boolean variable */
    GT_BOOL belly2belly;
    /* the number of CPU SDMAs that we need resources for */
    GT_U32   numCpuSdmas;

    /************************************
     * HARRIER port mode related code
     * *********************************/

    GT_U32  actualNum_cpssApi_harrier_defaultMap;/*actual number of valid entries in cpssApi_phoenix_defaultMap[] */
    CPSS_DXCH_PORT_MAP_STC   cpssApi_harrier_defaultMap[MAX_MAPPED_PORTS_AC5P_CNS];
    GT_U32  actualNum_cpssApi_hawk_defaultMap ;/*actual number of valid entries in cpssApi_phoenix_defaultMap[] */
    CPSS_DXCH_PORT_MAP_STC   cpssApi_hawk_defaultMap[MAX_MAPPED_PORTS_AC5P_CNS];/* filled at run time */
    GT_U32  actualNum_cpssApi_falcon_defaultMap;/*actual number of valid entries in cpssApi_falcon_defaultMap[] */
    CPSS_DXCH_PORT_MAP_STC   cpssApi_falcon_defaultMap[APP_PLATFORM_MAX_MAPPED_PORTS_CNS];/* filled at run time , by info from falcon_3_2_defaultMap[] */
    GT_U32  actualNum_cpssApi_phoenix_defaultMap ;/*actual number of valid entries in cpssApi_phoenix_defaultMap[] */
    CPSS_DXCH_PORT_MAP_STC   cpssApi_phoenix_defaultMap[MAX_MAPPED_PORTS_AC5X_CNS];/* filled at run time */
    GT_U32  actualNum_cpssApi_ironman_defaultMap;/*actual number of valid entries in cpssApi_ironman_defaultMap[] */
    CPSS_DXCH_PORT_MAP_STC   cpssApi_ironman_defaultMap[APP_PLATFORM_MAX_MAPPED_PORTS_CNS];/* filled at run time */
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_SIP_6_PLATFORM_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PORT_INIT_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatformPortInit.c. source file.
*/
typedef struct
{

    CPSS_OS_SIG_SEM portInitSeqSignalSem;
    GT_BOOL         isSip6 ;
    GT_TASK         pmTid;
    GT_BOOL         stopPmTask;
    GT_U32          appRefPortManagerTaskSleepTime;
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PORT_INIT_SRC_GLOBAL_DB;


typedef struct{

    /** The process unified event handle (got from CpssEventBind). */
    GT_UINTPTR evHndl;

    /* event handler number */
    GT_U32 index;

    /* event handler task Id */
    GT_TASK taskId;

    /* callback function */
    CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC callbackFuncPtr;

    /* List of Events handled */
    CPSS_UNI_EV_CAUSE_ENT *eventListPtr;

    /* Number of events in eventListPtr*/
    GT_U8 numOfEvents;

} EVENT_HANDLER_PARAM_STC;

/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_RUN_TIME_CONFIG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatfomRunTimeConfig.c source file.
*/
typedef struct
{
     GT_U8 eventHandlerNum;
     EVENT_HANDLER_PARAM_STC eventHandlerParams[CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS];
     GT_U32 taskCreated ;
     GT_U32 eventHandlerReset;
     GT_BOOL eventHandlerInitDone;
     CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC cpssAppPlatformGenEventHandlePtr;
     GT_TASK appRefTestsEventHandlerTid ;
     EVENT_HANDLER_PARAM_STC testEventHandlerParam;
} PRV_CPSS_COMMON_APP_REF_DIR_SHARED_RUN_TIME_CONFIG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_TM_DRV_MOD_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in appRef module.
*/
typedef struct
{
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_APP_UTILS_SRC_GLOBAL_DB       cpssAppUtilsCommonSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_UTILS_SRC_GLOBAL_DB        cpssPpUtilsSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PP_CONFIG_SRC_GLOBAL_DB       cpssPpConfigSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PCI_CONFIG_SRC_GLOBAL_DB      cpssPciConfigSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PROFILE_MGR_SRC_GLOBAL_DB     cpssProfileMgrSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_SIP_6_PLATFORM_SRC_GLOBAL_DB  cpssSip6PlatformSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PORT_INIT_SRC_GLOBAL_DB       cpssPortInitSrc;
    PRV_CPSS_COMMON_APP_REF_DIR_SHARED_RUN_TIME_CONFIG_SRC_GLOBAL_DB cpssRunTimeConfigSrc;
} PRV_CPSS_COMMON_APP_REF_MOD_SHARED_GLOBAL_DB;

#endif /*CPSS_APP_PLATFORM_REFERENCE*/
#endif  /* __prvCpssAskModGlobalSharedDb */
