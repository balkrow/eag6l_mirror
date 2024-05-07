
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
* @file prvCpssDxChPhyMpd.c
*
* @brief PHY management through MPD( Marvell PHY driver)
*
* @version   1
********************************************************************************
*/

#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include<cpss/dxCh/dxChxGen/phy/prvCpssDxChPhyMpdInit.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>

static MPD_SAMPLE_DEBUG_FLAG_INFO_STC mpdDebugFlags[MPD_NUM_OF_DEBUG_FLAGS_CNS];
static UINT_32 MPD_DEBUG_FREE_INDEX = 0;

static GT_U32 nextFreeGlobalIfIndex = 1;

static GT_U32 prvCpssPhyMpdIndexGet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32        portNum
)
{
    GT_U32 mpdIndex;
    GT_U32 portMacNum;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    mpdIndex = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].mpdIndex;
    if(mpdIndex == 0)
    {
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].mpdIndex = nextFreeGlobalIfIndex;
        return nextFreeGlobalIfIndex++;
    }
    else
    {
        return mpdIndex;
    }
}

static GT_STATUS internal_macPhySpeedSet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    GT_U32         mpdIndex;
    GT_U32         portMacNum;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_STATUS      rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
       return GT_OK;
    }

    memset(&opParams, 0, sizeof(opParams));

    /* speed can be defined as 10, 100 or 1000 for 1781 */
    if (speed > CPSS_PORT_SPEED_1000_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        CPSS_TO_MPD_SPEED(speed, opParams.phySpeed.speed);

        if(opParams.phySpeed.speed == MPD_SPEED_LAST_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        MPD_DBG(("Speed set [%d] : mpdIndex:%d speed: %d \n", portNum, mpdIndex, speed));
        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_SPEED_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedSetPtr = speed;

        return GT_OK;
    }
    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_macPhySpeedGet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_SPEED_ENT     *speedPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    GT_U32          mpdIndex;
    GT_U32          portMacNum;
    MPD_RESULT_ENT  ret = MPD_OK_E;

    MPD_OPERATIONS_PARAMS_UNT opParams;
    memset(&opParams, 0, sizeof(opParams));

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);
        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        MPD_TO_CPSS_SPEED(opParams.phyInternalOperStatus.phySpeed, *speedPtr);

        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchSpeedSetPtr = *speedPtr;

        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_macPhyFlowCntlANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT GT_BOOL                 *targetPauseAdvertise
)
{
    GT_U32         mpdIndex;
    GT_U32         portMacNum;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_STATUS      rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
       return GT_OK;
    }

    memset(&opParams, 0, sizeof(opParams));

    GT_UNUSED_PARAM(pauseAdvertise);

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {
        opParams.phyFc.advertiseFc = (state == GT_TRUE) ? TRUE: FALSE;

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_ADVERTISE_FC_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *targetState = state;
        *targetPauseAdvertise = pauseAdvertise;
        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_macPhyDuplexSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DUPLEX_ENT        dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    GT_U32         mpdIndex;
    GT_U32         portMacNum;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_STATUS      rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
       return GT_OK;
    }

    memset(&opParams, 0, sizeof(opParams));

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        opParams.phyDuplex.mode = (dMode == CPSS_PORT_FULL_DUPLEX_E) ? MPD_DUPLEX_ADMIN_MODE_FULL_E : MPD_DUPLEX_ADMIN_MODE_HALF_E;

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        MPD_DBG(("Duplex set [%d] : mpdIndex:%d duplexMode: %d \n", portNum, mpdIndex, dMode));

        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_DUPLEX_MODE_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *switchDuplexSetPtr = dMode;
        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_macPhyDuplexGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT  CPSS_PORT_DUPLEX_ENT       *dModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(dModePtr);
    GT_UNUSED_PARAM(stage);
    GT_UNUSED_PARAM(doPpMacConfigPtr);
    GT_UNUSED_PARAM(switchDuplexSetPtr);
    return GT_OK;
}

static GT_STATUS internal_macPhyPortPowerDownSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     powerDown,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_U32         mpdIndex;
    GT_U32         portMacNum;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_STATUS      rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
       return GT_OK;
    }

    memset(&opParams, 0, sizeof(opParams));

    if (stage == CPSS_MACDRV_STAGE_PRE_E)
    {

        opParams.phyDisable.forceLinkDown = (powerDown == GT_TRUE) ? TRUE : FALSE;

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        MPD_DBG(("Port power down set [%d] : mpdIndex:%d down: %d \n", portNum, mpdIndex, powerDown));

        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_PHY_DISABLE_OPER_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_macPhyPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     enable,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(enable);
    GT_UNUSED_PARAM(stage);
    GT_UNUSED_PARAM(doPpMacConfigPtr);
    GT_UNUSED_PARAM(targetEnableSetPtr);
    return GT_OK;
}

static GT_STATUS internal_macPhyPortEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *enablePtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(enablePtr);
    GT_UNUSED_PARAM(stage);
    GT_UNUSED_PARAM(doPpMacConfigPtr);
    GT_UNUSED_PARAM(targetEnableSetPtr);
    return GT_OK;
}

#ifdef PRV_MPD_DEBUG  /* for debug */
GT_STATUS portstatus
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum
)
{
    GT_U32          mpdIndex;
    GT_U32          portMacNum;
    MPD_RESULT_ENT  ret = MPD_OK_E;

    MPD_OPERATIONS_PARAMS_UNT opParams;
    memset(&opParams, 0, sizeof(opParams));

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

    MPD_DBG(("Port status [%d] : mpdIndex:%d \n", portNum, mpdIndex));
    ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &opParams);
    if ( MPD_OK_E != ret )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    MPD_DBG(("Port status [%d] : mpdIndex:%d isLinkUp: %d \n", portNum, mpdIndex, opParams.phyInternalOperStatus.isOperStatusUp));
    MPD_DBG(("Port status [%d] : mpdIndex:%d AN DOne: %d \n", portNum, mpdIndex, opParams.phyInternalOperStatus.isAnCompleted));
    MPD_DBG(("Port status [%d] : mpdIndex:%d phy speed: %d \n", portNum, mpdIndex, opParams.phyInternalOperStatus.phySpeed));
    return GT_OK;

}
#endif

/*(15)*/static GT_STATUS internal_macPhyPortLinkGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isLinkUpPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchLinkStatusGetPtr
)
{
    GT_U32          mpdIndex;
    GT_U32          portMacNum;
    MPD_RESULT_ENT  ret = MPD_OK_E;

    MPD_OPERATIONS_PARAMS_UNT opParams;
    memset(&opParams, 0, sizeof(opParams));

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* set the same value in switch MAC */
        *doPpMacConfigPtr = GT_TRUE;
        *isLinkUpPtr = opParams.phyInternalOperStatus.isOperStatusUp;
        /*MPD_DBG(("Port Enable set [%d] : mpdIndex:%d isLinkUp: %d \n", portNum, mpdIndex, *isLinkUpPtr));*/
        *switchLinkStatusGetPtr = *isLinkUpPtr;
        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static GT_STATUS internal_portLoopbackGet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(enablePtr);
    GT_UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_TRUE;
    return GT_OK;
}

static GT_STATUS internal_macAutonegSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 anEnable,
    IN  GT_U32                  capabilities,
    IN  CPSS_MACDRV_STAGE_ENT    stage
)
{
    GT_U32         mpdIndex;
    GT_U32         portMacNum;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_STATUS      rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
       return GT_OK;
    }

    memset(&opParams, 0, sizeof(opParams));

    if (stage == CPSS_MACDRV_STAGE_PRE_E) {

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        mpdIndex = PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum);

        opParams.phyAutoneg.enable = (anEnable == GT_TRUE) ? MPD_AUTO_NEGOTIATION_ENABLE_E:MPD_AUTO_NEGOTIATION_DISABLE_E;
        opParams.phyAutoneg.capabilities = capabilities;
        opParams.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;
        MPD_DBG(("Autoneg set [%d] : mpdIndex:%d capabilities: %d anEnable: %d \n", portNum, mpdIndex, capabilities, anEnable));
        ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_AUTONEG_E, &opParams);
        if ( MPD_OK_E != ret )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        return GT_OK;
    }

    /* post stage - do nothing */
    if (stage == CPSS_MACDRV_STAGE_POST_E) {
        return GT_OK;
    }
    return GT_OK;
}

