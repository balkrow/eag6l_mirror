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
 * @brief Scheduler  eligable functions defenition
 *
* @file prvCpssEligFuncDefs.h
*
* $Revision: 1.0 $
 */

#ifndef     ELIG_FUNC_DEFS_H
#define     ELIG_FUNC_DEFS_H

enum  prvShedEligFuncNode {

    /** Node is always eligible in priority 0 **/
    PRV_SCHED_ELIG_N_FP0 = 0,
    /** Node is always eligible in priority 1 **/
    PRV_SCHED_ELIG_N_FP1 = 1,
    /** Node is always eligible in priority 2 **/
    PRV_SCHED_ELIG_N_FP2 = 2,
    /** Node is always eligible in priority 3 **/
    PRV_SCHED_ELIG_N_FP3 = 3,
    /** Node is always eligible in priority 4 **/
    PRV_SCHED_ELIG_N_FP4 = 4,
    /** Node is always eligible in priority 5 **/
    PRV_SCHED_ELIG_N_FP5 = 5,
    /** Node is always eligible in priority 6 **/
    PRV_SCHED_ELIG_N_FP6 = 6,
    /** Node is always eligible in priority 7 **/
    PRV_SCHED_ELIG_N_FP7 = 7,
    /** Node is eligible in priority 0 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP0 = 8,
    /** Node is eligible in priority 1 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP1 = 9,
    /** Node is eligible in priority 2 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP2 = 10,
    /** Node is eligible in priority 3 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP3 = 11,
    /** Node is eligible in priority 4 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP4 = 12,
    /** Node is eligible in priority 5 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP5 = 13,
    /** Node is eligible in priority 6 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP6 = 14,
    /** Node is eligible in priority 7 when minTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_FP7 = 15,
    /** Eligible function propagated with MaxTB include MinTB shaping **/
    PRV_SCHED_ELIG_N_MAX_INC_MIN_SHP = 26,
    /** Node is always eligible in propagated priority **/
    PRV_SCHED_ELIG_N_PP = 27,
    /** Node is eligible when MinTB is positive **/
    PRV_SCHED_ELIG_N_MIN_SHP_PP = 28,
    /** Eligible function with propagated priority and shaping **/
    PRV_SCHED_ELIG_N_PP_SHP = 29,
    /** Scheduled in propageted priority ,then muxed out in priority 0**/
    PRV_SCHED_ELIG_N_SHP_4P_MIN_4P_MAX = 30,
/** Scheduled in propageted priority ,then muxed out in priority 0 with shaping**/
    PRV_SCHED_ELIG_N_SHP_PP_TB = 31,
    /** Eligible function with min max shaping,SchedPrio0 = Prop0(Min)+Prop0..7(Max)
     * SchedPrio1..7 = SchedPrio1..7(Min), PropOut = Prop  **/
    PRV_SCHED_ELIG_N_SHP_PP_MAX_TB_0 = 32,
    /*Eligible only if more than min and less than max ,fixed priority 0)*/
    PRV_SCHED_ELIG_N_MAX_LIM_SHP_FP0 = 33,
/** PPA - Propagated priority according to RevA . i.e. - only
 * propagated priority 0...3 are supported. when node gets propagated
 * priority 4...7 it is not eligible **/
        /** Eligible Priority 1 **/
    PRV_SCHED_ELIG_N_PRIO1 = 34,
    /** Eligible function priority 5 **/
    PRV_SCHED_ELIG_N_PRIO5 = 35,
    /** Eligible function shaping **/
    PRV_SCHED_ELIG_N_SHP = 36,
    /** Eligible function min shaping **/
    PRV_SCHED_ELIG_N_MIN_SHP = 37,
    /** Eligible function propagated priority **/
    PRV_SCHED_ELIG_N_PPA = 38,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) **/
    PRV_SCHED_ELIG_N_PPA_SP = 39,
    /** Eligible function propagated with shaping  **/
    PRV_SCHED_ELIG_N_PPA_SHP = 40,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) with min shaper **/
    PRV_SCHED_ELIG_N_PPA_SP_MIN_SHP = 41,
    /** Eligible function propagated priority and shaping with shaper
     * ignore for propagated priority = 3 **/
    PRV_SCHED_ELIG_N_PPA_SHP_IGN = 42,
    /** Eligible function propagated priority with strict priority
     * (priority = 5) with min shaper with shaper
     * ignore for propagated priority = 3 **/
    PRV_SCHED_ELIG_N_PPA_MIN_SHP_SP_IGN = 43,
    PRV_SCHED_ELIG_DEQ_DISABLE = 63
};



typedef enum  {
        PRV_SCHED_ELIG_FUNC_GROUP_NORMAL_E =0 ,
        PRV_SCHED_ELIG_FUNC_GROUP_SHAPING_E = 1 ,
        PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_WITH_SHAPING_E = 2,
        PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_E = 3 ,
        PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE1_E = 4,
        PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE1_WITH_SHAPING_E = 5
    }prvShedEligFuncGroupQueue;


/** Eligible functions for queue nodes enumerator .
*/
typedef enum  {

    /*PRV_SCHED_ELIG_FUNC_GROUP_NORMAL_E  =  0*/

    /** Eligible function priority 0 **/
    PRV_SCHED_ELIG_Q_PRIO0 = 0,
    /** Eligible function priority 1 **/
    PRV_SCHED_ELIG_Q_PRIO1 = 1,
    /** Eligible function priority 2 **/
    PRV_SCHED_ELIG_Q_PRIO2 = 2,
    /** Eligible function priority 3 **/
    PRV_SCHED_ELIG_Q_PRIO3 = 3,
    /** Eligible function priority 4 **/
    PRV_SCHED_ELIG_Q_PRIO4 = 4,
    /** Eligible function priority 5 **/
    PRV_SCHED_ELIG_Q_PRIO5 = 5,
    /** Eligible function priority 6 **/
    PRV_SCHED_ELIG_Q_PRIO6 = 6,
    /** Eligible function priority 7 **/
    PRV_SCHED_ELIG_Q_PRIO7 = 7,

     /*PRV_SCHED_ELIG_FUNC_GROUP_SHAPING_E  =  1*/


    /** Eligible function min shaping priority 0 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0 = 8,
    /** Eligible function min shaping priority 1 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO1 = 9,
    /** Eligible function min shaping priority 2 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO2 = 10,
    /** Eligible function min shaping priority 3 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO3 = 11,
    /** Eligible function min shaping priority 4 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO4 = 12,
    /** Eligible function min shaping priority 5 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO5 = 13,
    /** Eligible function min shaping priority 6 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO6 = 14,
    /** Eligible function min shaping priority 7 **/
    PRV_SCHED_ELIG_Q_MIN_SHP_PRIO7 = 15,

    /*PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_WITH_SHAPING_E  = 2*/

    /** Eligible function min max shaping, scheduling priority 0/0, propagated priority 0/0 **/
    PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0_WITH_SHAPING = 16,
    /** Eligible function min max shaping, scheduling priority 1/0, propagated priority 1/0 **/
    PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0_WITH_SHAPING = 17,
    /** Eligible function min max shaping, scheduling priority 2/0, propagated priority 2/0 **/
    PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0_WITH_SHAPING = 18,
    /** Eligible function min max shaping, scheduling priority 3/0, propagated priority 3/0 **/
    PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0_WITH_SHAPING = 19,
    /** Eligible function min max shaping, scheduling priority 4/0, propagated priority 4/0 **/
    PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0_WITH_SHAPING = 20,
    /** Eligible function min max shaping, scheduling priority 5/0, propagated priority 5/0 **/
    PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0_WITH_SHAPING = 21,
    /** Eligible function min max shaping, scheduling priority 6/0, propagated priority 6/0 **/
    PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0_WITH_SHAPING = 22,
    /** Eligible function min max shaping, scheduling priority 7/0, propagated priority 7/0 **/
    PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0_WITH_SHAPING = 23,

    /*PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE0_E  = 3*/

    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 0 */
    PRV_SCHED_ELIG_Q0_MIN_BW_TYPE0 = 24,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 1 */
    PRV_SCHED_ELIG_Q1_MIN_BW_TYPE0 = 25,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 2 */
    PRV_SCHED_ELIG_Q2_MIN_BW_TYPE0 = 26,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 3*/
    PRV_SCHED_ELIG_Q3_MIN_BW_TYPE0 = 27,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 4 */
    PRV_SCHED_ELIG_Q4_MIN_BW_TYPE0 = 28,
   /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 5 */
    PRV_SCHED_ELIG_Q5_MIN_BW_TYPE0 = 29,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 6 */
    PRV_SCHED_ELIG_Q6_MIN_BW_TYPE0 = 30,
    /** Eligible function min  BW shaping with priotity set to 0 after guaranteed BW is reached  for priority group 7 */
    PRV_SCHED_ELIG_Q7_MIN_BW_TYPE0 = 31,

     /*PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE1_E  = 4*/
    /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 0 */
    PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1 = 32,
     /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 1 */
    PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1 = 33,
     /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 2 */
    PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1 = 34,
     /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 3 */
    PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1 = 35,
    /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 4 */
    PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1 = 36,
    /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 5 */
    PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1 = 37,
    /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 6 */
    PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1 = 38,
    /** Eligible function min  BW  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 7 */
    PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1 = 39,

     /*PRV_SCHED_ELIG_FUNC_GROUP_MIN_BW_TYPE1_E  = 5*/
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group   0 */
    PRV_SCHED_ELIG_Q0_MIN_BW_TYPE1_WITH_SHAPING = 40,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 1 */
    PRV_SCHED_ELIG_Q1_MIN_BW_TYPE1_WITH_SHAPING = 41,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 2 */
    PRV_SCHED_ELIG_Q2_MIN_BW_TYPE1_WITH_SHAPING = 42,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 3 */
    PRV_SCHED_ELIG_Q3_MIN_BW_TYPE1_WITH_SHAPING = 43,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 4 */
    PRV_SCHED_ELIG_Q4_MIN_BW_TYPE1_WITH_SHAPING = 44,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 5 */
    PRV_SCHED_ELIG_Q5_MIN_BW_TYPE1_WITH_SHAPING = 45,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 6 */
    PRV_SCHED_ELIG_Q6_MIN_BW_TYPE1_WITH_SHAPING = 46,
    /** Eligible function min  BW and shaoing  with priotity set to  queue offset  after guaranteed BW is reached  for priority group 7 */
    PRV_SCHED_ELIG_Q7_MIN_BW_TYPE1_WITH_SHAPING = 47,

    PRV_SCHED_ELIG_Q_DEQ_DIS_E =63,
    PRV_SCHED_ELIG_Q_LAST
}prvShedEligFuncQueue;


#endif   /* ELIG_FUNC_DEFS_H */

