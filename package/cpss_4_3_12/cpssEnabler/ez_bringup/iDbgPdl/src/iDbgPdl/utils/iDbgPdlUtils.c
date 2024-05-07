/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file iDbgPdlUtils.c
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
 * @brief Debug lib Lib (tools) module
 *
 * @version   1
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <iDbgPdl/utils/iDbgPdlUtils.h>
#include <pdl/button/private/prvPdlBtn.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/cpu/private/prvPdlCpu.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/sfp/pdlSfpDebug.h>
#include <pdl/parser/pdlParser.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlConvertEnumToString
*
* DESCRIPTION:
*
*
*****************************************************************************/

PDL_STATUS iDbgPdlConvertEnumToString(
    IN  IDBG_PDL_UTILS_HW_TYPE_ENT      type,
    IN  UINT_32                         id,
    OUT char                         ** stringPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (stringPtr == NULL) {
        return PDL_BAD_PTR;
    }
    switch (type) {
        case IDBG_PDL_UTILS_LED_TYPE:
            return prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, id, stringPtr);
            break;
        case IDBG_PDL_UTILS_LED_STATE:
            return prvPdlibLibEnumToStrConvert(&pdlLedStateStrToEnum, id, stringPtr);
            break;
        case IDBG_PDL_UTILS_LED_COLOR:
            return prvPdlibLibEnumToStrConvert(&pdlLedColorStrToEnum, id, stringPtr);
            break;
        case IDBG_PDL_UTILS_CPU_FAMILY:
            return prvPdlibLibEnumToStrConvert(&prvPdlCpuFamilyStrToEnum, id, stringPtr);
            break;
        case IDBG_PDL_UTILS_SFP_VALUE_TYPE:
            return prvPdlibLibEnumToStrConvert(&prvPdSfpValueTypeStrToEnum, id, stringPtr);
            break;
        case IDBG_PDL_UTILS_INTERFACE_EXTENDED_TYPE:
            return prvPdlibLibEnumToStrConvert(&prvPdlInterfaceTypeStrToEnum, id, stringPtr);
            break;
        default:
            return PDL_BAD_PARAM;
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlConvertEnumToString */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlConvertEnumToString
*
* DESCRIPTION:
*
*
*****************************************************************************/

PDL_STATUS iDbgPdlConvertStringToEnum(
    IN  IDBG_PDL_UTILS_HW_TYPE_ENT      type,
    IN  char                          * string,
    OUT UINT_32                       * idPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (string == NULL) {
        return PDL_BAD_PTR;
    }

    switch (type) {
        case IDBG_PDL_UTILS_LED_TYPE:
            return prvPdlLibStrToEnumConvert(&pdlLedTypeStrToEnum, string, idPtr);
            break;
        case IDBG_PDL_UTILS_LED_STATE:
            return prvPdlLibStrToEnumConvert(&pdlLedStateStrToEnum, string, idPtr);
            break;
        case IDBG_PDL_UTILS_LED_COLOR:
            return prvPdlLibStrToEnumConvert(&pdlLedColorStrToEnum, string, idPtr);
            break;
        case IDBG_PDL_UTILS_CPU_FAMILY:
            return prvPdlLibStrToEnumConvert(&prvPdlCpuFamilyStrToEnum, string, idPtr);
            break;
        case IDBG_PDL_UTILS_SFP_VALUE_TYPE:
            return prvPdlLibStrToEnumConvert(&prvPdSfpValueTypeStrToEnum, string, idPtr);
            break;
        case IDBG_PDL_UTILS_INTERFACE_EXTENDED_TYPE:
            return prvPdlLibStrToEnumConvert(&prvPdlInterfaceTypeStrToEnum, string, idPtr);
            break;
        default:
            return PDL_BAD_PARAM;
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlConvertEnumToString */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlStringReplace
*
* DESCRIPTION:
*
*
*****************************************************************************/

PDL_STATUS iDbgPdlStringReplace(
    IN  char                          * string,
    IN  char                          * token,
    IN  char                          * newToken,
    IN  UINT_32                         newStringSize,
    OUT char                          * newString
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char                              * p, tempStr[200];
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* if token isn't in string */
    if (!(p = strstr(string, token))) {
        if (newStringSize > strlen(string)) {
            strcpy(newString, string);
            return PDL_OK;
        }
        return PDL_BAD_SIZE;
    }
    if (newStringSize >= 200) {
        return PDL_BAD_PARAM;
    }

    if (newStringSize > strlen(string) - strlen(token) + strlen(newToken)) {
        /* copy chars up to token */
        strncpy(tempStr, string, p-string);
        tempStr[p-string] = '\0';

        prvPdlibOsSnprintf(newString, newStringSize, "%s%s%s", tempStr, newToken, p+strlen(token));
        return PDL_OK;
    }
    return PDL_BAD_SIZE;
}

/*$ END OF iDbgPdlStringReplace */
