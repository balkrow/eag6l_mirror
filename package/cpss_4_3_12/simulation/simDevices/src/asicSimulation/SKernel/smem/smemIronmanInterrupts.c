/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemIronmanInterrupts.c
*
* DESCRIPTION:
*       Ironman interrupt tree related implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemIronman.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

/* Ironman interrupt tree database */
static SKERNEL_INTERRUPT_REG_INFO_STC ironmanInterruptsTreeDb[MAX_INTERRUPT_NODES];

static SKERNEL_INTERRUPT_REG_INFO_STC skernel_interrupt_reg_info__last_line__cns[] =
{{{SKERNEL_REGISTERS_DB_TYPE__LAST___E}}};


#define cnm_grp_0_0_intr  (ironman_FuncUnitsInterruptsSummary  [0])
#define cnm_grp_0_1_intr  (ironman_dataPathInterruptSummary    [0])
#define cnm_grp_0_2_intr  (ironman_portsInterruptsSummary      [0])
#define cnm_grp_0_3_intr  (ironman_dfxInterruptsSummary        [0])
#define cnm_grp_0_4_intr  (ironman_dfx1InterruptsSummary       [0])
#define cnm_grp_0_5_intr  (ironman_FuncUnits1InterruptsSummary [0])

#define cnm_grp_0_0_intr_x(x)  (ironman_FuncUnitsInterruptsSummary  [x])
#define cnm_grp_0_2_intr_x(x)  (ironman_portsInterruptsSummary  [x])


enum{  /*cnm_grp_0_0_intr  - functionalInterruptsSummaryCause/FuncUnitsInterruptsSummary */

    cnm_grp_0_0_cp_top_mt_int_sum                 = 2,
    cnm_grp_0_0_cp_top_pipe0_bma_int_sum          = 3,
    cnm_grp_0_0_cp_top_pipe0_cnc0_int_sum         = 4,
    cnm_grp_0_0_cp_top_pipe0_eft_int_sum          = 5,
    cnm_grp_0_0_cp_top_pipe0_epcl_int_sum         = 7,
    cnm_grp_0_0_cp_top_pipe0_eplr_int_sum         = 8,
    cnm_grp_0_0_cp_top_pipe0_eq_int_sum           = 9,
    cnm_grp_0_0_cp_top_pipe0_erep_int_sum         = 10,
    cnm_grp_0_0_cp_top_pipe0_ermrk_int_sum        = 11,
    cnm_grp_0_0_cp_top_pipe0_ha_int_sum           = 12,
    cnm_grp_0_0_cp_top_pipe0_hbu_int_sum          = 13,
    cnm_grp_0_0_cp_top_pipe0_ioam_int_sum         = 14,
    cnm_grp_0_0_cp_top_pipe0_iplr0_int_sum        = 15,
    cnm_grp_0_0_cp_top_pipe0_iplr1_int_sum        = 16,
    cnm_grp_0_0_cp_top_pipe0_ipvx_int_sum         = 17,
    cnm_grp_0_0_cp_top_pipe0_l2i_int_sum          = 18,
    cnm_grp_0_0_cp_top_pipe0_lpm_int_sum          = 19,
    cnm_grp_0_0_cp_top_pipe0_mll_int_sum          = 20,
    cnm_grp_0_0_cp_top_pipe0_pcl_int_sum          = 21,
    cnm_grp_0_0_cp_top_pipe0_preq_int_sum         = 22,
    cnm_grp_0_0_cp_top_pipe0_qag_int_sum          = 23,
    cnm_grp_0_0_cp_top_pipe0_sht_int_sum          = 24,
    cnm_grp_0_0_cp_top_pipe0_smu_int_sum          = 25,
    cnm_grp_0_0_cp_top_pipe0_tti_int_sum          = 26
};


enum{ /*cnm_grp_0_1_intr_ - SERInterruptsSummary/dataPathInterruptSummary */
    cnm_grp_0_1_intr_Tcam_Int_sum                 = 1,
    cnm_grp_0_1_intr_Rxdma_Int_sum                = 3,
    cnm_grp_0_1_intr_Txdma_Int_sum                = 4,
    cnm_grp_0_1_intr_Txfifo_Int_sum               = 5,
    cnm_grp_0_1_intr_Tsu_Int_sum                  = 6,
    cnm_grp_0_1_intr_anp_Int_sum_0                = 14,
    cnm_grp_0_1_intr_anp_Int_sum_1                = 15,
    cnm_grp_0_1_intr_anp_Int_sum_2                = 16,
    cnm_grp_0_1_intr_anp_Int_sum_3                = 17,
    cnm_grp_0_1_intr_anp_Int_sum_4                = 18,
    cnm_grp_0_1_intr_mac_Int_sum_0                = 20,
    cnm_grp_0_1_intr_mac_Int_sum_1                = 21,
    cnm_grp_0_1_intr_mac_Int_sum_2                = 22,
    cnm_grp_0_1_intr_pcs_Int_sum_0                = 26,
    cnm_grp_0_1_intr_pcs_Int_sum_1                = 27,
    cnm_grp_0_1_intr_pcs_Int_sum_2                = 28,
    cnm_grp_0_1_intr_sdw_Int_sum_0                = 29,
    cnm_grp_0_1_intr_sdw_Int_sum_1                = 30,
    cnm_grp_0_1_intr_sdw_Int_sum_2                = 31
};

enum{ /*cnm_grp_0_2_intr_ - stackPortsInterruptCause/portsInterruptsSummary */
    cnm_grp_0_2_intr_anp_Int_sum_0           = 4,
    cnm_grp_0_2_intr_anp_Int_sum_1           = 5,
    cnm_grp_0_2_intr_anp_Int_sum_2           = 6,
    cnm_grp_0_2_intr_anp_Int_sum_3           = 7,
    cnm_grp_0_2_intr_anp_Int_sum_4           = 8,
    cnm_grp_0_2_intr_mac_Int_sum_0           = 10,
    cnm_grp_0_2_intr_mac_Int_sum_1           = 11,
    cnm_grp_0_2_intr_mac_Int_sum_2           = 12,
    cnm_grp_0_2_intr_pcs_Int_sum_0           = 16,
    cnm_grp_0_2_intr_pcs_Int_sum_1           = 17,
    cnm_grp_0_2_intr_pcs_Int_sum_2           = 18,
    cnm_grp_0_2_intr_sdw_Int_sum_0           = 19,
    cnm_grp_0_2_intr_sdw_Int_sum_1           = 20,
    cnm_grp_0_2_intr_sdw_Int_sum_2           = 21
};

enum{ /*cnm_grp_0_3_intr_ -  dfxInterruptsSummaryCause */
    cnm_grp_0_3_intr_anp_Int_sum_0           = 4,
    cnm_grp_0_3_intr_anp_Int_sum_1           = 5,
    cnm_grp_0_3_intr_anp_Int_sum_2           = 6,
    cnm_grp_0_3_intr_anp_Int_sum_3           = 7,
    cnm_grp_0_3_intr_anp_Int_sum_4           = 8,
    cnm_grp_0_3_intr_mac_Int_sum_0           = 10,
    cnm_grp_0_3_intr_mac_Int_sum_1           = 11,
    cnm_grp_0_3_intr_mac_Int_sum_2           = 12,
    cnm_grp_0_3_intr_pcs_Int_sum_0           = 16,
    cnm_grp_0_3_intr_pcs_Int_sum_1           = 17,
    cnm_grp_0_3_intr_pcs_Int_sum_2           = 18,
    cnm_grp_0_3_intr_sdw_Int_sum_0           = 19,
    cnm_grp_0_3_intr_sdw_Int_sum_1           = 20,
    cnm_grp_0_3_intr_sdw_Int_sum_2           = 21
};

enum{ /*cnm_grp_0_4_intr_ -  dfx1InterruptsSummaryCause */
    cnm_grp_0_4_intr_anp_Int_sum_0           = 5,
    cnm_grp_0_4_intr_anp_Int_sum_1           = 6,
    cnm_grp_0_4_intr_anp_Int_sum_2           = 7,
    cnm_grp_0_4_intr_anp_Int_sum_3           = 8,
    cnm_grp_0_4_intr_mac_Int_sum_0           = 10,
    cnm_grp_0_4_intr_mac_Int_sum_1           = 11,
    cnm_grp_0_4_intr_mac_Int_sum_2           = 12,
    cnm_grp_0_4_intr_mac_Int_sum_3           = 13,
    cnm_grp_0_4_intr_pcs_Int_sum_0           = 18,
    cnm_grp_0_4_intr_pcs_Int_sum_1           = 19,
    cnm_grp_0_4_intr_pcs_Int_sum_2           = 20,
    cnm_grp_0_4_intr_pcs_Int_sum_3           = 21,
    cnm_grp_0_4_intr_sdw_Int_sum_0           = 22,
    cnm_grp_0_4_intr_sdw_Int_sum_1           = 23,
    cnm_grp_0_4_intr_sdw_Int_sum_2           = 24,
    cnm_grp_0_4_intr_sdw_Int_sum_3           = 25
};

enum{  /*cnm_grp_0_5_intr_  - functionalInterruptsSummary1Cause */
    cnm_grp_5_intr_ironman_MG1_int_out0_sum = 4,    /* MG1 */
    cnm_grp_5_intr_ironman_MG2_int_out0_sum = 7,    /* MG2 */

};

enum{
     MG_0_INDEX
    ,MG_1_INDEX
    ,MG_2_INDEX

    ,NUM_OF_MGS   /*3*/
};

#define SINGLE_TREE_ID  0

#define MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,mgIndex,father_myBitIndex,father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].mg_causeRegName),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].mg_maskRegName ),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

#define MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,mgIndex,father_myBitIndex,father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].SDMARegs.mg_causeRegName),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].SDMARegs.mg_maskRegName ),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

#define UNSED_MG {{0}}

/* duplicate the node of MG only to MG_x_0 (primary MG units) */
#define DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,UNSED_MG/* MG 0_1 not used */                                                                   \
    ,UNSED_MG/* MG 0_2 not used */

/* duplicate the node of MG to ALL MG_x_x */
#define DUP_ALL_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX]) \
    ,MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_INDEX,father_myBitIndex,father_interrupt[MG_2_INDEX])

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_father_FuncUnits1InterruptsSummary[NUM_OF_MGS];
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_FuncUnits1InterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummary1Cause ,functionalInterruptsSummary1Mask ,FuncUnits1IntsSum ,ironman_father_FuncUnits1InterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_GlobalInterruptsSummary    [NUM_OF_MGS] =
{
    { /* copied from GlobalInterruptsSummary */
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].globalInterruptCause),0,MG_0_INDEX | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS},/*SMEM_CHT_GLB_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.globalInterrupt[SINGLE_TREE_ID].globalInterruptMask) ,0,MG_0_INDEX | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS},
        /*myFatherInfo*/{
            /*myBitIndex*/0 ,
            /*interruptPtr*/NULL /* no father for me , thank you */
            },
        /*isTriggeredByWrite*/ 1
    }
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_1_INDEX,cnm_grp_5_intr_ironman_MG1_int_out0_sum,ironman_FuncUnits1InterruptsSummary[MG_0_INDEX])/* into MG_0_0 (tile 0)*/
    ,MG_INTERTTUPT_REG(globalInterruptCause,globalInterruptMask ,MG_2_INDEX,cnm_grp_5_intr_ironman_MG2_int_out0_sum,ironman_FuncUnits1InterruptsSummary[MG_0_INDEX])

};

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_FuncUnitsInterruptsSummary [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(functionalInterruptsSummaryCause ,functionalInterruptsSummaryMask ,FuncUnitsIntsSum ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_dataPathInterruptSummary   [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(SERInterruptsSummary             ,SERInterruptsMask               ,DataPathIntSum   ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_portsInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(stackPortsInterruptCause     ,stackPortsInterruptMask     ,PortsIntSum  ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_dfxInterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfxInterruptsSummaryCause     ,dfxInterruptsSummaryMask           ,dfxIntSum        ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_dfx1InterruptsSummary     [NUM_OF_MGS] =
{    DUP_PRIMARY_ONLY_MG_INTERTTUPT_REG(dfx1InterruptsSummaryCause    ,dfx1InterruptsSummaryMask          ,dfx1IntSum       ,ironman_GlobalInterruptsSummary) };

/* this node need support in primary and secondary MG units (to support CNC dump from the FUQ) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_misc_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(miscellaneousInterruptCause ,miscellaneousInterruptMask ,MgInternalIntSum ,ironman_GlobalInterruptsSummary) };

/* this node need support in all MG units (to support CM3 interrupts) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg1_internal_interrupts  [NUM_OF_MGS] =
{    DUP_ALL_MG_INTERTTUPT_REG(mg1InternalInterruptsSummaryCause ,mg1InternalInterruptsSummaryMask ,Mg1InternalIntSum ,ironman_GlobalInterruptsSummary) };

#define MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,mgIndex) \
     MG_SDMA_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,mgIndex,father_myBitIndex,father_interrupt[mgIndex])


/* duplicate the node of MG to all MG units */
#define DUP_SDMA_MG_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     MG_SDMA_INTERTTUPT_REG_ENTRY(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt,MG_0_INDEX)

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_sdma_receiveSDMA_interrupts  [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(receiveSDMAInterruptCause ,receiveSDMAInterruptMask ,RxSDMASum ,ironman_GlobalInterruptsSummary) };

/* this node need support in ALL MG units (to MG SDMA) */
static SKERNEL_INTERRUPT_REG_INFO_STC  ironman_mg_global_sdma_transmitSDMA_interrupts [NUM_OF_MGS] =
{    DUP_SDMA_MG_INTERTTUPT_REG(transmitSDMAInterruptCause ,transmitSDMAInterruptMask ,TxSDMASum ,ironman_GlobalInterruptsSummary) };

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_summary_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryCauseReg)},/*SMEM_CHT3_CNC_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.CNCInterruptSummaryMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_cnc0_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 0
    }
};

/* Interrupt register info for CNC block client interrupt cause */
#define CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(block, client)                                                                                 \
{                                                                                                                                                    \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].perBlockRegs.clientInterrupts.CNCBlockClientInterruptCauseRegister[block][client])},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].perBlockRegs.clientInterrupts.CNCBlockClientInterruptMaskRegister[block][client])},     \
    /*myFatherInfo*/{                                                                                                                                \
        /*myBitIndex*/0 ,                                                                                                                            \
        /*interruptPtr*/cnc_summary_interrupts                                                                                                       \
        },                                                                                                                                           \
    /*isTriggeredByWrite*/ 1                                                                                                                         \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cnc_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.wraparoundFunctionalInterruptCauseReg)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(CNC[0].globalRegs.wraparoundFunctionalInterruptMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/1 ,
        /*interruptPtr*/cnc_summary_interrupts
        },
    /*isTriggeredByWrite*/ 1
    },
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(0, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(0, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(0, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(1, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(1, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(1, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(2, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(2, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(2, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(3, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(3, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(3, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(4, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(4, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(4, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(5, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(5, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(5, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(6, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(6, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(6, 2),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(7, 0),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(7, 1),
    CNC_BLOCK_CLIENT_INTERRUPT_CAUSE_REGISTER_MAC(7, 2)
};

static SKERNEL_INTERRUPT_REG_INFO_STC egf_eft_global_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EGF_eft.global.EFTInterruptsMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eft_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC oam_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptCause)},/*SMEM_LION2_OAM_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(OAMUnit[0].OAMUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ioam_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC pcl_unit_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCL.PCLUnitInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_pcl_int_sum,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC tti_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptCause)},/*SMEM_LION3_TTI_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TTI.TTIEngine.TTIEngineInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_tti_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC bridge_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptCause)},/*SMEM_CHT_BRIDGE_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(L2I.bridgeEngineInterrupts.bridgeInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_l2i_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC preq_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PREQ.preqInterrupts.preqInterruptCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PREQ.preqInterrupts.preqInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_preq_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC mac_tbl_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptCauseReg)},/*SMEM_CHT_MAC_TBL_INTR_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(FDB.FDBCore.FDBInterrupt.FDBInterruptMaskReg)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_mt_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC TcamInterruptsSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryCause)},/**/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptsSummaryMask)},/**/
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_1_intr_Tcam_Int_sum ,
        /*interruptPtr*/&cnm_grp_0_1_intr /* tile0 */
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC tcam_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(TCAM.tcamInterrupts.TCAMInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&TcamInterruptsSummary[0],
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC ha_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptCause)},/*SMEM_LION3_HA_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(HA.HAInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ha_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC ermrk_interrupts[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptCause)},/*SMEM_LION3_ERMRK_INTERRUPT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(ERMRK.ERMRKInterruptMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_ermrk_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr,
        },
    /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC  preEgrInterruptSummary[] =
{
    {
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummary)}, /*SMEM_CHT_EQ_INT_CAUSE_REG*/
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.preEgrInterrupt.preEgrInterruptSummaryMask)},  /*SMEM_CHT_EQ_INT_MASK_REG*/
    /*myFatherInfo*/{
        /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eq_int_sum ,
        /*interruptPtr*/&cnm_grp_0_0_intr
        },
    /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC CpuCodeRateLimitersInterruptSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&preEgrInterruptSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(n)                                                                        \
{                                                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1])},    \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1])},     \
    /*myFatherInfo*/{                                                                                                          \
        /*myBitIndex*/n ,                                                                                                      \
        /*interruptPtr*/&CpuCodeRateLimitersInterruptSummary[0]                                                             \
        },                                                                                                                     \
    /*isTriggeredByWrite*/ 1                                                                                                   \
}

static SKERNEL_INTERRUPT_REG_INFO_STC cpu_code_rate_limiters_interrupts[] =
{
     CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 1)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 2)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 3)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 4)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 5)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 6)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 7)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 8)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC( 9)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(10)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(11)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(12)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(13)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(14)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(15)
    ,CPU_CODE_RATE_LIMITERS_INTERRUPT_CAUSE_MAC(16)
};

