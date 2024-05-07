/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChSseParser.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChSseParser_H
#define __prvCpssDxChSseParser_H

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_JSON_TOKENS         (200)


/**************************** APIs decleration***********************************
*/

/**
* @internal  CpssDxChSseParserInit function
* @endinternal
*
* @brief  Initialize Sub Seqence Parser
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS prvCpssDxChSseParserInit
(
    IN GT_BOOL enablePrints
);

/**
* @internal  CpssDxChSseParserDestroy function
* @endinternal
*
* @brief  Free Sub Seqence Parser internal resources
*
*/
GT_VOID prvCpssDxChSseParserDestroy
(
    IN GT_VOID
);

/**
* @internal  prvCpssDxChSseParserScriptParse function
* @endinternal
*
* @brief  Parse json script file
*
*  @param[in] scriptFilePath     - full path of script file
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS prvCpssDxChSseParserScriptParse
(
    GT_CHAR_PTR scriptFilePath
);

/**
* @internal  prvCpssDxChSseParserPrintsEnable function
* @endinternal
*
* @brief  Enable/disables SSE prints for debugging
*
*  @param[in] enablePrints     - Indicates whether the
*                                prints should be enabled
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS prvCpssDxChSseParserPrintsEnable
(
    IN  GT_BOOL enablePrints
);

/**
* @internal  prvCpssDxChSseParserPrintsEnableStatusGet function
* @endinternal
*
* @brief  returns SSE prints (for debugging) status
*
*  @param[in] *enablePrints     - pointer that indicates whether
*                                the prints are enabled
*
* @retval none
*/
GT_VOID prvCpssDxChSseParserPrintsEnableStatusGet
(
    OUT  GT_BOOL *enablePrints
);

/******************************************************************************
*                       Sanity Test section                                   *
*******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* __prvCpssDxChSseParser_H */


