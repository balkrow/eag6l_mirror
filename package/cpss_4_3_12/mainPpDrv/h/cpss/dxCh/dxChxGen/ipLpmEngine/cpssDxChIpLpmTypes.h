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
* @file cpssDxChIpLpmTypes.h
*
* @brief the CPSS DXCH LPM Engine Type.
*
* @version   13
********************************************************************************
*/

#ifndef __cpssDxChIpLpmTypesh
#define __cpssDxChIpLpmTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>

/**
* @enum CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT
 *
 * @brief the types of DXCH ip LPM shadows.
*/
typedef enum{

    /** @brief xCat type shadow (routing is router TCAM based)
     *  4 columns, row based search order
     *  IPv4 entry use single TCAM entry, IPv6 entry use single TCAM line
     */
    CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E                         = 3,

    /** @brief xCat type shadow (routing is policy based routing)
     *  4 columns, row based search order
     *  IPv4 & IPv6 entries use single TCAM entry
     */
    CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E    = 4,

    /** SIP 5 type shadow (routing is RAM based) */
    CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E                         = 5,

    /** SIP 6 type shadow (routing is RAM based) */
    CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E                         = 6,

    /** SIP 7 type shadow (routing is RAM based) */
    CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E                         = 7,

    /** last value in the enum */
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_LAST_E

} CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT;


/**
* @enum CPSS_DXCH_IP_LPM_VALIDITY_TYPE_ENT
 *
 * @brief the types of DXCH IP LPM validity.
*/
typedef enum{

    /** @brief perform LPM shadow validation            */
    CPSS_DXCH_IP_LPM_VALIDITY_SHADOW_E                         ,

    /** @brief perform LPM HW validation                */
    CPSS_DXCH_IP_LPM_VALIDITY_HW_E                             ,

    /** @brief check sync between LPM hw and LPM shadow */
    CPSS_DXCH_IP_LPM_VALIDITY_SHADOW_HW_SYNC_E                 ,

   /** @brief perform all validity  operations */
    CPSS_DXCH_IP_LPM_VALIDITY_ALL_E

} CPSS_DXCH_IP_LPM_VALIDITY_TYPE_ENT;


/* Old shadow types - remained for backward compatibility, unused in the code */
#define CPSS_DXCH_IP_TCAM_SHADOW_TYPE_ENT    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT
#define CPSS_DXCH_IP_TCAM_XCAT_SHADOW_E      CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E
#define CPSS_DXCH_IP_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E                   \
                                             CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E
