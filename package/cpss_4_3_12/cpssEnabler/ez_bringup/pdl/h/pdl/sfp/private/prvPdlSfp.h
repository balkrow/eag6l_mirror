/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\sfp\private\prvpdlsfp.h.
 *
 * @brief   Declares the prvpdlsfp class
 */

#ifndef __prvPdlSfph

#define __prvPdlSfph
/**
********************************************************************************
 * @file prvPdlSfp.h
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
 * @brief Platform driver layer - SFP private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/sfp/pdlSfp.h>
#include <pdlib/xml/private/prvXmlParser.h>

#define                    PDLP_SFP_DO_NOT_USE_DEV_CNS				-1
#define                    PDLP_SFP_DO_NOT_USE_PORT_CNS              PDLP_SFP_DO_NOT_USE_DEV_CNS


	
#define                 PDLP_SFP_I2C_BASE_ADDRESS_A0_CNS									0x50
#define                 PDLP_SFP_I2C_BASE_ADDRESS_A2_CNS									0x51
#define                 PDLP_SFP_I2C_START_OFFSET_0_CNS 							        0
#define                 PDLP_SFP_MSA_TRANSCEIVER_COMPLIANCE_CODES_1000BASE_MASK_CNS 		0xf
#define                 PDLP_SFP_MSA_TRANSMISSION_MEDIA_MULTI_MODE_CNS                      0xc
#define                 PDLP_SFP_I2C_OFFSET_SFP_IMPORTANT_BYTES_127_CNS					    127
#define                 PDLP_SFP_I2C_ONE_BYTE_LENGTH_1_CNS									1
#define                 PDLP_SFP_10G_ETHERNET_COMPLIANCE_MASK_CNS							0xf0
#define                 PDLP_SFP_BYTE_FULL_SIZE_CNS											8
#define                 PDLP_SFP_HALF_BYTE_SIZE_CNS											4
#define					PDLP_SFP_IS_PASSIVE_CABLE_IN_BYTE_8									4
#define          		PDLP_SFP_UNKNOWN_MODULE_TYPE_NAME_INDEX                             0
#define                 PDLP_SFP_CHECK_ONLY_HIGHER_BYTE_CNS                                 1<<4
#define                 PDLP_SFP_CHECK_ALL_BYTE_CNS                                         1
#define                 PDLP_SFP_I2C_BAYCOM_NAME_LENGTH_6_CNS                               6
#define                 PDL_SFP_MSA_TRANSCEIVER_COMPLIANCE_CODES_1000BAS_LX_CNS             2
#define                 PDLP_SFP_I2C_SPECIAL_CISCO_TWINAX_PN_CNS						    192
#define					PDLP_SFP_NUM_OF_SPECIAL_VENDOR_PN_TABLE_CNS                         6

#define					PDLP_SFP_CISCO_VENDOR_ID_CNS                         				2


#define                 PDLP_SFP_sfp_100_vendor_BAYCOM_MAC(module_type_byte_sfp, vendor_name)   ((0 == strncmp(vendor_name, "BAYCOM", PDLP_SFP_I2C_BAYCOM_NAME_LENGTH_6_CNS)) && (((module_type_byte_sfp & PDLP_SFP_MSA_TRANSCEIVER_COMPLIANCE_CODES_1000BASE_MASK_CNS) == 0) && (module_type_byte_sfp != PDLP_SFP_MSA_TRANSMISSION_MEDIA_MULTI_MODE_CNS)))

typedef enum {

	PDL_SFP_NOMINAL_BIT_RATE_100_MBITS_TO_SEC_E      =       1,
	PDL_SFP_NOMINAL_BIT_RATE_1000_MBITS_TO_SEC_E     =      10,
	PDL_SFP_NOMINAL_BIT_RATE_10000_MBITS_TO_SEC_E    =     100,
	PDL_SFP_NOMINAL_BIT_RATE_25000_MBITS_TO_SEC_E    =     25000
}PDLP_SFP_NOMINAL_BIT_RAT_ENT;


typedef enum {
	PDL_SFP_I2C_BYTES_IDENTIFIER_INDEX_0_E                    =     0,
	PDL_SFP_I2C_BYTES_CONNECTOR_TYPE_INDEX_2_E                =     2,
	PDL_SFP_I2C_BYTES_SFP_PLUSE_MODULE_TYPE_INDEX_3_E         =     3,
	PDL_SFP_I2C_BYTES_SFP_MODULE_TYPE_INDEX_6_E               =     6,
	PDL_SFP_I2C_BYTES_SFP_PLUSE_CABLE_TECH_INDEX_8_E		  =		8,
	PDL_SFP_I2C_BYTES_NOMINAL_BIT_RAT_INDEX_12_E	          =	   12,
    PDL_SFP_I2C_BYTES_SFP_LINK_CAPABILITIES_14_E              =    14, 
	PDL_SFP_I2C_BYTES_CABLE_LENGTH_INDEX_18_E   	          =	   18,
	PDL_SFP_I2C_BYTES_VENDOR_NAME_INDEX_20_E                  =    20,
	PDL_SFP_I2C_BYTES_SFP_EXTENDED_COMPLIANCE_CODE_INDEX_36_E =    36,
	PDL_SFP_I2C_BYTES_VENDOR_PN_INDEX_40_E                    =    40,
	PDL_SFP_I2C_BYTES_ACTIVE_AND_PASIVE_CABLES_INDEX_60_E     =	   60,
	PDL_SFP_I2C_BYTES_NOMINAL_HIGH_SPEED_MAX_BIT_RAT_INDEX_66_E	=  66,
	PDL_SFP_I2C_BYTES_SERIAL_NUMBER_INDEX_68_E				  =	   68,
	PDL_SFP_I2C_BYTES_EXTENDED_ID_INDEX_96_E				  =	   96,
	PDL_SFP_I2C_BYTES_EXTENDED_ID_SECOND_PAGE_INDEX_97_E	  =	   97,
	PDL_SFP_I2C_BYTES_VENDOR_ID_INDEX_98_E				      =	   98,
	PDL_SFP_I2C_BYTES_PRODUCT_NUMBER_INDEX_0x68_E             =  0x68, /* 104 */
	PDL_SFP_I2C_BYTES_PART_NUMBER_INDEX_0x6F_E				  =	 0x6F, /* 111 */
	PDL_SFP_I2C_BYTES_10G_IN_A2_INDEX_131_E                   =    131
}PDLP_SFP_i2c_bytes_ENT;

typedef struct {
	UINT_8                 buf[PDLP_SFP_I2C_OFFSET_SFP_IMPORTANT_BYTES_127_CNS];

} PDL_SFP_PRV_EPROM_BUF_127_STC;

typedef struct {
    char       * vendor_pn;                     
    UINT_32      sfp_compliance_code;
} PDL_SFP_PRV_SPECIAL_VENDOR_PN_STC;

/**
* @addtogroup Sfp
* @{
*/

/**
 * @struct  PDL_SFP_PRV_KEY_STC
 *
 * @brief   defines structure for sfp key in db
 */

typedef struct {
    /** @brief   The development */
    UINT_32                 dev;
    /** @brief   The port */
    UINT_32                 logicalPort;
} PDL_SFP_PRV_KEY_STC;

/**
 * @struct  PDL_SFP_PRV_DB_STC
 *
 * @brief   defines structure stored for sfp in db
 */

typedef struct {
    /** @brief   Information describing the public */
    PDL_SFP_INTERFACE_STC                   publicInfo;
} PDL_SFP_PRV_DB_STC;

/**
 * @fn  PDL_STATUS pdlSfpInit ( IN void );
 *
 * @brief   Init SFP module Create button DB and initialize
 *
 * @param   void    The void.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpInit(
    IN  void
);

/**
 * @fn  PDL_STATUS prvPdlSfpDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpDestroy(
    void
);

/**
 * @fn  PDL_STATUS prvPdlSfpDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpDestroy(
    void
);
/* @}*/
/* @}*/

#endif
