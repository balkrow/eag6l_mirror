/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file  cpssAppUtilsCommon.h
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   131
********************************************************************************
*/

#ifndef __cpssAppUtilsCommon_h
#define __cpssAppUtilsCommon_h

#ifdef __cplusplus
extern "C" {
#endif

#include <extUtils/common/cpssEnablerUtils.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#endif /*CHX_FAMILY*/
#include <cmdShell/common/cmdExtServices.h>
#if defined(CHX_FAMILY) && defined(IMPL_GALTIS)
#include <galtisAgent/wrapCpss/dxCh/diag/wrapCpssDxChDiagDataIntegrity.h>
#endif /*defined(CHX_FAMILY) && defined(IMPL_GALTIS)*/
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

extern GT_BOOL appDemoCheckEnableGet();
extern void appDemoCheckEnableSet(GT_BOOL checkEnable);
extern GT_U32 appDemoDbSizeGet();
extern CPSS_ENABLER_DB_ENTRY_STC appDemoDbIndexedEntryGet(GT_U32 index);

/**
* @internal appDemoDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
extern GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

/**
* @internal appDemoDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
extern GT_STATUS appDemoDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

#ifdef INCLUDE_MPD
#undef MAX_UINT_8
#include <mpd.h>

typedef struct GT_APPDEMO_XPHY_STATIC_INFO_STCT
{
    GT_U32                              portNum;
    GT_U16                              phyAddr;
    GT_U32                              phyNumber;
    MPD_PHY_USX_TYPE_ENT                usxType;
    MPD_TRANSCEIVER_TYPE_ENT            transceiverType;
} GT_APPDEMO_XPHY_STATIC_INFO_STC;

/* [ShaharG - Same as the above structure - need to consider to merge as part of App Platform re-design */
typedef struct CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STCT
{
    GT_U32                              portNum;
    GT_U16                              phyAddr;
    GT_U32                              phyNumber;
    MPD_PHY_USX_TYPE_ENT                usxType;
    MPD_TRANSCEIVER_TYPE_ENT            transceiverType;
} CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STC;

extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd2580_SR2[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd2540_SR2[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5pRd2580_SR1[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5pRd1780[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd1780[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayIronmanL_Db1781[];
extern GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayIronmanRd24g6xg1781[];
#endif

extern GT_BOOL appDemoCheckEnable;
/* allow to init with/without the WA for the 'port delete' */
extern GT_STATUS   falcon_initPortDelete_WA_set(IN GT_U32  waNeeded);

extern GT_U32                              falcon_MacToSerdesMap_arrSize;
extern CPSS_PORT_MAC_TO_SERDES_STC*        falcon_MacToSerdesMap;

extern GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
);
extern GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
);

#ifdef CHX_FAMILY
extern GT_CHAR * CPSS_MAPPING_2_STR
(
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm
);
#endif /*CHX_FAMILY*/

extern GT_U32   phoenix_only_ports_of_100G_mac; /* the default is 0 as the CPSS supports USX ports */
extern GT_STATUS phoenix_only_ports_of_100G_mac_set(IN GT_U32   use);
extern GT_U32   phoenix_only_ports_of_25G_mac; /* the default is 1 as the CPSS not supports ALL USX ports , but 'first port' of each 8 USX ports */
extern GT_STATUS phoenix_only_ports_of_25G_mac_set(IN GT_U32   use);

extern GT_U32  useSinglePipeSingleDp;
extern GT_STATUS falcon_useSinglePipeSingleDp_Set(IN GT_BOOL useSingleDp);

extern GT_STATUS
cpssEnablerIsAppPlat
(
);


/* Notification callback for catching events */
extern EVENT_NOTIFY_FUNC *notifyEventArrivedFunc;


/**
* @internal waTriggerInterruptsMaskNoLinkStatusChangeInterrupt function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
*         This function is almost the same as waTriggerInterruptsMask but uses
*         When it is no signal to not unmask CPSS_PP_PORT_LINK_STATUS_CHANGED_E interrupt
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/

extern GT_STATUS waTriggerInterruptsMaskNoLinkStatusChangeInterrupt
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

/**
* @internal waTriggerInterruptsMask function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
extern GT_STATUS waTriggerInterruptsMask
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);


/**
* @internal cpssDevSupportSystemResetSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset is supported, it is used to disable the
*         support on devices that do support. vice versa is not possible
*
* @retval GT_OK                    - on success else if failed
*/

extern GT_STATUS cpssDevSupportSystemResetSet(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] doHwReset                - indication to do HW reset
*
* @retval GT_OK                    - on success else if failed
*
* For HA to simulate Crash on WM use sequence
*    appDemoHaEmulateSwCrash()
*    cpssResetSystem(GT_FALSE)
*
*/

extern GT_STATUS cpssResetSystem(
    IN  GT_BOOL     doHwReset
);

#if defined(DXCH_CODE) || defined(CHX_FAMILY)
#ifdef IMPL_GALTIS
/* Event Counter increment function. Implemented in Galtis*/
extern DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *dxChDataIntegrityEventIncrementFunc;
#endif
#endif /* defined(DXCH_CODE) && defined(IMPL_GALTIS) */


extern GT_STATUS cpssDevSupportSystemResetSet(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
extern GT_STATUS userAppInit(GT_VOID);

GT_STATUS   applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
);

extern void simulationBindOsFuncs(void);
extern void simulationStart (void);

#ifdef __cplusplus
}
#endif

#endif  /* __cpssAppUtilsCommon_h */

