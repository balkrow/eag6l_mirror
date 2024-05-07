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
* @file prvCpssGenericSrvCpuIndirect.c
*
* @brief Indirect IPC support
*
* @version   1
********************************************************************************
*/
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#ifdef SHARED_MEMORY

GT_VOID prvCpssDrvHwDrvReload
(
    IN CPSS_HW_DRIVER_STC          *drv

);
#endif

/************************************************************/
/***    Direct I/O driver                                 ***/
/************************************************************/
typedef struct {
    CPSS_HW_DRIVER_STC  common;
    GT_UINTPTR  virtStart;
    GT_U32      targetStart;
    GT_U32      targetSize;
} PRV_SRVCPU_RAM_DIRECT_STC;
static GT_STATUS prvSrvCpuRamDirectRead(
    IN  PRV_SRVCPU_RAM_DIRECT_STC   *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    volatile GT_U32 *addr;
    GT_UINTPTR virtStart =drv->virtStart;
#ifdef SHARED_MEMORY
    virtStart = PRV_PER_PROCESS_SRAM_BASE_GET(drv->common.devNum);
#endif
    (void)addrSpace;
    if (regAddr < drv->targetStart)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    regAddr -= drv->targetStart;
    if (regAddr+count*4 > drv->targetSize)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    addr = (volatile GT_U32*)(virtStart + (GT_UINTPTR)regAddr);
    for (;count;count--,addr++,dataPtr++)
    {
        *dataPtr = *addr;
        GT_SYNC; /* to avoid read combining */
    }
    return GT_OK;
}
static GT_STATUS prvSrvCpuRamDirectWriteMask(
    IN  PRV_SRVCPU_RAM_DIRECT_STC   *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    volatile GT_U32 *addr;
    GT_UINTPTR virtStart =drv->virtStart;
#ifdef SHARED_MEMORY
    virtStart = PRV_PER_PROCESS_SRAM_BASE_GET(drv->common.devNum);
#endif
    (void)addrSpace;
    if (regAddr < drv->targetStart)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    regAddr -= drv->targetStart;
    if (regAddr+count*4 > drv->targetSize)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    addr = (volatile GT_U32*)(virtStart + (GT_UINTPTR)regAddr);
    for (;count;count--,addr++,dataPtr++)
    {
        GT_U32 data;
        if (mask != 0xffffffff)
        {
            data = *addr;
            data &= ~mask;
            data |= ((*dataPtr) & mask);
        }
        else
        {
            data = *dataPtr;
        }
        *addr = data;
        GT_SYNC; /* to avoid write combining */
    }
    return GT_OK;
}
static GT_VOID prvSrvCpuRamDirectDestroyDrv(
    IN  PRV_SRVCPU_RAM_DIRECT_STC   *drv
)
{
    cpssOsFree(drv);
}

#ifdef SHARED_MEMORY

GT_VOID cpssHwDriverSrvCpuRamDirectDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)prvSrvCpuRamDirectRead;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSrvCpuRamDirectWriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSrvCpuRamDirectDestroyDrv;
}
#endif

CPSS_HW_DRIVER_STC *prvSrvCpuRamDirectCreate(
    IN  GT_U8       devNum,
    IN  GT_UINTPTR  virtStart,
    IN  GT_U32      targetStart,
    IN  GT_U32      targetSize
)
{
    PRV_SRVCPU_RAM_DIRECT_STC *drv;
    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv, 0, sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvSrvCpuRamDirectRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSrvCpuRamDirectWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSrvCpuRamDirectDestroyDrv;
    drv->virtStart = virtStart;
    drv->targetStart = targetStart;
    drv->targetSize = targetSize;
    drv->common.type = CPSS_HW_DRIVER_TYPE_SRV_CPU_RAM_DIRECT_E;
    drv->common.devNum = devNum;

    drv->common.adressSpaceTypeValid = GT_TRUE;
    drv->common.adressSpaceType = CPSS_HW_DRIVER_AS_DRAM_E;

    return (CPSS_HW_DRIVER_STC*)drv;
}

/************************************************************/
/***    Indirect I/O driver                               ***/
/************************************************************/
typedef struct {
    CPSS_HW_DRIVER_STC  common;
    GT_U32              addrSpace;
    GT_U32              regStart;
    GT_U32              size;
} PRV_SRVCPU_RAM_INDIRECT_STC;
static GT_STATUS prvSrvCpuRamIndirectRead(
    IN  PRV_SRVCPU_RAM_INDIRECT_STC   *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    CPSS_HW_DRIVER_STC *p = drv->common.parent;
    (void)addrSpace;
    CPSS_HW_DRIVER_METHOD_READ       read = p->read;
#ifdef SHARED_MEMORY
    CPSS_HW_DRIVER_STC localDrv;
    localDrv.type =p->type;
    prvCpssDrvHwDrvReload(&localDrv);
    read = localDrv.read;
#endif
    if (regAddr + count*4 > drv->size)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
#ifdef CPU_BE
    for (; count; count--, regAddr+=4, dataPtr++)
    {
        GT_U32 data;
        GT_STATUS rc;
        rc = read(p, drv->addrSpace,
            drv->regStart+regAddr, &data, 1);
        if (rc == GT_OK)
        {
            *dataPtr = CPSS_32BIT_LE(data);
        }
    }
    return GT_OK;
#else /* CPU_BE */
    return read(p, drv->addrSpace,
            drv->regStart+regAddr, dataPtr, count);
#endif /* CPU_BE */
}
static GT_STATUS prvSrvCpuRamIndirectWriteMask(
    IN  PRV_SRVCPU_RAM_INDIRECT_STC   *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    CPSS_HW_DRIVER_STC *p = drv->common.parent;
    (void)addrSpace;
    CPSS_HW_DRIVER_METHOD_WRITE_MASK writeMask=p->writeMask;
#ifdef SHARED_MEMORY
    CPSS_HW_DRIVER_STC localDrv;
    localDrv.type =p->type;
    prvCpssDrvHwDrvReload(&localDrv);
    writeMask = localDrv.writeMask;
#endif
    if (regAddr + count*4 > drv->size)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
#ifdef CPU_BE
    GT_UNUSED_PARAM(mask);

    for (; count; count--, regAddr+=4, dataPtr++)
    {
        GT_U32 data;
        data = CPSS_32BIT_LE(*dataPtr);
        writeMask(p, drv->addrSpace,
            drv->regStart+regAddr, &data, 1, 0xffffffff);
    }
    return GT_OK;
#else /* CPU_BE */
    return writeMask(p, drv->addrSpace,
            drv->regStart+regAddr, dataPtr, count, mask);
#endif /* CPU_BE */
}
static GT_VOID prvSrvCpuRamIndirectDestroyDrv(
    IN  PRV_SRVCPU_RAM_INDIRECT_STC   *drv
)
{
    cpssOsFree(drv);
}
CPSS_HW_DRIVER_STC *prvSrvCpuRamIndirectCreate(
    IN  CPSS_HW_DRIVER_STC  *d,
    IN  GT_U32              addrSpace,
    IN  GT_U32              regStart,
    IN  GT_U32              size
)
{
    PRV_SRVCPU_RAM_INDIRECT_STC *drv;
    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv, 0, sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvSrvCpuRamIndirectRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSrvCpuRamIndirectWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSrvCpuRamIndirectDestroyDrv;
    drv->common.parent = d;
    drv->addrSpace = addrSpace;
    drv->regStart = regStart;
    drv->size = size;
    drv->common.type = CPSS_HW_DRIVER_TYPE_SRV_CPU_RAM_INDIRECT_E;
    return (CPSS_HW_DRIVER_STC*)drv;
}

#ifdef SHARED_MEMORY

GT_VOID cpssHwDriverSrvCpuRamIndirectDrvReload
(
    CPSS_HW_DRIVER_STC *drv
)
{
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)prvSrvCpuRamIndirectRead;
    drv->writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSrvCpuRamIndirectWriteMask;
    drv->destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSrvCpuRamIndirectDestroyDrv;
}
#endif
void prvIpcSync
(
    IN  void*   cookie,
    IN  IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    IN  void*   ptr,
    IN  IPC_UINTPTR_T targetPtr,
    IN  IPC_U32 size
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC   *d = (PRV_SRVCPU_IPC_CHANNEL_STC*)cookie;
#ifdef SHARED_MEMORY
    CPSS_HW_DRIVER_STC localDrv;
#endif
    CPSS_HW_DRIVER_STC  *drv =d->drv;

#ifdef SHARED_MEMORY
    localDrv.type = drv->type;
    prvCpssDrvHwDrvReload(&localDrv);
    drv = &localDrv;
#endif
    size = (size+3) & 0xfffffffc; /* align size to 4byte*/
    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E)
    {
        drv->read(d->drv, 0, targetPtr, (GT_U32*)ptr, size/4);
    }
    else
    {
        /* IPC_SHM_SYNC_FUNC_MODE_WRITE_E */
        drv->writeMask(d->drv, 0, targetPtr, (GT_U32*)ptr, size/4, 0xffffffff);
    }
}


GT_VOID prvIpcAccessInit(PRV_SRVCPU_IPC_CHANNEL_STC *s, GT_BOOL init)
{
    s->shmResvdRegs = (GT_UINTPTR)(s->targetBase+(s->size-4));
    if (init == GT_TRUE)
    {
        shmIpcInit(&(s->shm), (void*)((GT_UINTPTR)(s->shmAddr)), s->shmSize,
                   1, prvIpcSync, s);
        return;
    }
    /* init == GT_FALSE, probe */
    s->shm.shmLen = s->shmSize;
    s->shm.server = 1;
    s->shm.shm = s->shmAddr;
    s->shm.syncFunc = prvIpcSync;
    s->shm.syncCookie = s;
}

GT_STATUS prvIpcAccessDestroy(PRV_SRVCPU_IPC_CHANNEL_STC *s)
{
    if (s->drv)
        s->drv->destroy(s->drv);
    cpssOsFree(s);
    return GT_OK;
}
