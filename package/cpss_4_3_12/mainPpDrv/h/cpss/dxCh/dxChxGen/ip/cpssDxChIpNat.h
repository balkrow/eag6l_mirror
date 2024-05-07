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
* @file cpssDxChIpNat.h
*
* @brief The CPSS DXCH IP NAT structures and APIs
*
* @version   2
********************************************************************************
*/
#ifndef __cpssDxChIpNath
#define __cpssDxChIpNath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>

/**
* @struct CPSS_DXCH_IP_NAT44_ENTRY_STC
 *
 * @brief Struct for NAT44 entry.
*/
typedef struct{

    /** The MAC DA of the passenger packet */
    GT_ETHERADDR macDa;

    /** @brief Selector for modifying the packet Destination IP address
     *  GT_FALSE: Do not modify the Destination IP address of the packet
     *  GT_TRUE: Modify the Destination IP address of the packet
     */
    GT_BOOL modifyDip;

    /** The new Destination IP address of the packet */
    GT_IPADDR newDip;

    /** @brief Selector for modifying the packet Source IP address
     *  GT_FALSE: Do not modify the Source IP address of the packet
     *  GT_TRUE: Modify the Source IP address of the packet
     */
    GT_BOOL modifySip;

    /** The new Source IP address of the packet */
    GT_IPADDR newSip;

    /** @brief Selector for modifying the packet TCP / UDP Destination port
     *  GT_FALSE: Do not modify the TCP / UDP Destination port of the packet
     *  GT_TRUE: Modify the TCP / UDP Destination port of the packet
     */
    GT_BOOL modifyTcpUdpDstPort;

    /** @brief The New TCP / UDP Destination Port of the packet
     *  (APPLICABLE RANGES: 0..65535)
     */
    GT_U32 newTcpUdpDstPort;

    /** @brief Selector for modifying the packet TCP / UDP Source port
     *  GT_FALSE: Do not modify the TCP / UDP Source port of the packet
     *  GT_TRUE: Modify the TCP / UDP Source port of the packet
     */
    GT_BOOL modifyTcpUdpSrcPort;

    /** @brief The New TCP / UDP Source Port of the packet
     *  (APPLICABLE RANGES: 0..65535)
     */
    GT_U32 newTcpUdpSrcPort;

    /** @brief Defines the length of the SIP prefix
     *  (APPLICABLE RANGE: 1..32)
     *  (APPLICABLE DEVICES: Ironman)
     */
    GT_U32 sipPrefixLen;
    /** @brief Defines the length of the DIP prefix
     *  (APPLICABLE RANGE: 1..32)
     *  (APPLICABLE DEVICES: Ironman)
     */
    GT_U32 dipPrefixLen;

} CPSS_DXCH_IP_NAT44_ENTRY_STC;

/**
* @enum CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT
 *
 * @brief Enumeration of NAT Modify Command.
*/
typedef enum{

    /** Replace the SIP prefix with the NAT<Address>. */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E,

    /** Replace the DIP prefix with the NAT<Address>. */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,

    /** Replace the SIP address with NAT<Address>; */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E,

    /** Replace the DIP address with NAT<Address>; */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E,

    /** Replace the SIP prefix with NAT<Address>;
     *  without performing checksum-neutral mapping.
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman  */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_WO_E,

    /** Replace the DIP prefix with NAT<Address>;
     *  without performing checksum-neutral mapping.
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_WO_E

} CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT;

/**
* @struct CPSS_DXCH_IP_NAT66_ENTRY_STC
 *
 * @brief Struct for NAT66 entry.
*/
typedef struct{

    /** The MAC DA of the passenger packet */
    GT_ETHERADDR macDa;

    /** @brief Translation command
     *  if command is CPSS_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E, CPSS_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,
     *  The prefix size is taken from the <Prefix size> field in this entry
     *  if command is CPSS_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E, CPSS_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E,
     *  The address is taken from the <address> field in this entry
     */
    CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT modifyCommand;

    /** @brief New SIP/DIP address used to replace a packet's SIP/DIP according to modifyCommand.
     *  if command is CPSS_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E, CPSS_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E,
     *  then this is the new SIP or DIP.
     *  if command is CPSS_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E, CPSS_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E,
     *  then replace the <Prefix size> MSB of the SIP or DIP with the prefix of this entry (the prefix is aligned to the left)
     *  This means that the MSBs of the NAT Address will be assigned as the new packet Addr (SIP/DIP).
     */
    GT_IPV6ADDR address;

    /** @brief Address prefix size for NAT modifyCommand == Modify SIP prefix / Modify DIP prefix (according to RFC6296).
     *  (APPLICABLE RANGES: 1..64)
     */
    GT_U32 prefixSize;

} CPSS_DXCH_IP_NAT66_ENTRY_STC;

/**
* @enum CPSS_IP_NAT_TYPE_ENT
 *
 * @brief Enumeration of NAT types.
*/
typedef enum{

    /** NAT44: translation of ipv4 address to ipv4 address */
    CPSS_IP_NAT_TYPE_NAT44_E,

    /** NAT44: translation of ipv6 address to ipv6 address */
    CPSS_IP_NAT_TYPE_NAT66_E,

    CPSS_IP_NAT_TYPE_LAST_E

} CPSS_IP_NAT_TYPE_ENT;

/**
* @union CPSS_DXCH_IP_NAT_ENTRY_UNT
 *
 * @brief Union for NAT entry
 *
*/

typedef union{
    /** entry for NAT44 */
    CPSS_DXCH_IP_NAT44_ENTRY_STC nat44Entry;

    /** entry for NAT66 */
    CPSS_DXCH_IP_NAT66_ENTRY_STC nat66Entry;

} CPSS_DXCH_IP_NAT_ENTRY_UNT;


/**
* @struct CPSS_DXCH_IP_NAT_L2_PORT_CONFIG_STC
 *
 * @brief Struct for L2NAT EPORT configuration
*/
typedef struct{
    /** Flag to enable/disable L2NAT processing */
    GT_BOOL   l2NatEnable;
    /** Pointer to NAT table entry */
    GT_U32    natPtr;
} CPSS_DXCH_IP_NAT_L2_PORT_CONFIG_STC;

/**
* @struct CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC
 *
*  @brief Struct for L2NAT Global configuration
*       SrcId size is 12 bits
*/
typedef struct{
    /** The number of Src-ID bits used for representing
     *  the ingress port group for L2NAT */
    GT_U32     l2NatSrcIdSize;
    /** The Offset (lsb) in the Src-ID used for representing the
     *  ingress port group for L2NAT */
    GT_U32     l2NatSrcIdLsb;
} CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC;

/**
* @internal cpssDxChIpNatEntrySet function
* @endinternal
*
* @brief   Set a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] entryIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
* @param[in] natType                  - type of the NAT
* @param[in] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       See also cpssDxChTunnelStartEntrySet and cpssDxChIpRouterArpAddrWrite
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
GT_STATUS cpssDxChIpNatEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_IP_NAT_TYPE_ENT                natType,
    IN  CPSS_DXCH_IP_NAT_ENTRY_UNT          *entryPtr
);

/**
* @internal cpssDxChIpNatEntryGet function
* @endinternal
*
* @brief   Get a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] entryIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
*
* @param[out] natTypePtr               - (pointer to) type of the NAT
* @param[out] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table (for NAT capable devices), tunnel start entries table
*       and router ARP addresses table reside at the same physical memory.
*       See also cpssDxChTunnelStartEntrySet and cpssDxChIpRouterArpAddrWrite
*
*       For xCat3; AC5; Lion2; devices, Each line can hold:
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       Tunnel start entry at index n and router ARP addresses at indexes 4n..4n+3 share the same memory.
*       For example NAT entry/tunnel start entry at index 100
*       and router ARP addresses at indexes 400..403 share the
*       same physical memory.
*
*       For Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman devices, Each line can hold:
*       - 2 NAT entries
*       - 1 CPSS_TUNNEL_GENERIC_E or CPSS_TUNNEL_GENERIC_IPV6_E tunnel start entry type
*       - 2 other tunnel start entries
*       - 8 router ARP addresses entries
*/
GT_STATUS cpssDxChIpNatEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             entryIndex,
    OUT  CPSS_IP_NAT_TYPE_ENT               *natTypePtr,
    OUT  CPSS_DXCH_IP_NAT_ENTRY_UNT         *entryPtr
);

/**
* @internal cpssDxChIpNatDroppedPacketsCntGet function
* @endinternal
*
* @brief   Get the counter that counts the packets that were dropped since NAT could not be applied.
*         If the L4 header is not within the header 64 bytes (For tunnel-terminated packet it must
*         be within the passenger header 64 bytes), the packet is dropped and it is counted by
*         the NAT Drop Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] natDropPktsPtr           - (pointer to) the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Clear on read.
*
*/
GT_STATUS cpssDxChIpNatDroppedPacketsCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *natDropPktsPtr
);

/**
* @internal cpssDxChIpNatL2GlobalConfigSet function
* @endinternal
*
* @brief  This function is used to set the L2NAT SrcID size
*         and SrcID LSB in the global configuration register.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; xCat3; AC5; Lion2.
*
* @param[in]  devNum               - the device number
* @param[in]  natType              - NAT table type
* @param[in]  l2NatGlobalCfgPtr    - pointer to structure
*                                    CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIpNatL2GlobalConfigSet
(
    IN   GT_U8                                 devNum,
    IN   CPSS_IP_NAT_TYPE_ENT                  natType,
    IN   CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC *l2NatGlobalCfgPtr
);

/**
* @internal cpssDxChIpNatL2GlobalConfigGet function
* @endinternal
*
* @brief  This function is used to get the L2NAT SrcID size and
*         SrcID LSB from the global configuration register.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; xCat3; AC5; Lion2.
*
* @param[in]  devNum               - the device number
* @param[in]  natType              - NAT table type
* @param[out] l2NatGlobalCfgPtr    - pointer to structure
*                                    CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIpNatL2GlobalConfigGet
(
    IN   GT_U8                                 devNum,
    IN   CPSS_IP_NAT_TYPE_ENT                  natType,
    OUT  CPSS_DXCH_IP_NAT_L2_GLOBAL_CONFIG_STC *l2NatGlobalCfgPtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpNath */


