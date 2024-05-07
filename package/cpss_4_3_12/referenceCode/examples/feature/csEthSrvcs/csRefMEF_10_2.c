/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefMEF_10_2.c
*
* DESCRIPTION:
*  This files provide an example of policing traffic with MEF_10.2 with flow-id (using IPCL rule).
*  The IPCL rule provides a match based on a D-MAC given by the user
*
*  How to run: Use MEF_10_2script.txt  
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "csRefMEFUtils.h"
#include "../../../../cpssEnabler/mainOs/h/gtOs/cpssOs.h"
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicerLog.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicer.h>

static GT_BOOL prvMef10_2_initDone = GT_FALSE;
#define CPSS_MEF_10_2_INIALIZED {if(GT_FALSE == prvMef10_2_initDone) return GT_NOT_INITIALIZED;}


/****************************************************************************/
/************************** Main Functions *************************/
/****************************************************************************/

/**
* @internal csRefMef10_2_Init function
* @endinternal
*
* @brief   This function initialize both IPCL and Policer init configuration
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
* @param[in] portNum          - port number where sample packet enters
* @param[in] prvFlowBaseMaxMeterIndex - this threshold value determines if the metering entries will be flow-based or priority-based.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_2_Init
(
  GT_U8        devNum,
  GT_PORT_NUM  portNum
)
{
    GT_STATUS  rc = GT_OK;

    if(prvMef10_2_initDone == GT_TRUE)
    {
      osPrintf("[%s:%d]: Already initialized!\n",__FUNCNAME__, __LINE__);
      return GT_ALREADY_EXIST;
    }

    CPSS_CALL(csRefMefUtilQosHwPolicerInit(devNum));
    CPSS_CALL(csRefMefUtilPclInit(devNum));
    CPSS_CALL(csRefMefUtilPClIngressPortInit(devNum, portNum));

    prvMef10_2_initDone = GT_TRUE;

    return rc;
}


/**
* @internal csRefMef10_2_PClRuleSet_str function
* @endinternal
*
* @brief   This function receives a MAC address by a string, converts it into a struct and calls the function csRefMefUtilPClRuleSet with the reset of the arguments.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
* @param[in] daMacDaStr       - MAC address as string input from the user.
* @param[in] pclEntryIndex    - the PCL rule index.
* @param[in] egressPortNum    - an egress port to be redirected as part of the IPCL action field.
* @param[in] meterId          - The index of the metering table that will represent the first entry of the envelope.
* @param[in] pktCmd           - the packet command action to be set in the IPCL action field.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_2_PClRuleSet_str
(
  GT_U8         devNum,
  char         *daMacDaStr,       /* Packet's MAC-DA in string format. */
  GT_U32        pclEntryIndex,
  GT_PORT_NUM   egressPortNum,    /* Egress port */
  GT_U32        meterId,
  CPSS_PACKET_CMD_ENT pktCmd
)
{
  GT_STATUS     rc = GT_OK;
  GT_ETHERADDR  macAddrTrigger;

  CPSS_MEF_10_2_INIALIZED

  /* Destination MAC address to trigger. */
  CPSS_CALL(csRefInetStringToNet(INET_ADDRESS_MAC, daMacDaStr, macAddrTrigger.arEther));
  CPSS_CALL(csRefMefUtilPClRuleSet(devNum, &macAddrTrigger, pclEntryIndex, egressPortNum, meterId, pktCmd));

  return GT_OK;
}


/**
* @internal csRefMef10_2_DecoupledApply function
* @endinternal
*
* @brief   This function configures the MEF-10.2 policing algorithm values and create it in the metering table.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum        - device number.
* @param[in] meterId       - the index to entry in the metering table.
* @param[in] isColorAware  - 0- Not color aware. 1- Color aware.
* @param[in] cirVal        -  the cir value for green packets.
* @param[in] eirVal        - the eir value for the yellow packets.
* @param[in] cbsVal        - the cbs value.
* @param[in] pbsVal        - the pbs value.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_2_DecoupledApply
(
    GT_U8     devNum,
    GT_U32    meterId,
    GT_BOOL   isColorAware,
    GT_U32    cirVal,
    GT_U32    eirVal,
    GT_U32    cbsVal,
    GT_U32    pbsVal
)
{
    GT_STATUS                                rc = GT_OK;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC    meterEntry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT   meterEntryOut;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC     billingEntry;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         cpssPolicerStage;

    CPSS_MEF_10_2_INIALIZED;

    cpssPolicerStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

    osMemSet(&meterEntry, 0, sizeof(meterEntry));
    osMemSet(&billingEntry, 0, sizeof (billingEntry));
    osMemSet(&meterEntryOut, 0, sizeof(meterEntryOut));

    /* Configure the meter entry struct */
    meterEntry.byteOrPacketCountingMode = CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E;
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    meterEntry.remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L2_E;
    meterEntry.mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
    meterEntry.meterMode = CPSS_DXCH3_POLICER_METER_MODE_MEF0_E;
    meterEntry.meterColorMode = isColorAware == GT_TRUE ? CPSS_POLICER_COLOR_AWARE_E : CPSS_POLICER_COLOR_BLIND_E ;
    meterEntry.greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meterEntry.yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meterEntry.redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
    meterEntry.modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E;
    meterEntry.qosProfile = 0;
    meterEntry.tokenBucketParams.trTcmParams.cir = cirVal;  /* Input Kbps       */
    /* Setting the CBS and EBS is Burst size + Policer MRU(10240) */
    meterEntry.tokenBucketParams.trTcmParams.cbs = cbsVal;
    meterEntry.tokenBucketParams.trTcmParams.pir = eirVal;
    meterEntry.tokenBucketParams.trTcmParams.pbs = pbsVal;
    meterEntry.modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
    meterEntry.countingEntryIndex = meterId;

  /* Configure the billing entry struct */
    billingEntry.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
    billingEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    billingEntry.greenCntr.l[0] = 0x0;
    billingEntry.greenCntr.l[1] = 0x0;
    billingEntry.yellowCntr.l[0] = 0x0;
    billingEntry.yellowCntr.l[1] = 0x0;
    billingEntry.redCntr.l[0] = 0x0;
    billingEntry.redCntr.l[1] = 0x0;

    CPSS_CALL(cpssDxCh3PolicerBillingEntrySet(devNum, cpssPolicerStage, meterId, &billingEntry));
    CPSS_CALL(cpssDxCh3PolicerMeteringEntrySet(devNum, cpssPolicerStage,  meterId, &meterEntry, &meterEntryOut));
    
    return rc;
}
