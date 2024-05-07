/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefMEF_10_3.c
*
* DESCRIPTION:
*  This files provide an example of policing traffic with MEF_10.3 with flow-id (using IPCL rule).
*  The IPCL rule provides a match based on a D-MAC given by the user.
*  The example provides the user the ability to choose an envelope of different sizes n (2<=n<= 8) with two different modes:
*  1. Sequential Envelope Organization, for CF0=0
*  2. Interlaced Envelope Organization, for CF0=1. This mode is not trivial since the locations in the memory of the maxRate tables are not corresponded with the locations 
*     of the rates (in the metering table). This example modifies the values according to the input of the user.
*
*  How to run: Use MEF_10_3script.txt
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

#define MEF_10_3_MAX_ENVELOPE_SIZE     8
#define MEF_10_3_ENVELOPE_NUMBER       4
#define MEF_10_3_MAX_RATE_ENTRIES_BASE (128 - MEF_10_3_MAX_ENVELOPE_SIZE*MEF_10_3_ENVELOPE_NUMBER) /* Last 32 (4*8) entries are used for envelope. */

#define MEF_10_3_BUCKETS_NUM           2
#define MEF_10_3_MAX_BUCKET_SIZE       (MEF_10_3_BUCKETS_NUM * MEF_10_3_MAX_ENVELOPE_SIZE)  /* up to 8 entries, 2 buckets in each. */
#define MEF_10_3_BUCKET_TYPES_NUMBER   3   /* maxRate, rate ,burstSize - for each bucket. */

static CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT prvMef_10_3_green_cmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
static CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT prvMef_10_3_yellow_cmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
static CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT prvMef_10_3_red_cmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;

static GT_BOOL prvMef10_3_initDone = GT_FALSE;
#define CPSS_MEF_10_3_INIALIZED {if(GT_FALSE == prvMef10_3_initDone) return GT_NOT_INITIALIZED;}

/************************** Function Declarations *************************/
static GT_STATUS prvGetEnvelopeSize(GT_U32 envelopeId);
static void prvPrintMeteringAndMaxRateEntries(CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryArr);

/***************************** Policer Structures ****************************/
typedef struct
{
    GT_BOOL   valid; /* Entry contains valid information */
    GT_U32    cir;
    GT_U32    cirMax;
    GT_U32    cbs;
    GT_U32    eir;
    GT_U32    eirMax;
    GT_U32    ebs;
    CPSS_POLICER_COLOR_MODE_ENT  colorMode;/* color aware or colorBlind */
    GT_BOOL   couplingFlag;
}MEF_SERVICE_BANDWIDTH_PROFILE_STC;

typedef struct
{
  /* Entry index is the rank of a bandwidthProfile within an envelope. */
  MEF_SERVICE_BANDWIDTH_PROFILE_STC  bandwidthProfileFlow[MEF_10_3_MAX_ENVELOPE_SIZE];
  GT_BOOL        couplingFlag0;
}MEF_10_3_SERVICE_ENVELOPE_STC;

static MEF_10_3_SERVICE_ENVELOPE_STC prvCsRefEnvelopeConfiguration[MEF_10_3_ENVELOPE_NUMBER];
static GT_U32  maxRateInfo[MEF_10_3_MAX_BUCKET_SIZE][MEF_10_3_BUCKET_TYPES_NUMBER];
static GT_U32  envelopeSize = 0;


/****************************************************************************/
/************************** Main Functions *************************/
/****************************************************************************/

/**
* @internal csRefMef10_3_Init function
* @endinternal
*
* @brief   This function initialize both IPCL and Policer init configuration
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] portNum          - port number where sample packet enters
* @param[in] prvFlowBaseMaxMeterIndex - this threshold value determines if the metering entries will be flow-based or priority-based.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_3_Init
(
  GT_U8        devNum,
  GT_PORT_NUM  portNum,
  GT_U32       prvFlowBaseMaxMeterIndex
)
{
    GT_STATUS  rc = GT_OK;

    if(prvMef10_3_initDone == GT_TRUE)
    {
      osPrintf("Function %s: Already initialized!\n",__FUNCNAME__);
      return GT_ALREADY_EXIST;
    }

    /* Init Policer configurations */
    CPSS_CALL(csRefMefUtilQosHwPolicerInit(devNum));

    /* Init Policer configurations */
    CPSS_CALL(csRefMefUtilPclInit(devNum));

    osPrintf("--- cpssDxChPolicerMeterTableFlowBasedIndexConfigSet(): Set The envelope flow base index value to: %d.\n", prvFlowBaseMaxMeterIndex);
    CPSS_CALL(cpssDxChPolicerMeterTableFlowBasedIndexConfigSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                             prvFlowBaseMaxMeterIndex, CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_8_E));

    osMemSet(&prvCsRefEnvelopeConfiguration, 0, sizeof(prvCsRefEnvelopeConfiguration));

    /* Init the IPCL for the ingress port */
    CPSS_CALL(csRefMefUtilPClIngressPortInit(devNum, portNum));

    prvMef10_3_initDone = GT_TRUE;

    return rc;
}


