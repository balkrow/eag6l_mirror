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
* @file snetAasExactMatch.h
*
* @brief This is a external API definition for SIP7 Exact Match
*
* @version   1
********************************************************************************
*/
#ifndef __snetAasExactMatchh
#define __snetAasExactMatchh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* size of EXACT_MATCH KEY in words  */
#define SIP7_EMX_MAX_CHANNEL_CNS                  8

/* max number of Serial EM lookups  */
#define SIP7_EXACT_MATCH_SERIAL_EM_MAX_CNS        3
/* size of Serial EM Key in words (5B) */
#define SIP7_EXACT_MATCH_SERIAL_EM_MAX_KEY_SIZE_WORD_CNS        2

/* size of EXACT_MATCH KEY in bytes   */
#define SIP7_EXACT_MATCH_MAX_KEY_SIZE_BYTE_CNS    53
/* size of EXACT_MATCH KEY in words  */
#define SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS    14
/* size of EXACT_MATCH KEY in bits   */
#define SIP7_EXACT_MATCH_MAX_KEY_SIZE_BIT_CNS     424
/* size of REDUCED EM Key size in bits   */
#define SIP7_EXACT_MATCH_REDUCED_MAX_KEY_SIZE_BIT_CNS     40
/* max number of banks in EXACT_MATCH */
#define SIP7_EXACT_MATCH_MAX_NUM_BANKS_CNS        16
/* size of EXACT_MATCH expanded action in bytes */
#define SIP7_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS 45

/* size of EXACT_MATCH full action in words   */
#define SIP7_EXACT_MATCH_FULL_ACTION_SIZE_WORD_CNS   12

/* default entry match indicator */
#define SNET_SIP7_EXACT_MATCH_SERIAL_EM_DEFAULT_INDEX_CNS      (0x7ffffff0)

/* size of key field in exact match key entry in bit    */
#define SIP7_EXACT_MATCH_KEY_ONLY_FIELD_SIZE_CNS               128

typedef struct {
    GT_U32 valid            ;       /*1 bit */
    GT_U32 entry_type       ;       /*1 bit - 0  EXACT_MATCH Key and Action
                                    *         1  EXACT_MATCH Key Only       */

    /*if entry is key and action*/
    GT_U32 age              ;       /* 1 bit  */
    GT_U32 key_size         ;       /* 2 bits */
    GT_U32 lookup_number    ;       /* 1 bit  */
    GT_U32 key_31_0         ;
    GT_U32 key_39_32        ;       /* 40 bits */
    GT_U32 action_31_0      ;
    GT_U32 action_63_32     ;
    GT_U32 action_83_64     ;       /* 84 bits */

    /*if entry is key only */
    GT_U32 keyOnly_31_0       ;
    GT_U32 keyOnly_63_32      ;
    GT_U32 keyOnly_95_64      ;
    GT_U32 keyOnly_127_96     ;      /* 128 bits */

}SNET_SIP7_EXACT_MATCH_ENTRY_INFO;

/* Enum values represent byte size of each key */
typedef enum{
    SIP7_EXACT_MATCH_KEY_SIZE_5B_E  = 0,     /* 5 bytes  */
    SIP7_EXACT_MATCH_KEY_SIZE_21B_E = 1,     /* 21 bytes */
    SIP7_EXACT_MATCH_KEY_SIZE_37B_E = 2,     /* 37 bytes */
    SIP7_EXACT_MATCH_KEY_SIZE_53B_E = 3,     /* 53 bytes */

    SIP7_EXACT_MATCH_KEY_SIZE_LAST_E         /* last value */
}SIP7_EXACT_MATCH_KEY_SIZE_ENT;

/**
* @enum SIP7_EXACT_MATCH_UNIT_ENT
 *
 * @brief types of EM units
*/
typedef enum{

    /** EM 0 */
    SIP7_EXACT_MATCH_UNIT_0_E,
    /** EM 1 */
    SIP7_EXACT_MATCH_UNIT_1_E,
    /** EM 2 */
    SIP7_EXACT_MATCH_UNIT_2_E,
    /** EM 3 */
    SIP7_EXACT_MATCH_UNIT_3_E,
    /** EM 4 */
    SIP7_EXACT_MATCH_UNIT_4_E,
    /** EM 5 */
    SIP7_EXACT_MATCH_UNIT_5_E,
    /** EM 6 */
    SIP7_EXACT_MATCH_UNIT_6_E,
    /** EM 7 */
    SIP7_EXACT_MATCH_UNIT_7_E,
    /** REDUCED EM */
    SIP7_EXACT_MATCH_UNIT_REDUCED_E,
    /** total number of EM units */
    SIP7_EXACT_MATCH_UNIT_LAST_E
} SIP7_EXACT_MATCH_UNIT_ENT;

typedef enum{
    SIP7_EMX_CLIENT_TTI_E,     /* TTI             */
    SIP7_EMX_CLIENT_IPCL1_E,   /* Ingress Pcl 1   */
    SIP7_EMX_CLIENT_IPCL2_E,   /* Ingress Pcl 2   */
    SIP7_EMX_CLIENT_MPCL_E,    /* SIP7 Midway Pcl */
    SIP7_EMX_CLIENT_EPCL_E,    /* Egress Pcl      */
    SIP7_EMX_CLIENT_IPE_E,     /* SIP7 IPE        */
    SIP7_EMX_CLIENT_TTI_SERIAL_EM_E, /* SIP7 TTI Serial EM */
    SIP7_EMX_CLIENT_LAST_E     /* last value - used for context reset */
}SIP7_EMX_CLIENT_ENT;

/* Converts HW key size to macro  */
#define SMEM_SIP7_EXACT_MATCH_HW_KEY_SIZE_TO_MAC(hwKeySize)   \
    (                                                         \
     (hwKeySize) == 0x0 ? SIP7_EXACT_MATCH_KEY_SIZE_5B_E  :   \
     (hwKeySize) == 0x1 ? SIP7_EXACT_MATCH_KEY_SIZE_21B_E :   \
     (hwKeySize) == 0x2 ? SIP7_EXACT_MATCH_KEY_SIZE_37B_E :   \
     (hwKeySize) == 0x3 ? SIP7_EXACT_MATCH_KEY_SIZE_53B_E :   \
     0)

/**
* @enum SIP7_SERIAL_EM_TYPE_ENT
 *
 * @brief SERIAL EM types.
*/
typedef enum{
    SIP7_SERIAL_EM0_E,
    SIP7_SERIAL_EM1_E,
    SIP7_SERIAL_EM2_E
} SIP7_SERIAL_EM_TYPE_ENT;

/**
 * @enum SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_ENT
 *
 * @brief types of EM units
*/
typedef enum{

    /** VLAN Tag*/
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E,
    /** MPLS */
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_MPLS_E,
    /** UDBs 20-24 */
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_UDB_20_24_E,
    /** UDBs 25-29 */
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_UDB_25_29_E,
    /** total number of Key Types */
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_LAST_E
} SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_ENT;

/*
 * @enum SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_ENT
 *
 * @brief enum of the Serial EM VLAN Tag key fields
 */
typedef enum{
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB28_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_IS_RANGE_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_FOUND_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB29_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_IS_RANGE_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_FOUND_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_SRC_PORT_PROFILE_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_PCL_ID_E,
    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_LAST_E
}SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_ENT;

/*
 * @enum SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_ENT
 *
 * @brief enum of the Serial EM MPLS key fields
 */
typedef enum{
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_GENERIC_CLASSIFICATION_E,
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_VALID_E,
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_E,
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_MAC2ME_E,
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_PCL_ID_E,
    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LAST_E
}SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_ENT;

/**
* @struct SIP7_SERIAL_EM_ENTRY_STC
 *
 * @brief Exact Match Serial EM Entry
*/
typedef struct{
    /** @brief Serial EM Profile ID */
    GT_U32  serialEmProfileId;
    /** @brief Serial EM Key Type */
    SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_ENT  serialEmKeyType;
    /** @brief Flag to Enable UDB28 VLAN Tag Key */
    GT_BOOL serialEmEnableUDB28VlanTagKey;
    /** @brief Flag to Enable UDB29 VLAN Tag Key */
    GT_BOOL serialEmEnableUDB29VlanTagKey;
    /** @brief Serial EM Key Type */
    GT_U32  serialEmPclId;
} SIP7_SERIAL_EM_ENTRY_STC;

