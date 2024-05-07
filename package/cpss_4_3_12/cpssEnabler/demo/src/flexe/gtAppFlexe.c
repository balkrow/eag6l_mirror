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
* @file gtAppFlexe.c
*
* @brief Application Code for Flexe configuration
*
* @version   1
*********************************************************************************
**/

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <cpss/generic/flexe/cpssFlexe.h>
#include <flexe/prvAppFlexe.h>

/* Default configurations
   1. Create two Groups of 400G bandwidth
      (i)   Group0: 4x25G + 2x50G + 2x100G
      (ii)  Group1: 4x25G + 2x50G + 2x100G
   2. Add Clients -> 
*/

#define APP_FLEXE_GROUP_NUM(groupId)                    (0x200 + 0x10*groupId)
#define APP_FLEXE_CLIENT_NUM(groupId,clientId,speedId)  (100*groupId + 10*speedId + clientId)
#define APP_FLEXE_INSTANCE_NUM(groupId,instanceId)      (0x10*groupId + 0x2*instanceId)

#define APP_FLEXE_CLIENT_SPEED_25G  25
#define APP_FLEXE_CLIENT_SPEED_50G  50
#define APP_FLEXE_CLIENT_SPEED_100G 100

#define APP_FLEXE_NUM_CLIENTS_PER_GROUP 8

GT_STATUS appDemoDefaultConfigInit
(
    IN  GT_U8   devNum
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      groupNum = 0;
    CPSS_FLEXE_GROUP_INFO_STC   groupInfo;
    GT_U32                      clientNum = 0;
    CPSS_FLEXE_CLIENT_INFO_STC  clientInfo;
    CPSS_FLEXE_GROUP_CALENDAR_SWITCH_PARAMS_STC calendarSwitchInfo;
    GT_U32                      groupId, clientId, speedId=0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = 0x1;

    rc = cpssFlexeHwInit(devNum, portGroupsBmp);
    APP_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "cpssFlexeHwInit failed");

    for (groupId=0; groupId<2; groupId++)
    {
        /* Create Group of 400G bandwidth */
        cpssOsMemSet(&groupInfo, 0, sizeof(groupInfo));
        groupNum                 = APP_FLEXE_GROUP_NUM(groupId);
        groupInfo.bandwidth      = 400;
        groupInfo.numInstances   = 4;
        groupInfo.numPhys        = 4;
        groupInfo.instanceArr[0] = APP_FLEXE_INSTANCE_NUM(groupId,0);
        groupInfo.instanceArr[1] = APP_FLEXE_INSTANCE_NUM(groupId,1);
        groupInfo.instanceArr[2] = APP_FLEXE_INSTANCE_NUM(groupId,2);
        groupInfo.instanceArr[3] = APP_FLEXE_INSTANCE_NUM(groupId,3);
        rc = cpssFlexeGroupCreate(devNum, groupNum, &groupInfo);
        APP_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "cpssFlexeGroupCreate failed: GroupId: %d", groupId);

        /* Fill client Info */
        cpssOsMemSet(&calendarSwitchInfo, 0, sizeof(calendarSwitchInfo));
        calendarSwitchInfo.numClients = APP_FLEXE_NUM_CLIENTS_PER_GROUP;

        for (clientId=0; clientId<APP_FLEXE_NUM_CLIENTS_PER_GROUP; clientId++)
        {
            /* Add Client */
            cpssOsMemSet(&clientInfo, 0, sizeof(clientInfo));
            switch (clientId)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                clientInfo.clientSpeed  = APP_FLEXE_CLIENT_SPEED_25G;
                speedId = 0;
                break;
            case 4:
            case 5:
                clientInfo.clientSpeed = APP_FLEXE_CLIENT_SPEED_50G;
                speedId = 1;
                break;
            case 6:
            case 7:
                clientInfo.clientSpeed = APP_FLEXE_CLIENT_SPEED_100G;
                speedId = 2;
                break;
            default:
                break;
            }
            /* Add Clients */
            clientNum = APP_FLEXE_CLIENT_NUM(groupId,clientId,speedId);
            clientInfo.groupNum    = APP_FLEXE_GROUP_NUM(groupId);
            clientInfo.clientType  = CPSS_FLEXE_CLIENT_TYPE_L2_E;
            rc = cpssFlexeClientAdd(devNum, clientNum, &clientInfo);
            APP_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "cpssFlexeClientAdd failed: clientId:%d", clientId);

            calendarSwitchInfo.clientInfoArr[clientId].clientNum = clientNum;
        }

        /* Perform calendar switch */
        rc = cpssFlexeGroupCalendarSwitch(devNum, groupNum, &calendarSwitchInfo);
        APP_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "cpssFlexeGroupCalendarSwitch failed: groupId:%d", groupId);
    }

    return rc;
}
