/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlSfp.c
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
 * @brief Platform driver layer - SFP related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/sfp/pdlSfp.h>
#include <pdl/sfp/private/prvPdlSfp.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/sfp/pdlSfpDebug.h>
#include <pdl/interface/private/prvPdlI2c.h>
#ifdef LINUX_HW
extern int usleep(unsigned int);
#endif

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl sfp database */
static PRV_PDLIB_DB_TYP   pdlSfpDb;

static const char *  prvPdlSfpComplianceCodeStrARR[PDL_SFP_COMPLIANCE_CODE_LAST_E + 1] = {
	"Unknown",  
	"1000BASE-BX10",
	"1000BASE-T",
	"1000BASE-CX",
	"1000BASE-LX",
	"1000BASE-SX",
	"10G BASE-ER",
	"10G BASE-LRM",
	"10G BASE-LR",
	"10G BASE-SR",
	"10G BASE-LW",
	"10G BASE-SW",
	"10G BASE-EW",
	"100BASE-LX",
	"100BASE-FX",
	"10G BASE-CX1",
	"BASE-PX",
	"10G BASE-T",
    "User Defined",
    "1000BASE-BX10-D",
    "1000BASE-BX10-U",
    "10G BASE-BXD-I",
    "10G BASE-BXU-I",
    "1000BASE-EX",
	"1000BASE-CWDM",
	"10G BASE-CWDM",
	"10G BASE-DWDM",
	"Extended",
    "1000BASE-ZX",
	"Last"
};

static const char*        prvPdlSfpNoAccessToI2CAddressA2GbicsTable[] = {
	/* vendor pn */ 
	"UF-RJ45-1G",
	"SP7041-HPB-4",
	"SP7052-C"
};

static PDL_SFP_PRV_SPECIAL_VENDOR_PN_STC prvPdlSfpSpecialVendorPnARR[PDLP_SFP_NUM_OF_SPECIAL_VENDOR_PN_TABLE_CNS] = {  
    {"GLC-BX-D", PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_D_E},
    {"GLC-BX-U", PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_U_E},
    {"SFP-10G-BXD-I", PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXD_I_E},
    {"SFP-10G-BXU-I", PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXU_I_E},
    {"GLC-EX-SMD", PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_EX_E},
    {"MGBLH1", PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_EX_E}
};

static UINT_32  pdlsfpA2GbicsTableSize = sizeof(prvPdlSfpNoAccessToI2CAddressA2GbicsTable) / sizeof(char*);

static const PDL_SFP_COMPLIANCE_CODE_ENT prvPdlSfpComplianceTypeDbByte3SfpPluseARR [] =
{
	/* sfp byte 3 (low reg side)- Infiniband now system don't support it */  
	/* sfp byte 3 (high reg side)- 10G Ethernet SFP+ */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_SR_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LR_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LRM_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_ER_E
};

static const PDL_SFP_COMPLIANCE_CODE_ENT prvPdlSfpComplianceTypeDbByte6SfpARR [] =
{
	/* sfp byte 6 (low reg side)- Ethernet SFP */
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_SX_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_LX_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_CX_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_T_E,

	/* sfp byte 6 (high reg side)- alsow Ethernet SFP */ 
	PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_LX_LX10_E,	PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_FX_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_BASE_BX_10_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_BASE_PX_E
};


static const PDL_SFP_COMPLIANCE_CODE_ENT prvPdlSfpComplianceTypeDbByte131SfpPluseARR [] =
{
	PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_EW_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LW_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_SW_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LRM_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_ER_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LR_E, PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_SR_E
};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlSfpDebugFlag)
/** @brief   The interface type string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdSfpValueTypeStrToEnumPairs[] = {
    {"eeprom", PDL_SFP_VALUE_EEPROM_E },
    {"loss", PDL_SFP_VALUE_LOSS_E  },
    {"present", PDL_SFP_VALUE_PRESENT_E },
    {"tx_enable", PDL_SFP_VALUE_TX_ENABLE_E },
    {"tx_disable", PDL_SFP_VALUE_TX_DISABLE_E }
};
/** @brief   The interface type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdSfpValueTypeStrToEnum = {prvPdSfpValueTypeStrToEnumPairs, sizeof(prvPdSfpValueTypeStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/**
 * @fn  PDL_STATUS pdlSfpHwTxGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_TX_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware transmit get
 *
 * @param           dev         The device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_TX_ENT            * statusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             data;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;

    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "statusPtr POINTER NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sfpPtr->publicInfo.txEnableInfo.isSupported == FALSE) {
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = prvPdlInterfaceHwGetValue(sfpPtr->publicInfo.txEnableInfo.interfaceType, sfpPtr->publicInfo.txEnableInfo.interfaceId, &data);
    PDL_CHECK_STATUS(pdlStatus);
    if (data == sfpPtr->publicInfo.values.txEnableValue) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "[dev %d port %d] read %x expected %x", dev, logicalPort, data, sfpPtr->publicInfo.values.txEnableValue);
        *statusPtr = PDL_SFP_TX_ENABLE_E;
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "[dev %d port %d] read %x expected %x", dev, logicalPort, data, sfpPtr->publicInfo.values.txDisableValue);
        *statusPtr = PDL_SFP_TX_DISABLE_E;
    }
    return PDL_OK;
}

/*$ END OF pdlSfpHwTxGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwTxSet
*
* DESCRIPTION:   set sfp operational status (tx enable/disable)
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwTxSet ( IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_SFP_TX_ENT status )
 *
 * @brief   Pdl sfp hardware transmit set
 *
 * @param   dev     the device number.
 * @param   port    The port.
 * @param   status  The status.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwTxSet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN PDL_SFP_TX_ENT               status
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (status == PDL_SFP_TX_ENABLE_E) {
        if (sfpPtr->publicInfo.txEnableInfo.isSupported == FALSE) {
            return PDL_NOT_SUPPORTED;
        }
        pdlStatus = prvPdlInterfaceHwSetValue(sfpPtr->publicInfo.txEnableInfo.interfaceType, sfpPtr->publicInfo.txEnableInfo.interfaceId, sfpPtr->publicInfo.values.txEnableValue);
    }
    else {
        if (sfpPtr->publicInfo.txDisableInfo.isSupported == FALSE) {
            return PDL_NOT_SUPPORTED;
        }
        pdlStatus = prvPdlInterfaceHwSetValue(sfpPtr->publicInfo.txDisableInfo.interfaceType, sfpPtr->publicInfo.txDisableInfo.interfaceId, sfpPtr->publicInfo.values.txDisableValue);
    }
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlSfpHwTxSet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwLossGet
*
* DESCRIPTION:   Get sfp loss status
*
* PARAMETERS:
*
*****************************************************************************/

/*$ END OF PdlSfpHwTxSet */

/**
 * @fn  PDL_STATUS pdlSfpHwLossGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_LOSS_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware loss get
 *
 * @param           dev         the device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwLossGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_LOSS_ENT          * statusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             data = 0;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void**) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sfpPtr->publicInfo.lossInfo.isSupported == FALSE) {
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = prvPdlInterfaceHwGetValue(sfpPtr->publicInfo.lossInfo.interfaceType, sfpPtr->publicInfo.lossInfo.interfaceId, &data);
    PDL_CHECK_STATUS(pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "[dev %d port %d] read %x expected %x", dev, logicalPort, data, sfpPtr->publicInfo.values.lossValue);
    if (data == sfpPtr->publicInfo.values.lossValue) {
        *statusPtr = PDL_SFP_LOSS_TRUE_E;
    }
    else {
        *statusPtr = PDL_SFP_LOSS_FALSE_E;
    }
    return PDL_OK;
}

/*$ END OF pdlSfpHwLossGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSfpHwPresentGet
*
* DESCRIPTION:   Get sfp present status
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSfpHwPresentGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_PRESENT_ENT * statusPtr )
 *
 * @brief   Pdl sfp hardware present get
 *
 * @param           dev         the device number.
 * @param           port        The port.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwPresentGet(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    OUT PDL_SFP_PRESENT_ENT       * statusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             data = 0;
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sfpPtr->publicInfo.presentInfo.isSupported == FALSE) {
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = prvPdlInterfaceHwGetValue(sfpPtr->publicInfo.presentInfo.interfaceType, sfpPtr->publicInfo.presentInfo.interfaceId, &data);
    PDL_CHECK_STATUS(pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "[dev %d port %d] read %x expected %x", dev, logicalPort, data, sfpPtr->publicInfo.values.presentValue);
    if (data == sfpPtr->publicInfo.values.presentValue) {
        *statusPtr = PDL_SFP_PRESENT_TRUE_E;
    }
    else {
        *statusPtr = PDL_SFP_PRESENT_FALSE_E;
    }
    return PDL_OK;
}
/*$ END OF pdlSfpHwPresentGet */

/**
 * @fn  PDL_STATUS pdlSfpHwEepromRead ( IN UINT_32 dev, IN UINT_32 logicalPort, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr )
 *
 * @brief   Pdl sfp hardware eeprom read
 *
 * @param           dev     the device number.
 * @param           port            The port.
 * @param [in]      i2cAddress      i2c address to read from MUST be 0x50 or 0x51
 * @param           offset          The offset.
 * @param           length          The length.
 * @param [in,out]  dataPtr         If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSfpHwEepromRead(
    IN  UINT_32                     dev,
    IN  UINT_32                     logicalPort,
    IN  UINT_8                      i2cAddress,
    IN  UINT_16                     offset,
    IN  UINT_32                     length,
    OUT void                      * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (dataPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "NULL statusPtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sfpPtr->publicInfo.eepromInfo.isSupported == FALSE) {
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = pdlI2CHwGetBufferWithOffset(sfpPtr->publicInfo.eepromInfo.interfaceId, i2cAddress, offset, length, dataPtr);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF pdlSfpHwEepromRead */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sfpPtr->publicInfo.eepromInfo.isSupported == FALSE) {
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = pdlI2CHwSetValueWithAddressOffsetMask(sfpPtr->publicInfo.eepromInfo.interfaceId, i2cAddress, offset, mask, data);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF pdlSfpHwEepromRead */





static const char * prvPdlsfpStristr (
  /*!     INPUTS:             */
  const char * str_PTR,
  const char * substr_PTR
  /*!     INPUTS / OUTPUTS:   */

  /*!     OUTPUTS:            */

)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32 str_len;
    UINT_32 substr_len;
    UINT_32 i;
    UINT_32 j;
    UINT_32 k;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/

    if ((str_PTR == NULL) || (substr_PTR == NULL)) {
        return NULL;
    }
    /* strlen on NULL-pointer generates an exception, so it must be done after
       the above verification. */
    str_len    = strlen(str_PTR);
    substr_len = strlen(substr_PTR);
    if (substr_len == 0) {
        return  str_PTR;    /* return str_PTR if substr_PTR empty */
    }
    if (str_len == 0) {
        return NULL;               /* return NULL if str_PTR empty */
    }
    i = 0;
    for(;;) {
        while((i < str_len) && (tolower(str_PTR[i]) != tolower(substr_PTR[0]))) {
             ++i;
        }
        if (i == str_len) {
            return NULL;
        }
        j = 0;
        k = i;
        while ((i < str_len) && (j < substr_len) && (tolower(str_PTR[i]) == tolower(substr_PTR[j]))) {
            ++i;
            ++j;
        }
        if (j == substr_len) {
            return str_PTR+k;
        }
        if (i == str_len) {
            return NULL;
        }
        i = k+1;
    }
}
/*$ END OF  prvPdlsfpStristr */


/**
 * @fn  BOOLEAN prvPdlsfpAccessA2IsAllowed ( IN char* vendor_pn )
 *
 * @brief   check if access to address a2 in the EPROM is allowed, for get the module type in some cases  
 *
 * @param   char*     vendor_pn_PTR: the vendor pn  
 *
 * @return  A BOOLEAN.
 */
static BOOLEAN prvPdlsfpAccessA2IsAllowed(
	IN char* 	              vendor_pn_PTR 
){
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_8                 i;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	for(i = 0; i < pdlsfpA2GbicsTableSize; i++)
	{
		if(prvPdlsfpStristr(vendor_pn_PTR, prvPdlSfpNoAccessToI2CAddressA2GbicsTable[i]) != NULL)
			return FALSE;
	}
	return TRUE;
}
/*$ END OF  prvPdlsfpAccessA2IsAllowed */


/**
 * @fn  BOOLEAN prvpPdlsfpIsOnlyOneBitOn ( IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_SFP_TX_ENT status )
 *
 * @brief in some bytes that read from the EPROM(3,6 and like...) every bit remark a unique module type.the func check if there is only one bit that is on(1) and others are off(0), to avoid mistakes in get the unique module type 
 *
 * @param   UINT_8  byte: byte in size 8 bits, that check if has only one bit on or not  
 *
 * @return  A BOOLEAN.
 */
static BOOLEAN prvpPdlsfpIsOnlyOneBitOn(UINT_8 byte){
	return (byte != 0) && (byte & (byte - 1)) == 0;
}
/*$ END OF  prvpPdlsfpIsOnlyOneBitOn */


/**
* @fn  PDL_STATUS prvPdlsfpGetComplianceCodeFromDb ( IN  const PDL_SFP_COMPLIANCE_ENT *mapPTR, IN  UINT_8 compliance_code_byte, IN  UINT_8 map_size, OUT  PDL_SFP_COMPLIANCE_ENT *compliance_code_PTR)
	*
	* @brief   get the module of the SFP by reading the compliance_code_byte
	*
	* @param        *module_DB_PTR		  A pointer to module DB that holds the module Enum values by order as in the byte.
	* @param         compliance_code_byte     The byte that read from the EPROM and holds the value of the module, usually it bytes 3 and 6.
	* @param	     byte_size_to_check   The size of the specific DB, depends on byte size we want to read (low part of byte 3 holds infinitband. our system don't support it so no need to read it)
	* @param		 bit_on_checker       use this parameter to check what bit in compliance_code_byte is on
	* @param [out]   compliance_code_PTR      A pointer to  the result of the compliance code.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetComplianceCodeFromDb(
	IN  const PDL_SFP_COMPLIANCE_CODE_ENT       *module_DB_PTR,
	IN        UINT_8                     compliance_code_byte,
	IN        UINT_8                     byte_size_to_check,
	IN        UINT_8					 bit_on_checker,
	OUT       PDL_SFP_COMPLIANCE_CODE_ENT       *compliance_code_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	UINT_32                       i;   
	
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (module_DB_PTR == NULL || compliance_code_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL module_DB_PTR or compliance_code_PTR");
		return PDL_BAD_PTR;
	}
	if(!prvpPdlsfpIsOnlyOneBitOn(compliance_code_byte))
		*compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E;

	else	
		for(i = 0; i < byte_size_to_check; i++, bit_on_checker = bit_on_checker<<1){
			if(compliance_code_byte & bit_on_checker){
				*compliance_code_PTR = module_DB_PTR[i];
				break;
			}
		}
    
	return PDL_OK;
}
/*$ END OF  prvPdlsfpGetModuleFromDb */


