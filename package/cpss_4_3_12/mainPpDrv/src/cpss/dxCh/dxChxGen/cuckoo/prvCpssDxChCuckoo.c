#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>

#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>

/* global variables macros */

#define PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.cuckooDir.cuckooSrc._var,_value)

#define PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc._var)

static GT_STATUS prvCpssDxChCuckooDbRecursiveEntryAdd
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
);

void prvCpssDxChCuckooDebug1PrintEnableSet(GT_U32 enable)
{
    GT_BOOL bEnable = (enable) ? GT_TRUE : GT_FALSE;
    PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(debug1Print,bEnable);
}

void prvCpssDxChCuckooDebug2PrintEnableSet(GT_U32 enable)
{
    GT_BOOL bEnable = (enable) ? GT_TRUE : GT_FALSE;
    PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(debug2Print,bEnable);
}

#define PRV_CPSS_DXCH_CUCKOO_MIN_BANK_STEP_CNS          1 /* for PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E */
#define PRV_CPSS_DXCH_CUCKOO_MAX_BANK_STEP_CNS          4 /* for PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E */
#define PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS          16

/* get pointer to cuckooDB */
#define CUCKOO_DB_GET_MAC(clientType,managerId)  PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]

#define CUCKOO_PRINT_KEY(data,keySize,lookupNumber)                     \
{                                                                       \
    cpssOsPrintf("keySize [%d] lookup [%d] ",keySize,lookupNumber);     \
    cpssOsPrintf("pattern:");                                           \
    for (n=0; n<keySize; n++)                                           \
    {                                                                   \
        cpssOsPrintf("[%d]",data[n]);                                   \
    }                                                                   \
    cpssOsPrintf("\n");                                                 \
}

#define CUCKOO_PRINT_LOG_ENTRY(logEntry,index)                                   \
    cpssOsPrintf("%d. log_array[%d][%d]:\n",index+1,logEntry.bank,logEntry.line);\
    cpssOsPrintf("   ");                                                         \
    CUCKOO_PRINT_KEY(logEntry.data,logEntry.sizeInBytes,logEntry.lookupNumber);  \
    cpssOsPrintf("   isFirst [%d] isLock [%d] isFree [%d] stage [%d]\n",logEntry.isFirst, logEntry.isLock, logEntry.isFree, logEntry.stage);\
    cpssOsPrintf("\n");

#define CUCKOO_PRINT_DB_ENTRY(dbEntry,index,bank,line)                           \
    cpssOsPrintf("%d. db[%d][%d]:\n",index+1,bank,line);                         \
    cpssOsPrintf("   ");                                                         \
    CUCKOO_PRINT_KEY(dbEntry.data,dbEntry.sizeInBytes,dbEntry.lookupNumber);     \
    cpssOsPrintf("   isFirst [%d] isLock [%d] isFree [%d]\n",dbEntry.isFirst, dbEntry.isLock, dbEntry.isFree);\
    cpssOsPrintf("\n");

#define CUCKOO_PRINT_DB_ENTRY1(exactMatchManagerPtr,entryPtr,sizeInBytes,bank,line)     \
    cpssOsPrintf("db[%d][%d]:\n",bank,line);                                            \
    CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex); \
    CUCKOO_PRINT_KEY(entryPtr->pattern,sizeInBytes,entryPtr->hwExactMatchLookupNum);    \
    cpssOsPrintf("isFirst [%d] isLock [%d] isFree [%d] lookup [%d]\n",entryPtr->isFirst, exactMatchManagerPtr->indexArr[hwIndex].isLock, !(entryPtr->isUsedEntry), entryPtr->hwExactMatchLookupNum);\
    cpssOsPrintf("\n");

/* macro to check that x == 0 is GT_BAD_STATE */
#define CUCKOO_CHECK_X_NOT_ZERO_MAC(x)                  \
    if((x) == 0)                                        \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s] must not be ZERO",                    \
            #x);                                        \
    }

/* macro to convert from hwIndex to {bank,line} */
#define CUCKOO_BANK_LINE_FROM_HW_INDEX_GET_MAC(numOfHashes,hwIndex,bank,line)   \
{                                                                               \
    switch(numOfHashes)                                                         \
    {                                                                           \
    case 4:                                                                     \
        line = hwIndex >> 2;                                                    \
        bank = hwIndex & 0x3;                                                   \
        break;                                                                  \
    case 8:                                                                     \
        line = hwIndex >> 3;                                                    \
        bank = hwIndex & 0x7;                                                   \
        break;                                                                  \
    case 16:                                                                    \
        line = hwIndex >> 4;                                                    \
        bank = hwIndex & 0xF;                                                   \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(numOfHashes);                     \
    }                                                                           \
}

/* macro to convert from {bank,line} to hwIndex */
#define CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(numOfHashes,bank,line,hwIndex)   \
{                                                                               \
    switch(numOfHashes)                                                         \
    {                                                                           \
    case 1:                                                                     \
    case 4:                                                                     \
        hwIndex = (line << 2) | (bank & 0x3);                                   \
        break;                                                                  \
    case 8:                                                                     \
        hwIndex = (line << 3) | (bank & 0x7);                                   \
        break;                                                                  \
    case 16:                                                                    \
        hwIndex = (line << 4) | (bank & 0xF);                                   \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(numOfHashes);                     \
    }                                                                           \
}

/**
* @enum PRV_CPSS_DXCH_CUCKOO_MHT_ENT
*
* @brief Enum values represent num of Multiple Hash Tables
*/
typedef enum{

    /* Four Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_4_E,

    /* Eight Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_8_E,

    /* Sixteen Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_16_E,

    /* last value */
    PRV_CPSS_DXCH_CUCKOO_MHT_LAST_E

}PRV_CPSS_DXCH_CUCKOO_MHT_ENT;

/* max depths according to cuckoo algorithm {Multiple Hash Tables,key size} */
static const GT_U32 hash_depth[PRV_CPSS_DXCH_CUCKOO_MHT_LAST_E][PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E] ={
/*         5B             19B           33B           47B           */
/*4MTH*/ {_4_hash_depth, _2_hash_depth,            2,            2},
/*8MTH*/ {_8_hash_depth, _4_hash_depth,_2_hash_depth,_2_hash_depth},
/*16MTH*/{_16_hash_depth,_8_hash_depth,_5_hash_depth,_4_hash_depth}};

/* check that the key sizeInBytes is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes)\
{                                                                               \
    switch (clientType)                                                         \
    {                                                                           \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:                            \
        switch (sizeInBytes)                                                    \
        {                                                                       \
        case 5:                                                                 \
        case 19:                                                                \
        case 33:                                                                \
        case 47:                                                                \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in sizeInBytes [%d]",sizeInBytes);\
        }                                                                       \
        break;                                                                  \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:                                    \
        switch (sizeInBytes)                                                    \
        {                                                                       \
        case 6:     /* TBD to check */                                         \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in sizeInBytes [%d]",sizeInBytes);\
        }                                                                       \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);                      \
    }                                                                           \
}

/* check that the key lookup number is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,lookupNumber)\
{                                                                               \
    switch (clientType)                                                         \
    {                                                                           \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:                            \
        switch(lookupNumber)                                                    \
        {                                                                       \
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:                              \
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:                             \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in lookup number [%d]",lookupNumber);\
        }                                                                       \
        break;                                                                  \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:                                    \
        switch (lookupNumber)                                                   \
        {                                                                       \
        case 0: /* TBD */                                                       \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in lookup number [%d]",lookupNumber);\
        }                                                                       \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);                      \
    }                                                                           \
}

/* check that bank step is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,bankStep)     \
{                                                                               \
    switch (clientType)                                                         \
    {                                                                           \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:                            \
        switch (bankStep)                                                       \
        {                                                                       \
        case 1:                                                                 \
        case 2:                                                                 \
        case 3:                                                                 \
        case 4:                                                                 \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bankStep [%d]",bankStep);\
        }                                                                       \
        break;                                                                  \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:                                    \
        switch (bankStep)                                                       \
        {                                                                       \
        case 2: /* TBD to check */                                             \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bankStep [%d]",bankStep);\
        }                                                                       \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);                      \
    }                                                                           \
}

/* check that the bank is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank)    \
    if (bank >= PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]->db_banks)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bank [%d]",bank); \
    }

/* check that the line is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line)    \
    if (line >= PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]->db_lines)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in line [%d]",line); \
    }

/* check that DB was initialized  */
#define PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId)            \
    if (clientType >= PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in client type [%d]",clientType); \
    }                                                                           \
    if (managerId >= PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS)                  \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in manageId type [%d]",managerId); \
    }                                                                           \
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)                                \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error DB is not initialized"); \
    }

/* check that client type is supported */
#define PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType)            \
    if (clientType >= PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in client type [%d]",clientType); \
    }

#ifdef CPSS_LOG_ENABLE
    /* check that [bank][line] is not free in DB */
    #define PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line) \
    {                                                                               \
        PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC * cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType])); \
        PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFreefunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN); \
        rc = isFreefunc(managerId,bank,line,&is_free);                              \
        if (rc != GT_OK)                                                            \
        {                                                                           \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in one of the parameters: client type [%d] managerId [%d] bank [%d] line [%d]",clientType,managerId,bank,line);\
        }                                                                           \
        if (is_free)                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry DB[%d][%d] isFree [%d]",bank,line,&is_free);\
    }
#else
    #define PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line) \
    {                                                                               \
        PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC * cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType])); \
        PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFreefunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN); \
        rc = isFreefunc(managerId,bank,line,&is_free);                              \
        if (rc != GT_OK)                                                            \
        {                                                                           \
            return rc;                                                              \
        }                                                                           \
        if (is_free)                                                                \
            return /*no log info*/ GT_BAD_STATE;                                    \
    }
#endif

/* macro to convert cuckoo key size in bytes to bank step */
#define PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,bankStep)  \
    switch (sizeInBytes)                                                                    \
    {                                                                                       \
    case 5:                                                                                 \
        bankStep = 1;                                                                       \
        break;                                                                              \
    case 19:                                                                                \
        bankStep = 2;                                                                       \
        break;                                                                              \
    case 33:                                                                                \
        bankStep = 3;                                                                       \
        break;                                                                              \
    case 47:                                                                                \
        bankStep = 4;                                                                       \
        break;                                                                              \
    default:                                                                                \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);                                 \
    }

/**
* @internal em_prvCpssDxChCuckooDbHashFuncBind function
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
static GT_STATUS em_prvCpssDxChCuckooDbHashFuncBind
(
    IN  GT_U32                              managerId,
    ...
)
{
    va_list argParam;
    GT_U32 i,sizeInBytes;
    GT_U32 *numOfElem, *crcMultiHashArr;
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_KEY_STC emEntryKey;
    CPSS_EXACT_MATCH_MHT_ENT exactMatchBanksNum;
    GT_U32 exactMatchSize;

    /* init variable argument list */
    va_start(argParam, managerId);

    sizeInBytes = va_arg(argParam, GT_U32);
    for (i=0; i<sizeInBytes; i++)
        data[i] = (GT_U8)va_arg(argParam, GT_U32);
    numOfElem = va_arg(argParam, GT_U32*);
    crcMultiHashArr = va_arg(argParam, GT_U32*);

    /* free VA list pointer */
    va_end(argParam);

    switch(sizeInBytes)
    {
    case 5:
        emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        break;
    case 19:
        emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        break;
    case 33:
        emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
        break;
    case 47:
        emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
    }
    cpssOsMemCpy(emEntryKey.pattern, data, sizeof(GT_U8)*sizeInBytes);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)
    {
        case 4:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_4_E;
            break;
        case 8:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_8_E;
            break;
        case 16:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_16_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes);
    }

    switch(exactMatchManagerPtr->cpssHashParams.size)
    {
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_8KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_16KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_32KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_64KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_128KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_256KB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
    }
    /*******************************/
    /* calc the 4/8/16 hash values */
    /*******************************/

    return prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,exactMatchBanksNum,
                                                                   &emEntryKey,PRV_CPSS_DXCH_EXACT_MATCH_HASH_DATA_SIZE_VALUE_CNS,
                                                                   numOfElem,crcMultiHashArr);
}
#if 0
/**
* @internal fdb_prvCpssDxChCuckooHashFuncBind function
* @endinternal
*
* @brief Bind hash function specific to client.
*
* @param[in] clientType            - client type
* @param[in] managerId             - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
static GT_STATUS fdb_prvCpssDxChCuckooHashFuncBind
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT    clientType,
    IN  GT_U32                              managerId,
    ...
)
{
    va_list argParam;
    GT_U32 i,sizeInBytes;
    GT_U32 *numOfElem, *crcMultiHashArr;
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* exact match */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_KEY_STC emEntryKey;
    CPSS_EXACT_MATCH_MHT_ENT exactMatchBanksNum;
    GT_U32 exactMatchSize;

    /* fdb */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    CPSS_MAC_ENTRY_EXT_KEY_STC fdbEntryKey;

    /* init variable argument list */
    va_start(argParam, managerId);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        sizeInBytes = va_arg(argParam, GT_U32);
        for (i=0; i<sizeInBytes; i++)
            data[i] = (GT_U8)va_arg(argParam, GT_U32);
        numOfElem = va_arg(argParam, GT_U32*);
        crcMultiHashArr = va_arg(argParam, GT_U32*);

        /* free VA list pointer */
        va_end(argParam);

        switch(sizeInBytes)
        {
        case 5:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
            break;
        case 19:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
            break;
        case 33:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
            break;
        case 47:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
        }
        cpssOsMemCpy(emEntryKey.pattern, data, sizeof(GT_U8)*sizeInBytes);

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)
        {
            case 4:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_4_E;
                break;
            case 8:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_8_E;
                break;
            case 16:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_16_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes);
        }

        switch(exactMatchManagerPtr->cpssHashParams.size)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_8KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_16KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_32KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_64KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_128KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
        }
        /*******************************/
        /* calc the 4/8/16 hash values */
        /*******************************/

        return prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,exactMatchBanksNum,
                                                                       &emEntryKey,PRV_CPSS_DXCH_EXACT_MATCH_HASH_DATA_SIZE_VALUE_CNS,
                                                                       numOfElem,crcMultiHashArr);
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        for (i=0; i<6; i++) /* data contains only mac 6 bytes */
            data[i] = (GT_U8)va_arg(argParam, GT_U32);
        crcMultiHashArr = va_arg(argParam, GT_U32*);

        /* free VA list pointer */
        va_end(argParam);

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        fdbEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        for (i=0; i<6; i++)
            fdbEntryKey.key.macVlan.macAddr.arEther[i] = data[i];
        fdbEntryKey.key.macVlan.vlanId = 1; /* TBD temp only for test */
        fdbEntryKey.vid1 = 1;

        /*******************************/
        /* calc the 4/8/16 hash values */
        /*******************************/

        return cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(&fdbManagerPtr->cpssHashParams,&fdbEntryKey,0,/*multiHashStartBankIndex*/
                                                             fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,crcMultiHashArr);

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }
}
#endif

/**
* @internal em_prvCpssDxChCuckooDbIsLock function
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
static GT_STATUS em_prvCpssDxChCuckooDbIsLock
(
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_BOOL                                 *isLockPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex=0;
    GT_U32 i;
    GT_BOOL is_lock=GT_FALSE;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,size);
    CPSS_NULL_PTR_CHECK_MAC(isLockPtr);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    for (i=0; i<size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank+i);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,(bank+i),line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        is_lock = (exactMatchManagerPtr->indexArr[hwIndex].isLock == GT_TRUE);
        if (is_lock)
        {
            /* if one of the banks is locked; no need to check all other banks */
            *isLockPtr = is_lock;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
            }
            return GT_OK;
        }
    }

    *isLockPtr = is_lock;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
    }
    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbIsLock function
* @endinternal
*
* @brief Check if [bank][line] for number of banks is locked in DB.
*
* @param[in] clientType      - client type
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
static GT_STATUS fdb_prvCpssDxChCuckooDbIsLock
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_BOOL                                 *isLockPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  hwIndex=0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr=NULL;
    GT_U32 i;
    GT_BOOL is_lock=GT_FALSE;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(isLockPtr);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        for (i=0; i<size; i++)
        {
            PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

            CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,(bank+i),line,hwIndex);
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

            is_lock = (exactMatchManagerPtr->indexArr[hwIndex].isLock == GT_TRUE);
            if (is_lock)
            {
                /* if one of the banks is locked; no need to check all other banks */
                *isLockPtr = is_lock;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
                {
                    cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
                }
                return GT_OK;
            }
        }
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        /* convert enum key size to size in bytes */
      /*  EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(fdbEntryPtr->hwFdbEntryType,*sizeInBytesPtr); TBD */
     /* *lookupNumberPtr = fdbEntryPtr->hwExactMatchLookupNum; TBD */
        fdbEntryPtr = fdbEntryPtr; /*  TBD */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    *isLockPtr = is_lock;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
    }
    return GT_OK;
}
#endif

/**
* @internal em_prvCpssDxChCuckooDbSetLock function
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
static GT_STATUS em_prvCpssDxChCuckooDbSetLock
(
    IN GT_U32                                  managerId,
    IN GT_U32                                  bank,
    IN GT_U32                                  line,
    IN GT_U32                                  size,
    IN GT_BOOL                                 lock
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex=0;
    GT_U32 i;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,size);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    for (i=0; i<size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank+i);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,(bank+i),line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        if (lock)
        {
            exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_TRUE;

            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("lock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);
            }
        }
        else
        {
            /* this is a problem */
            if (exactMatchManagerPtr->indexArr[hwIndex].isLock == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to unlock not locked entry cuckooDB[%d][%d] isLocked [%d]",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);

            }
            exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_FALSE;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("unlock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);
            }
        }
    }

    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbSetLock function
* @endinternal
*
* @brief lock/unlock [bank][line] for number of banks in DB.
*
* @param[in] clientType      - client type
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
static GT_STATUS fdb_prvCpssDxChCuckooDbSetLock
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN GT_U32                                  managerId,
    IN GT_U32                                  bank,
    IN GT_U32                                  line,
    IN GT_U32                                  size,
    IN GT_BOOL                                 lock
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  hwIndex=0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr=NULL;
    GT_U32 i;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        for (i=0; i<size; i++)
        {
            PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

            CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,(bank+i),line,hwIndex);
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

            if (lock)
            {
                exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_TRUE;

                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
                {
                    cpssOsPrintf("lock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);
                }
            }
            else
            {
                /* this is a problem */
                if (exactMatchManagerPtr->indexArr[hwIndex].isLock == GT_FALSE)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to unlock not locked entry cuckooDB[%d][%d] isLocked [%d]",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);

                }
                exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_FALSE;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
                {
                    cpssOsPrintf("unlock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,exactMatchManagerPtr->indexArr[hwIndex].isLock);
                }
            }
        }
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        /* convert enum key size to size in bytes */
      /*  EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(fdbEntryPtr->hwFdbEntryType,*sizeInBytesPtr);  TBD */
     /* *lookupNumberPtr = fdbEntryPtr->hwExactMatchLookupNum;  TBD */
        fdbEntryPtr = fdbEntryPtr; /*  TBD */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    return GT_OK;
}
#endif
/**
* @internal em_prvCpssDxChCuckooDbIsFree function
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
static GT_STATUS em_prvCpssDxChCuckooDbIsFree
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex=0;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFreePtr);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
    CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
    *isFreePtr = (exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer == 0) ? GT_TRUE : GT_FALSE;

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFree [%d]\n",bank,line,*isFreePtr);
    }

    return GT_OK;
}
#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbIsFree function
* @endinternal
*
* @brief Check if [bank][line] is free in DB.
*
* @param[in] clientType      - client type
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
static GT_STATUS fdb_prvCpssDxChCuckooDbIsFree
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  hwIndex=0;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFreePtr);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        *isFreePtr = (exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer == 0) ? GT_TRUE : GT_FALSE;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);
        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        *isFreePtr = (fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer == 0) ? GT_TRUE : GT_FALSE;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFree [%d]\n",bank,line,*isFreePtr);
    }

    return GT_OK;
}
#endif
/**
* @internal prvCpssDxChCuckooDbLogIsFree function
* @endinternal
*
* @brief Check if [bank][line] is free in log operations.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogIsFree
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    GT_U32 j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFreePtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            *isFreePtr = (cuckooDbPtr->log_array[j-1].isFree == 1);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] isFree [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal em_prvCpssDxChCuckooDbLogIsFree function
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
static GT_STATUS em_prvCpssDxChCuckooDbLogIsFree
(
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    return prvCpssDxChCuckooDbLogIsFree(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,isFreePtr);
}

/**
* @internal em_prvCpssDxChCuckooDbSizeLookupNumberGet function
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
static GT_STATUS em_prvCpssDxChCuckooDbSizeLookupNumberGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    GT_STATUS                                               rc;
    GT_BOOL                                                 is_free;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(sizeInBytesPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

    dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    /* convert enum key size to size in bytes */
    EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(emEntryPtr->hwExactMatchKeySize,*sizeInBytesPtr);
    *lookupNumberPtr = emEntryPtr->hwExactMatchLookupNum;

    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,*sizeInBytesPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,*lookupNumberPtr);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] sizeInBytes [%d] lookupNumber [%d]\n",bank,line,*sizeInBytesPtr,*lookupNumberPtr);
    }

    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbSizeLookupNumberGet function
* @endinternal
*
* @brief Get [bank][line] size in Bytes and lookup number from DB.
*
* @param[in] clientType      - client type
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
static GT_STATUS fdb_prvCpssDxChCuckooDbSizeLookupNumberGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr;
    GT_STATUS                                               rc;
    GT_BOOL                                                 is_free;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(sizeInBytesPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        /* convert enum key size to size in bytes */
        EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(emEntryPtr->hwExactMatchKeySize,*sizeInBytesPtr);
        *lookupNumberPtr = emEntryPtr->hwExactMatchLookupNum;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, fdbManagerPtr->capacityInfo.maxTotalEntries);

        fdbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        /* convert enum key size to size in bytes */
      /*  EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(fdbEntryPtr->hwFdbEntryType,*sizeInBytesPtr);  TBD */
     /* *lookupNumberPtr = fdbEntryPtr->hwExactMatchLookupNum;  TBD */
        fdbEntryPtr = fdbEntryPtr; /*  TBD */
        *sizeInBytesPtr = 6;
        *lookupNumberPtr = 0;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,*sizeInBytesPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,*lookupNumberPtr);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] sizeInBytes [%d] lookupNumber [%d]\n",bank,line,*sizeInBytesPtr,*lookupNumberPtr);
    }

    return GT_OK;
}
#endif
/**
* @internal prvCpssDxChCuckooDbLogSizeLookupNumberGet function
* @endinternal
*
* @brief Get [bank][line] size in Bytes and lookup number from operation log.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogSizeLookupNumberGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    GT_U32 j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(sizeInBytesPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            if (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }

            *sizeInBytesPtr = cuckooDbPtr->log_array[j-1].sizeInBytes;
            *lookupNumberPtr = cuckooDbPtr->log_array[j-1].lookupNumber;

            PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,*sizeInBytesPtr);
            PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,*lookupNumberPtr);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] sizeInBytes [%d] lookupNumber [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].sizeInBytes,cuckooDbPtr->log_array[j-1].lookupNumber);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal em_prvCpssDxChCuckooDbLogSizeLookupNumberGet function
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
static GT_STATUS em_prvCpssDxChCuckooDbLogSizeLookupNumberGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    return prvCpssDxChCuckooDbLogSizeLookupNumberGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,sizeInBytesPtr,lookupNumberPtr);
}
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
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    GT_STATUS                                               rc;
    GT_BOOL                                                 is_free;
    GT_U32                                                  n;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

    dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    /* copy all data from DB */
    cpssOsMemCpy(dataPtr,emEntryPtr->pattern,sizeof(GT_U8)*sizeInBytes);
    *lookupNumberPtr = emEntryPtr->hwExactMatchLookupNum;

    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,*lookupNumberPtr);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d]: ",bank,line);
        CUCKOO_PRINT_KEY(dataPtr,sizeInBytes,*lookupNumberPtr);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbDataLookupNumberGet function
* @endinternal
*
* @brief Get [bank][line] data and lookup number from DB.
*
* @param[in] clientType      - client type
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
GT_STATUS fdb_prvCpssDxChCuckooDbDataLookupNumberGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr;
    GT_STATUS                                               rc;
    GT_BOOL                                                 is_free;
    GT_U32                                                  n;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        /* copy all data from DB */
        cpssOsMemCpy(dataPtr,emEntryPtr->pattern,sizeof(GT_U8)*sizeInBytes);
        *lookupNumberPtr = emEntryPtr->hwExactMatchLookupNum;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, fdbManagerPtr->capacityInfo.maxTotalEntries);

        fdbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        /* convert enum key size to size in bytes */
      /*  EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(fdbEntryPtr->hwFdbEntryType,*sizeInBytesPtr);  TBD */
     /* *lookupNumberPtr = fdbEntryPtr->hwExactMatchLookupNum;  TBD */
        fdbEntryPtr = fdbEntryPtr; /*  TBD */
        *lookupNumberPtr = 0;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,*lookupNumberPtr);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d]: ",bank,line);
        CUCKOO_PRINT_KEY(dataPtr,sizeInBytes,*lookupNumberPtr);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}
#endif

/**
* @internal prvCpssDxChCuckooDbLogDataLookupNumberGet function
* @endinternal
*
* @brief Get [bank][line] data and lookup number from operation log.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogDataLookupNumberGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    GT_BOOL is_free;
    GT_U32 j,n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupNumberPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            is_free = (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE);
            if (is_free)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }

            /* copy all data from log */
            cpssOsMemCpy(dataPtr,cuckooDbPtr->log_array[j-1].data,sizeof(GT_U8)*sizeInBytes);
            *lookupNumberPtr = cuckooDbPtr->log_array[j-1].lookupNumber;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d]: ",j-1,bank,line);
                CUCKOO_PRINT_KEY(cuckooDbPtr->log_array[j-1].data,sizeInBytes,cuckooDbPtr->log_array[j-1].lookupNumber);
                cpssOsPrintf("\n");
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal em_prvCpssDxChCuckooDbLogDataLookupNumberGet function
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
static GT_STATUS em_prvCpssDxChCuckooDbLogDataLookupNumberGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr,
    OUT GT_U32                                  *lookupNumberPtr
)
{
    return prvCpssDxChCuckooDbLogDataLookupNumberGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,sizeInBytes,dataPtr,lookupNumberPtr);
}
/**
* @internal em_prvCpssDxChCuckooDbIsFirst function
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
static GT_STATUS em_prvCpssDxChCuckooDbIsFirst
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFirstPtr);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

    dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    *isFirstPtr = emEntryPtr->isFirst;


    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFirst [%d]\n",bank,line,*isFirstPtr);
    }
    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbIsFirst function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in DB.
*
* @param[in] clientType      - client type
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
static GT_STATUS fdb_prvCpssDxChCuckooDbIsFirst
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFirstPtr);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        *isFirstPtr = emEntryPtr->isFirst;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, fdbManagerPtr->capacityInfo.maxTotalEntries);

        fdbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        /* convert enum key size to size in bytes */
      /*  EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(fdbEntryPtr->hwFdbEntryType,*sizeInBytesPtr);  TBD */
     /* *lookupNumberPtr = fdbEntryPtr->hwExactMatchLookupNum;  TBD */
        fdbEntryPtr = fdbEntryPtr; /*  TBD */
        *isFirstPtr = 0;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFirst [%d]\n",bank,line,*isFirstPtr);
    }
    return GT_OK;
}
#endif
/**
* @internal prvCpssDxChCuckooDbLogIsFirst function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in log operations.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogIsFirst
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    GT_BOOL is_free,j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFirstPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            is_free = (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE);
            if (is_free)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }
            *isFirstPtr = (cuckooDbPtr->log_array[j-1].isFirst == GT_TRUE);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] isFirst [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFirst);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal em_prvCpssDxChCuckooDbLogIsFirst function
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
static GT_STATUS em_prvCpssDxChCuckooDbLogIsFirst
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    return prvCpssDxChCuckooDbLogIsFirst(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,isFirstPtr);
}
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
)
{
    PRV_CPSS_DXCH_CUCKOO_MHT_ENT mhtIndex;
    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT keySize;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(depthPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    switch (cuckooDbPtr->db_banks)
    {
    case 4:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_4_E;
        break;
    case 8:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_8_E;
        break;
    case 16:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_16_E;
        break;
     default:
         CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(cuckooDbPtr->db_banks);
    }

    switch (sizeInBytes)
    {
    case 5:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E;
        break;
    case 19:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_19B_E;
        break;
    case 33:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_33B_E;
        break;
    case 47:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E;
        break;
    default:
         CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
    }
    *depthPtr = hash_depth[mhtIndex][keySize];
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("depth [%d] for sizeInBytes [%d]\n",*depthPtr,sizeInBytes);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbCountFreeGet function
* @endinternal
*
* @brief Count consecutive free banks from [bank][line] untill size.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbCountFreeGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *numFreePtr
)
{
    GT_STATUS rc;
    GT_U32 count=0, i;
    GT_BOOL is_free;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFreefunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN logIsFreeFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN);

     /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(numFreePtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    for (i=0; i<size; i++)
    {
        /* no more banks */
        if (bank+i >= cuckooDbPtr->db_banks)
        {
            break;
        }

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        /* check if [bank+i][line] is free in log */
        rc = logIsFreeFunc(managerId,bank+i,line,&is_free);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* didn't find [bank+i][line] in log; look in DB */
        if (rc == GT_NOT_FOUND)
        {
            /* check if [bank+i][line] is free in DB */
            rc = isFreefunc(managerId,bank+i,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free)
                count++; /* found free entry in DB */
            else
                break; /* no more consecutive free banks */
        }
        else
        {
            if (is_free)
                count++; /* found free entry in log */
            else
                break; /* no more consecutive free banks */
        }
    }
    *numFreePtr = count;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("number of free consecutive banks [%d] for Bank [%d] Line [%d] size [%d]\n",*numFreePtr,bank,line,size);
    }
    return GT_OK;
}

/**
* @internal em_prvCpssDxChCuckooDbCountFreeGet function
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
static GT_STATUS em_prvCpssDxChCuckooDbCountFreeGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *numFreePtr
)
{
    return prvCpssDxChCuckooDbCountFreeGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,size,numFreePtr);
}

/**
* @internal prvCpssDxChCuckooDbSortPopulatedBanks function
* @endinternal
*
* @brief Sort populated banks.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbSortPopulatedBanks
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId
)
{
    GT_U32 i,bank1,bank2,count;
    GT_BOOL move=GT_TRUE;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    while (move == GT_TRUE)
    {
       move = GT_FALSE;
       for (i=0; i<cuckooDbPtr->db_banks-1; i++)
       {
           bank1 = cuckooDbPtr->sorted_banks[i];
           bank2 = cuckooDbPtr->sorted_banks[i+1];
           if (cuckooDbPtr->bank_statistics[bank1].count < cuckooDbPtr->bank_statistics[bank2].count)
           {
               cuckooDbPtr->sorted_banks[i] = bank2;
               cuckooDbPtr->sorted_banks[i+1] = bank1;
               move = GT_TRUE;
           }
       }
    }

    /* after sorting the most populated banks are at the begining of the array.
       If the bank is full move it to the end of the array. */
    count=0;
    while (count<cuckooDbPtr->db_banks-1)
    {
        bank1 = cuckooDbPtr->sorted_banks[0];
        if (cuckooDbPtr->bank_statistics[bank1].count == cuckooDbPtr->db_lines)
        {
            for (i=0; i<cuckooDbPtr->db_banks-1; i++)
            {
                cuckooDbPtr->sorted_banks[i] = cuckooDbPtr->sorted_banks[i+1];
            }
            cuckooDbPtr->sorted_banks[i] = bank1;
            count++;
        }
        else
        {
            break;
        }

    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("sort bank [count]: ");
        for (i=0; i< cuckooDbPtr->db_banks; i++)
            cpssOsPrintf(" %d [%d] ", cuckooDbPtr->sorted_banks[i], cuckooDbPtr->bank_statistics[cuckooDbPtr->sorted_banks[i]].count );
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal em_prvCpssDxChCuckooDbSortPopulatedBanks function
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
static GT_STATUS em_prvCpssDxChCuckooDbSortPopulatedBanks
(
    IN  GT_U32                                  managerId
)
{
    return prvCpssDxChCuckooDbSortPopulatedBanks(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
}
/**
* @internal prvCpssDxChCuckooDbLogInfoSet function
* @endinternal
*
* @brief Set data in operation log in location [bank][line].
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogInfoSet
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_U32                                   lookupNumber
)
{
    GT_U32 size,offset,n;
    GT_BOOL is_free,is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFreefunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN isLockfunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsLockFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN logIsFreeFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FREE_FUN);

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    for (offset = 0; offset < size; offset++)
    {
        /* sanity check; set only on free entries */
        rc = logIsFreeFunc(managerId,bank+offset,line,&is_free);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [bank+offset][line] doesn't exist in log; check in DB */
        if (rc == GT_NOT_FOUND)
        {
            rc = isFreefunc(managerId,bank+offset,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free == GT_FALSE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in DB Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);

        }
        if (is_free == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in log Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);


        rc = isLockfunc(managerId,bank+offset,line,1,&is_lock);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+offset);

        /* insert info into log */
        if (offset == 0)
            cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_TRUE;
        else
            cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].bank = bank+offset;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].line = line;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].sizeInBytes = sizeInBytes;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isLock = is_lock;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFree = GT_FALSE;
        cpssOsMemCpy(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].data,dataPtr,sizeof(GT_U8)*sizeInBytes);
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].lookupNumber = lookupNumber;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Added Info into operation log:\n");
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations],cuckooDbPtr->numOfOperations);
        }
        cuckooDbPtr->numOfOperations++;
        cuckooDbPtr->bank_statistics[bank+offset].count++;
        cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]++;
        cuckooDbPtr->size_statistics[size-1]++;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("** increment bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
        }
    }

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbLogInfoSet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,sizeInBytes,dataPtr,lookupNumber);
}

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
)
{
    GT_U32 size,offset;
    GT_BOOL is_free;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0,n;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    for (offset = 0; offset < size; offset++)
    {
        /* sanity check; set only on occupied entries */
        rc = em_prvCpssDxChCuckooDbIsFree(managerId,bank+offset,line,&is_free);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (is_free == GT_TRUE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in DB Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank+offset);

        /* insert info into DB */
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_FALSE;
        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        emEntryPtr->hwExactMatchLookupNum = lookupNumber;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Updated cuckooDB Entry add:\n");
            CUCKOO_PRINT_DB_ENTRY1(exactMatchManagerPtr,emEntryPtr,sizeInBytes,(bank+offset),line);
        }

        if (updateStatistics) {
            cuckooDbPtr->bank_statistics[bank+offset].count++;
            cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]++;
            cuckooDbPtr->size_statistics[size-1]++;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("** increment bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
            }
        }
    }

    /* sort populated banks after log insertion */
    rc = em_prvCpssDxChCuckooDbSortPopulatedBanks(managerId);

    return rc;
}

/**
* @internal em_prvCpssDxChCuckooDbDEntryDelete function
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
)
{
    GT_U32 size,offset;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex=0;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);

    for (offset = 0; offset < size; offset++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank+offset);

        /* update info into DB */
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,(bank+offset),line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        exactMatchManagerPtr->indexArr[hwIndex].isLock = GT_FALSE;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Updated cuckooDB Entry delete from db[%d][%d]\n",bank+offset,line);
        }

        if (updateStatistics)
        {
            /* coherency check -- can't be ZERO because we are going to decrement it */
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].count);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

            cuckooDbPtr->bank_statistics[bank+offset].count--;
            cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]--;
            cuckooDbPtr->size_statistics[size-1]--;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("** decrement bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
            }
        }
    }

    /* sort populated banks after log insertion */
    rc = em_prvCpssDxChCuckooDbSortPopulatedBanks(managerId);

    return rc;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbDEntryDelete function
* @endinternal
*
* @brief Delete entry from DB in location [bank][line].
*
* @param[in] clientType      - client type
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
GT_STATUS fdb_prvCpssDxChCuckooDbEntryDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_BOOL                                  updateStatistics
)
{
    GT_U32 size,offset;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (offset = 0; offset < size; offset++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+offset);
#if 0
        /* insert info into DB */
        cuckooDbPtr->entries_db[bank+offset][line].isFirst = GT_FALSE;
        cuckooDbPtr->entries_db[bank+offset][line].sizeInBytes = 0;
        cuckooDbPtr->entries_db[bank+offset][line].isFree = GT_TRUE;
        cuckooDbPtr->entries_db[bank+offset][line].isLock = GT_FALSE;
        cpssOsMemSet(cuckooDbPtr->entries_db[bank+offset][line].data,0,sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Updated cuckooDB Entry delete:\n");
            CUCKOO_PRINT_DB_ENTRY1(cuckooDbPtr->entries_db[bank+offset][line],bank+offset,line);
        }
#endif
        if (updateStatistics)
        {
            /* coherency check -- can't be ZERO because we are going to decrement it */
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].count);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

            cuckooDbPtr->bank_statistics[bank+offset].count--;
            cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]--;
            cuckooDbPtr->size_statistics[size-1]--;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("** decrement bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
            }
        }
    }

    /* sort populated banks after log insertion */
    rc = prvCpssDxChCuckooSortPopulatedBanks(clientType,managerId);

    return rc;
}
#endif
/**
* @internal prvCpssDxChCuckooDbEntryFirstBankGet function
* @endinternal
*
* @brief Get entry's first bank.
*
* @param[in] clientType      - client type
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
static GT_STATUS prvCpssDxChCuckooDbEntryFirstBankGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *firstBankPtr
)
{
    GT_U32 free_count, back_index;
    GT_STATUS rc;
    GT_BOOL found_first;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFirstfunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFirstFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FIRST_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN countFreeGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbCountFreeGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FIRST_FUN logIsFirstFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogIsFirstFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_IS_FIRST_FUN);

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(firstBankPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* check we did not overreach banks */
    if ( bank >= cuckooDbPtr->db_banks)
    {
        *firstBankPtr = cuckooDbPtr->db_banks;
        return GT_OK;
    }

    /* count number of adjacent free banks */
    rc = countFreeGetFunc(managerId,bank,line,size,&free_count);
    if (free_count)
    {
        if (free_count == size)
        {
            *firstBankPtr = (bank + free_count + 1);
            return GT_OK;
        }
        *firstBankPtr = (bank + free_count);
        return GT_OK;
    }
    /* bank start with free - first start after free */
    else
    {
        /* check if bank is the first */
        rc = logIsFirstFunc(managerId,bank,line,&found_first);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* entry doesn't exist in log; search in db */
        if (rc == GT_NOT_FOUND)
        {
            rc = isFirstfunc(managerId,bank,line,&found_first);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (found_first)
        {
            *firstBankPtr = bank;
            return GT_OK;
        }
        /* entry begin before bank */
        else
        {
            /* go backwords untill finding first bank */
            for (back_index = PRV_CPSS_DXCH_CUCKOO_MIN_BANK_STEP_CNS; back_index < PRV_CPSS_DXCH_CUCKOO_MAX_BANK_STEP_CNS; back_index++)
            {
                /* check if bank-back_index is the first */
                rc = logIsFirstFunc(managerId,bank-back_index,line,&found_first);
                if (rc != GT_OK && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
                /* entry doesn't exist in log; search in db */
                if (rc == GT_NOT_FOUND)
                {
                    rc = isFirstfunc(managerId,bank-back_index,line,&found_first);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if (found_first)
                {
                    *firstBankPtr = (bank - back_index);
                    return GT_OK;
                }
            }
            *firstBankPtr = (bank - back_index);
            return GT_OK;
        }
    }
}

/**
* @internal em_prvCpssDxChCuckooDbEntryFirstBankGet function
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
static GT_STATUS em_prvCpssDxChCuckooDbEntryFirstBankGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *firstBankPtr
)
{
    return prvCpssDxChCuckooDbEntryFirstBankGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,size,firstBankPtr);
}

/**
* @internal prvCpssDxChCuckooDbLogInfoClear function
* @endinternal
*
* @brief Clear entry in operation log in location [bank][line].
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogInfoClear
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes
)
{
    GT_U32 i,size,n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (i = 0; i < size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        /* insert free entry info into log */
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].bank = bank+i;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].line = line;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].sizeInBytes = sizeInBytes;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isLock = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFree = GT_TRUE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].stage = 0;
        cpssOsMemSet(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].data,0,sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].lookupNumber = 0;

        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Added Clear Info into operation log:\n");
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations],cuckooDbPtr->numOfOperations);
        }

        /* coherency check -- can't be ZERO because we are going to decrement it */
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+i].count);
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]);
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

        cuckooDbPtr->numOfOperations++;
        cuckooDbPtr->bank_statistics[bank+i].count--;
        cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]--;
        cuckooDbPtr->size_statistics[size-1]--;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("** decrement bank [%d] count [%d]\n",bank+i,cuckooDbPtr->bank_statistics[bank+i].count);
        }
    }

    return GT_OK;

}

/**
* @internal em_prvCpssDxChCuckooDbLogInfoClear function
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
static GT_STATUS em_prvCpssDxChCuckooDbLogInfoClear
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes
)
{
    return prvCpssDxChCuckooDbLogInfoClear(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,bank,line,sizeInBytes);
}

/**
* @internal prvCpssDxChCuckooDbClientHashPrepare function
* @endinternal
*
* @brief Prepare hash array results per client in cuckoo format.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] numCrcMultiHashArr - number of crc hash array results
* @param[in] crcMultiHashArr - (pointer to) crc hash array results
* @param[in] *dataPtr        - (pointer to) data
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
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbClientHashPrepare
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  numCrcMultiHashArr,
    IN  GT_U32                                  crcMultiHashArr[],
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    IN  GT_BOOL                                 ignoreForbiddenIndexes,
    OUT GT_U32                                  *numHashArrPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
)
{
    GT_U32 shift,i, numOfHashes,size,bank,keySize,forbiddenIndexCounter=0,numHashArr=0;
    GT_U32 crcMultiHashIsForbiddenArr[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS] = {0};
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);

    /* prepare hash array */
    switch (clientType) {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        numOfHashes = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        if (ignoreForbiddenIndexes == GT_FALSE)
        {
            EM_CONV_SIZE_IN_BYTES_TO_KEY_ENUM_MAC(sizeInBytes,keySize);

            /* for every index in crcMultiHashArr, scan the relevant hwIndex list in DB and check
               for conflicts: one of the rules has same size,LSB and lookup number as the new/relocated entry. */
            rc = prvCpssDxChExactMatchManagerDbCalcForbiddenHashArray(managerId,GT_FALSE,dataPtr,keySize,lookupNumber,
                                                                      size,numCrcMultiHashArr,crcMultiHashArr,
                                                                      &forbiddenIndexCounter,crcMultiHashIsForbiddenArr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("number of forbidden hash indexes [%d]\n",forbiddenIndexCounter);
            }
        }

        break;
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);
        numOfHashes = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: client is not supported in cuckoo algorithm");
    }

    if (numOfHashes == 4)
    {
        shift = 2;
    }
    else if (numOfHashes == 8)
    {
        shift = 3;
    }
    else
    {
        shift = 4;
    }

    /* cuckoo hash array holds only heads (first banks) */
    for (i=0,numHashArr=0, bank=0; i<numCrcMultiHashArr/size; i++, bank +=size)
    {
        /* skip on forbidden indexes due to rule conflicts */
        if (forbiddenIndexCounter && (crcMultiHashIsForbiddenArr[bank] != 0))
        {
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("forbidden hwindex [0x%x] removed from hash array\n",crcMultiHashArr[bank]);
            }
            continue;
        }
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("added hwIndex [0x%x] to hash array\n",crcMultiHashArr[bank]);
        }

        hashArr[numHashArr].line = (crcMultiHashArr[bank] >> shift);
        /* validate line */
        PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,hashArr[numHashArr].line);
        hashArr[numHashArr].bank = bank;
        /* validate bank */
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,hashArr[numHashArr].bank);

        numHashArr++;

    }

    *numHashArrPtr = numHashArr;
    if (numHashArr == 0)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("no indexes to choose in cuckoo \n");
        }
    }
    return GT_OK;
}

/**
* @internal em_prvCpssDxChCuckooDbClientHashArrGet function
* @endinternal
*
* @brief Get hash array results per client.
*
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
static GT_STATUS em_prvCpssDxChCuckooDbClientHashArrGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    IN  GT_BOOL                                 ignoreForbiddenIndexes,
    OUT GT_U32                                  *numHashArrPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
)
{
    GT_STATUS rc;
    GT_U32 i,size=0,numHashArr=0;
    GT_U32 crcMultiHashArr[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(numHashArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    /* calc hash array */
    switch (sizeInBytes)
    {
    case 5:
        rc = em_prvCpssDxChCuckooDbHashFuncBind(managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],numHashArrPtr,crcMultiHashArr);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case 19:
        rc = em_prvCpssDxChCuckooDbHashFuncBind(managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                                dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                                dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],numHashArrPtr,crcMultiHashArr);
        if (rc != GT_OK)
        {
            return rc;
        }
        break;
    case 33:
        rc = em_prvCpssDxChCuckooDbHashFuncBind(managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                                dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                                dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                                dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                                dataPtr[31],dataPtr[32],numHashArrPtr,crcMultiHashArr);
        break;
    case 47:
        rc = em_prvCpssDxChCuckooDbHashFuncBind(managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                                dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                                dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                                dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                                dataPtr[31],dataPtr[32],dataPtr[33],dataPtr[34],dataPtr[35],dataPtr[36],dataPtr[37],dataPtr[38],
                                                dataPtr[39],dataPtr[40],dataPtr[41],dataPtr[42],dataPtr[43],dataPtr[44],dataPtr[45],dataPtr[46],
                                                numHashArrPtr,crcMultiHashArr);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
    }

    rc = prvCpssDxChCuckooDbClientHashPrepare(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,*numHashArrPtr,crcMultiHashArr,dataPtr,sizeInBytes,lookupNumber,ignoreForbiddenIndexes,&numHashArr,hashArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numHashArrPtr = numHashArr;

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("\nnum hash array %d: ",*numHashArrPtr);
        cpssOsPrintf("\nhashArr:\n");
        for (i=0; i<*numHashArrPtr; i++)
        {
            cpssOsPrintf("%d. bank [%d] line [%d]\n",i+1,hashArr[i].bank,hashArr[i].line);
        }
        cpssOsPrintf("\ncrcMultiHashArr:\n");
        for (i=0; i<(*numHashArrPtr*size); i++)
        {
            cpssOsPrintf("%d. %d\n",i+1,crcMultiHashArr[i]);
        }
    }

    return GT_OK;
}

#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbClientHashArrGet function
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
static GT_STATUS fdb_prvCpssDxChCuckooDbClientHashArrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    IN  GT_BOOL                                 ignoreForbiddenIndexes,
    OUT GT_U32                                  *numHashArrPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
)
{
    GT_STATUS rc;
    GT_U32 i,size=0,numHashArr=0;
    GT_U32 crcMultiHashArr[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(clientType,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(numHashArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    /* calc hash array */
    switch (clientType) {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        switch (sizeInBytes)
        {
        case 5:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],numHashArrPtr,crcMultiHashArr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case 19:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],numHashArrPtr,crcMultiHashArr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case 33:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                               dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                               dataPtr[31],dataPtr[32],numHashArrPtr,crcMultiHashArr);
            break;
        case 47:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                               dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                               dataPtr[31],dataPtr[32],dataPtr[33],dataPtr[34],dataPtr[35],dataPtr[36],dataPtr[37],dataPtr[38],
                                               dataPtr[39],dataPtr[40],dataPtr[41],dataPtr[42],dataPtr[43],dataPtr[44],dataPtr[45],dataPtr[46],
                                               numHashArrPtr,crcMultiHashArr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
        }

        rc = prvCpssDxChCuckooDbClientHashPrepare(clientType,managerId,*numHashArrPtr,crcMultiHashArr,dataPtr,sizeInBytes,lookupNumber,ignoreForbiddenIndexes,&numHashArr,hashArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        *numHashArrPtr = numHashArr;
        break;
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: client is not supported in cuckoo algorithm");
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("\nnum hash array %d: ",*numHashArrPtr);
        cpssOsPrintf("\nhashArr:\n");
        for (i=0; i<*numHashArrPtr; i++)
        {
            cpssOsPrintf("%d. bank [%d] line [%d]\n",i+1,hashArr[i].bank,hashArr[i].line);
        }
        cpssOsPrintf("\ncrcMultiHashArr:\n");
        for (i=0; i<(*numHashArrPtr*size); i++)
        {
            cpssOsPrintf("%d. %d\n",i+1,crcMultiHashArr[i]);
        }
    }

    return GT_OK;
}
#endif
#if 0
/**
* @internal prvCpssDxChCuckooDbDataLookupNumberCheck function
* @endinternal
*
* @brief Try to find data in DB.
*
* @param[in] clientType           - client type
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
GT_STATUS prvCpssDxChCuckooDbDataLookupNumberCheck
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    OUT GT_BOOL                                 *isFoundPtr
)
{
    GT_U32 bank,line,i,n;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];
    GT_U32 num_of_hash_results;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFoundPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* calc hash array; receive all hash indexes incuding the forbidden indexes. */
    rc = prvCpssDxChCuckooDbClientHashArrGet(clientType,managerId,dataPtr,sizeInBytes,0/*dummy*/,GT_TRUE,&num_of_hash_results,hash_result_array);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<num_of_hash_results; i++)
    {
        bank = hash_result_array[i].bank;
        line = hash_result_array[i].line;
        if (cpssOsMemCmp(cuckooDbPtr->entries_db[bank][line].data,dataPtr,sizeof(GT_U8)*sizeInBytes) == 0 &&
            cuckooDbPtr->entries_db[bank][line].sizeInBytes == sizeInBytes &&
            cuckooDbPtr->entries_db[bank][line].lookupNumber == lookupNumber)
        {
            *isFoundPtr = GT_TRUE;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                CUCKOO_PRINT_KEY(cuckooDbPtr->entries_db[bank][line].data,sizeInBytes,cuckooDbPtr->entries_db[bank][line].lookupNumber);
                cpssOsPrintf("is found in DB\n");
            }
            return GT_OK;
        }

    }

    *isFoundPtr = GT_FALSE;
    return GT_OK;
}
#endif

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
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    GT_U32 bank=0,line=0,i,n,emSizeInBytes;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];
    GT_U32 num_of_hash_results;
    GT_STATUS rc;
    *isFoundPtr = GT_FALSE;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFoundPtr);

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    /* calc hash array; receive all hash indexes incuding the forbidden indexes. */
    rc = em_prvCpssDxChCuckooDbClientHashArrGet(managerId,dataPtr,sizeInBytes,0/*dummy*/,GT_TRUE,&num_of_hash_results,hash_result_array);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<num_of_hash_results; i++)
    {
        bank = hash_result_array[i].bank;
        line = hash_result_array[i].line;

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        /* convert enum key size to size in bytes */
        EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(emEntryPtr->hwExactMatchKeySize,emSizeInBytes);

        if (cpssOsMemCmp(emEntryPtr->pattern,dataPtr,sizeof(GT_U8)*sizeInBytes) == 0 &&
            emSizeInBytes == sizeInBytes &&
            emEntryPtr->hwExactMatchLookupNum == lookupNumber)
        {
            *isFoundPtr = GT_TRUE;

            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                CUCKOO_PRINT_KEY(dataPtr,sizeInBytes,lookupNumber);
                cpssOsPrintf("is found in DB\n");
            }
            return GT_OK;
        }
    }

    *isFoundPtr = GT_FALSE;
    return GT_OK;
}
#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbDataLookupNumberCheck function
* @endinternal
*
* @brief Try to find data in DB.
*
* @param[in] clientType           - client type
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
GT_STATUS fdb_prvCpssDxChCuckooDbDataLookupNumberCheck
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    IN  GT_U32                                  lookupNumber,
    OUT GT_BOOL                                 *isFoundPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *emEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *fdbEntryPtr;
    GT_U32 bank=0,line=0,i,n,emSizeInBytes;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];
    GT_U32 num_of_hash_results;
    GT_STATUS rc;
    *isFoundPtr = GT_FALSE;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E]));

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    PRV_CPSS_DXCH_CUCKOO_DB_LOOKUP_NUMBER_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,lookupNumber);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFoundPtr);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        /* calc hash array; receive all hash indexes incuding the forbidden indexes. */
        rc = cuckooDbFuncPtr->prvCpssDxChCuckooDbClientHashArrGetFunc(managerId,dataPtr,sizeInBytes,0/*dummy*/,GT_TRUE,&num_of_hash_results,hash_result_array);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (i=0; i<num_of_hash_results; i++)
        {
            bank = hash_result_array[i].bank;
            line = hash_result_array[i].line;

            CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            emEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
            /* convert enum key size to size in bytes */
            EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(emEntryPtr->hwExactMatchKeySize,emSizeInBytes);

            if (cpssOsMemCmp(emEntryPtr->pattern,dataPtr,sizeof(GT_U8)*sizeInBytes) == 0 &&
                emSizeInBytes == sizeInBytes &&
                emEntryPtr->hwExactMatchLookupNum == lookupNumber)
            {
                *isFoundPtr = GT_TRUE;
            }
        }
        break;
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank,line);
        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, fdbManagerPtr->capacityInfo.maxTotalEntries);

        fdbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        fdbEntryPtr = fdbEntryPtr; /* TBD */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    if (*isFoundPtr == GT_TRUE)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
        {
            CUCKOO_PRINT_KEY(dataPtr,sizeInBytes,lookupNumber);
            cpssOsPrintf("is found in DB\n");
        }
    }

    return GT_OK;
}
#endif
/**
* @internal prvCpssDxChCuckooDbRehashEntry function
* @endinternal
*
* @brief Move entry using calculated hash locations ordered according to populated banks.
*
* @param[in] clientType           - client type
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
static GT_STATUS prvCpssDxChCuckooDbRehashEntry
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
)
{
    GT_STATUS rc,recursive_ret=GT_FALSE;
    GT_U32 num_of_hash_results, hash_index;
    GT_U32 ibank, revers_bank;
    GT_U32 min_depth,depth;
    GT_U32 bank,line,size,free_count,n;
    GT_BOOL is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoCurrent;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoNew;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN isLockFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsLockFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_HASH_ARR_GET_FUN clientHashArrGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbClientHashArrGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_HASH_ARR_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_MAX_DEPT_GET_FUN maxDepthGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbMaxDepthGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_MAX_DEPT_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN countFreeGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbCountFreeGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN logInfoSetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogInfoSetFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN);

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    CPSS_NULL_PTR_CHECK_MAC(entryInfoPtr);

    /* Copy current entry's info for next recursion step */
    entryInfoCurrent = *entryInfoPtr;

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* calc max depth for the entry to move */
    rc = maxDepthGetFunc(managerId,entryInfoCurrent.sizeInBytes,&depth);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* do min between new_depth and current depth */
    min_depth = (entryInfoCurrent.depth >= depth) ? depth : entryInfoCurrent.depth;

    /* Get hash array according to client exclude forbidden indexes. */
    rc = clientHashArrGetFunc(managerId,entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber,GT_FALSE,&num_of_hash_results,hash_result_array);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check all hash calc first before diving into next depth; sort according to most populated banks */
    for (ibank = 0; ibank < cuckooDbPtr->db_banks; ibank ++) {
        revers_bank = ibank;

        for (hash_index = 0; hash_index < num_of_hash_results; hash_index++)
        {
            /* try to move to new bank according to most populated bank */
            if (hash_result_array[hash_index].bank != cuckooDbPtr->sorted_banks[revers_bank]) {
                continue;
            }

            /* skip my bank */
            if ( entryInfoCurrent.bank == hash_result_array[hash_index].bank)
            {
                continue;
            }

            bank = hash_result_array[hash_index].bank;
            line = hash_result_array[hash_index].line;
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entryInfoCurrent.sizeInBytes,size);

            /* Check if [bank][line] for number of banks is locked in DB */
            rc = isLockFunc(managerId,bank,line,size,&is_lock);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* if location is locked we can't move it; continue to next hash_index */
            if (is_lock == GT_TRUE)
            {
                continue;
            }

            /* count number of adjacent free banks */
            rc = countFreeGetFunc(managerId, bank, line, size, &free_count);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (free_count >= size)
            {
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("*************3 insert to empty db[%d][%d]:\n",bank,line);
                    CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber);
                }

                /* found empty space; update log */
                rc = logInfoSetFunc(managerId, bank, line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data, entryInfoCurrent.lookupNumber);
                if (rc != GT_OK)
                {
                    return rc;
                }
                return GT_OK;
            }
        }
    }

    /* for entries try all new hash locations untill moved or failed all */
    for (ibank = 0; ibank < cuckooDbPtr->db_banks; ibank++) {
        revers_bank = ibank;

        for (hash_index = 0; hash_index < num_of_hash_results; hash_index++)
        {
            /* try to move to new bank according to most populated bank */
            if (hash_result_array[hash_index].bank != cuckooDbPtr->sorted_banks[revers_bank]) {
                continue;
            }

            /* skip my bank */
            if ( entryInfoCurrent.bank == hash_result_array[hash_index].bank)
            {
                continue;
            }

            /* insert to hash location data in current location */
            /* prepare entryInfo */
            entryInfoNew.bank = hash_result_array[hash_index].bank;
            entryInfoNew.line = hash_result_array[hash_index].line;
            entryInfoNew.sizeInBytes = entryInfoCurrent.sizeInBytes;
            cpssOsMemCpy(entryInfoNew.data,entryInfoCurrent.data,sizeof(GT_U8)*entryInfoCurrent.sizeInBytes);
            entryInfoNew.lookupNumber = entryInfoCurrent.lookupNumber;
            entryInfoNew.depth = min_depth;

            recursive_ret = prvCpssDxChCuckooDbRecursiveEntryAdd(clientType,managerId,&entryInfoNew);
            /* moved the entry */
            if (recursive_ret == GT_OK)
            {
                return GT_OK;
            }
        }
    }

    /* failed to set */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

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
)
{
    return prvCpssDxChCuckooDbRehashEntry(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,entryInfoPtr);
}

/**
* @internal prvCpssDxChCuckooDbRoolbackLogArray function
* @endinternal
*
* @brief   Delete all log operations from startIndex untill the
*          end due to failure in moving last entries.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[in] startIndex           - start location in log operations.
* @param[in] updateCounters       - whether to update counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbRoolbackLogArray
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   startIndex,
    IN GT_BOOL                                  updateCounters
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 size;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    if (startIndex >= cuckooDbPtr->numOfOperations)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Error: startIndex should be less then cuckooDB.numOfOperations [%d]\n",cuckooDbPtr->numOfOperations);

    if (updateCounters)
    {
        /* update statistics before delete */
        for (i=startIndex; i<cuckooDbPtr->numOfOperations; i++)
        {
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(cuckooDbPtr->log_array[i].sizeInBytes,size);
            if (cuckooDbPtr->log_array[i].isFree == GT_FALSE)
            {
                /* coherency check -- can't be ZERO because we are going to decrement it */
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]);
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count--;
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]--;
                cuckooDbPtr->size_statistics[size-1]--;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("** decrement bank [%d] count [%d]\n",cuckooDbPtr->log_array[i].bank,cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                }
            }
            else
            {
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count++;
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]++;
                cuckooDbPtr->size_statistics[size-1]++;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("** increment bank [%d] count [%d]\n",cuckooDbPtr->log_array[i].bank,cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                }
            }
        }
    }

    for (i=startIndex; i<cuckooDbPtr->numOfOperations; i++)
    {
        cuckooDbPtr->log_array[i].bank = 0;
        cuckooDbPtr->log_array[i].line = 0;
        cpssOsMemSet(cuckooDbPtr->log_array[i].data, 0, sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);
        cuckooDbPtr->log_array[i].lookupNumber = 0;
        cuckooDbPtr->log_array[i].isFirst = 0;
        cuckooDbPtr->log_array[i].sizeInBytes = 0;
        cuckooDbPtr->log_array[i].isFree = GT_TRUE;
        cuckooDbPtr->log_array[i].stage = 0;
    }
    cuckooDbPtr->numOfOperations =  startIndex;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cleared operation log from index [%d]\n",startIndex);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogArrayDelete function
* @endinternal
*
* @brief   Delete all log operations.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogArrayDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId
)
{
    return prvCpssDxChCuckooDbRoolbackLogArray(clientType,managerId,0,GT_FALSE);
}

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
)
{
    return prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
}

/**
* @internal prvCpssDxChCuckooDbRecursiveEntryAdd function
* @endinternal
*
* @brief Add new entry in calculated hash index. If location
*        is occupied try to move all entries from this location.
*
* @param[in] clientType           - client type
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
static GT_STATUS prvCpssDxChCuckooDbRecursiveEntryAdd
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
)
{
    GT_STATUS rc,recursive_ret;
    GT_U32 entry_first_bank,entries_index, log_current_index,n;
    GT_U32 free_count; /* number of adjacent free banks */
    GT_U32 size;       /* number of banks */
    GT_U32 size_to_replaceInBytes;
    GT_U32 entry_size=0, entry_sizeInBytes=0, entry_lookupNumber=0;
    GT_U32 lock_size, lock_start, lock_end, bank_check_end; /* parameters for locking entries */
    GT_U8 entry_data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    GT_BOOL is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoCurrent;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoNew;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN isLockFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsLockFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_LOCK_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_FUN sizeLookupNumberGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbSizeLookupNumberGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_SIZE_LOOKUP_NUMBER_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_FUN dataLookupNumberGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbDataLookupNumberGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_DATA_LOOKUP_NUMBER_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOCK_SET_FUN lockSetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLockSetFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOCK_SET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_REHASH_ENTRY_FUN rehashEntryFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbRehashEntryFunc,PRV_CPSS_DXCH_CUCKOO_DB_REHASH_ENTRY_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN countFreeGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbCountFreeGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_COUNT_FREE_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_FIRST_BANK_GET_FUN entryFirstBankGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbEntryFirstBankGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_FIRST_BANK_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN logInfoSetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogInfoSetFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_SET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_FUN logDataLookupNumberGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogDataLookupNumberGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_DATA_LOOKUP_NUMBRT_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_FUN logSizeLookupNumberGetFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogSizeLookupNumberGetFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_SIZE_LOOKUP_NUMBER_GET_FUN);
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_CLEAR_FUN logInfoClearFunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbLogInfoClearFunc,PRV_CPSS_DXCH_CUCKOO_DB_LOG_INFO_CLEAR_FUN);

    typedef struct
    {
        GT_U32 size;        /* number of banks data occupy */
        GT_U32 sizeInBytes; /* number of bytes data occupy */
        GT_U32 bank;
        GT_U32 lookup;
    }entry_stc;
    GT_U32 entries_max;     /*number of entries to move in first_entries array */
    entry_stc first_entries[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS]; /* array of entries to move */

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    CPSS_NULL_PTR_CHECK_MAC(entryInfoPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* Copy current entry's info for next recursion step */
    entryInfoCurrent = *entryInfoPtr;

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entryInfoCurrent.sizeInBytes,size);

    /* if location is free insert - first recursive stop */
    cuckooDbPtr->recurse_count++;

    /* Check if [bank][line] for number of banks is locked in DB */
    rc = isLockFunc(managerId,entryInfoCurrent.bank,entryInfoCurrent.line,size,&is_lock);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if location is locked we can't move it; return fail */
    if (is_lock == GT_TRUE)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("ERROR entry is locked  bank [%d] line [%d] depth [%d]\n",entryInfoCurrent.bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* count number of adjacent free banks */
    rc = countFreeGetFunc(managerId,entryInfoCurrent.bank, entryInfoCurrent.line, size, &free_count);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* found adjacent free banks sufficient for current entry */
    if (free_count >= size)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("*************1 insert to empty db[%d][%d]:\n",entryInfoCurrent.bank,entryInfoCurrent.line);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber);
        }

        /* found empty space; update log */
        rc = logInfoSetFunc(managerId,entryInfoCurrent.bank, entryInfoCurrent.line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data, entryInfoCurrent.lookupNumber);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }

    /* if depth is 0 - second recursive stop */
    CUCKOO_CHECK_X_NOT_ZERO_MAC(entryInfoCurrent.depth);
    entryInfoCurrent.depth--;
    if (entryInfoCurrent.depth <= 0)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("ERROR entry reached max depth  bank [%d] line [%d] depth [%d]\n",entryInfoCurrent.bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* find entry's first bank  */
    rc = entryFirstBankGetFunc(managerId,entryInfoCurrent.bank, entryInfoCurrent.line, size, &entry_first_bank);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* set lock boundaries */
    lock_start = (entry_first_bank < entryInfoCurrent.bank) ? entry_first_bank : entryInfoCurrent.bank;
    /* check that lock end did not over reach max banks */
    bank_check_end = lock_end = (entryInfoCurrent.bank + size >= cuckooDbPtr->db_banks) ? cuckooDbPtr->db_banks : entryInfoCurrent.bank + size;

    /* zero entries array */
    cpssOsMemSet(first_entries,0,sizeof(entry_stc)*PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS);

    /* create array of entries to change location from lock_start untill bank_check_end */
    for (entries_max = 0; entry_first_bank < bank_check_end; entries_max++)
    {
        rc = logSizeLookupNumberGetFunc(managerId,entry_first_bank, entryInfoCurrent.line, &entry_sizeInBytes, &entry_lookupNumber);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [entry_first_bank][line] doesn't exist in operation log; look in DB */
        if (rc == GT_NOT_FOUND) {
            rc = sizeLookupNumberGetFunc(managerId, entry_first_bank, entryInfoCurrent.line, &entry_sizeInBytes, &entry_lookupNumber);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* dont move locked entries */
        PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entry_sizeInBytes,entry_size);
        rc = isLockFunc(managerId,entry_first_bank,entryInfoCurrent.line,entry_size,&is_lock);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (is_lock == GT_TRUE)
        {
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("ERROR entry is locked  bank [%d] line [%d] depth [%d]\n",entry_first_bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        first_entries[entries_max].bank = entry_first_bank;
        first_entries[entries_max].size = entry_size;
        first_entries[entries_max].sizeInBytes = entry_sizeInBytes;
        first_entries[entries_max].lookup = entry_lookupNumber;

        /* update last */
        if (entry_size + entry_first_bank > lock_end)
        {
            lock_end = entry_size + entry_first_bank;
        }

        /* find entry inside array it's first bank  */
        rc = entryFirstBankGetFunc(managerId,entry_first_bank + entry_size, entryInfoCurrent.line, size, &entry_first_bank);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* set lock */
    lock_size = lock_end - lock_start;

    /* lock [lock_start][line] for lock_size inorder not to have loops in recurse */
    rc = lockSetFunc(managerId, lock_start, entryInfoCurrent.line, lock_size, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("locked myself bank [%d] line [%d] lock size [%d] lookupNumer [%d]\n",lock_start,entryInfoCurrent.line,lock_size,entryInfoCurrent.lookupNumber);
    }

    /* try to move all entries; save log current index incase of failure for roolback */
    log_current_index = cuckooDbPtr->numOfOperations;
    for (entries_index = 0; entries_index < entries_max; entries_index++)
    {
        size_to_replaceInBytes = first_entries[entries_index].sizeInBytes;

        /* data and lookup number of entry to move */
        rc = logDataLookupNumberGetFunc(managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes, entry_data, &entry_lookupNumber);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [first_entries[entries_index].bank][line] doesn't exist in log; look in DB */
        if (rc == GT_NOT_FOUND) {
            rc = dataLookupNumberGetFunc(managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes, entry_data, &entry_lookupNumber);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* for entries try all new hash locations untill move succeeded or tried and failed all */
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("try to move from db[%d][%d]:\n",first_entries[entries_index].bank,entryInfoCurrent.line);
            CUCKOO_PRINT_KEY(entry_data,size_to_replaceInBytes,entry_lookupNumber);
        }

        /* prepare entryInfoNew */
        entryInfoNew.bank = entryInfoCurrent.bank;
        entryInfoNew.sizeInBytes = size_to_replaceInBytes;
        cpssOsMemCpy(entryInfoNew.data,entry_data,sizeof(GT_U8)*size_to_replaceInBytes);
        entryInfoNew.depth = entryInfoCurrent.depth;
        entryInfoNew.line = entryInfoCurrent.line;
        entryInfoNew.lookupNumber = entry_lookupNumber;

        /* try to move entryInfoNew */
        recursive_ret = rehashEntryFunc(managerId,&entryInfoNew);
        /* failed to move - stop and return false */
        if (recursive_ret == GT_FAIL)
        {
            /* unlock [lock_start][line] for lock_size */
            rc = lockSetFunc(managerId,lock_start, entryInfoCurrent.line, lock_size, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("failed to move from db[%d][%d]:\n",first_entries[entries_index].bank,entryInfoCurrent.line);
                CUCKOO_PRINT_KEY(entry_data,size_to_replaceInBytes,entry_lookupNumber);
                cpssOsPrintf("FAILURE unlocked myself bank [%d] line [%d] lock size [%d]\n",lock_start,entryInfoCurrent.line,lock_size);
            }

            /* roolback log operations */
            if (cuckooDbPtr->numOfOperations > log_current_index) {
                rc = prvCpssDxChCuckooDbRoolbackLogArray(clientType,managerId,log_current_index,GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("reset log from %d to %d\n", cuckooDbPtr->numOfOperations, log_current_index);
                }
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* move succeeded; mark entry as free in operation log */
        rc = logInfoClearFunc(managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("free location db[%d][%d] sizeInBytes [%d]\n",first_entries[entries_index].bank,entryInfoCurrent.line,size_to_replaceInBytes);
        }
    }

    /* moved all entries */
    /* unlock [lock_start][line] for lock_size */
    rc = lockSetFunc(managerId,lock_start, entryInfoCurrent.line, lock_size, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("SUCCESS unlocked myself bank [%d] line [%d] lock size [%d]\n",lock_start,entryInfoCurrent.line,lock_size);
        cpssOsPrintf("*************2 insert after moving all entries db[%d][%d]:\n",entryInfoCurrent.bank,entryInfoCurrent.line);
        CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,entryInfoCurrent.lookupNumber);
    }

    /* set data in log */
    rc = logInfoSetFunc(managerId,entryInfoCurrent.bank, entryInfoCurrent.line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data, entryInfoCurrent.lookupNumber);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogUpdateStages function
* @endinternal
*
* @brief    Update stages in cuckoo operation log.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogUpdateStages
(
   IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
   IN GT_U32                                   managerId
)
{
    GT_U32 i, line, prevLine,n;
    GT_U32 stageCounter=0;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* count max stage */
    for (i=0, prevLine=cuckooDbPtr->log_array[0].line; i<cuckooDbPtr->numOfOperations; i++)
    {
        line = cuckooDbPtr->log_array[i].line;
        /* don't count clear operations */
        if (cuckooDbPtr->log_array[i].isFree != GT_TRUE)
        {
            /* parts are on same stage */
            if (prevLine != line)
                stageCounter++;

            prevLine = line;
        }
    }

    /* update stage in log operations starting from the last element */
    for (i=0, prevLine=cuckooDbPtr->log_array[0].line; i<cuckooDbPtr->numOfOperations; i++)
    {
        line = cuckooDbPtr->log_array[i].line;
        /* don't count clear operations */
        if (cuckooDbPtr->log_array[i].isFree != GT_TRUE)
        {
            /* parts are on same stage */
            if (prevLine != line)
            {
                CUCKOO_CHECK_X_NOT_ZERO_MAC(stageCounter);
                stageCounter--;
            }

            prevLine = line;
        }
        cuckooDbPtr->log_array[i].stage = stageCounter;

        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[i],i);
        }
    }

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbLogUpdateStages(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
}

/**
* @internal prvCpssDxChCuckooDbLogPtrGet function
* @endinternal
*
* @brief   The function returns pointer to log array in cuckoo DB.
*
* @param[in] clientType           - client type
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
static GT_STATUS prvCpssDxChCuckooDbLogPtrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT            clientType,
    IN  GT_U32                                      managerId,
    OUT GT_U32                                      *numOfOperationsPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC       **cuckooDbLogPtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(numOfOperationsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbLogPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    *cuckooDbLogPtr = cuckooDbPtr->log_array;
    *numOfOperationsPtr = cuckooDbPtr->numOfOperations;

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbLogPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,numOfOperationsPtr,cuckooDbLogPtr);
}

/**
* @internal prvCpssDxChCuckooDbStatisticsPtrGet function
* @endinternal
*
* @brief   The function returns pointer to statistics in cuckoo DB.
*
* @param[in] clientType           - client type
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
static GT_STATUS prvCpssDxChCuckooDbStatisticsPtrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT            clientType,
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC         **cuckooDbBankStPtr,
    OUT GT_U32                                      **cuckooDbSizeStPtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbBankStPtr);
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbSizeStPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    *cuckooDbBankStPtr = cuckooDbPtr->bank_statistics;
    *cuckooDbSizeStPtr = cuckooDbPtr->size_statistics;

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbStatisticsPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,cuckooDbBankStPtr,cuckooDbSizeStPtr);
}

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
)
{
    GT_U32 bank;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr=NULL;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;
    GT_U32 tblSize;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    if (managerId >= PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in manageId type [%d]",managerId);
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: cuckooDB for clientType [%d] managerId [%d] already exist",PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] = (PRV_CPSS_DXCH_CUCKOO_DB_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] == NULL)
    {
        goto exit_cleanly_lbl;
    }
    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    cpssOsMemSet(cuckooDbPtr,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));

    cpssOsMemSet(cuckooDbPtr->log_array,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC)*PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS);
    cuckooDbPtr->numOfOperations = 0;
    cuckooDbPtr->recurse_count = 0;

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
    cuckooDbPtr->db_banks = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

    switch(exactMatchManagerPtr->cpssHashParams.size)
    {
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
            tblSize=_8KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
            tblSize=_16KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
            tblSize=_32KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
            tblSize=_64KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
            tblSize=_128KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
            tblSize=_256KB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
    }
    cuckooDbPtr->db_lines = tblSize/cuckooDbPtr->db_banks;

    cuckooDbPtr->bank_statistics = (PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
    if (cuckooDbPtr->bank_statistics == NULL)
    {
        goto exit_cleanly_lbl;
    }

    cuckooDbPtr->sorted_banks = (GT_U32 *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(GT_U32));
    if (cuckooDbPtr->sorted_banks == NULL)
    {
        goto exit_cleanly_lbl;
    }

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    /* bind functions */
    rc = prvCpssDxChCuckooDbCallbacksInit(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    return GT_OK;

exit_cleanly_lbl:

    CPSS_LOG_ERROR_MAC("Error: Failed to allocate memory for cuckoo DB\n");
    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(cuckooDbPtr);
        cuckooDbPtr = NULL;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
}
#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbInit function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
GT_STATUS fdb_prvCpssDxChCuckooDbInit
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
)
{
    GT_U32 bank;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr=NULL;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr=NULL;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;
    GT_U32 tblSize;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    if (managerId >= PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in manageId type [%d]",managerId);
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: cuckooDB for clientType [%d] managerId [%d] already exist",clientType,managerId);
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = (PRV_CPSS_DXCH_CUCKOO_DB_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        goto exit_cleanly_lbl;
    }
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    cpssOsMemSet(cuckooDbPtr,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));

    cpssOsMemSet(cuckooDbPtr->log_array,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC)*PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS);
    cuckooDbPtr->numOfOperations = 0;
    cuckooDbPtr->recurse_count = 0;

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        cuckooDbPtr->db_banks = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        switch(exactMatchManagerPtr->cpssHashParams.size)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
                tblSize=_8KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
                tblSize=_16KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
                tblSize=_32KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
                tblSize=_64KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
                tblSize=_128KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
                tblSize=_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
        }
        cuckooDbPtr->db_lines = tblSize/cuckooDbPtr->db_banks;

        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);
        cuckooDbPtr->db_banks = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        switch(fdbManagerPtr->cpssHashParams.size)
        {
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E:
                tblSize=_8KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E:
                tblSize=_16KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E:
                tblSize=_32KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E:
                tblSize=_64KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E:
                tblSize=_128KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E:
                tblSize=_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->cpssHashParams.size);
        }
        cuckooDbPtr->db_lines = tblSize/cuckooDbPtr->db_banks;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
    }

    cuckooDbPtr->bank_statistics = (PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
    if (cuckooDbPtr->bank_statistics == NULL)
    {
        goto exit_cleanly_lbl;
    }

    cuckooDbPtr->sorted_banks = (GT_U32 *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(GT_U32));
    if (cuckooDbPtr->sorted_banks == NULL)
    {
        goto exit_cleanly_lbl;
    }

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    /* bind functions */
    rc = prvCpssDxChCuckooDbCallbacksInit(clientType);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    return GT_OK;

exit_cleanly_lbl:

    CPSS_LOG_ERROR_MAC("Error: Failed to allocate memory for cuckoo DB\n");
    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(cuckooDbPtr);
        cuckooDbPtr = NULL;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
}
#endif

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] == NULL)
    {
        return GT_OK;
    }

    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);

    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId]);
        PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] = NULL;
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E][managerId] = NULL;

    return GT_OK;
}
#if 0
/**
* @internal fdb_prvCpssDxChCuckooDbDelete function
* @endinternal
*
* @brief   Delete cuckoo DB per client type and managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS fdb_prvCpssDxChCuckooDbDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        return GT_OK;
    }

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    if (cuckooDbPtr)
    {
#if 0
        if (cuckooDbPtr->entries_db)
        {
            for (bank=0; bank<cuckooDbPtr->db_banks; bank++)
            {
                if (cuckooDbPtr->entries_db[bank])
                {
                    cpssOsFree(cuckooDbPtr->entries_db[bank]);
                    cuckooDbPtr->entries_db[bank] = NULL;
                }
            }
            cpssOsFree(cuckooDbPtr->entries_db);
            cuckooDbPtr->entries_db = NULL;
        }
#endif
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]);
        PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = NULL;
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = NULL;

    return GT_OK;
}
#endif
/**
* @internal prvCpssDxChCuckooDbPrintStatistics function
* @endinternal
*
* @brief   Print cuckoo DB statistics per client type and
*          managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbPrintStatistics
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 i,j,bank,line,count_banks=0;
    GT_BOOL is_free;
    GT_STATUS rc;
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC *cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[clientType]));
    PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN isFreefunc = PRV_CALLBACK_GET(cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc,PRV_CPSS_DXCH_CUCKOO_DB_IS_FREE_FUN);

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);


    for (line = 0; line < cuckooDbPtr->db_lines; line++)
    {
        for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
        {
            rc = isFreefunc(managerId,bank,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free == GT_FALSE)
            {
                count_banks++;
            }
        }
    }

    cpssOsPrintf("CuckooDB Statistics:\n\n");
    cpssOsPrintf("Capacity percents (%d%)\n",(count_banks*100)/(cuckooDbPtr->db_banks*cuckooDbPtr->db_lines));
    cpssOsPrintf("entry size ");
    for (i=0; i<PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E; i++)
    {
        cpssOsPrintf("[%d]: %d ",i+1,cuckooDbPtr->size_statistics[i]);
    }
    cpssOsPrintf("\n\n");

    for (i=0; i<cuckooDbPtr->db_banks; i++)
    {
        cpssOsPrintf("bank [%d]: %d\n", i,cuckooDbPtr->bank_statistics[i].count);
        for (j=0; j<PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E; j++)
        {
            cpssOsPrintf("\t size[%d] = %d\n",j+1,cuckooDbPtr->bank_statistics[i].sizes[j]);
        }
    }
    cpssOsPrintf("\n");

    cpssOsPrintf("sort bank [count]: ");
    for (i=0; i< cuckooDbPtr->db_banks; i++) {
        cpssOsPrintf(" %d [%d] ", cuckooDbPtr->sorted_banks[i], cuckooDbPtr->bank_statistics[cuckooDbPtr->sorted_banks[i]].count);
    }
    cpssOsPrintf("\n");

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr=NULL;
    GT_U32 i,bank;
    GT_U32 hwIndex,dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *entryPtr;
    GT_U32 sizeInBytes,size,step;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    /* check all exact match entries defined in the DB */
    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        /* Validate used list content */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;

            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            entryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

            /* convert Exact Match key enum to size in bytes */
            EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->hwExactMatchKeySize,sizeInBytes);
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);

            /* calc bank from hwIndex */
            EM_BANK_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank);

            /* update statistics */
            for (i=0; i<size; i++)
            {
                cuckooDbPtr->bank_statistics[bank+i].count++;
                cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]++;
                cuckooDbPtr->size_statistics[size-1]++;
            }

              /* jump to the next hwIndex to check */
            step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
            hwIndex = hwIndex+step;
        }
        else
        {
            /* else - In case of invalid entry dbIndex will not be valid to check */
            hwIndex++;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbInitDebug function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId
*          for debug.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
static GT_STATUS prvCpssDxChCuckooDbInitDebug
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId,
    IN GT_U32                               db_banks,
    IN GT_U32                               db_lines
)
{
    GT_U32 bank;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: cuckooDB for clientType [%d] managerId [%d] already exist",clientType,managerId);
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = (PRV_CPSS_DXCH_CUCKOO_DB_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        goto exit_cleanly_lbl;
    }
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    cpssOsMemSet(cuckooDbPtr,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));

    cpssOsMemSet(cuckooDbPtr->log_array,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC)*PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS);
    cuckooDbPtr->numOfOperations = 0;
    cuckooDbPtr->recurse_count = 0;
    cuckooDbPtr->db_banks = db_banks;
    cuckooDbPtr->db_lines = db_lines;
    cuckooDbPtr->bank_statistics = (PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
    if (cuckooDbPtr->bank_statistics == NULL)
    {
        goto exit_cleanly_lbl;
    }

    cuckooDbPtr->sorted_banks = (GT_U32 *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(GT_U32));
    if (cuckooDbPtr->sorted_banks == NULL)
    {
        goto exit_cleanly_lbl;
    }

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    return GT_OK;

exit_cleanly_lbl:

    CPSS_LOG_ERROR_MAC("Error: Failed to allocate memory for cuckoo DB\n");
    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(cuckooDbPtr);
        cuckooDbPtr = NULL;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
}

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
)
{
    return prvCpssDxChCuckooDbInitDebug(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId,db_banks,db_lines);
}

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
)
{
    GT_U32 i, bank, line, n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC                             *cuckooDbPtr = NULL;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  dbIndex,hwIndex=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *entryPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,managerId);
    EM_MANAGER_ID_CHECK(managerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

    /* update all the log operations starting from the last element */
    for (i=0; i<cuckooDbPtr->numOfOperations; i++)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[i],i);
        }
        bank = cuckooDbPtr->log_array[i].bank;
        line = cuckooDbPtr->log_array[i].line;

        CUCKOO_HW_INDEX_FROM_BANK_LINE_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,bank,line,hwIndex);
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        entryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

        cpssOsMemCpy(entryPtr->pattern, cuckooDbPtr->log_array[i].data, sizeof(GT_U8)*cuckooDbPtr->log_array[i].sizeInBytes);
        entryPtr->hwExactMatchLookupNum = cuckooDbPtr->log_array[i].lookupNumber;
        entryPtr->isFirst = cuckooDbPtr->log_array[i].isFirst;
        EM_CONV_SIZE_IN_BYTES_TO_KEY_ENUM_MAC(cuckooDbPtr->log_array[i].sizeInBytes,entryPtr->hwExactMatchKeySize);
        entryPtr->isUsedEntry = !cuckooDbPtr->log_array[i].isFree;
        exactMatchManagerPtr->indexArr[hwIndex].isLock = cuckooDbPtr->log_array[i].isLock;
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_FUNC_PTR_STC * cuckooDbFuncPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        cuckooDbFuncPtr =&(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc.cuckooDbFuncPtr[PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E]));

        cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFreeFunc                  = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbIsFree);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbIsFirstFunc                 = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbIsFirst);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbIsLockFunc                  = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbIsLock);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbSizeLookupNumberGetFunc     = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbSizeLookupNumberGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbDataLookupNumberGetFunc     = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbDataLookupNumberGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbClientHashArrGetFunc        = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbClientHashArrGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbMaxDepthGetFunc             = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbMaxDepthGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbStatisticsPtrGetFunc        = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbStatisticsPtrGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLockSetFunc                 = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbSetLock);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbEntryAddFunc                = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbEntryAdd);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbEntryDeleteFunc             = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbEntryDelete);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbDataLookupNumberCheckFunc   = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbDataLookupNumberCheck);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbHashFuncBindFunc            = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbHashFuncBind);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbInitFunc                    = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbInit);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbDeleteFunc                  = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbDelete);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbPrintStatisticsFunc         = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbPrintStatistics);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbSyncStatisticsFunc          = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbSyncStatistics);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbInitDebugFunc               = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbInitDebug);
        cuckooDbFuncPtr->prvCpssDxChCuckooRelocateEntriesDebugFunc      = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooRelocateEntriesDebug);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbRehashEntryFunc             = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbRehashEntry);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbCountFreeGetFunc            = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbCountFreeGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbSortPopulatedBanksFunc      = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbSortPopulatedBanks);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbEntryFirstBankGetFunc       = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbEntryFirstBankGet);

        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogPtrGetFunc               = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogPtrGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogUpdateStagesFunc         = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogUpdateStages);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogArrayDeleteFunc          = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogArrayDelete);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogInfoSetFunc              = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogInfoSet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogDataLookupNumberGetFunc  = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogDataLookupNumberGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogIsFreeFunc               = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogIsFree);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogIsFirstFunc              = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogIsFirst);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogSizeLookupNumberGetFunc  = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogSizeLookupNumberGet);
        cuckooDbFuncPtr->prvCpssDxChCuckooDbLogInfoClearFunc            = PRV_CALLBACK_ID_GET(em_prvCpssDxChCuckooDbLogInfoClear);
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
    }

    return GT_OK;
}

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
)
{
    /*due to const will be located in read only section*/
    static GT_STATUS (* const cuckooFunctionDb[])(GT_VOID *) =
    {
       (GT_VOID*)em_prvCpssDxChCuckooDbIsFree,
       (GT_VOID*)em_prvCpssDxChCuckooDbIsFirst,
       (GT_VOID*)em_prvCpssDxChCuckooDbIsLock,
       (GT_VOID*)em_prvCpssDxChCuckooDbSizeLookupNumberGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbDataLookupNumberGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbClientHashArrGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbMaxDepthGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbStatisticsPtrGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbDataLookupNumberCheck,
       (GT_VOID*)em_prvCpssDxChCuckooDbSetLock,
       (GT_VOID*)em_prvCpssDxChCuckooDbEntryAdd,
       (GT_VOID*)em_prvCpssDxChCuckooDbEntryDelete,
       (GT_VOID*)em_prvCpssDxChCuckooDbHashFuncBind,
       (GT_VOID*)em_prvCpssDxChCuckooDbInit,
       (GT_VOID*)em_prvCpssDxChCuckooDbDelete,
       (GT_VOID*)em_prvCpssDxChCuckooDbPrintStatistics,
       (GT_VOID*)em_prvCpssDxChCuckooDbSyncStatistics,
       (GT_VOID*)em_prvCpssDxChCuckooDbInitDebug,
       (GT_VOID*)em_prvCpssDxChCuckooRelocateEntriesDebug,
       (GT_VOID*)em_prvCpssDxChCuckooDbRehashEntry,
       (GT_VOID*)em_prvCpssDxChCuckooDbCountFreeGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbSortPopulatedBanks,
       (GT_VOID*)em_prvCpssDxChCuckooDbEntryFirstBankGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogPtrGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogUpdateStages,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogArrayDelete,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogInfoSet,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogDataLookupNumberGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogIsFree,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogIsFirst,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogSizeLookupNumberGet,
       (GT_VOID*)em_prvCpssDxChCuckooDbLogInfoClear,
       NULL
    };

    return (GT_VOID *)cuckooFunctionDb;
}

