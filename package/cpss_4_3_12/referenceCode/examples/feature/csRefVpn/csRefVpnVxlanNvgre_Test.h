/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csRefVpnVxlanNvgre_Test.c
*
* DESCRIPTION:
*  This file contains test scenarios for the vxLan, using the APIs in vpnVxlanNvgre.h
*
*         Test-1 Default setting.
*           1.1. configure UNI interface on physical port
*           1.2. configure NNI on different physical port
*           1.3. Send Ethernet packet to UNI port
*                 Verify packet egress NNI port, with IPv6 vxLAN encapsulation
*           1.4. On the other direction, send IPv6 vxLan packet (swapped DIP and SIP) encapsulating Ethernet packet.
*                 Verify packet egress UNI port, after removing vxLAN encapsulation.
*
*         Test-2 (continous of Test-1)
*           2.1. Add NNI (different IPv6 DIP/SIP) in same VSI. 
*           2.2. Send Ethernet packet with unknown DA, to UNI port
*                 Verify 2 packets with different IPv6 vxLAN encapsulation. 
*           2.3. On the other direction, send 2 IPv6 vxLan packet (swapped DIP and SIP) encapsulating Ethernet packet.
*                 Verify 2 packets egress UNI port, after removing vxLAN encapsulation.
*
*         Test-3....
*         Test-4....
*         Test-5....
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>

/***********************************************/
/*         Type Definitions  */
/***********************************************/

/**
* @struct CSREF_VPN_INTERFACE_TYPE_ENT
 *
 * @brief Structure with parameters which are used for
 * Ingress Uni classification, and Egress Uni setting.
*/ 
typedef enum{
  CSREF_VPN_INTERFACE_TYPE_UNI_E,
  CSREF_VPN_INTERFACE_TYPE_NNI_E   
}CSREF_VPN_INTERFACE_TYPE_ENT;


/**
* @struct CSREF_TEST_UNI_ATTRIBUTE_STC
 *
 * @brief Structure with parameters which are used for
 * Ingress Uni classification, and Egress Uni setting.
*/ 
typedef struct{
  GT_PORT_NUM     portNum;       /* Physical port, on which to define the interface. */
  GT_U16          vlanId;        /* Ingress packet's vlan id. */
  GT_ETHERADDR    srcMacAddress; /* packet's source MAC address. */
}CSREF_TEST_UNI_ATTRIBUTE_STC;


/**
* @struct CSREF_TEST_UNI_ATTRIBUTE_STC
 *
 * @brief Structure with parameters which are used for
 * Ingress Nni classification, and Egress Nni setting.
*/ 
typedef struct{
  GT_PORT_NUM     portNum;               /* Physical port, on which to define the interface. */
  GT_U8           remotePeIpAddress[16]; /* 16 bytes array of remote device IP address. */
  GT_ETHERADDR    nhMacAddress;          /* MAC address of next attached device. DA of encasulated packets */
  GT_BOOL         innerPacketTagged;     /* Encapsulated packet is tagged */
}CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC;

/***********************************************/
/*         Function Definitions                */
/***********************************************/

/**
* @internal csRefVpnVxLanTestGenInfoSet function
* @endinternal
*
* @brief   Set test general infomation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] domainVsiId      - domain VSI id
* @param[in] ipType           - IP protocol type, IPv4 or IPv6.
* @param[in] localIpAddress   - String of IPv/IPv6 address format, DIP of ingress packet, SIP of egress packet.
* @param[in] mac2meAddressPtr - pointer to mac2me MAC address
*
* @retval GT_OK            - on success
* @retval GT_BAD_PARAM     - wrong value in any of the parameter 
* @retval GT_FAIL          - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanTestGenInfoSet
(
  IN GT_U8        devNum,
  IN GT_U32       domainVsiId,   /* domain VSI id  */
  IN CPSS_IP_PROTOCOL_STACK_ENT ipType,
  IN GT_CHAR     *localIpAddress,
  IN GT_CHAR     *mac2meAddress
);

/**
* @internal csRefVpnVxLanTestNniInfoEntrySet function
* @endinternal
*
* @brief   Set NNI interface entry in test database.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - device number
* @param[in] portNum           - Physical port, on which to define this interface.
* @param[in] remotePeIpAddress - String of IPv4/v6 address format, SIP of ingress packet, DIP of egress packet.
* @param[in] nhMacAddress      - String of MAC address format. Next ho MAC address. DA of egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter 
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanTestNniInfoEntrySet
(
  IN GT_U8           devNum,
  IN GT_PORT_NUM     portNum,
  IN GT_CHAR        *remoteIpAddress,
  IN GT_CHAR        *nhMacAddress
);

/**
* @internal csRefVpnVxLanTestUniInfoEntrySet function
* @endinternal
*
* @brief   Set UNI interface entry in test database.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum         - device number
* @param[in] portNum        - Physical port, on which to define this interface.
* @param[in] vlanId         - Ingress packet's vlan id.
* @param[in] srcMacAddress  - String of MAC address format. Packet source MAC address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter 
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanTestUniInfoEntrySet
(
  IN GT_U8           devNum,
  IN GT_PORT_NUM     portNum,
  IN GT_U16          vlanId,
  IN GT_CHAR        *srcMacAddress
);

/**
* @internal csRefVpnVxLanTestConfigure function
* @endinternal
*
* @brief   This test function uses the Vxlan API to configure the device for the various of test scenario.
*            Phase1: Configure one NNI and one UNI in one segment L2-VPN
*            Phase2: Add NNI and UNI and check flooding behavior.
*            Phase3: Same configuration in phase1, different segment id and different port.
*                        Show that routing/bridging is only within the segment
*            Phase4: Routing of vxLAN passenger packet after TT.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device number
* @param[in] deafultSet   - run default setting
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfig
(
  IN GT_U8         devNum,
  IN GT_BOOL       deafultSet
);

/**
* @internal csRefVpnVxLanConfigClear function
* @endinternal
*
* @brief   This function clean all configuration done by csRefVpnVxLanTestConfigure
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum     - device number
* @param[in] resetDb    - Reset DB or not
*                         For robustness purposes
*                         don't reset DB between phases
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxLanConfigClean
(
  IN GT_U8         devNum
);

