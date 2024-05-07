/* Copyright (C) 2023 Marvell International Ltd.
 * This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlLib.c
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
 * @brief Platform driver layer - Lib related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <stdarg.h>
#include <pdl/init/pdlInit.h>
#include <pdl/lib/private/prvPdlLib.h>
/*#include <pdl/lib/private/prvPdlLibLinkedList.h>
#include <pdl/lib/private/prvPdlLibArray.h>
#include <pdl/xml/xmlParser.h>*/
#include <pdlib/lib/pdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/parser/private/prvPdlParser.h>

#ifndef _WIN32
int vsnprintf(char * s, size_t n, const char * format, va_list arg);
#endif /*_WIN32*/

#define MIN(a, b)       ((a) < (b) ? (a) : (b))
/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The prv pdl operating system callbacks pointer */
static PDL_OS_CALLBACK_API_STC  prvPdlOsCallbacks;
static PDL_OS_CALLBACK_API_STC * prvPdlOsCallbacksPtr = NULL;
static PDL_OS_INIT_TYPE_ENT prvPdlLibCurrentInitType = PDL_OS_INIT_TYPE_FULL_E;

extern PDL_STATUS iDbgPdlLoggerPdlDebugTextAdd(IN char * format, ...);

#if 0
/** @brief   The xml error status string to enum pairs[ pdl port speed last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvXmlErrorStatusStrToEnumPairs[] = {
    {"XML_CODE_OK",   XML_PARSER_RET_CODE_OK             },
    {"XML_CODE_ERROR",   XML_PARSER_RET_CODE_ERROR          },
    {"XML_CODE_NOT_FOUND",   XML_PARSER_RET_CODE_NOT_FOUND      },
    {"XML_CODE_NO_MEM",   XML_PARSER_RET_CODE_NO_MEM         },
    {"XML_CODE_WRONG_PARAM",   XML_PARSER_RET_CODE_WRONG_PARAM    },
    {"XML_CODE_WRONG_FORMAT",   XML_PARSER_RET_CODE_WRONG_FORMAT   },
    {"XML_CODE_BAD_SIZE",   XML_PARSER_RET_CODE_BAD_SIZE       },

};
/** @brief   The xml error string to enum
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvXmlErrorStatusStrToEnum = {prvXmlErrorStatusStrToEnumPairs, sizeof(prvXmlErrorStatusStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};
*/
#endif
/** @brief   The debug flags */
BOOLEAN      prvPdlInitDone = FALSE;
BOOLEAN      prvPdlInitDebugFlag = TRUE;
BOOLEAN      prvPdlBtnDebugFlag = TRUE;
BOOLEAN      prvPdlPpDebugFlag = TRUE;
BOOLEAN      prvPdlSfpDebugFlag = TRUE;
BOOLEAN      prvPdlSerdesDebugFlag = TRUE;
BOOLEAN      prvPdlOobPortDebugFlag = TRUE;
BOOLEAN      prvPdlFanDebugFlag = TRUE;
BOOLEAN      prvPdlSensorDebugFlag = TRUE;
BOOLEAN      prvPdlPowerDebugFlag = TRUE;
BOOLEAN      prvPdlPhyDebugFlag = TRUE;
BOOLEAN      prvPdlLedDebugFlag = TRUE;
BOOLEAN      prvPdlLibDebugFlag = TRUE;
BOOLEAN      prvPdlCpuDebugFlag = TRUE;
BOOLEAN      prvPdlFanControllerDebugFlag = TRUE;
BOOLEAN      prvPdlInterfaceDebugFlag = TRUE;
BOOLEAN      prvPdlI2CInterfaceMuxDebugFlag = TRUE;
BOOLEAN      prvPdlAutoParserDebugFlag = TRUE;

UINT_32  prvPdlSpeedEnumToValue[PDL_PORT_SPEED_LAST_E] = {
    10,
    100,
    1000,
    2500,
    5000,
    10000,
    20000,
    24000,
    25000,
    40000,
    50000,
    100000
};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlLibDebugFlag)

/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlLibDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }



/**
 * @fn  PDL_STATUS prvPdlLibStrToEnumConvert( IN PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr, IN char * namePtr, OUT UINT_32 * valuePtr )
 *
 * @brief   Prv pdl library string to enum convert
 *
 * @param [in,out]  convertDbPtr    If non-null, the convert database pointer.
 * @param [in,out]  namePtr         If non-null, the name pointer.
 * @param [in,out]  valuePtr        If non-null, the value pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLibStrToEnumConvert(
    IN  PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC * convertDbPtr,
    IN  char                                * namePtr,
    OUT UINT_32                             * valuePtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    UINT_32 i;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    if (namePtr == NULL || valuePtr == NULL || convertDbPtr == NULL) {
        return PDL_BAD_PTR;
    }

    for (i = 0; i < convertDbPtr->size; i++) {
        if (prvPdlStrcicmp(namePtr, convertDbPtr->str2Enum[i].namePtr) == 0) {
            *valuePtr = convertDbPtr->str2Enum[i].value;
            return PDL_OK;
        }
    }

    return PDL_NOT_FOUND;
}

/*$ END OF prvPdlLibStrToEnumConvert */



/**
 * @fn  void prvPdlStatusDebugLogHdr ( IN const char * func_name_PTR, IN const char * line_num, IN PDL_STATUS   status, IN UINT_32  pdlIdentifier)
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 * @param   status                 error code.
 * @param   pdlIdentifier          identifier for the status type.
 */

BOOLEAN prvPdlStatusDebugLogHdr(
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num,
    IN   PDL_STATUS   status,
    IN   UINT_32      pdlIdentifier
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN     ret;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    ret = prvPdlibStatusDebugLogHdr(func_name_PTR, line_num, status, pdlIdentifier);

    iDbgPdlLoggerPdlDebugTextAdd("FUNCTION: %s, LINE: %d\n", func_name_PTR, line_num);
    return ret;
}
/*$ END OF prvPdlStatusDebugLogHdr */

/**
 * @fn  void prvPdlDebugLogHdr ( IN const char * func_name_PTR, IN   UINT_32 line_num )
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR          The function name pointer.
 * @param   line_num               Describes the format to use.
 */

BOOLEAN prvPdlDebugLogHdr(
    IN   const char * func_name_PTR,
    IN   UINT_32      line_num
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN ret;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    ret = prvPdlibDebugLogHdr(func_name_PTR, line_num);
    iDbgPdlLoggerPdlDebugTextAdd("FUNCTION: %s, LINE: %d", func_name_PTR, line_num);
    return ret;
}
/*$ END OF prvPdlDebugLogHdr */

/**
 * @fn  void prvPdlDebugRawLog ( IN const char * format, IN ... )
 *
 * @brief   Prv pdl debug log
 *
 * @param   func_name_PTR   The function name pointer.
 * @param   format          Describes the format to use.
 * @param   ...             The ...
 */

void prvPdlDebugRawLog(
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format,
    IN ...
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char      buffer[1024], buffer2[200];
    va_list   argptr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    va_start(argptr, format);
#ifndef _WIN32
    vsnprintf(buffer, sizeof(buffer), format, argptr);
#else
    vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, format, argptr);
#endif
    va_end(argptr);
    prvPdlibOsSnprintf(buffer2, 200, "%s Line[%d]", func_name_PTR, line);
    /* init not completed, debug is not ready yet*/
    if (prvPdlInitDone == FALSE) {
    	if (prvPdlOsCallbacksPtr && prvPdlOsCallbacksPtr->pdLibCallbacks.printStringPtr) {
    		prvPdlOsCallbacksPtr->pdLibCallbacks.printStringPtr("%s %s", buffer2, buffer);
    	}
    }
    else {
        if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->pdLibCallbacks.debugLogPtr == NULL) {
            return;
        }
        if (prvPdlOsCallbacksPtr->pdLibCallbacks.debugLogPtr) {
        	prvPdlOsCallbacksPtr->pdLibCallbacks.debugLogPtr(buffer2, buffer);
        }
    }
    iDbgPdlLoggerPdlDebugTextAdd(buffer);
}
/*$ END OF prvPdlDebugLog */

/**
 * @fn  void prvPdlDebugRawPrint ( IN const char * format, IN ... )
 *
 * @brief   Prv pdl debug print
 *
 * @param   func_name_PTR   The function name pointer.
 * @param   format          Describes the format to use.
 * @param   ...             The ...
 */

void prvPdlDebugRawPrint(
    IN const char * func_name_PTR,
    IN UINT_32      line,
    IN const char * format,
    IN ...
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char      buffer[1024], buffer2[200];
    va_list   argptr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    va_start(argptr, format);
#ifndef _WIN32
    vsnprintf(buffer, sizeof(buffer), format, argptr);
#else
    vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, format, argptr);
#endif
    va_end(argptr);
    prvPdlibOsSnprintf(buffer2, 200, "%s Line[%d]", func_name_PTR, line);
    if (prvPdlOsCallbacksPtr && prvPdlOsCallbacksPtr->pdLibCallbacks.printStringPtr) {
    	prvPdlOsCallbacksPtr->pdLibCallbacks.printStringPtr("%s %s", buffer2, buffer);
    }
    iDbgPdlLoggerPdlDebugTextAdd(buffer);
}
/*$ END OF prvPdlDebugRawPrint */

/**
 * @fn  void prvPdlLock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlLock(
    IN PDL_OS_LOCK_TYPE_ENT         lockType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->lockFuncArr[lockType].lockFun == NULL || lockType == PDL_OS_LOCK_TYPE_NONE_E) {
        return;
    }
    prvPdlOsCallbacksPtr->lockFuncArr[lockType].lockFun();
}
/*$ END OF prvPdlLock */

/**
 * @fn  void prvPdlUnlock ( IN PDL_OS_LOCK_TYPE_ENT lockType )
 *
 * @brief   uses lock mechanism given by higher layer to provide mutual exclusion
 *
 * @param [in]      lockType        what are we protecting
 */

void prvPdlUnlock(
    IN PDL_OS_LOCK_TYPE_ENT         lockType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->lockFuncArr[lockType].unlockFun == NULL || lockType == PDL_OS_LOCK_TYPE_NONE_E) {
        return;
    }
    prvPdlOsCallbacksPtr->lockFuncArr[lockType].unlockFun();
}
/*$ END OF prvPdlUnlock */

/**
 * @fn  PDL_STATUS prvPdlCmRegRead
 *
 * @brief   Reads PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlCmRegRead(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppCmRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ppCmRegReadClbk(devIdx, regAddr, mask, dataPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlCmRegRead */

/**
 * @fn  PDL_STATUS prvPdlCmRegWrite
 *
 * @brief   Writes PP C&M register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlCmRegWrite(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppCmRegWriteClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ppCmRegWriteClbk(devIdx, regAddr, mask, data);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlCmRegWrite */

/**
 * @fn  PDL_STATUS prvPdlRegRead
 *
 * @brief   Reads PP register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlRegRead(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ppRegReadClbk(devIdx, regAddr, mask, dataPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlRegRead */

/**
 * @fn  PDL_STATUS prvPdlRegWrite
 *
 * @brief   Writes PP register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      regAddr       register address
 * @param [in]      mask          mask to read
 * @param [in]      data          data to write
 */

PDL_STATUS prvPdlRegWrite(
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ppRegWriteClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ppRegWriteClbk(devIdx, regAddr, mask, data);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlRegWrite */

/**
 * @fn  PDL_STATUS prvPdlSmiRegRead
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlSmiRegRead(
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    OUT GT_U16    *dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->smiRegReadClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, dataPtr);
}
/*$ END OF prvPdlSmiRegRead */

/**
 * @fn  PDL_STATUS prvPdlSmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      phyPageSelReg phy page selected register
 * @param [in]      phyPage       phy page number
 * @param [in]      regAddr       register address
 * @param [out]     dataPtr       data to write
 */

PDL_STATUS prvPdlSmiRegWrite(
    IN  GT_U8      devIdx,
    IN  GT_U32     portGroupsBmp,
    IN  GT_U32     smiInterface,
    IN  GT_U32     smiAddr,
    IN  GT_U8      phyPageSelReg,
    IN  GT_U8      phyPage,
    IN  GT_U32     regAddr,
    IN  GT_U16     mask,
    IN  GT_U16     data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    GT_U16          oldData, newData;
    PDL_STATUS      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiRegWriteClbk == NULL || prvPdlOsCallbacksPtr->smiRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    pdlStatus = prvPdlOsCallbacksPtr->smiRegReadClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, &oldData);
    PDL_CHECK_STATUS(pdlStatus);
    newData = (~mask & oldData) | (mask & data);
    return prvPdlOsCallbacksPtr->smiRegWriteClbk(devIdx, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage, regAddr, newData);
}
/*$ END OF prvPdlSmiRegWrite */

/**
 * @fn  PDL_STATUS prvPdlSmiPpuLock
 *
 * @brief   Control the SMI MAC polling unit using given callback
 *
 * @param [in]      devIdx        device index
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      lock          stop/start smi auto polling unit
 * @param [out]     prevLockStatePtr previous state of the lock
 */

PDL_STATUS prvPdlSmiPpuLock(
    IN  GT_U8       devIdx,
    IN  GT_U32      smiInterface,
    IN  BOOLEAN     lock,
    OUT BOOLEAN    *prevLockStatePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->smiPpuLockClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->smiPpuLockClbk(devIdx, smiInterface, lock, prevLockStatePtr);
}
/*$ END OF prvPdlSmiPpuLock */

/**
 * @fn  PDL_STATUS prvPdlXsmiRegRead
 *
 * @brief   Reads XSMI register using given callback
 *
 * @param [in]      devIdx        device index to read register from
 * @param [in]      xsmiInterface Xsmi interface (0..3)
 * @param [in]      xsmiAddr      Xsmi adresss
 * @param [in]      regAddr       register address
 * @param [in]      phyDev        the PHY device to read from (APPLICABLE RANGES: 0..31).
 * @param [out]     dataPtr       pointer to where data read will be stored
 */

PDL_STATUS prvPdlXsmiRegRead(
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    OUT GT_U16     *dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->xsmiRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->xsmiRegReadClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr);
}
/*$ END OF prvPdlXsmiRegRead */


/**
 * @fn  PDL_STATUS prvPdlXsmiRegWrite
 *
 * @brief   Reads SMI register using given callback
 *
 * @param [in]      devIdx        device index to write register to
 * @param [in]      portGroupsBmp bitmap of ports
 * @param [in]      smiInterface  smi interface (0..3)
 * @param [in]      smiAddr       smi adresss
 * @param [in]      regAddr       register address
 * @param [out]     data          data to write
 */

PDL_STATUS prvPdlXsmiRegWrite(
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    IN  GT_U16     mask,
    IN  GT_U16     data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    GT_U16          oldData, newData;
    PDL_STATUS      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->xsmiRegWriteClbk == NULL || prvPdlOsCallbacksPtr->xsmiRegReadClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    pdlStatus = prvPdlOsCallbacksPtr->xsmiRegReadClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, &oldData);
    PDL_CHECK_STATUS(pdlStatus);
    newData = (~mask & oldData) | (mask & data);
    return prvPdlOsCallbacksPtr->xsmiRegWriteClbk(devIdx, xsmiInterface, xsmiAddr, regAddr, phyDev, newData);
}
/*$ END OF prvPdlXsmiRegWrite */

/**
 * @fn  void prvPdlI2cResultHandler
 *
 * @brief   Call to given call-back which handle I2C operation result
 *
 * @param [in]      i2c_ret_status  return status from I2C operation
 * @param [in]      slave_address   I2C device (slave) address
 * @param [in]      bus_id          I2C bus id
 * @param [in]      offset          I2C offset
 * @param [in]      i2c_write       whether the operation was I2C write or read
 */
void prvPdlI2cResultHandler(
    /*!     INPUTS:             */
    IN  PDL_STATUS i2c_ret_status,
    IN  UINT_8     slave_address,
    IN  UINT_8     bus_id,
    IN  UINT_8     offset,
    IN  BOOLEAN    i2c_write
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/

    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/

    if (prvPdlOsCallbacksPtr && prvPdlOsCallbacksPtr->i2cResultHandlerClbk) {
        prvPdlOsCallbacksPtr->i2cResultHandlerClbk(i2c_ret_status, slave_address, bus_id, offset, i2c_write);
    }
}
/*$ END OF prvPdli2cHandler */




/**
 * @fn  PDL_STATUS prvPdlLedStreamPortPositionSet
 *
 * @brief   set ledstream port position
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      position      led stream port position
 */

PDL_STATUS prvPdlLedStreamPortPositionSet(
    IN  GT_U8                   devIdx,
    IN  GT_U32                  portNum,
    IN  GT_U32                  position
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortPositionSetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortPositionSetClbk(devIdx, portNum, position);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamPortPositionSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet
 *
 * @brief   set ledstream port polarity
 *
 * @param [in]      devIdx        device index
 * @param [in]      portNum       port number
 * @param [in]      classNum      class number
 * @param [in]      invertEnable  polarity invert enable/disable
 */

PDL_STATUS prvPdlLedStreamPortClassPolarityInvertEnableSet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          portNum,
    IN  GT_U32                          classNum,
    IN  BOOLEAN                         invertEnable
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortClassPolarityInvertEnableSetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamPortClassPolarityInvertEnableSetClbk(devIdx, portNum, classNum, invertEnable);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamPortClassPolarityInvertEnableSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamConfigSet
 *
 * @brief   set ledstream configuration
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      ledConfPtr              led stream configuration paramters
 */

PDL_STATUS prvPdlLedStreamConfigSet(
    IN  GT_U8                               devIdx,
    IN  GT_U32                              ledInterfaceNum,
    IN  PDL_LED_STEAM_INTERFACE_CONF_STC   *ledConfPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamConfigSetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledStreamConfigSetClbk(devIdx, ledInterfaceNum, ledConfPtr);
#else
    return PDL_OK;
#endif
}
/*$ END OF prvPdlLedStreamConfigSet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationGet
 *
 * @brief   ledstream manipulation get value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [out]     classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationGet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    OUT PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationGetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationGetClbk(devIdx, ledInterfaceNum, portType, classNum, classParamsPtr);
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlLedStreamClassManipulationGet */

/**
 * @fn  PDL_STATUS prvPdlLedStreamClassManipulationSet
 *
 * @brief   ledstream manipulation set value
 *
 * @param [in]      devIdx                  device index
 * @param [in]      ledInterfaceNum         ledstream interface number
 * @param [in]      portType                port type
 * @param [in]      classNum                class number
 * @param [in]      classParamsPtr          value
 */

PDL_STATUS prvPdlLedStreamClassManipulationSet(
    IN  GT_U8                           devIdx,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    IN  PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationSetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->ledStreamCallbackInfo.ledsStreamClassManipulationSetClbk(devIdx, ledInterfaceNum, portType, classNum, classParamsPtr);
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlLedStreamClassManipulationSet */

/**
 * @fn  PDL_STATUS pdlLibDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Lib debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLibDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlLibDebugFlag = state;
    return PDL_OK;
}

/* ***************************************************************************
* FUNCTION NAME: prvPdlLibPortModeSupported
*
* DESCRIPTION:   verify port interface mode & speed are supported by HW
*
*****************************************************************************/

BOOLEAN prvPdlLibPortModeSupported(
    UINT_8                      devIdx,
    UINT_32                     mac_port,
    PDL_PORT_SPEED_ENT          speed,
    PDL_INTERFACE_MODE_ENT      interface_mode
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->portModeSupportedClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->portModeSupportedClbk(devIdx, mac_port, speed, interface_mode);
}
/*$ END OF prvPdlRegWrite */

/**
 * @fn  PDL_STATUS pdlLibBoardDescriptionDbGet ( IN UINT_32 strLen, OUT char *boardDescStrPtr )
 *
 * @brief   get board description string
 *
 * @param [in]   strLen             given string pointer max size (length)
 * @param [out]  boardDescStrPtr    string pointer
 *
 * @return  PDL_OK                  success
 * @return  PDL_BAD_PTR             illegal pointer supplied.
 * @return  PDL_NOT_INITIALIZED     board description doesn't exist in XML
 * @return  PDL_BAD_SIZE            board description from XML is longer than given string pointer
 */

PDL_STATUS pdlLibBoardDescriptionDbGet(
    IN  UINT_32               strLen,
    OUT char                * boardDescStrPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_BOARD_INFORMATION_PARAMS_STC boardInformation;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (boardDescStrPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (PDL_OK != pdlProjectParamsGet(PDL_FEATURE_ID_BOARD_INFORMATION_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&boardInformation) ||
        FALSE == pdlIsFieldHasValue(boardInformation.boardDescription_mask)) {
        return PDL_NOT_INITIALIZED;
    }
    if (strlen(boardInformation.boardDescription) > strLen) {
        return PDL_BAD_SIZE;
    }
    strcpy(boardDescStrPtr, boardInformation.boardDescription);
    return PDL_OK;
}
/**
 * @fn  PDL_STATUS prvPdlI2cChannelToDevIdConvert
 *
 * @brief   Convert i2cbase address & channel id (retrieved from XML) to i2c device id
 *
 * @param [in]      i2cBaseAddress      i2c base address
 * @param [in]      i2cChannelId        i2c channel id
 * @param [out]     i2cDevIdPtr         i2c device id (as found in /dev/)
 */

PDL_STATUS prvPdlI2cChannelToDevIdConvert(
    IN  UINT_32      i2cBaseAddress,
    IN  UINT_32      i2cChannelId,
    OUT UINT_32    * i2cDevIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->i2cChannelToDevIdConvertClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    return prvPdlOsCallbacksPtr->i2cChannelToDevIdConvertClbk(i2cBaseAddress, i2cChannelId, i2cDevIdPtr);
}
/*$ END OF prvPdlI2cChannelToDevIdConvert */


/**
 * @fn  PDL_STATUS pdlLibConvertEnumValueToString ( IN PDL_ENUM_IDS_ENT enumId, IN UINT_32 intValue, OUT char ** strValuePtr )
 *
 * @brief   Convert enum int to string value.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  intValue        enum int value to convert.
 * @param [out] strValuePtr     pointer to string value to return.
 *
 * @return  PDL_OK              success
 * @return  PDL_BAD_PTR         if invalid id or pointer was supplied.
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertEnumValueToString(
    IN PDL_ENUM_IDS_ENT         enumId,
    IN UINT_32                  intValue,
    OUT char                 ** strValuePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (strValuePtr == NULL) {
        return PDL_BAD_PTR;
    }

    switch (enumId) {
        case PDL_ENUM_ID_SPEED_TYPE_E:
        case PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E:
        case PDL_ENUM_ID_FAN_ROLE_TYPE_E:
        case PDL_ENUM_ID_PHY_TYPE_TYPE_E:
        case PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E:
        case PDL_ENUM_ID_CPU_TYPE_TYPE_E:
        case PDL_ENUM_ID_POWER_STATUS_TYPE_E:
        case PDL_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E:
        case PDL_ENUM_ID_BUTTON_TYPE_E:
        case PDL_ENUM_ID_FAN_TYPE_E:
        case PDL_ENUM_ID_SENSOR_HW_TYPE_E:
        case PDL_ENUM_ID_BANK_SOURCE_TYPE_E:
            return pdlEnum2StrConvert(enumId, PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(intValue), strValuePtr);
            break;
        case PDL_ENUM_ID_TRANSCEIVER_TYPE_E:
        case PDL_ENUM_ID_INTERFACE_TYPE_E:
        case PDL_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E:
        case PDL_ENUM_ID_MPP_PIN_MODE_TYPE_E:
        case PDL_ENUM_ID_I2C_ACCESS_TYPE_E:
        case PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E:
        case PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E:
            return pdlEnum2StrConvert(enumId, intValue, strValuePtr);
            break;
        default:
            return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLibConvertEnumValueToString */

/**
 * @fn  PDL_STATUS pdlLibConvertStringToEnumValue ( IN PDL_ENUM_IDS_ENT enumId, IN char * strValuePtr, OUT UINT_32 * intValuePtr )
 *
 * @brief   Convert string to enum int.
 *
 * @param [in]  enumId          Enum identification.
 * @param [in]  strValuePtr     string value to convert
 * @param [out] intValuePtr     enum int value to return.
 *
 * @return  PDL_OK              success
 * @return  PDL_BAD_PTR         if invalid id or pointer was supplied.
 * @return  PDL_BAD_PARAM       if invalid id or pointer was supplied.
 * @return  PDL_NOT_INITIALIZED if database wasn't initialized.
 * @return  PDL_NOT_FOUND       if no match was found.
 *
 */
PDL_STATUS pdlLibConvertStringToEnumValue(
    IN PDL_ENUM_IDS_ENT         enumId,
    IN char                   * strValuePtr,
    OUT UINT_32               * intValuePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                 value;
    PDL_STATUS              pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (strValuePtr == NULL || intValuePtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlStr2EnumConvert(enumId, strValuePtr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    switch (enumId) {
        case PDL_ENUM_ID_SPEED_TYPE_E:
        case PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E:
        case PDL_ENUM_ID_FAN_ROLE_TYPE_E:
        case PDL_ENUM_ID_PHY_TYPE_TYPE_E:
        case PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E:
        case PDL_ENUM_ID_CPU_TYPE_TYPE_E:
        case PDL_ENUM_ID_POWER_STATUS_TYPE_E:
        case PDL_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E:
        case PDL_ENUM_ID_BUTTON_TYPE_E:
        case PDL_ENUM_ID_FAN_TYPE_E:
        case PDL_ENUM_ID_SENSOR_HW_TYPE_E:
        case PDL_ENUM_ID_BANK_SOURCE_TYPE_E:
            *intValuePtr = value - 1;
            break;
        case PDL_ENUM_ID_TRANSCEIVER_TYPE_E:
        case PDL_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E:
        case PDL_ENUM_ID_INTERFACE_TYPE_E:
        case PDL_ENUM_ID_MPP_PIN_MODE_TYPE_E:
        case PDL_ENUM_ID_I2C_ACCESS_TYPE_E:
        case PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E:
        case PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E:
            *intValuePtr = value;
            break;
        default:
            return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLibConvertStringToEnumValue */

/**
 * @fn  PDL_STATUS prvPdlLibSleep
 *
 * @brief   sleep for a milliseconds amount of time
 *
 * @param [in]      msec                duration of sleep in milliseconds
 */

PDL_STATUS prvPdlLibSleep(
    UINT_32                     msec
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->sleepClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    prvPdlOsCallbacksPtr->sleepClbk(msec);
    return PDL_OK;
}
/*$ END OF prvPdlLibSleep */

/**
 * @fn  PDL_STATUS prvPdlLibMpdLogicalPortValueSet
 *
 * @brief   set MPD logical port phy value
 *
 * @param [in]      mpdLogicalPort      MPD logical port number
 * @param [in]      phyDeviceOrPage     phy device or page
 * @param [in]      regAddress          phy reg address
 * @param [in]      mask                operation mask value
 * @param [in]      data                data to write
 */

PDL_STATUS prvPdlLibMpdLogicalPortValueSet(
    IN  UINT_32                         mpdLogicalPort,
    IN  UINT_8                          phyDeviceOrPage,
    IN  UINT_16                         regAddress,
    IN  UINT_16                         mask,
    IN  UINT_16                         data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (prvPdlOsCallbacksPtr == NULL || prvPdlOsCallbacksPtr->mpdLogicalPortValueSetClbk == NULL) {
        return PDL_NOT_IMPLEMENTED;
    }
    prvPdlOsCallbacksPtr->mpdLogicalPortValueSetClbk(mpdLogicalPort, phyDeviceOrPage, regAddress, mask, data);
    return PDL_OK;
}
/*$ END OF prvPdlLibMpdLogicalPortValueSet */



/**
 * @fn  PDL_STATUS pdlLibInit ( IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   Pdl library initialize
 *
 * @param [in,out]  callbacksPTR    If non-null, the callbacks pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLibInit(
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlLibCurrentInitType = initType;

    if (callbacksPTR == NULL) {
        return PDL_BAD_PTR;
    }
    prvPdlOsCallbacksPtr = &prvPdlOsCallbacks;
    memcpy(prvPdlOsCallbacksPtr, callbacksPTR, sizeof(PDL_OS_CALLBACK_API_STC));
    return PDL_OK;
}

/*$ END OF PdlBtnInit */
