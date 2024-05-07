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
* @file cpssDxChPtpManager.c
*
* @brief Precision Time Protocol Manager Support - function implementations.
*
* @version   1
********************************************************************************
*/
#include <math.h>

#define CPSS_LOG_IN_MODULE_ENABLE

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtpLog.h>
#include <cpss/dxCh/dxChxGen/ptpManager/private/prvCpssDxChPtpManagerLog.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include<cpss/dxCh/dxChxGen/phy/prvCpssDxChPhyMpdInit.h>

/* TAI Iterator LOOP Header */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum,taiNumber,taiIdPtr)    \
    while (numIterations > 0)                                                               \
    {                                                                                       \
        if (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)                                    \
        {                                                                                   \
             switch (PRV_CPSS_PP_MAC(devNum)->devType)                                      \
             {                                                                              \
                case CPSS_AC5Y_DEVICES_CASES_MAC:                                           \
                    numIterations = 2; /* TAI0 & TAI4 */                                    \
                    break;                                                                  \
                                                                                            \
                default:                                                                    \
                    numIterations = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais;\
                    break;                                                                  \
            }                                                                               \
            (taiIdPtr)->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;                           \
            taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;                                       \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            switch (PRV_CPSS_PP_MAC(devNum)->devType)                                       \
            {                                                                               \
                case CPSS_AC5Y_DEVICES_CASES_MAC:                                           \
                    if ((taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_0_E) && (taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_4_E))\
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);      \
                    break;                                                                  \
                                                                                            \
                default:                                                                    \
                    /* do nothing */                                                        \
                    break;                                                                  \
            }                                                                               \
            (taiIdPtr)->taiNumber = taiNumber;                                              \
        }                                                                                   \

/* TAI Iterator LOOP footer */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, taiIdPtr)            \
        numIterations--;                                                                    \
        switch (PRV_CPSS_PP_MAC(devNum)->devType)                                           \
        {                                                                                   \
            case CPSS_AC5Y_DEVICES_CASES_MAC:                                               \
                taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_4_E;                                   \
                break;                                                                      \
                                                                                            \
            default:                                                                        \
                taiNumber++;                                                                \
                break;                                                                      \
        }                                                                                   \
        (taiIdPtr)->taiNumber = taiNumber;                                                  \
    }

/* check tai number */
#define PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(_dev, _taiNumber)                               \
    switch (PRV_CPSS_PP_MAC(devNum)->devType)                                                   \
    {                                                                                           \
        case CPSS_AC5Y_DEVICES_CASES_MAC:                                                       \
            if ((_taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_ALL_E) && (_taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_0_E) && (_taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_4_E))\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                  \
            break;                                                                              \
                                                                                                \
        default:                                                                                \
            /* do noting */                                                                     \
            break;                                                                              \
    }                                                                                           \
    if (_taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)                                           \
    {                                                                                           \
        /* do nothing */                                                                        \
    }                                                                                           \
    else if (_taiNumber >= PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.ptpInfo.maxNumOfTais)             \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
    }                                                                                           \

/* wether to skip this port */
static GT_BOOL prvCpssDxChPortSkipCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_BOOL isCpu, isValid;
    GT_STATUS rc,rc1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if ((portNum) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
            return GT_TRUE; /* no more */
        rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
        rc1 = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
        if((rc != GT_OK) || (isCpu == GT_TRUE) ||(rc1 != GT_OK) || (isValid != GT_TRUE))
            return GT_TRUE;
    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_NOT_EXISTS_E)
            return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal prvCpssDxChPtpManagerPortTimestampingModeDefaultConfig function
