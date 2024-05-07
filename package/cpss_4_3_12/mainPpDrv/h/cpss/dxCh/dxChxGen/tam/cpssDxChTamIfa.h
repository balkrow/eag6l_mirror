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
* @file cpssDxChTamIfa.h
*
* @brief CPSS definitions for configuring, gathering info and statistics
*        for the TAM IFA (Telemetry Analytics and Monitoring Inband Flow Analyzer)
*        feature
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChTamIfah
#define __cpssDxChTamIfah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/** Maximum number of IFA flows in the system */
#define CPSS_DXCH_MAX_IFA_FLOWS_INDEX_CNS      512

/**
* @enum CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_COUNT_MODE_ENT
 *
 * @brief Enumeration of TAM IFA Egress Port Drop Counting modes.
*/
typedef enum{

    /** Byte based drop counting mode. */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_BYTE_BASED_DROP_COUNTING_MODE_E,

    /** Packet based drop counting mode. */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_PACKET_BASED_DROP_COUNTING_MODE_E

} CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_BYTE_MODE_ENT
 *
 * @brief Enumeration of TAM IFA Egress Port Drop Byte modes.
*/
typedef enum{

    /** L2 based drop byte mode. */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_L2_BASED_DROP_BYTE_MODE_E,

    /** L3 based drop byte mode. */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_L3_BASED_DROP_BYTE_MODE_E

} CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_BYTE_MODE_ENT;

/**
 * @struct CPSS_DXCH_IFA_PARAMS_STC
 *
 * @brief This structure defines the fields to handle IFA Ipv4/ipv6 packets in the
 *  SCPU
 *
*/
typedef struct{

    /** @brief Egress Port drop counting mode */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_COUNT_MODE_ENT  egressPortDropCountingMode;

    /** @brief Egress Port drop byte mode not relevant in packet based mode */
    CPSS_DXCH_TAM_IFA_EGRESS_PORT_DROP_BYTE_MODE_ENT   egressPortDropByteMode;

    /** @brief used by SCPU to calculate the egress port utilization in seconds , default is 1min */
    GT_U32  samplePeriod;

}CPSS_DXCH_IFA_PARAMS_STC;

/**
 * @struct CPSS_DXCH_IFA_PORT_CFG_STC
 *
 * @brief This structure defines per port IFA configuration details
 * required by SCPU for polling
 *
*/
typedef struct{

    /** @brief Physical Port Number */
    GT_PHYSICAL_PORT_NUM portNum;

    /** @brief Port Speed */
    CPSS_PORT_SPEED_ENT  speed;

    /** @brief CNC Unit number */
    GT_U32  cncBlockNum;

    /** @brief CNC Base Entry number inside the unit */
    GT_U32  cncBaseEntryNum;

    /** @brief Number of CNC counter indexes
     *  for example : for TxQ CNC client it should denote number of TX queue
     *  per port */
    GT_U32  numOfCncCounter;

}CPSS_DXCH_IFA_PORT_CFG_STC;

/* @internal cpssDxChIfaGlobalConfigSet function
* @endinternal
*
* @brief   Set global IFA parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] ifaParamsPtr          - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaGlobalConfigSet
(
    IN  GT_U8                     devNum,
    IN  CPSS_DXCH_IFA_PARAMS_STC  *ifaParamsPtr
);

/* @internal cpssDxChIfaGlobalConfigGet function
* @endinternal
*
* @brief  Get global IFA parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[out] ifaParamsPtr         - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaGlobalConfigGet
(
    IN   GT_U8                     devNum,
    OUT  CPSS_DXCH_IFA_PARAMS_STC  *ifaParamsPtr
);

/* @internal cpssDxChIfaPortConfigSet function
* @endinternal
*
* @brief   Set IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] portCfgArraySize      - count of physical ports array indexes.
* @param[in] portCfgArr            - list of per port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - portCfgArr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaPortConfigSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portCfgArraySize,
    IN  CPSS_DXCH_IFA_PORT_CFG_STC portCfgArr[] /*arraySize=portCfgArraySize*/
);

/* @internal cpssDxChIfaPortConfigGet function
* @endinternal
*
* @brief  Get IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in]  portNum              - physical port number.
* @param[out] portCfgPtr           - (pointer to) port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaPortConfigGet
(
    IN   GT_U8                      devNum,
    IN   GT_PHYSICAL_PORT_NUM       portNum,
    OUT  CPSS_DXCH_IFA_PORT_CFG_STC *portCfgPtr
);

/**
* @internal cpssDxChIfaEnableSet function
* @endinternal
*
* @brief Enable/Disable IFA
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum       - device number
* @param[in] ifaEnable    - GT_TRUE - enable
*                           GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on DB not found.
*/
GT_STATUS cpssDxChIfaEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ifaEnable
);

/**
* @internal cpssDxChIfaEnableGet function
* @endinternal
*
* @brief Get IFA Enable/Disable status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum       - device number
* @param[out] ifaEnablePtr - (pointer to) IFA enable status
*                            GT_TRUE - enable
*                            GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - on DB not found.
*/
GT_STATUS cpssDxChIfaEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *ifaEnablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTamIfah */

