/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlPpReg.c
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
 * @brief Platform driver layer - packet processor register related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/parser/pdlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/cpu/pdlCpu.h>
#ifdef LINUX_HW
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

static PRV_PDLIB_DB_TYP         prvPdlPpRegDb;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS prvPdlPpRegHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   PDL packet processor register hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegHwGetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    OUT UINT_32                               * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PP_REG_STC                  * ppRegPtr;
    PDL_INTERFACE_PRV_KEY_STC                   ppRegKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    ppRegKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlPpRegDb, (void*) &ppRegKey, (void**) &ppRegPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    return prvPdlRegRead(ppRegPtr->dev, ppRegPtr->regAddr, ppRegPtr->mask, dataPtr);
}

/*$ END OF prvPdlPpRegHwGetValue */


/**
 * @fn  PDL_STATUS prvPdlPpRegHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 data )
 *
 * @brief   PDL packet processor register hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PP_REG_STC                  * ppRegPtr;
    PDL_INTERFACE_PRV_KEY_STC                   ppRegKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    ppRegKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlPpRegDb, (void*) &ppRegKey, (void**) &ppRegPtr);
    PDL_CHECK_STATUS(pdlStatus);

    return prvPdlRegWrite(ppRegPtr->dev, ppRegPtr->regAddr, ppRegPtr->mask, data);
}
/*$ END OF prvPdlPpRegHwSetValue */



/**
 * @fn  PDL_STATUS PdlPpRegDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_PP_REG_STC * attributesPtr )
 *
 * @brief   Pdl packet processor register debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlPpRegDebugDbGetAttributes(
    IN  PDL_INTERFACE_TYP                             interfaceId,
    OUT PDL_INTERFACE_PP_REG_STC                    * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PP_REG_STC                  * ppRegPtr;
    PDL_INTERFACE_PRV_KEY_STC                   ppRegKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    ppRegKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlPpRegDb, (void*) &ppRegKey, (void**) &ppRegPtr);
    if (pdlStatus == PDL_OK) {
        memcpy(attributesPtr, ppRegPtr, sizeof(PDL_INTERFACE_PP_REG_STC));
    }

    return pdlStatus;
}
/*$ END OF PdlPpRegDebugDbGetAttributes */


/**
 * @fn  PDL_STATUS prvPdlPpRegInterfaceRegister ( IN UINT_8 devNum, IN UINT_32 regAddr, IN UINT_32 mask, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register packet processor register interface
 *
 * @param [in]  devNum          device number
 * @param [in]  regAddr         PP register address
 * @param [in]  mask            mask for register operation
 * @param [out] interfaceIdPtr  Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPpRegInterfaceRegister(
    IN UINT_8               devNum,
    IN UINT_32              regAddr,
    IN UINT_32              mask,
    OUT PDL_INTERFACE_TYP * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PP_REG_STC                    ppRegEntry, * ppRegEntryPtr;
    PDL_INTERFACE_PRV_KEY_STC                   ppRegKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    memset(&ppRegEntry, 0, sizeof(ppRegEntry));

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlPpRegDb, &ppRegKey.interfaceId);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = ppRegKey.interfaceId;

    ppRegEntry.dev = devNum;
    ppRegEntry.regAddr = regAddr;
    ppRegEntry.mask = mask;

    pdlStatus = prvPdlibDbAdd(prvPdlPpRegDb, (void *)&ppRegKey, (void **)&ppRegEntry, (void **)&ppRegEntryPtr);
    return pdlStatus;
}
/*$ END OF prvPdlPpRegInterfaceRegister */

/**
 * @fn  PDL_STATUS prvPdlPpRegCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of packet processor register interfaces
 *
 * @param[out] countPtr Number of packet processor register interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegCountGet(
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

    return prvPdlibDbGetNumOfEntries(prvPdlPpRegDb, countPtr);
}

/**
 * @fn  PDL_STATUS prvPdlPpRegInit ( IN PDL_OS_INIT_TYPE_ENT initType )
 *
 * @brief   Init packet processor register  module
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegInit(
    IN  void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                 dbAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PP_REG_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlPpRegDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlPpRegInit */

/**
 * @fn  PDL_STATUS prvPdlPpRegDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPpRegDestroy(
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
    pdlStatus = prvPdlibDbDestroy(prvPdlPpRegDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlPpRegDestroy */
