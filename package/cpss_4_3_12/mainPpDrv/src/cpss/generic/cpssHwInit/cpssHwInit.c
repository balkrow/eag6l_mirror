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
* @file cpssHwInit.c
*
* @brief Includes CPSS level basic HW initialization functions.
*
* @version   43
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChCpssHwInitLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdmaNetIfFunc.h>

/**
* @internal prvCpssDxChNetIfMultiNetIfNumberGet function
* @endinternal
*
* @brief   private (internal) function to :
*         build the information about all the network interfaces according to phase1 and phase2 params
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - The device number.
*
* @param[out] numOfNetIfsPtr           - (pointer to) the number of network intrfaces according to port mapping
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_APPLICABLE_DEVICE - on non-applicable device
*
*/
GT_STATUS prvCpssDxChNetIfMultiNetIfNumberGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *numOfNetIfsPtr
)
{
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_STATUS rc;

    if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "GDMA not support this function as TO_CPU,FROM_CPU each supports diff number of queues");
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /*BC3/Aldrin2/Sip6*/
    {
        rc = prvCpssDxChPortMappingCPUPortGet(devNum, /*OUT*/cpuPortNumArr, numOfNetIfsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* the mapping is enough . no special address space needed */
            return GT_OK;
        }

        if(*numOfNetIfsPtr > 1)
        {
            /* check that MG1-3 are mapped for management access */
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].resource.mg1.start == 0)
            {
                *numOfNetIfsPtr = 1;
            }
            else if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].resource.mg2.start == 0)
            {
                *numOfNetIfsPtr = 2;
            }
            else if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].resource.mg3.start == 0)
            {
                *numOfNetIfsPtr = 3;
            }
       }
    }
    else
    {
        *numOfNetIfsPtr =1;
    }
    return GT_OK;

}

/**
* @internal prvCpssDxChNetIfNumQueuesGet function
* @endinternal
*
* @brief  The function return the number of queue for FROM_CPU,TO_CPU
*           for NON-GDMA devices the number of queues for FROM_CPU,TO_CPU are the same.
*           for     GDMA devices the number of queues for FROM_CPU,TO_CPU could differ from each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @param[out] toCpu_numOfQueuesPtr    - (pointer to) the number of queues that bound for TO_CPU
*                                       ignored if NULL
* @param[out] fromCpu_numOfQueuesPtr  - (pointer to) the number of queues that bound for FROM_CPU
*                                       ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_APPLICABLE_DEVICE - on non-applicable device
*
*/
GT_STATUS prvCpssDxChNetIfNumQueuesGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *toCpu_numOfQueuesPtr,
    OUT GT_U32                      *fromCpu_numOfQueuesPtr
)
{
    GT_STATUS   rc;
    GT_U32  numOfNetIfs;
    if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        rc = prvCpssDxChGdmaNetIfNumQueuesGet(devNum,toCpu_numOfQueuesPtr,fromCpu_numOfQueuesPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfNetIfs);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(toCpu_numOfQueuesPtr)
        {
            *toCpu_numOfQueuesPtr = CPSS_MAX_RX_QUEUE_CNS/*8*/ * numOfNetIfs;
        }
        if(fromCpu_numOfQueuesPtr)
        {
            *fromCpu_numOfQueuesPtr = CPSS_MAX_TX_QUEUE_CNS/*8*/ * numOfNetIfs;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigPortToPhymacObjectBind function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - error
*/
GT_STATUS prvCpssPpConfigPortToPhymacObjectBind
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr

)
{
    GT_U32 portMacNum; /* MAC number */

    /* validate devNum and portNum */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*NOTE : NULL pointer allow to 'clean' the DB of the port.*/

    /* Get PHY MAC object ptr */

    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portMacObjPtr = macDrvObjPtr;

    return GT_OK;
}