* @endinternal
*
* @brief   Configure default Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPtpManagerPortTimestampingModeDefaultConfig
(
    IN  GT_SW_DEV_NUM                        devNum
)
{
    GT_U32 i;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    /*avoid memory leak ,release memory if already allocated*/
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr);

    /* DB allocation */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr = (PRV_CPSS_DXCH_PTP_MANAGER_PTP_TIMESTAMPING_OPERATION_MODE_STC*)
        cpssOsMalloc(PRV_CPSS_PP_MAC(devNum)->numOfPorts*sizeof(PRV_CPSS_DXCH_PTP_MANAGER_PTP_TIMESTAMPING_OPERATION_MODE_STC));

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* Shadow of PTP Delay Values per port speed/interface/fec */
    for (i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[i].tsMode = CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E;
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[i].operationMode = CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] ptpRefClockConfPtr       - (pointer to) PTP reference clock configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*                                       The command is configured in cpssDxChPtpManagerPortCommandAssignment
*                                       per{source port, domain index, message type}.
*                                       Applicable if command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E or
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_REF_CLOCK_CFG_STC          *ptpRefClockConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumber;
    GT_U32 ptpClkInKhz;
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT pClkSelMode;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC todStep;
    GT_U32 numIterations=1,i,j,k,m,offset;
    const CPSS_DXCH_PTP_MANAGER_DB_DELAYS_STC *delayArrPtr = NULL;
    GT_U32 iterNum = 0,ii,numOfPorts;
    GT_BOOL isLinkUp,isIronmanM;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue,triggerTime,maskValue;
    /* internal phy support */
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(outputInterfaceConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpOverEthernetConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpOverIpUdpConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpTsTagGlobalConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ptpRefClockConfPtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,outputInterfaceConfPtr->taiNumber);

    /* set default values in DB */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
    {
        /*avoid memory leak ,release memory if already allocated*/
        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr);

        /* allocation of physical ports information - phyPortInfoArray */
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr = (PRV_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC*)
            cpssOsMalloc(PRV_CPSS_PP_MAC(devNum)->numOfPorts*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E*sizeof(PRV_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC));

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        switch(PRV_CPSS_PP_MAC(devNum)->devType)
        {
        case CPSS_IRONMAN_M_DEVICES_CASES_MAC:
            isIronmanM = GT_TRUE;
            break;
        default:
            isIronmanM = GT_FALSE;
        }

        /* Shadow of PTP Delay Values per port speed/interface/fec */
        for (i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
        {
            for (j = 0; j < CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E; j++)
            {
                for (k = 0; k < CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E; k++)
                {
                    for (m = 0; m < CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E; m++)
                    {
                        offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].egressPipeDelayCtsu = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelayEncrypted = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelayFsuOffset = 0;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = 0;
                    }
                }
            }
        }

        if (isIronmanM)
        {
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPhyPortArr != NULL)
            {
                delayArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPhyPortArr;
                iterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPhyPortArrNumElements;
                for (ii = 0; ii < iterNum; ii++)
                {
                    /* ports that are connected to the internal phy */
                    for (i = 0; i < 8; i++)
                    {
                        j = delayArrPtr[ii].speed;
                        k = delayArrPtr[ii].intMode;
                        m = delayArrPtr[ii].fecMode;

                        offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayArrPtr[ii].egressPipeDelay;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayArrPtr[ii].ingressPortDelay;
                    }
                }
            }
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArr != NULL)
            {
                delayArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArr;
                iterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArrNumElements;
                for (ii = 0; ii < iterNum; ii++)
                {
                    /* ports that are connected to SFP + CPU port */
                    for (i = 8; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
                    {
                        j = delayArrPtr[ii].speed;
                        k = delayArrPtr[ii].intMode;
                        m = delayArrPtr[ii].fecMode;

                        offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayArrPtr[ii].egressPipeDelay;
                        PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayArrPtr[ii].ingressPortDelay;
                    }
                }
            }
        }
        else if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArr != NULL)
        {
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
                numOfPorts = 48; /* USX MAC 0..47 */
            else
                numOfPorts = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

            delayArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArr;
            iterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayPortArrNumElements;
            for (ii = 0; ii < iterNum; ii++)
            {
                for (i = 0; i < numOfPorts; i++)
                {
                    j = delayArrPtr[ii].speed;
                    k = delayArrPtr[ii].intMode;
                    m = delayArrPtr[ii].fecMode;

                    offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayArrPtr[ii].egressPipeDelay;
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayArrPtr[ii].ingressPortDelay;
                }
            }
        }

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayCpuPortArr != NULL)
        {
            delayArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayCpuPortArr;
            iterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delayCpuPortArrNumElements;
            for (ii = 0; ii < iterNum; ii++)
            {
                for (i = 48; i <= 49; i++) /* CPU MAC 48..49 */
                {
                    j = delayArrPtr[ii].speed;
                    k = delayArrPtr[ii].intMode;
                    m = delayArrPtr[ii].fecMode;

                    offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayArrPtr[ii].egressPipeDelay;
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayArrPtr[ii].ingressPortDelay;
                }
            }
        }

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delay100GPortArr != NULL)
        {
            delayArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delay100GPortArr;
            iterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.delay100GPortArrNumElements;
            for (ii = 0; ii < iterNum; ii++)
            {
                for (i = 50; i <= 53; i++) /* 100G MAC 50..53 */
                {
                    j = delayArrPtr[ii].speed;
                    k = delayArrPtr[ii].intMode;
                    m = delayArrPtr[ii].fecMode;

                    offset = i*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             j*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                             k*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + m;

                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayArrPtr[ii].egressPipeDelay;
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayArrPtr[ii].ingressPortDelay;
                }
            }
        }

        /* additional setting for Ironman-M internal PHY */
        if (isIronmanM)
        {
            /* sequence to sync between switch TAI4 (free-running) and internal PHY */

            /* Set TAI4 Output interface configuration to pulse */
            rc = cpssDxChPtpTaiOutputInterfaceSet(devNum,CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E,CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E,CPSS_DXCH_PTP_TAI_NUMBER_4_E);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsMemSet(&triggerTime, 0, sizeof(triggerTime));
            triggerTime.nanoSeconds = 500000000; /* 500 milisec */
            maskValue.fracNanoSeconds = 0x0;
            maskValue.nanoSeconds = 0xFFFFFFFF;
            maskValue.seconds.l[0] = 0xFFFF;
            maskValue.seconds.l[1] = 0xFFFF;

            /* Generates a trigger on the PTP_PULSE_OUT interface at a specified time */
            rc = cpssDxChPtpManagerTriggerGenerate(devNum,CPSS_DXCH_PTP_TAI_NUMBER_4_E,0xFF,&triggerTime,&maskValue);

            cpssOsMemSet(&opParams, 0, sizeof(opParams));
            opParams.phyPtpInit.reason = MPD_PTP_LOAD_TIME_FOR_PULSE_E;
            opParams.phyPtpInit.loadTime.nanoSeconds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.phyDelayInfo.phyPulseLinkDelay; /* HW dealy */
            opParams.phyPtpInit.loadTime.nanoSeconds += triggerTime.nanoSeconds; /* SW delay */

            /* In IM-M only single PHY1781; init phy with pulse delay time */
            ret = mpdPerformOperationOnPhy(0 /* single internal phy number */, MPD_OP_PHY_CODE_SET_PTP_INIT_E, &opParams);
            if (MPD_OK_E != ret)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

             /* Set a new TOD Value '0' for TAI4 */
            cpssOsMemSet(&todValue, 0, sizeof(todValue));
            rc = cpssDxChPtpManagerTodValueSet(devNum,CPSS_DXCH_PTP_TAI_NUMBER_4_E,&todValue,GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* wait according to time that the pulse was accepted in MPD .*/
            cpssOsTimerWkAfter(900);

            rc = cpssDxChPtpTaiOutputInterfaceSet(devNum,CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E,CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E,CPSS_DXCH_PTP_TAI_NUMBER_4_E);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPtpManagerMasterPpsActivate(devNum,CPSS_DXCH_PTP_TAI_NUMBER_4_E,PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.phyDelayInfo.phyPpsLinkDelay,1,0);
            if (rc != GT_OK)
            {
                return rc;
            }

            triggerTime.seconds.l[0] = 1;

            cpssOsMemSet(&opParams, 0, sizeof(opParams));
            opParams.phyPtpInit.reason = MPD_PTP_LOAD_TIME_FOR_PPS_E;
            opParams.phyPtpInit.loadTime.nanoSeconds = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.phyDelayInfo.phyPpsLinkDelay;
            opParams.phyPtpInit.loadTime.lowSeconds = triggerTime.seconds.l[0];

            /* In IM-M only single PHY1781; init phy with pulse delay time */
            ret = mpdPerformOperationOnPhy(0 /* single internal phy number */, MPD_OP_PHY_CODE_SET_PTP_INIT_E, &opParams);
            if (MPD_OK_E != ret)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone = GT_TRUE;

        /* loop on all ports */
        for(ii = 0; ii < PRV_CPSS_PP_MAC(devNum)->numOfPorts; ii++)
        {
            if (prvCpssDxChPortSkipCheck(devNum,ii) || prvCpssDxChPortRemotePortCheck(devNum,ii))
                continue;

            rc = cpssDxChPortLinkStatusGet(devNum, ii, &isLinkUp);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (isLinkUp == GT_FALSE)
                continue;

            /* set delays in HW for ports with linkUp */
            rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum].ppCommonPortFuncPtrsStc).ppPortPtpParamsSetFunc(devNum,ii);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    taiNumber = outputInterfaceConfPtr->taiNumber;

    switch (outputInterfaceConfPtr->outputInterfaceMode)
    {
    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E;
        break;

    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E;
        break;

    case CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E:
        pClkSelMode = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ptpClkInKhz = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz;

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* select the PCLK source */
        rc = cpssDxChPtpTaiPClkSelectionModeSet(devNum,&taiId,pClkSelMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            if (outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E ||
                outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E)
            {
                /* set master device driving the PTP_PULSE */
                rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_EGRESS_E,taiId.taiNumber);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        if (outputInterfaceConfPtr->outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E)
        {
            if (outputInterfaceConfPtr->nanoSeconds != 0)
            {
                /* set PCLK cycle */
                rc = cpssDxChPtpTaiPClkCycleSet(devNum,&taiId,outputInterfaceConfPtr->nanoSeconds);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* Configure TAIs nanosec step values */
        todStep.nanoSeconds = 1000000/ptpClkInKhz;
        if (1000000.0/ptpClkInKhz - todStep.nanoSeconds > 0.5)
        {
            todStep.nanoSeconds++;
            /* NOTE: the casting to GT_32 is to solve known ARM issue: casting a negative float to an unsigned int returns '0' */
            todStep.fracNanoSeconds = (GT_32)((1000000.0/ptpClkInKhz - todStep.nanoSeconds) * 0x100000000);

            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                /* need to adjust fracNanoSec from 0xAAAAAAAB as the 'Drift compensation mechanism' is calculated based on 0xAAAAAAAA */
                todStep.fracNanoSeconds = 0xAAAAAAAA;
            }
        }
        else
        {
            /* NOTE: the casting to GT_U32 is o.k. when casting a positive float */
            todStep.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                    (1000000.0/ptpClkInKhz - todStep.nanoSeconds) +
                    (1000000.0/ptpClkInKhz - todStep.nanoSeconds));
        }

        rc = cpssDxChPtpTaiTodStepSet(devNum, &taiId, &todStep);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Configures input TAI clock's selection */
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            if (ptpRefClockConfPtr->refClockSource == CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E)
            {
                rc = cpssDxChPtpTaiInputClockSelectSet(devNum,taiId.taiNumber,CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E,CPSS_DXCH_PTP_25_FREQ_E /*dummy*/);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                rc = prvCpssDxChPtpSip6PLLConfig(devNum,CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E,CPSS_DXCH_PTP_25_FREQ_E,taiId.taiNumber);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            if (ptpRefClockConfPtr->refClockSource == CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E)
            {
                rc = cpssDxChPtpTaiInputClockSelectSet(devNum,taiId.taiNumber,CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E,CPSS_DXCH_PTP_25_FREQ_E /*dummy*/);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                rc = prvCpssDxChPtpSip6PLLConfig(devNum,ptpRefClockConfPtr->refClockSource,ptpRefClockConfPtr->refClockFrequency,taiId.taiNumber);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        /* AC5P & Harrier must set to PTP_PLL */
        else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            rc = prvCpssDxChPtpSip6PLLConfig(devNum,ptpRefClockConfPtr->refClockSource,ptpRefClockConfPtr->refClockFrequency,taiId.taiNumber);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* Ironman must set to CORE_PLL */
        else
        {
            /* nothing to do; Ironman is set to CORE_PLL */
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    /* configure the base of CPU code assigned to PTP packets */
    rc = cpssDxChPtpCpuCodeBaseSet(devNum,cpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable PTP over Ethernet frames identification */
    rc = cpssDxChPtpOverEthernetEnableSet(devNum,ptpOverEthernetConfPtr->ptpOverEthernetEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ptpOverEthernetConfPtr->ptpOverEthernetEnable == GT_TRUE)
    {
        /* set the PTP frame Ethertype0 */
        rc = cpssDxChPtpEtherTypeSet(devNum,0,ptpOverEthernetConfPtr->etherType0value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set the PTP frame Ethertype1 */
        rc = cpssDxChPtpEtherTypeSet(devNum,1,ptpOverEthernetConfPtr->etherType1value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable PTP over UDP rames identification */
    rc = cpssDxChPtpOverUdpEnableSet(devNum,ptpOverIpUdpConfPtr->ptpOverIpUdpEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ptpOverIpUdpConfPtr->ptpOverIpUdpEnable == GT_TRUE)
    {
        /* set UDP destination port0 */
        rc = cpssDxChPtpUdpDestPortsSet(devNum,0,ptpOverIpUdpConfPtr->udpPort0value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set UDP destination port1 */
        rc = cpssDxChPtpUdpDestPortsSet(devNum,1,ptpOverIpUdpConfPtr->udpPort1value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* select mode for handling the UDP checksum per packet type */
        rc = cpssDxChPtpTsUdpChecksumUpdateModeSet(devNum,&(ptpOverIpUdpConfPtr->udpCsUpdMode));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Avoid bypass all CTSUs */
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = cpssDxChPtpTsuBypassEnableSet(devNum,GT_FALSE);
    }

    /* set Timestamp Global Configuration */
    rc = cpssDxChPtpTsTagGlobalCfgSet(devNum,ptpTsTagGlobalConfPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* TOD step resolution is not fine enough to give a presice TOD. Need to configure TAI Drift compensation patch. */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        rc = prvCpssDxChPtpTaiDriftCompensationControlConfig(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPtpInit function
* @endinternal
*
* @brief  Set one-time global PTP-related settings.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] ptpRefClockConfPtr       - (pointer to) PTP reference clock configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*                                       The command is configured in cpssDxChPtpManagerPortCommandAssignment
*                                       per{source port, domain index, message type}.
*                                       Applicable if command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E or
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_REF_CLOCK_CFG_STC          *ptpRefClockConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPtpInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr, ptpTsTagGlobalConfPtr, ptpRefClockConfPtr, cpuCode));

    rc = internal_cpssDxChPtpManagerPtpInit(devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr,  ptpTsTagGlobalConfPtr, ptpRefClockConfPtr, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, outputInterfaceConfPtr, ptpOverEthernetConfPtr, ptpOverIpUdpConfPtr, ptpTsTagGlobalConfPtr, ptpRefClockConfPtr, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    if (inputMode == CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* select the triggering mode */
        rc = cpssDxChPtpTaiPulseInModeSet(devNum,taiId.taiNumber,inputMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set slave device reacting to an incoming PTP_PULSE */
        rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_INGRESS_E,taiId.taiNumber);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerInputPulseSet function
* @endinternal
*
* @brief   Set input pulse configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] inputMode                - Whether an incoming pulse comes from the PULSE pin (slave PPS) or the CLK pin.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,
*                                           CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerInputPulseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, inputMode));

    rc = internal_cpssDxChPtpManagerInputPulseSet(devNum, taiNumber, inputMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, inputMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable PPS output */
        rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger a signal at a specific nanosecond following a whole second */
        rc = cpssDxChPtpTaiClockGenerationEdgeSet(devNum, &taiId, phaseValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set PPS frequency */
        rc = cpssDxChPtpTaiClockCycleSet(devNum,&taiId,seconds,nanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerMasterPpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Master PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerMasterPpsActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, phaseValue, seconds, nanoSeconds));

    rc = internal_cpssDxChPtpManagerMasterPpsActivate(devNum, taiNumber, phaseValue, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, phaseValue, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable PPS input */
        rc = cpssDxChPtpTaiClockReceptionModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger a signal at a specific nanosecond following a whole second */
        rc = cpssDxChPtpTaiClockReceptionEdgeSet(devNum, &taiId, phaseValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set PPS frequency */
        rc = cpssDxChPtpTaiClockCycleSet(devNum,&taiId,seconds,nanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerSlavePpsActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave PPS.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] phaseValue               - rising phase value time of generation clock, in nanoseconds.
* @param[in] seconds                  - seconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] nanoSeconds              - nanoseconds part of PPS Period time.
*                                      (APPLICABLE RANGES: 0..2^28-1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerSlavePpsActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, phaseValue, seconds, nanoSeconds));

    rc = internal_cpssDxChPtpManagerSlavePpsActivate(devNum, taiNumber, phaseValue, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, phaseValue, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI and enabling TSU for selected port.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
*                                       NOTE: for high accuracy use CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TSU_CONTROL_STC control;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* dummy; not relevant for Ironman */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        control.unitEnable = GT_TRUE;
        control.taiNumber = taiId.taiNumber;

        if (!prvCpssDxChPortSkipCheck(devNum,portNum) && !prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                /* Set Egress Timestamp TAI Select */
                rc = cpssDxChPtpTsDelayEgressTimestampTaiSelectSet(devNum,portNum,taiId.taiNumber);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

     if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        return GT_OK;

    /* set the mode in which egress port is bound to TAI */
    rc = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet(devNum,portNum,egressPortBindMode);

    return rc;
}

/**
* @internal cpssDxChPtpManagerPortAllocateTai function
* @endinternal
*
* @brief   Allocating Port Interface to TAI and enabling TSU for selected port.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] egressPortBindMode       - determines the mode in which egress port is bound to TAI,
*                                       either static or dynamic per packet.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E, (dynamic bind, per ingress port of a specific packet)
*                                           CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E)  (static bind per taiNumber param)
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
*                                       NOTE: for high accuracy use CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortAllocateTai);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, egressPortBindMode));

    rc = internal_cpssDxChPtpManagerPortAllocateTai(devNum, portNum, taiNumber, egressPortBindMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, egressPortBindMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerTodValueGet to retreive captured TOD values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            /* select taiNumber for triggering */
            rc = prvCpssDxChPtpTaiGroupSelect(devNum,(1 << taiNumber));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* prepare the trigger caputre TOD counter value */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger capture */
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)


    return GT_OK;
}



/**
* @internal cpssDxChPtpManagerCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerTodValueGet to retreive captured TOD values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerCaptureTodValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber));

    rc = internal_cpssDxChPtpManagerCaptureTodValueSet(devNum, taiNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT clockGenerationMode = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* disable PTP CLOCK OUT (a.k.a. 1pps) before update TOD second field */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            rc = cpssDxChPtpTaiClockGenerationModeGet(devNum,&taiId,&clockGenerationMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* assigns the TOD a new value */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,todValuePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,clockGenerationMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueSet function
* @endinternal
*
* @brief   Set a new TOD Value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] todValuePtr               - (pointer to) TOD value.
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, todValuePtr, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueSet(devNum, taiNumber, todValuePtr, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, todValuePtr, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPtpManagerConvertFloatToBinary function
* @endinternal
*
* @brief   convert fractional nanosecond value from Float to Binary.
*
* @param[in]  fUpdateValue             - fractional nanosecond value in float format
*
* @param[out] bUpdateValuePtr          - (pointer to) fractional nanosecond value in binary format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpManagerConvertFloatToBinary
(
    IN GT_FLOAT64                             fUpdateValue,
    OUT GT_U32                                *bUpdateValuePtr
)
{
    GT_U32 i,bFractional=0;
    GT_BOOL isNegative,finished=GT_FALSE;

    /* stage 1: Determine Sign */
    if (fUpdateValue >= 0)
        isNegative = GT_FALSE;
    else
    {
        isNegative = GT_TRUE;
        fUpdateValue *=-1;
    }

    /* stage 2: Convert the Fractional Portion to Binary */
    /* special case; bit 31 is used for sign to we can't reach exactly 0.5 value in binary */
    if (fUpdateValue == 0.5)
    {
        bFractional = pow(2,31)-1;
        finished = GT_TRUE;
    }
    else if (fUpdateValue == 0)
    {
        bFractional = 0;
        finished = GT_TRUE;
    }
    else
    {
        for(i=0; i<=31; i++)
        {
           fUpdateValue = fUpdateValue * 2.0;
           if (fUpdateValue == 1.0)
           {
               bFractional += pow(2,31-i);
               /* Value must be a whole multiple of 2 */
               finished = GT_TRUE;
               break;
           }
           else if (fUpdateValue > 1.0)
           {
               bFractional += pow(2,31-i);
               fUpdateValue -= 1.0;
           }
        }
    }

    if (finished == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Value must be a whole multiple of 2");

    if (isNegative == GT_TRUE)
    {
        /* for negative value use 2's complement value conversion for bFractional */
        bFractional = (~bFractional + 1);
    }

    *bUpdateValuePtr = bFractional;

    return GT_OK;
}

/**
* @internal prvCpssDxChPtpManagerConvertBinaryToFloat function
* @endinternal
*
* @brief   convert fractional nanosecond value from Binary to Float.
*
* @param[in]  bUpdateValue             - fractional nanosecond value in binary format
*
* @param[out] fUpdateValuePtr          - (pointer to) fractional nanosecond value in float format
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPtpManagerConvertBinaryToFloat
(
    IN  GT_U32                                 bUpdateValue,
    OUT GT_FLOAT64                             *fUpdateValuePtr
)
{
    GT_U32 i;
    GT_32 j;

    *fUpdateValuePtr = 0;

    /* special case; bit 31 is used for sign to we can't reach exactly 0.5 value in binary */
    if (pow(2,31)-1 == bUpdateValue)
    {
        *fUpdateValuePtr = 0.5;
        return GT_OK;
    }

    /* stage 1: Determine Sign */
    *fUpdateValuePtr = -1*pow(2,-1)*(bUpdateValue >> 31);

    /* stage 2: Convert the Fractional Portion to Float */
    for(i=31; i>0; i--)
    {
        j=i-33;
        *fUpdateValuePtr += pow(2,j)*((bUpdateValue >> (i-1)) & 1);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPtpManagerConvertIntegerToFloat function
* @endinternal
*
* @brief   convert fractional nanosecond value from Integer to Float.
*
* @param[in]  updateValue              - fractional nanosecond value in integer format
*
* @param[out] fUpdateValuePtr          - (pointer to) fractional nanosecond value in float format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssDxChPtpManagerConvertIntegerToFloat
(
    IN  GT_32                                  updateValue,
    OUT GT_FLOAT64                             *fUpdateValuePtr
)
{
    GT_BOOL isNegative;

    /* stage 1: Determine Sign */
    if (updateValue >= 0)
        isNegative = GT_FALSE;
    else
    {
        isNegative = GT_TRUE;
        updateValue *=-1;
    }

    /* stage 2: Convert the Integer to Float
                Need this conversion to receive exact value, not approximate vaue; as pow function return double */
    switch (updateValue)
    {
    case 0xFFFFFFFF:
        *fUpdateValuePtr = pow(2,0);
        break;
    case 0x7FFFFFFF:
        *fUpdateValuePtr = pow(2,-1);
        break;
    case 0x3FFFFFFF:
        *fUpdateValuePtr = pow(2,-2);
        break;
    case 0x1FFFFFFF:
        *fUpdateValuePtr = pow(2,-3);
        break;
    case 0xFFFFFFF:
        *fUpdateValuePtr = pow(2,-4);
        break;
    case 0x7FFFFFF:
        *fUpdateValuePtr = pow(2,-5);
        break;
    case 0x3FFFFFF:
        *fUpdateValuePtr = pow(2,-6);
        break;
    case 0x1FFFFFF:
        *fUpdateValuePtr = pow(2,-7);
        break;
    case 0xFFFFFF:
        *fUpdateValuePtr = pow(2,-8);
        break;
    case 0x7FFFFF:
        *fUpdateValuePtr = pow(2,-9);
        break;
    case 0x3FFFFF:
        *fUpdateValuePtr = pow(2,-10);
        break;
    case 0x1FFFFF:
        *fUpdateValuePtr = pow(2,-11);
        break;
    case 0xFFFFF:
        *fUpdateValuePtr = pow(2,-12);
        break;
    case 0x7FFFF:
        *fUpdateValuePtr = pow(2,-13);
        break;
    case 0x3FFFF:
        *fUpdateValuePtr = pow(2,-14);
        break;
    case 0x1FFFF:
        *fUpdateValuePtr = pow(2,-15);
        break;
    case 0xFFFF:
        *fUpdateValuePtr = pow(2,-16);
        break;
    case 0x7FFF:
        *fUpdateValuePtr = pow(2,-17);
        break;
    case 0x3FFF:
        *fUpdateValuePtr = pow(2,-18);
        break;
    case 0x1FFF:
        *fUpdateValuePtr = pow(2,-19);
        break;
    case 0xFFF:
        *fUpdateValuePtr = pow(2,-20);
        break;
    case 0x7FF:
        *fUpdateValuePtr = pow(2,-21);
        break;
    case 0x3FF:
        *fUpdateValuePtr = pow(2,-22);
        break;
    case 0x1FF:
        *fUpdateValuePtr = pow(2,-23);
        break;
    case 0xFF:
        *fUpdateValuePtr = pow(2,-24);
        break;
    case 0x7F:
        *fUpdateValuePtr = pow(2,-25);
        break;
    case 0x3F:
        *fUpdateValuePtr = pow(2,-26);
        break;
    case 0x1F:
        *fUpdateValuePtr = pow(2,-27);
        break;
    case 0xF:
        *fUpdateValuePtr = pow(2,-28);
        break;
    case 0x7:
        *fUpdateValuePtr = pow(2,-29);
        break;
    case 0x3:
        *fUpdateValuePtr = pow(2,-30);
        break;
    case 0x1:
        *fUpdateValuePtr = pow(2,-31);
        break;
    case 0:
        *fUpdateValuePtr = 0;
        break;
    }

    /* stage 3: Add sign */
    if (isNegative == GT_TRUE)
        *fUpdateValuePtr *=-1.0;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value; the input is the numerator while the denominator Equals 2^32.
*                                        (APPLICABLE RANGES: -0.5..0.5 ns) (APPLICABLE DEVICES: Falcon)
*                                        (APPLICABLE RANGES: -0.25..0.25 ns) (APPLICABLE DEVICES: AC5P; AC5X; Harrier)
*                                        (APPLICABLE RANGES: -0.125..0.125 ns) (APPLICABLE DEVICES: Ironman)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_32                                  updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;
    GT_U32 numIterations=1;
    GT_U32 bUpdateValue;
    GT_FLOAT64 fUpdateValue;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    /* convert fractional nanosecond drift value from integer to float */
    rc = prvCpssDxChPtpManagerConvertIntegerToFloat(updateValue,&fUpdateValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (fUpdateValue > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.fracNanoSeconds || fUpdateValue < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.fracNanoSeconds*(-1))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    cpssOsMemSet(&todValue, 0, sizeof(todValue));

    /* convert fractional nanosecond drift value from float to Binary */
    rc = prvCpssDxChPtpManagerConvertFloatToBinary(fUpdateValue,&bUpdateValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* adjusts the "fractional nanosecond drift" value which is added to the TOD step logic */
        todValue.fracNanoSeconds = bUpdateValue;
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger frequency update information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueFrequencyUpdate function
* @endinternal
*
* @brief   Adjusts the fractional nanosecond drift value which is added to the TOD stepSet logic.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] taiNumber                 - TAI number.
* @param[in] updateValue               - fractional nanosecond drift value; the input is the numerator while the denominator Equals 2^32.
*                                        (APPLICABLE RANGES: -0.5..0.5 ns) (APPLICABLE DEVICES: Falcon)
*                                        (APPLICABLE RANGES: -0.25..0.25 ns) (APPLICABLE DEVICES: AC5P; AC5X; Harrier)
*                                        (APPLICABLE RANGES: -0.125..0.125 ns) (APPLICABLE DEVICES: Ironman)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_32                                  updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueFrequencyUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, updateValue, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueFrequencyUpdate(devNum, taiNumber, updateValue, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, updateValue, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefullStep            - Gracefull Step in nanoseconds. Applicable if "function" ==
*                                       CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_MANAGER_TOD_COUNT_STC   *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefullStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT clockGenerationMode = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E;
    CPSS_DXCH_PTP_TOD_COUNT_STC bUpdateValue;
    GT_FLOAT64 fUpdateValue;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(updateValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    /* convert fractional nanosecond drift value from integer to float */
    rc = prvCpssDxChPtpManagerConvertIntegerToFloat(updateValuePtr->fracNanoSeconds,&fUpdateValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (fUpdateValue > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.fracNanoSeconds ||
        fUpdateValue < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.fracNanoSeconds*(-1))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    switch (function)
    {
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
    case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* convert fractional nanosecond drift value from float to Binary */
    rc = prvCpssDxChPtpManagerConvertFloatToBinary(fUpdateValue,&(bUpdateValue.fracNanoSeconds));
    if (rc != GT_OK)
    {
        return rc;
    }
    bUpdateValue.seconds = updateValuePtr->seconds;
    bUpdateValue.nanoSeconds = updateValuePtr->nanoSeconds;

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* disable PTP CLOCK OUT (a.k.a. 1pps) before update TOD second field */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            rc = cpssDxChPtpTaiClockGenerationModeGet(devNum,&taiId,&clockGenerationMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* increment/decrement the TOD Counter by a specified value */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&bUpdateValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* prepare the trigger function information */
        rc = cpssDxChPtpTodCounterFunctionSet(devNum,0 /* not relevant */,&taiId,function);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable capturing the TOD value before the update */
        rc = cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum,&taiId,capturePrevTodValueEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* allow the new capture to override entry 0 */
        rc = cpssDxChPtpTaiCaptureOverrideEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((function == CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E) || (function == CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E))
        {
            /* set the graceful step */
            rc = cpssDxChPtpTaiGracefulStepSet(devNum,&taiId,gracefullStep);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E))
        {
            if (clockGenerationMode != CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E)
            {
                rc = cpssDxChPtpTaiClockGenerationModeSet(devNum,&taiId,clockGenerationMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueUpdate function
* @endinternal
*
* @brief  update the current TOD value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] updateValuePtr           - (pointer to) increment/decrement value.
* @param[in] function                 - TOD counter functions.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,
*                                           CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E)
* @param[in] gracefullStep            - Gracefull Step in nanoseconds. Applicable if "function" ==
*                                       CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E or CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] capturePrevTodValueEnable - enable/disable capturing previous value.
*                                        GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                        GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_MANAGER_TOD_COUNT_STC   *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefullStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, updateValuePtr, function, gracefullStep, capturePrevTodValueEnable));

    rc = internal_cpssDxChPtpManagerTodValueUpdate(devNum, taiNumber, updateValuePtr, function, gracefullStep, capturePrevTodValueEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, updateValuePtr, function, gracefullStep, capturePrevTodValueEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry1Ptr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    if (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    taiId.taiNumber = taiNumber;

    /* ensure TOD0 value is captured */
    rc = cpssDxChPtpTaiTodCaptureStatusGet(devNum,&taiId,0,&(todValueEntry0Ptr->todValueIsValid));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (todValueEntry0Ptr->todValueIsValid == GT_TRUE)
    {
        /* get captured TOD0 value */
        rc = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,&(todValueEntry0Ptr->todValue));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* ensure TOD1 value is captured */
    rc = cpssDxChPtpTaiTodCaptureStatusGet(devNum,&taiId,1,&(todValueEntry1Ptr->todValueIsValid));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (todValueEntry1Ptr->todValueIsValid == GT_TRUE)
    {
        /* get captured TOD1 value */
        rc = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E,&(todValueEntry1Ptr->todValue));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTodValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr));

    rc = internal_cpssDxChPtpManagerTodValueGet(devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, todValueEntry0Ptr, todValueEntry1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(triggerTimePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskValuePtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* define the pulse width */
        rc = cpssDxChPtpTaiExternalPulseWidthSet(devNum,&taiId,extPulseWidth);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* setting trigger generation time */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,triggerTimePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* setting trigger generation time mask */
        rc = cpssDxChPtpTaiTodSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,maskValuePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable the trigger */
        rc = cpssDxChPtpTaiOutputTriggerEnableSet(devNum,&taiId,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger update*/
        rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTriggerGenerate function
* @endinternal
*
* @brief   Generates a trigger on the PTP_PULSE_OUT interface at a specified time.
*
* @note    NOTE: This functionality should only be triggered on the master device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] extPulseWidth            - external pulse width in nanoseconds of generated output trigger.
*                                       (APPLICABLE RANGES: 0..2^28-1.)
* @param[in] triggerTimePtr           - (pointer to) trigger generation time.
* @param[in] maskValuePtr             - (pointer to) trigger generation time mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTriggerGenerate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, extPulseWidth, triggerTimePtr, maskValuePtr));

    rc = internal_cpssDxChPtpManagerTriggerGenerate(devNum, taiNumber, extPulseWidth, triggerTimePtr, maskValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, extPulseWidth, triggerTimePtr, maskValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] domainConfPtr            - (pointer to) PTP domain configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
)
{
    GT_STATUS rc;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(domainConfPtr);

    /* define the PTP protocol (disable/v1 or v2) per domain index */
    rc = cpssDxChPtpDomainModeSet(devNum,domainConfPtr->domainIndex,domainConfPtr->domainMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (domainConfPtr->domainMode == CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E)
    {
        /* Set PTP V1 domain Id */
        rc = cpssDxChPtpDomainV1IdSet(devNum,domainConfPtr->domainIndex,domainConfPtr->v1DomainIdArr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (domainConfPtr->domainMode == CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E)
    {
        /* Set PTP V2 domain Id */
        rc = cpssDxChPtpDomainV2IdSet(devNum,domainConfPtr->domainIndex,domainConfPtr->v2DomainId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerGlobalPtpDomainSet function
* @endinternal
*
* @brief   Configures the global settings for PTP Domain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] domainConfPtr            - (pointer to) PTP domain configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerGlobalPtpDomainSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainConfPtr));

    rc = internal_cpssDxChPtpManagerGlobalPtpDomainSet(devNum, domainConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - PTP Message Type to trigger the selected Command.
* @param[in] command                  - command assigned  to the packet.
*                                      (APPLICABLE VALUES:
*                                         CPSS_PACKET_CMD_FORWARD_E,
*                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                         CPSS_PACKET_CMD_DROP_HARD_E,
*                                         CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);


    return cpssDxChPtpMessageTypeCmdSet(devNum,portNum,domainIndex,messageType,command);
}

/**
* @internal cpssDxChPtpManagerPortCommandAssignment function
* @endinternal
*
* @brief   Assign packet command per{ingress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - PTP Message Type to trigger the selected Command.
* @param[in] command                  - command assigned  to the packet.
*                                      (APPLICABLE VALUES:
*                                         CPSS_PACKET_CMD_FORWARD_E,
*                                         CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                         CPSS_PACKET_CMD_DROP_HARD_E,
*                                         CPSS_PACKET_CMD_DROP_SOFT_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortCommandAssignment
(
    IN GT_U8                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                     portNum,
    IN GT_U32                                   domainIndex,
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortCommandAssignment);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, messageType, command));

    rc = internal_cpssDxChPtpManagerPortCommandAssignment(devNum, portNum, domainIndex, messageType, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, messageType, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_ACTION_NONE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_DROP_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    CPSS_DXCH_PTP_TSU_CONTROL_STC control;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;
    GT_U32 numIterations=1;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* dummy; not relevant for Ironman */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        control.unitEnable = GT_TRUE;
        control.taiNumber = taiId.taiNumber;

        if (!prvCpssDxChPortSkipCheck(devNum,portNum) && !prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* set the PTP domain table per egress physical port and per domain index */
        rc = cpssDxChPtpEgressDomainTableSet(devNum,portNum,domainIndex,domainEntryPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPtpTsLocalActionTableGet(devNum,portNum,domainIndex,messageType,&entry);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Per IEEE1588-2008 section 11.4.5 ingress link delay measured in peer-to-peer delay mechanism, must be added only to the correction field
           of Sync messages in 1-step clock, and to Follow-up messages in 2-step clocks. (For follow-up messages, the ingress link delay should be added by application). */
        entry.tsAction = action;
        if ((entry.tsAction == CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E || entry.tsAction == CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E) &&
            (messageType == CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E))

        {
            entry.ingrLinkDelayEnable = GT_TRUE;
        }
        else
        {
            entry.ingrLinkDelayEnable = GT_FALSE;
        }

        /* set action per egress port, domain index and message type */
        rc = cpssDxChPtpTsLocalActionTableSet(devNum,portNum,domainIndex,messageType,&entry);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortPacketActionsConfig function
* @endinternal
*
* @brief  Configure action to be performed on PTP packets per{egress port, domain index, message type}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier).
* @param[in] domainIndex              - domain index.
*                                       (APPLICABLE RANGES: 0..4.)
* @param[in] domainEntryPtr           - (pointer to) domain table entry.
* @param[in] messageType              - PTP message type
* @param[in] action                   - PTP action on the packet
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TS_ACTION_NONE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_DROP_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,
*                                           CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortPacketActionsConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action));

    rc = internal_cpssDxChPtpManagerPortPacketActionsConfig(devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, domainIndex, domainEntryPtr, messageType, action));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(tsTagPortCfgPtr);

    /* Set Timestamp Port Configuration */
    return cpssDxChPtpTsTagPortCfgSet(devNum,portNum,tsTagPortCfgPtr);
}

/**
* @internal cpssDxChPtpManagerPortTsTagConfig function
* @endinternal
*
* @brief    Controlling Timestamp tag for Ingress identification and Egress Addition.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortTsTagConfig
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortTsTagConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsTagPortCfgPtr));

    rc = internal_cpssDxChPtpManagerPortTsTagConfig(devNum, portNum, tsTagPortCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsTagPortCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] messageType            - PTP Message Type
* @param[in] queueNum               - Queue number; 0 - queue0, 1 - queue1.
*                                     (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
)
{
    GT_U32 regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    if (messageType >= CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueMsgType;

    /* Configure queue number per message type */
    return prvCpssHwPpSetRegField(devNum, regAddr, messageType /*offset*/, 1, queueNum);

}

/**
* @internal cpssDxChPtpManagerTsQueueConfig function
* @endinternal
*
* @brief  Configure queue number per message type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] messageType            - PTP Message Type
* @param[in] queueNum               - Queue number; 0 - queue0, 1 - queue1.
*                                     (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTsQueueConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, messageType, queueNum));

    rc = internal_cpssDxChPtpManagerTsQueueConfig(devNum, messageType, queueNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, messageType, queueNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Egress Timestamp Global Queue Entry */
    return cpssDxChPtpTsEgressTimestampQueueEntryRead(devNum,queueNum,entryPtr);

}

/**
* @internal cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(devNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC  entry;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Egress Timestamp Queue Entry per port */
    rc = cpssDxChPtpTsuTxTimestampQueueRead(devNum,portNum,queueNum,&entry);
    if (rc != GT_OK)
    {
        return rc;
    }

    entryPtr->entryValid = entry.entryValid;
    entryPtr->queueEntryId = entry.queueEntryId;
    entryPtr->taiSelect = entry.taiSelect;
    entryPtr->todUpdateFlag = entry.todUpdateFlag;
    entryPtr->seconds = (entry.timestamp >> 30);
    entryPtr->nanoSeconds = (entry.timestamp & 0x3FFFFFFF);

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  GT_U32                                                      queueNum,
    OUT CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerEgressTimestampPortQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(devNum, portNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *entryPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    if (queueNum >= BIT_1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Read Ingress Timestamp Global Queue Entry */
    return cpssDxChPtpTsIngressTimestampQueueEntryRead(devNum,queueNum,entryPtr);

}

/**
* @internal cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Global Queue Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - queue Number
*                                       0 - queue0, 1 - queue1
*                                       (APPLICABLE RANGES: 0..1.)
* @param[out] entryPtr                - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpManagerIngressTimestampGlobalQueueEntryRead(devNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPtpManagerDelayOffsetInDbGet function
* @endinternal
*
* @brief   Get PTP delay offset in DB for specific physical port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] foundOffsetPtr       - (pointer to) wether offset was found according to port parameters.
* @param[out] offsetPtr            - (pointer to) offset of delay params in DB if foundOffsetPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
GT_STATUS prvCpssDxChPtpManagerDelayOffsetInDbGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                              *foundOffsetPtr,
    OUT GT_U32                               *offsetPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
    CPSS_DXCH_PORT_FEC_MODE_ENT fecMode;
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT ptpSpeed;
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT ptpInterfaceMode;
    CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT ptpFecMode;
    GT_BOOL foundParams = GT_TRUE;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(foundOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);

    *foundOffsetPtr = GT_FALSE;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        return GT_OK;

    rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch (speed)
    {
    case CPSS_PORT_SPEED_100_E:
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100M_E;
        }
        else
        {
            foundParams = GT_FALSE;
        }
        break;
    case CPSS_PORT_SPEED_1000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_1G_E;
        break;
    case CPSS_PORT_SPEED_10000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_10G_E;
        break;
    case CPSS_PORT_SPEED_25000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_25G_E;
        break;
    case CPSS_PORT_SPEED_40000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_40G_E;
        break;
    case CPSS_PORT_SPEED_50000_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_50G_E;
        break;
    case CPSS_PORT_SPEED_100G_E:
    case CPSS_PORT_SPEED_102G_E:
    case CPSS_PORT_SPEED_106G_E:
    case CPSS_PORT_SPEED_107G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100G_E;
        break;
    case CPSS_PORT_SPEED_200G_E:
    case CPSS_PORT_SPEED_212G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_200G_E;
        break;
    case CPSS_PORT_SPEED_400G_E:
    case CPSS_PORT_SPEED_424G_E:
        ptpSpeed = CPSS_DXCH_PTP_MANAGER_PORT_SPEED_400G_E;
        break;
    default:
        /* set default '0' */
        foundParams = GT_FALSE;
    }

    if (foundParams)
    {

        rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&interfaceMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        switch (interfaceMode)
        {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_SXLX_E:
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_DEFAULT_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_2LANE_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            ptpInterfaceMode = CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_4LANE_E;
            break;
        default:
            /* set default '0' */
            foundParams = GT_FALSE;
        }

        if (foundParams)
        {
            rc = cpssDxChPortFecModeGet(devNum,portNum,&fecMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            switch (fecMode)
            {
            case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_FC_FEC_MODE_ENABLED_E;
                break;
            case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:
            case CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_DISABLED_E;
                break;
            case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_528_514_MODE_ENABLED_E;
                break;
            case CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E:
                ptpFecMode = CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            default:
                /* set default '0' */
                foundParams = GT_FALSE;
            }
        }
    }

    if (foundParams)
    {
        *offsetPtr = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 ptpSpeed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
                                 ptpInterfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + ptpFecMode;
        *foundOffsetPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpManagerPortTimestampingModeConfig function
* @endinternal
*
* @brief   Configure Enhanced Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsMode                   - enhance timestamping mode.
* @param[in] operationMode            - operation mode.
*                                       (APPLICABLE VALUES:
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E,
*
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E
*                                           (APPLICABLE DEVICES: Ironman).
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E,
*
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E
*                                           (APPLICABLE DEVICES: Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
)
{
    GT_STATUS rc;
    GT_U32  regAddr,regAddr1;
    GT_U32  tsuIndex = 0;
    GT_U32  tsuLocalChannel = 0;
    GT_U32  domainProfile, messageType, amdOffset;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;
    GT_U32  portMacNum;
    GT_BOOL egressTsCorrectionEnable,mchEnable=GT_FALSE,dispatchEnable;
    GT_BOOL fsuEnable,amdEnable,tsPchInsertionEnable,egressTsEncryptedEnable;
    GT_BOOL oneStepTsEnable;
    GT_U32  value,data,mchCaptureStep=0;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32  unitIndex, channel, isPca;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if ((operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E) && !PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &tsuIndex, &tsuLocalChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    switch (tsMode)
    {
    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_NONE_E:
        /* This mode is used for non PTP configurations. */

        egressTsCorrectionEnable = GT_FALSE;
        dispatchEnable = GT_FALSE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_FALSE;
        egressTsEncryptedEnable = GT_FALSE;
        tsPchInsertionEnable = GT_FALSE;
        mchEnable = GT_FALSE;
        mchCaptureStep = 0;
        oneStepTsEnable = GT_FALSE;

        break;

    case  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
        /* This mode should be used for high accuracy timestamping on single lane ports, without RSFEC and
           PTP on clear (not encrypted).*/

        switch (operationMode)
        {
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E:
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        egressTsCorrectionEnable = GT_TRUE;
        dispatchEnable = GT_FALSE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_FALSE;
        egressTsEncryptedEnable = GT_FALSE;
        tsPchInsertionEnable = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E) ? GT_TRUE : GT_FALSE;
        mchEnable = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E) ? GT_TRUE : GT_FALSE;
        mchCaptureStep = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E) ? 1 /* TWO_STEP */ : 0 /* ONE_STEP */;
        /* oneStepTsEnable needs to be 1 only when MAC corrects the CF, not when MAC captures timestamp. */
        oneStepTsEnable = GT_TRUE;
        break;

    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
        /* When working in this mode, an external copper PHY is connected between the switch port (Host)
           and the media/copper side.
           For increased timestamping accuracy, timestamping should be done in the PHY itself (avoiding the
           PDV in the PHY itself) both on the ingress path and on the egress path. The communication
           between the switch and the PHY for passing the timestamp information is done via a dedicated 8B
           header. This header is not a part of the payload, but rather it replaces the preamble of the frame. */

        /* this mode should be used only when the PHY supports (and is configured to use) MCH/PCH. */
        switch (operationMode)
        {
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E:
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        egressTsCorrectionEnable = GT_FALSE;
        dispatchEnable = GT_FALSE;
        fsuEnable = GT_FALSE;
        amdEnable = GT_FALSE;
        tsPchInsertionEnable = GT_TRUE;
        egressTsEncryptedEnable = GT_FALSE;
        mchEnable = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E) ? GT_TRUE : GT_FALSE;
        mchCaptureStep = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E) ? 1 /* TWO_STEP */ : 0 /* ONE_STEP */;
        oneStepTsEnable = GT_FALSE;
        break;

    case CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
        /* This mode should be used when MACSEC encryption is enabled for the PTP flow on the egress port.
           When encryption is incorporated, the frame cannot be modified post the MACSEC block.
           The timestamping is thus done at an earlier stage by utilizing a dedicated high accuracy engine.
                                                                                                          .
           In IM MACSEC is supported as:
           1. 2-step in the MAC/PHY - accurate
           2. 1-step in the Switch CTSU - not as accurate */

        switch (operationMode)
        {
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E:
            break;
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E:
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E:
        case CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E:
            if (!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        egressTsCorrectionEnable = GT_FALSE;
        dispatchEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) ? GT_FALSE : GT_TRUE;
        /* Timestamp to the Future is not supported in AC5X */
        fsuEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
                     PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) ? GT_FALSE : GT_TRUE;
        amdEnable = GT_FALSE;
        tsPchInsertionEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E &&
                                operationMode != CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E) ? GT_TRUE : GT_FALSE;
        egressTsEncryptedEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) ? GT_FALSE : GT_TRUE;

        mchEnable = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E &&
                     (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E ||
                      operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E)) ? GT_TRUE : GT_FALSE;
        mchCaptureStep = (mchEnable == GT_TRUE) ? 1 /* TWO_STEP */ : 0 /* ONE_STEP */;
        oneStepTsEnable = GT_FALSE;

        /* NOTE: fsu_offset and Encrypted Egress pipe delay are set by port manager and are relevant for this mode only (prvCpssDxChPortPtpDelayParamsSetWrapper) */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsPrecision[tsuLocalChannel];

    /* enable/disable egress TS Correction; Egress TS encrypted; Egress TS PCH Insertion */
    data = (tsPchInsertionEnable << 3) | (egressTsEncryptedEnable << 1) | egressTsCorrectionEnable;
    rc = prvCpssHwPpWriteRegBitMask(devNum,regAddr,0xB,data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* enable/disable egress MCH enable; Egress MCH Capture Step */
        data = (mchCaptureStep << 1) | mchEnable;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 6, 2, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable ingress PCH unit */
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portEmacUsxPchRxControl;
            data = BOOL2BIT_MAC(mchEnable);
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portPmacUsxPchRxControl;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portEmacUsxPchRxControl;
            data = BOOL2BIT_MAC(mchEnable);
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }

            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portPmacUsxPchRxControl;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }

        /* Enable/Disable MAC 2-step ptp */
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portControl;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portControl;
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        data = (operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E) ? 1 : 0;

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 9, 1, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable/disable 1-step capable datapath (if available)*/
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.xifMode;
        }
        else if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.xifMode;
        }
        else
        {
            regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 5, 1, oneStepTsEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable DISPATCH in TsLocalActionTable */
    for (domainProfile=0; domainProfile<=4; domainProfile++)
    {
        for (messageType=0; messageType<CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E; messageType++)
        {
            rc = cpssDxChPtpTsLocalActionTableGet(devNum,portNum,domainProfile,messageType,&entry);
            if (rc != GT_OK)
            {
                return rc;
            }

            entry.packetDispatchingEnable = dispatchEnable;

            rc = cpssDxChPtpTsLocalActionTableSet(devNum,portNum,domainProfile,messageType,&entry);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* update PTP timestamping operation mode in DB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].tsMode = tsMode;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].operationMode = operationMode;

    /* bellow configuration is not relevant for Ironman */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        return GT_OK;

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl1;
        amdOffset = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ? 1 : 19;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portTsdControl0;
        amdOffset = 1;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;
        amdOffset = 1;
    }

    /* enable/disable Future Timestamp feature (bit 0) */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, fsuEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* configure registers for 200G/400G ports */
    regAddr1 = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).segPortTsdControl0;

    if (regAddr1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr1, 0, 1, fsuEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable/disable Alignment Marker Dispatcher feature (bit 1/19) */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, amdOffset, 1, amdEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (regAddr1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr1, 1, 1, amdEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* MacSec additional configuration */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {

        if ((PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_400_E) || (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_100_E))
        {
            /* save value for restore */
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).tsuControl2;
            rc = prvCpssDrvHwPpReadRegBitMask(devNum,regAddr,0x83,&data);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(devNum, portNum, &unitIndex, &channel, &isPca);
                if (rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
                if (rc != GT_OK)
                {
                    return rc;
                }

                switch (speed)
                {

                case CPSS_PORT_SPEED_10000_E:
                    value = 16;
                    break;
                case CPSS_PORT_SPEED_25000_E:
                    value = 32;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    value = 64;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                case CPSS_PORT_SPEED_102G_E:
                case CPSS_PORT_SPEED_106G_E:
                case CPSS_PORT_SPEED_107G_E:
                    value = 128;
                    break;
                default:
                    value = 0; /* default */
                }

                /* Ch %n Wait Slots: This field specifies a time (measured in time slots of the given channel) necessary to ensure that the entire Tx pipe
                   (starting from SFF in PCA Cluster and until TXF in EPI Cluster) becomes empty after traffic (of the given channel) was stopped in the SFF. */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_SFF[unitIndex].channelControlConfig[channel];
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 8, value);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;

                /* minimal_tx_stop_toggle: De-bouncing protection to the TX_STOP signal; small range to reduce jitter */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 25, 5, 10);
                if (rc != GT_OK)
                {
                    return rc;
                }

                switch (speed)
                {

                case CPSS_PORT_SPEED_10000_E:
                    value = 136;
                    break;
                case CPSS_PORT_SPEED_25000_E:
                    value = 54;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                case CPSS_PORT_SPEED_102G_E:
                case CPSS_PORT_SPEED_106G_E:
                case CPSS_PORT_SPEED_107G_E:
                    value = 10;
                    break;
                default:
                    value = 16; /* default */
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl2;

                /* bits 0..15 - minimal_empty_for_stop_tx: Minimal TXFIFO empty cycles before assertion of stop tx*/
                data = value;
                /* bit 16 - fsu_preemption_hold_en: Ability to En/Dis impact over p.MAC HOLD signal; stop transmission in mii while PTP is in */
                data |= 0x10000;
                /* bits 17..20 - hold_kept_active: Number of CCs hold signal is kept asserted after exp_stop_tx has de-asserted. Must be above 'd2 */
                data |= 0x1E0000;

                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 21, data);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPmacControl;

                /* tx_hold_req_from_hw: tx_hold_req controlled by HW; otherwise preemption_hold is not asserted */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 29, 1, 1);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).tsuControl2;

                /* bit 0 - c_rx_mode_ow: When 1, c_rx_mode_ow value is taken from RF and not from internal logic; bypass TSX for RX to avoid jumps in TOD when matching in TSD */
                /* bit 1 - c_tx_mode_ow: When 1, c_tx_mode_ow value is taken from RF and not from internal logic; bypass TSX for TX to avoid jumps in TOD when matching in TSD */
                /* bit 7 - c_modulo_tx_ow: When 1, c_modulo_tx value is taken from RF and not from internal logic; TSX bypass mode */
                rc = prvCpssDrvHwPpWriteRegBitMask(devNum,regAddr,0x83,0x83);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(devNum, portNum, &unitIndex, &channel, &isPca);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* Ch %n Wait Slots: This field specifies a time (measured in time slots of the given channel) necessary to ensure that the entire Tx pipe
                   (starting from SFF in PCA Cluster and until TXF in EPI Cluster) becomes empty after traffic (of the given channel) was stopped in the SFF. */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_SFF[unitIndex].channelControlConfig[channel];
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 8, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;

                /* minimal_tx_stop_toggle: De-bouncing protection to the TX_STOP signal; small range to reduce jitter */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 25, 5, 16);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl2;

                /* bits 17..20 - hold_kept_active: Number of CCs hold signal is kept asserted after exp_stop_tx has de-asserted. Must be above 'd2 */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 21, 0x60000);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPmacControl;

                /* tx_hold_req_from_hw: tx_hold_req controlled by HW; otherwise preemption_hold is not asserted */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 29, 1, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }

                regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).tsuControl2;

                /* bit 0 - c_rx_mode_ow: When 1, c_rx_mode_ow value is taken from RF and not from internal logic; bypass TSX for RX to avoid jumps in TOD when matching in TSD */
                /* bit 1 - c_tx_mode_ow: When 1, c_tx_mode_ow value is taken from RF and not from internal logic; bypass TSX for TX to avoid jumps in TOD when matching in TSD */
                /* bit 7 - c_modulo_tx_ow: When 1, c_modulo_tx value is taken from RF and not from internal logic; TSX bypass mode */
                rc = prvCpssDrvHwPpWriteRegBitMask(devNum,regAddr,0x83,data);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortTimestampingModeConfig function
* @endinternal
*
* @brief   Configure Enhanced Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsMode                   - enhance timestamping mode.
* @param[in] operationMode            - operation mode.
*                                       (APPLICABLE VALUES:
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E,
*
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E
*                                           (APPLICABLE DEVICES: Ironman).
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E
*
*                                       for tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E:
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E,
*
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E,
*                                           CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E
*                                           (APPLICABLE DEVICES: Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortTimestampingModeConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsMode, operationMode));

    rc = internal_cpssDxChPtpManagerPortTimestampingModeConfig(devNum, portNum, tsMode, operationMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsMode, operationMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortDelayValuesSet
*           function
* @endinternal
*
* @brief   Set delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[in] delayValPtr              - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortDelayValuesSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    IN  CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{

    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(delayValPtr);
    if (speed >= CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (interfaceMode >= CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (fecMode >= CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* configure values into DB */
    offset = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             speed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             interfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + fecMode;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay = delayValPtr->egressPipeDelay;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelayEncrypted = delayValPtr->egressPipeDelayEncrypted;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelayFsuOffset = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay = delayValPtr->ingressPortDelay;

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortDelayValuesSet function
* @endinternal
*
* @brief   Set delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[in] delayValPtr              - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    IN  CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortDelayValuesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));

    rc = internal_cpssDxChPtpManagerPortDelayValuesSet(devNum, portNum, speed, interfaceMode, fecMode, delayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortDelayValuesGet
*           function
* @endinternal
*
* @brief   Get delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[out] delayValPtr             - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPortDelayValuesGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_U32 offset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(delayValPtr);
    if (speed >= CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (interfaceMode >= CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (fecMode >= CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->ptpMgrInfo.initDone == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* get values from DB */
    offset = portNum*CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             speed*CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E +
             interfaceMode*CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E + fecMode;

    delayValPtr->egressPipeDelay = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelay;
    delayValPtr->egressPipeDelayEncrypted = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.egressPipeDelayEncrypted;
    delayValPtr->egressPipeDelayFsuOffset = 0;
    delayValPtr->ingressPortDelay = PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr[offset].ptpDelayValues.ingressPortDelay;

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortDelayValuesGet function
* @endinternal
*
* @brief   Get delay values per port {speed,interface,fec}
*          mode.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speed                    - speed
* @param[in] interfaceMode            - interface mode
* @param[in] fecMode                  - forward error correction mode
* @param[out] delayValPtr             - (pointer to) delay values
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPortDelayValuesGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT                speed,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT       interfaceMode,
    IN  CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT             fecMode,
    OUT CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC          *delayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortDelayValuesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));

    rc = internal_cpssDxChPtpManagerPortDelayValuesGet(devNum, portNum, speed, interfaceMode, fecMode, delayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed, interfaceMode, fecMode, delayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read MAC Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] entryPtr                - (pointer to) MAC Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
* @note For USGMII ports, cpss reads it from the integrated-PHY.
*
*/
static GT_STATUS internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    GT_STATUS   rc;                         /* return code      */
    GT_U32      regAddr[2];                 /* register address */
    GT_U32      entryClearRegAddr = 0;      /* register address */
    GT_U32      hwData[2];                  /* HW data          */
    GT_U32      i;                          /* loop index       */
    GT_U32      portMacNum;                 /* MAC number       */
    GT_BOOL     invalidate = GT_TRUE;
    CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT signatureMode;
    GT_U32 signatureValue,tmp;
    /* internal phy support */
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_U32 mpdIndex;
    CPSS_MACDRV_OBJ_STC *portMacObjPtr;/* Get PHY MAC object pointer */
    GT_U32 egressPipeDelayCtsu;
    CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC  egrDelayVal;
    GT_U32 nanoSeconds;
    GT_U32 seconds;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* Get MCH signature configuration per port */
    rc = cpssDxChPtpManagerPortMacTsQueueSigatureGet(devNum,portNum,&signatureMode,&signatureValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Ironman-M internal PHY */
    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_IRONMAN_M_DEVICES_CASES_MAC:
        {
            /* if this port is managed by the phy read TS and signature from internal PHY */
            portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portNum);
            mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);
            opParams.phyPtpTsq.signature = signatureValue;

            /* Valid mpd Index start from 1 */
            if ((portMacObjPtr != NULL) && mpdIndex != 0)
            {
                /* Perform PHY operation on port */
                ret = mpdPerformPhyOperation(mpdIndex,MPD_OP_CODE_GET_PTP_TSQ_E,&opParams);
                if (MPD_OK_E != ret)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                switch (signatureMode)
                {
                case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E:
                    /* Signature[13:0] */
                    entryPtr->signature.signatureCtr.signatureCtr = (opParams.phyPtpTsq.signature & 0x3FFF);
                    break;

                case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E:
                    /* {QueueSelect [0], QueueEntryId [9:0]} */
                    entryPtr->signature.signatureQueueEntryId.queueEntryId = (opParams.phyPtpTsq.signature & 0x3FF);
                    entryPtr->signature.signatureQueueEntryId.queueSelect = ((opParams.phyPtpTsq.signature >> 10) & 1);
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if ((PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E ||
                     PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E) &&
                     PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E)
                {
                    /* get Egress Pipe delay from ctsu */
                    rc = cpssDxChPtpPortTxPipeStatusDelayGet(devNum,portNum,&egressPipeDelayCtsu);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* nanosecond timestamp in the range -2^30 to 2^30 */
                    nanoSeconds = (opParams.phyPtpTsq.timestamp && 0x3FFFFFFF) + egressPipeDelayCtsu;
                    seconds = (opParams.phyPtpTsq.timestamp >> 30);
                    if (nanoSeconds >= 1000000000)
                    {
                        nanoSeconds -= 1000000000;
                        seconds += 1;
                        opParams.phyPtpTsq.timestamp = ((seconds & 0x3) << 30) | (nanoSeconds & 0x3FFFFFFF);
                    }
                    else
                    {
                        opParams.phyPtpTsq.timestamp += egressPipeDelayCtsu;
                    }
                }
                entryPtr->seconds = (opParams.phyPtpTsq.timestamp >> 30);
                entryPtr->nanoSeconds = (opParams.phyPtpTsq.timestamp & 0x3FFFFFFF);

                return GT_OK;
            }
            break;
        }

        default:
            break;
    }

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).tsqSignature;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).tsqTimestamp;
        entryClearRegAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).tsqClearControl;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).tsqSignature;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).tsqTimestamp;
        entryClearRegAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).tsqClearControl;
    }
    else
    {
        regAddr[0] = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        regAddr[1] = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    cpssOsMemSet(entryPtr, 0, sizeof(*entryPtr));

    for (i = 0; i < 2; i++)
    {
        rc = prvCpssHwPpReadRegister(devNum, regAddr[i], &(hwData[i]));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* check that the first word is valid */
        if ((i == 0) &&  (((hwData[0] >> 16) & 1) == 0) )
        {

          /* Do not read/clear word 1/2.
             The design is not well protected if reading an empty queue word1/word2 while a new entry is pushed in */
          invalidate = GT_FALSE;
          break;
        }
    }

    /* invalidate (pop) the entry in the queue; indication for hw to invalidate the entry. */
    if (invalidate == GT_TRUE)
    {
        /* design looks only if the register was written with any value; no need to set & unset */
        rc = prvCpssHwPpWriteRegister(devNum, entryClearRegAddr, 0);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* the design requiers additional dummy read as a delay */
        rc = prvCpssHwPpReadRegister(devNum, entryClearRegAddr, &tmp);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    entryPtr->entryValid     = BIT2BOOL_MAC(((hwData[0] >> 16) & 1));
    if (entryPtr->entryValid == GT_FALSE)
        return GT_OK;

    switch (signatureMode)
    {
    case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E:
        /* Signature[13:0] */
        entryPtr->signature.signatureCtr.signatureCtr = (hwData[0] & 0x3FFF);
        break;

    case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E:
        /* {QueueSelect [0], QueueEntryId [9:0]} */
        entryPtr->signature.signatureQueueEntryId.queueEntryId = (hwData[0] & 0x3FF);
        entryPtr->signature.signatureQueueEntryId.queueSelect = ((hwData[0] >> 10) & 1);
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E ||
         PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].tsMode == CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E) &&
         PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortTsOperationModeArrPtr[portNum].operationMode == CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E)
    {
        /* get Egress Pipe Delay Corrections */
        rc = cpssDxChPtpTsDelayEgressPipeDelayGet(devNum,portNum,&egrDelayVal);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* nanosecond timestamp in the range -2^30 to 2^30 */
        nanoSeconds = (hwData[1] && 0x3FFFFFFF) + egrDelayVal.egressPipeDelay;
        seconds = (hwData[1] >> 30);
        if (nanoSeconds >= 1000000000)
        {
            nanoSeconds -= 1000000000;
            seconds += 1;
            hwData[1] = ((seconds & 0x3) << 30) | (nanoSeconds & 0x3FFFFFFF);
        }
        else
        {
            hwData[1] += egrDelayVal.egressPipeDelay;
        }
    }
    entryPtr->seconds = (hwData[1] >> 30);
    entryPtr->nanoSeconds = (hwData[1] & 0x3FFFFFFF);

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead function
* @endinternal
*
* @brief   Read MAC Egress Timestamp Queue Entry per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] entryPtr                - (pointer to) MAC Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
* @note The Entry is deleted from the Queue by reading it.
* @note For USGMII ports, cpss reads it from the integrated-PHY.
*
*/
GT_STATUS cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC      *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, entryPtr));

    rc = internal_cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead(devNum, portNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] FollowUpEn                           - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[in] SyncEn                               - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value;
    GT_BOOL isAc5y = GT_FALSE;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumMax;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(devNum,isAc5y);

    taiNumMax = (isAc5y == GT_FALSE) ? CPSS_DXCH_PTP_TAI_NUMBER_3_E : CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    if (taiNumber > taiNumMax)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg0[taiNumber];
    value = (BOOL2BIT_MAC(followUpEnable) << 1) | BOOL2BIT_MAC(syncEnable);
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 2, value);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] FollowUpEn                           - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[in] SyncEn                               - Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsMsgSelectionEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, followUpEnable, syncEnable));

    rc = internal_cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum, taiNumber, followUpEnable, syncEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, followUpEnable, syncEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] followUpEnablePtr                   - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[out] syncEnablePtr                       - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value;
    GT_BOOL isAc5y = GT_FALSE;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumMax;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(followUpEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(syncEnablePtr);
    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(devNum,isAc5y);

    taiNumMax = (isAc5y == GT_FALSE) ? CPSS_DXCH_PTP_TAI_NUMBER_3_E : CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    if (taiNumber > taiNumMax)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg0[taiNumber];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *followUpEnablePtr = BIT2BOOL_MAC((value >> 1) & 1);
    *syncEnablePtr = BIT2BOOL_MAC(value & 1);

    return GT_OK;
}

/**
* @internal cpssDxChPtpManager8021AsMsgSelectionEnableGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset enable/disable per message Type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[out] followUpEnablePtr                   - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a FollowUp message "FollowUp information TLV".
* @param[out] syncEnablePtr                       - (pointer to) Enable/Disable cumulativeScaledRateOffset setting
*                                                   in a Sync message "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsMsgSelectionEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, followUpEnablePtr, syncEnablePtr));

    rc = internal_cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum, taiNumber, followUpEnablePtr, syncEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, followUpEnablePtr, syncEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offset                               - value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_BOOL isAc5y = GT_FALSE;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumMax;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(devNum,isAc5y);

    taiNumMax = (isAc5y == GT_FALSE) ? CPSS_DXCH_PTP_TAI_NUMBER_3_E : CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    if (taiNumber > taiNumMax)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg1[taiNumber];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 32, offset);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet function
* @endinternal
*
* @brief   Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offset                               - value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, offset));

    rc = internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum, taiNumber, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offsetPtr                            - (pointer to) value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      *offsetPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_BOOL isAc5y = GT_FALSE;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumMax;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(devNum,isAc5y);

    taiNumMax = (isAc5y == GT_FALSE) ? CPSS_DXCH_PTP_TAI_NUMBER_3_E : CPSS_DXCH_PTP_TAI_NUMBER_0_E;
    if (taiNumber > taiNumMax)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).cumulativeScaleRateTaiReg1[taiNumber];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 32, offsetPtr);

    return rc;
}

/**
* @internal cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet function
* @endinternal
*
* @brief   Get PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV"
*          either in a Sync or a FollowUp message.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                               - physical device number
* @param[in] taiNumber                            - select TAI for timestamping
*                                                   (APPLICABLE VALUES: TAI_NUMBER_0..TAI_NUMBER_3).
* @param[in] offsetPtr                            - (pointer to) value of cumulativeScaledRateOffset to be set in
*                                                   the "FollowUp information TLV".
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, offsetPtr));

    rc = internal_cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum, taiNumber, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet function
* @endinternal
*
* @brief   Set MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] signatureMode            - MCH signature mode.
* @param[in] signatureValue           - MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
static GT_STATUS internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    signatureMode,
    IN  GT_U32                                                      signatureValue
)
{
    GT_STATUS rc;
    GT_U32 regAddr, regAddr1, value=0;
    GT_U32 portMacNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    if ((signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E) && (signatureValue > 0x3FFF))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portEmacUsxPchTxControl1;
        regAddr1 = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portPmacUsxPchTxControl1;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portEmacUsxPchTxControl1;
        regAddr1 = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portPmacUsxPchTxControl1;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        regAddr1 = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    switch (signatureMode)
    {
    case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E:
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 19, 1, 0); /* signature mode EMAC */
        if (rc != GT_OK)
        {
            return rc;
        }

        return prvCpssHwPpSetRegField(devNum, regAddr1, 19, 1, 0); /* signature mode PMAC */

    case CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E:
        value = (1 << 17);       /* signature mode */
        value |= signatureValue; /* signature value */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 18, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* load signature EMAC */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* un-load signature EMAC */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpSetRegField(devNum, regAddr1, 2, 18, value);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* load signature PMAC */
        rc = prvCpssHwPpSetRegField(devNum, regAddr1, 18, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* un-load signature PMAC */
        rc = prvCpssHwPpSetRegField(devNum, regAddr1, 18, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureSet function
* @endinternal
*
* @brief   Set MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] signatureMode            - MCH signature mode.
* @param[in] signatureValue           - MCH initial signature value.
*                                       valid when signatureMode == CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E.
*                                       (APPLICABLE RANGE: (0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureSet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    IN  CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    signatureMode,
    IN  GT_U32                                                      signatureValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortMacTsQueueSigatureSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signatureMode, signatureValue));

    rc = internal_cpssDxChPtpManagerPortMacTsQueueSigatureSet(devNum, portNum, signatureMode, signatureValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signatureMode, signatureValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet function
* @endinternal
*
* @brief   Get MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[out] signatureModePtr        - (pointer to) MCH signature mode.
* @param[out] signatureValuePtr       - (pointer to) MCH initial signature value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
static GT_STATUS internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    *signatureModePtr,
    OUT GT_U32                                                      *signatureValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr,value;
    GT_U32 portMacNum;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(signatureModePtr);
    CPSS_NULL_PTR_CHECK_MAC(signatureValuePtr);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portEmacUsxPchTxControl1;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portEmacUsxPchTxControl1;
    }
    else
    {
        regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 18, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (value >> 17)
    {
        *signatureModePtr = CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E;
        /* Signature[13:0] the 14 lsbits can be free running while the upper 2 msbits needs to be fixed '0'.*/
        *signatureValuePtr = value & 0x3FFF;
    }
    else
    {
        *signatureModePtr = CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E;
        /* Signature[15:0] */
        *signatureValuePtr = value & 0xFFFF;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPortMacTsQueueSigatureGet function
* @endinternal
*
* @brief   Get MCH signature configuration per port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[out] signatureModePtr        - (pointer to) MCH signature mode.
* @param[out] signatureValuePtr       - (pointer to) MCH initial signature value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*
*/
GT_STATUS cpssDxChPtpManagerPortMacTsQueueSigatureGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT                    *signatureModePtr,
    OUT GT_U32                                                      *signatureValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPortMacTsQueueSigatureGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, signatureModePtr, signatureValuePtr));

    rc = internal_cpssDxChPtpManagerPortMacTsQueueSigatureGet(devNum, portNum, signatureModePtr, signatureValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, signatureModePtr, signatureValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerTaiTodStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
    OUT CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    if (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    taiId.taiNumber = taiNumber;

    rc = cpssDxChPtpTaiTodStepGet(devNum,&taiId,todStepPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_2_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_3_E, (APPLICABLE DEVICES: Ironman)
*                                           CPSS_DXCH_PTP_TAI_NUMBER_4_E  (APPLICABLE DEVICES: Ironman))
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerTaiTodStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
    OUT CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerTaiTodStepGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, todStepPtr));

    rc = internal_cpssDxChPtpManagerTaiTodStepGet(devNum, taiNumber, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPhyTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured by the internal phy.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   EXTRA LIMITATIONS:       integrated-phy.
*
* @param[in] devNum                   - device number
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPhyTodValueGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    /* internal phy support */
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(todValueEntry1Ptr);

     /* Ironman-M internal PHY */
    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_IRONMAN_M_DEVICES_CASES_MAC:
        {
            /* In IM-M only single PHY1781; get captured FRC (Free Running Counter) */
            ret = mpdPerformOperationOnPhy(0 /* single internal phy number */, MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E, &opParams);
            if (MPD_OK_E != ret)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* ensure TOD0 value is captured */
            if (opParams.phyPtpCapturedFrc.todCapture0.valid == GT_TRUE)
            {
                todValueEntry0Ptr->todValueIsValid = GT_TRUE;
                todValueEntry0Ptr->todValue.fracNanoSeconds = opParams.phyPtpCapturedFrc.todCapture0.fractionalNanoseconds;
                todValueEntry0Ptr->todValue.nanoSeconds = opParams.phyPtpCapturedFrc.todCapture0.nanoseconds;
                todValueEntry0Ptr->todValue.seconds.l[1] = opParams.phyPtpCapturedFrc.todCapture0.highSeconds;
                todValueEntry0Ptr->todValue.seconds.l[0] = opParams.phyPtpCapturedFrc.todCapture0.lowSeconds;
            }

            /* ensure TOD1 value is captured */
            if (opParams.phyPtpCapturedFrc.todCapture1.valid == GT_TRUE)
            {
                todValueEntry1Ptr->todValueIsValid = GT_TRUE;
                todValueEntry1Ptr->todValue.fracNanoSeconds = opParams.phyPtpCapturedFrc.todCapture1.fractionalNanoseconds;
                todValueEntry1Ptr->todValue.nanoSeconds = opParams.phyPtpCapturedFrc.todCapture1.nanoseconds;
                todValueEntry1Ptr->todValue.seconds.l[1] = opParams.phyPtpCapturedFrc.todCapture1.highSeconds;
                todValueEntry1Ptr->todValue.seconds.l[0] = opParams.phyPtpCapturedFrc.todCapture1.lowSeconds;

            }
            break;
        }

        default:
                break;

    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPhyTodValueGet function
* @endinternal
*
* @brief  Get all valid TOD values that were captured by the internal phy.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   EXTRA LIMITATIONS:       integrated-phy.
*
* @param[in] devNum                   - device number
* @param[out] todValueEntry0Ptr       - (pointer to) captured TOD value of first entry.
* @param[out] todValueEntry1Ptr       - (pointer to) captured TOD value of second entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPhyTodValueGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPhyTodValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, todValueEntry0Ptr, todValueEntry1Ptr));

    rc = internal_cpssDxChPtpManagerPhyTodValueGet(devNum, todValueEntry0Ptr, todValueEntry1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, todValueEntry0Ptr, todValueEntry1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerPhyCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current PHY TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerPhyTodValueGet to retreive
*                captured TOD values.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   EXTRA LIMITATIONS:       integrated-phy.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerPhyCaptureTodValueSet
(
    IN GT_U8                                        devNum
)
{
    /* internal phy support */
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);

    /* Ironman-M internal PHY */
    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_IRONMAN_M_DEVICES_CASES_MAC:
        {
            /* In IM-M only single PHY1781; start FRC (Free Running Counter) capture */
            ret = mpdPerformOperationOnPhy(0 /* single internal phy number */, MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E, &opParams /* dummy */);
            if (MPD_OK_E != ret)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            break;
        }

        default:
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerPhyCaptureTodValueSet function
* @endinternal
*
* @brief   capture the current PHY TOD values.
*
* @note    NOTE: Call API cpssDxChPtpManagerPhyTodValueGet to retreive
*                captured TOD values.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   EXTRA LIMITATIONS:       integrated-phy.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerPhyCaptureTodValueSet
(
    IN GT_U8                                        devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerPhyCaptureTodValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPtpManagerPhyCaptureTodValueSet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerSlaveSerActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave Serial.
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] nanoSeconds              - nanoseconds part of SER RX latency.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] fracNanoSeconds          - fractional nanosecond part of SER RX latency; the input is the numerator while the denominator Equals 2^32.
*                                      (APPLICABLE RANGES: -0.5..0.46875 ns)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
static GT_STATUS internal_cpssDxChPtpManagerSlaveSerActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                nanoSeconds,
    IN  GT_32                                 fracNanoSeconds
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 regAddr;
    GT_U32 numIterations=1;
    GT_U32 bFracNanoSeconds;
    GT_FLOAT64 fFracNanoSeconds;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    /* convert fractional nanosecond RX SER latency value from integer to float */
    rc = prvCpssDxChPtpManagerConvertIntegerToFloat(fracNanoSeconds,&fFracNanoSeconds);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (0.46875 < fFracNanoSeconds || fFracNanoSeconds < -0.5)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    if (nanoSeconds > 0xFFFF)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    /* convert fractional nanosecond RX SER latency value from float to Binary */
    rc = prvCpssDxChPtpManagerConvertFloatToBinary(fFracNanoSeconds,&bFracNanoSeconds);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable SER RX input */
        rc = cpssDxChPtpTaiSerialReceptionModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_SER_RECEPTION_MODE_SERIAL_RECEPTION_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* TAI SER-TX to SER-RX latency in nano-seconds. */
        regAddr  = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum,0, taiId.taiNumber).frameLatency;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, nanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* fractional nano-second part of the SER-TX to SER-RX latency. */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 24, 8, bFracNanoSeconds);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerSlaveSerActivate function
* @endinternal
*
* @brief   Activating the TAI as Slave Serial.
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
* @param[in] nanoSeconds              - nanoseconds part of SER RX latency.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] fracNanoSeconds          - fractional nanosecond part of SER RX latency; the input is the numerator while the denominator Equals 2^32.
*                                      (APPLICABLE RANGES: -0.5..0.46875 ns)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
*/
GT_STATUS cpssDxChPtpManagerSlaveSerActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                nanoSeconds,
    IN  GT_32                                 fracNanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerSlaveSerActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, nanoSeconds, fracNanoSeconds));

    rc = internal_cpssDxChPtpManagerSlaveSerActivate(devNum, taiNumber, nanoSeconds, fracNanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, nanoSeconds, fracNanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerMasterSerActivate function
* @endinternal
*
* @brief   Activating the TAI as Master Serial.
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerMasterSerActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber
)
{
    GT_STATUS rc;
    CPSS_DXCH_PTP_TAI_ID_STC taiId;
    GT_U32 regAddr;
    GT_U32 numIterations=1;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AAS_E);
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_CHECK_MAC(devNum,taiNumber);

    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_START_MAC(devNum, taiNumber, &taiId)
    {
        /* enable SER TX output */
        rc = cpssDxChPtpTaiSerialGenerationModeSet(devNum,&taiId,CPSS_DXCH_PTP_TAI_SER_GEN_MODE_SERIAL_GEN_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Apply SW reset to the Master TAI Tx */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, 0, taiId.taiNumber).TAICtrlReg0;
        /* Soft reset for SER-TX logic: 0x1 = Reset Enter */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, 0x1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set Master TAI Manchester half-bit time */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, 0, taiId.taiNumber).manchesterControl;
        rc = prvCpssHwPpSetRegField(devNum, regAddr , 0, 15, 0x31);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Release the SW reset of the Master TAI Tx */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, 0, taiId.taiNumber).TAICtrlReg0;
        /* Soft reset for SER-TX logic: 0x0 = Reset Exit; normal mode */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, 0x0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_MANAGER_TAI_ITERATOR_LOOP_END_MAC(taiNumber, &taiId)

    return GT_OK;
}

/**
* @internal cpssDxChPtpManagerMasterSerActivate function
* @endinternal
*
* @brief   Activating the TAI as Master Serial.
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerMasterSerActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerMasterSerActivate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber));

    rc = internal_cpssDxChPtpManagerMasterSerActivate(devNum, taiNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpManagerDbgPtpPLLConfig function
* @endinternal
*
* @brief   Debug API: config the PTP PLL parameters to support different frequencies.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES: Ironman.
*
* @param[in] devNum                   - device number
* @params[in] refClockSource          - reference clock source
* @param[in] clockFrequency           - PTP PLL frequency
* @param[in] taiNumber                - TAI Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpManagerDbgPtpPLLConfig
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT      refClockSource,
    IN CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT        clockFrequency,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
)
{
    return prvCpssDxChPtpSip6PLLConfig(devNum,refClockSource,clockFrequency,taiNumber);
}

/**
* @internal cpssDxChPtpManagerDbgPtpPLLConfig function
* @endinternal
*
* @brief   Debug API: config the PTP PLL parameters to support different frequencies.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES: Ironman.
*
* @param[in] devNum                   - device number
* @params[in] refClockSource          - reference clock source
* @param[in] clockFrequency           - PTP PLL frequency
* @param[in] taiNumber                - TAI Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpManagerDbgPtpPLLConfig
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT      refClockSource,
    IN CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT        clockFrequency,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpManagerDbgPtpPLLConfig);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, refClockSource, clockFrequency, taiNumber));

    rc = internal_cpssDxChPtpManagerDbgPtpPLLConfig(devNum, refClockSource, clockFrequency, taiNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, refClockSource, clockFrequency, taiNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
