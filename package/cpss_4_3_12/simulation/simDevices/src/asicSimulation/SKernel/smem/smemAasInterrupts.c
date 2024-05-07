/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemAasInterrupts.c
*
* DESCRIPTION:
*       Aas interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemAas.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetAasExactMatch.h>

/* Aas interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC aasInterruptsTreeDb[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define  aas1_GlobalInterruptsSummary_tree(treeId)                                                      \
static SKERNEL_INTERRUPT_REG_INFO_STC  aas1_GlobalInterruptsSummary_##treeId =                          \
{                                                                                                       \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].globalInterruptCause)},\
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].globalInterruptMask)}, \
    /*myFatherInfo*/{                                                                                   \
        /*myBitIndex*/0 ,                                                                               \
        /*interruptPtr*/NULL /* no father for me , thank you */                                         \
        },                                                                                              \
    /*isTriggeredByWrite*/ 1                                                                            \
}

aas1_GlobalInterruptsSummary_tree(0);
aas1_GlobalInterruptsSummary_tree(1);
aas1_GlobalInterruptsSummary_tree(2);

#define  aas1_aggregationInterruptsSummary_tree(treeId)                                      \
static SKERNEL_INTERRUPT_REG_INFO_STC  aas1_aggregationInterruptsSummary_##treeId =          \
{                                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].cnmAggregationInterruptCause)},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].cnmAggregationInterruptMask)},     \
    /*myFatherInfo*/{                                                                                   \
        /*myBitIndex*/1 ,                                                                               \
        /*interruptPtr*/&aas1_GlobalInterruptsSummary_##treeId                                             \
        },                                                                                              \
    /*isTriggeredByWrite*/ 1                                                                            \
}

aas1_aggregationInterruptsSummary_tree(0);
aas1_aggregationInterruptsSummary_tree(1);
aas1_aggregationInterruptsSummary_tree(2);

#define  aas1_gdmaInterruptsSummary_tree(treeId, gdmaIndex)                                                     \
static SKERNEL_INTERRUPT_REG_INFO_STC  aas1_gdmaInterruptsSummary_##treeId##_##gdmaIndex  =                     \
    {                                                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].gdmaInterruptCause[gdmaIndex])},   \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.Interrupts.Trees[treeId].gdmaInterruptMask[gdmaIndex])},    \
    /*myFatherInfo*/{                                                                                           \
        /*myBitIndex*/5 + (gdmaIndex),                                                                               \
        /*interruptPtr*/&aas1_aggregationInterruptsSummary_##treeId                                             \
        },                                                                                                      \
    /*isTriggeredByWrite*/ 1                                                                                    \
    }

aas1_gdmaInterruptsSummary_tree(0, 0);
aas1_gdmaInterruptsSummary_tree(0, 1);
aas1_gdmaInterruptsSummary_tree(0, 2);
aas1_gdmaInterruptsSummary_tree(0, 3);
aas1_gdmaInterruptsSummary_tree(0, 4);
aas1_gdmaInterruptsSummary_tree(0, 5);
aas1_gdmaInterruptsSummary_tree(0, 6);
aas1_gdmaInterruptsSummary_tree(0, 7);

aas1_gdmaInterruptsSummary_tree(1, 0);
aas1_gdmaInterruptsSummary_tree(1, 1);
aas1_gdmaInterruptsSummary_tree(1, 2);
aas1_gdmaInterruptsSummary_tree(1, 3);
aas1_gdmaInterruptsSummary_tree(1, 4);
aas1_gdmaInterruptsSummary_tree(1, 5);
aas1_gdmaInterruptsSummary_tree(1, 6);
aas1_gdmaInterruptsSummary_tree(1, 7);

aas1_gdmaInterruptsSummary_tree(2, 0);
aas1_gdmaInterruptsSummary_tree(2, 1);
aas1_gdmaInterruptsSummary_tree(2, 2);
aas1_gdmaInterruptsSummary_tree(2, 3);
aas1_gdmaInterruptsSummary_tree(2, 4);
aas1_gdmaInterruptsSummary_tree(2, 5);
aas1_gdmaInterruptsSummary_tree(2, 6);
aas1_gdmaInterruptsSummary_tree(2, 7);

#define AAS1_GDMA_NODE_MAC(treeId, gdmaUnit/*0..3*/, gdmaIndex/*0..7*/, gdmaQueue)                             \
    {                                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.GDMA[gdmaUnit].interruptAndDebugRegs.gdmaQInterruptCause[gdmaQueue])},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNM.GDMA[gdmaUnit].interruptAndDebugRegs.gdmaQInterruptMask[gdmaQueue])},     \
    /*myFatherInfo*/{                                                                                                           \
        /*myBitIndex*/1 + ((gdmaQueue) % 16),                                   \
        /*interruptPtr*/&aas1_gdmaInterruptsSummary_##treeId##_##gdmaIndex      \
        },                                                                      \
    /*isTriggeredByWrite*/ 1                                                    \
    }

#define AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,gdmaIndex/*0..7*/,gdmaUnit/*0..3*/,startQueue)                                 \
     AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 0 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 1 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 2 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 3 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 4 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 5 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 6 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 7 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 8 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, ( 9 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (10 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (11 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (12 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (13 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (14 + startQueue))                   \
    ,AAS1_GDMA_NODE_MAC(treeId, gdmaUnit, gdmaIndex, (15 + startQueue))

#define AAS1_GDMA_ALL_GDMA_NODE_MAC(treeId)                                   \
     AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,0,0, 0)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,1,0,16)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,2,1, 0)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,3,1,16)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,4,2, 0)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,5,2,16)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,6,3, 0)                          \
    ,AAS1_GDMA_ALL_QUEUES_SUMMARY_MAC(treeId,7,3,16)


static SKERNEL_INTERRUPT_REG_INFO_STC  aas1_gdmaInterruptNodes[]  =
{
     AAS1_GDMA_ALL_GDMA_NODE_MAC(0)
    ,AAS1_GDMA_ALL_GDMA_NODE_MAC(1)
    ,AAS1_GDMA_ALL_GDMA_NODE_MAC(2)
};



/**
* @internal internal_smemAas1InterruptTreeInit_perTile function
* @endinternal
*
* @brief   Init the interrupts tree for the Aas device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID internal_smemAas1InterruptTreeInit_perTile
(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    IN      GT_U32                      tileId,
    INOUT    GT_U32                    *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = *indexPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = *currDbPtrPtr;


    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_GlobalInterruptsSummary_0);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_aggregationInterruptsSummary_0);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_0);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_1);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_2);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_3);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_4);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_5);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_6);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  &aas1_gdmaInterruptsSummary_0_7);
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr,  aas1_gdmaInterruptNodes);

    *indexPtr = index;
    *currDbPtrPtr = currDbPtr;
}


/*******************************************************************************
*   smemAasInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Aas device
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
GT_VOID smemAasInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    devObjPtr->myInterruptsDbPtr = aasInterruptsTreeDb;
    devObjPtr->support3InterruptTrees = 1;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    internal_smemAas1InterruptTreeInit_perTile(devObjPtr,0/*tileId*/,&index,&currDbPtr);

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}