/**
* @internal csRefMef10_3_PClRuleSet_str function
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
GT_STATUS csRefMef10_3_PClRuleSet_str
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

  CPSS_MEF_10_3_INIALIZED;

  /* Destination MAC address to trigger. */
  CPSS_CALL(csRefInetStringToNet(INET_ADDRESS_MAC, daMacDaStr, macAddrTrigger.arEther));
  CPSS_CALL(csRefMefUtilPClRuleSet(devNum, &macAddrTrigger, pclEntryIndex, egressPortNum, meterId, pktCmd));

  return GT_OK;
}


/**
* @internal csRefMef10_3_bandwidthProfileFlowConfig function
* @endinternal
*
* @brief   This function receives from the users envelope parameters, according to the MEF-10.3 Standart and stored in a designated array to be used in the apply stage.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] envelopeId            - envelope id.
* @param[in] bandwidthProfileRank  - the index of the array to fill - should be equivalent to the rank of the entry (0-7).
* @param[in] cir     - the CIR value for the entry.
* @param[in] cirMax  - the cirMax value for the entry.
* @param[in] cbs     - the cbs value for the entry.
* @param[in] eir     - the eir value for the entry.
* @param[in] eirMax  - the eirMax value for the entry.
* @param[in] ebs     - the ebs value for the entry.
* @param[in] colorMode    - determine if the entry is color aware or not.
* @param[in] couplingFlag - the coupling mode of the entry (0/1).
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_3_bandwidthProfileFlowConfig
(
    GT_U32    envelopeId,
    GT_U32    bandwidthProfileRank,
    GT_U32    cir,        /* Kbps (1K = 1000) */
    GT_U32    cirMax,     /* Kbps (1K = 1000) */
    GT_U32    cbs,        /* bytes    */
    GT_U32    eir,        /* Kbps (1K = 1000) */
    GT_U32    eirMax,     /* Kbps (1K = 1000) */
    GT_U32    ebs,        /*  bytes   */
    CPSS_POLICER_COLOR_MODE_ENT  colorMode,/* color aware(1) or colorBlind(0) */
    GT_BOOL   couplingFlag
)
{

  CPSS_MEF_10_3_INIALIZED;

  if(MEF_10_3_ENVELOPE_NUMBER   <= envelopeId)
  {
      return GT_BAD_PARAM;
  }

  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].cir          = cir;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].cirMax       = cirMax;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].cbs          = cbs;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].eir          = eir;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].eirMax       = eirMax;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].ebs          = ebs;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].colorMode    = colorMode;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].couplingFlag = couplingFlag;
  prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[bandwidthProfileRank].valid        = GT_TRUE;

  return GT_OK;
}



/**
* @internal csRefMef10_3_EnvelopeConfigApply function
* @endinternal
*
* @brief   This function configures the envelope parameters in both the metering table and the max-rate table. It uses the global array prvCsRefEnvelopeConfiguration[] which
*          holds the envelope info. for the sequential mode(CF0=0), it is required to relocate the values of the max-rate table so it will be aligned with the values of the 
*          metering entry.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum         - device number.
* @param[in] envelopeId     - envelope id.
* @param[in] meterId        - the index of the envelope in the metering table. Should be equivalant to the policy-id value that was set in the IPCL rule for the first(!) entry.
* @param[in] couplingFlag0  - the coupling mode of the entire envelope. 0- sequential mode. 1- interlace mode.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_3_EnvelopeConfigApply
(
    GT_U8     devNum,
    GT_U32    envelopeId,
    GT_U32    meterId,
    GT_BOOL   couplingFlag0
)
{
  GT_STATUS                                rc = GT_OK;
  GT_U32                                   startEntryIndex, entryIndex;
  GT_U32                                   cir,cbs,cirMax,eir,ebs,eirMax;
  CPSS_DXCH3_POLICER_METERING_ENTRY_STC    entryArr[8];
  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT   cpssOutArr[8];
  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC     cpssBilling;
  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         cpssPolicerStage;   
  /* up to 8 entries, 2 buckets in each. maxRate[0],rate[1],burstSize[2] for each bucket. */
  GT_U32                                   maxRateEntryBaseIndex = MEF_10_3_MAX_RATE_ENTRIES_BASE + (envelopeId*MEF_10_3_MAX_ENVELOPE_SIZE);

  CPSS_MEF_10_3_INIALIZED;
  
  if(MEF_10_3_ENVELOPE_NUMBER   <= envelopeId)
  {
      return GT_BAD_PARAM;
  }

  cpssPolicerStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;

  osMemSet(&maxRateInfo, 0, sizeof(maxRateInfo));
  osMemSet(&entryArr, 0, sizeof(entryArr));
  osMemSet(&cpssOutArr, 0, sizeof(cpssOutArr));
  startEntryIndex = meterId; /*EnvelopeIndex */

  CPSS_CALL(prvGetEnvelopeSize(envelopeId));

  for (entryIndex = 0; entryIndex < envelopeSize; entryIndex++ )
  {
    cir = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].cir;
    cbs = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].cbs;
    ebs = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].ebs;
    eir = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].eir;
    cirMax = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].cirMax;
    eirMax = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].eirMax;

    /* MEF envelope index programmed in h/w will be based on the h/w meter index of meter entry with highest rank (first element) */
    entryArr[entryIndex].meterMode = (entryIndex == 0) ? 
    CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E : CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E;  /* MEF_10.3 Mode */
    entryArr[entryIndex].mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
    entryArr[entryIndex].meterColorMode = prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].colorMode;
    entryArr[entryIndex].greenPcktCmd  = prvMef_10_3_green_cmd;
    entryArr[entryIndex].yellowPcktCmd = prvMef_10_3_yellow_cmd;
    entryArr[entryIndex].redPcktCmd    = prvMef_10_3_red_cmd;
    entryArr[entryIndex].modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E;
    entryArr[entryIndex].remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L2_E;
    entryArr[entryIndex].byteOrPacketCountingMode = CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E;
    entryArr[entryIndex].packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    entryArr[entryIndex].tokenBucketParams.envelope.cir = cir;
    entryArr[entryIndex].tokenBucketParams.envelope.cbs = cbs;
    entryArr[entryIndex].tokenBucketParams.envelope.eir = eir;
    entryArr[entryIndex].tokenBucketParams.envelope.ebs = ebs;
    entryArr[entryIndex].countingEntryIndex = meterId;

    /* If couplingFlag0 == 1 all other CF should be 0 */
    entryArr[entryIndex].couplingFlag =
        (couplingFlag0 == GT_TRUE) ? GT_FALSE :
        prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].couplingFlag;

    /* Set CIRmax and EIRmax of all entries of the envelope. */
    if(couplingFlag0 == GT_TRUE)
    {
      maxRateInfo[envelopeSize-1-entryIndex][0] = cirMax;
      maxRateInfo[envelopeSize-1-entryIndex][1] = cir;
      maxRateInfo[envelopeSize-1-entryIndex][2] = cbs;
      maxRateInfo[2*envelopeSize-1-entryIndex][0] = eirMax;
      maxRateInfo[2*envelopeSize-1-entryIndex][1] = eir;
      maxRateInfo[2*envelopeSize-1-entryIndex][2] = ebs;
    }
    else
    {
      maxRateInfo[2*entryIndex][0]   = cirMax;
      maxRateInfo[2*entryIndex][1]   = cir;
      maxRateInfo[2*entryIndex][2]   = cbs;
      maxRateInfo[2*entryIndex+1][0] = eirMax;
      maxRateInfo[2*entryIndex+1][1] = eir;
      maxRateInfo[2*entryIndex+1][2] = ebs;
    }
  }

  /* Set the MaxRate Table in the same locations as the correspond rate values of the metering entries */
  for (entryIndex = 0; entryIndex < envelopeSize; entryIndex++)
  {
    if(couplingFlag0 == GT_TRUE)
    {
      entryArr[entryIndex].maxRateIndex = maxRateEntryBaseIndex + (envelopeSize-1-entryIndex);
    }
    else
    {
      entryArr[entryIndex].maxRateIndex = maxRateEntryBaseIndex + entryIndex;
    }

    CPSS_CALL(cpssDxChPolicerTokenBucketMaxRateSet(devNum, cpssPolicerStage, maxRateEntryBaseIndex + entryIndex, 0, 
                    maxRateInfo[2*entryIndex][1], maxRateInfo[2*entryIndex][2], maxRateInfo[2*entryIndex][0]));
    CPSS_CALL(cpssDxChPolicerTokenBucketMaxRateSet(devNum, cpssPolicerStage, maxRateEntryBaseIndex + entryIndex, 1,
                    maxRateInfo[2*entryIndex+1][1], maxRateInfo[2*entryIndex+1][2], maxRateInfo[2*entryIndex+1][0]));
  }

  prvPrintMeteringAndMaxRateEntries(entryArr);
  
  CPSS_CALL(cpssDxChPolicerMeteringEntryEnvelopeSet(devNum, cpssPolicerStage, startEntryIndex, envelopeSize, couplingFlag0, entryArr, cpssOutArr));

  /* Bind counting entry */
  for (entryIndex = 0; entryIndex < envelopeSize; entryIndex++ )
  {
    /*Filling Billing Counters for each Meter configured in MEF */
    osMemSet(&cpssBilling, 0, sizeof (cpssBilling));
    cpssBilling.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
    cpssBilling.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    CPSS_CALL(cpssDxCh3PolicerBillingEntrySet(devNum, cpssPolicerStage, meterId, &cpssBilling));
  }

  return rc;
}


/**
* @internal csRefMef10_3_set_color_mode function
* @endinternal
*
* @brief   This function sets the envelope's color commands. To apply these changes, csRefMef10_3_EnvelopeConfigApply must be called after.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] green_mode   - sets the green color mode.
* @param[in] yellow_mode  - sets the yellow color mode.
* @param[in] red_mode     - sets the red color mode.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMef10_3_set_color_mode
(
  CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT green_mode,
  CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT yellow_mode,
  CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT red_mode
)
{
  prvMef_10_3_green_cmd = green_mode;
  prvMef_10_3_yellow_cmd = yellow_mode;
  prvMef_10_3_red_cmd = red_mode;

  osPrintf( "---csRefMef10_3_set_color_mode():\n");
  osPrintf( "   prvMef_10_3_green_cmd: %d\n", prvMef_10_3_green_cmd);
  osPrintf( "   prvMef_10_3_yellow_cmd: %d\n", prvMef_10_3_yellow_cmd);
  osPrintf( "   prvMef_10_3_red_cmd: %d\n", prvMef_10_3_red_cmd);
  osPrintf( "   Run csRefMef10_3_EnvelopeConfigApply to apply the changes.\n\n");

    return GT_OK;  
}


/****************************************************************************/
/************************** Local Functions *************************/
/****************************************************************************/

/**
* @internal prvGetEnvelopeSize function
* @endinternal
*
* @brief   This function calculates the envelope size.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] envelopeId   - the envelope id to be calculated.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS prvGetEnvelopeSize
(
  GT_U32 envelopeId
)
{
    int entryIndex;

    for (entryIndex = 0; entryIndex < MEF_10_3_MAX_ENVELOPE_SIZE; entryIndex++ )
    {
        if(prvCsRefEnvelopeConfiguration[envelopeId].bandwidthProfileFlow[entryIndex].valid != GT_TRUE)
        {
          /* Rank values must be consecutive from 1 to n, where n is number of entries in the enevlop.
             Therfore stop for first non valid entry. Ignoring next entries. */
          break;
        }
        envelopeSize ++;
    }
    if(envelopeSize < 2) /* an enevlope must contain 2 enetries at least. */
    {
        osPrintf( "\n Envelope %d contains no information.\n", envelopeId);
        return GT_BAD_SIZE;
    }

    return GT_OK;
}

/**
* @internal prvPrintMeteringAndMaxRateEntries function
* @endinternal
*
* @brief   This function prints the envelope values by the tables that hold those values: the metering table and the max-rate table.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] entryArr   - the struct array holding the envelope info.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
static void prvPrintMeteringAndMaxRateEntries
(
  CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryArr
)
{
  int    tableToPrint;
  GT_U32 entryIndex;

  for (tableToPrint=1; tableToPrint>= 0; tableToPrint--)
  {
    if (tableToPrint == 0)
    {
      osPrintf( "\nEnvelope Max Rate Table:\n");
      osPrintf( " Bucket-0 Bucket-1 Index\n");

      for (entryIndex = 0; entryIndex < envelopeSize; entryIndex++ )
      {
        osPrintf("%8d %8d %4d\n", maxRateInfo[2*entryIndex][tableToPrint], maxRateInfo[2*entryIndex+1][tableToPrint], entryArr[entryIndex].maxRateIndex);
      }
    }
    else
    {
      osPrintf( "\nEnvelope Rate Table:\n");
      osPrintf( " Bucket-0 Bucket-1\n");
      for (entryIndex = 0; entryIndex < envelopeSize; entryIndex++ )
      {
        osPrintf("%8d %8d\n", maxRateInfo[2*entryIndex][tableToPrint], maxRateInfo[2*entryIndex+1][tableToPrint]);
      }
    }
  }
}
