/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfNetIfLinuxKernelEthernetDriver.h
*
* DESCRIPTION:
*      Rx/TX SDMA testing via the mvcpss.ko Linux Kernel Ethernet driver header file
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __prvTgfNetIfLinuxKernelEthernetDriverh
#define __prvTgfNetIfLinuxKernelEthernetDriverh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestInit function
* @endinternal
*
* @brief   Prepares loopback on port #0 + mirror to CPU for Linux kernel Ethernet driver loopback test
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestInit
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRestore function
* @endinternal
*
* @brief   Restore test configurations after Linux kernel Ethernet Driver test
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRun function
* @endinternal
*
* @brief   Linux Kernel Ethernet Driver test - send 1000 DHCP packets and see that they are received.
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRun
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfNetIfLinuxKernelEthernetDriverh */
