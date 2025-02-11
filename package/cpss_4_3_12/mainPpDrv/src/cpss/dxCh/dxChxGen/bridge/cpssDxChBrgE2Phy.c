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
* @file cpssDxChBrgE2Phy.c
*
* @brief Eport to Physical port tables facility CPSS DxCh implementation.
*
*
* @version   15
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>

/**
* @internal internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
static GT_STATUS internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwFormat = 0;
    GT_U32    hwDev, hwPort;
    GT_U32    vidx;
    GT_U32    numOfBits;
    GT_U32    numOfBits_phyPort;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(physicalInfoPtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    numOfBits =
        PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 24 :
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 23 : 22;
    numOfBits_phyPort = numOfBits - 14;

    switch (physicalInfoPtr->type)
    {
        case CPSS_INTERFACE_PORT_E:

            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                   physicalInfoPtr->devPort.portNum);

            hwDev =  PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                         physicalInfoPtr->devPort.portNum);
            hwPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(physicalInfoPtr->devPort.hwDevNum,
                                                          physicalInfoPtr->devPort.portNum);

            if((hwPort >= (GT_U32)(1 << numOfBits_phyPort)) ||
               (hwDev > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)))
            {
                /* 8 bits for the physical port number and 10 bits for device number */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,0); /* target is not trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,0); /* target not VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,10,hwDev);
            U32_SET_FIELD_MAC(hwFormat,14, numOfBits_phyPort, hwPort);

            break;
         case CPSS_INTERFACE_TRUNK_E:
            if(physicalInfoPtr->trunkId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                /* 12 bits for the trunkId */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,1); /* target is trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,0); /* target not VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,12,physicalInfoPtr->trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            if(physicalInfoPtr->type == CPSS_INTERFACE_VID_E)
            {
                vidx = 0xFFF;
            }
            else
            {
                vidx = physicalInfoPtr->vidx;

                /* AC5P and above device have 12K VIDX entries.
                   But only 4K may be indexed by this table. */
                if ((vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum)) ||
                    (vidx > 0xFFF))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            /* build value to set to hw */
            U32_SET_FIELD_MAC(hwFormat,2,1,0); /* target is not trunk */
            U32_SET_FIELD_MAC(hwFormat,3,1,1); /* target is VIDX */
            U32_SET_FIELD_MAC(hwFormat,4,12,vidx);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* MTU Index was added by a separate API: cpssDxChBrgGenMtuPortProfileIdxSet */
    /* we not change the bits 0,1 <MTU Index> */
    hwFormat >>= 2;

    /* set ePort-to-physical-port-target-mapping table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                     2,
                                     numOfBits - 2,
                                     hwFormat);
    return rc;
}

/**
* @internal cpssDxChBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportToPhysicalPortTargetMappingTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, physicalInfoPtr));

    rc = internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, portNum, physicalInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, physicalInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
static GT_STATUS internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_INTERFACE_INFO_STC             *physicalInfoPtr
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32    hwFormat;
    GT_U32    value;
    GT_U32    numOfBits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(physicalInfoPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    numOfBits =
        PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 24 :
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 23 : 22;

    /* get ePort-to-physical-port-target-mapping table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                     0,
                                     numOfBits,
                                     &hwFormat);
    if(rc != GT_OK)
        return rc;

    if (U32_GET_FIELD_MAC(hwFormat,3,1) == 0) /* target is not VIDX */
    {
        if (U32_GET_FIELD_MAC(hwFormat,2,1) == 0) /* target is not trunk */
        {
            physicalInfoPtr->devPort.hwDevNum  = U32_GET_FIELD_MAC(hwFormat, 4,10);
            physicalInfoPtr->devPort.portNum = U32_GET_FIELD_MAC(hwFormat,14, numOfBits-14);

            physicalInfoPtr->type = CPSS_INTERFACE_PORT_E;
        }
        else /* target is trunk */
        {

            physicalInfoPtr->trunkId = (GT_TRUNK_ID)U32_GET_FIELD_MAC(hwFormat,4,12);
            physicalInfoPtr->type    = CPSS_INTERFACE_TRUNK_E;
        }
    }
    else /* target is VIDX */
    {
        value = U32_GET_FIELD_MAC(hwFormat,4,12);
        if(value == 0xFFF)
        {
            physicalInfoPtr->type = CPSS_INTERFACE_VID_E;
        }
        else
        {
            physicalInfoPtr->vidx = value;
            physicalInfoPtr->type = CPSS_INTERFACE_VIDX_E;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgEportToPhysicalPortTargetMappingTableGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_INTERFACE_INFO_STC             *physicalInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportToPhysicalPortTargetMappingTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, physicalInfoPtr));

    rc = internal_cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, physicalInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, physicalInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] egressInfoPtr            - (pointer to) physical information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   *egressInfoPtr
)
{
    GT_STATUS rc;
    GT_U32    hwDataArr[4]; /* to hold entry fields that need to be updated */
    GT_U32    hwMaskArr[4]; /* mask for entry's fields in hwDataArr */
    GT_U32    value;        /*field value*/
    const PRV_CPSS_ENTRY_FORMAT_TABLE_STC * fieldsInfoPtr = NULL; /* pointer to HW table's fields info */
    SIP5_HA_EPORT_TABLE_1_FIELDS_ENT fieldId;               /* field id */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(egressInfoPtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_PP_MAC(devNum)->l2NatSupported == GT_TRUE)
    {
        if((egressInfoPtr->l2NatEnable == GT_TRUE) &&
           ((egressInfoPtr->tunnelStart == GT_TRUE) || (egressInfoPtr->modifyMacDa == GT_TRUE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "If l2Nat is enabled, tunnelStart and modifyMacDa must be disable \n");
        }
    }

    cpssOsMemSet(hwMaskArr, 0, sizeof(hwMaskArr));
    fieldsInfoPtr = PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr;
    fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E;

    value   = BOOL2BIT_MAC(egressInfoPtr->forceArpTsPortDecision);
    U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

    fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E;
    value   = BOOL2BIT_MAC(egressInfoPtr->tunnelStart);
    U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

    if (egressInfoPtr->tunnelStart == GT_TRUE)
    {
        if(egressInfoPtr->tunnelStartPtr >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tunnelStartPtr[%d] >= max [%d] \n",
                egressInfoPtr->tunnelStartPtr ,
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart);
        }

        fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E;
        value   = egressInfoPtr->tunnelStartPtr;
        U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

        switch(egressInfoPtr->tsPassengerPacketType)
        {
            case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                value = 0;
                break;
            case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E;
        U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

        /* Disable MAC DA when TS is enabled */
        fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E;
        value   = 0;
        U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);
    }
    else
    {

        if((PRV_CPSS_PP_MAC(devNum)->l2NatSupported == GT_TRUE) && (egressInfoPtr->l2NatEnable == GT_TRUE))
        {
            if(egressInfoPtr->natPtr >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "natPtr[%d] >= max [%d] \n",
                    egressInfoPtr->natPtr ,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart);
            }
            fieldId = SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E;
            value   = egressInfoPtr->l2NatEnable;
            U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

            fieldId = SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E;
            value   = egressInfoPtr->natPtr;
            U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);
        }
        else
        {
            if(PRV_CPSS_PP_MAC(devNum)->l2NatSupported == GT_TRUE)
            {
                fieldId = SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E;
                value   = 0;
                U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);
            }
            if(egressInfoPtr->arpPtr >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "arpPtr[%d] >= max [%d] \n",
                    egressInfoPtr->arpPtr ,
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp);
            }

            fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E;
            value   = egressInfoPtr->arpPtr;
            U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);

            fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E;
            value   = BOOL2BIT_MAC(egressInfoPtr->modifyMacDa);
            U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);
        }

        fieldId = SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E;
        value   = BOOL2BIT_MAC(egressInfoPtr->modifyMacSa);
        U32_SET_FIELD_AND_MASK_BY_ID_MAC(hwDataArr, hwMaskArr, fieldsInfoPtr, fieldId, value);
    }

    /* Write HA Egress ePort 1 table (after update) */
    rc = prvCpssDxChWriteTableEntryMasked(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   hwDataArr,
                                   hwMaskArr);
    return rc;
}

/**
* @internal cpssDxChBrgEportToPhysicalPortEgressPortInfoSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] egressInfoPtr            - (pointer to) physical information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   *egressInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportToPhysicalPortEgressPortInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egressInfoPtr));

    rc = internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, portNum, egressInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egressInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] egressInfoPtr            - (pointer to) physical Information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  *egressInfoPtr
)
{
    GT_STATUS rc;
    GT_U32    hwDataArr[4];/* need to hold 'read entry' */
    GT_U32    value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(egressInfoPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* read HA Egress ePort 1 table  */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   hwDataArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    egressInfoPtr->l2NatEnable = GT_FALSE;
    egressInfoPtr->modifyMacDa = GT_FALSE;
    egressInfoPtr->modifyMacSa = GT_FALSE;

    U32_GET_FIELD_BY_ID_MAC(hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
        SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E,
        value);
    egressInfoPtr->forceArpTsPortDecision = BIT2BOOL_MAC(value);

    U32_GET_FIELD_BY_ID_MAC(hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
        SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E,
        value);
    egressInfoPtr->tunnelStart = BIT2BOOL_MAC(value);

    if (egressInfoPtr->tunnelStart == GT_TRUE)
    {
        U32_GET_FIELD_BY_ID_MAC(hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
            SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E,
            value);
        egressInfoPtr->tunnelStartPtr = value;

        U32_GET_FIELD_BY_ID_MAC(hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
            SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E,
            value);

        switch(value)
        {
            case 0:
                egressInfoPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
                break;
            case 1:
                egressInfoPtr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->l2NatSupported == GT_TRUE)
        {
            U32_GET_FIELD_BY_ID_MAC(hwDataArr,
                                    PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
                                    SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E,
                                    value);
            egressInfoPtr->l2NatEnable = BIT2BOOL_MAC(value);

            if(egressInfoPtr->l2NatEnable == GT_TRUE)
            {
                U32_GET_FIELD_BY_ID_MAC(hwDataArr,
                                        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
                                        SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E,
                                        value);
                egressInfoPtr->natPtr = value;
            }
        }

        if((PRV_CPSS_PP_MAC(devNum)->l2NatSupported == GT_FALSE) || (egressInfoPtr->l2NatEnable == GT_FALSE))
        {
            U32_GET_FIELD_BY_ID_MAC(hwDataArr,
                                    PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
                                    SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E,
                                    value);
            egressInfoPtr->arpPtr = value;
        }


        U32_GET_FIELD_BY_ID_MAC(hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
            SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E,
            value);
        egressInfoPtr->modifyMacSa = BIT2BOOL_MAC(value);

        U32_GET_FIELD_BY_ID_MAC(hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E].fieldsInfoPtr,
            SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E,
            value);
        egressInfoPtr->modifyMacDa = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgEportToPhysicalPortEgressPortInfoGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] egressInfoPtr            - (pointer to) physical Information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  *egressInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportToPhysicalPortEgressPortInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egressInfoPtr));

    rc = internal_cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum, portNum, egressInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egressInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgEportAssignEgressAttributesLocallySet function
* @endinternal
*
* @brief   Enable/disable per ePort, assigning the Egress ePort Attributes Locally.
*           In multi-device systems, there are some use cases that require egress
*           attributes to be applied by the local device, even when the packet target
*           device is the not the local device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - egress port number
* @param[in] enable                - GT_FALSE -
*                                   disable assigning Egress ePort Attributes Locally.
*                                    GT_TRUE  -
*                                   enable Assigning Egress ePort Attributes Locally.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgEportAssignEgressAttributesLocallySet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E );

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    return prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ASSIGN_EGR_ATTR_LOCAL_E,
            GT_FALSE, /*accessPhysicalPort*/
            GT_TRUE, /*accessEPort*/
            BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChBrgEportAssignEgressAttributesLocallySet function
* @endinternal
*
* @brief   Enable/disable per ePort, assigning the Egress ePort Attributes Locally.
*           In multi-device systems, there are some use cases that require egress
*           attributes to be applied by the local device, even when the packet target
*           device is the not the local device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - egress port number
* @param[in] enable                - GT_FALSE -
*                                   disable assigning Egress ePort Attributes Locally.
*                                    GT_TRUE  -
*                                   enable Assigning Egress ePort Attributes Locally.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportAssignEgressAttributesLocallySet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportAssignEgressAttributesLocallySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgEportAssignEgressAttributesLocallySet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssDxChBrgEportAssignEgressAttributesLocallyGet function
* @endinternal
*
* @brief   Get the status of Enable/disable per ePort, assigning the Egress ePort Attributes Locally.
*           In multi-device systems, there are some use cases that require egress
*           attributes to be applied by the local device, even when the packet target
*           device is the not the local device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - egress port number
* @param[in] enablePtr             - (pointer to)
*                                    GT_FALSE -
*                                   disable assigning Egress ePort Attributes Locally.
*                                    GT_TRUE  -
*                                   enable Assigning Egress ePort Attributes Locally.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgEportAssignEgressAttributesLocallyGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_BOOL         *enablePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ASSIGN_EGR_ATTR_LOCAL_E,
            GT_FALSE, /*accessPhysicalPort*/
            &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;

}
/**
* @internal cpssDxChBrgEportAssignEgressAttributesLocallyGet function
* @endinternal
*
* @brief   Get the status of Enable/disable per ePort, assigning the Egress ePort Attributes Locally.
*           In multi-device systems, there are some use cases that require egress
*           attributes to be applied by the local device, even when the packet target
*           device is the not the local device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - egress port number
* @param[in] enablePtr             - (pointer to)
*                                    GT_FALSE -
*                                   disable assigning Egress ePort Attributes Locally.
*                                    GT_TRUE  -
*                                   enable Assigning Egress ePort Attributes Locally.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportAssignEgressAttributesLocallyGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgEportAssignEgressAttributesLocallyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgEportAssignEgressAttributesLocallyGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

