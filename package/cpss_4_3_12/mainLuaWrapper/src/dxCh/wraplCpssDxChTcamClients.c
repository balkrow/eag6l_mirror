/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChTcamClients.c
*
* DESCRIPTION:
*       A lua wrapper for TCAM Clients functions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <extUtils/tcamClients/tgfTcamClients.h>
#include <extUtils/tcamClients/prvTgfTcamClients.h>
#include <generic/private/prvWraplGeneral.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <dxCh/wraplCpssDxChPcl.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>

use_prv_struct(CPSS_DXCH_PCL_ACTION_STC)
use_prv_struct(CPSS_INTERFACE_INFO_STC)

/* Packet Type Group Indexes */
#define PRV_PKT_TYPES_ETH  0
#define PRV_PKT_TYPES_IPV4 1
#define PRV_PKT_TYPES_IPV6 2

/* maximal amount of subfields in PCL rule */
#define MAX_SUBFIELDS_NUM 32
#define PRV_ONE_LOOKUP_MAX_FIELDS_CNS 16

static GT_STATUS nameToFieldId
(
    IN  const char*   name,
    OUT TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT  *fieldIdPtr
)
{
    static const struct
    {
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT fieldId;
        const char* name;
    } voc[] =
    {
        #define VOC(_n) {TGF_TCAM_CLIENTS_PCL_RULE_FIELD_##_n##_E, #_n}
        #define VOC_REG(_i) {(TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E + _i), "APP_REGISTRED" #_i}
        VOC(STAMP_LIST_END),
        VOC(IMETA_PKT_TYPE_BMP),
        VOC(IMETA_IS_IP),
        VOC(IMETA_IS_IPV4),
        VOC(IMETA_IS_IPV6),
        VOC(IMETA_IS_ARP),
        VOC(IMETA_IS_MPLS),
        VOC(IMETA_IS_L3_INVALID),
        VOC(IMETA_IS_L4_VALID),
        VOC(IMETA_TAG0_EXISTS),
        VOC(EMETA_TAG0_EXISTS),
        VOC(IMETA_TAG1_EXISTS),
        VOC(EMETA_TAG1_EXISTS),
        VOC(IMETA_IP_PROTOCOL),
        VOC(EMETA_IP_PROTOCOL),
        VOC(IMETA_SRC_PHY_PORT),
        VOC(IMETA_SRC_EPORT),
        VOC(EMETA_SRC_PHY_PORT),
        VOC(IMETA_QOS_PROFILE),
        VOC(EMETA_QOS_PROFILE),
        VOC(IMETA_VID0),
        VOC(EMETA_VID0),
        VOC(IMETA_VID1),
        VOC(EMETA_VID1),
        VOC(IMETA_UP0),
        VOC(EMETA_UP0),
        VOC(IMETA_UP1),
        VOC(EMETA_UP1),
        VOC(IMETA_DSCP),
        VOC(IMETA_TAG0_TPID_INDEX),
        VOC(IMETA_TAG1_TPID_INDEX),
        VOC(IMETA_IS_PTP),
        VOC(EMETA_IS_PTP),
        VOC(L2_DST_MAC),
        VOC(L2_SRC_MAC),
        VOC(L3_ETHERTYPE),
        VOC(L3_IPV4_DSCP),
        VOC(L3_IPV6_DSCP),
        VOC(L4_TCP_SRC_PORT),
        VOC(L4_TCP_DST_PORT),
        VOC(AMF_EVLAN),
        VOC(AMF_SRC_EPG),
        VOC(AMF_SRC_IS_VPORT),
        VOC(AMF_SRC_VPORT_EXT),
        VOC(AMF_DST_EPG),
        VOC(AMF_QOS_PROFILE),
        VOC(AMF_SRC_EPORT),
        VOC(AMF_TAG1_VID),
        VOC(AMF_TAG1_UP),
        VOC(AMF_VRF_ID),
        VOC(AMF_SRC_ID),
        VOC(AMF_HASH_BYTES),
        VOC(AMF_GEN_CLASS),
        VOC(AMF_PACKET_CMD),
        VOC(AMF_CPU_CODE),
        VOC(AMF_TRG_DEV),
        VOC(AMF_TRG_IS_VIDX),
        VOC(AMF_TRG_IS_TRUNK),
        VOC(AMF_TRG_IS_VPORT),
        VOC(AMF_TRG_VPORT_EXT),
        VOC(AMF_TRG_PORT),
        VOC(AMF_IS_TS),
        VOC(AMF_ARP_OR_TS_PTR),
        VOC(AMF_EM_PRPFILE_ID1),
        VOC(AMF_EM_PRPFILE_ID2),
        VOC(AMF_POLICER_PTR),
        VOC(AMF_BILLING_ENABLE),
        VOC(AMF_METERING_ENABLE),
        VOC(AMF_COPY_RESERVED),
        VOC(AMF_FLOW_ID),
        VOC(AMF_PCL_ID2),
        VOC(IFIX_PCL_ID),
        VOC(IFIX_UDB_VALID),
        VOC(IFIX_MAC2ME),
        VOC(IFIX_SRC_DEV_IS_OWN),
        VOC(IFIX_MAC_DA),
        VOC(FIX_IPV4_DIP),
        VOC(FIX_IPV6_DIP),
        VOC(FIX_IPV4_SIP),
        VOC(FIX_IPV6_SIP),
        VOC(EFIX_PCL_ID),
        VOC(EFIX_UDB_VALID),
        VOC(EFIX_SRC_EPG),
        VOC(EFIX_DST_EPG),
        VOC(EFIX_EM_PROFILE),
        VOC_REG(0),
        VOC_REG(1),
        VOC_REG(2),
        VOC_REG(3),
        VOC_REG(4),
        VOC_REG(5),
        VOC_REG(6),
        VOC_REG(7)
        #undef VOC
        #undef VOC_REG
    };
    static GT_U32 vocSize = sizeof(voc) / sizeof(voc[0]);
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(fieldIdPtr);

    for (ii = 0; (ii < vocSize); ii++)
    {
        if (cpssOsStrCmp(voc[ii].name, name) == 0)
        {
            *fieldIdPtr = voc[ii].fieldId;
            return GT_OK;
        }
    }
    return GT_BAD_PARAM;
}

static GT_STATUS nameToLookup
(
    IN  const char*   name,
    OUT TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  *pclLookupPtr
)
{
    static const struct
    {
        TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;
        const char* name;
    } voc[] =
    {
        #define VOC(_n) {TGF_TCAM_CLIENTS_PCL_LOOKUP_##_n##_E, #_n}
        VOC(IPCL0),
        VOC(IPCL1),
        VOC(IPCL2),
        VOC(MPCL),
        VOC(EPCL)
        #undef VOC
    };

    static GT_U32 vocSize = sizeof(voc) / sizeof(voc[0]);
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(pclLookupPtr);

    for (ii = 0; (ii < vocSize); ii++)
    {
        if (cpssOsStrCmp(voc[ii].name, name) == 0)
        {
            *pclLookupPtr = voc[ii].pclLookup;
            return GT_OK;
        }
    }
    return GT_BAD_PARAM;
}

static GT_STATUS nameToPktTypeGroup
(
    IN  const char*   name,
    OUT GT_U32       *pktTypeGroupPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(pktTypeGroupPtr);
    if (cpssOsStrCmp(name, "ETH") == 0)
    {
        *pktTypeGroupPtr = PRV_PKT_TYPES_ETH;
        return GT_OK;
    }
    if (cpssOsStrCmp(name, "IPV4") == 0)
    {
        *pktTypeGroupPtr = PRV_PKT_TYPES_IPV4;
        return GT_OK;
    }
    if (cpssOsStrCmp(name, "IPV6") == 0)
    {
        *pktTypeGroupPtr = PRV_PKT_TYPES_IPV6;
        return GT_OK;
    }
    return GT_BAD_PARAM;
}

static GT_STATUS nameToFieldType
(
    IN  const char*   name,
    OUT TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ENT  *fieldTypePtr
)
{
    static const struct
    {
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ENT fieldType;
        const char* name;
    } voc[] =
    {
        #define VOC(_n) {TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_##_n##_E, #_n}
        VOC(UDB_CFG),
        VOC(UDB_APP),
        VOC(ACTION_MODIFIED),
        VOC(FIXED),
        VOC(MPCL_LIST_OF_FIELDS)
        #undef VOC
    };

    static GT_U32 vocSize = sizeof(voc) / sizeof(voc[0]);
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(fieldTypePtr);

    for (ii = 0; (ii < vocSize); ii++)
    {
        if (cpssOsStrCmp(voc[ii].name, name) == 0)
        {
            *fieldTypePtr = voc[ii].fieldType;
            return GT_OK;
        }
    }
    return GT_BAD_PARAM;
}

static GT_STATUS nameToUdbAnchor
(
    IN  const char*   name,
    OUT CPSS_DXCH_PCL_OFFSET_TYPE_ENT  *udbAnchorPtr
)
{
    static const struct
    {
        CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbAnchor;
        const char* name;
    } voc[] =
    {
        #define VOC(_n) {CPSS_DXCH_PCL_OFFSET_##_n##_E, #_n}
        VOC(L2),
        VOC(L3),
        VOC(L4),
        VOC(IPV6_EXT_HDR),
        VOC(TCP_UDP_COMPARATOR),
        VOC(L3_MINUS_2),
        VOC(MPLS_MINUS_2),
        VOC(TUNNEL_L2),
        VOC(TUNNEL_L3_MINUS_2),
        VOC(METADATA),
        VOC(TUNNEL_L4),
        VOC(INVALID)
        #undef VOC
    };

    static GT_U32 vocSize = sizeof(voc) / sizeof(voc[0]);
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(udbAnchorPtr);

    for (ii = 0; (ii < vocSize); ii++)
    {
        if (cpssOsStrCmp(voc[ii].name, name) == 0)
        {
            *udbAnchorPtr = voc[ii].udbAnchor;
            return GT_OK;
        }
    }
    return GT_BAD_PARAM;
}

/*******************************************************************************
* wrlPrvTgfTcamClientsPclFieldRegister
*
* DESCRIPTION:
*   Register PCL Field
*
* @param[in] L         - 1 - device numer
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvTgfTcamClientsPclFieldRegister
(
    IN lua_State *L
)
{
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId;
    GT_U32                                   fieldIdInReg;
    GT_STATUS                                rc;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC fieldsInfo;
    const char* fieldTypeName;
    const char* udbAnchorName;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }

    if (lua_gettop(L) < 5)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, fieldIdInReg,    1, GT_U32);
    fieldId = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E + fieldIdInReg;
    fieldTypeName = lua_tolstring(L, 2, NULL/*lenPtr*/);
    rc = nameToFieldType(fieldTypeName, &fieldsInfo.fieldType);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    udbAnchorName = lua_tolstring(L, 3, NULL/*lenPtr*/);
    rc = nameToUdbAnchor(udbAnchorName, &fieldsInfo.udbAnchor);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, fieldsInfo.bitOffset,    4, GT_U32);
    PARAM_NUMBER(rc, fieldsInfo.bitLength,    5, GT_U32);

    rc = tgfTcamClientsPclFieldRegister(fieldId, &fieldsInfo);
    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsDbHandleDelete
*
* DESCRIPTION:
*       Delete  TCAM Clents DB.
*
* @param[in] L         - None
*
* RETURNS: GT_OK
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvLuaTgfTcamClientsDbHandleDelete
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        return GT_NOT_INITIALIZED;
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        rc = tgfTcamClientsConfigurationDbDelete(tcamClientsDbHandle);
        prvTgfTcamClientCommomHandlersHandleSet(0, NULL);
    }
    return rc;
}
int wrlPrvLuaTgfTcamClientsDbHandleDelete
(
    IN lua_State *L
)
{
    GT_STATUS rc = prvLuaTgfTcamClientsDbHandleDelete();
    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsDbHandleCreate
*
* DESCRIPTION:
*       Create  TCAM Clents DB - sample configuration.
*
* @param[in] L         - None
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvLuaTgfTcamClientsDbHandleCreate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle == NULL)
    {
        appCfgPtr = tgfTcamClientsSamplePclConfigurationGet();
        rc = tgfTcamClientsConfigurationDbCreate(
            appCfgPtr,
            &tcamClientsDbHandle);
        prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    }
    return rc;
}
int wrlPrvLuaTgfTcamClientsDbHandleCreate
(
    IN lua_State *L
)
{
    GT_STATUS rc = prvLuaTgfTcamClientsDbHandleCreate();
    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* prvLuaTgfTcamClientsDbHandlePrint
*
* DESCRIPTION:
*       Print  TCAM Clents DB.
*
* @param[in] L         - None
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvLuaTgfTcamClientsDbHandlePrint
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *dbPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        return GT_NOT_INITIALIZED;
    }

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        dbPtr = (const PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC*)tcamClientsDbHandle;
        rc = prvTgfTcamClientsInternalDbDump(&(dbPtr->db));
    }

    return rc;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsDevInit
*
* DESCRIPTION: used sample configuration
i*
* @param[in] L         - 1 - device numer
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvLuaTgfTcamClientsDevInit
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc = GT_OK;
    TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }
    /* used sample configuration */
    rc = prvLuaTgfTcamClientsDbHandleCreate();
    if (rc != GT_OK)
    {
        return rc;
    }
    if (prvTgfTcamClientCommomHandlersDeviceConfiguredGet(0, devNum))
    {
        return GT_OK;
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    rc = tgfTcamClientsDeviceConfigure(
        devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }
    return rc;
}
int wrlPrvLuaTgfTcamClientsDevInit
(
    IN lua_State *L
)
{
    GT_STATUS rc;
    GT_U8     devNum;

    if (lua_gettop(L) < 1)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    rc = prvLuaTgfTcamClientsDevInit(devNum);
    lua_pushinteger(L, rc);
    return 1;
}

/**
* @internal prvLuaTgfTcamClientsOneLookupDevInit function
* @endinternal
*
* @brief   Configure device for One Lookup PCL configuration.
*
* @param[in] devNum          - deviceNumber
* @param[in] pclLookup       - PCL lookup
* @param[in] tcamKeySize     - TCAM Key Size
* @param[in] subFieldArrPtr  - (pointer to)array of subfields included in TCAM Key
*
* @retval GT_OK on success, other om Fail
*/
GT_STATUS prvLuaTgfTcamClientsOneLookupDevInit
(
    IN   GT_U8                                    devNum,
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT          pclLookup,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT             tcamKeySize,
    IN   TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *subFieldArrPtr
)
{
    GT_STATUS                                      rc = GT_OK;
    const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr;
    TGF_TCAM_CLIENTS_DB_HANDLE                     tcamClientsDbHandle;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (prvTgfTcamClientCommomHandlersLibInitDoneGet() == GT_FALSE)
    {
        tgfTcamClientsLibInit();
        prvTgfTcamClientCommomHandlersLibInitDoneSet();
    }
    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    if (tcamClientsDbHandle)
    {
        prvLuaTgfTcamClientsDbHandleDelete();
        /* all devices cleared from bitmap */
    }
    appCfgPtr = tgfTcamClientsOneLookupPclConfigurationBuild(
        pclLookup, tcamKeySize, subFieldArrPtr);
    if (appCfgPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    rc = tgfTcamClientsConfigurationDbCreate(
        appCfgPtr,
        &tcamClientsDbHandle);
    if (rc != GT_OK)
    {
        return rc;
    }
    prvTgfTcamClientCommomHandlersHandleSet(0, tcamClientsDbHandle);
    rc = tgfTcamClientsDeviceConfigure(
        devNum, tcamClientsDbHandle);
    if (rc == GT_OK)
    {
        prvTgfTcamClientCommomHandlersDeviceConfiguredSet(0, devNum);
    }
    return rc;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsPclCfgTblSet
*
* DESCRIPTION:
*       Set PCL Configuration Table Entry Configuration.
*
* @param[in] L         -
    IN   GT_U8                                    devNum,
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT          pclLookup,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT             tcamKeySize,
    IN   TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *subFieldArrPtr
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvLuaTgfTcamClientsOneLookupDevInit
(
    IN lua_State *L
)
{
    GT_STATUS                                       rc = GT_OK;
    GT_U8                                           devNum;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT                    tcamKeySize;
    const char *name;
    static TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subFieldArr[PRV_ONE_LOOKUP_MAX_FIELDS_CNS];
    GT_U32 subfieldIdx;
    GT_U32 tcamKeySizeInBytes;

    if (lua_gettop(L) < 4)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    /*pclLookup*/
    name = lua_tolstring(L, 2, NULL/*lenPtr*/);
    rc = nameToLookup(name, &pclLookup);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, tcamKeySizeInBytes, 3, GT_U32);
    switch (tcamKeySizeInBytes)
    {
        case 10: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E; break;
        case 20: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E; break;
        case 30: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E; break;
        case 40: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_40_B_E; break;
        case 50: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E; break;
        case 60: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_60_B_E; break;
        case 80: tcamKeySize = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E; break;
        default:
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
    }

    /*subFieldArr*/
    cpssOsMemSet(
        subFieldArr,
        0, sizeof(subFieldArr));
    if (lua_type(L, 4) != LUA_TTABLE)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    for (subfieldIdx = 0; (1); subfieldIdx++)
    {
        if (subfieldIdx >= PRV_ONE_LOOKUP_MAX_FIELDS_CNS)
        {
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
        }
        lua_rawgeti(L, 4, (subfieldIdx + 1));
        if ((lua_type(L, -1) == LUA_TNONE)
            || (lua_type(L, -1) == LUA_TNIL))
        {
            /* end of passed list */
            lua_pop(L, 1);
            break;
        }
        if (lua_type(L, -1) != LUA_TTABLE)
        {
            lua_pop(L, 1);
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
        }
        lua_rawgeti(L,-1, 1);
        name = lua_tolstring(L, -1, NULL/*lenPtr*/);
        lua_pop(L, 1);
        if (name == NULL)
        {
            lua_pop(L, 1);
            break;
        }
        rc = nameToFieldId(name, &(subFieldArr[subfieldIdx].fieldId));
        if (rc != GT_OK)
        {
            lua_pop(L, 1);
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
        }
        lua_rawgeti(L,-1, 2);
        PARAM_NUMBER(rc, subFieldArr[subfieldIdx].bitOffset, -1, GT_U8);
        lua_pop(L, 1);
        lua_rawgeti(L,-1, 3);
        PARAM_NUMBER(rc, subFieldArr[subfieldIdx].bitLength, -1, GT_U8);
        lua_pop(L, 1);
        /* pop table node */
        lua_pop(L, 1);
    }
    /* end of array stamp */
    subFieldArr[subfieldIdx].fieldId =
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;

    rc = prvLuaTgfTcamClientsOneLookupDevInit(
        devNum, pclLookup, tcamKeySize, subFieldArr);

    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsPclRuleSet
*
* DESCRIPTION:
*       Set TCAM Rule.
*
* @param[in] L         -
*   IN GT_U8                                           devNum,
*   IN GT_U32                                          pclCfgProfileId,
*   IN GT_U32                                          udbPacketTypeGroupIndex,
*   IN TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup,
*   IN GT_U32                                          tcamSubkeyIndex,
*   IN GT_U32                                          ruleIndex,
*   IN GT_BOOL                                         writeValidRule,
*   IN GT_U32                                          commonKeyHeaderMask,
*   IN GT_U32                                          commonKeyHeaderPattern
*   IN TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    *subFieldConditionArrPtr,
*   IN CPSS_DXCH_PCL_ACTION_STC                        *actionPtr
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvLuaTgfTcamClientsPclRuleSet
(
    IN lua_State *L
)
{
    GT_STATUS                                       rc = GT_OK;
    GT_U8                                           devNum;
    GT_U32                                          pclCfgProfileId;
    GT_U32                                          udbPacketTypeGroupIndex;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup;
    GT_U32                                          tcamSubkeyIndex;
    GT_U32                                          ruleIndex;
    GT_BOOL                                         writeValidRule;
    GT_U32                                          commonKeyHeaderMask;
    GT_U32                                          commonKeyHeaderPattern;
    const char *name;
    static TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC subFieldConditionArr[MAX_SUBFIELDS_NUM + 1];
    static CPSS_DXCH_PCL_ACTION_STC                     action;
    GT_U32 subfieldIdx;
    TGF_TCAM_CLIENTS_DB_HANDLE                      tcamClientsDbHandle;

    if (lua_gettop(L) < 9)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    rc = prvLuaTgfTcamClientsDevInit(devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, rc);
        return 1;
    }
    PARAM_NUMBER(rc, pclCfgProfileId, 2, GT_U32);

    /*udbPacketTypeGroupIndex*/
    name = lua_tolstring(L, 3, NULL/*lenPtr*/);
    rc = nameToPktTypeGroup(name, &udbPacketTypeGroupIndex);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, rc);
        return 1;
    }

    /*pclLookup*/
    name = lua_tolstring(L, 4, NULL/*lenPtr*/);
    rc = nameToLookup(name, &pclLookup);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, rc);
        return 1;
    }

    PARAM_NUMBER(rc, tcamSubkeyIndex, 5, GT_U32);
    PARAM_NUMBER(rc, ruleIndex, 6, GT_U32);
    PARAM_BOOL(rc, writeValidRule, 7);
    PARAM_NUMBER(rc, commonKeyHeaderMask, 8, GT_U32);
    PARAM_NUMBER(rc, commonKeyHeaderPattern, 9, GT_U32);

    /*subFieldConditionArr*/
    cpssOsMemSet(
        subFieldConditionArr,
        0, sizeof(subFieldConditionArr));
    if (lua_type(L, 10) != LUA_TTABLE)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    for (subfieldIdx = 0; (1); subfieldIdx++)
    {
        if (subfieldIdx >= MAX_SUBFIELDS_NUM)
        {
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
        }
        lua_rawgeti(L, 10, (subfieldIdx + 1));
        if ((lua_type(L, -1) == LUA_TNONE)
            || (lua_type(L, -1) == LUA_TNIL))
        {
            /* end of passed list */
            lua_pop(L, 1);
            break;
        }
        if (lua_type(L, -1) != LUA_TTABLE)
        {
            lua_pushinteger(L, GT_BAD_PARAM);
            return 1;
        }
        lua_rawgeti(L, -1, 1);
        name = lua_tolstring(L, -1, NULL/*lenPtr*/);
        lua_pop(L, 1);
        if (name == NULL) break;
        rc = nameToFieldId(name, &(subFieldConditionArr[subfieldIdx].subField.fieldId));
        if (rc != GT_OK)
        {
            lua_pushinteger(L, rc);
            return 1;
        }
        lua_rawgeti(L, -1, 2);
        PARAM_NUMBER(rc, subFieldConditionArr[subfieldIdx].subField.bitOffset, -1, GT_U8);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 3);
        PARAM_NUMBER(rc, subFieldConditionArr[subfieldIdx].subField.bitLength, -1, GT_U8);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 4);
        PARAM_NUMBER(rc, subFieldConditionArr[subfieldIdx].mask, -1, GT_U32);
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 5);
        PARAM_NUMBER(rc, subFieldConditionArr[subfieldIdx].pattern, -1, GT_U32);
        lua_pop(L, 1);
        lua_pop(L, 1);
    }
    /* end of array stamp */
    subFieldConditionArr[subfieldIdx].subField.fieldId =
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E;

    /*action*/
    cpssOsMemSet(&action, 0, sizeof(action));
    if (lua_type(L, 11) != LUA_TTABLE)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    lua_pushvalue(L, 11);
    prv_lua_to_c_CPSS_DXCH_PCL_ACTION_STC(L, &action);
    lua_pop(L, 1);

    tcamClientsDbHandle = prvTgfTcamClientCommomHandlersHandleGet(0);
    rc = tgfTcamClientsPclRuleSet(
        devNum, tcamClientsDbHandle,
        pclCfgProfileId, udbPacketTypeGroupIndex, pclLookup,
        tcamSubkeyIndex, ruleIndex, writeValidRule,
        &commonKeyHeaderMask, &commonKeyHeaderPattern,
        subFieldConditionArr, &action);

    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlPrvLuaTgfTcamClientsPclCfgTblSet
*
* DESCRIPTION:
*       Set PCL Configuration Table Entry Configuration.
*
* @param[in] L         -
*   IN GT_U8                            devNum,
*   IN CPSS_INTERFACE_INFO_STC          *interfaceInfoPtr,
*   IN TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup,
*   IN GT_BOOL                          enable,
*   IN GT_U32                           pclCfgProfileId,
*   IN GT_U32                           pclId
*
* RETURNS: GT_OK - OK, other - fail
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvLuaTgfTcamClientsPclCfgTblSet
(
    IN lua_State *L
)
{
    GT_STATUS                        rc;
    const char                       *name;
    GT_U8                            devNum;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup;
    GT_BOOL                          enable;
    GT_U32                           pclCfgProfileId;
    GT_U32                           pclId;

    if (lua_gettop(L) < 6)
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    PARAM_NUMBER(rc, devNum,    1, GT_U8);
    rc = prvLuaTgfTcamClientsDevInit(devNum);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, rc);
        return 1;
    }
    /*interfaceInfo*/
    lua_pushvalue(L, 2);
    prv_lua_to_c_CPSS_INTERFACE_INFO_STC(L, &interfaceInfo);
    lua_pop(L, 1);

    /*pclLookup*/
    name = lua_tolstring(L, 3, NULL/*lenPtr*/);
    rc = nameToLookup(name, &pclLookup);
    if (rc != GT_OK)
    {
        lua_pushinteger(L, rc);
        return 1;
    }
    PARAM_BOOL(rc, enable, 4);
    PARAM_NUMBER(rc, pclCfgProfileId, 5, GT_U32);
    PARAM_NUMBER(rc, pclId, 6, GT_U32);

    rc = tgfTcamClientsPclCfgTblSet(
        devNum, &interfaceInfo, pclLookup,
        enable, pclCfgProfileId, pclId);
    lua_pushinteger(L, rc);
    return 1;
}



