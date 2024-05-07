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
* @file flexeAgentRegAddr.c
*
* @brief FLEXE AGENT register read/write implementation
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_IN_CPSS
#include <hw.h>
#include <FreeRTOS_CLI.h>
#include <global.h>
#include <srvCpuServices.h>
#else

#endif

#include <flexeAgentTypes.h>
#include <flexeAgentRegAddr.h>
#include <flexeAgentTransit.h>

#define FLEXE_CALC_MASK_MAC(fieldLen, fieldOffset, mask)     \
            if(((fieldLen) + (fieldOffset)) >= 32)     \
            { \
                (mask) = (GT_U32)(0 - (1<< (fieldOffset)));    \
            }\
            else \
            { \
                (mask) = (((1<<((fieldLen) + (fieldOffset)))) - (1 << (fieldOffset))); \
            }

GT_VOID flexeAgentBaseAddressGet
(
    IN  GT_U8       tileId,
    OUT GT_U32      *baseAddress
)
{
    *baseAddress = FLEXE_BASE_ADDRESS(tileId);
    return;
}

#ifndef FLEXE_AGENT_IN_CPSS
/**
* @internal flexeAgentHwAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         some registers cannot be accessed by more than one
*         client concurrently Concurrent access might result in
*         invalid data read/write.
*         Multi-Processor Environment This case is protected by
*         HW Semaphore HW Semaphore is defined based in MSYS /
*         CM3 resources In case customer does not use MSYS / CM3
*         resources, the customer will need to implement its own
*         HW Semaphore This protection is relevant ONLY in case
*         Service CPU Firmware is loaded to CM3
*
*
*/
GT_VOID flexeAgentHwAccessLock
(
    IN GT_U8 hwSem
)
{
    MV_U32 tmp;

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
    if (hwSem < FLEXE_SEMA_LAST ) {
        /* When running on any Service CPU, HW semaphore always used */
        do
        {
            MV_MEMIO32_WRITE(MG_BASE | (0x800 + hwSem *4), CPUID);
            tmp = MV_MEMIO32_READ(MG_BASE | (0x800 + hwSem * 4));
        }while((tmp & 0xFF) != CPUID);
    }
}

GT_VOID flexeAgentHwAccessUnLock
(
    IN GT_U8 hwSem
)
{
    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
    if (hwSem < FLEXE_SEMA_LAST ) {
        /* When running on any Service CPU, HW semaphore always used */
        MV_MEMIO32_WRITE(MG_BASE | (0x800 + hwSem *4), 0xFF);
    }
}
#endif

GT_STATUS flexeAgentRegisterFieldSet
(
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
)
{
    GT_U32 mask;
    GT_U32 regValue;

    FLEXE_CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    regValue = ((fieldData << fieldOffset) & mask);

    return flexeAgentHwRegWrite(regAddr, regValue, mask);
}

GT_STATUS flexeAgentRegisterFieldGet
(
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
)
{
    GT_STATUS rc;
    GT_U32 mask;
    GT_U32 regValue;

    rc = flexeAgentHwRegRead(regAddr, &regValue, FLEXE_REG_MASK_NONE);
    if(rc != GT_OK)
    {
        return rc;
    }

    FLEXE_CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldDataPtr = (GT_U32)((regValue & mask) >> fieldOffset);
    return rc;
}
