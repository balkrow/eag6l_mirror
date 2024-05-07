/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */

/**
 * @brief  functions for set/get  common for all platforms registers
 *
* @file set_hw_registers.c
*
* $Revision: 2.0 $
*/

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrors.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>


/*Last node is always null node*/

#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC (prvCpssSchedGetAnodesNullNode(TM_ENV(ctl)))
#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_B_NODE_MAC (prvCpssSchedGetBnodesNullNode(TM_ENV(ctl)))
#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC (prvCpssSchedGetQnodesNullNode(TM_ENV(ctl)))



/**
 */
int prvSchedLowLevelLKNGet
(
    PRV_CPSS_SCHED_HANDLE hndl,
     enum schedLevel lvl,
     uint32_t *Lptr,
     uint32_t *Kptr,
     uint32_t *Nptr
)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(CPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerRateShap_Addr)


    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    switch(lvl)
    {
        case SCHED_B_LEVEL:
            TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerRateShap_Addr, BPerRateShap_Addr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerRateShap_Addr, L ,*Lptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerRateShap_Addr, K ,*Kptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerRateShap_Addr, N ,*Nptr, (uint16_t))
            break;
        case SCHED_C_LEVEL:
            TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.CPerRateShap_Addr, CPerRateShap_Addr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerRateShap_Addr, L ,*Lptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerRateShap_Addr, K ,*Kptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerRateShap_Addr, N ,*Nptr, (uint16_t))
            break;
        case SCHED_A_LEVEL:
            TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerRateShap_Addr, APerRateShap_Addr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerRateShap_Addr, L ,*Lptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerRateShap_Addr, K ,*Kptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerRateShap_Addr, N ,*Nptr, (uint16_t))
            break;
        case SCHED_Q_LEVEL:
            TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerRateShap_Addr, QPerRateShap_Addr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerRateShap_Addr, L ,*Lptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerRateShap_Addr, K ,*Kptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerRateShap_Addr, N ,*Nptr, (uint16_t))
            break;
        case SCHED_P_LEVEL:
            TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerRateShap_Addr, PPerRateShap_Addr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerRateShap_Addr, L ,*Lptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerRateShap_Addr, K ,*Kptr, (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerRateShap_Addr, N ,*Nptr, (uint16_t))
            break;
        default:
            break;
    }

out:

    return rc;
}



/**
 */
int prvSchedLowLevelSetHwPeriodicScheme(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
#if 0
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ScrubSlots_Addr)
#endif
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(CPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerRateShap_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerRateShap_Addr)


    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
    {
        rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,SCHED_P_LEVEL);
        if (rc)
            goto out;

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET( PPerRateShap_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerRateShap_Addr , L , ctl->level_data[SCHED_P_LEVEL].L)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerRateShap_Addr , K , ctl->level_data[SCHED_P_LEVEL].K)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerRateShap_Addr , N , ctl->level_data[SCHED_P_LEVEL].N)
        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PPerRateShap_Addr, PPerRateShap_Addr)
        if (rc) goto out;

        rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,SCHED_C_LEVEL);
        if (rc) goto out;

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(CPerRateShap_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerRateShap_Addr , L , ctl->level_data[SCHED_C_LEVEL].L)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerRateShap_Addr , K , ctl->level_data[SCHED_C_LEVEL].K)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerRateShap_Addr , N , ctl->level_data[SCHED_C_LEVEL].N)
        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.CPerRateShap_Addr, CPerRateShap_Addr)
        if (rc) goto out;

    }

    rc = prvSchedLowLevelSetPerLevelShapingParameters(hndl,SCHED_B_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerRateShap_Addr , L , ctl->level_data[SCHED_B_LEVEL].L)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerRateShap_Addr , K , ctl->level_data[SCHED_B_LEVEL].K)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerRateShap_Addr , N , ctl->level_data[SCHED_B_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerRateShap_Addr, BPerRateShap_Addr)
        if (rc) goto out;


    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,SCHED_A_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(APerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerRateShap_Addr , L , ctl->level_data[SCHED_A_LEVEL].L)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerRateShap_Addr , K , ctl->level_data[SCHED_A_LEVEL].K)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerRateShap_Addr , N , ctl->level_data[SCHED_A_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerRateShap_Addr, APerRateShap_Addr)
    if (rc) goto out;

    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,SCHED_Q_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerRateShap_Addr , L , ctl->level_data[SCHED_Q_LEVEL].L)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerRateShap_Addr , K , ctl->level_data[SCHED_Q_LEVEL].K)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerRateShap_Addr , N , ctl->level_data[SCHED_Q_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerRateShap_Addr, QPerRateShap_Addr)
    if (rc) goto out;
#if 0

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(addressSpacePtr->Sched.ScrubSlots_Addr, ScrubSlots_Addr)
    if (rc) goto out;
    /* assign register fields */
    /* write register */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ScrubSlots_Addr , PortSlots , ctl->level_data[SCHED_P_LEVEL].scrub_slots)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ScrubSlots_Addr , ClvlSlots , ctl->level_data[SCHED_C_LEVEL].scrub_slots)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ScrubSlots_Addr , BlvlSlots , ctl->level_data[SCHED_B_LEVEL].scrub_slots)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ScrubSlots_Addr , AlvlSlots , ctl->level_data[SCHED_A_LEVEL].scrub_slots)
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ScrubSlots_Addr , QueueSlots ,ctl->level_data[Q_LEVEL].scrub_slots)
    TXQ_SCHED_WRITE_REGISTER(addressSpacePtr->Sched.ScrubSlots_Addr, ScrubSlots_Addr)
    if (rc) goto out;
#endif
out:

    return rc;
}


/**
 */
int prvSchedLowLevelSetHwNodeMapping(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel lvl, uint32_t index)
{
    int rc =  -EFAULT;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortRangeMap)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvltoPortAndBlvlRangeMap)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BLvltoClvlAndAlvlRangeMap)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ALvltoBlvlAndQueueRangeMap)

    TM_REGISTER_VAR(TM_Sched_QueueAMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_Queuemapping)



    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;


    switch (lvl)
    {
    case SCHED_P_LEVEL:
          if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
          {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(PortRangeMap)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortRangeMap , Lo , ctl->tm_port_array[index].mapping.childLo)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortRangeMap, Hi , ctl->tm_port_array[index].mapping.childHi)
           }
          else
          {
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap , index , PortRangeMap)
             if (rc) goto out;

              /* assign register fields */
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortRangeMap, clow , ctl->tm_port_array[index].mapping.childLo)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortRangeMap ,chigh , ctl->tm_port_array[index].mapping.childHi)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortRangeMap ,pdwrr_en , ctl->tm_port_array[index].dwrr_enable_per_priority_mask)
          }


           /* write register */
           TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap, index ,PortRangeMap)

        break;
    case SCHED_C_LEVEL:
         if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
         {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ClvltoPortAndBlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , Port , ctl->tm_c_node_array[index].mapping.nodeParent)

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , BlvlLo , ctl->tm_c_node_array[index].mapping.childLo)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , BlvlHi , ctl->tm_c_node_array[index].mapping.childHi)
          }
          else
          {
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , index , ClvltoPortAndBlvlRangeMap)
             if (rc) goto out;

              /* assign register fields */
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , port , ctl->tm_c_node_array[index].mapping.nodeParent)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap, blow , ctl->tm_c_node_array[index].mapping.childLo)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , bhigh , ctl->tm_c_node_array[index].mapping.childHi)
          }
            /* write register */
          TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , index , ClvltoPortAndBlvlRangeMap)

        break;
    case SCHED_B_LEVEL:
          if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
          {
        /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BLvltoClvlAndAlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , Clvl , ctl->tm_b_node_array[index].mapping.nodeParent)

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , AlvlLo , ctl->tm_b_node_array[index].mapping.childLo)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , AlvlHi , ctl->tm_b_node_array[index].mapping.childHi)
           }
          else
          {
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , index , BLvltoClvlAndAlvlRangeMap)
             if (rc) goto out;

              /* assign register fields */
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , clvl , ctl->tm_b_node_array[index].mapping.nodeParent)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap, alow , ctl->tm_b_node_array[index].mapping.childLo)
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , ahigh , ctl->tm_b_node_array[index].mapping.childHi)
          }

            /* write register */
           TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , index , BLvltoClvlAndAlvlRangeMap)

        break;
    case SCHED_A_LEVEL:
            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ALvltoBlvlAndQueueRangeMap)
                if (rc) goto out;

                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , Blvl , ctl->tm_a_node_array[index].mapping.nodeParent)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap, QueueLo , ctl->tm_a_node_array[index].mapping.childLo)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , QueueHi , ctl->tm_a_node_array[index].mapping.childHi)
             }
             else
             {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , index , ALvltoBlvlAndQueueRangeMap)
                if (rc) goto out;

                 /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , Blvl , ctl->tm_a_node_array[index].mapping.nodeParent)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap, qlow , ctl->tm_a_node_array[index].mapping.childLo)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , qhigh , ctl->tm_a_node_array[index].mapping.childHi)
             }

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , index , ALvltoBlvlAndQueueRangeMap)

        break;
    case SCHED_Q_LEVEL:
            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_QueueAMap)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_QueueAMap , Alvl , ctl->tm_queue_array[index].mapping.nodeParent)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),\
                    addressSpacePtr->Sched.QueueAMap , index , TM_Sched_QueueAMap)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , index , Queuemapping)

                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Queuemapping , alvl , ctl->tm_queue_array[index].mapping.nodeParent)

                /*write*/
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , index , Queuemapping)
            }

        break;
    }
out:

    return rc;
}




