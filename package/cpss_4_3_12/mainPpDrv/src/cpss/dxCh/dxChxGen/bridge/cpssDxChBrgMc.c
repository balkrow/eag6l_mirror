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
* @file cpssDxChBrgMc.c
*
* @brief CPSS DXCh Multicast Group facility implementation.
*
* @version   30
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* check vidxId , when used as 'key' we return GT_BAD_PARAM on error */
/* note : index is in the range: 0..maxNum-1 */
#define PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(_devNum,_index)                                \
    if((_index) == FLOOD_VIDX_CNS && (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.vidxNum < _4K))\
    {                                                                                   \
        /* we support special flood vidx value to enter the API */                      \
                                                                                        \
    }                                                                                   \
    else                                                                                \
    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(_devNum))                                  \
    {                                                                                   \
        if((_index) >=                                                                  \
            PRV_CPSS_ACCESS_TABLE_INFO_PTR(_devNum)[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries)\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vidx limited to max[%d] got[%d]",\
                PRV_CPSS_ACCESS_TABLE_INFO_PTR(_devNum)[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries-1,_index);  \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if((_index) >= PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.vidxNum)     \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vidx limited to max[%d] got[%d]",\
                PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.vidxNum-1,_index);    \
    }

/**
* @internal prvDxChBrgMcMemberBitSet function
* @endinternal
*
* @brief   Handles port member in the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be added or deleted from
*                                      the flood VIDX entry.
*                                      The flood VIDX 0xFFF.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: Lion2 0..127)
*                                      CPU port supported
* @param[in] bitValue                 - 0: Delete port from the Multicast Group entry.
*                                      1: Add port to the Multicast Group entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than allowed value.
*/
static GT_STATUS prvDxChBrgMcMemberBitSet
(
    IN GT_U8                    devNum,
    IN GT_U16                   vidx,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   bitValue
)
{
    GT_U32  bitOffset;  /* bit offset of port in the MC Entry */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;  /* local port - support multi-port-groups device */
    GT_U16  floodVidx;   /* reserved VIDX for flooding traffic */
    GT_PHYSICAL_PORT_NUM savedPortNum; /* global port num to save        */
    CPSS_PORTS_BMP_STC   portBitmap;   /* bitmap to read, update, write */
    GT_STATUS            rc;           /* return code */
    GT_BOOL              isCpu;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    floodVidx = (GT_U16)PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);
    rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((isCpu == GT_FALSE) || (vidx != floodVidx))
    {
        if (vidx == floodVidx)
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                /* the HW implementation detached the 0xFFF VIDX from the single destination traffic
                   so we can let application to manipulate all VIDX entries  */
            }
            else
            {
                /* allow CPU port member manipulation in the flood VIDX.
                   Other members cannot be changed in the flood VIDX. */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(devNum,vidx);
    }

    if (bitValue > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* call the generic function that handle port in tables of BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntrySetPort(devNum,
                    CPSS_DXCH_TABLE_MULTICAST_E,
                    vidx,portNum,
                    BIT2BOOL_MAC(bitValue));

        return rc;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        if (isCpu == GT_FALSE)
        {
            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

            if (localPort > 27)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* Calculate HW bit offset for requested port number */
            bitOffset  = localPort + 1;
        }
        else
        {
            /* CPU port member bit */
            bitOffset = 0;
            portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        /* Set VIDX MC Entry bit is associated with the added port number */
        return prvCpssDxChPortGroupWriteTableEntryField(devNum,portGroupId,
                                               CPSS_DXCH_TABLE_MULTICAST_E,
                                               (GT_U32)vidx,  /* entry index */
                                               0,             /* word0 */
                                               bitOffset,     /* start bit */
                                               1,             /* field length */
                                               bitValue);     /* field value */
    }
    else
    {
        savedPortNum = portNum;

        if (isCpu == GT_TRUE)
        {
            /* CPU port conversion - set actual CPU port */

            /* CPU port group id  */
            portGroupId = (PRV_CPSS_PP_MAC(devNum)->cpuPortMode !=
                          CPSS_NET_CPU_PORT_MODE_SDMA_E) ?
                          PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum) :
                          PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

            portNum = (portGroupId * 16 + 15);
        }
        else
        {
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
        }

        portNum = PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(devNum,portNum);

        if(portNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* Updated entry must be written by cpssDxChBrgMcEntryWrite.             */
            /* This function supports the management of internal port 12 (and 64+12) */
            /* that connects between two hemispheres. Such internal port included by */
            /* default to all MC entries. It must be excluded for such MC entry      */
            /* that does not contain ports in other hemisphere.                      */
            rc = cpssDxChBrgMcEntryRead(devNum, vidx, &portBitmap);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (bitValue == 0)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portBitmap, savedPortNum);
            }
            else
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap, savedPortNum);
            }
            return cpssDxChBrgMcEntryWrite(devNum, vidx, &portBitmap);
        }
        else
        {
            return prvCpssDxChPortGroupWriteTableEntryField(
                    devNum, portGroupId,
                    CPSS_DXCH_TABLE_MULTICAST_E,
                    (GT_U32)vidx,  /* entry index */
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                    portNum,       /* start bit */
                    1,             /* field length */
                    bitValue);     /* field value */
        }
    }
}

/**
* @internal internal_cpssDxChBrgMcInit function
* @endinternal
*
* @brief   Initializes Bridge engine Multicast Library.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcInit
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return GT_OK;
}

/**
* @internal cpssDxChBrgMcInit function
* @endinternal
*
* @brief   Initializes Bridge engine Multicast Library.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChBrgMcInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcIpv6BytesSelectSet function
* @endinternal
*
* @brief   Sets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[in] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcIpv6BytesSelectSet
(
    IN GT_U8  devNum,
    IN GT_U8  dipBytesSelectMapArr[4],
    IN GT_U8  sipBytesSelectMapArr[4]
)
{
    GT_U32      i;              /* iterator */
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(dipBytesSelectMapArr);
    CPSS_NULL_PTR_CHECK_MAC(sipBytesSelectMapArr);

    /* Verify validity of the passed parameters */
    for (i = 0; i < 4; i++)
    {
        if ((sipBytesSelectMapArr[i] > 0xF) || (dipBytesSelectMapArr[i] > 0xF))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    IPv6McBridgingBytesSelection.IPv6MCBridgingBytesSelectionConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIpv6BytesSelection;
    }

    /* Convert DIP and SIP Big-Endian Network    */
    /* format to the Little-Endian PP HW format. */
    regValue = 0;
    for (i = 0; i < 4; i++)
    {
        regValue |= ((0xF - dipBytesSelectMapArr[i]) << (4*i)) |
                    ((0xF - sipBytesSelectMapArr[i]) << (4*i+16));
    }

    /* Set DIP and SIP values to the HW register */
    return prvCpssHwPpWriteRegister(devNum, regAddr, regValue);
}

/**
* @internal cpssDxChBrgMcIpv6BytesSelectSet function
* @endinternal
*
* @brief   Sets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[in] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectSet
(
    IN GT_U8  devNum,
    IN GT_U8  dipBytesSelectMapArr[4],
    IN GT_U8  sipBytesSelectMapArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcIpv6BytesSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dipBytesSelectMapArr, sipBytesSelectMapArr));

    rc = internal_cpssDxChBrgMcIpv6BytesSelectSet(devNum, dipBytesSelectMapArr, sipBytesSelectMapArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dipBytesSelectMapArr, sipBytesSelectMapArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcIpv6BytesSelectGet function
* @endinternal
*
* @brief   Gets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[out] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcIpv6BytesSelectGet
(
    IN  GT_U8  devNum,
    OUT GT_U8  dipBytesSelectMapArr[4],
    OUT GT_U8  sipBytesSelectMapArr[4]
)
{
    GT_U32      i;              /* iterator */
    GT_U32      regAddr;        /* register address */
    GT_STATUS   retStatus;      /* generic return status code */
    GT_U32      regValue;       /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(dipBytesSelectMapArr);
    CPSS_NULL_PTR_CHECK_MAC(sipBytesSelectMapArr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    IPv6McBridgingBytesSelection.IPv6MCBridgingBytesSelectionConfig;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIpv6BytesSelection;
    }

    /* Read DIP and SIP values from HW */
    retStatus = prvCpssHwPpReadRegister(devNum, regAddr, &regValue);
    if (GT_OK != retStatus)
    {
        return retStatus;
    }

    /* Convert DIP and SIP Little-Endian PP HW  */
    /* format to the Big-Endian Network format. */
    for (i = 0; i < 4; i++)
    {
        dipBytesSelectMapArr[i] =
            (GT_U8)(0xF - ((regValue >> (4*i)) & 0xF));
        sipBytesSelectMapArr[i] =
            (GT_U8)(0xF - ((regValue >> (4*i+16)) & 0xF));
    }

    return retStatus;
}

/**
* @internal cpssDxChBrgMcIpv6BytesSelectGet function
* @endinternal
*
* @brief   Gets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[out] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectGet
(
    IN  GT_U8  devNum,
    OUT GT_U8  dipBytesSelectMapArr[4],
    OUT GT_U8  sipBytesSelectMapArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcIpv6BytesSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dipBytesSelectMapArr, sipBytesSelectMapArr));

    rc = internal_cpssDxChBrgMcIpv6BytesSelectGet(devNum, dipBytesSelectMapArr, sipBytesSelectMapArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dipBytesSelectMapArr, sipBytesSelectMapArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0xFFF.
* @param[in] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[in] vidx                     MC group of specified PP device.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, ports bitmap value or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
GT_STATUS prvCpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
)
{
    GT_STATUS   rc;  /* generic return status code */
    GT_U32      hwPortBitmap;     /* bmp of ports in HW entry   */
    GT_U32      hwPortBitmapSize; /* bmp of ports in HW entry size in bits */
    GT_U32      i;                /* iterator */
    CPSS_PORTS_BMP_STC portsMembersBmp; /* local variable to avoid portBitmapPtr change */
    GT_U16      floodVidx;   /* reserved VIDX for flooding traffic */

    floodVidx = (GT_U16)PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , portBitmapPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryWrite(devNum,
                CPSS_DXCH_TABLE_MULTICAST_E,
                vidx,portBitmapPtr,
                GT_FALSE,
                GT_FALSE);/*don't care*/

        return rc;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* bit0 - CPU port and 28 ports */
        hwPortBitmapSize = 29;

        if (portBitmapPtr->ports[0] >= (GT_U32)(1 << (hwPortBitmapSize - 1)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwPortBitmap = (portBitmapPtr->ports[0] << 1);
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
            portBitmapPtr, CPSS_CPU_PORT_NUM_CNS))
        {
            /* bit0 related to CPU port membership */
            hwPortBitmap |= 1;
        }

        /* Write bitmap of ports are belonged to the vidx MC group */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_MULTICAST_E,
                                             (GT_U32)vidx,  /* entry index  */
                                             0,             /* word0        */
                                             0,                   /* start bit    */
                                             hwPortBitmapSize,    /* field length */
                                             hwPortBitmap);

        return rc;
    }
    else
    {
        cpssOsMemCpy(&portsMembersBmp, portBitmapPtr, sizeof(CPSS_PORTS_BMP_STC));

        if(vidx != floodVidx)
        {
            /* CPU port bitmap conversion */
            /* Update port bitmap according to physical CPU port connection */
            rc = prvCpssDxChBrgCpuPortBmpConvert(devNum,
                                                 GT_TRUE,
                                                 &portsMembersBmp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* port 12 on each hemisphere is used to duplicate the packet to the other hemisphere;
               this port should be member of the VIDX only if there are additional members on the
               other hemisphere; note that port 12 is reversed logic (0: member, 1: not a member) */
            /* check if the VLAN contains members on the 1st hemisphere (other than port 12) */

            if (((portsMembersBmp.ports[0] & 0xffffefff) == 0) && (portsMembersBmp.ports[1] == 0))
            {
                /* internally set port 12 on 2nd hemisphere to indicate it is NOT a member */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembersBmp, (64+12));
            }
            else
            {
                /* internally clear port 12 on 2nd hemisphere to indicate it IS a member */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersBmp, (64+12));
            }

            /* check if the VIDX contains members on the 2nd hemisphere (other than port 12) */
            if (((portsMembersBmp.ports[2] & 0xffffefff) == 0) && (portsMembersBmp.ports[3] == 0))
            {
                /* internally set port 12 on 1st hemisphere to indicate it is NOT a member */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembersBmp, 12);
            }
            else
            {
                /* internally clear port 12 on 1st hemisphere to indicate it IS a member */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersBmp, 12);
            }
        }


        /* loop on all TXQ units */
        for (i = 0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
        {
            rc =  prvCpssDxChPortGroupWriteTableEntry(devNum,
                   i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                   CPSS_DXCH_TABLE_MULTICAST_E,
                   (GT_U32)vidx,  /* entry index */
                   &(portsMembersBmp.ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]));

            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }
}

/**
* @internal internal_cpssDxChBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0xFFF.
* @param[in] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[in] vidx                     MC group of specified PP device.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, ports bitmap value or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
static GT_STATUS internal_cpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
)
{
    GT_STATUS   rc;          /* generic return status code */
    GT_U16      floodVidx;   /* reserved VIDX for flooding traffic */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(devNum,vidx);
    CPSS_NULL_PTR_CHECK_MAC(portBitmapPtr);

    floodVidx = (GT_U16)PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* the HW implementation detached the 0xFFF VIDX from the single destination traffic
           so we can let application to manipulate all VIDX entries  */
    }
    else
    /* The flood VIDX entry cannot be changed */
    if (vidx == floodVidx)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChBrgMcEntryWrite(devNum, vidx, portBitmapPtr);
    return rc;
}

/**
* @internal cpssDxChBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0xFFF.
* @param[in] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[in] vidx                     MC group of specified PP device.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, ports bitmap value or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
GT_STATUS cpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, portBitmapPtr));

    rc = internal_cpssDxChBrgMcEntryWrite(devNum, vidx, portBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, portBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgMcEntryRead function
* @endinternal
*
* @brief   Reads the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is allowed for reading.
*                                      The flood VIDX is 0xFFF.
*
* @param[out] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[out] vidx                     MC group of specified PP device.
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcEntryRead
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidx,
    OUT CPSS_PORTS_BMP_STC   *portBitmapPtr
)
{
    GT_STATUS   rc;               /* return code                */
    GT_U32      hwPortBitmap;     /* bmp of ports in HW entry   */
    GT_U32      hwPortBitmapSize; /* bmp of ports in HW entry size in bits */
    GT_U16      floodVidx;   /* reserved VIDX for flooding traffic */
    GT_U32      i;              /* iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portBitmapPtr);

    floodVidx = (GT_U16)PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);

    /* The flood VIDX entry can be read */
    if (vidx != floodVidx)
    {
        /* Check Multicast Group entry index (VIDX) */
        PRV_CPSS_DXCH_VIDX_NUM_CHECK_MAC(devNum,vidx);
    }

    cpssOsMemSet(portBitmapPtr, 0, sizeof(CPSS_PORTS_BMP_STC));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* call the generic function that handle BMP of ports in the EGF_SHT */
        rc = prvCpssDxChHwEgfShtPortsBmpTableEntryRead(devNum,
                CPSS_DXCH_TABLE_MULTICAST_E,
                vidx,portBitmapPtr,
                GT_FALSE,
                NULL);/*don't care*/

        if(rc != GT_OK)
        {
            return rc;
        }

        /* adjusted physical ports of BMP to hold only bmp of existing ports*/
        rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , portBitmapPtr , portBitmapPtr);

        return rc;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Gather the bitmap of ports from the VIDX Multicast Table Entry */
        hwPortBitmapSize = 29;

        /* Read the VIDX Multicast Table Entry from the HW */
        rc = prvCpssDxChReadTableEntryField(
                                          devNum,
                                          CPSS_DXCH_TABLE_MULTICAST_E,
                                          (GT_U32)vidx, /* entry index  */
                                          0,            /* word0        */
                                          0,                  /* start bit    */
                                          hwPortBitmapSize,   /* field length */
                                          &hwPortBitmap);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* HW format to SW format */
        portBitmapPtr->ports[0] = (hwPortBitmap >> 1);
        portBitmapPtr->ports[1] = 0;

        if ((hwPortBitmap & 1) != 0)
        {
            /* CPU port is a member of MC group */
            CPSS_PORTS_BMP_PORT_SET_MAC(
                portBitmapPtr, CPSS_CPU_PORT_NUM_CNS);
        }

        return GT_OK;
    }
    else
    {
        /* loop on all TXQ units */
        for (i = 0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
        {
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                  i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                  CPSS_DXCH_TABLE_MULTICAST_E,
                  (GT_U32)vidx,  /* entry index */
                  &(portBitmapPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* remove parity (bit 64 in each hemisphere) from output.
           variable i is 2 for Lion2/3 after previouse loop.
           need to clean word#4 for Lion2/3 */
        portBitmapPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS] = 0;

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* port 12 on each hemisphere is used to duplicate the packet to the other hemisphere;
               this port should be member of the VIDX only if there are additional members on the
               other hemisphere; note that port 12 is reversed logic (0: member, 1: not a member) */
            /* clear bits of these ports regarless HW state */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(portBitmapPtr, 12);
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(portBitmapPtr, (64+12));
        }

        /* CPU port bitmap conversion */
        if(vidx != floodVidx)
        {
            rc = prvCpssDxChBrgCpuPortBmpConvert(devNum,
                                                 GT_FALSE,
                                                 portBitmapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }
}

/**
* @internal cpssDxChBrgMcEntryRead function
* @endinternal
*
* @brief   Reads the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is allowed for reading.
*                                      The flood VIDX is 0xFFF.
*
* @param[out] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[out] vidx                     MC group of specified PP device.
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcEntryRead
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidx,
    OUT CPSS_PORTS_BMP_STC   *portBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, portBitmapPtr));

    rc = internal_cpssDxChBrgMcEntryRead(devNum, vidx, portBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, portBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcGroupDelete function
* @endinternal
*
* @brief   Deletes the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0xFFF.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcGroupDelete
(
    IN GT_U8    devNum,
    IN GT_U16   vidx
)
{
    CPSS_PORTS_BMP_STC  portBitmap;  /* MC group ports bitmap */

    /* Nullify MC group ports bitmap */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

    /* Write nullified MC group ports bitmap to the MC Table Entry */
    return cpssDxChBrgMcEntryWrite(devNum, vidx, &portBitmap);
}

/**
* @internal cpssDxChBrgMcGroupDelete function
* @endinternal
*
* @brief   Deletes the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0xFFF.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcGroupDelete
(
    IN GT_U8    devNum,
    IN GT_U16   vidx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcGroupDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx));

    rc = internal_cpssDxChBrgMcGroupDelete(devNum, vidx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be added to the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0xFFF.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_U16                   vidx,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    return prvDxChBrgMcMemberBitSet(devNum, vidx, portNum, 1);
}

/**
* @internal cpssDxChBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be added to the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0xFFF.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_U16                   vidx,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcMemberAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, portNum));

    rc = internal_cpssDxChBrgMcMemberAdd(devNum, vidx, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcMemberDelete function
* @endinternal
*
* @brief   Delete port member from the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be deleted from the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0xFFF.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcMemberDelete
(
    IN GT_U8                        devNum,
    IN GT_U16                       vidx,
    IN GT_PHYSICAL_PORT_NUM         portNum
)
{
    return prvDxChBrgMcMemberBitSet(devNum, vidx, portNum, 0);
}

/**
* @internal cpssDxChBrgMcMemberDelete function
* @endinternal
*
* @brief   Delete port member from the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be deleted from the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0xFFF.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcMemberDelete
(
    IN GT_U8                        devNum,
    IN GT_U16                       vidx,
    IN GT_PHYSICAL_PORT_NUM         portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcMemberDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, portNum));

    rc = internal_cpssDxChBrgMcMemberDelete(devNum, vidx, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcPhysicalSourceFilteringEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    return prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         hwData);
}

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcPhysicalSourceFilteringEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - filtering enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = BIT2BOOL_MAC(hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - filtering enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcPhysicalSourceFilteringEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet function
* @endinternal
*
* @brief   Set indication that the physical port can 'ignore' MC src physical port filtering.
*         Enable/Disable 'ignoring' MC src physical port filtering .
*         Filter relate to sending Multicast packets back to its source physical port
*         by ignoring the 'SrcMcFilter' that may be set by ingress eport
*         see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*         NOTE: by default ALL physical ports are set 'IgnorePhySrcMcFilter = false'
*         except for cascade port that are set as 'IgnorePhySrcMcFilter = true'
*         (done by API cpssDxChCscdPortTypeSet(...))
*         BUT it may be required for some cases to override those 'default' settings.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ignoreFilter             - Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          ignoreFilter
)
{
    GT_STATUS  rc;
    GT_U32  data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    data = BOOL2BIT_MAC(ignoreFilter);
    /* Used to enable Multicast filtering over cascade ports even if the
       packet is assigned <Phy Src MC Filter En> = 0.
       This value should be set to 1 over cascade ports. */
    rc = prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IGNORE_PHY_SRC_MC_FILTER_E,
            GT_TRUE, /*accessPhysicalPort*/
            GT_FALSE, /*accessEPort*/
            data);
    return rc;
}

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet function
* @endinternal
*
* @brief   Set indication that the physical port can 'ignore' MC src physical port filtering.
*         Enable/Disable 'ignoring' MC src physical port filtering .
*         Filter relate to sending Multicast packets back to its source physical port
*         by ignoring the 'SrcMcFilter' that may be set by ingress eport
*         see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*         NOTE: by default ALL physical ports are set 'IgnorePhySrcMcFilter = false'
*         except for cascade port that are set as 'IgnorePhySrcMcFilter = true'
*         (done by API cpssDxChCscdPortTypeSet(...))
*         BUT it may be required for some cases to override those 'default' settings.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ignoreFilter             - Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          ignoreFilter
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ignoreFilter));

    rc = internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet(devNum, portNum, ignoreFilter);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ignoreFilter));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet function
* @endinternal
*
* @brief   Get indication if the physical port 'ignoring' MC src physical port filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ignoreFilterPtr          - (pointer to) Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL          *ignoreFilterPtr
)
{
    GT_STATUS  rc;
    GT_U32  data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(ignoreFilterPtr);

    /* Used to enable Multicast filtering over cascade ports even if the
       packet is assigned <Phy Src MC Filter En> = 0.
       This value should be set to 1 over cascade ports. */
    rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
        PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IGNORE_PHY_SRC_MC_FILTER_E,
            GT_TRUE, /*accessPhysicalPort*/
            &data);

    *ignoreFilterPtr = BIT2BOOL_MAC(data);

    return rc;
}

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet function
* @endinternal
*
* @brief   Get indication if the physical port 'ignoring' MC src physical port filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ignoreFilterPtr          - (pointer to) Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL          *ignoreFilterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ignoreFilterPtr));

    rc = internal_cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet(devNum, portNum, ignoreFilterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ignoreFilterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

