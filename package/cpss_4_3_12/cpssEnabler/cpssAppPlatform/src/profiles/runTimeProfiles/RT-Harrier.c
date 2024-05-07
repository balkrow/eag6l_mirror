
/*******************************************************************************
*              (c), Copyright 2020, Marvell International Ltd.                 *
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
 * @file RT-Harrier.c
 *
 * DESCRIPTION:
 *            Run time profiles for Harrier.
 *
 *
 * FILE REVISION NUMBER:
 *       $Revision: 1 $
 *
 *******************************************************************************/
#include <profiles/cpssAppPlatformProfile.h>

static CPSS_UNI_EV_CAUSE_ENT genEventsArr_harrier[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E, CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E,
                           CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PCL_ACTION_TRIGGERED_E,
                         };

static CPSS_UNI_EV_CAUSE_ENT linkChangeEventsArr_harrier[] = {
                           CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                           CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                           CPSS_PP_PORT_AN_RESTART_E,
                           CPSS_PP_PORT_AN_HCD_FOUND_E
                         };

static CPSS_UNI_EV_CAUSE_ENT auEventsArr_harrier[] = {
                           CPSS_PP_EB_AUQ_PENDING_E, CPSS_PP_EB_AUQ_ALMOST_FULL_E,
                           CPSS_PP_EB_AUQ_FULL_E, CPSS_PP_EB_AUQ_OVER_E,
                           CPSS_PP_EB_FUQ_PENDING_E, CPSS_PP_EB_FUQ_FULL_E,
                           CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E, CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
                         };


static CPSS_UNI_EV_CAUSE_ENT rxEventsArr1_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE0_E, CPSS_PP_RX_ERR_QUEUE0_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr2_harrier[] = {
                                                CPSS_PP_RX_BUFFER_QUEUE1_E, CPSS_PP_RX_ERR_QUEUE1_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr3_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE2_E, CPSS_PP_RX_ERR_QUEUE2_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr4_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE3_E, CPSS_PP_RX_ERR_QUEUE3_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr5_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE4_E, CPSS_PP_RX_ERR_QUEUE4_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr6_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE5_E, CPSS_PP_RX_ERR_QUEUE5_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr7_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE6_E, CPSS_PP_RX_ERR_QUEUE6_E
                                              };

