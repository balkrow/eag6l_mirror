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
* @file tm_nodes_index_fetch.c
*
* @brief APIs to fetch index from node params
*
* @version   1
********************************************************************************
*/

#ifndef __KERNEL__
#include <assert.h>
#else
#define assert(expr)
#endif

/*#define RESHUFFLING_SHIELD */

#include "tm_nodes_create.h"

#include "tm_core_types.h"

#include "tm_virtual_layer_interface.h"
#include "tm_locking_interface.h"
#include "tm_errcodes.h"
#include "tm_set_local_db_defaults.h"
#include "set_hw_registers.h"
#include "tm_os_interface.h"

#include "tm_nodes_ctl_internal.h"
#include "tm_hw_configuration_interface.h"
#include "tm_shaping_utils.h"
/* for validation shaping profiles during node creation */
#include "tm_shaping_internal.h"

#include "rm_status.h"
#include "rm_chunk.h"

#define TM_NODE_CHILDREN_MAX  256

/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_a_node_to_b_node_index_fetch_is_match
(
    tm_handle   hndl,
    uint32_t    parent_b_node_index,
    struct tm_a_node_params *a_params,
    uint32_t    a_node_index,
    uint32_t    *first_queue_index_ptr
)
{
    struct tm_a_node    *a_node = NULL;
    uint32_t num_children = a_params->num_of_children;
    TM_CTL(ctl, hndl);

    a_node = &ctl->tm_a_node_array[a_node_index];
    if ((a_node->mapping.nodeStatus != TM_NODE_USED) &&
        ((uint32_t)a_node->mapping.nodeParent == parent_b_node_index) &&
        (a_node->mapping.childLo <= a_node->mapping.childHi) &&
        ((uint32_t)(a_node->mapping.childHi - a_node->mapping.childLo) == num_children-1))
    {
        *first_queue_index_ptr = (uint32_t)a_node->mapping.childLo;
        return 0;
    }
    return 1;
}

/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_a_node_to_b_node_index_fetch_a_node_params_match
(
    tm_handle   hndl,
    struct tm_a_node_params *a_params,
    uint32_t    a_node_index
)
{
    struct tm_a_node    *a_node = NULL;
    TM_CTL(ctl, hndl);

    a_node = &ctl->tm_a_node_array[a_node_index];
    if ((a_node->dwrr_quantum        == a_params->quantum) &&
        (a_node->elig_prio_func      == a_params->elig_prio_func_ptr) &&
        (a_node->shaping_profile_ref == a_params->shaping_profile_ref) &&
        (ctl->tm_a_lvl_drop_prof_ptr[a_node_index] == a_params->wred_profile_ref))
    {
        return 0;
    }

    return 1;
}


/**
 * @brief 1. B node can have many child a nodes.
 *        2. Match a node params from hardware with the params passed to
 *          identify appropriate index.
 *        3. If more than one match is found, passed index is chosen if available
 *          otherwise the smallest index is chosen.
 */
int tm_create_a_node_to_b_node_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_b_node_index,
    struct tm_a_node_params *a_params,
    uint32_t    *a_node_index_ptr,
    uint32_t    *first_queue_index_ptr
)
{
    struct tm_a_node    *a_node = NULL;
    struct tm_b_node    *b_node = NULL;
    uint32_t a_node_index = *a_node_index_ptr;
    uint32_t first_queue_index = 0;
    uint32_t i;
    uint32_t num_children = a_params->num_of_children;
    int ret = 0;

    uint32_t match_a_node_index[TM_NODE_CHILDREN_MAX];
    uint32_t match_first_queue_index[TM_NODE_CHILDREN_MAX];
    uint8_t match_index_valid[TM_NODE_CHILDREN_MAX];
    uint32_t match_index_count = 0;

    TM_CTL(ctl, hndl);

    tm_memset(match_a_node_index, 0, sizeof(match_a_node_index));
    tm_memset(match_first_queue_index, 0, sizeof(match_first_queue_index));
    tm_memset(match_index_valid, 0, sizeof(match_index_valid));

    /* check whether input b_node index is valid */
    if (parent_b_node_index >= ctl->tm_total_b_nodes)
    {
        return -EINVAL;
    }

    /* check b_node mapping parameters are valid */
    b_node = &ctl->tm_b_node_array[parent_b_node_index];
    if (((uint32_t)b_node->mapping.childLo >= ctl->tm_total_a_nodes) ||
        ((uint32_t)b_node->mapping.childHi >= ctl->tm_total_a_nodes) ||
        (b_node->mapping.childHi < b_node->mapping.childLo))
    {
        return -EINVAL;
    }

    match_index_count = 0;
    /* check whether we can allocate at given index */
    if((a_node_index < ctl->tm_total_a_nodes) &&
       (a_node_index >= (uint32_t)b_node->mapping.childLo) &&
       (a_node_index <= (uint32_t)b_node->mapping.childHi))
    {
        ret = tm_create_a_node_to_b_node_index_fetch_is_match (hndl, parent_b_node_index, a_params,
                                                         a_node_index, &first_queue_index);
        if (ret == 0)
        {
            match_a_node_index[match_index_count] = a_node_index;
            match_first_queue_index[match_index_count] = first_queue_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* Check whether we can allocate at any other index */
    for (i = (uint32_t)b_node->mapping.childLo; i <= (uint32_t)b_node->mapping.childHi; i++)
    {
        /* Already checked */
        if (i == a_node_index)
        {
            continue;
        }

        ret = tm_create_a_node_to_b_node_index_fetch_is_match(hndl, parent_b_node_index, a_params,
                                                         i, &first_queue_index);

        if (ret == 0)
        {
            match_a_node_index[match_index_count] = i;
            match_first_queue_index[match_index_count] = first_queue_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* For all matched mapping indexes, find if the a_node params are also a match */
    for (i=0; i<match_index_count; i++)
    {
        ret = tm_create_a_node_to_b_node_index_fetch_a_node_params_match(hndl, a_params,
                                                        match_a_node_index[i]);
        if (ret == 0)
        {
            *a_node_index_ptr = match_a_node_index[i];
            *first_queue_index_ptr = match_first_queue_index[i];
            return 0;
        }
    }

    if (match_index_count != 0)
    {
            *a_node_index_ptr = match_a_node_index[0];
            *first_queue_index_ptr = match_first_queue_index[0];
    }

    /* Set first_queue_index to NO_PREDEFINED_RANGE */
    /* Ideally we should always get a match. Should not enter below case */
    if((a_node_index >= ctl->tm_total_a_nodes) ||
       (a_node_index < (uint32_t)b_node->mapping.childLo) ||
       (a_node_index > (uint32_t)b_node->mapping.childHi))
    {
        *first_queue_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
    }
    else
    {
        a_node = &ctl->tm_a_node_array[a_node_index];
        if(a_node == NULL)
        {
            return 1;
        }

        if ((a_node->mapping.childHi >= a_node->mapping.childLo) &&
            ((uint32_t)(a_node->mapping.childHi - a_node->mapping.childLo) != (num_children - 1)))
        {
            *first_queue_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
        }
    }

    if(a_node != NULL)
    {
        *first_queue_index_ptr = (uint32_t)a_node->mapping.childLo;
    }

    return 0;
}

/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_b_node_to_c_node_index_fetch_is_match
(
    tm_handle   hndl,
    uint32_t    parent_c_node_index,
    struct tm_b_node_params *b_params,
    uint32_t    b_node_index,
    uint32_t    *first_a_node_index_ptr
)
{
    struct tm_b_node    *b_node = NULL;
    uint32_t num_children = b_params->num_of_children;
    TM_CTL(ctl, hndl);

    b_node = &ctl->tm_b_node_array[b_node_index];
    if ((b_node->mapping.nodeStatus != TM_NODE_USED) &&
        ((uint32_t)b_node->mapping.nodeParent == parent_c_node_index) &&
        (b_node->mapping.childLo <= b_node->mapping.childHi) &&
        ((uint32_t)(b_node->mapping.childHi - b_node->mapping.childLo) == num_children-1))
    {
        *first_a_node_index_ptr = (uint32_t)b_node->mapping.childLo;
        return 0;
    }
    return 1;
}

/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_b_node_to_c_node_index_fetch_b_node_params_match
(
    tm_handle   hndl,
    struct tm_b_node_params *b_params,
    uint32_t    b_node_index
)
{
    struct tm_b_node    *b_node = NULL;
    TM_CTL(ctl, hndl);

    b_node = &ctl->tm_b_node_array[b_node_index];
    if ((b_node->dwrr_quantum        == b_params->quantum) &&
        (b_node->elig_prio_func      == b_params->elig_prio_func_ptr) &&
        (b_node->shaping_profile_ref == b_params->shaping_profile_ref) &&
        (ctl->tm_b_lvl_drop_prof_ptr[b_node_index] == b_params->wred_profile_ref))
    {
        return 0;
    }

    return 1;
}


/**
 * @brief 1. C node can have many child a nodes.
 *        2. Match a node params from hardware with the params passed to
 *          identify appropriate index.
 *        3. If more than one match is found, passed index is chosen if available
 *          otherwise the smallest index is chosen.
 */
int tm_create_b_node_to_c_node_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_c_node_index,
    struct tm_b_node_params *b_params,
    uint32_t    *b_node_index_ptr,
    uint32_t    *first_a_node_index_ptr
)
{
    struct tm_b_node    *b_node = NULL;
    struct tm_c_node    *c_node = NULL;
    uint32_t b_node_index = *b_node_index_ptr;
    uint32_t first_a_node_index = 0;
    uint32_t i;
    uint32_t num_children = b_params->num_of_children;
    int ret = 0;

    uint32_t match_b_node_index[TM_NODE_CHILDREN_MAX];
    uint32_t match_first_a_node_index[TM_NODE_CHILDREN_MAX];
    uint8_t match_index_valid[TM_NODE_CHILDREN_MAX];
    uint32_t match_index_count = 0;

    TM_CTL(ctl, hndl);

    tm_memset(match_b_node_index, 0, sizeof(match_b_node_index));
    tm_memset(match_first_a_node_index, 0, sizeof(match_first_a_node_index));
    tm_memset(match_index_valid, 0, sizeof(match_index_valid));

    /* check whether input b_node index is valid */
    if (parent_c_node_index >= ctl->tm_total_c_nodes)
    {
        return -EINVAL;
    }

    /* check b_node mapping parameters are valid */
    c_node = &ctl->tm_c_node_array[parent_c_node_index];
    if (((uint32_t)c_node->mapping.childLo >= ctl->tm_total_b_nodes) ||
        ((uint32_t)c_node->mapping.childHi >= ctl->tm_total_b_nodes) ||
        (c_node->mapping.childHi < c_node->mapping.childLo))
    {
        return -EINVAL;
    }

    match_index_count = 0;
    /* check whether we can allocate at given index */
    if((b_node_index < ctl->tm_total_b_nodes) &&
       (b_node_index >= (uint32_t)c_node->mapping.childLo) &&
       (b_node_index <= (uint32_t)c_node->mapping.childHi))
    {
        ret = tm_create_b_node_to_c_node_index_fetch_is_match (hndl, parent_c_node_index, b_params,
                                                         b_node_index, &first_a_node_index);
        if (ret == 0)
        {
            match_b_node_index[match_index_count] = b_node_index;
            match_first_a_node_index[match_index_count] = first_a_node_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* Check whether we can allocate at any other index */
    for (i = (uint32_t)c_node->mapping.childLo; i <= (uint32_t)c_node->mapping.childHi; i++)
    {
        /* Already checked */
        if (i == b_node_index)
        {
            continue;
        }

        ret = tm_create_b_node_to_c_node_index_fetch_is_match(hndl, parent_c_node_index, b_params,
                                                         i, &first_a_node_index);

        if (ret == 0)
        {
            match_b_node_index[match_index_count] = i;
            match_first_a_node_index[match_index_count] = first_a_node_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* For all matched mapping indexes, find if the a_node params are also a match */
    for (i=0; i<match_index_count; i++)
    {
        ret = tm_create_b_node_to_c_node_index_fetch_b_node_params_match(hndl, b_params,
                                                        match_b_node_index[i]);
        if (ret == 0)
        {
            *b_node_index_ptr = match_b_node_index[i];
            *first_a_node_index_ptr = match_first_a_node_index[i];
            return 0;
        }
    }

    if (match_index_count != 0)
    {
            *b_node_index_ptr = match_b_node_index[0];
            *first_a_node_index_ptr = match_first_a_node_index[0];
    }

    /* Set first_queue_index to NO_PREDEFINED_RANGE */
    /* Ideally we should always get a match. Should not enter below case */
    if((b_node_index >= ctl->tm_total_b_nodes) ||
       (b_node_index < (uint32_t)c_node->mapping.childLo) ||
       (b_node_index > (uint32_t)c_node->mapping.childHi))
    {
        *first_a_node_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
    }
    else
    {
        b_node = &ctl->tm_b_node_array[b_node_index];
        if(b_node == NULL)
        {
            return 1;
        }

        if ((b_node->mapping.childHi >= b_node->mapping.childLo) &&
            ((uint32_t)(b_node->mapping.childHi - b_node->mapping.childLo) != (num_children - 1)))
        {
            *first_a_node_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
        }
    }

    if(b_node != NULL)
    {
        *first_a_node_index_ptr = (uint32_t)b_node->mapping.childLo;
    }

    return 0;
}


/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_c_node_to_port_index_fetch_is_match
(
    tm_handle   hndl,
    uint32_t    parent_port_index,
    struct tm_c_node_params *c_params,
    uint32_t    c_node_index,
    uint32_t    *first_b_node_index_ptr
)
{
    struct tm_c_node    *c_node = NULL;
    uint32_t num_children = c_params->num_of_children;
    TM_CTL(ctl, hndl);

    c_node = &ctl->tm_c_node_array[c_node_index];
    if ((c_node->mapping.nodeStatus != TM_NODE_USED) &&
        ((uint32_t)c_node->mapping.nodeParent == parent_port_index) &&
        (c_node->mapping.childLo <= c_node->mapping.childHi) &&
        ((uint32_t)(c_node->mapping.childHi - c_node->mapping.childLo) == num_children-1))
    {
        *first_b_node_index_ptr = (uint32_t)c_node->mapping.childLo;
        return 0;
    }
    return 1;
}

/**
 * @retval     0 - match
 *             1 - no match
 */
static int tm_create_c_node_to_port_index_fetch_c_node_params_match
(
    tm_handle   hndl,
    struct tm_c_node_params *c_params,
    uint32_t    c_node_index
)
{
    struct tm_c_node    *c_node = NULL;
    uint32_t    isDropProfilesMatch = 1;
    uint32_t    i;
    TM_CTL(ctl, hndl);

    for (i=0; i<TM_WRED_COS; i++)
    {
        if (ctl->tm_c_lvl_drop_prof_ptr[i][c_node_index] != c_params->wred_profile_ref[i])
        {
            isDropProfilesMatch = 0;
            break;
        }
    }

    c_node = &ctl->tm_c_node_array[c_node_index];
    if ((c_node->dwrr_quantum        == c_params->quantum) &&
        (c_node->elig_prio_func      == c_params->elig_prio_func_ptr) &&
        (c_node->shaping_profile_ref == c_params->shaping_profile_ref) &&
        (isDropProfilesMatch == 1))
    {
        return 0;
    }

    return 1;
}


/**
 * @brief 1. Port node can have many child a nodes.
 *        2. Match a node params from hardware with the params passed to
 *          identify appropriate index.
 *        3. If more than one match is found, passed index is chosen if available
 *          otherwise the smallest index is chosen.
 */
int tm_create_c_node_to_port_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_port_index,
    struct tm_c_node_params *c_params,
    uint32_t    *c_node_index_ptr,
    uint32_t    *first_b_node_index_ptr
)
{
    struct tm_c_node    *c_node = NULL;
    struct tm_port    *port = NULL;
    uint32_t c_node_index = *c_node_index_ptr;
    uint32_t first_b_node_index = 0;
    uint32_t i;
    uint32_t num_children = c_params->num_of_children;
    int ret = 0;

    uint32_t match_c_node_index[TM_NODE_CHILDREN_MAX];
    uint32_t match_first_b_node_index[TM_NODE_CHILDREN_MAX];
    uint8_t match_index_valid[TM_NODE_CHILDREN_MAX];
    uint32_t match_index_count = 0;

    TM_CTL(ctl, hndl);

    tm_memset(match_c_node_index, 0, sizeof(match_c_node_index));
    tm_memset(match_first_b_node_index, 0, sizeof(match_first_b_node_index));
    tm_memset(match_index_valid, 0, sizeof(match_index_valid));

    /* check whether input b_node index is valid */
    if (parent_port_index >= ctl->tm_total_b_nodes)
    {
        return -EINVAL;
    }

    /* check b_node mapping parameters are valid */
    port = &ctl->tm_port_array[parent_port_index];
    if (((uint32_t)port->mapping.childLo >= ctl->tm_total_c_nodes) ||
        ((uint32_t)port->mapping.childHi >= ctl->tm_total_c_nodes) ||
        (port->mapping.childHi < port->mapping.childLo))
    {
        return -EINVAL;
    }

    match_index_count = 0;
    /* check whether we can allocate at given index */
    if((c_node_index < ctl->tm_total_c_nodes) &&
       (c_node_index >= (uint32_t)port->mapping.childLo) &&
       (c_node_index <= (uint32_t)port->mapping.childHi))
    {
        ret = tm_create_c_node_to_port_index_fetch_is_match (hndl, parent_port_index, c_params,
                                                         c_node_index, &first_b_node_index);
        if (ret == 0)
        {
            match_c_node_index[match_index_count] = c_node_index;
            match_first_b_node_index[match_index_count] = first_b_node_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* Check whether we can allocate at any other index */
    for (i = (uint32_t)port->mapping.childLo; i <= (uint32_t)port->mapping.childHi; i++)
    {
        /* Already checked */
        if (i == c_node_index)
        {
            continue;
        }

        ret = tm_create_c_node_to_port_index_fetch_is_match(hndl, parent_port_index, c_params,
                                                         i, &first_b_node_index);

        if (ret == 0)
        {
            match_c_node_index[match_index_count] = i;
            match_first_b_node_index[match_index_count] = first_b_node_index;
            match_index_valid[match_index_count] = 1;
            match_index_count ++;
        }
    }

    /* For all matched mapping indexes, find if the c_node params are also a match */
    for (i=0; i<match_index_count; i++)
    {
        ret = tm_create_c_node_to_port_index_fetch_c_node_params_match(hndl, c_params,
                                                        match_c_node_index[i]);
        if (ret == 0)
        {
            *c_node_index_ptr = match_c_node_index[i];
            *first_b_node_index_ptr = match_first_b_node_index[i];
            return 0;
        }
    }

    if (match_index_count != 0)
    {
            *c_node_index_ptr = match_c_node_index[0];
            *first_b_node_index_ptr = match_first_b_node_index[0];
    }

    /* Set first_b_node_index to NO_PREDEFINED_RANGE */
    /* Ideally we should always get a match. Should not enter below case */
    if((c_node_index >= ctl->tm_total_c_nodes) ||
       (c_node_index < (uint32_t)port->mapping.childLo) ||
       (c_node_index > (uint32_t)port->mapping.childHi))
    {
        *first_b_node_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
    }
    else
    {
        c_node = &ctl->tm_c_node_array[c_node_index];

        if(c_node == NULL)
        {
            return (1);
        }

        if ((c_node->mapping.childHi >= c_node->mapping.childLo) &&
            ((uint32_t)(c_node->mapping.childHi - c_node->mapping.childLo) != (num_children - 1)))
        {
            *first_b_node_index_ptr = (uint32_t)NO_PREDEFINED_RANGE;
        }
    }

    if(c_node != NULL)
    {
        *first_b_node_index_ptr = (uint32_t)c_node->mapping.childLo;
    }

    return 0;
}
