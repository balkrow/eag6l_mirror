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
 * @brief TM nodes configuration reading interface.
 *
* @file tm_nodes_read.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_NODES_READ_H
#define SCHED_NODES_READ_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>


/***************************************************************************
 * Read Configuration
 ***************************************************************************/

/** Read queue software configuration.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     queue_index     Queue index.
 *   @param[out]    q_params        Queue parameters structure pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if queue_index is out of range.
 *   @retval -ENODATA if queue_index is not in use.
 */
int prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_index,
                                struct sched_queue_params *q_params);


/** Read A-node software configuration.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     a_node_index    A-node index.
 *   @param[out]    a_params        A-node parameters structure pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if a_node_index is out of range.
 *   @retval -ENODATA if a_node_index is not in use.
 */
int prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t a_node_index,
                                 struct sched_a_node_params *a_params);


/** Read B-node software configuration.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     b_node_index    B-node index.
 *   @param[out]    b_params        B-node parameters structure pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if b_node_index is out of range.
 *   @retval -ENODATA if b_node_index is not in use.
 */
int prvCpssSchedNodesReadBNodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t b_node_index,
                                 struct sched_b_node_params *b_params);


/** Read C-node software configuration.
 *
 *   @param[in]     hndl            TM lib handle.
 *   @param[in]     c_node_index    C-node index.
 *   @param[out]    c_params        C-node parameters structure pointer.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if c_node_index is out of range.
 *   @retval -ENODATA if c_node_index is not in use.
 */
int prvCpssSchedNodesReadCNodeConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t c_node_index,
                                 struct sched_c_node_params *c_params);


/** Read Port software configuration.
 *
 *   @param[in]     hndl          TM lib handle.
 *   @param[in]     port_index    Port index.
 *   @param[out]    params        Port parameters structure pointer.
 *
 *   @note The CIR and EIR bw may deviate from the originally configured
 *   by tm_create_port or tm_update_port by the
 *   bw accuracy parameter for SCHED_P_LEVEL provisioned in tm_configure_periodic_scheme API.
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -EFAULT if port_index is out of range.
 *   @retval -ENODATA if port_index is not in use.
 */
int prvCpssSchedNodesReadPortConfiguration(PRV_CPSS_SCHED_HANDLE hndl, uint32_t port_index,
                               struct sched_port_params *params);



/** traverse  port tree applying per node callback.
 *
 *   @param[in]		hndl		TM lib handle
 *
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 */

typedef int(*SCHED_PORT_TRAVERSE_TREE_FUN)(	int, 			/* if true  - istart node evaluation / otherwise - end node evaluation */
											void *,			/*environment handle hndl */
											unsigned int,	/* level :  HXTM_Q_LEVEL :: HXTM_P_LEVEL */
											unsigned int,	/* node index - valid */
											unsigned int, /* node status */
											unsigned int,	/* node parent index , if applicable - valid , or 0 - for ports*/
											unsigned int,	/* node eligible function*/
											unsigned int,	/* dwrr bmp*/
											void *);		/* user data ptr */


int prvCpssSchedNodesReadTraversePortTreeSw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int port_index, SCHED_PORT_TRAVERSE_TREE_FUN dump_fun, void * vpEnv, void* vpUserData);


int prvCpssSchedNodesReadTraversePortTreeHw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int port_index, SCHED_PORT_TRAVERSE_TREE_FUN dump_fun, void * vpEnv, void* vpUserData);

int prvCpssSchedNodesReadPortMappingHw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int port_index);
int prvCpssSchedNodesReadCnodeMappingHw(PRV_CPSS_SCHED_HANDLE hndl, unsigned int ic);
int prvCpssSchedNodesReadBnodeMappingHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    unsigned int ib,
    GT_U32      *bnodeParentPtr,
    GT_U32      *bnodeChildLoPtr,
    GT_U32      *bnodeChildHiPtr,
    GT_U32      *bnodeEfPtr

);

int prvCpssSchedNodesReadAnodeMappingHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    unsigned int ia,
    GT_U32      *anodeParentPtr,
    GT_U32      *anodeChildLoPtr,
    GT_U32      * anodeChildHiPtr,
    GT_U32      *anodeEfPtr

);

int prvCpssSchedNodesReadQnodeMappingHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    unsigned int iq,
    GT_U32      *qnodeParentPtr,
    GT_U32      *qnodeEfPtr

);
int prvCpssSchedNodesReadQnodeMappingSw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    unsigned int iq,
    GT_U32      *qnodeParentPtr,
    GT_U32      *qnodeEfPtr

);
int prvCpssSchedNodesReadAnodeMappingSw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    unsigned int ia,
    GT_U32      *anodeParentPtr,
    GT_U32      *anodeChildLoPtr,
    GT_U32      *anodeChildHiPtr,
    GT_U32      *anodeEfPtr,
    GT_U32      *anodeStatusPtr
);



#endif   /* TM_NODES_READ_H */

