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
* @file prvCpssDxChCuckoo.h
*
* @brief PRV CPSS Cuckoo declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChCuckooh
#define __prvCpssDxChCuckooh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/* max depths according to cuckoo algorithm definitions for {numOfBanks,numOfLines} */
#define _16_hash_depth   2
#define _8_hash_depth    3
#define _5_hash_depth    5
#define _4_hash_depth    5
#define _2_hash_depth    50

/* TBD AAS : The max key size needs updated when Exact manager is supported in AAS */
#define PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS           (CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS - 6)
#define PRV_CPSS_DXCH_CUCKOO_INVALID_BANK_CNS           0x1F
#define PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS        32
#define PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS             1024

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC
*
* @brief hash entry information.
*/
typedef struct
{
    GT_U32 line;
    GT_U32 bank;
}PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC;

/**
* @enum PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT
*
*  @brief cuckoo supported clients
*/
typedef enum{

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E,

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E

} PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT;

/**
* @enum PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT
*
* @brief Cuckoo key size
*/
typedef enum{

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_19B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_33B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E /*TBD need to add also fdb later*/

} PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC
*
* @brief log operation for inserting new entry to HW.
*/
typedef struct
{
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* aligned to 32 bits */

    GT_U32 sizeInBytes  : 6;    /* size in bytes of data; supporting max size 47 bytes */
    GT_U32 stage        : 7;    /* depth of recurse; supporting max stage 100 */
    GT_U32 line         : 14;   /* line num in DB; supporting max 16K lines */
    GT_U32 bank         : 4;    /* bank num in DB; supporting max 16 banks; values (0..15) */
    GT_U32 isFree       : 1;    /* is the entry free; true / false */
    /* aligned to 32 bits */

    GT_U32 isLock       : 1;    /* is the entry locked; true / false */
    GT_U32 isFirst      : 1;    /* is this the 'head' of the entry; true / false */
    GT_U32 lookupNumber : 1;    /* lookup number; supporting 2 lookups */

    /* left with 'spare' of : 32-(1+1+1) = 29 bits */
}PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC
*
* @brief entry information for recurse.
*/
typedef struct
{
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* aligned to 32 bits */

    GT_U32 sizeInBytes  : 6;    /* size in bytes of data; supporting max size 47 bytes */
    GT_U32 depth        : 7;    /* depth in recurse; supporting max 100 */
    GT_U32 line         : 14;   /* line num in DB; supporting max 16K lines */
    GT_U32 bank         : 5;    /* bank num in DB; supporting max 16 banks + invalid bank; values (0..15) + invalid value */

    /* aligned to 32 bits */
    GT_U32 lookupNumber : 1;    /* lookup number; supporting 2 lookups */

    /* left with 'spare' of : 32-1 = 31 bits */
}PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC
*
* @brief statistics for cuckoo algorithm.
*/
typedef struct
{
    GT_U32 count;
    GT_U32 sizes[PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E];
}PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC;

/**
 * @struct PRV_CPSS_DXCH_CUCKOO_DB_STC
 *
 *  @brief cuckoo information for DB
 *
*/
typedef struct
{
    /* statistics per bank */
    PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *bank_statistics;
    /* statistics per size */
    GT_U32 size_statistics[PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E];
    /* statistics number of recurse checks*/
    GT_U32 recurse_count;

    /* log operations*/
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC log_array[PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS];
    /* pointer to last occupied location in log_array. the search should always start from the end */
    GT_U32 numOfOperations;

    /* populated banks */
    GT_U32 *sorted_banks;

    /* system definition */
    GT_U32 db_lines;
    GT_U32 db_banks;

} PRV_CPSS_DXCH_CUCKOO_DB_STC;

/**
* @internal prvCpssDxChCuckooDbCallbacksInit function
* @endinternal
*
* @brief   Initialize cuckoo DB callbacks per client type.
*
* @param[in] clientType    - client type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbCallbacksInit
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType
);

/**
* @internal prvCpssDxChCuckooCallbacksBind function
* @endinternal
*
* @brief  Bind callback DB
*
* @note   APPLICABLE DEVICES:      All devices
*
*/
GT_VOID * prvCpssDxChCuckooCallbacksBind
(
    GT_VOID
);

/**
* @internal em_prvCpssDxChCuckooDbMaxDepthGet function
* @endinternal
*
* @brief Get max depth according to key size in bytes.
*
* @param[in] managerId       - manager identification
* @param[in] sizeInBytes     - key size in bytes
*
* @param[out] depthPtr       - (pointer to) depth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbMaxDepthGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U32                                  *depthPtr
);

/**
* @internal em_prvCpssDxChCuckooDbDataLookupNumberGet function
* @endinternal
*
* @brief Get [bank][line] data and lookup number from DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - number of bytes
*
* @param[out] dataPtr        - (pointer to) [bank][line] data in DB
* @param[out] lookupNumberPtr - (pointer to) [bank][line] lookup number in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbDataLookupNumberGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
);

/**
* @internal em_prvCpssDxChCuckooDbDataLookupNumberCheck function
* @endinternal
*
* @brief Try to find data in DB.
*
* @param[in] managerId            - manager identification
* @param[in] dataPtr              - (pointer to) data
* @param[in] sizeInBytes          - key size in bytes
* @param[in] lookupNumber         - lookup number
*
* @param[out] isFoundPtr          - (pointer to) whether data is
*                                   found in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
GT_STATUS em_prvCpssDxChCuckooDbDataLookupNumberCheck
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    OUT GT_BOOL                                 *isFoundPtr
);

/**
* @internal em_prvCpssDxChCuckooDbRehashEntry function
* @endinternal
*
* @brief Move entry using calculated hash locations ordered according to populated banks.
*
* @param[in] managerId            - manager identification
* @param[in] entryInfoPtr         - (pointer to) entry to add
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbRehashEntry
(
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
);

/**
* @internal em_prvCpssDxChCuckooDbLogPtrGet function
* @endinternal
*
* @brief   The function returns pointer to log array in cuckoo DB.
*
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS em_prvCpssDxChCuckooDbLogPtrGet
(
    IN  GT_U32                                      managerId,
    OUT GT_U32                                      *numOfOperationsPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC       **cuckooDbLogPtr
);

/**
* @internal em_prvCpssDxChCuckooDbLogUpdateStages function
* @endinternal
*
* @brief    Update stages in cuckoo operation log.
*
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS em_prvCpssDxChCuckooDbLogUpdateStages
(
   IN GT_U32                                   managerId
);

/**
* @internal em_prvCpssDxChCuckooDbLogArrayDelete function
* @endinternal
*
* @brief   Delete all log operations.
*
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS em_prvCpssDxChCuckooDbLogArrayDelete
(
    IN GT_U32                                   managerId
);

/**
* @internal em_prvCpssDxChCuckooDbEntryAdd function
* @endinternal
*
* @brief Set data in DB in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] *dataPtr        - (pointer to) data
* @param[in] lookupNumber    - lookup number
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbEntryAdd
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_U32                                   lookupNumber,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal em_prvCpssDxChCuckooDbEntryDelete function
* @endinternal
*
* @brief Delete entry from DB in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbEntryDelete
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal em_prvCpssDxChCuckooDbLogInfoSet function
* @endinternal
*
* @brief Set data in operation log in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] dataPtr         - (pointer to) data
* @param[in] lookupNumber    - lookup number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS em_prvCpssDxChCuckooDbLogInfoSet
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_U32                                   lookupNumber
);

/**
* @internal em_prvCpssDxChCuckooDbStatisticsPtrGet function
* @endinternal
*
* @brief   The function returns pointer to statistics in cuckoo DB.
*
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS em_prvCpssDxChCuckooDbStatisticsPtrGet
(
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC         **cuckooDbBankStPtr,
    OUT GT_U32                                      **cuckooDbSizeStPtr
);

/**
* @internal em_prvCpssDxChCuckooDbInit function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
GT_STATUS em_prvCpssDxChCuckooDbInit
(
    IN GT_U32                               managerId
);

/**
* @internal em_prvCpssDxChCuckooDbDelete function
* @endinternal
*
* @brief   Delete cuckoo DB per client type and managerId.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS em_prvCpssDxChCuckooDbDelete
(
    IN GT_U32                               managerId
);

/**
* @internal em_prvCpssDxChCuckooDbPrintStatistics function
* @endinternal
*
* @brief   Print cuckoo DB statistics per client type and
*          managerId.
*
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS em_prvCpssDxChCuckooDbPrintStatistics
(
    IN GT_U32                               managerId
);

/**
* @internal em_prvCpssDxChCuckooDbSyncStatistics function
* @endinternal
*
* @brief   Sync cuckoo DB statistics per client type and
*          managerId after failure.
*
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS em_prvCpssDxChCuckooDbSyncStatistics
(
    IN GT_U32                               managerId
);

/**
* @internal em_prvCpssDxChCuckooDbInitDebug function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId
*          for debug.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
GT_STATUS em_prvCpssDxChCuckooDbInitDebug
(
    IN GT_U32                               managerId,
    IN GT_U32                               db_banks,
    IN GT_U32                               db_lines
);

/**
* @internal em_prvCpssDxChCuckooRelocateEntriesDebug function
* @endinternal
*
* @brief    Add new entry to DB by finding free space or
*           relocate existing entries.
*
*   Entry at index Y will be copied to index Z, then new entry X will be copied to index Y
*   so traffic of existing entries won't be lossed and new entry could be added by using index X.
*
*   New entry X --> Y[i] -->Z[j]
*
* @param[in] managerId      - manager identification
*
*  NOTE: this function relocates entries only in DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*/
GT_STATUS em_prvCpssDxChCuckooRelocateEntriesDebug
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN function
* @endinternal
*
* @brief Check if [bank][line] is free in DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFreePtr      - (pointer to) whether [bank][line]
*                               is free in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_FUN function
* @endinternal
*
* @brief Get [bank][line] size in Bytes and lookup number from DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] sizeInBytesPtr      - (pointer to) [bank][line] size in bytes in DB
* @param[out] lookupNumberPtr     - (pointer to) [bank][line] lookup nuber in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_FUN function
* @endinternal
*
* @brief Get [bank][line] data and lookup number from DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - number of bytes
*
* @param[out] dataPtr        - (pointer to) [bank][line] data in DB
* @param[out] lookupNumberPtr - (pointer to) [bank][line] lookup number in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_IS_FIRST_FUN function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFirstPtr      - (pointer to) whether
*                               [bank][line] is first in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_IS_FIRST_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_FUN function
* @endinternal
*
* @brief Try to find data in DB.
*
* @param[in] managerId            - manager identification
* @param[in] dataPtr              - (pointer to) data
* @param[in] sizeInBytes          - key size in bytes
* @param[in] lookupNumber         - lookup number
*
* @param[out] isFoundPtr          - (pointer to) whether data is
*                                   found in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    OUT GT_BOOL                                 *isFoundPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOCK_SET_FUN function
* @endinternal
*
* @brief lock/unlock [bank][line] for number of banks in DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks to lock/unlock
* @param[in] lock            - whether to lock/unlock [bank][line]
*                              for number of banks in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOCK_SET_FUN)
(
    IN GT_U32                                  managerId,
    IN GT_U32                                  bank,
    IN GT_U32                                  line,
    IN GT_U32                                  size,
    IN GT_BOOL                                 lock
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN function
* @endinternal
*
* @brief Check if [bank][line] for number of banks is locked in DB.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks to check
*
* @param[out] isLockPtr      - (pointer to) whether [bank][line] for
*                               number of banks is locked in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN)
(
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_BOOL                                 *isLockPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_ADD_FUN function
* @endinternal
*
* @brief Set data in DB in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] *dataPtr        - (pointer to) data
* @param[in] lookupNumber    - lookup number
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_ADD_FUN)
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_U32                                   lookupNumber,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_MAX_DEPT_GET_FUN function
* @endinternal
*
* @brief Get max depth according to key size in bytes.
*
* @param[in] managerId       - manager identification
* @param[in] sizeInBytes     - key size in bytes
*
* @param[out] depthPtr       - (pointer to) depth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_MAX_DEPT_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U32                                  *depthPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_HASH_ARR_GET_FUN function
* @endinternal
*
* @brief Get hash array results per client.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] dataPtr         - key array in bytes
* @param[in] sizeInBytes     - key size in bytes
* @param[in] lookupNumber    - lookup number
* @param[in] ignoreForbiddenIndexes - whether to ignore rules conflicts
*
* @param[out] numHashArrPtr  - (pointer to) number of hash array results
* @param[out] hashArr        - (pointer to) hash array results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_HASH_ARR_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    IN  GT_BOOL                                 ignoreForbiddenIndexes,
    OUT GT_U32                                  *numHashArrPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
);

/**
* @internal prvCpssDxChCuckooDbHashFuncBind function
* @endinternal
*
* @brief Bind hash function specific to client.
*
* @param[in] managerId             - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_HASH_BIND_FUN)
(
    IN  GT_U32                              managerId,
    ...
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_PTR_GET_FUN function
* @endinternal
*
* @brief   The function returns pointer to log array in cuckoo DB.
*
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_PTR_GET_FUN)
(
    IN  GT_U32                                      managerId,
    OUT GT_U32                                      *numOfOperationsPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC       **cuckooDbLogPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_UPDATE_STAGES_FUN function
* @endinternal
*
* @brief    Update stages in cuckoo operation log.
*
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_UPDATE_STAGES_FUN)
(
   IN GT_U32                                   managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_DELETE_FUN function
* @endinternal
*
* @brief   Delete all log operations.
*
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_DELETE_FUN)
(
    IN GT_U32                                   managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_DELETE_FUN function
* @endinternal
*
* @brief Delete entry from DB in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_DELETE_FUN)
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN function
* @endinternal
*
* @brief Set data in operation log in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] dataPtr         - (pointer to) data
* @param[in] lookupNumber    - lookup number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN)
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_U32                                   lookupNumber
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_STATISTICS_PTR_GET_FUN function
* @endinternal
*
* @brief   The function returns pointer to statistics in cuckoo DB.
*
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_STATISTICS_PTR_GET_FUN)
(
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC         **cuckooDbBankStPtr,
    OUT GT_U32                                      **cuckooDbSizeStPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_INIT_FUN function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_INIT_FUN)
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_DELETE_FUN function
* @endinternal
*
* @brief   Delete cuckoo DB per client type and managerId.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_DELETE_FUN)
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_PRINT_STATISTICS_FUN function
* @endinternal
*
* @brief   Print cuckoo DB statistics per client type and
*          managerId.
*
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_PRINT_STATISTICS_FUN)
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_SYNC_STATISTICS_FUN function
* @endinternal
*
* @brief   Sync cuckoo DB statistics per client type and
*          managerId after failure.
*
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_SYNC_STATISTICS_FUN)
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_INIT_DEBUG_FUN function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId
*          for debug.
*
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_INIT_DEBUG_FUN)
(
    IN GT_U32                               managerId,
    IN GT_U32                               db_banks,
    IN GT_U32                               db_lines
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_FUN function
* @endinternal
*
* @brief    Add new entry to DB by finding free space or
*           relocate existing entries.
*
*   Entry at index Y will be copied to index Z, then new entry X will be copied to index Y
*   so traffic of existing entries won't be lossed and new entry could be added by using index X.
*
*   New entry X --> Y[i] -->Z[j]
*
* @param[in] managerId      - manager identification
*
*  NOTE: this function relocates entries only in DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_FUN)
(
    IN GT_U32                               managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_REHASH_ENTRY_FUN function
* @endinternal
*
* @brief Move entry using calculated hash locations ordered according to populated banks.
*
* @param[in] managerId            - manager identification
* @param[in] entryInfoPtr         - (pointer to) entry to add
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_REHASH_ENTRY_FUN)
(
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_FUN function
* @endinternal
*
* @brief Get [bank][line] data and lookup number from operation log.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - number of bytes
*
* @param[out] dataPtr        - (pointer to) [bank][line] data in
*                               operation log
* @param[out] lookupNumberPtr- (pointer to) [bank][line] lookup number in
*                               operation log
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN function
* @endinternal
*
* @brief Check if [bank][line] is free in log operations.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFreePtr      - (pointer to) whether [bank][line]
*                               is free in log operations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN)
(
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FIRST_FUN function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in log operations.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFirstPtr      - (pointer to) whether
*                               [bank][line] is first in log
*                               operations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FIRST_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_FUN function
* @endinternal
*
* @brief Get [bank][line] size in Bytes and lookup number from operation log.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] sizeInBytesPtr      - (pointer to) [bank][line]
*                                   size in bytes in operation log
* @param[out] lookupNumberPtr     - (pointer to) [bank][line]
*                                   lookup number in operation log
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN function
* @endinternal
*
* @brief Count consecutive free banks from [bank][line] untill size.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks
*
* @param[out] numFreePtr      - (pointer to) consecutive free banks
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *numFreePtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_SORT_POPULATED_BANKS_FUN function
* @endinternal
*
* @brief Sort populated banks.
*
* @param[in] managerId       - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_SORT_POPULATED_BANKS_FUN)
(
    IN  GT_U32                                  managerId
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_FIRST_BANK_GET_FUN function
* @endinternal
*
* @brief Get entry's first bank.
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks
*
* @param[out] firstBankPtr      - (pointer to) entry's first bank
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_FIRST_BANK_GET_FUN)
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *firstBankPtr
);

/**
* @internal PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_CLEAR_FUN function
* @endinternal
*
* @brief Clear entry in operation log in location [bank][line].
*
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
typedef GT_STATUS (*PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_CLEAR_FUN)
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes
);

#ifdef SHARED_MEMORY
typedef GT_U32                                                  CUCKOO_DB_IS_FREE_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_IS_FIRST_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_IS_LOCK_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_CLIENT_HASH_ARR_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_MAX_DEPT_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_STATISTICS_PTR_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOCK_SET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_ENTRY_ADD_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_ENTRY_DELETE_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_HASH_BIND_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_INIT_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_DELETE_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_PRINT_STATISTICS_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_SYNC_STATISTICS_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_INIT_DEBUG_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_REHASH_ENTRY_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_COUNT_FREE_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_SORT_POPULATED_BANKS_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_ENTRY_FIRST_BANK_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_PTR_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_UPDATE_STAGES_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_ARRAY_DELETE_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_INFO_SET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_IS_FREE_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_IS_FIRST_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef GT_U32                                                  CUCKOO_DB_LOG_INFO_CLEAR_CALLBACK_FUNC;
#else
typedef PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN                     CUCKOO_DB_IS_FREE_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_IS_FIRST_FUN                    CUCKOO_DB_IS_FIRST_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN                     CUCKOO_DB_IS_LOCK_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_FUN      CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_FUN      CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_HASH_ARR_GET_FUN         CUCKOO_DB_CLIENT_HASH_ARR_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_MAX_DEPT_GET_FUN                CUCKOO_DB_MAX_DEPT_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_STATISTICS_PTR_GET_FUN          CUCKOO_DB_STATISTICS_PTR_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_FUN    CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOCK_SET_FUN                    CUCKOO_DB_LOCK_SET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_ADD_FUN                   CUCKOO_DB_ENTRY_ADD_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_DELETE_FUN                CUCKOO_DB_ENTRY_DELETE_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_HASH_BIND_FUN                   CUCKOO_DB_HASH_BIND_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_INIT_FUN                        CUCKOO_DB_INIT_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_DELETE_FUN                      CUCKOO_DB_DELETE_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_PRINT_STATISTICS_FUN            CUCKOO_DB_PRINT_STATISTICS_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_SYNC_STATISTICS_FUN             CUCKOO_DB_SYNC_STATISTICS_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_INIT_DEBUG_FUN                  CUCKOO_DB_INIT_DEBUG_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_FUN      CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_REHASH_ENTRY_FUN                CUCKOO_DB_REHASH_ENTRY_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN              CUCKOO_DB_COUNT_FREE_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_SORT_POPULATED_BANKS_FUN        CUCKOO_DB_SORT_POPULATED_BANKS_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_FIRST_BANK_GET_FUN        CUCKOO_DB_ENTRY_FIRST_BANK_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_PTR_GET_FUN                 CUCKOO_DB_LOG_PTR_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_UPDATE_STAGES_FUN           CUCKOO_DB_LOG_UPDATE_STAGES_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_DELETE_FUN            CUCKOO_DB_LOG_ARRAY_DELETE_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN                CUCKOO_DB_LOG_INFO_SET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_FUN  CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN                 CUCKOO_DB_LOG_IS_FREE_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FIRST_FUN                CUCKOO_DB_LOG_IS_FIRST_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_FUN  CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC;
typedef PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_CLEAR_FUN              CUCKOO_DB_LOG_INFO_CLEAR_CALLBACK_FUNC;
#endif

typedef struct
{
    CUCKOO_DB_IS_FREE_CALLBACK_FUNC                         prvCpssDxChCuckooDbIsFreeFunc;
    CUCKOO_DB_IS_FIRST_CALLBACK_FUNC                        prvCpssDxChCuckooDbIsFirstFunc;
    CUCKOO_DB_IS_LOCK_CALLBACK_FUNC                         prvCpssDxChCuckooDbIsLockFunc;
    CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC          prvCpssDxChCuckooDbSizeLookupNumberGetFunc;
    CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_CALLBACK_FUNC          prvCpssDxChCuckooDbDataLookupNumberGetFunc;
    CUCKOO_DB_CLIENT_HASH_ARR_GET_CALLBACK_FUNC             prvCpssDxChCuckooDbClientHashArrGetFunc;
    CUCKOO_DB_MAX_DEPT_GET_CALLBACK_FUNC                    prvCpssDxChCuckooDbMaxDepthGetFunc;
    CUCKOO_DB_STATISTICS_PTR_GET_CALLBACK_FUNC              prvCpssDxChCuckooDbStatisticsPtrGetFunc;
    CUCKOO_DB_DATA_LOOKUP_NUMBER_CHECK_CALLBACK_FUNC        prvCpssDxChCuckooDbDataLookupNumberCheckFunc;
    CUCKOO_DB_LOCK_SET_CALLBACK_FUNC                        prvCpssDxChCuckooDbLockSetFunc;
    CUCKOO_DB_ENTRY_ADD_CALLBACK_FUNC                       prvCpssDxChCuckooDbEntryAddFunc;
    CUCKOO_DB_ENTRY_DELETE_CALLBACK_FUNC                    prvCpssDxChCuckooDbEntryDeleteFunc;
    CUCKOO_DB_HASH_BIND_CALLBACK_FUNC                       prvCpssDxChCuckooDbHashFuncBindFunc;
    CUCKOO_DB_INIT_CALLBACK_FUNC                            prvCpssDxChCuckooDbInitFunc;
    CUCKOO_DB_DELETE_CALLBACK_FUNC                          prvCpssDxChCuckooDbDeleteFunc;
    CUCKOO_DB_PRINT_STATISTICS_CALLBACK_FUNC                prvCpssDxChCuckooDbPrintStatisticsFunc;
    CUCKOO_DB_SYNC_STATISTICS_CALLBACK_FUNC                 prvCpssDxChCuckooDbSyncStatisticsFunc;
    CUCKOO_DB_INIT_DEBUG_CALLBACK_FUNC                      prvCpssDxChCuckooDbInitDebugFunc;
    CUCKOO_DB_RELOCATE_ENTRIES_DEBUG_CALLBACK_FUNC          prvCpssDxChCuckooRelocateEntriesDebugFunc;
    CUCKOO_DB_REHASH_ENTRY_CALLBACK_FUNC                    prvCpssDxChCuckooDbRehashEntryFunc;
    CUCKOO_DB_COUNT_FREE_GET_CALLBACK_FUNC                  prvCpssDxChCuckooDbCountFreeGetFunc;
    CUCKOO_DB_SORT_POPULATED_BANKS_CALLBACK_FUNC            prvCpssDxChCuckooDbSortPopulatedBanksFunc;
    CUCKOO_DB_ENTRY_FIRST_BANK_GET_CALLBACK_FUNC            prvCpssDxChCuckooDbEntryFirstBankGetFunc;

    CUCKOO_DB_LOG_PTR_GET_CALLBACK_FUNC                     prvCpssDxChCuckooDbLogPtrGetFunc;
    CUCKOO_DB_LOG_UPDATE_STAGES_CALLBACK_FUNC               prvCpssDxChCuckooDbLogUpdateStagesFunc;
    CUCKOO_DB_LOG_ARRAY_DELETE_CALLBACK_FUNC                prvCpssDxChCuckooDbLogArrayDeleteFunc;
    CUCKOO_DB_LOG_INFO_SET_CALLBACK_FUNC                    prvCpssDxChCuckooDbLogInfoSetFunc;
    CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_CALLBACK_FUNC      prvCpssDxChCuckooDbLogDataLookupNumberGetFunc;
    CUCKOO_DB_LOG_IS_FREE_CALLBACK_FUNC                     prvCpssDxChCuckooDbLogIsFreeFunc;
    CUCKOO_DB_LOG_IS_FIRST_CALLBACK_FUNC                    prvCpssDxChCuckooDbLogIsFirstFunc;
    CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_CALLBACK_FUNC      prvCpssDxChCuckooDbLogSizeLookupNumberGetFunc;
    CUCKOO_DB_LOG_INFO_CLEAR_CALLBACK_FUNC                  prvCpssDxChCuckooDbLogInfoClearFunc;

} PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC, *PRV_CPSS_DXCH_CUCKOO_FUNC_PTR;

#ifdef __cplusplus
}
#endif

#endif   /* prvCpssDxChCuckooh */