static CPSS_UNI_EV_CAUSE_ENT rxEventsArr8_harrier[] = { CPSS_PP_RX_BUFFER_QUEUE7_E,     CPSS_PP_RX_ERR_QUEUE7_E,
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

/* Gen Event handler - AC5X */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_harrier =
{
    _SM_(eventListPtr) genEventsArr_harrier,
    _SM_(numOfEvents) sizeof(genEventsArr_harrier)/sizeof(genEventsArr_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefGenEventHandle,
    _SM_(taskPriority) 200
};

/* RX Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier1 =
{
    _SM_(eventListPtr) rxEventsArr1_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr1_harrier)/sizeof(rxEventsArr1_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 199
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier2 =
{
    _SM_(eventListPtr) rxEventsArr2_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr2_harrier)/sizeof(rxEventsArr2_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 198
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier3 =
{
    _SM_(eventListPtr) rxEventsArr3_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr3_harrier)/sizeof(rxEventsArr3_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 197
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier4 =
{
    _SM_(eventListPtr) rxEventsArr4_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr4_harrier)/sizeof(rxEventsArr4_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 196
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier5 =
{
    _SM_(eventListPtr) rxEventsArr5_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr5_harrier)/sizeof(rxEventsArr5_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 195
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier6 =
{
    _SM_(eventListPtr) rxEventsArr6_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr6_harrier)/sizeof(rxEventsArr6_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 194
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier7 =
{
    _SM_(eventListPtr) rxEventsArr7_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr7_harrier)/sizeof(rxEventsArr7_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 193
};

CPSS_APP_PLATFORM_EVENT_HANDLE_STC  rx_event_handle_harrier8 =
{
    _SM_(eventListPtr) rxEventsArr8_harrier,
    _SM_(numOfEvents) sizeof(rxEventsArr8_harrier)/sizeof(rxEventsArr8_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefRxEventHandle,
    _SM_(taskPriority) 192
};

/* AU Event handler - AC5X */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  au_event_handle_harrier =
{
    _SM_(eventListPtr) auEventsArr_harrier,
    _SM_(numOfEvents) sizeof(auEventsArr_harrier)/sizeof(auEventsArr_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefAuEventHandle,
    _SM_(taskPriority) 191
};

/* Link Change Event handler - AC5X */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  link_change_event_handle_harrier =
{
    _SM_(eventListPtr) linkChangeEventsArr_harrier,
    _SM_(numOfEvents) sizeof(linkChangeEventsArr_harrier)/sizeof(linkChangeEventsArr_harrier[0]),
    _SM_(callbackFuncPtr) cpssAppRefLinkChangeEvHandle,
    _SM_(taskPriority) 190
};

CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP extPhyConfig_harrier[] =
{
    {0 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x00}, {NULL, 0}},
    {1 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x01}, {NULL, 0}},
    {2 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x02}, {NULL, 0}},
    {3 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x03}, {NULL, 0}},
    {4 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x04}, {NULL, 0}},
    {5 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x05}, {NULL, 0}},
    {6 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x06}, {NULL, 0}},
    {7 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x07}, {NULL, 0}},
    {8 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x08}, {NULL, 0}},
    {9 ,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x09}, {NULL, 0}},
    {10,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0A}, {NULL, 0}},
    {11,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0B}, {NULL, 0}},
    {12,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0C}, {NULL, 0}},
    {13,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0D}, {NULL, 0}},
    {14,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0E}, {NULL, 0}},
    {15,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x0F}, {NULL, 0}},
    {16,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x10}, {NULL, 0}},
    {17,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x11}, {NULL, 0}},
    {18,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x12}, {NULL, 0}},
    {19,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x13}, {NULL, 0}},
    {20,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x14}, {NULL, 0}},
    {21,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x15}, {NULL, 0}},
    {22,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x16}, {NULL, 0}},
    {23,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x17}, {NULL, 0}},
    {24,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x18}, {NULL, 0}},
    {25,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x19}, {NULL, 0}},
    {26,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1A}, {NULL, 0}},
    {27,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1B}, {NULL, 0}},
    {28,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1C}, {NULL, 0}},
    {29,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1D}, {NULL, 0}},
    {30,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1E}, {NULL, 0}},
    {31,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E  ,{CPSS_PHY_SMI_INTERFACE_1_E},  0x1F}, {NULL, 0}},
    {32,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x00}, {NULL, 0}},
    {33,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x01}, {NULL, 0}},
    {34,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x02}, {NULL, 0}},
    {35,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x03}, {NULL, 0}},
    {36,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x04}, {NULL, 0}},
    {37,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x05}, {NULL, 0}},
    {38,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x06}, {NULL, 0}},
    {39,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x07}, {NULL, 0}},
    {40,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x1C}, {NULL, 0}},
    {41,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x1D}, {NULL, 0}},
    {42,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x1E}, {NULL, 0}},
    {43,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x1F}, {NULL, 0}},
    {44,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x0C}, {NULL, 0}},
    {45,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x0D}, {NULL, 0}},
    {46,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x0E}, {NULL, 0}},
    {47,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E ,{CPSS_PHY_XSMI_INTERFACE_0_E}, 0x0F}, {NULL, 0}}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC WM_HARRIER_noPorts_portInitlist[] =
{
   { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {0, 19, 1, APP_INV_PORT_CNS }    , CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E , CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
  ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {APP_INV_PORT_CNS }              , CPSS_PORT_INTERFACE_MODE_QSGMII_E     , CPSS_PORT_SPEED_NA_E, CPSS_PORT_FEC_MODE_DISABLED_E,
        CPSS_APP_PLATFORM_DEFAULT_PORT_STATE_FIX_E, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , GT_FALSE}
};

CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC portManager_ports_param_set_harrier =
{
    _SM_(taskPriority) 500,
    _SM_(portManagerFunc) NULL
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_harrier =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) NULL,
    _SM_(phyMapListPtr) extPhyConfig_harrier,
    _SM_(phyMapListPtrSize) 48,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_noPorts_harrier =
{
    _SM_(devNum) 0,
    _SM_(portTypeListPtr) WM_HARRIER_noPorts_portInitlist,
    _SM_(phyMapListPtr) NULL,
    _SM_(phyMapListPtrSize) 0,
    _SM_(apPortTypeListPtr) NULL,
    _SM_(apPortTypeListPtrSize) 0,
    _SM_(overrideTxParams) GT_TRUE
};
/* Run Time profile for generic event Handler (without AP events) */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_HARRIER_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_harrier}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* Run Time profile for port manager */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_PORT_MANAGER_INFO_HARRIER[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E,
#ifdef ANSI_PROFILES
      {_SM_(portManagerHandlePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&portManager_ports_param_set_harrier}
#else
      {_SM_(portManagerHandlePtr) &portManager_ports_param_set_harrier}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/* RX profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RX_WM_HARRIER_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier7}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC LINK_WM_HARRIER_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_harrier}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_HARRIER_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_harrier}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_harrier}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier1}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier2}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier3}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier4}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier5}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier6}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier7}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &rx_event_handle_harrier8}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_harrier}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_harrier}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_WM_HARRIER_INFO_NOPORTS[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_noPorts_harrier}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_noPorts_harrier}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};


