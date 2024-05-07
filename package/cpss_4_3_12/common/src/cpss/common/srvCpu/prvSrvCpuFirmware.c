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
* @file prvSrvCpuFirmware.c
*
* @brief Firmware management APIs
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/srvCpu/prvSrvCpuFirmware.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef CONFIG_LINKED_FW_1
  #include CONFIG_LINKED_FW_1
#endif
#ifdef CONFIG_LINKED_FW_2
  #include CONFIG_LINKED_FW_2
#endif
#ifdef CONFIG_LINKED_FW_3
  #include CONFIG_LINKED_FW_3
#endif
#ifdef CONFIG_LINKED_FW_4
  #include CONFIG_LINKED_FW_4
#endif

/* SerDes linked FW's */
#ifdef CONFIG_SRDS_LINKED_FW_1
  #include CONFIG_SRDS_LINKED_FW_1
#endif
#ifdef CONFIG_SRDS_LINKED_FW_2
  #include CONFIG_SRDS_LINKED_FW_2
#endif
#ifdef CONFIG_SRDS_LINKED_FW_3
  #include CONFIG_SRDS_LINKED_FW_3
#endif
#ifdef CONFIG_SRDS_LINKED_FW_4
  #include CONFIG_SRDS_LINKED_FW_4
#endif
#ifdef CONFIG_SRDS_LINKED_FW_5
  #include CONFIG_SRDS_LINKED_FW_5
#endif
#ifdef CONFIG_SRDS_LINKED_FW_6
  #include CONFIG_SRDS_LINKED_FW_6
#endif
#ifdef CONFIG_SRDS_LINKED_FW_7
  #include CONFIG_SRDS_LINKED_FW_7
#endif
#ifdef CONFIG_SRDS_LINKED_FW_8
  #include CONFIG_SRDS_LINKED_FW_8
#endif
#ifdef CONFIG_SRDS_LINKED_FW_9
  #include CONFIG_SRDS_LINKED_FW_9
#endif
#ifdef CONFIG_SRDS_LINKED_FW_10
  #include CONFIG_SRDS_LINKED_FW_10
#endif
#ifdef CONFIG_SRDS_LINKED_FW_11
  #include CONFIG_SRDS_LINKED_FW_11
#endif


static PRV_CPSS_FIRMWARE_MEM_FILE_DB_ENTRY_STC firmwares[] = {

    #ifdef CONFIG_LINKED_FW_1
        {CONFIG_LINKED_FW_1_INFO},
    #endif
    #ifdef CONFIG_LINKED_FW_2
        {CONFIG_LINKED_FW_2_INFO},
    #endif
    #ifdef CONFIG_LINKED_FW_3
        {CONFIG_LINKED_FW_3_INFO},
    #endif
    #ifdef CONFIG_LINKED_FW_4
        {CONFIG_LINKED_FW_4_INFO},
    #endif

    /* end */
    { {0,0,0,0,0,0,0,0}, NULL, 0, 0, NULL }
};

typedef struct {
    PRV_CPSS_FIRMWARE_MEM_FILE_DB_ENTRY_STC    mem_db_entry;
    GT_U32      pos;
    GT_U32      state;
    GT_U32      cnt;
} PRV_MEM_FILE_STC;

/** Array of linked SerDes firmwares */
static PRV_CPSS_SERDES_FIRMWARE_MEM_FILE_DB_ENTRY_STC serdesFirmwaresMemDb[] = {

    /* SerDes linked FW's */
    #ifdef CONFIG_SRDS_LINKED_FW_1
      {CONFIG_SRDS_LINKED_FW_1_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_2
      {CONFIG_SRDS_LINKED_FW_2_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_3
      {CONFIG_SRDS_LINKED_FW_3_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_4
      {CONFIG_SRDS_LINKED_FW_4_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_5
      {CONFIG_SRDS_LINKED_FW_5_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_6
      {CONFIG_SRDS_LINKED_FW_6_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_7
      {CONFIG_SRDS_LINKED_FW_7_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_8
      {CONFIG_SRDS_LINKED_FW_8_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_9
      {CONFIG_SRDS_LINKED_FW_9_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_10
      {CONFIG_SRDS_LINKED_FW_10_INFO},
    #endif
    #ifdef CONFIG_SRDS_LINKED_FW_11
      {CONFIG_SRDS_LINKED_FW_11_INFO},
    #endif
      /* end */
      { NULL, NULL, 0 }
};

/** Number of entries in SerDes firmware memories database */
GT_U32 serdesFirmwaresMemDbSize = sizeof(serdesFirmwaresMemDb) / sizeof(PRV_CPSS_SERDES_FIRMWARE_MEM_FILE_DB_ENTRY_STC);

/**
* @internal prvCpssSerdesFirmareMemGet function
* @endinternal
*
* @brief   Function retrieves firmware memory data from database.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe
*
* @param[in] fwName                 - SerDes firmware name
* @param[out] serdesFwDataPtr       - (pointer to) data related to SerDes firmware
*
* @retval GT_OK                     - on success 
* @retval GT_NOT_FOUND              - on not found 
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_BAD_STATE              - on database error
* @retval GT_NO_RESOURCE            - on system error
*/
GT_STATUS prvCpssSerdesFirmareMemGet
(
    IN  GT_CHAR    *fwName,
    OUT PRV_CPSS_SERDES_FIRMWARE_DATA_STC  *serdesFwDataPtr
)
{
    GT_U32 ii;
    GT_U8 * fwMemPtr = NULL;

    CPSS_NULL_PTR_CHECK_MAC(fwName);
    CPSS_NULL_PTR_CHECK_MAC(serdesFwDataPtr);

    /* Lookup for the SerDes entry by the name in database */
    for (ii = 0; ii < serdesFirmwaresMemDbSize; ii++)
    {
        if (0 == cpssOsStrCmp(serdesFirmwaresMemDb[ii].fwName, fwName))
        {
            /* Check that the firmware size is not zero */
            if (serdesFirmwaresMemDb[ii].fwSize == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            /* The entry is found */
            break;
        }
    }

    /* The cycle has reached the end */
    if (ii == serdesFirmwaresMemDbSize)
    {
        /* The entry for the SerDes firmware not found */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Allocate buffer by the size of the firmware memory size */
    fwMemPtr = (GT_U8 *)cpssOsMalloc(serdesFirmwaresMemDb[ii].fwSize);
    if (NULL == fwMemPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Copy content of the firmware memory to the output buffer */
    cpssOsMemCpy(fwMemPtr, serdesFirmwaresMemDb[ii].fwDataPtr, serdesFirmwaresMemDb[ii].fwSize);

    /* Assign the memory buffer and its size to the output structure */
    serdesFwDataPtr->fwMemType = FW_LINKED_INFO;
    serdesFwDataPtr->fwData.fwLinkedInfo.fwMemPtr = fwMemPtr;
    serdesFwDataPtr->fwData.fwLinkedInfo.fwMemSize = serdesFirmwaresMemDb[ii].fwSize;

    return GT_OK;
}

PRV_CPSS_FIRMWARE_ENTRY_STC* prvCpssSrvCpuFirmareOpen(
    IN  const GT_U32 fw_id
)
{
    GT_U32 i;
    PRV_MEM_FILE_STC  *f;

    for (i = 0; firmwares[i].fwSize; i++)
    {
        if (firmwares[i].fw_entry.fw_id == fw_id)
            break;
    }
    if (!firmwares[i].fwSize)
        return NULL;

    f = (PRV_MEM_FILE_STC *)cpssOsMalloc(sizeof(*f));
    if (!f)
        return NULL;
    cpssOsMemCpy(&f->mem_db_entry, &firmwares[i], sizeof(PRV_CPSS_FIRMWARE_MEM_FILE_DB_ENTRY_STC));
    f->pos               = 0;
    f->state             = 0;
    f->cnt               = 0;

    return &(f->mem_db_entry.fw_entry);
}


GT_STATUS prvCpssSrvCpuFirmareClose(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC *f
)
{
    if (f)
        cpssOsFree(f);
    return GT_OK;
}


GT_VOID prvCpssSrvCpuFirmareRewind(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC *f
)
{
    ((PRV_MEM_FILE_STC *)f)->pos = 0;
}

static GT_U32 prvCpssSrvCpuFirmareReadRaw(
    IN  PRV_MEM_FILE_STC            *f,
    OUT GT_U8                       *bufPtr,
    IN  GT_U32                      bufLen
)
{
    GT_U32 c;
    if (f->pos >= f->mem_db_entry.fwSize)
        return 0;
    for (c = 0; c < bufLen && f->pos < f->mem_db_entry.fwSize; c++, f->pos++)
        bufPtr[c] = f->mem_db_entry.fwData[f->pos];
    return c;
}

static GT_U32 prvCpssSrvCpuFirmareReadSimpleCompression(
    IN  PRV_MEM_FILE_STC            *f,
    OUT GT_U8                       *bufPtr,
    IN  GT_U32                       bufLen
)
{
    GT_U32 c;
    if (f->pos >= f->mem_db_entry.fwSize)
        return 0;
    for (c = 0; c < bufLen; )
    {
        if (f->pos >= f->mem_db_entry.fwSize && f->state == 0)
            break;
        switch (f->state)
        {
            case 0:
                f->cnt = (f->mem_db_entry.fwData[f->pos] & 0x7f) + 1;
                if ((f->mem_db_entry.fwData[f->pos] & 0x80) == 0)
                    f->state = 1; /* bytes */
                else
                    f->state = 2; /* zeroes */
                f->pos++;
                break;
            case 1: /* bytes */
                bufPtr[c++] = f->mem_db_entry.fwData[f->pos++];
                f->cnt--;
                if (f->cnt == 0)
                    f->state = 0;
                break;
            case 2: /* zeroes */
                bufPtr[c++] = 0;
                f->cnt--;
                if (f->cnt == 0)
                    f->state = 0;
                break;
        }
    }
    return c;
}

GT_U32 prvCpssSrvCpuFirmareRead(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC   *fp,
    OUT void                             *bufPtr,
    IN  GT_U32                            bufLen
)
{
    PRV_MEM_FILE_STC  *f = (PRV_MEM_FILE_STC*)fp;

    if (!f)
        return PRV_CPSS_FW_READ_ERR;
    if (f->mem_db_entry.compress == 0)
        return prvCpssSrvCpuFirmareReadRaw(f, (GT_U8*)bufPtr, bufLen);
    if (f->mem_db_entry.compress == 2)
        return prvCpssSrvCpuFirmareReadSimpleCompression(f, (GT_U8*)bufPtr, bufLen);
    return PRV_CPSS_FW_READ_ERR;
}

