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
 * @brief basic H/W read wtite utilities for read/write registers implementation for cpss
 *
* @file cpss_hw_registers_proc.c
*
* $Revision: 2.0 $
 */

#include "cpss_tm_rw_registers_proc.h"

#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssCommon/cpssPresteraDefs.h>

#define TM_GLOBAL_ADDR       0xC000000
#define TM_GLOBAL_REG        0xC0FFFF8
#define TM_GLOBAL_READ_REG   0xC0FFFF0

/* #define WRITE_REGISTERS_DUMP  */

#ifdef WRITE_REGISTERS_DUMP
#include <stdio.h>
#undef CPSS_OS_FILE
#undef CPSS_OS_FILE_INVALID
#undef cpssOsFprintf
#undef cpssOsFopen
#undef cpssOsFclose

#define CPSS_OS_FILE FILE*
#define CPSS_OS_FILE_INVALID NULL
#define cpssOsFprintf   fprintf
#define cpssOsFopen     fopen
#define cpssOsFclose    fclose
#endif

#ifdef WRITE_REGISTERS_DUMP
    CPSS_OS_FILE    fregistersDump=CPSS_OS_FILE_INVALID;
    int     stringIndex=0;
#endif

/* #define RW_TEST  */
/* #define RW_TEST_LOG  */

#ifdef RW_TEST_LOG
    #define RW_TEST
    FILE *  fRWerrorLog=NULL;
#endif

void tm_convert_to_u64
(
    void *dataIn,
    GT_U64 *dataOut
)
{
    /* JIRA CPSS-10204*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    dataOut->l[0]=*((GT_U32 *)dataIn);
    dataOut->l[1]=*(((GT_U32 *)dataIn)+1);
#else
    dataOut->l[1]=*((GT_U32 *)(dataIn));
    dataOut->l[0]=*(((GT_U32 *)dataIn)+1);
#endif

}


void tm_convert_from_u64
(
    GT_U64 *dataIn,
    void   *dataOut
)
{
    /* JIRA CPSS-10204*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    *(((GT_U32 *)dataOut))   = dataIn->l[0];
    *(((GT_U32 *)dataOut)+1) = dataIn->l[1];
#else
    *(((GT_U32 *)dataOut))   = dataIn->l[1];
    *(((GT_U32 *)dataOut)+1) = dataIn->l[0];
#endif

}



/**
 */
int tm_read_register_proc(GT_U8 devNumber,GT_U8 burstMode,  GT_U64 reg_addr,  GT_U64 *data)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];
    GT_U32      dontCare=0x00000123;

    /* 20 LSbits of Cider address */
    regAddr[0] = TM_U32_GET_FIELD(reg_addr.l[0],0,20) + TM_GLOBAL_ADDR;
    /* 23 MSbits of Cider address */
    regAddr[1] = TM_U32_GET_FIELD(reg_addr.l[0],20,12) | (TM_U32_GET_FIELD(reg_addr.l[1],0,11) << 12);

    CPSS_API_LOCK_MAC(devNumber,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if (burstMode)
    {

        ret = prvCpssDrvHwPpPortGroupWriteRamWithoutSkip(devNumber, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, TM_GLOBAL_REG, 1, &(regAddr[1]));
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpReadRegister(devNumber, regAddr[0], &(regValue[0])); /* LSB */
        if (GT_OK != ret) goto out_lbl;

/*        ret = prvCpssHwPpReadRegister(devNumber, regAddr[0]+4, &(regValue[1]));*/ /* MSB */
        ret = prvCpssHwPpReadRegister(devNumber, TM_GLOBAL_READ_REG, &(regValue[1])); /* MSB */
        if (GT_OK != ret) goto out_lbl;
        data->l[0] = regValue[0];
        data->l[1] = regValue[1];
    }
    else
    {
        ret = prvCpssDrvHwPpPortGroupWriteRamWithoutSkip(devNumber, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, TM_GLOBAL_REG, 1, &(regAddr[1]));
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssDrvHwPpPortGroupWriteRamWithoutSkip(devNumber, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, TM_GLOBAL_REG+4, 1, &dontCare);
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpReadRegister(devNumber, regAddr[0], &(regValue[0])); /* LSB */
        if (GT_OK != ret) goto out_lbl;

/*        ret = prvCpssHwPpReadRegister(devNumber, regAddr[0]+4, &(regValue[1]));*/ /* MSB */
        ret = prvCpssHwPpReadRegister(devNumber, TM_GLOBAL_READ_REG, &(regValue[1])); /* MSB */
        if (GT_OK != ret) goto out_lbl;

        data->l[0] = regValue[0];
        data->l[1] = regValue[1];
    }

#ifdef WRITE_REGISTERS_DUMP
    fregistersDump=cpssOsFopen("/tmp/write_registers_dump.txt","a");
    if (fregistersDump != CPSS_OS_FILE_INVALID)
    {
        cpssOsFprintf(fregistersDump,"%6d R address=0x%08x%08x     value=0x%08x%08x\n",
                      stringIndex,reg_addr.l[1],reg_addr.l[0],regValue[1],regValue[0]);
        stringIndex++;
        cpssOsFclose(fregistersDump);
    }
#endif

out_lbl:
    CPSS_API_UNLOCK_MAC(devNumber,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return ret;
}



int tm_write_register_proc(GT_U8 devNumber,GT_U8    burstMode,  GT_U64 reg_addr,  GT_U64 *data)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];
    GT_U32      dontCare=0x00000123;
#ifdef RW_TEST
    GT_STATUS   read_ret;
    GT_U64      readValue;
#endif

    /* 20 LSbits of Cider address */
    regAddr[0] = TM_U32_GET_FIELD(reg_addr.l[0],0,20) + TM_GLOBAL_ADDR;
    /* 23 MSbits of Cider address */
    regAddr[1] = TM_U32_GET_FIELD(reg_addr.l[0],20,12) | (TM_U32_GET_FIELD(reg_addr.l[1],0,11) << 12);

    regValue[0] = data->l[0];
    regValue[1] = data->l[1];

#ifdef WRITE_REGISTERS_DUMP
    fregistersDump=cpssOsFopen("/tmp/write_registers_dump.txt","a");
    if (fregistersDump != CPSS_OS_FILE_INVALID)
    {
        cpssOsFprintf(fregistersDump,"%6d W address=0x%08x%08x     value=0x%08x%08x\n",
                      stringIndex,reg_addr.l[1],reg_addr.l[0],regValue[1],regValue[0]);
        stringIndex++;
        cpssOsFclose(fregistersDump);
    }
#endif

    CPSS_API_LOCK_MAC(devNumber,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if (burstMode)
    {
        ret = prvCpssHwPpWriteRam(devNumber, TM_GLOBAL_REG, 1, &(regAddr[1]));
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpWriteRam(devNumber, regAddr[0], 2, regValue);
    }
    else
    {/*non burst mode*/
        ret = prvCpssHwPpWriteRam(devNumber, TM_GLOBAL_REG, 1, &(regAddr[1]));
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpWriteRam(devNumber, TM_GLOBAL_REG+4, 1, &dontCare);
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpWriteRam(devNumber, regAddr[0], 1, &(regValue[0]));
        if (GT_OK != ret) goto out_lbl;

        ret = prvCpssHwPpWriteRam(devNumber, regAddr[0]+4, 1, &(regValue[1]));
        if (GT_OK != ret) goto out_lbl;

    }
#ifdef RW_TEST
    read_ret=tm_read_register_proc(devNumber,burstMode,reg_addr,&readValue) ;
    if (GT_OK != read_ret) goto out_lbl;
    if ((readValue.l[0] != regValue[0]) || (readValue.l[1] != regValue[1]))
    {
#ifdef RW_TEST_LOG
        fRWerrorLog=fopen("/tmp/rw_errors_log.txt","a");
        if (fRWerrorLog)
        {
            cpssOsFprintf(fRWerrorLog,"address=0x%08x%08x   written_value=0x%08x%08x   read_value=0x%08x%08x \n",
                          reg_addr.l[1],reg_addr.l[0],regValue[1],regValue[0],readValue.l[1],readValue.l[0]);
        }
        fclose(fRWerrorLog);
#endif
        /* place to set breakpoint */
        read_ret=ret;
    }
#endif

out_lbl:
    CPSS_API_UNLOCK_MAC(devNumber,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return ret;
}



#include  "tm_os_interface.h"



int tm_prv_print_rw_registers_log_status(void)
{
    tm_printf(" --------------------\n");
    tm_printf("\n  print write calls : ");
    tm_printf(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_write_requests) ? "yes" : "no");
    tm_printf("\n  print read  calls : ");
    tm_printf(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_read_requests) ? "yes" : "no");
    tm_printf("\n");
    return 0;
}


int tm_prv_set_rw_registers_log_status(int write_status, int read_status)
{
    switch (write_status)
    {
        case 0:
        case 1:break;
        default : tm_printf("wrong write flag value , abort ...\n"); return 1;
    }
    switch (read_status)
    {
        case 0:
        case 1:break;
        default : tm_printf("wrong read flag value , abort ...\n"); return 1;
    }
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_write_requests) = write_status;
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_read_requests) = read_status;
    return tm_prv_print_rw_registers_log_status();
}
