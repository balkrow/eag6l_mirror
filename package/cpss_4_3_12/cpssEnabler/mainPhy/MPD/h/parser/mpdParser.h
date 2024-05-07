#ifndef __MPD_PARSER_HEADER_FILE__
#define __MPD_PARSER_HEADER_FILE__

#include <mpdPrefix.h>
#include <pdlib/xml/xmlParser.h>
#include <parser/auto/mpdParser.h>

/**
 * @fn  BOOLEAN mpdIsProjectFeatureSupported ( IN MPD_FEATURE_ID_ENT featureId)
 *
 * @brief   Returns whether feature is supported
 *
 * @param [in]  featureId   - feature id
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdIsProjectFeatureSupported(
    /*!     INPUTS:             */
    IN MPD_FEATURE_ID_ENT           featureId
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

/**
 * @fn  MPD_RESULT_ENT mpdProjectParamsGet ( IN MPD_FEATURE_ID_ENT featureId, OUT MPD_FEATURE_DATA_PARAMS_UNT *dataUntPtr)
 *
 * @brief   Returns feature's parsed data
 *
 * @param [in]  featureId   - feature id
 * @param [out]  dataUntPtr  - parsed data
 *
 * @return  A MPD_RESULT_ENT.
 */
extern MPD_RESULT_ENT mpdProjectParamsGet(
    /*!     INPUTS:             */
    IN MPD_FEATURE_ID_ENT            featureId,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    OUT MPD_FEATURE_DATA_PARAMS_UNT   *dataUntPtr
);

/**
 * @fn  BOOLEAN mpdIsFieldHasValue ( IN UINT_8 fieldMask )
 *
 * @brief   Returns whether field's data was read from xml
 *
 * @param [in] fieldMask   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdIsFieldHasValue(
    /*!     INPUTS:             */
    IN UINT_8 fieldMask
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF mpdIsFieldHasValue */

/**
 * @fn  BOOLEAN mpdSetFieldHasValue ( IN UINT_8 * fieldMaskPtr )
 *
 * @brief   Set whether field's data has data
 *
 * @param [in,out]  fieldMaskPtr   - field's mask value
 *
 * @return  TRUE if feature is supported
 *          FALSE if feature is not supported
 */
extern BOOLEAN mpdSetFieldHasValue(
    /*!     INPUTS:             */
    INOUT UINT_8  * fieldMaskPtr
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF mpdSetFieldHasValue */

/**
 * @fn  MPD_RESULT_ENT mpdProjectParamsInit (IN XML_PARSER_ROOT_DESCRIPTOR_TYP mpdInitxmlRootId)
 *
 * @brief   Initialize parser component
 *
 * @param [in]   mpdInitxmlRootId  - xml root handle.
 *
 * @return  A MPD_RESULT_ENT.
 */
extern MPD_RESULT_ENT mpdProjectParamsInit(
    /*!     INPUTS:             */
    IN XML_PARSER_ROOT_DESCRIPTOR_TYP  mpdInitxmlRootId
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

/**
 * @fn  MPD_RESULT_ENT mpdProjectParamsFree (void)
 *
 * @brief   Free parser component allocations
 *
 * @return  A MPD_RESULT_ENT.
 */
extern MPD_RESULT_ENT mpdProjectParamsFree(
    /*!     INPUTS:             */
    void
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT mpdEnum2StrConvert(
    MPD_ENUM_IDS_ENT enum_type,
    UINT_32 int_value,
    char ** str_value_PTR
);

extern MPD_RESULT_ENT mpdStr2EnumConvert(
    MPD_ENUM_IDS_ENT enum_type,
    char * str_value_PTR,
    UINT_32 * int_value_PTR
);

extern MPD_RESULT_ENT mpdEnum2StrEntryGet(
    MPD_ENUM_IDS_ENT enum_type,
    UINT_32 int_value,
    MPD_ENUM_STR2VALUE_STC ** entry_PTR
);

#endif/* #ifndef __MPD_PARSER_HEADER_FILE__ */
