/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlInit.c
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
 * @brief Platform driver layer - PDL driver initialization
 *
 * @version   1
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/

#include <pdl/button/private/prvPdlBtn.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/fan/private/prvPdlFanAdt7476.h>
#include <pdl/fan/private/prvPdlFanEMC2305.h>
#include <pdl/fan/private/prvPdlFanNCT7802.h>
#include <pdl/fan/private/prvPdlFanPWM.h>
#include <pdl/fan/private/prvPdlFanTc654.h>
#include <pdl/init/pdlInit.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/oob_port/private/prvPdlOobPort.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/power_supply/private/prvPdlPower.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdlib/common/pdlTypes.h>
#include <pdlib/lib/pdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdlib/xml/private/prvXmlParserBuilder.h>
#include <pdlib/xml/xmlParser.h>
#include <pdl/interface/private/prvPdlPpReg.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

static BOOLEAN                                 pdlFanInternalDriverBind = FALSE;
static XML_PARSER_ROOT_DESCRIPTOR_TYP          pdlInitxmlRootId;

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlInitDebugFlag)
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                  \
    if (__xmlStatus != PDL_OK) {                                                                       \
    if (prvPdlInitDebugFlag) {                                                                         \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __xmlStatus;                                                                                \
    }

/**
 * @fn  PDL_STATUS pdlInitDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Init debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInitDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlInitDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  void pdlInitDone ()
 *
 * @brief   Called by application to indicate PDL init has been completed for all XML files
 *
 */

PDL_STATUS pdlInitDone(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* mark init is completed and switch all debug flags to FALSE since they can be controlled using CLI commands */
    prvPdlInitDone = TRUE;
    prvPdlInitDebugFlag = FALSE;
    prvPdlBtnDebugFlag = FALSE;
    prvPdlPpDebugFlag = FALSE;
    prvPdlSfpDebugFlag = FALSE;
    prvPdlSerdesDebugFlag = FALSE;
    prvPdlOobPortDebugFlag = FALSE;
    prvPdlFanDebugFlag = FALSE;
    prvPdlSensorDebugFlag = FALSE;
    prvPdlPowerDebugFlag = FALSE;
    prvPdlPhyDebugFlag = FALSE;
    prvPdlLedDebugFlag = FALSE;
    prvPdlLibDebugFlag = FALSE;
    prvPdlCpuDebugFlag = FALSE;
    prvPdlFanControllerDebugFlag = FALSE;
    prvPdlInterfaceDebugFlag = FALSE;
    prvPdlI2CInterfaceMuxDebugFlag = FALSE;
    prvPdlAutoParserDebugFlag = FALSE;

    /* verify all external drivers are bounded correctly */
    pdlStatus = prvPdlSensorVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlLedVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlFanVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlInitDone */

/**
 * @fn  PDL_STATUS pdlInit ( IN char * xmlFilePathPtr, IN char * xmlTagPrefixPtr, IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   initialize PDL
 *
 * @param [in]  xmlFilePathPtr  XML location.
 * @param [in]  xmlTagPrefixPtr XML tag's prefix.
 * @param [in]  callbacksPTR    application-specific implementation for os services.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlInit(
    IN char                       * xmlFilePathPtr,
    IN char                       * xmlTagPrefixPtr,
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              rc;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    rc = pdlibLibInit(&callbacksPTR->pdLibCallbacks);
    PDL_CHECK_STATUS(rc);

    if (initType == PDL_OS_INIT_TYPE_LIB_E) {
        return PDL_OK;
    }

    rc = pdlLibInit(callbacksPTR, initType);
    xmlStatus = xmlParserBuild(xmlFilePathPtr, xmlTagPrefixPtr, &pdlInitxmlRootId);
    XML_CHECK_STATUS(xmlStatus);

    rc = pdlProjectParamsInit(pdlInitxmlRootId);
    PDL_CHECK_STATUS(rc);

    rc = pdlInterfaceInit();
    PDL_CHECK_STATUS(rc);
    rc = prvPdlI2cInit();
    PDL_CHECK_STATUS(rc);
    rc = prvPdlGpioInit(initType);
    PDL_CHECK_STATUS(rc);
    rc = pdlSmiXsmiInit();
    PDL_CHECK_STATUS(rc);
    rc = prvPdlPpRegInit();
    PDL_CHECK_STATUS(rc);

    if (initType == PDL_OS_INIT_TYPE_STACK_E) {
        rc = pdlPhyInit();
        PDL_CHECK_STATUS(rc);
        rc = pdlSerdesInit();
        PDL_CHECK_STATUS(rc);
        rc = pdlPacketProcessorInit();
        PDL_CHECK_STATUS(rc);
        rc = pdlSensorInit();
        PDL_CHECK_STATUS(rc);
    }

    else {

        rc = pdlPhyInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlSerdesInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlSensorInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlPacketProcessorInit();
        PDL_CHECK_STATUS(rc);

        if (pdlFanInternalDriverBind == FALSE) {
            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksTc654, PDL_FAN_CONTROLLER_TYPE_TC654_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksAdt7476, PDL_FAN_CONTROLLER_TYPE_ADT7476_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksEMC2305, PDL_FAN_CONTROLLER_TYPE_EMC2305_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksPWM, PDL_FAN_CONTROLLER_TYPE_PWM_E);
            PDL_CHECK_STATUS(rc);
            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksNCT7802, PDL_FAN_CONTROLLER_TYPE_NCT7802_E);
            PDL_CHECK_STATUS(rc);
            pdlFanInternalDriverBind = TRUE;
        }

        rc = pdlBtnInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlFanInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlPowerInit();
        PDL_CHECK_STATUS(rc);

        rc = prvPdlOobPortInit();
        PDL_CHECK_STATUS(rc);

        rc = pdlLedInit();
        PDL_CHECK_STATUS(rc);

        rc = prvPdlI2cBusFdInit();
        PDL_CHECK_STATUS(rc);

    }

    return PDL_OK;
}
/*$ END OF pdlInit */

/**
 * @fn  void pdlDestroy ()
 *
 * @brief   release all memory allocated by Pdl
 *
 */

PDL_STATUS pdlDestroy(
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (initType == PDL_OS_INIT_TYPE_LIB_E) {
        return PDL_OK;
    }

    if (initType != PDL_OS_INIT_TYPE_STACK_E) {
        prvPdlOobDestroy();

        prvPdlI2cBusFdDestroy();

        prvPdlLedDestroy();

        prvPdlPowerDestroy();

        prvPdlBtnDestroy();

        prvPdlFanDestroy();
    }

    prvPdlPacketProcessorDestroy();

    prvPdlSensorDestroy();

    prvPdlSerdesDestroy();

    prvPdlPhyDestroy();

    prvPdlSmiXsmiDestroy();
    prvPdlGpioDestroy();
    prvPdlI2cDestroy();
    prvPdlPpRegDestroy();
    pdlInterfaceDestroy();

    pdlProjectParamsFree();

    xmlParserDestroy(&pdlInitxmlRootId);

    return PDL_OK;
}
/*$ END OF pdlDestroy */
