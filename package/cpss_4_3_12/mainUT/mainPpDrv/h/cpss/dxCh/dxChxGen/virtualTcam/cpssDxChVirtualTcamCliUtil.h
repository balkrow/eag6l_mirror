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
* @file cpssDxChVirtualTcamCliUtil.h
*
* @brief The headers of utils dedicated to test Virtual TCAM Manager using CLI.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChVirtualTcamCliUtilh
#define __cpssDxChVirtualTcamCliUtilh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get CPSS Common Driver facility types definitions */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

/*typedef const char *ENUM_STR;*/

void cliWrapCpssDxChVirtualTcamHelp(void);

GT_STATUS cliWrapCpssDxChVirtualTcamManagerCreate
(
    IN GT_U32   vTcamMngId,
    IN GT_BOOL  haSupportEnabled
);

GT_STATUS cliWrapCpssDxChVirtualTcamManagerDevPortGroupBmpAdd
(
    IN GT_U32             vTcamMngId,
    IN GT_U8              devNum,
    IN GT_PORT_GROUPS_BMP portGroupsBmp
);

GT_STATUS cliWrapCpssDxChVirtualTcamManagerDevPortGroupBmpRemove
(
    IN GT_U32             vTcamMngId,
    IN GT_U8              devNum,
    IN GT_PORT_GROUPS_BMP portGroupsBmp,
    IN GT_BOOL            invalidateRulesInRemoved
);

GT_STATUS cliWrapCpssDxChVirtualTcamManagerDevAdd
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devNum
);

GT_STATUS cliWrapCpssDxChVirtualTcamManagerDevRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devNum
);

GT_STATUS cliWrapCpssDxChVirtualTcamAppAllocationSet
(
    IN  GT_U32 entryIndex, /* size of array set to (entryIndex + 1), 0xFFFFFFFF - set to 0 */
    IN  GT_U32 startRowIndex,
    IN  GT_U32 numOfRows,
    IN  GT_U32 columnsBitmap
);

GT_STATUS cliWrapCpssDxChVirtualTcamCreate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    clientGroup,
    IN  GT_U32    hitNumber,
    IN  const char *ruleSize,
    IN  GT_U32    tcamSpaceUnmovable, /*CPSS_DXCH_VIRTUAL_TCAM_UNMOVABLE_TYPE_ENT*/
    IN  GT_BOOL   autoResize,
    IN  GT_U32    guaranteedNumOfRules,
    IN  const char *ruleAdditionMethod
);

GT_STATUS cliWrapCpssDxChVirtualTcamInfoGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId
);

GT_STATUS cliWrapCpssDxChVirtualTcamUsageGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleWrite
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_U32    priorityBase,
    IN  GT_32     priorityInc,
    IN  const char *ruleType,
    IN  GT_U32    ruleStampBase,
    IN  GT_32     ruleStampInc,
    IN  GT_U32    actionStampBase,
    IN  GT_32     actionStampInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleRead
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  const char *ruleType
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleCheck
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  const char *ruleType,
    IN  GT_U32    ruleStampBase,
    IN  GT_32     ruleStampInc,
    IN  GT_U32    actionStampBase,
    IN  GT_32     actionStampInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamActionUpdate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  const char *ruleType,
    IN  GT_U32    actionStampBase,
    IN  GT_32     actionStampInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamActionGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  const char *ruleType
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleDelete
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleMove
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    srcRuleIdBase,
    IN  GT_32     srcRuleIdInc,
    IN  GT_U32    dstRuleIdBase,
    IN  GT_32     dstRuleIdInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamRulePriorityUpdate
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_U32    priorityBase,
    IN  GT_32     priorityInc,
    IN  const char *rulePosition
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleValidStatusSet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc,
    IN  GT_BOOL   validStatus
);

GT_STATUS cliWrapCpssDxChVirtualTcamRuleValidStatusGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamNextRuleIdGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    ruleId,
    IN  GT_U32    numOfRules
);

GT_STATUS cliWrapCpssDxChVirtualTcamRulePriorityGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamDbRuleIdToHwIndexConvert
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId,
    IN  GT_U32    numOfRules,
    IN  GT_U32    ruleIdBase,
    IN  GT_32     ruleIdInc
);

GT_STATUS cliWrapCpssDxChVirtualTcamSpaceLayoutGet
(
    IN  GT_U32    vTcamMngId,
    IN  GT_U32    vTcamId
);

GT_STATUS cliWrapCpssDxChVirtualTcamCleanupVtcamRules
(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
);

GT_STATUS cliWrapCpssDxChVirtualTcamCleanupVtmgrVtcams
(
    IN GT_U32   vTcamMngId
);

GT_STATUS cliWrapCpssDxChVirtualTcamCleanupVtmgrRemove
(
    IN GT_U32   vTcamMngId
);

GT_STATUS cliWrapCpssDxChVirtualTcamCleanupAll
(
    GT_VOID
);

GT_STATUS cliWrapCpssDxChVirtualTcamResetSip5PhysicalTcam
(
    GT_U32 devNum
);

GT_STATUS cliWrapPrvCpssSystemRecoveryStateUpdate
(
    IN const char *sysRecoveryState,
    IN const char *sysRecoveryProcess,
    IN const char *sysRecoveryHaState
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVirtualTcamCliUtilh */

