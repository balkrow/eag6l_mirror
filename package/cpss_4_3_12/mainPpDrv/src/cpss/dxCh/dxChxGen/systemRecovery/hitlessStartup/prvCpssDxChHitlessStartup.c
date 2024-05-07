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
* @file prvCpssDxChHitlessStartup.c
*
* @brief private CPSS DxCh hitless startup facility API.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hitlessStartup/private/prvCpssDxChHitlessStartup.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelHandler.h>

#define PRV_SERVICE_CPU_GLOBAL_CONFIGURATION_OFFSET 0x500 /*offset is in bytes*/

extern GT_U32 prvCpssDrPpConInitMg0UnitBaseAddressGet
(
     IN GT_U8      devNum
);

extern GT_STATUS prvFindCauseRegAddrByEvent
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *regAddr
);

extern GT_STATUS prvFindIntCauseBit
(
    GT_U8          devNum,
    GT_U32         portGroupId,
    CPSS_UNI_EV_CAUSE_ENT  uniEvent,
    GT_U32         evExtData/*portNum*/,
    GT_U32         *bit
);


static const CPSS_UNI_EV_CAUSE_ENT  hitlessStartupClearEventArr[] = {
    CPSS_PP_PORT_AN_HCD_FOUND_E,
    CPSS_PP_PORT_AN_RESTART_E,
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    /*must be last*/
    CPSS_UNI_RSRVD_EVENT_E};


/*get init state register address */
static GT_STATUS prvHitlessStartupInitStateRegAddrGet
(
    GT_U8 devNum,
    GT_U32 *addressPtr
)
{
    GT_U32 regAddr = 0x0;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl18;
    }
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl2;
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("\n prvHitlessStartupInitStateRegAddrGet returned GT_NOT_SUPPORTED,initState reg is not set for this device");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    *addressPtr = regAddr ;
    CPSS_LOG_INFORMATION_MAC("\n prvHitlessStartupInitStateRegAddrGet returned initState regAddr 0x%x ",regAddr);
    return GT_OK ;
}

/*set value to init state register */
GT_STATUS prvHitlessStartupInitStateSet
(
    GT_U8 devNum,
    GT_U32 value
)
{
    GT_STATUS rc ;
    GT_U32 regAddr;

    rc = prvHitlessStartupInitStateRegAddrGet(devNum,&regAddr);
    if (rc != GT_OK)
    {
        return rc ;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,value);
    if (rc != GT_OK)
    {
        return rc ;
    }
    return GT_OK ;
}

/*get value of init state register*/
GT_STATUS prvHitlessStartupInitStateGet
(
    GT_U8 devNum,
    GT_U32 *valuePtr
)
{
    GT_STATUS rc ;
    GT_U32 regAddr ;

    rc = prvHitlessStartupInitStateRegAddrGet(devNum,&regAddr);
    if (rc != GT_OK)
    {
        return rc ;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,valuePtr);
    if (rc != GT_OK)
    {
        return rc ;
    }
    return GT_OK ;
}

/**
* @internal prvCpssDxChHitlessStartupStateCheckAndSet function
* @endinternal
*
* @brief   This function checks expected Init State register value and set new value
*/
GT_STATUS prvCpssDxChHitlessStartupStateCheckAndSet
(
    GT_U8 devNum,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT expValue,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT newValue
)
{
    GT_STATUS rc ;
    GT_U32 value = 0;

    if (expValue > PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E ||
        newValue > PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E ||
        newValue < expValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvHitlessStartupInitStateGet(devNum,&value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG.hsSkipDfxInitStateCheck == GT_FALSE)
    {
        if (expValue != value)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        cpssOsPrintf("\n skip HS DFX init State check - debug mode\n");
    }

    rc = prvHitlessStartupInitStateSet(devNum, newValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHitlessStartupPortConfiguredCheck function
* @endinternal
*
* @brief   This function checks if port configured by MI
*/
GT_STATUS prvCpssDxChHitlessStartupPortConfiguredCheck
(
    GT_U8                                 devNum,
    GT_PHYSICAL_PORT_NUM                  portNum,
    GT_BOOL                               *isConfigure
)
{
    GT_STATUS rc ;
    PRV_CPSS_BOOT_CH_PORT_STATUS_STC      portInfo;

    *isConfigure = GT_FALSE;
    rc = prvCpssDxChBootChannelHandlerPortStatusGet(devNum, portNum, &portInfo);
    if (rc != GT_OK)
    {
        /*in case of port not configured by MI GT_NOT_FOUND is returned */
        if (rc == GT_NOT_FOUND)
        {
            return GT_OK;
        }
        else
            return rc;
    }

    *isConfigure = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvHitlessStartupMiClose function
* @endinternal
*
* @brief   This function close MI process after HS process finished
*/
GT_STATUS prvHitlessStartupMiClose
(
    GT_U8 devNum
)
{
    GT_STATUS rc ;
    GT_U32 regAddr , value, initStatus = 0;

    regAddr = prvCpssDrPpConInitMg0UnitBaseAddressGet(devNum) + PRV_SERVICE_CPU_GLOBAL_CONFIGURATION_OFFSET ;
    rc = cpssDrvPpHwRegisterRead(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,&value);
    if (rc != GT_OK)
    {
        return rc ;
    }


    rc = prvHitlessStartupInitStateGet(devNum, &initStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*check that HS done and MI can be close */
    if (initStatus != PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E)
    {
        CPSS_LOG_INFORMATION_MAC("\nprvHitlessStartupMiClose returned without reset ,initStatus != PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E");
        return GT_OK;
    }

    /*reset bits 28,29*/
    value = value & 0xCFFFFFFF;
    rc = cpssDrvPpHwRegisterWrite(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    CPSS_LOG_INFORMATION_MAC("\n close MI process");
    return GT_OK ;
}

static GT_STATUS prvHitlessStartupEventClean
(
    IN GT_U8               devNum,
    IN GT_U32              portGroup,
    IN CPSS_UNI_EV_CAUSE_ENT      uniEv,
    IN GT_U32      portNum
)
{
    GT_U32 rc;
    GT_U32 regAddr=0x0 ,causeBit;
    GT_U32 regValue;

    rc = prvFindCauseRegAddrByEvent(devNum,portGroup, uniEv/*uniEv*/, portNum, &regAddr);
    if (rc != GT_OK)
    {
        return rc ;
    }

    rc = prvFindIntCauseBit(devNum,portGroup,uniEv,portNum,&causeBit);
    if (rc != GT_OK)
    {
        return rc ;
    }



    /* Clear the interrupt cause register. */
    rc = cpssDrvPpHwRegBitMaskRead(devNum, portGroup, regAddr, 0xFFFFFFFF, &regValue);
    if (rc != GT_OK)
    {
        return rc ;
    }

    CPSS_LOG_INFORMATION_MAC("check port %d uniEv %d reg 0x%x regValue 0x%x bit 0x%x bitValue  0x%x",
                             portNum,uniEv,
                             regAddr,regValue,
                             causeBit,(causeBit&regValue));
    if ((causeBit & regValue ) != 0)
    {
        /* change the interrupt bit in cause register.  */
        regValue = regValue & ~causeBit ;
        CPSS_LOG_INFORMATION_MAC("bit is up ,set bit to 0");
    }

    /*return register values that doesnt related to interrupt */
    rc = cpssDrvPpHwRegBitMaskWrite(devNum, portGroup, regAddr, 0xFFFFFFFF, regValue);
    if (rc != GT_OK)
    {
        return rc ;
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChHitlessStartupEventClean
(
    IN GT_U8      devNum
)
{

    GT_STATUS rc;
    GT_U32 portMacNum,ii;
    GT_PHYSICAL_PORT_NUM          portNum;
    CPSS_PM_PORT_PARAMS_STC       portParams;
    CPSS_UNI_EV_CAUSE_ENT uniEv;

    for (portMacNum = 0; portMacNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portMacNum++)
    {

            PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum,portMacNum);

            /* convert MAC number to physical port number */
            rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                portMacNum, &portNum);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChPortManagerPortParamsGet(devNum,portNum,&portParams);
            if (GT_OK != rc)
            {
                if(rc == GT_NOT_INITIALIZED)
                {
                    continue;
                }
            }

            ii=0;
            while(hitlessStartupClearEventArr[ii] != CPSS_UNI_RSRVD_EVENT_E)
            {
                uniEv = hitlessStartupClearEventArr[ii];
                PRV_CPSS_INT_SCAN_LOCK();
                rc = prvHitlessStartupEventClean(devNum, 0, uniEv, portNum);
                PRV_CPSS_INT_SCAN_UNLOCK();
                if(GT_OK != rc)
                {
                    return rc;
                }
                ii++;
            }
    }
    return GT_OK;
}



