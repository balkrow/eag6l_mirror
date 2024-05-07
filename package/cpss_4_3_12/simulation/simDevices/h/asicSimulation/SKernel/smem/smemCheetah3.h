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
* @file smemCheetah3.h
*
* @brief Data definitions for Cheetah3 memories.
*
* @version   34
********************************************************************************
*/
#ifndef __smemCht3h
#define __smemCht3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemCheetah2.h>

/* Central counters blocks number */
#define     CNC_CNT_BLOCKS(dev)  ((dev)->cncBlocksNum)

typedef enum{
    SMEM_CHT_PLR_MEMORY_TYPE_METERING_E,
    SMEM_CHT_PLR_MEMORY_TYPE_COUNTING_E,
    SMEM_CHT_PLR_MEMORY_TYPE_METERING_CONFIG_E,/*new in sip 5.15 */
    SMEM_CHT_PLR_MEMORY_TYPE_CONF_LEVEL_SIGN_E, /*new in sip 5.15 */
    SMEM_CHT_PLR_MEMORY_TYPE_OTHER_E
}SMEM_CHT_PLR_MEMORY_TYPE_ENT;


/**
* @internal smemCht3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObj                - pointer to device object.
*/
void smemCht3Init2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemCht3TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemCht3TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXcatA1TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemXcatA1TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXCatA1UnitPex function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PEX/MBus unit
*
* @param[in] devObjPtr                - pointer to device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] pexBaseAddr              - PCI/PEX/MNus unit base address
*/
void smemXCatA1UnitPex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_U32 pexBaseAddr
);

/* policer management counters mask */
/* 0x40 between sets , 0x10 between counters */
/* 0x00 , 0x10 , 0x20 , 0x30 */
/* 0x40 , 0x50 , 0x60 , 0x70 */
/* 0x80 , 0x90 , 0xa0 , 0xb0 */
#define POLICER_MANAGEMENT_COUNTER_MASK_CNS         0xFFFFFF0F
/* policer management counters base address */
#define POLICER_MANAGEMENT_COUNTER_ADDR_CNS         0x00000500


/* CH3 - policer management counters mask */
/* 0x20 between sets , 0x8 between counters */
/* 0x00 , 0x08 , 0x10 , 0x18 */
/* 0x20 , 0x28 , 0x30 , 0x38 */
/* 0x40 , 0x48 , 0x50 , 0x58 */
#define CH3_POLICER_MANAGEMENT_COUNTER_MASK_CNS         0xFFFFFF87



ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWritePclAction         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWritePolicerTbl        );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteCncFastDumpTrigger);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteRouterAction         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteFDBGlobalCfgReg      );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXcatA1ActiveWriteVlanTbl            );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXcatActiveWriteMacModeSelect        );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicerTbl           );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIPFixTimeStamp       );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteLogTargetMap         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteCncInterruptsMaskReg );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicyTcamConfig_0   );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicerMemoryControl );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIplr0Tables                    );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIplr1Tables                    );

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteBridgeGlobalConfig2Reg);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEgressFilterVlanMap);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEgressFilterVlanMember);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEqGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIpclGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteTtiInternalMetalFix);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteTtiGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteHaGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteMllGlobalConfigReg);

/*l2 mll*/
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteL2MllVidxEnable);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteL2MllPointerMap);

/*read active memory*/
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveCncBlockRead            );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveCncWrapAroundStatusRead );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixNanoTimeStamp  );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSecLsbTimeStamp);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSecMsbTimeStamp);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSampleLog);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadPolicerManagementCounters);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIeeeMcConfReg);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIplr0Tables);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIplr1Tables);

/* PCL/Router TCAM BIST done simulation */
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveReadTcamBistConfigAction);

void smemLion2RegsInfoSet_GOP_SERDES
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   s/*serdes*/,
    IN GT_U32                   minus_s/*compensation serdes*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_gigPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_XLGIP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset
);

void smemLion2RegsInfoSet_GOP_MPCSIP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_PTP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_SIP5_PP_REGS_ADDR_STC       * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemCht3h */


