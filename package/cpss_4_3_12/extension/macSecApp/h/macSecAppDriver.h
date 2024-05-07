
/***************************************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                                     *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.                       *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT                      *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE                            *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.                         *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,                           *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.                       *
****************************************************************************************************
*/
/**
****************************************************************************************************
* @file macSecAppDriver.h
*
* @brief CPSS declarations relate to MAC Security (or MACsec) MKA App feature.
*
* @version   1
*****************************************************************************************************
*/

#ifndef __macSecAppDriverh
#define __macSecAppDriverh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include "macsec_types.h"

/**
 --------------------------------------------------------------------------------------------------------------------------------------------------
  CPSS MACSec MKA App data definitions
 --------------------------------------------------------------------------------------------------------------------------------------------------
 **/

/*
 * Identifier type for interface.
 */
typedef GT_UINTPTR MACSEC_APP_INTF_ID;

/*
 * Identifier type for peer.
 */
typedef GT_UINTPTR MACSEC_APP_PEER_ID;

/*
 * Identifier type for data keys.
 */
typedef GT_UINTPTR MACSEC_APP_DKEY_ID;


/*************************** Data Structures ***********************************/

/**
 * @struct: MACSEC_APP_PEER_STC
 *
 * @brief: MACSEC peer object information
 */
typedef struct
{
    /** Local vPort */
    GT_U32 localvPortId;

    /** Remote vPort */
    GT_U32 peervPortId;

    /** Address of the peer interface */
    GT_ETHERADDR    peerMacAddr;
} MACSEC_APP_PEER_STC;


/*************************** Constants definitions ***********************************/

/**
* @internal macSecSecAppPeerCreate function
* @endinternal
*
* @brief   Create a peer object in an interface
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] direction             - select Egress or Ingress MACSec transformer
* @param[in] peerDataPtr           - (pointer to) peer object.
* @param[out] peerId               - Newly created peer object id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS macSecSecAppPeerCreate
(
    IN   GT_U8                          devNum,
#if 0
    IN   GT_MACSEC_UNIT_BMP             unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT direction,
#endif
    IN   GT_U32                         macSecIntfId,
    IN   MACSEC_APP_PEER_STC            *peerDataPtr,
    OUT  MACSEC_APP_PEER_ID             *peerId
);

/**
* @internal macSecSecAppPeerGet function
* @endinternal
*
* @brief   Create a peer object in an interface
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] direction             - select Egress or Ingress MACSec transformer
* @param[in] peerDataPtr           - (pointer to) peer object.
* @param[out] peerId               - Newly created peer object id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS macSecSecAppPeerGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   MACSEC_APP_PEER_STC                *peerDataPtr,
    OUT  MACSEC_APP_PEER_ID                 *peerId
);


/**
* @internal macSecSecAppPeerDelete function
* @endinternal
*
* @brief   Destroy a peer object in an interface
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] peerId                - peer object ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS macSecSecAppPeerDelete
(
    IN   GT_U8                    devNum,
    IN   GT_MACSEC_UNIT_BMP       unitBmp,
    IN   MACSEC_APP_PEER_ID       peerId
);

/**
* @internal macSecSecAppPeerGetNext function
* @endinternal
*
* @brief   Gets the next peer object for the interface.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] prevPeerId            - peer object ID.
*                                    0 - to return firt peer object ID.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS macSecSecAppPeerGetNext
(
    IN   GT_U8                    devNum,
    IN   GT_MACSEC_UNIT_BMP       unitBmp,
    IN   MACSEC_APP_PEER_ID       prevPeerId,
    OUT  MACSEC_APP_PEER_ID      *currPeerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __macSecAppDriverh */