int prvSchedLowLevelSetQueueQuantum(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_ind)
{
    int rc =  -EFAULT;

    struct tm_queue *queue = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

   TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueQuantum)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (queue_ind < ctl->tm_total_queues)
    {
        queue = &(ctl->tm_queue_array[queue_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QueueQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueQuantum , Quantum , queue->dwrr_quantum)
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueQuantum , queue_ind ,QueueQuantum)
        /* write register */
        if (rc) goto out;


    }
out:

    return rc;
}


int prvSchedLowLeveGetQueueParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct queue_hw_data_t *queue_hw_data)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


    TM_REGISTER_VAR(TM_Sched_QueueAMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_Queuemapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueQuantum)
    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)


    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)return rc;

    if (index < ctl->tm_total_queues)
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
           TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl), \
                   addressSpacePtr->Sched.QueueAMap , index, TM_Sched_QueueAMap)
           if (rc) return rc;
           TXQ_SCHED_REGISTER_GET(TM_Sched_QueueAMap , Alvl , queue_hw_data->parent_a_node,(uint16_t))

           TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl), \
               addressSpacePtr->Sched.QueueEligPrioFuncPtr , index , TM_Sched_QueueEligPrioFuncPtr)
           if (rc) return rc;
           TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFuncPtr , Ptr , queue_hw_data->elig_prio_func_ptr, (uint8_t))
        }
        else
        {
           TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                   addressSpacePtr->Sched.Queuemapping , index , Queuemapping)

           TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Queuemapping , alvl , queue_hw_data->parent_a_node,(uint16_t))
           TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Queuemapping , Ptr , queue_hw_data->elig_prio_func_ptr,(uint8_t))
        }
        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueQuantum, index, QueueQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueQuantum, Quantum, queue_hw_data->dwrr_quantum, (uint16_t));

    }

    return rc;
}


int prvSchedLowLevelAnodeDwrrSet(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind,uint8_t dwrr_enable_per_priority_mask)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)
    TM_REGISTER_VAR(PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_7_ALvltoBlvlAndQueueRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        node->dwrr_enable_per_priority_mask = dwrr_enable_per_priority_mask;
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_AlvlDWRRPrioEn)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),
                addressSpacePtr->Sched.AlvlDWRRPrioEn , node_ind , TM_Sched_AlvlDWRRPrioEn)
            /* write register */
            if (rc) goto out;
       }
       else
       {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_ind , ALvltoBlvlAndQueueRangeMap)

            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ALvltoBlvlAndQueueRangeMap , adwrr_en , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_ind , ALvltoBlvlAndQueueRangeMap)
            /* write register */
            if (rc) goto out;
       }
    }

out:

    return rc;
}


int prvSchedLowLevelAnodeDwrrGet(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind,uint8_t * dwrr_enable_per_priority_mask)
{
    int rc =  -EFAULT;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)
    TM_REGISTER_VAR(PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (node_ind < ctl->tm_total_a_nodes)
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {

            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),
                addressSpacePtr->Sched.AlvlDWRRPrioEn, node_ind  ,TM_Sched_AlvlDWRRPrioEn)

            /* read register */
            if (rc) goto out;

            TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlDWRRPrioEn, En, *dwrr_enable_per_priority_mask, (uint8_t));
        }
        else
        {
             TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),
                addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_ind , PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap)

              /* read register */
             if (rc) goto out;

             TXQ_SCHED_REGISTER_GET(PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap , adwrr_en ,  *dwrr_enable_per_priority_mask, (uint8_t))
        }
    }

out:

    return rc;
}




int prvSchedLowLevelSet_a_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)
    TM_REGISTER_VAR(PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_7_ALvltoBlvlAndQueueRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(AlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlQuantum , node_ind , AlvlQuantum)
        if (rc) goto out;

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {

            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_AlvlDWRRPrioEn)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlDWRRPrioEn , node_ind , TM_Sched_AlvlDWRRPrioEn)
            /* write register */
            if (rc) goto out;
        }
        else
        {
            /*read*/
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
               addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_ind ,ALvltoBlvlAndQueueRangeMap)

            /*modify*/
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ALvltoBlvlAndQueueRangeMap , adwrr_en , node->dwrr_enable_per_priority_mask)

            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_ind ,ALvltoBlvlAndQueueRangeMap)
            /* write register */
            if (rc) goto out;
        }
    }


out:

    return rc;
}
int prvSchedLowLevelGetANodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct a_node_hw_data_t *a_node_hw_data)
{
    int rc = -EFAULT;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ALvltoBlvlAndQueueRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;

    if (index < ctl->tm_total_a_nodes)
    {
        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap, index, ALvltoBlvlAndQueueRangeMap);
        if (rc)
            return rc;
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, Blvl, a_node_hw_data->parent_b_node, (uint16_t));
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, QueueHi, a_node_hw_data->last_child_queue, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, QueueLo, a_node_hw_data->first_child_queue, (uint16_t));
        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, qhigh, a_node_hw_data->last_child_queue, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, qlow, a_node_hw_data->first_child_queue, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, afunc_ptr, a_node_hw_data->elig_prio_func_ptr, (uint8_t));
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),
                addressSpacePtr->Sched.AlvlEligPrioFuncPtr, index, TM_Sched_AlvlEligPrioFuncPtr);

            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFuncPtr, Ptr, a_node_hw_data->elig_prio_func_ptr, (uint8_t));
        }

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlQuantum, index, AlvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlQuantum, Quantum, a_node_hw_data->dwrr_quantum, (uint16_t));

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlDWRRPrioEn, index, TM_Sched_AlvlDWRRPrioEn);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlDWRRPrioEn, En, a_node_hw_data->dwrr_priority, (uint8_t));
        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALvltoBlvlAndQueueRangeMap, adwrr_en,a_node_hw_data->dwrr_priority, (uint8_t));
        }



    }
    return rc;
}





/**
 */
int prvSchedLowLevelSet_b_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;
    struct tm_b_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_BlvlDWRRPrioEn)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(BLvltoClvlAndAlvlRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlQuantum)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (node_ind < ctl->tm_total_b_nodes)
    {
        node = &(ctl->tm_b_node_array[node_ind]);

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {

            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_BlvlDWRRPrioEn)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlDWRRPrioEn , node_ind , TM_Sched_BlvlDWRRPrioEn)
            /* write register */
            if (rc) goto out;
        }
        else
        {
            /*read*/
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
               addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , node_ind , BLvltoClvlAndAlvlRangeMap)

            /*modify*/
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(BLvltoClvlAndAlvlRangeMap , bdwrr_en , node->dwrr_enable_per_priority_mask)

            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , node_ind , BLvltoClvlAndAlvlRangeMap)
            /* write register */
            if (rc) goto out;
        }

         /*Quantum only supported for full tree*/
        if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E !=
            PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
        {
            return 0;
        }

         /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlQuantum , node_ind , BlvlQuantum)
        if (rc) goto out;


    }
out:

    return rc;
}

int prvSchedLowLevelGetBNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct b_node_hw_data_t *b_node_hw_data)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BLvltoClvlAndAlvlRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_BlvlDWRRPrioEn)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)

     rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;


    if (index < ctl->tm_total_b_nodes)
    {
        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap, index, BLvltoClvlAndAlvlRangeMap);
        if (rc)
            return rc;
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, Clvl, b_node_hw_data->parent_c_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, AlvlHi, b_node_hw_data->last_child_a_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, AlvlLo, b_node_hw_data->first_child_a_node, (uint16_t));

            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlEligPrioFuncPtr, index, TM_Sched_BlvlEligPrioFuncPtr);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFuncPtr, Ptr, b_node_hw_data->elig_prio_func_ptr, (uint8_t));


            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlDWRRPrioEn, index, TM_Sched_BlvlDWRRPrioEn);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlDWRRPrioEn, En, b_node_hw_data->dwrr_priority, (uint8_t));

        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, clvl, b_node_hw_data->parent_c_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, ahigh, b_node_hw_data->last_child_a_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, alow, b_node_hw_data->first_child_a_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, bfunc_ptr, b_node_hw_data->elig_prio_func_ptr, (uint8_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BLvltoClvlAndAlvlRangeMap, bdwrr_en, b_node_hw_data->dwrr_priority, (uint8_t));
        }

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlQuantum, index, BlvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlQuantum, Quantum, b_node_hw_data->dwrr_quantum, (uint16_t));

    }
    return rc;
}





/**
 */
int prvSchedLowLevelSet_c_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    struct tm_c_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlQuantum)
    TM_REGISTER_VAR(TM_Sched_ClvlDWRRPrioEn)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(ClvltoPortAndBlvlRangeMap)

    TM_CTL(ctl, hndl)
    /*only supported for full tree*/
    if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E !=
        PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
    {
        return 0;
    }

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (node_ind < ctl->tm_total_c_nodes)
    {
        node = &(ctl->tm_c_node_array[node_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ClvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlQuantum , node_ind , ClvlQuantum)
        if (rc) goto out;

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {

            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_ClvlDWRRPrioEn)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlDWRRPrioEn , node_ind , TM_Sched_ClvlDWRRPrioEn)
            /* write register */
            if (rc) goto out;
        }
        else
        {
            /*read*/
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
               addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , node_ind ,ClvltoPortAndBlvlRangeMap)

            /*modify*/
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ClvltoPortAndBlvlRangeMap , cdwrr_en , node->dwrr_enable_per_priority_mask)

            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , node_ind ,ClvltoPortAndBlvlRangeMap)
            /* write register */
            if (rc) goto out;
        }
    }


out:

    return rc;
}


int prvSchedLowLevelGetCNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct c_node_hw_data_t *c_node_hw_data)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvltoPortAndBlvlRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlQuantum)
    TM_REGISTER_VAR(TM_Sched_ClvlDWRRPrioEn)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;

    if (index < ctl->tm_total_c_nodes)
    {

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap, index, ClvltoPortAndBlvlRangeMap);
        if (rc)
            return rc;

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, Port, c_node_hw_data->parent_port, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, BlvlHi, c_node_hw_data->last_child_b_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, BlvlLo, c_node_hw_data->first_child_b_node, (uint16_t));

            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlEligPrioFuncPtr, index, TM_Sched_ClvlEligPrioFuncPtr);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFuncPtr, Ptr, c_node_hw_data->elig_prio_func_ptr, (uint8_t));


            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlDWRRPrioEn, index, TM_Sched_ClvlDWRRPrioEn);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlDWRRPrioEn, En, c_node_hw_data->dwrr_priority, (uint8_t));

        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, port, c_node_hw_data->parent_port, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, bhigh, c_node_hw_data->last_child_b_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, blow, c_node_hw_data->first_child_b_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, cfunc_ptr, c_node_hw_data->elig_prio_func_ptr, (uint8_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvltoPortAndBlvlRangeMap, cdwrr_en, c_node_hw_data->dwrr_priority, (uint8_t));
        }

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlQuantum, index, ClvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlQuantum, Quantum, c_node_hw_data->dwrr_quantum, (uint16_t));


    }
    return rc;

}


/**
 */

/**
 */
int prvSchedLowLevelSetPortScheduling(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    struct tm_port *port = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosHi)

    TM_CTL(ctl, hndl);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_TRUE)
    {
        /*Do nothing there is a pizza arbiter */
        return GT_OK;
    }

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);

        /* DWRR for Port */
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(PortQuantumsPriosLo)
        if (rc) goto out;

        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortQuantumsPriosLo , Quantum0 , port->dwrr_quantum_per_level[0].quantum)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortQuantumsPriosLo , Quantum1 , port->dwrr_quantum_per_level[1].quantum)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortQuantumsPriosLo , Quantum2 , port->dwrr_quantum_per_level[2].quantum)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PortQuantumsPriosLo , Quantum3 , port->dwrr_quantum_per_level[3].quantum)


        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortQuantumsPriosLo , port_ind , PortQuantumsPriosLo)
        if (rc) goto out;

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_RESET(TM_Sched_PortQuantumsPriosHi)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum4 , port->dwrr_quantum_per_level[4].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum5 , port->dwrr_quantum_per_level[5].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum6 , port->dwrr_quantum_per_level[6].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum7 , port->dwrr_quantum_per_level[7].quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortQuantumsPriosHi,
            port_ind ,TM_Sched_PortQuantumsPriosHi)
        if (rc) goto out;

    }
out:

    return rc;
}






/**
 */
int prvSchedLowLevelSetPortAllParameters(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    TM_CTL(ctl, hndl);
    if (port_ind < ctl->tm_total_ports)
    {
        rc = prvSchedLowLevelSetHwNodeMapping(hndl, SCHED_P_LEVEL, port_ind);
        if (rc < 0)
            goto out;

        rc = prvSchedLowLevelSetPortShapingParametersToHw(hndl, port_ind);
        if (rc < 0)
            goto out;

        rc = prvSchedLowLevelSetPortScheduling(hndl, port_ind);
        if (rc < 0)
            goto out;


        rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, SCHED_P_LEVEL, port_ind);
    }
out:
    /*   reset_hw_connection(ctl); already done  */
    return rc;
}

int prvSchedLowLevelGetPortParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct port_hw_data_t *port_hw_data)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosHi)
    TM_REGISTER_VAR(TM_Sched_PortDWRRPrioEn)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    /* No Drop Profile Ptr for Port */

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;

    if (index < ctl->tm_total_ports)
    {
        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap, index, PortRangeMap);
        if (rc)
            return rc;
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, Hi, port_hw_data->last_child_c_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, Lo, port_hw_data->first_child_c_node, (uint16_t));

            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortEligPrioFuncPtr, index, TM_Sched_PortEligPrioFuncPtr);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFuncPtr, Ptr, port_hw_data->elig_prio_func_ptr, (uint8_t));


            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDWRRPrioEn, index, TM_Sched_PortDWRRPrioEn);
            if (rc)
                return rc;

            TXQ_SCHED_REGISTER_GET(TM_Sched_PortDWRRPrioEn, En, port_hw_data->dwrr_priority, (uint8_t));
        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, chigh, port_hw_data->last_child_c_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, clow, port_hw_data->first_child_c_node, (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, pfunc_ptr, port_hw_data->elig_prio_func_ptr, (uint8_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortRangeMap, pdwrr_en, port_hw_data->dwrr_priority, (uint8_t));
        }

        /* DWRR for Port */
        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortQuantumsPriosLo, index, PortQuantumsPriosLo);
        if (rc)
            return rc;
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortQuantumsPriosLo, Quantum0, port_hw_data->dwrr_quantum[0], (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortQuantumsPriosLo, Quantum1, port_hw_data->dwrr_quantum[1], (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortQuantumsPriosLo, Quantum2, port_hw_data->dwrr_quantum[2], (uint16_t));
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortQuantumsPriosLo, Quantum3, port_hw_data->dwrr_quantum[3], (uint16_t));
        }
        else
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortQuantumsPriosLo, Quantum, port_hw_data->dwrr_quantum[0], (uint16_t));
        }

            /*not present at Ironman*/
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortQuantumsPriosHi,
                index ,TM_Sched_PortQuantumsPriosHi);
            if (rc)
                return rc;
            TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum4, port_hw_data->dwrr_quantum[4], (uint16_t));
            TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum5, port_hw_data->dwrr_quantum[5], (uint16_t));
            TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum6, port_hw_data->dwrr_quantum[6], (uint16_t));
            TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum7, port_hw_data->dwrr_quantum[7], (uint16_t));
         }


        /* No Drop Profile Ptr for Port */
    }
    return rc;
}



/**
 */
int prvSchedLowLevelSetTreeDequeueStatusToHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(TreeDeqEn_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(TreeDeqEn_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(TreeDeqEn_Addr , En , ctl->tree_deq_status)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.TreeDeqEn_Addr, TreeDeqEn_Addr )
    if (rc) goto out;
out:

    return rc;
}

int prvSchedLowLevelGetTreeDequeueStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(TreeDeqEn_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.TreeDeqEn_Addr, TreeDeqEn_Addr)
    if (rc)  goto out;

    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(TreeDeqEn_Addr, En , ctl->tree_deq_status, (uint8_t) ) /* no casting */
out:

    return rc;
}

/**
 */
int prvSchedLowLevelSetTreeDwrrPriorityStatusToHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PDWRREnReg)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(PDWRREnReg)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PDWRREnReg , PrioEn , ctl->tree_dwrr_enable_per_priority_mask)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PDWRREnReg, PDWRREnReg)
    if (rc) goto out;
out:

    return rc;
}

int prvSchedLowLevelGetTreeDwrrPriorityStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl)

{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PDWRREnReg)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PDWRREnReg, PDWRREnReg)
    if (rc) goto out;

    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PDWRREnReg , PrioEn , ctl->tree_dwrr_enable_per_priority_mask , (uint8_t) )
 out:

    return rc;
}


/**
 */








/*
    function  prvSchedLowLevelSetNodeShapingParametersToHw(hndl,level,node_ind, profile); is platform dependent - in file ...platform.c
*/

#define PROFILE_VALIDATION(nodes,nodes_count)   \
        NODE_VALIDATION(nodes_count)\
        if (rc==0)\
        {\
            sh_profile_index=ctl->nodes[node_ind].shaping_profile_ref;\
            if (sh_profile_index==SCHED_INF_SHP_PROFILE)\
                profile = &(ctl->infiniteShapingProfile);\
            else\
                rc= -EFAULT;\
        }

int prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind)
{
    int rc = 0;
    uint32_t    sh_profile_index;
    struct prvCpssDxChTxqSchedShapingProfile *profile = NULL;

    TM_CTL(ctl, hndl)

    /* get shaper parameters from attached shaping profile */
    switch (level)
    {
    case SCHED_Q_LEVEL:
        PROFILE_VALIDATION(tm_queue_array,ctl->tm_total_queues)
        if (rc) goto out;
        break;
    case SCHED_A_LEVEL:
        PROFILE_VALIDATION(tm_a_node_array,ctl->tm_total_a_nodes)
        if (rc) goto out;
        break;
    case SCHED_B_LEVEL:
        PROFILE_VALIDATION(tm_b_node_array,ctl->tm_total_b_nodes)
        if (rc) goto out;
        break;
    case SCHED_C_LEVEL:
        PROFILE_VALIDATION(tm_c_node_array,ctl->tm_total_c_nodes)
        if (rc) goto out;
        break;
    default:
        rc =  -EFAULT;
        goto out;
        break;
    }
    rc = prvSchedLowLevelSetNodeShapingParametersToHw(hndl,level,node_ind, profile);
out:

    return rc;
}

/**
 */
int prvSchedLowLevelSetNodeEligPriorityFuncEx(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index,uint8_t elig_prio_func_ptr)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)
    TM_REGISTER_VAR(PDQ_SIP_6_30_PortRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_7_PortRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_ClvltoPortAndBlvlRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_7_ClvltoPortAndBlvlRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_BLvltoClvlAndAlvlRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_7_BLvltoClvlAndAlvlRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_ALvltoBlvlAndQueueRangeMap)
    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  return rc;

    switch (node_level)
    {
    case SCHED_P_LEVEL:
        if (node_index < ctl->tm_total_ports)
        {
            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_PortEligPrioFuncPtr)
                if (rc) break;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortEligPrioFuncPtr , node_index , TM_Sched_PortEligPrioFuncPtr)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortRangeMap,
                    node_index, PortRangeMap);
                if (rc)
                {
                    return rc;
                }
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortRangeMap , pfunc_ptr , elig_prio_func_ptr)

                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortRangeMap , node_index ,
                    PortRangeMap)
                if (rc)
                {
                    return rc;
                }
            }

        }
        break;
    case SCHED_C_LEVEL:
        if (node_index < ctl->tm_total_c_nodes)
        {

            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_ClvlEligPrioFuncPtr)
                if (rc) break;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlEligPrioFuncPtr , node_index , TM_Sched_ClvlEligPrioFuncPtr)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                    addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap, node_index, ClvltoPortAndBlvlRangeMap);
                if (rc)
                {
                    return rc;
                }
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ClvltoPortAndBlvlRangeMap , cfunc_ptr , elig_prio_func_ptr)

                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , node_index ,ClvltoPortAndBlvlRangeMap)
                if (rc)
                {
                    return rc;
                }
            }
        }
        break;
    case SCHED_B_LEVEL:
        if (node_index < ctl->tm_total_b_nodes)
        {

            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_BlvlEligPrioFuncPtr)
                if (rc) break;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlEligPrioFuncPtr , node_index , TM_Sched_BlvlEligPrioFuncPtr)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                    addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap, node_index, BLvltoClvlAndAlvlRangeMap);
                if (rc)
                {
                    return rc;
                }
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(BLvltoClvlAndAlvlRangeMap , bfunc_ptr , elig_prio_func_ptr)

                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , node_index , BLvltoClvlAndAlvlRangeMap)
                if (rc)
                {
                    return rc;
                }
            }
        }
        break;
    case SCHED_A_LEVEL:
        if (node_index < ctl->tm_total_a_nodes)
        {
            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_AlvlEligPrioFuncPtr)
                if (rc) break;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlEligPrioFuncPtr , node_index , TM_Sched_AlvlEligPrioFuncPtr)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                    addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap, node_index, ALvltoBlvlAndQueueRangeMap);
                if (rc)
                {
                    return rc;
                }
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ALvltoBlvlAndQueueRangeMap , afunc_ptr , elig_prio_func_ptr)

                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , node_index , ALvltoBlvlAndQueueRangeMap)
                if (rc)
                {
                    return rc;
                }
            }
        }
        break;
    case SCHED_Q_LEVEL:
        if (node_index < ctl->tm_total_queues)
        {
            if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_RESET(TM_Sched_QueueEligPrioFuncPtr)
                if (rc) break;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.QueueEligPrioFuncPtr , node_index , TM_Sched_QueueEligPrioFuncPtr)
            }
            else
            {
                /*read*/
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                    addressSpacePtr->Sched.Queuemapping, node_index, Queuemapping);
                if (rc)
                {
                    return rc;
                }
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Queuemapping , Ptr , elig_prio_func_ptr)

                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , node_index , Queuemapping)
                if (rc)
                {
                    return rc;
                }
            }
        }
        break;
    }

    return rc;
}

/**
 */
int prvSchedLowLevelSetNodeEligPriorityFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index)
{
    int rc =  -EFAULT;

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
        case SCHED_P_LEVEL:
        {
            ctl->tm_port_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_port_array[node_index].elig_prio_func);
        }
        case SCHED_C_LEVEL:
        {
            ctl->tm_c_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_c_node_array[node_index].elig_prio_func);
        }
        case SCHED_B_LEVEL:
        {
            ctl->tm_b_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_b_node_array[node_index].elig_prio_func);
        }
        case SCHED_A_LEVEL:
        {
            ctl->tm_a_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_a_node_array[node_index].elig_prio_func);
        }
        case SCHED_Q_LEVEL:
        {
            ctl->tm_queue_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_queue_array[node_index].elig_prio_func);
        }
        default: break;
    }

    return rc;
}

int prvSchedLowLevelDisableNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index)
{
    int rc =  -EFAULT;

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
        case SCHED_P_LEVEL:
        {
            ctl->tm_port_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case SCHED_C_LEVEL:
        {
             ctl->tm_c_node_array[node_index].node_temporary_disabled = 1;
             return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case SCHED_B_LEVEL:
        {
            ctl->tm_b_node_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case SCHED_A_LEVEL:
        {
            ctl->tm_a_node_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelDisableAlevelEligPrioFunc(hndl,node_index, 1);
        }
        case SCHED_Q_LEVEL:
        {
            ctl->tm_queue_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        default: break;
    }

    return rc;
}

/**
 */
int prvSchedLowLevelDisablePortEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Disable Ports */
    for (i = 0; i < (int)total_ports; i++)
    {
        if ((PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE))
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_PortEligPrioFuncPtr)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFuncPtr , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
             /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortEligPrioFuncPtr ,
            i , TM_Sched_PortEligPrioFuncPtr)
            if (rc) goto out;
        }
        else
        {


             /*read*/
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortRangeMap, i,PortRangeMap);
             if (rc)
             {
                 return rc;
             }

             if(PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
             {
                /*  Since we do not have room for PRV_SCHED_ELIG_DEQ_DISABLE ,set the elig function to shaping .
                    Use 0 token in order to disable
                */

                /* Shaping with 0 bw */
                  /* reset register reserved fields */
                TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MinToken ,0)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MaxToken ,0)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv , i ,PortTokenBucketTokenEnDiv)
                if (rc) goto out;
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortRangeMap , pfunc_ptr , PRV_SCHED_ELIG_N_MIN_SHP_FP1)
             }
             else
             {
                  /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortRangeMap , pfunc_ptr , PRV_SCHED_ELIG_DEQ_DISABLE)
             }

             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap , i , PortRangeMap)
             if (rc)
             {
                 return rc;
             }
        }

    }
out:

    return rc;
}


int prvSchedLowLevelSetDefaultPortMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Default map p[i] to c[i]*/
    for (i = 0; i < (int)total_ports; i++)
    {

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_PortRangeMap)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap , Lo , i)
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap, Hi , i)
        }
        else
        {
            /*read*/
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap, i,PortRangeMap);
            if (rc)
            {
                return rc;
            }
            /*modify*/
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortRangeMap , clow , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortRangeMap , chigh , i)

        }

          /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortRangeMap , i , PortRangeMap)
        if (rc)
        {
            return rc;
        }

    }
out:

    return rc;
}


int prvSchedLowLevelSetDefaultCnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvltoPortAndBlvlRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Default map c[i] to b[i] , parent p[i]*/
    for (i = 0; i < (int)total_c_nodes; i++)
    {
        /* assign register fields */
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
             /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ClvltoPortAndBlvlRangeMap)
            if (rc) goto out;
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , Port , i)

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , BlvlLo , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , BlvlHi , i)
        }
        else
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap, i, ClvltoPortAndBlvlRangeMap);
            if (rc) goto out;
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , port , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , bhigh , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvltoPortAndBlvlRangeMap , blow , i)
        }
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , i , ClvltoPortAndBlvlRangeMap)

    }
out:

    return rc;
}


int prvSchedLowLevelSetDefaultBnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BLvltoClvlAndAlvlRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Default map b[i] to b[i] , parent p[i]*/
    for (i = 0; i < (int)total_b_nodes; i++)
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BLvltoClvlAndAlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , Clvl , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , AlvlLo , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , AlvlHi , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
        }
        else
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap, i, BLvltoClvlAndAlvlRangeMap);
            if (rc) goto out;
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , clvl , i)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , ahigh , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BLvltoClvlAndAlvlRangeMap , alow , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
        }

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , i , BLvltoClvlAndAlvlRangeMap)

    }
out:

    return rc;
}


int prvSchedLowLevelSetDefaultAnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t first_a_node,uint32_t total_a_nodes)
{
    int rc =  -EFAULT;
    uint32_t i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ALvltoBlvlAndQueueRangeMap)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Default map b[i] to b[i] , parent p[i]*/
    for (i = first_a_node; i < first_a_node+total_a_nodes; i++)
    {   if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
             /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET( ALvltoBlvlAndQueueRangeMap)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , Blvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_B_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap, QueueLo ,PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , QueueHi , PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)
        }
        else
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap, i, ALvltoBlvlAndQueueRangeMap);
            if (rc) goto out;
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , Blvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_B_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , qhigh , PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ALvltoBlvlAndQueueRangeMap , qlow , PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)
        }

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , i , ALvltoBlvlAndQueueRangeMap)


    }
out:

    return rc;
}




int prvSchedLowLevelSetDefaultQnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t qIndex)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    struct tm_queue     *queue;

    TM_REGISTER_VAR(TM_Sched_QueueAMap)
    TM_REGISTER_VAR(PDQ_SIP_6_30_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_Queuemapping)

    TM_CTL(ctl, hndl)

    /*this Q is used in SW*/
    if(qIndex<ctl->tm_total_queues)
    {
        queue=&(ctl->tm_queue_array[qIndex]);
        queue->mapping.nodeParent=(uint16_t)PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC;
    }

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
         /* reset register reserved fields */
         TXQ_SCHED_REGISTER_RESET(TM_Sched_QueueAMap)
         if (rc) goto out;
         /* assign register fields */
         TXQ_SCHED_REGISTER_SET(TM_Sched_QueueAMap , Alvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
         /* write register */
         TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.QueueAMap , qIndex , TM_Sched_QueueAMap)
     }
     else
     {
         TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , qIndex ,Queuemapping)

         TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Queuemapping , alvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)

         TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , qIndex ,Queuemapping)

     }


