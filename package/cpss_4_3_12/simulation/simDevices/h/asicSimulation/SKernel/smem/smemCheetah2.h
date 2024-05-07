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
* @file smemCheetah2.h
*
* @brief Data definitions for cheetah2 memories.
*
* @version   12
********************************************************************************
*/
#ifndef __smemCht2h
#define __smemCht2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemCheetah.h>

/**
* @internal smemCht2TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemCht2TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/* active functions for Write */
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht2ActiveFuqBaseWrite);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemCht2h */



