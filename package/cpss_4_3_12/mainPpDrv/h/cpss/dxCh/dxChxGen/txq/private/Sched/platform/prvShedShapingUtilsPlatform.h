/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief shaping calculation constants  &tm_shaping_uitils interface.
 *
* @file tm_shaping_utils_platform.h
*
* $Revision: 1.0 $
 */
#ifndef SCHED_SHAPING_UTILS_PLATFORM_H
#define SCHED_SHAPING_UTILS_PLATFORM__H


/******************************************************************
*       scheduling  scheme for BC2
* according to preliminary analysis
*******************************************************************/
#define MAX_DIVIDER_EXP           7       /* maximum value of divider exponent nodes */
#define SIP_7_MAX_DIVIDER_EXP     5
#define SIP_7_P_LEVEL_MAX_DIVIDER_EXP     SIP_7_MAX_DIVIDER_EXP



/*******************************************************************************************************
*  shaping configuration definitions
********************************************************************************************************/


#define     TOKEN_WIDTH_BITS            11

#define     MAX_POSSIBLE_TOKEN          (1<<TOKEN_WIDTH_BITS)-1

/* all values below  provides max b/w  100Gbit/s   and maximum accuracy */

#define     PORT_OPTIMAL_RES_EXP        4


/* FALCON unique defines*/

#define     FALCON_QUEUE_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_A_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_B_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_C_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_P_NODE_OPTIMAL_RES_EXP       7



#define     FALCON_PORT_PERIODS_MAX         128

#define     SIP6_TOKEN_WIDTH_BITS         12
#define     SIP7_TOKEN_WIDTH_BITS         13

#define FALCON_Q_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_A_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_B_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_C_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_P_UNIT               100000       /* -  100 kbit/second resolution */


#define HAWK_Q_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_A_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_B_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_C_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_P_UNIT               50000       /* -  50 kbit/second resolution */


#define PHOENIX_Q_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_A_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_B_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_C_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_P_UNIT            25000       /* -  25 kbit/second resolution */


#define IRONMAN_Q_UNIT            25000       /* -  25 kbit/second resolution */
#define IRONMAN_A_UNIT            25000       /* -  25 kbit/second resolution */
#define IRONMAN_B_UNIT            25000       /* - 25 kbit/second resolution */
#define IRONMAN_C_UNIT            25000       /* - 25 kbit/second resolution */
#define IRONMAN_P_UNIT            25000       /* -  25 kbit/second resolution */


#define SEAHAWK_Q_UNIT               25000       /* -  25 kbit/second resolution */
#define SEAHAWK_A_UNIT               25000       /* -  25 kbit/second resolution */
#define SEAHAWK_B_UNIT               25000       /* -  25 kbit/second resolution */
#define SEAHAWK_C_UNIT               25000       /* -  25 kbit/second resolution */
#define SEAHAWK_P_UNIT               500000       /* -  500 kbit/second resolution */


#define FALCON_OPTIMAL_NODE_FREQUENCY     200000000
#define HAWK_OPTIMAL_NODE_FREQUENCY       204000000
#define PHOENIX_OPTIMAL_NODE_FREQUENCY    150000000
#define IRONMAN_OPTIMAL_NODE_FREQUENCY    150000000
#define SEAHAWK_OPTIMAL_NODE_FREQUENCY    350000000









#endif   /* TM_SHAPING_UTILS_PLATFORM_H */
