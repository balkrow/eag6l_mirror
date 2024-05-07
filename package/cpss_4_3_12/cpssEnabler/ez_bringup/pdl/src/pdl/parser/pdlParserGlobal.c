#include <pdl/common/pdlTypes.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdlib/xml/private/prvXmlParserBuilder.h>
#include <pdl/parser/pdlParser.h>

/** @brief   The pdl prv database feature array[ PDL_FEATURE_ID_LAST_E ] */
static PDL_FEATURE_DATA_STC        * prvPdlFeatureDataDbPtr;

/* AUTO Generated APIs */
extern PDL_STATUS prvPdlCodeParser(XML_PARSER_NODE_DESCRIPTOR_TYP xml_root);
extern PDL_STATUS prvPdlFeaturesDataHandler(void);
extern PDL_STATUS prvPdlFeaturesDataGet(PDL_FEATURE_DATA_STC ** data_PTR);
extern PDL_STATUS prvPdlCodeDestroy(void);
/* END of AUTO Generated APIs */


/**
 * @fn  BOOLEAN pdlIsProjectFeatureSupported ( IN PDL_FEATURE_ID_ENT featureId)
 *
 * @brief   Returns whether feature is supported
 *
 * @param   featureId   - feature id
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN pdlIsProjectFeatureSupported(
    /*!     INPUTS:             */
    IN PDL_FEATURE_ID_ENT           featureId
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    BOOLEAN supported = FALSE;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/

    if (!prvPdlFeatureDataDbPtr) {
        return FALSE;
    }

    if (featureId < PDL_FEATURE_ID_LAST_E) {
        supported = prvPdlFeatureDataDbPtr[featureId].is_supported;
    }

    return supported;
}
/*$ END OF pdlIsProjectFeatureSupported */

/**
 * @fn  BOOLEAN pdlIsFieldHasValue ( IN UINT_8 fieldMask )
 *
 * @brief   Returns whether field's data was read from xml
 *
 * @param   fieldMask   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN pdlIsFieldHasValue(
    /*!     INPUTS:             */
    IN UINT_8 fieldMask
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    if (fieldMask & PDL_FIELD_HAS_VALUE_CNS) {
        return TRUE;
    }

    return FALSE;
}
/*$ END OF pdlIsFieldHasValue */

/**
 * @fn  BOOLEAN prvPdlSetFieldHasValue ( INOUT UINT_8 * fieldMaskPtr )
 *
 * @brief   Set whether field's data has data
 *
 * @param   fieldMaskPtr   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN prvPdlSetFieldHasValue(
    /*!     INPUTS:             */
    INOUT UINT_8  * fieldMaskPtr
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    if (fieldMaskPtr == NULL) {
        return FALSE;
    }

    *fieldMaskPtr |= PDL_FIELD_HAS_VALUE_CNS;

    return TRUE;
}
/*$ END OF prvPdlSetFieldHasValue */

/**
 * @fn  PDL_STATUS pdlProjectParamsInit ( IN PDL_FEATURE_ID_ENT featureId, OUT PDL_FEATURE_DATA_PARAMS_UNT *dataUntPtr)
 *
 * @brief   Returns feature's parsed data
 *
 * @param   featureId   - feature id
 * @param   dataUntPtr  - parsed data
 *
 * @return  A PDL_STATUS.
 */
extern PDL_STATUS pdlProjectParamsGet(
    /*!     INPUTS:             */
    IN PDL_FEATURE_ID_ENT            featureId,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    OUT PDL_FEATURE_DATA_PARAMS_UNT   *dataUntPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    UINT_32  size;
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    /* some tests */
    if (featureId >= PDL_FEATURE_ID_LAST_E) {
        return PDL_OUT_OF_RANGE;
    }

    if (!dataUntPtr) {
        return PDL_BAD_PTR;
    }

    if (!prvPdlFeatureDataDbPtr || !prvPdlFeatureDataDbPtr[featureId].data_PTR) {
        return PDL_NOT_INITIALIZED;
    }

    if (!prvPdlFeatureDataDbPtr[featureId].is_supported) {
        return PDL_NOT_SUPPORTED;
    }

    size = prvPdlFeatureDataDbPtr[featureId].data_size;

    if (size == 0) {
        return PDL_BAD_SIZE;
    }

    memcpy(dataUntPtr, prvPdlFeatureDataDbPtr[featureId].data_PTR, size);

    return PDL_OK;
}
/*$ END OF pdlConfParamsGet */

/**
 * @fn  PDL_STATUS pdlProjectParamsInit (IN XML_PARSER_ROOT_DESCRIPTOR_TYP pdlInitXmlRootId)
 *
 * @brief   Initialize parser component
 *
 * @param   pdlInitXmlRootId  - xml root handle.
 *
 * @return  A PDL_STATUS.
 */
extern PDL_STATUS pdlProjectParamsInit(
    /*!     INPUTS:             */
    IN XML_PARSER_ROOT_DESCRIPTOR_TYP  pdlInitXmlRootId
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                  res;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/

    /* parse XML file and store data in feature's structures */
    res = prvPdlCodeParser(pdlInitXmlRootId);
    if (res != PDL_OK) {
        return res;
    }

    /* store features data in main db */
    prvPdlFeaturesDataHandler();

    /* get main db pointer */
    res = prvPdlFeaturesDataGet(&prvPdlFeatureDataDbPtr);

    return res;
}
/*$ END OF pdlProjectParamsInit */

/**
 * @fn  PDL_STATUS pdlProjectParamsFree (void)
 *
 * @brief   Free parser component allocations
 *
 * @return  A PDL_STATUS.
 */
extern PDL_STATUS pdlProjectParamsFree(
    /*!     INPUTS:             */
    void
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS                  res;
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    /* free allocated memory */
    res = prvPdlCodeDestroy();

    return res;
}
/*$ END OF pdlProjectParamsFree */
