/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChIp.c
*
* DESCRIPTION:
*       A lua wrapper for ip address functions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 22 $
*******************************************************************************/

#include <generic/private/prvWraplGeneral.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <dxCh/wraplCpssDxChDeviceInfo.h>
#include <dxCh/wraplCpssDxChBrgFdb.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>

#define MAX_VRID 4096

/*******************************************************************************
*   Ip address wrapper definitions                                             *
*******************************************************************************/


/*******************************************************************************
*   Ip address wrapper errors                                                  *
*******************************************************************************/
extern GT_CHAR wrongIpProtocolDefinitionErrorMessage[];
extern void prv_lua_to_c_CPSS_DXCH_IP_LTT_ENTRY_STC(lua_State *L,
                                                    CPSS_DXCH_IP_LTT_ENTRY_STC *val);

GT_CHAR ipUcRouteEntryReadErrorMessage[]            =
                                            "Could not read ip unicast entry";
GT_CHAR ipUcRouteEntryFromLuaGettingErrorMessage[]  =
                                    "There is no ip unicast entry in lua stack";
GT_CHAR ipUcRouteEntryWriteErrorMessage[]           =
                                            "Could not write ip unicast entry";
GT_CHAR invalidIpUcRouteEntryErrorMessage[]         =
                                            "Next-hop entry is not initialized";
GT_CHAR ipPreffixAddingErrorMessage[]               = "Ip prefix adding error";
GT_CHAR ipPreffixDeletingErrorMessage[]             =
                                                    "Ip prefix deleting error";
GT_CHAR ipPrefixSearchingErrorMessage[]             =
                                                    "Ip prefix searching error";
GT_CHAR ipVrfCfgFromLuaGettingErrorMessage[]           =
                                            "There is no vrf config entry in lua stack";
GT_CHAR invalidIpLpmDbGetErrorMessage[]         =
                                            "Failed to get LPM DB configuration";

GT_CHAR ecmpEntryReadErrorMessage[]            =
                                            "Could not read ECMP entry";
GT_CHAR ecmpIndirectEntryReadErrorMessage[]            =
                                            "Could not read ECMP indirect entry";

/***** declarations ********/

use_prv_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC);
use_prv_struct(CPSS_DXCH_IP_ECMP_ENTRY_STC);
use_prv_print_struct(CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
use_prv_print_struct(GT_ETHERADDR);
use_prv_print_struct(GT_IPADDR);
use_prv_print_struct(GT_IPV6ADDR);
use_prv_print_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC);
use_prv_print_struct(CPSS_DXCH_IP_ECMP_ENTRY_STC);
use_prv_print_struct(CPSS_DXCH_IP_LPM_VR_CONFIG_STC);
use_prv_print_struct(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);

/***** declarations ********/


/*******************************************************************************
* prvCpssDxChIsRoutingPolicyBased
*
* DESCRIPTION:
*       Checks If Policy based routing
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       lpmDBId             - LPM DB number
*
* OUTPUTS:
*       isPolicyBasedPtr  - (pointer to)GT_TRUE - policy based routing, GT_FALSE - other
*
* RETURNS:
*       GT_OK - on sucess, other on fail
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChIsRoutingPolicyBased
(
    IN  GT_U32               lpmDBId,
    OUT GT_BOOL              *isPolicyBasedPtr
)
{
    GT_STATUS                                    status;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC tcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           memoryCfg;

    /* initialize pointers in the structure - should point to real memory */
    cpssOsMemSet(&indexesRange, 0, sizeof(indexesRange));
    cpssOsMemSet(&tcamLpmManagerCapcityCfg, 0, sizeof(tcamLpmManagerCapcityCfg));
    cpssOsMemSet(&memoryCfg, 0, sizeof(memoryCfg));
    memoryCfg.tcamDbCfg.indexesRangePtr = &indexesRange;
    memoryCfg.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;

    status = cpssDxChIpLpmDBConfigGet(
        lpmDBId, &shadowType, &protocolStack, &memoryCfg);
    if (status != GT_OK)
    {
        return status;
    }

    if (shadowType == CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E)
    {
        *isPolicyBasedPtr = GT_TRUE;
    }
    else
    {
        *isPolicyBasedPtr = GT_FALSE;
    }

    return GT_OK;
}

/*******************************************************************************
* prvCpssDxChMaximumEcmpIndex
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum ecmp index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum             - device number
*
* OUTPUTS:
*       routerEcmpndexPtr  - router ecmp index
*       errorMessagePtr    - error message
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChMaximumEcmpIndex
(
    IN  GT_U8                   devNum,
    OUT GT_U32_PTR              maximumEcmpIndexPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;
    *errorMessagePtr    = NULL;

    *maximumEcmpIndexPtr  =
                WRL_PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.ecmpQos;

    return status;
}

/*******************************************************************************
* prvCpssDxChMaximumRouterArpIndex
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum router arp index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum             - device number
*
* OUTPUTS:
*       routerArpIndexPtr  - router arp index
*       errorMessagePtr    - error message
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChMaximumRouterArpIndex
(
    IN  GT_U8                   devNum,
    OUT GT_U32_PTR              maximumRouterArpIndexPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;
    *errorMessagePtr    = NULL;

    *maximumRouterArpIndexPtr  =
                WRL_PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp;

    return status;
}


/*******************************************************************************
* prvCpssDxChMaximumRouterNextHopEntryIndex
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum router next hop entry index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number
*
* OUTPUTS:
*       maximumRouterNextHopEntryIndexPtr
*                             - router next-hop index
*       errorMessagePtr       - error message
*
* RETURNS:
*       GT_OK
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChMaximumRouterNextHopEntryIndex
(
    IN  GT_U8                   devNum,
    OUT GT_U32_PTR              maximumRouterNextHopEntryIndexPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;
    *errorMessagePtr    = NULL;

    *maximumRouterNextHopEntryIndexPtr  =
        WRL_PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;

    return status;
}


/*******************************************************************************
* pvrCpssDxChIpUnicastEntryFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of port members array from lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                    - lua state
*        L_index              - entry lua stack index
*
* OUTPUTS:
*       isIpUnicastEntryTaken - property, that ip unicast entry was in stack
*       ipUnicastEntry        - ip unicast entry
*       errorMessagePtr       - error message
*
* RETURNS:
*       GT_OK, if success otherwise error code and message
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChIpUnicastEntryFromLuaGet
(
    IN  lua_State               *L,
    IN  GT_32                   L_index,
    OUT GT_BOOL                 *isIpUnicastEntryTaken,
    OUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                                *ipUnicastEntry,
    OUT GT_CHAR_PTR             *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;

    *errorMessagePtr = NULL;

    if (0 != lua_istable(L, L_index))
    {
        lua_pushvalue(L, L_index);
        prv_lua_to_c_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC(L, ipUnicastEntry);
        lua_pop(L, 1);
        *isIpUnicastEntryTaken = GT_TRUE;
    }
    else
    {
        *isIpUnicastEntryTaken = GT_FALSE;
    }

    return status;
}


/*******************************************************************************
* pvrCpssDxChIpLttEntryFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of vrf config from lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                    - lua state
*        L_index              - entry lua stack index
*
* OUTPUTS:
*       isLttEntryTaken       - property, that ltt entry was in stack
*       lttEntry              - ltt unicast entry
*       errorMessagePtr       - error message
*
* RETURNS:
*       GT_OK, if success otherwise error code and message
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDxChIpLttEntryFromLuaGet
(
    IN  lua_State               *L,
    IN  GT_32                   L_index,
    OUT GT_BOOL                 *isLttEntryTaken,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC *lttEntry,
    OUT GT_CHAR_PTR             *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;

    *errorMessagePtr = NULL;

    if (0 != lua_istable(L, L_index))
    {
        lua_pushvalue(L, L_index);
        prv_lua_to_c_CPSS_DXCH_IP_LTT_ENTRY_STC(L, lttEntry);
        lua_pop(L, 1);
        *isLttEntryTaken = GT_TRUE;
    }
    else
    {
        *isLttEntryTaken = GT_FALSE;
    }

    return status;
}

/*******************************************************************************
* pvrCpssDxChIpUnicastEntryOnConditionPush
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of conditional pushing of ip unicast entry pushing to
*       lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status                - caller status
*       L                     - lua state
*       routeEntry            - ip unicast entry
*       condition           - pushing condition
*
* OUTPUTS:
*
* RETURNS:
*       1 if string was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
int pvrCpssDxChIpUnicastEntryOnConditionPush
(
    IN GT_STATUS                status,
    IN lua_State                *L,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                                *routeEntry,
    IN GT_BOOL                  condition
)
{
    if ((GT_OK == status) && (GT_TRUE == condition))
    {
        prv_c_to_lua_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC(L, routeEntry);

        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* pvrCpssDxChEcmpEntryOnConditionPush
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of conditional pushing of ecmp entry pushing to
*       lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status              - caller status
*       L                   - lua state
*       ecmpEntry           - ecmp entry
*       condition           - pushing condition
*
* OUTPUTS:
*
* RETURNS:
*       1 if string was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
int pvrCpssDxChEcmpEntryOnConditionPush
(
    IN GT_STATUS                    status,
    IN lua_State                    *L,
    IN CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntry,
    IN GT_BOOL                      condition
)
{
    if ((GT_OK == status) && (GT_TRUE == condition))
    {
        prv_c_to_lua_CPSS_DXCH_IP_ECMP_ENTRY_STC(L, ecmpEntry);

        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* prvCpssDxChRouterArpEntryGetAndItsValidityCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reading of router arp entry and its validity check.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number
*       routerArpIndex        - starting router arp index
*       zeroMacAddressIsValid - process zero mac address as valid
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       arpMacAddrPtr         - mac-address of router arp entry
*       errorMessagePtr       - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChRouterArpEntryGetAndItsValidityCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  routerArpEntryIndex,
    IN  GT_BOOL                 zeroMacAddressIsValid,
    OUT GT_BOOL                 *isValidRouterArpEntryPtr,
    OUT GT_ETHERADDR            *arpMacAddrPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_BOOL                     isValidArpMacAddress    = GT_TRUE;
    GT_BOOL                     isZeroArpMacAddress     = GT_FALSE;
    GT_STATUS                   status                  = GT_OK;

    status = cpssDxChIpRouterArpAddrRead(devNum, routerArpEntryIndex,
                                         arpMacAddrPtr);
    P_CALLING_API(
        cpssDxChIpRouterArpAddrRead,
        PC_NUMBER(devNum,               devNum,             GT_U8,
        PC_NUMBER(routerArpEntryIndex,  routerArpIndex,     GT_U32,
        PC_STRUCT(arpMacAddrPtr,        arpMacAddr,         GT_ETHERADDR,
        PC_LAST_PARAM))),
        PC_STATUS);

    if ((GT_OK == status) && (GT_FALSE == zeroMacAddressIsValid))
    {
        status = prvCpssIsZeroMacAddress(arpMacAddrPtr, &isZeroArpMacAddress,
                                         errorMessagePtr);
        isValidArpMacAddress    = GT_FALSE == isZeroArpMacAddress;
    }

    if ((GT_OK == status) && (GT_TRUE == isValidArpMacAddress))
    {
        *isValidRouterArpEntryPtr = GT_TRUE;
    }
    else
    {
        *isValidRouterArpEntryPtr = GT_FALSE;
    }

    return status;
}

/*******************************************************************************
* prvCpssDxChEcmpEntryGetAndItsValidityCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reading of ecmp entry and its validity check.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number
*       ecmpEntryIndex        - starting ecmp entry index
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isValidEcmpEntryPtr   - readed ecmp entry validity
*       ecmpEntryPtr     - readed ecmp entry
*       errorMessagePtr       - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChEcmpEntryGetAndItsValidityCheck
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ecmpEntryIndex,
    OUT GT_BOOL                     *isValidEcmpEntryPtr,
    OUT CPSS_DXCH_IP_ECMP_ENTRY_STC *ecmpEntryPtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                   status                  = GT_OK;

    *errorMessagePtr = NULL;

    *isValidEcmpEntryPtr = GT_TRUE;

    status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, ecmpEntryPtr);
    P_CALLING_API(
        cpssDxChIpEcmpEntryRead,
        PC_NUMBER(devNum,               devNum,             GT_U8,
        PC_NUMBER(ecmpEntryIndex,       ecmpEntryIndex,     GT_U32,
        PC_STRUCT(ecmpEntryPtr,         ecmpEntryPtr,       CPSS_DXCH_IP_ECMP_ENTRY_STC,
        PC_LAST_PARAM))),
        PC_STATUS);

    if(GT_OK != status)
    {
        *errorMessagePtr = ecmpEntryReadErrorMessage;
         *isValidEcmpEntryPtr   = GT_FALSE;
    }
    return status;
}


/*******************************************************************************
* prvCpssDxChEcmpIndirectEntryGetAndItsValidityCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reading of ecmp indirect entry and its validity check.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                 - device number
*       ecmpIndirectEntryIndex - starting ecmp indirect entry index
*       P_CALLING_FORMAL_DATA  - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isValidEcmpIndirectEntryPtr   - readed ecmp indirect entry validity
*       ecmpIndirectEntryPtr          - readed ecmp indirect entry
*       errorMessagePtr               - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChEcmpIndirectEntryGetAndItsValidityCheck
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      indirectEntryIndex,
    OUT GT_BOOL                     *isValidEcmpIndirectEntryPtr,
    OUT GT_U32                      *nextHopIndexPtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                   status                  = GT_OK;

    *errorMessagePtr = NULL;

    *isValidEcmpIndirectEntryPtr = GT_TRUE;

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
        (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
    {
          status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum, indirectEntryIndex, nextHopIndexPtr);
    }
    else
    {
        /* 1:1 */
        *nextHopIndexPtr=indirectEntryIndex;
    }

    P_CALLING_API(
        cpssDxChIpEcmpIndirectNextHopEntryGet,
        PC_NUMBER(devNum,               devNum,                 GT_U8,
        PC_NUMBER(indirectEntryIndex,   indirectEntryIndex,     GT_U32,
        PC_NUMBER(nextHopIndexPtr,      nextHopIndexPtr,        GT_U32,
        PC_LAST_PARAM))),
        PC_STATUS);

    if(GT_OK != status)
    {
        *errorMessagePtr = ecmpIndirectEntryReadErrorMessage;
         *isValidEcmpIndirectEntryPtr   = GT_FALSE;
    }
    return status;
}

