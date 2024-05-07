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
* @file prvTgfPortArbitrationGroups.c
*
* @brief SIP6 Port Scheduling Profile - check backword compatible mapping to SIP6 scheduler
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#define PRV_TESTED_PORT_IX_CNS 0

#define PRV_TESTED_PORT (prvTgfPortsArray[PRV_TESTED_PORT_IX_CNS ])

CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT testedProfile;
GT_BOOL                                oldSpPrecedenceEnable;



/*Configurations  and expected results*/
#define PRV_ARB_CONF_0 {\
    /*0*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*1*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*2*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*3*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E,\
    /*4*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*5*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*6*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*7*/CPSS_PORT_TX_SP_ARB_GROUP_E}

#define PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_0 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP4_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP5_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP6_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP7_E}

#define PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_0 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP2_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP3_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP4_E}





#define PRV_ARB_CONF_1 {\
    /*0*/CPSS_PORT_TX_SP_ARB_GROUP_E, \
    /*1*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*2*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*3*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E,\
    /*4*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E,\
    /*5*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E,\
    /*6*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E,\
    /*7*/CPSS_PORT_TX_SP_ARB_GROUP_E}

#define PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_1 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP7_E}

#define PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_1 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP0_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP2_E}


#define PRV_ARB_CONF_2 {\
    /*0*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*1*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*2*/CPSS_PORT_TX_WRR_ARB_GROUP_1_E, \
    /*3*/CPSS_PORT_TX_WRR_ARB_GROUP_1_E,\
    /*4*/CPSS_PORT_TX_WRR_ARB_GROUP_1_E,\
    /*5*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*6*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*7*/CPSS_PORT_TX_SP_ARB_GROUP_E}

#define PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_2 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP2_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP2_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP2_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP5_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP6_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP7_E}

#define PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_2 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP2_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP3_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP4_E}


#define PRV_ARB_CONF_3 {\
    /*0*/CPSS_PORT_TX_SP_ARB_GROUP_E, \
    /*1*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*2*/CPSS_PORT_TX_WRR_ARB_GROUP_0_E, \
    /*3*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*4*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*5*/CPSS_PORT_TX_SP_ARB_GROUP_E,\
    /*6*/CPSS_PORT_TX_WRR_ARB_GROUP_1_E,\
    /*7*/CPSS_PORT_TX_WRR_ARB_GROUP_1_E}

#define PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_3 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP1_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP3_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP4_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP5_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP6_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP6_E}


#define PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_3 {\
    /*0*/CPSS_PDQ_SCHED_ELIG_N_FP2_E, \
    /*1*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*2*/CPSS_PDQ_SCHED_ELIG_N_FP0_E, \
    /*3*/CPSS_PDQ_SCHED_ELIG_N_FP3_E,\
    /*4*/CPSS_PDQ_SCHED_ELIG_N_FP4_E,\
    /*5*/CPSS_PDQ_SCHED_ELIG_N_FP5_E,\
    /*6*/CPSS_PDQ_SCHED_ELIG_N_FP1_E,\
    /*7*/CPSS_PDQ_SCHED_ELIG_N_FP1_E}




CPSS_PORT_TX_Q_ARB_GROUP_ENT configurations [][8]= {PRV_ARB_CONF_0,PRV_ARB_CONF_1,PRV_ARB_CONF_2,PRV_ARB_CONF_3};
CPSS_PORT_TX_Q_ARB_GROUP_ENT saveConfigurations[8];

extern  GT_STATUS prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE ** outPtr
);


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
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    rc = cpssDxChPortTxQSpPrecedenceEnableSet(prvTgfDevNum,spPrecedenceEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQSpPrecedenceEnableSet: %d", prvTgfDevNum);

    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(prvTgfDevNum,PRV_TESTED_PORT,&testedProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxBindPortToSchedulerProfileGet: %d", prvTgfDevNum);


    for(i=0;i<8;i++)
    {
        rc =cpssDxChPortTxQArbGroupSet(prvTgfDevNum,i,configurations[confId][i],testedProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQArbGroupSet: %d", prvTgfDevNum);
    }



}


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
)
{

    *confSizePtr = sizeof(configurations)/sizeof(configurations[0]);

}


/**
* @internal prvTgfPortArbitrationGroupCheckResult function
* @endinternal
*
* @brief   Check configuration result
*
*
*/
GT_VOID prvTgfPortArbitrationGroupCheckResult
(
    GT_U32 confId
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT expResNonSpPrecedence[][8]=
        {
         PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_0,
         PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_1,
         PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_2,
         PRV_ARB_EXP_RES_NO_SP_PRECEDENCE_RESULT_3
         };

    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT expResSpPrecedence[][8]=
        {
         PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_0,
         PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_1,
         PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_2,
         PRV_ARB_EXP_RES_SP_PRECEDENCE_RESULT_3
         };

    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT eligFunc;
    GT_U32                            dummy;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE    *aNodePtr=NULL;
    GT_BOOL                           spPrecedence;


    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(prvTgfDevNum,PRV_TESTED_PORT,&aNodePtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortPhysicalPortDetailedMapGet: %d", prvTgfDevNum);

    rc = cpssDxChPortTxQSpPrecedenceEnableGet(prvTgfDevNum,&spPrecedence);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortPhysicalPortDetailedMapGet: %d", prvTgfDevNum);

    for(i=0;i<8;i++)
    {
        rc =prvCpssSip6TxqPdqQSchedParamsGet(prvTgfDevNum,aNodePtr->queuesData.tileNum,aNodePtr->queuesData.pdqQueueFirst +i,&eligFunc,&dummy);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssSip6TxqPdqQSchedParamsGet: %d", prvTgfDevNum);


        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_FALSE==spPrecedence?expResNonSpPrecedence[confId][i]:expResSpPrecedence[confId][i],
                eligFunc,
                "Unexpected elig function for conf %d queue %d (SP precedence %d )",
                confId,i,spPrecedence);
    }



}



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
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(prvTgfDevNum,PRV_TESTED_PORT,&testedProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxBindPortToSchedulerProfileGet: %d", prvTgfDevNum);

        rc = cpssDxChPortTxQSpPrecedenceEnableGet(prvTgfDevNum,&oldSpPrecedenceEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQSpPrecedenceEnableGet: %d", prvTgfDevNum);


    for(i=0;i<8;i++)
    {
        rc =cpssDxChPortTxQArbGroupGet(prvTgfDevNum,i,testedProfile,&saveConfigurations[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQArbGroupSet: %d", prvTgfDevNum);
    }

}


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
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;



    rc = cpssDxChPortTxQSpPrecedenceEnableSet(prvTgfDevNum,oldSpPrecedenceEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQSpPrecedenceEnableSet: %d", prvTgfDevNum);


    for(i=0;i<8;i++)
    {
        rc =cpssDxChPortTxQArbGroupSet(prvTgfDevNum,i,saveConfigurations[i],testedProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChPortTxQArbGroupSet: %d", prvTgfDevNum);
    }

}



