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
* @file cpssDxChStreamGateControl.h
*
* @brief CPSS DxCh Stream Gate Control (SGC) configurations APIs
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChStreamhGateControl
#define __cpssDxChStreamhGateControl

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>



/*************************************************************************************************************
  constants & structures definitions
 *************************************************************************************************************/

/* Maximum number of Time slots per table set */
#define CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS        256

/* Maximum number of interval max profiles */
#define CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS      256


/**
 * @enum CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_ENT
 *
 * @brief Indicates the nanoseconds resolution for port speeds
 */
typedef enum{

    /** @brief :Granularity is 16 nanoseconds
     */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_16NS_E,

    /** @brief :Granularity is 32 nanoseconds
     */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_32NS_E,

    /** @brief :Granularity is 64 nanoseconds
     */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_64NS_E,

    /** @brief :Granularity is 128 nanoseconds
     */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E

} CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_ENT;

/**
 * @enum CPSS_DXCH_STREAM_SGC_GATE_STATE_ENT
 *
 * @brief Stream gate state
 */
typedef enum{

    /** @brief :Gate is open
    */
    CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E,

    /** @brief :Gate is close
    */
    CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E

} CPSS_DXCH_STREAM_SGC_GATE_STATE_ENT;

/**
 * @struct CPSS_DXCH_STREAM_SGC_EXCEPTION_CONFIG_STC
 *
 * @brief Stream Gate Control (SGC) exception parameters
 */
typedef struct{

    /** @brief  The packet command.
     *  (APPLICABLE VALUES: CPSS_PACKET_CMD_FORWARD_E
     *                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
     *                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *                      CPSS_PACKET_CMD_DROP_HARD_E
     *                      CPSS_PACKET_CMD_DROP_SOFT_E)
     */
    CPSS_PACKET_CMD_ENT     command;

    /** @brief  The CPU code.
     */
    CPSS_NET_RX_CPU_CODE_ENT     cpuCode;

} CPSS_DXCH_STREAM_SGC_EXCEPTION_CONFIG_STC;

/**
 * @struct CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC
 *
 * @brief Global parameters that are required for the configuration of Stream Gate Control (SGC)
 */
typedef struct{

    /** @brief  Option of filtering an incoming packet based on GOP timestamp or current TOD
     *  GT_FALSE - Use always the GOP timestamp
     *  GT_TRUE  - Use of free running TAI and ignore the GOP timestamp
     */
     GT_BOOL     sgcIgnoreTimestampUseTod;

    /** @brief  Indicates if Byte Count Mode is L2 or L3.
     *  GT_FALSE - Byte Count Mode is L2
     *  GT_TRUE  - Byte Count Mode is L3
     */
    GT_BOOL     sgcCountingModeL3;

    /** @brief  Gate closed exception parameters.
     */
    CPSS_DXCH_STREAM_SGC_EXCEPTION_CONFIG_STC     gateClosedException;

    /** @brief  Interval Max Octet Exceeded exception parameters.
     */
    CPSS_DXCH_STREAM_SGC_EXCEPTION_CONFIG_STC     intervalMaxOctetExceededException;

    /** @brief  Enable to the SGC Algorithm.
     *  GT_FALSE - Disable
     *  GT_TRUE  - Enable
     */
    GT_BOOL     sgcGlobalEnable;

} CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC;

/**
 * @struct CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC
 *
 * @brief Parameters to indicate the time for the reconfiguration to take place
 */
typedef struct {

     /** @brief Reconfiguration of Time Of Day (TOD) MSbits
      *  (APPLICABLE RANGES: 0..2^48 -1)
      */
    GT_U64     todMsb;

     /** @brief Reconfiguration of Time Of Day (TOD) LSBits: 30 bits ns
      *  (APPLICABLE RANGES: 0..2^30 -1)
      */
    GT_U32     todLsb;

     /** @brief Reconfiguration time Extension in ns
      *  (APPLICABLE RANGES: 0..2^16 -1)
      */
    GT_U32     configChangeTimeExtension;

} CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC;

/**
 * @struct CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC
 *
 * @brief Holds table set information
 */
typedef struct {

     /** @brief  Indicates the nanoseconds resolution for port speeds
      */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_ENT   remainingBitsResolution;

     /** @brief  Indicates the polarity of the Byte Counter Adjust (Negative or Positive)
      *  GT_FALSE - Decrease, Byte count adjust is negative
      *  GT_TRUE  - Increase, Byte count adjust is positive
      */
    GT_BOOL     byteCountAdjustPolarity;

     /** @brief  Packet byte count adjust if byte count is known (different than 0x3fff)
      *  Byte count = Desc<byte count> + adjust
      *  (APPLICABLE RANGES: 0..127)
      */
    GT_U32     byteCountAdjust;

     /** @brief  Number of bit per remainingBitsResolution ns
      *  (APPLICABLE RANGES: 0..255)
      */
    GT_U32     remainingBitsFactor;

     /** @brief  Table set cycle time
      */
    GT_U32     cycleTime;

     /** @brief  Delay (in Nano sec) offset of the ingress timestamp that the MAC assigned
      *  (APPLICABLE RANGES: 0..2^16 -1)
      */
    GT_U32     ingressTimestampOffset;

     /** @brief  Negative Delay (in Nano sec) adjust from the port to the Gate Control Logic (GCL) unit
      *  <GCL TOD> = TOD - <GCL ingress time offset>
      *  (APPLICABLE RANGES: 0..2^16 -1)
      */
    GT_U32     ingressTodOffset;

} CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC;

/**
 * @struct CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC
 *
 * @brief Parameters that are required time slot configurations
 */
typedef struct{

     /** @brief  Delay (in Nano sec) from the Cycle start to transition to the next time slot.
      *  values should represent time difference from one time slot to previous one.
      */
    GT_U32     timeToAdvance;

     /** @brief Pointer (indirection to profile) to IntervalMax number
      *  (maximum bytes allowed during the timer interval)
      *  (APPLICABLE RANGES: 0..255)
      */
    GT_U32     intervalMaxOctetProfile;

     /** @brief Select if to keep incoming TC or set it to Internal Priority Value (IPV)
      *  GT_TRUE - keep incoming TC
      *  GT_FALSE - set TC to Internal Priority Value (IPV)
      */
    GT_BOOL     keepTc;

     /** @brief Set Internal Priority Value (IPV). Applicable only if keepTc is false.
      *  (APPLICABLE RANGES: 0..7)
      */
    GT_U32     ipv;

     /** @brief Packet length aware
      *  GT_FALSE - Determine gate O/C based on Start Of Packet (SOP) time
      *  GT_TRUE - Determine gate O/C based on both Start Of Packet (SOP) and End Of Packet (EOP) time
      */
    GT_BOOL     lengthAware;

     /** @brief Open/close state
     */
    CPSS_DXCH_STREAM_SGC_GATE_STATE_ENT     streamGateState;

     /** @brief Indication whether to reset or continue the byte counting
      *  GT_FALSE - continue
      *  GT_TRUE - reset the octet counter
      */
    GT_BOOL     newSlot;

} CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC;

/**
 * @struct CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC
 *
 * @brief Parameters that are required time slot configurations
 */
typedef struct{

     /** @brief  Delay (in Nano sec) from the Cycle start to transition to the next time slot.
      *  values should represent time difference from one time slot to previous one.
      */
    GT_U32     timeToAdvance;

   /** @brief Per gate bit indicates if gate is opened or closed in current time slot.
      *      0x0 = Open; Open; Gate Open; gate open;
      *     0x1 = Close; Close; Gate Close; gate close;
      *    (APPLICABLE RANGES: 0..255)
      */
    GT_U32     gateStateBmp;

     /** @brief Signals whether to hold preemptable port on current time slot.
      *  GT_TRUE - Port Hold
      *  GT_FALSE -Port Release
      */
    GT_BOOL     hold;

} CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC;


/**
 * @struct CPSS_DXCH_STREAM_SGC_GATE_INFO_STC
 *
 * @brief Parameters that are required for on the fly configuration of an active gates tables set
 */
typedef struct{

     /** @brief  The maximum allowed number of bytes per time slot.
      */
    GT_U32     intervalMaxArr[CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS];

     /** @brief  Table Set configurations
      */
    CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC     tableSetInfo;

    /** @brief  Time Slot configurations per gate
     */
    CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC  timeSlotInfoArr[CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS];

} CPSS_DXCH_STREAM_SGC_GATE_INFO_STC;


/**
 * @struct CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_ENT
 *
 * @brief Enum specify tpe of port channel to bind to table (Applicable for preemption capable ports.)
 */
typedef enum {
    /** @brief Bind express channel.
     */
    CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_EXP_E,
    /** @brief Bind preemptive channel.
     */
    CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_PRE_E,

} CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_ENT;