out:

    return rc;
}



/**
 *  Configure user Q level Eligible Priority Function
 */
int prvSchedLowLevelSetQLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    struct tm_elig_prio_func_queue *params;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueEligPrioFunc)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    params = &(ctl->tm_elig_prio_q_lvl_tbl[func_offset]);
    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QueueEligPrioFunc)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueEligPrioFunc , FuncOut0 ,  params->tbl_entry.func_out[0])
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueEligPrioFunc , FuncOut1 ,  params->tbl_entry.func_out[1])
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueEligPrioFunc , FuncOut2 ,  params->tbl_entry.func_out[2])
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueEligPrioFunc , FuncOut3 ,  params->tbl_entry.func_out[3])
    /* write register */
    TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueEligPrioFunc, func_offset, QueueEligPrioFunc)
    if (rc)
        goto out;

out:

    return rc;
}

/**
 *  Configure user Node level Eligible Priority Function
 */
int prvSchedLowLevelSetALevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    GT_U32 tableSize;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    TXQ_SCHED_VALIDATE_ELIG_FUNCTION(func_offset,tableSize);

    for (i = 0; i < 8; i++)
    {
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(AlvlEligPrioFunc_Entry)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlEligPrioFunc_Entry, func_offset + i*tableSize ,AlvlEligPrioFunc_Entry)
        /* write register */
        if (rc) goto out;
    }
out:

    return rc;
}

int prvSchedLowLevelSetBLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 tableSize;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    TXQ_SCHED_VALIDATE_ELIG_FUNCTION(func_offset,tableSize);

    for (i = 0; i < 8; i++)
    {
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BlvlEligPrioFunc_Entry);
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlEligPrioFunc_Entry, func_offset + i*tableSize ,BlvlEligPrioFunc_Entry)
        if (rc) goto out;
    }
out:

    return rc;
}


int prvSchedLowLevelSetCLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc = 0;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 tableSize;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlEligPrioFunc_Entry)
    TM_CTL(ctl, hndl)

    if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
    {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        TXQ_SCHED_VALIDATE_ELIG_FUNCTION(func_offset,tableSize);

        for (i = 0; i < 8; i++)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ClvlEligPrioFunc_Entry);
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlEligPrioFunc_Entry, func_offset + i*tableSize ,ClvlEligPrioFunc_Entry)
            if (rc) goto out;
        }
    }
out:

    return rc;
}

/*

SIP6:

FuncOut3[8] - Elig.
FuncOut3[7:5] - Scheduling Priority.
FuncOut3[4:2] - Propagated Priority.
FuncOut3[1] - Use Min Token Bucket.
FuncOut3[0] - Use Max Token Bucket.

SIP7:

FuncOut3[5] - Elig.
FuncOut3[4:2] - Propagated Priority.
FuncOut3[1] - Use Min Token Bucket.
FuncOut3[0] - Use Max Token Bucket.

*/

uint16_t prvSchedSip7PLevelEligPrioFuncEntryZip
(
    uint16_t func
)
{
    uint16_t tmp = func&0x1F;
    /*Scheduling Priority is missing at SIP7 P level*/
    uint16_t eligBit = (func&(1<<8))>>3;
    return tmp|eligBit;
}

uint16_t prvSchedSip7PLevelEligPrioFuncEntryUnzip
(
    uint16_t func
)
{
    uint16_t tmp = func&0x1F;
    /*Scheduling Priority is missing at SIP7 P level*/
    uint16_t eligBit = (func&(1<<5))<<3;
    return tmp|eligBit;
}

int prvSchedLowLevelSetPLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc = 0;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 tableSize;

    TM_REGISTER_VAR(PDQ_SIP_6_30_PortEligPrioFunc_Entry)
    TM_REGISTER_VAR(PDQ_SIP_7_PortEligPrioFunc_Entry)
    TM_CTL(ctl, hndl)

    if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_TRUE)
    {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        TXQ_SCHED_VALIDATE_ELIG_FUNCTION(func_offset,tableSize);

        for (i = 0; i < 8; i++)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_FROM_SIP_6_30(PortEligPrioFunc_Entry);
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortEligPrioFunc_Entry , FuncOut0 , prvSchedSip7PLevelEligPrioFuncEntryZip(ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[0]))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortEligPrioFunc_Entry , FuncOut1 , prvSchedSip7PLevelEligPrioFuncEntryZip(ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[1]))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortEligPrioFunc_Entry , FuncOut2 , prvSchedSip7PLevelEligPrioFuncEntryZip(ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[2]))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(PortEligPrioFunc_Entry , FuncOut3 , prvSchedSip7PLevelEligPrioFuncEntryZip(ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[3]))
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortEligPrioFunc_Entry, func_offset + i*tableSize ,PortEligPrioFunc_Entry)
            if (rc) goto out;
        }
    }
out:

    return rc;
}




int prvSchedLowLevelSetQLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    uint16_t j;
    int rc = 0;


    prvShedEligFuncQueue funcArr[] =
    {         /*no shaping*/
                       PRV_SCHED_ELIG_Q_PRIO0,PRV_SCHED_ELIG_Q_PRIO1,PRV_SCHED_ELIG_Q_PRIO2,
                       PRV_SCHED_ELIG_Q_PRIO3,PRV_SCHED_ELIG_Q_PRIO4,PRV_SCHED_ELIG_Q_PRIO5,
                       PRV_SCHED_ELIG_Q_PRIO6,PRV_SCHED_ELIG_Q_PRIO7,

             /*shaping with min BW TYPE0*/

                       PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING,PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0_WITH_SHAPING,PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0_WITH_SHAPING,PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0_WITH_SHAPING,PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0_WITH_SHAPING,

            /*min BW TYPE0*/
                       /**/
                       PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0,PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0,
                       PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0,PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0,
                       PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0,PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0,
                       PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0,PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0,

            /*shaping*/
                       PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0,PRV_SCHED_ELIG_Q_MIN_SHP_PRIO1,PRV_SCHED_ELIG_Q_MIN_SHP_PRIO2,
                       PRV_SCHED_ELIG_Q_MIN_SHP_PRIO3,PRV_SCHED_ELIG_Q_MIN_SHP_PRIO4,PRV_SCHED_ELIG_Q_MIN_SHP_PRIO5,
                       PRV_SCHED_ELIG_Q_MIN_SHP_PRIO6,PRV_SCHED_ELIG_Q_MIN_SHP_PRIO7,


            /*min BW TYPE1*/
                       PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1,PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1,
                       PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1,PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1,
                       PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1,PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1,
                       PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1,PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1,


           /*shaping with min BW TYPE1*/
                       PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING,PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1_WITH_SHAPING,PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1_WITH_SHAPING,PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1_WITH_SHAPING,
                       PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1_WITH_SHAPING,PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1_WITH_SHAPING,

                        /*disable*/
                       PRV_SCHED_ELIG_Q_DEQ_DIS_E
     };

    TM_CTL(ctl, hndl)


    for (j = 0; j <sizeof(funcArr)/sizeof(funcArr[0]);j++)
    {
        if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(PDQ_ENV_DEVNUM_NUM(ctl))->devFamily)
        {/*not present at Ironman . Only 16 functions.*/

            if(funcArr[j]==PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1)
            {
                /*Ironman does not support TYPE1 and disable*/
                break;
            }

        }
        rc = prvSchedLowLevelSetQLevelEligPrioFuncEntry(hndl, funcArr[j]);
        if (rc) break;
    }

    return rc;
}


/**
 */
int prvSchedLowLevelSetALevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    uint16_t j;

     enum prvShedEligFuncNode funcArr[] ={
           /*no shaping*/
                       PRV_SCHED_ELIG_N_FP0,PRV_SCHED_ELIG_N_FP1,PRV_SCHED_ELIG_N_FP2,
                       PRV_SCHED_ELIG_N_FP3,PRV_SCHED_ELIG_N_FP4,PRV_SCHED_ELIG_N_FP5,
                       PRV_SCHED_ELIG_N_FP6,PRV_SCHED_ELIG_N_FP7,

            /*shaping*/
                       PRV_SCHED_ELIG_N_MIN_SHP_FP0,PRV_SCHED_ELIG_N_MIN_SHP_FP1,PRV_SCHED_ELIG_N_MIN_SHP_FP2,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP3,PRV_SCHED_ELIG_N_MIN_SHP_FP4,PRV_SCHED_ELIG_N_MIN_SHP_FP5,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP6,PRV_SCHED_ELIG_N_MIN_SHP_FP7,
            /*disable*/
           PRV_SCHED_ELIG_DEQ_DISABLE
     };

    TM_CTL(ctl, hndl)

    for (j = 0; j <sizeof(funcArr)/sizeof(funcArr[0]);j++)
    {
        if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(PDQ_ENV_DEVNUM_NUM(ctl))->devFamily)
        {
            if((enum prvShedEligFuncNode)PRV_SCHED_ELIG_DEQ_DISABLE == funcArr[j])
            {
                continue;
            }
        }
        rc = prvSchedLowLevelSetALevelEligPrioFuncEntry(hndl, funcArr[j]);
        if (rc) break;
    }

    return rc;
}


/**
 */
int prvSchedLowLevelSetBLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    uint16_t j;

    enum prvShedEligFuncNode funcArrSip6[] ={
                                      /*no shaping*/PRV_SCHED_ELIG_N_FP1,
                                      /*shaping*/   PRV_SCHED_ELIG_N_MIN_SHP_FP1,

                                        /*disable*/
                                        PRV_SCHED_ELIG_DEQ_DISABLE
                                      };

    enum prvShedEligFuncNode funcArrSip7[] ={
           /*no shaping*/
                       PRV_SCHED_ELIG_N_FP0,PRV_SCHED_ELIG_N_FP2,
                       PRV_SCHED_ELIG_N_FP3,PRV_SCHED_ELIG_N_FP4,PRV_SCHED_ELIG_N_FP5,
                       PRV_SCHED_ELIG_N_FP6,PRV_SCHED_ELIG_N_FP7,

            /*shaping*/
                       PRV_SCHED_ELIG_N_MIN_SHP_FP0,PRV_SCHED_ELIG_N_MIN_SHP_FP2,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP3,PRV_SCHED_ELIG_N_MIN_SHP_FP4,PRV_SCHED_ELIG_N_MIN_SHP_FP5,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP6,PRV_SCHED_ELIG_N_MIN_SHP_FP7
     };

    TM_CTL(ctl, hndl)



    for (j = 0; j <sizeof(funcArrSip6)/sizeof(funcArrSip6[0]);j++)
    {
        if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(PDQ_ENV_DEVNUM_NUM(ctl))->devFamily)
        {
            if((enum prvShedEligFuncNode)PRV_SCHED_ELIG_DEQ_DISABLE == funcArrSip6[j])
            {
                continue;
            }
        }

        rc = prvSchedLowLevelSetBLevelEligPrioFuncEntry(hndl, funcArrSip6[j]);
        if (rc) break;
    }

    if(0==rc)
    {

        /*SIP7 support full set of eligible functions*/
        if (GT_TRUE==PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)))
        {
            for (j = 0; j <sizeof(funcArrSip7)/sizeof(funcArrSip7[0]);j++)
            {
                rc = prvSchedLowLevelSetBLevelEligPrioFuncEntry(hndl, funcArrSip7[j]);
                if (rc) break;
            }
        }
     }


    return rc;
}


/**
 */
int prvSchedLowLevelSetCLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    uint16_t j;

    enum prvShedEligFuncNode funcArrSip7[] ={
           /*no shaping*/
                       PRV_SCHED_ELIG_N_FP0,PRV_SCHED_ELIG_N_FP2,
                       PRV_SCHED_ELIG_N_FP3,PRV_SCHED_ELIG_N_FP4,PRV_SCHED_ELIG_N_FP5,
                       PRV_SCHED_ELIG_N_FP6,PRV_SCHED_ELIG_N_FP7,

            /*shaping*/
                       PRV_SCHED_ELIG_N_MIN_SHP_FP0,PRV_SCHED_ELIG_N_MIN_SHP_FP1,PRV_SCHED_ELIG_N_MIN_SHP_FP2,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP3,PRV_SCHED_ELIG_N_MIN_SHP_FP4,PRV_SCHED_ELIG_N_MIN_SHP_FP5,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP6,PRV_SCHED_ELIG_N_MIN_SHP_FP7
     };

    enum prvShedEligFuncNode funcArrSip6[] ={
                                      /*no shaping*/PRV_SCHED_ELIG_N_FP1,
                                        /*disable*/
                                        PRV_SCHED_ELIG_DEQ_DISABLE
                                      };


    TM_CTL(ctl, hndl)


    for (j = 0; j <sizeof(funcArrSip6)/sizeof(funcArrSip6[0]);j++)
    {
        if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(PDQ_ENV_DEVNUM_NUM(ctl))->devFamily)
        {
            if((enum prvShedEligFuncNode)PRV_SCHED_ELIG_DEQ_DISABLE == funcArrSip6[j])
            {
                continue;
            }
        }

        rc = prvSchedLowLevelSetCLevelEligPrioFuncEntry(hndl, funcArrSip6[j]);
        if (rc) break;
    }

    if(0==rc)
    {

        /*SIP7 support full set of eligible functions*/
        if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E == PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
        {
            for (j = 0; j <sizeof(funcArrSip7)/sizeof(funcArrSip7[0]);j++)
            {
                rc = prvSchedLowLevelSetCLevelEligPrioFuncEntry(hndl, funcArrSip7[j]);
                if (rc) break;
            }
        }
     }



    return rc;
}


/**
 */
int prvSchedLowLevelSetPLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
    uint16_t j;

     enum prvShedEligFuncNode funcArrSip7[] ={
           /*no shaping*/
                       PRV_SCHED_ELIG_N_FP0,PRV_SCHED_ELIG_N_FP2,
                       PRV_SCHED_ELIG_N_FP3,PRV_SCHED_ELIG_N_FP4,PRV_SCHED_ELIG_N_FP5,
                       PRV_SCHED_ELIG_N_FP6,PRV_SCHED_ELIG_N_FP7,

            /*shaping*/
                       PRV_SCHED_ELIG_N_MIN_SHP_FP0,PRV_SCHED_ELIG_N_MIN_SHP_FP2,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP3,PRV_SCHED_ELIG_N_MIN_SHP_FP4,PRV_SCHED_ELIG_N_MIN_SHP_FP5,
                       PRV_SCHED_ELIG_N_MIN_SHP_FP6,PRV_SCHED_ELIG_N_MIN_SHP_FP7
     };

    enum prvShedEligFuncNode funcArrSip6[] ={
                                      /*no shaping*/PRV_SCHED_ELIG_N_FP1,
                                        /*disable*/
                                        PRV_SCHED_ELIG_DEQ_DISABLE
                                      };


    TM_CTL(ctl, hndl)

    for (j = 0; j <sizeof(funcArrSip6)/sizeof(funcArrSip6[0]);j++)
    {
        if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(PDQ_ENV_DEVNUM_NUM(ctl))->devFamily)
        {
            if((enum prvShedEligFuncNode)PRV_SCHED_ELIG_DEQ_DISABLE == funcArrSip6[j])
            {
                continue;
            }
        }

        rc = prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl, funcArrSip6[j]);
        if (rc) break;
    }

    if(0==rc)
    {
        if (GT_TRUE==PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)))
        {
             rc = prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl,PRV_SCHED_ELIG_N_MIN_SHP_FP1);
        }
        if(0==rc)
        {
            /*SIP7 support full set of eligible functions*/
            if (GT_TRUE==PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)))
            {
                for (j = 0; j <sizeof(funcArrSip7)/sizeof(funcArrSip7[0]);j++)
                {
                    rc = prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl, funcArrSip7[j]);
                    if (rc) break;
                }
            }
         }
     }

    return rc;
}

/**
 */
int prvSchedLowLevelClearCLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_CLvlDef)

    TM_CTL(ctl, hndl)

    /*not present at Ironman*/
   if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
   {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        if (index < ctl->tm_total_c_nodes)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_CLvlDef)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_CLvlDef , Deficit , 0x1)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.CLvlDef, index ,TM_Sched_CLvlDef)
            if (rc) goto out;
        }
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearBLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_BlvlDef)

    TM_CTL(ctl, hndl)

       /*not present at Ironman*/
   if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
   {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        if (index < ctl->tm_total_b_nodes)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_BlvlDef)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlDef , Deficit , 0x1)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlDef, index,TM_Sched_BlvlDef)
            if (rc)  goto out;
        }
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearALevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_AlvlDef)

    TM_CTL(ctl, hndl)

           /*not present at Ironman*/
   if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
   {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        if (index < ctl->tm_total_a_nodes)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_AlvlDef)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlDef , Deficit , 0x1)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlDef, index, TM_Sched_AlvlDef)
            if (rc)
              goto out;
        }
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearQLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_QueueDef)

    TM_CTL(ctl, hndl)

   if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
   {
        rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
        if (rc)  goto out;

        if (index < ctl->tm_total_queues)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_QueueDef)
            if (rc) goto out;
            /* assign register fields */
             TXQ_SCHED_REGISTER_SET(TM_Sched_QueueDef , Deficit , 0x1)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.QueueDef, index, TM_Sched_QueueDef)
            if (rc)
              goto out;
        }
    }
out:

    return rc;
}


int prvSchedLowLevelGetGeneralHwParams(PRV_CPSS_SCHED_HANDLE hndl)
{


    TM_CTL(ctl, hndl)

    /*fixed to 0,no package in Falcon*/
    ctl->min_pkg_size = 0;

    return 0;

}


/**
 */
int prvSchedLowLevelGetPortStatus(PRV_CPSS_SCHED_HANDLE hndl,
                     uint8_t index,
                     struct sched_port_status *tm_status)
{


    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio0)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio1)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio2)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio3)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio4)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio5)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio6)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio7)


    TM_CTL(ctl, hndl)


    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;


     TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortShpBucketLvls, index , PortShpBucketLvls)
     if (rc) return rc;

     TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortShpBucketLvls, MinLvl , tm_status->min_bucket_level , (uint32_t) ) /* casting to uint32_t */
     TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortShpBucketLvls, MaxLvl , tm_status->max_bucket_level , (uint32_t) )  /* casting to uint32_t */

    /*registers are not supported for ironman*/
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        /*Prio 0*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio0, index , TM_Sched_PortDefPrio0)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio0, Deficit , tm_status->deficit[0]  ,  (uint32_t) )  /* casting to uint32_t */

        /*Prio 1*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio1, index , TM_Sched_PortDefPrio1)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio1, Deficit , tm_status->deficit[1]  ,  (uint32_t) )  /* casting to uint32_t */

         /*Prio 2*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio2, index , TM_Sched_PortDefPrio2)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio2, Deficit , tm_status->deficit[2]  ,  (uint32_t) )  /* casting to uint32_t */

        /*Prio 3*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio3, index , TM_Sched_PortDefPrio3)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio3, Deficit , tm_status->deficit[3]  ,  (uint32_t) )  /* casting to uint32_t */

        /*Prio 4*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio4, index , TM_Sched_PortDefPrio4)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio4, Deficit , tm_status->deficit[4]  ,  (uint32_t) )  /* casting to uint32_t */

        /*Prio 5*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio5, index , TM_Sched_PortDefPrio5)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio5, Deficit , tm_status->deficit[5]  ,  (uint32_t) )  /* casting to uint32_t */

         /*Prio 6*/

        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio6, index , TM_Sched_PortDefPrio6)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio6, Deficit , tm_status->deficit[6]  ,  (uint32_t) )  /* casting to uint32_t */

        /*Prio 7*/

         TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortDefPrio7, index , TM_Sched_PortDefPrio7)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio7, Deficit , tm_status->deficit[7]  ,  (uint32_t) )  /* casting to uint32_t */
    }
    return rc;


}


/**
 */
int prvSchedLowLevelGetCNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_CLvlDef)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  return rc;

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlShpBucketLvls, index , ClvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

       /*registers are not supported for ironman*/
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.CLvlDef, index , TM_Sched_CLvlDef)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_CLvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    }

    return rc;
}


/**
 */
int prvSchedLowLevelGetBNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_BlvlDef)
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) return rc;

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlShpBucketLvls, index , BlvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

     /*registers are not supported for ironman*/
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlDef, index , TM_Sched_BlvlDef)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    }
    return rc;
}


/**
 */
int prvSchedLowLevelGetANodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    uint32_t lineNum = index/32;
    uint32_t bitOffset = index%32;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_AlvlDef)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ALevelShaperBucketNeg)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  return rc;

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlShpBucketLvls, index , AlvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

       /*registers are not supported for ironman*/
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlDef, index , TM_Sched_AlvlDef)
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    }

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALevelShaperBucketNeg, lineNum , ALevelShaperBucketNeg)
    if (rc)
        return rc;

    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ALevelShaperBucketNeg, MinTBNeg, tm_status->min_bucket_sign ,  (uint32_t) )  /* casting to uint32_t */

    tm_status->min_bucket_sign>>=bitOffset;
    tm_status->min_bucket_sign&=0x1;
    return rc;
}


/**
 */
int prvSchedLowLevelGetQueueStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    uint32_t lineNum = index/32;
    uint32_t bitOffset = index%32;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_QueueDef)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueShaperBucketNeg)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  return rc;

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueShpBucketLvls, index , QueueShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */


    /*registers are not supported for ironman*/
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.QueueDef, index , TM_Sched_QueueDef)
        if (rc)
            return rc;

        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    }

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueShaperBucketNeg, lineNum , QueueShaperBucketNeg)
    if (rc)
        return rc;

    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueShaperBucketNeg, MinTBNeg, tm_status->min_bucket_sign ,  (uint32_t) )  /* casting to uint32_t */

    tm_status->min_bucket_sign>>=bitOffset;
    tm_status->min_bucket_sign&=0x1;

    return rc;
}






/**
 */





/**
 */
int prvSchedLowLevelGetSchedErrors(PRV_CPSS_SCHED_HANDLE hndl, struct schedErrorInfo *info)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ErrCnt_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ExcpCnt_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  return rc;

    TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ErrCnt_Addr , ErrCnt_Addr)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ErrCnt_Addr , Cnt , info->error_counter ,(uint16_t) )  /* casting to uint16_t */

    TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ExcpCnt_Addr , ExcpCnt_Addr)
    if (rc)
        return rc;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ExcpCnt_Addr, Cnt , info->exception_counter ,(uint16_t) )  /* casting to uint16_t */

    return rc;
}




/**
 */
int prvSchedLowLevelGetSchedStatus
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t * sched_status_ptr
)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    GT_U64 data;

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)return rc;

    rc = prvSchedRegisterRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(addressSpacePtr->Sched.ErrorStatus_Addr), &data);
    if (rc)
        return rc;

   *sched_status_ptr = data.l[0];

    return rc;
}









/**
 * get A to P level Eligible Priority Function
 */
int prvSchedLowLevelGetNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint16_t func_offset, struct tm_elig_prio_func_node *params)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    int i,j;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlEligPrioFunc_Entry)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlEligPrioFunc_Entry)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlEligPrioFunc_Entry)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PortEligPrioFunc_Entry)
    GT_U32 tableSize;

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)return rc;

    TXQ_SCHED_NODE_ELIG_FUNC_NUM_GET(tableSize);

    switch (level) {
        case SCHED_Q_LEVEL:
            rc = -EFAULT;
            break;
        case SCHED_A_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlEligPrioFunc_Entry, func_offset + i*tableSize ,AlvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlEligPrioFunc_Entry , FuncOut0 , params->tbl_entry[i].func_out[0], (uint16_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlEligPrioFunc_Entry , FuncOut1 , params->tbl_entry[i].func_out[1], (uint16_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlEligPrioFunc_Entry , FuncOut2 , params->tbl_entry[i].func_out[2], (uint16_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlEligPrioFunc_Entry , FuncOut3 , params->tbl_entry[i].func_out[3], (uint16_t))
            }
            break;
        case SCHED_B_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlEligPrioFunc_Entry, func_offset + i*tableSize ,BlvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
            }
            break;
        case SCHED_C_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlEligPrioFunc_Entry, func_offset + i*tableSize ,ClvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(ClvlEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
            }
            break;
        case SCHED_P_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PortEligPrioFunc_Entry, func_offset + i*tableSize ,PortEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PortEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
                if(GT_TRUE==PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)))
                {
                    for(j=0;j<4;j++)
                    {
                        params->tbl_entry[i].func_out[j] = prvSchedSip7PLevelEligPrioFuncEntryUnzip(params->tbl_entry[i].func_out[j]);
                    }
                }
            }
            break;
        default:
            goto out;
        }
out:
    return rc;
}

/**
 * get q level Eligible Priority Function
 */
int prvSchedLowLevelGetQueueEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset, struct tm_elig_prio_func_queue *params)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueEligPrioFunc)

    TM_CTL(ctl, hndl);

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueEligPrioFunc, func_offset  ,QueueEligPrioFunc)
    if (rc) goto out;
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueEligPrioFunc, FuncOut0, params->tbl_entry.func_out[0], (uint16_t));
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueEligPrioFunc, FuncOut1, params->tbl_entry.func_out[1], (uint16_t));
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueEligPrioFunc, FuncOut2, params->tbl_entry.func_out[2], (uint16_t));
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueEligPrioFunc, FuncOut3, params->tbl_entry.func_out[3], (uint16_t));

out:
    return rc;
}






int prvSchedLowLevelSetANodeQuantum
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t node_ind
)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlQuantum)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(AlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlQuantum , node_ind , AlvlQuantum)
        if (rc) goto out;

    }

out:

    return rc;
}


int prvSchedLowLevelDisableAlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl,uint32_t first_a_node, uint32_t total_a_nodes)
{
    int rc =  -EFAULT;
    uint32_t i;
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ALvltoBlvlAndQueueRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv)

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Disable Ports */
    for (i = first_a_node; i < first_a_node+total_a_nodes; i++)
    {
         if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
         {
             /* reset register reserved fields */
             TXQ_SCHED_REGISTER_RESET(TM_Sched_AlvlEligPrioFuncPtr)
             if (rc) break;
             /* assign register fields */
             TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE)
             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlEligPrioFuncPtr , i , TM_Sched_AlvlEligPrioFuncPtr)
         }
         else
         {
             /*read*/
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(
                 addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap, i, ALvltoBlvlAndQueueRangeMap);
             if (rc)
             {
                 return rc;
             }
             if(PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
             {

                 /* reset register reserved fields */
                 TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv)
                 if (rc) goto out;
                 /* assign register fields */
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MinToken ,0)
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MaxToken ,0)
                 /* write register */
                 TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv , i ,AlvlTokenBucketTokenEnDiv)
                 if (rc) goto out;
                  /*modify*/
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ALvltoBlvlAndQueueRangeMap , afunc_ptr , PRV_SCHED_ELIG_N_MIN_SHP_FP1)
            }
            else
            {
                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ALvltoBlvlAndQueueRangeMap , afunc_ptr , PRV_SCHED_ELIG_DEQ_DISABLE)

            }


         /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ALvltoBlvlAndQueueRangeMap , i ,ALvltoBlvlAndQueueRangeMap)
             if (rc)
             {
                 return rc;
             }
         }

    }
out:

    return rc;
}


int prvSchedLowLevelDisableBlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BLvltoClvlAndAlvlRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Disable Ports */
    for (i = 0; i < (int)total_b_nodes; i++)
    {
         if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
         {
             /* reset register reserved fields */
             TXQ_SCHED_REGISTER_RESET(TM_Sched_BlvlEligPrioFuncPtr)
             if (rc) break;
             /* assign register fields */
             TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE) /* DeQ disable function ID */
             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlEligPrioFuncPtr , i , TM_Sched_BlvlEligPrioFuncPtr)
         }
         else
         {
             /*read*/
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap, i,BLvltoClvlAndAlvlRangeMap);
             if (rc)
             {
                 return rc;
             }


             if(PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
             {
                   /* reset register reserved fields */
                 TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv)
                 if (rc) goto out;
                /* assign register fields */
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MinToken ,0)
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MaxToken ,0)
                 /* write register */
                 TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.BlvlTokenBucketTokenEnDiv , i ,BlvlTokenBucketTokenEnDiv)
                 if (rc) goto out;

                 /*modify*/
                 TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(BLvltoClvlAndAlvlRangeMap , bfunc_ptr , PRV_SCHED_ELIG_N_MIN_SHP_FP1)
             }
             else
             {   /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(BLvltoClvlAndAlvlRangeMap , bfunc_ptr , PRV_SCHED_ELIG_DEQ_DISABLE)

             }

             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BLvltoClvlAndAlvlRangeMap , i ,BLvltoClvlAndAlvlRangeMap)
             if (rc)
             {
                 return rc;
             }

         }
    }
out:

    return rc;
}


int prvSchedLowLevelDisableQlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_q_nodes)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)
    TM_REGISTER_VAR(PDQ_SIP_6_30_Queuemapping)
    TM_REGISTER_VAR(PDQ_SIP_7_Queuemapping)
    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* Disable Ports */
    for (i = 0; i < (int)total_q_nodes; i++)
    {
         if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
         {
             /* reset register reserved fields */
             TXQ_SCHED_REGISTER_RESET(TM_Sched_QueueEligPrioFuncPtr)
             if (rc) break;
             /* assign register fields */
             TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFuncPtr , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE)
             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.QueueEligPrioFuncPtr , i , TM_Sched_QueueEligPrioFuncPtr)
         }
         else
         {
             /*read*/
             TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping, i, Queuemapping);
             if (rc)
             {
                 return rc;
             }
             /*modify*/
             TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Queuemapping , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE)

             /* write register */
             TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Queuemapping , i , Queuemapping)
             if (rc)
             {
                 return rc;
             }
         }
    }
out:

    return rc;
}


int prvSchedLowLevelDisableClevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes)
{
    int rc =  -EFAULT;
    int i;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvltoPortAndBlvlRangeMap)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;



    /* Disable Ports */
    for (i = 0; i < (int)total_c_nodes; i++)
    {

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(TM_Sched_ClvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , PRV_SCHED_ELIG_DEQ_DISABLE)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlEligPrioFuncPtr , i , TM_Sched_ClvlEligPrioFuncPtr)
        }
        else
        {
            /*read*/
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap, i, ClvltoPortAndBlvlRangeMap);
            if (rc)
            {
                return rc;
            }

            if(PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                /* reset register reserved fields */
                TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MinToken ,0)
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MaxToken ,0)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv , i ,ClvlTokenBucketTokenEnDiv)
                if (rc) goto out;

                /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ClvltoPortAndBlvlRangeMap , cfunc_ptr , PRV_SCHED_ELIG_N_MIN_SHP_FP1)

            }
            else
            {
                 /*modify*/
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(ClvltoPortAndBlvlRangeMap , cfunc_ptr , PRV_SCHED_ELIG_DEQ_DISABLE)
            }

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvltoPortAndBlvlRangeMap , i ,ClvltoPortAndBlvlRangeMap)
            if (rc)
            {
                return rc;
            }
        }

    }
out:

    return rc;
}

int prvSchedAdressSpacePointerGet(PRV_CPSS_SCHED_HANDLE hndl,struct prvCpssDxChTxqSchedAddressSpace ** adressSpacePtrPtr)
{

    GT_U8 devNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION *tilePtr;
    TM_CTL(ctl, hndl)

    PDQ_ENV_MAGIC_CHECK(ctl);
    /*get device*/
    devNum =PDQ_ENV_DEVNUM_NUM(ctl);
    /*get  tile*/
    tilePtr =PRV_CPSS_TILE_HANDLE_GET_MAC(devNum,0);
    *adressSpacePtrPtr = (struct prvCpssDxChTxqSchedAddressSpace *)(tilePtr->general.addressSpacePtr);

    return 0;

}


int prvSchedLowLevelPortPizzaLastSliceSet
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t lastSlice
)
{
    int rc =  -EFAULT;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(Port_Pizza_Last_Slice)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* reset register reserved fields */
    TXQ_SCHED_REGISTER_DEVDEPEND_RESET_FROM_SIP_6_30(Port_Pizza_Last_Slice)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Port_Pizza_Last_Slice , pzlast_slice ,lastSlice)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Port_Pizza_Last_Slice, Port_Pizza_Last_Slice)
   if (rc) goto out;

out:

    return rc;
}

int prvSchedLowLevelPortPizzaLastSliceGet
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t *lastSlicePtr
)
{
    int rc =  -EFAULT;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(Port_Pizza_Last_Slice)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    /* read register */
    TXQ_SCHED_READ_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.Port_Pizza_Last_Slice, Port_Pizza_Last_Slice)
    if (rc) goto out;

    /* assign register fields */
    TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Port_Pizza_Last_Slice , pzlast_slice ,*lastSlicePtr,(uint32_t))
out:
    return rc;
}

int prvSchedLowLevelPizzaSliceSet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 sliceInd, GT_U32 port_ind)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 internalIndex;

    TM_REGISTER_VAR(PDQ_SIP_6_30_Port_Pizza_Slice)
    TM_REGISTER_VAR(PDQ_SIP_7_Port_Pizza_Slice)

    TM_CTL(ctl, hndl);

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    /* read register */
    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortQuantumsPriosLo/*same as Port_Pizza_Slice*/ ,
        sliceInd>>2/*each line contain 4 slices*/ ,Port_Pizza_Slice)
    if (rc) goto out;

    internalIndex = sliceInd&0x3;

    /* assign register field */
    switch(internalIndex)
    {
        case 0:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Port_Pizza_Slice , slice0_port_id , port_ind);
          break;
        case 1:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Port_Pizza_Slice , slice1_port_id , port_ind);
          break;
        case 2:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Port_Pizza_Slice , slice2_port_id , port_ind);
          break;
        case 3:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(Port_Pizza_Slice , slice3_port_id , port_ind);
          break;
     }

    /* write register */
    TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortQuantumsPriosLo,
        sliceInd>>2 ,Port_Pizza_Slice)

    if (rc) goto out;


out:

    return rc;
}


int prvSchedLowLevelPizzaSliceGet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 sliceInd, GT_U32 * port_ind_ptr)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 internalIndex;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(Port_Pizza_Slice)

    TM_CTL(ctl, hndl);

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    /* read register */
    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortQuantumsPriosLo/*same as Port_Pizza_Slice*/ ,
        sliceInd>>2/*each line contain 4 slices*/ , Port_Pizza_Slice)
    if (rc) goto out;

    internalIndex = sliceInd&0x3;

    /* assign register field */
    switch(internalIndex)
    {
        case 0:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Port_Pizza_Slice , slice0_port_id , *port_ind_ptr,(GT_U32));
          break;
        case 1:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Port_Pizza_Slice , slice1_port_id , *port_ind_ptr,(GT_U32));
          break;
        case 2:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Port_Pizza_Slice , slice2_port_id , *port_ind_ptr,(GT_U32));
          break;
        case 3:
          TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(Port_Pizza_Slice , slice3_port_id , *port_ind_ptr,(GT_U32));
          break;
     }
out:

    return rc;
}



int prvSchedLowLevelPizzaSliceLineGet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 lineInd, GT_U32 * dataPtr)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    char *tmpPtr;
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_FROM_SIP_6_30(Port_Pizza_Slice_Line)

    TM_CTL(ctl, hndl);

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    /* read register */
    TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_FROM_SIP_6_30(addressSpacePtr->Sched.PortQuantumsPriosLo/*same as Port_Pizza_Slice*/ ,
        lineInd, Port_Pizza_Slice_Line)
    if (rc) goto out;

    if(PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        tmpPtr = TM_REGISTER_VAR_NAME(PDQ_SIP_6_30_Port_Pizza_Slice_Line);
    }
    else
    {
        tmpPtr = TM_REGISTER_VAR_NAME(PDQ_SIP_7_Port_Pizza_Slice_Line);
    }

    (*dataPtr) = ((GT_U32)(tmpPtr[0]))&0xFF;
    (*dataPtr) |= (((GT_U32)(tmpPtr[1]))&0xFF)<<8;
    (*dataPtr) |= ((GT_U32)tmpPtr[2]&0xFF)<<16;
    (*dataPtr) |= ((GT_U32)(tmpPtr[3])&0xFF)<<24;

out:

    return rc;
}

int prvSchedLowLevelPizzaSliceEmptyLineSet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 lineInd)
{
    int rc =  -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    GT_U32 i;

    TM_REGISTER_VAR(Port_Pizza_Slice_Line)

    TM_CTL(ctl, hndl);

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc) goto out;

    for(i=0;i<7;i++)
    {   /*sip 7 slice width is 7 bits. 7*4=28 bits ,so lets set 32 bit  as a worst case.*/
        TM_REGISTER_VAR_NAME(Port_Pizza_Slice_Line)[i]= (i<4)?0xFF:0x0;
    }

    /* write register */
    TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PortQuantumsPriosLo/*same as Port_Pizza_Slice*/ ,
        lineInd, Port_Pizza_Slice_Line);

    if (rc) goto out;

out:

    return rc;
}




int prvSchedLowLevelSetBNodeQuantum
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t node_ind
)
{
    int rc =  -EFAULT;

    struct tm_b_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlQuantum)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    if (node_ind < ctl->tm_total_b_nodes)
    {
        node = &(ctl->tm_b_node_array[node_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlQuantum , node_ind , BlvlQuantum)
        if (rc) goto out;

    }

out:

    return rc;
}




int prvSchedLowLevelSetCNodeQuantum
(
    PRV_CPSS_SCHED_HANDLE hndl,
    uint32_t node_ind
)
{
    int rc =  -EFAULT;

    struct tm_c_node *node = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(ClvlQuantum)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)  goto out;

    if (node_ind < ctl->tm_total_c_nodes)
    {
        node = &(ctl->tm_c_node_array[node_ind]);

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(ClvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(ClvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.ClvlQuantum , node_ind , ClvlQuantum)
        if (rc) goto out;

    }

out:

    return rc;
}

