/*******************************************************************************
* prvCpssDxChIpUcRouteEntryGetAndItsValidityCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reading of ip unicast route entry and its validity check.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number
*       ipUcRouteEntryIndex   - starting ip unicast route entry index
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isValidIpUcRouteEntryPtr
*                             - readed ip unicast route entry validity
*       ipUcRouteEntryPtr     - readed ip unicast route entry
*       errorMessagePtr       - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpUcRouteEntryGetAndItsValidityCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  ipUcRouteEntryIndex,
    OUT GT_BOOL                 *isValidIpUcRouteEntryPtr,
    OUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                                *ipUcRouteEntryPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                   status                  = GT_OK;

    *errorMessagePtr = NULL;

    *isValidIpUcRouteEntryPtr = GT_TRUE;

    ipUcRouteEntryPtr->type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

    status = cpssDxChIpUcRouteEntriesRead(devNum, ipUcRouteEntryIndex,
                                          ipUcRouteEntryPtr, 1);
    P_CALLING_API(
        cpssDxChIpUcRouteEntriesRead,
        PC_NUMBER(devNum,               devNum,                          GT_U8,
        PC_NUMBER(ipUcRouteEntryIndex,  baseRouteEntryIndex,            GT_U32,
        PC_STRUCT(ipUcRouteEntryPtr,    routeEntriesArray,
                                               CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC,
        PC_NUMBER(1,                    numOfRouteEntries,              GT_U32,
        PC_LAST_PARAM)))),
        PC_STATUS);

    if ((GT_OK != status) && (GT_BAD_STATE != status))
    {
        *errorMessagePtr = ipUcRouteEntryReadErrorMessage;
    }

    if (GT_BAD_STATE == status)
    {
        status                      = GT_OK;
        *isValidIpUcRouteEntryPtr   = GT_FALSE;
    }

    if ((GT_OK == status) && (GT_TRUE == *isValidIpUcRouteEntryPtr))
    {
        if (CPSS_DXCH_IP_UC_ROUTE_ENTRY_E == ipUcRouteEntryPtr->type)
        {
            if ( ( (CPSS_PACKET_CMD_ROUTE_E == ipUcRouteEntryPtr->entry.regularEntry.cmd) ||
                   (CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E == ipUcRouteEntryPtr->entry.regularEntry.cmd) ) &&
                                     (0 == ipUcRouteEntryPtr->entry.regularEntry.nextHopVlanId)  )
            {

                *isValidIpUcRouteEntryPtr   = GT_FALSE;
            }
        }
        else
        {
            *isValidIpUcRouteEntryPtr   = GT_FALSE;
        }
    }


    return status;
}


/*******************************************************************************
* prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Ip unicast route entry validity check under all devices.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                - device number
*       ipUcRouteEntryIndex   - starting ip unicast route entry index
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isValidIpUcRouteEntryPtr
*                             - readed ip unicast route entry validity
*       errorMessagePtr       - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices
(
    IN  GT_U32                  ipUcRouteEntryIndex,
    OUT GT_BOOL                 *isValidIpUcRouteEntryPtr,
    OUT GT_CHAR_PTR             *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_U8                       devNum                  = 0xFF;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                                ipUcRouteEntry;
    GT_STATUS                   status                  = GT_OK;

    *isValidIpUcRouteEntryPtr = GT_FALSE;

    while ((GT_OK == status) && (GT_FALSE == *isValidIpUcRouteEntryPtr) &&
           (GT_OK == prvWrlDevFilterNextDevGet(devNum, &devNum)))
    {
        P_CALLING_API(
            prvWrlDevFilterNextDevGet,
            PC_NUMBER(devNum,                   devNum,                  GT_U8,
            PC_LAST_PARAM),
            PC_STATUS);

        if (prvCpssIsDevExists(devNum))
        {
            status = prvCpssDxChIpUcRouteEntryGetAndItsValidityCheck(
                         devNum, ipUcRouteEntryIndex, isValidIpUcRouteEntryPtr,
                         &ipUcRouteEntry, errorMessagePtr
                         P_CALLING_ACTUAL_DATA);
        }
    }

    return status;
}


/*******************************************************************************
* prvCpssDxChIpPrefixIpUcRouteEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of ip prefix ip uc route entry index.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       vrfId                     - a Virtual Router ID
*       ipProtocolStack           - ip protocol type
*       ipAddrPtr                 - ip v4 or v6 address
*       prefixLen                 - prefix length
*       P_CALLING_FORMAL_DATA     - general debug information (environment
*                                   variables etc); could be empty
*
* OUTPUTS:
*       isIpV4PrefixExistPtr      - ip prefix existance property
*       ipUcRouteEntryIndexPtr    - ip unicast route entry index
*       errorMessagePtr           - error message
*
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChIpPrefixIpUcRouteEntryGet
(
    IN  GT_U32                      vrfId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack,
    IN  GT_IP_ADDR_TYPE_UNT         *ipAddrPtr,
    IN  GT_U32                      prefixLen,
    OUT GT_BOOL                     *isIpV4PrefixExistPtr,
    OUT GT_U32                      *ipUcRouteEntryIndexPtr,
    OUT CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT *routeTypePtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
                                    nextHopInfo;
    GT_U32                          tcamRowIndex;
    GT_U32                          tcamColumnIndex;
    GT_STATUS                       status          = GT_OK;

    *errorMessagePtr = NULL;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    switch (ipProtocolStack)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
        status = cpssDxChIpLpmIpv4UcPrefixSearch(0, vrfId, &(ipAddrPtr->ipv4Addr),
                                                 prefixLen, &nextHopInfo,
                                                 &tcamRowIndex,
                                                 &tcamColumnIndex);
        P_CALLING_API(
            cpssDxChIpLpmIpv4UcPrefixSearch,
            PC_NUMBER(0,                        lpmDBId,                GT_U32,
            PC_NUMBER(vrfId,                    vrId,                   GT_U32,
            PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,              GT_IPADDR,
            PC_NUMBER(prefixLen,                prefixLen,              GT_U32,
            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                        CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
            PC_NUMBER(tcamRowIndex,             tcamRowIndex,           GT_U32,
            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,        GT_U32,
            PC_LAST_PARAM))))))),
            PC_STATUS);
        break;

    case CPSS_IP_PROTOCOL_IPV6_E:
        status = cpssDxChIpLpmIpv6UcPrefixSearch(0, vrfId, &(ipAddrPtr->ipv6Addr),
                                                 prefixLen, &nextHopInfo,
                                                 &tcamRowIndex,
                                                 &tcamColumnIndex);
        P_CALLING_API(
            cpssDxChIpLpmIpv6UcPrefixSearch,
            PC_NUMBER(0,                        lpmDBId,                GT_U32,
            PC_NUMBER(vrfId,                    vrId,                   GT_U32,
            PC_STRUCT(&(ipAddrPtr->ipv6Addr),   ipAddr,            GT_IPV6ADDR,
            PC_NUMBER(prefixLen,                prefixLen,              GT_U32,
            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                        CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
            PC_NUMBER(tcamRowIndex,             tcamRowIndex,           GT_U32,
            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,        GT_U32,
            PC_LAST_PARAM))))))),
            PC_STATUS);
        break;

    default:
        status              = GT_BAD_PARAM;
        *errorMessagePtr    = wrongIpProtocolDefinitionErrorMessage;
        break;
    }
    if ((GT_OK != status) && (GT_NOT_FOUND != status))
    {
        *errorMessagePtr = ipPrefixSearchingErrorMessage;
    }

    if ((GT_OK == status) || (GT_NOT_FOUND == status))
    {
        *isIpV4PrefixExistPtr   = GT_NOT_FOUND != status;
        *ipUcRouteEntryIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
        *routeTypePtr = nextHopInfo.ipLttEntry.routeType;
        status                  = GT_OK;
    }

    return status;
}

/*******************************************************************************
* prvCpssDxChValidIpPrefixIpUcRouteEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of ip prefix ip uc route entry index, if ip prefix is valid.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       vrfId                     - a Virtual Router ID
*       ipProtocolStack           - ip protocol type
*       ipAddrPtr                 - ip v4 or v6 address
*       prefixLen                 - prefix length
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isIpPrefixValidPtr        - ip prefix validity property
*       ipUcRouteEntryIndexPtr    - ip unicast route entry index
*       errorMessagePtr           - error message
*
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChValidIpPrefixIpUcRouteEntryGet
(
    IN  GT_U32                      vrfId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack,
    IN  GT_IP_ADDR_TYPE_UNT         *ipAddrPtr,
    IN  GT_U8                       prefixLen,
    OUT GT_BOOL                     *isIpPrefixValidPtr,
    OUT GT_U32                      *ipUcRouteEntryIndexPtr,
    OUT CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT *routeTypePtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                       status      = GT_OK;

    status = prvCpssDxChIpPrefixIpUcRouteEntryGet(vrfId, ipProtocolStack,
                                                  ipAddrPtr, prefixLen,
                                                  isIpPrefixValidPtr,
                                                  ipUcRouteEntryIndexPtr,
                                                  routeTypePtr,
                                                  errorMessagePtr
                                                  P_CALLING_ACTUAL_DATA);
    if ((GT_OK != status) && (GT_NOT_FOUND != status))
    {
        *errorMessagePtr = ipPrefixSearchingErrorMessage;
    }

    if ((GT_OK == status) && (GT_TRUE == *isIpPrefixValidPtr))
    {
        status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                     *ipUcRouteEntryIndexPtr, isIpPrefixValidPtr,
                     errorMessagePtr P_CALLING_ACTUAL_DATA);
    }

    return status;
}


/*******************************************************************************
* prvCpssDxChNextValidIpPrefixDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next valid ip prefix its length and its ip unicats route
*       entry index.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       ipProtocolStack               - ip protocol typ
*       ipAddrPtr                     - starting ip v4 or v6 address
*                                       (recomended 0.0.0.0 for v4 or 0::0
*                                       for v6)
*       vrId                          - virtual router ID
*       prefixLenPtr                  - starting prefix length (recomended 0)
*       P_CALLING_FORMAL_DATA         - general debug information (environment
*                                       variables etc); could be empty
*
* OUTPUTS:
*       ipAddrPtr                     - founded prefix ip v4 or v6 address
*       prefixLenPtr                  - founded prefix length
*       vrId                          - founded virtual router ID related to the prefix
*       isIpPrefixFoundPtr            - ip prefix founding property
*       ipUcRouteEntryIndexPtr        - ip unicast route entry index
*       errorMessagePtr               - error message
*       isEcmpPtr                     - is route entry Multipath entry
*       baseEcmpIndexPtr              - ECMP base index
*       numEcmpPathsPtr               - ECMP base index
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/


GT_STATUS prvCpssDxChNextValidIpPrefixDataGet
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT    ipProtocolStack,
    INOUT GT_IP_ADDR_TYPE_UNT           *ipAddrPtr,
    INOUT GT_U32_PTR                    prefixLenPtr,
    OUT   GT_BOOL                       *isIpPrefixFoundPtr,
    OUT   GT_U32_PTR                    ipUcRouteEntryIndexPtr,
    OUT   GT_CHAR_PTR                   *errorMessagePtr,
    INOUT GT_U32_PTR                    vrId,
    OUT   GT_U32_PTR                    isEcmpPtr,
    OUT   GT_U32_PTR                    baseEcmpIndexPtr,
    OUT   GT_U32_PTR                    numEcmpPathsPtr
    P_CALLING_FORMAL_DATA
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
                                        nextHopInfo;
    GT_BOOL                             isIpV4PrefixChecked = GT_FALSE;
    GT_U32                              tcamRowIndex        = 0;
    GT_U32                              tcamColumnIndex     = 0;
    static GT_U32                       counter             = 0;
    GT_STATUS                           status              = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC         ecmpEntry;
    GT_U8                               devNum              = 0xFF;
    GT_U32                              ecmpEntryIndex      = 0;
    GT_U32                              i;
    GT_U32                              ipUcRouteEntryIndex = 0;
    GT_U32                              nextHopIndex=0;/* the index of the next hop in the NH table*/

    *isIpPrefixFoundPtr = GT_TRUE;
    *isEcmpPtr          = 0;

    cpssOsMemSet(&ecmpEntry, 0 , sizeof(ecmpEntry));
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    while (*vrId < (GT_U32)MAX_VRID) {
        *isIpPrefixFoundPtr = GT_TRUE;
        cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
        while ((GT_TRUE == *isIpPrefixFoundPtr) &&
               (GT_FALSE == isIpV4PrefixChecked))
        {
            switch (ipProtocolStack)
            {
            case CPSS_IP_PROTOCOL_IPV4_E:
                if ((*prefixLenPtr == 0) && (counter == 0))
                {
                    status = cpssDxChIpLpmIpv4UcPrefixSearch(0, *vrId,
                                                              &(ipAddrPtr->ipv4Addr),
                                                              *prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    P_CALLING_API(
                        cpssDxChIpLpmIpv4UcPrefixSearch,
                        PC_NUMBER(0,                        lpmDBId,            GT_U32,
                        PC_NUMBER(*vrId,                        vrId,               GT_U32,
                        PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                        PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                        PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                        PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                        PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                        PC_LAST_PARAM))))))),
                        PC_STATUS);
                    counter++;

                }
                else
                {
                    status = cpssDxChIpLpmIpv4UcPrefixGetNext(0, *vrId,
                                                              &(ipAddrPtr->ipv4Addr),
                                                              prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    P_CALLING_API(
                        cpssDxChIpLpmIpv4UcPrefixGetNext,
                        PC_NUMBER(0,                        lpmDBId,            GT_U32,
                        PC_NUMBER(*vrId,                        vrId,               GT_U32,
                        PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                        PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                        PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                        PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                        PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                        PC_LAST_PARAM))))))),
                        PC_STATUS);
                }
                break;

            case CPSS_IP_PROTOCOL_IPV6_E:
                if ((*prefixLenPtr == 0) && (counter == 0))
                {
                    status = cpssDxChIpLpmIpv6UcPrefixSearch(0, *vrId,
                                                              &(ipAddrPtr->ipv6Addr),
                                                              *prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    P_CALLING_API(
                        cpssDxChIpLpmIpv6UcPrefixSearch,
                        PC_NUMBER(0,                        lpmDBId,            GT_U32,
                        PC_NUMBER(*vrId,                        vrId,               GT_U32,
                        PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                        PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                        PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                        PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                        PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                        PC_LAST_PARAM))))))),
                        PC_STATUS);
                    counter++;

                }
                else
                {
                    status = cpssDxChIpLpmIpv6UcPrefixGetNext(0, *vrId,
                                                              &(ipAddrPtr->ipv6Addr),
                                                              prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    P_CALLING_API(
                        cpssDxChIpLpmIpv6UcPrefixGetNext,
                        PC_NUMBER(0,                        lpmDBId,            GT_U32,
                        PC_NUMBER(*vrId,                        vrId,               GT_U32,
                        PC_STRUCT(&(ipAddrPtr->ipv6Addr),   ipAddr,        GT_IPV6ADDR,
                        PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                        PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                        PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                        PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                        PC_LAST_PARAM))))))),
                        PC_STATUS);

                }
                break;

            default:
                status              = GT_BAD_PARAM;
                *errorMessagePtr    = wrongIpProtocolDefinitionErrorMessage;
                break;
            }

            if ((GT_OK == status) || (GT_NOT_FOUND == status))
            {
                *isIpPrefixFoundPtr   = GT_NOT_FOUND != status;
                *ipUcRouteEntryIndexPtr =
                    nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                status                  = GT_OK;
            }

            if ((GT_OK == status) && (GT_TRUE == *isIpPrefixFoundPtr))
            {
                if ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                    (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E))
                {

                    ecmpEntryIndex    = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    while (GT_OK == prvWrlDevFilterNextDevGet(devNum, &devNum))
                    {
                        P_CALLING_API(
                            prvWrlDevFilterNextDevGet,
                            PC_NUMBER(devNum,                   devNum,                  GT_U8,
                            PC_LAST_PARAM),
                            PC_STATUS);

                        if (prvCpssIsDevExists(devNum))
                        {
                            status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
                            if (GT_OK != status)
                            {
                                return status;
                            }
                            for (i=0; i<ecmpEntry.numOfPaths; i++)
                            {
                                if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
                                    (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
                                {
                                    /* take the ecmpEntry data and fetch with it the correct entry in the ECMP indirect table */
                                    status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum,ecmpEntry.routeEntryBaseIndex+i,&nextHopIndex);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                    ipUcRouteEntryIndex = nextHopIndex;
                                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                                    ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                                    P_CALLING_ACTUAL_DATA);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                }
                                else
                                {
                                    ipUcRouteEntryIndex = ecmpEntry.routeEntryBaseIndex + i;
                                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                                ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                                P_CALLING_ACTUAL_DATA);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                }
                            }
                            *ipUcRouteEntryIndexPtr = ecmpEntry.routeEntryBaseIndex; /* used as index to indirect table incase of sip6/sip5_25*/
                            *isEcmpPtr = 1;
                            *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                            *numEcmpPathsPtr  = ecmpEntry.numOfPaths;
                            break;
                        }
                    }
                }
                else
                {
                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                            nextHopInfo.ipLttEntry.routeEntryBaseIndex,
                                            &isIpV4PrefixChecked,
                                            errorMessagePtr
                                            P_CALLING_ACTUAL_DATA);
                }
            }
        }
        if (GT_TRUE == *isIpPrefixFoundPtr) {
            break;
        }
        else {
            *vrId = *vrId + 1;
            cpssOsMemSet(ipAddrPtr, 0, sizeof(GT_IP_ADDR_TYPE_UNT));
            counter = 0;
            *prefixLenPtr = 0;
        }
    }
    if (*vrId == MAX_VRID)
    {
        counter = 0;
    }
    return status;
}

/*******************************************************************************
* prvCpssDxChNextValidIpPrefixEcmpDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next valid ip prefix its length its ip route entry index and
*       its ecmp info.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       ipProtocolStack               - ip protocol typ
*       ipAddrPtr                     - starting ip v4 or v6 address
*                                       (recomended 0.0.0.0 for v4 or 0::0
*                                       for v6)
*       vrId                          - virtual router ID
*       prefixLenPtr                  - starting prefix length (recomended 0)
*       P_CALLING_FORMAL_DATA         - general debug information (environment
*                                       variables etc); could be empty
*
* OUTPUTS:
*       ipAddrPtr                     - founded prefix ip v4 or v6 address
*       prefixLenPtr                  - founded prefix length
*       vrId                          - founded virtual router ID related to the prefix
*       isIpPrefixFoundPtr            - ip prefix founding property
*       ipUcRouteEntryIndexPtr        - ip unicast route entry index
*       errorMessagePtr               - error message
*       isEcmpPtr                     - is route entry Multipath entry
*       baseEcmpIndexPtr              - ECMP base index
*       numEcmpPathsPtr               - ECMP base index
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/


GT_STATUS prvCpssDxChNextValidIpPrefixEcmpDataGet
(
    IN    CPSS_IP_PROTOCOL_STACK_ENT    ipProtocolStack,
    INOUT GT_IP_ADDR_TYPE_UNT           *ipAddrPtr,
    INOUT GT_U32_PTR                    prefixLenPtr,
    OUT   GT_BOOL                       *isIpPrefixFoundPtr,
    OUT   GT_U32_PTR                    ipUcRouteEntryIndexPtr,
    OUT   GT_CHAR_PTR                   *errorMessagePtr,
    INOUT GT_U32_PTR                    vrId,
    OUT   GT_U32_PTR                    isEcmpPtr,
    OUT   GT_U32_PTR                    baseEcmpIndexPtr,
    OUT   GT_U32_PTR                    numEcmpPathsPtr
    P_CALLING_FORMAL_DATA
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                              nextHopIndex=0;/* the index of the next hop in the NH table*/
    GT_BOOL                             isIpV4PrefixChecked = GT_FALSE;
    GT_U32                              tcamRowIndex        = 0;
    GT_U32                              tcamColumnIndex     = 0;
    static GT_U32                       counter             = 0;
    GT_STATUS                           status              = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC         ecmpEntry;
    GT_U8                               devNum              = 0xFF;
    GT_U32                              ecmpEntryIndex      = 0;
    GT_U32                              i;
    GT_U32                              ipUcRouteEntryIndex = 0;

    *isIpPrefixFoundPtr = GT_TRUE;
    *isEcmpPtr          = 0;

    cpssOsMemSet(&ecmpEntry, 0 , sizeof(ecmpEntry));
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    while (*vrId < (GT_U32)MAX_VRID) {
        *isIpPrefixFoundPtr = GT_TRUE;
        cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
        while ((GT_TRUE == *isIpPrefixFoundPtr) &&
               (GT_FALSE == isIpV4PrefixChecked))
        {
            switch (ipProtocolStack)
            {
            case CPSS_IP_PROTOCOL_IPV4_E:
                if ((*prefixLenPtr == 0) && (counter == 0))
                {
                    status = cpssDxChIpLpmIpv4UcPrefixSearch(0, *vrId,
                                                              &(ipAddrPtr->ipv4Addr),
                                                              *prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    /*deal only with ECMP prefixes */
                    if ((status==GT_OK)&&
                        ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                        (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E)))
                    {
                        P_CALLING_API(
                            cpssDxChIpLpmIpv4UcPrefixSearch,
                            PC_NUMBER(0,                        lpmDBId,            GT_U32,
                            PC_NUMBER(*vrId,                        vrId,               GT_U32,
                            PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                            PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                            PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                            PC_LAST_PARAM))))))),
                            PC_STATUS);
                    }
					counter++;
                }
                else
                {
                    status = cpssDxChIpLpmIpv4UcPrefixGetNext(0, *vrId,
                                                              &(ipAddrPtr->ipv4Addr),
                                                              prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    /*deal only with ECMP prefixes */
                    if ((status==GT_OK)&&
                        ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                        (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E)))
                    {
                        P_CALLING_API(
                            cpssDxChIpLpmIpv4UcPrefixGetNext,
                            PC_NUMBER(0,                        lpmDBId,            GT_U32,
                            PC_NUMBER(*vrId,                        vrId,               GT_U32,
                            PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                            PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                            PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                            PC_LAST_PARAM))))))),
                            PC_STATUS);
                    }
                }
                break;

            case CPSS_IP_PROTOCOL_IPV6_E:
                if ((*prefixLenPtr == 0) && (counter == 0))
                {
                    status = cpssDxChIpLpmIpv6UcPrefixSearch(0, *vrId,
                                                              &(ipAddrPtr->ipv6Addr),
                                                              *prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    /*deal only with ECMP prefixes */
                    if ((status==GT_OK)&&
                        ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                        (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E)))
                    {
                        P_CALLING_API(
                            cpssDxChIpLpmIpv6UcPrefixSearch,
                            PC_NUMBER(0,                        lpmDBId,            GT_U32,
                            PC_NUMBER(*vrId,                        vrId,               GT_U32,
                            PC_STRUCT(&(ipAddrPtr->ipv4Addr),   ipAddr,          GT_IPADDR,
                            PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                            PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                            PC_LAST_PARAM))))))),
                            PC_STATUS);
                    }
					counter++;
                }
                else
                {
                    status = cpssDxChIpLpmIpv6UcPrefixGetNext(0, *vrId,
                                                              &(ipAddrPtr->ipv6Addr),
                                                              prefixLenPtr,
                                                              &nextHopInfo,
                                                              &tcamRowIndex,
                                                              &tcamColumnIndex);
                    /*deal only with ECMP prefixes */
                    if ((status==GT_OK)&&
                        ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                        (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E)))
                    {
                        P_CALLING_API(
                            cpssDxChIpLpmIpv6UcPrefixGetNext,
                            PC_NUMBER(0,                        lpmDBId,            GT_U32,
                            PC_NUMBER(*vrId,                        vrId,               GT_U32,
                            PC_STRUCT(&(ipAddrPtr->ipv6Addr),   ipAddr,        GT_IPV6ADDR,
                            PC_NUMBER(*prefixLenPtr,            prefixLen,          GT_U32,
                            PC_STRUCT(&nextHopInfo,             nextHopInfo,
                                                    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                            PC_NUMBER(tcamRowIndex,             tcamRowIndex,       GT_U32,
                            PC_NUMBER(tcamColumnIndex,          tcamColumnIndex,    GT_U32,
                            PC_LAST_PARAM))))))),
                            PC_STATUS);
                    }
                }
                break;

            default:
                status              = GT_BAD_PARAM;
                *errorMessagePtr    = wrongIpProtocolDefinitionErrorMessage;
                break;
            }

            if ((GT_OK == status) || (GT_NOT_FOUND == status))
            {
                *isIpPrefixFoundPtr   = GT_NOT_FOUND != status;
                *ipUcRouteEntryIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                status                  = GT_OK;
            }

            if ((GT_OK == status) && (GT_TRUE == *isIpPrefixFoundPtr))
            {
                if ((nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) ||
                    (nextHopInfo.ipLttEntry.routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E))
                {

                    ecmpEntryIndex    = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    while (GT_OK == prvWrlDevFilterNextDevGet(devNum, &devNum))
                    {
                        P_CALLING_API(
                            prvWrlDevFilterNextDevGet,
                            PC_NUMBER(devNum,                   devNum,                  GT_U8,
                            PC_LAST_PARAM),
                            PC_STATUS);

                        if (prvCpssIsDevExists(devNum))
                        {
                            status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
                            if (GT_OK != status)
                            {
                                return status;
                            }

                            for (i=0; i<ecmpEntry.numOfPaths; i++)
                            {
                                if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
                                    (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
                                {
                                    /* take the ecmpEntry data and fetch with it the correct entry in the ECMP indirect table */
                                    status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum,ecmpEntry.routeEntryBaseIndex+i,&nextHopIndex);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                    ipUcRouteEntryIndex = nextHopIndex;
                                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                                    ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                                    P_CALLING_ACTUAL_DATA);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                }
                                else
                                {
                                    /*for SIP5 devices  ECMP table point directly to the NH table */
                                    ipUcRouteEntryIndex = ecmpEntry.routeEntryBaseIndex + i;
                                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                                ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                                P_CALLING_ACTUAL_DATA);
                                    if (GT_OK != status)
                                    {
                                        return status;
                                    }
                                }
                            }


                            *isEcmpPtr = 1;
                            *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                            *numEcmpPathsPtr  = ecmpEntry.numOfPaths;
                            *ipUcRouteEntryIndexPtr = ecmpEntry.routeEntryBaseIndex; /* used as index to indirect table incase of sip6/sip5_25*/
                            break;
                        }
                    }
                }
                else
                {/* do not show non ECMP entries
                    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                            nextHopInfo.ipLttEntry.routeEntryBaseIndex,
                                            &isIpV4PrefixChecked,
                                            errorMessagePtr
                                            P_CALLING_ACTUAL_DATA);*/
                }
            }
        }
        if (GT_TRUE == *isIpPrefixFoundPtr) {
            break;
        }
        else {
            *vrId = *vrId + 1;
            cpssOsMemSet(ipAddrPtr, 0, sizeof(GT_IP_ADDR_TYPE_UNT));
            counter = 0;
            *prefixLenPtr = 0;
        }
    }
    if (*vrId == MAX_VRID)
    {
        counter = 0;
    }
    return status;
}
/*******************************************************************************
* prvCpssDxChNextValidFdbIpPrefixDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of FDB next valid ip prefix its length its ip route entry index and
*       its ecmp info.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                         - lua state
*       L_index                   - entry lua stack index
*       ipProtocolStack           - ip protocol typ
*       ipAddrPtr                 - starting ip v4 or v6 address
*                                   (recomended 0.0.0.0 for v4 or 0::0
*                                   for v6)
*       prefixLenPtr              - starting prefix length (recomended 0)*
*       vrId                      - virtual router ID
*       searchOnlyValid           - property of searching only valid entries
*       macEntryIndexPtr          - next mac entry index
*       P_CALLING_FORMAL_DATA     - general debug information (environment
*                                   variables etc); could be empty
*
* OUTPUTS:
*       ipAddrPtr                 - founded prefix ip v4 or v6 address
*       prefixLenPtr              - founded prefix length
*       isIpPrefixFoundPtr        - ip prefix founding property
*       ipUcRouteEntryIndexPtr    - ip unicast route entry index
*       vrId                      - founded virtual router ID related to the prefix
*       errorMessagePtr           - error message
*       isEcmpPtr                 - is route entry Multipath entry
*       baseEcmpIndexPtr          - ECMP base index
*       numEcmpPathsPtr           - ECMP base index
*       macEntryIndexPtr          - next mac entry index
*       indexWasFoundPtr          - propety, that next mac entry index was found
*       entryWasFoundPtr          - propety, that next mac entry was found
*       searchedValidPtr          - searched valid property
*       searchedSkipPtr           - searched skip property
*       searchedAgedPtr           - searched aged property
*       patternMacEtnryPtr        - pattern mac entry from lua stack
*       maskMacEtnryPtr           - mask mac entry from lua stack
*       searchedMacEtnryPtr       - searched mac entry
*       errorMessagePtr           - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChNextValidFdbIpPrefixDataGet
(
    IN lua_State                        *L,
    IN  GT_32                           L_index,
    IN    GT_U8                         devNum,
    IN    CPSS_IP_PROTOCOL_STACK_ENT    ipProtocolStack,
    INOUT GT_IP_ADDR_TYPE_UNT           *ipAddrPtr,
    INOUT GT_U32_PTR                    prefixLenPtr,
    OUT   GT_BOOL                       *isIpPrefixFoundPtr,
    OUT   GT_U32_PTR                    ipUcRouteEntryIndexPtr,
    INOUT GT_U32_PTR                    vrId,
    OUT   GT_U32_PTR                    isEcmpPtr,
    OUT   GT_U32_PTR                    baseEcmpIndexPtr,
    OUT   GT_U32_PTR                    numEcmpPathsPtr,
    IN    GT_BOOL                   searchOnlyValid,
    INOUT GT_U32_PTR                macEntryIndexPtr,
    OUT   GT_BOOL                   *indexWasFoundPtr,
    OUT   GT_BOOL                   *entryWasFoundPtr,
    OUT   GT_BOOL                   *searchedValidPtr,
    OUT   GT_BOOL                   *searchedSkipPtr,
    OUT   GT_BOOL                   *searchedAgedPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *patternMacEtnryPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *maskMacEtnryPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *searchedMacEntryPtr,
    OUT   CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC     *ipUcRouteEntryPtr,
    OUT   GT_CHAR_PTR               *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                              nextHopIndex=0;/* the index of the next hop in the NH table*/
    GT_BOOL                             isIpV4PrefixChecked = GT_FALSE;
    static GT_U32                       counter             = 0;
    GT_STATUS                           status              = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC         ecmpEntry;
    GT_U32                              ecmpEntryIndex      = 0;
    GT_U32                              i;
    GT_U32                              ipUcRouteEntryIndex = 0;

    GT_BOOL                 isSearchedAgedPropertyTaken = GT_FALSE;
    GT_BOOL                 patternAged                 = GT_FALSE;
    GT_BOOL                 maskAged                    = (GT_BOOL) - 1;
    GT_BOOL                 isSearchedMacEntryTaken     = GT_FALSE;
    GT_U32                  fdbTableSize    = 0;

    searchOnlyValid = searchOnlyValid;

    *isIpPrefixFoundPtr = GT_FALSE;
    *isEcmpPtr          = 0;

    cpssOsMemSet(&ecmpEntry, 0 , sizeof(ecmpEntry));
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    cpssOsMemSet(ipUcRouteEntryPtr, 0, sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    cpssOsMemSet(searchedMacEntryPtr, 0xFF, sizeof(CPSS_MAC_ENTRY_EXT_STC));

    if (prvCpssIsDevExists(devNum))
    {
        status = prvCpssDxChBrgFdbTableSizeGet(devNum, &fdbTableSize, errorMessagePtr);
        /* if the index is bigger then the FDB table size we should finish*/
        if (*macEntryIndexPtr >= fdbTableSize)
        {
            status=GT_NO_MORE;
            *indexWasFoundPtr = GT_FALSE;
            *entryWasFoundPtr = GT_FALSE;
            return status;
        }
        status = prvCpssDxChBrgNextEntryIndexAndAgedAndMacEntryFromLuaGet
                        (L, L_index, devNum, GT_TRUE, macEntryIndexPtr,
                         indexWasFoundPtr, &isSearchedAgedPropertyTaken,
                         &patternAged, &maskAged, &isSearchedMacEntryTaken,
                         patternMacEtnryPtr, maskMacEtnryPtr, errorMessagePtr
                         P_CALLING_ACTUAL_DATA);

        if ((GT_OK == status) && (GT_TRUE == *indexWasFoundPtr))
        {
            status = prvCpssDxChBrgFdbNextEntryGet(devNum,
                                                   isSearchedAgedPropertyTaken || isSearchedMacEntryTaken,
                                                   GT_TRUE,
                                                   patternAged,
                                                   maskAged,
                                                   patternMacEtnryPtr,
                                                   maskMacEtnryPtr,
                                                   macEntryIndexPtr,
                                                   searchedValidPtr,
                                                   searchedSkipPtr,
                                                   searchedAgedPtr,
                                                   searchedMacEntryPtr,
                                                   entryWasFoundPtr,
                                                   errorMessagePtr
                                                   P_CALLING_ACTUAL_DATA);
            *indexWasFoundPtr = *entryWasFoundPtr;
        }

        /*deal only with ip uc prefixes */
        if ((*entryWasFoundPtr==GT_TRUE)&&
            (((ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)&&
             (searchedMacEntryPtr->key.entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E)) ||
             ((ipProtocolStack==CPSS_IP_PROTOCOL_IPV6_E)&&
              (searchedMacEntryPtr->key.entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E))))
        {
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
            {
                status = cpssDxChIpFdbRoutePrefixLenGet(devNum, ipProtocolStack, prefixLenPtr);
                if (status!=GT_OK)
                {
                     *indexWasFoundPtr=GT_TRUE;
                     *entryWasFoundPtr=GT_FALSE;
                     return status;
                }
            }
            else
            {
                if (ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)
                    *prefixLenPtr=32;
                else
                    *prefixLenPtr=128;

            }
            if (ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)
            {
                 *vrId = searchedMacEntryPtr->key.key.ipv4Unicast.vrfId;
                 cpssOsMemCpy(&(ipAddrPtr->ipv4Addr.arIP[0]), &(searchedMacEntryPtr->key.key.ipv4Unicast.dip[0]), sizeof(ipAddrPtr->ipv4Addr.arIP));
            }
            else /* ipv6 key*/
            {
                 *vrId = searchedMacEntryPtr->key.key.ipv6Unicast.vrfId;
                 /* only after readding the data we have the full key
                    bits 0-21 is taken from the data entry
                    bits 22-127 is taken from the key entry */
                 ipAddrPtr->ipv6Addr.arIP[2]=(searchedMacEntryPtr->key.key.ipv6Unicast.dip[2]&0x3);
                 cpssOsMemCpy(&(ipAddrPtr->ipv6Addr.arIP[3]), &(searchedMacEntryPtr->key.key.ipv6Unicast.dip[3]), 13*sizeof(GT_U8));

                /* read ipv6 data */
                if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
                {    /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
                    *macEntryIndexPtr = *macEntryIndexPtr + 1;

                    status = prvCpssDxChBrgFdbNextEntryGet(devNum,
                                                       isSearchedAgedPropertyTaken || isSearchedMacEntryTaken,
                                                       GT_TRUE,
                                                       patternAged,
                                                       maskAged,
                                                       patternMacEtnryPtr,
                                                       maskMacEtnryPtr,
                                                       macEntryIndexPtr,
                                                       searchedValidPtr,
                                                       searchedSkipPtr,
                                                       searchedAgedPtr,
                                                       searchedMacEntryPtr,
                                                       entryWasFoundPtr,
                                                       errorMessagePtr
                                                       P_CALLING_ACTUAL_DATA);
                    *indexWasFoundPtr = *entryWasFoundPtr;

                    if ((*entryWasFoundPtr!=GT_TRUE)||
                        (ipProtocolStack!=CPSS_IP_PROTOCOL_IPV6_E)||
                        (searchedMacEntryPtr->key.entryType!=CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E))
                    {
                        /* the entry is not an ipv6 entry data - should not be treated */
                        /* the index was found but it does not fit to an ipv6 data entry */
                        *indexWasFoundPtr=GT_TRUE;
                        *entryWasFoundPtr=GT_FALSE;
                    }

                    /* only after readding the data we have the full key
                    bits 0-21 is taken from the data entry
                    bits 22-127 is taken from the key entry */
                    ipAddrPtr->ipv6Addr.arIP[0]=searchedMacEntryPtr->key.key.ipv6Unicast.dip[0];
                    ipAddrPtr->ipv6Addr.arIP[1]=searchedMacEntryPtr->key.key.ipv6Unicast.dip[1];
                    ipAddrPtr->ipv6Addr.arIP[2]|=((searchedMacEntryPtr->key.key.ipv6Unicast.dip[2]>>2)&0x3F)<<2;
                }
                else
                {
                    /* ipv6 show is not supported for non sip6 - due to lack of capability to know where the data is located */
                     *indexWasFoundPtr=GT_TRUE;
                     *entryWasFoundPtr=GT_FALSE;
                }
            }


            if (*entryWasFoundPtr==GT_TRUE)
            {
                switch (searchedMacEntryPtr->fdbRoutingInfo.routingType)
                {
                case CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E:
                    nextHopInfo.ipLttEntry.routeType=CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

                     /* keep in the routeEntryBaseIndex the pointer of the arp */
                    if (searchedMacEntryPtr->fdbRoutingInfo.isTunnelStart==GT_TRUE)
                    {
                          *indexWasFoundPtr=GT_TRUE;
                          *entryWasFoundPtr=GT_FALSE;
                          break;
                    }

                    *ipUcRouteEntryIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;

                    *isEcmpPtr = 0;
                    *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    *numEcmpPathsPtr  = 0;

                    /* set ipUcRouteEntryPtr values */
                    ipUcRouteEntryPtr->type=CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
                    ipUcRouteEntryPtr->entry.regularEntry.isTunnelStart = GT_FALSE;
                    ipUcRouteEntryPtr->entry.regularEntry.nextHopVlanId = searchedMacEntryPtr->fdbRoutingInfo.nextHopVlanId;
                    ipUcRouteEntryPtr->entry.regularEntry.nextHopARPPointer = searchedMacEntryPtr->fdbRoutingInfo.nextHopARPPointer;
                    cpssOsMemCpy(&ipUcRouteEntryPtr->entry.regularEntry.nextHopInterface, &searchedMacEntryPtr->dstInterface, sizeof(CPSS_INTERFACE_INFO_STC));
                    *isIpPrefixFoundPtr = GT_TRUE;
                    counter++;
                    break;
                case CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E:
                    nextHopInfo.ipLttEntry.routeType=CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;

                    nextHopInfo.ipLttEntry.routeEntryBaseIndex = searchedMacEntryPtr->fdbRoutingInfo.multipathPointer;
                    ecmpEntryIndex = searchedMacEntryPtr->fdbRoutingInfo.multipathPointer;

                    status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
                    if (GT_OK != status)
                    {
                        return status;
                    }
                    nextHopInfo.ipLttEntry.numOfPaths = ecmpEntry.numOfPaths;

                    for (i=0; i<ecmpEntry.numOfPaths; i++)
                    {
                        if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
                            (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
                        {
                            /* take the ecmpEntry data and fetch with it the correct entry in the ECMP indirect table */
                            status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum,ecmpEntry.routeEntryBaseIndex+i,&nextHopIndex);
                            if (GT_OK != status)
                            {
                                return status;
                            }
                            ipUcRouteEntryIndex = nextHopIndex;
                            status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                        ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                        P_CALLING_ACTUAL_DATA);
                            if (GT_OK != status)
                            {
                                return status;
                            }
                        }
                        else
                        {
                            /*for SIP5 devices  ECMP table point directly to the NH table */
                            ipUcRouteEntryIndex = ecmpEntry.routeEntryBaseIndex + i;
                            status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                        ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                        P_CALLING_ACTUAL_DATA);
                            if (GT_OK != status)
                            {
                                return status;
                            }
                        }
                    }

                    *isEcmpPtr = 1;
                    *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    *numEcmpPathsPtr  = ecmpEntry.numOfPaths;
                    *ipUcRouteEntryIndexPtr = ecmpEntry.routeEntryBaseIndex; /* used as index to indirect table incase of sip6/sip5_25*/
                    *isIpPrefixFoundPtr = GT_TRUE;
                    counter++;
                    break;
                case CPSS_FDB_UC_ROUTING_TYPE_MULTICAST_NH_ENTRY_E:
                    nextHopInfo.ipLttEntry.routeType=CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
                    nextHopInfo.ipLttEntry.routeEntryBaseIndex = searchedMacEntryPtr->fdbRoutingInfo.nextHopMcPointer;
                    *isEcmpPtr = 0;
                    *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    *numEcmpPathsPtr  = 1;/* indication that we have a nextHop pointer */
                    *isIpPrefixFoundPtr = GT_TRUE;
                    *ipUcRouteEntryIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                    counter++;
                    break;
                default:
                     *indexWasFoundPtr=GT_TRUE;
                     *entryWasFoundPtr=GT_FALSE;
                }
            }
        }
        else
        {
            /* the entry is not an ip entry - should not be treated */
            /* the index was found but it does not fit to an ipv4 entry */
            *indexWasFoundPtr=GT_TRUE;
            *entryWasFoundPtr=GT_FALSE;
        }
    }
    return status;
}

