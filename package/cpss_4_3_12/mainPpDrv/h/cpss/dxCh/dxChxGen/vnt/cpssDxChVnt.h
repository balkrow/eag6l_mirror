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
* @file cpssDxChVnt.h
*
* @brief CPSS DXCH Virtual Network Tester (VNT) Technology facility API.
* VNT features:
* - Operation, Administration, and Maintenance (OAM).
* - Connectivity Fault Management (CFM).
*
* @version   12
********************************************************************************
*/
#ifndef __cpssDxChVnth
#define __cpssDxChVnth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT
 *
 * @brief Enumeration for CFM opcode change mode.
*/
typedef enum{

    /** @brief When replying to a CFM message, the opcode is taken from
     *  CFM LBR Opcode value.
     */
    CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E,

    /** @brief When replying to a CFM message, the least significant bit of
     *  the opcode is changed to zero.
     */
    CPSS_DXCH_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E

} CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT;

/**
* @internal cpssDxChVntOamPortLoopBackModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable 802.3ah Loopback mode on the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         Loopback Mode is enabled for the port,
*         - All traffic destined for this port but was not ingressed on this port
*         is discarded, except for FROM_CPU traffic - which is to allow the CPU
*         to send OAM control packets.
*         - Egress VLAN and spanning tree filtering is bypassed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number ,CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  Enable OAM loopback mode.
*                                      GT_FALSE: Disable OAM loopback mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the Policy TCAM, after the existing rule which traps OAMPDUs,
*       add a rule to redirect all traffic received on
*       the port back to the same port for getting loopback.
*       For xCat3 and above not need rule for OAM PDU trap.
*       Use cpssDxChVntOamPortPduTrapEnableSet for it.
*
*/
GT_STATUS cpssDxChVntOamPortLoopBackModeEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChVntOamPortLoopBackModeEnableGet function
* @endinternal
*
* @brief   Get 802.3ah Loopback mode on the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         Loopback Mode is enabled for the port,
*         - All traffic destined for this port but was not ingressed on this port
*         is discarded, except for FROM_CPU traffic - which is to allow the CPU
*         to send OAM control packets.
*         - Egress VLAN and spanning tree filtering is bypassed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number ,CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - pointer to Loopback mode status:
*                                      GT_TRUE:  Enable OAM loopback mode.
*                                      GT_FALSE: Disable OAM loopback mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the Policy TCAM, after the existing rule which traps OAMPDUs,
*       add a rule to redirect all traffic received on
*       the port back to the same port for getting loopback.
*       For xCat3 and above not need rule for OAM PDU trap.
*       Use cpssDxChVntOamPortPduTrapEnableSet for it.
*
*/
GT_STATUS cpssDxChVntOamPortLoopBackModeEnableGet
(
    IN  GT_U8                   devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChVntOamPortUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port for unidirectional transmit.
*         If unidirectional transmit enabled, sending
*         OAM (Operation, Administration, and Maintenance) control packets
*         and data traffic over failed links (ports with link down) is allowed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
* @param[in] enable                   - GT_TRUE:   Enable the port for unidirectional transmit.
*                                      GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntOamPortUnidirectionalEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    port,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChVntOamPortUnidirectionalEnableGet function
* @endinternal
*
* @brief   Gets the current status of unidirectional transmit for the port.
*         If unidirectional transmit enabled, sending
*         OAM (Operation, Administration, and Maintenance) control packets
*         and data traffic over failed links (ports with link down) is allowed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] port                     - physical number including CPU port.
*
* @param[out] enablePtr                - pointer to current status of unidirectional transmit:
*                                      GT_TRUE:   Enable the port for unidirectional transmit.
*                                      GT_FALSE:  Disable the port for unidirectional transmit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*/
GT_STATUS cpssDxChVntOamPortUnidirectionalEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM    port,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChVntCfmEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CFM (Connectivity Fault Management) PDUs.
*         If the packet EtherType matches the CFM EtherType, the ingress Policy
*         key implicitly uses the three User-Defined-Byte (UDB0, UDB1, UDB2) to
*         contain the three CFM data fields, MD Level, Opcode, and Flags,
*         respectively.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - CFM (Connectivity Fault Management) EtherType,
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong etherType values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntCfmEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
);

/**
* @internal cpssDxChVntCfmEtherTypeGet function
* @endinternal
*
* @brief   Gets the current EtherType to identify CFM
*         (Connectivity Fault Management) PDUs.
*         If the packet EtherType matches the CFM EtherType, the ingress Policy
*         key implicitly uses the three User-Defined-Byte (UDB0, UDB1, UDB2) to
*         contain the three CFM data fields, MD Level, Opcode, and Flags,
*         respectively.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - pointer to CFM (Connectivity Fault Management) EtherType.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntCfmEtherTypeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *etherTypePtr
);

/**
* @internal cpssDxChVntCfmLbrOpcodeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] opcode                   - CFM LBR Opcode (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, opcode.
* @retval GT_OUT_OF_RANGE          - on wrong opcode values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntCfmLbrOpcodeSet
(
    IN GT_U8    devNum,
    IN GT_U32   opcode
);

/**
* @internal cpssDxChVntCfmLbrOpcodeGet function
* @endinternal
*
* @brief   Gets the current CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] opcodePtr                - pointer to CFM LBR Opcode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntCfmLbrOpcodeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *opcodePtr
);

/**
* @internal cpssDxChVntCfmReplyModeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntCfmReplyModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT mode
);

/**
* @internal cpssDxChVntCfmReplyModeGet function
* @endinternal
*
* @brief   Gets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - output parameter is NULL pointer.
*/
GT_STATUS cpssDxChVntCfmReplyModeGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_VNT_CFM_REPLY_MODE_ENT *modePtr
);

/**
* @internal cpssDxChVntPortGroupLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers.
*         Starting from Falcon the function returns current timestamp.
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*                                        timestamp in PP Core Clock resolution.
*                                        e.g. for 200  MHz:   granularity - 5 nSec
*                                        Falcon use CNM clock 312.5 MHz for timestamp
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntPortGroupLastReadTimeStampGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *timeStampValuePtr
);

/**
* @internal cpssDxChVntLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers.
*         Starting from Falcon the function returns current timestamp.
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*                                        timestamp in PP Core Clock resolution.
*                                        e.g. for 200  MHz:   granularity - 5 nSec
*                                        Falcon use CNM clock 312.5 MHz for timestamp
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntLastReadTimeStampGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *timeStampValuePtr
);

/**
* @internal cpssDxChVntOamPortPduTrapEnableSet function
* @endinternal
*
* @brief   Enable/Disable trap to CPU of 802.3ah Link Layer Control protocol on
*         the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         PDU trap is enabled for the port:
*         - Packets identified as OAM-PDUs destined for this port are trapped to the CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number ,CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  Enable trap to CPU of 802.3ah Link Layer Control protocol.
*                                      GT_FALSE: Disable trap to CPU of 802.3ah Link Layer Control protocol.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChVntOamPortPduTrapEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChVntOamPortPduTrapEnableGet function
* @endinternal
*
* @brief   Get trap to CPU status(Enable/Disable) of 802.3ah Link Layer Control protocol on
*         the specified port.
*         If OAM (Operation, Administration, and Maintenance)
*         PDU trap is enabled for the port:
*         - Packets identified as OAM-PDUs are trapped to the CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number ,CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - pointer to Loopback mode status:
*                                      GT_TRUE:  Enable trap to CPU of 802.3ah Link Layer Control protocol.
*                                      GT_FALSE: Disable trap to CPU of 802.3ah Link Layer Control protocol..
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on bad pointer of enablePtr
*/
GT_STATUS cpssDxChVntOamPortPduTrapEnableGet
(
    IN  GT_U8                   devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVnth */



