/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file RT-WM-Ironman.c
*
* @brief Run time profiles for Ironman
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

static CPSS_UNI_EV_CAUSE_ENT genEventsArr[] = {
                           CPSS_PP_MISC_ILLEGAL_ADDR_E, CPSS_PP_GPP_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PCL_ACTION_TRIGGERED_E, CPSS_PP_PHA_E,
                           CPSS_PP_EM_AUTO_LEARN_INDIRECT_ADDRESS_OUT_OF_RANGE_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E, CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E, CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_ILLEGAL_ADDRESS_ACCESS_EVENT_E, CPSS_PP_EM_AUTO_LEARN_ILLEGAL_TABLE_OVERLAP_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,
                           CPSS_PP_EM_AUTO_LEARN_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,
                           CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
                           CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,
                           CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E,
                           CPSS_SRVCPU_SDK_WATCHDOG_E, CPSS_SRVCPU_SDK_DOORBELL_E,CPSS_PP_TQ_PORT_MICRO_BURST_E
                         };

static CPSS_UNI_EV_CAUSE_ENT linkChangeEventsArr[] = {
                           CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                           CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                           CPSS_PP_PORT_AN_HCD_FOUND_E,
                           CPSS_PP_PORT_AN_RESTART_E,
                           CPSS_PP_PORT_AN_PARALLEL_DETECT_E
                         };

static CPSS_UNI_EV_CAUSE_ENT auEventsArr[] = {
                           CPSS_PP_EB_AUQ_PENDING_E, CPSS_PP_EB_AUQ_ALMOST_FULL_E,
                           CPSS_PP_EB_AUQ_FULL_E, CPSS_PP_EB_AUQ_OVER_E,
                           CPSS_PP_EB_FUQ_PENDING_E, CPSS_PP_EB_FUQ_FULL_E,
                           CPSS_PP_EB_SECURITY_BREACH_UPDATE_E, CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E, CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
                         };


static CPSS_UNI_EV_CAUSE_ENT rxEventsArr1[] = { CPSS_PP_RX_BUFFER_QUEUE0_E, CPSS_PP_RX_ERR_QUEUE0_E,
                                                CPSS_PP_RX_BUFFER_QUEUE1_E, CPSS_PP_RX_ERR_QUEUE1_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr2[] = { CPSS_PP_RX_BUFFER_QUEUE2_E, CPSS_PP_RX_ERR_QUEUE2_E,
                                                CPSS_PP_RX_BUFFER_QUEUE3_E, CPSS_PP_RX_ERR_QUEUE3_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr3[] = { CPSS_PP_RX_BUFFER_QUEUE4_E, CPSS_PP_RX_ERR_QUEUE4_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr4[] = { CPSS_PP_RX_BUFFER_QUEUE5_E, CPSS_PP_RX_ERR_QUEUE5_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr5[] = { CPSS_PP_RX_BUFFER_QUEUE6_E, CPSS_PP_RX_ERR_QUEUE6_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr6[] = { CPSS_PP_RX_BUFFER_QUEUE7_E, CPSS_PP_RX_ERR_QUEUE7_E,
                                              /*MG1*/
                                               (CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE0_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE1_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE2_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE3_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE4_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE5_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE6_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_BUFFER_QUEUE7_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE0_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE1_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE2_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE3_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE4_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE5_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE6_E
                                              ,(CPSS_UNI_EV_CAUSE_ENT) CPSS_PP_MG1_RX_ERR_QUEUE7_E
                                              };

/* Rx Event call back function */
GT_STATUS cpssAppRefRxEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Generic Event call back function */
GT_STATUS cpssAppRefGenEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* AU Event call back function */
GT_STATUS cpssAppRefAuEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Link change Event call back function */
GT_STATUS cpssAppRefLinkChangeEvHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Gen Event handler - AC5X*/
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_ironman =
{
    _SM_(eventListPtr) genEventsArr,
    _SM_(numOfEvents) sizeof(genEventsArr)/sizeof(genEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

/* RX Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman1 =
{
    _SM_(eventListPtr) rxEventsArr1,
    _SM_(numOfEvents) sizeof(rxEventsArr1)/sizeof(rxEventsArr1[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 199
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman2 =
{
    _SM_(eventListPtr) rxEventsArr2,
    _SM_(numOfEvents) sizeof(rxEventsArr2)/sizeof(rxEventsArr2[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 198
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman3 =
{
    _SM_(eventListPtr) rxEventsArr3,
    _SM_(numOfEvents) sizeof(rxEventsArr3)/sizeof(rxEventsArr3[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 197
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman4 =
{
    _SM_(eventListPtr) rxEventsArr4,
    _SM_(numOfEvents) sizeof(rxEventsArr4)/sizeof(rxEventsArr4[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 196
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman5 =
{
    _SM_(eventListPtr) rxEventsArr5,
    _SM_(numOfEvents) sizeof(rxEventsArr5)/sizeof(rxEventsArr5[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 195
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_ironman6 =
{
    _SM_(eventListPtr) rxEventsArr6,
    _SM_(numOfEvents) sizeof(rxEventsArr6)/sizeof(rxEventsArr6[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 194
};

/* AU Event handler - AC5X */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  au_event_handle_ironman =
{
    _SM_(eventListPtr) auEventsArr,
    _SM_(numOfEvents) sizeof(auEventsArr)/sizeof(auEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefAuEventHandle,
    _SM_(taskPriority) 193
};

/* Link Change Event handler - AC5X */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  link_change_event_handle_ironman =
{
    _SM_(eventListPtr) linkChangeEventsArr,
    _SM_(numOfEvents) sizeof(linkChangeEventsArr)/sizeof(linkChangeEventsArr[0]),
    _SM_(callbackFuncPtr) cpssAppRefLinkChangeEvHandle,
    _SM_(taskPriority) 190
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC WM_IRONMAN_m_portInitlist_noPorts[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0,7,1, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E,  {8,9,10,11, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }              , CPSS_PORT_INTERFACE_MODE_QSGMII_E     , CPSS_PORT_SPEED_NA_E    , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
};


static CPSS_APP_PLATFORM_PORT_CONFIG_STC WM_IRONMAN_m_portInitlist[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0,7,1, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E, CPSS_PORT_SPEED_1000_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E,  {8,9,10,11, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }              , CPSS_PORT_INTERFACE_MODE_QSGMII_E     , CPSS_PORT_SPEED_NA_E    , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC WM_IRONMAN_portInitlist[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E,  {0,4,8,12,16,20,48,49,50,51,52,53,54, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }              , CPSS_PORT_INTERFACE_MODE_QSGMII_E     , CPSS_PORT_SPEED_NA_E    , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC WM_IRONMAN_portInitlist_noPorts[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E,  {0,4,8,12,16,20,48,49,50,51,52,53,54, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E  , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }              , CPSS_PORT_INTERFACE_MODE_QSGMII_E     , CPSS_PORT_SPEED_NA_E    , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
};



CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC portManager_ports_param_set_ironman =
{
    _SM_(taskPriority) 500,
    _SM_(portManagerFunc) NULL
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC WM_traffic_enable_info_ironman_noPorts =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) WM_IRONMAN_portInitlist_noPorts,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC WM_traffic_enable_info_ironman_m_noPorts =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) WM_IRONMAN_m_portInitlist_noPorts,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};


CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC WM_traffic_enable_info_ironman_m =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) WM_IRONMAN_m_portInitlist,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC WM_traffic_enable_info_ironman =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) WM_IRONMAN_portInitlist,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};

/*Run Time profile for generic event Handler (without AP events) -AC5X*/
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_IRONMAN_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for port manager */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_PORT_MANAGER_INFO_IRONMAN[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E,
#ifdef ANSI_PROFILES
      {_SM_(portManagerHandlePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&portManager_ports_param_set_ironman}
#else
      {_SM_(portManagerHandlePtr) &portManager_ports_param_set_ironman}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Ironman Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_IRONMAN_NOPORTS_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&WM_traffic_enable_info_ironman_noPorts}
#else
      {_SM_(trafficEnablePtr) &WM_traffic_enable_info_ironman_noPorts}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Ironman Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_IRONMAN_M_INFO_NOPORTS[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&WM_traffic_enable_info_ironman_m_noPorts}
#else
      {_SM_(trafficEnablePtr) &WM_traffic_enable_info_ironman_m_noPorts}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};


/* Ironman Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_IRONMAN_M_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&WM_traffic_enable_info_ironman_m}
#else
      {_SM_(trafficEnablePtr) &WM_traffic_enable_info_ironman_m}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Ironman Application run Time profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_IRONMAN_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_ironman}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_ironman6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&WM_traffic_enable_info_ironman}
#else
      {_SM_(trafficEnablePtr) &WM_traffic_enable_info_ironman}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};