static SKERNEL_INTERRUPT_REG_INFO_STC mll_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptCauseReg)},/*SMEM_LION3_MLL_INTERRUPT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(MLL.MLLGlobalCtrl.MLLInterruptMaskReg)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_mll_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC policer_ipfix_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[0].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_iplr0_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[1].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_iplr1_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptCause)},/*SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG*/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PLR[2].policerInterruptMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_0_cp_top_pipe0_eplr_int_sum ,
            /*interruptPtr*/&cnm_grp_0_0_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define NUM_USX_PORTS_PER_GROUP 8

/* define the USX MAC global interrupt summary */
#define  USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_usx_global_interruptSummary_group##group = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group].MTI_GLOBAL.globalInterruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_EXTERNAL[group].MTI_GLOBAL.globalInterruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

/* define the USX MAC global interrupt summary per USX ports 0..7 */
#define MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(unit, group) \
    USX_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, cnm_grp_0_##unit##_intr_mac_Int_sum_0 + (group/3)/*bitInMg*/, cnm_grp_0_##unit##_intr/*regInMg*/)

MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(1, 0); /* MAC_BR       * 8 ports (0..7)     instance 0 */
MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(1, 3); /* MAC_NON_BR   * 8 ports (0..7)     instance 0 */

MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(2, 1); /* MAC_BR       * 8 ports (8..15)    instance 1 */
MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(2, 4); /* MAC_NON_BR   * 8 ports (8..15)    instance 1 */

MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(3, 2); /* MAC_BR       * 8 ports (16..23)   instance 2 */
MTI_USX_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(3, 5); /* MAC_NON_BR   * 8 ports (16..23)   instance 2 */


/* node for USX mac ports interrupt summary cause */
#define  USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(group, port) \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_usx_port_interruptSummary_##group##_##port = \
{                                                                                   \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portSummaryInterruptCause)}, /* */ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portSummaryInterruptMask) }, /* */ \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(2+port),                             \
        /*interruptPtr*/&mti_usx_global_interruptSummary_group##group      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                              \
}

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0, 7);

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1, 7);

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2, 7);

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3, 7);

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4, 7);

USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 0);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 1);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 2);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 3);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 4);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 5);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 6);
USX_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5, 7);

/* node for USX mac ports interrupt cause */
#define USX_PORT_INTERRUPT_CAUSE_MAC(group, port)                   \
{                                                                   \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI[group * NUM_USX_PORTS_PER_GROUP + port].MTI_EXT_PORT.portInterruptMask) },  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                               \
        /*myBitIndex*/(1),                                          \
        /*interruptPtr*/&mti_usx_port_interruptSummary_##group##_##port \
    },                                                              \
    /*isTriggeredByWrite*/ 1                                        \
}

/* define USX ports interrupt summary per group */
#define MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(group)          \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 0),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 1),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 2),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 3),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 4),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 5),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 6),                         \
    USX_PORT_INTERRUPT_CAUSE_MAC(group, 7)

/* node for CPU ports interrupt summary cause */
#define MTI_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(index)             \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_cpu_global_interruptSummary_##index = \
{                                                                   \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_GLOBAL.globalInterruptSummaryCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_GLOBAL.globalInterruptSummaryMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                               \
        /*myBitIndex*/(cnm_grp_0_4_intr_mac_Int_sum_0 + (index)),   \
        /*interruptPtr*/&cnm_grp_0_4_intr                           \
    },                                                              \
    /*isTriggeredByWrite*/ 1                                        \
}

MTI_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(0);
MTI_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(1);
MTI_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(2);
MTI_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(3);

/* node for CPU ports interrupt summary cause */
#define MTI_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(index)             \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_cpu_port_interruptSummary_##index = \
{                                                                   \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portSummaryInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portSummaryInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                               \
        /*myBitIndex*/(2),   \
        /*interruptPtr*/&mti_cpu_global_interruptSummary_##index                           \
    },                                                              \
    /*isTriggeredByWrite*/ 1                                        \
}

MTI_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(0);
MTI_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(1);
MTI_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(2);
MTI_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(3);

/* node for CPU ports interrupt cause */
#define MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(index)          \
{                                                                   \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                               \
        /*myBitIndex*/(1),                                          \
        /*interruptPtr*/&mti_cpu_port_interruptSummary_##index      \
    },                                                              \
    /*isTriggeredByWrite*/ 1                                        \
}

/* node for USX CPU global interrupt summary cause */
#define MTI_USX_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(group, index)    \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_usx_cpu_global_interruptSummary_##index = \
{                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_GLOBAL.globalInterruptSummaryCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_GLOBAL.globalInterruptSummaryMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(cnm_grp_0_##group##_intr_mac_Int_sum_0 + 2), \
        /*interruptPtr*/&cnm_grp_0_##group##_intr                   \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

MTI_USX_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(1, 4);   /* USX CPU port 52 - instance 0  */
MTI_USX_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(2, 5);   /* USX CPU port 53 - instance 1  */
MTI_USX_CPU_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MAC(3, 6);   /* USX CPU port 54 - instance 2  */

/* node for USX CPU ports interrupt summary cause */
#define MTI_USX_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(index)    \
static SKERNEL_INTERRUPT_REG_INFO_STC  mti_usx_cpu_port_interruptSummary_##index = \
{                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portSummaryInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portSummaryInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(2), \
        /*interruptPtr*/&mti_usx_cpu_global_interruptSummary_##index                   \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

MTI_USX_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(4);
MTI_USX_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(5);
MTI_USX_CPU_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(6);

/* node for USX CPU ports interrupt cause */
#define MTI_USX_CPU_PORT_INTERRUPT_CAUSE_MAC(index)         \
{                                                           \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portInterruptCause)}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_MTI_cpu_port[index].MTI_EXT_PORT.portInterruptMask)},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(1),                                  \
        /*interruptPtr*/&mti_usx_cpu_port_interruptSummary_##index   \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

