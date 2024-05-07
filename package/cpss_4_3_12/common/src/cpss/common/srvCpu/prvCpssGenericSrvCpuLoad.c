/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file prvCpssGenericSrvCpuLoad.c
*
* @brief Service CPU firmware load/start APIs
*
* @version   1
********************************************************************************
*/
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/generic/ipc/mvShmIpc.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/version/cpssGenStream.h>

#include <cpss/common/srvCpu/fw_ids_gen.h>

#define FW_DEFAULT_PATH         "/opt/marvell/firmware"
#define SRVCPU_ID_ALL           0xFF
#define ARRAY_SIZE(x)           ((sizeof(x)) / (sizeof(x[0])))
#define DEV_FMLY_SUB_FMLY(x)    (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(x)->devSubFamily<<16 |\
                                 PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(x)->devFamily)
#ifdef CPU_BE
  #define LE_SWAP_32BIT(x) (x)
  #define LE_SWAP_16BIT(x) (x)
#else
  #define LE_SWAP_32BIT(x) CPSS_BYTE_SWAP_32BIT(x)
  #define LE_SWAP_16BIT(x) CPSS_BYTE_SWAP_16BIT(x)
#endif

#ifndef ASIC_SIMULATION

typedef struct
{
    GT_BOOL isFsFile;
    union {
        CPSS_OS_FILE_TYPE_STC fsFile;
        PRV_CPSS_FIRMWARE_ENTRY_STC   *memFile;
    };
} UNIFIED_FILE_STC;

static GT_VOID prvCpssSrvCpuUniFileClose(UNIFIED_FILE_STC *uniFile);

typedef struct {
    CPSS_PP_FAMILY_TYPE_ENT devFamily; /* devSubFamily<<16 | devFamily. Relevant only for default DB */
    GT_U32  fw_id;
    GT_U8 module_ver_major;
    GT_U8 module_ver_minor;
    GT_U8 mcv_major;
    GT_U8 mcv_minor;
    GT_U16 srvCpuId;
    GT_U8 readOnly; /* used only by DEFAULT_FW_DB  */
    GT_U8 devNum; /* used only by optional FW DB */
    GT_CHAR* name;
    struct FW_DB_ENTRY_STC* next;
} FW_DB_ENTRY_STC;

#define PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.srvcpuDir._var)

#define PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR_SET(_var, _value)\
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.srvcpuDir._var, _value)

typedef struct {
    GT_U32  fw_id;
    GT_U8 module_ver_major;
    GT_U8 module_ver_minor;
    GT_U8 mcv_major;
    GT_U8 mcv_minor;
} OPTIONAL_FW_DB_ENTRY_STC;

static const FW_DB_ENTRY_STC DEFAULT_FW_DB[] = {
    /*   Device Family type		      FW ID       Ver[maj,mnr]  MCV[maj,mnr] srvCpuId         readOnly  N/A     Name	                     N/A   */
    {CPSS_PP_FAMILY_DXCH_BOBCAT2_E, AP_BC2_FW_ID,       1,0 ,       1,0 ,  SRVCPU_ID_DONT_CARE,  0,      0,   "Bc2Fw",                       NULL},
    {CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E << 16 | CPSS_PP_FAMILY_DXCH_BOBCAT2_E,
                                    AP_BOBK_MSYS_FW_ID, 1,0 ,       1,0 ,  SRVCPU_ID_DONT_CARE,  0,      0,   "BobkFw",                      NULL}, /* Bobk */
    {CPSS_PP_FAMILY_DXCH_XCAT3_E,   AP_BC2_FW_ID,       1,0 ,       1,0 ,  SRVCPU_ID_DONT_CARE,  0,      0,   "Bc2Fw",                       NULL}, /* AC3 */
    {CPSS_PP_FAMILY_DXCH_ALDRIN_E,  AP_ALDRIN_FW_ID,    1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3AldrinFw",  NULL},
    {CPSS_PP_FAMILY_DXCH_AC3X_E,    AP_ALDRIN_FW_ID,    1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3AldrinFw",  NULL},
    {CPSS_PX_FAMILY_PIPE_E,         AP_PIPE_FW_ID,      1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3PipeFw",    NULL},
    {CPSS_PP_FAMILY_DXCH_BOBCAT3_E, AP_BC3_FW_ID,       1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3Bc3Fw",     NULL},
    {CPSS_PP_FAMILY_DXCH_FALCON_E,  AP_RAVEN_FW_ID,     1,0 ,       1,0 ,    SRVCPU_ID_ALL,      0,      0,   "mvHwsServiceCpuCm3RavenFw",   NULL},
    {CPSS_PP_FAMILY_DXCH_ALDRIN2_E, AP_ALDRIN2_FW_ID,   1,0,        1,0,         0,              0,      0,   "mvHwsServiceCpuCm3Aldrin2Fw", NULL},
    {CPSS_PP_FAMILY_DXCH_AC5_E,     AP_AC5_FW_ID,       1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3Ac5Fw",     NULL},
    {CPSS_PP_FAMILY_DXCH_IRONMAN_E, AP_IRONMAN_FW_ID,   1,0 ,       1,0 ,        0,              0,      0,   "mvHwsServiceCpuCm3IronmanFw", NULL},
    {CPSS_PP_FAMILY_DXCH_AC5X_E,    DUMMY_AC5X_FW_ID,   1,0 ,       1,0 ,        0,              0,      0,   "L1FwDummyAc5xFw",             NULL},
};


static const OPTIONAL_FW_DB_ENTRY_STC OPTIONAL_FW_DB[] = {
    /*      FW ID             Ver[maj,mnr]  MCV[maj,mnr] */
    {DBA_BC3_FW_ID,                1,0 ,         1,0} ,

    {FLOW_MNG_FALCON_FW_ID,        1,0 ,         1,0} ,
    {FLOW_MNG_ALDRIN2_FW_ID,       1,0 ,         1,0} ,

    {TAM_BC3_FW_ID,                1,0 ,         1,0} ,
    {TAM_FALCON_FW_ID,             1,0 ,         1,0} ,
    {TAM_ALDRIN2_FW_ID,            1,0 ,         1,0} ,

    {IPFIX_AC5P_FW_ID,             1,0 ,         1,0} ,
    {IPFIX_AC5X_FW_ID,             1,0 ,         1,0} ,
    {IPFIX_FALCON_FW_ID,             1,0 ,         1,0} ,
};



static const FW_DB_ENTRY_STC* prvCpssSrvCpuGetOptionalFW(
    IN GT_U8 devNum,
    IN GT_U16 srvCpuId
)
{
    FW_DB_ENTRY_STC *entry;
    unsigned i;

    for (entry = (FW_DB_ENTRY_STC*)PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(optionalFwListHead);
         entry; entry = (FW_DB_ENTRY_STC*)entry->next)
    {
        if ( (entry->devNum == devNum) && (entry->srvCpuId == srvCpuId) )
        {
            /* If device's srvCpu is reserved(read only), it couldn't here,
               becuase we might be running before init system
            */
            for (i=0; i<ARRAY_SIZE(DEFAULT_FW_DB); i++)
            {
                if ( (DEFAULT_FW_DB[i].devFamily == DEV_FMLY_SUB_FMLY(devNum)) &&
                     (DEFAULT_FW_DB[i].srvCpuId == srvCpuId) )
                {
                    if (DEFAULT_FW_DB[i].readOnly)
                        return &DEFAULT_FW_DB[i]; /* Read-only entry*/
                    else
                        return entry;
                }
            }
            return entry;
        }
    }

    return NULL;
}


static GT_STATUS internal_cpssSrvCpuSetOptionalFW(
    IN GT_U8 devNum,
    IN GT_U16 srvCpuId,
    IN GT_U32 fw_id,
    IN GT_CHAR* name
)
{
    FW_DB_ENTRY_STC *entry = NULL;
    GT_U32 i;
    GT_BOOL newEnrty = GT_TRUE;
    FW_DB_ENTRY_STC *optionalFwListHead = (FW_DB_ENTRY_STC*)PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(optionalFwListHead);

    if (!name || (!cpssOsStrlen(name)) || !fw_id)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* If an optional FW already exist for [devNum, srvCpuID] just replace the name */
    for (entry = optionalFwListHead; entry; entry = (FW_DB_ENTRY_STC*)entry->next)
    {
        if ( (entry->devNum == devNum) && (entry->srvCpuId == srvCpuId) ) {
            cpssOsFree(entry->name);
            newEnrty = GT_FALSE;
            break;
        }
    }

    if (newEnrty)
    {
        entry = cpssOsMalloc(sizeof(FW_DB_ENTRY_STC));
        if (!entry)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    entry->name = cpssOsMalloc(cpssOsStrlen(name));
    if (!entry->name)
    {
        cpssOsFree(entry);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    entry->devNum = devNum;
    entry->srvCpuId = srvCpuId;
    entry->fw_id = fw_id;
    entry->readOnly = 0;
    entry->mcv_major = entry->mcv_minor = 0xFF; /* Set default 'No validation' */
    for (i = 0; i < ARRAY_SIZE(OPTIONAL_FW_DB); i++)
    {
        if (OPTIONAL_FW_DB[i].fw_id == fw_id)
        {
            entry->module_ver_major = OPTIONAL_FW_DB[i].module_ver_major;
            entry->module_ver_minor = OPTIONAL_FW_DB[i].module_ver_minor;
            entry->mcv_major = OPTIONAL_FW_DB[i].mcv_major;
            entry->mcv_minor = OPTIONAL_FW_DB[i].mcv_minor;
            break;
        }
    }
    cpssOsStrCpy(entry->name, name);

    if (newEnrty)
    {
        if (!optionalFwListHead)
            entry->next = NULL;
        else
            entry->next = (struct FW_DB_ENTRY_STC*)optionalFwListHead;
        PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR_SET(optionalFwListHead, entry);
    }

    return GT_OK;
}


/**
*@internal cpssSrvCpuSetOptionalFW function
* @endinternal
*
* @brief Add a FW to be loaded to [dev, srvCpu]
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
*
* @param[in] devNum                   - device number,
* @param[in] srvCpuId                 - serice CPU ID
* @param[in] fw_id                    - a unique FW id, usually defined in
*      fw_ids.h
* @param[in] name                     - FW name (W/O .fw suffix)
*/
GT_STATUS cpssSrvCpuSetOptionalFW(
    IN GT_U8 devNum,
    IN GT_U8 srvCpuId,
    IN GT_U32 fw_id,
    IN GT_CHAR* name
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSrvCpuSetOptionalFW);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srvCpuId, fw_id, name));

    rc = internal_cpssSrvCpuSetOptionalFW(devNum, srvCpuId, fw_id, name);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srvCpuId, fw_id, name));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


GT_VOID prvCpssGenericSrvCpuDestroy
(
    GT_VOID
)
{
    FW_DB_ENTRY_STC *entry, *tmp;

    entry = (FW_DB_ENTRY_STC*)PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(optionalFwListHead);
    while (entry)
    {
        tmp = (FW_DB_ENTRY_STC*)entry->next;
        cpssOsFree(entry);
        entry = tmp;
    }
}


static const struct PRV_CPSS_SRVCPU_TYPES_STCT {
    GT_U16      devId;
    GT_U16      devIdMask;
    PRV_SRVCPU_OPS_FUNC ops;
} prvSrvCpuDescr[] = {
    { 0xfc00, 0xff00, &prvSrvCpuOpsMsys }, /* bobcat2 */
    { 0xbe00, 0xff00, &prvSrvCpuOpsBobK }, /* cetus */
    { 0xbc00, 0xff00, &prvSrvCpuOpsBobK }, /* caelum */
    { 0xf400, 0xfe00, &prvSrvCpuOpsMsys }, /* alleycat3\s */
    { 0xc800, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin */
    { 0xd400, 0xff00, &prvSrvCpuOpsCM3  }, /* bc3 */
    { 0xc400, 0xff00, &prvSrvCpuOpsCM3  }, /* pipe */
    { 0xcc00, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin2 */
    { 0x8600, 0xfc00, &prvSrvCpuOpsCM3  }, /* Falcon - 12.8/6.4 : 0x86xx or 0x84xx (note the mask 0xfc00)  */
    { 0xb400, 0xff00, &prvSrvCpuOpsCM3_AC5  }, /* AC5 */
    { 0x9800, 0xff00, &prvSrvCpuOpsCM3_AC5  }, /* AC5X */
    { 0x9400, 0xff00, &prvSrvCpuOpsCM3_AC5P  }, /* AC5P */
    { 0xa200, 0xf000, &prvSrvCpuOpsCM3_AC5  }, /* ironman M 0xa200 or ironman L 0xa000*/
    { 0xffff, 0xffff, NULL }
};

PRV_SRVCPU_OPS_FUNC prvSrvCpuGetOps(
    IN  GT_U8   devNum
)
{
    GT_U32 i;
    GT_U16 deviceId; /* PCI device Id */

    i = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->devType;
    deviceId = (i >> 16) & 0xffff;

    for (i = 0; prvSrvCpuDescr[i].ops; i++)
    {
        if ((deviceId & prvSrvCpuDescr[i].devIdMask) == (prvSrvCpuDescr[i].devId & prvSrvCpuDescr[i].devIdMask))
            break;
    }

    if(prvSrvCpuDescr[i].ops == NULL)
    {
        CPSS_LOG_ERROR_MAC("The deviceId[0x%4.4x] not defined as 'supporting' Service CPU (see prvSrvCpuDescr[])",
            deviceId);
    }
    return prvSrvCpuDescr[i].ops;
}
#else
/* next 2 functions needed for 'ASIC_SIMULATION_ENV_FORBIDDEN' that those functions needed by linker by 'EXTHWG_POE_CM3_Ipc_driverInit' */
PRV_SRVCPU_OPS_FUNC prvSrvCpuGetOps(
    IN  GT_U8   devNum
)
{
    (void)devNum;
    CPSS_LOG_ERROR_MAC("prvSrvCpuGetOps : not implemented for WM");
    return NULL;
}
GT_VOID prvIpcAccessInit(PRV_SRVCPU_IPC_CHANNEL_STC *s, GT_BOOL init)
{
    (void)s;
    (void)init;
    CPSS_LOG_ERROR_MAC("prvIpcAccessInit : not implemented for WM");
}
#endif

#ifndef ASIC_SIMULATION

#define REMOTE_UPGRADE_HEADER_SIZE 0x0C /* 12  */
#define REMOTE_UPGRADE_MSG_MIN_LEN 0x4  /* 4  */
#define REMOTE_UPGRADE_MSG_MAX_LEN 0xF8 /* 248 */

/* defined in boot_channel.h */
typedef enum {  MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NOR_FLASH_NOT_EXIST_E   = 0x012,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_PRIMARY_FW_HEADER_TYPE_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_PRIMARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_SECONDARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NEW_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E
} MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_ENT;

typedef struct { /* remote upgrade message header */
/*  type        name              byte order */
    GT_U16      opCode;       /*   0  - 1    */
    GT_U16      bmp;          /*   2  - 3    */
    GT_U32      bufNum;       /*   4  - 7    */
    GT_U16      bufSize;      /*   8  - 9    */
    GT_U8       checkSum;     /*   10        */
    GT_U8       reseved;      /*   11        */
} upgradeMsgHdr;


typedef struct {
    GT_U32  checksum; /* Always first */
    PRV_CPSS_FIRMWARE_ENTRY_STC   fw_entry;
} FS_HEADER_STC;

typedef struct {
    PRV_CPSS_FIRMWARE_ENTRY_STC   fw_entry;
    GT_U8      *fwData;
    GT_U32     fwArrSize; /* size of fw */
    GT_U32     compress;   /* 0 - no compression, 2 - simple compression */
    GT_U8      *metaData;
}  MEM_FILE_DB_ENTRY_STC;


/* 8 bit checksum */
static GT_U8 prvChecksum8(GT_U8* start, GT_U32 len, GT_U8 csum)
{
    GT_U8  sum = csum;
    GT_U8* startp = start;

    do {
        sum += *startp;
        startp++;

    } while(--len);

    return (sum);
}


static GT_U16 prvGenChecksum16(GT_U16* start, GT_U32 u16Len)
{
    GT_U16  sum = 0;

    do {
        sum += *start;
        start++;
    } while(--u16Len);

    return (sum);
}


GT_STATUS internal_cpssSrvCpuSetUsrPath(
    IN const char* path
)
{
    CPSS_OS_FILE_TYPE_STC fsFile;
    GT_CHAR* prvSrvCpuUsrPath = PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(prvSrvCpuUsrPath);

    if ( !path || (cpssOsStrlen(path) >= PRV_CPSS_GENERIC_SRV_CPU_USR_PATH_LEN_CNS) )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    if (!cpssOsFopen(path, "r", &fsFile) )
        cpssOsPrintf("\nSpecified path for firmware files (%s) does not exist!\n", path);
    else
        cpssOsFclose(&fsFile);

    cpssOsStrCpy(prvSrvCpuUsrPath, path);
    return GT_OK;
}


/**
* @internal cpssSrvCpuSetUsrPath function
* @endinternal
*
* @brief Adds FWs search path.
*     This path shall have the highest precedence.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
*
* @param[in] path                    - Up to 128 chars path
*/
GT_STATUS cpssSrvCpuSetUsrPath(
    IN const char* path
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSrvCpuSetUsrPath);

    CPSS_LOG_API_ENTER_MAC((funcId, path));

    rc = internal_cpssSrvCpuSetUsrPath(path);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, path));

    return rc;
}


static GT_VOID prvCpssGenericSrvCpuIpcInit(
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    PRV_SRVCPU_IPC_CHANNEL_STC *s,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
    if (ipcConfig != NULL)
    {
        /* setup ipc... */
        prvIpcAccessInit(s, GT_TRUE);
        for (; ipcConfig->channel != 0xff; ipcConfig++)
        {
            shmIpcConfigChannel(&(s->shm), ipcConfig->channel,
                    ipcConfig->maxrx, ipcConfig->rxsize,
                    ipcConfig->maxtx, ipcConfig->txsize);
        }
        /* create fwChannel object */

        /* Write magic to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 0, IPC_RESVD_MAGIC);
        /* Write pointer to SHM registers to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 1, s->shmAddr + 0x100);
    }
    if (preStartCb != NULL)
        preStartCb((GT_UINTPTR)s, preStartCookie);
    if (fwChannelPtr != NULL)
        *fwChannelPtr = (GT_UINTPTR)s;
    else
        prvIpcAccessDestroy(s);
}

/**
* @internal prvCpssFindFile function
* @endinternal
*
* @brief   Searches for the SerDes firmware file in the specified path. 
*          If the firmware file is found, the function returns its full path.
*
* @param[in]  fwFilePathAndName         - buffer with the full path to the firmware file 
* @param[in]  fwFilePathAndNameLength   - the length of the buffer with the full path to the firmware file 
* @param[out] serdesFwDataPtr           - (pointer to) data related to SerDes firmware
*
* @retval GT_OK                         - on success 
* @retval GT_NOT_FOUND                  - on not found
* @retval GT_NO_RESOURCE                - on system error
*/
GT_STATUS prvCpssFindFile
(
    IN const GT_CHAR * fwFilePathAndName,
    IN GT_U32          fwFilePathAndNameLength,
    OUT PRV_CPSS_SERDES_FIRMWARE_DATA_STC * serdesFwDataPtr
) 
{
    CPSS_OS_FILE                fd;
    CPSS_OS_FILE_TYPE_STC       file;
    GT_CHAR                    *fwFilePathAndNamePtr = NULL;


    fd = cpssOsFopen(fwFilePathAndName, "r", &file);
    if (fd != 0)
    {
        cpssOsFclose(&file);
        fwFilePathAndNamePtr = cpssOsMalloc(fwFilePathAndNameLength);
        if (NULL == fwFilePathAndNamePtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        cpssOsStrCpy(fwFilePathAndNamePtr, fwFilePathAndName);
        /* Assign SerDes data to the output structure */
        serdesFwDataPtr->fwMemType = FW_PATH_INFO;
        serdesFwDataPtr->fwData.fwPathInfo.fwFullPathNamePtr = fwFilePathAndNamePtr;
    }
    else
    {
        /* Firmware file is not found in specified path */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssSerdesFwSearch function
* @endinternal
*
* @brief   This function searches for the SerDes firmware file in multiple locations based on priority.
*          If the firmware file is found in any of the specified paths, it returns the full path to the firmware file.
*          The search is done in the following order:
*  
*          1. User path: The path specified by the prvSrvCpuUsrPath variable.
*          2. Default firmware path: The path specified by FW_DEFAULT_PATH macro.
*          3. Default firmware path with the "baseline" directory appended.
*          4. Linked FW: If the firmware is not found in any of the previous paths, it tries to retrieve
*            the firmware from linked memory using prvCpssSerdesFirmareMemGet function.
*  
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe
*
* @param[in] devNum                 - the device number to search for firmware
* @param[in] fwName                 - the name of the firmware
* @param[in] fwVersion              - the firmware version
* @param[out] serdesFwDataPtr       - (pointer to) data related to SerDes firmware
*
* @retval GT_OK                     - on success 
* @retval GT_NOT_FOUND              - on not found 
* @retval GT_BAD_PARAM              - on device is not exist 
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_BAD_STATE              - on database error
* @retval GT_NO_RESOURCE            - on system error
*/
static GT_STATUS prvCpssSerdesFwSearch
(
    IN GT_32                                devNum,
    IN GT_CHAR *                            fwName,
    IN GT_CHAR *                            fwVersion,
    OUT PRV_CPSS_SERDES_FIRMWARE_DATA_STC  *serdesFwDataPtr
)
{
    GT_STATUS rc;
    char fullFileName[160];

    GT_CHAR *prvUsrPathPtr = PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(prvSrvCpuUsrPath);

    switch (devNum)
    {
        case AC5P_DEV_ID:
            cpssOsStrCat(fwName, "AC5P");
            break;
        case AC5X_DEV_ID:
            cpssOsStrCat(fwName, "AC5X");
            break;
        case HARRIER_DEV_ID:
            cpssOsStrCat(fwName, "HARRIER");
            break;
        case IRONMAN_DEV_ID:
            cpssOsStrCat(fwName, "IRONMAN");
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* User path */
    if (prvUsrPathPtr)
    {
        cpssOsSprintf(fullFileName, "%s/%s%s.srds", prvUsrPathPtr, fwName, fwVersion);
        rc = prvCpssFindFile(fullFileName, cpssOsStrlen(fullFileName), serdesFwDataPtr);
        if (rc == GT_OK)
        {
            return GT_OK;
        }
        if (rc != GT_NOT_FOUND)
        {
            return rc;
        }
    }
    /* /opt/marvell/fw */
    cpssOsSprintf(fullFileName, "%s/%s%s.srds", FW_DEFAULT_PATH, fwName, fwVersion);
    rc = prvCpssFindFile(fullFileName, cpssOsStrlen(fullFileName), serdesFwDataPtr);
    if (rc == GT_OK)
    {
        return GT_OK;
    }
    if (rc != GT_NOT_FOUND)
    {
        return rc;
    }

    /* /opt/marvell/fw/cpss_baseline */
    cpssOsSprintf(fullFileName, "%s/%s/%s%s.srds", FW_DEFAULT_PATH, CPSS_STREAM_NAME_CNS, fwName, fwVersion);
    rc = prvCpssFindFile(fullFileName, cpssOsStrlen(fullFileName), serdesFwDataPtr);
    if (rc == GT_OK)
    {
        return GT_OK;
    }
    if (rc != GT_NOT_FOUND)
    {
        return rc;
    }

    /* linked FW */
    return prvCpssSerdesFirmareMemGet(fwName, serdesFwDataPtr);
}

static GT_STATUS prvCpssSrvCpuFwSearchPath(IN char *fileName, IN GT_U32  fw_id, OUT UNIFIED_FILE_STC *uniFile)
{
    GT_BOOL use_linked_fw = GT_FALSE;
    char buf[160];
    GT_CHAR* prvSrvCpuUsrPath = PRV_SHARED_COMMON_DIR_SRVCPU_SRC_GLOBAL_VAR(prvSrvCpuUsrPath);

    CPSS_NULL_PTR_CHECK_MAC(uniFile);
    CPSS_NULL_PTR_CHECK_MAC(fileName);

    uniFile->isFsFile = GT_TRUE;
    uniFile->fsFile.type = CPSS_OS_FILE_REGULAR;

    /* User path */
    if (prvSrvCpuUsrPath[0])
    {
        cpssOsSprintf(buf, "%s/%s.fw", prvSrvCpuUsrPath, fileName);
        uniFile->fsFile.fd = cpssOsFopen(buf, "r", &(uniFile->fsFile));
        if (uniFile->fsFile.fd != 0)
            goto FW_LOADED;
        else
            use_linked_fw = GT_TRUE;

    }

    if (!use_linked_fw)
    {
        /* /opt/marvell/fw */
        cpssOsSprintf(buf, "%s/%s.fw", FW_DEFAULT_PATH, fileName);
        uniFile->fsFile.fd = cpssOsFopen(buf, "r", &(uniFile->fsFile));
        if (uniFile->fsFile.fd != 0)
            goto FW_LOADED;

        /* /opt/marvell/fw/cpss_baseline */
        cpssOsSprintf(buf, "%s/%s/%s.fw", FW_DEFAULT_PATH, CPSS_STREAM_NAME_CNS, fileName);
        uniFile->fsFile.fd = cpssOsFopen(buf, "r", &(uniFile->fsFile));
        if (uniFile->fsFile.fd != 0)
            goto FW_LOADED;
    }

    /* linked FW */
    uniFile->memFile = prvCpssSrvCpuFirmareOpen(fw_id);
    if (uniFile->memFile) {
        uniFile->isFsFile = GT_FALSE;
        cpssOsSprintf(buf, "linked FW (corresponding to %s)", fileName);
        goto FW_LOADED;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

FW_LOADED:
    cpssOsPrintf("Loaded ");
    if (uniFile->isFsFile)
        cpssOsPrintf("FW ");
    cpssOsPrintf("%s\n", buf);

    return GT_OK;
}


static GT_U32 prvCpssSrvCpuUniFileRead(void* buf, GT_U32 size, UNIFIED_FILE_STC *uniFile)
{
    GT_U32 k;

    if (uniFile->isFsFile)
        k = cpssOsFread(buf, 1, size, &uniFile->fsFile);
        /* tbd check feof or ferr*/
    else
        k = prvCpssSrvCpuFirmareRead(uniFile->memFile, buf, size);

    return k;
}


static GT_VOID prvCpssSrvCpuUniFileRewind(UNIFIED_FILE_STC *uniFile)
{
    uniFile->isFsFile ?
        cpssOsRewind(uniFile->fsFile.fd) : prvCpssSrvCpuFirmareRewind(uniFile->memFile);
}


static GT_STATUS prvCpssSrvCpuUniFileFsChecksum(UNIFIED_FILE_STC *uniFile)
{
    GT_STATUS rc = GT_OK;
    GT_U32 u32 = 0;
    GT_U32 file_checksum;
    GT_U32 calc_checksum = 0;

    if (uniFile->isFsFile)
    {
        cpssOsFread(&file_checksum, sizeof(file_checksum), 1, &uniFile->fsFile);
        file_checksum = LE_SWAP_32BIT(file_checksum);

        while (cpssOsFread(&u32, 1, sizeof(u32), &uniFile->fsFile)) {
            calc_checksum += LE_SWAP_32BIT(u32);
            u32=0;
        }
        if (calc_checksum != file_checksum)
            rc = GT_FAIL;
    }

    return rc;
}


static GT_VOID prvCpssSrvCpuUniFileClose(UNIFIED_FILE_STC *uniFile)
{
    if (uniFile->isFsFile)
        cpssOsFclose(&uniFile->fsFile);
    else
        prvCpssSrvCpuFirmareClose(uniFile->memFile);
}


static GT_VOID prvCpssSrvCpuFwHeaderSwap(PRV_CPSS_FIRMWARE_ENTRY_STC *hdr)
{
    hdr->headerVersion = LE_SWAP_16BIT(hdr->headerVersion);
    hdr->fw_id = LE_SWAP_32BIT(hdr->fw_id);
    hdr->fw_ver = LE_SWAP_32BIT(hdr->fw_ver);
    hdr->mcv = LE_SWAP_16BIT(hdr->mcv);
    hdr->loadAddr = LE_SWAP_32BIT(hdr->loadAddr);
    hdr->ipcShmAddr = LE_SWAP_32BIT(hdr->ipcShmAddr);
    hdr->ipcShmSize = LE_SWAP_16BIT(hdr->ipcShmSize);
    hdr->metaDataSize = LE_SWAP_16BIT(hdr->metaDataSize);
}


static GT_STATUS prvCpssSrvCpuCpHdrVer1ToSram(void* metaData, PRV_CPSS_FIRMWARE_ENTRY_STC *fw_header, CPSS_HW_DRIVER_STC *drv)
{
#define HOST_MAGIC  0x1A1B1C
#define META_DATA_MAX_SIZE 64

    typedef struct {
        GT_U32      magic_and_hdr_size;
        GT_U16      hdr_ver;
        GT_U16      hdr_chksum;
        GT_U32      fw_id;
        GT_U32      fw_version;
        GT_U16      mcv;
        GT_U8       scpuNumReq;
        GT_U8       noAllocFlag;
        GT_U32      ipcShmAddr;
        GT_U32      ipcShmSize;
    } SRAM_HDR_V1_HEAD_STC;

    SRAM_HDR_V1_HEAD_STC *hdr_v1_head;
    char *sram_header;
    unsigned header_size;
    unsigned count = 0;

    if (fw_header->metaDataSize > META_DATA_MAX_SIZE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    header_size = sizeof(SRAM_HDR_V1_HEAD_STC) + fw_header->metaDataSize;
    sram_header = cpssOsMalloc(header_size);
    if (!sram_header)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    hdr_v1_head = (SRAM_HDR_V1_HEAD_STC*)sram_header;
    hdr_v1_head->magic_and_hdr_size = HOST_MAGIC << 8 | (header_size/4);
    hdr_v1_head->hdr_ver = 1;
    hdr_v1_head->hdr_chksum = 0;
    hdr_v1_head->fw_id = hdr_v1_head->fw_id;
    hdr_v1_head->fw_version = fw_header->fw_ver;
    hdr_v1_head->mcv = fw_header->mcv;
    /*hdr_v1_head->noAllocFlag = 0x
    hdr_v1_head->scpuNumReq = 0xFF;*/ /* Ignored */
    hdr_v1_head->ipcShmAddr = fw_header->ipcShmAddr;
    hdr_v1_head->ipcShmSize = fw_header->ipcShmSize;
    cpssOsMemCpy(sram_header + sizeof(SRAM_HDR_V1_HEAD_STC), metaData, fw_header->metaDataSize);

    hdr_v1_head->hdr_chksum = prvGenChecksum16((GT_U16*)sram_header, header_size/2 );

    (void)count;
    (void)drv;
    /* tbd bobk free sram ?
    while (count < header_size)
    {
        drv->writeMask(drv, 0, 192*_1K - count - sizeof(GT_U32), (GT_U32 *)(sram_header + count), 4, 0xffffffff);
        count += 4;
    }*/

    cpssOsFree(sram_header);

    return GT_OK;
}


static GT_STATUS prvCpssSrvCpuCpHdrToSram(void* metaData, PRV_CPSS_FIRMWARE_ENTRY_STC *fw_header, CPSS_HW_DRIVER_STC *drv)
{
    GT_STATUS rc;

    switch (fw_header->headerVersion)
    {
    case 1:
        rc = prvCpssSrvCpuCpHdrVer1ToSram(metaData, fw_header, drv);
        break;
    default:
        rc = GT_BAD_VALUE;
    }

    return rc;
}

static GT_STATUS prvCpssSrvCpuSerdesFwMetaDataRead
(   
    IN GT_U16                                                metaDataSize,
    IN UNIFIED_FILE_STC                                     *unifiedFile,
    OUT GT_U32                                              *numOfSerdesFirmwarePtr,
    OUT PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_HEADER_STC *serdesFirmwarePtr
)
{
    GT_U32                                                  ii;
    GT_U32                                                  numOfSerdesFirmwares;
    GT_U32                                                  metaDataWord;
    PRV_CPSS_FW_METADATA_TYPE_ENT                           metaDataType;
    GT_U32                                                  numOfBytes;
    GT_U32                                                  numOfRecords;

    CPSS_NULL_PTR_CHECK_MAC(numOfSerdesFirmwarePtr);
    CPSS_NULL_PTR_CHECK_MAC(serdesFirmwarePtr);

    *numOfSerdesFirmwarePtr = numOfSerdesFirmwares = 0;

    CPSS_LOG_INFORMATION_MAC("Meta Data Size[%d] before read", metaDataSize);

    while (metaDataSize)
    {
        /* Read metadata word */
        numOfBytes = prvCpssSrvCpuUniFileRead(&metaDataWord, sizeof(GT_U32), unifiedFile);
        if (numOfBytes == PRV_CPSS_FW_READ_ERR)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Metadata read fail");
        }

        /* Get metadata type */
        metaDataType = LE_SWAP_16BIT(metaDataWord & 0xFFFF);
        /* Get number of records to read */
        numOfRecords = LE_SWAP_16BIT(metaDataWord >> 16);
        if (metaDataType == PRV_CPSS_FW_METADATA_SERDES_FIRMWARE_TYPE_E)
        {
            if (numOfRecords == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Number of FW headers is zero");
            }
            numOfSerdesFirmwares = numOfRecords;
            break;
        }

        /* Skip metadata for current type */
        numOfBytes = prvCpssSrvCpuUniFileRead(&metaDataWord, 2 * numOfRecords * sizeof(GT_U32), unifiedFile);
        if (numOfBytes == PRV_CPSS_FW_READ_ERR)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Metadata read fail");
        }

        metaDataSize -= numOfBytes + 1;

        CPSS_LOG_INFORMATION_MAC("Meta Data Size[%d] in read cycle", metaDataSize);
    }

    CPSS_LOG_INFORMATION_MAC("Meta Data Size[%d] after read cycle", metaDataSize);

    if (numOfSerdesFirmwares > PRV_CPSS_SERDES_FW_MAX_FILES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("Number of retrieved SerDes FW's[%d]", numOfSerdesFirmwares);

    for (ii = 0; ii < numOfSerdesFirmwares; ii++)
    {
        /* Read metadata word */
        numOfBytes = prvCpssSrvCpuUniFileRead(&serdesFirmwarePtr[ii], 2 * sizeof(GT_U32), unifiedFile);
        if (numOfBytes == PRV_CPSS_FW_READ_ERR)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Metadata read fail");
        }

        /* Swap data */
        serdesFirmwarePtr[ii].devNum = LE_SWAP_16BIT(serdesFirmwarePtr[ii].devNum);
        serdesFirmwarePtr[ii].firmwareId = LE_SWAP_16BIT(serdesFirmwarePtr[ii].firmwareId);
        if (serdesFirmwarePtr[ii].firmwareId > PRV_CPSS_SRV_CPU_SERDES_FW_SERDES_LAST_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        serdesFirmwarePtr[ii].firmwareVersion = LE_SWAP_32BIT(serdesFirmwarePtr[ii].firmwareVersion);

        switch (serdesFirmwarePtr[ii].devNum)
        {
            case AC5P_DEV_ID:
                break;
            case AC5X_DEV_ID:
                break;
            case HARRIER_DEV_ID:
                break;
            case IRONMAN_DEV_ID:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("SerDes#[%d], device[%d], FW ID[%d]", ii, serdesFirmwarePtr[ii].devNum, serdesFirmwarePtr[ii].firmwareId);
    }

    *numOfSerdesFirmwarePtr = numOfSerdesFirmwares;

    return GT_OK;
}

/**
* @internal prvCpssSrvCpuSerdesFwSearchAndValidate function
* @endinternal
*
* @brief   Search and validate SerDes firmware in file system
*
* @param[in] metaDataSize               - The metadata size holds the size (In bytes) of the remaining part of the header
* @param[in] unifiedFilePtr             - (pointer to) unified file structure
* @param[inout] serdesFirmwarePtr       - (pointer to) serdes firmware header 
* @param[out] serdesFwDataPtr           - (pointer to) data related to SerDes firmware 
* @param[out] numOfSerdesFirmwaresPtr   - (pointer to) number of serdes firmwares  
*
* @retval GT_OK                         - on success
* @retval GT_BAD_PTR                    - on bad pointer 
* @retval GT_BAD_PARAM                  - wrong devNum or serdes ID
* @retval GT_FAIL                       - otherwise
*
*/
static GT_STATUS prvCpssSrvCpuSerdesFwSearchAndValidate
(   
    IN GT_U16                                                    metaDataSize,
    IN UNIFIED_FILE_STC                                         *unifiedFilePtr,
    INOUT PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_HEADER_STC   *serdesFirmwarePtr,
    OUT PRV_CPSS_SERDES_FIRMWARE_DATA_STC                       *serdesFwDataPtr,
    OUT GT_U32                                                  *numOfSerdesFirmwaresPtr
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  ii;
    GT_CHAR                                                 vendorRevision[20];
    GT_U32                                                  numOfSerdesFirmwares;
    GT_U32                                                  firmwareIdIndex;
    GT_CHAR                                                 serdesFwName[PRV_CPSS_SERDES_FW_NAME_MAX_LEN_CNS];
    GT_CHAR * serdesFwNamePtr = serdesFwName;
    GT_CHAR * serdesFwVersionPtr = vendorRevision;

    static const GT_CHAR * serdesFirmwareNameArray[] = 
    {

        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_MAIN_E,   */ "COMPHYC28GP4X1MAIN",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_CMN_25_E, */ "COMPHYC28GP4X1CMN25",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_156_E,    */ "COMPHYC28GP4X1156",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_LANE_E,   */ "COMPHYC28GP4X1LANE",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_MAIN_E,   */ "COMPHYC28GP4X4MAIN",  
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_CMN_25_E, */ "COMPHYC28GP4X4CMN25",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_156_E,    */ "COMPHYC28GP4X4156",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_LANE_E,   */ "COMPHYC28GP4X4LANE",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C112GX4_MAIN_E,    */ "COMPHYC112GX4MAIN",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_N5XC56GP5X4_MAIN_E,*/ "COMPHYN5XC56GP5X4MAIN",
        /* PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C12GP41P2V_MAIN_E, */ "COMPHYC12GP41P2VMAIN"
    };

    CPSS_NULL_PTR_CHECK_MAC(unifiedFilePtr);
    CPSS_NULL_PTR_CHECK_MAC(serdesFirmwarePtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfSerdesFirmwaresPtr);

    rc = prvCpssSrvCpuSerdesFwMetaDataRead(metaDataSize, unifiedFilePtr, &numOfSerdesFirmwares, serdesFirmwarePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Erorr reading metadata");
    }

    CPSS_LOG_INFORMATION_MAC("Number of SerDes FW's [%d]", numOfSerdesFirmwares);

    for (ii = 0; ii < numOfSerdesFirmwares; ii++)
    {
        switch (serdesFirmwarePtr[ii].firmwareId)
        {
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_MAIN_E:
                firmwareIdIndex = 0;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_CMN_25_E:
                firmwareIdIndex = 1;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_156_E:
                firmwareIdIndex = 2;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_LANE_E:
                firmwareIdIndex = 3;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_MAIN_E:
                firmwareIdIndex = 4;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_CMN_25_E:
                firmwareIdIndex = 5;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_156_E:
                firmwareIdIndex = 6;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_LANE_E:
                firmwareIdIndex = 7;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C112GX4_MAIN_E:
                firmwareIdIndex = 8;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_N5XC56GP5X4_MAIN_E:
                firmwareIdIndex = 9;
                break;
            case PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C12GP41P2V_MAIN_E:
                firmwareIdIndex = 10;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        cpssOsSprintf(serdesFwNamePtr, "%s", serdesFirmwareNameArray[firmwareIdIndex]);

        cpssOsSprintf(serdesFwVersionPtr, ".%d.%d.%d.%d",
                      ((serdesFirmwarePtr[ii].firmwareVersion >> 24) & 0xFF),
                      ((serdesFirmwarePtr[ii].firmwareVersion >> 16) & 0xFF),
                      ((serdesFirmwarePtr[ii].firmwareVersion >> 8 ) & 0xFF),
                      ((serdesFirmwarePtr[ii].firmwareVersion      ) & 0xFF));

        cpssOsStrCat(serdesFwNamePtr, serdesFwVersionPtr);

        if (GT_NOT_FOUND == prvCpssSerdesFwSearch(serdesFirmwarePtr[ii].devNum, serdesFwNamePtr, serdesFwVersionPtr, 
                                                  &serdesFwDataPtr[ii]))
        {
            CPSS_LOG_INFORMATION_MAC("SerDes FW[%s] for device[%d] was not found", serdesFwNamePtr, serdesFirmwarePtr[ii].devNum);
        }
    }

    *numOfSerdesFirmwaresPtr = numOfSerdesFirmwares;

    return GT_OK;
}

#endif /* ASIC_SIMULATION */


/**
* @internal prvCpssGenericSrvCpuInitCust function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin;
*         Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - Device Number
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xfe - MSYS
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*         GT_NO_RESOURCE
*         GT_NO_SUCH
*/
GT_STATUS prvCpssGenericSrvCpuInitCust
(
    IN  GT_U8       devNum,
    IN  GT_U8       scpuId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#define VER_NO_PRV_PART_MAC(x)  (GT_U16)(x>>16)
#define MAJ_MINOR_VER_MAC(x)    (x##_major<<8 | x##_minor)
#define NO_VER_VALIDATION       0xFFFF

#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    PRV_SRVCPU_IPC_CHANNEL_STC *s = NULL;
    GT_U32    p;
    GT_STATUS rc;
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32 buf[128];
    GT_U32 k, i;
    UNIFIED_FILE_STC unifiedFile;
    FS_HEADER_STC fs_header;
    PRV_CPSS_FIRMWARE_ENTRY_STC *fw_header;
    const FW_DB_ENTRY_STC *db_entry;
    GT_U32 l1_firmware = GT_FALSE;
    GT_U32 l1_dummy_firmware = GT_FALSE;
    PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_HEADER_STC serdesFirmware[PRV_CPSS_SERDES_FW_MAX_FILES_CNS];
    PRV_CPSS_SERDES_FIRMWARE_DATA_STC serdesFwData[PRV_CPSS_SERDES_FW_MAX_FILES_CNS];
    GT_U32 numOfSerdesFirmwares;

    hwInfo = &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (drv == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    opsFunc = prvSrvCpuGetOps(devNum);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    db_entry = prvCpssSrvCpuGetOptionalFW(devNum, scpuId);

    if (!db_entry)
    {
        for (i=0; i<ARRAY_SIZE(DEFAULT_FW_DB); i++)
        {
            if ( (DEFAULT_FW_DB[i].devFamily == DEV_FMLY_SUB_FMLY(devNum)) &&
                 ( (DEFAULT_FW_DB[i].srvCpuId == scpuId) || (DEFAULT_FW_DB[i].srvCpuId == SRVCPU_ID_ALL) )
               )
            {
                db_entry = &DEFAULT_FW_DB[i];
                break;
            }
        }
    }
    else if (db_entry->readOnly)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_SET_ERROR, "dev %d srvCpuId %d is forbidden for private FW", db_entry->devNum, db_entry->srvCpuId);
    }

    if (!db_entry)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    cpssOsMemSet(&unifiedFile, 0, sizeof(unifiedFile));

    if (GT_NOT_FOUND == prvCpssSrvCpuFwSearchPath(db_entry->name, db_entry->fw_id, &unifiedFile))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "FW %s was not found", db_entry->name);
    }

    /* Analyze firmware and indicate if it's an L1 or L1 dummy type */
    if ((db_entry->fw_id >> 16) == DUMMY_FW)
    {
        /* L1 FW Dummy */
        l1_dummy_firmware = GT_TRUE;
        l1_firmware = GT_TRUE;
        CPSS_LOG_INFORMATION_MAC("FW ID[%d] is L1 dummy firmware", db_entry->fw_id);
    }

    if ((db_entry->fw_id >> 16) == L1_FW)
    {
        /* L1 FW */
        l1_firmware = GT_TRUE;
        CPSS_LOG_INFORMATION_MAC("FW ID[%d] is L1 firmware", db_entry->fw_id);
    }

    if (unifiedFile.isFsFile)
    {
        if (GT_FAIL==prvCpssSrvCpuUniFileFsChecksum(&unifiedFile))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Checksum failed for %s", db_entry->name);

        prvCpssSrvCpuUniFileRewind(&unifiedFile);

        k = prvCpssSrvCpuUniFileRead(&fs_header, sizeof(fs_header), &unifiedFile);
        if (k==PRV_CPSS_FW_READ_ERR)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Failed reading %s", db_entry->name);
        }
        fw_header = &fs_header.fw_entry;
        prvCpssSrvCpuFwHeaderSwap(fw_header);

        CPSS_LOG_INFORMATION_MAC("FW ID[%d] is not dummy", fw_header->fw_id);
        if (db_entry->fw_id != fw_header->fw_id)
        {
            prvCpssSrvCpuUniFileClose(&unifiedFile);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "%s incompatible FW ID: CPSS: 0x%x FW 0x%x",
                db_entry->name, db_entry->fw_id, fw_header->fw_id);
        }

        cpssOsPrintf("CPSS module ver %u.%u, Minimal Compatible Version %u.%u\n",
                     db_entry->module_ver_major, db_entry->module_ver_minor, db_entry->mcv_major, db_entry->mcv_minor);
        cpssOsPrintf("FW ver %u.%u, Minimal Compatible Version %u.%u\n",
                     fw_header->fw_ver >> 24, (fw_header->fw_ver >> 16) & 0xFF , fw_header->mcv >> 8, fw_header->mcv & 0xFF);

        if ( (MAJ_MINOR_VER_MAC(db_entry->mcv) != NO_VER_VALIDATION) ) /* Does FW requires version validation ? */
        {
             if ( (MAJ_MINOR_VER_MAC(db_entry->mcv) > VER_NO_PRV_PART_MAC(fw_header->fw_ver)) )
             {
                 prvCpssSrvCpuUniFileClose(&unifiedFile);
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                     "%s: CPSS module's Minimal Compatible Version(MCV) %d.%d > FW version %d.%d. Must be smaller",
                     db_entry->name, db_entry->mcv_major, db_entry->mcv_minor,
                     VER_NO_PRV_PART_MAC(fw_header->fw_ver)>>8, VER_NO_PRV_PART_MAC(fw_header->fw_ver) & 0xFF);
             }

             if ( (fw_header->mcv > MAJ_MINOR_VER_MAC(db_entry->module_ver)) )
             {
                 prvCpssSrvCpuUniFileClose(&unifiedFile);
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                     "%s: FW's Minimal Compatible Version(MCV) %d.%d > CPSS module version %d.%d. Must be smaller",
                     db_entry->name, (fw_header->mcv)>>8, (fw_header->mcv) & 0xFF,
                     db_entry->module_ver_major, db_entry->module_ver_minor);
             }
        }
    }
    else /* mem file*/
        fw_header = unifiedFile.memFile;


    /* Check metadata for SerDes FW */
    if (l1_firmware == GT_TRUE)
    {
        rc = prvCpssSrvCpuSerdesFwSearchAndValidate(fw_header->metaDataSize, &unifiedFile, serdesFirmware, serdesFwData, &numOfSerdesFirmwares);
        if (rc != GT_OK)
        {
            prvCpssSrvCpuUniFileClose(&unifiedFile);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "%s erorr in SerDes FW header validation:  FW 0x%x",
                db_entry->name, fw_header->fw_id);
        }
    }


    if (l1_dummy_firmware == GT_FALSE)
    {
        rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_RESET_E, NULL);
        if (rc != GT_OK)
        {
            prvCpssSrvCpuUniFileClose(&unifiedFile);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_LOAD_E, &s);
        if ( (rc != GT_OK) || !s )
        {
            prvCpssSrvCpuUniFileClose(&unifiedFile);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        s->shmAddr = fw_header->ipcShmAddr;
        s->shmSize = fw_header->ipcShmSize;

        if (unifiedFile.isFsFile)
        {
                if (sizeof(buf) < fw_header->metaDataSize)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                k = prvCpssSrvCpuUniFileRead(buf, fw_header->metaDataSize, &unifiedFile);
                if (!k || (k==PRV_CPSS_FW_READ_ERR) )
                    /* tbd check err and prvIpcAccessDestroy(s)*/
                {
                    prvCpssSrvCpuUniFileClose(&unifiedFile);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssSrvCpuCpHdrToSram(buf, fw_header, s->drv);
        }
        else
            rc = prvCpssSrvCpuCpHdrToSram( ((PRV_CPSS_FIRMWARE_MEM_FILE_DB_ENTRY_STC*)fw_header)->metaData, fw_header, s->drv);

        if (GT_OK != rc)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        p = 0;
        k = 0;
        /* Copy the FW itself to SRAM */
        while (1)
        {
            k = prvCpssSrvCpuUniFileRead(buf, sizeof(buf), &unifiedFile);
            if (!k || (k==PRV_CPSS_FW_READ_ERR) )
                /* tbd check err and prvIpcAccessDestroy(s)*/
                break;
            s->drv->writeMask(s->drv, 0, fw_header->loadAddr + p, buf, k/4, 0xffffffff);
            p += k;
        }
        if (k == PRV_CPSS_FW_READ_ERR)
        {
            /* failure reading firmware */
            prvIpcAccessDestroy(s);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    if (l1_firmware == GT_TRUE)
    {
        /** Call API to pass data structure for L1 firmware */
    }

    prvCpssSrvCpuUniFileClose(&unifiedFile);

    if (l1_dummy_firmware == GT_FALSE)
    {
        prvCpssGenericSrvCpuIpcInit(ipcConfig, s, preStartCb, preStartCookie, fwChannelPtr);

        rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_UNRESET_E, NULL);
    }

    return rc;

#endif
}

/**
* @internal prvCpssGenericSrvCpuInit function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuInit
(
    IN  GT_U8       devNum,
    IN  GT_U8       scpuId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#ifndef ASIC_SIMULATION
    GT_U32 index;
    GT_STATUS rc;

    index = SRVCPU_IDX_GET_MAC(scpuId);
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (index >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum) == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssGenericSrvCpuInitCust(devNum, scpuId, ipcConfig, preStartCb, preStartCookie, fwChannelPtr);

    /* store fwChannel */
    if (fwChannelPtr != NULL && (index < SRVCPU_MAX_IPC_CHANNEL))
    {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[index] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
    }

    return rc;
#else
    (void)devNum;
    (void)scpuId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    return GT_OK;
#endif 
}

/**
* @internal prvCpssGenericSrvCpuProbeCust function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - Device Number
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
*/
GT_STATUS prvCpssGenericSrvCpuProbeCust
(
    IN  GT_U8      devNum,
    IN  GT_U32      scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    GT_BOOL isOn;
    GT_STATUS rc;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    CPSS_SYSTEM_RECOVERY_INFO_STC currentSystemRecoveryInfo;
    CPSS_SYSTEM_RECOVERY_INFO_STC newSystemRecoveryInfo;

    if (fwChannelPtr != NULL)
        *fwChannelPtr = 0;

    hwInfo = &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    opsFunc = prvSrvCpuGetOps(devNum);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_IS_ON_E, &isOn);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    if (isOn != GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_CHECK_IPC_E, &s);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (hasIPC == GT_FALSE)
    {
        /* No IPC check, return OK */
        if (fwChannelPtr != NULL)
        {
            *fwChannelPtr = (GT_UINTPTR)s;
        }
        else
        {
            prvIpcAccessDestroy(s);
        }
        return GT_OK;
    }
    s->shmSize = 0x800; /* handled later */
    prvIpcAccessInit(s, GT_FALSE);
    /* Check magick in reserved area */
    if (shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 0) != IPC_RESVD_MAGIC)
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shmAddr = shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 1) - 0x100;
    if (s->shmAddr < s->targetBase || s->shmAddr >= s->targetBase+(s->size-4))
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shm.shm = s->shmAddr;
    s->shm.shmLen = s->shmSize;
    /* check if ipc link was initialized */
    if ( (shmIpcMagicGet(&(s->shm)) != IPC_SHM_MAGIC)  &&
         (shmIpcMagicGet(&(s->shm)) != IPC_SHM_MAGIC+1) )
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    /* fix mirror buffer */
    s->shm.shmLen = shmIpcSizeGet(&(s->shm));
    if (s->shmSize < s->shm.shmLen && s->base == 0)
    {
        s->shmSize = s->shm.shmLen;
    }

    /* check ipc transaction */
    if (checkIpcCb != NULL)
    {
        rc = cpssSystemRecoveryStateGet(&currentSystemRecoveryInfo);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        newSystemRecoveryInfo = currentSystemRecoveryInfo;
        newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        if (checkIpcCb((GT_UINTPTR)s, checkIpcCookie) != GT_TRUE)
        {
            prvIpcAccessDestroy(s);
            prvCpssSystemRecoveryStateUpdate(&currentSystemRecoveryInfo);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
        prvCpssSystemRecoveryStateUpdate(&currentSystemRecoveryInfo);
    }
    if (fwChannelPtr != NULL)
    {
        *fwChannelPtr = (GT_UINTPTR)s;
    }
    else
    {
        prvIpcAccessDestroy(s);
    }

    return GT_OK;
#endif
}


/**
* @internal prvCpssGenericSrvCpuProbe function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuProbe
(
    IN  GT_U8       devNum,
    IN  GT_U8       scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{

#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    GT_STATUS rc;
    GT_U32 index;

    index = SRVCPU_IDX_GET_MAC(scpuId);
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (index >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum) == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssGenericSrvCpuProbeCust(devNum, scpuId,
            hasIPC, checkIpcCb, checkIpcCookie, fwChannelPtr);

    /* store fwChannel */
    if (rc == GT_OK)
    {
        if (fwChannelPtr != NULL  && (index < SRVCPU_MAX_IPC_CHANNEL))
        {
             PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[index] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
        }
    }
    return rc;

#endif
}


/**
* @internal prvCpssDbaToSrvCpuLoad function
* @endinternal
*
* @brief   Upload DBA FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDbaToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    if (GT_OK != cpssSrvCpuSetOptionalFW(devNum, 1, DBA_BC3_FW_ID, "bc3DbaFw") )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    return prvCpssGenericSrvCpuInit(devNum, 1, NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/**
* @internal prvCpssTamToSrvCpuLoad function
* @endinternal
*
* @brief   Upload TAM FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - device is not supported
*/
GT_STATUS prvCpssTamToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    GT_U8 scpuId;
    GT_CHAR *fwName;
    GT_U32 fwId;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        scpuId = 16;
        fwName = "falconTamFw";
        fwId = TAM_FALCON_FW_ID;
        break;

    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        fwName = "aldrin2TamFw";
        scpuId =1;
        fwId = TAM_ALDRIN2_FW_ID;
        break;

    case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        fwName = "aldrin2TamFw";
        scpuId =1;
        fwId = TAM_BC3_FW_ID;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (GT_OK != cpssSrvCpuSetOptionalFW(devNum, scpuId, fwId, fwName) )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    return prvCpssGenericSrvCpuInit(devNum, scpuId , NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/**
* @internal prvCpssFlowManagerToSrvCpuLoad function
* @endinternal
*
* @brief   Upload Flow Manager FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Aldrin2;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
* @param[in] sCpuId                - service CPU ID.
* @param[out] fwChannel            - IPC channel.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssFlowManagerToSrvCpuLoad
(
    IN  GT_U8      devNum,
    IN  GT_U32     sCpuId,
    OUT GT_UINTPTR *fwChannel
)
{
#ifndef ASIC_SIMULATION
    PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 1,   2, 200, 2, 200 },
            { 0xff,0,  0, 0,  0 }
    };
    if (GT_OK != cpssSrvCpuSetOptionalFW(devNum, sCpuId, FLOW_MNG_ALDRIN2_FW_ID, "aldrin2FlowMngFw") )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    return prvCpssGenericSrvCpuInit(devNum, sCpuId, ipcCfg, NULL, 0, fwChannel);
#else
    (void)devNum;
    (void)sCpuId;
    (void)fwChannel;
    return GT_OK;
#endif
}

/**
* @internal prvCpssIpfixManagerToSrvCpuLoad function
* @endinternal
*
* @brief   Upload Flow Manager FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Falcon;
*
* @param[in] devNum                - device number.
* @param[in] sCpuId                - service CPU ID.
* @param[out] fwChannel            - IPC channel.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssIpfixManagerToSrvCpuLoad
(
    IN  GT_U8      devNum,
    IN  GT_U32     sCpuId,
    OUT GT_UINTPTR *fwChannel
)
{
#ifndef ASIC_SIMULATION
    PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 1,    32, 44,256, 44 },
            { 0xff,  0,  0,  0,  0 }
    };
    /* Currently UART console is not working for other Falcon Eagle CM3
     * instances #17-19 so for now going ahead with only CM3#16 TBD.
     */
    if( sCpuId > 19 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    if (GT_OK != cpssSrvCpuSetOptionalFW(devNum, sCpuId, FLOW_MNG_FALCON_FW_ID, "falconFlowMngFw") )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    return prvCpssGenericSrvCpuInit(devNum, sCpuId, ipcCfg, NULL, 0, fwChannel);
#else
    (void)devNum;
    (void)sCpuId;
    (void)fwChannel;
    return GT_OK;
#endif
}

/**
* @internal prvCpssIpfixFwToSrvCpuLoad function
* @endinternal
*
* @brief   Upload IPFIX FW to service CPU
*
* @note   APPLICABLE DEVICES:      AC5X, AC5P;
*
* @param[in] devNum                - device number.
* @param[in] sCpuId                - service CPU ID.
* @param[out] fwChannel            - IPC channel.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssIpfixFwToSrvCpuLoad
(
    IN  GT_U8      devNum,
    IN  GT_U32     sCpuId,
    OUT GT_UINTPTR *fwChannel
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
                { 1,    64, 56, 16, 28 },
                { 0xff,  0,  0,  0,  0 }
        };
        rc = cpssSrvCpuSetOptionalFW(devNum, sCpuId, IPFIX_AC5X_FW_ID, "ac5xIpfixFw");
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        return prvCpssGenericSrvCpuInit(devNum, sCpuId, ipcCfg, NULL, 0, fwChannel);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
                { 1,   256, 56, 16, 28 },
                { 0xff,  0,  0,  0,  0 }
        };
        rc = cpssSrvCpuSetOptionalFW(devNum, sCpuId, IPFIX_AC5P_FW_ID, "ac5pIpfixFw");
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        return prvCpssGenericSrvCpuInit(devNum, sCpuId, ipcCfg, NULL, 0, fwChannel);
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
                { 1,    32, 28, 256, 28 },
                { 0xff,  0,  0,   0,  0 }
        };
        /* Currently UART console is not working for other Falcon Eagle CM3
         * instances #17-19 and is working only for instance #16.
         */
        if( sCpuId > 19 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
        }
        rc = cpssSrvCpuSetOptionalFW(devNum, sCpuId, IPFIX_FALCON_FW_ID, "falconIpfixFw");
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        return prvCpssGenericSrvCpuInit(devNum, sCpuId, ipcCfg, NULL, 0, fwChannel);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;

#else
    (void)devNum;
    (void)sCpuId;
    (void)fwChannel;
    return GT_OK;
#endif
}

/*******************************************************************************
* prvCpssGenericSrvCpuRemove
*
* DESCRIPTION:
*       Cleanup service CPU resources while cpssDxChCfgDevRemove
*
* APPLICABLE DEVICES:
*       AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Pipe.
*
* NOT APPLICABLE DEVICES:
*       Lion2.
*
* INPUTS:
*       devNum                   - device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssGenericSrvCpuRemove
(
    IN GT_U8    devNum,
    IN GT_U8    scpuId
)
{
#ifndef ASIC_SIMULATION
    GT_U8 cpuIdx;
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) ||
        (scpuId >= SRVCPU_MAX_IPC_CHANNEL && scpuId != SRVCPU_IPC_CHANNEL_ALL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (scpuId == SRVCPU_IPC_CHANNEL_ALL)
    {
        for (cpuIdx = 0; cpuIdx < SRVCPU_MAX_IPC_CHANNEL; cpuIdx++) {
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)!=NULL&&PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[cpuIdx] != NULL)
            {
                prvIpcAccessDestroy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[cpuIdx]);
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[cpuIdx] = NULL;
            }
        }
    }
    else if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)!=NULL&&PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[scpuId] != NULL)
    {
        prvIpcAccessDestroy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[SRVCPU_IDX_GET_MAC(scpuId)]);
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[SRVCPU_IDX_GET_MAC(scpuId)] = NULL;

    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
    (void)scpuId;
#endif
    return GT_OK;
}

/**
* @internal internal_cpssGenericSrvCpuResetUnreset function
* @endinternal
*
* @brief   API to reset \ unreset  service CPU.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                - device number
* @param[in] sCpuId                - service CPU ID.
* @param[in] resetOp               - reset - 0 , unreset - 1
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS internal_cpssGenericSrvCpuResetUnreset
(
    IN  GT_U8   devNum,
    IN  GT_U8   scpuId,
    IN  GT_U8   resetOp
)
{
    PRV_SRVCPU_OPS_FUNC opsFunc;
    GT_STATUS rc;
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    hwInfo = &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    opsFunc = prvSrvCpuGetOps(devNum);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (!resetOp) /* resetOp == 0, execute reset command */
        rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_RESET_E, NULL);
    else /* resetOp == 1, unreset SrvCpu */
        rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_UNRESET_E, NULL);

    return rc;
}

/**
* @internal internal_cpssGenericSrvCpuFWUpgrade function
* @endinternal
*
* @brief   Send buffer with FW upgrade to service CPU,
*         to be programmed on flash, using boot channel.
*         buffer size is limited to 240 bytes.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number,
* @param[in] bufBmp                   - bitmap of the FW:
*                                      0x01 - primary SuperImage
*                                      0x02 - secondary SuperImage,
*                                      0x03 - both.
* @param[in] bufNum                   - block number,
* @param[in] bufSize                  - buffer size:
*                                      must be: 32B < fwBufSize < 240B,
*                                      last bufSize=bufSize|0x100)
* @param[in] bufPtr                   - FW data ptr,
*
* @param[out] status                   - microInit boot channel retCode.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS internal_cpssGenericSrvCpuFWUpgrade
(
    IN  GT_U8   devNum,
    IN  GT_U32  bufBmp,
    IN  GT_U32  bufNum,
    IN  GT_U32  bufSize,
    IN  GT_U8  *bufPtr,
    IN  GT_U8   scpuId,
    OUT GT_U32 *status
)
{

#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC      *s;
    GT_32                           rc = 0, bc_rc = 0;
    GT_U32                          i, j, msgSize;
    GT_U32                          maxDelay;
    GT_U32                          bufWithHeader[256];
    GT_U32                          dummyBuf;
    upgradeMsgHdr                   *msgHeader = NULL;

    cpssOsPrintf("bufNum=%d bufSize=0x%X\n",bufNum, bufSize );

    msgSize = bufSize & 0xFF;
    if( (msgSize<REMOTE_UPGRADE_MSG_MIN_LEN) || (msgSize>REMOTE_UPGRADE_MSG_MAX_LEN) ) {
        cpssOsPrintf("buffer size must be %d < X < %d \n", REMOTE_UPGRADE_MSG_MIN_LEN, REMOTE_UPGRADE_MSG_MAX_LEN);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (scpuId >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    s = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[scpuId];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    /* prepare command parameters */
    msgHeader = (upgradeMsgHdr *)&bufWithHeader[0];
    msgHeader->opCode  = 0x10;          /* remote upgrade opCode */
    msgHeader->bmp     = bufBmp & 0xFF;
    msgHeader->bufNum  = bufNum;
    msgHeader->bufSize = (GT_U16)bufSize;
    msgHeader->checkSum = 0;
    msgHeader->reseved  = 0;
    cpssOsMemCpy( (msgHeader + 1), bufPtr, msgSize );

    msgSize = msgSize + REMOTE_UPGRADE_HEADER_SIZE;
    /* calculate checksum and update packet header */
    msgHeader->checkSum = prvChecksum8((GT_U8*)msgHeader,msgSize,0);

    i = 0;
    /* wait reply status about 2 sec for regular packets
         and 30 sec for finalize procedure of bmp #3 (crc validation and copy image to other offset) */
    if ((bufSize | 0x100) && (bufBmp ==3)) maxDelay = 30000;
    else maxDelay = 2000;

    for ( j=0 ; j < 3; j++ ) { /* 3 retries for packet with wrong checkSum */
        if (GT_OK != (rc = shmIpcBootChnSend(&(s->shm), 0, msgHeader, msgSize) ) ){
            if (j >= 3){
                cpssOsPrintf("shmIpcBootChnSend failed %d\n", rc);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            continue;
        }

        do
        {
            cpssOsTimerWkAfter(1);

            i++;
            /* for each boot channel send operation, feedback receive is necessary.
               if not expected any valuable feedback data - dummy buffer must be used */
            rc=shmIpcBootChnRecv(&s->shm, &bc_rc, &dummyBuf, 4);
            if (rc > 0)
                break;
            if (i > maxDelay)
            {
                cpssOsPrintf("\n\nTimeOut error\n\n");
                *status = bc_rc;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        } while(i <= maxDelay);

        /* only in case of invalid checksum - send message again */
        if( bc_rc != MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E )
            break;
    }
    *status = bc_rc;

    return (rc>0) ? bc_rc : GT_FAIL;


#else /* defined(ASIC_SIMULATION) */

    if (bufPtr) {
        devNum  = devNum;
        bufBmp  = bufBmp;
        bufNum  = bufNum;
        bufSize = bufSize;
        scpuId  = scpuId;
    };
    *status = 4;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#endif /* defined(ASIC_SIMULATION) */
}

/**
* @internal cpssGenericSrvCpuResetUnreset function
* @endinternal
*
* @brief   API to reset \ unreset  service CPU,
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                - device number
* @param[in] sCpuId                - service CPU ID.
* @param[in] resetOp               - reset - 0 , unreset - 1
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS cpssGenericSrvCpuResetUnreset
(
    IN  GT_U8   devNum,
    IN  GT_U8   scpuId,
    IN  GT_U8   resetOp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericSrvCpuResetUnreset);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, scpuId, resetOp));

    rc = internal_cpssGenericSrvCpuResetUnreset(devNum, scpuId, resetOp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, scpuId, resetOp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssGenericSrvCpuFWUpgrade function
* @endinternal
*
* @brief   Send buffer with FW upgrade to service CPU,
*         to be programmed on flash, using boot channel.
*         buffer size is limited to 240 bytes.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number,
* @param[in] bufBmp                   - bitmap of the FW:
*                                      0x01 - primary SuperImage
*                                      0x02 - secondary SuperImage,
*                                      0x03 - both.
* @param[in] bufNum                   - block number,
* @param[in] bufSize                  - buffer size:
*                                      must be: 32B < fwBufSize < 240B,
*                                      last bufSize=bufSize|0x100)
* @param[in] bufPtr                   - FW data ptr,
*
* @param[out] status                   - microInit boot channel retCode.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssGenericSrvCpuFWUpgrade
(
    IN  GT_U8   devNum,
    IN  GT_U32  bufBmp,
    IN  GT_U32  bufNum,
    IN  GT_U32  bufSize,
    IN  GT_U8  *bufPtr,
    IN  GT_U8   scpuId,
    OUT GT_U32 *status
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericSrvCpuFWUpgrade);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bufBmp, bufNum, bufSize, bufPtr, scpuId, status));

    rc = internal_cpssGenericSrvCpuFWUpgrade(devNum, bufBmp, bufNum, bufSize, bufPtr, scpuId, status);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bufBmp, bufNum, bufSize, bufPtr, scpuId, status));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

