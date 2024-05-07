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
* @file prvTgfTcamClientsData.c
*
* @brief TCAM Clients data (sample used for tests)
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <extUtils/tcamClients/tgfTcamClients.h>

/*===================================================================================*/
static PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_STC prvTgfTcamClientCommomHandlersData =
{
    GT_FALSE,
    {
        {NULL, {0}},
        {NULL, {0}},
        {NULL, {0}},
        {NULL, {0}}
    }
};

GT_BOOL prvTgfTcamClientCommomHandlersLibInitDoneGet(GT_VOID)
{
    return prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone;
}

GT_VOID prvTgfTcamClientCommomHandlersLibInitDoneSet(GT_VOID)
{
    if (prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone == GT_FALSE)
    {
        cpssOsMemSet(
            &prvTgfTcamClientCommomHandlersData, 0,
            sizeof(prvTgfTcamClientCommomHandlersData));
    }
    prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone = GT_TRUE;
}

TGF_TCAM_CLIENTS_DB_HANDLE prvTgfTcamClientCommomHandlersHandleGet(GT_U32 index)
{
    if (prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone == GT_FALSE) return NULL;
    if (index >= PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS) return NULL;
    return prvTgfTcamClientCommomHandlersData.handleArr[index].tcamClientsDbHandle;
}

GT_VOID prvTgfTcamClientCommomHandlersHandleSet(GT_U32 index,  TGF_TCAM_CLIENTS_DB_HANDLE handle)
{
    if (prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone == GT_FALSE) return;
    if (index >= PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS) return;
    if (handle == NULL)
    {
        cpssOsMemSet(
            &(prvTgfTcamClientCommomHandlersData.handleArr[index].initializedDevicesBmpArr), 0,
            sizeof(prvTgfTcamClientCommomHandlersData.handleArr[0]).initializedDevicesBmpArr);
    }
    prvTgfTcamClientCommomHandlersData.handleArr[index].tcamClientsDbHandle = handle;
}

GT_U32 prvTgfTcamClientCommomHandlersDeviceConfiguredGet(GT_U32 index, GT_U8 devNum)
{
    if (prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone == GT_FALSE) return 0;
    if (index >= PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS) return 0;
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) return 0;
    return
        ((prvTgfTcamClientCommomHandlersData.handleArr[index].initializedDevicesBmpArr[devNum / 32]
         >> (devNum % 32)) & 1);
}

GT_VOID prvTgfTcamClientCommomHandlersDeviceConfiguredSet(GT_U32 index, GT_U8 devNum)
{
    if (prvTgfTcamClientCommomHandlersData.tcamClientsLibInitDone == GT_FALSE) return;
    if (index >= PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS) return;
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) return;
    prvTgfTcamClientCommomHandlersData.handleArr[index].initializedDevicesBmpArr[devNum / 32]
        |= (1 << (devNum % 32));
}

/*===================================================================================*/

/* recommended IPCL/MPCL/EPCL Subkey Header*/
static const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC defaultCommonSubkeyHeader =
{
    2 /*numOfIngressUdbCfg*/,
    { /*ingressUdbDefaultConfigArr*/
        /** @brief Applicable Flow Sub-template
         *  1 = IPv4_TCP
         *  2 = IPv4_UDP
         *  4 = MPLS
         *  8 = IPv4_FRAGMENT
         *  16 = IPv4_OTHER
         *  32 = ETHERNET_OTHER
         *  64 = IPv6
         *  128 = UDE0-4
         */
        {12, CPSS_DXCH_PCL_OFFSET_METADATA_E, 91}, /*IMETA_PKT_TYPE_BMP*/
        /** @brief Indicates the packet type to be used in IPCL key preparation.
         *  Bits5:3 of this byte
         *  0 = Other
         *  1 = ARP
         *  2 = IPv6 over MPLS
         *  3 = IPv4 over MPLS
         *  4 = MPLS
         *  5 = Reserved
         *  6 = IPv6
         *  7 = IPv4
         */
        {13, CPSS_DXCH_PCL_OFFSET_METADATA_E, 90}  /*IMETA_PKT_TYPE_BMP*/
    },

    2 /*numOfEgressUdbCfg*/,
    { /*egressUdbDefaultConfigArr*/
        /** @brief Applicable Flow Sub-template
         * 0x0001   - IPV4_TCP_E
         * 0x0002   - IPV4_UDP_E
         * 0x0004   - MPLS_E
         * 0x0008   - IPV4_FRAGMENT_E
         * 0x0010   - IPV4_OTHER_E
         * 0x0020   - ETHERNET_OTHER_
         * 0x0040   - IPV6_E
         * 0x0080   - IPV6_TCP_E
         * 0x0100   - IPV6_UDP_E
         * 0x0200   - UDE0_E
         * 0x0400   - UDE1_E
         * 0x0800   - UDE2_E
         * 0x1000   - UDE3_E
         * 0x2000   - UDE4_E
         * 0x4000   - UDE5_E
         * 0x8000   - UDE6_E
         */
        {12, CPSS_DXCH_PCL_OFFSET_METADATA_E, 152}, /*IMETA_PKT_TYPE_BMP*/
        /** @brief Indicates the packet type to be used in IPCL key preparation.
         */
        {13, CPSS_DXCH_PCL_OFFSET_METADATA_E, 153}  /*IMETA_PKT_TYPE_BMP*/
    },

    2 /*ipclNumOfMuxEntries*/,
    {/*ipclMuxSuperkeyBasesArr*/
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_FIXED_E,
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_UDB_E
    },
    {/*ipclMuxSuperkeyOffsetsArr*/
        0 /*PCL_ID*/, 6/*UDB12-13*/
    },
    2 /*mpclNumOfMuxEntries*/,
    {/*mpclMuxSuperkeyBasesArr*/
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_FIXED_E,
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_UDB_E
    },
    {/*mpclMuxSuperkeyOffsetsArr*/
        0 /*PCL_ID*/, 1/*UDB12-13*/
    },
    2 /*epclNumOfMuxEntries*/,
    {/*epclMuxSuperkeyBasesArr*/
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_EPCL_FIXED_E,
        TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_UDB_E
    },
    {/*epclMuxSuperkeyOffsetsArr*/
        0 /*PCL_ID*/, 6/*UDB12-13*/
    },
};

static const TGF_TCAM_CLIENTS_PCL_RULE_ACTION_MODIFIED_SUB_FIELD_USE_STC sampleRuleActionModifiedSubFieldsUseArr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_EVLAN_E, 0, 15}, 15},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_QOS_PROFILE_E, 0, 10}, 15},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_EPORT_E, 0, 16}, 15},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_PCL_ID2_E, 0, 24}, 9},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_PORT_E, 0, 16}, 6},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};

static const TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC sampleRuleIngressUdbSubFieldsUseArr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_EXISTS_E, 0, 1}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_VID0_E, 0, 12}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_UP0_E, 0, 3}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_QOS_PROFILE_E, 0, 10}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E, 0, 6}, {3, 3, 3, 3}}, /*ETH and IPV4*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV6_DSCP_E, 0, 6}, {4, 4, 4, 4}}, /*IPV6*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IP_PROTOCOL_E, 0, 8}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E, 0, 8}, {7, 7, 7, 7}},
    /* SIP and DIP always included in suprkey */
    /* DST_MAC included in IPCL and MPCL suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_TPID_INDEX_E, 0, 3}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG1_TPID_INDEX_E, 0, 3}, {7, 7, 7, 7}},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_PTP_E, 0, 1}, {7, 7, 7, 7}},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, {0,0,0,0}}
};

static const TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC sampleRuleEgressUdbSubFieldsUseArr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_TAG0_EXISTS_E, 0, 1}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_VID0_E, 0, 12}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_UP0_E, 0, 3}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_QOS_PROFILE_E, 0, 10}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E, 0, 6}, 3}, /*ETH and IPV4*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV6_DSCP_E, 0, 6}, 4}, /*IPV6*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IP_PROTOCOL_E, 0, 8}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E, 0, 8}, 7},
    /* SIP and DIP always included in suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IS_PTP_E, 0, 1}, 7},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E, 0, 48}, 7},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};

static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  sampleIpclCfgToTcamProfileIdMapArr[] =
{
    /*pcfCfgTableProfileId, tcamProfileIdArr*/
    {1, {1, 1, 2, 1}}, /* 60-bytes notIpv6 (1) and Ipv6 (2) */
    /* end of list stamp */
    {0xFF, {0, 0, 0, 0}}
};
static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  sampleMpclCfgToTcamProfileIdMapArr[] =
{
    /*pcfCfgTableProfileId, tcamProfileIdArr*/
    {1, {5, 5, 6, 5}}, /* 60-bytes notIpv6 (1) and Ipv6 (2) */
    /* end of list stamp */
    {0xFF, {0, 0, 0, 0}}
};
static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  sampleEpclCfgToTcamProfileIdMapArr[] =
{
    /*pcfCfgTableProfileId, tcamProfileIdArr*/
    {1, {3, 3, 4, 3}}, /* 60-bytes notIpv6 (3) and Ipv6 (4) */
    /* end of list stamp */
    {0xFF, {0, 0, 0, 0}}
};

static const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC sampleProfileSubkeyCfgArr[] =
{
    {
        1, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {1, 1, 1, 1}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        2, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {2, 2, 2, 2}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    /* For IPCL1 configured the same as fot IPCL0                         */
    /* They can be mapped to different profiles/muxlines tables instances */
    /* and another registers will get the same values                     */
    {
        1, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {1, 1, 1, 1}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        2, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {2, 2, 2, 2}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        3, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {3, 3, 3, 3}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        4, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {4, 4, 4, 4}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        5, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {5, 5, 5, 5}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    {
        6, /*tcamProfileId*/
        TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,
        {/*tcamProfileCfg*/
            /*subKeySizes*/
            {
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                CPSS_DXCH_TCAM_RULE_SIZE_60_B_E
            },
            {6, 6, 6, 6}, /*subKeyMuxTableLineIndexes*/
            {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
            0, /*sharedMuxTableLineIndex*/
            0, /*sharedMuxTableLineOffset*/
        }
    },
    /* end of list stamp */
    {0xFFFFFFFF, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,
        {{CPSS_DXCH_TCAM_RULE_SIZE_80_B_E}, {0}, {0}, 0, 0}}
};

/* IPCL 60-bytes ETH_OTHER and IPV4 */
static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC sampleMuxingTableEntry1Arr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_EXISTS_E, 0, 1}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_VID0_E, 0, 12}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_UP0_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_QOS_PROFILE_E, 0, 10}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E, 0, 6}, 0x3F}, /*ETH and IPV4*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IP_PROTOCOL_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_TPID_INDEX_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG1_TPID_INDEX_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_PTP_E, 0, 1}, 0x3F},
    /* DST_MAC included in IPCL and MPCL suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E, 0, 48}, 0x3F},
    /* SIP and DIP always included in suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_SIP_E, 0, 32}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_DIP_E, 0, 32}, 0x3F},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};
/* IPCL 60-bytes IPV6 */
static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC sampleMuxingTableEntry2Arr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_EXISTS_E, 0, 1}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_VID0_E, 0, 12}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_UP0_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_QOS_PROFILE_E, 0, 10}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV6_DSCP_E, 0, 6}, 0x3F}, /*IPV6*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IP_PROTOCOL_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_TPID_INDEX_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG1_TPID_INDEX_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_PTP_E, 0, 1}, 0x3F},
    /* SIP and DIP always included in suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_SIP_E, 0, 128}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_DIP_E, 0, 128}, 0x3F},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};
/* EPCL 60-bytes ETH_OTHER and IPV4 */
static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC sampleMuxingTableEntry3Arr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_TAG0_EXISTS_E, 0, 1}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_VID0_E, 0, 12}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_UP0_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_QOS_PROFILE_E, 0, 10}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E, 0, 6}, 0x3F}, /*ETH and IPV4*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IP_PROTOCOL_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E, 0, 48}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E, 0, 48}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IS_PTP_E, 0, 1}, 0x3F},
    /* SIP and DIP always included in suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_SIP_E, 0, 32}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_DIP_E, 0, 32}, 0x3F},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};
/* EPCL 60-bytes IPV6 */
static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC sampleMuxingTableEntry4Arr[] =
{
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_TAG0_EXISTS_E, 0, 1}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_VID0_E, 0, 12}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_UP0_E, 0, 3}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_QOS_PROFILE_E, 0, 10}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E, 0, 6}, 0x3F}, /*ETH and IPV4*/
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IP_PROTOCOL_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E, 0, 8}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IS_PTP_E, 0, 1}, 0x3F},
    /* SIP and DIP always included in suprkey */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_SIP_E, 0, 128}, 0x3F},
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_DIP_E, 0, 128}, 0x3F},
    /* end of list stamp */
    {{TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E, 0, 0}, 0}
};

static const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC sampleMuxingTableSubfieldsCfgArr[] =
{
    /* subKeyMuxTableLineIndex, pktTypeGroupIndex, pclLookup,  */
    /* muxWordWithCommonHeaderIndexesBmp, muxedSubfieldEntryArrPtr */
    {1, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 1, sampleMuxingTableEntry1Arr},
    {2, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 1, sampleMuxingTableEntry2Arr},
    {1, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E, 1, sampleMuxingTableEntry1Arr},
    {2, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E, 1, sampleMuxingTableEntry2Arr},
    {3, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,  1, sampleMuxingTableEntry3Arr},
    {4, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,  1, sampleMuxingTableEntry4Arr},
    {5, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,  1, sampleMuxingTableEntry1Arr},
    {6, 2, TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,  1, sampleMuxingTableEntry2Arr},
    /*END-OF-LIST Stamp*/
    {0xFF, 0, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, 0, NULL}
};

static const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC samplePclTcamFullCfg =
{
    &defaultCommonSubkeyHeader, /*subkeyCommonHeaderPtr*/

    3, /*numOfPktTypeGroups*/
    {/*udbPktTypesBmpArr*/
        /*[0] - Ethernet other, MPLS, UDE*/
        (
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E)
        ),
        /*[1] All IPV4 types */
        (
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E)
        ),
        /*[2] All IPV6 types */
        (
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E) |
            (1 << CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E)
        ),
    },
    /*ingressSuperKeyFieldsSet*/
    {
        {/*udbCfgAllFields*/
            {/*udbIndexesBmpArr*/
                {0xFFFFCFFF, 0x0FFFFFFF}, /*IPCL0 indexes 0-11,14-59*/
                {0xFFFFCFFF, 0x0FFFFFFF}, /*IPCL1 indexes 0-11,14-59*/
                {0xFFFFCFFF, 0x0FFFFFFF}, /*IPCL2 indexes 0-11,14-59*/
                {0xFFFFCC00, 0x0FFFFFFF}  /*MPCL indexes 10-11,14-59*/
            },
            sampleRuleIngressUdbSubFieldsUseArr /*ingressSubFieldsUseArrPtr*/
        },
        sampleRuleActionModifiedSubFieldsUseArr /*actionModifiedSubfieldsUseArePtr*/
    },
    /*egressSuperKeyFieldsSet*/
    {
        /*udbIndexesBmp*/
        {0xFFFFCFFF, 0x0FFFFFFF}, /*EPCL indexes 0-11,14-59*/
        sampleRuleEgressUdbSubFieldsUseArr /*egressSubFieldsUseArrPtr*/
    },
    /*cfgToTcamProfileIdMap*/
    {
        sampleIpclCfgToTcamProfileIdMapArr,/*ipclCfgToTcamProfileIdMapArrPtr*/
        sampleMpclCfgToTcamProfileIdMapArr,/*mpclCfgToTcamProfileIdMapArrPtr*/
        sampleEpclCfgToTcamProfileIdMapArr/*epclCfgToTcamProfileIdMapArrPtr*/
    },
    sampleProfileSubkeyCfgArr, /*profileSubkeyCfgArrPtr*/
    sampleMuxingTableSubfieldsCfgArr /*muxingTableSubfieldsCfgArrPtr*/
};

/**
* @internal tgfTcamClientsSamplePclConfigurationGet function
* @endinternal
*
* @brief   Get Sample PCL configuration.
*
* @retval Address of sample Configuration
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsSamplePclConfigurationGet
(
    GT_VOID
)
{
    return &samplePclTcamFullCfg;
}

#define PRV_ONE_LOOKUP_MAX_FIELDS_CNS 16

/**
* @internal tgfTcamClientsOneLookupPclConfigurationBuild function
* @endinternal
*
* @brief   Build One Lookup PCL configuration.
*
* @param[in] pclLookup                - PCL lookup
* @param[in] tcamKeySize              - TCAM Key Size
* @param[in] subFieldArrPtr           - (pointer to)array of subfields included in TCAM Key
*
* @retval pointer to static nemory with the fulfilled structure
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsOneLookupPclConfigurationBuild
(
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT               pclLookup,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  tcamKeySize,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *subFieldArrPtr
)
{
    static TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC  oneLookupPclTcamCfg;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* samplePtr;
    static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  cfgToTcamProfileIdMapArr[] =
    {
        /*pcfCfgTableProfileId, tcamProfileIdArr*/
        {1, {1, 1, 1, 1}}, /* same profile for all packet types */
        /* end of list stamp */
        {0xFF, {0, 0, 0, 0}}
    };
    static TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC profileSubkeyCfgArr[] =
    {
        {
            1, /*tcamProfileId*/
            TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, /*to update*/
            {/*tcamProfileCfg*/
                /*subKeySizes*/
                {
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E /*to update*/
                },
                {1, 1, 1, 1}, /*subKeyMuxTableLineIndexes*/
                {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
                0, /*sharedMuxTableLineIndex*/
                0, /*sharedMuxTableLineOffset*/
            }
        },
        /* end of list stamp */
        {0xFFFFFFFF, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,
            {{CPSS_DXCH_TCAM_RULE_SIZE_80_B_E}, {0}, {0}, 0, 0}}
    };
    static TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC     egressSubFieldsUseArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC    ingressSubFieldsUseArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC muxingTableSubfieldsArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC   sampleMuxingTableSubfieldsCfgArr[2];
    GT_STATUS rc;
    GT_U32 muxLineWordsMask;
    GT_U32 numOfFields;
    GT_U32 numOfUdbFields;
    GT_U32 idx;
    GT_U32 udbFldIdx;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT field;
    GT_U32 isIngress;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC fieldsInfo;

    isIngress = (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E) ? 0 : 1;
    numOfFields = 0;
    numOfUdbFields = 0;
    for (idx = 0; (idx < PRV_ONE_LOOKUP_MAX_FIELDS_CNS); idx++)
    {
        field = subFieldArrPtr[idx].fieldId;
        if (field == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E) break;
        numOfFields++;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            numOfUdbFields++;
        }
    }
    if (numOfFields >= PRV_ONE_LOOKUP_MAX_FIELDS_CNS)
    {
        return NULL;
    }
    switch (tcamKeySize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E: muxLineWordsMask = 0x01; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E: muxLineWordsMask = 0x03; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E: muxLineWordsMask = 0x07; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E: muxLineWordsMask = 0x0F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E: muxLineWordsMask = 0x1F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E: muxLineWordsMask = 0x3F; break;
        default: return NULL;
    }

    samplePtr = tgfTcamClientsSamplePclConfigurationGet();
    cpssOsMemSet(&oneLookupPclTcamCfg, 0, sizeof(TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC));
    /* copy from sample */
    /* common key header */
    oneLookupPclTcamCfg.subkeyCommonHeaderPtr = samplePtr->subkeyCommonHeaderPtr;
    /* packet type groups */
    oneLookupPclTcamCfg.numOfPktTypeGroups   = 3;
    oneLookupPclTcamCfg.udbPktTypesBmpArr[0] = samplePtr->udbPktTypesBmpArr[0];
    oneLookupPclTcamCfg.udbPktTypesBmpArr[1] = samplePtr->udbPktTypesBmpArr[1];
    oneLookupPclTcamCfg.udbPktTypesBmpArr[2] = samplePtr->udbPktTypesBmpArr[2];
    /* Action Modified fields */
    oneLookupPclTcamCfg.ingressSuperKeyFieldsSet.actionModifiedSubfieldsArrPtr =
        samplePtr->ingressSuperKeyFieldsSet.actionModifiedSubfieldsArrPtr;
    /* bitmaps of UDB indexes to use */
    cpssOsMemCpy(
        oneLookupPclTcamCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.udbIndexesBmpArr,
        samplePtr->ingressSuperKeyFieldsSet.udbCfgAllFields.udbIndexesBmpArr,
        sizeof(samplePtr->ingressSuperKeyFieldsSet.udbCfgAllFields.udbIndexesBmpArr));
    cpssOsMemCpy(
        oneLookupPclTcamCfg.egressSuperKeyFieldsSet.udbIndexesBmp,
        samplePtr->egressSuperKeyFieldsSet.udbIndexesBmp,
        sizeof(samplePtr->egressSuperKeyFieldsSet.udbIndexesBmp));

    /* profile mapping cfgProfile -> tcamProfile for all packet types: 1 -> 1 */
    oneLookupPclTcamCfg.cfgToTcamProfileIdMap.ipclCfgToTcamProfileIdMapArrPtr =
        cfgToTcamProfileIdMapArr;
    oneLookupPclTcamCfg.cfgToTcamProfileIdMap.mpclCfgToTcamProfileIdMapArrPtr =
        cfgToTcamProfileIdMapArr;
    oneLookupPclTcamCfg.cfgToTcamProfileIdMap.epclCfgToTcamProfileIdMapArrPtr =
        cfgToTcamProfileIdMapArr;
    /* TCAM Profile configuration */
    profileSubkeyCfgArr[0].pclLookup = pclLookup;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[0] = tcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[1] = tcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[2] = tcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[3] = tcamKeySize;
    oneLookupPclTcamCfg.profileSubkeyCfgArrPtr = profileSubkeyCfgArr;

    cpssOsMemSet(egressSubFieldsUseArr,   0, sizeof(egressSubFieldsUseArr));
    cpssOsMemSet(ingressSubFieldsUseArr,  0, sizeof(ingressSubFieldsUseArr));
    cpssOsMemSet(muxingTableSubfieldsArr, 0, sizeof(muxingTableSubfieldsArr));
    cpssOsMemSet(sampleMuxingTableSubfieldsCfgArr, 0, sizeof(sampleMuxingTableSubfieldsCfgArr));
    sampleMuxingTableSubfieldsCfgArr[1].subKeyMuxTableLineIndex = 0xFF;
    udbFldIdx = 0;
    for (idx = 0; (idx < numOfFields); idx++)
    {
        field = subFieldArrPtr[idx].fieldId;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            if (isIngress)
            {
                ingressSubFieldsUseArr[udbFldIdx].subField = subFieldArrPtr[idx];
                ingressSubFieldsUseArr[udbFldIdx].udbPktTypeGroupsBmpArr[pclLookup] = 7; /*all packet types*/
            }
            else
            {
                egressSubFieldsUseArr[udbFldIdx].subField = subFieldArrPtr[idx];
                egressSubFieldsUseArr[udbFldIdx].udbPktTypeGroupsBmp = 7; /*all packet types*/
            }
            udbFldIdx++;
        }
        muxingTableSubfieldsArr[idx].subField          = subFieldArrPtr[idx];
        muxingTableSubfieldsArr[idx].muxWordIndexesBmp = muxLineWordsMask;
    }
    /* end of list stamps */
    if (isIngress)
    {
        egressSubFieldsUseArr[0].subField.fieldId  = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
        ingressSubFieldsUseArr[numOfUdbFields].subField.fieldId = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
    }
    else
    {
        egressSubFieldsUseArr[numOfUdbFields].subField.fieldId  = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
        ingressSubFieldsUseArr[0].subField.fieldId = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
    }
    muxingTableSubfieldsArr[numOfFields].subField.fieldId   = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;

    oneLookupPclTcamCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.ingressSubFieldsUseArrPtr = ingressSubFieldsUseArr;
    oneLookupPclTcamCfg.egressSuperKeyFieldsSet.egressSubFieldsUseArrPtr = egressSubFieldsUseArr;
    oneLookupPclTcamCfg.muxingTableSubfieldsCfgArrPtr = sampleMuxingTableSubfieldsCfgArr;
    sampleMuxingTableSubfieldsCfgArr[0].subKeyMuxTableLineIndex = 1;
    sampleMuxingTableSubfieldsCfgArr[0].muxedSubfieldEntryArrPtr = muxingTableSubfieldsArr;
    sampleMuxingTableSubfieldsCfgArr[0].muxWordWithCommonHeaderIndexesBmp = 1;
    sampleMuxingTableSubfieldsCfgArr[0].pclLookup = pclLookup;
    sampleMuxingTableSubfieldsCfgArr[0].pktTypeGroupIndex = 0;

    return &oneLookupPclTcamCfg;
}

/**
* @internal tgfTcamClientsIngressAndEgressPclConfigurationBuild function
* @endinternal
*
* @brief   Build One Ingress (or Midway) Lookup and Egress Lookup PCL configuration.
*
* @param[in] ipclLookup               - ingress PCL lookup
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved IPCL UDBs
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved EPCL UDBs
* @param[in] ipclTcamKeySize          - IPCL TCAM Key Size
* @param[in] epclTcamKeySize          - EPCL TCAM Key Size
* @param[in] ipclSubFieldArrPtr       - (pointer to)array of subfields included in IPCL TCAM Key
* @param[in] epclSubFieldArrPtr       - (pointer to)array of subfields included in EPCL TCAM Key
*
* @retval pointer to static nemory with the fulfilled structure
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsIngressAndEgressPclConfigurationBuild
(
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT               ipclLookup,
    IN   GT_U32                                        ipclReservedUdbBmpArr[],
    IN   GT_U32                                        epclReservedUdbBmpArr[],
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  ipclTcamKeySize,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  epclTcamKeySize,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *ipclSubFieldArrPtr,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *epclSubFieldArrPtr
)
{
    static TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC  twoLookupPclTcamCfg;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* samplePtr;
    static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  ipclCfgToTcamProfileIdMapArr[] =
    {
        /*pcfCfgTableProfileId, tcamProfileIdArr*/
        {1, {1, 1, 1, 1}}, /* same profile for all packet types */
        /* end of list stamp */
        {0xFF, {0, 0, 0, 0}}
    };
    static const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  epclCfgToTcamProfileIdMapArr[] =
    {
        /*pcfCfgTableProfileId, tcamProfileIdArr*/
        {1, {2, 2, 2, 2}}, /* same profile for all packet types */
        /* end of list stamp */
        {0xFF, {0, 0, 0, 0}}
    };
    static TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC profileSubkeyCfgArr[] =
    {
        {
            1, /*tcamProfileId*/
            TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E, /*to update*/
            {/*tcamProfileCfg*/
                /*subKeySizes*/
                {
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E /*to update*/
                },
                {1, 1, 1, 1}, /*subKeyMuxTableLineIndexes*/
                {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
                0, /*sharedMuxTableLineIndex*/
                0, /*sharedMuxTableLineOffset*/
            }
        },
        {
            2, /*tcamProfileId*/
            TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E,
            {/*tcamProfileCfg*/
                /*subKeySizes*/
                {
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,/*to update*/
                    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E /*to update*/
                },
                {2, 2, 2, 2}, /*subKeyMuxTableLineIndexes*/
                {0, 0, 0, 0},/*subKeyMuxTableLineOffsets*/
                0, /*sharedMuxTableLineIndex*/
                0, /*sharedMuxTableLineOffset*/
            }
        },
        /* end of list stamp */
        {0xFFFFFFFF, TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,
            {{CPSS_DXCH_TCAM_RULE_SIZE_80_B_E}, {0}, {0}, 0, 0}}
    };
    static TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC     egressSubFieldsUseArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC    ingressSubFieldsUseArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC ipclMuxingTableSubfieldsArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC epclMuxingTableSubfieldsArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    static TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC   sampleMuxingTableSubfieldsCfgArr[3];
    GT_STATUS rc;
    GT_U32 ipclMuxLineWordsMask;
    GT_U32 epclMuxLineWordsMask;
    GT_U32 ipclNumOfFields;
    GT_U32 ipclNumOfUdbFields;
    GT_U32 epclNumOfFields;
    GT_U32 epclNumOfUdbFields;
    GT_U32 idx;
    GT_U32 groupIdx;
    GT_U32 udbFldIdx;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT field;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC fieldsInfo;
    GT_U32 wordsInUdbBmp;

    ipclNumOfFields = 0;
    ipclNumOfUdbFields = 0;
    for (idx = 0; (idx < PRV_ONE_LOOKUP_MAX_FIELDS_CNS); idx++)
    {
        field = ipclSubFieldArrPtr[idx].fieldId;
        if (field == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E) break;
        ipclNumOfFields++;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            ipclNumOfUdbFields++;
        }
    }
    if (ipclNumOfFields >= PRV_ONE_LOOKUP_MAX_FIELDS_CNS)
    {
        return NULL;
    }
    epclNumOfFields = 0;
    epclNumOfUdbFields = 0;
    for (idx = 0; (idx < PRV_ONE_LOOKUP_MAX_FIELDS_CNS); idx++)
    {
        field = epclSubFieldArrPtr[idx].fieldId;
        if (field == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E) break;
        epclNumOfFields++;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            epclNumOfUdbFields++;
        }
    }
    if (epclNumOfFields >= PRV_ONE_LOOKUP_MAX_FIELDS_CNS)
    {
        return NULL;
    }
    switch (ipclTcamKeySize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E: ipclMuxLineWordsMask = 0x01; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E: ipclMuxLineWordsMask = 0x03; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E: ipclMuxLineWordsMask = 0x07; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E: ipclMuxLineWordsMask = 0x0F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E: ipclMuxLineWordsMask = 0x1F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E: ipclMuxLineWordsMask = 0x3F; break;
        default: return NULL;
    }
    switch (epclTcamKeySize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E: epclMuxLineWordsMask = 0x01; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E: epclMuxLineWordsMask = 0x03; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E: epclMuxLineWordsMask = 0x07; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E: epclMuxLineWordsMask = 0x0F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E: epclMuxLineWordsMask = 0x1F; break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E: epclMuxLineWordsMask = 0x3F; break;
        default: return NULL;
    }

    samplePtr = tgfTcamClientsSamplePclConfigurationGet();
    cpssOsMemSet(&twoLookupPclTcamCfg, 0, sizeof(TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC));
    /* copy from sample */
    /* common key header */
    twoLookupPclTcamCfg.subkeyCommonHeaderPtr = samplePtr->subkeyCommonHeaderPtr;
    /* packet type groups */
    twoLookupPclTcamCfg.numOfPktTypeGroups   = 3;
    twoLookupPclTcamCfg.udbPktTypesBmpArr[0] = samplePtr->udbPktTypesBmpArr[0];
    twoLookupPclTcamCfg.udbPktTypesBmpArr[1] = samplePtr->udbPktTypesBmpArr[1];
    twoLookupPclTcamCfg.udbPktTypesBmpArr[2] = samplePtr->udbPktTypesBmpArr[2];
    /* Action Modified fields */
    twoLookupPclTcamCfg.ingressSuperKeyFieldsSet.actionModifiedSubfieldsArrPtr =
        samplePtr->ingressSuperKeyFieldsSet.actionModifiedSubfieldsArrPtr;
    /* bitmaps of UDB indexes to use */
    wordsInUdbBmp = (TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS + 31) / 32;
    for (groupIdx = 0; (groupIdx < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); groupIdx++)
    {
        for (idx = 0; (idx < wordsInUdbBmp); idx++)
        {
            twoLookupPclTcamCfg.ingressSuperKeyFieldsSet.udbCfgAllFields
                .udbIndexesBmpArr[groupIdx][idx] =
                    samplePtr->ingressSuperKeyFieldsSet
                        .udbCfgAllFields.udbIndexesBmpArr[groupIdx][idx]
                        & (~ ipclReservedUdbBmpArr[idx]);
        }
    }
    for (idx = 0; (idx < wordsInUdbBmp); idx++)
    {
        twoLookupPclTcamCfg.egressSuperKeyFieldsSet.udbIndexesBmp[idx] =
            samplePtr->egressSuperKeyFieldsSet.udbIndexesBmp[idx]
                & (~ epclReservedUdbBmpArr[idx]);
    }

    /* profile mapping cfgProfile -> tcamProfile for all packet types: 1 -> 1 */
    twoLookupPclTcamCfg.cfgToTcamProfileIdMap.ipclCfgToTcamProfileIdMapArrPtr =
        ipclCfgToTcamProfileIdMapArr;
    twoLookupPclTcamCfg.cfgToTcamProfileIdMap.mpclCfgToTcamProfileIdMapArrPtr =
        ipclCfgToTcamProfileIdMapArr;
    twoLookupPclTcamCfg.cfgToTcamProfileIdMap.epclCfgToTcamProfileIdMapArrPtr =
        epclCfgToTcamProfileIdMapArr;
    /* TCAM Profile configuration */
    profileSubkeyCfgArr[0].pclLookup = ipclLookup;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[0] = ipclTcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[1] = ipclTcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[2] = ipclTcamKeySize;
    profileSubkeyCfgArr[0].tcamProfileCfg.subKeySizes[3] = ipclTcamKeySize;
    profileSubkeyCfgArr[1].tcamProfileCfg.subKeySizes[0] = epclTcamKeySize;
    profileSubkeyCfgArr[1].tcamProfileCfg.subKeySizes[1] = epclTcamKeySize;
    profileSubkeyCfgArr[1].tcamProfileCfg.subKeySizes[2] = epclTcamKeySize;
    profileSubkeyCfgArr[1].tcamProfileCfg.subKeySizes[3] = epclTcamKeySize;
    twoLookupPclTcamCfg.profileSubkeyCfgArrPtr = profileSubkeyCfgArr;

    cpssOsMemSet(egressSubFieldsUseArr,   0, sizeof(egressSubFieldsUseArr));
    cpssOsMemSet(ingressSubFieldsUseArr,  0, sizeof(ingressSubFieldsUseArr));
    cpssOsMemSet(ipclMuxingTableSubfieldsArr, 0, sizeof(ipclMuxingTableSubfieldsArr));
    cpssOsMemSet(epclMuxingTableSubfieldsArr, 0, sizeof(epclMuxingTableSubfieldsArr));
    cpssOsMemSet(sampleMuxingTableSubfieldsCfgArr, 0, sizeof(sampleMuxingTableSubfieldsCfgArr));
    sampleMuxingTableSubfieldsCfgArr[2].subKeyMuxTableLineIndex = 0xFF;
    udbFldIdx = 0;
    for (idx = 0; (idx < ipclNumOfFields); idx++)
    {
        field = ipclSubFieldArrPtr[idx].fieldId;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            ingressSubFieldsUseArr[udbFldIdx].subField = ipclSubFieldArrPtr[idx];
            ingressSubFieldsUseArr[udbFldIdx].udbPktTypeGroupsBmpArr[ipclLookup] = 7; /*all packet types*/
            udbFldIdx++;
        }
        ipclMuxingTableSubfieldsArr[idx].subField          = ipclSubFieldArrPtr[idx];
        ipclMuxingTableSubfieldsArr[idx].muxWordIndexesBmp = ipclMuxLineWordsMask;
    }
    udbFldIdx = 0;
    for (idx = 0; (idx < epclNumOfFields); idx++)
    {
        field = epclSubFieldArrPtr[idx].fieldId;
        rc = tgfTcamClientsPclFieldDefinitionGet(field, &fieldsInfo);
        if ((rc != GT_OK))
        {
            return NULL;
        }
        if (fieldsInfo.fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
        {
            egressSubFieldsUseArr[udbFldIdx].subField = epclSubFieldArrPtr[idx];
            egressSubFieldsUseArr[udbFldIdx].udbPktTypeGroupsBmp = 7; /*all packet types*/
            udbFldIdx++;
        }
        epclMuxingTableSubfieldsArr[idx].subField          = epclSubFieldArrPtr[idx];
        epclMuxingTableSubfieldsArr[idx].muxWordIndexesBmp = epclMuxLineWordsMask;
    }
    /* end of list stamps */

    ingressSubFieldsUseArr[ipclNumOfUdbFields].subField.fieldId   = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
    egressSubFieldsUseArr[epclNumOfUdbFields].subField.fieldId    = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
    ipclMuxingTableSubfieldsArr[ipclNumOfFields].subField.fieldId = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;
    epclMuxingTableSubfieldsArr[epclNumOfFields].subField.fieldId = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;

    twoLookupPclTcamCfg.ingressSuperKeyFieldsSet.udbCfgAllFields.ingressSubFieldsUseArrPtr = ingressSubFieldsUseArr;
    twoLookupPclTcamCfg.egressSuperKeyFieldsSet.egressSubFieldsUseArrPtr = egressSubFieldsUseArr;
    twoLookupPclTcamCfg.muxingTableSubfieldsCfgArrPtr = sampleMuxingTableSubfieldsCfgArr;
    sampleMuxingTableSubfieldsCfgArr[0].subKeyMuxTableLineIndex = 1;
    sampleMuxingTableSubfieldsCfgArr[0].muxedSubfieldEntryArrPtr = ipclMuxingTableSubfieldsArr;
    sampleMuxingTableSubfieldsCfgArr[0].muxWordWithCommonHeaderIndexesBmp = 1;
    sampleMuxingTableSubfieldsCfgArr[0].pclLookup = ipclLookup;
    sampleMuxingTableSubfieldsCfgArr[0].pktTypeGroupIndex = 0;
    sampleMuxingTableSubfieldsCfgArr[1].subKeyMuxTableLineIndex = 2;
    sampleMuxingTableSubfieldsCfgArr[1].muxedSubfieldEntryArrPtr = epclMuxingTableSubfieldsArr;
    sampleMuxingTableSubfieldsCfgArr[1].muxWordWithCommonHeaderIndexesBmp = 1;
    sampleMuxingTableSubfieldsCfgArr[1].pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E;
    sampleMuxingTableSubfieldsCfgArr[1].pktTypeGroupIndex = 0;

    return &twoLookupPclTcamCfg;
}

