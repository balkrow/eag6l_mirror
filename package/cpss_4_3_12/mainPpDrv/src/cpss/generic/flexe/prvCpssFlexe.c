/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
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
* @file prvCpssFlexe.c
*
* @brief Private CPSS FlexE API implementations
*
* @version   1
********************************************************************************
*/

#include <cpss/generic/flexe/prvCpssFlexe.h>
#include <cpss/generic/flexe/prvCpssFlexeIpc.h>
#include <cpss/generic/flexe/prvCpssFlexePhy.h>

GT_STATUS prvCpssFlexeAvailableResourceGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC   *groupInfoPtr,
    OUT GT_U8                       *tileIdPtr,
    OUT GT_U8                       *instanceBmpPtr,
    OUT GT_U8                       *bondMaskArrPtr,
    OUT GT_U8                       *interleaveCfgPtr
)
{
    GT_STATUS   rc = GT_OK;

    (void) devNum;
    (void) groupInfoPtr;
    (void) tileIdPtr;
    (void) instanceBmpPtr;
    (void) bondMaskArrPtr;
    (void) interleaveCfgPtr;

    return rc;
}

GT_STATUS prvCpssFlexeFreeIdGet
(
    GT_U32          numElements,
    GT_U32          *num2IdArrPtr,
    GT_U32          *freeIdPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i  = 0;
    for (i=0; i<numElements; i++)
    {
        if(num2IdArrPtr[i] == PRV_CPSS_FLEXE_ID_MAX_CNS)
        {
            *freeIdPtr = i;
            break;
        }
    }
    if (i == numElements)
    {
        rc = GT_NOT_FOUND;
    }
    return rc;
}

GT_STATUS prvCpssFlexeGroupDbUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC   *groupInfoPtr,
    IN  GT_U8                       *tileIdPtr,
    IN  GT_U8                       *shimIdPtr,
    IN  GT_U8                       *instanceBmpPtr,
    IN  GT_U8                       *bondMaskArrPtr,
    IN  GT_U8                       *interleaveCfgPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_FLEXE_DB_STC   *flexeDbPtr;
    GT_U32    freeId = 0;
    PRV_CPSS_FLEXE_DB_GROUP_TABLE_ENTRY_STC *groupTableEntryPtr;

    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(flexeDbPtr);

    flexeDbPtr->groupTable.numActiveGroups++;
    rc = prvCpssFlexeFreeIdGet(flexeDbPtr->resources.maxGroups,
                               flexeDbPtr->groupTable.num2Id,
                               &freeId);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeFreeIdGet failed");
    flexeDbPtr->groupTable.num2Id[groupNum] = freeId;

    groupTableEntryPtr = &flexeDbPtr->groupTable.groupEntryArr[freeId] ;
    cpssOsMemCpy(&groupTableEntryPtr->groupInfo,
                 groupInfoPtr, sizeof(CPSS_FLEXE_GROUP_INFO_STC));

    groupTableEntryPtr->isValid     = GT_TRUE;
    groupTableEntryPtr->groupNum    = groupNum;
    groupTableEntryPtr->tileId      = *tileIdPtr;
    groupTableEntryPtr->shimId      = *shimIdPtr;
   /* groupTableEntryPtr->firstPhyId  = *firstPhyId; */

    (void) devNum;
    (void) groupNum;
    (void) groupInfoPtr;
    (void) tileIdPtr;
    (void) shimIdPtr;
    (void) instanceBmpPtr;
    (void) bondMaskArrPtr;    
    (void) interleaveCfgPtr;

    return rc;
}

/**
 * @internal prvCpssFlexeGroupCreate function
 * @endinternal
 *
 * @brief Create an FlexE group and add related PHYs
 *
 * @param[in] devNum    - device number
 * @param[in] groupId   - FlexE group identifier
 * @param[in] groupInfo - FlexE group information
 *
 */
GT_STATUS prvCpssFlexeGroupCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC      *groupInfoPtr
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT ipcMsg;
    GT_U8                           phyId=0;
    CPSS_FLEXE_PHY_CONFIG_STC       phyCfg;

    cpssOsMemSet(&phyCfg, 0, sizeof(phyCfg));
    phyCfg.macIdx = 0;
    phyCfg.serdesLaneSpeed = (CPSS_FLEXE_SERDES_LANE_SPEED_ENT)(groupInfoPtr->bandwidth /
                                                                groupInfoPtr->numInstances);
    switch (groupInfoPtr->bandwidth)
    {
    case CPSS_FLEXE_GROUP_BANDWIDTH_50G_E: GT_ATTR_FALLTHROUGH;
    case CPSS_FLEXE_GROUP_BANDWIDTH_100G_E:
        phyCfg.numSerdesLanes = 1;
        break;
    case CPSS_FLEXE_GROUP_BANDWIDTH_150G_E: GT_ATTR_FALLTHROUGH;
    case CPSS_FLEXE_GROUP_BANDWIDTH_200G_E:
        phyCfg.numSerdesLanes = 2;
        break;
    case CPSS_FLEXE_GROUP_BANDWIDTH_250G_E: GT_ATTR_FALLTHROUGH;
    case CPSS_FLEXE_GROUP_BANDWIDTH_300G_E:
        phyCfg.numSerdesLanes = 3;
        break;
    case CPSS_FLEXE_GROUP_BANDWIDTH_350G_E: GT_ATTR_FALLTHROUGH;
    case CPSS_FLEXE_GROUP_BANDWIDTH_400G_E:
        phyCfg.numSerdesLanes = 4;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "Illegal bandwidth value");
        break;
    }
    phyCfg.autoNegEnabled = GT_FALSE;

    rc = prvCpssFlexePhyCreate(devNum, phyId, &phyCfg);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexePhyCreate failed");


    cpssOsMemSet(&ipcMsg, 0, sizeof(PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT));
    /* populate group create IPC message*/
    ipcMsg.groupCreate.opcode      = CPSS_FLEXE_IPC_MSG_OPCODE_GROUP_CREATE_E;
    rc = prvCpssFlexeAvailableResourceGet(devNum, groupInfoPtr,
                                          &ipcMsg.groupCreate.tileId,
                                          &ipcMsg.groupCreate.instanceBmp,
                                          ipcMsg.groupCreate.bondMaskArr,
                                          &ipcMsg.groupCreate.interleaveCfg);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeAvailableTileIdGet failed");

    rc = prvCpssFlexeIpcMessageSend(devNum, &ipcMsg);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeIpcMessageSend failed");

    rc = prvCpssFlexeGroupDbUpdate(devNum, groupNum, groupInfoPtr,
                                          &ipcMsg.groupCreate.tileId,
                                          &ipcMsg.groupCreate.shimId,
                                          &ipcMsg.groupCreate.instanceBmp,
                                          ipcMsg.groupCreate.bondMaskArr,
                                          &ipcMsg.groupCreate.interleaveCfg);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeIpcMessageSend failed");
    return GT_OK;
}