static void * prvPhyMpdOsMalloc
(
     UINT_32  size
)
{
    return cpssOsMalloc(size);
}
static void prvPhyMpdOsFree
(
     void* data
)
{
    cpssOsFree(data);
}

static BOOLEAN prvPhyMpdOsSleep
(
     IN UINT_32 delay
)
{
    cpssOsTimerWkAfter(delay);
    return FALSE;
}

static BOOLEAN prvPhyMpdMdioRead (
    IN  UINT_32      mpdIndex,
    IN  UINT_8       mdioAddress,
    IN  UINT_16      deviceOrPage,
    IN  UINT_16      address,
    OUT UINT_16    * valuePtr
)
{
    UINT_8          dev, port;
    UINT_8          smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntryPtr;
    GT_STATUS       status = GT_OK;

    (void) deviceOrPage;
    portEntryPtr = prvMpdGetPortEntry(mpdIndex);
    if (portEntryPtr == NULL) {
        return FALSE;
    }

    if ((UINT_32)portEntryPtr->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = portEntryPtr->initData_PTR->port;
    dev = portEntryPtr->initData_PTR->mdioInfo.mdioDev;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    smiInterface = portEntryPtr->initData_PTR->mdioInfo.mdioBus;
    status = cpssSmiRegisterReadShort( dev,
                                       portGroupsBmp,
                                       smiInterface,
                                       mdioAddress,
                                       address,
                                       valuePtr);

    if (status != GT_OK)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOLEAN prvPhyMpdMdioWrite (
    IN UINT_32      mpdIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
)
{
    UINT_8          dev, port;
    UINT_8          smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntryPtr;
    GT_STATUS       status = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    (void) deviceOrPage;

    if (cpssSystemRecoveryStateGet(&tempSystemRecovery_Info) != GT_OK )
    {
        return FALSE;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) )
    {
        return TRUE;
    }

    portEntryPtr = prvMpdGetPortEntry(mpdIndex);
    if (portEntryPtr == NULL) {
        return FALSE;
    }

    if ((UINT_32)portEntryPtr->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = portEntryPtr->initData_PTR->port;
    dev = portEntryPtr->initData_PTR->mdioInfo.mdioDev;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    smiInterface = portEntryPtr->initData_PTR->mdioInfo.mdioBus;
    status = cpssSmiRegisterWriteShort( dev,
                                        portGroupsBmp,
                                        smiInterface,
                                        mdioAddress,
                                        address,
                                        value);
    if (status != GT_OK){
        return FALSE;
    }

    return TRUE;
}

static MPD_RESULT_ENT prvPhyMpdLog(
    const char              * logTextPtr
)
{
    cpssOsPrintf(logTextPtr);
    return MPD_OK_E;
}

static BOOLEAN prvPhyMpdDebugBind (
    IN const char   * compPtr,
    IN const char   * pkgPtr,
    IN const char   * flagPtr,
    IN const char   * helpPtr,
    OUT UINT_32     * flagIdPtr
)
{
    UINT_32 len = 0,index;

    if (MPD_DEBUG_FREE_INDEX >= MPD_NUM_OF_DEBUG_FLAGS_CNS) {
            /* no more debug flags */
        return FALSE;
    }

    index = MPD_DEBUG_FREE_INDEX++;
    mpdDebugFlags[index].isFree = FALSE;
    mpdDebugFlags[index].flagStatus = FALSE;

    len += (compPtr != NULL)?cpssOsStrlen(compPtr):0;
    len += (pkgPtr != NULL)?cpssOsStrlen(pkgPtr):0;
    len += (flagPtr != NULL)?cpssOsStrlen(flagPtr):0;

    if (len)
    {
        len += 6;
        mpdDebugFlags[index].flagName_PTR = prvPhyMpdOsMalloc(len+1/*room for '\0'*/);
        mpdDebugFlags[index].flagName_PTR[0] = '\0';
        cpssOsStrCat(mpdDebugFlags[index].flagName_PTR,compPtr);
        cpssOsStrCat(mpdDebugFlags[index].flagName_PTR,"-->");
        cpssOsStrCat(mpdDebugFlags[index].flagName_PTR,pkgPtr);
        cpssOsStrCat(mpdDebugFlags[index].flagName_PTR,"-->");
        cpssOsStrCat(mpdDebugFlags[index].flagName_PTR,flagPtr);

    }

    len = (helpPtr != NULL)?cpssOsStrlen(helpPtr):0;
    if (len)
    {
        mpdDebugFlags[index].flagHelp_PTR = prvPhyMpdOsMalloc(len);
        mpdDebugFlags[index].flagHelp_PTR[0] = '\0';
        cpssOsStrNCpy(mpdDebugFlags[index].flagHelp_PTR,helpPtr,len);
    }

    *flagIdPtr = index;
    return TRUE;
}

/**
 * @brief   example implementation of debug check is Flag active
 *
 * @return Active / InActive
 */
static BOOLEAN prvPhyMpdDebugIsActive
(
    IN UINT_32  flagId
)
{
    if (flagId >= MPD_NUM_OF_DEBUG_FLAGS_CNS)
    {
        return FALSE;
    }
    if (mpdDebugFlags[flagId].isFree == TRUE)
    {
        return FALSE;
    }
    return mpdDebugFlags[flagId].flagStatus;
}

void prvPhyMpdDebugLog
(
    IN const char    * funcNamePtr,
    IN const char    * formatPtr,
    IN ...
)
{
    va_list      argptr;
    cpssOsPrintf("%s:",funcNamePtr);
    va_start(argptr, formatPtr);
    vprintf( formatPtr, argptr);
    va_end(argptr);
    printf("\n");
}

MPD_RESULT_ENT prvPhyMpdHandleFailure
(
    IN UINT_32                  rel_ifIndex,
    IN MPD_ERROR_SEVERITY_ENT   severity,
    IN UINT_32                  line,
    IN const char               * callingFuncPtr,
    IN const char               * errorTextPtr
)
{
    char * severityText[MPD_ERROR_SEVERITY_WARNING_E+1] = {"Fatal", "Error", "Minor", "Warning"};
    cpssOsPrintf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
                (severity<=MPD_ERROR_SEVERITY_WARNING_E)?severityText[severity]:"Unknown",
                 rel_ifIndex,
                 line,
                 callingFuncPtr,
                 errorTextPtr);

    if (severity == MPD_ERROR_SEVERITY_FATAL_E)
    {
        cpssOsPrintf("Fatal\n");
    }

    return MPD_OK_E;
}

/**
* @internal ironman_phyInit function
* @endinternal
*
* @brief   PHY related configurations
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChIronman_phyInit
(
    IN  GT_U8  devNum
)
{
    GT_STATUS   rc;
    GT_U32      smiAddr;
    GT_U16      data;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if( rc != GT_OK)
    {
        return rc;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) )
    {
        return GT_OK;
    }

    /* LED configurations */
    for(smiAddr = 8; smiAddr < 16; smiAddr++)
    {
        /* Page 28 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 28);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* read register 2 from page 28 */
        rc = cpssSmiRegisterReadShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 2, &data);
        if( rc != GT_OK)
        {
            return rc;
        }
        data &= ~(1 << 7);
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 2, data);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* read register 0 from page 28 */
        rc = cpssSmiRegisterReadShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 0, &data);
        if( rc != GT_OK)
        {
            return rc;
        }

        data |= (1 << 4);
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 0, data);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* read register 1 from page 28 */
        rc = cpssSmiRegisterReadShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 1, &data);
        if( rc != GT_OK)
        {
            return rc;
        }
        data |= (1 << 10);
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 1, data);
        if( rc != GT_OK)
        {
            return rc;
        }

         /* Page 3 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 3);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* LED Link/Activity */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 16, 0x1181);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* Change to Page 0 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCfgPortPhyObjReload function
* @endinternal
*
* @brief  Init/Reload the MAC DRV functions.
* @note   Called once for Static library linking.
*         Called multiple times during Shared lib usage.
*
* @param[in,out] macPhyObjPtr       - Pointer to MAC DRV object.
*/
GT_VOID prvCpssDxChCfgPortPhyObjReload (CPSS_MACDRV_OBJ_STC* macPhyObjPtr)
{
    /* object function init */
    macPhyObjPtr->macDrvMacSpeedSetFunc         = internal_macPhySpeedSet;
    macPhyObjPtr->macDrvMacSpeedGetFunc         = internal_macPhySpeedGet;
    macPhyObjPtr->macDrvMacFlowCntlANSetFunc    = internal_macPhyFlowCntlANSet;
    macPhyObjPtr->macDrvMacDuplexSetFunc        = internal_macPhyDuplexSet;
    macPhyObjPtr->macDrvMacDuplexGetFunc        = internal_macPhyDuplexGet;
    macPhyObjPtr->macDrvMacPortPowerDownSetFunc = internal_macPhyPortPowerDownSet;
    macPhyObjPtr->macDrvMacPortEnableSetFunc    = internal_macPhyPortEnableSet;
    macPhyObjPtr->macDrvMacPortEnableGetFunc    = internal_macPhyPortEnableGet;
    macPhyObjPtr->macDrvMacPortlinkGetFunc      = internal_macPhyPortLinkGet;
    macPhyObjPtr->macDrvMacPortLoopbackGetFunc  = internal_portLoopbackGet;
    macPhyObjPtr->macDrvMacAutonegSetFunc       = internal_macAutonegSet;
}

/**
* @internal prvCpssDxChPhyMpdSharedDbInit function
* @endinternal
*
* @brief  Init the MPD Global Shared Memory
* @note   This function should be called before prvCpssDxChPhyMpdInit
*
* @param[in,out] mpdGlobalSharedDbPtr   - Pointer to MPD Global Shared Data
*/
GT_VOID prvCpssDxChPhyMpdSharedDbInit
(
    INOUT GT_VOID       ** mpdGlobalSharedDbPtr
)
{
    MPD_RESULT_ENT          ret = MPD_OK_E;

    ret = prvMpdDriverInitSharedDb (prvPhyMpdOsMalloc, mpdGlobalSharedDbPtr);
    if (ret != MPD_OK_E) {
        CPSS_LOG_ERROR_MAC ("Failed in prvMpdDriverInitSharedDb");
    }
}

/**
* @internal prvCpssDxChPhyMpdNonSharedDbInit function
* @endinternal
*
* @brief  Init the MPD Global Non-Shared Variable Structure
* @note   This function should be called before prvCpssDxChPhyMpdInit
*
* @param[in,out] mpdGlobalNonSharedDbPtr    - Pointer to MPD Global Non-Shared Data
*/
GT_VOID prvCpssDxChPhyMpdNonSharedDbInit
(
    INOUT GT_VOID       ** mpdGlobalNonSharedDbPtr
)
{
    MPD_RESULT_ENT          ret = MPD_OK_E;

    ret = prvMpdDriverInitNonSharedDb (mpdGlobalNonSharedDbPtr);
    if (ret != MPD_OK_E) {
        CPSS_LOG_ERROR_MAC ("Failed in prvMpdDriverInitNonSharedDb");
    }
}