/**
 * @struct CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_REQ_STC
 *
 * @brief Configuration request parameters
 */
typedef struct
{
    /** @brief  Last table entry
      *  (APPLICABLE RANGES: 0..255)
      */

  GT_U32  lastEntry;
 /** @brief Packet length aware
      *  GT_FALSE - Determine gate O/C based on Start Of Packet (SOP) time
      *  GT_TRUE - Determine gate O/C based on both Start Of Packet (SOP) and End Of Packet (EOP) time
      */
  GT_BOOL lengthAware;

  /** @brief  Number of bit per Port_Remain_Per_NS_Resolution .
   The formula is

   remainingBitsFactor = 2^(bitsFactorResolution )*(Port speed in bits /1000000000)

 *  (APPLICABLE RANGES: 0..2^11-1)
 */
  GT_U32     remainingBitsFactor;

  /*brief   Define ns resolution of remain_per_ns field in config_change_time_req register
                    Resolution is 2^Port Remain Per NS Resolution
                                                           Example:
                                                           0 - defines port speed: bits in 1 ns
                                                           1- defines port speed: bits in 2 ns
                                                           2 - defines port speed: bits in 4 ns
                                                           3 - defines port speed: bits in 8 ns
                                                           4 - defines port speed: bits in 16 ns
                                                           and so on
                                                           Default value 0x7 is 128 nanosec

                                                           (APPLICABLE RANGES: 0..65535)


  */
  GT_U32     bitsFactorResolution;

  /** @brief  Table set cycle time
      */
  GT_U32  cycleTime;

    /** @brief TOD offset in nano sec from actual port transmission time.
   *  (APPLICABLE RANGES: 0..2^30 -1)
   */
  GT_U32     egressTodOffset;

  /** @brief  Port MAX allowed BC Offset .
              Configured in Bytes
              Amount of configured Byte will be reduced from computed Max allowed BC
   *  (APPLICABLE RANGES: 0..2^14 -1)
   */
 GT_U32    maxAlwdBcOffset;

 /** @brief Type of channel (Express/preemptive) that is required for binding.
            Applicable for ports that support preemption.

 *  (APPLICABLE DEVICE: AC5P,Harrier,Ironman,AAS)
 */
  CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_ENT channelType;

} CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_REQ_STC;

/**
 * @struct CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC
 *
 * @brief Parameters that are required for binding port to  tables set
 */

typedef struct
{
  /** @brief   Configuration request  parameters
  */
  CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_REQ_STC gateReconfigRequestParam;

 /** @brief    execution time parameters*/
  CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC gateReconfigTimeParam;

} CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC;




/*************************************************************************************************************
  CPSS API functions declerations
 *************************************************************************************************************/