#define CPSS_DXCH_IP_TCAM_SHADOW_TYPE_LAST_E CPSS_DXCH_IP_LPM_SHADOW_TYPE_LAST_E

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC
 *
 * Description:
 *      This struct hold the capacity configuration required from the Tcam LPM
 *      manager. According to these requirements the TCAM entries will be
 *      fragmented.
 *
 * Fields:
 *  numOfIpv4Prefixes           - The total number of Ipv4 Uc prefixes and
 *                                ipv4 MC group prefixes required.
 *  numOfIpv4McSourcePrefixes   - The number of Ipv4 MC source prefixes required.
 *  numOfIpv6Prefixes           - The total number of Ipv6 Uc prefixes,
 *                                ipv6 MC group prefixes and ipv6 MC sources
 *                                prefixes required.
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STCT
{
    GT_U32  numOfIpv4Prefixes;
    GT_U32  numOfIpv4McSourcePrefixes;
    GT_U32  numOfIpv6Prefixes;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC;

/*
 * typedef: struct CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC
 *
 * Description:
 *      This struct hold the indexes available for the Tcam LPM manager.
 *      it defines the From-To indexes in the TCAM the lpm manager can use.
 *
 *      Cheetah+ systems: The indexes are the PCL Rule indexes dedicated for the
 *                        IP.
 *                        Pay attention that when used to indicate PCL Tcam
 *                        lines, the line x means pcl tcam indexes x,x+512.
 *      Cheetah2 systems: The indexes are the LINE indexes dedicated to for the
 *                        IP (UC + MC) out of the 1024 lines avilable. pay
 *                        attention that the a line x in the router tcam means
 *                        router tcam indexes x,x+1024,x+2048,x+3072,x+4096.
 *      Cheetah2 with VR support systems:
 *                        The indexes are the LINE indexes dedicated to for the
 *                        IP UC out of the total lines available in PCL/Router.
 *                        Pay attention that when used to indicate Router Tcam
 *                        lines, the line x means router tcam indexes
 *                        x,x+1024,x+2048,x+3072,x+4096.
 *                        Pay attention that when used to indicate PCL Tcam
 *                        lines, the line x means pcl tcam indexes x,x+512.
 *
 * Fields:
 *  firstIndex - this is the first index availble (from)
 *  lastIndex  - this is the last index availble (to - including)
 *
 */
typedef struct CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STCT
{
    GT_U32  firstIndex;
    GT_U32  lastIndex;
}CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC;


/**
* @union CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
 *
 * @brief This union holds the two possible accosiactions a prefix can have
 * for Cheetha+ devices it will be a PCL action.
 * for Cheetha2 devices it will be a LTT entry.
 *
*/

typedef union{
    CPSS_DXCH_PCL_ACTION_STC pclIpUcAction;

    CPSS_DXCH_IP_LTT_ENTRY_STC ipLttEntry;

} CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT;



/*
 * Typedef: struct CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC
 *
 * Description: IPv4 UC prefix insert data
 *
 * Fields:
 *
 *    vrId              - The virtual router identifier.
 *    ipAddr            - The destination IP address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info accosiated with this UC prefix.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *    returnStatus      - the return status for this prefix.
 *
 */
typedef struct CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STCT
{
    GT_U32                                 vrId;
    GT_IPADDR                              ipAddr;
    GT_U32                                 prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override;
    GT_STATUS                              returnStatus;
} CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC
 *
 * Description: IPv6 UC prefix insert data
 *
 * Fields:
 *
 *    vrId              - The virtual router identifier.
 *    ipAddr            - The destination IP address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info accosiated with this UC prefix.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *    returnStatus      - the return status for this prefix.
 *
 */
typedef struct CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STCT
{
    GT_U32                                 vrId;
    GT_IPV6ADDR                            ipAddr;
    GT_U32                                 prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override;
    GT_STATUS                              returnStatus;
} CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC;

/*
 * Typedef: struct CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC
 *
 * Description: FCOE prefix insert data
 *
 * Fields:
 *
 *    vrId              - The virtual router identifier.
 *    fcoeAddr          - The FCOE address of this prefix.
 *    prefixLen         - The prefix length of ipAddr
 *    nextHopInfo       - the route entry info accosiated with this UC prefix.
 *    override          - GT_TRUE, override an existing prefix.
 *                        GT_FALSE, don't override an existing prefix,and return
 *                        an error.
 *    returnStatus      - the return status for this prefix.
 *
 */
typedef struct CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STCT
{
    GT_U32                                 vrId;
    GT_FCID                                fcoeAddr;
    GT_U32                                 prefixLen;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override;
    GT_STATUS                              returnStatus;
} CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_LPM_VR_CONFIG_STCT
 *
 * Description: IP virtual router configuration parameters.
 *
 * Fields:
 *
 *      supportIpv4Uc                 - whether this VR support UC Ipv4.
 *      defIpv4UcNextHopInfo          - the route entry info associated with the default
 *                                      Ipv4 UC route entry.
 *      supportIpv4Mc                 - whether this VR support MC Ipv4.
 *      defIpv4McRouteLttEntry        - the LTT entry pointing to the default ipv4 MC route
 *                                      entry.
 *      supportIpv6Uc                 - whether this VR support UC Ipv6.
 *      defIpv6UcNextHopInfo          - the route entry info associated with the default
 *                                      Ipv6 UC route entry.
 *      supportIpv6Mc                 - whether this VR support MC Ipv6
 *      defIpv6McRouteLttEntry        - the LTT entry pointing to the default Ipv6 MC route
 *                                      entry;
 *      supportFcoe                   - whether this VR supports FCoE
 *      defaultFcoeForwardingEntry    - the forwarding entry info associated to the
 *                                      default FCoE forwarding entry.
 *      treatIpv4ClassEasNonRegularUc - shows how to deal with class E:
 *                                      GT_TRUE: Class E treated as special range with dedicated default
 *                                      GT_FALSE: Class E treated as any other unicast
 *      defIpv4ClassENextHopInfo      - the route entry info associated with the E class default.
 *
 * Comments:
 *       supportIpv4Uc, supportIpv4Mc, supportIpv6Uc, supportIpv6Mc, supportFcoe can't be all
 *       GT_FALSE.
 */
typedef struct CPSS_DXCH_IP_LPM_VR_CONFIG_STCT
{
    GT_BOOL                                 supportIpv4Uc;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defIpv4UcNextHopInfo;
    GT_BOOL                                 supportIpv4Mc;
    CPSS_DXCH_IP_LTT_ENTRY_STC              defIpv4McRouteLttEntry;
    GT_BOOL                                 supportIpv6Uc;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defIpv6UcNextHopInfo;
    GT_BOOL                                 supportIpv6Mc;
    CPSS_DXCH_IP_LTT_ENTRY_STC              defIpv6McRouteLttEntry;
    GT_BOOL                                 supportFcoe;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defaultFcoeForwardingNextHopInfo;
    GT_BOOL                                 treatIpv4ClassEasNonRegularUc;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  defIpv4ClassENextHopInfo;
} CPSS_DXCH_IP_LPM_VR_CONFIG_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_LPM_TCAM_CONFIG_STCT
 *
 * Description: Memory configurations for IP LPM TCAM-based shadow
 *
 * Fields:
 *      indexesRangePtr       - the range of TCAM indexes available for this
 *                              LPM DB (see explanation in
 *                              CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC)
 *      partitionEnable       - GT_TRUE:  partition the TCAM range according to the
 *                                        capacityCfgPtr, any unused TCAM entries will
 *                                        be allocated to IPv4 UC entries
 *                              GT_FALSE: allocate TCAM entries on demand while
 *                                        guarantee entries as specified in capacityCfgPtr
 *      tcamLpmManagerCapcityCfgPtr - holds the capacity configuration required
 *                                    from this TCAM LPM manager
 *      tcamManagerHandlerPtr       - the TCAM manager handler
 *
 * Comments:
 *      1. if partitionEnable = GT_FALSE then the TCAM is one big shared resource
 *         for all kinds of prefixes. In a shared resource situation a best effort
 *         is applied to put the inserted prefixes in a way they wouldn't interfere
 *         with other prefixes. But there could be a situation where due to
 *         fragmentation a prefix couldn't be inserted, in this situation all
 *         prefix insert function has a defragmationEnable to enable a performance
 *         costing de-fragmentation process in order to fit the inserted prefix.
 *      2. TCAM Manager - external TCAM Manager module. TCAM manager is intended to
 *         manage tcams of XCAT and above devices otherwise legacy TCAM Entries Handler is
 *         used. TCAM manager could be created explicitly before calling to
 *         cpssDxChIpLpmDBCreate or (if tcamManagerHandlerPtr == NULL) implicitly
 *         in cpssDxChIpLpmDBCreate (backward compatible mode).
 *
 */
typedef struct CPSS_DXCH_IP_LPM_TCAM_CONFIG_STCT
{
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr;
    GT_BOOL                                      partitionEnable;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr;
    GT_VOID                                      *tcamManagerHandlerPtr;
} CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC;

/**
* @union CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT
 *
 * @brief Memory configurations for IP LPM shadows
 *
*/

typedef union{
    /** configurations for TCAM based shadows */
    CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC tcamDbCfg;

    /** @brief configurations for RAM based shadows
     *  Comments:
     *  None
     */
    CPSS_DXCH_LPM_RAM_CONFIG_STC ramDbCfg;

} CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpLpmTypesh */


