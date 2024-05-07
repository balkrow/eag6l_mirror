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
* @file prvCpssDxChPortDpAas.h
*
* @brief  CPSS AQas Data Path Pizza and resource configuration.
*
* @version   0
********************************************************************************
*/
#ifndef __prvCpssDxChPortDpAas_h
#define __prvCpssDxChPortDpAas_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>

/**
 * @struct PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC
 *
 *  @brief This structure contains port mapping info.
 */
typedef struct
{
    /** global datapath index */
    GT_U32  dataPath;
    /* GT_TRUE - PCA, MIF, MAC; GT_FALSE - SDMA, Loopback */
    GT_BOOL isMac;
    /* GT_TRUE - port Preemtion Enabled (pair of DMAs), GT_FALSE - regular (one DMA) */
    GT_BOOL isPreemtionEnabled;
    /* global MAC number */
    GT_U32  globalMac;
    /* local MAC number in Datapath */
    GT_U32  localMac;
    /* global DMA number (if pair of DMA - for express) */
    GT_U32  globalDma;
    /* local DMA number in Datapath (if pair of DMA - for express) */
    GT_U32  localDma;

    /* channel number used for lookup on PcaArbiterChannelResources[]
     * For PCA Loopback it may be different from localDma="LOOPBACK channel".
     */
    GT_U32  resourceChannel;
    /* portNum used for RxDma channel config.
     * For PCA Loopback may be different from original portNum.
     */
    GT_U32  portNum;

    /* MIF unit index - global as in Registers DB */
    GT_U32  mifUnit;
    /* local Channel in MIF unit (if pair -  - for express)*/
    GT_U32  localMif;
    /* global DMA number of preeptive channel */
    GT_U32  globalDmaPreemtive;
    /* local DMA number of preeptive channel */
    GT_U32  localDmaPreemtive;
    /* local Channel in MIF unit for preemtive channel */
    GT_U32  localMifPreemtive;
    /* Speed im megabit per second */
    GT_U32  speedInMbPerSec;
    /* Num of serdes used by Mac */
    GT_U32  numOfSerdes;
} PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC;

/**
* @internal prvCpssDxChPortAasMacToMif function
* @endinternal
*
* @brief    Convert Local port to MIF unit and channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] dataPath               - PCA data path (1..3)
* @param[in] localMacInDp           - local MAC number (in DP)
* @param[in] numOfSerdes            - num of serdes (as port mode)
* @param[in] preemptionRole         - Preemption Role (regular, express, preemptive)
* @param[out] mifUnitNumPtr         - (pointer to)MIF unit number (global)
* @param[out] mufLocalChannelNumPtr - (pointer to)MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_NOT FOUND             - on not mapable MAC unit
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChPortAasMacToMif
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    dataPath,
    IN  GT_U32                                    localMacInDp,
    IN  GT_U32                                    numOfSerdes,
    IN  PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionRole,
    OUT GT_U32                                    *mufUnitNumPtr, /*mif unit index in regs DB*/
    OUT GT_U32                                    *mufLocalChannelNumPtr
);

/**
* @internal prvCpssDxChPortAasPortInfoGet function
* @endinternal
*
* @brief   Gets port info.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
* @param[out] portInfoPtr           - (pointer to)  port info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortAasPortInfoGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     portSpeed,
    IN  GT_BOOL                                 isLoopback,
    OUT PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC *portInfoPtr
);

/**
* @internal prvCpssDxChPortDpAasDeviceInit function
* @endinternal
*
* @brief    Initialyze AAS DP units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpAasDeviceInit
(
    IN    GT_U8                                devNum
);

/**
* @internal prvCpssDxChPortDpAasPortConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
);

/**
* @internal prvCpssDxChPortDpAasPortDownForce function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*           Called also at configure fail - when ifMode and portSpeed not in CPSS DB.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - port interface mode
* @param[in] portSpeed              - port speed enum
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortDownForce
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
);

/**
* @internal prvCpssDxChPortDpAasPortDown function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
);

/**
* @internal prvCpssDxChPortDpAasPortMifPfcEnableSet function
* @endinternal
*
* @brief  Set MIF PFC enable/disable
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[in] enableTx              - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx              - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpAasPortMifPfcEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enableTx,
    IN  GT_BOOL                 enableRx
);

/**
* @internal prvCpssDxChPortDpAasPortMifPfcEnableGet function
* @endinternal
*
* @brief  Get MIF PFC enable/disable
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpAasPortMifPfcEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enableTxPtr,
    OUT GT_BOOL                 *enableRxPtr
);

/**
* @internal prvCpssDxChPortDpAasSffStatisticsConfigure function
* @endinternal
*
* @brief    Configure SFF Statistics parameters.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitlNum               - unit number
* @param[in] enable                 - GT_TRUE - enable, GT_FALSE - disable
* @param[in] sopNotEop              - GT_TRUE - Start of Packet, GT_FALSE - End of Packet
* @param[in] selectedChannelNum     - selected channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChPortDpAasSffStatisticsConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_BOOL                              enable,
    IN    GT_BOOL                              sopNotEop,
    IN    GT_U32                               selectedChannelNum
);

/**
* @internal prvCpssDxChPortDpAasSpeedResourceCheck function
* @endinternal
*
* @brief    Check DMA Channel available for requested speed.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] resourcePortNum        - resource physical port
* @param[in] ifMode                 - Interface mode
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - not enough resources or busy
*/
GT_STATUS prvCpssDxChPortDpAasSpeedResourceCheck
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    GT_PHYSICAL_PORT_NUM                 resourcePortNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__prvCpssDxChPortDpAas_h*/