/* define the ANP1 global interrupt summary */
#define  ANP_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, bitInMg, regInMg) \
static SKERNEL_INTERRUPT_REG_INFO_STC  anp_global_interruptSummary_group##group = \
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.interruptSummaryCause)}, /**/  \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.interruptSummaryMask)},  /**/  \
    /*myFatherInfo*/{                              \
        /*myBitIndex*/ bitInMg ,                   \
        /*interruptPtr*/&regInMg                   \
        },                                         \
    /*isTriggeredByWrite*/ 1                       \
}

#define ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(group, unit) \
    ANP_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (cnm_grp_0_##unit##_intr_anp_Int_sum_0 + (group%5))/*bitInMg*/, cnm_grp_0_##unit##_intr/*regInMg*/)

#define ANP_CPU_4_6_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(group, unit) \
    ANP_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (cnm_grp_0_##unit##_intr_anp_Int_sum_4)/*bitInMg*/, cnm_grp_0_##unit##_intr/*regInMg*/)

#define ANP_CPU_0_3_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(group) \
    ANP_SUMMARY_GLOBAL_INTERRUPT_SUMMARY_MAC(group, (cnm_grp_0_4_intr_anp_Int_sum_0 + ((group)%15))/*bitInMg*/, cnm_grp_0_4_intr/*regInMg*/)

ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (0, 1);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (1, 1);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (2, 1);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (3, 1);
ANP_CPU_4_6_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (4, 1);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (5, 2);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (6, 2);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (7, 2);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (8, 2);
ANP_CPU_4_6_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC (9, 2);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(10, 3);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(11, 3);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(12, 3);
ANP_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(13, 3);
ANP_CPU_4_6_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(14, 3);
ANP_CPU_0_3_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(15);
ANP_CPU_0_3_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(16);
ANP_CPU_0_3_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(17);
ANP_CPU_0_3_SUMMARY_PORT_GROUP_INTERRUPT_SUMMARY_MAC(18);


#define ANP_PORT_INTERRUPT_CAUSE_MAC(group)      \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptCause[0])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/ \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip_6_10_ANP[group].ANP.portInterruptMask[0])},  /* SMEM_CHT_PORT_INT_CAUSE_MASK_REG */  \
    /*myFatherInfo*/{                                       \
        /*myBitIndex*/(2),    \
        /*interruptPtr*/&anp_global_interruptSummary_group##group \
    },                                                      \
    /*isTriggeredByWrite*/ 1                                \
}

static SKERNEL_INTERRUPT_REG_INFO_STC ironman_mac_ports_interrupts[] =
{
     MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(0) /* ports  0..7  */
    ,MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(1) /* ports  8..15 */
    ,MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(2) /* ports 16..23 */
    ,MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(3) /* ports 24..31 */
    ,MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(4) /* ports 32..39 */
    ,MTI_USX_PORTS_PER_GROUP_INTERRUPT_CAUSE_MAC(5) /* ports 40..47 */
    ,MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(0)            /* ports 48 */
    ,MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(1)            /* ports 49 */
    ,MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(2)            /* ports 50 */
    ,MTI_CPU_PORT_INTERRUPT_CAUSE_MAC(3)            /* ports 51 */
    ,MTI_USX_CPU_PORT_INTERRUPT_CAUSE_MAC(4)        /* ports 52 */
    ,MTI_USX_CPU_PORT_INTERRUPT_CAUSE_MAC(5)        /* ports 53 */
    ,MTI_USX_CPU_PORT_INTERRUPT_CAUSE_MAC(6)        /* ports 54 */

    ,ANP_PORT_INTERRUPT_CAUSE_MAC(0)  /* ANP1 USX0 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(1)  /* ANP1 USX1 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(2)  /* ANP1 USX2 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(3)  /* ANP1 USX3 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(4)  /* ANP1 USX4 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(5)  /* ANP1 USX5 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(6)  /* ANP1 USX6 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(7)  /* ANP1 USX7 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(8)  /* ANP1 USX8 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(9)  /* ANP1 USX9 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(10) /* ANP1 USX10 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(11) /* ANP1 USX11 */

    ,ANP_PORT_INTERRUPT_CAUSE_MAC(12) /* ANP CPU_0 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(13) /* ANP CPU_1 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(14) /* ANP CPU_2 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(15) /* ANP CPU_3 */

    ,ANP_PORT_INTERRUPT_CAUSE_MAC(16) /* ANP CPU_4 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(17) /* ANP CPU_5 */
    ,ANP_PORT_INTERRUPT_CAUSE_MAC(18) /* ANP CPU_6 */
};

static SKERNEL_INTERRUPT_REG_INFO_STC RxDmaInterruptsSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_cause)},/**/
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_mask)},/**/
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Rxdma_Int_sum ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};


static SKERNEL_INTERRUPT_REG_INFO_STC rxdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_0_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_0_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/1 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_1_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_1_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_2_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_rxDMA[0].interrupts.rxdma_interrupt_2_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&RxDmaInterruptsSummary[0]
            },
        /*isTriggeredByWrite*/ 1
    },
};

static SKERNEL_INTERRUPT_REG_INFO_STC txdma_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txDMA[0].interrupts.txd_interrupt_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txDMA[0].interrupts.txd_interrupt_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Txdma_Int_sum,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

static SKERNEL_INTERRUPT_REG_INFO_STC txfifo_interrupts[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txFIFO[0].interrupts.txf_interrupt_cause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(sip6_txFIFO[0].interrupts.txf_interrupt_mask)},
        /*myFatherInfo*/{
            /*myBitIndex*/cnm_grp_0_1_intr_Txfifo_Int_sum ,
            /*interruptPtr*/&cnm_grp_0_1_intr
            },
        /*isTriggeredByWrite*/ 1
    }
};

#define xsmi_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,mgIndex,father_myBitIndex,father_interrupt) \
{                                                                                              \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.mg_causeRegName),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_LEGACY_MAC(MG.mg_maskRegName ),0,mgIndex | MG_TREE_ID(SINGLE_TREE_ID) | MG_UNIT_USED_CNS}, \
        /*myFatherInfo*/{                                                                      \
            /*myBitIndex*/  father_myBitIndex ,                                                \
            /*interruptPtr*/&father_interrupt                                                  \
            },                                                                                 \
        /*isTriggeredByWrite*/ 1                                                               \
}

/* duplicate the node of MG XSMI to ALL MG_x_x */
#define DUP_ALL_XSMI_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName,father_myBitIndex,father_interrupt) \
     xsmi_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_0_INDEX,father_myBitIndex,father_interrupt[MG_0_INDEX]) \
    ,xsmi_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_1_INDEX,father_myBitIndex,father_interrupt[MG_1_INDEX]) \
    ,xsmi_INTERTTUPT_REG(mg_causeRegName,mg_maskRegName ,MG_2_INDEX,father_myBitIndex,father_interrupt[MG_2_INDEX])

static SKERNEL_INTERRUPT_REG_INFO_STC xsmi_interrupts[NUM_OF_MGS] =
{    DUP_ALL_XSMI_INTERTTUPT_REG(XSMI.XSMIInterruptCause ,XSMI.XSMIInterruptMask ,xsmiIntSum ,ironman_GlobalInterruptsSummary)};

static SKERNEL_INTERRUPT_REG_INFO_STC  ironmanTsuInterruptSummary =
{                                                  \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].ctsuGlobalInterruptSummaryCause)},
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].ctsuGlobalInterrupSummarytMask)},
    /*myFatherInfo*/{
        /*myBitIndex*/ cnm_grp_0_1_intr_Tsu_Int_sum,
        /*interruptPtr*/&cnm_grp_0_1_intr
        },
    /*isTriggeredByWrite*/ 1
};

static SKERNEL_INTERRUPT_REG_INFO_STC ironmanTsuChannelInterruptSummary[] =
{
    {
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_0_15_InterruptSummaryCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_0_15_InterruptSummaryMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/2 ,
            /*interruptPtr*/&ironmanTsuInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_16_31_InterruptSummaryCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_16_31_InterruptSummaryMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/3 ,
            /*interruptPtr*/&ironmanTsuInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_32_47_InterruptSummaryCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_32_47_InterruptSummaryMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/4 ,
            /*interruptPtr*/&ironmanTsuInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
   ,{
        /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_48_63_InterruptSummaryCause)},
        /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channel_48_63_InterruptSummaryMask)},
        /*myFatherInfo*/{
            /*myBitIndex*/5 ,
            /*interruptPtr*/&ironmanTsuInterruptSummary
            },
        /*isTriggeredByWrite*/ 1
    }
};

/* node for TSU channel interrupt cause registers */
#define  GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(index)               \
{                                                         \
    /*causeReg*/ {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channelInterruptCause[(index)])}, /*SMEM_CHT_PORT_INT_CAUSE_REG*/          \
    /*maskReg*/  {SKERNEL_INTERRUPT_BIND_REG_SIP5_MAC(PCA_CTSU[0].channelInterruptMask[(index)])},  /*SMEM_CHT_PORT_INT_CAUSE_MASK_REG*/     \
    /*myFatherInfo*/{                                     \
        /*myBitIndex*/(((index)%16)+1),                                   \
        /*interruptPtr*/&ironmanTsuChannelInterruptSummary[(index/16)]      \
    },                                                    \
    /*isTriggeredByWrite*/ 1                            \
}

static SKERNEL_INTERRUPT_REG_INFO_STC pca_tsu_interrupts[] =
{
     GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(0 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(1 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(2 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(3 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(4 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(5 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(6 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(7 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(8 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(9 )
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(10)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(11)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(12)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(13)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(14)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(15)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(16)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(17)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(18)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(19)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(20)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(21)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(22)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(23)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(24)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(25)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(26)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(27)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(28)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(29)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(30)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(31)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(32)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(33)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(34)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(35)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(36)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(37)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(38)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(39)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(40)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(41)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(42)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(43)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(44)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(45)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(46)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(47)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(48)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(49)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(50)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(51)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(52)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(53)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(54)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(55)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(56)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(57)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(58)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(59)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(60)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(61)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(62)
    ,GROUP_CHANNEL_INTERRUPT_CAUSE_MAC(63)
};


/* get the MG index from the currMgInfoPtr->causeReg.mgUnitIndex */
#define   GET_MG_INDEX(currMgInfoPtr)    (currMgInfoPtr->causeReg.mgUnitIndex & 0xFF)


/**
* @internal internal_smemIronmanInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the Ironman device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static GT_VOID internal_smemIronmanInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT         * devObjPtr,
    INOUT    GT_U32                     * indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    GT_U32 index = *indexPtr;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = *currDbPtrPtr;
    GT_U32  mgMax;
    GT_U32  mgIndex;

    {/* MG unit related interrupts */
        /*smemBobcat2GlobalInterruptTreeInit(devObjPtr, &index, &currDbPtr);*/

        /* for MG0 of the tile */
        mgIndex = 0;
        mgMax = NUM_OF_MGS;
        /* (skip MG[0]) add support for MG : 1,2 */
        /*mgIndex++;*//* 1 */

        for(/* continue*/; mgIndex < mgMax ; mgIndex++)
        {

            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_FuncUnitsInterruptsSummary[mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_dataPathInterruptSummary[mgIndex]);

            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_FuncUnits1InterruptsSummary[mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_portsInterruptsSummary[mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_dfxInterruptsSummary[mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_dfx1InterruptsSummary[mgIndex]);

            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg1_internal_interrupts[mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &xsmi_interrupts[mgIndex]);

            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_GlobalInterruptsSummary  [mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_misc_interrupts[mgIndex]         );
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_sdma_receiveSDMA_interrupts [mgIndex]);
            INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, &currDbPtr, &ironman_mg_global_sdma_transmitSDMA_interrupts[mgIndex]);
        }
    }

    {   /* per pipe / tile interrupts */
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tti_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, bridge_interrupts                 );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, preq_interrupts                   );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mac_tbl_interrupts                );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ha_interrupts                     );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ermrk_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, pcl_unit_interrupts               );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, tcam_interrupts                   );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cpu_code_rate_limiters_interrupts );
/*        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, preEgrInterruptSummary            );*/
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, egf_eft_global_interrupts         );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, mll_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, policer_ipfix_interrupts          );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, cnc_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, oam_interrupts                    );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, rxdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txdma_interrupts                  );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, txfifo_interrupts                 );
        INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, pca_tsu_interrupts                );
    }

    *indexPtr = index;
    *currDbPtrPtr = currDbPtr;
}

/*******************************************************************************
*   smemIronmanInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Ironman device
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
GT_VOID smemIronmanInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 index = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr;

    /* add here additions to 'compilation' info , that since 2 arrays point to
       each other we need to set it in runtime */

    /* Initialize Ports2 father Summary interrupts for primary MG units in all tiles */
    ironman_FuncUnits1InterruptsSummary[MG_0_INDEX].myFatherInfo.interruptPtr = &ironman_GlobalInterruptsSummary[MG_0_INDEX];

    devObjPtr->myInterruptsDbPtr = ironmanInterruptsTreeDb;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    internal_smemIronmanInterruptTreeInit(devObjPtr,&index,&currDbPtr);

    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, ironman_mac_ports_interrupts);

    /* End of interrupts - must be last */
    INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, &currDbPtr, skernel_interrupt_reg_info__last_line__cns);
}

void print_ironmanInterruptsTreeDb_mgInterrupts(void)
{
    GT_U32  ii=0;
    SKERNEL_INTERRUPT_REG_INFO_STC * currDbPtr = ironmanInterruptsTreeDb;
    GT_U32  mgUnitIndex;

    while(currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        if(0 == strncmp(currDbPtr->causeReg.registerName,"MG.",3))
        {
            mgUnitIndex = GET_MG_INDEX(currDbPtr);

            printf("index [%3.3d] in MG[%d] hold [%s] \n",
                ii,
                mgUnitIndex,
                currDbPtr->causeReg.registerName);
        }

        ii ++;
        currDbPtr++;
    }

    printf("last index [%d] \n",
        ii-1);

}

