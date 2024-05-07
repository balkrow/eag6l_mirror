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
* @file prvTgfPortArbitrationGroups.h
*
* @brief Port Profile MRU check - basic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortArbitrationGroupsh
#define __prvTgfPortArbitrationGroupsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
* @internal prvTgfPortArbitrationGroupConfigure function
* @endinternal
*
* @brief   Configure scheduling profile
*
* @param[in] confId -currently checked configuration index
*
*
*
*/

GT_VOID prvTgfPortArbitrationGroupConfigure
(
    GT_U32 confId,
    GT_BOOL spPrecedenceEnable
);


/**
* @internal prvTgfPortArbitrationGroupConfigurationsSizeGet function
* @endinternal
*
* @brief   Get the size of configuration array(array that hold profile configrations)
*
*/
GT_VOID prvTgfPortArbitrationGroupConfigurationsSizeGet
(
    GT_U32 *confSizePtr
);

/**
* @internal prvTgfPortArbitrationGroupCheckResult function
* @endinternal
*
* @brief   Check configuration result
*
* @param[in] confId -currently checked configuration index
*
*/

GT_VOID prvTgfPortArbitrationGroupCheckResult
(
    GT_U32 confId
);

/**
* @internal prvTgfPortArbitrationGroupSave function
* @endinternal
*
* @brief   save before  test configuration
*

*/
GT_VOID prvTgfPortArbitrationGroupSave
(
   GT_VOID
);

/**
* @internal prvTgfPortArbitrationGroupRestore function
* @endinternal
*
* @brief   restore  test configuration
*

*/
GT_VOID prvTgfPortArbitrationGroupRestore
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortProfileMruCheckh */




