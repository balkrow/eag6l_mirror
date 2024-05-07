/* ----------------------------------------------------------------------------------------------
   Generated from MPD-Profile-Yang-Module based on revision 2023-01-05:1.9 on 05/03/2023 06:59:46
   ---------------------------------------------------------------------------------------------- */
#ifndef __MPD_MPD_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__
#define __MPD_MPD_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__
    /* typedef start */
    typedef struct
    {
        UINT_32 int_value;
        const char * str_value;
    } MPD_ENUM_STR2VALUE_STC;
    /* typedef end */
    /* typedef start */
    typedef enum
    {
        MPD_ENUM_ID_SPEED_TYPE_E,
        MPD_ENUM_ID_OP_CONNECTED_TYPE_E,
        MPD_ENUM_ID_HOST_LINE_SIDE_TYPE_E,
        MPD_ENUM_ID_PHY_ENUM_TYPE_E,
        MPD_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E,
        MPD_ENUM_ID_USX_MODE_TYPE_E,
        MPD_ENUM_ID_TRANSCEIVER_ENUM_TYPE_E,
        MPD_ENUM_ID_LED_MODE_ENUM_TYPE_E
    } MPD_ENUM_IDS_ENT;
    /* typedef end */
    /* typedef start */
    typedef struct
    {
        MPD_ENUM_IDS_ENT enum_id;
        MPD_ENUM_STR2VALUE_STC * convertArr_PTR;
        UINT_32 arr_size;
    } MPD_ENUM_IDS_ENUM_STR2VALUE_STC;
    /* typedef end */
    typedef void * PRV_PDLIB_DB_TYP;
    #define MPD_MDIO_ADDRESS_TYPE_MIN_VALUE_CNS 0
    #define MPD_MDIO_ADDRESS_TYPE_MAX_VALUE_CNS 31
    typedef UINT_8 MPD_MDIO_ADDRESS_TYPE_TYP;

    #define MPD_PHY_NUMBER_TYPE_MIN_VALUE_CNS 0
    #define MPD_PHY_NUMBER_TYPE_MAX_VALUE_CNS 127
    typedef UINT_8 MPD_PHY_NUMBER_TYPE_TYP;

    typedef enum 
    {
        MPD_SPEED_TYPE_10M_E = 0,
        MPD_SPEED_TYPE_100M_E = 1,
        MPD_SPEED_TYPE_1G_E = 2,
        MPD_SPEED_TYPE_10G_E = 3,
        MPD_SPEED_TYPE_2_5G_E = 5,
        MPD_SPEED_TYPE_5G_E = 6,
        MPD_SPEED_TYPE_20G_E = 8,
        MPD_SPEED_TYPE_25G_E = 21,
        MPD_SPEED_TYPE_40G_E = 9,
        MPD_SPEED_TYPE_50G_E = 14,
        MPD_SPEED_TYPE_100G_E = 13,
        MPD_SPEED_TYPE_200G_E = 24,
        MPD_SPEED_TYPE_400G_E = 25,
        MPD_SPEED_TYPE_ALL_E = 29,
        MPD_SPEED_TYPE_LAST_E = 30    
    } MPD_SPEED_TYPE_ENT;
    /* MPD_ENUM_ID_SPEED_TYPE_E */

    typedef enum 
    {
        MPD_OP_CONNECTED_TYPE_FIBER_E = 0,
        MPD_OP_CONNECTED_TYPE_DAC_E = 1,
        MPD_OP_CONNECTED_TYPE_RJ45_SFP_E = 2,
        MPD_OP_CONNECTED_TYPE_UNKNOWN_E = 3,
        MPD_OP_CONNECTED_TYPE_ALL_E = 4,
        MPD_OP_CONNECTED_TYPE_LAST_E = 5    
    } MPD_OP_CONNECTED_TYPE_ENT;
    /* MPD_ENUM_ID_OP_CONNECTED_TYPE_E */

    typedef enum 
    {
        MPD_HOST_LINE_SIDE_TYPE_HOST_E = 0,
        MPD_HOST_LINE_SIDE_TYPE_LINE_E = 1,
        MPD_HOST_LINE_SIDE_TYPE_ALL_E = 2,
        MPD_HOST_LINE_SIDE_TYPE_LAST_E = 3    
    } MPD_HOST_LINE_SIDE_TYPE_ENT;
    /* MPD_ENUM_ID_HOST_LINE_SIDE_TYPE_E */

    typedef enum 
    {
        MPD_PHY_ENUM_TYPE_NO_PHY_DIRECT_ATTACHED_FIBER_E = 0,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1543_E = 1,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1545_E = 2,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1548_E = 3,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1680_E = 4,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1680L_E = 5,
        MPD_PHY_ENUM_TYPE_ALASKA_88E151X_E = 6,
        MPD_PHY_ENUM_TYPE_ALASKA_88E3680_E = 7,
        MPD_PHY_ENUM_TYPE_ALASKA_88E32X0_E = 8,
        MPD_PHY_ENUM_TYPE_ALASKA_88E33X0_E = 9,
        MPD_PHY_ENUM_TYPE_ALASKA_88E20X0_E = 10,
        MPD_PHY_ENUM_TYPE_ALASKA_88E2180_E = 11,
        MPD_PHY_ENUM_TYPE_ALASKA_88E2540_E = 12,
        MPD_PHY_ENUM_TYPE_ALASKA_88X3540_E = 13,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1780_E = 14,
        MPD_PHY_ENUM_TYPE_ALASKA_88E2580_E = 15,
        MPD_PHY_ENUM_TYPE_ALASKA_88X3580_E = 16,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1112_E = 17,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1781_E = 18,
        MPD_PHY_ENUM_TYPE_ALASKA_88E1781_INTERNAL_E = 19,
        MPD_PHY_ENUM_TYPE_LAST_E = 20    
    } MPD_PHY_ENUM_TYPE_ENT;
    /* MPD_ENUM_ID_PHY_ENUM_TYPE_E */

    typedef enum 
    {
        MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_RAM_E = 1,
        MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_FLASH_E = 2,
        MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_LAST_E = 3    
    } MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENT;
    /* MPD_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E */

    typedef enum 
    {
        MPD_USX_MODE_TYPE_NONE_E = 0,
        MPD_USX_MODE_TYPE_SXGMII_E = 1,
        MPD_USX_MODE_TYPE_10G_DXGMII_E = 2,
        MPD_USX_MODE_TYPE_20G_DXGMII_E = 3,
        MPD_USX_MODE_TYPE_10G_QXGMII_E = 4,
        MPD_USX_MODE_TYPE_20G_QXGMII_E = 5,
        MPD_USX_MODE_TYPE_OXGMII_E = 6,
        MPD_USX_MODE_TYPE_LAST_E = 7    
    } MPD_USX_MODE_TYPE_ENT;
    /* MPD_ENUM_ID_USX_MODE_TYPE_E */

    typedef enum 
    {
        MPD_TRANSCEIVER_ENUM_TYPE_SFP_E = 0,
        MPD_TRANSCEIVER_ENUM_TYPE_RJ45_E = 1,
        MPD_TRANSCEIVER_ENUM_TYPE_COMBO_E = 2,
        MPD_TRANSCEIVER_ENUM_TYPE_LAST_E = 3    
    } MPD_TRANSCEIVER_ENUM_TYPE_ENT;
    /* MPD_ENUM_ID_TRANSCEIVER_ENUM_TYPE_E */

    typedef enum 
    {
        MPD_LED_MODE_ENUM_TYPE_SINGLE_E = 0,
        MPD_LED_MODE_ENUM_TYPE_MATRIX_E = 1,
        MPD_LED_MODE_ENUM_TYPE_LAST_E = 2    
    } MPD_LED_MODE_ENUM_TYPE_ENT;
    /* MPD_ENUM_ID_LED_MODE_ENUM_TYPE_E */

    typedef struct 
    {
         UINT_8 maintap;
         UINT_8 maintap_mask;
         UINT_8 pretap;
         UINT_8 pretap_mask;
         UINT_8 posttap;
         UINT_8 posttap_mask;
    } MPD_COMPHY_TX_PARAMS_PARAMS_STC;

    typedef struct 
    {
         UINT_8 pretap;
         UINT_8 pretap_mask;
         UINT_8 peaktap;
         UINT_8 peaktap_mask;
         UINT_8 margintap;
         UINT_8 margintap_mask;
         UINT_8 posttap;
         UINT_8 posttap_mask;
    } MPD_VOLTRON_TX_PARAMS_PARAMS_STC;

    typedef struct 
    {
         BOOLEAN shortReachSerdes;
         UINT_8 shortReachSerdes_mask;
         BOOLEAN doSwapAbcd;
         UINT_8 doSwapAbcd_mask;
    } MPD_MTD_SPECIFIC_INFORMATION_TYPE_PARAMS_STC;

    typedef struct 
    {
         MPD_TRANSCEIVER_ENUM_TYPE_ENT transceiverType;
         UINT_8 transceiverType_mask;
    } MPD_COMBO_PORT_INFORMATION_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         MPD_SPEED_TYPE_ENT speed;
        /* key index 1 */         MPD_OP_CONNECTED_TYPE_ENT opConnected;
        /* key index 2 */         MPD_HOST_LINE_SIDE_TYPE_ENT side;
    } PDL_PACKED_STRUCT_END MPD_COMPHY_TX_PARAMS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ MPD_COMPHY_TX_PARAMS_LIST_KEYS_STC list_keys;
         MPD_COMPHY_TX_PARAMS_PARAMS_STC comphyTxParams;
         UINT_8 comphyTxParams_mask;
    } MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         MPD_TRANSCEIVER_ENUM_TYPE_ENT transceiverType;
         UINT_8 transceiverType_mask;
    } MPD_FIBER_PORT_INFORMATION_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 1 */         MPD_OP_CONNECTED_TYPE_ENT opConnected;
        /* key index 0 */         MPD_SPEED_TYPE_ENT speed;
        /* key index 2 */         MPD_HOST_LINE_SIDE_TYPE_ENT side;
    } PDL_PACKED_STRUCT_END MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_KEYS_STC list_keys;
         MPD_VOLTRON_TX_PARAMS_PARAMS_STC voltronTxParams;
         UINT_8 voltronTxParams_mask;
    } MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         MPD_TRANSCEIVER_ENUM_TYPE_ENT transceiverType;
         UINT_8 transceiverType_mask;
    } MPD_RJ45_PORT_INFORMATION_TYPE_PARAMS_STC;

    typedef struct 
    {
         /* db of type MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP comphyTxParamsList_PTR;
         UINT_8 comphyTxParamsList_mask;
    } MPD_COMPHY_TX_PARAMS_TYPE_PARAMS_STC;

    typedef struct 
    {
         /* db of type MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP comphyVoltronTxParamsList_PTR;
         UINT_8 comphyVoltronTxParamsList_mask;
    } MPD_COMPHY_VOLTRON_TX_PARAMS_TYPE_PARAMS_STC;

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         UINT_8 logicalPort;
    } PDL_PACKED_STRUCT_END MPD_PORT_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ MPD_PORT_LIST_KEYS_STC list_keys;
         UINT_8 mdioBus;
         UINT_8 mdioBus_mask;
         UINT_32 vctOffset;
         UINT_8 vctOffset_mask;
         MPD_COMPHY_TX_PARAMS_TYPE_PARAMS_STC comphyTxParamsType;
         UINT_8 comphyTxParamsType_mask;
         MPD_FIBER_PORT_INFORMATION_TYPE_PARAMS_STC fiberPortInformationType;
         UINT_8 fiberPortInformationType_mask;
         MPD_MDIO_ADDRESS_TYPE_TYP mdioAddress;
         UINT_8 mdioAddress_mask;
         MPD_MDIO_ADDRESS_TYPE_TYP oobAddress;
         UINT_8 oobAddress_mask;
         BOOLEAN invertMdi;
         UINT_8 invertMdi_mask;
         MPD_COMPHY_VOLTRON_TX_PARAMS_TYPE_PARAMS_STC comphyVoltronTxParamsType;
         UINT_8 comphyVoltronTxParamsType_mask;
         UINT_8 mdioDevice;
         UINT_8 mdioDevice_mask;
         MPD_MTD_SPECIFIC_INFORMATION_TYPE_PARAMS_STC mtdSpecificInformationType;
         UINT_8 mtdSpecificInformationType_mask;
         MPD_COMBO_PORT_INFORMATION_TYPE_PARAMS_STC comboPortInformationType;
         UINT_8 comboPortInformationType_mask;
         MPD_RJ45_PORT_INFORMATION_TYPE_PARAMS_STC rj45PortInformationType;
         UINT_8 rj45PortInformationType_mask;
    } MPD_PORT_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    /* packed typedef start */
    PDL_PRAGMA_NOALIGN
    typedef PDL_PACKED_STRUCT_START struct
    {
        /* key index 0 */         MPD_PHY_NUMBER_TYPE_TYP phyNumber;
    } PDL_PACKED_STRUCT_END MPD_PHY_LIST_KEYS_STC;
    PDL_PRAGMA_ALIGN
    /* packed typedef end */
    /* LIST KEYS STRUCT DEFINITION */typedef struct 
    {
         /* list key */ MPD_PHY_LIST_KEYS_STC list_keys;
         MPD_LED_MODE_ENUM_TYPE_ENT ledMode;
         UINT_8 ledMode_mask;
         /* db of type MPD_PORT_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP portList_PTR;
         UINT_8 portList_mask;
         MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENT phyFirmwareDownloadMethod;
         UINT_8 phyFirmwareDownloadMethod_mask;
         BOOLEAN isOob;
         UINT_8 isOob_mask;
         MPD_USX_MODE_TYPE_ENT usxMode;
         UINT_8 usxMode_mask;
         MPD_PHY_ENUM_TYPE_ENT phyType;
         UINT_8 phyType_mask;
         UINT_8 oobDevice;
         UINT_8 oobDevice_mask;
    } MPD_PHY_LIST_PARAMS_STC;
    /* LIST STRUCT DEFINITION */

    typedef struct 
    {
         BOOLEAN disableOnInit;
         UINT_8 disableOnInit_mask;
         /* db of type MPD_PHY_LIST_PARAMS_STC */ PRV_PDLIB_DB_TYP phyList_PTR;
         UINT_8 phyList_mask;
         BOOLEAN automaticParsing;
         UINT_8 automaticParsing_mask;
    } MPD_MPD_PARAMS_STC;

    typedef struct 
    {
         MPD_MPD_PARAMS_STC mpd;
         UINT_8 mpd_mask;
         BOOLEAN mpdFeatureIsSupported;
         UINT_8 mpdFeatureIsSupported_mask;
    } MPD_MPD_PLATFORM_PARAMS_STC;

    /* FEATURES IDS ENUM DECLARTATION */
    typedef enum {
        MPD_FEATURE_ID_MPD_E,
        MPD_FEATURE_ID_LAST_E
    } MPD_FEATURE_ID_ENT;
    /* FEATURES UNION DECLARTATION */
    typedef union {
        MPD_MPD_PARAMS_STC mpd;
    } MPD_FEATURE_DATA_PARAMS_UNT;
    /* STRUCTURE FOR DATA MAPPING */
    typedef struct {
        MPD_FEATURE_ID_ENT feature_id;
        BOOLEAN is_supported;
        UINT_8 is_supported_mask;
        MPD_FEATURE_DATA_PARAMS_UNT * data_PTR;
        UINT_32 data_size;
    } MPD_FEATURE_DATA_STC;
#endif/* #ifndef __MPD_MPD_Platform_AUTO_GENERATED_PARSER_HEADER_FILE__ */
