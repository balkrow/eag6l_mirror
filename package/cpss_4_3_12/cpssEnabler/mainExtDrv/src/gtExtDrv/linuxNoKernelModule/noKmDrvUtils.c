/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvUtils.c
*
* @brief misc utils
*
* @version   1
********************************************************************************
*/

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

/* must be before prvNoKmDrv.h - to prevent redefinitions */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtDmaLayer.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include "prvNoKmDrv.h"
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
/* definitions collide */
#undef _4K
#undef _64K
#undef _256K
#undef _512K
#undef _1M
#undef _2M
#undef _4M
#undef _8M
#undef _16M
#undef _64M
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#endif /*CHX_FAMILY*/
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

/***************************************************/
/*   register read/write definitions               */
/***************************************************/
GT_U32 prvNoKm_reg_read(GT_UINTPTR regsBase, GT_U32 regAddr)
{
    GT_U32 val;
#ifdef ASIC_SIMULATION
    ASIC_SIMULATION_ReadMemory(regsBase,regAddr,1,&val,regsBase+regAddr);
#else /*!ASIC_SIMULATION*/
    val = *((volatile GT_U32*)(regsBase+regAddr));
#endif /*!ASIC_SIMULATION*/
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "RD base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,val);
#endif
    return CPSS_32BIT_LE(val);
}

void prvNoKm_reg_write(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 value)
{
#ifdef ASIC_SIMULATION
    ASIC_SIMULATION_WriteMemory(regsBase,regAddr,1,&value,regsBase+regAddr);
#else /*!ASIC_SIMULATION*/
    *((volatile GT_U32*)(regsBase+regAddr)) = htole32(value);
#endif /*!ASIC_SIMULATION*/
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "WR base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,value);
#endif
}
void prvNoKm_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 mask, GT_U32 value)
{
    GT_U32 val;
    val = prvNoKm_reg_read(regsBase, regAddr);
    val &= ~mask;
    val |= (value & mask);
    prvNoKm_reg_write(regsBase, regAddr, val);
}

#define BLOCK_SIZE 0x10000

/*******************************************************************************
* prvNoKmDrv_configure_dma
*   Configure DMA for PP
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma(
    IN GT_UINTPTR   regsBase,
    IN int          targetIsMbus
)
{
    GT_U32     attr, target, dmaSize, win;
    GT_UINTPTR dmaBase;

    if (extDrvGetDmaBase(&dmaBase) != GT_OK || dmaBase == 0L)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }

#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "prvExtDrvDmaPtr=%p\n", (void*)dmaBase);
#endif
    extDrvGetDmaSize(&dmaSize);

    if ((dmaBase & 0x000fffff) != 0 || (dmaSize & 0x000fffff) != 0)
    {
        fprintf(stderr, "DMA base address %p and size 0x%x must be at least 1M aligned\n", (void*)dmaBase, dmaSize);
        return GT_FAIL;
    }

    attr    = (targetIsMbus) ? 0x00003e00 : 0x00000e04;
    target  = (targetIsMbus) ? 0x000003e0 : 0x000000e4;

    /* Set Unit Default ID (UDID) Register (0x00000204)
     * DATTR(11:4) = 0x000000e0 == identical to base address reg attr
     * DIDR(3:0)   = 0x00000004 == Target Unit ID PCIe
     */
    prvNoKm_reg_write(regsBase, 0x00000204, target);

    for (win = 0; win < 6 && dmaSize; win++)
    {
        GT_U32 size_mask, size_next;
        /* Calculate the BLOCK_SIZE==2^n where
         * baseAddr is BLOCK_SIZE aligned
         * and
         * dmaSize <= BLOCK_SIZE
         */
        size_mask = 0x00000000;
        while ((dmaBase & size_mask) == 0)
        {
            size_next = (size_mask<<1) | BLOCK_SIZE;
            if ((dmaBase & size_next) || size_next > dmaSize)
                break;
            size_mask = size_next;
            if (size_mask == 0xffff0000)
                break;
        }
        /* Configure DMA base in Base Address n Register (0x0000020c+n*8)
         * Attr(15:8)  = 0x00000e00 == target specific attr
         * Target(3:0) = 0x00000004 == target resource == PCIe
         */
        prvNoKm_reg_write(regsBase, 0x0000020c+win*8, (((GT_U32)dmaBase) & 0xffff0000) | attr);
        /* write dmaBase[63:32] to register 0x23c+n*4 (high address remap n) */
        prvNoKm_reg_write(regsBase, 0x0000023c+win*4, PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1]);

        /* Configure size n register (0x00000210+n*8)
         * Set max size
         */
        prvNoKm_reg_write(regsBase, 0x00000210+win*8, size_mask);

        /* Window Control register n (0x00000254+n*4)
         * WinApn(2:1)    = 0x00000006 == RW
         * BARenable(0:0) = 0x00000000 == Enable
         */
        prvNoKm_reg_write(regsBase, 0x00000254+win*4, 0x00000006);

        if (size_mask == 0xffff0000)
            break;
        dmaBase += (size_mask + BLOCK_SIZE); /* += BLOCK_SIZE */
        dmaSize -= (size_mask + BLOCK_SIZE); /* -= BLOCK_SIZE */
    }

    return GT_OK;
}

/* PCI ATU (Address Translation Unit) registers */
#define OATU_PCI_BASE_ADDR  0x80000000 /*2G*/
#define OATU_PCI_SIZE       0x80000000 /*2G*/

typedef enum {
    ATU_REGION_CTRL_1_REG_OFFSET_IN_TABLE_E,
    ATU_REGION_CTRL_2_REG_OFFSET_IN_TABLE_E,
    ATU_LOWER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_UPPER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_LIMIT_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_LOWER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E,
    ATU_UPPER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E

} ATU_REGION_REG_OFFSET_IN_TABLE;

struct SIP6_ATU_FIELD
{
    GT_U32 offset;
    GT_U32 value;
} atu_table[7] = {
    {0x0, 0x00000000},         /* ATU_REGION_CTRL_1_REG        */
    {0x4, 0x80000000},         /* ATU_REGION_CTRL_2_REG        */
    {0x8, OATU_PCI_BASE_ADDR}, /* ATU_LOWER_BASE_ADDRESS_REG   */
    {0xC, 0x00000000},         /* ATU_UPPER_BASE_ADDRESS_REG   */
    {0x10, 0xffffffff},        /* ATU_LIMIT_ADDRESS_REG        */
    {0x14, 0xffffffff},        /* ATU_LOWER_TARGET_ADDRESS_REG */
    {0x18, 0xffffffff}         /* ATU_UPPER_TARGET_ADDRESS_REG */
};

#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200

#define oATU_REGISTER_ADDRESS(_register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_register))

/*******************************************************************************
* prvNoKmDrv_configure_pre_sip6_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_pre_sip6_dma_per_devNum(
    IN GT_U8            devNum,
    IN GT_U16           busNo,
    IN GT_U8            devSel,
    IN GT_U8            funcNo,
    IN CPSS_HW_INFO_STC *hwInfoPtr
)
{
    GT_STATUS rc;
    GT_U32 targetIsMbus = 0;

    (void)devNum;
    (void)busNo;
    (void)devSel;
    (void)funcNo;

    if(hwInfoPtr->resource.switching.start == 0) {
        return GT_NO_RESOURCE;
    }

    if (hwInfoPtr->busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E) {
        targetIsMbus = 1;
    }

    rc = prvNoKmDrv_configure_dma(hwInfoPtr->resource.switching.start, targetIsMbus);

    return rc;
}

/*/Cider/EBU/AC5/AC5 {Current}/<CNM_IP>CNM_IP/<CNM_IP> <ADDR_DEC> Address Decoder/Units %c*/
#define PHOENIX_CNM_ADDR_DECODER_BASE_ADDR 0x80400000
#define AC5_ON_CHIP_DDR_ADDR  0x200000000L
typedef enum{
CNM_WINDOW_INDEX_RESERVED_0         = 0 ,
CNM_WINDOW_INDEX_AMB3_0             = 1 ,
CNM_WINDOW_INDEX_PCIe               = 2 ,
CNM_WINDOW_INDEX_CPU_ACP            = 3 ,
CNM_WINDOW_INDEX_AMB2               = 4 ,
CNM_WINDOW_INDEX_MG0_CM3_SRAM       = 5 ,
CNM_WINDOW_INDEX_MG1_CM3_SRAM       = 6 ,
CNM_WINDOW_INDEX_MG2_CM3_SRAM       = 7 ,
CNM_WINDOW_INDEX_ROM                = 8 ,
CNM_WINDOW_INDEX_SRAM               = 9 ,
CNM_WINDOW_INDEX_RESERVED_2         = 10,
CNM_WINDOW_INDEX_RESERVED_3         = 11,
CNM_WINDOW_INDEX_DDR                = 12,
CNM_WINDOW_INDEX_iNIC               = 13,
CNM_WINDOW_INDEX_AMB3_1             = 14,
CNM_WINDOW_INDEX_RESERVED_4         = 15,
}CNM_WINDOW_INDEX_ENT;

#if defined(CONFIG_V2_DMA)
/*******************************************************************************
* prvNoKmDrv_get_dma_buf_by_devNum
*   Get DMA buffer by device number
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_get_dma_buf_by_devNum(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32     dmaSize = DMA_LEN;
    GT_UINTPTR dmaBase;

    if ((rc = extDrvGetDeviceDmaBase(devNum, dmaSize, &dmaBase)) != GT_OK)
    {
        printf("extDrvGetDeviceDmaBase() failed, dma not configured\n");
        return rc;
    }

    return rc;
}

/*******************************************************************************
* prvNoKmDrv_set_pci_info_by_devNum
*   Store PCIe BDF info of the device by device number in the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_set_pci_info_by_devNum(
    IN GT_U8    devNum,
    IN GT_U8    pciDomainNum,
    IN GT_U8    pciBusNum,
    IN GT_U8    pciDevNum,
    IN GT_U8    pciFunNum
)
{
    if(osSharedGlobalVarsPtr == NULL)
        return GT_BAD_PTR;

    if(devNum >= GT_MEMORY_DMA_DEVICE_INFO_CNS)
        return GT_BAD_PARAM;

    PRV_SHARED_DEVICE_INFO_DB[devNum].isPciInfoValid = GT_TRUE;
    PRV_SHARED_DEVICE_INFO_DB[devNum].pciDomainNum = pciDomainNum;
    PRV_SHARED_DEVICE_INFO_DB[devNum].pciBusNum = pciBusNum;
    PRV_SHARED_DEVICE_INFO_DB[devNum].pciDevNum = pciDevNum;
    PRV_SHARED_DEVICE_INFO_DB[devNum].pciFunNum = pciFunNum;

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_set_dma_info_by_devNum
*   Store DMA buffer info of the device by device number in the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_set_dma_info_by_devNum(
    IN GT_U8        devNum,
    IN GT_U8        dmaBufCount,
    IN GT_U32       dmaBufLen,
    IN GT_U64_BIT   *dmaBufPhy64
)
{
    if(osSharedGlobalVarsPtr == NULL)
        return GT_BAD_PTR;

    if(devNum >= GT_MEMORY_DMA_DEVICE_INFO_CNS)
        return GT_BAD_PARAM;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].isPciInfoValid == GT_FALSE)
        return GT_SET_ERROR;

    PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64 = osMalloc(sizeof(GT_U64_BIT) * dmaBufCount);
    if(PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64) {

        osMemSet(PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64, 0, (sizeof(GT_U64_BIT) * dmaBufCount));
        osMemCpy(PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64, dmaBufPhy64, (sizeof(GT_U64_BIT) * dmaBufCount));

        PRV_SHARED_DEVICE_INFO_DB[devNum].isDmaInfoValid = GT_TRUE;
        PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufLen = dmaBufLen;
        PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufCount = dmaBufCount;
        return GT_OK;
    }

    return GT_FAIL;
}

/*******************************************************************************
* prvNoKmDrv_clear_dma_info_by_devNum
*   Clear DMA buffer info of the device by device number in the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_clear_dma_info_by_devNum(
    IN GT_U8        devNum
)
{
    if(osSharedGlobalVarsPtr == NULL)
        return GT_BAD_PTR;

    if(devNum >= GT_MEMORY_DMA_DEVICE_INFO_CNS)
        return GT_BAD_PARAM;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].isPciInfoValid == GT_FALSE)
        return GT_GET_ERROR;

    PRV_SHARED_DEVICE_INFO_DB[devNum].isDmaInfoValid = GT_FALSE;
    PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufLen = 0;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64)
        osFree(PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64);

    PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufCount = 0;
    PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_get_pci_info_by_devNum
*   Get PCIe BDF info of the device by device number in the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_get_pci_info_by_devNum(
    IN  GT_U8    devNum,
    OUT GT_U8    *pciDomainNum,
    OUT GT_U8    *pciBusNum,
    OUT GT_U8    *pciDevNum,
    OUT GT_U8    *pciFunNum
)
{
    if(osSharedGlobalVarsPtr == NULL)
        return GT_BAD_PTR;

    if(devNum >= GT_MEMORY_DMA_DEVICE_INFO_CNS)
        return GT_BAD_PARAM;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].isPciInfoValid == GT_FALSE)
        return GT_GET_ERROR;

    *pciDomainNum = PRV_SHARED_DEVICE_INFO_DB[devNum].pciDomainNum;
    *pciBusNum = PRV_SHARED_DEVICE_INFO_DB[devNum].pciBusNum;
    *pciDevNum = PRV_SHARED_DEVICE_INFO_DB[devNum].pciDevNum;
    *pciFunNum = PRV_SHARED_DEVICE_INFO_DB[devNum].pciFunNum;

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_get_dma_info_by_devNum
*   Get DMA buffer info of the device by device number from the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_get_dma_info_by_devNum(
    IN  GT_U8       devNum,
    OUT GT_U8       *dmaBufCount,
    OUT GT_U32      *dmaBufLen,
    OUT GT_U64_BIT  **dmaBufPhy64
)
{
    if(osSharedGlobalVarsPtr == NULL)
        return GT_BAD_PTR;

    if(devNum >= GT_MEMORY_DMA_DEVICE_INFO_CNS)
        return GT_BAD_PARAM;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].isPciInfoValid == GT_FALSE)
        return GT_GET_ERROR;

    if(PRV_SHARED_DEVICE_INFO_DB[devNum].isDmaInfoValid == GT_FALSE)
        return GT_NO_RESOURCE;

    *dmaBufCount = PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufCount;
    *dmaBufLen = PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufLen;
    *dmaBufPhy64 = PRV_SHARED_DEVICE_INFO_DB[devNum].dmaBufPhy64;

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_get_dma_window_config_by_devNum
*   Get DMA range start and end for DMA window configuration by device number
*   from the DB
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_get_dma_window_config_by_devNum(
    IN  GT_U8       devNum,
    OUT GT_U64_BIT  *dmaScatterBufStart,
    OUT GT_U64_BIT  *dmaScatterBufEnd,
    OUT GT_U32      *dmaScatterBufSize
)
{
    GT_STATUS       rc;
    GT_U8           iter0;
    GT_U8           iter1;

    GT_U8           dmaScatterBufCount;
    GT_U32          dmaBufBlockLen;
    GT_U64_BIT      *dmaScatterBufList = NULL;

    GT_U64_BIT      *dmaScatterBufListSorted = NULL;
    GT_U64_BIT      dmaScatterBufTemp = 0;

    GT_U64_BIT      dmaTempScatterBufStart = 0;
    GT_U64_BIT      dmaTempScatterBufEnd = 0;
    GT_U32          dmaTempScatterBufSize = 0;

    GT_BOOL         dmaWindowConfigCheckStatus = GT_FALSE;

    rc = prvNoKmDrv_get_dma_info_by_devNum(devNum, &dmaScatterBufCount, &dmaBufBlockLen, &dmaScatterBufList);
    if(rc != GT_OK) {
        printf("prvNoKmDrv_get_dma_info_by_devNum failed, rc = %d\n", rc);
        PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;
        return rc;
    }

    if(!dmaScatterBufCount || !dmaBufBlockLen || !dmaScatterBufList)
        return GT_FAIL;

    /*
    * Sort DMA addresses in ascending order for further calculations
    */
    dmaScatterBufListSorted = (GT_U64_BIT *)osMalloc(dmaScatterBufCount * sizeof(*dmaScatterBufListSorted));
    if(!dmaScatterBufListSorted)
        return GT_FAIL;

    osMemSet(dmaScatterBufListSorted, 0, dmaScatterBufCount * sizeof(*dmaScatterBufListSorted));
    osMemCpy(dmaScatterBufListSorted, dmaScatterBufList, dmaScatterBufCount * sizeof(*dmaScatterBufListSorted));

    for(iter0=0; iter0<dmaScatterBufCount; iter0++)
    {
        for(iter1=iter0+1; iter1<dmaScatterBufCount; iter1++)
        {
            if(dmaScatterBufListSorted[iter0] > dmaScatterBufListSorted[iter1])
            {
                dmaScatterBufTemp = dmaScatterBufListSorted[iter0];
                dmaScatterBufListSorted[iter0] = dmaScatterBufListSorted[iter1];
                dmaScatterBufListSorted[iter1] = dmaScatterBufTemp;
            }
        }
    }

    dmaScatterBufList = dmaScatterBufListSorted;

    /*
    * Check 1: Is DMA buffer size aligned to 1MB boundary and is less than 2GB?
    */
    if (((dmaBufBlockLen & 0x000fffff) != 0) || (dmaBufBlockLen > 0x80000000))
    {
        printf("DMA memory block size 0x%X is not 1MB aligned or is more than 2GB in size\n", dmaBufBlockLen);
        PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;
        osFree(dmaScatterBufListSorted);
        return GT_FAIL;
    }

    /*
    * Check 2: Is every DMA buffer base aligned to 1MB boundary?
    */
    for(iter0=0; iter0<dmaScatterBufCount; iter0++)
    {
        if ((dmaScatterBufList[iter0] & 0x000fffff) != 0)
        {
            printf("DMA memory block[%d] base 0x%llX is not 1MB aligned\n", iter0, dmaScatterBufList[iter0]);
            PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;
            osFree(dmaScatterBufListSorted);
            return GT_FAIL;
        }
    }

    /*
    * Check 3: Is any DMA buffer on 2GB boundary?
    */
    for(iter0=0; iter0<dmaScatterBufCount; iter0++)
    {
        if (((dmaScatterBufList[iter0] & 0xFFFFFFFF) < 0x80000000U) &&
            (((dmaScatterBufList[iter0] & 0xFFFFFFFF) + dmaBufBlockLen) >
                0x80000000U))
        {
            printf("DMA memory block[%d] base 0x%llX intersects with both 2GB halves of 4GB 32-bit space\n",
                iter0, dmaScatterBufList[iter0]);

            PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;
            osFree(dmaScatterBufListSorted);
            return GT_FAIL;
        }
    }

    dmaTempScatterBufStart = dmaScatterBufList[PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex];

    for(; PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex<dmaScatterBufCount; PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex++) {
        dmaTempScatterBufEnd = dmaScatterBufList[PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex] + dmaBufBlockLen;
        dmaTempScatterBufSize = dmaTempScatterBufEnd - dmaTempScatterBufStart;

        /*
        * Check 4: Is DMA scatter buffer size more than 2GB?
        */
        if (dmaTempScatterBufSize > 0x80000000) {
#if defined(NOKMDRV_DEBUG)
            printf("DMA scatter buffer list size 0x%X is more than '2GB'. Split the list here...\n", dmaTempScatterBufSize);
#endif
            dmaWindowConfigCheckStatus = GT_TRUE;
        }

        /*
        * Check 5: Are DMA scatter buffers crossing 2GB boundary?
        */
        if (((dmaTempScatterBufStart & 0xFFFFFFFF) < 0x80000000U) &&
            ((dmaTempScatterBufEnd & 0xFFFFFFFF) > 0x80000000U))
        {
#if defined(NOKMDRV_DEBUG)
            printf("DMA memory block[%d] in scatter buffer list with base 0x%llX is intersecting with both 2G halves of 4G 32-bit space. Split the list here...\n",
                PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex, dmaTempScatterBufStart, dmaTempScatterBufSize);
#endif
            dmaWindowConfigCheckStatus = GT_TRUE;
        }

        if(dmaWindowConfigCheckStatus) {
            dmaWindowConfigCheckStatus = GT_FALSE;

            /*
             * Return to configure DMA Window
             */
            osFree(dmaScatterBufListSorted);
            return GT_OK;

        } else {
            *dmaScatterBufStart = dmaTempScatterBufStart;
            *dmaScatterBufEnd = dmaTempScatterBufEnd;
            *dmaScatterBufSize = dmaTempScatterBufSize;
        }
    }

    /*
    * Return to configure DMA Window as we have reached end of list
    */
    PRV_SHARED_DEVICE_INFO_DB[devNum].dmaScatterBufIndex = 0;
    osFree(dmaScatterBufListSorted);
    return GT_NO_MORE;
}
#endif

/***************************************************************************************
 * Debug functions
 ***************************************************************************************/


GT_STATUS prvNoKmDrv_debug_print_CnM_Address_Decoder(
    IN GT_U8    devNum,
    IN GT_U32   clientIndex /* 0.. 8*//*0x80400000 + c*0x10000: where c (0-8) represents Address Decoder instance*/
)
{
    GT_U32  regValue,ii,addr,regId, baseAddr = PHOENIX_CNM_ADDR_DECODER_BASE_ADDR;
    struct{
        GT_U32  offset;
        char*   name;
    }infoReg[6] =    {
         {0x00000100 , "Window Attributes and Size High"}
        ,{0x00000104 , "Window Size Low"}
        ,{0x00000108 , "Window Base and Remap High"}
        ,{0x0000010C , "Window Base Low"}
        ,{0x00000110 , "Window Remap Low"}
        ,{0x00000114 , "AXI Attributes"}
    };

    char* clientNames[9] = {
      /*0*/   "CPU - 0x8040_0000"
      /*1*/  ,"CoreSight Trace - 0x8041_0000"
      /*2*/  ,"AMB2 - 0x8042_0000"
      /*3*/  ,"AMB3 - 0x8043_0000"
      /*4*/  ,"iNIC - 0x8044_0000"
      /*5*/  ,"XOR0 - 0x8045_0000"
      /*6*/  ,"XOR1 - 0x8046_0000"
      /*7*/  ,"PCIe - 0x8047_0000"
      /*8*/  ,"GIC  - 0x8048_0000"};

    if(clientIndex >= 9 )
    {
        printf("index[%d] must be less than 9 \n",clientIndex);
        return GT_BAD_PARAM;
    }

    baseAddr &= 0xffff0000;
    baseAddr += 0x00010000 * clientIndex;

    printf("client[%s] \n",clientNames[clientIndex]);

    for(ii = 0 ; ii < 16 ; ii++)
    {
        printf("start window[%d] \n",ii);

        for(regId = 0 ; regId < 6 ; regId++)
        {
            addr = baseAddr + infoReg[regId].offset + ii*0x18;
            regValue = 0;
            prvCpssDrvHwPpReadRegister(devNum,addr, &regValue);

            printf("addr[0x%8.8x] : value [0x%8.8x] --> [%s][%d]  \n",
                addr , regValue , infoReg[regId].name,regId);
        }

        printf("end window[%d] \n\n\n",ii);
    }

    return GT_OK;
}

GT_STATUS prvNoKmDrv_debug_print_MG_Address_Decoder(
    IN GT_U8    devNum,
    IN GT_U32   mgIndex
)
{
    GT_U32  regValue,ii,addr,regId,mgOffset;
    struct{
        GT_U32  offset;
        GT_U32  step;
        char*   name;
    }infoReg[4] =    {
         {0x0000020C , 0x8,"Base Address"}
        ,{0x00000210 , 0x8,"Size (S)"}
        ,{0x0000023C , 0x4,"High Address Remap (HA)"}
        ,{0x00000254 , 0x4,"Window Control Register"}
    };

#ifdef CHX_FAMILY
    {
        GT_BOOL isError;
        PRV_CPSS_DXCH_UNIT_ENT   unitId;
        unitId =  mgIndex == 0 ?
                PRV_CPSS_DXCH_UNIT_MG_E :
                PRV_CPSS_DXCH_UNIT_MG_0_1_E + mgIndex - 1;
        mgOffset = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&isError);
        if(isError)
        {
            printf("MG[%d] not supported \n",mgIndex);
            return GT_BAD_PARAM;
        }
    }
#else /*! CHX_FAMILY*/
    mgOffset = 0;
    GT_UNUSED_PARAM(mgIndex);
#endif/*! CHX_FAMILY*/

    for(ii = 0 ; ii < 6 ; ii++)
    {
        printf("start window[%d] \n",ii);

        for(regId = 0 ; regId < 4 ; regId++)
        {
            addr = mgOffset + infoReg[regId].offset + ii*infoReg[regId].step;
            regValue = 0;
            prvCpssDrvHwPpReadRegister(devNum,addr, &regValue);

            printf("addr[0x%8.8x] : value [0x%8.8x] --> [%s][%d]  \n",
                addr , regValue , infoReg[regId].name,regId);
        }

        printf("end window[%d] \n\n\n",ii);
    }

    return GT_OK;
}


/* debug function to print the oATU registers of the device */
GT_STATUS prvNoKmDrv_debug_print_oAtu_window(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32 data;
    GT_U32 regAddr;

    printf("DUMP oATU settings device[%d] \n",devNum);

#define PRINT_oATU_REG_VALUE(_regAddr,value,_regName)          \
    printf("oATU addr[0x%8.8x] : value [0x%8.8x] --> [%s] \n", \
           _regAddr , value , #_regName)

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LOWER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LOWER_BASE_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_UPPER_BASE_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_UPPER_BASE_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LIMIT_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LIMIT_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_LOWER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_LOWER_TARGET_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_UPPER_TARGET_ADDRESS_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_UPPER_TARGET_ADDRESS_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_REGION_CTRL_1_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_REGION_CTRL_1_REG);

    regAddr = oATU_REGISTER_ADDRESS(atu_table[ATU_REGION_CTRL_2_REG_OFFSET_IN_TABLE_E].offset);
    rc = prvCpssDrvHwPpBar0ReadReg(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }
    PRINT_oATU_REG_VALUE(regAddr,data,ATU_REGION_CTRL_2_REG);

    return GT_OK;
}

/* debug function to print a register value in Bar0/oATU memory space of the device */
GT_STATUS prvNoKmDrv_debug_print_oAtu_register(
    IN GT_U8    devNum,
    IN GT_U32   regOffset
)
{
    GT_STATUS rc;
    GT_U32  data;

    rc = prvCpssDrvHwPpBar0ReadReg(devNum,regOffset,&data);
    if(rc != GT_OK)
    {
        return rc;
    }

    printf("oAtu : devNum[%d] Register[0x%8.8x] Read value[0x%8.8x] \n",
    devNum,regOffset,data);

    return GT_OK;
}


#if defined(CONFIG_V2_DMA)
/* debug function to dump DMA to VIRT address mapping */
GT_STATUS prvNoKmDrv_dump_dma_maping(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    rc = extDrvDumpDmaMapping(PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]));
    return rc;
}
#endif
