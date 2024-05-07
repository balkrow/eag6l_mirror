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
* @file prvCpssMainPpDrvModGlobalNonSharedDb.h
*
* @brief This file define mainPpDrv module non shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssMainPpDrvModGlobalNonSharedDb
#define __prvCpssMainPpDrvModGlobalNonSharedDb
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>



/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_TABLES_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory, prvCpssDxChHwTables.c file
*/
typedef struct
{
    /**@brief - info to support multiple family devices
     *     each family need info about the DB created for other families
     *     as the Init of tables is done incrementally
     *         part of fix for : CPSS-13946 : CPSS table initialization with multiple
     *                  device Aldrin2 + Falcon failed
     */
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC    tempTablesPerFamilyArr[CPSS_PP_FAMILY_LAST_E];

    /** bit per family in the DB about the tables already initialized */
    GT_U32   tablesDbInitPreFamily[(CPSS_PP_FAMILY_LAST_E+31)/32];

    /** pointer to xCat3, AC5 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *dxChXcat3TablesInfoPtr;

    /** pointer to Lion2 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *lion2TablesInfoPtr;

    /** pointer to BobCat2 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobcat2TablesInfoPtr;

    /** pointer to Bobk tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobkTablesInfoPtr;

    /**@brief - pointer to BobCat3 tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[BobCat3].bobcat3Tables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobcat3TablesInfoPtr;

    /**@brief - pointer to Aldrin2 tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Aldrin2].aldrin2Tables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *aldrin2TablesInfoPtr;

    /**@brief - pointer to Falcon tables info DB
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *falconTablesInfoPtr;

    /**@brief - pointer to AC5P tables info DB
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *hawkTablesInfoPtr;

    /**@brief - pointer to AC5X tables info DB
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *phoenixTablesInfoPtr;

    /**@brief - pointer to Harrier tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Harrier].harrierTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *harrierTablesInfoPtr;

    /**@brief - pointer to Ironman tables info DB
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *ironman_L_TablesInfoPtr;

    /**@brief - pointer to AAS tables info DB
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *aas_TablesInfoPtr;

    /** init done variables */
   GT_BOOL directAccessBobkTableInfo_initDone;

   /** Bobcat2 direct table information */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobcat2TableInfoPtr;

   /** Bobcat2B0 direct table override information */
   PRV_CPSS_DXCH_TABLES_INFO_EXT_STC *bobcat2B0TablesInfo_overrideA0Ptr;

   /** BobK direct table information */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobkTableInfoPtr;

    /**@brief - pointer to Bobcat3 direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Bobcat3].bobcat3Tables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobcat3TableInfoPtr;

    /**@brief - pointer to Aldrin2 direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Aldrin2].aldrin2Tables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *aldrin2directAccessTableInfoPtr;

   /** Object that holds callback function to table HW access */
   CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC prvDxChTableCpssHwAccessObj;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_TABLES_SRC_GLOBAL_DB;

/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory
*/
typedef struct
{
    /** data of prvCpssDxChHwTables.c file */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_TABLES_SRC_GLOBAL_DB tablesSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB;


/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_NON_SHARED_PORT_PA_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant not to be shared
 * between processes (when CPSS is linked as shared object)
*    in mainPpDrv module ,port  directory , sources :
*    port/PizzaArbiter/cpssDxChPortPizzaArbiter.c
*/
typedef struct
{

    /** substructures of this pointed by per-defice info */
    PRV_CPSS_DXCH_PA_WS_SET_STC  prv_paWsSet;
    PRV_CPSS_DXCH_DEV_X_UNITDEFLIST  prv_dev_unitDefList[8];
    PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC prv_paUnitsDrv;
    /** prvCpssDxChPortDynamicPAUnitDrv.c variables */
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc2     [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bobk    [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin  [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc3     [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin2 [CPSS_DXCH_PA_UNIT_MAX_E];

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_NON_SHARED_PORT_PA_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsHarrierpDevInit.c
*/
typedef struct
{
    HWS_UNIT_BASE_ADDR_CALC_BIND    falconBaseAddrCalcFunc;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB   falconDevinitSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB;


/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortSpeed.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    /* Table of pointers to ifMode configuration functions per DXCH ASIC family
     * per interface mode if mode not supported NULL pointer provided
     */
    PRV_CPSS_DXCH_PORT_SPEED_SET_FUN portSpeedSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1][PRV_CPSS_XG_PORT_OPTIONS_MAX_E];

    /* Array of pointers to port interface mode get functions per DXCH ASIC family */
    PRV_CPSS_DXCH_PORT_SPEED_GET_FUN portSpeedGetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1];
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB;


#endif
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory , source : cpssHwInit.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32 dummy;
} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB;

#ifdef CHX_FAMILY

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq  directory , source : prvCpssDxChTxqMain.c
*/
typedef struct
{
    /*add here global non- shared variables used in source files*/

    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC  queuePdsProfiles[PRV_CPSS_DXCH_SIP6_TXQ_MAX_NUM_OF_SUPPORTED_SPEEDS_CNS];
    GT_BOOL                                          profilesInitialized;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq  directory , source : prvCpssDxChTxqDebugUtils.c
*/
typedef struct
{
    /*add here global non -shared variables used in source files*/

    GT_CHAR logBuffer[PRV_TXQ_LOG_STRING_BUFFER_SIZE_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_DBG_SRC_GLOBAL_DB;


#endif
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB hwInitSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB;

#ifdef CHX_FAMILY

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_AP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortAp.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32                  allowPrint;
    GT_U32                  apSemPrintEn;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_AP_SRC_GLOBAL_DB;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_MANAGER_SAMPLES_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortCtrl.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32      debug_halt_on_TXQ_stuck;
    GT_U32      tickTimeMsec; /* length of system tick in millisecond */
    GT_U32      prvCpssDxChPortXlgUnidirectionalEnableSetTraceEnable;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_CTRL_SRC_GLOBAL_DB;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortSerdesCfg.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL                                             forceDisableLowPowerMode;
    GT_BOOL                                             skipLion2PortDelete;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,port directory
*/
typedef struct
{
    /** DB of port statistic module */
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_AP_SRC_GLOBAL_DB              portApSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_CTRL_SRC_GLOBAL_DB            portCtrlSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB      portSerdesCfgSrc;
#ifdef CHX_FAMILY
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB           portSpeedSrc;
#endif
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_NON_SHARED_PORT_PA_SRC_GLOBAL_DB         portPaSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB txqMainSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_DBG_SRC_GLOBAL_DB  txqDbgSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_GLOBAL_DB;

/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_PHY_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module, phy directory (MPD Module)
*/
typedef struct
{
    GT_VOID     * mpdGlobalNonSharedDb;
} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_PHY_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,dxCh catchUp  directory , source : prvCpssDxChCatchUpDbg.h
*/
typedef struct
{
    PRV_CPSS_DXCH_CATCHUP_PARAMS_STC ppConfigCatchUpParams[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* array of MAC counters offsets, assigned with value PRV_CPSS_DXCH_CATCHUP_8_BIT_PATTERN_CNS for CatchUpDbg procedure. */
    GT_U16      gtMacCounterOffset[CPSS_LAST_MAC_COUNTER_NUM_E];
    GT_BOOL     isMacCounterOffsetInit ;

} PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,catch Up  directory , source : prvCpssDxChCatchUp.h
*/
typedef struct
{
    PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC *auqMsgEnableStatus[PRV_CPSS_MAX_PP_DEVICES_CNS] ;

} PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB;



/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,catch up directory
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB     catchUpDbgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB         catchUpSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB;

#endif /*CHX_FAMILY */
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module
*/
typedef struct
{
    /*add here directories*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB hwInitDir;
#ifdef CHX_FAMILY
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_GLOBAL_DB      txqDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB catchUpDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_GLOBAL_DB          portDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB  labServicesDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB dxChCpssHwInit;
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_PHY_DIR_NON_SHARED_GLOBAL_DB      phyDir;
#endif

} PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB;




#endif /* __prvCpssMainPpDrvModGlobalNonSharedDb */


