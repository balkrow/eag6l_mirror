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
* @file snetAasIpe.h
*
* @brief API declaration and data type definition for Ipe
*
* @version   1
********************************************************************************
*/
#ifndef __snetAasIpeh
#define __snetAasIpeh

#include <asicSimulation/SKernel/smain/smain.h>


/**
 * @enum SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of (IPE) State Address Profile table
*/
typedef enum{
     SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_BAR_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_START_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_MASK_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_START_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_MASK_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_EN_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_BITS_E
     ,SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_ENT;



/* macro to set value to field of (IPE) State Addres Profile format in buffer */
#define SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_ENTRY_FIELD_SET_MAC(_dev,_memPtr,_index,fieldName,_value)    \
    SNET_TABLE_ENTRY_FIELD_SET(_dev,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_IPE_STATE_ADDRESS_PROFILE_E)

/* macro to get field value of (IPE) State Addres Profile format in buffer */
#define SIP7_IPE_STATE_ADDRESS_PROFILE_ENTRY_FIELD_GET_MAC(_dev,_memPtr,_index,fieldName)    \
    SNET_TABLE_ENTRY_FIELD_GET(_dev,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_IPE_STATE_ADDRESS_PROFILE_E)

/**
* @internal snetAasIpeTablesFormatInit function
* @endinternal
*
* @brief   init the format of IPE tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetAasIpeTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetAasIpeh */



