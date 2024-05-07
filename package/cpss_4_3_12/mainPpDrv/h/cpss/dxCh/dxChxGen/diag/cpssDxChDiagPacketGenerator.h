/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssDxChDiagPacketGenerator.h
*
* @brief Diag Packet Generator APIs for CPSS DxCh.
*
* Sequence for generating packets on port:
* 1. Connect the port to packet generator and set configurations
* (cpssDxChDiagPacketGeneratorConnectSet)
* 2. Start transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 3. If single burst transmit mode used:
* Check transmission done status (cpssDxChDiagPacketGeneratorBurstTransmitStatusGet)
* If continues transmit mode used:
* Stop transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 4. Return the port to normal operation, disconnect the packet generator
* (cpssDxChDiagPacketGeneratorConnectSet)
*
* @version   7
********************************************************************************
*/
#ifndef __cpssDxChDiagPacketGeneratorh
#define __cpssDxChDiagPacketGeneratorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/diag/cpssDiagPacketGenerator.h>
#include <cpss/common/port/cpssPortCtrl.h>

/* Maximal profiles per PG channel */
#define CPSS_DXCH_DIAG_PG_MAX_PROFILES_CNS          4

/* Maximal PG instances */
#define CPSS_DXCH_DIAG_PG_INSTANCES_CNS             2

/**
* @struct CPSS_DXCH_DIAG_PG_PROFILE_CONFIG_STC
*
* @brief Packet generator's profile configurations.
*  
*/
typedef struct{

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief Enables insertion of vlan tag.
     *  GT_FALSE - vlan tag is not inserted.
     *  GT_TRUE - vlan tag is inserted after MAC SA.
     */
    GT_BOOL vlanTagEnable;

    /** @brief The VPT of the VLAN tag added to packet. (APPLICABLE RANGES: 0..7)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 vpt;

    /** @brief The CFI of the VLAN tag added to packet. (APPLICABLE RANGES: 0..1)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 cfi;

    /** @brief The VLAN ID of the VLAN tag added to packet. (APPLICABLE RANGES: 0..4095)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U16 vid;

    /** @brief The etherType to be sent in the packet after the VLAN tag(if any).
     */
    GT_U16 etherType;

    /** @brief Up to 31 bytes of header can be created, some using dedicated fields
     *  to construct proper Ethernet frame with optional VLAN tag.
     *  The rest of the header is fixed. If vlanTagEnable == TRUE, only first 13 bytes are used. */
    GT_U8 headerByteArr[17];

    /** The payload type to be placed after etherType. */
    CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT payloadType;

    /** Defines initial pattern for payload generation while a given profile is selected */
    GT_U8 payloadPatternArr[2];

    /** @brief Packet Length defines length of generated packet while a given profile is selected.
     *  The length (in bytes) is sum of header and payload length CRC not included.
     *  If minimal packet length < 60B, MAC (egress) pads it to 60B.
     *  (APPLICABLE RANGES: 60..10240) */
    GT_U32 packetLength;

    /** @brief The field controls packet rate and specifies a number of gaps inserted after each EoP.
     *  The minimal field value is 0, means there is no gap after EoP and the packets rate is equal to the port's link speed.
     *  The expected packets rate on the link could be found in HW excel table.
        (APPLICABLE RANGES: 0..15) */
    GT_U32 rateControlGap;

}CPSS_DXCH_DIAG_PG_PROFILE_CONFIG_STC;

/**
* @struct CPSS_DXCH_DIAG_PG_CONFIG_STC
*
* @brief Packet generator configurations.
*  
*/
typedef struct{
    /** Physical port to connect to packet generator */
    GT_PHYSICAL_PORT_NUM           portNum;

    /** @brief Configure express/preemptive channel of the port to operate on packet generator
     *  Used only when preemption is enabled on port.
     *  Only CPSS_PORT_PREEMPTION_CLASS_EXPRESS_E and CPSS_PORT_PREEMPTION_CLASS_PREEMPTIVE_E options are supported */
    CPSS_PORT_PREEMPTION_CLASS_ENT preemptionClass;

    /** @brief Four profiles are supported per tested channel.
     *  All four indexes must be configured.
     *  E.g. if the only required profile = 0x3, the profileIndexes = {0x3, 0x3, 0x3, 0x3} */
    GT_U32 profileIndexes[CPSS_DXCH_DIAG_PG_MAX_PROFILES_CNS];

    /** Packet generator transmit mode. */
    CPSS_DIAG_PG_TRANSMIT_MODE_ENT transmitMode;

    /** @brief Number of packets to be sent.
     *  
     *  over testing for a given test channel of Packet Generator module
     *  (APPLICABLE RANGES: 0..0xFFFF) */
    GT_U32  packetsNumber;

}CPSS_DXCH_DIAG_PG_CONFIG_STC;


/**
* @struct CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC
*
* @brief Packet generator's configurations.
*/
typedef struct{

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** @brief Enables MAC DA incremental for every transmitted packet.
     *  GT_FALSE - MAC DA is constant for all packets.
     *  GT_TRUE - MAC DA is incremented for every packet.
     *  NOT APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
     */
    GT_BOOL macDaIncrementEnable;

    GT_U32 macDaIncrementLimit;

    /** Ethernet Source MAC address. */
    GT_ETHERADDR macSa;

    /** @brief Enables insertion of vlan tag.
     *  GT_FALSE - vlan tag is not inserted.
     *  GT_TRUE - vlan tag is inserted after MAC SA.
     */
    GT_BOOL vlanTagEnable;

    /** @brief The VPT of the VLAN tag added to packet. (APPLICABLE RANGES: 0..7)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 vpt;

    /** @brief The CFI of the VLAN tag added to packet. (APPLICABLE RANGES: 0..1)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U8 cfi;

    /** @brief The VLAN ID of the VLAN tag added to packet. (APPLICABLE RANGES: 1..4095)
     *  (relevant only if vlanTagEnable == GT_TRUE)
     */
    GT_U16 vid;

    /** @brief The etherType to be sent in the packet after the VLAN tag(if any).
     *  (APPLICABLE RANGES: 0..0xFFFF)
     */
    GT_U16 etherType;

    /** The payload type to be placed after etherType. */
    CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT payloadType;

    GT_U8 cyclicPatternArr[8];

    /** The packet length type. */
    CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT packetLengthType;

    /** @brief The packet length without CRC. (APPLICABLE RANGES: 20..16383)
     *  (relevant only if packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E)
     *  For AC5X, AC5P and Harrier devices represents minimum frame size for random length packets
     *  and the packet length for constant length packets  
     */
    GT_U32 packetLength;

    /** @brief Enables undersized packets transmission.
     *  (relevant only if packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E)
     */
    GT_BOOL undersizeEnable;

    /** Packet generator transmit mode. */
    CPSS_DIAG_PG_TRANSMIT_MODE_ENT transmitMode;

    /** @brief Packet count to transmit in single burst. (APPLICABLE RANGES: 1..8191)
     *  The total packets number to transmit is:
     *  (packetCount) (packetCountMultiplier value).
     *  (relevant only if transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
     *  packetCountMultiplier- Packet count packet count multiplier.
     *  The total packets number to transmit is:
     *  (packetCount) (packetCountMultiplier value).
     *  (relevant only if transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
     */
    GT_U32 packetCount;

    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT packetCountMultiplier;

    /** Interpacket gap in bytes. (APPLICABLE RANGES: 0..0xFFFF) */
    GT_U32 ipg;

    /** Interface size (what is the word width to the port, instead of the TxDMA, BC2 only). */
    CPSS_DIAG_PG_IF_SIZE_ENT interfaceSize;

} CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC;


/**
* @internal cpssDxChDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator.
*         Set packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Falcon; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
* @param[in] connect                  - GT_TRUE:  the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from
*                                      packet generator; return the port's MAC
*                                      to normal egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectSet
(
    IN GT_U8                                 devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN GT_BOOL                               connect,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port.
*         Get packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier;
* @note   NOT APPLICABLE DEVICES:  Falcon; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
*
* @param[out] connectPtr               - (pointer to)
*                                      GT_TRUE: port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT GT_BOOL                              *connectPtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Falcon;  Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:  enable(start) transmission
*                                       GT_FALSE: disable(stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
);

/**
* @internal cpssDxChDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE: burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
);


/**
* @internal prvCpssDxChMacTGDrvInit function
* @endinternal
*
* @brief   Init Mac TG driver
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChMacTGDrvInit
(
    IN    GT_U8                   devNum
);

/**
* @internal cpssDxChDiagPacketGeneratorProfileConfigSet function
* @endinternal
*
* @brief  Set packet generator profile configuration.
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] profileIndex          - profile index
*                                    (APPLICABLE RANGES: 0..3)
* @param[in] configPtr             - (pointer to) packet generator channel profile configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorProfileConfigSet
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 profileIndex,
    IN CPSS_DXCH_DIAG_PG_PROFILE_CONFIG_STC  *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorProfileConfigGet function
* @endinternal
*
* @brief  Get packet generator profile configuration.
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] profileIndex          - profile index
*                                    (APPLICABLE RANGES: 0..3)
* @param[out] configPtr            - (pointer to) packet generator channel profile configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or profile index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorProfileConfigGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 profileIndex,
    OUT CPSS_DXCH_DIAG_PG_PROFILE_CONFIG_STC  *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorConfigSet function
* @endinternal
*
* @brief  Set packet generator configurations.
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] packetGeneratorIndex  - packet generator index
*                                    (APPLICABLE RANGES: 0..1)
* @param[in] enable                - Configure the packet generator for port's channel.
*                                       GT_TRUE  - apply configuration of port's chanel to the current packet generator
*                                       GT_FALSE - restore configuration of the current packet generator
*  
* @param[in] configPtr             - (pointer to) packet generator channel configurations.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, packet generator index or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssDxChDiagPacketGeneratorConfigSet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                packetGeneratorIndex,
    IN GT_BOOL                               enable,
    IN CPSS_DXCH_DIAG_PG_CONFIG_STC         *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorConfigGet function
* @endinternal
*
* @brief  Get packet generator configurations.
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] packetGeneratorIndex  - packet generator index
*                                    (APPLICABLE RANGES: 0..1)
* @param[out] enablePtr            - (pointer to) status of the packet generator 
*                                       GT_TRUE  - the current packet generator connected to port's channel
*                                       GT_FALSE - the current packet generator not connected to any channel
*  
* @param[out] configPtr            - (pointer to) packet generator channel configurations
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or packet generator index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              packetGeneratorIndex,
    OUT GT_BOOL                            *enablePtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIG_STC       *configPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorStatisticGet function
* @endinternal
*
* @brief  Get packet generator statistic counter.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] packetGeneratorIndex  - packet generator index
*                                    (APPLICABLE RANGES: 0..1)
* @param[out] counterPtr           - (pointer to)  number of sent packets for the given packet generator
*  
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error 
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device or packet generator index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorStatisticGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  packetGeneratorIndex,
    OUT GT_U32                 *counterPtr
);

/**
* @internal cpssDxChDiagPacketGeneratorTransmitStart function
* @endinternal
*
* @brief   Start/Stop transmission on specific packet generator.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Falcon; Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] packetGeneratorIndex  - packet generator index
*                                    (APPLICABLE RANGES: 0..1)
* @param[in] start                 - GT_TRUE:  start transmission
*                                    GT_FALSE: stop transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong packet generator or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorTransmitStart
(
    IN  GT_U8                devNum,
    IN  GT_U32               packetGeneratorIndex,
    IN  GT_BOOL              start
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagPacketGeneratorh */
