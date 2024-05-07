/* -------------------------------------------------------------------------------------------------------
   Generated from MTS-Platform-Legacy-Yang-Module based on revision 2023-05-16:3.60 on 04/06/2023 16:40:45
   ------------------------------------------------------------------------------------------------------- */
#ifndef __PDL_MTS_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__
#define __PDL_MTS_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__
    /* typedef start */
    typedef struct
    {
        UINT_32 int_value;
        const char * str_value;
    } PDL_ENUM_STR2VALUE_STC;
    /* typedef end */
    /* typedef start */
    typedef enum
    {
        PDL_ENUM_ID_I2C_ACCESS_TYPE_E,
        PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E,
        PDL_ENUM_ID_MPP_PIN_MODE_TYPE_E,
        PDL_ENUM_ID_MPP_PIN_INIT_VALUE_TYPE_E,
        PDL_ENUM_ID_INTERFACE_TYPE_E,
        PDL_ENUM_ID_SMI_XSMI_TYPE_TYPE_E,
        PDL_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E,
        PDL_ENUM_ID_SPEED_TYPE_E,
        PDL_ENUM_ID_TRANSCEIVER_TYPE_E,
        PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E,
        PDL_ENUM_ID_RX_PARAM_TYPE_E,
        PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E,
        PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E,
        PDL_ENUM_ID_FAN_ROLE_TYPE_E,
        PDL_ENUM_ID_HW_FAN_OPERATION_TYPE_E,
        PDL_ENUM_ID_FAN_ADT7476_FAULT_DETECTION_TYPE_E,
        PDL_ENUM_ID_FAN_TYPE_E,
        PDL_ENUM_ID_BANK_SOURCE_TYPE_E,
        PDL_ENUM_ID_PSEPORT_TYPE_E,
        PDL_ENUM_ID_PSE_TYPE_E,
        PDL_ENUM_ID_POE_HARDWARE_TYPE_E,
        PDL_ENUM_ID_POE_MAPPING_METHOD_TYPE_E,
        PDL_ENUM_ID_POE_POWERED_PD_PORTS_TYPE_E,
        PDL_ENUM_ID_POE_MCU_TYPE_E,
        PDL_ENUM_ID_POE_VENDOR_TYPE_E,
        PDL_ENUM_ID_POE_COMMUNICATION_TYPE_E,
        PDL_ENUM_ID_POE_HOST_SERIAL_CHANNEL_ID_E,
        PDL_ENUM_ID_POWER_STATUS_TYPE_E,
        PDL_ENUM_ID_PHY_TYPE_TYPE_E,
        PDL_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E,
        PDL_ENUM_ID_COLOR_TYPE_E,
        PDL_ENUM_ID_LED_STATE_TYPE_E,
        PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E,
        PDL_ENUM_ID_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_E,
        PDL_ENUM_ID_PORT_LEDS_SUPPORTED_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_BLINK_SELECT_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_ORDER_MODE_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_BLINK_DURATION_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_PULSE_STRETCH_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_CLASS5_SELECT_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_CLASS13_SELECT_TYPE_E,
        PDL_ENUM_ID_LED_STREAM_PORT_TYPE_E,
        PDL_ENUM_ID_CPU_TYPE_TYPE_E,
        PDL_ENUM_ID_BUTTON_TYPE_E,
        PDL_ENUM_ID_SENSOR_HW_TYPE_E,
        PDL_ENUM_ID_SENSOR_FAN_NCT7802_TYPE_E,
        PDL_ENUM_ID_SENSOR_FAN_ADT7476_TYPE_E,
        PDL_ENUM_ID_THRESHOLD_TYPE_E,
        PDL_ENUM_ID_NEGOTIATION_MODE_TYPE_E
    } PDL_ENUM_IDS_ENT;
    /* typedef end */
    /* typedef start */
    typedef struct
    {
        PDL_ENUM_IDS_ENT enum_id;
        PDL_ENUM_STR2VALUE_STC * convertArr_PTR;
        UINT_32 arr_size;
    } PDL_ENUM_IDS_ENUM_STR2VALUE_STC;
    /* typedef end */
    typedef void * PRV_PDLIB_DB_TYP;
    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_ISRREGADDRESS_PARAMS_STC;

    typedef struct 
    {
         UINT_8 txamp;
         UINT_8 txamp_mask;
         UINT_8 emph1;
         UINT_8 emph1_mask;
         UINT_8 emph0;
         UINT_8 emph0_mask;
         BOOLEAN txampshft;
         UINT_8 txampshft_mask;
         UINT_8 slewrate;
         UINT_8 slewrate_mask;
         BOOLEAN txampadjen;
         UINT_8 txampadjen_mask;
         BOOLEAN txemphen;
         UINT_8 txemphen_mask;
    } PDL_LANE_ATTRIBUTE_TX_FINE_TUNE_PARAMS_STC;

    typedef enum 
    {
        PDL_I2C_ACCESS_TYPE_ONE_BYTE_E = 1,
        PDL_I2C_ACCESS_TYPE_TWO_BYTES_E = 2,
        PDL_I2C_ACCESS_TYPE_FOUR_BYTES_E = 3,
        PDL_I2C_ACCESS_TYPE_LAST_E = 4    
    } PDL_I2C_ACCESS_TYPE_ENT;
    /* PDL_ENUM_ID_I2C_ACCESS_TYPE_E */

    #define PDL_I2C_BUS_ID_TYPE_MIN_VALUE_CNS 0
    #define PDL_I2C_BUS_ID_TYPE_MAX_VALUE_CNS 1
    typedef UINT_8 PDL_I2C_BUS_ID_TYPE_TYP;

    #define PDL_I2C_ADDRESS_TYPE_MIN_VALUE_CNS 0
    #define PDL_I2C_ADDRESS_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 PDL_I2C_ADDRESS_TYPE_TYP;

    typedef UINT_16 PDL_I2C_OFFSET_TYPE_TYP;

    #define PDL_I2C_MASK_TYPE_MIN_VALUE_CNS 1
    #define PDL_I2C_MASK_TYPE_MAX_VALUE_CNS 65535
    typedef UINT_16 PDL_I2C_MASK_TYPE_TYP;

    typedef UINT_16 PDL_I2C_VALUE_TYPE_TYP;

    typedef enum 
    {
        PDL_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA_E = 1,
        PDL_I2C_TRANSACTION_TYPE_OFFSET_THEN_DATA_E = 2,
        PDL_I2C_TRANSACTION_TYPE_LAST_E = 3    
    } PDL_I2C_TRANSACTION_TYPE_ENT;
    /* PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E */

    #define PDL_MPP_DEVICE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_MPP_DEVICE_NUMBER_TYPE_MAX_VALUE_CNS 255
    typedef UINT_8 PDL_MPP_DEVICE_NUMBER_TYPE_TYP;

    typedef enum 
    {
        PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_PIN_E = 1,
        PDL_MPP_PIN_MODE_TYPE_GPIO_WRITABLE_PIN_E = 2,
        PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_WRITABLE_PIN_E = 3,
        PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_INTERRUPT_PIN_E = 4,
        PDL_MPP_PIN_MODE_TYPE_LAST_E = 5    
    } PDL_MPP_PIN_MODE_TYPE_ENT;
    /* PDL_ENUM_ID_MPP_PIN_MODE_TYPE_E */

    #define PDL_MPP_PIN_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_MPP_PIN_NUMBER_TYPE_MAX_VALUE_CNS 63
    typedef UINT_8 PDL_MPP_PIN_NUMBER_TYPE_TYP;

    #define PDL_MPP_PIN_VALUE_TYPE_MIN_VALUE_CNS 0
    #define PDL_MPP_PIN_VALUE_TYPE_MAX_VALUE_CNS 1
    typedef UINT_8 PDL_MPP_PIN_VALUE_TYPE_TYP;

    typedef enum 
    {
        PDL_MPP_PIN_INIT_VALUE_TYPE_0_E = 1,
        PDL_MPP_PIN_INIT_VALUE_TYPE_1_E = 2,
        PDL_MPP_PIN_INIT_VALUE_TYPE_HW_E = 3,
        PDL_MPP_PIN_INIT_VALUE_TYPE_LAST_E = 4    
    } PDL_MPP_PIN_INIT_VALUE_TYPE_ENT;
    /* PDL_ENUM_ID_MPP_PIN_INIT_VALUE_TYPE_E */

    #define PDL_GPIO_DEVICE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_GPIO_DEVICE_NUMBER_TYPE_MAX_VALUE_CNS 255
    typedef UINT_8 PDL_GPIO_DEVICE_NUMBER_TYPE_TYP;
/* Converted gpio-device-number-type to mpp-device-number-type */

    #define PDL_GPIO_PIN_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_GPIO_PIN_NUMBER_TYPE_MAX_VALUE_CNS 63
    typedef UINT_8 PDL_GPIO_PIN_NUMBER_TYPE_TYP;
/* Converted gpio-pin-number-type to mpp-pin-number-type */

    #define PDL_GPIO_PIN_VALUE_TYPE_MIN_VALUE_CNS 0
    #define PDL_GPIO_PIN_VALUE_TYPE_MAX_VALUE_CNS 1
    typedef UINT_8 PDL_GPIO_PIN_VALUE_TYPE_TYP;
/* Converted gpio-pin-value-type to mpp-pin-value-type */

    typedef struct 
    {
         UINT_8 gpioTdo;
         UINT_8 gpioTdo_mask;
         UINT_8 gpioTms;
         UINT_8 gpioTms_mask;
         UINT_8 gpioTck;
         UINT_8 gpioTck_mask;
         PDL_MPP_DEVICE_NUMBER_TYPE_TYP cpldPinsDeviceNum;
         UINT_8 cpldPinsDeviceNum_mask;
         UINT_8 gpioTdi;
         UINT_8 gpioTdi_mask;
    } PDL_ONLINE_UPGRADE_PARAMS_STC;

    typedef enum 
    {
        PDL_INTERFACE_TYPE_INTERFACE_GPIO_E = 1,
        PDL_INTERFACE_TYPE_INTERFACE_I2C_E = 2,
        PDL_INTERFACE_TYPE_INTERFACE_PPREG_E = 3,
        PDL_INTERFACE_TYPE_LAST_E = 4    
    } PDL_INTERFACE_TYPE_ENT;
    /* PDL_ENUM_ID_INTERFACE_TYPE_E */

    #define PDL_SMI_XSMI_INTERFACE_ID_TYPE_MIN_VALUE_CNS 0
    #define PDL_SMI_XSMI_INTERFACE_ID_TYPE_MAX_VALUE_CNS 3
    typedef UINT_8 PDL_SMI_XSMI_INTERFACE_ID_TYPE_TYP;

    typedef struct 
    {
         UINT_8 gpioReadDeviceNumber;
         UINT_8 gpioReadDeviceNumber_mask;
         PDL_MPP_PIN_NUMBER_TYPE_TYP gpioReadPinNumber;
         UINT_8 gpioReadPinNumber_mask;
    } PDL_GPIO_READ_ADDRESS_PARAMS_STC;

    #define PDL_SMI_XSMI_ADDRESS_TYPE_MIN_VALUE_CNS 0
    #define PDL_SMI_XSMI_ADDRESS_TYPE_MAX_VALUE_CNS 31
    typedef UINT_8 PDL_SMI_XSMI_ADDRESS_TYPE_TYP;

    typedef struct 
    {
         PDL_MPP_PIN_NUMBER_TYPE_TYP gpioWritePinNumber;
         UINT_8 gpioWritePinNumber_mask;
         UINT_8 gpioWriteDeviceNumber;
         UINT_8 gpioWriteDeviceNumber_mask;
    } PDL_GPIO_WRITE_ADDRESS_PARAMS_STC;

    typedef UINT_16 PDL_SMI_XSMI_REGISTER_TYPE_TYP;

    #define PDL_SMI_XSMI_DEVICE_OR_PAGE_TYPE_MIN_VALUE_CNS 0
    #define PDL_SMI_XSMI_DEVICE_OR_PAGE_TYPE_MAX_VALUE_CNS 255
    typedef UINT_8 PDL_SMI_XSMI_DEVICE_OR_PAGE_TYPE_TYP;

    typedef enum 
    {
        PDL_SMI_XSMI_TYPE_TYPE_SMI_E = 1,
        PDL_SMI_XSMI_TYPE_TYPE_XSMI_E = 2,
        PDL_SMI_XSMI_TYPE_TYPE_LAST_E = 3    
    } PDL_SMI_XSMI_TYPE_TYPE_ENT;
    /* PDL_ENUM_ID_SMI_XSMI_TYPE_TYPE_E */

    #define PDL_SMI_XSMI_MASK_VALUE_TYPE_MIN_VALUE_CNS 0
    #define PDL_SMI_XSMI_MASK_VALUE_TYPE_MAX_VALUE_CNS 65535
    typedef UINT_16 PDL_SMI_XSMI_MASK_VALUE_TYPE_TYP;

    #define PDL_NUMBER_OF_PPS_TYPE_MIN_VALUE_CNS 1
    #define PDL_NUMBER_OF_PPS_TYPE_MAX_VALUE_CNS 2
    typedef UINT_8 PDL_NUMBER_OF_PPS_TYPE_TYP;

    #define PDL_PP_DEVICE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_PP_DEVICE_NUMBER_TYPE_MAX_VALUE_CNS 1
    typedef UINT_8 PDL_PP_DEVICE_NUMBER_TYPE_TYP;

    #define PDL_PP_PORT_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_PP_PORT_NUMBER_TYPE_MAX_VALUE_CNS 271
    typedef UINT_32 PDL_PP_PORT_NUMBER_TYPE_TYP;

    #define PDL_PANEL_GROUP_NUMBER_TYPE_MIN_VALUE_CNS 1
    #define PDL_PANEL_GROUP_NUMBER_TYPE_MAX_VALUE_CNS 4
    typedef UINT_8 PDL_PANEL_GROUP_NUMBER_TYPE_TYP;

    typedef UINT_8 PDL_PANEL_PORT_NUMBER_TYPE_TYP;

    #define PDL_LOGICAL_PORT_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_LOGICAL_PORT_NUMBER_TYPE_MAX_VALUE_CNS 61
    typedef UINT_32 PDL_LOGICAL_PORT_NUMBER_TYPE_TYP;

    typedef enum 
    {
        PDL_PANEL_GROUP_ORDERING_TYPE_RIGHTDOWN_E = 1,
        PDL_PANEL_GROUP_ORDERING_TYPE_DOWNRIGHT_E = 2,
        PDL_PANEL_GROUP_ORDERING_TYPE_SINGLE2NDROW_E = 3,
        PDL_PANEL_GROUP_ORDERING_TYPE_SINGLE1STROW_E = 4,
        PDL_PANEL_GROUP_ORDERING_TYPE_LAST_E = 5    
    } PDL_PANEL_GROUP_ORDERING_TYPE_ENT;
    /* PDL_ENUM_ID_PANEL_GROUP_ORDERING_TYPE_E */

    typedef struct 
    {
         PDL_SMI_XSMI_DEVICE_OR_PAGE_TYPE_TYP phyDeviceOrPage;
         UINT_8 phyDeviceOrPage_mask;
         PDL_SMI_XSMI_MASK_VALUE_TYPE_TYP phyValue;
         UINT_8 phyValue_mask;
         PDL_SMI_XSMI_REGISTER_TYPE_TYP phyRegister;
         UINT_8 phyRegister_mask;
         PDL_SMI_XSMI_MASK_VALUE_TYPE_TYP phyMask;
         UINT_8 phyMask_mask;
    } PDL_PHY_MASK_VALUE_PARAMS_STC;

    typedef enum 
    {
        PDL_SPEED_TYPE_10M_E = 1,
        PDL_SPEED_TYPE_100M_E = 2,
        PDL_SPEED_TYPE_1G_E = 3,
        PDL_SPEED_TYPE_2_5G_E = 4,
        PDL_SPEED_TYPE_5G_E = 5,
        PDL_SPEED_TYPE_10G_E = 6,
        PDL_SPEED_TYPE_20G_E = 7,
        PDL_SPEED_TYPE_24G_E = 8,
        PDL_SPEED_TYPE_25G_E = 9,
        PDL_SPEED_TYPE_40G_E = 10,
        PDL_SPEED_TYPE_50G_E = 11,
        PDL_SPEED_TYPE_100G_E = 12,
        PDL_SPEED_TYPE_22G_E = 13,
        PDL_SPEED_TYPE_LAST_E = 14    
    } PDL_SPEED_TYPE_ENT;
    /* PDL_ENUM_ID_SPEED_TYPE_E */

    #define PDL_LANE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_LANE_NUMBER_TYPE_MAX_VALUE_CNS 257
    typedef UINT_32 PDL_LANE_NUMBER_TYPE_TYP;

    #define PDL_PORT_LANE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_PORT_LANE_NUMBER_TYPE_MAX_VALUE_CNS 7
    typedef UINT_8 PDL_PORT_LANE_NUMBER_TYPE_TYP;

    typedef struct 
    {
         UINT_32 classnum;
         UINT_8 classnum_mask;
         BOOLEAN polarityenable;
         UINT_8 polarityenable_mask;
         UINT_8 ledinterface;
         UINT_8 ledinterface_mask;
         UINT_8 ledposition;
         UINT_8 ledposition_mask;
    } PDL_LEDSTREAM_PORT_INIT_PARAMS_STC;

    typedef enum 
    {
        PDL_TRANSCEIVER_TYPE_FIBER_E = 1,
        PDL_TRANSCEIVER_TYPE_COPPER_E = 2,
        PDL_TRANSCEIVER_TYPE_COMBO_E = 3,
        PDL_TRANSCEIVER_TYPE_LAST_E = 4    
    } PDL_TRANSCEIVER_TYPE_ENT;
    /* PDL_ENUM_ID_TRANSCEIVER_TYPE_E */

    typedef enum 
    {
        PDL_CONNECTOR_TYPE_TYPE_SFPPLUS_E = 1,
        PDL_CONNECTOR_TYPE_TYPE_DAC_E = 2,
        PDL_CONNECTOR_TYPE_TYPE_RJ45_E = 3,
        PDL_CONNECTOR_TYPE_TYPE_LAST_E = 4    
    } PDL_CONNECTOR_TYPE_TYPE_ENT;
    /* PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E */

    typedef struct 
    {
         BOOLEAN txSwap;
         UINT_8 txSwap_mask;
         BOOLEAN rxSwap;
         UINT_8 rxSwap_mask;
    } PDL_LANE_ATTRIBUTES_GROUP_PARAMS_STC;

    typedef enum 
    {
        PDL_RX_PARAM_TYPE_SQLCH_E = 1,
        PDL_RX_PARAM_TYPE_FFERES_E = 2,
        PDL_RX_PARAM_TYPE_FFECAP_E = 3,
        PDL_RX_PARAM_TYPE_ALIGN90_E = 4,
        PDL_RX_PARAM_TYPE_DCGAIN_E = 5,
        PDL_RX_PARAM_TYPE_BANDWIDTH_E = 6,
        PDL_RX_PARAM_TYPE_LAST_E = 7    
    } PDL_RX_PARAM_TYPE_ENT;
    /* PDL_ENUM_ID_RX_PARAM_TYPE_E */

    typedef enum 
    {
        PDL_CONNECTOR_TECHNOLOGY_TYPE_ANY_E = 1,
        PDL_CONNECTOR_TECHNOLOGY_TYPE_PASSIVE_E = 2,
        PDL_CONNECTOR_TECHNOLOGY_TYPE_ACTIVE_E = 3,
        PDL_CONNECTOR_TECHNOLOGY_TYPE_LAST_E = 4    
    } PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT;
    /* PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E */

    typedef enum 
    {
        PDL_L1_INTERFACE_MODE_TYPE_SGMII_E = 1,
        PDL_L1_INTERFACE_MODE_TYPE_1000BASE_X_E = 3,
        PDL_L1_INTERFACE_MODE_TYPE_100BASE_FX_E = 7,
        PDL_L1_INTERFACE_MODE_TYPE_KR_E = 8,
        PDL_L1_INTERFACE_MODE_TYPE_SR_LR_E = 9,
        PDL_L1_INTERFACE_MODE_TYPE_QSGMII_E = 10,
        PDL_L1_INTERFACE_MODE_TYPE_XHGS_E = 13,
        PDL_L1_INTERFACE_MODE_TYPE_KR2_E = 14,
        PDL_L1_INTERFACE_MODE_TYPE_KR4_E = 16,
        PDL_L1_INTERFACE_MODE_TYPE_SXGMII_E = 17,
        PDL_L1_INTERFACE_MODE_TYPE_DXGMII_E = 18,
        PDL_L1_INTERFACE_MODE_TYPE_QUSGMII_E = 19,
        PDL_L1_INTERFACE_MODE_TYPE_QXGMII_E = 20,
        PDL_L1_INTERFACE_MODE_TYPE_OUSGMII_E = 21,
        PDL_L1_INTERFACE_MODE_TYPE_OXGMII_E = 22,
        PDL_L1_INTERFACE_MODE_TYPE_XHGS_SR_E = 23,
        PDL_L1_INTERFACE_MODE_TYPE_2500BASE_X_E = 24,
        PDL_L1_INTERFACE_MODE_TYPE_CR_E = 25,
        PDL_L1_INTERFACE_MODE_TYPE_LAST_E = 26    
    } PDL_L1_INTERFACE_MODE_TYPE_ENT;
    /* PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E */

    #define PDL_LIST_NUMBER_TYPE_MIN_VALUE_CNS 1
    #define PDL_LIST_NUMBER_TYPE_MAX_VALUE_CNS 65535
    typedef UINT_16 PDL_LIST_NUMBER_TYPE_TYP;

    typedef char * PDL_LIST_COMMENT_TYPE_TYP;

    #define PDL_FAN_ID_TYPE_MIN_VALUE_CNS 1
    #define PDL_FAN_ID_TYPE_MAX_VALUE_CNS 16
    typedef UINT_8 PDL_FAN_ID_TYPE_TYP;

    #define PDL_FAN_NUMBER_TYPE_MIN_VALUE_CNS 1
    #define PDL_FAN_NUMBER_TYPE_MAX_VALUE_CNS 8
    typedef UINT_8 PDL_FAN_NUMBER_TYPE_TYP;

    #define PDL_FAN_CONTROLLER_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_FAN_CONTROLLER_NUMBER_TYPE_MAX_VALUE_CNS 16
    typedef UINT_8 PDL_FAN_CONTROLLER_NUMBER_TYPE_TYP;

    typedef enum 
    {
        PDL_FAN_ROLE_TYPE_NORMAL_E = 1,
        PDL_FAN_ROLE_TYPE_REDUNDANT_E = 2,
        PDL_FAN_ROLE_TYPE_LAST_E = 3    
    } PDL_FAN_ROLE_TYPE_ENT;
    /* PDL_ENUM_ID_FAN_ROLE_TYPE_E */

    typedef enum 
    {
        PDL_HW_FAN_OPERATION_TYPE_INITIALIZE_E = 1,
        PDL_HW_FAN_OPERATION_TYPE_ALL_FAN_MAX_SPEED_E = 2,
        PDL_HW_FAN_OPERATION_TYPE_ALL_FAN_NORMAL_SPEED_E = 3,
        PDL_HW_FAN_OPERATION_TYPE_ENABLE_FAN_E = 4,
        PDL_HW_FAN_OPERATION_TYPE_DISABLE_REDUNDANT_FAN_E = 5,
        PDL_HW_FAN_OPERATION_TYPE_LAST_E = 6    
    } PDL_HW_FAN_OPERATION_TYPE_ENT;
    /* PDL_ENUM_ID_HW_FAN_OPERATION_TYPE_E */

    typedef enum 
    {
        PDL_FAN_ADT7476_FAULT_DETECTION_TYPE_INTERRUPT_STATUS_E = 1,
        PDL_FAN_ADT7476_FAULT_DETECTION_TYPE_TACHI_LOW_BYTE_E = 2,
        PDL_FAN_ADT7476_FAULT_DETECTION_TYPE_LAST_E = 3    
    } PDL_FAN_ADT7476_FAULT_DETECTION_TYPE_ENT;
    /* PDL_ENUM_ID_FAN_ADT7476_FAULT_DETECTION_TYPE_E */

    typedef enum 
    {
        PDL_FAN_TYPE_TC654_E = 1,
        PDL_FAN_TYPE_ADT7476_E = 2,
        PDL_FAN_TYPE_EMC2305_E = 3,
        PDL_FAN_TYPE_PWM_E = 4,
        PDL_FAN_TYPE_USERDEFINED_E = 5,
        PDL_FAN_TYPE_NCT7802_E = 6,
        PDL_FAN_TYPE_LAST_E = 7    
    } PDL_FAN_TYPE_ENT;
    /* PDL_ENUM_ID_FAN_TYPE_E */

    #define PDL_BANK_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_BANK_NUMBER_TYPE_MAX_VALUE_CNS 15
    typedef UINT_8 PDL_BANK_NUMBER_TYPE_TYP;

    typedef struct 
    {
         UINT_8 txamp;
         UINT_8 txamp_mask;
         UINT_8 emph1;
         UINT_8 emph1_mask;
         UINT_8 emph0;
         UINT_8 emph0_mask;
         BOOLEAN txampshft;
         UINT_8 txampshft_mask;
         UINT_8 slewrate;
         UINT_8 slewrate_mask;
         BOOLEAN txampadjen;
         UINT_8 txampadjen_mask;
         BOOLEAN txemphen;
         UINT_8 txemphen_mask;
    } PDL_LANE_ATTRIBUTES_INFO_TX_FINE_TUNE_PARAMS_STC;

    typedef UINT_16 PDL_BANK_WATTS_TYPE_TYP;

    typedef enum 
    {
        PDL_BANK_SOURCE_TYPE_PS_E = 1,
        PDL_BANK_SOURCE_TYPE_RPS_E = 2,
        PDL_BANK_SOURCE_TYPE_PDPORTS_E = 3,
        PDL_BANK_SOURCE_TYPE_LAST_E = 4    
    } PDL_BANK_SOURCE_TYPE_ENT;
    /* PDL_ENUM_ID_BANK_SOURCE_TYPE_E */

    #define PDL_PSEINDEX_TYPE_MIN_VALUE_CNS 0
    #define PDL_PSEINDEX_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 PDL_PSEINDEX_TYPE_TYP;

    typedef enum 
    {
        PDL_PSEPORT_TYPE_AF_E = 1,
        PDL_PSEPORT_TYPE_AT_E = 2,
        PDL_PSEPORT_TYPE_60W_E = 3,
        PDL_PSEPORT_TYPE_BT_TYPE2_E = 4,
        PDL_PSEPORT_TYPE_BT_TYPE3_E = 5,
        PDL_PSEPORT_TYPE_BT_TYPE4_E = 6,
        PDL_PSEPORT_TYPE_LAST_E = 7    
    } PDL_PSEPORT_TYPE_ENT;
    /* PDL_ENUM_ID_PSEPORT_TYPE_E */

    typedef enum 
    {
        PDL_PSE_TYPE_UNDEFINED_E = 1,
        PDL_PSE_TYPE_690XX_E = 2,
        PDL_PSE_TYPE_691XX_E = 3,
        PDL_PSE_TYPE_692XX_E = 4,
        PDL_PSE_TYPE_2388XX_E = 5,
        PDL_PSE_TYPE_692BT_E = 6,
        PDL_PSE_TYPE_ADI_AT_E = 7,
        PDL_PSE_TYPE_ADI_BT_E = 8,
        PDL_PSE_TYPE_TI_BT_E = 9,
        PDL_PSE_TYPE_LAST_E = 10    
    } PDL_PSE_TYPE_ENT;
    /* PDL_ENUM_ID_PSE_TYPE_E */

    #define PDL_PSE_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_PSE_NUMBER_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 PDL_PSE_NUMBER_TYPE_TYP;

    #define PDL_PSE_ADDRESS_TYPE_MIN_VALUE_CNS 0
    #define PDL_PSE_ADDRESS_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 PDL_PSE_ADDRESS_TYPE_TYP;

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_PDREGINFOSIXTY_PARAMS_STC;

    typedef enum 
    {
        PDL_POE_HARDWARE_TYPE_POE_NOT_SUPPORTED_E = 0,
        PDL_POE_HARDWARE_TYPE_POE_SUPPORT_E = 1,
        PDL_POE_HARDWARE_TYPE_POE_PLUS_SUPPORT_E = 2,
        PDL_POE_HARDWARE_TYPE_POE_60W_SUPPORT_E = 3,
        PDL_POE_HARDWARE_TYPE_POE_BT_TYPE2_SUPPORT_E = 4,
        PDL_POE_HARDWARE_TYPE_POE_BT_TYPE3_SUPPORT_E = 5,
        PDL_POE_HARDWARE_TYPE_POE_BT_TYPE4_SUPPORT_E = 6,
        PDL_POE_HARDWARE_TYPE_LAST_E = 7    
    } PDL_POE_HARDWARE_TYPE_ENT;
    /* PDL_ENUM_ID_POE_HARDWARE_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_PDREGINFOAF_PARAMS_STC;

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_PDREGINFOAT_PARAMS_STC;

    typedef enum 
    {
        PDL_POE_MAPPING_METHOD_TYPE_PORT_HW_TYPE_E = 0,
        PDL_POE_MAPPING_METHOD_TYPE_STANDARD_AF_E = 1,
        PDL_POE_MAPPING_METHOD_TYPE_LAST_E = 2    
    } PDL_POE_MAPPING_METHOD_TYPE_ENT;
    /* PDL_ENUM_ID_POE_MAPPING_METHOD_TYPE_E */

    typedef enum 
    {
        PDL_POE_POWERED_PD_PORTS_TYPE_PD_NOT_SUPPORTED_E = 0,
        PDL_POE_POWERED_PD_PORTS_TYPE_PD_SUPPORT_E = 1,
        PDL_POE_POWERED_PD_PORTS_TYPE_PD_PLUS_SUPPORT_E = 2,
        PDL_POE_POWERED_PD_PORTS_TYPE_PD_60W_SUPPORT_E = 3,
        PDL_POE_POWERED_PD_PORTS_TYPE_PD_BT_SUPPORT_E = 4,
        PDL_POE_POWERED_PD_PORTS_TYPE_LAST_E = 5    
    } PDL_POE_POWERED_PD_PORTS_TYPE_ENT;
    /* PDL_ENUM_ID_POE_POWERED_PD_PORTS_TYPE_E */

    typedef struct 
    {
         PDL_PDREGINFOAF_PARAMS_STC pdreginfoaf;
         UINT_8 pdreginfoaf_mask;
         BOOLEAN supported;
         UINT_8 supported_mask;
         PDL_PSEPORT_TYPE_ENT poePoweredPdPortsType;
         UINT_8 poePoweredPdPortsType_mask;
    } PDL_AFINFO_PARAMS_STC;

    typedef enum 
    {
        PDL_POE_MCU_TYPE_DRAGONITE_E = 0,
        PDL_POE_MCU_TYPE_CM3_E = 1,
        PDL_POE_MCU_TYPE_LAST_E = 2    
    } PDL_POE_MCU_TYPE_ENT;
    /* PDL_ENUM_ID_POE_MCU_TYPE_E */

    typedef enum 
    {
        PDL_POE_VENDOR_TYPE_NONE_E = 0,
        PDL_POE_VENDOR_TYPE_MICROCHIP_E = 1,
        PDL_POE_VENDOR_TYPE_TI_E = 2,
        PDL_POE_VENDOR_TYPE_ADI_E = 3,
        PDL_POE_VENDOR_TYPE_LAST_E = 4    
    } PDL_POE_VENDOR_TYPE_ENT;
    /* PDL_ENUM_ID_POE_VENDOR_TYPE_E */

    typedef struct 
    {
         PDL_PDREGINFOAT_PARAMS_STC pdreginfoat;
         UINT_8 pdreginfoat_mask;
         BOOLEAN supported;
         UINT_8 supported_mask;
         PDL_PSEPORT_TYPE_ENT poePoweredPdPortsType;
         UINT_8 poePoweredPdPortsType_mask;
    } PDL_ATINFO_PARAMS_STC;

    typedef enum 
    {
        PDL_POE_COMMUNICATION_TYPE_COMMUNICATION_TYPE_SERIAL_E = 0,
        PDL_POE_COMMUNICATION_TYPE_COMMUNICATION_TYPE_I2C_E = 1,
        PDL_POE_COMMUNICATION_TYPE_COMMUNICATION_TYPE_MEMORY_E = 2,
        PDL_POE_COMMUNICATION_TYPE_LAST_E = 3    
    } PDL_POE_COMMUNICATION_TYPE_ENT;
    /* PDL_ENUM_ID_POE_COMMUNICATION_TYPE_E */

    typedef enum 
    {
        PDL_POE_HOST_SERIAL_CHANNEL_ID_UART_E = 0,
        PDL_POE_HOST_SERIAL_CHANNEL_ID_I2C_E = 1,
        PDL_POE_HOST_SERIAL_CHANNEL_ID_DRAGONITE_SHARED_MEMORY_E = 2,
        PDL_POE_HOST_SERIAL_CHANNEL_ID_IPC_SHARED_MEMORY_E = 3,
        PDL_POE_HOST_SERIAL_CHANNEL_ID_LAST_E = 4    
    } PDL_POE_HOST_SERIAL_CHANNEL_ID_ENT;
    /* PDL_ENUM_ID_POE_HOST_SERIAL_CHANNEL_ID_E */

    typedef enum 
    {
        PDL_POWER_STATUS_TYPE_STANDBY_E = 1,
        PDL_POWER_STATUS_TYPE_NOTAVAILABLE_E = 2,
        PDL_POWER_STATUS_TYPE_BACKUP_E = 3,
        PDL_POWER_STATUS_TYPE_READY_E = 4,
        PDL_POWER_STATUS_TYPE_LAST_E = 5    
    } PDL_POWER_STATUS_TYPE_ENT;
    /* PDL_ENUM_ID_POWER_STATUS_TYPE_E */

    typedef struct 
    {
         PDL_PDREGINFOSIXTY_PARAMS_STC pdreginfosixty;
         UINT_8 pdreginfosixty_mask;
         BOOLEAN supported;
         UINT_8 supported_mask;
         PDL_PSEPORT_TYPE_ENT poePoweredPdPortsType;
         UINT_8 poePoweredPdPortsType_mask;
    } PDL_SIXTYWINFO_PARAMS_STC;

    #define PDL_PHY_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_PHY_NUMBER_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 PDL_PHY_NUMBER_TYPE_TYP;

    #define PDL_PHY_POSITION_TYPE_MIN_VALUE_CNS 0
    #define PDL_PHY_POSITION_TYPE_MAX_VALUE_CNS 7
    typedef UINT_8 PDL_PHY_POSITION_TYPE_TYP;

    typedef enum 
    {
        PDL_PHY_TYPE_TYPE_NO_PHY_DIRECT_ATTACHED_FIBER_E = 1,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1543_E = 2,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1545_E = 3,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1548_E = 4,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1680_E = 5,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1680L_E = 6,
        PDL_PHY_TYPE_TYPE_ALASKA_88E151X_E = 7,
        PDL_PHY_TYPE_TYPE_ALASKA_88E3680_E = 8,
        PDL_PHY_TYPE_TYPE_ALASKA_88E32X0_E = 9,
        PDL_PHY_TYPE_TYPE_ALASKA_88E33X0_E = 10,
        PDL_PHY_TYPE_TYPE_ALASKA_88E20X0_E = 11,
        PDL_PHY_TYPE_TYPE_ALASKA_88E2180_E = 12,
        PDL_PHY_TYPE_TYPE_ALASKA_88E2540_E = 13,
        PDL_PHY_TYPE_TYPE_ALASKA_88X3540_E = 14,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1780_E = 15,
        PDL_PHY_TYPE_TYPE_ALASKA_88E2580_E = 16,
        PDL_PHY_TYPE_TYPE_ALASKA_88X3580_E = 17,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1112_E = 18,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1781_E = 19,
        PDL_PHY_TYPE_TYPE_ALASKA_88E1781_INTERNAL_E = 20,
        PDL_PHY_TYPE_TYPE_LAST_E = 21    
    } PDL_PHY_TYPE_TYPE_ENT;
    /* PDL_ENUM_ID_PHY_TYPE_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_ISRREGVALUE_PARAMS_STC;

    typedef enum 
    {
        PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_NONE_E = 1,
        PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_RAM_E = 2,
        PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_FLASH_E = 3,
        PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_LAST_E = 4    
    } PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENT;
    /* PDL_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E */

    typedef enum 
    {
        PDL_COLOR_TYPE_GREEN_E = 1,
        PDL_COLOR_TYPE_AMBER_E = 2,
        PDL_COLOR_TYPE_BLUE_E = 3,
        PDL_COLOR_TYPE_LAST_E = 4    
    } PDL_COLOR_TYPE_ENT;
    /* PDL_ENUM_ID_COLOR_TYPE_E */

    typedef enum 
    {
        PDL_LED_STATE_TYPE_SOLID_E = 1,
        PDL_LED_STATE_TYPE_LOWRATEBLINKING_E = 2,
        PDL_LED_STATE_TYPE_NORMALRATEBLINKING_E = 3,
        PDL_LED_STATE_TYPE_HIGHRATEBLINKING_E = 4,
        PDL_LED_STATE_TYPE_LAST_E = 5    
    } PDL_LED_STATE_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STATE_TYPE_E */

    #define PDL_STACK_LED_ID_TYPE_MIN_VALUE_CNS 1
    #define PDL_STACK_LED_ID_TYPE_MAX_VALUE_CNS 8
    typedef UINT_8 PDL_STACK_LED_ID_TYPE_TYP;

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_STATUSREGINFO_PARAMS_STC;

    typedef enum 
    {
        PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_GPIO_E = 1,
        PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_I2C_E = 2,
        PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_EXTERNAL_DRIVER_E = 3,
        PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_PP_REGISTER_E = 4,
        PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LAST_E = 5    
    } PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_ENT;
    /* PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E */

    typedef enum 
    {
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_GPIO_E = 1,
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_I2C_E = 2,
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_PHY_E = 3,
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_PP_E = 4,
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_EXTERNAL_DRIVER_E = 5,
        PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LAST_E = 6    
    } PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_ENT;
    /* PDL_ENUM_ID_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_E */

    typedef enum 
    {
        PDL_PORT_LEDS_SUPPORTED_TYPE_LEFT_AND_RIGHT_E = 1,
        PDL_PORT_LEDS_SUPPORTED_TYPE_SINGLE_E = 2,
        PDL_PORT_LEDS_SUPPORTED_TYPE_LAST_E = 3    
    } PDL_PORT_LEDS_SUPPORTED_TYPE_ENT;
    /* PDL_ENUM_ID_PORT_LEDS_SUPPORTED_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_CLEARCONTROL_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_VALUE_TYPE_TYP ledI2CValue;
         UINT_8 ledI2CValue_mask;
    } PDL_LED_I2C_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_VALUE_TYPE_TYP ledI2CValue;
         UINT_8 ledI2CValue_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2CMask;
         UINT_8 ledI2CMask_mask;
    } PDL_LED_PP_COLOR_I2C_PARAMS_STC;

    typedef enum 
    {
        PDL_LED_STREAM_BLINK_SELECT_TYPE_BLINK_SELECT_0_E = 1,
        PDL_LED_STREAM_BLINK_SELECT_TYPE_BLINK_SELECT_1_E = 2,
        PDL_LED_STREAM_BLINK_SELECT_TYPE_LAST_E = 3    
    } PDL_LED_STREAM_BLINK_SELECT_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_BLINK_SELECT_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_ORDER_MODE_TYPE_ORDER_MODE_BY_PORT_E = 1,
        PDL_LED_STREAM_ORDER_MODE_TYPE_ORDER_MODE_BY_CLASS_E = 2,
        PDL_LED_STREAM_ORDER_MODE_TYPE_LAST_E = 3    
    } PDL_LED_STREAM_ORDER_MODE_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_ORDER_MODE_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_BLINK_DUTY_CYCLE_0_E = 1,
        PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_BLINK_DUTY_CYCLE_1_E = 2,
        PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_BLINK_DUTY_CYCLE_2_E = 3,
        PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_BLINK_DUTY_CYCLE_3_E = 4,
        PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_LAST_E = 5    
    } PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_AFCONTROL_PARAMS_STC;

    typedef struct 
    {
         PDL_SMI_XSMI_TYPE_TYPE_ENT phySmiInterfaceType;
         UINT_8 phySmiInterfaceType_mask;
         PDL_PHY_POSITION_TYPE_TYP phyPosition;
         UINT_8 phyPosition_mask;
         PDL_SMI_XSMI_ADDRESS_TYPE_TYP phySmiXsmiAddress;
         UINT_8 phySmiXsmiAddress_mask;
         PDL_PHY_NUMBER_TYPE_TYP phyNumber;
         UINT_8 phyNumber_mask;
         PDL_SMI_XSMI_INTERFACE_ID_TYPE_TYP phySmiXsmiInterfaceId;
         UINT_8 phySmiXsmiInterfaceId_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP phyPpDevice;
         UINT_8 phyPpDevice_mask;
    } PDL_PHY_READ_WRITE_ADDRESS_PARAMS_STC;

    typedef enum 
    {
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_0_E = 1,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_1_E = 2,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_2_E = 3,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_3_E = 4,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_4_E = 5,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_5_E = 6,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_6_E = 7,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_BLINK_DURATION_7_E = 8,
        PDL_LED_STREAM_BLINK_DURATION_TYPE_LAST_E = 9    
    } PDL_LED_STREAM_BLINK_DURATION_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_BLINK_DURATION_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_0_NO_E = 1,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_1_E = 2,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_2_E = 3,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_3_E = 4,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_4_E = 5,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_5_E = 6,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_6_E = 7,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_PULSE_STRETCH_7_E = 8,
        PDL_LED_STREAM_PULSE_STRETCH_TYPE_LAST_E = 9    
    } PDL_LED_STREAM_PULSE_STRETCH_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_PULSE_STRETCH_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_ATCONTROL_PARAMS_STC;

    typedef enum 
    {
        PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE__CLOCK_OUT_FREQUENCY_500_E = 1,
        PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_CLOCK_OUT_FREQUENCY_1000_E = 2,
        PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_CLOCK_OUT_FREQUENCY_2000_E = 3,
        PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_CLOCK_OUT_FREQUENCY_3000_E = 4,
        PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_LAST_E = 5    
    } PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_CLASS5_SELECT_TYPE_CLASS_5_SELECT_HALF_DUPLEX_E = 1,
        PDL_LED_STREAM_CLASS5_SELECT_TYPE_CLASS_5_SELECT_FIBER_LINK_UP_E = 2,
        PDL_LED_STREAM_CLASS5_SELECT_TYPE_LAST_E = 3    
    } PDL_LED_STREAM_CLASS5_SELECT_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_CLASS5_SELECT_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_CLASS13_SELECT_TYPE_CLASS_13_SELECT_LINK_DOWN_E = 1,
        PDL_LED_STREAM_CLASS13_SELECT_TYPE_CLASS_13_SELECT_COPPER_LINK_UP_E = 2,
        PDL_LED_STREAM_CLASS13_SELECT_TYPE_LAST_E = 3    
    } PDL_LED_STREAM_CLASS13_SELECT_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_CLASS13_SELECT_TYPE_E */

    typedef enum 
    {
        PDL_LED_STREAM_PORT_TYPE_PORT_TYPE_TRI_SPEED_E = 1,
        PDL_LED_STREAM_PORT_TYPE_PORT_TYPE_XG_E = 2,
        PDL_LED_STREAM_PORT_TYPE_LAST_E = 3    
    } PDL_LED_STREAM_PORT_TYPE_ENT;
    /* PDL_ENUM_ID_LED_STREAM_PORT_TYPE_E */

    typedef struct 
    {
         UINT_32 mask;
         UINT_8 mask_mask;
         UINT_32 reg;
         UINT_8 reg_mask;
         UINT_32 goodValue;
         UINT_8 goodValue_mask;
    } PDL_SIXTYCONTROL_PARAMS_STC;

    typedef enum 
    {
        PDL_CPU_TYPE_TYPE_XP_EMBEDDED_E = 1,
        PDL_CPU_TYPE_TYPE_ARMADA_MV78130_E = 2,
        PDL_CPU_TYPE_TYPE_ARMADA_MV78160_E = 3,
        PDL_CPU_TYPE_TYPE_ARMADA_MV78230_E = 4,
        PDL_CPU_TYPE_TYPE_ARMADA_MV78260_E = 5,
        PDL_CPU_TYPE_TYPE_ARMADA_MV78460_E = 6,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6810_E = 7,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6811_E = 8,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6820_E = 9,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6821_E = 10,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6W21_E = 11,
        PDL_CPU_TYPE_TYPE_ARMADA_88F6828_E = 12,
        PDL_CPU_TYPE_TYPE_ARMV8_A55_E = 13,
        PDL_CPU_TYPE_TYPE_LAST_E = 14    
    } PDL_CPU_TYPE_TYPE_ENT;
    /* PDL_ENUM_ID_CPU_TYPE_TYPE_E */

    typedef struct 
    {
         BOOLEAN isAtInfoUsed;
         UINT_8 isAtInfoUsed_mask;
         PDL_ATINFO_PARAMS_STC atinfo;
         UINT_8 atinfo_mask;
         BOOLEAN isIsrRegValueUsed;
         UINT_8 isIsrRegValueUsed_mask;
         BOOLEAN isAfControlUsed;
         UINT_8 isAfControlUsed_mask;
         BOOLEAN isAtControlUsed;
         UINT_8 isAtControlUsed_mask;
         BOOLEAN isClearControlUsed;
         UINT_8 isClearControlUsed_mask;
         BOOLEAN isStatusRegInfoUsed;
         UINT_8 isStatusRegInfoUsed_mask;
         BOOLEAN isAfInfoUsed;
         UINT_8 isAfInfoUsed_mask;
         PDL_ISRREGVALUE_PARAMS_STC isrregvalue;
         UINT_8 isrregvalue_mask;
         PDL_SIXTYWINFO_PARAMS_STC sixtywinfo;
         UINT_8 sixtywinfo_mask;
         PDL_STATUSREGINFO_PARAMS_STC statusreginfo;
         UINT_8 statusreginfo_mask;
         PDL_ATCONTROL_PARAMS_STC atcontrol;
         UINT_8 atcontrol_mask;
         PDL_AFCONTROL_PARAMS_STC afcontrol;
         UINT_8 afcontrol_mask;
         PDL_SIXTYCONTROL_PARAMS_STC sixtycontrol;
         UINT_8 sixtycontrol_mask;
         BOOLEAN isSixyControlUsed;
         UINT_8 isSixyControlUsed_mask;
         BOOLEAN isSixtywInfoUsed;
         UINT_8 isSixtywInfoUsed_mask;
         PDL_CLEARCONTROL_PARAMS_STC clearcontrol;
         UINT_8 clearcontrol_mask;
         PDL_AFINFO_PARAMS_STC afinfo;
         UINT_8 afinfo_mask;
    } PDL_PD_GROUP_PARAMS_STC;

    #define PDL_CPU_PORT_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_CPU_PORT_NUMBER_TYPE_MAX_VALUE_CNS 2
    typedef UINT_8 PDL_CPU_PORT_NUMBER_TYPE_TYP;

    #define PDL_CPU_SDMA_PORT_INDEX_TYPE_MIN_VALUE_CNS 1
    #define PDL_CPU_SDMA_PORT_INDEX_TYPE_MAX_VALUE_CNS 8
    typedef UINT_8 PDL_CPU_SDMA_PORT_INDEX_TYPE_TYP;

    typedef struct 
    {
         PDL_CPU_TYPE_TYPE_ENT cpuType;
         UINT_8 cpuType_mask;
    } PDL_CPU_INFO_PARAMS_STC;

    #define PDL_CPU_LOGICAL_PORT_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define PDL_CPU_LOGICAL_PORT_NUMBER_TYPE_MAX_VALUE_CNS 287
    typedef UINT_32 PDL_CPU_LOGICAL_PORT_NUMBER_TYPE_TYP;

    typedef struct 
    {
         PDL_CPU_TYPE_TYPE_ENT cpuType;
         UINT_8 cpuType_mask;
         UINT_8 numOfUsbDevices;
         UINT_8 numOfUsbDevices_mask;
    } PDL_CPU_CURRENT_INFO_PARAMS_STC;

    typedef enum 
    {
        PDL_BUTTON_TYPE_RESET_E = 1,
        PDL_BUTTON_TYPE_LED_E = 2,
        PDL_BUTTON_TYPE_LAST_E = 3    
    } PDL_BUTTON_TYPE_ENT;
    /* PDL_ENUM_ID_BUTTON_TYPE_E */

    typedef enum 
    {
        PDL_SENSOR_HW_TYPE_BOARD_E = 1,
        PDL_SENSOR_HW_TYPE_CPU_E = 2,
        PDL_SENSOR_HW_TYPE_PHY_E = 3,
        PDL_SENSOR_HW_TYPE_MAC_E = 4,
        PDL_SENSOR_HW_TYPE_POE_E = 5,
        PDL_SENSOR_HW_TYPE_FAN_E = 6,
        PDL_SENSOR_HW_TYPE_LAST_E = 7    
    } PDL_SENSOR_HW_TYPE_ENT;
    /* PDL_ENUM_ID_SENSOR_HW_TYPE_E */

    typedef struct 
    {
         PDL_CPU_INFO_PARAMS_STC cpuInfo;
         UINT_8 cpuInfo_mask;
    } PDL_CPU_INFO_TYPE_LEGACY_PARAMS_STC;

    typedef enum 
    {
        PDL_SENSOR_FAN_NCT7802_TYPE_LTD_E = 1,
        PDL_SENSOR_FAN_NCT7802_TYPE_RTD1_E = 2,
        PDL_SENSOR_FAN_NCT7802_TYPE_RTD2_E = 3,
        PDL_SENSOR_FAN_NCT7802_TYPE_RTD3_E = 4,
        PDL_SENSOR_FAN_NCT7802_TYPE_LAST_E = 5    
    } PDL_SENSOR_FAN_NCT7802_TYPE_ENT;
    /* PDL_ENUM_ID_SENSOR_FAN_NCT7802_TYPE_E */

    typedef enum 
    {
        PDL_SENSOR_FAN_ADT7476_TYPE_MQ1_E = 1,
        PDL_SENSOR_FAN_ADT7476_TYPE_MU1_E = 2,
        PDL_SENSOR_FAN_ADT7476_TYPE_MQ2_E = 3,
        PDL_SENSOR_FAN_ADT7476_TYPE_LAST_E = 4    
    } PDL_SENSOR_FAN_ADT7476_TYPE_ENT;
    /* PDL_ENUM_ID_SENSOR_FAN_ADT7476_TYPE_E */

    typedef enum 
    {
        PDL_THRESHOLD_TYPE_NORMAL_E = 1,
        PDL_THRESHOLD_TYPE_WARNING_E = 2,
        PDL_THRESHOLD_TYPE_CRITICAL_E = 3,
        PDL_THRESHOLD_TYPE_LAST_E = 4    
    } PDL_THRESHOLD_TYPE_ENT;
    /* PDL_ENUM_ID_THRESHOLD_TYPE_E */

    typedef struct 
    {
         PDL_CPU_CURRENT_INFO_PARAMS_STC cpuCurrentInfo;
         UINT_8 cpuCurrentInfo_mask;
    } PDL_CPU_INFO_TYPE_CURRENT_PARAMS_STC;

    typedef UINT_8 PDL_EXTERNAL_DRIVER_ID_TYPE_TYP;

    typedef enum 
    {
        PDL_NEGOTIATION_MODE_TYPE_NONE_E = 1,
        PDL_NEGOTIATION_MODE_TYPE_OUT_BAND_E = 2,
        PDL_NEGOTIATION_MODE_TYPE_IN_BAND_E = 3,
        PDL_NEGOTIATION_MODE_TYPE_LAST_E = 4    
    } PDL_NEGOTIATION_MODE_TYPE_ENT;
    /* PDL_ENUM_ID_NEGOTIATION_MODE_TYPE_E */

    typedef struct 
    {
         UINT_8 muxGpioWritePinNumber;
         UINT_8 muxGpioWritePinNumber_mask;
         UINT_8 muxGpioPinValue;
         UINT_8 muxGpioPinValue_mask;
         UINT_8 muxGpioDeviceNumber;
         UINT_8 muxGpioDeviceNumber_mask;
    } PDL_GROUPING_GPIO_MUX_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GPIO_READ_ADDRESS_PARAMS_STC gpioReadAddress;
         UINT_8 gpioReadAddress_mask;
    } PDL_GROUPING_GPIO_READ_ADDRESS_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_GPIO_MUX_GROUP_TYPE_PARAMS_STC gpioMuxGroupType;
         UINT_8 gpioMuxGroupType_mask;
    } PDL_MUX_GPIO_INFO_PARAMS_STC;

    typedef struct 
    {
         PDL_GPIO_WRITE_ADDRESS_PARAMS_STC gpioWriteAddress;
         UINT_8 gpioWriteAddress_mask;
    } PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         char functionCallParameterName[128];
    } PDL_PACKED_STRUCT_END PDL_FUNCTION_CALL_PARAMETERS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FUNCTION_CALL_PARAMETERS_LIST_KEYS_STC list_keys;
         UINT_32 functionCallParameterValue;
         UINT_8 functionCallParameterValue_mask;
         char * functionCallParameterComment;
         UINT_8 functionCallParameterComment_mask;
    } PDL_FUNCTION_CALL_PARAMETERS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         UINT_8 ledPinValue;
         UINT_8 ledPinValue_mask;
         PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC gpioWriteAddressGroupType;
         UINT_8 gpioWriteAddressGroupType_mask;
    } PDL_LED_GPIO_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN isMuxI2CDisableUsed;
         UINT_8 isMuxI2CDisableUsed_mask;
         PDL_I2C_ACCESS_TYPE_ENT muxI2CAccess;
         UINT_8 muxI2CAccess_mask;
         PDL_I2C_OFFSET_TYPE_TYP muxI2COffset;
         UINT_8 muxI2COffset_mask;
         PDL_I2C_VALUE_TYPE_TYP muxI2CDisableValue;
         UINT_8 muxI2CDisableValue_mask;
         PDL_I2C_BUS_ID_TYPE_TYP muxI2CBusId;
         UINT_8 muxI2CBusId_mask;
         PDL_I2C_ADDRESS_TYPE_TYP muxI2CAddress;
         UINT_8 muxI2CAddress_mask;
         PDL_I2C_MASK_TYPE_TYP muxI2CMask;
         UINT_8 muxI2CMask_mask;
         PDL_I2C_TRANSACTION_TYPE_ENT muxI2CTransactionType;
         UINT_8 muxI2CTransactionType_mask;
         PDL_I2C_VALUE_TYPE_TYP muxI2CValue;
         UINT_8 muxI2CValue_mask;
    } PDL_GROUPING_I2C_MUX_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP functionCallNumber;
    } PDL_PACKED_STRUCT_END PDL_FUNCTION_CALL_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FUNCTION_CALL_LIST_KEYS_STC list_keys;
         /* db of type PDL_FUNCTION_CALL_PARAMETERS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP functionCallParametersList_PTR;
         UINT_8 functionCallParametersList_mask;
         PDL_LIST_COMMENT_TYPE_TYP functionCallComment;
         UINT_8 functionCallComment_mask;
         char * functionCallName;
         UINT_8 functionCallName_mask;
    } PDL_FUNCTION_CALL_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_I2C_MUX_GROUP_TYPE_PARAMS_STC i2CMuxGroupType;
         UINT_8 i2CMuxGroupType_mask;
    } PDL_MUX_I2C_INFO_PARAMS_STC;

    typedef struct 
    {
         PDL_MUX_I2C_INFO_PARAMS_STC muxI2CInfo;
         UINT_8 muxI2CInfo_mask;
    } PDL_MUX_I2C_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
    } PDL_PACKED_STRUCT_END PDL_LED_PP_GPIO_COLOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_PP_GPIO_COLOR_LIST_KEYS_STC list_keys;
         PDL_LED_GPIO_PARAMS_STC ledGpio;
         UINT_8 ledGpio_mask;
    } PDL_LED_PP_GPIO_COLOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_MUX_GPIO_INFO_PARAMS_STC muxGpioInfo;
         UINT_8 muxGpioInfo_mask;
    } PDL_MUX_GPIO_PARAMS_STC;

    typedef enum 
    {
        PDL_MUX_TYPE_NONE_E,
        PDL_MUX_TYPE_MUX_I2C_E,
        PDL_MUX_TYPE_MUX_GPIO_E
    }PDL_MUX_TYPE_ENT;
    typedef union 
    {
         PDL_MUX_I2C_PARAMS_STC muxI2C;
         PDL_MUX_GPIO_PARAMS_STC muxGpio;
    } PDL_MUX_TYPE_PARAMS_UNT;
    typedef struct 
    {
        PDL_MUX_TYPE_ENT type;
        PDL_MUX_TYPE_PARAMS_UNT data;
    } PDL_MUX_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 muxNumber;
    } PDL_PACKED_STRUCT_END PDL_MUX_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_MUX_LIST_KEYS_STC list_keys;
         PDL_MUX_TYPE_PARAMS_STC muxType;
         UINT_8 muxType_mask;
    } PDL_MUX_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_MUX_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP muxList_PTR;
         UINT_8 muxList_mask;
    } PDL_GROUPING_MUX_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_BUS_ID_TYPE_TYP i2CBusId;
         UINT_8 i2CBusId_mask;
         PDL_I2C_ADDRESS_TYPE_TYP i2CAddress;
         UINT_8 i2CAddress_mask;
         PDL_I2C_ACCESS_TYPE_ENT i2CAccess;
         UINT_8 i2CAccess_mask;
         PDL_I2C_OFFSET_TYPE_TYP i2COffset;
         UINT_8 i2COffset_mask;
         PDL_GROUPING_MUX_GROUP_TYPE_PARAMS_STC muxGroupType;
         UINT_8 muxGroupType_mask;
         PDL_I2C_TRANSACTION_TYPE_ENT i2CTransactionType;
         UINT_8 i2CTransactionType_mask;
         PDL_I2C_MASK_TYPE_TYP i2CMask;
         UINT_8 i2CMask_mask;
         BOOLEAN isI2CChannelUsed;
         UINT_8 isI2CChannelUsed_mask;
         UINT_32 i2CBaseAddress;
         UINT_8 i2CBaseAddress_mask;
         UINT_8 i2CChannelId;
         UINT_8 i2CChannelId_mask;
    } PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressFieldsGroupType;
         UINT_8 i2CReadWriteAddressFieldsGroupType_mask;
    } PDL_I2C_READ_WRITE_ADDRESS_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_READ_WRITE_ADDRESS_PARAMS_STC i2CReadWriteAddress;
         UINT_8 i2CReadWriteAddress_mask;
    } PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
         PDL_I2C_VALUE_TYPE_TYP i2CReadValue;
         UINT_8 i2CReadValue_mask;
    } PDL_READ_I2C_INTERFACE_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_VALUE_TYPE_TYP stateokvalue;
         UINT_8 stateokvalue_mask;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
    } PDL_STATEINFOCONTROL_PARAMS_STC;

    typedef struct 
    {
         UINT_32 ppRegAddress;
         UINT_8 ppRegAddress_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP ppRegDevice;
         UINT_8 ppRegDevice_mask;
         UINT_32 ppRegMask;
         UINT_8 ppRegMask_mask;
    } PDL_GROUPING_PP_REG_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_VALUE_TYPE_TYP i2CWriteValue;
         UINT_8 i2CWriteValue_mask;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
    } PDL_WRITE_I2C_INTERFACE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_GPIO_READ_ADDRESS_GROUP_TYPE_PARAMS_STC gpioReadAddressGroupType;
         UINT_8 gpioReadAddressGroupType_mask;
         PDL_MPP_PIN_VALUE_TYPE_TYP gpioReadValue;
         UINT_8 gpioReadValue_mask;
    } PDL_READ_GPIO_INTERFACE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC gpioWriteAddressGroupType;
         UINT_8 gpioWriteAddressGroupType_mask;
         PDL_MPP_PIN_VALUE_TYPE_TYP gpioWriteValue;
         UINT_8 gpioWriteValue_mask;
    } PDL_WRITE_GPIO_INTERFACE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_PP_REG_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC ppRegReadWriteAddressFieldsGroupType;
         UINT_8 ppRegReadWriteAddressFieldsGroupType_mask;
    } PDL_PP_REG_READ_WRITE_ADDRESS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
    } PDL_TEMPERATURE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP index;
    } PDL_PACKED_STRUCT_END PDL_LEDSTART_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTART_LIST_KEYS_STC list_keys;
         UINT_8 ledstart;
         UINT_8 ledstart_mask;
    } PDL_LEDSTART_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_PP_REG_READ_WRITE_ADDRESS_PARAMS_STC ppRegReadWriteAddress;
         UINT_8 ppRegReadWriteAddress_mask;
    } PDL_GROUPING_PP_REG_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_STATEINFOCONTROL_PARAMS_STC stateinfocontrol;
         UINT_8 stateinfocontrol_mask;
         BOOLEAN isStateinfocontrolSupported;
         UINT_8 isStateinfocontrolSupported_mask;
         PDL_TEMPERATURE_PARAMS_STC temperature;
         UINT_8 temperature_mask;
    } PDL_BOARD_SENSOR_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP index;
    } PDL_PACKED_STRUCT_END PDL_LEDEND_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDEND_LIST_KEYS_STC list_keys;
         UINT_8 ledend;
         UINT_8 ledend_mask;
    } PDL_LEDEND_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
        /* key index 1 */         PDL_LED_STATE_TYPE_ENT ledState;
    } PDL_PACKED_STRUCT_END PDL_LED_EXTDRV_I2C_COLOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_EXTDRV_I2C_COLOR_LIST_KEYS_STC list_keys;
         PDL_LED_I2C_PARAMS_STC ledI2C;
         UINT_8 ledI2C_mask;
    } PDL_LED_EXTDRV_I2C_COLOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP index;
    } PDL_PACKED_STRUCT_END PDL_LEDCHAINBYPASS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDCHAINBYPASS_LIST_KEYS_STC list_keys;
         BOOLEAN ledchainbypass;
         UINT_8 ledchainbypass_mask;
    } PDL_LEDCHAINBYPASS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 ppRegWriteIndex;
    } PDL_PACKED_STRUCT_END PDL_PP_REG_WRITE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PP_REG_WRITE_LIST_KEYS_STC list_keys;
         UINT_32 ppRegWriteValue;
         UINT_8 ppRegWriteValue_mask;
         PDL_GROUPING_PP_REG_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC ppRegReadWriteAddressGroupType;
         UINT_8 ppRegReadWriteAddressGroupType_mask;
    } PDL_PP_REG_WRITE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_PP_REG_WRITE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ppRegWriteList_PTR;
         UINT_8 ppRegWriteList_mask;
    } PDL_GROUPING_PP_REG_WRITE_LIST_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
    } PDL_PACKED_STRUCT_END PDL_LED_EXTDRV_GPIO_COLOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_EXTDRV_GPIO_COLOR_LIST_KEYS_STC list_keys;
         UINT_8 ledPinValue;
         UINT_8 ledPinValue_mask;
         PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC gpioWriteAddressGroupType;
         UINT_8 gpioWriteAddressGroupType_mask;
    } PDL_LED_EXTDRV_GPIO_COLOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_PHY_MASK_VALUE_PARAMS_STC phyMaskValue;
         UINT_8 phyMaskValue_mask;
    } PDL_GROUPING_PHY_MASK_VALUE_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
        /* key index 1 */         PDL_LED_STATE_TYPE_ENT ledState;
    } PDL_PACKED_STRUCT_END PDL_LED_PP_REGISTER_COLOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_PP_REGISTER_COLOR_LIST_KEYS_STC list_keys;
         PDL_GROUPING_PP_REG_WRITE_LIST_GROUP_TYPE_PARAMS_STC ppRegWriteListGroupType;
         UINT_8 ppRegWriteListGroupType_mask;
    } PDL_LED_PP_REGISTER_COLOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_PHY_READ_WRITE_ADDRESS_PARAMS_STC phyReadWriteAddress;
         UINT_8 phyReadWriteAddress_mask;
    } PDL_GROUPING_PHY_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP ledPhyValueNumber;
    } PDL_PACKED_STRUCT_END PDL_LED_PHY_VALUE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_PHY_VALUE_LIST_KEYS_STC list_keys;
         PDL_LIST_COMMENT_TYPE_TYP ledPhyValueComment;
         UINT_8 ledPhyValueComment_mask;
         PDL_GROUPING_PHY_MASK_VALUE_GROUP_TYPE_PARAMS_STC phyMaskValueGroupType;
         UINT_8 phyMaskValueGroupType_mask;
    } PDL_LED_PHY_VALUE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP phyInitIndex;
    } PDL_PACKED_STRUCT_END PDL_PHY_INIT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PHY_INIT_LIST_KEYS_STC list_keys;
         PDL_GROUPING_PHY_MASK_VALUE_GROUP_TYPE_PARAMS_STC phyMaskValueGroupType;
         UINT_8 phyMaskValueGroupType_mask;
    } PDL_PHY_INIT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_PP_REG_WRITE_LIST_GROUP_TYPE_PARAMS_STC ppRegWriteListGroupType;
         UINT_8 ppRegWriteListGroupType_mask;
    } PDL_LED_PP_REGISTER_INIT_VALUE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP ledPhyInitNumber;
    } PDL_PACKED_STRUCT_END PDL_LED_PHY_INIT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_PHY_INIT_LIST_KEYS_STC list_keys;
         PDL_GROUPING_PHY_MASK_VALUE_GROUP_TYPE_PARAMS_STC phyMaskValueGroupType;
         UINT_8 phyMaskValueGroupType_mask;
         PDL_LIST_COMMENT_TYPE_TYP ledPhyInitComment;
         UINT_8 ledPhyInitComment_mask;
    } PDL_LED_PHY_INIT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_SPEED_TYPE_ENT speed;
    } PDL_PACKED_STRUCT_END PDL_L1_INTERFACE_MODE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_L1_INTERFACE_MODE_LIST_KEYS_STC list_keys;
         /* db of type PDL_PHY_INIT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP phyInitList_PTR;
         UINT_8 phyInitList_mask;
         PDL_L1_INTERFACE_MODE_TYPE_ENT mode;
         UINT_8 mode_mask;
    } PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_PP_REG_WRITE_LIST_GROUP_TYPE_PARAMS_STC ppRegWriteListGroupType;
         UINT_8 ppRegWriteListGroupType_mask;
    } PDL_LED_PP_REGISTER_OFF_VALUE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 readGpioIndex;
    } PDL_PACKED_STRUCT_END PDL_POWER_CONNECTION_READ_ONLY_GPIO_GROUP_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_POWER_CONNECTION_READ_ONLY_GPIO_GROUP_LIST_KEYS_STC list_keys;
         PDL_GROUPING_GPIO_READ_ADDRESS_GROUP_TYPE_PARAMS_STC gpioReadAddressGroupType;
         UINT_8 gpioReadAddressGroupType_mask;
         PDL_MPP_PIN_VALUE_TYPE_TYP activeValue;
         UINT_8 activeValue_mask;
    } PDL_POWER_CONNECTION_READ_ONLY_GPIO_GROUP_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_LED_PP_REGISTER_COLOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledPpRegisterColorList_PTR;
         UINT_8 ledPpRegisterColorList_mask;
         PDL_LED_PP_REGISTER_INIT_VALUE_PARAMS_STC ledPpRegisterInitValue;
         UINT_8 ledPpRegisterInitValue_mask;
         PDL_LED_PP_REGISTER_OFF_VALUE_PARAMS_STC ledPpRegisterOffValue;
         UINT_8 ledPpRegisterOffValue_mask;
    } PDL_LED_PP_REGISTER_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP l1InterfaceModeList_PTR;
         UINT_8 l1InterfaceModeList_mask;
    } PDL_GROUPING_PORT_L1_INTERFACE_MODES_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_I2C_VALUE_TYPE_TYP activeValue;
         UINT_8 activeValue_mask;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
    } PDL_POWER_CONNECTION_READ_ONLY_I2C_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LANE_NUMBER_TYPE_TYP laneNumber;
    } PDL_PACKED_STRUCT_END PDL_SERDES_LANE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_SERDES_LANE_LIST_KEYS_STC list_keys;
         PDL_PORT_LANE_NUMBER_TYPE_TYP portLaneNumber;
         UINT_8 portLaneNumber_mask;
    } PDL_SERDES_LANE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         BOOLEAN ledI2CAlternateExist;
         UINT_8 ledI2CAlternateExist_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2COffValue;
         UINT_8 ledI2COffValue_mask;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2CInitValue;
         UINT_8 ledI2CInitValue_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2CAlternateValue;
         UINT_8 ledI2CAlternateValue_mask;
    } PDL_LED_EXTDRV_I2C_PARAMS_STC;

    typedef struct 
    {
         PDL_READ_GPIO_INTERFACE_PARAMS_STC readGpioInterface;
         UINT_8 readGpioInterface_mask;
         PDL_INTERFACE_TYPE_ENT readInterfaceSelect;
         UINT_8 readInterfaceSelect_mask;
         PDL_READ_I2C_INTERFACE_PARAMS_STC readI2CInterface;
         UINT_8 readI2CInterface_mask;
    } PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LANE_NUMBER_TYPE_TYP laneNumber;
    } PDL_PACKED_STRUCT_END PDL_PORT_SERDES_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PORT_SERDES_LIST_KEYS_STC list_keys;
         PDL_PORT_LANE_NUMBER_TYPE_TYP portLaneNumber;
         UINT_8 portLaneNumber_mask;
    } PDL_PORT_SERDES_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_SERDES_LANE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP serdesLaneList_PTR;
         UINT_8 serdesLaneList_mask;
    } PDL_SERDES_CURRENT_PARAMS_STC;

    typedef struct 
    {
         PDL_WRITE_I2C_INTERFACE_PARAMS_STC writeI2CInterface;
         UINT_8 writeI2CInterface_mask;
         PDL_WRITE_GPIO_INTERFACE_PARAMS_STC writeGpioInterface;
         UINT_8 writeGpioInterface_mask;
         PDL_INTERFACE_TYPE_ENT writeInterfaceSelect;
         UINT_8 writeInterfaceSelect_mask;
    } PDL_GROUPING_WRITE_INTERFACE_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC readInterfaceGroupType;
         UINT_8 readInterfaceGroupType_mask;
    } PDL_GBICCONNECTEDINFO_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_PORT_SERDES_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP portSerdesList_PTR;
         UINT_8 portSerdesList_mask;
    } PDL_SERDES_GROUP_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC readInterfaceGroupType;
         UINT_8 readInterfaceGroupType_mask;
    } PDL_PUSH_VALUE_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN serdesExist;
         UINT_8 serdesExist_mask;
         PDL_SERDES_GROUP_PARAMS_STC serdesGroup;
         UINT_8 serdesGroup_mask;
    } PDL_SERDES_LEGACY_V1_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_FUNCTION_CALL_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP functionCallList_PTR;
         UINT_8 functionCallList_mask;
    } PDL_GROUPING_FUNCTION_CALL_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC readInterfaceGroupType;
         UINT_8 readInterfaceGroupType_mask;
    } PDL_LASERISLOSSINFO_PARAMS_STC;

    typedef enum 
    {
        PDL_SERDES_LANE_INFORMATION_NONE_E,
        PDL_SERDES_LANE_INFORMATION_SERDES_CURRENT_E,
        PDL_SERDES_LANE_INFORMATION_SERDES_LEGACY_V1_E
    }PDL_SERDES_LANE_INFORMATION_ENT;
    typedef union 
    {
         PDL_SERDES_CURRENT_PARAMS_STC serdesCurrent;
         PDL_SERDES_LEGACY_V1_PARAMS_STC serdesLegacyV1;
    } PDL_SERDES_LANE_INFORMATION_PARAMS_UNT;
    typedef struct 
    {
        PDL_SERDES_LANE_INFORMATION_ENT type;
        PDL_SERDES_LANE_INFORMATION_PARAMS_UNT data;
    } PDL_SERDES_LANE_INFORMATION_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN blinkenable;
         UINT_8 blinkenable_mask;
         BOOLEAN pulsestretchenable;
         UINT_8 pulsestretchenable_mask;
         PDL_LED_STREAM_BLINK_SELECT_TYPE_ENT blinkselect;
         UINT_8 blinkselect_mask;
         UINT_32 classnum;
         UINT_8 classnum_mask;
         BOOLEAN disableonlinkdown;
         UINT_8 disableonlinkdown_mask;
         PDL_LED_STREAM_PORT_TYPE_ENT porttype;
         UINT_8 porttype_mask;
         BOOLEAN invertenable;
         UINT_8 invertenable_mask;
         BOOLEAN forceenable;
         UINT_8 forceenable_mask;
    } PDL_GROUPING_LEDSTREAM_MANIPULATION_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP phyInitIndex;
    } PDL_PACKED_STRUCT_END PDL_PHY_POST_INIT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PHY_POST_INIT_LIST_KEYS_STC list_keys;
         PDL_GROUPING_PHY_MASK_VALUE_GROUP_TYPE_PARAMS_STC phyMaskValueGroupType;
         UINT_8 phyMaskValueGroupType_mask;
    } PDL_PHY_POST_INIT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_FUNCTION_CALL_GROUP_TYPE_PARAMS_STC functionCallGroupType;
         UINT_8 functionCallGroupType_mask;
    } PDL_LED_PP_OFF_VALUE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_WRITE_INTERFACE_GROUP_TYPE_PARAMS_STC writeInterfaceGroupType;
         UINT_8 writeInterfaceGroupType_mask;
    } PDL_TXISENABLEDCONTROL_PARAMS_STC;

    typedef struct 
    {
         UINT_32 forcedata;
         UINT_8 forcedata_mask;
         PDL_GROUPING_LEDSTREAM_MANIPULATION_GROUP_TYPE_PARAMS_STC ledstreamManipulationGroupType;
         UINT_8 ledstreamManipulationGroupType_mask;
    } PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_WRITE_INTERFACE_GROUP_TYPE_PARAMS_STC writeInterfaceGroupType;
         UINT_8 writeInterfaceGroupType_mask;
    } PDL_TXISDISABLEDCONTROL_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_FUNCTION_CALL_GROUP_TYPE_PARAMS_STC functionCallGroupType;
         UINT_8 functionCallGroupType_mask;
    } PDL_LED_PP_INIT_VALUE_PARAMS_STC;

    typedef struct 
    {
         UINT_32 forcedata;
         UINT_8 forcedata_mask;
         PDL_GROUPING_LEDSTREAM_MANIPULATION_GROUP_TYPE_PARAMS_STC ledstreamManipulationGroupType;
         UINT_8 ledstreamManipulationGroupType_mask;
    } PDL_GROUPING_LEDSTREAM_PORT_MANIPULATION_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
    } PDL_MEMORYCONTROL_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP manipulationIndex;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_KEYS_STC list_keys;
         PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_GROUP_TYPE_PARAMS_STC ledstreamInterfaceManipulationGroupType;
         UINT_8 ledstreamInterfaceManipulationGroupType_mask;
    } PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_LED_PHY_VALUE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledPhyValueList_PTR;
         UINT_8 ledPhyValueList_mask;
    } PDL_GROUPING_LED_PHY_VALUE_LIST_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP manipulationIndex;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAM_PORT_MANIPULATION_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAM_PORT_MANIPULATION_LIST_KEYS_STC list_keys;
         PDL_GROUPING_LEDSTREAM_PORT_MANIPULATION_GROUP_TYPE_PARAMS_STC ledstreamPortManipulationGroupType;
         UINT_8 ledstreamPortManipulationGroupType_mask;
    } PDL_LEDSTREAM_PORT_MANIPULATION_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LED_PHY_VALUE_LIST_GROUP_TYPE_PARAMS_STC ledPhyValueListGroupType;
         UINT_8 ledPhyValueListGroupType_mask;
    } PDL_LED_PP_COLOR_PHY_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PHY_VALUE_LIST_GROUP_TYPE_PARAMS_STC ledPhyValueListGroupType;
         UINT_8 ledPhyValueListGroupType_mask;
    } PDL_LED_PHY_OFF_VALUE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_MPP_PIN_NUMBER_TYPE_TYP pinNumber;
    } PDL_PACKED_STRUCT_END PDL_PIN_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PIN_LIST_KEYS_STC list_keys;
         PDL_MPP_PIN_MODE_TYPE_ENT pinMode;
         UINT_8 pinMode_mask;
         BOOLEAN pinPolaritySwap;
         UINT_8 pinPolaritySwap_mask;
         PDL_MPP_PIN_INIT_VALUE_TYPE_ENT pinInitialValue;
         UINT_8 pinInitialValue_mask;
         char * pinDescription;
         UINT_8 pinDescription_mask;
    } PDL_PIN_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_FUNCTION_CALL_GROUP_TYPE_PARAMS_STC functionCallGroupType;
         UINT_8 functionCallGroupType_mask;
    } PDL_LED_PP_COLOR_PP_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_MPP_DEVICE_NUMBER_TYPE_TYP mppDeviceNumber;
    } PDL_PACKED_STRUCT_END PDL_MPP_DEVICE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_MPP_DEVICE_LIST_KEYS_STC list_keys;
         UINT_32 ppGpioInitBaseAddress;
         UINT_8 ppGpioInitBaseAddress_mask;
         UINT_32 cpuGpioRegistersRegionBaseAddress;
         UINT_8 cpuGpioRegistersRegionBaseAddress_mask;
         UINT_32 ppMppInitBaseAddress;
         UINT_8 ppMppInitBaseAddress_mask;
         UINT_32 cpuMppRegistersRegionBaseAddress;
         UINT_8 cpuMppRegistersRegionBaseAddress_mask;
         /* db of type PDL_PIN_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP pinList_PTR;
         UINT_8 pinList_mask;
    } PDL_MPP_DEVICE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_RX_PARAM_TYPE_ENT rxParam;
    } PDL_PACKED_STRUCT_END PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_KEYS_STC list_keys;
         UINT_32 rxParamValue;
         UINT_8 rxParamValue_mask;
    } PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
        /* key index 1 */         PDL_LED_STATE_TYPE_ENT ledState;
    } PDL_PACKED_STRUCT_END PDL_LED_PP_COLOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_PP_COLOR_LIST_KEYS_STC list_keys;
         PDL_LED_PP_COLOR_PHY_PARAMS_STC ledPpColorPhy;
         UINT_8 ledPpColorPhy_mask;
         PDL_LED_PP_COLOR_PP_PARAMS_STC ledPpColorPp;
         UINT_8 ledPpColorPp_mask;
         PDL_LED_PP_COLOR_I2C_PARAMS_STC ledPpColorI2C;
         UINT_8 ledPpColorI2C_mask;
    } PDL_LED_PP_COLOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP laneAttributesInfoRxFineTuneList_PTR;
         UINT_8 laneAttributesInfoRxFineTuneList_mask;
    } PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN ledI2CAlternateExist;
         UINT_8 ledI2CAlternateExist_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2COffValue;
         UINT_8 ledI2COffValue_mask;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2CInitValue;
         UINT_8 ledI2CInitValue_mask;
         PDL_I2C_VALUE_TYPE_TYP ledI2CAlternateValue;
         UINT_8 ledI2CAlternateValue_mask;
    } PDL_LED_PP_I2C_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 2 */         PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT connectorTechnology;
        /* key index 0 */         PDL_L1_INTERFACE_MODE_TYPE_ENT interfaceMode;
        /* key index 1 */         PDL_CONNECTOR_TYPE_TYPE_ENT connectorType;
    } PDL_PACKED_STRUCT_END PDL_LANE_ATTRIBUTES_INFO_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LANE_ATTRIBUTES_INFO_LIST_KEYS_STC list_keys;
         BOOLEAN isTxFineTuneSupported;
         UINT_8 isTxFineTuneSupported_mask;
         BOOLEAN isRxFineTuneSupported;
         UINT_8 isRxFineTuneSupported_mask;
         PDL_LANE_ATTRIBUTES_INFO_TX_FINE_TUNE_PARAMS_STC laneAttributesInfoTxFineTune;
         UINT_8 laneAttributesInfoTxFineTune_mask;
         PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_PARAMS_STC laneAttributesInfoRxFineTune;
         UINT_8 laneAttributesInfoRxFineTune_mask;
    } PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_RX_PARAM_TYPE_ENT rxParam;
    } PDL_PACKED_STRUCT_END PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_KEYS_STC list_keys;
         UINT_32 rxParamValue;
         UINT_8 rxParamValue_mask;
    } PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         BOOLEAN ledPhyInitValuesExist;
         UINT_8 ledPhyInitValuesExist_mask;
         /* db of type PDL_LED_PHY_INIT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledPhyInitList_PTR;
         UINT_8 ledPhyInitList_mask;
         BOOLEAN ledPhyOffExist;
         UINT_8 ledPhyOffExist_mask;
         PDL_LED_PHY_OFF_VALUE_PARAMS_STC ledPhyOffValue;
         UINT_8 ledPhyOffValue_mask;
    } PDL_LED_PP_PHY_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP laneAttributesInfoList_PTR;
         UINT_8 laneAttributesInfoList_mask;
    } PDL_LANE_ATTRIBUTES_CURRENT_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP laneAttributeRxFineTuneList_PTR;
         UINT_8 laneAttributeRxFineTuneList_mask;
    } PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN isTxFineTuneSupported;
         UINT_8 isTxFineTuneSupported_mask;
         BOOLEAN isRxFineTuneSupported;
         UINT_8 isRxFineTuneSupported_mask;
         PDL_LANE_ATTRIBUTE_TX_FINE_TUNE_PARAMS_STC laneAttributeTxFineTune;
         UINT_8 laneAttributeTxFineTune_mask;
         PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_PARAMS_STC laneAttributeRxFineTune;
         UINT_8 laneAttributeRxFineTune_mask;
    } PDL_LANE_ATTRIBUTE_PARAMS_STC;

    typedef struct 
    {
         PDL_LED_PP_OFF_VALUE_PARAMS_STC ledPpOffValue;
         UINT_8 ledPpOffValue_mask;
         PDL_LED_PP_INIT_VALUE_PARAMS_STC ledPpInitValue;
         UINT_8 ledPpInitValue_mask;
    } PDL_LED_PP_VALUES_PARAMS_STC;

    typedef struct 
    {
         PDL_LED_PP_PHY_PARAMS_STC ledPpPhy;
         UINT_8 ledPpPhy_mask;
         PDL_LED_PP_I2C_PARAMS_STC ledPpI2C;
         UINT_8 ledPpI2C_mask;
         PDL_EXTERNAL_DRIVER_ID_TYPE_TYP externalDriverId;
         UINT_8 externalDriverId_mask;
         /* db of type PDL_LED_PP_GPIO_COLOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledPpGpioColorList_PTR;
         UINT_8 ledPpGpioColorList_mask;
         /* db of type PDL_LED_PP_COLOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledPpColorList_PTR;
         UINT_8 ledPpColorList_mask;
         PDL_LED_PP_VALUES_PARAMS_STC ledPpValues;
         UINT_8 ledPpValues_mask;
         PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_ENT ledInterfaceSelect;
         UINT_8 ledInterfaceSelect_mask;
    } PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 1 */         PDL_CONNECTOR_TYPE_TYPE_ENT connectorType;
        /* key index 2 */         PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT connectorTechnology;
        /* key index 0 */         PDL_L1_INTERFACE_MODE_TYPE_ENT interfaceMode;
    } PDL_PACKED_STRUCT_END PDL_INFO_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_INFO_LIST_KEYS_STC list_keys;
         PDL_LANE_ATTRIBUTE_PARAMS_STC laneAttribute;
         UINT_8 laneAttribute_mask;
    } PDL_INFO_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_LEFTLED_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_INFO_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP infoList_PTR;
         UINT_8 infoList_mask;
    } PDL_LANE_ATTRIBUTES_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamInterfaceManipulationList_PTR;
         UINT_8 ledstreamInterfaceManipulationList_mask;
    } PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_LIST_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_OOBLEFTLED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PHY_NUMBER_TYPE_TYP phyNumber;
    } PDL_PACKED_STRUCT_END PDL_PHY_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PHY_LIST_KEYS_STC list_keys;
         PDL_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENT downloadMethod;
         UINT_8 downloadMethod_mask;
         PDL_PHY_TYPE_TYPE_ENT phyType;
         UINT_8 phyType_mask;
    } PDL_PHY_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_LANE_ATTRIBUTES_PARAMS_STC laneAttributes;
         UINT_8 laneAttributes_mask;
         BOOLEAN laneAttributesExist;
         UINT_8 laneAttributesExist_mask;
    } PDL_LANE_ATTRIBUTES_LEGACY_V1_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_RIGHTLED_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LEDSTREAM_PORT_MANIPULATION_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamPortManipulationList_PTR;
         UINT_8 ledstreamPortManipulationList_mask;
    } PDL_GROUPING_LEDSTREAM_PORT_MANIPULATION_LIST_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_OOBRIGHTLED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_COLOR_TYPE_ENT ledColor;
        /* key index 1 */         PDL_LED_STATE_TYPE_ENT ledState;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAM_INTERFACE_MODE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAM_INTERFACE_MODE_LIST_KEYS_STC list_keys;
         PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_LIST_TYPE_PARAMS_STC ledstreamInterfaceManipulationListType;
         UINT_8 ledstreamInterfaceManipulationListType_mask;
    } PDL_LEDSTREAM_INTERFACE_MODE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_FAN_NUMBER_TYPE_TYP fanNumber;
    } PDL_PACKED_STRUCT_END PDL_FAN_CONNECTED_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FAN_CONNECTED_LIST_KEYS_STC list_keys;
         PDL_FAN_ROLE_TYPE_ENT fanRole;
         UINT_8 fanRole_mask;
         PDL_FAN_ID_TYPE_TYP fanId;
         UINT_8 fanId_mask;
    } PDL_FAN_CONNECTED_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef enum 
    {
        PDL_LANE_INFORMATION_NONE_E,
        PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E,
        PDL_LANE_INFORMATION_LANE_ATTRIBUTES_LEGACY_V1_E
    }PDL_LANE_INFORMATION_ENT;
    typedef union 
    {
         PDL_LANE_ATTRIBUTES_CURRENT_PARAMS_STC laneAttributesCurrent;
         PDL_LANE_ATTRIBUTES_LEGACY_V1_PARAMS_STC laneAttributesLegacyV1;
    } PDL_LANE_INFORMATION_PARAMS_UNT;
    typedef struct 
    {
        PDL_LANE_INFORMATION_ENT type;
        PDL_LANE_INFORMATION_PARAMS_UNT data;
    } PDL_LANE_INFORMATION_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_OOBSINGLELED_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC ledPpGroupType;
         UINT_8 ledPpGroupType_mask;
    } PDL_SINGLELED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP fanOperationNumber;
    } PDL_PACKED_STRUCT_END PDL_FAN_OPERATION_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FAN_OPERATION_LIST_KEYS_STC list_keys;
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressGroupType;
         UINT_8 i2CReadWriteAddressGroupType_mask;
         PDL_I2C_VALUE_TYPE_TYP fanOperationValue;
         UINT_8 fanOperationValue_mask;
         PDL_LIST_COMMENT_TYPE_TYP fanOperationComment;
         UINT_8 fanOperationComment_mask;
    } PDL_FAN_OPERATION_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC i2CReadWriteAddressFieldsGroupType;
         UINT_8 i2CReadWriteAddressFieldsGroupType_mask;
    } PDL_FAN_HW_I2C_READ_WRITE_ADDRESS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LANE_NUMBER_TYPE_TYP laneNumber;
    } PDL_PACKED_STRUCT_END PDL_LANE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LANE_LIST_KEYS_STC list_keys;
         PDL_LANE_ATTRIBUTES_GROUP_PARAMS_STC laneAttributesGroup;
         UINT_8 laneAttributesGroup_mask;
         PDL_LANE_INFORMATION_PARAMS_STC laneInformation;
         UINT_8 laneInformation_mask;
    } PDL_LANE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LIST_NUMBER_TYPE_TYP ledstreamPortModeIndex;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAM_PORT_MODE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAM_PORT_MODE_LIST_KEYS_STC list_keys;
         PDL_GROUPING_LEDSTREAM_PORT_MANIPULATION_LIST_TYPE_PARAMS_STC ledstreamPortManipulationListType;
         UINT_8 ledstreamPortManipulationListType_mask;
         char * ledstreamPortModeDescription;
         UINT_8 ledstreamPortModeDescription_mask;
    } PDL_LEDSTREAM_PORT_MODE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_LIST_TYPE_PARAMS_STC ledstreamInterfaceManipulationListType;
         UINT_8 ledstreamInterfaceManipulationListType_mask;
    } PDL_LEDSTREAM_INTERFACE_OFF_VALUE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_HW_FAN_OPERATION_TYPE_ENT fanOperationType;
    } PDL_PACKED_STRUCT_END PDL_FAN_ADT7476_OPERATION_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FAN_ADT7476_OPERATION_LIST_KEYS_STC list_keys;
         /* db of type PDL_FAN_OPERATION_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP fanOperationList_PTR;
         UINT_8 fanOperationList_mask;
    } PDL_FAN_ADT7476_OPERATION_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_LEDSTREAM_PORT_MODE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamPortModeList_PTR;
         UINT_8 ledstreamPortModeList_mask;
    } PDL_GROUPING_LEDSTREAM_PORT_VALUES_LIST_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PP_DEVICE_NUMBER_TYPE_TYP ppDeviceNumber;
    } PDL_PACKED_STRUCT_END PDL_PACKET_PROCESSOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PACKET_PROCESSOR_LIST_KEYS_STC list_keys;
         /* db of type PDL_LANE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP laneList_PTR;
         UINT_8 laneList_mask;
    } PDL_PACKET_PROCESSOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 sensorNumber;
    } PDL_PACKED_STRUCT_END PDL_SW_FAN_CONTROLLER_SENSOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_SW_FAN_CONTROLLER_SENSOR_LIST_KEYS_STC list_keys;
         UINT_8 thresholdTemperature;
         UINT_8 thresholdTemperature_mask;
    } PDL_SW_FAN_CONTROLLER_SENSOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_FAN_ADT7476_FAULT_DETECTION_TYPE_ENT fanAdt7476FaultDetection;
         UINT_8 fanAdt7476FaultDetection_mask;
         /* db of type PDL_FAN_ADT7476_OPERATION_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP fanAdt7476OperationList_PTR;
         UINT_8 fanAdt7476OperationList_mask;
    } PDL_FAN_ADT7476_SPECIFIC_PARAMS_STC;

    typedef struct 
    {
         PDL_LEDSTREAM_INTERFACE_OFF_VALUE_PARAMS_STC ledstreamInterfaceOffValue;
         UINT_8 ledstreamInterfaceOffValue_mask;
         /* db of type PDL_LEDSTREAM_INTERFACE_MODE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamInterfaceModeList_PTR;
         UINT_8 ledstreamInterfaceModeList_mask;
         BOOLEAN ledstreamInterfaceOffExist;
         UINT_8 ledstreamInterfaceOffExist_mask;
    } PDL_GROUPING_LEDSTREAM_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_PORT_VALUES_LIST_TYPE_PARAMS_STC ledstreamPortValuesListType;
         UINT_8 ledstreamPortValuesListType_mask;
    } PDL_LEFTLED_PORT_MODES_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 fanSpeed;
    } PDL_PACKED_STRUCT_END PDL_THRESHOLD_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_THRESHOLD_LIST_KEYS_STC list_keys;
         PDL_THRESHOLD_TYPE_ENT thresholdState;
         UINT_8 thresholdState_mask;
         /* db of type PDL_SW_FAN_CONTROLLER_SENSOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP swFanControllerSensorList_PTR;
         UINT_8 swFanControllerSensorList_mask;
    } PDL_THRESHOLD_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_FAN_CONTROLLER_NUMBER_TYPE_TYP fanControllerNumber;
    } PDL_PACKED_STRUCT_END PDL_FAN_CONTROLLER_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FAN_CONTROLLER_LIST_KEYS_STC list_keys;
         PDL_FAN_TYPE_ENT fanHw;
         UINT_8 fanHw_mask;
         /* db of type PDL_FAN_CONNECTED_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP fanConnectedList_PTR;
         UINT_8 fanConnectedList_mask;
         PDL_EXTERNAL_DRIVER_ID_TYPE_TYP externalDriverId;
         UINT_8 externalDriverId_mask;
         PDL_FAN_HW_I2C_READ_WRITE_ADDRESS_PARAMS_STC fanHwI2CReadWriteAddress;
         UINT_8 fanHwI2CReadWriteAddress_mask;
         PDL_FAN_ADT7476_SPECIFIC_PARAMS_STC fanAdt7476Specific;
         UINT_8 fanAdt7476Specific_mask;
         char * fanHwDisplayName;
         UINT_8 fanHwDisplayName_mask;
    } PDL_FAN_CONTROLLER_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_PORT_VALUES_LIST_TYPE_PARAMS_STC ledstreamPortValuesListType;
         UINT_8 ledstreamPortValuesListType_mask;
    } PDL_RIGHTLED_PORT_MODES_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LEDEND_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledendList_PTR;
         UINT_8 ledendList_mask;
         /* db of type PDL_LEDCHAINBYPASS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledchainbypassList_PTR;
         UINT_8 ledchainbypassList_mask;
         UINT_32 ledclockfrequency;
         UINT_8 ledclockfrequency_mask;
         /* db of type PDL_LEDSTART_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstartList_PTR;
         UINT_8 ledstartList_mask;
    } PDL_GROUPING_LEDSTREAM_SIP6_CONFIG_TYPE_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_THRESHOLD_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP thresholdList_PTR;
         UINT_8 thresholdList_mask;
    } PDL_SW_FAN_CONTROLLERS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_SIP6_CONFIG_TYPE_PARAMS_STC ledstreamSip6ConfigType;
         UINT_8 ledstreamSip6ConfigType_mask;
    } PDL_SIP6LEDCONFIG_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_PORT_VALUES_LIST_TYPE_PARAMS_STC ledstreamPortValuesListType;
         UINT_8 ledstreamPortValuesListType_mask;
    } PDL_SINGLELED_PORT_MODES_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LED_EXTDRV_I2C_COLOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledExtdrvI2CColorList_PTR;
         UINT_8 ledExtdrvI2CColorList_mask;
         PDL_EXTERNAL_DRIVER_ID_TYPE_TYP externalDriverId;
         UINT_8 externalDriverId_mask;
         PDL_LED_EXTDRV_I2C_PARAMS_STC ledExtdrvI2C;
         UINT_8 ledExtdrvI2C_mask;
         PDL_LED_PP_REGISTER_PARAMS_STC ledPpRegister;
         UINT_8 ledPpRegister_mask;
         /* db of type PDL_LED_EXTDRV_GPIO_COLOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledExtdrvGpioColorList_PTR;
         UINT_8 ledExtdrvGpioColorList_mask;
         PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_ENT ledInterfaceSelect;
         UINT_8 ledInterfaceSelect_mask;
    } PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_ENT blink1Dutycycle;
         UINT_8 blink1Dutycycle_mask;
         PDL_LED_STREAM_CLOCK_OUT_FREQUENCY_TYPE_ENT ledclockfrequency;
         UINT_8 ledclockfrequency_mask;
         PDL_LED_STREAM_BLINK_DUTY_CYCLE_TYPE_ENT blink0Dutycycle;
         UINT_8 blink0Dutycycle_mask;
         PDL_LED_STREAM_BLINK_DURATION_TYPE_ENT blink1Duration;
         UINT_8 blink1Duration_mask;
         PDL_LED_STREAM_BLINK_DURATION_TYPE_ENT blink0Duration;
         UINT_8 blink0Duration_mask;
         PDL_LED_STREAM_CLASS13_SELECT_TYPE_ENT class13Select;
         UINT_8 class13Select_mask;
         PDL_SIP6LEDCONFIG_PARAMS_STC sip6Ledconfig;
         UINT_8 sip6Ledconfig_mask;
         PDL_LED_STREAM_ORDER_MODE_TYPE_ENT ledorganize;
         UINT_8 ledorganize_mask;
         PDL_LED_STREAM_CLASS5_SELECT_TYPE_ENT class5Select;
         UINT_8 class5Select_mask;
         BOOLEAN disableonlinkdown;
         UINT_8 disableonlinkdown_mask;
         PDL_LED_STREAM_PULSE_STRETCH_TYPE_ENT pulsestretch;
         UINT_8 pulsestretch_mask;
         BOOLEAN clkinvert;
         UINT_8 clkinvert_mask;
         UINT_32 ledend;
         UINT_8 ledend_mask;
         UINT_32 ledstart;
         UINT_8 ledstart_mask;
         BOOLEAN invertenable;
         UINT_8 invertenable_mask;
         BOOLEAN sip6Config;
         UINT_8 sip6Config_mask;
    } PDL_LEDSTREAM_INTERFACE_CONFIGSET_PARAMS_STC;

    typedef struct 
    {
         PDL_SINGLELED_PORT_MODES_PARAMS_STC singleledPortModes;
         UINT_8 singleledPortModes_mask;
         PDL_LEFTLED_PORT_MODES_PARAMS_STC leftledPortModes;
         UINT_8 leftledPortModes_mask;
         PDL_RIGHTLED_PORT_MODES_PARAMS_STC rightledPortModes;
         UINT_8 rightledPortModes_mask;
    } PDL_LEDSTREAM_PORT_MODES_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_STACK_LED_PARAMS_STC;

    typedef struct 
    {
         UINT_32 blinkCounterOffDuration;
         UINT_8 blinkCounterOffDuration_mask;
         UINT_32 blinkCounterOnRegisterOffset;
         UINT_8 blinkCounterOnRegisterOffset_mask;
         UINT_32 blinkCounterOffRegisterOffset;
         UINT_8 blinkCounterOffRegisterOffset_mask;
         UINT_32 blinkCounterOnDuration;
         UINT_8 blinkCounterOnDuration_mask;
    } PDL_GROUPING_LED_GPIO_BLINK_COUNTER_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_LIST_TYPE_PARAMS_STC ledstreamInterfaceManipulationListType;
         UINT_8 ledstreamInterfaceManipulationListType_mask;
    } PDL_LEDSTREAM_INTERFACE_INIT_VALUE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_GPIO_BLINK_COUNTER_GROUP_TYPE_PARAMS_STC ledGpioBlinkCounterGroupType;
         UINT_8 ledGpioBlinkCounterGroupType_mask;
    } PDL_LOWRATEBLINKCOUNTERS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_STACK_LED_ID_TYPE_TYP stackLedId;
    } PDL_PACKED_STRUCT_END PDL_STACKLED_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_STACKLED_LIST_KEYS_STC list_keys;
         PDL_STACK_LED_PARAMS_STC stackLed;
         UINT_8 stackLed_mask;
    } PDL_STACKLED_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LEDSTREAM_GROUP_TYPE_PARAMS_STC ledstreamGroupType;
         UINT_8 ledstreamGroupType_mask;
         PDL_LEDSTREAM_PORT_MODES_PARAMS_STC ledstreamPortModes;
         UINT_8 ledstreamPortModes_mask;
    } PDL_LEDSTREAM_INTERFACE_MODES_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_GPIO_BLINK_COUNTER_GROUP_TYPE_PARAMS_STC ledGpioBlinkCounterGroupType;
         UINT_8 ledGpioBlinkCounterGroupType_mask;
    } PDL_NORMALRATEBLINKCOUNTERS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_STACK_LED_CONTROLLER_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_GPIO_BLINK_COUNTER_GROUP_TYPE_PARAMS_STC ledGpioBlinkCounterGroupType;
         UINT_8 ledGpioBlinkCounterGroupType_mask;
    } PDL_HIGHRATEBLINKCOUNTERS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_MPP_DEVICE_NUMBER_TYPE_TYP mppDeviceNumber;
    } PDL_PACKED_STRUCT_END PDL_LED_GPIO_RATE_CONTROL_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LED_GPIO_RATE_CONTROL_LIST_KEYS_STC list_keys;
         PDL_NORMALRATEBLINKCOUNTERS_PARAMS_STC normalrateblinkcounters;
         UINT_8 normalrateblinkcounters_mask;
         BOOLEAN normalrateblinkIsSupported;
         UINT_8 normalrateblinkIsSupported_mask;
         PDL_HIGHRATEBLINKCOUNTERS_PARAMS_STC highrateblinkcounters;
         UINT_8 highrateblinkcounters_mask;
         BOOLEAN lowrateblinkIsSupported;
         UINT_8 lowrateblinkIsSupported_mask;
         PDL_LOWRATEBLINKCOUNTERS_PARAMS_STC lowrateblinkcounters;
         UINT_8 lowrateblinkcounters_mask;
         BOOLEAN highrateblinkIsSupported;
         UINT_8 highrateblinkIsSupported_mask;
    } PDL_LED_GPIO_RATE_CONTROL_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_GROUP_NUMBER_TYPE_TYP panelGroupNumber;
        /* key index 1 */         PDL_PANEL_PORT_NUMBER_TYPE_TYP panelPortNumber;
    } PDL_PACKED_STRUCT_END PDL_PORTLEDS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PORTLEDS_LIST_KEYS_STC list_keys;
         PDL_SINGLELED_PARAMS_STC singleled;
         UINT_8 singleled_mask;
         PDL_LEFTLED_PARAMS_STC leftled;
         UINT_8 leftled_mask;
         PDL_RIGHTLED_PARAMS_STC rightled;
         UINT_8 rightled_mask;
    } PDL_PORTLEDS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_LED_GPIO_RATE_CONTROL_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledGpioRateControlList_PTR;
         UINT_8 ledGpioRateControlList_mask;
    } PDL_GROUPING_LED_GPIORATE_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_GPIORATE_GROUP_TYPE_PARAMS_STC ledGpiorateGroupType;
         UINT_8 ledGpiorateGroupType_mask;
    } PDL_GPIOBLINKRATECONTROL_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_POWER_CONNECTION_READ_ONLY_GPIO_GROUP_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP powerConnectionReadOnlyGpioGroupList_PTR;
         UINT_8 powerConnectionReadOnlyGpioGroupList_mask;
         PDL_POWER_CONNECTION_READ_ONLY_I2C_PARAMS_STC powerConnectionReadOnlyI2C;
         UINT_8 powerConnectionReadOnlyI2C_mask;
         PDL_INTERFACE_TYPE_ENT interfaceSelect;
         UINT_8 interfaceSelect_mask;
    } PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC powerConnectionReadOnlyGroupType;
         UINT_8 powerConnectionReadOnlyGroupType_mask;
    } PDL_PD_ACTIVEINFO_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC powerConnectionReadOnlyGroupType;
         UINT_8 powerConnectionReadOnlyGroupType_mask;
    } PDL_PSE_GROUP_ACTIVEINFO_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_SYSTEMLED_PARAMS_STC;

    typedef struct 
    {
         PDL_SERDES_LANE_INFORMATION_PARAMS_STC serdesLaneInformation;
         UINT_8 serdesLaneInformation_mask;
    } PDL_GROUPING_SERDES_GROUP_TYPE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC powerConnectionReadOnlyGroupType;
         UINT_8 powerConnectionReadOnlyGroupType_mask;
    } PDL_PS_ACTIVEINFO_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC powerConnectionReadOnlyGroupType;
         UINT_8 powerConnectionReadOnlyGroupType_mask;
    } PDL_STATUS_VALUE_PARAMS_STC;

    typedef struct 
    {
         PDL_PSEINDEX_TYPE_TYP index2;
         UINT_8 index2_mask;
         PDL_PSEINDEX_TYPE_TYP index1;
         UINT_8 index1_mask;
         PDL_PSE_GROUP_ACTIVEINFO_PARAMS_STC pseGroupActiveinfo;
         UINT_8 pseGroupActiveinfo_mask;
         PDL_PSEPORT_TYPE_ENT portType;
         UINT_8 portType_mask;
    } PDL_PSE_GROUP_PARAMS_STC;

    typedef struct 
    {
         PDL_PS_ACTIVEINFO_PARAMS_STC psActiveinfo;
         UINT_8 psActiveinfo_mask;
    } PDL_PS_PARAMS_STC;

    typedef struct 
    {
         char * mpdExternalFileName;
         UINT_8 mpdExternalFileName_mask;
    } PDL_MPD_EXTERNAL_FILE_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_RPSLED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PP_DEVICE_NUMBER_TYPE_TYP ppDeviceNumber;
        /* key index 1 */         UINT_8 ledstreamInterfaceNumber;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAM_INTERFACE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAM_INTERFACE_LIST_KEYS_STC list_keys;
         PDL_LEDSTREAM_INTERFACE_INIT_VALUE_PARAMS_STC ledstreamInterfaceInitValue;
         UINT_8 ledstreamInterfaceInitValue_mask;
         PDL_LEDSTREAM_INTERFACE_CONFIGSET_PARAMS_STC ledstreamInterfaceConfigset;
         UINT_8 ledstreamInterfaceConfigset_mask;
         PDL_LEDSTREAM_INTERFACE_MODES_PARAMS_STC ledstreamInterfaceModes;
         UINT_8 ledstreamInterfaceModes_mask;
         BOOLEAN ledstreamInterfaceInitExist;
         UINT_8 ledstreamInterfaceInitExist_mask;
    } PDL_LEDSTREAM_INTERFACE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         UINT_8 phyLogicalPort;
         UINT_8 phyLogicalPort_mask;
         /* db of type PDL_PHY_POST_INIT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP phyPostInitList_PTR;
         UINT_8 phyPostInitList_mask;
         PDL_GROUPING_PHY_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC phyReadWriteAddressGroupType;
         UINT_8 phyReadWriteAddressGroupType_mask;
    } PDL_PORT_PHY_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_GROUP_NUMBER_TYPE_TYP panelGroupNumber;
        /* key index 1 */         PDL_PANEL_PORT_NUMBER_TYPE_TYP panelPortNumber;
    } PDL_PACKED_STRUCT_END PDL_LEDSTREAMPORTLEDS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEDSTREAMPORTLEDS_LIST_KEYS_STC list_keys;
         PDL_LEDSTREAM_PORT_INIT_PARAMS_STC ledstreamPortInit;
         UINT_8 ledstreamPortInit_mask;
    } PDL_LEDSTREAMPORTLEDS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_STACKLED_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP stackledList_PTR;
         UINT_8 stackledList_mask;
         PDL_STACK_LED_CONTROLLER_PARAMS_STC stackLedController;
         UINT_8 stackLedController_mask;
    } PDL_STACKLEDS_PARAMS_STC;

    typedef struct 
    {
         PDL_NEGOTIATION_MODE_TYPE_ENT negotiationMode;
         UINT_8 negotiationMode_mask;
    } PDL_NEGOTIATION_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_LEDSTREAMPORTLEDS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamportledsList_PTR;
         UINT_8 ledstreamportledsList_mask;
    } PDL_LEDSTREAMPORTSLEDS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_PORT_L1_INTERFACE_MODES_GROUP_TYPE_PARAMS_STC portL1InterfaceModesGroupType;
         UINT_8 portL1InterfaceModesGroupType_mask;
    } PDL_FIBER_L1_INTERFACE_MODES_GROUP_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_POELED_PARAMS_STC;

    typedef struct 
    {
         PDL_NUMBER_OF_PPS_TYPE_TYP numberOfPps;
         UINT_8 numberOfPps_mask;
    } PDL_PACKET_PROCESSORS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_PORT_L1_INTERFACE_MODES_GROUP_TYPE_PARAMS_STC portL1InterfaceModesGroupType;
         UINT_8 portL1InterfaceModesGroupType_mask;
    } PDL_COPPER_L1_INTERFACE_MODES_GROUP_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 sensorNumber;
    } PDL_PACKED_STRUCT_END PDL_SENSOR_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_SENSOR_LIST_KEYS_STC list_keys;
         PDL_SENSOR_HW_TYPE_ENT sensorType;
         UINT_8 sensorType_mask;
         BOOLEAN boardInterfaceIsI2C;
         UINT_8 boardInterfaceIsI2C_mask;
         PDL_PHY_NUMBER_TYPE_TYP phyNumber;
         UINT_8 phyNumber_mask;
         PDL_EXTERNAL_DRIVER_ID_TYPE_TYP externalDriverId;
         UINT_8 externalDriverId_mask;
         char * sensorDescription;
         UINT_8 sensorDescription_mask;
         PDL_SENSOR_FAN_NCT7802_TYPE_ENT fanNct7802SpecificSensorId;
         UINT_8 fanNct7802SpecificSensorId_mask;
         PDL_FAN_CONTROLLER_NUMBER_TYPE_TYP fanControllerNumber;
         UINT_8 fanControllerNumber_mask;
         PDL_SENSOR_FAN_ADT7476_TYPE_ENT fanAdt7476SpecificSensorId;
         UINT_8 fanAdt7476SpecificSensorId_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP deviceNumber;
         UINT_8 deviceNumber_mask;
         PDL_BOARD_SENSOR_PARAMS_STC boardSensor;
         UINT_8 boardSensor_mask;
    } PDL_SENSOR_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_FANLED_PARAMS_STC;

    typedef struct 
    {
         PDL_TXISENABLEDCONTROL_PARAMS_STC txisenabledcontrol;
         UINT_8 txisenabledcontrol_mask;
         PDL_TXISDISABLEDCONTROL_PARAMS_STC txisdisabledcontrol;
         UINT_8 txisdisabledcontrol_mask;
         PDL_LASERISLOSSINFO_PARAMS_STC laserislossinfo;
         UINT_8 laserislossinfo_mask;
         PDL_GBICCONNECTEDINFO_PARAMS_STC gbicconnectedinfo;
         UINT_8 gbicconnectedinfo_mask;
         PDL_MEMORYCONTROL_PARAMS_STC memorycontrol;
         UINT_8 memorycontrol_mask;
    } PDL_GBIC_GROUP_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_PORTLEDSTATELED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_PORT_NUMBER_TYPE_TYP portNumber;
    } PDL_PACKED_STRUCT_END PDL_PORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PORT_LIST_KEYS_STC list_keys;
         PDL_GROUPING_SERDES_GROUP_TYPE_PARAMS_STC serdesGroupType;
         UINT_8 serdesGroupType_mask;
         BOOLEAN swapAbcd;
         UINT_8 swapAbcd_mask;
         PDL_LOGICAL_PORT_NUMBER_TYPE_TYP logicalPortNumber;
         UINT_8 logicalPortNumber_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP ppDeviceNumber;
         UINT_8 ppDeviceNumber_mask;
         PDL_PP_PORT_NUMBER_TYPE_TYP ppPortNumber;
         UINT_8 ppPortNumber_mask;
         PDL_COPPER_L1_INTERFACE_MODES_GROUP_PARAMS_STC copperL1InterfaceModesGroup;
         UINT_8 copperL1InterfaceModesGroup_mask;
         PDL_FIBER_L1_INTERFACE_MODES_GROUP_PARAMS_STC fiberL1InterfaceModesGroup;
         UINT_8 fiberL1InterfaceModesGroup_mask;
         PDL_TRANSCEIVER_TYPE_ENT transceiver;
         UINT_8 transceiver_mask;
         BOOLEAN isPhyUsed;
         UINT_8 isPhyUsed_mask;
         PDL_PORT_PHY_PARAMS_STC portPhy;
         UINT_8 portPhy_mask;
         PDL_GBIC_GROUP_PARAMS_STC gbicGroup;
         UINT_8 gbicGroup_mask;
    } PDL_PORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_POWER_STATUS_TYPE_ENT statusType;
    } PDL_PACKED_STRUCT_END PDL_STATUS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_STATUS_LIST_KEYS_STC list_keys;
         PDL_STATUS_VALUE_PARAMS_STC statusValue;
         UINT_8 statusValue_mask;
    } PDL_STATUS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_PORT_LEDS_SUPPORTED_TYPE_ENT oobledsSupportedType;
         UINT_8 oobledsSupportedType_mask;
         PDL_PANEL_GROUP_NUMBER_TYPE_TYP panelGroupNumber;
         UINT_8 panelGroupNumber_mask;
         PDL_PANEL_PORT_NUMBER_TYPE_TYP panelPortNumber;
         UINT_8 panelPortNumber_mask;
         PDL_OOBSINGLELED_PARAMS_STC oobsingleled;
         UINT_8 oobsingleled_mask;
         PDL_OOBLEFTLED_PARAMS_STC oobleftled;
         UINT_8 oobleftled_mask;
         PDL_OOBRIGHTLED_PARAMS_STC oobrightled;
         UINT_8 oobrightled_mask;
    } PDL_OOBLEDS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_GROUP_NUMBER_TYPE_TYP groupNumber;
    } PDL_PACKED_STRUCT_END PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC list_keys;
         char * prefixName;
         UINT_8 prefixName_mask;
         PDL_PANEL_PORT_NUMBER_TYPE_TYP firstPortNumberShift;
         UINT_8 firstPortNumberShift_mask;
         BOOLEAN isLogicalPortRequired;
         UINT_8 isLogicalPortRequired_mask;
         /* db of type PDL_PORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP portList_PTR;
         UINT_8 portList_mask;
         PDL_PANEL_GROUP_ORDERING_TYPE_ENT groupOrdering;
         UINT_8 groupOrdering_mask;
         PDL_SPEED_TYPE_ENT speed;
         UINT_8 speed_mask;
    } PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC powerConnectionReadOnlyGroupType;
         UINT_8 powerConnectionReadOnlyGroupType_mask;
    } PDL_CONNECTEDINFO_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_CLOUDMGMTLED_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP frontPanelGroupList_PTR;
         UINT_8 frontPanelGroupList_mask;
    } PDL_NETWORK_PORTS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_BANK_NUMBER_TYPE_TYP bankNumber;
    } PDL_PACKED_STRUCT_END PDL_POEBANK_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_POEBANK_LIST_KEYS_STC list_keys;
         PDL_BANK_WATTS_TYPE_TYP bankWatts;
         UINT_8 bankWatts_mask;
         PDL_BANK_SOURCE_TYPE_ENT bankSource;
         UINT_8 bankSource_mask;
    } PDL_POEBANK_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_STATUS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP statusList_PTR;
         UINT_8 statusList_mask;
         PDL_CONNECTEDINFO_PARAMS_STC connectedinfo;
         UINT_8 connectedinfo_mask;
    } PDL_RPS_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_POEBANK_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP poebankList_PTR;
         UINT_8 poebankList_mask;
    } PDL_POE_POWER_BANKS_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_MPP_DEVICE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP mppDeviceList_PTR;
         UINT_8 mppDeviceList_mask;
    } PDL_MPP_GPIO_PINS_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_LOCATORLED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_GROUP_NUMBER_TYPE_TYP panelGroupNumber;
        /* key index 1 */         PDL_PANEL_PORT_NUMBER_TYPE_TYP panelPortNumber;
    } PDL_PACKED_STRUCT_END PDL_PSEPORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PSEPORT_LIST_KEYS_STC list_keys;
         PDL_PSE_GROUP_PARAMS_STC pseGroup;
         UINT_8 pseGroup_mask;
    } PDL_PSEPORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_PACKET_PROCESSOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP packetProcessorList_PTR;
         UINT_8 packetProcessorList_mask;
    } PDL_SERDES_PARAMS_STC;

    typedef struct 
    {
         PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC ledExtdrvGroupType;
         UINT_8 ledExtdrvGroupType_mask;
    } PDL_SPEEDLED_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PSE_NUMBER_TYPE_TYP pseNumber;
    } PDL_PACKED_STRUCT_END PDL_PSE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PSE_LIST_KEYS_STC list_keys;
         PDL_PSE_ADDRESS_TYPE_TYP pseAddress;
         UINT_8 pseAddress_mask;
    } PDL_PSE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_BUTTON_TYPE_ENT buttonId;
    } PDL_PACKED_STRUCT_END PDL_BUTTONS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_BUTTONS_LIST_KEYS_STC list_keys;
         PDL_PUSH_VALUE_PARAMS_STC pushValue;
         UINT_8 pushValue_mask;
    } PDL_BUTTONS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_PSE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP pseList_PTR;
         UINT_8 pseList_mask;
         PDL_PSE_TYPE_ENT pseInformation;
         UINT_8 pseInformation_mask;
         /* db of type PDL_PSEPORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP pseportList_PTR;
         UINT_8 pseportList_mask;
    } PDL_PSEPORTS_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_PHY_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP phyList_PTR;
         UINT_8 phyList_mask;
    } PDL_PHYS_PARAMS_STC;

    typedef struct 
    {
         PDL_PORT_LEDS_SUPPORTED_TYPE_ENT portledsSupportedType;
         UINT_8 portledsSupportedType_mask;
         /* db of type PDL_PORTLEDS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP portledsList_PTR;
         UINT_8 portledsList_mask;
    } PDL_PORTSLEDS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_CPU_SDMA_PORT_INDEX_TYPE_TYP cpuSdmaPortIndex;
    } PDL_PACKED_STRUCT_END PDL_CPU_SDMA_PORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_CPU_SDMA_PORT_LIST_KEYS_STC list_keys;
         PDL_CPU_LOGICAL_PORT_NUMBER_TYPE_TYP logicalPortNumber;
         UINT_8 logicalPortNumber_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP ppDeviceNumber;
         UINT_8 ppDeviceNumber_mask;
         PDL_PP_PORT_NUMBER_TYPE_TYP ppPortNumber;
         UINT_8 ppPortNumber_mask;
    } PDL_CPU_SDMA_PORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_GROUPING_PHY_READ_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC phyReadWriteAddressGroupType;
         UINT_8 phyReadWriteAddressGroupType_mask;
         PDL_CPU_PORT_NUMBER_TYPE_TYP cpuPortNumber;
         UINT_8 cpuPortNumber_mask;
         UINT_8 ethId;
         UINT_8 ethId_mask;
         UINT_8 logicalPort;
         UINT_8 logicalPort_mask;
         UINT_8 oobDevice;
         UINT_8 oobDevice_mask;
         PDL_SPEED_TYPE_ENT speed;
         UINT_8 speed_mask;
    } PDL_OOB_PORT_PARAMS_STC;

    typedef struct 
    {
         PDL_LEDSTREAMPORTSLEDS_PARAMS_STC ledstreamportsleds;
         UINT_8 ledstreamportsleds_mask;
         /* db of type PDL_LEDSTREAM_INTERFACE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP ledstreamInterfaceList_PTR;
         UINT_8 ledstreamInterfaceList_mask;
    } PDL_LEDSTREAMLEDS_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_CPU_SDMA_PORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP cpuSdmaPortList_PTR;
         UINT_8 cpuSdmaPortList_mask;
    } PDL_CPU_SDMA_PORT_MAPPING_PARAMS_STC;

    typedef struct 
    {
         PDL_SW_FAN_CONTROLLERS_PARAMS_STC swFanControllers;
         UINT_8 swFanControllers_mask;
         BOOLEAN fancontrollerIsSupported;
         UINT_8 fancontrollerIsSupported_mask;
         /* db of type PDL_FAN_CONTROLLER_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP fanControllerList_PTR;
         UINT_8 fanControllerList_mask;
         BOOLEAN swfancontrollerIsSupported;
         UINT_8 swfancontrollerIsSupported_mask;
    } PDL_FANS_PARAMS_STC;

    typedef enum 
    {
        PDL_CPU_INFO_TYPE_NONE_E,
        PDL_CPU_INFO_TYPE_CPU_INFO_TYPE_LEGACY_E,
        PDL_CPU_INFO_TYPE_CPU_INFO_TYPE_CURRENT_E
    }PDL_CPU_INFO_TYPE_ENT;
    typedef union 
    {
         PDL_CPU_INFO_TYPE_LEGACY_PARAMS_STC cpuInfoTypeLegacy;
         PDL_CPU_INFO_TYPE_CURRENT_PARAMS_STC cpuInfoTypeCurrent;
    } PDL_CPU_INFO_TYPE_PARAMS_UNT;
    typedef struct 
    {
        PDL_CPU_INFO_TYPE_ENT type;
        PDL_CPU_INFO_TYPE_PARAMS_UNT data;
    } PDL_CPU_INFO_TYPE_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN fanledIsSupported;
         UINT_8 fanledIsSupported_mask;
         PDL_SYSTEMLED_PARAMS_STC systemled;
         UINT_8 systemled_mask;
         BOOLEAN stackledIsSupported;
         UINT_8 stackledIsSupported_mask;
         BOOLEAN cloudmgmtledIsSupported;
         UINT_8 cloudmgmtledIsSupported_mask;
         PDL_GPIOBLINKRATECONTROL_PARAMS_STC gpioblinkratecontrol;
         UINT_8 gpioblinkratecontrol_mask;
         BOOLEAN rpsledIsSupported;
         UINT_8 rpsledIsSupported_mask;
         BOOLEAN speedledIsSupported;
         UINT_8 speedledIsSupported_mask;
         BOOLEAN gpioblinkrateconrtolIsSupported;
         UINT_8 gpioblinkrateconrtolIsSupported_mask;
         PDL_PORTSLEDS_PARAMS_STC portsleds;
         UINT_8 portsleds_mask;
         PDL_LEDSTREAMLEDS_PARAMS_STC ledstreamleds;
         UINT_8 ledstreamleds_mask;
         BOOLEAN oobledstateIsSupported;
         UINT_8 oobledstateIsSupported_mask;
         PDL_PORTLEDSTATELED_PARAMS_STC portledstateled;
         UINT_8 portledstateled_mask;
         BOOLEAN locatorledIsSupported;
         UINT_8 locatorledIsSupported_mask;
         PDL_STACKLEDS_PARAMS_STC stackleds;
         UINT_8 stackleds_mask;
         PDL_CLOUDMGMTLED_PARAMS_STC cloudmgmtled;
         UINT_8 cloudmgmtled_mask;
         BOOLEAN systemledIsSupported;
         UINT_8 systemledIsSupported_mask;
         PDL_POELED_PARAMS_STC poeled;
         UINT_8 poeled_mask;
         PDL_RPSLED_PARAMS_STC rpsled;
         UINT_8 rpsled_mask;
         PDL_SPEEDLED_PARAMS_STC speedled;
         UINT_8 speedled_mask;
         PDL_LOCATORLED_PARAMS_STC locatorled;
         UINT_8 locatorled_mask;
         BOOLEAN poeledIsSupported;
         UINT_8 poeledIsSupported_mask;
         BOOLEAN ledstreamIsSupported;
         UINT_8 ledstreamIsSupported_mask;
         BOOLEAN portledstateIsSupported;
         UINT_8 portledstateIsSupported_mask;
         PDL_OOBLEDS_PARAMS_STC oobleds;
         UINT_8 oobleds_mask;
         PDL_FANLED_PARAMS_STC fanled;
         UINT_8 fanled_mask;
         BOOLEAN portledsIsSupported;
         UINT_8 portledsIsSupported_mask;
    } PDL_LEDS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_EXTERNAL_DRIVER_ID_TYPE_TYP externalDriverId;
    } PDL_PACKED_STRUCT_END PDL_EXTERNAL_DRIVER_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_EXTERNAL_DRIVER_LIST_KEYS_STC list_keys;
         char * externalDriverDescription;
         UINT_8 externalDriverDescription_mask;
    } PDL_EXTERNAL_DRIVER_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         UINT_8 sensorToShow;
         UINT_8 sensorToShow_mask;
         /* db of type PDL_SENSOR_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP sensorList_PTR;
         UINT_8 sensorList_mask;
    } PDL_SENSORS_PARAMS_STC;

    typedef struct 
    {
         PDL_PS_PARAMS_STC ps;
         UINT_8 ps_mask;
         PDL_RPS_PARAMS_STC rps;
         UINT_8 rps_mask;
         BOOLEAN rpsIsSupported;
         UINT_8 rpsIsSupported_mask;
    } PDL_POWER_PARAMS_STC;

    typedef struct 
    {
         char * fwFileName;
         UINT_8 fwFileName_mask;
         BOOLEAN dteSupport;
         UINT_8 dteSupport_mask;
         BOOLEAN pseportsAreSupported;
         UINT_8 pseportsAreSupported_mask;
         PDL_POE_HOST_SERIAL_CHANNEL_ID_ENT hostSerialChannelId;
         UINT_8 hostSerialChannelId_mask;
         PDL_POE_MAPPING_METHOD_TYPE_ENT class0MappingMethod;
         UINT_8 class0MappingMethod_mask;
         UINT_32 poePoweredPdPortsDefaultRequestValueMw;
         UINT_8 poePoweredPdPortsDefaultRequestValueMw_mask;
         PDL_POE_COMMUNICATION_TYPE_ENT poeCommunicationTypeValue;
         UINT_8 poeCommunicationTypeValue_mask;
         char * pseFwFileName;
         UINT_8 pseFwFileName_mask;
         PDL_POE_HARDWARE_TYPE_ENT poeHwTypeValue;
         UINT_8 poeHwTypeValue_mask;
         PDL_POE_POWER_BANKS_PARAMS_STC poePowerBanks;
         UINT_8 poePowerBanks_mask;
         PDL_PSEPORTS_PARAMS_STC pseports;
         UINT_8 pseports_mask;
         PDL_POE_VENDOR_TYPE_ENT poeVendor;
         UINT_8 poeVendor_mask;
         PDL_POE_MCU_TYPE_ENT mcuType;
         UINT_8 mcuType_mask;
    } PDL_POE_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_BUTTONS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP buttonsList_PTR;
         UINT_8 buttonsList_mask;
    } PDL_BUTTONS_PARAMS_STC;

    typedef struct 
    {
         PDL_CPU_SDMA_PORT_MAPPING_PARAMS_STC cpuSdmaPortMapping;
         UINT_8 cpuSdmaPortMapping_mask;
         PDL_CPU_INFO_TYPE_PARAMS_STC cpuInfoType;
         UINT_8 cpuInfoType_mask;
    } PDL_CPU_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_EXTERNAL_DRIVER_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP externalDriverList_PTR;
         UINT_8 externalDriverList_mask;
    } PDL_EXTERNAL_DRIVER_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PANEL_GROUP_NUMBER_TYPE_TYP panelGroupNumber;
        /* key index 1 */         PDL_PANEL_PORT_NUMBER_TYPE_TYP panelPortNumber;
    } PDL_PACKED_STRUCT_END PDL_PDPORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_PDPORT_LIST_KEYS_STC list_keys;
         PDL_POE_POWERED_PD_PORTS_TYPE_ENT poePoweredPdPortsType;
         UINT_8 poePoweredPdPortsType_mask;
         PDL_PD_GROUP_PARAMS_STC pdGroup;
         UINT_8 pdGroup_mask;
    } PDL_PDPORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         UINT_8 placeHolder;
         UINT_8 placeHolder_mask;
    } PDL_STACK_INFO_PARAMS_STC;

    typedef struct 
    {
         PDL_PD_ACTIVEINFO_PARAMS_STC pdActiveinfo;
         UINT_8 pdActiveinfo_mask;
    } PDL_PD_PARAMS_STC;

    typedef struct 
    {
         char * boardDescription;
         UINT_8 boardDescription_mask;
    } PDL_BOARD_INFORMATION_PARAMS_STC;

    typedef struct 
    {
         /* db of type PDL_PDPORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP pdportList_PTR;
         UINT_8 pdportList_mask;
         PDL_PD_PARAMS_STC pd;
         UINT_8 pd_mask;
    } PDL_PDPORTS_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LANE_NUMBER_TYPE_TYP laneNumber;
    } PDL_PACKED_STRUCT_END PDL_LEFT_SERDES_LANE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_LEFT_SERDES_LANE_LIST_KEYS_STC list_keys;
         PDL_PORT_LANE_NUMBER_TYPE_TYP portLaneNumber;
         UINT_8 portLaneNumber_mask;
    } PDL_LEFT_SERDES_LANE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         BOOLEAN isIsrRegAddressUsed;
         UINT_8 isIsrRegAddressUsed_mask;
         UINT_32 poePoweredPdPortsDefaultRequestValueMw;
         UINT_8 poePoweredPdPortsDefaultRequestValueMw_mask;
         UINT_32 poePoweredPdNumOfPorts;
         UINT_8 poePoweredPdNumOfPorts_mask;
         BOOLEAN poePdPowerNegotiationSupported;
         UINT_8 poePdPowerNegotiationSupported_mask;
         PDL_PDPORTS_PARAMS_STC pdports;
         UINT_8 pdports_mask;
         PDL_ISRREGADDRESS_PARAMS_STC isrregaddress;
         UINT_8 isrregaddress_mask;
         BOOLEAN poePdPowerManagementSupported;
         UINT_8 poePdPowerManagementSupported_mask;
    } PDL_POEPD_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_LANE_NUMBER_TYPE_TYP laneNumber;
    } PDL_PACKED_STRUCT_END PDL_RIGHT_SERDES_LANE_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_RIGHT_SERDES_LANE_LIST_KEYS_STC list_keys;
         PDL_PORT_LANE_NUMBER_TYPE_TYP portLaneNumber;
         UINT_8 portLaneNumber_mask;
    } PDL_RIGHT_SERDES_LANE_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         PDL_ONLINE_UPGRADE_PARAMS_STC onlineUpgrade;
         UINT_8 onlineUpgrade_mask;
         BOOLEAN onlineUpgradeSupported;
         UINT_8 onlineUpgradeSupported_mask;
    } PDL_CPLD_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PP_DEVICE_NUMBER_TYPE_TYP leftPpDeviceNumber;
        /* key index 1 */         PDL_PP_PORT_NUMBER_TYPE_TYP leftPpPortNumber;
    } PDL_PACKED_STRUCT_END PDL_BACK_TO_BACK_LINK_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_BACK_TO_BACK_LINK_LIST_KEYS_STC list_keys;
         PDL_LOGICAL_PORT_NUMBER_TYPE_TYP leftLogicalPortNumber;
         UINT_8 leftLogicalPortNumber_mask;
         PDL_PP_PORT_NUMBER_TYPE_TYP rightPpPortNumber;
         UINT_8 rightPpPortNumber_mask;
         PDL_L1_INTERFACE_MODE_TYPE_ENT mode;
         UINT_8 mode_mask;
         PDL_SPEED_TYPE_ENT speed;
         UINT_8 speed_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP rightPpDeviceNumber;
         UINT_8 rightPpDeviceNumber_mask;
         PDL_LOGICAL_PORT_NUMBER_TYPE_TYP rightLogicalPortNumber;
         UINT_8 rightLogicalPortNumber_mask;
    } PDL_BACK_TO_BACK_LINK_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         PDL_PP_PORT_NUMBER_TYPE_TYP leftPpPortNumber;
    } PDL_PACKED_STRUCT_END PDL_BACK_TO_BACK_PORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ PDL_BACK_TO_BACK_PORT_LIST_KEYS_STC list_keys;
         PDL_LOGICAL_PORT_NUMBER_TYPE_TYP leftLogicalPortNumber;
         UINT_8 leftLogicalPortNumber_mask;
         PDL_PP_PORT_NUMBER_TYPE_TYP rightPpPortNumber;
         UINT_8 rightPpPortNumber_mask;
         /* db of type PDL_LEFT_SERDES_LANE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP leftSerdesLaneList_PTR;
         UINT_8 leftSerdesLaneList_mask;
         PDL_L1_INTERFACE_MODE_TYPE_ENT mode;
         UINT_8 mode_mask;
         /* db of type PDL_RIGHT_SERDES_LANE_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP rightSerdesLaneList_PTR;
         UINT_8 rightSerdesLaneList_mask;
         PDL_SPEED_TYPE_ENT speed;
         UINT_8 speed_mask;
         PDL_LOGICAL_PORT_NUMBER_TYPE_TYP rightLogicalPortNumber;
         UINT_8 rightLogicalPortNumber_mask;
    } PDL_BACK_TO_BACK_PORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         /* db of type PDL_BACK_TO_BACK_LINK_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP backToBackLinkList_PTR;
         UINT_8 backToBackLinkList_mask;
    } PDL_BACK_TO_BACK_LINKS_PARAMS_STC;

    typedef struct 
    {
         PDL_PP_DEVICE_NUMBER_TYPE_TYP leftPpDeviceNumber;
         UINT_8 leftPpDeviceNumber_mask;
         /* db of type PDL_BACK_TO_BACK_PORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP backToBackPortList_PTR;
         UINT_8 backToBackPortList_mask;
         PDL_PP_DEVICE_NUMBER_TYPE_TYP rightPpDeviceNumber;
         UINT_8 rightPpDeviceNumber_mask;
    } PDL_BACK_TO_BACK_LINK_INFORMATION_PARAMS_STC;

    typedef struct 
    {
         PDL_BACK_TO_BACK_LINK_INFORMATION_PARAMS_STC backToBackLinkInformation;
         UINT_8 backToBackLinkInformation_mask;
    } PDL_BACK_TO_BACK_CURRENT_PARAMS_STC;

    typedef struct 
    {
         PDL_BACK_TO_BACK_LINKS_PARAMS_STC backToBackLinks;
         UINT_8 backToBackLinks_mask;
    } PDL_BACK_TO_BACK_LEGACY_V1_PARAMS_STC;

    typedef enum 
    {
        PDL_BACK_TO_BACK_INFORMATION_NONE_E,
        PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_CURRENT_E,
        PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_LEGACY_V1_E
    }PDL_BACK_TO_BACK_INFORMATION_ENT;
    typedef union 
    {
         PDL_BACK_TO_BACK_CURRENT_PARAMS_STC backToBackCurrent;
         PDL_BACK_TO_BACK_LEGACY_V1_PARAMS_STC backToBackLegacyV1;
    } PDL_BACK_TO_BACK_INFORMATION_PARAMS_UNT;
    typedef struct 
    {
        PDL_BACK_TO_BACK_INFORMATION_ENT type;
        PDL_BACK_TO_BACK_INFORMATION_PARAMS_UNT data;
    } PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN powerFeatureIsSupported;
         UINT_8 powerFeatureIsSupported_mask;
         BOOLEAN cpuSdmaPortMappingIsSupported;
         UINT_8 cpuSdmaPortMappingIsSupported_mask;
         BOOLEAN negotiationFeatureIsSupported;
         UINT_8 negotiationFeatureIsSupported_mask;
         BOOLEAN cpldFeatureIsSupported;
         UINT_8 cpldFeatureIsSupported_mask;
         PDL_MPD_EXTERNAL_FILE_PARAMS_STC mpdExternalFile;
         UINT_8 mpdExternalFile_mask;
         BOOLEAN buttonFeatureIsSupported;
         UINT_8 buttonFeatureIsSupported_mask;
         PDL_NETWORK_PORTS_PARAMS_STC networkPorts;
         UINT_8 networkPorts_mask;
         PDL_LEDS_PARAMS_STC leds;
         UINT_8 leds_mask;
         BOOLEAN sensorFeatureIsSupported;
         UINT_8 sensorFeatureIsSupported_mask;
         PDL_BUTTONS_PARAMS_STC buttons;
         UINT_8 buttons_mask;
         PDL_MPP_GPIO_PINS_PARAMS_STC mppGpioPins;
         UINT_8 mppGpioPins_mask;
         BOOLEAN backToBackIsSupported;
         UINT_8 backToBackIsSupported_mask;
         BOOLEAN stackFeatureIsSupported;
         UINT_8 stackFeatureIsSupported_mask;
         PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC backToBackInformation;
         UINT_8 backToBackInformation_mask;
         BOOLEAN poeFeatureIsSupported;
         UINT_8 poeFeatureIsSupported_mask;
         BOOLEAN serdesFeatureIsSupported;
         UINT_8 serdesFeatureIsSupported_mask;
         PDL_CPU_PARAMS_STC cpu;
         UINT_8 cpu_mask;
         PDL_SERDES_PARAMS_STC serdes;
         UINT_8 serdes_mask;
         BOOLEAN gbicIsSupported;
         UINT_8 gbicIsSupported_mask;
         BOOLEAN ledFeatureIsSupported;
         UINT_8 ledFeatureIsSupported_mask;
         BOOLEAN isLogicalPortRequired;
         UINT_8 isLogicalPortRequired_mask;
         BOOLEAN oobFeatureIsSupported;
         UINT_8 oobFeatureIsSupported_mask;
         PDL_SENSORS_PARAMS_STC sensors;
         UINT_8 sensors_mask;
         PDL_EXTERNAL_DRIVER_PARAMS_STC externalDriver;
         UINT_8 externalDriver_mask;
         BOOLEAN cpuFeatureIsSupported;
         UINT_8 cpuFeatureIsSupported_mask;
         BOOLEAN cpuInfoFeatureIsSupported;
         UINT_8 cpuInfoFeatureIsSupported_mask;
         BOOLEAN packetProcessorIsSupported;
         UINT_8 packetProcessorIsSupported_mask;
         PDL_POEPD_PARAMS_STC poepd;
         UINT_8 poepd_mask;
         PDL_OOB_PORT_PARAMS_STC oobPort;
         UINT_8 oobPort_mask;
         BOOLEAN mpdExternalFileSupported;
         UINT_8 mpdExternalFileSupported_mask;
         BOOLEAN poePdIsSupported;
         UINT_8 poePdIsSupported_mask;
         BOOLEAN networkPortsFeatureIsSupported;
         UINT_8 networkPortsFeatureIsSupported_mask;
         BOOLEAN phyFeatureIsSupported;
         UINT_8 phyFeatureIsSupported_mask;
         PDL_POE_PARAMS_STC poe;
         UINT_8 poe_mask;
         PDL_NEGOTIATION_PARAMS_STC negotiation;
         UINT_8 negotiation_mask;
         BOOLEAN boardDescriptionFeatureIsSupported;
         UINT_8 boardDescriptionFeatureIsSupported_mask;
         BOOLEAN externalDriverFeatureIsSupported;
         UINT_8 externalDriverFeatureIsSupported_mask;
         PDL_PHYS_PARAMS_STC phys;
         UINT_8 phys_mask;
         PDL_POWER_PARAMS_STC power;
         UINT_8 power_mask;
         PDL_BOARD_INFORMATION_PARAMS_STC boardInformation;
         UINT_8 boardInformation_mask;
         PDL_PACKET_PROCESSORS_PARAMS_STC packetProcessors;
         UINT_8 packetProcessors_mask;
         BOOLEAN mppPinFeatureIsSupported;
         UINT_8 mppPinFeatureIsSupported_mask;
         PDL_FANS_PARAMS_STC fans;
         UINT_8 fans_mask;
         PDL_STACK_INFO_PARAMS_STC stackInfo;
         UINT_8 stackInfo_mask;
         BOOLEAN fanFeatureIsSupported;
         UINT_8 fanFeatureIsSupported_mask;
         PDL_CPLD_PARAMS_STC cpld;
         UINT_8 cpld_mask;
    } PDL_MTS_PLATFORM_PARAMS_STC;

    /* FEATURES IDS ENUM DECLARTATION */
    typedef enum {
        PDL_FEATURE_ID_LEDS_E,
        PDL_FEATURE_ID_STACK_INFO_E,
        PDL_FEATURE_ID_BUTTONS_E,
        PDL_FEATURE_ID_CPU_INFO_TYPE_E,
        PDL_FEATURE_ID_FANS_E,
        PDL_FEATURE_ID_CPLD_E,
        PDL_FEATURE_ID_OOB_PORT_E,
        PDL_FEATURE_ID_CPU_E,
        PDL_FEATURE_ID_SERDES_E,
        PDL_FEATURE_ID_POWER_E,
        PDL_FEATURE_ID_MPD_EXTERNAL_FILE_E,
        PDL_FEATURE_ID_MPP_GPIO_PINS_E,
        PDL_FEATURE_ID_SENSORS_E,
        PDL_FEATURE_ID_POEPD_E,
        PDL_FEATURE_ID_CPU_SDMA_PORT_MAPPING_E,
        PDL_FEATURE_ID_EXTERNAL_DRIVER_E,
        PDL_FEATURE_ID_BOARD_INFORMATION_E,
        PDL_FEATURE_ID_PACKET_PROCESSORS_E,
        PDL_FEATURE_ID_NETWORK_PORTS_E,
        PDL_FEATURE_ID_POE_E,
        PDL_FEATURE_ID_NEGOTIATION_E,
        PDL_FEATURE_ID_PHYS_E,
        PDL_FEATURE_ID_BACK_TO_BACK_INFORMATION_E,
        PDL_FEATURE_ID_LAST_E
    } PDL_FEATURE_ID_ENT;
    /* FEATURES UNION DECLARTATION */
    typedef union {
        PDL_LEDS_PARAMS_STC leds;
        PDL_STACK_INFO_PARAMS_STC stackInfo;
        PDL_BUTTONS_PARAMS_STC buttons;
        PDL_CPU_INFO_TYPE_PARAMS_STC cpuInfoType;
        PDL_FANS_PARAMS_STC fans;
        PDL_CPLD_PARAMS_STC cpld;
        PDL_OOB_PORT_PARAMS_STC oobPort;
        PDL_CPU_PARAMS_STC cpu;
        PDL_SERDES_PARAMS_STC serdes;
        PDL_POWER_PARAMS_STC power;
        PDL_MPD_EXTERNAL_FILE_PARAMS_STC mpdExternalFile;
        PDL_MPP_GPIO_PINS_PARAMS_STC mppGpioPins;
        PDL_SENSORS_PARAMS_STC sensors;
        PDL_POEPD_PARAMS_STC poepd;
        PDL_CPU_SDMA_PORT_MAPPING_PARAMS_STC cpuSdmaPortMapping;
        PDL_EXTERNAL_DRIVER_PARAMS_STC externalDriver;
        PDL_BOARD_INFORMATION_PARAMS_STC boardInformation;
        PDL_PACKET_PROCESSORS_PARAMS_STC packetProcessors;
        PDL_NETWORK_PORTS_PARAMS_STC networkPorts;
        PDL_POE_PARAMS_STC poe;
        PDL_NEGOTIATION_PARAMS_STC negotiation;
        PDL_PHYS_PARAMS_STC phys;
        PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC backToBackInformation;
    } PDL_FEATURE_DATA_PARAMS_UNT;
    /* STRUCTURE FOR DATA MAPPING */
    typedef struct {
        PDL_FEATURE_ID_ENT feature_id;
        BOOLEAN is_supported;
        UINT_8 is_supported_mask;
        PDL_FEATURE_DATA_PARAMS_UNT * data_PTR;
        UINT_32 data_size;
    } PDL_FEATURE_DATA_STC;
#endif/* #ifndef __PDL_MTS_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__ */