/**
* @internal cpssDxChStreamSgcGlobalConfigSet function
* @endinternal
*
* @brief  Set global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
);

/**
* @internal cpssDxChStreamSgcGlobalConfigGet function
* @endinternal
*
* @brief  Get global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum         - device number.
* @param[out] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGlobalConfigGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
);

/**
* @internal cpssDxChStreamSgcGateConfigSet function
* @endinternal
*
* @brief  Set direct gate configurations to take place upon API complete
*
*         Note: it might have some traffic affect
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] gateId                - the Gate-ID assigned to the flow
*                                    (APPLICABLE RANGES: 0..510)
*                                    0x1FF - indicates Gate Control List is disabled
* @param[in] tableSetId            - the Table Set ID that is mapped to the gateId
*                                    (APPLICABLE RANGES: 0..57)
* @param[in] slotsNum              - number of time slots to fill
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] imxProfilesNum        - number of interval max profiles
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGateConfigSet
(
    IN GT_U8                               devNum,
    IN GT_U32                              gateId,
    IN GT_U32                              tableSetId,
    IN GT_U32                              slotsNum,
    IN GT_U32                              imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  *gateParamsPtr
);

/**
* @internal cpssDxChStreamSgcGateConfigGet function
* @endinternal
*
* @brief  Get direct gate configurations
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - device number.
* @param[in]  gateId                - the Gate-ID assigned to the flow
*                                     (APPLICABLE RANGES: 0..510)
*                                     0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId            - the Table Set ID that is mapped to the gateId
*                                     (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum              - number of time slots to fill
*                                     (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum        - number of interval max profiles
*                                     (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGateConfigGet
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                gateId,
    IN  GT_U32                                tableSetId,
    IN  GT_U32                                slotsNum,
    IN  GT_U32                                imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC    *gateParamsPtr
);

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigSet function
* @endinternal
*
* @brief  Set time based gate configurations which takes place at specific time
*
* Note: in order to complete the reconfiguration process properly without affecting the
*       traffic and to enable next future reconfigurations, must call following CPSS API
*       cpssDxChStreamSgcTimeBasedGateReConfigComplete before reconfiguration time occurs.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                   - device number.
* @param[in]  gateId                   - the Gate-ID assigned to the flow
*                                        (APPLICABLE RANGES: 0..510)
*                                         0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId               - the Table Set ID that is mapped to the gateId
*                                        (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                 - number of time slots to fill
*                                        (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum           - number of interval max profiles
*                                        (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr             - (pointer to) gate parameters
* @param[in] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       gateId,
    IN GT_U32                                       tableSetId,
    IN GT_U32                                       slotsNum,
    IN GT_U32                                       imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    IN CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
);

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigGet function
* @endinternal
*
* @brief  Get time based gate re-configurations parameters
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                    - device number.
* @param[in]  gateId                    - the Gate-ID assigned to the flow
*                                         (APPLICABLE RANGES: 0..510)
*                                          0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId                - the Table Set ID that is mapped to the gateId
*                                         (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                  - number of time slots to fill
*                                         (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum            - number of interval max profiles
*                                         (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr             - (pointer to) gate parameters
* @param[out] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       gateId,
    IN  GT_U32                                       tableSetId,
    IN  GT_U32                                       slotsNum,
    IN  GT_U32                                       imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    OUT CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
);

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigComplete function
* @endinternal
*
* @brief  Read re-configuration pending bit to verify if re-configuration time was matched.
*         If matched continue with last configurations to enable future re-configurations.
*         - get re-configuration mapping of GCL ID to Table Set ID
*         - set new mapping according to re-configuration mapping
*         - clear Valid status bit to indicate process is ready
*         - Set the Table Set to active
*
*         Note:
*         - Must call this API after re-configuration time.
*           If API is called before time is matched it will return with failure status.
*         - Must call CPSS API cpssDxChStreamSgcTimeBasedGateReConfigSet first
*           to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - if pending status bit was not cleared
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigComplete
(
    IN GT_U8     devNum
);

/**
* @internal cpssDxChStreamEgressTableSetConfigSet function
* @endinternal
*
* @brief  802.1Qbv  - Configure tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[in] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressTableSetConfigSet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    IN CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
);

/**
* @internal cpssDxChStreamEgressTableSetConfigGet function
* @endinternal
*
* @brief  802.1Qbv  - Get  tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[out] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressTableSetConfigGet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
);

/**
* @internal cpssDxChStreamEgressPortBindSet function
* @endinternal
*
* @brief  Bind port to spesific table set using bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[in] tableSetId            - the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[in] bindEntryPtr         - (pointer to) bind entry  parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamEgressPortBindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
);

/**
* @internal cpssDxChStreamEgressPortBindGet function
* @endinternal
*
* @brief  Get  port  bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[out] tableSetPtr            - (pointer to) the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[out] bindEntryPtr         - (pointer to) bind entry  parameters.
*         Note that reconfiguration time is not updated .(i.e gateReconfigTimeParam)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamEgressPortBindGet
(
    IN   GT_U8                devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32               *tableSetPtr,
    OUT  CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
);


/**
* @internal cpssDxChStreamEgressPortBindComplete function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
*         Note: must call CPSS API cpssDxChStreamEgressPortBindSet first
*               to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT - after max number of retries checking if pending status bit is cleared
*
*/
GT_STATUS cpssDxChStreamEgressPortBindComplete
(
    IN GT_U8     devNum
);

/**
* @internal cpssDxChStreamEgressPortUnbindSet function
* @endinternal
*
* @brief   Unbind port from  table set.
*
*         Note: Can not be performed under traffic.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE  - port under traffic
*/
GT_STATUS cpssDxChStreamEgressPortUnbindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal cpssDxChStreamEgressPortQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[in] gate                       - gate  that this queue  is mapped to(APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressPortQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    IN GT_U32 gate
);


/**
* @internal cpssDxChStreamEgressPortQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[out] gatePtr                       - (pointer to)gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressPortQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    OUT GT_U32 *gatePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChStreamh */

