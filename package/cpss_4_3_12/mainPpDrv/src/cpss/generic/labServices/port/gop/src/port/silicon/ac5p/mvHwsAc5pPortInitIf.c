/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAc5pPortInitIf.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#if !defined (MICRO_INIT) && !defined (IRONMAN_DEV_SUPPORT)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ironman/mvHwsIronmanPortIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#if (!defined MV_HWS_REDUCED_BUILD) || defined (IRONMAN_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/mvHwsIronmanPortAnp.h>
#endif

extern  GT_BOOL hwsPpHwTraceFlag;

#if (!defined MV_HWS_REDUCED_BUILD) || defined (MICRO_INIT) || defined (IRONMAN_DEV_SUPPORT)

/**
* @internal mvHwsHawkPortCommonInit function
* @endinternal
*
* @brief   Falcon port common part (regular and AP) init
* (without mac/psc and unreset - thet is done from firmware)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - port info struct
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsHawkPortCommonInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS *portInitInParamPtr
)
{
    GT_STATUS                   rc;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      startPort = phyPortNum;
    GT_U32                      numOfPortsInGroup, i;
    MV_HWS_PORT_IN_GROUP_ENT    portFullConfig;
    GT_U32                      usxStep = 1;
    GT_BOOL                     singlePortInit;

    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    if (HWS_PAM4_MODE_CHECK(portMode))
    {
        curPortParams.portFecMode = RS_FEC_544_514;
    }
#ifndef IRONMAN_DEV_SUPPORT
    if(Ironman != HWS_DEV_SILICON_TYPE(devNum))
    {
        CHECK_STATUS(mvHwsMifChannelEnable(devNum, phyPortNum, portMode, GT_TRUE, portInitInParamPtr->isPreemptionEnabled));
    }

    /* USG modes - QSGMII and 10G_OUSGMII does not support send local fault feature in order to solve
       false link up indication problem  */
    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
#endif
    {
        singlePortInit = (portMode == _10G_QXGMII) ? GT_FALSE : GT_TRUE;
    }
#ifndef IRONMAN_DEV_SUPPORT
    else
    {
        singlePortInit = (HWS_USX_SLOW_MODE_CHECK(portMode) || curPortParams.portMacType != MTI_USX_MAC) ? GT_TRUE : GT_FALSE;
    }
#endif

    if(singlePortInit == GT_TRUE)
    {
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
        if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
        {
            CHECK_STATUS(mvHwsIronmanEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));
        }
#ifndef IRONMAN_DEV_SUPPORT
        else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
        {
            CHECK_STATUS(mvHwsEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));
        }
#endif
    }
    else
    {

    /*
       Problem:
        Each MAC has its own PCS following by single shared multiplexing logic.
        When powering up the first port, due to common reset and clock gate to
        all PCSs, what happens is that all of our PCS get lock hence pcs link
        indication for all PCSs is reached! As our MACs reset value towards
        the PCS is IDLEs, the link partner get to link on ports which we did
        not initialize on our side.

       Solution:
        To configure all our MACs to send local_fault before powering
        up the first MAC, thus although PCS link is achieved, the link partner
        will get local fault hence will keep its link indication down.
    */
        CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum, phyPortNum, portMode, &portFullConfig));

        if(portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E)
        {
            rc = mvHwsUsxExtSerdesGroupParamsGet(devNum, phyPortNum, portMode, &startPort, &usxStep, &numOfPortsInGroup);
            CHECK_STATUS(rc);
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
            /* the requested port should be configured first because of fec mode from port element table
               that may be diffrent from other ports in bundle*/
            if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
            {
                CHECK_STATUS(mvHwsIronmanEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));
            }
#ifndef IRONMAN_DEV_SUPPORT
            else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
            {
                CHECK_STATUS(mvHwsEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));
            }
#endif

            for (i = startPort; numOfPortsInGroup > 0; i+=usxStep)
            {
                if(i != phyPortNum)
                {
#ifndef  MV_HWS_FREE_RTOS
                    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                        hwsOsPrintf("****** Start of mvHwsPortSendLocalFaultSet for portNum=%d ******\n", i);
                        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
                    }
#endif

                    /* align FEC mode of all ports in USX group accoring to the the first */
                    CHECK_STATUS(hwsPortModeParamsSetFec(devNum, 0, i, portMode, curPortParams.portFecMode));

                    /*  port local fault enable */
                    CHECK_STATUS(mvHwsPortSendLocalFaultSet(devNum, 0, i, portMode, GT_TRUE));

#ifndef  MV_HWS_FREE_RTOS
                    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                        hwsOsPrintf("****** End of mvHwsPortSendLocalFaultSet for portNum=%d ******\n", i);
                        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
                    }
#endif
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
                    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
                    {
                        CHECK_STATUS(mvHwsIronmanEthPortPowerUp(devNum, i, portMode, portInitInParamPtr));
                    }
#ifndef IRONMAN_DEV_SUPPORT
                    else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
                    {
                        CHECK_STATUS(mvHwsEthPortPowerUp(devNum, i, portMode, portInitInParamPtr));
                    }
#endif
                }
                numOfPortsInGroup--;
            }
        }
        else
        {
#ifndef  MV_HWS_FREE_RTOS
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsOsPrintf("****** Start of mvHwsPortSendLocalFaultSet for portNum=%d ******\n", phyPortNum);
            }
#endif
            /* if port changes its speed within the common port mode - need to set replication factor:
               example:
               - 8 ports - 2500G (mode USX_20G_OXGMII)
               - partial power down to 1 port
               - partial power up with different speed 1G (mode USX_20G_OXGMII)
            */
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
            if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
            {
                CHECK_STATUS(mvHwsIronmanReplicationSet(devNum, 0, phyPortNum, portMode, portInitInParamPtr, GT_FALSE));
            }
#ifndef IRONMAN_DEV_SUPPORT
            else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
            {
                CHECK_STATUS(mvHwsMtiUsxReplicationSet(devNum, 0, phyPortNum, portMode, portInitInParamPtr, GT_FALSE));
            }
#endif
            /*  port local fault disable */
            CHECK_STATUS(mvHwsPortSendLocalFaultSet(devNum, 0, phyPortNum, portMode, GT_FALSE));

#ifndef  MV_HWS_FREE_RTOS
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsOsPrintf("****** End of mvHwsPortSendLocalFaultSet for portNum=%d ******\n", phyPortNum);
            }
#endif
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsHawkPortInit function
* @endinternal
*
* @brief   Falcon port init
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - Input parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsHawkPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    /*GT_BOOL serdesInit = GT_TRUE;*/
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    GT_U32 sdIndex;
    MV_HWS_PORT_IN_GROUP_ENT    portFullConfig = MV_HWS_PORT_IN_GROUP_FIRST_E;
#ifndef IRONMAN_DEV_SUPPORT
    GT_BOOL isANPStatic = GT_FALSE;
#endif

    if (NULL == portInitInParamPtr)
    {
        return GT_BAD_PTR;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d create mode %d ******\n", phyPortNum, portMode);
    }
#endif
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

#ifndef IRONMAN_DEV_SUPPORT
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
    {
        CHECK_STATUS(mvHwsIronmanAnpPortEnable(devNum, phyPortNum, portMode, portInitInParamPtr->apEnabled, GT_TRUE));
    }
    else
#endif
    {
        CHECK_STATUS(mvHwsAnpPortEnable(devNum, phyPortNum, portMode, portInitInParamPtr->apEnabled, GT_TRUE));
    }
#endif

    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
#ifndef IRONMAN_DEV_SUPPORT
    mvHwsAnpGetStaticPort(devNum,phyPortNum,&isANPStatic);


    if (( portInitInParamPtr->apEnabled == GT_FALSE ) && (isANPStatic == GT_FALSE))
#endif
    {
        if((curPortParams.portMacType == MTI_USX_MAC) ||
           (Ironman == HWS_DEV_SILICON_TYPE(devNum)))
        {
            CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum, phyPortNum, portMode, &portFullConfig));
        }

        if(portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E)
        {
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

            for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
            {
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT) /*MB- TBD*/
                if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
                {
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curLanesList[sdIndex], SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_0_TX_IDLE_E, 1, NULL));
                }
#ifndef IRONMAN_DEV_SUPPORT
                else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
                {
                    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                            curLanesList[sdIndex],
                                                            SDW_LANE_CONTROL_0,
                                                            0x1000000,
                                                            0x1000000));

                    CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, curLanesList[sdIndex], GT_FALSE));
                }
#endif
            }

            /* power up the serdes */
            CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
#if !defined(MICRO_INIT) && !defined(IRONMAN_DEV_SUPPORT)
            if(!cpssDeviceRunCheck_onEmulator())
            {
#endif
                CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
#if !defined(MICRO_INIT) && !defined(IRONMAN_DEV_SUPPORT) /*MB- TBD*/
            }
#endif
        }
    }
    /* init all modules as in ap port (msdb,mpf,mtipExt) */
    CHECK_STATUS(mvHwsHawkPortCommonInit(devNum,portGroup,phyPortNum,portMode,portInitInParamPtr));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

#ifndef IRONMAN_DEV_SUPPORT
/**
* @internal mvHwsHawkPortAnpStart function
* @endinternal
*
* @brief   Hawk port anp start (start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsHawkPortAnpStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_AP_CFG                  *apCfgPtr
)
{
    GT_UNUSED_PARAM(portGroup);
    HWS_NULL_PTR_CHECK_MAC(apCfgPtr);

    CHECK_STATUS(mvHwsAnpPortStart(devNum,phyPortNum,apCfgPtr));
    return GT_OK;
}
#endif

#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT) && !defined(IRONMAN_DEV_SUPPORT)
/**
* @internal mvHwsHawkPortApReset function
* @endinternal
*
* @brief   Clears the AP port mode and release all its resources
*         according to selected. Does not verify that the
*         selected mode/port number is valid at the core level
*         and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHawkPortApReset
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode
)
{

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** AP port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

#if 0
    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, portGroup,phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMsdbConfigChannel(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsMtipExtLowJitterEnable(devNum, phyPortNum, portMode, GT_FALSE));

    /* TODO - add support for power reduction
    CHECK_STATUS(mvHwsMtipExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE));*/
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}
#endif

/**
* @internal mvHwsHawkPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsHawkPortReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION  action
)
{
    GT_STATUS rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                      startPort = phyPortNum;
    GT_U32                      numOfPortsInGroup, i;
    MV_HWS_PORT_IN_GROUP_ENT stateInPortGroup = MV_HWS_PORT_IN_GROUP_LAST_E;
    MV_HWS_PORT_IN_GROUP_ENT stateInSerdesGroup = MV_HWS_PORT_IN_GROUP_LAST_E;
    GT_U32 usxStep = 1;
    GT_BOOL singlePortInit;

    action = action;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

#ifndef IRONMAN_DEV_SUPPORT
    if (!HWS_DEV_SIP_6_30_CHECK_MAC(devNum))
    {
        CHECK_STATUS(mvHwsMifChannelEnable(devNum, phyPortNum, portMode, GT_FALSE, GT_TRUE));
    }
#endif
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
    {
        CHECK_STATUS(mvHwsIronmanAnpPortEnable(devNum, phyPortNum, portMode, GT_FALSE/*don't care*/, GT_FALSE));
    }
#ifndef IRONMAN_DEV_SUPPORT
    else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
    {
        CHECK_STATUS(mvHwsAnpPortEnable(devNum, phyPortNum, portMode, GT_FALSE/*don't care*/, GT_FALSE));
    }
#endif
    /* should be done before mvHwsExtMacClockEnable() that disables clocks */
    CHECK_STATUS(mvHwsExtIfLastInPortGroupCheck(devNum, phyPortNum, portMode, &stateInPortGroup));
    CHECK_STATUS(mvHwsExtIfLastInSerdesGroupCheck(devNum, phyPortNum, portMode, &stateInSerdesGroup));

    /* USG modes - QSGMII and 10G_OUSGMII does not support send local fault feature in order to solve
       false link up indication problem  */
#ifndef IRONMAN_DEV_SUPPORT
    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
#endif
    {
        singlePortInit = (portMode == _10G_QXGMII) ? GT_FALSE : GT_TRUE;
    }
#ifndef IRONMAN_DEV_SUPPORT
    else
    {
        singlePortInit = (HWS_USX_SLOW_MODE_CHECK(portMode) || curPortParams.portMacType != MTI_USX_MAC) ? GT_TRUE : GT_FALSE;
    }
#endif

    if(singlePortInit == GT_TRUE)
    {
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
        if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
        {
            CHECK_STATUS(mvHwsIronmanEthPortPowerDown(devNum, phyPortNum, portMode, stateInPortGroup, stateInSerdesGroup));
        }
#ifndef IRONMAN_DEV_SUPPORT
        else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
        {
            CHECK_STATUS(mvHwsEthPortPowerDown(devNum, phyPortNum, portMode, stateInPortGroup, stateInSerdesGroup));
        }
#endif
    }
    else
    {
        /*
           Problem:
            Each MAC has its own PCS following by single shared multiplexing logic.
            When powering up the first port, due to common reset and clock gate to
            all PCSs, what happens is that all of our PCS get lock hence pcs link
            indication for all PCSs is reached! As our MACs reset value towards
            the PCS is IDLEs, the link partner get to link on ports which we did
            not initialize on our side.

           Solution:
            idea to configure all our MACs to send local_fault before powering
            up the first MAC, thus although PCS link is achieved, the link partner
            will get local fault hence will keep its link indication down.
        */
        if(stateInSerdesGroup == MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            rc = mvHwsUsxExtSerdesGroupParamsGet(devNum, phyPortNum, portMode, &startPort, &usxStep, &numOfPortsInGroup);
            CHECK_STATUS(rc);

            for (i = startPort; numOfPortsInGroup > 0; i+=usxStep)
            {
                if(i != phyPortNum)
                {
                    #ifndef  MV_HWS_FREE_RTOS
                    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
                        hwsOsPrintf("****** Start of mvHwsPortSendLocalFaultSet for phyPortNum=%d ******\n", i);
                    }
                    #endif

                    CHECK_STATUS(mvHwsPortSendLocalFaultSet(devNum, 0, i, portMode, GT_FALSE));

                    #ifndef  MV_HWS_FREE_RTOS
                    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
                        hwsOsPrintf("****** End of mvHwsPortSendLocalFaultSet for phyPortNum=%d ******\n", i);
                    }
                    #endif
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
                    if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
                    {
                        CHECK_STATUS(mvHwsIronmanEthPortPowerDown(devNum, i, portMode, MV_HWS_PORT_IN_GROUP_EXISTING_E, MV_HWS_PORT_IN_GROUP_EXISTING_E));
                    }
#ifndef IRONMAN_DEV_SUPPORT
                    else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
                    {
                        CHECK_STATUS(mvHwsEthPortPowerDown(devNum, i, portMode, MV_HWS_PORT_IN_GROUP_EXISTING_E, MV_HWS_PORT_IN_GROUP_EXISTING_E));
                    }
#endif
                }
                numOfPortsInGroup--;
            }
#if !defined(HARRIER_DEV_SUPPORT) && !defined(AC5P_DEV_SUPPORT)
            /* close the latest port is SD group */
            if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
            {
                CHECK_STATUS(mvHwsIronmanEthPortPowerDown(devNum, phyPortNum, portMode, stateInPortGroup, MV_HWS_PORT_IN_GROUP_LAST_E));
            }
#ifndef IRONMAN_DEV_SUPPORT
            else
#endif
#endif
#ifndef IRONMAN_DEV_SUPPORT
            {
                CHECK_STATUS(mvHwsEthPortPowerDown(devNum, phyPortNum, portMode, stateInPortGroup, MV_HWS_PORT_IN_GROUP_LAST_E));
            }
#endif
        }
        else
        {
#ifndef  MV_HWS_FREE_RTOS
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
                hwsOsPrintf("****** Start of mvHwsPortSendLocalFaultSet for phyPortNum=%d ******\n", phyPortNum);
            }
#endif
            CHECK_STATUS(mvHwsPortSendLocalFaultSet(devNum, 0, phyPortNum, portMode, GT_TRUE));

#ifndef  MV_HWS_FREE_RTOS
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
                hwsOsPrintf("****** End of mvHwsPortSendLocalFaultSet for phyPortNum=%d ******\n", phyPortNum);
            }
#endif
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}
#endif

