/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
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
* @file flexeAgentTypes.h
*
* @brief FLEXE AGENT types definition
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <printf.h>

#ifndef FLEXE_AGENT_TYPES_H
#define FLEXE_AGENT_TYPES_H

#ifdef FLEXE_AGENT_IN_CPSS
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#else

#ifdef ASIC_SIMULATION
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#include <common.h>

/* typedefs */
typedef unsigned int        GT_U32;
typedef int                 GT_BOOL;
typedef void                GT_VOID;
typedef char                GT_CHAR;

typedef int8_t              GT_8;
typedef int16_t             GT_16;
typedef int32_t             GT_32;
typedef int64_t             GT_64;

typedef uint8_t             GT_U8;
typedef uint16_t            GT_U16;
typedef uint32_t            GT_U32;

typedef GT_U32 GT_STATUS;


#ifndef NULL
#define NULL ((void*)0)
#endif

#undef IN
#define IN
#undef OUT
#define OUT
#undef INOUT
#define INOUT

#define GT_TRUE  1
#define GT_FALSE 0

/* -- generic return codes -- */
#define GT_ERROR                    (-1)
#define GT_OK                       (0x00) /* Operation succeeded */
#define GT_FAIL                     (0x01) /* Operation failed    */
#define GT_BAD_VALUE                (0x02) /* Illegal value        */
#define GT_OUT_OF_RANGE             (0x03) /* Value is out of range*/
#define GT_BAD_PARAM                (0x04) /* Illegal parameter in function called  */
#define GT_BAD_PTR                  (0x05) /* Illegal pointer value                 */
#define GT_BAD_SIZE                 (0x06) /* Illegal size                          */
#define GT_BAD_STATE                (0x07) /* Illegal state of state machine        */
#define GT_SET_ERROR                (0x08) /* Set operation failed                  */
#define GT_GET_ERROR                (0x09) /* Get operation failed                  */
#define GT_CREATE_ERROR             (0x0A) /* Fail while creating an item           */
#define GT_NOT_FOUND                (0x0B) /* Item not found                        */
#define GT_NO_MORE                  (0x0C) /* No more items found                   */
#define GT_NO_SUCH                  (0x0D) /* No such item                          */
#define GT_TIMEOUT                  (0x0E) /* Time Out                              */
#define GT_NO_CHANGE                (0x0F) /* The parameter is already in this value*/
#define GT_NOT_SUPPORTED            (0x10) /* This request is not support           */
#define GT_NOT_IMPLEMENTED          (0x11) /* This request is not implemented       */
#define GT_NOT_INITIALIZED          (0x12) /* The item is not initialized           */
#define GT_NO_RESOURCE              (0x13) /* Resource not available (memory ...)   */
#define GT_FULL                     (0x14) /* Item is full (Queue or table etc...)  */
#define GT_EMPTY                    (0x15) /* Item is empty (Queue or table etc...) */
#define GT_INIT_ERROR               (0x16) /* Error occurred while INIT process     */
#define GT_NOT_READY                (0x1A) /* The other side is not ready yet       */
#define GT_ALREADY_EXIST            (0x1B) /* Tried to create existing item         */
#define GT_OUT_OF_CPU_MEM           (0x1C) /* Cpu memory allocation failed.         */
#define GT_ABORTED                  (0x1D) /* Operation has been aborted.           */
#define GT_NOT_APPLICABLE_DEVICE    (0x1E) /* API not applicable to device , can
                                                be returned only on devNum parameter  */
#endif

#ifndef FLEXE_AGENT_IN_CPSS
    #define FLEXE_TASK                GT_VOID
#else
    #define FLEXE_TASK                unsigned __TASKCONV
#endif

#define FLEXE_NA_8BIT             (0xFF)
#define FLEXE_NA_8BIT_SIGNED      (0x7F)
#define FLEXE_NA_16BIT            (0xFFFF)
#define FLEXE_NA_16BIT_SIGNED     (0x7FFF)
#define FLEXE_NA_32BIT            (0xFFFFFFFF)
#define FLEXE_NA_32BIT_SIGNED     (0x7FFFFFFF)
#define FLEXE_PTR_ENTRY_UNUSED    (0xFFFFFFFB)

#undef   FLEXE_DEBUG_PRINT_EN
#undef   FLEXE_INFO_PRINT_EN
#define FLEXE_ERR_PRINT_EN

#ifdef FLEXE_DEBUG_PRINT_EN
    #define FLEXE_DEBUG_MSG_PRINT_MAC(debugMsg, ...) \
        printf("\n[DEBUG, %s, %d]. " debugMsg, __func__ , __LINE__, ##__VA_ARGS__);

    #define FLEXE_DEBUG_PRINT_MAC(debugMsg, ...) \
        printf(debugMsg "\n", ##__VA_ARGS__);
#else
    #define FLEXE_DEBUG_MSG_PRINT_MAC(debugMsg, ...)
    #define FLEXE_DEBUG_PRINT_MAC(debugMsg, ...)
#endif

#ifdef FLEXE_INFO_PRINT_EN
    #define FLEXE_INFO_MSG_PRINT_MAC(infoMsg, ...) \
        printf("\n[INFO, %s, %d]. "  infoMsg, __func__ , __LINE__, ##__VA_ARGS__);
#else
    #define FLEXE_INFO_MSG_PRINT_MAC(infoMsg, ...)
#endif

#ifdef FLEXE_ERR_PRINT_EN
    #define FLEXE_ERR_MSG_PRINT_MAC(errMsg, ...) \
        printf("\n[ERROR, %s. %d]. "  errMsg, __func__ , __LINE__, ##__VA_ARGS__);
#define FLEXE_ERR_CHECK_AND_PRINT_MAC(rc, errMsg, ...) \
        if(rc)                                         \
        {                                              \
            printf("\n[ERROR (%d), %s. %d]. "  errMsg "\n", rc, __func__ , __LINE__, ##__VA_ARGS__); \
        }
#define FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, errMsg, ...) \
        if(rc)                                         \
        {                                              \
            printf("\n[ERROR (%d), %s. %d]. "  errMsg "\n", rc, __func__ , __LINE__, ##__VA_ARGS__); \
            return rc;                                 \
        }
#else
    #define FLEXE_ERR_MSG_PRINT_MAC(errMsg, ...)
    #define FLEXE_ERR_CHECK_AND_PRINT_MAC(errMsg, ...)
    #define FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, errMsg, ...)
#endif

#define L1_LOG_ERROR_AND_RETURN_MAC(_rc, ...) \
    return _rc;

/* check NULL pointer */
#define FLEXE_NULL_PTR_CHECK_MAC(ptr) \
    if((ptr) == NULL) { \
        FLEXE_ERR_MSG_PRINT_MAC("Bad pointer ");\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG); \
    }

#define FLEXE_NULL_PTR_CHECK_AND_RETRUN_NULL_MAC(ptr) \
    if((ptr) == NULL) { \
        FLEXE_ERR_MSG_PRINT_MAC("Bad pointer ");\
        return(NULL); \
    }

#define FLEXE_NULL_PTR_CHECK_AND_SKIP_MAC(ptr) \
    if((ptr) == NULL) { \
        FLEXE_ERR_MSG_PRINT_MAC("Bad pointer ");\
        return(GT_OK); \
    }

#define FLEXE_EXCEED_OUT_OF_BOUNDS_MAC(tested_value, max_bounds) \
    if((tested_value) > (max_bounds)) { \
        FLEXE_ERR_MSG_PRINT_MAC("Exceed of bounds (%d)",tested_value);\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG); \
    }

/* check out of bounds for maximum value */
#define FLEXE_OUT_OF_BOUNDS_CHECK_MAC(tested_value, max_bounds) \
    if((tested_value) >= (max_bounds)) { \
        FLEXE_ERR_MSG_PRINT_MAC("Out of bounds (%d)",tested_value);\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG); \
    }

/* check out of bounds for minimum value */
#define FLEXE_OUT_OF_MIN_BOUNDS_CHECK_MAC(tested_value, min_bounds) \
    if((tested_value) < (min_bounds)) { \
        FLEXE_ERR_MSG_PRINT_MAC("Out of bounds (%d)",tested_value);\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG); \
    }

#define FLEXE_CHECK_STATUS_CONDITION_MAC(status, condition, rc) \
    if ((status) != (condition)) { \
        return (rc); \
    }

#define FLEXE_CHECK_STATUS_MAC(func) \
    { \
        GT_STATUS funcRetVal; \
        funcRetVal = (func); \
        if (funcRetVal != GT_OK) { \
            FLEXE_ERR_MSG_PRINT_MAC("Func failed (%d)",funcRetVal); \
            return (funcRetVal); \
        } \
    }

#define FLEXE_CHECK_STATUS_SEMAPHORE_RETURN(func, SEM, condition) \
    { \
        GT_STATUS funcRetVal; \
        funcRetVal = (func); \
        if (funcRetVal != (condition)) { \
            FLEXE_ERR_MSG_PRINT_MAC("Func failed (%d)",funcRetVal); \
            xSemaphoreGive( SEM ); \
            return (funcRetVal); \
        } \
    }

#define FLEXE_CHECK_STATUS_SPECIFIC_MAC(status, rc) \
    if ((status) != GT_OK) { \
        return (rc); \
    }

#define FLEXE_MAX_VALUE_GET_MAC(x, y, result) \
    if (((FLEXE_U32)(x)) >= ((FLEXE_U32)(y))) \
    { \
        result = (x); \
    } \
    else \
    { \
        result = (y); \
    }

#define FLEXE_ABS(val) \
    ((val) < 0) ? -(val) : (val)

#ifndef  FLEXE_UNUSED_PARAM
    #define FLEXE_UNUSED_PARAM(x) (GT_VOID)x
#endif


/* Define single bit masks.             */
#define BIT_0           (0x1)
#define BIT_1           (0x2)
#define BIT_2           (0x4)
#define BIT_3           (0x8)
#define BIT_4           (0x10)
#define BIT_5           (0x20)
#define BIT_6           (0x40)
#define BIT_7           (0x80)
#define BIT_8           (0x100)
#define BIT_9           (0x200)
#define BIT_10          (0x400)
#define BIT_11          (0x800)
#define BIT_12          (0x1000)
#define BIT_13          (0x2000)
#define BIT_14          (0x4000)
#define BIT_15          (0x8000)
#define BIT_16          (0x10000)
#define BIT_17          (0x20000)
#define BIT_18          (0x40000)
#define BIT_19          (0x80000)
#define BIT_20          (0x100000)
#define BIT_21          (0x200000)
#define BIT_22          (0x400000)
#define BIT_23          (0x800000)
#define BIT_24          (0x1000000)
#define BIT_25          (0x2000000)
#define BIT_26          (0x4000000)
#define BIT_27          (0x8000000)
#define BIT_28          (0x10000000)
#define BIT_29          (0x20000000)
#define BIT_30          (0x40000000)
#define BIT_31          (0x80000000)


/* Sets the field located at the specified offset & length in data.     */
#define FLEXE_U32_BITS_SET(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* Returns the info located at the specified offset & length in data */
#define FLEXE_U32_BITS_GET(data,offset,length)           \
        (((data) >> (offset)) & BIT_MASK_MAC(length))

/* Semaphores */

#define FLEXE_SEMA_SHIM_CORE       0
#define FLEXE_SEMA_SHIM_OH         1
#define FLEXE_SEMA_XC_CORE         2
#define FLEXE_SEMA_XC_OAM          3
#define FLEXE_SEMA_CODER           4
#define FLEXE_SEMA_LAST            5
#define FLEXE_MAX_SEMA             128

#define flexeAgentOsMemSet  memset

#define FLEXE_NUM_INSTANCES_MAX 8
#define FLEXE_NUM_SHIMS_MAX     2
#define FLEXE_NUM_TIlES_MAX     2
#define FLEXE_NUM_SLOTS_MAX     80

/* [TBD] Update later if required */
#define FLEXE_NUM_CLIENTS_MAX   80

typedef enum {
    FLEXE_AGENT_CLIENT_TYPE_L1_E,
    FLEXE_AGENT_CLIENT_TYPE_L2_E,
} FLEXE_AGENT_CLIENT_TYPE_ENT;

typedef enum {
    FLEXE_AGENT_MAC_TYPE_25G_E,
    FLEXE_AGENT_MAC_TYPE_50G_E,
    FLEXE_AGENT_MAC_TYPE_100G_E,
    FLEXE_AGENT_MAC_TYPE_200G_E,
    FLEXE_AGENT_MAC_TYPE_400G_E,
} FLEXE_AGENT_MAC_TYPE_ENT;

typedef enum {
    FLEXE_AGENT_PIZZA_SELECT_A_E,
    FLEXE_AGENT_PIZZA_SELECT_B_E
} FLEXE_AGENT_PIZZA_SELECT_ENT;

typedef enum {
    FLEXE_AGENT_XC_FLOW_TYPE_A_E,
    FLEXE_AGENT_XC_FLOW_TYPE_B_E,
    FLEXE_AGENT_XC_FLOW_TYPE_C_E
} FLEXE_AGENT_XC_FLOW_TYPE_ENT;

typedef struct {
    FLEXE_AGENT_XC_FLOW_TYPE_ENT  flowType;
    GT_U32      channelId;
    GT_U32      peerChannelId;
} FLEXE_AGENT_XC_CALENDAR_STC;

typedef struct {
    GT_BOOL                     shimId;
    GT_BOOL                     updateCal;
    GT_U32                      channelId;
    GT_U32                      clientNum;
    GT_U32                      peerChannelId;
    GT_U32                      peerClientShimId;
    FLEXE_AGENT_CLIENT_TYPE_ENT clientType;
    GT_U32                      macNum;
    GT_U32                      clientSpeed;
    FLEXE_AGENT_MAC_TYPE_ENT    macType;
    GT_U8                       pizza[FLEXE_NUM_SLOTS_MAX];
    FLEXE_AGENT_XC_CALENDAR_STC xcCal[FLEXE_NUM_CLIENTS_MAX];
} FLEXE_AGENT_CLIENT_CONFIG_STC;

typedef enum {
    FLEXE_AGENT_GROUP_INSTANCE_TYPE_50G_E,
    FLEXE_AGENT_GROUP_INSTANCE_TYPE_100G_E
} FLEXE_AGENT_GROUP_INSTANCE_TYPE_ENT;

typedef struct {
    GT_U8       id;
    /* For 100G two physical instances are combined to 1 */
    GT_U8       id1;
    GT_U8       bondMask;
    GT_U32      instanceNum;
    FLEXE_AGENT_GROUP_INSTANCE_TYPE_ENT  type;
} FLEXE_AGENT_GROUP_INSTANCE_STC;

typedef struct {
    GT_U8                          shimId;
    GT_U32                         numInstances;
    FLEXE_AGENT_GROUP_INSTANCE_STC instance[FLEXE_NUM_INSTANCES_MAX];
    GT_U8                          interleaveCfg;
} FLEXE_AGENT_GROUP_CONFIG_STC;

typedef struct {
    GT_U32                          numClients;
    FLEXE_AGENT_CLIENT_CONFIG_STC   clientCfg[FLEXE_NUM_CLIENTS_MAX];
} FLEXE_AGENT_CLIENTS_INFO_STC;

#endif /*FLEXE_AGENT_TYPES_H*/

