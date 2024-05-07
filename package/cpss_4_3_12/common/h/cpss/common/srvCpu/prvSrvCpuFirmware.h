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
* @file prvSrvCpuFirmware.h
*
* @brief Firmware management APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssGenericSrvCpuFirmware_h__
#define __prvCpssGenericSrvCpuFirmware_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

#define PRV_CPSS_FW_READ_ERR	0XFFFFFFFF

/* Maximal FW files to be loaded by FW loader */
#define PRV_CPSS_SERDES_FW_MAX_FILES_CNS                   4
/* Maximal length for external FW path and file name */
#define PRV_CPSS_SERDES_FW_PATH_AND_NAME_MAX_LEN_CNS     300
/* Maximal length for SerDes file name */
#define PRV_CPSS_SERDES_FW_NAME_MAX_LEN_CNS               40

/**
* @struct PRV_CPSS_FW_METADATA_TYPE_ENT
 *
 * @brief Describes the type of each internal metadata segment.
*/
typedef enum
{
    /** Metadata refers to the SerDes FWs which are supported by a specific L1FW */
    PRV_CPSS_FW_METADATA_SERDES_FIRMWARE_TYPE_E = 1,
    /** Should be last */
    PRV_CPSS_FW_METADATA_LAST_TYPE_E
}PRV_CPSS_FW_METADATA_TYPE_ENT;


/**
* @struct PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_TYPE_ENT
 *
 * @brief Port SerDes firmware types.
*/
typedef enum
{
    /** Lane speed 28G, number of Lanes 1 - used in AC5X */
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_MAIN_E,
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_CMN_25_E,
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_156_E,
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X1_LANE_E,
    /** Lane speed 28G, number of Lanes 4 - used in AC5X */
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_MAIN_E,  
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_CMN_25_E,
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_156_E,   
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C28GP4X4_LANE_E,  
    /** Lane speed 112G, number of Lanes 4 - used in AC5X, AC5P */
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C112GX4_MAIN_E,
    /** Lane speed 56G, number of Lanes 4 - used in Harrier */
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_N5XC56GP5X4_MAIN_E,
    /** Lane speed 12G, number of Lanes 4 - used in Ironman */
    PRV_CPSS_SRV_CPU_SERDES_FW_COMPHY_C12GP41P2V_MAIN_E,
    /** Should be last */
    PRV_CPSS_SRV_CPU_SERDES_FW_SERDES_LAST_E
}PRV_CPSS_SRV_CPU_SERDES_FIRMWARE_TYPE_ENT;

/**
* @struct PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_HEADER_STC
 *
 * @brief A structure to hold SerDes firmware header data.
*/
typedef struct 
{
    /** Device number associated with the firmware */
    GT_U32  devNum;
    /** Unique identifier for the firmware */
    GT_U16  firmwareId;
    /** Version of the firmware (major and minor parts) */
    GT_U32  firmwareVersion;
}PRV_CPSS_GENERIC_SRV_CPU_SERDES_FIRMWARE_HEADER_STC;

/**
* @struct PRV_CPSS_SERDES_FIRMWARE_MEM_FILE_DB_ENTRY_STC
 *
 * @brief A structure to hold SerDes firmware embedded data.
*/
typedef struct {
    /** Firmware name assosiated with the firmware */
    GT_CHAR    *fwName;
    /** Pointer to array that contains data for the firmware */
    GT_U8      *fwDataPtr;
    /** Size of the array that contains data for the firmware */
    GT_U32      fwSize;
}PRV_CPSS_SERDES_FIRMWARE_MEM_FILE_DB_ENTRY_STC;

/**
 * @brief Enum to indicate the type of SerDes firmware data in the union.
 */
typedef enum {
    FW_PATH_INFO,
    FW_LINKED_INFO
} FW_MEM_TYPE_ENT;

/**
 * @brief Structure to hold data related to SerDes firmware.
 *
 * This structure is used to store data related to SerDes firmware in different ways based on the value
 * of the `fwMemType` member. It can either hold the full path name of the SerDes firmware or
 * pointers to the linked firmware in the memory, along with the firmware binary offset.
 */
typedef struct {
    /** Member to indicate the type of SerDes firmware data in the union */
    FW_MEM_TYPE_ENT fwMemType;

    /** Union to hold data related to SerDes firmware */
    union {
        /** Structure to hold data related to SerDes firmware path information */
        struct {
            /** Pointer to the full path name of the SerDes firmware */
            GT_CHAR * fwFullPathNamePtr;
            /** Binary offset of the SerDes firmware */
            GT_U32 fwBinaryOffset;
        } fwPathInfo;

        /** Structure to hold data related to linked SerDes firmware information */
        struct {
            /** Pointer to the SerDes firmware in the embedded memory */
            GT_U8 * fwMemPtr;
            /** The memory size of the loaded SerDes firmware in the embedded memory */
            GT_U8 fwMemSize;
            /** Binary offset of the SerDes firmware */
            GT_U32 fwBinaryOffset;
        } fwLinkedInfo;
    } fwData;
} PRV_CPSS_SERDES_FIRMWARE_DATA_STC;

typedef struct
{
        GT_U16  headerVersion; /* sip6 and below: 1, sip7: 2 */
        GT_U16  mcv;    /* Minimal Compatible Version, verified vs. FW version */
        GT_U32  fw_id;  /* FW unique identifier */
        GT_U32  fw_ver; /* 8bit Maj | 8bit Minor | 16bit user version */
        GT_U32  loadAddr; /* Sram addrees to load FW to */
        GT_U32  ipcShmAddr;
        GT_U16  ipcShmSize;
        GT_U16  metaDataSize; /* Size of rest of header data, stored seperatly from this header */
} PRV_CPSS_FIRMWARE_ENTRY_STC;

typedef struct
{
    GT_U32  checkSum;           /* A 32-bit field that holds a number to detect corruption in the header of SerDes file */
    GT_U16  headerSize;         /* An 8-bit field that provides the total number of the header in bytes */
    GT_U16  headerVersion;      /* A 16-bit field that describes the version of this header. 1 - for SerDes FW */
    GT_U16  devNum;             /* A 16-bit field that holds the silicon definition */
    GT_U16  firmwareId;         /* A 16-bit field that holds the identification of the SerDes firmware module as an enumerated field (e.g., COMPHY28GX4MAIN) */
    GT_U32  firmwareVersion;    /* A 32-bit field that holds the firmware version of the vendor as a 4-digit number (e.g., 1.2.3.5) */
} PRV_CPSS_SERDES_FIRMWARE_ENTRY_STC;

typedef struct {
    PRV_CPSS_FIRMWARE_ENTRY_STC   fw_entry;
    GT_U8      *fwData;
    GT_U32     fwSize; /* size of fw */
    GT_U32     compress;   /* 0 - no compression, 2 - simple compression */
    GT_U8      *metaData;
}  PRV_CPSS_FIRMWARE_MEM_FILE_DB_ENTRY_STC;


PRV_CPSS_FIRMWARE_ENTRY_STC* prvCpssSrvCpuFirmareOpen(
    IN  const GT_U32 fw_id
);
GT_U32 prvCpssSrvCpuFirmareRead(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC *f,
    OUT void                             *bufPtr,
    IN  GT_U32                            bufLen
);
GT_VOID prvCpssSrvCpuFirmareRewind(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC *f
);
GT_STATUS prvCpssSrvCpuFirmareClose(
    IN  PRV_CPSS_FIRMWARE_ENTRY_STC *f
);

/**
* @internal prvCpssSerdesFirmareMemGet function
* @endinternal
*
* @brief   Function retrieves firmware memory data from database.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe
*
* @param[in] fwName                 - SerDes firmware name
* @param[out] serdesFwDataPtr       - (pointer to) data related to SerDes firmware
*
* @retval GT_OK                     - on success 
* @retval GT_NOT_FOUND              - on not found 
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_BAD_STATE              - on database error
* @retval GT_NO_RESOURCE            - on system error
*/
GT_STATUS prvCpssSerdesFirmareMemGet
(
    IN  GT_CHAR    *fwName,
    OUT PRV_CPSS_SERDES_FIRMWARE_DATA_STC  *serdesFwDataPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenericSrvCpuFirmware_h__ */
