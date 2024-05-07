/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlSmiXsmi.c
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
 * @brief Platform driver layer - SMI/XSMI related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

static PRV_PDLIB_DB_TYP           prvPdlSmiXsmiDb;
/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl Smi/Xsmi hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwGetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    OUT UINT_16                               * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_16                                     xsmiData = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    switch (smiXsmiPtr->publicInfo.interfaceType) {
        case PDL_INTERFACE_EXTENDED_TYPE_SMI_E:
            prvPdlLock(PDL_OS_LOCK_TYPE_SMI_E);
            pdlStatus = prvPdlSmiRegRead(smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address, 22, phyDeviceOrPage, (UINT_32) regAddress, dataPtr);
            if (pdlStatus != PDL_OK) {
                prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);
            }
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);
            break;
        case PDL_INTERFACE_EXTENDED_TYPE_XSMI_E:
            prvPdlLock(PDL_OS_LOCK_TYPE_XSMI_E);
            pdlStatus = prvPdlXsmiRegRead(smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address, regAddress, phyDeviceOrPage, &xsmiData);
            if (pdlStatus != PDL_OK) {
                prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
            }
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
            *dataPtr = (UINT_32) xsmiData;
            break;
        default:
            return PDL_ERROR;
    }

    return PDL_OK;
}

/*$ END OF pdlSmiXsmiHwGetValue */

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 data )
 *
 * @brief   Pdl Smi/Xsmi hardware set value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    IN  UINT_16                                 mask,
    OUT UINT_16                                 data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));
    smiXsmiKey.interfaceId = interfaceId;

    pdlStatus  = prvPdlibDbFind(prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (smiXsmiPtr->publicInfo.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_SMI_E || smiXsmiPtr->publicInfo.interfaceType  == PDL_INTERFACE_EXTENDED_TYPE_XSMI_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "writing  %s dev[%d] interfaceId [%d] address[%d] phyDevOrPage[%d] regAddress[0x%x] mask[0x%x] data[0x%x]\n",
                                                    smiXsmiPtr->publicInfo.interfaceType==PDL_INTERFACE_EXTENDED_TYPE_SMI_E ? "SMI":"XSMI", smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address, phyDeviceOrPage, regAddress, mask, data);
    }

    switch (smiXsmiPtr->publicInfo.interfaceType) {

        case PDL_INTERFACE_EXTENDED_TYPE_SMI_E:
            prvPdlLock(PDL_OS_LOCK_TYPE_SMI_E);
            pdlStatus = prvPdlSmiRegWrite(smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address, 22, phyDeviceOrPage, (UINT_32) regAddress, mask, data);
            if (pdlStatus != PDL_OK) {
                prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);
            }
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlUnlock(PDL_OS_LOCK_TYPE_SMI_E);
            break;
        case PDL_INTERFACE_EXTENDED_TYPE_XSMI_E:
            prvPdlLock(PDL_OS_LOCK_TYPE_XSMI_E);
            pdlStatus = prvPdlXsmiRegWrite(smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId, smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address, regAddress, phyDeviceOrPage, (UINT_16) mask, (UINT_16) data);
            if (pdlStatus != PDL_OK) {
                prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
            }
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlUnlock(PDL_OS_LOCK_TYPE_XSMI_E);
            break;
        case PDL_INTERFACE_EXTENDED_TYPE_MPD_E:
            PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "writing  mpd_phy logicalPort[%d] phyDevOrPage[%d] regAddress[0x%x] mask[0x%x] data[0x%x]\n",
                                                        smiXsmiPtr->publicInfo.phyInfo.mpdLogicalPort, phyDeviceOrPage, regAddress, mask, data);
            pdlStatus = prvPdlLibMpdLogicalPortValueSet(smiXsmiPtr->publicInfo.phyInfo.mpdLogicalPort, phyDeviceOrPage, regAddress, (UINT_16) mask, (UINT_16) data);
            PDL_CHECK_STATUS(pdlStatus);
            break;
        default:
            return PDL_ERROR;
    }
    return PDL_OK;

}

/*$ END OF pdlSmiXsmiHwSetValue */

/**
 * @fn  PDL_STATUS pdlSmiXsmiCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of smi/xsmi interfaces
 *
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlSmiXsmiCountGet(
    OUT UINT_32 * countPtr
)
{
    /****************************************************************************/
    /*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /****************************************************************************/

    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (countPtr == NULL) {
        return PDL_BAD_PTR;
    }

    return prvPdlibDbGetNumOfEntries(prvPdlSmiXsmiDb, countPtr);

    return PDL_OK;
}

/*$ END OF pdlSmiXsmiCountGet */

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiInterfaceRegister ( IN UINT_8 dev, IN UINT_8 address, IN UINT_8 interfaceId, IN PDL_INTERFACE_EXTENDED_TYPE_ENT interfaceType, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register smi/xsmi interface usage
 *
 * @param [in]  dev                         device number
 * @param [in]  address                     address
 * @param [in]  interfaceId                 interface id
 * @param [in]  interfaceType               interface type (smi/xsmi)
 * @param [out] interfaceIdPtr              Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlSmiXsmiInterfaceRegister(
    IN UINT_8                           dev,
    IN UINT_8                           address,
    IN UINT_8                           interfaceId,
    IN PDL_INTERFACE_EXTENDED_TYPE_ENT  interfaceType,
    OUT PDL_INTERFACE_TYP             * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiEntryPtr, smiXsmiEntry;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    memset(&smiXsmiEntry, 0, sizeof(smiXsmiEntry));
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlSmiXsmiDb, &smiXsmiKey.interfaceId);
    PDL_CHECK_STATUS(pdlStatus);

    if (interfaceType != PDL_INTERFACE_EXTENDED_TYPE_SMI_E && interfaceType != PDL_INTERFACE_EXTENDED_TYPE_XSMI_E) {
        return PDL_BAD_PARAM;
    }

    smiXsmiEntry.publicInfo.phyInfo.readWriteAddress.dev = dev;
    smiXsmiEntry.publicInfo.phyInfo.readWriteAddress.address = address;
    smiXsmiEntry.publicInfo.phyInfo.readWriteAddress.interfaceId = interfaceId;
    smiXsmiEntry.publicInfo.interfaceType = interfaceType;

    pdlStatus = prvPdlibDbAdd(prvPdlSmiXsmiDb, (void *)&smiXsmiKey, (void **)&smiXsmiEntry, (void **)&smiXsmiEntryPtr);
    *interfaceIdPtr = smiXsmiKey.interfaceId;
    return pdlStatus;
}
/*$ END OF prvPdlSmiXsmiInterfaceRegister */

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiInterfaceRegister ( UINT_32 mpdLogicalPort, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register smi/xsmi mpd interface usage
 *
 * @param [in]  mpdLogicalPort              mpd logical port number
 * @param [out] interfaceIdPtr              Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlMpdSmiXsmiInterfaceRegister(
    IN  UINT_32                         mpdLogicalPort,
    OUT PDL_INTERFACE_TYP             * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiEntryPtr, smiXsmiEntry;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    memset(&smiXsmiEntry, 0, sizeof(smiXsmiEntry));
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlSmiXsmiDb, &smiXsmiKey.interfaceId);
    PDL_CHECK_STATUS(pdlStatus);

    smiXsmiEntry.publicInfo.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_MPD_E;
    smiXsmiEntry.publicInfo.phyInfo.mpdLogicalPort = mpdLogicalPort;

    pdlStatus = prvPdlibDbAdd(prvPdlSmiXsmiDb, (void *)&smiXsmiKey, (void **)&smiXsmiEntry, (void **)&smiXsmiEntryPtr);
    *interfaceIdPtr = smiXsmiKey.interfaceId;
    return pdlStatus;
}
/*$ END OF prvPdlSmiXsmiInterfaceRegister */


/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbGetAttributes
*
* DESCRIPTION:   get interface attributes from DB
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbGetAttributes(
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_SMI_XSMI_STC          * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy(attributesPtr, &smiXsmiPtr->publicInfo, sizeof(PDL_INTERFACE_SMI_XSMI_STC));
    return PDL_OK;
}
/*$ END OF pdlSmiXsmiDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbSetAttributes
*
* DESCRIPTION:   set interface attributes from DB
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbSetAttributes(
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_SMI_XSMI_STC         * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_SMI_XSMI_DB_STC         * smiXsmiPtr;
    PDL_INTERFACE_PRV_KEY_STC                   smiXsmiKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&smiXsmiKey, 0, sizeof(smiXsmiKey));
    smiXsmiKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlSmiXsmiDb, (void*) &smiXsmiKey, (void**) &smiXsmiPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (smiXsmiPtr->publicInfo.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_MPD_E) {
        return PDL_NOT_SUPPORTED;
    }

    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_DEVICE) {
        smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.dev = attributesPtr->phyInfo.readWriteAddress.dev;
    }
    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_INTERFACE_ID) {
        smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.interfaceId = attributesPtr->phyInfo.readWriteAddress.interfaceId;
    }
    if (attributesMask & PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_ADDRESS) {
        smiXsmiPtr->publicInfo.phyInfo.readWriteAddress.address = attributesPtr->phyInfo.readWriteAddress.address;
    }
    return PDL_OK;
}
/*$ END OF pdlSmiXsmiDebugDbSetAttributes */

/**
 * @fn  PDL_STATUS pdlSmiXsmiInit ( IN void )
 *
 * @brief   Pdl SMI/XSMI initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiInit(
    IN  void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC       dbAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_SMI_XSMI_DB_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlSmiXsmiDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlSmiXsmiInit */

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbDestroy(prvPdlSmiXsmiDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlSmiXsmiDestroy */

