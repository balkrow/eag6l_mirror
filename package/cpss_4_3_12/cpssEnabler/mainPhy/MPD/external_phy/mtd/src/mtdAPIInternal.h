/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and data that are strictly internal
for the proper functioning of the API on the Marvell 88X32X0, 88X33X0, 
88X35X0, CUX36X0, CUE26X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#ifndef MTDINTRNL_H
#define MTDINTRNL_H

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

#include "mtdDiagnostics.h"

/* Master-Slave Protocol Definitions */
/* MDIO Register to slave */
#define MAX_BUFF_SIZE_OUT_REG        1,49192 /* MDIO Registers used to communicate with slave code */
#define ACTUAL_BUFF_SIZE_IN_REG      1,49193
#define COMMAND_REG                  1,49194  
#define WORDS_WRITTEN_REG            1,49195  
#define LOW_ADDRESS_REG              1,49196  
#define HIGH_ADDRESS_REG             1,49197  
#define DATA_REG                     1,49198
#define CHECKSUM_REG                 1,49199
#define WORDS_RCVD_REG               1,49200
#define FLASH_MFG_TYPE               1,0xC031
#define NUM_SECTIONS                 1,0xC032

/* Host Commands */
/* #define ERASE_FLASH_PROGRAM_AREA  (0x1) */ /* Erases but does not verify flash is erased */
#define ERASE_FLASH_VERIFY_PROGRAM_AREA (0xA) /* Erases and verifies entire sector was erased  Adds about 1 second to erase time. */
#define FILL_BUFFER               (0x2)
/*#define WRITE_BUFFER              (0x3) no longer used */
/*#define READ_BUFFER               (0x4) no longer used */
/*#define ERASE_FLASH               (0x5) no longer used */
#define WRITE_VERIFY_BUFFER       (0x6)
#define VERIFY_FLASH (0x7)      /* Reads flash and makes sure header and checksum */
                               /* and app checksum match what's stored in the */
                               /* flash header and app area */


/* Slave Responses */
#define MTD_SLAVE_OK          (0x100)
#define MTD_SLAVE_ERR_CMD     (0x200)
#define MTD_SLAVE_FLASH_BUSY  (0x300)
#define MTD_SLAVE_VERIFY_ERR  (0x400)

/* Gross timeouts for different flash operations. These are worst-case times. PHY will respond as fast as it can. If these are reached */
/* it's most likely a failure. */
#define MTD_FLASH_ERASE_WRITE 30000 /* for erase or write/verify operations, M25P40 can take 3 sec * 5 sectors plus verify time */
#define MTD_FLASH_FAST_RESPONSE 500 /* for commands that don't require interaction with the flash device itself, should be quick */

#define MTD_SERDES_LOADER_DONE 0xBFF /* Status of X3540 serdes loader executable when it's done executing */
#define MTD_SERDES_LOADER_SKIPPED 0xBFE /* Status of X3860 serdes loader when it skips loading */



/* This macro is handy for calling a function when you want to test the */
/* return value and return MTD_FAIL, if the function returned MTD_FAIL, */
/* otherwise continue. Will set the return code to MTD_IO_ERROR. */
#define TRY_IO(xFuncToTry) if (xFuncToTry == MTD_FAIL) {*errCode = MTD_IO_ERROR; return MTD_FAIL;}


MTD_STATUS mtdMdioFlashDownload(MTD_DEV_PTR devPtr,MTD_U16 port,
                                MTD_U8 data[],MTD_U32 size,MTD_U16 *errCode);

MTD_STATUS mtdMdioRamDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],
                              MTD_U32 size, MTD_U16 port,
                              MTD_U16 *errCode); 

MTD_STATUS mtdParallelMdioRamDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],
                               MTD_U32 size, MTD_U16 ports[],
                               MTD_U16 numPorts, MTD_U16 erroredPorts[], 
                               MTD_U16 *errCode);

MTD_STATUS mtdParallelMdioFlashDownload(MTD_DEV_PTR devPtr,MTD_U8 data[],
                                 MTD_U32 size, 
                                 MTD_U16 ports[], MTD_U16 numPorts,
                                 MTD_U16 erroredPorts[],
                                 MTD_U16 *errCode);

MTD_STATUS mtdTrySlaveCommand(MTD_DEV_PTR devPtr, MTD_U16 port, MTD_U16 command, MTD_U16 *response, MTD_U16 timeoutMs);
MTD_STATUS mtdTryParallelSlaveCommand(MTD_DEV_PTR devPtr, MTD_U16 command, MTD_U16 ports[], MTD_U16 numPorts, MTD_U16 erroredPorts[], MTD_U16 timeoutMs);


typedef MTD_U32 MEM_SIZE_BYTES;

MEM_SIZE_BYTES mtdGetDevMemorySize(MTD_DEV_PTR devPtr);
MEM_SIZE_BYTES mtdGetDevMaxImageSize(MTD_DEV_PTR devPtr);

MTD_STATUS mtdGetExtendedBase(MTD_DEV_PTR devPtr, MTD_U16  port, MTD_U32 *baseType);

#if MTD_ORIGSERDES
/******************************************************************************
MTD_STATUS mtdDoMeasHalfEye
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 curSelLane,
    IN MTD_U16 reg_8xBE,
    OUT MTD_U32 **data
)

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    curSelLane - XFI lane number, 0 or 1
    reg_8xBE - center phase location

 Outputs:
    data - 26 x 128 array of 32-bit eye data (readout )

 Returns:
    MTD_OK if query was successful, MTD_FAIL if not

 Description:
    The function is translated from the source code of Marvell's SMI Eye Diagram GUI.
    It is called by mtdGetSerdesEyeStatistics to read Serdes eye diagram data
    from register 30.8012 & 30.8013.

 Side effects:
    None

 Notes/Warnings:
    Eye data can only be collected from active Serdes lanes.
******************************************************************************/
MTD_STATUS mtdDoMeasHalfEye
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 curSelLane,
    IN MTD_U16 reg_8xBE,
    OUT MTD_U32 data[][MTD_EYEDIAGRAM_NCOLS]
);
#endif /* MTD_ORIGSERDES */

/******************************************************************************
MTD_STATUS mtdCheckDeviceCapabilities
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 baseType,
    IN MTD_BOOL ignoreFwReportedVals,
    OUT MTD_BOOL *phyHasMacsec,
    OUT MTD_BOOL *phyHasCopperInterface,
    OUT MTD_BOOL *is5GDevice,
    OUT MTD_BOOL *is2P5GDevice,
    OUT MTD_BOOL *isTDevice,
    OUT MTD_BOOL *isOctalDevice
);


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    baseType - the extended base type of PHY
    ignoreFwReportedVals - if MTD_TRUE will ignore whatever the firmware
        reports and instead query all registers directly

 Outputs:
    phyHasMacsec = MTD_TRUE or MTD_FALSE depending on if PHY has
        Macsec/PTP capability
    phyHasCopperInterface = MTD_TRUE or MTD_FALSE depending on if PHY 
        has copper capability
    is5GDevice = MTD_TRUE or MTD_FALSE depending on if the highest
        speed of this PHY is 5G
    is2P5GDevice = MTD_TRUE or MTD_FALSE depending on if the highest
        speed of this PHY is 2.5G
    isTDevice - MTD_TRUE if it's an X3610/E2610 that is a T part
        number, otherwise MTD_FALSE
    isOctalDevice - MTD_TRUE if it's an X3580/E2580, otherwise MTD_FALSE 

 Returns:
    MTD_OK if query is successful, MTD_FAIL otherwise (values returned
    are not reliable if function returns MTD_FAIL)

 Description:
    Queries internal device registers to determine if this device has
    a Macsec and/or a copper interface (T unit) ability or is an
    E20X0 or an E21X1 device.

    If internal processor firmware is running, gets the abilities from
    firmware status registers.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
MTD_STATUS mtdCheckDeviceCapabilities
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 baseType,
    IN MTD_BOOL ignoreFwReportedVals,
    OUT MTD_BOOL *phyHasMacsec,
    OUT MTD_BOOL *phyHasCopperInterface,
    OUT MTD_BOOL *is5GDevice,
    OUT MTD_BOOL *is2P5GDevice,
    OUT MTD_BOOL *isTDevice,
    OUT MTD_BOOL *isOctalDevice
);

/******************************************************************************
MTD_STATUS mtdIsPhyRevisionValid
(
    IN MTD_DEVICE_ID phyRev
);


 Inputs:
    phyRev - a revision id to be checked against MTD_DEVICE_ID type

 Outputs:
    None

 Returns:
    MTD_OK if phyRev is a valid revision, MTD_FAIL otherwise

 Description:
    Takes phyRev and returns MTD_OK if it is one of the MTD_DEVICE_ID
    type, otherwise returns MTD_FAIL.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
MTD_STATUS mtdIsPhyRevisionValid
(
    IN MTD_DEVICE_ID phyRev
);

/******************************************************************************
MTD_STATUS deviceFamilyValid
(
    IN MTD_DEVICE_ID phyRev
);


 Inputs:
    phyRev - a revision id to be checked against MTD_DEVICE_ID type

 Outputs:
    None

 Returns:
    MTD_OK if phyRev is a recognized device family, MTD_FAIL otherwise

 Description:
    Takes phyRev and returns MTD_OK if it is a recognized family of PHY
    or MTD_FAIL if not.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
MTD_STATUS deviceFamilyValid
(
    IN MTD_DEVICE_ID phyRev
);

/* Header information */
typedef struct 
{
    MTD_U32 dataLength;                 
    MTD_U32 dataDestination;       
    MTD_U16 secChecksum;         
    MTD_U16 data_only; 
    MTD_U16 port_skip;
    MTD_U32 nextHeaderOffset;       
} MTD_FILE_HEADER_TYPE;


MTD_BOOL mtdGetHeader(MTD_FILE_HEADER_TYPE *hdr, MTD_U8 *buf); /* Decodes the header and return MTD_TRUE if the checksum is good. */


#if C_LINKAGE
#if defined __cplusplus 
}
#endif 
#endif

#endif /* defined MTDINTRNL_H */
