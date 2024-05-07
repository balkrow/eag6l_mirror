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
* @file  prvTgfTmHaBasic.h
*
* @brief Traffic Manager High Availability Basic API testing
*
* @version   1
********************************************************************************
*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) /*not supported in CAP - UTs have appDemo references */

GT_VOID prvTgfTmHaBasicValidityCheck
(
    GT_VOID
);


GT_VOID prvTgfTmHaBasicCatchupCheck
(
    GT_VOID
);

GT_VOID prvTgfTmHaScenariosCheck
(
    GT_VOID
);

GT_VOID prvTgfTmHaBasicMemLeakCheck
(
    GT_VOID
);

GT_VOID prvTgfTmHaBasicResourceManagerStoreCheck
(
    GT_VOID
);

GT_VOID prvTgfTmHaBasicResourceManagerRecoveryCheck
(
    GT_VOID
);

#endif