/**
 * struct: EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC
 *
 * purpose : hold info about a field that is part of the key
 *   startBitInKey - the starting bit of the field in the key (include this bit)
 *   endBitInKey   - the end bit of the field in the key (include this bit)
 *   debugName     - used for debugging --- internal use
*/
typedef struct{
    GT_U32                      startBitInKey;
    GT_U32                      endBitInKey;
    char*                       debugName;
}EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC;

/**
* @internal snetSip7ExactMatchTablesFormatInit function
* @endinternal
*
* @brief init the format of SIP7 Exact Match tables.
*
* @param[in] devObjPtr      - pointer to device object.
*/
void snetSip7ExactMatchTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

/**
* @internal snetAasExactMatchLookupInEmx function
* @endinternal
*
* @brief   For a given key, perform the lookup in Exact match table and
*          fill the results array
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX Client
* @param[in] exactMatchProfileIdTableIndex  - index to the EM Profile table
* @param[in] keyArrayPtr                    - superkey array (size up to 120 bytes)
* @param[in] keySize                        - size of the TCAM key
* @param[in] matchIndexArr                  - Matched lookup index array filled by TCAM
* @param[in] isMatchDoneInTcamOrEmArr       - Array to denote hit by TCAM or EM
* @param[in] tcamOverExactMatchPriorityArr  - Array to denote priority is TCAM or EM
*/
GT_U32 snetAasExactMatchLookupInEmx
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            exactMatchProfileIdTableIndex,
    IN  GT_U32                            *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT            tcamKeySize,
    IN  GT_U32                            matchIndexArr[],
    IN  GT_BOOL                           isMatchDoneInTcamOrEmArr[],
    IN  GT_BOOL                           tcamOverExactMatchPriorityArr[]
);

/**
* @internal snetAasExactMatchLookup function
* @endinternal
*
* @brief   For a given key, perform the lookup in Exact match table and
*          fill the results array
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] emxClient                      - EMX Client
* @param[in] keyArrayPtr                    - superkey array (size up to 120 bytes)
* @param[in] keySize                        - size of the EM key
* @param[in] exactMatchProfileIdArr         -  Exact Match lookup profile ID array
* @param[in] emUnitNumArr                   - Array of EM unit numbers
* @param[out] exactMatchClientMatchArr      - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                     - Exact Match hits number of results
*
*/
GT_U32 snetAasExactMatchLookup
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT            keySize,
    IN  GT_U32                            exactMatchProfileIdArr[],
    OUT SIP7_EXACT_MATCH_UNIT_ENT         emUnitNumArr[],
    OUT GT_BOOL                           exactMatchClientMatchArr[],
    OUT GT_U32                            resultArr[]
);

/**
* @internal snetAasExactMatchProfileIdGet function
* @endinternal
*
* @brief   Gets Exact Match Lookup Profile ID1/ID2 for EMX Clients
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX client
* @param[in] index                          - keyType in case of TTI
*                                             or index for Exact match Profile-ID mapping table
*                                             in case of PCL/EPCL
* @param[out] exactMatchProfileIdArr        - Exact Match Lookup
*                                             Profile ID Array
*/
GT_VOID snetAasExactMatchProfileIdGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            index,
    OUT GT_U32                            exactMatchProfileIdArr[]
);

/**
* @internal snetAasExactMatchParseEntry function
* @endinternal
*
* @brief   Parses Exact match entry to the struct
*
* @param[in] devObjPtr                    - (pointer to) the device object
* @param[in] exactMatchEntryPtr           - (pointer to) Exact match entry
* @param[in] entryIndex                   -  entry index (hashed index)
* @param[out] keySize                     - (pointer to) Exact match entry parsed into fields
*
*/
GT_VOID snetAasExactMatchParseEntry
(
    IN    SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN    GT_U32                               *exactMatchEntryPtr,
    IN    GT_U32                                entryIndex,
    OUT   SNET_SIP7_EXACT_MATCH_ENTRY_INFO     *exactMatchEntryInfoPtr
);

/**
* @internal snetAasExactMatchActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in] devObjPtr                       - pointer to device object.
* @param[in] descrPtr                        - pointer to frame descriptor.
* @param[in] matchIndex                      - index to the action table .
* @param[in] emxClient                       - EMX client
* @param[in] lookupNum                       - EM lookup number
* @param[in] exactMatchProfileIdTableIndex   - index for Exact match Profile-ID mapping table
* @param[out] actionDataPtr                  - (pointer to) action data
*/
GT_VOID snetAasExactMatchActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                               matchIndex,
    IN SIP7_EMX_CLIENT_ENT                  emxClient,
    IN GT_U32                               lookupNum,
    IN GT_U32                               exactMatchProfileIdTableIndex,
    OUT GT_U32                             *actionDataPtr
);

/**
* @internal snetAasExactMatchUnitNumGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in] devObjPtr                       - pointer to device object.
* @param[in] emxClient                       - EMX client
* @param[in] lookupNum                       - EM lookup number
* @param[out] emUnitNumPtr                   - (pointer to) emUnit number
*/
GT_BOOL snetAasExactMatchUnitNumGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SIP7_EMX_CLIENT_ENT                 emxClient,
    IN GT_U32                              lookupNum,
    OUT SIP7_EXACT_MATCH_UNIT_ENT           *emUnitNumPtr
);

/**
* @internal snetAasSerialEmProfileParamsGet function
* @endinternal
*
* @brief   Gets the Serial EM Lookup Profile EM0/EM1/EM2 for EMX Clients
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX client
* @param[in] serialEmPortProfileId          - Serial EM Port Profile Id
* @param[out] serialEmProfileParamsPtr      - Pointer to SIP7_SERIAL_EM_ENTRY_STC
*
*/
GT_VOID snetAasSerialEmProfileParamsGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  SIP7_SERIAL_EM_TYPE_ENT           serialEm,
    IN  GT_U32                            serialEmPortProfileId,
    OUT SIP7_SERIAL_EM_ENTRY_STC          *serialEmProfileParamsPtr
);

/**
* @internal snetAasExactMatchSerialEmLookup function
* @endinternal
*
* @brief   For a given Profile ID, perform the Serial EM lookups and
*          update the Action pointer
*
* @param[in]  devObjPtr                - pointer to device object.
* @param[in]  descrPtr                 - pointer to frame descriptor.
* @param[in]  emxClient                - EMX Client
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
GT_VOID snetAasExactMatchSerialEmLookup
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    OUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
);

/**
* @internal snetAasExactMatchSerialEmActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in] devObjPtr                       - pointer to device object.
* @param[in] descrPtr                        - pointer to frame descriptor.
* @param[in] matchIndex                      - index to the action table .
* @param[in] emUnitNum                       - Exact Match unit number
* @param[in] lookupNum                       - EM lookup number
* @param[in] serialEmProfileId               - index for Exact match Serial EM Profile-ID mapping table
* @param[out] actionDataPtr                  - (pointer to) action data
*/
GT_VOID snetAasExactMatchSerialEmActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                              matchIndex,
    IN SIP7_EXACT_MATCH_UNIT_ENT           emUnitNum,
    IN GT_U32                              lookupNum,
    IN GT_U32                              serialEmProfileId,
    OUT GT_U32                             *actionDataPtr
);

/**
* @internal snetAasExactMatchSerialEmTTProcessAction function
* @endinternal
*
* @brief  Serial EM T.T Engine processing for outgoing frame on AAS and above:
*         actions to descriptor processing
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in]     emUnitNum                - Exact Match unit number
* @param[in]     lookupNum                - Exact Match lookup Number
* @param[in]     matchIndex               - index in action table
* @param[in]     serialEmProfileId        - Serial EM profile ID
* @param[out]    internalTtiInfoPtr       - (pointer to) internal TTI info
*/
GT_VOID snetAasExactMatchSerialEmTTProcessAction
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SIP7_EXACT_MATCH_UNIT_ENT         emUnitNum,
    IN    GT_U32                            lookupNum,
    IN    GT_U32                            matchIndex,
    IN    GT_U32                            serialEmProfileId,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetAasExactMatchh */

