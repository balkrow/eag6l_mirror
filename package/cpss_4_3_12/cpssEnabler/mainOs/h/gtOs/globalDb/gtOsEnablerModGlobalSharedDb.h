/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file gtOsEnablerModGlobalSharedDb.h
*
* @brief This file define cpssEnabler module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsEnablerModGlobalSharedDb
#define __gtOsEnablerModGlobalSharedDb

#ifdef LINUX
#include <pthread.h>
#include <stdio.h>
#endif

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/simStubs.h>
#include <gtOs/osObjIdLib.h>
#include <gtUtil/gtBmPool.h>
#ifdef ASIC_SIMULATION
#include <gtOs/gtOsSharedMemory.h>
#endif
#include <cpss/extServices/cpssExtServices.h>
#ifdef LINUX
#include <gtOs/linuxSimEther.h>
#include <gtOs/linuxLtaskLib.h>
#include <gtOs/gtOsSharedMalloc.h>
#endif

#ifdef SHARED_MEMORY
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedIPC.h>
#ifndef ASIC_SIMULATION
#include <gtOs/gtOsSharedPp.h>
#endif
#endif

/**
* @struct PRV_CPSS_ENABLER_DEVICE_DMA_INFO_DB
*
* @brief  Structure contain global variables such as device details required by DMA driver
*         that are meant to be shared between processes
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    /* Is the PCIe BDF info valid */
    GT_BOOL     isPciInfoValid;
    /* PCIe bus number of the device */
    GT_U8       pciDomainNum;
    /* PCIe bus number of the device */
    GT_U8       pciBusNum;
    /* PCIe device number of the device */
    GT_U8       pciDevNum;
    /* PCIe function number of the device */
    GT_U8       pciFunNum;
    /* Is the PCIe BDF info valid */
    GT_BOOL     isDmaInfoValid;
    /* Number of DMA buffers - Number of elements in the list of dmaBufPhy64[]*/
    GT_U8       dmaBufCount;
    /* Size of each DMA buffer */
    GT_U32      dmaBufLen;
    /* List of physical addresses of each DMA buffer */
    GT_U64_BIT  *dmaBufPhy64;
    /* Index into dmaBufPhy64[] used to build DMA range list for DMA window configuration */
    GT_U32      dmaScatterBufIndex;
} PRV_CPSS_ENABLER_DMA_DEVICE_INFO_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
*         between processes about DMA 'per window'.
*         NOTE: in SMMU system each device need to have it's own DMA , as multiple
*           devices can not overlap DMA memory
*         a system without SMMU can be with single DMA window.
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    OSMEM_POOL_FREE_ELEM_PTR freeDmaPool;
#if (! defined CONFIG_V2_DMA)
#ifdef LINUX
    /*DMA section*/
    SHARED_MEM_STC cdma_mem;
#endif /* LINUX */

    /* DMA virtual base address (as returned from the mmap(...))*/
    GT_UINTPTR  dmaBaseVirtAddr;/*baseVirtAddr*/
    /* DMA base address (as converted from the dmaBaseVirtAddr)*/
    GT_UINTPTR  dmaBaseAddr;/*baseAddr*/
    /* the DMA window size in bytes */
    GT_U32      dmaWindowSize;/*dmaSize*/
#endif /*! CONFIG_V2_DMA*/
} PRV_CPSS_ENABLER_DMA_WINDOW_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinuxMem file
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL  firstInitialization;

    GT_MUTEX freeDmaPoolMtx;

#ifdef ASIC_SIMULATION
    /* id of the shared memory */
    GT_SH_MEM_ID simDmaAsSharedMemId;
    GT_MUTEX simDmaMutexId;
    /* pointer to the start of the shared memory */
    GT_VOID* simDmaAsSharedMemPtr;
    /* size of the shared memory */
    GT_U32 simDmaAsSharedMemSize;
    /* offset ued from start of the shared memory  */
    GT_U32 simDmaAsSharedMemOffsetUsed;
#endif

    PRV_CPSS_ENABLER_DMA_WINDOW_DB dmaWindowsArr[GT_MEMORY_DMA_CONFIG_WINDOWS_CNS];
  /** @brief : number of active windows in dmaWindowsArr[]
   */
  GT_U32               dmaWindowsNumOfActiveWindows;
  /** @brief : the current index in dmaWindowsArr[] , that will be accessed.
   */
  GT_U32               dmaWindowsCurrentWindow;

  PRV_CPSS_ENABLER_DMA_DEVICE_INFO_DB dmaDeviceInfo[GT_MEMORY_DMA_DEVICE_INFO_CNS];
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinuxIo file
*/
typedef struct
{
    /*add here global  shared variables used in source files*/

    void* writeFunctionParam;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SELECT_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinuxSelect.c file
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    GT_POOL_ID   socketFdPool; /* pool ID to be used for allocating memory for FD sets */

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SELECT_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SOCKET_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinuxSocket.c file
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    GT_POOL_ID   socketAddrPool; /* pool ID to be used for allocating memory for socket address structures */

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SOCKET_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_MEM_LIB_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / gtOsMemLib file
*/

typedef struct
{
    /*add here global  shared variables used in source files*/
    GT_BOOL firstMemInit;
    OSMEM_POOL_HDR_PTR* os_mem_pools_array ;
    GT_U32 os_mem_pools_array_size;
    GT_MUTEX os_mem_pools_mtx_id ;
    OSMEM_CHUNK_ARRAY os_mem_chunks_array;
    GT_U32 os_mem_poolsLimit;

    DMA_ALLOCATION_STC* globalDmaAllocationsFirstPtr;
    DMA_ALLOCATION_STC* globalDmaAllocationsLastPtr;
    GT_U32              underSystemReset;
    GT_U32              dmaTotalAlloc;
    GT_U32              dmaTotalAlloc_traceEnable;
    GT_U32              heap_start_counter_value;
    GT_32               countDynamic;
    GT_32               countStatic;
    GT_32               countDynamicOver;
    GT_U32              heap_bytes_allocated;
    GT_U32              mallocCounter;
    GT_U32              mallocFailCounterLimit;
    GT_BOOL             mallocFailEnable;
    MEM_ALLOCATION_STC  globalMemAllocations;
#ifdef INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG
    MEM_ALLOCATION_STC* globalMemAllocationsLastPtr;
#endif /* INTERNAL_OS_MALLOC_MEMORY_LEAKAGE_DBG */
    OSMEM_MEMORY_LEAKAGE_STATE_DBG_ENT globalMemoryLeakageState;
    GT_BOOL             dmaOverflowDetectionEnabled;
    GT_U32              lastValue;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_MEM_LIB_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_MALLOC_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /shrMemSharedMalloc.c  file
*/

typedef struct
{
    /*add here global  shared variables used in source files*/
    GT_VOID * shmFreeBufferList;
    GT_VOID * prvShrMemBlockList;
    /* the size of huge buffers (include also  managed by pool manager)*/
    GT_U32    occupiedBuffersSize;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_MALLOC_SRC_GLOBAL_DB;


#ifdef SHARED_MEMORY
#ifndef ASIC_SIMULATION

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_MALLOC_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /shrMemPpInit.c  file
*/

typedef struct
{
    /*add here global  shared variables used in source files*/

    /* Registry of mmaped sections (located into shared memory) */
    PP_MMAP_INFO_STC pp_mmap_sections_registry[PP_REGISTRY_SIZE_CNS];
    int pp_mmap_registry_index;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_PP_INIT_SRC_GLOBAL_DB;

#endif
#endif

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_BM_POOL_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / gtBmPool.c  file
*/

typedef struct
{
    /*add here global  shared variables used in source files*/

    /* Flag to prevent initialization more than once */
    GT_BOOL       isInitialized;

    /* a semaphore to prevent more than one process to access the pool module */
     GT_MUTEX        poolMtx;

    /* Array of created pools */
    GT_VOID *     poolArray ;
    GT_U32        freePoolHeaderIdx ;/* Index of the next free pool header.*/

   /* Max number of pools supported by this module */
    GT_U32        poolMaxPools ;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_BM_POOL_SRC_GLOBAL_DB;

#ifdef IMPL_GALTIS
typedef struct
{
    /*add here global  shared variables used in source files*/

    /* keep RxTx parameters struct definition */
    CPSS_RX_TX_MP_MODE_PARAMS_STC RxTxProcessParams;
#ifdef CHX_FAMILY
    CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC transmitParams;
#endif
    CPSS_RX_TX_PROC_COM_CMD_STC rxTxGaltisCommands;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_SHR_MEM_SHR_DATA_SRC_GLOBAL_DB;
#endif

#ifdef ASIC_SIMULATION
typedef struct
{
    /*add here global  shared variables used in source files*/

     SHOSTP_intr_STC  SHOSTP_intr_table[SHOSTP_max_intr_CNS];
     GT_SEM           SHOSTP_intr_table_mtx;
     GT_BOOL          initDone;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_LINUX_SIM_STUBS_SRC_GLOBAL_DB;
#endif

typedef struct
{
    /*add here global  shared variables used in source files*/

    GT_BOOL gtOsWrapperOpen;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_GEN_SRC_GLOBAL_DB;

#ifdef LINUX
typedef struct
{
    /*add here global  shared variables used in source files*/

    pthread_mutex_t semaphores_mtx;
    int initialized;
    OS_OBJECT_LIST_STC semaphores;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_PTHR_LSEM_SRC_GLOBAL_DB;

typedef struct
{
    /*add here global  shared variables used in source files*/

    pthread_mutex_t mutex;
    OS_OBJECT_LIST_STC list;

} PRV_PROTECTED_LIST;

typedef struct
{
    /*add here global  shared variables used in source files*/

    int initialized;
    PRV_PROTECTED_LIST mqList;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_DB;

typedef struct
{
    /*add here global  shared variables used in source files*/

#ifdef SHARED_MEMORY
    int initialized;
#endif
    PRV_PROTECTED_LIST userspaceMutexeList;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_DB;

typedef struct
{
    /*add here global  shared variables used in source files*/

    PRV_PROTECTED_LIST mqList;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_DB;

#ifdef ASIC_SIMULATION
typedef struct
{
    /*add here global  shared variables used in source files*/

    GT_Rx_FUNCP userRxCallback;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_SIM_ETHER_SRC_GLOBAL_DB;
#endif

typedef struct
{
    /*add here global  shared variables used in source files*/

    OS_OBJECT_LIST_STC tasks;
    _V2L_pthread_hash_STC pthreadHashTbl[V2L_PTHREAD_HASH_TABLE_SIZE_CNS];
#ifdef SHARED_MEMORY
    int initialized;
#endif
    pthread_mutex_t tasks_mtx;
    /* taskLock data */
    pthread_mutex_t taskLock_mtx;
    pthread_cond_t taskLock_cond;
    int taskLock_count;
    pthread_t taskLock_owner;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_LTASK_LIB_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / ltaskLib.c file
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    pthread_mutex_t v2pthread_file_lock; /* v2pthread_file_lock is a mutex used to make open log file a critical section */
    FILE *fpLog;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_LTASK_LIB_SRC_GLOBAL_DB;

#endif /*LINUX*/

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_TEST_PERF_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /osTestPerf.c  file
*/

typedef struct
{
    GT_U32   start_s;
    GT_U32   start_n;
    GT_SEM   t_sem1;
    GT_SEM   t_sem2;
    GT_SEM   t_semE;
    int t_count;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_TEST_PERF_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_HITLESS_UPGRD_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /gtHitlessUpgrd.c  file
*/

typedef struct
{
    /* osAttachStageDone - determines if the OS already performed attach. Needed  */
    /*                     to prevent duplicate executions due to calls from tapi */
    /*                     attach and core attach.                                */
    GT_BOOL osAttachStageDone;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_HITLESS_UPGRD_SRC_GLOBAL_DB;

#ifdef SHARED_MEMORY
/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_ENUM_SHDATA_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /shrMemEnumeratedSharedData.c  file
*/

typedef struct
{
    uintptr_t sharedDataArray[CPSS_SHARED_DATA_MAX_E];
    GT_SEM semMtx;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_ENUM_SHDATA_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_IPC_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory /shrMemIPC.c  file
*/

typedef struct
{
    CPSS_MP_REMOTE remoteArray[CPSS_MP_REMOTE_MAX_E];
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_IPC_SRC_GLOBAL_DB;
#endif

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB                 osLinuxMemSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB                  osLinuxIoSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SELECT_SRC_GLOBAL_DB              osLinuxSelectSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_LINUX_SOCKET_SRC_GLOBAL_DB              osLinuxSocketSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_MEM_LIB_SRC_GLOBAL_DB                gtOsMemLibSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_BM_POOL_SRC_GLOBAL_DB                   gtBmPoolSrc;
#ifdef IMPL_GALTIS
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_SHR_MEM_SHR_DATA_SRC_GLOBAL_DB          gtShrMemShrDataSrc;
#endif
#ifdef ASIC_SIMULATION
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_LINUX_SIM_STUBS_SRC_GLOBAL_DB           gtLinuxSimStubsSrc;
#endif
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_GEN_SRC_GLOBAL_DB              gtOsLinuxGenSrc;
#ifdef LINUX
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_PTHR_LSEM_SRC_GLOBAL_DB        gtOsLinuxPthrLsemSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_PTHR_LMSGQLIB_SRC_GLOBAL_DB    gtOsLinuxPthrLsmggLibSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_KE_LSEMLIB_SRC_GLOBAL_DB       gtOsKeLsemLibSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_MSG_Q_SRC_GLOBAL_DB            gtOsLinuxMsgQSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_LTASK_LIB_SRC_GLOBAL_DB                    ltaskLibSrc;
#ifdef ASIC_SIMULATION
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_SIM_ETHER_SRC_GLOBAL_DB        gtOsLinuxSimEtherSrc;
#endif
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_OS_LINUX_LTASKLIB_SRC_GLOBAL_DB         gtOsLtaskLibSrc;
#endif

    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_MALLOC_SRC_GLOBAL_DB                 gtOsShmemMallocSrc;
#ifdef SHARED_MEMORY
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_ENUM_SHDATA_SRC_GLOBAL_DB            gtOsShmemEnumShdataSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_IPC_SRC_GLOBAL_DB                    gtOsShmemIpcSrc;
#ifndef ASIC_SIMULATION
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_SHMEM_PP_INIT_SRC_GLOBAL_DB                gtOsShmemPpInitSrc;
#endif
#endif

    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_OS_TEST_PERF_SRC_GLOBAL_DB                 osTestPrefSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GT_HITLESS_UPGRD_SRC_GLOBAL_DB             gtHitlessUpgrdSrc;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module
*/
typedef struct
{
    /*add here directories*/
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GLOBAL_DB mainOsDir;

} PRV_CPSS_ENABLER_MOD_SHARED_GLOBAL_DB;

#endif /* __gtOsEnablerModGlobalSharedDb */


