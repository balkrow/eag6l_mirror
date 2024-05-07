#ifndef __PDL_PARSER_HEADER_FILE__
#define __PDL_PARSER_HEADER_FILE__

#include <pdlib/xml/xmlParser.h>
#include <pdl/parser/auto/pdlParser.h>

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
);


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
);

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
);
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
);
/*$ END OF prvPdlSetFieldHasValue */

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
);

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
);

extern PDL_STATUS pdlEnum2StrConvert(
    PDL_ENUM_IDS_ENT enum_type,
    UINT_32 int_value,
    char ** str_value_PTR
);

extern PDL_STATUS pdlStr2EnumConvert(
    PDL_ENUM_IDS_ENT enum_type,
    char * str_value_PTR,
    UINT_32 * int_value_PTR
);

extern PDL_STATUS pdlEnum2StrEntryGet(
    PDL_ENUM_IDS_ENT enum_type,
    UINT_32 int_value,
    PDL_ENUM_STR2VALUE_STC ** entry_PTR
);

#endif/* #ifndef __PDL_PARSER_HEADER_FILE__ */