/*******************************************************************************
* prvCpssDxChNextValidFdbIpPrefixEcmpDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of FDB next valid ip prefix its length its ip route entry index and
*       its ecmp info.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                         - lua state
*       L_index                   - entry lua stack index
*       ipProtocolStack           - ip protocol typ
*       ipAddrPtr                 - starting ip v4 or v6 address
*                                   (recomended 0.0.0.0 for v4 or 0::0
*                                   for v6)
*       prefixLenPtr              - starting prefix length (recomended 0)*
*       vrId                      - virtual router ID
*       searchOnlyValid           - property of searching only valid entries
*       macEntryIndexPtr          - next mac entry index
*       P_CALLING_FORMAL_DATA     - general debug information (environment
*                                   variables etc); could be empty
*
* OUTPUTS:
*       ipAddrPtr                 - founded prefix ip v4 or v6 address
*       prefixLenPtr              - founded prefix length
*       isIpPrefixFoundPtr        - ip prefix founding property
*       ipUcRouteEntryIndexPtr    - ip unicast route entry index
*       vrId                      - founded virtual router ID related to the prefix
*       errorMessagePtr           - error message
*       isEcmpPtr                 - is route entry Multipath entry
*       baseEcmpIndexPtr          - ECMP base index
*       numEcmpPathsPtr           - ECMP base index
*       macEntryIndexPtr          - next mac entry index
*       indexWasFoundPtr          - propety, that next mac entry index was found
*       entryWasFoundPtr          - propety, that next mac entry was found
*       searchedValidPtr          - searched valid property
*       searchedSkipPtr           - searched skip property
*       searchedAgedPtr           - searched aged property
*       patternMacEtnryPtr        - pattern mac entry from lua stack
*       maskMacEtnryPtr           - mask mac entry from lua stack
*       searchedMacEtnryPtr       - searched mac entry
*       errorMessagePtr           - error message
*
* RETURNS:
*       execution status
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChNextValidFdbIpPrefixEcmpDataGet
(
    IN lua_State                        *L,
    IN  GT_32                           L_index,
    IN    GT_U8                         devNum,
    IN    CPSS_IP_PROTOCOL_STACK_ENT    ipProtocolStack,
    INOUT GT_IP_ADDR_TYPE_UNT           *ipAddrPtr,
    INOUT GT_U32_PTR                    prefixLenPtr,
    OUT   GT_BOOL                       *isIpPrefixFoundPtr,
    OUT   GT_U32_PTR                    ipUcRouteEntryIndexPtr,
    INOUT GT_U32_PTR                    vrId,
    OUT   GT_U32_PTR                    isEcmpPtr,
    OUT   GT_U32_PTR                    baseEcmpIndexPtr,
    OUT   GT_U32_PTR                    numEcmpPathsPtr,
    IN    GT_BOOL                   searchOnlyValid,
    INOUT GT_U32_PTR                macEntryIndexPtr,
    OUT   GT_BOOL                   *indexWasFoundPtr,
    OUT   GT_BOOL                   *entryWasFoundPtr,
    OUT   GT_BOOL                   *searchedValidPtr,
    OUT   GT_BOOL                   *searchedSkipPtr,
    OUT   GT_BOOL                   *searchedAgedPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *patternMacEtnryPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *maskMacEtnryPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *searchedMacEntryPtr,
    OUT   GT_CHAR_PTR               *errorMessagePtr
    P_CALLING_FORMAL_DATA
)
{
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U32                              nextHopIndex=0;/* the index of the next hop in the NH table*/
    GT_BOOL                             isIpV4PrefixChecked = GT_FALSE;
    static GT_U32                       counter             = 0;
    GT_STATUS                           status              = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC         ecmpEntry;
    GT_U32                              ecmpEntryIndex      = 0;
    GT_U32                              i;
    GT_U32                              ipUcRouteEntryIndex = 0;

    GT_BOOL                 isSearchedAgedPropertyTaken = GT_FALSE;
    GT_BOOL                 patternAged                 = GT_FALSE;
    GT_BOOL                 maskAged                    = (GT_BOOL) - 1;
    GT_BOOL                 isSearchedMacEntryTaken     = GT_FALSE;
    GT_U32                  fdbTableSize    = 0;

    searchOnlyValid = searchOnlyValid;

    *isIpPrefixFoundPtr = GT_FALSE;
    *isEcmpPtr          = 0;

    cpssOsMemSet(&ecmpEntry, 0 , sizeof(ecmpEntry));
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    cpssOsMemSet(searchedMacEntryPtr, 0xFF, sizeof(CPSS_MAC_ENTRY_EXT_STC));

    if (prvCpssIsDevExists(devNum))
    {
        status = prvCpssDxChBrgFdbTableSizeGet(devNum, &fdbTableSize, errorMessagePtr);
        /* if the index is bigger then the FDB table size we should finish*/
        if (*macEntryIndexPtr >= fdbTableSize)
        {
            status=GT_NO_MORE;
            *indexWasFoundPtr = GT_FALSE;
            *entryWasFoundPtr = GT_FALSE;
            return status;
        }
        status = prvCpssDxChBrgNextEntryIndexAndAgedAndMacEntryFromLuaGet
                        (L, L_index, devNum, GT_TRUE, macEntryIndexPtr,
                         indexWasFoundPtr, &isSearchedAgedPropertyTaken,
                         &patternAged, &maskAged, &isSearchedMacEntryTaken,
                         patternMacEtnryPtr, maskMacEtnryPtr, errorMessagePtr
                         P_CALLING_ACTUAL_DATA);

        if ((GT_OK == status) && (GT_TRUE == *indexWasFoundPtr))
        {
            status = prvCpssDxChBrgFdbNextEntryGet(devNum,
                                                   isSearchedAgedPropertyTaken || isSearchedMacEntryTaken,
                                                   GT_TRUE,
                                                   patternAged,
                                                   maskAged,
                                                   patternMacEtnryPtr,
                                                   maskMacEtnryPtr,
                                                   macEntryIndexPtr,
                                                   searchedValidPtr,
                                                   searchedSkipPtr,
                                                   searchedAgedPtr,
                                                   searchedMacEntryPtr,
                                                   entryWasFoundPtr,
                                                   errorMessagePtr
                                                   P_CALLING_ACTUAL_DATA);
            *indexWasFoundPtr = *entryWasFoundPtr;
        }

        /*deal only with ip uc prefixes */
        if ((*entryWasFoundPtr==GT_TRUE)&&
            (((ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)&&
             (searchedMacEntryPtr->key.entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E)&&
             (searchedMacEntryPtr->fdbRoutingInfo.routingType==CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E)) ||    /*deal only with ECMP prefixes */
             ((ipProtocolStack==CPSS_IP_PROTOCOL_IPV6_E)&&
              (searchedMacEntryPtr->key.entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E))))
        {
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
            {
                status = cpssDxChIpFdbRoutePrefixLenGet(devNum, ipProtocolStack, prefixLenPtr);
                if (status!=GT_OK)
                {
                     *indexWasFoundPtr=GT_TRUE;
                     *entryWasFoundPtr=GT_FALSE;
                     return status;
                }
            }
            else
            {
                if (ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)
                    *prefixLenPtr=32;
                else
                    *prefixLenPtr=128;

            }
            if (ipProtocolStack==CPSS_IP_PROTOCOL_IPV4_E)
            {
                 *vrId = searchedMacEntryPtr->key.key.ipv4Unicast.vrfId;
                 cpssOsMemCpy(&(ipAddrPtr->ipv4Addr.arIP[0]), &(searchedMacEntryPtr->key.key.ipv4Unicast.dip[0]), sizeof(ipAddrPtr->ipv4Addr.arIP));
            }
            else /* ipv6 key*/
            {
                 *vrId = searchedMacEntryPtr->key.key.ipv6Unicast.vrfId;
                 /* only after readding the data we have the full key
                    bits 0-21 is taken from the data entry
                    bits 22-127 is taken from the key entry */
                 ipAddrPtr->ipv6Addr.arIP[2]=(searchedMacEntryPtr->key.key.ipv6Unicast.dip[2]&0x3);
                 cpssOsMemCpy(&(ipAddrPtr->ipv6Addr.arIP[3]), &(searchedMacEntryPtr->key.key.ipv6Unicast.dip[3]), 13*sizeof(GT_U8));

                /* read ipv6 data */
                if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
                {    /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
                    *macEntryIndexPtr = *macEntryIndexPtr + 1;

                    status = prvCpssDxChBrgFdbNextEntryGet(devNum,
                                                       isSearchedAgedPropertyTaken || isSearchedMacEntryTaken,
                                                       GT_TRUE,
                                                       patternAged,
                                                       maskAged,
                                                       patternMacEtnryPtr,
                                                       maskMacEtnryPtr,
                                                       macEntryIndexPtr,
                                                       searchedValidPtr,
                                                       searchedSkipPtr,
                                                       searchedAgedPtr,
                                                       searchedMacEntryPtr,
                                                       entryWasFoundPtr,
                                                       errorMessagePtr
                                                       P_CALLING_ACTUAL_DATA);
                    *indexWasFoundPtr = *entryWasFoundPtr;

                    if ((*entryWasFoundPtr!=GT_TRUE)||
                        (ipProtocolStack!=CPSS_IP_PROTOCOL_IPV6_E)||
                        (searchedMacEntryPtr->key.entryType!=CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E)||
                        (searchedMacEntryPtr->fdbRoutingInfo.routingType!=CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E))
                    {
                        /* the entry is not an ipv6 entry data - should not be treated */
                        /* the index was found but it does not fit to an ipv6 data entry */
                        *indexWasFoundPtr=GT_TRUE;
                        *entryWasFoundPtr=GT_FALSE;
                    }

                    /* only after readding the data we have the full key
                    bits 0-21 is taken from the data entry
                    bits 22-127 is taken from the key entry */
                    ipAddrPtr->ipv6Addr.arIP[0]=searchedMacEntryPtr->key.key.ipv6Unicast.dip[0];
                    ipAddrPtr->ipv6Addr.arIP[1]=searchedMacEntryPtr->key.key.ipv6Unicast.dip[1];
                    ipAddrPtr->ipv6Addr.arIP[2]|=((searchedMacEntryPtr->key.key.ipv6Unicast.dip[2]>>2)&0x3F)<<2;
                }
                else
                {
                    /* ipv6 show is not supported for non sip6 - due to lack of capability to know where the data is located */
                     *indexWasFoundPtr=GT_TRUE;
                     *entryWasFoundPtr=GT_FALSE;
                }
            }

            if (*entryWasFoundPtr==GT_TRUE)
            {
                *isIpPrefixFoundPtr = GT_TRUE;
                nextHopInfo.ipLttEntry.routeEntryBaseIndex = searchedMacEntryPtr->fdbRoutingInfo.multipathPointer;
                nextHopInfo.ipLttEntry.routeType=CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
                ecmpEntryIndex = searchedMacEntryPtr->fdbRoutingInfo.multipathPointer;
                counter++;

                status = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &ecmpEntry);
                if (GT_OK != status)
                {
                    return status;
                }
                nextHopInfo.ipLttEntry.numOfPaths = ecmpEntry.numOfPaths;

                for (i=0; i<ecmpEntry.numOfPaths; i++)
                {
                    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
                        (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
                    {
                        /* take the ecmpEntry data and fetch with it the correct entry in the ECMP indirect table */
                        status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum,ecmpEntry.routeEntryBaseIndex+i,&nextHopIndex);
                        if (GT_OK != status)
                        {
                            return status;
                        }

                        ipUcRouteEntryIndex = nextHopIndex;
                        status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                    ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                    P_CALLING_ACTUAL_DATA);
                        if (GT_OK != status)
                        {
                            return status;
                        }
                    }
                    else
                    {
                        /*for SIP5 devices  ECMP table point directly to the NH table */
                        ipUcRouteEntryIndex = ecmpEntry.routeEntryBaseIndex + i;
                        status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                                    ipUcRouteEntryIndex, &isIpV4PrefixChecked, errorMessagePtr
                                    P_CALLING_ACTUAL_DATA);
                        if (GT_OK != status)
                        {
                            return status;
                        }
                    }
                }

                *isEcmpPtr = 1;
                *baseEcmpIndexPtr = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
                *numEcmpPathsPtr  = ecmpEntry.numOfPaths;
                *ipUcRouteEntryIndexPtr = ecmpEntry.routeEntryBaseIndex; /* used as index to indirect table incase of sip6/sip5_25*/
            }
        }
        else
        {
            /* the entry is not an ip entry - should not be treated */
            /* the index was found but it does not fit to an ipv4 entry */
            *indexWasFoundPtr=GT_TRUE;
            *entryWasFoundPtr=GT_FALSE;
        }
    }
    return status;
}

/*******************************************************************************
* wrlCpssDxChIsRoutingPolicyBased
*
* DESCRIPTION:
*       Checks If Policy based routing.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (lpmDBId pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK and BOOLEAN isPolicyBased
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIsRoutingPolicyBased
(
    IN lua_State            *L
)
{
    GT_U32                  lpmDBId                 = (GT_U32) lua_tonumber(L, 1);
    GT_STATUS               status                  = GT_OK;
    int                     returned_param_count    = 0;
    GT_BOOL                 isPolicyBased           = GT_FALSE;

    status = prvCpssDxChIsRoutingPolicyBased(
        lpmDBId, &isPolicyBased);

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, "cpssDxChIpLpmDBConfigGet failed");
    returned_param_count    +=
        pvrCpssBooleanToLuaPush(status, L, isPolicyBased);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChMaximumRouterArpIndex
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum router arp index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       2; GT_OK and maximum router arp index are pused to lua stack if no
*       errors occurs and such port exists
*       2; error code and error message, if error occures
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChMaximumRouterArpIndex
(
    IN lua_State                *L
)
{
    GT_U8                       devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                      maximumRouterArpIndex
                                                        = 0;
    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    int                         returned_param_count    = 0;

    status = prvCpssDxChMaximumRouterArpIndex(devNum, &maximumRouterArpIndex,
                                                  &error_message);

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaPush(status, L, maximumRouterArpIndex);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChMaximumRouterNextHopEntryIndex
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of maximum router next hop entry index (wrapper).
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       2; GT_OK and maximum router arp index are pused to lua stack if no
*       errors occurs and such port exists
*       2; error code and error message, if error occures
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChMaximumRouterNextHopEntryIndex
(
    IN lua_State                *L
)
{
    GT_U8                       devNum          = (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                      maximumRouterNextHopEntryIndex
                                                = 0;
    GT_STATUS                   status          = GT_OK;
    GT_CHAR_PTR                 error_message   = NULL;
    int                         returned_param_count    = 0;

    status = prvCpssDxChMaximumRouterNextHopEntryIndex(
                 devNum, &maximumRouterNextHopEntryIndex, &error_message);

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaPush(status, L, maximumRouterNextHopEntryIndex);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChRouterArpEntryIfValidGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of router arp entry, if it is valid.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and router arp entry is pused to lua stack if no errors
*       occurs and this entry is valid
*       1; GT_OK is pused to lua stack if no errors occurs and valid router arp
*       entry is not exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChRouterArpEntryIfValidGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              routerArpEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_BOOL                             isValidRouterArpEntry   = GT_TRUE;
    GT_ETHERADDR                        arpMacAddr;
    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    P_CALLING_CHECKING;

    cpssOsMemSet(&arpMacAddr, 0, sizeof(arpMacAddr));

    status = prvCpssDxChRouterArpEntryGetAndItsValidityCheck
                 (devNum, routerArpEntryIndex, GT_FALSE, &isValidRouterArpEntry,
                  &arpMacAddr, &error_message P_CALLING_ACTUAL_DATA);

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, routerArpEntryIndex,
                                          isValidRouterArpEntry);
    returned_param_count    +=
        pvrCpssMacAddrToLuaOnConditionPush(status, L, &arpMacAddr,
                                           isValidRouterArpEntry);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChNextRouterArpEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next arp entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and next router arp entry is pused to lua stack if no errors
*       occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChNextRouterArpEntryGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              routerArpEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_U32                              maximumRouterArpIndex   = 0;
    GT_BOOL                             routerArpEntryWasFound  = GT_FALSE;
    GT_ETHERADDR                        arpMacAddr;
    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    P_CALLING_CHECKING;

    cpssOsMemSet(&arpMacAddr, 0, sizeof(arpMacAddr));

    status = prvCpssDxChMaximumRouterArpIndex(devNum, &maximumRouterArpIndex,
                                                  &error_message);

    while ((GT_OK == status) && (GT_FALSE == routerArpEntryWasFound) &&
           (routerArpEntryIndex < maximumRouterArpIndex))
    {
        status = prvCpssDxChRouterArpEntryGetAndItsValidityCheck(
                     devNum, routerArpEntryIndex++, GT_FALSE,
                     &routerArpEntryWasFound, &arpMacAddr, &error_message
                     P_CALLING_ACTUAL_DATA);
    }

    if (GT_TRUE == routerArpEntryWasFound)
    {
        routerArpEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, routerArpEntryIndex,
                                          routerArpEntryWasFound);
    returned_param_count    +=
        pvrCpssMacAddrToLuaOnConditionPush(status, L, &arpMacAddr,
                                           routerArpEntryWasFound);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChRouterArpEntryIndexGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next arp entry index with same mac address.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number, start entry index and
*                              mac-address should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and founded router arp entry index is pused to lua stack if no
*       errors occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChRouterArpEntryIndexGet
(
    IN lua_State            *L
)
{
    GT_U8                   devNum                          =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  routerArpEntryIndex             =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_U32                  maximumRouterArpIndex           = 0;
    GT_BOOL                 routerArpEntryWasFound          = GT_FALSE;
    GT_ETHERADDR            etalonMacAddr, checkedArpMacAddr;
    GT_STATUS               status                          = GT_OK;
    GT_CHAR_PTR             error_message                   = NULL;
    int                     returned_param_count            = 0;
    P_CALLING_CHECKING;

    status = pvrCpssMacAddrFromLuaGet(L, 3, &etalonMacAddr, &error_message);

    cpssOsMemSet(&checkedArpMacAddr, 0, sizeof(checkedArpMacAddr));

    if (GT_OK == status)
    {
        status = prvCpssDxChMaximumRouterArpIndex(devNum,
                                                  &maximumRouterArpIndex,
                                                  &error_message);
    }

    while ((GT_OK == status) && (GT_FALSE == routerArpEntryWasFound) &&
           (routerArpEntryIndex < maximumRouterArpIndex))
    {
        status = prvCpssDxChRouterArpEntryGetAndItsValidityCheck(
                     devNum, routerArpEntryIndex++, GT_TRUE,
                     &routerArpEntryWasFound, &checkedArpMacAddr,
                     &error_message P_CALLING_ACTUAL_DATA);

        if ((GT_OK == status) && (GT_TRUE == routerArpEntryWasFound))
        {
            status = prvCpssIsSameMacAddresses(&etalonMacAddr,
                                               &checkedArpMacAddr,
                                               &routerArpEntryWasFound,
                                               &error_message);
        }
    }

    if (GT_TRUE == routerArpEntryWasFound)
    {
        routerArpEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, routerArpEntryIndex,
                                          routerArpEntryWasFound);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChNextInvalidRouterArpEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next invalid arp entry. Valid enties could be assigned as
*       invalid on overruleUnusedInLuaArpEntries property.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and next router arp entry is pused to lua stack if no errors
*       occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChNextInvalidRouterArpEntryGet
(
    IN lua_State            *L
)
{
    GT_U8                   devNum                          =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  routerArpEntryIndex             =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
                                /* considering of unused entries as invalid */
    GT_U32                  maximumRouterArpIndex           = 0;
    GT_BOOL                 routerArpEntryValidity          = GT_TRUE;
    GT_BOOL                 invalidRouterArpEntryWasFound
                                                         = GT_FALSE;
    GT_ETHERADDR            arpMacAddr;
    GT_STATUS               status                          = GT_OK;
    GT_CHAR_PTR             error_message                   = NULL;
    int                     returned_param_count            = 0;
    P_CALLING_CHECKING;

    cpssOsMemSet(&arpMacAddr, 0, sizeof(arpMacAddr));

    status = prvCpssDxChMaximumRouterArpIndex(devNum, &maximumRouterArpIndex,
                                              &error_message);

    while ((GT_OK == status) && (GT_FALSE == invalidRouterArpEntryWasFound) &&
           (routerArpEntryIndex < maximumRouterArpIndex))
    {
        status = prvCpssDxChRouterArpEntryGetAndItsValidityCheck(
                     devNum, routerArpEntryIndex++, GT_FALSE,
                     &routerArpEntryValidity, &arpMacAddr, &error_message
                     P_CALLING_ACTUAL_DATA);

        if (GT_OK == status)
        {
            invalidRouterArpEntryWasFound = GT_FALSE == routerArpEntryValidity;
        }
    }

    if ((GT_OK == status) && (GT_TRUE == invalidRouterArpEntryWasFound))
    {
        routerArpEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, routerArpEntryIndex,
                                          invalidRouterArpEntryWasFound);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChIpUcRouteEntryRead
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Reading of ip unicast entry.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                 - lua state (ip unicast entry index pushed to lua
*                           stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and ip unicast entry is pused to lua stack if no errors
*       occurs
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpUcRouteEntryRead
(
    IN lua_State            *L
)
{
    GT_U8                   devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  routeEntryIndex         =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* route entry index */
    GT_BOOL                 routeEntryTypeTaken
                                                    = GT_FALSE;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                            routeEntry;
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    GT_U32                  tempVal;
    int                     returned_param_count    = 0;
    P_CALLING_CHECKING;

    status = pvrCpssNumberFromLuaGet(L, 3, &routeEntryTypeTaken,
                                     &tempVal,
                                     &error_message);

    if ((GT_OK == status) && (GT_FALSE == routeEntryTypeTaken))
    {
        routeEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    }

    if (GT_OK == status)
    {
        status = cpssDxChIpUcRouteEntriesRead(devNum, routeEntryIndex,
                                              &routeEntry, 1);
        P_CALLING_API(
            cpssDxChIpUcRouteEntriesRead,
            PC_NUMBER(devNum,           devNum,                         GT_U8,
            PC_NUMBER(routeEntryIndex,  baseRouteEntryIndex,           GT_U32,
            PC_STRUCT(&routeEntry,      routeEntriesArray,
                                              CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC,
            PC_NUMBER(1,                numOfRouteEntries,
                                                                       GT_U32,
            PC_LAST_PARAM)))),
            PC_STATUS);
        if (GT_OK != status)
        {
            error_message = ipUcRouteEntryReadErrorMessage;
        }
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssDxChIpUnicastEntryOnConditionPush(status, L, &routeEntry,
                                                 GT_TRUE);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChIpUcRouteEntryWrite
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Writing of ip unicast entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                - lua state (device number and route entry index
*                           pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK if operation succed
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpUcRouteEntryWrite
(
    IN lua_State            *L
)
{
    GT_U8                   devNum          = (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  routeEntryIndex = (GT_U32) lua_tonumber(L, 2);
                                                        /* route entry index */
    GT_BOOL                 isRouteEntryTaken
                                            = GT_FALSE;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
                            routeEntry;
    GT_STATUS               status          = GT_OK;
    GT_CHAR_PTR             error_message   = NULL;
    GT_U32                  tmpHWDevNum;
    P_CALLING_CHECKING;

    status = pvrCpssDxChIpUnicastEntryFromLuaGet(L, 3, &isRouteEntryTaken,
                                                 &routeEntry, &error_message);

    tmpHWDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    routeEntry.entry.regularEntry.nextHopInterface.devPort.hwDevNum = tmpHWDevNum;
    if (GT_FALSE == isRouteEntryTaken)
    {
        error_message   = ipUcRouteEntryFromLuaGettingErrorMessage;
    }

    if (GT_OK == status)
    {
        status = cpssDxChIpUcRouteEntriesWrite(devNum, routeEntryIndex,
                                               &routeEntry, 1);
        P_CALLING_API(
            cpssDxChIpUcRouteEntriesWrite,
            PC_NUMBER(devNum,           devNum,                          GT_U8,
            PC_NUMBER(routeEntryIndex,  baseRouteEntryIndex,            GT_U32,
            PC_STRUCT(&routeEntry,      routeEntriesArray,
                                               CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC,
            PC_NUMBER(1,                numOfRouteEntries,              GT_U32,
            PC_LAST_PARAM)))),
            PC_STATUS);
        if (GT_OK != status)
        {
            error_message = ipUcRouteEntryWriteErrorMessage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}


/*******************************************************************************
* wrlCpssDxChIpUcRouteEntryValidityCheck
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Ip unicast route entry validity check under all devices.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                 - lua state (ip unicast entry index pushed to lua
*                           stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and ip unicast entry validity is pused to lua stack if no
*       errors occurs
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS wrlCpssDxChIpUcRouteEntryValidityCheck
(
    IN lua_State            *L
)
{
    GT_U32                  ipUcRouteEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 1);
                                                /* ip uc route entry index */
    GT_BOOL                 isValidIpUcRouteEntry   = GT_FALSE;
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    int                     returned_param_count    = 0;
    P_CALLING_CHECKING;

    status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                 ipUcRouteEntryIndex, &isValidIpUcRouteEntry, &error_message
                 P_CALLING_ACTUAL_DATA);

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssBooleanToLuaPush(status, L, isValidIpUcRouteEntry);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChEcmpEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of ecmp entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and ecmp entry is pused to lua stack if no
*       errors occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChEcmpEntryGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              ecmpEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_U32                              maximumEcmpIndex   = 0;
    GT_BOOL                             ecmpEntryWasFound  = GT_FALSE;
    CPSS_DXCH_IP_ECMP_ENTRY_STC         ecmpEntry;
    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    P_CALLING_CHECKING;

    cpssOsMemSet(&ecmpEntry, 0, sizeof(ecmpEntry));
    status = prvCpssDxChMaximumEcmpIndex(devNum, &maximumEcmpIndex,
                                              &error_message);

    while ((GT_OK == status) && (GT_FALSE == ecmpEntryWasFound) &&
           (ecmpEntryIndex < maximumEcmpIndex))
    {
        status = prvCpssDxChEcmpEntryGetAndItsValidityCheck(
                     devNum, ecmpEntryIndex++, &ecmpEntryWasFound,
                     &ecmpEntry, &error_message P_CALLING_ACTUAL_DATA);
    }

    if (GT_TRUE == ecmpEntryWasFound)
    {
        ecmpEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ecmpEntryIndex,
                                          ecmpEntryWasFound);
    returned_param_count    +=
        pvrCpssDxChEcmpEntryOnConditionPush(status, L, &ecmpEntry,
                                                 ecmpEntryWasFound);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChEcmpIndirectIndexGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of ecmp indirect mapping.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and ecmp indirect index is pused to lua stack if no
*       errors occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChEcmpIndirectIndexGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              indirectEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_U32                              maximumIndirectIndex   = 0;
    GT_BOOL                             ecmpIndirectEntryWasFound  = GT_FALSE;
    GT_U32                              ecmpIndirectMappingIndex = 0;
    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    P_CALLING_CHECKING;

    maximumIndirectIndex = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(devNum);

    while ((GT_OK == status) && (GT_FALSE == ecmpIndirectEntryWasFound) &&
           (indirectEntryIndex < maximumIndirectIndex))
    {
        status = prvCpssDxChEcmpIndirectEntryGetAndItsValidityCheck(
                     devNum, indirectEntryIndex++, &ecmpIndirectEntryWasFound,
                     &ecmpIndirectMappingIndex, &error_message P_CALLING_ACTUAL_DATA);
    }

    if (GT_TRUE == ecmpIndirectEntryWasFound)
    {
        indirectEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, indirectEntryIndex,
                                          ecmpIndirectEntryWasFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ecmpIndirectMappingIndex,
                                                 ecmpIndirectEntryWasFound);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChNextIpUcRouteEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next arp entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and next ip unicast route entry is pused to lua stack if no
*       errors occurs and such entry exists
*       1; GT_OK is pused to lua stack if no errors occurs and such entry is not
*       exists
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChNextIpUcRouteEntryGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              ipUcRouteEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */
    GT_U32                              maximumRouterArpIndex   = 0;
    GT_BOOL                             ipUcRouteEntryWasFound  = GT_FALSE;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC     ipUcRouteEntry;
    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    P_CALLING_CHECKING;

    cpssOsMemSet(&ipUcRouteEntry, 0, sizeof(ipUcRouteEntry));

    status = prvCpssDxChMaximumRouterNextHopEntryIndex(devNum, &maximumRouterArpIndex,
                                              &error_message);

    while ((GT_OK == status) && (GT_FALSE == ipUcRouteEntryWasFound) &&
           (ipUcRouteEntryIndex < maximumRouterArpIndex))
    {
        status = prvCpssDxChIpUcRouteEntryGetAndItsValidityCheck(
                     devNum, ipUcRouteEntryIndex++, &ipUcRouteEntryWasFound,
                     &ipUcRouteEntry, &error_message P_CALLING_ACTUAL_DATA);
    }

    if (GT_TRUE == ipUcRouteEntryWasFound)
    {
        ipUcRouteEntryIndex--;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          ipUcRouteEntryWasFound);
    returned_param_count    +=
        pvrCpssDxChIpUnicastEntryOnConditionPush(status, L, &ipUcRouteEntry,
                                                 ipUcRouteEntryWasFound);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChIpLpmIpUcPrefixExist
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Checks if ip v4 or v6 prefix exists.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (vrfId, ip protocol, ip address and
*                              its length pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and ip unicast entry validity is pused to lua stack if no
*       errors occurs
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpLpmIpUcPrefixExist
(
    IN lua_State            *L
)
{
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT     ipAddress;
    GT_U32                  vrfId               = (GT_U32) lua_tonumber(L, 1);
    GT_U32                  prefixLen           = (GT_U8)  lua_tonumber(L, 4);
                                                           /* prefix length */
    GT_BOOL                 isIpV4PrefixExist       = GT_FALSE;
    GT_U32                  ipUcRouteEntryIndex     = 0;
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT             routeType=CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    GT_CHAR_PTR             routeTypeStr            = NULL;
    int                     returned_param_count    = 0;
    P_CALLING_CHECKING;

    status = pvrCpssIpProtocolFromLuaGet(L, 2, &ipProtocolStack,
                                         &error_message);
    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 3, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        status = prvCpssDxChIpPrefixIpUcRouteEntryGet(vrfId,
                                                      ipProtocolStack,
                                                      &ipAddress, prefixLen,
                                                      &isIpV4PrefixExist,
                                                      &ipUcRouteEntryIndex,
                                                      &routeType,
                                                      &error_message
                                                      P_CALLING_ACTUAL_DATA);
    }

    switch(routeType){
    case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
        routeTypeStr = "ECMP";
        break;
    case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
        routeTypeStr = "QOS";
        break;
    case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E:
        routeTypeStr = "REGULAR";
        break;
    default:
        break;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssBooleanToLuaPush(status, L, isIpV4PrefixExist);
    returned_param_count    +=
        pvrCpssStringToLuaOnConditionPush(status, L, routeTypeStr,
                                          isIpV4PrefixExist);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          isIpV4PrefixExist);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChNextValidIpPrefixDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next valid ip v4 or v6 prefix its length and its ip unicats
*       route entry index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (ip protocol, prefix ip address and
*                              prefix length should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and prefix ip address, prefix length, prefix next-hop entry
*       index
*       1; GT_OK if next valid ip prefix not found and no error occures
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChNextValidIpPrefixDataGet
(
    IN lua_State            *L
)
{
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack         =
                                                        CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      prefixLen               =
                                                    (GT_U32) lua_tonumber(L, 3);
                                                            /* prefix length */
    GT_BOOL                     isIpV4PrefixFound       = GT_FALSE;
    GT_U32                      ipUcRouteEntryIndex     = 0;
    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    GT_U32                      vrId                    = (GT_U32) lua_tonumber(L, 4);
    int                         returned_param_count    = 0;
    GT_U32                      isEcmp                  = 0;
    GT_U32                      baseEcmpIndex           = 0;
    GT_U32                      numEcmpPaths            = 0;
    P_CALLING_CHECKING;

    status = pvrCpssIpProtocolFromLuaGet(L, 1, &ipProtocolStack,
                                         &error_message);

    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 2, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        status = prvCpssDxChNextValidIpPrefixDataGet(ipProtocolStack, &ipAddress,
                                                     &prefixLen,
                                                     &isIpV4PrefixFound,
                                                     &ipUcRouteEntryIndex,
                                                     &error_message,
                                                     &vrId,
                                                     &isEcmp,
                                                     &baseEcmpIndex,
                                                     &numEcmpPaths
                                                     P_CALLING_ACTUAL_DATA);
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);

    returned_param_count    +=
        pvrCpssIpAddrToLuaOnConditionPush(status, L, ipProtocolStack,
                                          &ipAddress, isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, prefixLen,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, vrId,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, isEcmp,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, baseEcmpIndex,
                                          isIpV4PrefixFound);

    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, numEcmpPaths,
                                          isIpV4PrefixFound);
    return returned_param_count;
}

/*******************************************************************************
* wrlCpssDxChNextValidIpPrefixEcmpDataGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next valid ip v4 or v6 prefix its length its ip route entry index
*       and its ecmp info.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (ip protocol, prefix ip address and
*                              prefix length should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       2; GT_OK and prefix ip address, prefix length, prefix next-hop entry
*       index and ecmp info
*       1; GT_OK if next valid ip prefix not found and no error occures
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChNextValidIpPrefixEcmpDataGet
(
    IN lua_State            *L
)
{
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack         =
                                                        CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      prefixLen               =
                                                    (GT_U32) lua_tonumber(L, 3);
                                                            /* prefix length */
    GT_BOOL                     isIpV4PrefixFound       = GT_FALSE;
    GT_U32                      ipUcRouteEntryIndex     = 0;
    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    GT_U32                      vrId                    = (GT_U32) lua_tonumber(L, 4);
    int                         returned_param_count    = 0;
    GT_U32                      isEcmp                  = 0;
    GT_U32                      baseEcmpIndex           = 0;
    GT_U32                      numEcmpPaths            = 0;
    P_CALLING_CHECKING;

    status = pvrCpssIpProtocolFromLuaGet(L, 1, &ipProtocolStack,
                                         &error_message);

    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 2, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        status = prvCpssDxChNextValidIpPrefixEcmpDataGet(ipProtocolStack, &ipAddress,
                                                     &prefixLen,
                                                     &isIpV4PrefixFound,
                                                     &ipUcRouteEntryIndex,
                                                     &error_message,
                                                     &vrId,
                                                     &isEcmp,
                                                     &baseEcmpIndex,
                                                     &numEcmpPaths
                                                     P_CALLING_ACTUAL_DATA);
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);

    returned_param_count    +=
        pvrCpssIpAddrToLuaOnConditionPush(status, L, ipProtocolStack,
                                          &ipAddress, isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, prefixLen,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, vrId,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, isEcmp,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, baseEcmpIndex,
                                          isIpV4PrefixFound);

    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, numEcmpPaths,
                                          isIpV4PrefixFound);
    return returned_param_count;
}

/*******************************************************************************
* prvCpssDxChBrgFdbIpNextEntryAndItsIndexPushToLua
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next ip fdb entry and its index with getting data from lua,
*       pushing result to lua stack.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                   - lua state (in stack should be device number,
*                             starting index, optional parrenrs and masks)
*       searchOnlyValid     - property of searching only valid entries
*       P_CALLING_FORMAL_DATA
*                           - general debug information (environment variables
*                             etc); could be empty
*
* OUTPUTS:
*       status and FDB entry index, if exists, to lua stack
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int prvCpssDxChBrgFdbIpNextEntryAndItsIndexPushToLua
(
    IN  lua_State           *L,
    IN GT_BOOL              searchOnlyValid
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    int                     returned_param_count    = 0;

    GT_U8                   devNum                  =(GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  index                   =(GT_U32) lua_tonumber(L, 2);
                                                                    /* index */
    GT_U32                  prefixLen;
    GT_U32                  vrId;
    CPSS_IP_PROTOCOL_STACK_ENT   ipProtocolStack      = CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_BOOL                     isIpV4PrefixFound       = GT_FALSE;
    GT_U32                      ipUcRouteEntryIndex     = 0;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC     ipUcRouteEntry;
    GT_U32                      isEcmp                  = 0;
    GT_U32                      baseEcmpIndex           = 0;
    GT_U32                      numEcmpPaths            = 0;

    GT_BOOL                 indexWasFound           = GT_FALSE;
    GT_BOOL                 searchedValid           = GT_FALSE;
    GT_BOOL                 searchedSkip            = GT_TRUE;
    GT_BOOL                 searchedAged            = GT_FALSE;
    CPSS_MAC_ENTRY_EXT_STC  patternMacEtnry, maskMacEtnry;
    CPSS_MAC_ENTRY_EXT_STC  searchedMacEntry;
    GT_BOOL                 entryWasFound           = GT_FALSE;

    status = pvrCpssIpProtocolFromLuaGet(L, 3, &ipProtocolStack, &error_message);

    cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));

    if (GT_OK == status)
    {
       status = prvCpssDxChNextValidFdbIpPrefixDataGet(L,4,devNum,
                                                            ipProtocolStack,
                                                            &ipAddress,
                                                            &prefixLen,
                                                            &isIpV4PrefixFound,
                                                            &ipUcRouteEntryIndex,
                                                            &vrId,
                                                            &isEcmp,
                                                            &baseEcmpIndex,
                                                            &numEcmpPaths,
                                                            searchOnlyValid,
                                                            &index,
                                                            &indexWasFound,
                                                            &entryWasFound,
                                                            &searchedValid,
                                                            &searchedSkip,
                                                            &searchedAged,
                                                            &patternMacEtnry,
                                                            &maskMacEtnry,
                                                            &searchedMacEntry,
                                                            &ipUcRouteEntry,
                                                            &error_message
                                                            P_CALLING_ACTUAL_DATA);
    }
    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, index, indexWasFound);
    returned_param_count    +=
        pvrCpssValidSkipAgedToLuaOnConditionPush(status, L, searchedValid,
                                                 searchedSkip, searchedAged,
                                                 entryWasFound);
    returned_param_count    +=
        pvrCpssMacEntryToLuaOnConditionPush(status, L, &searchedMacEntry,
                                            entryWasFound);

    returned_param_count    +=
        pvrCpssIpAddrToLuaOnConditionPush(status, L, ipProtocolStack,
                                          &ipAddress, isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, prefixLen,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, vrId,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, isEcmp,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, baseEcmpIndex,
                                          isIpV4PrefixFound);

    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, numEcmpPaths,
                                          isIpV4PrefixFound);

    returned_param_count    +=
        pvrCpssDxChIpUnicastEntryOnConditionPush(status, L, &ipUcRouteEntry,
                                          isIpV4PrefixFound);
    return returned_param_count;
}

/*******************************************************************************
* prvCpssDxChBrgFdbIpEcmpNextEntryAndItsIndexPushToLua
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next ip ecmp fdb entry and its index with getting data from lua,
*       pushing result to lua stack.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                   - lua state (in stack should be device number,
*                             starting index, optional parrenrs and masks)
*       searchOnlyValid     - property of searching only valid entries
*       P_CALLING_FORMAL_DATA
*                           - general debug information (environment variables
*                             etc); could be empty
*
* OUTPUTS:
*       status and FDB entry index, if exists, to lua stack
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int prvCpssDxChBrgFdbIpEcmpNextEntryAndItsIndexPushToLua
(
    IN  lua_State           *L,
    IN GT_BOOL              searchOnlyValid
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS               status                  = GT_OK;
    GT_CHAR_PTR             error_message           = NULL;
    int                     returned_param_count    = 0;

    GT_U8                   devNum                  =(GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                  index                   =(GT_U32) lua_tonumber(L, 2);
                                                                    /* index */
    GT_U32                  prefixLen;
    GT_U32                  vrId;
    CPSS_IP_PROTOCOL_STACK_ENT   ipProtocolStack      = CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_BOOL                     isIpV4PrefixFound       = GT_FALSE;
    GT_U32                      ipUcRouteEntryIndex     = 0;
    GT_U32                      isEcmp                  = 0;
    GT_U32                      baseEcmpIndex           = 0;
    GT_U32                      numEcmpPaths            = 0;

    GT_BOOL                 indexWasFound           = GT_FALSE;
    GT_BOOL                 searchedValid           = GT_FALSE;
    GT_BOOL                 searchedSkip            = GT_TRUE;
    GT_BOOL                 searchedAged            = GT_FALSE;
    CPSS_MAC_ENTRY_EXT_STC  patternMacEtnry, maskMacEtnry;
    CPSS_MAC_ENTRY_EXT_STC  searchedMacEntry;
    GT_BOOL                 entryWasFound           = GT_FALSE;

    status = pvrCpssIpProtocolFromLuaGet(L, 3, &ipProtocolStack, &error_message);

    cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));

    if (GT_OK == status)
    {
       status = prvCpssDxChNextValidFdbIpPrefixEcmpDataGet(L,4,devNum,
                                                            ipProtocolStack,
                                                            &ipAddress,
                                                            &prefixLen,
                                                            &isIpV4PrefixFound,
                                                            &ipUcRouteEntryIndex,
                                                            &vrId,
                                                            &isEcmp,
                                                            &baseEcmpIndex,
                                                            &numEcmpPaths,
                                                            searchOnlyValid,
                                                            &index,
                                                            &indexWasFound,
                                                            &entryWasFound,
                                                            &searchedValid,
                                                            &searchedSkip,
                                                            &searchedAged,
                                                            &patternMacEtnry,
                                                            &maskMacEtnry,
                                                            &searchedMacEntry,
                                                            &error_message
                                                            P_CALLING_ACTUAL_DATA);
    }
    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, index, indexWasFound);
    returned_param_count    +=
        pvrCpssValidSkipAgedToLuaOnConditionPush(status, L, searchedValid,
                                                 searchedSkip, searchedAged,
                                                 entryWasFound);
    returned_param_count    +=
        pvrCpssMacEntryToLuaOnConditionPush(status, L, &searchedMacEntry,
                                            entryWasFound);

    returned_param_count    +=
        pvrCpssIpAddrToLuaOnConditionPush(status, L, ipProtocolStack,
                                          &ipAddress, isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, prefixLen,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, ipUcRouteEntryIndex,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, vrId,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, isEcmp,
                                          isIpV4PrefixFound);
    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, baseEcmpIndex,
                                          isIpV4PrefixFound);

    returned_param_count    +=
        pvrCpssNumberToLuaOnConditionPush(status, L, numEcmpPaths,
                                          isIpV4PrefixFound);
    return returned_param_count;
}


/*******************************************************************************
* wrlCpssDxChBrgFdbIpNextValidEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next fdb valid entry index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state (in stack should be device number,
*                             starting index, optional parrenrs and masks)
*
* OUTPUTS:
*       number of returned to lua elements, status FDB entry index,
*       valid/skip/aged struct and mac-entry to lua stack, if exists,
*       otherwise 0
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChBrgFdbIpNextValidEntryGet
(
    IN lua_State            *L
)
{
    P_CALLING_CHECKING;

    return prvCpssDxChBrgFdbIpNextEntryAndItsIndexPushToLua(
               L, GT_TRUE P_CALLING_ACTUAL_DATA);
}

/*******************************************************************************
* wrlCpssDxChBrgFdbIpEcmpNextValidEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next ip ecmp fdb valid entry index.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state (in stack should be device number,
*                             starting index, optional parrenrs and masks)
*
* OUTPUTS:
*       number of returned to lua elements, status FDB entry index,
*       valid/skip/aged struct and mac-entry to lua stack, if exists,
*       otherwise 0
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChBrgFdbIpEcmpNextValidEntryGet
(
    IN lua_State            *L
)
{
    P_CALLING_CHECKING;

    return prvCpssDxChBrgFdbIpEcmpNextEntryAndItsIndexPushToLua(
               L, GT_TRUE P_CALLING_ACTUAL_DATA);
}

/**
* @internal wrlCpssDxChIpLpmVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual's router ID.(APPLICABLE RANGES: 0..4095)
* @param[in] vrConfigPtr              - (pointer to) Virtual router configuration.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PARAM             - if wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_BAD_PTR               - if illegal pointer value
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note Refer to cpssDxChIpLpmVirtualRouterSharedAdd for limitation when shared
*       virtual router is used.
*
*/

int wrlCpssDxChIpLpmVirtualRouterAdd
(
    IN lua_State                *L
)
{
    GT_U32                      lpmDBId;
    GT_U32                      vrfId ;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC        vrConfig;
    GT_BOOL                               isLttEntryTaken  = GT_FALSE;

    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;

    cpssOsMemSet(&vrConfig, 0, sizeof(vrConfig));
    lpmDBId = (GT_U32) lua_tonumber(L, 1);
    vrfId = (GT_U32) lua_tonumber(L, 2);
    vrConfig.supportIpv4Uc = (GT_BOOL) lua_toboolean(L, 3);

    status = pvrCpssDxChIpLttEntryFromLuaGet(L, 4, &isLttEntryTaken,
                                                 &vrConfig.defIpv4UcNextHopInfo.ipLttEntry, &error_message);


    if (GT_FALSE == isLttEntryTaken)
    {
        error_message   = ipVrfCfgFromLuaGettingErrorMessage;
    }
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
        return prvWrlCpssStatusToLuaPush(status, L, error_message);
    }
    vrConfig.supportIpv4Mc = (GT_BOOL) lua_toboolean(L, 5);
    status = pvrCpssDxChIpLttEntryFromLuaGet(L, 6, &isLttEntryTaken,
                                                 &vrConfig.defIpv4McRouteLttEntry, &error_message);


    if (GT_FALSE == isLttEntryTaken)
    {
        error_message   = ipVrfCfgFromLuaGettingErrorMessage;
    }
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
        return prvWrlCpssStatusToLuaPush(status, L, error_message);
    }
    vrConfig.supportIpv6Uc = (GT_BOOL) lua_toboolean(L, 7);
    status = pvrCpssDxChIpLttEntryFromLuaGet(L, 8, &isLttEntryTaken,
                                                 &vrConfig.defIpv6UcNextHopInfo.ipLttEntry, &error_message);


    if (GT_FALSE == isLttEntryTaken)
    {
        error_message   = ipVrfCfgFromLuaGettingErrorMessage;
    }
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
        return prvWrlCpssStatusToLuaPush(status, L, error_message);
    }
    vrConfig.supportIpv6Mc = (GT_BOOL) lua_toboolean(L, 9);
    status = pvrCpssDxChIpLttEntryFromLuaGet(L, 10, &isLttEntryTaken,
                                                 &vrConfig.defIpv6McRouteLttEntry, &error_message);


    if (GT_FALSE == isLttEntryTaken)
    {
        error_message   = ipVrfCfgFromLuaGettingErrorMessage;
    }
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
        return prvWrlCpssStatusToLuaPush(status, L, error_message);
    }
    vrConfig.supportFcoe = (GT_BOOL) lua_toboolean(L, 11);
    status = pvrCpssDxChIpLttEntryFromLuaGet(L, 12, &isLttEntryTaken,
                                                 &vrConfig.defaultFcoeForwardingNextHopInfo.ipLttEntry, &error_message);


    if (GT_FALSE == isLttEntryTaken)
    {
        error_message   = ipVrfCfgFromLuaGettingErrorMessage;
    }
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
        return prvWrlCpssStatusToLuaPush(status, L, error_message);
    }
    status = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrfId, &vrConfig);
    if (GT_OK != status)
    {
        error_message = ipVrfCfgFromLuaGettingErrorMessage;
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}




/*******************************************************************************
* wrlCpssDxChIpLpmIpUcPrefixAdd
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Creates a new or override an existing ip v4 or v6 prefix in specified
*       Virtual Router for the specified LPM DB.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (vrf-id, ip protocol, prefix ip address
*                              and prefix length should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK if operation succed
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpLpmIpUcPrefixAdd
(
    IN lua_State                *L
)
{
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack         =
                                                        CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      vrfId                   =
                                                    (GT_U32) lua_tonumber(L, 1);
    GT_U32                      prefixLen               =
                                                    (GT_U8)  lua_tonumber(L, 4);
                                                            /* prefix length */
    GT_BOOL                     isValidIpUcRouteEntry   = GT_TRUE;
    GT_U32                      ipUcRouteEntryIndex     =
                                                   (GT_U32)  lua_tonumber(L, 5);
                                                            /* prefix length */
    GT_BOOL                     isEArch              =
                                                    (GT_BOOL) lua_toboolean(L,6);
                                                        /* true if the device is bc2 */
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
                                nextHopEntry;
    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    P_CALLING_CHECKING;

    cpssOsMemSet(&nextHopEntry, 0, sizeof(nextHopEntry));

    status = pvrCpssIpProtocolFromLuaGet(L, 2, &ipProtocolStack,
                                         &error_message);

    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 3, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                     ipUcRouteEntryIndex, &isValidIpUcRouteEntry,
                     &error_message P_CALLING_ACTUAL_DATA);
        if ((GT_OK == status) && (GT_FALSE == isValidIpUcRouteEntry))
        {
            status          = GT_BAD_PARAM;
            error_message = invalidIpUcRouteEntryErrorMessage;
        }
    }

    /* keep all value of ucNextHop.ipLttEntry = 0 set the rout index        */
    nextHopEntry.ipLttEntry.routeEntryBaseIndex = ipUcRouteEntryIndex;
    if (isEArch == GT_TRUE) {
        nextHopEntry.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }


    if (GT_OK == status)
    {
        switch (ipProtocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            status = cpssDxChIpLpmIpv4UcPrefixAdd(0, vrfId, &(ipAddress.ipv4Addr),
                                                  prefixLen, &nextHopEntry,
                                                  GT_TRUE, GT_FALSE);
            P_CALLING_API(
                cpssDxChIpLpmIpv4UcPrefixAdd,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv4Addr),    ipAddr,          GT_IPADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_STRUCT(&nextHopEntry,            nextHopInfo,
                                        CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                PC_BOOL  (GT_TRUE,                  override,
                PC_BOOL  (GT_TRUE,                  defragmentationEnable,
                PC_LAST_PARAM))))))),
                PC_STATUS);
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            status = cpssDxChIpLpmIpv6UcPrefixAdd(0, vrfId, &(ipAddress.ipv6Addr),
                                                  prefixLen, &nextHopEntry,
                                                  GT_TRUE, GT_TRUE);
            P_CALLING_API(
                cpssDxChIpLpmIpv6UcPrefixAdd,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv6Addr),    ipAddr,        GT_IPV6ADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_STRUCT(&nextHopEntry,            nextHopInfo,
                                        CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                PC_BOOL  (GT_TRUE,                  override,
                PC_BOOL  (GT_TRUE,                  defragmentationEnable,
                PC_LAST_PARAM))))))),
                PC_STATUS);
            break;

        default:
            status              = GT_BAD_PARAM;
            error_message       = wrongIpProtocolDefinitionErrorMessage;
            break;
        }

        if (GT_OK != status)
        {
            error_message = ipPreffixAddingErrorMessage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*******************************************************************************
* wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Creates a new or override an existing ip v4 or v6 prefix in specified
*       Virtual Router for the specified LPM DB.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state
*                              1. vrf-id,
*                              2. ip protocol,
*                              3. prefix ip address
*                              4. prefix length
*                              5. ecmpIpEntryIndex
*                              6. ipUcRouteEntryIndex
*                              7. numOfPaths
*                              8. randomEnable
*                              should be pushed to lua stack
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK if operation succed
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange
(
    IN lua_State                *L
)
{
    GT_U8                       devNum = 0;
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack         =
                                                    CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      vrfId                   =
                                                    (GT_U32) lua_tonumber(L, 1);
                                /* below 2. ip protocol, 3. prefix ip address */

    GT_U32                      prefixLen               =
                                                    (GT_U8)  lua_tonumber(L, 4);
                                                            /* prefix length */
    GT_BOOL                     isValidIpUcRouteEntry   = GT_TRUE;
    GT_U32                      ecmpIpEntryIndex        =
                                                    (GT_U32)  lua_tonumber(L, 5);
    GT_U32                      ipUcRouteEntryIndex     =
                                                    (GT_U32)  lua_tonumber(L, 6);
                                                            /* prefix length */
    GT_U32                      numOfPaths              =
                                                    (GT_U32)  lua_tonumber(L,7);
                                                        /* num of Next Hops */
    GT_BOOL                     randomEnable            =
                                                    (GT_BOOL) lua_toboolean(L, 8);

    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopEntry;
    CPSS_DXCH_IP_ECMP_ENTRY_STC            ecmpEntry;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT    shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT          protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT  memoryCfg;

    GT_STATUS                   status                  = GT_OK;
    GT_CHAR_PTR                 error_message           = NULL;
    P_CALLING_CHECKING;

    cpssOsMemSet(&nextHopEntry, 0, sizeof(nextHopEntry));
    cpssOsMemSet(&ecmpEntry, 0, sizeof(ecmpEntry));

    status = pvrCpssIpProtocolFromLuaGet(
        L, 2, &ipProtocolStack, &error_message);
    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 3, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        status = prvCpssDxChIpUcRouteEntryValidityCheckOnAllDevices(
                     ipUcRouteEntryIndex, &isValidIpUcRouteEntry,
                     &error_message P_CALLING_ACTUAL_DATA);
        if ((GT_OK == status) && (GT_FALSE == isValidIpUcRouteEntry))
        {
            status          = GT_BAD_PARAM;
            error_message = invalidIpUcRouteEntryErrorMessage;
        }
    }

    /* keep all value of ucNextHop.ipLttEntry = 0 set the rout index        */
    nextHopEntry.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;

    status = cpssDxChIpLpmDBConfigGet(0,&shadowType, &protocolStack, &memoryCfg);
    if (GT_OK != status)
    {
        error_message = invalidIpLpmDbGetErrorMessage;
    }
    else
    {
        if((shadowType==CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)||
           (shadowType==CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E))
        {
            nextHopEntry.ipLttEntry.routeType   = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
        }
    }

    nextHopEntry.ipLttEntry.numOfPaths          = numOfPaths;
    nextHopEntry.ipLttEntry.routeEntryBaseIndex = ecmpIpEntryIndex;

    ecmpEntry.routeEntryBaseIndex = ipUcRouteEntryIndex;
    ecmpEntry.numOfPaths          = numOfPaths;
    ecmpEntry.randomEnable        = randomEnable;

    if (GT_OK == status)
    {
        devNum = 0xFF;
        while (GT_OK == prvWrlDevFilterNextDevGet(devNum, &devNum))
        {
            status = cpssDxChIpEcmpEntryWrite(
                devNum, ecmpIpEntryIndex, &ecmpEntry);
        }

        switch (ipProtocolStack)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                status = cpssDxChIpLpmIpv4UcPrefixAdd(
                    0, vrfId, &(ipAddress.ipv4Addr),
                    prefixLen, &nextHopEntry,
                    GT_TRUE,GT_FALSE);
                P_CALLING_API(
                    cpssDxChIpLpmIpv4UcPrefixAdd,
                    PC_NUMBER(0,                        lpmDBId,            GT_U32,
                    PC_NUMBER(vrfId,                    vrId,               GT_U32,
                    PC_STRUCT(&(ipAddress.ipv4Addr),    ipAddr,          GT_IPADDR,
                    PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                    PC_STRUCT(&nextHopEntry,            nextHopInfo,
                                            CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                    PC_BOOL  (GT_TRUE,                  override,
                    PC_BOOL  (GT_TRUE,                  defragmentationEnable,
                    PC_LAST_PARAM))))))),
                    PC_STATUS);
                break;

            case CPSS_IP_PROTOCOL_IPV6_E:
                status = cpssDxChIpLpmIpv6UcPrefixAdd(
                    0, vrfId, &(ipAddress.ipv6Addr),
                    prefixLen, &nextHopEntry,
                    GT_TRUE, GT_TRUE);
                P_CALLING_API(
                    cpssDxChIpLpmIpv6UcPrefixAdd,
                    PC_NUMBER(0,                        lpmDBId,            GT_U32,
                    PC_NUMBER(vrfId,                    vrId,               GT_U32,
                    PC_STRUCT(&(ipAddress.ipv6Addr),    ipAddr,        GT_IPV6ADDR,
                    PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                    PC_STRUCT(&nextHopEntry,            nextHopInfo,
                                            CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT,
                    PC_BOOL  (GT_TRUE,                  override,
                    PC_BOOL  (GT_TRUE,                  defragmentationEnable,
                    PC_LAST_PARAM))))))),
                    PC_STATUS);
                break;

            default:
                status              = GT_BAD_PARAM;
                error_message       = wrongIpProtocolDefinitionErrorMessage;
                break;
        }

        if (GT_OK != status)
        {
            error_message = ipPreffixAddingErrorMessage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*******************************************************************************
* wrlCpssDxChIpLpmIpUcPrefixDel
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Deletes an existing Ipv6 prefix in a Virtual Router for the specified
*       LPM DB.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (vrfId, ip protocol, prefix ip address
*                              and prefix length should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK if operation succed
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpLpmIpUcPrefixDel
(
    IN lua_State                *L
)
{
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack   =
                                                    CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      vrfId            = (GT_U32) lua_tonumber(L, 1);
    GT_U32                      prefixLen         = (GT_U8)  lua_tonumber(L, 4);
                                                        /* prefix length */
    GT_STATUS                   status            = GT_OK;
    GT_CHAR_PTR                 error_message     = NULL;
    P_CALLING_CHECKING;

    status = pvrCpssIpProtocolFromLuaGet(L, 2, &ipProtocolStack,
                                         &error_message);

    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 3, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        switch (ipProtocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            status = cpssDxChIpLpmIpv4UcPrefixDel(0, vrfId, &(ipAddress.ipv4Addr),
                                                  prefixLen);
            P_CALLING_API(
                cpssDxChIpLpmIpv4UcPrefixDel,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv4Addr),    ipAddr,          GT_IPADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_LAST_PARAM)))),
                PC_STATUS);
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            status = cpssDxChIpLpmIpv6UcPrefixDel(0, vrfId, &(ipAddress.ipv6Addr),
                                                  prefixLen);
            P_CALLING_API(
                cpssDxChIpLpmIpv6UcPrefixDel,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv6Addr),    ipAddr,        GT_IPV6ADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_LAST_PARAM)))),
                PC_STATUS);
            break;

        default:
            status              = GT_BAD_PARAM;
            error_message       = wrongIpProtocolDefinitionErrorMessage;
            break;
        }

        if (GT_OK != status)
        {
            error_message = ipPreffixDeletingErrorMessage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*******************************************************************************
* wrlCpssDxChIpLpmIpUcPrefixDelEcmpRange
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Delete an existing ip v4 or v6 prefix ECMP entries in specified
*       Virtual Router for the specified LPM DB.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (vrfId, ip protocol, prefix ip address
*                              and prefix length should be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK if operation succed
*       2; error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpLpmIpUcPrefixDelEcmpRange
(
    IN lua_State                *L
)
{
    GT_U8                       devNum            = 0xFF;
    CPSS_IP_PROTOCOL_STACK_ENT  ipProtocolStack   = CPSS_IP_PROTOCOL_IPV4_E;
    GT_IP_ADDR_TYPE_UNT         ipAddress;
    GT_U32                      vrfId             = (GT_U32) lua_tonumber(L, 1);
    GT_U32                      prefixLen         = (GT_U8)  lua_tonumber(L, 4);
                                                        /* prefix length */
    GT_U32                      ecmpIpEntryIndex  = (GT_U32)  lua_tonumber(L, 5);
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;

    GT_STATUS                   status            = GT_OK;
    GT_CHAR_PTR                 error_message     = NULL;
    P_CALLING_CHECKING;


    cpssOsMemSet(&ecmpEntry, 0, sizeof(ecmpEntry));

    status = pvrCpssIpProtocolFromLuaGet(L, 2, &ipProtocolStack,
                                         &error_message);

    if (GT_OK == status)
    {
        status = pvrCpssIpAddrFromLuaGet(L, 3, ipProtocolStack, &ipAddress,
                                         &error_message);
    }
    else
    {
        cpssOsMemSet(&ipAddress, 0, sizeof(ipAddress));
    }

    if (GT_OK == status)
    {
        /* Clear the ECMP entry */
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.randomEnable = GT_FALSE;
        ecmpEntry.routeEntryBaseIndex = 0;

        while (GT_OK == prvWrlDevFilterNextDevGet(devNum, &devNum))
        {
            status = cpssDxChIpEcmpEntryWrite(
                devNum, ecmpIpEntryIndex, &ecmpEntry);
        }

        switch (ipProtocolStack)
        {
        case CPSS_IP_PROTOCOL_IPV4_E:
            status = cpssDxChIpLpmIpv4UcPrefixDel(0, vrfId, &(ipAddress.ipv4Addr),
                                                  prefixLen);
            P_CALLING_API(
                cpssDxChIpLpmIpv4UcPrefixDel,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv4Addr),    ipAddr,          GT_IPADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_LAST_PARAM)))),
                PC_STATUS);
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            status = cpssDxChIpLpmIpv6UcPrefixDel(0, vrfId, &(ipAddress.ipv6Addr),
                                                  prefixLen);
            P_CALLING_API(
                cpssDxChIpLpmIpv6UcPrefixDel,
                PC_NUMBER(0,                        lpmDBId,            GT_U32,
                PC_NUMBER(vrfId,                    vrId,               GT_U32,
                PC_STRUCT(&(ipAddress.ipv6Addr),    ipAddr,        GT_IPV6ADDR,
                PC_NUMBER(prefixLen,                prefixLen,          GT_U32,
                PC_LAST_PARAM)))),
                PC_STATUS);
            break;

        default:
            status              = GT_BAD_PARAM;
            error_message       = wrongIpProtocolDefinitionErrorMessage;
            break;
        }

        if (GT_OK != status)
        {
            error_message = ipPreffixDeletingErrorMessage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*******************************************************************************
* wrlCpssDxChIpEcmpIndirectNextHopEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next hop index from indirect table.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*        L                   - lua state (device number and entry index should
*                              be pushed to lua stack)
*
* OUTPUTS:
*
* RETURNS:
*       1; GT_OK and next hop index is pused to lua stack if no
*       errors occurs and such entry exists
*       2;error code and error message, if error occures
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChIpEcmpIndirectNextHopEntryGet
(
    IN lua_State            *L
)
{
    GT_U8                               devNum                  =
                                                    (GT_U8)  lua_tonumber(L, 1);
                                                                    /* devId */
    GT_U32                              ipUcRouteEntryIndex     =
                                                    (GT_U32) lua_tonumber(L, 2);
                                                        /* arp entry index */

    GT_STATUS                           status                  = GT_OK;
    GT_CHAR_PTR                         error_message           = NULL;
    int                                 returned_param_count    = 0;
    GT_U32                              nextHopIndex=0;/* the index of the next hop in the NH table*/

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)||
        (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum)== GT_TRUE))
    {
        /* take the ecmpEntry data and fetch with it the correct entry in the ECMP indirect table */
        status = cpssDxChIpEcmpIndirectNextHopEntryGet(devNum, ipUcRouteEntryIndex,&nextHopIndex);
    }
    else
    {
        /* 1:1 */
        nextHopIndex=ipUcRouteEntryIndex;
    }

    returned_param_count    +=
        prvWrlCpssStatusToLuaPush(status, L, error_message);
    returned_param_count    +=
        pvrCpssNumberToLuaPush(status, L, nextHopIndex);

    return returned_param_count;
}

