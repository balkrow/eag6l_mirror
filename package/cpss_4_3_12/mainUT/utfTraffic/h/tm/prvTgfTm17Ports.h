/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvTgfTm17Ports.h
*
* @brief Traffic Manager functional testing private API declarations
*
* @version   1
********************************************************************************
*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) /*not supported in CAP - UTs have appDemo references */

typedef enum {

    PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E,

    PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_DELETE_E,

} PRV_TGF_TM_HA_CHECK_SCENARIO_ENT;

GT_VOID prvTgfTm17PortsVlanConfigSet
(
    GT_VOID
);

GT_VOID prvTgfTm17PortsInitSystem
(
    GT_VOID
);

GT_VOID prvTgfTm17PortsTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount
);

GT_VOID prvTgfTm17PortsConfigRestore
(
    GT_VOID
);

GT_VOID prvTgfTmHa17Ports
(
    GT_VOID
);

GT_VOID prvTgfTmHa17PortsFlowsAddTestRestore
(
    PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario
);


/**
 * @brief
 *  1, Perform Play which does add and delete of flows. This
 *  involved adding and deleting of scheduler nodes
 *  2. Perform HA
 *  3  Replay the entires. During replay only play existing
 *  entries and ignore deleted entries
 *  4. Check validity
 */
GT_VOID prvTgfTmHa17PortsFlowsAddTest
(
    IN PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario,
    IN GT_BOOL       emulateCrash
);

#endif
