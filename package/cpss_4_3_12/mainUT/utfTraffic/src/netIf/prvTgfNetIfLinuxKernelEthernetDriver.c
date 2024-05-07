/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfNetIfLinuxKernelEthernetDriver.c
*
* DESCRIPTION:
*      Rx/TX SDMA testing via the mvcpss.ko Linux Kernel Ethernet driver
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <gtOs/gtOsTask.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <common/tgfCommon.h>
#include <common/tgfNetIfGen.h>
#include <common/tgfPortGen.h>

#define PRV_TGF_LB_PORT_IDX_CNS        3

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestInit function
* @endinternal
*
* @brief   Prepares loopback on port #0 + mirror to CPU for Linux kernel Ethernet driver loopback test
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestInit
(
    GT_VOID
)
{

    GT_STATUS rc;
    CPSS_OS_FILE_TYPE_STC f = { 0, 0, 0 };
    char ifname[32], cmd[128];
    size_t len;
    GT_U32                   portIter    = 0;
    GT_U32                   mgNum;
    CPSS_INTERFACE_INFO_STC  portInterface;
    CPSS_INTERFACE_INFO_STC  targetPortInterface;

    if (GT_FALSE == prvTgfResetModeGet())
    {
            SKIP_TEST_MAC;
    }

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in call to prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in call to tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /* Enable capture on target port */
    targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in call to tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId(prvTgfDevNum, 0, &mgNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in call to prvCpssDxChNetifCheckNetifNumAndConvertToMgUnitId");

    /* Create Linux sub interface: */
    if (cpssOsFopen("/sys/class/net/mvpp0/mg", "w", &f) == CPSS_OS_FILE_INVALID)
    {
        cpssOsPrintf("Cannot open mg selection interface. Probably internal CPU mode\n");
        mgNum = 0xffffffff;
    }

    if (mgNum != 0xffffffff)
    {
        sprintf(cmd, "%u", mgNum);
        len = cpssOsStrlen(cmd);
        if (cpssOsFwrite(cmd, len, 1, &f) != 1)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot set mg number %u to interface!\n", mgNum);
        }

        cpssOsFclose(&f);
        PRV_UTF_LOG1_MAC("Linux kernel mvppnd driver test: mg number set to: %u\n", mgNum);
    }

    /* Create Linux sub interface: */
    if (cpssOsFopen("/sys/class/net/mvpp0/if_create", "w", &f) == CPSS_OS_FILE_INVALID)
    {
        cpssOsPrintf("Cannot open creation interface.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, -1, "Linux kernel mvppnd driver was not loaded!\n");
    }

    sprintf(ifname, "Ethernet%u %u", prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                     prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    len = cpssOsStrlen(ifname);
    if (cpssOsFwrite(ifname, len, 1, &f) != 1)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot create interface!\n");
    }

    cpssOsFclose(&f);

    /* Set Linux interface (main and sub) up: */
    if (osSpawn("ifconfig mvpp0 up") != GT_OK)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot start global interface!\n");
    }

    sprintf(cmd, "ifconfig Ethernet%u up 172.30.0.8", prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    if (osSpawn(cmd) != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot start port interface. Command %s failed!\n", cmd);
    }
}

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRestore function
* @endinternal
*
* @brief   Restore test configurations after Linux kernel Ethernet Driver test
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_OS_FILE_TYPE_STC f = { 0, 0, 0 };
    char ifname[32], cmd[128];
    size_t len;
    CPSS_INTERFACE_INFO_STC  portInterface;
    CPSS_INTERFACE_INFO_STC  targetPortInterface;

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* Disable loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in call to tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /* Disable capture on target port */
    targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in call to tgfTrafficGeneratorPortTxEthCaptureSet");

    sprintf(cmd, "ifconfig Ethernet%u down", prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    osSpawn(cmd);
    osSpawn("ifconfig mvpp0 down");
    /* Delete Linux sub interface: */
    if (cpssOsFopen("/sys/class/net/mvpp0/if_delete", "w", &f) == CPSS_OS_FILE_INVALID)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot open deletion interface!\n");
    }

    sprintf(ifname, "Ethernet%u", prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    len = cpssOsStrlen(ifname);
    if (cpssOsFwrite(ifname, len, 1, &f) != 1)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, -1, "Linux kernel mvppnd driver: Cannot remove sub interface!\n");
    }

    cpssOsFclose(&f);

    prvTgfResetAndInitSystem();

}

/**
* @internal prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRun function
* @endinternal
*
* @brief   Linux Kernel Ethernet Driver test - send 1000 DHCP packets and see that they are received.
*/
GT_VOID prvTgfNetIfRxToTxLinuxKernelEthernetDriverTestRun
(
    GT_VOID
)
{
    CPSS_OS_FILE_TYPE_STC f = { 0, 0, 0 };
    char str[32], cmd[128];
    GT_32 val;
    GT_U32 port = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    sprintf(cmd, "/sys/class/net/Ethernet%u/statistics/rx_packets", port);
    if (cpssOsFopen(cmd, "r", &f) == CPSS_OS_FILE_INVALID)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1000, "Linux kernel mvppnd driver test: Failed to open statistics file\n");
    }

    sprintf(cmd, "/bin/sh -c \'busybox udhcpc -i Ethernet%u -B -t 1000 -T 0 -A 0 -n 2> /dev/null\'", port);
    osSpawn(cmd); /* Send 1000 DHCP packets */

    val = cpssOsFread(str, 1, sizeof(str) - 1, &f);

    if (val  <= 0)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(0, 1000, "Linux kernel mvppnd driver test: read %d items from statistics file\n", val);
    }

    str[val] = '\0';

    cpssOsFclose(&f);
    val = cpssOsStrTol(str, NULL, 10); /* Get count of packets received on interface */

    PRV_UTF_LOG1_MAC("Linux kernel mvppnd driver test: Received %d packets\n", val);
    if (val < 1000) /* Expect 1000 packets to be received in loop */
        {
            /* test failure */
            UTF_VERIFY_EQUAL1_STRING_MAC(1000, val, "Port Rx packet count is %d (should be at least 1000)\n", val);
        }

}
