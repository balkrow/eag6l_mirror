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
* @file snetAasIpe.c
*
* @brief This is a external API definition for IPE unit.
*
* @version   1
********************************************************************************
*/
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemAas.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TStart.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetAasHa.h>
#include <asicSimulation/SKernel/suserframes/snetAasIpe.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>


#define SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_NAME         \
     STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_BAR_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_START_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_MASK_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_START_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_MASK_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_EN_E)\
     ,STR(SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_BITS_E)

static char * sip7IpeStateAddressProfileFieldsTableNames[
    SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_NAME};


static SNET_ENTRY_FORMAT_TABLE_STC sip7IpeStateAddressProfileTableFieldsFormat[
    SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS___LAST_VALUE___E] =
{
    /* SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_BAR_E */
    STANDARD_FIELD_MAC(22)
    /* SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_START_E */
    ,STANDARD_FIELD_MAC(8)
    /* SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_AOFFST_MASK_E */
    ,STANDARD_FIELD_MAC(22)
    /* SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_START_E */
    ,STANDARD_FIELD_MAC(5)
    /* SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_HASH_MASK_E */
    ,STANDARD_FIELD_MAC(22)
    /*SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_EN_E*/
    ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP7_IPE_STATE_ADDRESS_PROFILE_TABLE_FIELDS_SAH_ICT_AOF_BITS_E*/
    ,STANDARD_FIELD_MAC(5)
};



/**
* @internal snetAasIpeTablesFormatInit function
* @endinternal
*
* @brief   init the format of IPVX tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetAasIpeTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    LION3_TABLES_FORMAT_INIT_MAC(
         devObjPtr, SKERNEL_TABLE_FORMAT_IPE_STATE_ADDRESS_PROFILE_E,
         sip7IpeStateAddressProfileTableFieldsFormat, sip7IpeStateAddressProfileFieldsTableNames);
}
