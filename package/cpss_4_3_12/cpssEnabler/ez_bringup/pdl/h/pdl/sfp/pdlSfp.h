/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\sfp\pdlsfp.h.
 *
 * @brief   Declares the pdlsfp class
 */

#ifndef __pdlSfph

#define __pdlSfph
/**
********************************************************************************
 * @file pdlSfp.h
 * @copyright
 *    (c), Copyright (C) 2023, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - SFP/GBIC library
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

#define                 PDL_SFP_GBIC_PRODUCT_NUMBER_SIZE_CNS								  7
#define					PDL_SFP_GBIC_PART_NUMBER_SIZE_CNS									  9
#define                 PDL_SFP_GBIC_SERIAL_NUMBER_SIZE_CNS									 16
#define                 PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS								 16  
#define					PDL_SFP_GBIC_VENDOR_PN_SIZE_CNS                                      16
 /** 
 * @defgroup SFP SFP
 * @{SFP related functions including
 * - tx enable set/get
 * - loss info get
 * - present info get
 * - eeprom read
*/

/**
 * @enum    PDL_SFP_TX_ENT
 *
 * @brief   Enumerator for SFP tx enable status
 */

typedef enum {
    PDL_SFP_TX_ENABLE_E = 1,
    PDL_SFP_TX_DISABLE_E
} PDL_SFP_TX_ENT;

/**
 * @enum    PDL_SFP_PRESENT_ENT
 *
 * @brief   Enumerator for SFP present status
 */

typedef enum {
    PDL_SFP_PRESENT_TRUE_E = 1,
    PDL_SFP_PRESENT_FALSE_E
} PDL_SFP_PRESENT_ENT;

/**
 * @enum    PDL_SFP_LOSS_ENT
 *
 * @brief   Enumerator for SFP loss status
 */

typedef enum {
    PDL_SFP_LOSS_TRUE_E = 1,
    PDL_SFP_LOSS_FALSE_E
}PDL_SFP_LOSS_ENT;

/**
 * @enum    PDL_SFP_OPERATION_MODE_ENT
 *
 * @brief   Enumerator for SFP oparetion types
 */
typedef enum {
	PDL_SFP_OP_MODE_FIBER_SFP_E,            /* XFP, SFP  */
	PDL_SFP_OP_MODE_DAC_E,					/* direct attached (copper pigtail) */
	PDL_SFP_OP_MODE_COPPER_SFP_E,           /* copper SFP */
	PDL_SFP_OP_MODE_FIBER_SFP_PLUSE_E,		/* SFP+ - including fiber sfp28 */
	PDL_SFP_OP_MODE_UNKNOWN_E,              /* in case not preset or failed to identify */
	PDL_SFP_OP_MODE_LAST_E
} PDL_SFP_OPERATION_MODE_ENT;

typedef enum {
	PDL_SFP_PORT_TYPE_UNKNOWN_E,                  
	PDL_SFP_PORT_TYPE_100_E,                      
	PDL_SFP_PORT_TYPE_1000_E,                     
	PDL_SFP_PORT_TYPE_100_1000_E,                 
	PDL_SFP_PORT_TYPE_10000_E,                    
	PDL_SFP_PORT_TYPE_1000_10000_E,
	PDL_SFP_PORT_TYPE_25G_E, /* supports lower speeds as well (10G & 1G) */
	PDL_SFP_PORT_TYPE_100G_25G_E,                 
	PDL_SFP_PORT_TYPE_100G_E,                     
	PDL_SFP_PORT_TYPE_100G_50G_25G_E,             
	PDL_SFP_PORT_TYPE_50G_25G_E,                  
	PDL_SFP_PORT_TYPE_40G_E,                      
	PDL_SFP_PORT_TYPE_100G_50G_E,                 
	PDL_SFP_PORT_TYPE_LAST_E                     
} PDL_SFP_PORT_TYPE_ENT;

typedef enum {
	/*unknown */
	PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E = 0,

	/*dac*/
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CX_1_E = 15,

	/* SFP+  in byte 3:          in byte 131(in a2) */
	      /* 00010000 = 16       10000000 = 128 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_SR_E = 9,
	      /* 00100000 = 32       01000000 = 64 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LR_E = 8,
	      /* 01000000 = 64       00100000 = 16 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LRM_E = 7,
	      /* 10000000 = 128      00100000 = 32 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_ER_E = 6,
	      /*                     00000010 = 2 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_EW_E = 12,
	      /*                     00000100 = 4 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LW_E = 10,
	      /*                     00001000 = 8 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_SW_E = 11,

	/*SFP  in byte 6: */
	    /* 00000001 = 1 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_SX_E = 5,
	    /* 00000010 = 2 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_LX_E = 4,
	    /* 00000100 = 4 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_CX_E = 3,
	    /* 00010000 = 8 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_T_E = 2,
	   /* 00010000 = 16 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_LX_LX10_E = 13,
	   /* 00100000 = 32 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_FX_E = 14,
	   /* 01000000 = 64 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_BASE_BX_10_E = 1,
	   /* 10000000 = 128 */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_BASE_PX_E = 16,

	/* in byte 2: */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10000BASE_T_E = 17,
	/*user_defined*/
	PDL_SFP_COMPLIANCE_CODE_USER_DEFINED_E = 18,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_D_E = 19,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_U_E = 20,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXD_I_E = 21,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXU_I_E = 22,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_EX_E = 23,
    PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_ZX_E = 28,
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_CWDM_SFP_E = 24,
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CWDM_SFP_E = 25,
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_DWDM_SFP_E = 26,
	/* in byte 36 */
	PDL_SFP_COMPLIANCE_CODE_EXTENDED_E = 27,
	PDL_SFP_COMPLIANCE_CODE_LAST_E = 29
} PDL_SFP_COMPLIANCE_CODE_ENT;

typedef enum {
	/*unknown */
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_UNKNOWN_E = 0,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_SR_E = 2,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_LR_E = 3,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_ER_E = 4,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_RS_FEC_E = 0xb,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_FC_FEC_E = 0xc,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_NO_FEC_E = 0xd,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_WITH_SFI_E = 0x16,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_SR_E = 0x1c,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_5GBASE_T_E = 0x1d,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_2_5GBASE_T_E = 0x1e,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_BR_E = 0x37,
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_BR_E = 0x38
} PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT;

/* if byte 3, 6, 8 and 36 are zero and gibic is CISCO vendor we should match value of byte 96 with this table which include 1G modules */
typedef enum {
	/* 1G gbic types */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1470_E = 0x1,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1490_E = 0x2,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1510_E = 0x3,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1530_E = 0x4,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1550_E = 0x5,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1570_E = 0x6,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1590_E = 0x7,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1610_E = 0x8,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100FX_GE_E = 0x2A,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100FX_FE_E	= 0x2B,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100LX_FE_E	= 0x2C,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_U_E	= 0x2D,
	/*PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_U_I_E = 0x2D,*/
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_D_E   = 0x2E,
	/*PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_D_I_E = 0x2E,*/
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100BX_10_U_FE_E = 0x2F,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100BX_10_D_FE_E = 0x30,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CAB_SFP_50CM_E	= 0x31,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100EX_FE_E	= 0x32,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100ZX_FE_E	= 0x33,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_EX_SMD_E	= 0x3A,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_GE_100EX_E	= 0x3B,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_1002BX_U_E = 0x3C,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_1002BX_D_E = 0x3D,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_2BX_U_E	= 0x3E,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_2BX_D_E	= 0x3F,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T3_GE_E = 0x4B,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T3_FE_E = 0x4C,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T1_FE_E = 0x4D,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_EPON_PX20_U_E = 0x4E,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CSFP_1G_ELPB_E = 0x4F,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_U_I_E	= 0x50,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_D_I_E	= 0x51,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX80_U_I_E	= 0x52,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX80_D_I_E	= 0x53,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_DA_I_E= 0x54,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_T_I_E	= 0x55,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_B_E	= 0x56,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_B_I_E	= 0x57,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_C_E	= 0x58,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_C_I_E	= 0x59,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_GE_DR_LX_E	= 0x5A,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_G_FAST_E	= 0x5B, /* (G.Fast SFP DSL Module Specification: EDCS_1494569) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_T_X_E	= 0x5C, /* (100/1000/10GBase_T SFP Product Requirements: EDCS_226119) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_SYNCEM_T_E	= 0x5D, /* (100/1000/10GBase_T SFP Product Requirements: EDCS_226119) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_SYNCES_T_E	= 0x5E, /* (100/1000/10GBase_T SFP Product Requirements: EDCS_226119) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_OC_VCOP_E  = 0x5F, /* (VCoP SFP Transceiver Module Cisco Product Requirements: EDCS_1518792) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_PDH_VCOP_E = 0x60, /* (VCoP SFP Transceiver Module Cisco Product Requirements: EDCS_1518792) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SC_2G_BX_D_E  = 0x61, /* (C_SFP Module Cisco Product Requirements: EDCS_806570)                 */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_DS1_TCOP_E = 0x62, /* (VCoP SFP Transceiver Module Cisco Product Requirements: EDCS_1518792) */

	/* 10G gbic types */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU1M_E = 0x80,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU3M_E = 0x81,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU5M_E = 0x82,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU7M_E = 0x83,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_CX1_1S_E = 0x84,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_LBX1_E = 0x85,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_USR_E = 0x86,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_LRM_SM_E = 0x87,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_E = 0x88,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_CR_E = 0x89,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_BER_E = 0x8A,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B01_E = 0x8B,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B02_E = 0x8C,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B03_E = 0x8D,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_FET_10G_E = 0x8E,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1271_E = 0x8F,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1291_E = 0x90,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1311_E = 0x91,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1331_E = 0x92,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1351_E = 0x93,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1411_E = 0x94,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1431_E = 0x95,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1451_E = 0x96,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1471_E = 0x97,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1491_E = 0x98,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1511_E = 0x99,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1531_E = 0x9A,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1551_E = 0x9B,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1571_E = 0x9C,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1591_E = 0x9D,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1611_E = 0x9E,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_ZR_I_E = 0x9F,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_61_41_E = 0xA0,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_60_61_E = 0xA1,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_59_79_E = 0xA2,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_58_98_E = 0xA3,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_58_17_E = 0xA4,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_57_36_E = 0xA5,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_56_55_E = 0xA6,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_55_75_E = 0xA7,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_54_94_E = 0xA8,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_54_13_E = 0xA9,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_53_33_E = 0xAA,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_52_52_E = 0xAB,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_51_72_E = 0xAC,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_50_92_E = 0xAD,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_50_12_E = 0xAE,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_49_32_E = 0xAF,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_48_51_E = 0xB0,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_47_72_E = 0xB1,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_46_92_E = 0xB2,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_46_12_E = 0xB3,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_45_32_E = 0xB4,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_44_53_E = 0xB5,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_43_73_E = 0xB6,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_42_94_E = 0xB7,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_42_14_E = 0xB8,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_41_35_E = 0xB9,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_40_56_E = 0xBA,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_39_77_E = 0xBB,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_38_98_E = 0xBC,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_38_19_E = 0xBD,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_37_40_E = 0xBE,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_36_61_E = 0xBF,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_35_82_E = 0xC0,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_35_04_E = 0xC1,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_34_25_E = 0xC2,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_33_47_E = 0xC3,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_32_68_E = 0xC4,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_31_90_E = 0xC5,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_31_12_E = 0xC6,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_30_33_E = 0xC7,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BXD_I_E = 0xC8,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BXU_I_E = 0xC9,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BX40D_I_E = 0xCA,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BX40U_I_E = 0xCB,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1470_E = 0xCC,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1490_E = 0xCD,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1510_E = 0xCE,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1530_E = 0xCF,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1550_E = 0xD0,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1570_E = 0xD1,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1590_E = 0xD2,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1610_E = 0xD3,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1G_SX_E = 0xD6,
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1G_LH_E = 0xD7,

	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_NEXT_PAGE_E = 0xFF, /* move to page 2 (byte 97) */
	PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_LAST_E
} PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ENT;

typedef struct{
									/* The gbic sfp type 100/1000/100_1000/10000/1000_10000 */	
	PDL_SFP_PORT_TYPE_ENT              sfp_port_type;                  
	                                   /* fiber/direct_attach/copper_sfp */
	PDL_SFP_OPERATION_MODE_ENT         sfp_operation_mode;    
										/* 1000BASE-T/CX/LX/SX */
	PDL_SFP_COMPLIANCE_CODE_ENT			   sfp_compliance_code;
	PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT sfp_extended_compliance_code;
	UINT_8								cable_length; /* cable length */
	PDL_CONNECTOR_TECHNOLOGY_ENT			cable_technology; /* passive/active*/
	char                                product_number[PDL_SFP_GBIC_PRODUCT_NUMBER_SIZE_CNS+1];
	char                                part_number[PDL_SFP_GBIC_PART_NUMBER_SIZE_CNS+1];
	char                                serial_number[PDL_SFP_GBIC_SERIAL_NUMBER_SIZE_CNS+1];
	char                                vendor[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS+1];
	char                               *sfp_type_name;
	char                                vendor_pn [PDL_SFP_GBIC_VENDOR_PN_SIZE_CNS+1];
} PDL_sfp_gbic_data_STC;



typedef enum {
	PDL_SFP_CONNECTOR_TYPE_UNKNOWN_E        = 0,    
	PDL_SFP_CONNECTOR_TYPE_LC_E             = 0x7,  
	PDL_SFP_CONNECTOR_TYPE_COPPER_PIGTAIL_E = 0x21, /*dac */
	PDL_SFP_CONNECTOR_TYPE_RJ45_E,
	PDL_SFP_CONNECTOR_TYPE_NOT_CONNECTED_E
} PDL_SFP_CONNECTOR_TYPE_ENT;



/**
 * @struct  PDL_SFP_VALUES_STC
 *
 * @brief   defines structure that hold all SFP specific values
 */

typedef struct {
    /** @brief   The present value */
    UINT_32                               presentValue;
    /** @brief   The loss value */
    UINT_32                               lossValue;
    /** @brief   The transmit enable value */
    UINT_32                               txEnableValue;
    /** @brief   The transmit disable value */
    UINT_32                               txDisableValue;
} PDL_SFP_VALUES_STC;

/**
 * @struct  PDL_SFP_INTERFACE_STC
 *
 * @brief   defines interface for working with sfp (interface information) + predefined values
 */

typedef struct {

    /**
     * @struct  presentInfo
     *
     * @brief   Information about the present.
     */

    struct {
        /** @brief   Whether this functionality is supported */
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } presentInfo;

    /**
     * @struct  lossInfo
     *
     * @brief   Information about the loss.
     */

    struct {
        /** @brief   Whether this functionality is supported */
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } lossInfo;

    /**
     * @struct  txEnableInfo
     *
     * @brief   Information about the transmit enable.
     */

    struct {
        /** @brief   Whether this functionality is supported */
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } txEnableInfo;

    /**
     * @struct  txDisableInfo
     *
     * @brief   Information about the transmit disable.
     */

    struct {
        /** @brief   Whether this functionality is supported */
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } txDisableInfo;

    /**
     * @struct  eepromInfo
     *
     * @brief   Information about the eeprom.
     */

    struct {
        /** @brief   Whether this functionality is supported */
        BOOLEAN                                 isSupported;
        /** @brief   Type of the interface */
        PDL_INTERFACE_TYPE_ENT                  interfaceType;
        /** @brief   Identifier for the interface */
        PDL_INTERFACE_TYP                       interfaceId;
    } eepromInfo;                               /* used to access EEPROM (get gbic model/name, etc.) */
    /** @brief   The values */
    PDL_SFP_VALUES_STC                          values;
} PDL_SFP_INTERFACE_STC;

/**
 * @fn  PDL_STATUS pdlSfpHwTxGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_TX_ENT * statusPtr );
 *
 * @brief   Get sfp operational status (tx enable/disable)
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_TX_ENT            * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwTxSet ( IN UINT_32 dev, IN UINT_32 port, IN PDL_SFP_TX_ENT status );
 *
 * @brief   set sfp operational status (tx enable/disable)
 *
 * @param [in]  dev     - dev number.
 * @param [in]  port    - port number.
 * @param [in]  status  - sfp status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxSet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN PDL_SFP_TX_ENT               status
);

/**
 * @fn  PDL_STATUS pdlSfpHwLossGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_LOSS_ENT * statusPtr );
 *
 * @brief   Get sfp loss status
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp loss status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwLossGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_LOSS_ENT          * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwPresentGet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_SFP_PRESENT_ENT * statusPtr );
 *
 * @brief   Get sfp present status
 *
 * @param [in]  dev         - dev number.
 * @param [in]  port        - port number.
 * @param [out] statusPtr   - sfp present status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwPresentGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_PRESENT_ENT       * statusPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwEepromRead ( IN UINT_32 dev, IN UINT_32 port, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr );
 *
 * @brief   Read sfp eeprom
 *
 * @param [in]  dev             - dev number.
 * @param [in]  port            - port number.
 * @param [in]  i2cAddress      - i2c address to read from MUST be 0x50 or 0x51
 * @param [in]  offset          - offset.
 * @param [in]  length          - length.
 * @param [out] dataPtr         - data that was read from eeprom will be written to this pointer (MUST
 *                                point to a memory allocation the size of length)
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpHwEepromRead(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN  UINT_8                      i2cAddress,
    IN  UINT_16                     offset,
    IN  UINT_32                     length,
    OUT void                      * dataPtr
);

/**
 * @fn  PDL_STATUS pdlSfpHwEepromWrite ( IN UINT_32 dev, IN UINT_32 logicalPort, IN UINT_8 i2cAddress, IN UINT_16 offset, IN UINT_32 data )
 *
 * @brief   Pdl sfp hardware eeprom/internal phy write
 *
 * @param           dev     the device number.
 * @param           port            The port.
 * @param [in]      i2cAddress      i2c address to write MUST be 0x50 or 0x51 or 0x56
 * @param           offset          The offset.
 * @param           mask            The mask.
 * @param [in]      data            The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwEepromWrite(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN  UINT_8                      i2cAddress,
    IN  UINT_16                     offset,
    IN  UINT_32                     mask,
    IN  UINT_32                     data
);

/**
 * @fn  PDL_STATUS pdlSfpDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Sfp debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpDebugSet(
    IN  BOOLEAN             state
);


/**
	* @fn  PDL_STATUS pdlsfpGetGbicData ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char *vendor_pn_PTR)
	*
	* @brief   fill the PDL_sfp_gbic_data_STC fields with the relevant data 
	*
	* @param         dev				  the dev number of the port that needed to be checked 
	* @param         logicalPort		  the logical port number of the port needed to be checked
	* @param [out]   gbic_data_PTR        A pointer to the result of the gbic data structure 
	*
	* @return  A PDL_STATUS.
*/
PDL_STATUS pdlsfpGetGbicData(
	IN UINT_32 dev, 
	IN UINT_32 logicalPort, 
	OUT PDL_sfp_gbic_data_STC *typePtr
);

/**
	* @fn  PDL_STATUS pdlsfpGetTypeName ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char **type_name_PTR)
	*
	* @brief   get string that describe the gbic module type
	*
	* @param         compliance_code	  number that represent the compliance code
	* @param         extended_compliance_code	  number that represent the extended compliance code
	* @param [out]   type_name_PTR        A pointer to the result of the module type name.
	*
	* @return  A PDL_STATUS.
*/
PDL_STATUS pdlsfpGetTypeName ( 
	IN   PDL_SFP_COMPLIANCE_CODE_ENT      module_type,
	IN   PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT   extended_compliance_code,
	OUT  char	                     **type_name_PTR
);
#endif