/**
	* @fn  PDL_STATUS prvPdlsfpGetOpModeFromDB ( IN PDL_SFP_COMPLIANCE_ENT compliance_code, OUT PDL_SFP_OPERATION_MODE_ENT *op_mode_PTR)
	*
	* @brief   check if the operation mode exist in the DB's(of byte 6 and byte 3) and return it if dose.
	*
	* @param         compliance_code		  module type Enum
	* @param [out]   op_mode_PTR          A pointer to the result of the op_mode type.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetOpModeFromDB(
	IN        PDL_SFP_COMPLIANCE_CODE_ENT    compliance_code,
	OUT       PDL_SFP_OPERATION_MODE_ENT     *op_mode_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	UINT_32                       i;   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (op_mode_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL op_mode_PTR ");
		return PDL_BAD_PTR;
	}		
		/* search module in sfp table, (that table contains copper type, but already checked it in get_calbe_type ) */
		for(i = 0; i < PDLP_SFP_BYTE_FULL_SIZE_CNS; i++){
			if(compliance_code == prvPdlSfpComplianceTypeDbByte6SfpARR [i]){
				*op_mode_PTR = PDL_SFP_OP_MODE_FIBER_SFP_E;				
				return PDL_OK;
			}
		}
		
		/* search module in sfp+ table */
		for(i = 0; i < PDLP_SFP_HALF_BYTE_SIZE_CNS; i++){
			if(compliance_code == prvPdlSfpComplianceTypeDbByte3SfpPluseARR [i]){
				*op_mode_PTR = PDL_SFP_OP_MODE_FIBER_SFP_PLUSE_E;				
				return PDL_OK;
			}
		}
		
		/* if don't find it in tables it unknown type */
		*op_mode_PTR = PDL_SFP_OP_MODE_UNKNOWN_E;
		return PDL_OK;
}
/*$ END OF  prvPdlsfpGetOpModeFromDB */

/**
	* @fn  PDL_STATUS prvPdlsfpGetSupportedExtendedComplianceCodeName ( IN PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT compliance_code, OUT char *compliance_name_PTR)
	*
	* @brief   get supported compliance code by name
	*
	* @param         compliance_code		  compliance code Enum
	* @param [out]   compliance_name_PTR_PTR  A pointer to the result of the compliance code name.
	*
	* @return  A PDL_STATUS, PDL_OK if valid compliance code and PDL_FAIL if not valid.
*/
static PDL_STATUS prvPdlsfpGetSupportedExtendedComplianceCodeName(
	IN  PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT    compliance_code,
	OUT char                        		   **compliance_name_PTR_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	char * tmpComplianceCode_PTR = NULL;
	PDL_STATUS pdlStatus = PDL_OK;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	switch (compliance_code) {
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_SR_E:
			tmpComplianceCode_PTR = "25G BASE-SR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_LR_E:
			tmpComplianceCode_PTR = "25G BASE-LR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_ER_E:
			tmpComplianceCode_PTR = "25G BASE-ER";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_RS_FEC_E:
			tmpComplianceCode_PTR = "25G BASE-CR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_FC_FEC_E:
			tmpComplianceCode_PTR = "25G BASE-CR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_NO_FEC_E:
			tmpComplianceCode_PTR = "25G BASE-CR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_WITH_SFI_E:
			tmpComplianceCode_PTR = "10G BASE-T SFI";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_SR_E:
			tmpComplianceCode_PTR = "10G BASE-T SR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_5GBASE_T_E:
			tmpComplianceCode_PTR = "5G BASE-T";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_2_5GBASE_T_E:
			tmpComplianceCode_PTR = "2.5G BASE-T";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_BR_E:
			tmpComplianceCode_PTR = "10G BASE-BR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_BR_E:
			tmpComplianceCode_PTR = "25G BASE-BR";
			break;
		case PDL_SFP_EXTENDED_COMPLIANCE_CODE_UNKNOWN_E:
		default:
			tmpComplianceCode_PTR = "UNKNOWN";
			PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"UNKNOWN extended compliance code 0x%x",compliance_code);
			pdlStatus = PDL_FAIL;
	}

	if (compliance_name_PTR_PTR) {
		*compliance_name_PTR_PTR = tmpComplianceCode_PTR;
	}
	return pdlStatus;
}



/**
	* @fn  PDL_STATUS prvPdlsfpGetComplianceSpecificVendor ( IN  PDL_SFP_PRV_EPROM_BUF_127_STC eprom_buf_PTR, IN OUT PDL_SFP_COMPLIANCE_CODE_ENT *compliance_code_PTR)
	*
	* @brief   get the module type of the gbic
	*
	* @param [IN]      eprom_buf_PTR      a struct that point to the 127 first bytes that read from EPROM.
	* @param [IN, OUT] compliance_code_PTR A pointer to the result of the compliance code
	*
	* @return  A PDL_STATUS.
*/

static PDL_STATUS prvPdlsfpGetComplianceSpecificVendor (
	IN PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	IN OUT PDL_SFP_COMPLIANCE_CODE_ENT *compliance_code_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_8						  vendor_id_byte;
	UINT_8                        extended_id_byte;
	PDL_SFP_COMPLIANCE_CODE_ENT	sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || compliance_code_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR or compliance_code_PTR");
		return PDL_BAD_PTR;
	}

	if (*compliance_code_PTR != PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"No need for extended ID check");
		return PDL_OK;
	}
	/* check if we have known special handling for gbic's vendor */
	vendor_id_byte = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_VENDOR_ID_INDEX_98_E];
	if (vendor_id_byte != PDLP_SFP_CISCO_VENDOR_ID_CNS) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"No Known Special handling vendor is %d",vendor_id_byte);
		return PDL_OK;
	}
	extended_id_byte = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_EXTENDED_ID_INDEX_96_E];
	switch (extended_id_byte) {
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1470_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1490_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1510_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1530_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1550_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1570_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1590_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_1610_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_CWDM_SFP_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100FX_GE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100FX_FE_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_FX_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100LX_FE_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_100BASE_LX_LX10_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_U_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_U_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1000BX_10_D_E  :
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_BX_10_D_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100BX_10_U_FE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100BX_10_D_FE_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_BASE_BX_10_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CAB_SFP_50CM_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100EX_FE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_100ZX_FE_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_EX_SMD_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_EX_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_GE_100EX_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_1002BX_U_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_1002BX_D_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_2BX_U_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_2BX_D_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T3_GE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T3_FE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CISCO_SFP_T1_FE_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_EPON_PX20_U_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CSFP_1G_ELPB_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_U_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_D_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX80_U_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX80_D_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_BX40_DA_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_FE_T_I_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_B_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_B_I_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_C_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_GPON_C_I_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_GLC_GE_DR_LX_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_G_FAST_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_T_X_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_SYNCEM_T_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_SYNCES_T_E	:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_OC_VCOP_E :
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_PDH_VCOP_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SC_2G_BX_D_E :
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_ONS_SI_DS1_TCOP_E:
			break;
		/* 10G gbic types */
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU1M_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU3M_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU5M_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_H10GB_CU7M_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_CX1_1S_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CX_1_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_LBX1_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_USR_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_LRM_SM_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_LRM_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_CR_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_BER_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B01_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B02_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_ELPBK_B03_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_FET_10G_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1271_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1291_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1311_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1331_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1351_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1411_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1431_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1451_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1471_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1491_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1511_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1531_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1551_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1571_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1591_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_8GFC_CWDM_1611_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CWDM_SFP_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10GB_ZR_I_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_61_41_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_60_61_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_59_79_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_58_98_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_58_17_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_57_36_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_56_55_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_55_75_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_54_94_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_54_13_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_53_33_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_52_52_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_51_72_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_50_92_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_50_12_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_49_32_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_48_51_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_47_72_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_46_92_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_46_12_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_45_32_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_44_53_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_43_73_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_42_94_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_42_14_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_41_35_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_40_56_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_39_77_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_38_98_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_38_19_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_37_40_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_36_61_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_35_82_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_35_04_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_34_25_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_33_47_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_32_68_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_31_90_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_31_12_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_DWDM_SFP10G_30_33_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_DWDM_SFP_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BXD_I_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXD_I_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BXU_I_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_BXU_I_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BX40D_I_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_10G_BX40U_I_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1470_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1490_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1510_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1530_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1550_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1570_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1590_E:
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_CWDM_SFP10G_1610_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CWDM_SFP_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1G_SX_E:
			sfp_compliance_code = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_SX_E;
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_SFP_1G_LH_E:
			break;
		case PDL_SFP_CISCO_EXTENDED_COMPLIANCE_CODE_NEXT_PAGE_E:
			/* once we receive definition for next page we can complete this */
		default:
			PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"Unknown extended_id_byte 0x%x",extended_id_byte);
	}

	*compliance_code_PTR = sfp_compliance_code;

	return PDL_OK;
}
/*$ END OF prvPdlsfpGetComplianceSpecificVendor */

/**
	* @fn  PDL_STATUS prvPdlsfpGetComplianceCodeByLinkCapabilities ( IN UINT_8 compliance_link_capabilities, OUT PDL_SFP_COMPLIANCE_CODE_ENT *compliance_code_PTR)
	*
	* @brief        get the module type of the gbic by checking it's link capabilities.
	* @param        compliance_link_capabilities - link capabilities can be read data from EPROM (address a0, byte 14, units in kilometer)  
	* @param [out]  compliance_code_PTR - A pointer to the result of the module type.
	* @return  A PDL_STATUS.
*/

static void prvPdlsfpGetComplianceCodeByLinkCapabilities(
    IN  UINT_8 compliance_link_capabilities,
    OUT PDL_SFP_COMPLIANCE_CODE_ENT *compliance_code_PTR
)
{
    if (compliance_link_capabilities <= 10){
        *compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_LX_E;
    }
    if ((compliance_link_capabilities > 10) && (compliance_link_capabilities <= 40)){
        *compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_EX_E;
    }
    else if ((compliance_link_capabilities > 40) && (compliance_link_capabilities <= 80)){
        *compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_ZX_E;
    } 
}

/**
	* @fn  PDL_STATUS prvPdlsfpGetModuleType ( IN PDL_SFP_PRV_DB_STC *sfp_PTR, IN  PDL_SFP_PRV_EPROM_BUF_127_STC eprom_buf_PTR, IN  PDL_SFP_PORT_TYPE_ENT port_type, OUT  PDL_SFP_COMPLIANCE_ENT *compliance_code_PTR)
	*
	* @brief   get the module type of the gbic
	*
	* @param         sfp_PTR      		  with this parameter can be read data from EPROM(here we read from address a2)  
	* @param         eprom_buf_PTR        a struct that point to the 127 first bytes that read from EPROM.
	* @param         port_type		      port type Enum 
	* @param [out]   compliance_code      A pointer to the result of the module type.
	*
	* @return  A PDL_STATUS.
*/

static PDL_STATUS prvPdlsfpGetComplianceCode (
	IN  PDL_SFP_PRV_DB_STC 	        *sfp_PTR,
	IN  PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	IN	PDL_SFP_PORT_TYPE_ENT       port_type,
	OUT PDL_SFP_COMPLIANCE_CODE_ENT         *compliance_code_PTR, /*  unknown, not connected, sfp , sfp+, dac */
	OUT PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT *extended_compliance_code_PTR
	
)
{	
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	PDL_STATUS                    pdlStatus;
    UINT_32                       i;
	UINT_8						  connector_type_byte;
	UINT_8                        compliance_code_byte;
    UINT_8                        compliance_link_capabilities;
	UINT_8						  compliance_code_byte_pluse;
	UINT_8						  compliance_code_byte_extended;
	UINT_8                        module_type_byte_131_a2_10G = 0;
	char                          vendor_pn[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS+1];
    char                          vendor_pn_special[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS+1], * vendor_pn_special_PTR;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || compliance_code_PTR == NULL||sfp_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR or compliance_code_PTR or sfp_PTR address");
		return PDL_BAD_PTR;
	}
   

	*compliance_code_PTR			= PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E;
	connector_type_byte         = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_CONNECTOR_TYPE_INDEX_2_E];
	compliance_code_byte        = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_MODULE_TYPE_INDEX_6_E];
	compliance_code_byte_pluse	= eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_PLUSE_MODULE_TYPE_INDEX_3_E];
	compliance_code_byte_extended = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_EXTENDED_COMPLIANCE_CODE_INDEX_36_E];
    compliance_link_capabilities  = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_LINK_CAPABILITIES_14_E];
	
	/* dac */
	if(connector_type_byte == PDL_SFP_CONNECTOR_TYPE_COPPER_PIGTAIL_E)
		*compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CX_1_E;
	/* fiber module types */
	else {		
	    if (connector_type_byte == PDL_SFP_CONNECTOR_TYPE_RJ45_E) {
            if((port_type == PDL_SFP_PORT_TYPE_10000_E) ||
                (port_type == PDL_SFP_PORT_TYPE_1000_10000_E)) {
                *compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_ETHERNET_10000BASE_T_E;
                return PDL_OK;
            }
        }

        if(compliance_code_byte != 0){
            pdlStatus = prvPdlsfpGetComplianceCodeFromDb(prvPdlSfpComplianceTypeDbByte6SfpARR , compliance_code_byte, PDLP_SFP_BYTE_FULL_SIZE_CNS, PDLP_SFP_CHECK_ALL_BYTE_CNS,compliance_code_PTR);
            if (*compliance_code_PTR == PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_LX_E){
                prvPdlsfpGetComplianceCodeByLinkCapabilities(compliance_link_capabilities, compliance_code_PTR);
            }
        }
        else if((compliance_code_byte_pluse & 0xf0) != 0){
            pdlStatus = prvPdlsfpGetComplianceCodeFromDb(prvPdlSfpComplianceTypeDbByte3SfpPluseARR , compliance_code_byte_pluse, PDLP_SFP_HALF_BYTE_SIZE_CNS, PDLP_SFP_CHECK_ONLY_HIGHER_BYTE_CNS, compliance_code_PTR);
        }
        /* check if allow access to a2, maybe data of 10G module is there in 131 byte */
        else if(((port_type == PDL_SFP_PORT_TYPE_10000_E) ||
            (port_type == PDL_SFP_PORT_TYPE_1000_10000_E))) {
            /* if need to read data from a2 address, need to valid the vendor has access by vendor_pn */
            memcpy( vendor_pn, &eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_VENDOR_PN_INDEX_40_E], PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS);
            vendor_pn[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS] = '\0';

            if(prvPdlsfpAccessA2IsAllowed(vendor_pn)) {
                pdlStatus = pdlI2CHwGetBufferWithOffset (sfp_PTR->publicInfo.eepromInfo.interfaceId,
                    PDLP_SFP_I2C_BASE_ADDRESS_A2_CNS,
                    PDL_SFP_I2C_BYTES_10G_IN_A2_INDEX_131_E,
                    PDLP_SFP_I2C_ONE_BYTE_LENGTH_1_CNS,
                    &module_type_byte_131_a2_10G);
                /* if no supported in a2 address */
                if(pdlStatus != PDL_OK){
                    *compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E;
                    return pdlStatus;
                }

                pdlStatus = prvPdlsfpGetComplianceCodeFromDb(prvPdlSfpComplianceTypeDbByte131SfpPluseARR , module_type_byte_131_a2_10G, PDLP_SFP_BYTE_FULL_SIZE_CNS, PDLP_SFP_CHECK_ALL_BYTE_CNS, compliance_code_PTR);
                PDL_CHECK_STATUS (pdlStatus);
            }
        }
        /* special vendor_pn */
        if (*compliance_code_PTR == PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E) {
        	if (prvPdlsfpGetSupportedExtendedComplianceCodeName(compliance_code_byte_extended, NULL) == PDL_OK) {
        		*compliance_code_PTR = PDL_SFP_COMPLIANCE_CODE_EXTENDED_E;
        		if (extended_compliance_code_PTR) {
					*extended_compliance_code_PTR = compliance_code_byte_extended;
				}
        	} else {

        		/* if port type and compliance code are unknown then check if gbic match CISCO extended ID table */
        		prvPdlsfpGetComplianceSpecificVendor(eprom_buf_PTR, compliance_code_PTR);
        		if (*compliance_code_PTR == PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E) {
					/* if need to read data from a2 address, need to valid the vendor has access by vendor_pn */
					memcpy( vendor_pn, &eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_VENDOR_PN_INDEX_40_E], PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS);
					vendor_pn[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS] = '\0';
					if(prvPdlsfpAccessA2IsAllowed(vendor_pn))
					{
						pdlStatus = pdlI2CHwGetBufferWithOffset (sfp_PTR->publicInfo.eepromInfo.interfaceId,
																 PDLP_SFP_I2C_BASE_ADDRESS_A2_CNS,
																 PDLP_SFP_I2C_SPECIAL_CISCO_TWINAX_PN_CNS,
																 PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS,
																 (UINT_8*)vendor_pn_special);
						PDL_CHECK_STATUS (pdlStatus);
						vendor_pn_special_PTR = vendor_pn_special;
						for (i = 0; i < PDLP_SFP_NUM_OF_SPECIAL_VENDOR_PN_TABLE_CNS; i++){
							if (prvPdlsfpStristr(vendor_pn_special_PTR, prvPdlSfpSpecialVendorPnARR[i].vendor_pn) != NULL){
								*compliance_code_PTR = prvPdlSfpSpecialVendorPnARR[i].sfp_compliance_code;
								break;
							}
						}
					}
        		}
        	}
        }
        if (*compliance_code_PTR == PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E){
            return PDL_NO_RESOURCE;
        }
	}

	return PDL_OK;
}
/*$ END OF prvPdlsfpGetModuleType */


/**
	* @fn  PDL_STATUS prvPdlsfpGetOpMode ( IN PDL_SFP_COMPLIANCE_ENT compliance_code, OUT  PDL_SFP_OPERATION_MODE_ENT *op_mode_PTR)
	*
	* @brief   get the operation mode of the gbic
	*
	* @param         compliance_code		  compliance code Enum
	* @param         extended_compliance_code extended compliance code Enum
	* @param [out]   op_mode_PTR          A pointer to the result of the operation mode type.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetOpMode (
	IN  PDL_SFP_COMPLIANCE_CODE_ENT   compliance_code,
	IN  PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT   extended_compliance_code,
	OUT PDL_SFP_OPERATION_MODE_ENT   *op_mode_PTR /* unknown, not connected, sfp , sfp+, dac */
)
{	
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	
	if (op_mode_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL op_mode_PTR address");
		return PDL_BAD_PTR;
	}

	switch(compliance_code)
	{

		case PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E:
			*op_mode_PTR = PDL_SFP_OP_MODE_UNKNOWN_E;
			break;

		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CX_1_E:
			*op_mode_PTR = PDL_SFP_OP_MODE_DAC_E;
			break;

		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_10000BASE_T_E:
		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_T_E:
			*op_mode_PTR = PDL_SFP_OP_MODE_COPPER_SFP_E;
			break;

		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_1000BASE_CWDM_SFP_E:
			*op_mode_PTR = PDL_SFP_OP_MODE_FIBER_SFP_E;
			break;

		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_CWDM_SFP_E:
		case PDL_SFP_COMPLIANCE_CODE_ETHERNET_10GBASE_DWDM_SFP_E:
			*op_mode_PTR = PDL_SFP_OP_MODE_FIBER_SFP_PLUSE_E;
			break;
		default:
			switch (extended_compliance_code) {
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_RS_FEC_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_FC_FEC_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_CR_NO_FEC_E:
					*op_mode_PTR = PDL_SFP_OP_MODE_DAC_E;
					break;
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_WITH_SFI_E :
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_T_SR_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_5GBASE_T_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_2_5GBASE_T_E:
					*op_mode_PTR = PDL_SFP_OP_MODE_COPPER_SFP_E;
					break;
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_SR_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_LR_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_ER_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_25GBASE_BR_E:
				case PDL_SFP_EXTENDED_COMPLIANCE_CODE_10GBASE_BR_E:
					*op_mode_PTR = PDL_SFP_OP_MODE_FIBER_SFP_PLUSE_E;
					break;
				default:
					pdlStatus = prvPdlsfpGetOpModeFromDB(compliance_code, op_mode_PTR);
			}
	}

	return pdlStatus;
}
/*$ END OF prvPdlsfpGetOpMode */


/*
	* @fn  PDL_STATUS prvPdlsfpGetCableLength ( IN PDL_SFP_OPERATION_MODE_ENT op_mode, OUT  UINT_8 *length_PTR)
	*
	* @brief   get the length of the cable
	*
	* @param         eprom_buf_PTR	  A pointer to a0, buffer that read from the EPROM
	* @param         op_mode		  unknown, not connected, sfp , sfp+, dac
	* @param [out]   length_PTR       A pointer to the result of the length. if not supported value is 0.
	*
	* note: currently working for DAC cable, need to add support for fiber
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetCableLength (
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	IN  PDL_SFP_OPERATION_MODE_ENT   op_mode,
	OUT UINT_8   					*length_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS                     pdlStatus = PDL_OK;
	UINT_8 cable_length_byte;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (length_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL length_PTR");
		return PDL_BAD_PTR;
	}

	cable_length_byte             = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_CABLE_LENGTH_INDEX_18_E];
	if (op_mode == PDL_SFP_OP_MODE_DAC_E) {
		/* this value represents actual length */
		*length_PTR = cable_length_byte;
	}
	return pdlStatus;
}

/*
	* @fn  PDL_STATUS prvPdlsfpGetCableTechnology ( OUT  PDL_CONNECTOR_TECHNOLOGY_ENT *cable_technology_PTR)
	*
	* @brief   get the length of the cable
	*
	* @param         eprom_buf_PTR	  A pointer to a0, buffer that read from the EPROM
	* @param [out]   cable_technology_PTR   A pointer to the result of the the technology type. if not recognized return unknown.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetCableTechnology (
	IN  PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	OUT PDL_CONNECTOR_TECHNOLOGY_ENT			*cable_technology_PTR
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS                     pdlStatus = PDL_OK;
	UINT_8 cable_tech_byte;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/

	if (cable_technology_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL cable_technology_PTR");
		return PDL_BAD_PTR;
	}

	cable_tech_byte             = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_PLUSE_CABLE_TECH_INDEX_8_E];
	if (cable_tech_byte & (1 << PDL_CONNECTOR_TECHNOLOGY_PASSIVE_E)) {
		*cable_technology_PTR = PDL_CONNECTOR_TECHNOLOGY_PASSIVE_E;
	} else if (cable_tech_byte & (1 << PDL_CONNECTOR_TECHNOLOGY_ACTIVE_E)) {
		*cable_technology_PTR = PDL_CONNECTOR_TECHNOLOGY_ACTIVE_E;
	} else {
		*cable_technology_PTR = PDL_CONNECTOR_TECHNOLOGY_PASSIVE_E;
	}
	return pdlStatus;
}

/**
	* @fn  PDL_STATUS prvPdlsfpGetVendorName ( IN PDL_SFP_PRV_EPROM_BUF_127_STC eprom_buf_PTR, OUT  char *vendor_name_PTR)
	*
	* @brief   get the gbic vendor name
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param [out]   vendor_name_PTR      A pointer to the result of the vendor name.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetVendorName ( 
	IN  PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	OUT char                        *vendor_name_PTR 
)
{	
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	
	if (eprom_buf_PTR == NULL || vendor_name_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR or vendor_name_PTR address");
		return PDL_BAD_PTR;
	}

	memcpy(vendor_name_PTR, &(eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_VENDOR_NAME_INDEX_20_E]),PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS);
	vendor_name_PTR[PDL_SFP_I2C_VENDOR_NAME_LENGTH_16_CNS] = '\0';
	
	
	return pdlStatus;
}
/*$ END OF prvPdlsfpGetVendorName */


/**
	* @fn  PDL_STATUS prvPdlsfpGetPortType ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, IN char *vendor_name_PTR, OUT  char *vendor_name_PTR)
	*
	* @brief   get the gbic port type
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param         vendor_name_PTR	  the vendor name
	* @param [out]   port_type_PTR        A pointer to the result of the port type.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetPortType ( 
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,
	IN   char                        *vendor_name_PTR,    
	OUT  PDL_SFP_PORT_TYPE_ENT	     *port_type_PTR
)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  
	UINT_8   					   nominat_bit_rat_byte, high_signaling_bit_rate_byte;
	UINT_8                         pasiv_and_active_compliance_byte;  
	UINT_8                         sfp_pluse_tec_byte; 
	UINT_8						   compliance_code_byte;
	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || vendor_name_PTR == NULL|| port_type_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR, vendor_name  or port_type address");
		return PDL_BAD_PTR;
	}

	
	nominat_bit_rat_byte             = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_NOMINAL_BIT_RAT_INDEX_12_E];
	pasiv_and_active_compliance_byte = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_ACTIVE_AND_PASIVE_CABLES_INDEX_60_E];
	sfp_pluse_tec_byte				 = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_PLUSE_CABLE_TECH_INDEX_8_E];	
	compliance_code_byte			 = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SFP_MODULE_TYPE_INDEX_6_E];

	high_signaling_bit_rate_byte  = eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_NOMINAL_HIGH_SPEED_MAX_BIT_RAT_INDEX_66_E];

	/* copper */
	if(compliance_code_byte == PDL_SFP_MSA_TRANSCEIVER_COMPLIANCE_CODES_1000BAS_LX_CNS)
		*port_type_PTR = PDL_SFP_PORT_TYPE_100_1000_E;

	/* 100 */
	else if((nominat_bit_rat_byte < PDL_SFP_NOMINAL_BIT_RATE_1000_MBITS_TO_SEC_E && nominat_bit_rat_byte >= PDL_SFP_NOMINAL_BIT_RATE_100_MBITS_TO_SEC_E) || PDLP_SFP_sfp_100_vendor_BAYCOM_MAC(compliance_code_byte,vendor_name_PTR))
		*port_type_PTR = PDL_SFP_PORT_TYPE_100_E;

	/* 1000 */
	else if(nominat_bit_rat_byte < PDL_SFP_NOMINAL_BIT_RATE_10000_MBITS_TO_SEC_E && nominat_bit_rat_byte >= PDL_SFP_NOMINAL_BIT_RATE_1000_MBITS_TO_SEC_E)
		*port_type_PTR = PDL_SFP_PORT_TYPE_1000_E;

	/* 1000-10000 */
	else if(((nominat_bit_rat_byte >= PDL_SFP_NOMINAL_BIT_RATE_10000_MBITS_TO_SEC_E) && (nominat_bit_rat_byte != 0xFF)) || ((nominat_bit_rat_byte == 0) && (pasiv_and_active_compliance_byte & 1) && (sfp_pluse_tec_byte & PDLP_SFP_IS_PASSIVE_CABLE_IN_BYTE_8)))
		*port_type_PTR = PDL_SFP_PORT_TYPE_1000_10000_E;		
	else if (nominat_bit_rat_byte == 0xFF && high_signaling_bit_rate_byte) {
		/* is given in this field in units of 250 MBd */
		if ((high_signaling_bit_rate_byte * 250) >= PDL_SFP_NOMINAL_BIT_RATE_25000_MBITS_TO_SEC_E) {
			*port_type_PTR = PDL_SFP_PORT_TYPE_25G_E;
		} else {
			*port_type_PTR = PDL_SFP_PORT_TYPE_1000_10000_E;
		}
	} else
		*port_type_PTR = PDL_SFP_PORT_TYPE_UNKNOWN_E;
		

	return PDL_OK;
}
/*$ END OF prvPdlsfpGetPortType */


/**
	* @fn  PDL_STATUS prvPdlsfpGetProductNumber ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char *product_number_PTR)
	*
	* @brief   get the gbic product number
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param [out]   port_type_PTR        A pointer to the result of the product number.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetProductNumber ( 
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,   
	OUT  char	                     *product_number_PTR
	)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || product_number_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR, product_number_PTR address");
		return PDL_BAD_PTR;
	}

	memcpy(product_number_PTR, &(eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_PRODUCT_NUMBER_INDEX_0x68_E]), PDL_SFP_GBIC_PRODUCT_NUMBER_SIZE_CNS);
	product_number_PTR[PDL_SFP_GBIC_PRODUCT_NUMBER_SIZE_CNS] = '\0';
	
	return pdlStatus;
}
/*$ END OF prvPdlsfpGetProductNumber */


/**
	* @fn  PDL_STATUS prvPdlsfpGetProductNumber ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char *prat_number_PTR)
	*
	* @brief   get the gbic part number
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param [out]   prat_number_PTR      A pointer to the result of the part number.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetPartNumber ( 
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,   
	OUT  char	                     *prat_number_PTR
	)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || prat_number_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR, prat_number address");
		return PDL_BAD_PTR;
	}

	memcpy(prat_number_PTR, &(eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_PART_NUMBER_INDEX_0x6F_E]), PDL_SFP_GBIC_PART_NUMBER_SIZE_CNS);
	prat_number_PTR[PDL_SFP_GBIC_PART_NUMBER_SIZE_CNS] = '\0';

	return pdlStatus;
}
/*$ END OF prvPdlsfpGetPartNumber */


/**
	* @fn  PDL_STATUS prvPdlsfpGetSerialNumber ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char *serial_number_PTR)
	*
	* @brief   get the gbic serial number
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param [out]   serial_number_PTR    A pointer to the result of the serial number.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetSerialNumber ( 
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,   
	OUT  char	                     *serial_number_PTR
)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || serial_number_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR, serial_number_PTR address");
		return PDL_BAD_PTR;
	}

	memcpy(serial_number_PTR, &(eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_SERIAL_NUMBER_INDEX_68_E]), PDL_SFP_GBIC_SERIAL_NUMBER_SIZE_CNS);
	serial_number_PTR[PDL_SFP_GBIC_SERIAL_NUMBER_SIZE_CNS] = '\0';

	return pdlStatus;
}
/*$ END OF prvPdlsfpGetSerialNumber */



/**
	* @fn  PDL_STATUS prvPdlsfpGetVendorPn ( IN PDL_SFP_PRV_EPROM_BUF_127_STC *eprom_buf_PTR, OUT  char *vendor_pn_PTR)
	*
	* @brief   get the gbic vendor pn number
	*
	* @param         eprom_buf_PTR		  A pointer to a0, buffer that read from the EPROM
	* @param [out]   vendor_pn_PTR        A pointer to the result of the vendor pn number.
	*
	* @return  A PDL_STATUS.
*/
static PDL_STATUS prvPdlsfpGetVendorPn ( 
	IN   PDL_SFP_PRV_EPROM_BUF_127_STC   *eprom_buf_PTR,   
	OUT  char	                     *vendor_pn_PTR
	)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  
	PDL_STATUS                     pdlStatus = PDL_OK;
	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (eprom_buf_PTR == NULL || vendor_pn_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL eprom_buf_PTR, vendor_pn address");
		return PDL_BAD_PTR;
	}

	memcpy(vendor_pn_PTR, &(eprom_buf_PTR->buf[PDL_SFP_I2C_BYTES_VENDOR_PN_INDEX_40_E]), PDL_SFP_GBIC_VENDOR_PN_SIZE_CNS);
	vendor_pn_PTR[PDL_SFP_GBIC_VENDOR_PN_SIZE_CNS] = '\0';

	return pdlStatus;
}
/*$ END OF prvPdlsfpGetVendorPn */


