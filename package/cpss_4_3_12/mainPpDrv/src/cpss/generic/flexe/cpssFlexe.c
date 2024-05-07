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
* @file cpssFlexe.c
*
* @brief CPSS FlexE API Implementation
*
* @version   1
********************************************************************************
*/

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/flexe/cpssFlexe.h>
#include <cpss/generic/flexe/prvCpssFlexe.h>
#include <cpss/generic/flexe/prvCpssFlexeIpc.h>

/**
 * @internal internal_cpssFlexeHwInit function
 * @endinternal
 *
 * @brief initialize an FlexE Hardware
 *
 * @param[in] devNum - device number
 */
GT_STATUS internal_cpssFlexeHwInit
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_FLEXE_DB_STC   *flexeDbPtr;
    PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT ipcMsg;

    /*Check Params */
    if (portGroupsBmp >= (1<<PRV_CPSS_FLEXE_TILES_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portGroupsBmp cannot be greater than %d\n",
                                      (1<<PRV_CPSS_FLEXE_TILES_MAX_CNS));
    }

    /* Allocate and Initialize FLEXE database */
    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    if (flexeDbPtr == NULL)
    {
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.genericFlexeDir.flexeDbPtr) = cpssOsMalloc(sizeof(PRV_CPSS_FLEXE_DB_STC));
    }
    PRV_CPSS_FLEXE_DB_PTR_GET(flexeDbPtr);
    PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(flexeDbPtr);

    cpssOsMemSet(flexeDbPtr, 0, sizeof(PRV_CPSS_FLEXE_DB_STC));

    /* Initialize DB */
    if ((portGroupsBmp & 0x3) != 0x3)
    {
        flexeDbPtr->resources.maxTiles = GT_TRUE;
    }
    flexeDbPtr->resources.usedTileBmp = (GT_U32)portGroupsBmp;

#if 0
    /* Initialize Firmware channel */
    rc = prvCpssFlexeIpcFwInit(devNum, 0/*sCpuNum*/);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeIpcFwInit failed");
#endif

    cpssOsMemSet(&ipcMsg, 0, sizeof(PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT));
    ipcMsg.hwInit.opcode        = CPSS_FLEXE_IPC_MSG_OPCODE_HW_INIT_E;
    ipcMsg.hwInit.portGroupsBmp = portGroupsBmp;
    rc = prvCpssFlexeIpcMessageSend(devNum, &ipcMsg);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssFlexeIpcMessageSend failed");

    return rc;
}

/**
 * @internal cpssFlexeHwInit function
 * @endinternal
 *
 * @brief initialize an FlexE core.
 *          1. prvCpssFlexeXcInit()
 *          2. prvCpssFlexeShimInit()
 *
 * @param[in] devNum - device numberl
 */
GT_STATUS cpssFlexeHwInit
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssFlexeHwInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_FLEXE_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp));

    rc = internal_cpssFlexeHwInit(devNum, portGroupsBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_FLEXE_CNS);
    return rc;
}

/**
 * @internal internal_cpssFlexeGroupCreate function
 * @endinternal
 *
 * @brief Create an FlexE group and add related PHYs
 *
 * @param[in] devNum    - device number
 * @param[in] groupNum  - FlexE group number
 * @param[in] groupInfoPtr - (pointer to) FlexE group information structure
 *
 * @retval GT_OK                    - success
 *         GT_NOT_APPLICALBE_DEVICE - not applicable device
 *         GT_OUT_OF_RANGE          - value out of range
 *         GT_BAD_PTR               - illegal pointer (ex: null)
 *         GT_FAIL                  - hardware error
 */
static GT_STATUS internal_cpssFlexeGroupCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC      *groupInfoPtr
)
{
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(groupInfoPtr);
    PRV_CPSS_GROUP_ID_CHECK_MAC(groupNum);
    PRV_CPSS_GROUP_INFO_CHECK_MAC(groupInfoPtr);

    rc = prvCpssFlexeGroupCreate(devNum, groupNum, groupInfoPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
 * @internal cpssFlexeGroupCreate function
 * @endinternal
 *
 * @brief Create an FlexE group and add related PHYs
 *
 * @param[in] devNum    - device number
 * @param[in] groupNum  - FlexE group number
 * @param[in] groupInfo - FlexE group information
 *
 * @retval GT_OK                    - success
 *         GT_NOT_APPLICALBE_DEVICE - not applicable device
 *         GT_OUT_OF_RANGE          - value out of range
 *         GT_BAD_PTR               - illegal pointer (ex: null)
 *         GT_FAIL                  - hardware error
 */
GT_STATUS cpssFlexeGroupCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC      *groupInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssFlexeGroupCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_FLEXE_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, groupNum, groupInfoPtr));

    rc = internal_cpssFlexeGroupCreate(devNum, groupNum, groupInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, groupNum, groupInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_FLEXE_CNS);
    return rc;
}

/**
 * @internal cpssFlexeClientAdd function
 * @endinternal
 *
 * @brief Add a client to active group
 *
 * @param[in] devNum       - device number
 * @param[in] groupNum     - Flexe Group number
 * @param[in] clientNum    - Client Number
 * @param[in] clientType   - client type L1/L2
 * @param[in] clientSpeed  - Client speed
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientAdd
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            clientNum,
    IN  CPSS_FLEXE_CLIENT_INFO_STC        *clientInfoPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* [TBD] Implementation */
    (void) devNum;
    (void) clientNum;
    (void) clientInfoPtr;

    return rc;
}

/**
 * @internal cpssFlexeGroupCalendarSwitch function
 * @endinternal
 *
 * @brief   Initialize a calendar switch on a given group
 *
 * @param[in] devNum      - device number
 * @param[in] groupNum    - FlexE group number
 * @param[in] paramsPtr   - calendar switch parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupCalendarSwitch
(
    IN  GT_U8                                      devNum,
    IN  GT_U32                                     groupNum,
    IN  CPSS_FLEXE_GROUP_CALENDAR_SWITCH_PARAMS_STC *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;

    /* [TBD] Implementation */
    (void) devNum;
    (void) groupNum;
    (void) paramsPtr;

    return rc;
}

