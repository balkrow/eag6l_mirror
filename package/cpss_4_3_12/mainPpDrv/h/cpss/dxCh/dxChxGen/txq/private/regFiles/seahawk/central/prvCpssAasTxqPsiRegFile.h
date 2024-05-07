/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,     *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
* prvCpssAasTxqPsiRegFile.h
*
*Automaticly generated from :
*
* DESCRIPTION:
*      Describe TXQ PSI registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef _TXQ_AAS__PSI_REGFILE_H_
#define _TXQ_AAS__PSI_REGFILE_H_

/* start of register PSI_Interrupt_Cause*/

#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_OFFSET            0
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_OFFSET           1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_OFFSET            2
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_OFFSET           3
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_SDQ_ACC_MAP_ERR_FIELD_OFFSET           4
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PDQ_ACC_MAP_ERR_FIELD_OFFSET           5


#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_SIZE          1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_SIZE          1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_SDQ_ACC_MAP_ERR_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_CAUSE_PDQ_ACC_MAP_ERR_FIELD_SIZE         1

/* end of PSI_Interrupt_Cause*/


/* start of register PSI_Interrupt_Mask*/

#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_1_FIELD_OFFSET           1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_2_FIELD_OFFSET           2
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_3_FIELD_OFFSET           3
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_4_FIELD_OFFSET           4
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_5_FIELD_OFFSET           5


#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_1_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_2_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_3_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_4_FIELD_SIZE         1
#define      TXQ_AAS_PSI_PSI_INTERRUPT_MASK_PSI_INT_CAUSE_MASK_5_FIELD_SIZE         1

/* end of PSI_Interrupt_Mask*/


/* start of register PSI_Last_Address_Violation*/

#define      TXQ_AAS_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_OFFSET         0


#define      TXQ_AAS_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_SIZE           32

/* end of PSI_Last_Address_Violation*/


/* start of register PSI_Metal_Fix_Register*/

#define      TXQ_AAS_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_OFFSET          0


#define      TXQ_AAS_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_SIZE            32

/* end of PSI_Metal_Fix_Register*/


/* start of register Debug_CFG_Register*/

#define      TXQ_AAS_PSI_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_OFFSET            0
#define      TXQ_AAS_PSI_DEBUG_CFG_REGISTER_PDQ_CG_EN_FIELD_OFFSET          1


#define      TXQ_AAS_PSI_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_SIZE          1
#define      TXQ_AAS_PSI_DEBUG_CFG_REGISTER_PDQ_CG_EN_FIELD_SIZE            1

/* end of Debug_CFG_Register*/


/* start of register SDQ_Map_Error_Capture*/

#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ACC_ERROR_VALID_FIELD_OFFSET         0
#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_QUEUE_FIELD_OFFSET         1
#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_ID_FIELD_OFFSET            16


#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ACC_ERROR_VALID_FIELD_SIZE           1
#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_QUEUE_FIELD_SIZE           15
#define      TXQ_AAS_PSI_SDQ_MAP_ERROR_CAPTURE_SDQ_ERROR_ID_FIELD_SIZE          2

/* end of SDQ_Map_Error_Capture*/


/* start of register SNK_Access_Error*/

#define      TXQ_AAS_PSI_SNK_ACCESS_ERROR_SNK_ACC_ERR_FIELD_OFFSET          0


#define      TXQ_AAS_PSI_SNK_ACCESS_ERROR_SNK_ACC_ERR_FIELD_SIZE            32

/* end of SNK_Access_Error*/


/* start of register Psi_Idle*/

#define      TXQ_AAS_PSI_PSI_IDLE_PSI_IDLE_FIELD_OFFSET         0
#define      TXQ_AAS_PSI_PSI_IDLE_PDQ_REQ_FIFO_EMPTY_FIELD_OFFSET           1


#define      TXQ_AAS_PSI_PSI_IDLE_PSI_IDLE_FIELD_SIZE           1
#define      TXQ_AAS_PSI_PSI_IDLE_PDQ_REQ_FIFO_EMPTY_FIELD_SIZE         1

/* end of Psi_Idle*/


/* start of register debug_invq2q_read*/

#define      TXQ_AAS_PSI_DEBUG_INVQ2Q_READ_DEBUG_INVQ2Q_READ_FIELD_OFFSET           0


#define      TXQ_AAS_PSI_DEBUG_INVQ2Q_READ_DEBUG_INVQ2Q_READ_FIELD_SIZE         17

/* end of debug_invq2q_read*/


/* start of table entry q2q*/

#define      TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_OFFSET          0
#define      TXQ_AAS_PSI_Q2Q_VALID_FIELD_OFFSET         16


#define      TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_SIZE            16
#define      TXQ_AAS_PSI_Q2Q_VALID_FIELD_SIZE           1

/* end of q2q*/

/* manual - start of register BP PDQ Port <<%i>> SDQ <<%n>>*/

#define      TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_OFFSET          0


#define      TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_SIZE           23

/* end of register BP PDQ Port <<%i>> SDQ <<%n>> */

/* manual -  start of register Credit_Value*/

#define      TXQ_AAS_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET         0


#define      TXQ_AAS_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE           6

/* end of Credit_Value*/


#endif