/**
	* @fn  PDL_STATUS pdlsfpGetTypeName ( IN PDL_SFP_COMPLIANCE_CODE_ENT compliance_code, PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT extended_compliance_code, OUT  char **type_name_PTR)
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
	IN   PDL_SFP_COMPLIANCE_CODE_ENT   compliance_code,
	IN   PDL_SFP_EXTENDED_COMPLIANCE_CODE_ENT   extended_compliance_code,
	OUT  char	                     **type_name_PTR
	)
{	
	/* ***************************************************************************/
	/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
	/* ***************************************************************************/  

	/* ***************************************************************************/
	/*                      F U N C T I O N   L O G I C                          */
	/* ***************************************************************************/

	if (type_name_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL type_name address");
		return PDL_BAD_PTR;
	}

	/* validate compliance_code not contain garbage values */
	if(compliance_code >= PDL_SFP_COMPLIANCE_CODE_LAST_E)
		*type_name_PTR = (char*) prvPdlSfpComplianceCodeStrARR[PDLP_SFP_UNKNOWN_MODULE_TYPE_NAME_INDEX];
	else {
		if (!((compliance_code == PDL_SFP_COMPLIANCE_CODE_EXTENDED_E) && (prvPdlsfpGetSupportedExtendedComplianceCodeName(extended_compliance_code, type_name_PTR) == PDL_OK)))
			*type_name_PTR = (char*) prvPdlSfpComplianceCodeStrARR[compliance_code];
	}

	return PDL_OK;

}
/*$ END OF pdlsfpGetTypeName */



/**
	* @fn  PDL_STATUS pdlsfpGetGbicData ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT  PDL_sfp_gbic_data_STC *gbic_data_PTR)
	*
	* @brief   fill the PDL_sfp_gbic_data_STC fields with the relevant data 
	*
	* @param         dev				  the dev number of the port that needed to be checked 
	* @param         logicalPort		  the logical port number of the port needed to be checked
	* @param [out]   gbic_data_PTR        A pointer to the result of the gbic data structure 
	*
	* @return  A PDL_STATUS.
*/
PDL_STATUS pdlsfpGetGbicData (
	IN  UINT_32							 dev,
	IN  UINT_32							 logicalPort,
	OUT  PDL_sfp_gbic_data_STC		    *gbic_data_PTR  
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/  
	PDL_STATUS                          pdlStatus;
	PDL_SFP_PRV_DB_STC 	               *sfpPtr;
	PDL_SFP_PRESENT_ENT                 conection_status;
	PDL_SFP_PRV_KEY_STC                 sfpKey;
	PDL_SFP_PRV_EPROM_BUF_127_STC       eprom_buf;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    
	if (gbic_data_PTR == NULL) {
		PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__,"NULL gbic_data_PTR, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort,dev);
		return PDL_BAD_PTR;
	}

	sfpKey.dev = dev;
	sfpKey.logicalPort = logicalPort;
	pdlStatus = prvPdlibDbFind (pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
	PDL_CHECK_STATUS (pdlStatus);
	if (sfpPtr->publicInfo.presentInfo.isSupported == FALSE) {
		return PDL_NOT_SUPPORTED;
	}

	/* check connector connection */
    pdlStatus = pdlSfpHwPresentGet(dev, logicalPort, &conection_status);
    PDL_CHECK_STATUS (pdlStatus);
    if (conection_status == PDL_SFP_PRESENT_FALSE_E) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "[dev %d port %d] not present", dev, logicalPort);
        return PDL_FAIL;
    }

#ifdef LINUX_HW
    /* Must put a delay here before trying access Mux and read from GBIC */
    usleep(20000);
#endif
    memset(&eprom_buf,0,sizeof(eprom_buf));
    /*  to detect our relevant data we need to use bytes- 2, 6, 8, 12, 20-36. to read from eeprom takes expansive time */
	/*  so, in order to save it, we read at once from byte 0 to 13 and use the important bytes */
	pdlStatus = pdlI2CHwGetBufferWithOffset (sfpPtr->publicInfo.eepromInfo.interfaceId,
		PDLP_SFP_I2C_BASE_ADDRESS_A0_CNS,          
		PDLP_SFP_I2C_START_OFFSET_0_CNS,  
		PDLP_SFP_I2C_OFFSET_SFP_IMPORTANT_BYTES_127_CNS,
		eprom_buf.buf);
	PDL_CHECK_STATUS (pdlStatus);
	
	/* system don't support QSFP */
	if(eprom_buf.buf[PDL_SFP_I2C_BYTES_IDENTIFIER_INDEX_0_E] > 0xB){
		gbic_data_PTR->sfp_port_type      = PDL_SFP_PORT_TYPE_UNKNOWN_E;
		gbic_data_PTR->sfp_operation_mode = PDL_SFP_OP_MODE_UNKNOWN_E;
		gbic_data_PTR->sfp_compliance_code    = PDL_SFP_COMPLIANCE_CODE_UNKNOWN_E;
		return 	PDL_OK;
	}

	/* get gbic data */
	pdlStatus = prvPdlsfpGetVendorName(&eprom_buf, gbic_data_PTR->vendor);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetPortType(&eprom_buf, gbic_data_PTR->vendor, &gbic_data_PTR->sfp_port_type);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetComplianceCode(sfpPtr, &eprom_buf, (UINT_8)gbic_data_PTR->sfp_port_type, &gbic_data_PTR->sfp_compliance_code, &gbic_data_PTR->sfp_extended_compliance_code);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetOpMode(gbic_data_PTR->sfp_compliance_code, gbic_data_PTR->sfp_extended_compliance_code, &gbic_data_PTR->sfp_operation_mode);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetProductNumber(&eprom_buf, gbic_data_PTR->product_number);
	PDL_CHECK_STATUS (pdlStatus);
	
	pdlStatus = prvPdlsfpGetPartNumber(&eprom_buf, gbic_data_PTR->part_number);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetSerialNumber(&eprom_buf, gbic_data_PTR->serial_number);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = pdlsfpGetTypeName(gbic_data_PTR->sfp_compliance_code, gbic_data_PTR->sfp_extended_compliance_code, &(gbic_data_PTR->sfp_type_name));
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetVendorPn(&eprom_buf, gbic_data_PTR->vendor_pn);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetCableLength(&eprom_buf, gbic_data_PTR->sfp_operation_mode, &gbic_data_PTR->cable_length);
	PDL_CHECK_STATUS (pdlStatus);

	pdlStatus = prvPdlsfpGetCableTechnology(&eprom_buf, &gbic_data_PTR->cable_technology);
	PDL_CHECK_STATUS (pdlStatus);

	return 	PDL_OK;
}
/*$ END OF pdlsfpGetGbicData */


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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlSfpDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlSfpDebugSet */


/**
 * @fn  PDL_STATUS pdlSfpDebugInterfaceGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get sfp access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  dev             - dev number.
 * @param [in]  port            - port number.
 * @param [out] interfacePtr    - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlSfpDebugInterfaceGet(
    IN  UINT_32                   dev,
    IN  UINT_32                   logicalPort,
    OUT PDL_SFP_INTERFACE_STC   * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "NULL interfacePtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    if (pdlStatus == PDL_OK) {
        memcpy(interfacePtr, &sfpPtr->publicInfo, sizeof(sfpPtr->publicInfo));
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.lossValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.presentValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.txEnableValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceId);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.txDisableValue);
    }
    else {
        return pdlStatus;
    }

    return PDL_OK;
}

/*$ END OF pdlBtnDebugInterfaceGet */

/**
 * @fn  PDL_STATUS pdlSfpDebugInterfaceSet ( IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_SFP_INTERFACE_STC * interfacePtr );
 *
 * @brief   Set sfp access information (used to change goodValue of present / loss / tx enable
 *          interfaces
 *          @note
 *          Used to change data retrieved from XML, can later be saved
 *
 * @param [in]  dev           - dev number.
 * @param [in]  port          - port number.
 * @param [in]  interfacePtr  - sfp interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSfpDebugInterfaceSet(
    IN  UINT_32                   dev,
    IN  UINT_32                   logicalPort,
    IN PDL_SFP_INTERFACE_STC    * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SFP_PRV_DB_STC                * sfpPtr;
    PDL_SFP_PRV_KEY_STC                 sfpKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSfpDebugFlag)(__FUNCTION__, __LINE__, "NULL interfacePtr, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->eepromInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->lossInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.lossValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->presentInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.presentValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txEnableInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.txEnableValue);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->txDisableInfo.interfaceId);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, interfacePtr->values.txDisableValue);
    sfpKey.dev = dev;
    sfpKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlSfpDb, (void*) &sfpKey, (void*) &sfpPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(&sfpPtr->publicInfo, interfacePtr, sizeof(sfpPtr->publicInfo));
    return PDL_OK;
}
/*$ END OF pdlSfpDebugInterfaceSet */

/**
 * @fn  PDL_STATUS pdlSfpInit ( IN void )
 *
 * @brief   Pdl sfp initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlSfpInit(
    IN  void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus, pdlStatus2;
    PRV_PDLIB_DB_ATTRIBUTES_STC                               dbAttributes;

    PDL_NETWORK_PORTS_PARAMS_STC                            portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC                  *frontPanelGroupPtr;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;

    PDL_SFP_PRV_KEY_STC                                     sfpEntryKey;
    PDL_SFP_PRV_DB_STC                                      sfpEntry, *sfpEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_SFP_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_SFP_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &pdlSfpDb);
    PDL_CHECK_STATUS(pdlStatus);

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    /* create dev/port mapping db */
    pdlStatus = prvPdlibDbGetFirst(portsParams.frontPanelGroupList_PTR, (void **)&frontPanelGroupPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus2 = prvPdlibDbGetFirst(frontPanelGroupPtr->portList_PTR, (void **)&frontPanelPortEntryPtr);
        while (pdlStatus2 == PDL_OK) {
            memset(&sfpEntry, 0, sizeof(sfpEntry));

            sfpEntry.publicInfo.eepromInfo.isSupported = pdlIsFieldHasValue(frontPanelPortEntryPtr->gbicGroup.memorycontrol_mask);
            if (sfpEntry.publicInfo.eepromInfo.isSupported) {
                pdlStatus2 = prvPdlInterfaceI2CReadWriteGroupRegister(&frontPanelPortEntryPtr->gbicGroup.memorycontrol.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                      &sfpEntry.publicInfo.eepromInfo.interfaceId);
                PDL_CHECK_STATUS(pdlStatus2);
                sfpEntry.publicInfo.eepromInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
            }

            sfpEntry.publicInfo.lossInfo.isSupported = pdlIsFieldHasValue(frontPanelPortEntryPtr->gbicGroup.laserislossinfo_mask);
            if (sfpEntry.publicInfo.lossInfo.isSupported) {
                pdlStatus2 = prvPdlInterfaceReadGroupRegister(&frontPanelPortEntryPtr->gbicGroup.laserislossinfo.readInterfaceGroupType, &sfpEntry.publicInfo.lossInfo.interfaceId);
                PDL_CHECK_STATUS(pdlStatus2);
                if (frontPanelPortEntryPtr->gbicGroup.laserislossinfo.readInterfaceGroupType.readInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_GPIO_E) {
                    sfpEntry.publicInfo.values.lossValue = frontPanelPortEntryPtr->gbicGroup.laserislossinfo.readInterfaceGroupType.readGpioInterface.gpioReadValue;
                    sfpEntry.publicInfo.lossInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;

                }
                else if (frontPanelPortEntryPtr->gbicGroup.laserislossinfo.readInterfaceGroupType.readInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_I2C_E) {
                    sfpEntry.publicInfo.values.lossValue = frontPanelPortEntryPtr->gbicGroup.laserislossinfo.readInterfaceGroupType.readI2CInterface.i2CReadValue;
                    sfpEntry.publicInfo.lossInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }

            sfpEntry.publicInfo.presentInfo.isSupported = pdlIsFieldHasValue(frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo_mask);
            if (sfpEntry.publicInfo.presentInfo.isSupported) {
                pdlStatus2 = prvPdlInterfaceReadGroupRegister(&frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo.readInterfaceGroupType, &sfpEntry.publicInfo.presentInfo.interfaceId);
                PDL_CHECK_STATUS(pdlStatus2);
                if (frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo.readInterfaceGroupType.readInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_GPIO_E) {
                    sfpEntry.publicInfo.values.presentValue = frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo.readInterfaceGroupType.readGpioInterface.gpioReadValue;
                    sfpEntry.publicInfo.presentInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;
                }
                else if (frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo.readInterfaceGroupType.readInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_I2C_E) {
                    sfpEntry.publicInfo.values.presentValue = frontPanelPortEntryPtr->gbicGroup.gbicconnectedinfo.readInterfaceGroupType.readI2CInterface.i2CReadValue;
                    sfpEntry.publicInfo.presentInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }

            sfpEntry.publicInfo.txDisableInfo.isSupported = pdlIsFieldHasValue(frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol_mask);
            if (sfpEntry.publicInfo.txDisableInfo.isSupported) {
                pdlStatus2 = prvPdlInterfaceWriteGroupRegister(&frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol.writeInterfaceGroupType, &sfpEntry.publicInfo.txDisableInfo.interfaceId);
                PDL_CHECK_STATUS(pdlStatus2);
                if (frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol.writeInterfaceGroupType.writeInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_GPIO_E) {
                    sfpEntry.publicInfo.values.txDisableValue = frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol.writeInterfaceGroupType.writeGpioInterface.gpioWriteValue;
                    sfpEntry.publicInfo.txDisableInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;
                }
                else if (frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol.writeInterfaceGroupType.writeInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_I2C_E) {
                    sfpEntry.publicInfo.values.txDisableValue = frontPanelPortEntryPtr->gbicGroup.txisdisabledcontrol.writeInterfaceGroupType.writeI2CInterface.i2CWriteValue;
                    sfpEntry.publicInfo.txDisableInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }

            sfpEntry.publicInfo.txEnableInfo.isSupported = pdlIsFieldHasValue(frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol_mask);
            if (sfpEntry.publicInfo.txEnableInfo.isSupported) {
                pdlStatus2 = prvPdlInterfaceWriteGroupRegister(&frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol.writeInterfaceGroupType, &sfpEntry.publicInfo.txEnableInfo.interfaceId);
                PDL_CHECK_STATUS(pdlStatus2);
                if (frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol.writeInterfaceGroupType.writeInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_GPIO_E) {
                    sfpEntry.publicInfo.values.txEnableValue = frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol.writeInterfaceGroupType.writeGpioInterface.gpioWriteValue;
                    sfpEntry.publicInfo.txEnableInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;
                }
                else if (frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol.writeInterfaceGroupType.writeInterfaceSelect == PDL_INTERFACE_TYPE_INTERFACE_I2C_E) {
                    sfpEntry.publicInfo.values.txEnableValue = frontPanelPortEntryPtr->gbicGroup.txisenabledcontrol.writeInterfaceGroupType.writeI2CInterface.i2CWriteValue;
                    sfpEntry.publicInfo.txEnableInfo.interfaceType = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }

            sfpEntryKey.dev = frontPanelPortEntryPtr->ppDeviceNumber;
            if (pdlIsFieldHasValue(frontPanelPortEntryPtr->logicalPortNumber_mask)) {
                sfpEntryKey.logicalPort = frontPanelPortEntryPtr->logicalPortNumber;
            }
            else {
                sfpEntryKey.logicalPort = frontPanelPortEntryPtr->ppPortNumber;
            }
            pdlStatus = prvPdlibDbAdd(pdlSfpDb, (void*)&sfpEntryKey, (void*)&sfpEntry, (void**)&sfpEntryPtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus2 = prvPdlibDbGetNext(frontPanelGroupPtr->portList_PTR, (void *)&frontPanelPortEntryPtr->list_keys, (void **)&frontPanelPortEntryPtr);
        }

        pdlStatus = prvPdlibDbGetNext(portsParams.frontPanelGroupList_PTR, (void *)&frontPanelGroupPtr->list_keys, (void **)&frontPanelGroupPtr);
    }

	
    return PDL_OK;
}

/*$ END OF pdlSfpInit */


/**
 * @fn  PDL_STATUS prvPdlSfpDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSfpDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbDestroy(pdlSfpDb);
    return pdlStatus;
}

/*$ END OF prvPdlSfpDestroy */






