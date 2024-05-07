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
* @file prvCpssDxChHwCnmAddressDecoder.h
*
* @brief Private definition for CNM address decoder units.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwCnmAddressDecoderh
#define __prvCpssDxChHwCnmAddressDecoderh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT
 *
 * @brief : the Sip6 (AC5X,Ironman) and AC5 address decoder units
 *          other sip6 devices not have those units
*/
typedef enum{
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_CPU_E = 0,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_CORESIGHT_TRACE_E = 1,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_AMB2_E = 2,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_AMB3_E = 3,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_INIC_E = 4,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_XOR0_E = 5,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_XOR1_E = 6,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_PCIE_E = 7,
    SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_GIC_E  = 8,

}SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT;

/**
* @enum SIP6_CNM_WINDOW_INDEX_ENT
 *
 * @brief : the Sip6 (AC5X,Ironman) and AC5 'window indexes' with in the 16 windows
 *          of the address decoder units
*/
typedef enum{
    SIP6_CNM_WINDOW_INDEX_RESERVED_0_E         = 0 ,
    SIP6_CNM_WINDOW_INDEX_AMB3_0_E             = 1 ,
    SIP6_CNM_WINDOW_INDEX_PCIe_E               = 2 ,
    SIP6_CNM_WINDOW_INDEX_CPU_ACP_E            = 3 ,
    SIP6_CNM_WINDOW_INDEX_AMB2_E               = 4 ,
    SIP6_CNM_WINDOW_INDEX_MG0_CM3_SRAM_E       = 5 ,
    SIP6_CNM_WINDOW_INDEX_MG1_CM3_SRAM_E       = 6 ,
    SIP6_CNM_WINDOW_INDEX_MG2_CM3_SRAM_E       = 7 ,
    SIP6_CNM_WINDOW_INDEX_ROM_E                = 8 ,
    SIP6_CNM_WINDOW_INDEX_SRAM_E               = 9 ,
    SIP6_CNM_WINDOW_INDEX_RESERVED_2_E         = 10,
    SIP6_CNM_WINDOW_INDEX_RESERVED_3_E         = 11,
    SIP6_CNM_WINDOW_INDEX_DDR_E                = 12,
    SIP6_CNM_WINDOW_INDEX_iNIC_E               = 13,
    SIP6_CNM_WINDOW_INDEX_AMB3_1_E             = 14,
    SIP6_CNM_WINDOW_INDEX_RESERVED_4_E         = 15,
}SIP6_CNM_WINDOW_INDEX_ENT;

/**
* @enum SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT
 *
 * @brief : the Sip7 address decoder units
*/
typedef enum{
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_iNIC_E       = 0,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_PCIe_E       = 1,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_iunit_E      = 2,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA0_E      = 3,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA1_E      = 4,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA2_E      = 5,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA3_E      = 6,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_EHSM_DMA_E   = 7,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AAC_0_E      = 8,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AAC_1_E      = 9,
    SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AMB_E        = 10,

}SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT;

/**
* @enum SIP7_CNM_WINDOW_INDEX_ENT
 *
 * @brief : the Sip7 'window indexes' with in the 16 windows of the address decoder units
*/
typedef enum{
    SIP7_CNM_WINDOW_INDEX_RESERVED_0_E                          = 0,
    SIP7_CNM_WINDOW_INDEX_ROM_E                                 = 1,
    SIP7_CNM_WINDOW_INDEX_PRAM_ROM_E                            = 2,
    SIP7_CNM_WINDOW_INDEX_eHSM_CMD_E                            = 3,
    SIP7_CNM_WINDOW_INDEX_S_ENIC0_E         /*(Towards DTCM)*/  = 4,
    SIP7_CNM_WINDOW_INDEX_S_ENIC1_E         /*(Towards SRAM)*/  = 5,
    SIP7_CNM_WINDOW_INDEX_iNIC_Mapped_iNIC_and_S_iNIC_E         = 6,
    SIP7_CNM_WINDOW_INDEX_iNIC_Mapped_tile0_E                   = 7,
    SIP7_CNM_WINDOW_INDEX_RESERVED_8_E                          = 8,
    SIP7_CNM_WINDOW_INDEX_iNIC_Mapped_tile1_E                   = 9,
    SIP7_CNM_WINDOW_INDEX_RESERVED_10_E                         =10,
    SIP7_CNM_WINDOW_INDEX_PCIe_E                                =11,
    SIP7_CNM_WINDOW_INDEX_DRAM_E                                =12,
    SIP7_CNM_WINDOW_INDEX_RESERVED_13_E                         =13,
    SIP7_CNM_WINDOW_INDEX_RESERVED_14_E                         =14,
    SIP7_CNM_WINDOW_INDEX_RESERVED_15_E                         =15,
}SIP7_CNM_WINDOW_INDEX_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwCnmAddressDecoderh */

