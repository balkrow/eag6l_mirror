#include <mpdPrefix.h>
#include <mpdTypes.h>
#include <mpd.h>
#include <mpdPrv.h>
#include <pdlib/lib/pdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdlib/xml/private/prvXmlParserBuilder.h>
#include <parser/mpdParser.h>

/* AUTO Generated APIs */
extern MPD_RESULT_ENT prvMpdCodeParser(XML_PARSER_NODE_DESCRIPTOR_TYP xml_root);
extern MPD_RESULT_ENT prvMpdFeaturesDataHandler(void);
extern MPD_RESULT_ENT prvMpdFeaturesDataGet(MPD_FEATURE_DATA_STC ** data_PTR);
extern MPD_RESULT_ENT prvMpdCodeDestroy(void);
/* END of AUTO Generated APIs */



/**
 * @fn  BOOLEAN mpdIsProjectFeatureSupported ( IN MPD_FEATURE_ID_ENT featureId)
 *
 * @brief   Returns whether feature is supported
 *
 * @param   featureId   - feature id
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdIsProjectFeatureSupported(
    /*!     INPUTS:             */
    IN MPD_FEATURE_ID_ENT           featureId
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

    if (featureId < MPD_FEATURE_ID_LAST_E) {
        supported = PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_ACCESS_MAC()[featureId].is_supported;
    }

    return supported;
}
/*$ END OF mpdIsProjectFeatureSupported */

/**
 * @fn  BOOLEAN mpdIsFieldHasValue ( IN UINT_8 fieldMask )
 *
 * @brief   Returns whether field's data was read from xml
 *
 * @param   fieldMask   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdIsFieldHasValue(
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

    if (fieldMask & MPD_FIELD_HAS_VALUE_CNS) {
        return TRUE;
    }

    return FALSE;
}
/*$ END OF mpdIsFieldHasValue */

/**
 * @fn  BOOLEAN mpdSetFieldHasValue ( INOUT UINT_8 * fieldMaskPtr )
 *
 * @brief   Set whether field's data has data
 *
 * @param   fieldMaskPtr   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdSetFieldHasValue(
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

    *fieldMaskPtr |= MPD_FIELD_HAS_VALUE_CNS;

    return TRUE;
}
/*$ END OF mpdSetFieldHasValue */

/**
 * @fn  MPD_STATUS mpdProjectParamsGet ( IN MPD_FEATURE_ID_ENT featureId, OUT MPD_FEATURE_DATA_PARAMS_UNT *dataUntPtr)
 *
 * @brief   Returns feature's parsed data
 *
 * @param   featureId   - feature id
 * @param   dataUntPtr  - parsed data
 *
 * @return  A MPD_STATUS.
 */
extern MPD_RESULT_ENT mpdProjectParamsGet(
    /*!     INPUTS:             */
    IN MPD_FEATURE_ID_ENT            featureId,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    OUT MPD_FEATURE_DATA_PARAMS_UNT   *dataUntPtr
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
    if (featureId >= MPD_FEATURE_ID_LAST_E) {
        return MPD_OUT_OF_RANGE_E;
    }

    if (!dataUntPtr) {
        return MPD_OP_FAILED_E;
    }

    if (!PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_ACCESS_MAC()[featureId].is_supported) {
        return MPD_NOT_SUPPORTED_E;
    }

    size = PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_ACCESS_MAC()[featureId].data_size;

    if (size == 0) {
        return MPD_OP_FAILED_E;
    }

    memcpy(dataUntPtr, PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_ACCESS_MAC()[featureId].data_PTR, size);

    return MPD_OK_E;
}
/*$ END OF mpdProjectParamsGet */

/**
 * @fn  MPD_STATUS mpdProjectParamsInit (IN XML_PARSER_ROOT_DESCRIPTOR_TYP mpdInitXmlRootId)
 *
 * @brief   Initialize parser component
 *
 * @param[in]   mpdInitXmlRootId  - xml root handle.
 *
 * @return  A MPD_STATUS.
 */
extern MPD_RESULT_ENT mpdProjectParamsInit(
    /*!     INPUTS:             */
    IN XML_PARSER_ROOT_DESCRIPTOR_TYP  mpdInitXmlRootId
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    MPD_RESULT_ENT                  res;
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    /* parse XML file and store data in feature's structures */
    res = prvMpdCodeParser(mpdInitXmlRootId);
    if (res != MPD_OK_E) {
        return res;
    }

    /* store features data in main db */
    prvMpdFeaturesDataHandler();

    /* get main db pointer */
    res = prvMpdFeaturesDataGet((MPD_FEATURE_DATA_STC **)&(PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_VOID_ACCESS_MAC()));

    return res;
}
/*$ END OF mpdProjectParamsInit */

/**
 * @fn  MPD_STATUS mpdProjectParamsFree (void)
 *
 * @brief   Free parser component allocations
 *
 * @return  A MPD_STATUS.
 */
extern MPD_RESULT_ENT mpdProjectParamsFree(
    /*!     INPUTS:             */
    void
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    MPD_RESULT_ENT                  res;
    /*!*************************************************************************/
    /*!                      F U N C T I O N   L O G I C                       */
    /*!*************************************************************************/

    /* free allocated memory */
    res = prvMpdCodeDestroy();

    return res;
}
/*$ END OF mpdProjectParamsFree */
