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
* @file cpssDxChSseIf.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChSseIf_H
#define __cpssDxChSseIf_H

#include <cpss/common/labServices/port/gop/port/sse/mvHwsSseCommon.h>


#ifdef __cplusplus
extern "C" {
#endif

/**************************** APIs decleration***********************************
*/

/**
* @internal cpssDxChSseIfInit function
* @endinternal
*
* @brief  Initialize Sub Sequence Engine
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS cpssDxChSseIfInit
(
    GT_VOID
);

/**
* @internal cpssDxChSseIfDestroy function
* @endinternal
*
* @brief  Free all Sub Sequence Engine resources
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS cpssDxChSseIfDestroy
(
    GT_VOID
);

/**
* @internal cpssDxChSseLoadIfScript function
* @endinternal
*
* @brief  Load, and activate Sub Sequence Parser
*
* @param[in] devNum               - physical device number
* @param[in] scriptFilePath       - indicates a script file name
*                                   in a full path manner
* @retval 0                       - on success
* @retval 1                       - on error (on syntax error)
*
*/
GT_STATUS cpssDxChSseLoadIfScript
(
    IN  GT_U8    devNum,
    GT_CHAR_PTR  scriptFilePath
);

/**
* @internal cpssDxChSseSeqDelete function
* @endinternal
*
* @brief  Delete all sequences and key attributes from all LUTs
*
* @retval None
*
*/
GT_STATUS cpssDxChSseSeqDelete
(
     GT_VOID
);

/**
* @internal cpssDxChSsePrintsEnable function
* @endinternal
*
* @brief  Load, and activate Sub Sequence Parser
*
* @param[in] enablePrints         - enables/disables debug
*                                   prints
* @retval 0                       - on success
* @retval 1                       - on error (on syntax error)
*
*/
GT_STATUS cpssDxChSsePrintsEnable
(
     IN GT_BOOL enablePrints
);


#ifdef __cplusplus
}
#endif
#endif /* __cpssDxChSseIf_H */


