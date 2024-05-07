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


#include "csRefVpnVxlanNvgre_Test.h"
#include "csRefVpnVxlanNvgre.h"
#include "../../prvCsrefLog.h"
#include "../../infrastructure/csRefSysConfig/csRefGlobalSystemInfo.h"
#include "../../infrastructure/csRefServices/vsiDomainDb.h"
#include "../../infrastructure/csRefServices/stringToNetUtils.h" 

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>

/******** External appDemo API ***********/

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

/***********************************************/
/*         vxLAN related DB initialization  */
/***********************************************/



#define VPN_EVID_FIRST_ENTRY_CNS      4096
#define VPN_EVID_NUM_OF_ENTRIES_CNS   5
static  GT_U32 prvVpnCurrAvaiableEvid = 0xFFFFFF;

#define VPN_EVIDX_FIRST_ENTRY_CNS     4096
#define VPN_EVIDX_NUM_OF_ENTRIES_CNS  10
static  GT_U32  prvVpnCurrAvaiableEvidX = 0xFFFFFF;

#define VPN_EPORT_FIRST_ENTRY_CNS     3000
#define VPN_EPORT_NUM_OF_ENTRIES_CNS  10
static  GT_U32  prvVpnCurrAvaiableEport = 0xFFFFFF;




static CPSS_PORTS_BMP_STC prvVpnPhysicalEportBmp;

static GT_BOOL prvVxLanRelateDbInitDone = GT_FALSE;

static GT_STATUS prvVxLanRelateDBInit
(
  IN  GT_U8    devNum,
  IN  GT_BOOL  doInit 
)
{
  GT_STATUS rc=GT_OK;

  if(prvVxLanRelateDbInitDone == doInit)
    return GT_OK;

  CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC((&prvVpnPhysicalEportBmp));

  prvVpnCurrAvaiableEvid  = VPN_EVID_FIRST_ENTRY_CNS;

  prvVpnCurrAvaiableEvidX = VPN_EVIDX_FIRST_ENTRY_CNS;

  prvVpnCurrAvaiableEport = VPN_EPORT_FIRST_ENTRY_CNS;

  /* This value configured in cpssInitSystem flow, function prvTcamLibInit*/
  rc = csRefInfraVsiDbInit(devNum, VPN_EVIDX_FIRST_ENTRY_CNS-1, doInit);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);


  prvVxLanRelateDbInitDone = doInit;
  return GT_OK;
}

/***********************************************/
/*         END of vxLAN related DB initialization  */
/***********************************************/



/***********************************************/
/*         local service function and variable definition  */
/***********************************************/

/* Keep infomation for clean up */

#define CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS  5
static CSREF_TEST_UNI_ATTRIBUTE_STC  prvUniInterfacesArray[CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS];
static CSREF_TEST_IPV6_NNI_ATTRIBUTE_STC prvNniInterfacesArray[CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS];
static GT_U32  prvNniInterfacesCreated, prvUniInterfacesCreated;

static GT_U32          prvDomainVsiId = 100;
static GT_ETHERADDR    prvMac2meAddress = {{0x0,0x0,0x10,0x10,0x10,0x10}};
static GT_U8           prvLocalIpAddress[16];



/* Configure eVlan to represent the VSI domain, in the device. */
static GT_STATUS prvVxLanIPv6VpnVsiCreate
(
  IN GT_U8        devNum,
  IN GT_U32       serviceId,
  OUT GT_U32     *eVlanAssigned /* Representing the VSI */
)
{
  GT_STATUS rc=GT_OK;
  GT_U32          eVlanId = 0xFFFFFF, floodingEvidx = 0xFFFFFF;

  if(prvVpnCurrAvaiableEvid >= (VPN_EVID_FIRST_ENTRY_CNS + VPN_EVID_NUM_OF_ENTRIES_CNS))
    return GT_NO_RESOURCE;

  eVlanId = prvVpnCurrAvaiableEvid;
  floodingEvidx = prvVpnCurrAvaiableEvidX;

  /* Create VSI domain with allocated eVlanAssigned. */
  rc = csRefInfraVsiDomain(devNum, eVlanId, VPN_VSI_DB_OPERATION_CREATE_E, serviceId, floodingEvidx);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  *eVlanAssigned = eVlanId;
  prvVpnCurrAvaiableEvid++;
  prvVpnCurrAvaiableEvidX++;

  return GT_OK;
}


/* Configure UNI or NNI interface flow.*/
static GT_STATUS prvVxLanIPv6VpnInterfaceCreate
(
  GT_U8                        devNum,  
  CSREF_VPN_INTERFACE_TYPE_ENT interfaceType,
  GT_U32                       configEntryIndex, /* Entry index in local DB */
  GT_U32                       eVlanAssigned /* Representing the VSI */
)
{
  GT_STATUS     rc;
  GT_PORT_NUM   portNum;
  GT_PORT_NUM   assignEportNum;
  GT_BOOL       innerPacketTagged;
  GT_U8        *localIp;
  GT_U8        *remoteIp;
  GT_ETHERADDR  macAddress;

  if(prvVpnCurrAvaiableEport  >= VPN_EPORT_FIRST_ENTRY_CNS + VPN_EPORT_NUM_OF_ENTRIES_CNS)
    return GT_NO_RESOURCE;


  /* Allocate ePort for the interface */  
  assignEportNum = prvVpnCurrAvaiableEport;

  /* Create interface */   
  switch(interfaceType)
  {
    case CSREF_VPN_INTERFACE_TYPE_UNI_E:
    {
        macAddress = prvUniInterfacesArray[configEntryIndex].srcMacAddress;
        portNum    = prvUniInterfacesArray[configEntryIndex].portNum;        
        rc = csRefVpnUniInterfaceCreate(devNum, portNum, prvUniInterfacesArray[configEntryIndex].vlanId,
                                        &macAddress, prvDomainVsiId, eVlanAssigned, assignEportNum);
        CSREF_LOG_RETURN_NOT_OK_MAC(rc);
    }
    break;

    case CSREF_VPN_INTERFACE_TYPE_NNI_E:
    {
        localIp      = prvLocalIpAddress;
        remoteIp     = prvNniInterfacesArray[configEntryIndex].remotePeIpAddress;
        macAddress   = prvNniInterfacesArray[configEntryIndex].nhMacAddress;
        innerPacketTagged = prvNniInterfacesArray[configEntryIndex].innerPacketTagged;  
        portNum           =  prvNniInterfacesArray[configEntryIndex].portNum;
  
        rc = csRefVpnVxlanNniInterfaceCreate(devNum, portNum, localIp, remoteIp,
                                             CPSS_IP_PROTOCOL_IPV6_E, prvDomainVsiId,
                                             eVlanAssigned, assignEportNum,
                                             innerPacketTagged, &macAddress);
        CSREF_LOG_RETURN_NOT_OK_MAC(rc);
    }
    break;

    default:
        CSREF_LOG_RETURN_NOT_OK_MAC(GT_BAD_PARAM);
        break;
  }

  /* Add created interface to VSI domain. */
  rc = csRefInfraVsiEport(devNum, eVlanAssigned, VPN_VSI_DB_OPERATION_ADD_E, assignEportNum);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  CPSS_PORTS_BMP_PORT_SET_MAC((&prvVpnPhysicalEportBmp),portNum);

  prvVpnCurrAvaiableEport++;

  return GT_OK;
}



static GT_STATUS vxLanIPv6ConfigureSequence
(
  GT_U8         devNum
)
{
  GT_STATUS   rc=GT_OK;
  GT_U32      eVlanAssigned;
  GT_U32      entryIndex;

  if((prvUniInterfacesCreated > CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS)||
     (prvUniInterfacesCreated > CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS) )
  CSREF_LOG_RETURN_NOT_OK_MAC(GT_BAD_STATE);

  
  /* Set MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefVpnMac2MeSet(devNum, &prvMac2meAddress);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  /* Create VSI domains, with empty ePort list. */
  rc = prvVxLanIPv6VpnVsiCreate(devNum, prvDomainVsiId, &eVlanAssigned);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);
 
  
  /* Create NNI interfaces and add it to VSI domain. */
  for(entryIndex = 0 ; entryIndex < prvNniInterfacesCreated ; entryIndex++)
  {
     rc = prvVxLanIPv6VpnInterfaceCreate(devNum, CSREF_VPN_INTERFACE_TYPE_NNI_E, entryIndex, eVlanAssigned);
     CSREF_LOG_RETURN_NOT_OK_MAC(rc);
  }

  /* Create UNI interfaces and add it to VSI domain. */
  for(entryIndex = 0 ; entryIndex < prvUniInterfacesCreated ; entryIndex++)
  {
     rc = prvVxLanIPv6VpnInterfaceCreate(devNum, CSREF_VPN_INTERFACE_TYPE_UNI_E, entryIndex, eVlanAssigned);
     CSREF_LOG_RETURN_NOT_OK_MAC(rc);
  }

  return GT_OK;
}


/*==============================================================================*/
/*==============================================================================*/
/*==============================================================================*/
/*==============================================================================*/


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
)
{
  GT_STATUS rc = GT_OK;
  GT_UNUSED_PARAM(devNum);

  rc = csRefInetStringToNet(INET_ADDRESS_IPV6, localIpAddress,  prvLocalIpAddress);
  CSREF_PRINT_IPV6_ADDR_MAC(prvLocalIpAddress);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  rc = csRefInetStringToNet(INET_ADDRESS_MAC , mac2meAddress, &(prvMac2meAddress.arEther[0]));
  CSREF_PRINT_MAC_ADDR_MAC(prvMac2meAddress.arEther);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  if(CPSS_IP_PROTOCOL_IPV4_E == ipType)
    rc = csRefInetStringToNet(INET_ADDRESS_IPV4, localIpAddress,  prvLocalIpAddress);
  else if(CPSS_IP_PROTOCOL_IPV6_E == ipType)
    rc = csRefInetStringToNet(INET_ADDRESS_IPV6, localIpAddress,  prvLocalIpAddress);
  else
    rc = GT_BAD_PARAM; /* Must be change to IPv4 or IPv6*/
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  prvNniInterfacesCreated = 0; 
  prvUniInterfacesCreated = 0;

  osMemSet(prvNniInterfacesArray, 0xFF, sizeof(prvNniInterfacesArray));
  osMemSet(prvUniInterfacesArray, 0xFF, sizeof(prvUniInterfacesArray));

  prvDomainVsiId = domainVsiId;
    
  return GT_OK;
}


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
)
{
  GT_STATUS rc = GT_OK;
  GT_UNUSED_PARAM(devNum);


  if(prvNniInterfacesCreated >= CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS)
     CSREF_LOG_RETURN_NOT_OK_MAC(GT_NO_RESOURCE);

  rc = csRefInetStringToNet(INET_ADDRESS_IPV6, remoteIpAddress, prvNniInterfacesArray[prvNniInterfacesCreated].remotePeIpAddress);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  rc = csRefInetStringToNet(INET_ADDRESS_MAC , nhMacAddress, &(prvNniInterfacesArray[prvNniInterfacesCreated].nhMacAddress.arEther[0]));
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  prvNniInterfacesArray[prvNniInterfacesCreated].portNum = portNum;

  prvNniInterfacesCreated++;

  return GT_OK;
}



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
)
{
  GT_STATUS rc = GT_OK;
  GT_UNUSED_PARAM(devNum);

  if(prvUniInterfacesCreated >= CSREF_VPN_MAX_NUMBER_OF_UNI_NNI_CNS)
     CSREF_LOG_RETURN_NOT_OK_MAC(GT_NO_RESOURCE);

  rc = csRefInetStringToNet(INET_ADDRESS_MAC , srcMacAddress, 
                            &(prvUniInterfacesArray[prvUniInterfacesCreated].srcMacAddress.arEther[0]));
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  prvUniInterfacesArray[prvUniInterfacesCreated].vlanId      = vlanId;
  prvUniInterfacesArray[prvUniInterfacesCreated].portNum     = portNum;

  prvUniInterfacesCreated++;

  return GT_OK;
}


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
)
{
  GT_STATUS rc=GT_OK;

  /* eArch element DB initializatin. */
  rc = prvVxLanRelateDBInit(devNum, GT_TRUE);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  /* Default setting.
     Segment ID=200, system Mac2me = 00:00:B0:01:01:01.
     one UNI interface :- portNum=0, vid=10, packet's MAC SA = "48:96:B0:A8:01:01"
     one NNI interface :- portNum=3, local ip = FFFF:FFFF::1001, remote ip = FFFF:FFFF::6400:1001, egress packet DA = AA:DD:EE:00:00:01
  */
  if(GT_TRUE == deafultSet)
  {
    rc = csRefVpnVxLanTestGenInfoSet(0, prvDomainVsiId, CPSS_IP_PROTOCOL_IPV6_E, "FFFF:FFFF::1001", "00:00:B0:01:01:01");
    CSREF_LOG_RETURN_NOT_OK_MAC(rc);

    rc = csRefVpnVxLanTestUniInfoEntrySet(0, 0, 10, "48:96:B0:A8:01:01");
    CSREF_LOG_RETURN_NOT_OK_MAC(rc);

    rc = csRefVpnVxLanTestNniInfoEntrySet(0, 3, "FFFF:FFFF::6400:1001", "AA:DD:EE:00:00:01");
    CSREF_LOG_RETURN_NOT_OK_MAC(rc);
  }
  
  rc = vxLanIPv6ConfigureSequence(devNum);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  return GT_OK;
}

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
)
{
  GT_STATUS rc = GT_OK;

  /* Clear MAC2ME address, which is must for trigerring VxLAN processing of a packet. */
  rc = csRefVpnMac2MeDelete(devNum);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  /* Remove reference to ePorts, from VSI domain L2-MLL, and delete the eVlan.  */
  for( prvVpnCurrAvaiableEvid-- ; prvVpnCurrAvaiableEvid > VPN_EVID_FIRST_ENTRY_CNS ; prvVpnCurrAvaiableEvid--)
  {
    rc = csRefInfraVsiDomain(devNum, prvVpnCurrAvaiableEvid, VPN_VSI_DB_OPERATION_DELETE_E,
                                        0xFFFFFF/*dummy*/, 0xFFFFFF/*dummy*/);
    CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  }

  /* Remove UNI/NNI interface configuration. */
  rc = csRefVpnConfigClean(devNum, VPN_EPORT_FIRST_ENTRY_CNS, VPN_EPORT_NUM_OF_ENTRIES_CNS, prvVpnPhysicalEportBmp);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);


  /* Must be at the end. */
  /* eArch element DB reset. */
  rc = prvVxLanRelateDBInit(devNum, GT_FALSE);
  CSREF_LOG_RETURN_NOT_OK_MAC(rc);

  return GT_OK;
}