/**
* @internal prvCpssDxChPhyMpdInit function
* @endinternal
*
* @brief   Init the phy driver
*
* @param[in] devNum                - device number
* @param[in] phyInfo               - array of phy info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhyMpdInit
(
    IN GT_U8                                   devNum,
    IN const PRV_CPSS_DXCH_PHY_STATIC_INFO_STC *phyInfo
)
{
    GT_U32               mpdIndex = 0, i;
    GT_STATUS            rc = GT_OK;
    MPD_RESULT_ENT       ret = MPD_OK_E;
    MPD_CALLBACKS_STC    phyCallbacks;
    MPD_PORT_INIT_DB_STC phyEntry;
    CPSS_MACDRV_OBJ_STC* macPhyObj88e1781Ptr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecoveryInfo;
    MPD_OPERATIONS_PARAMS_UNT     phy_params;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&phyCallbacks,0,sizeof(phyCallbacks));

    phyCallbacks.sleep_PTR               = prvPhyMpdOsSleep;
    phyCallbacks.alloc_PTR               = prvPhyMpdOsMalloc;
    phyCallbacks.free_PTR                = prvPhyMpdOsFree;
    phyCallbacks.debug_bind_PTR          = prvPhyMpdDebugBind;
    phyCallbacks.is_active_PTR           = prvPhyMpdDebugIsActive;
    phyCallbacks.debug_log_PTR           = prvPhyMpdDebugLog;
    phyCallbacks.txEnable_PTR            = NULL;
    phyCallbacks.handle_failure_PTR      = prvPhyMpdHandleFailure;
    phyCallbacks.mdioRead_PTR            = prvPhyMpdMdioRead;
    phyCallbacks.mdioWrite_PTR           = prvPhyMpdMdioWrite;
    phyCallbacks.logging_PTR             = prvPhyMpdLog;

    ret = mpdDriverInitDb(&phyCallbacks);
    if (ret != MPD_OK_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdDriverInitDb");
    }

    cpssOsPrintf(" PHY init from CPSS \n");

    /* MAC DRV object for 88E1781 PHY */
    macPhyObj88e1781Ptr = cpssOsMalloc(sizeof(CPSS_MACDRV_OBJ_STC));
    if (macPhyObj88e1781Ptr == NULL) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "failed to allocate memory");
    }

    /* NULL set for allocated memory */
    cpssOsMemSet (macPhyObj88e1781Ptr,0,sizeof(CPSS_MACDRV_OBJ_STC));

    cpssOsMemSet(&phyEntry,0,sizeof(phyEntry));

    for(i = 0; phyInfo[i].portNum != 0xff ; i++)
    {
        phyEntry.port                        = phyInfo[i].portNum;
        phyEntry.phyNumber                   = phyInfo[i].phyNumber;
        phyEntry.phyType                     = phyInfo[i].phyType;
        phyEntry.transceiverType             = phyInfo[i].transceiverType;
        phyEntry.mdioInfo.mdioAddress        = phyInfo[i].phyAddr;
        phyEntry.mdioInfo.mdioDev            = devNum;
        phyEntry.mdioInfo.mdioBus            = phyInfo[i].smiInterface;
        phyEntry.disableOnInit               = GT_FALSE;
        phyEntry.usxInfo.usxType             = phyInfo[i].usxType;

        mpdIndex = prvCpssPhyMpdIndexGet(devNum, phyInfo[i].portNum);

        if (mpdPortDbUpdate(mpdIndex, &phyEntry) != MPD_OK_E) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdPortDbUpdate");
        }

        /* Only 88E1781 Integrated PHY is supported.
         * If more PHY types are to be supported, need to add them in:
         * CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT - for Shared Lib support */

        if (phyInfo[i].phyType == MPD_TYPE_88E1781_E) {

            macPhyObj88e1781Ptr->phyMacType = CPSS_DXCH_PHY_MAC_TYPE_88E1781_E;

            prvCpssDxChCfgPortPhyObjReload (macPhyObj88e1781Ptr);

            /* bind ports to phy object  */
            rc = cpssDxChCfgBindPortPhymacObject(devNum, phyInfo[i].portNum , macPhyObj88e1781Ptr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(ret, "failed in cpssDxChCfgBindPortPhymacObject");
            }
        }
    }

    if((tempSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
       (tempSystemRecoveryInfo.systemRecoveryState   != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
        /* Driver init without writing to HW */
        if (mpdSharedLibReducedInit() != MPD_OK_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdDriverInitHw");
        }
    }
    else
    {
        /* Driver init with writing to HW */
        if (mpdDriverInitHw() != MPD_OK_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdDriverInitHw");
        }
        for(i = 0; phyInfo[i].portNum != 0xff ; i++)
        {
            if (phyInfo[i].phyType == MPD_TYPE_88E1781_E)
            {
                cpssOsMemSet(&phy_params, 0, sizeof(phy_params));
                mpdIndex = prvCpssPhyMpdIndexGet(devNum, phyInfo[i].portNum);

                phy_params.phyTune.tuneParams.txTune.comphy.preTap      = 0;
                phy_params.phyTune.tuneParams.txTune.comphy.mainTap     = 31;
                phy_params.phyTune.tuneParams.txTune.comphy.postTap     = 0;
                phy_params.phyTune.txConfig                             = MPD_SERDES_CONFIG_PERMANENT_E;
                phy_params.phyTune.hostOrLineSide                       = MPD_PHY_SIDE_HOST_E;
                phy_params.phyTune.tuneParams.paramsType                = MPD_SERDES_PARAMS_TX_E;
                ret = mpdPerformPhyOperation(mpdIndex, MPD_OP_CODE_SET_SERDES_TUNE_E, &phy_params);
                if (ret != MPD_OK_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPhySpeedSet - error at mpdPerformPhyOperation");
                }
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPhyMpdReducedInit function
* @endinternal
*
* @brief   Init the phy driver for Secondary Applications in
*          shared library mode.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhyMpdReducedInit
(
    IN GT_U8                                   devNum
)
{
    MPD_RESULT_ENT       ret = MPD_OK_E;
    MPD_CALLBACKS_STC    phyCallbacks;

    GT_UNUSED_PARAM (devNum);

    /* Bind Shared DB */
    ret = prvMpdDriverInitSharedDb (prvPhyMpdOsMalloc, &(PRV_MPD_SHARED_DB));
    if (ret != MPD_OK_E) {
        CPSS_LOG_ERROR_MAC ("Failed in Non-Primary application prvMpdDriverInitSharedDb");
    }

    cpssOsMemSet(&phyCallbacks,0,sizeof(phyCallbacks));

    phyCallbacks.sleep_PTR               = prvPhyMpdOsSleep;
    phyCallbacks.alloc_PTR               = prvPhyMpdOsMalloc;
    phyCallbacks.free_PTR                = prvPhyMpdOsFree;
    phyCallbacks.debug_bind_PTR          = prvPhyMpdDebugBind;
    phyCallbacks.is_active_PTR           = prvPhyMpdDebugIsActive;
    phyCallbacks.debug_log_PTR           = prvPhyMpdDebugLog;
    phyCallbacks.txEnable_PTR            = NULL;
    phyCallbacks.handle_failure_PTR      = prvPhyMpdHandleFailure;
    phyCallbacks.mdioRead_PTR            = prvPhyMpdMdioRead;
    phyCallbacks.mdioWrite_PTR           = prvPhyMpdMdioWrite;
    phyCallbacks.logging_PTR             = prvPhyMpdLog;

    ret = mpdDriverInitDb(&phyCallbacks);
    if (ret != MPD_OK_E) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdDriverInitDb");
    }

    cpssOsPrintf(" MPD Reduced init from CPSS \n");

    if (mpdSharedLibReducedInit() != MPD_OK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "failed in mpdSharedLibInitPhyDriver");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPhyMpdDbClear function
* @endinternal
*
* @brief   clear phy related DB
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPhyMpdDbClear
(
    IN GT_U8         devNum
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_IRONMAN_M_DEVICES_CASES_MAC:
            nextFreeGlobalIfIndex = 1;
            /* check that MPD was initialized */
            if (prvMpdSharedGlobalDb_PTR)
            {
                mpdDriverDestroy();
            }
            break;
        default:
            break;
    }
    return GT_OK;
}


