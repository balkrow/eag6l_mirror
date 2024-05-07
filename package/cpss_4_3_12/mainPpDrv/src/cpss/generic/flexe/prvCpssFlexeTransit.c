/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
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
* @file prvCpssFlexeTransit.c
*
* @brief FLEXE Primay transit API implementation
*
* @version   1
********************************************************************************
*/

/**
 * @internal prvCpssFlexeIpcMessageSendData function
 * @endinternal
 *
 * @brief send ipc mesage data to service cpu
 *
 * @param[in] devNum         - device number
 * @param[in] ipcMsgDataPtr  - (pointer to) IPC message data
 *
 * @retVal    GT_OK     - on success
 */
GT_STATUS prvCpssFlexeIpcMessageSendData
(
    IN GT_U8    devNum,
    IN GT_U8    *ipcMsgDataPtr,
    IN GT_U8    ipcMsgSize
)
{
    GT_STATUS   rc = GT_OK;
#ifndef FLEXE_AGENT_IN_CPSS
    rc = prvCpssGenericSrvCpuIpcMessageSendCpuId(devNum, 0/*scpuId*/,1/*channel*/,ipcMsgDataPtr, ipcMsgSize);
#else
    PRV_CPSS_FLEXE_DB_STC   *flexeDbPtr;

    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(flexeDbPtr);
    cpssOsMemCpy(flexeDbPtr->transitData.ipcP2A[flexeDbPtr->transitData.firstFreeP2A].data,
                 ipcMsgDataPtr, sizeof(PRV_CPSS_FLEXE_IPC_MSG_MAX_SIZE_CNS));
    flexeDbPtr->transitData.ipcP2A[flexeDbPtr->transitData.firstFreeP2A].sizeInBytes = ipcMsgSize;
    flexeDbPtr->transitData.firstFreeP2A ++;
    if (flexeDbPtr->transitData.firstFreeP2A >= FLEXE_AGENT_IPC_MSG_MAX_NUM)
    {
        flexeDbPtr->transitData.firstFreeP2A = 0;
    }
    /* [TBD] Handle buffer full event */
    rc = GT_OK;
#endif
    return rc;
}

GT_STATUS   prvCpssFlexeIpcFwInit
(
    IN  GT_U8       devNum,
    IN  GT_U8       sCpuId
)
{
    GT_STATUS       rc = GT_OK;

#if 0
    GT_UINTPTR      fwChannel = NULL;
    PRV_CPSS_FLEXE_DB_STC   *flexeDbPtr;
    PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 1,    64, 56, 16, 28 },
            { 0xff,  0,  0,  0,  0 }
    };

    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(flexeDbPtr);

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum) == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssGenericSrvCpuInitCust(devNum, 0/*scpuId*/, ipcConfig, preStartCb, preStartCookie, fwChannelPtr);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeFreeIdGet failed");

    /* store fwChannel */
    if (fwChannelPtr != NULL)
    {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[0] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
    }
#endif

#ifdef FLEXE_AGENT_IN_CPSS
    PRV_CPSS_FLEXE_DB_STC   *flexeDbPtr;

    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(flexeDbPtr);
    cpssOsMemSet(flexeDbPtr->transitData, 0, sizeof(flexeDbPtr->transitData));

#endif

    (void) devNum;
    (void) sCpuId;

    return rc;
}

