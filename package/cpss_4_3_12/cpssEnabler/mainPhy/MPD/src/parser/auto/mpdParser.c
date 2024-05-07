/* ----------------------------------------------------------------------------------------------
   Generated from MPD-Profile-Yang-Module based on revision 2023-01-05:1.9 on 05/03/2023 06:59:46
   ---------------------------------------------------------------------------------------------- */
#include "mpdPrefix.h"
#include "mpdTypes.h"
#include "mpdPrv.h"
#include "mpdApi.h"
#include "pdlib/common/pdlTypes.h"
#include "pdlib/lib/private/prvPdlLib.h"
#include "pdlib/lib/pdlLib.h"
#include "pdlib/xml/private/prvXmlParser.h"
#include "pdlib/xml/private/prvXmlParserBuilder.h"
#include "pdlib/xml/private/prvXmlParser.h"
#include "pdlib/xml/private/prvXmlParserBuilder.h"
#include "parser/mpdParser.h"
#include "parser/private/prvMpdParser.h"

    /* common defiinitions */
    #ifndef MIN
    #define MIN(a,b) (a>b)?b:a
    #endif
    /* handle endianess */
    static int is_big_endian(void) { unsigned int x = 1; char *c = (char*) &x; return !((int)*c); }
     static UINT_32 swap_32(UINT_32 val) { PDL_PRAGMA_NOALIGN union U_TYP { UINT_32 word; UINT_8 bytes[4]; } PDL_PACKED_STRUCT_END v1, v2; PDL_PRAGMA_ALIGN v1.word = val;  v2.bytes[0] = v1.bytes[3]; v2.bytes[1] = v1.bytes[2]; v2.bytes[2] = v1.bytes[1]; v2.bytes[3] = v1.bytes[0]; return v2.word; }
    static int prvMpdIsBe;
    
    static MPD_MPD_PLATFORM_PARAMS_STC prvMpdMpdPlatform;
    
    /* handle enum conversion structs */
    #define MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS 14
    static MPD_ENUM_STR2VALUE_STC mpdSpeedTypeEnumStr2Value[] = {
        {MPD_SPEED_TYPE_10M_E, "10M"},
        {MPD_SPEED_TYPE_100M_E, "100M"},
        {MPD_SPEED_TYPE_1G_E, "1G"},
        {MPD_SPEED_TYPE_10G_E, "10G"},
        {MPD_SPEED_TYPE_2_5G_E, "2.5G"},
        {MPD_SPEED_TYPE_5G_E, "5G"},
        {MPD_SPEED_TYPE_20G_E, "20G"},
        {MPD_SPEED_TYPE_25G_E, "25G"},
        {MPD_SPEED_TYPE_40G_E, "40G"},
        {MPD_SPEED_TYPE_50G_E, "50G"},
        {MPD_SPEED_TYPE_100G_E, "100G"},
        {MPD_SPEED_TYPE_200G_E, "200G"},
        {MPD_SPEED_TYPE_400G_E, "400G"},
        {MPD_SPEED_TYPE_ALL_E, "all"},
    };
    #define MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS 5
    static MPD_ENUM_STR2VALUE_STC mpdOpConnectedTypeEnumStr2Value[] = {
        {MPD_OP_CONNECTED_TYPE_FIBER_E, "fiber"},
        {MPD_OP_CONNECTED_TYPE_DAC_E, "dac"},
        {MPD_OP_CONNECTED_TYPE_RJ45_SFP_E, "rj45_sfp"},
        {MPD_OP_CONNECTED_TYPE_UNKNOWN_E, "unknown"},
        {MPD_OP_CONNECTED_TYPE_ALL_E, "all"},
    };
    #define MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS 3
    static MPD_ENUM_STR2VALUE_STC mpdHostLineSideTypeEnumStr2Value[] = {
        {MPD_HOST_LINE_SIDE_TYPE_HOST_E, "host"},
        {MPD_HOST_LINE_SIDE_TYPE_LINE_E, "line"},
        {MPD_HOST_LINE_SIDE_TYPE_ALL_E, "all"},
    };
    #define MPD_PHY_ENUM_TYPE_ENUM_STR2VALUE_CNS 20
    static MPD_ENUM_STR2VALUE_STC mpdPhyEnumTypeEnumStr2Value[] = {
        {MPD_PHY_ENUM_TYPE_NO_PHY_DIRECT_ATTACHED_FIBER_E, "no-phy-direct-attached-fiber"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1543_E, "alaska-88E1543"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1545_E, "alaska-88E1545"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1548_E, "alaska-88E1548"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1680_E, "alaska-88E1680"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1680L_E, "alaska-88E1680L"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E151X_E, "alaska-88E151X"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E3680_E, "alaska-88E3680"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E32X0_E, "alaska-88E32X0"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E33X0_E, "alaska-88E33X0"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E20X0_E, "alaska-88E20X0"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E2180_E, "alaska-88E2180"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E2540_E, "alaska-88E2540"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88X3540_E, "alaska-88X3540"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1780_E, "alaska-88E1780"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E2580_E, "alaska-88E2580"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88X3580_E, "alaska-88X3580"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1112_E, "alaska-88E1112"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1781_E, "alaska-88E1781"},
        {MPD_PHY_ENUM_TYPE_ALASKA_88E1781_INTERNAL_E, "alaska-88E1781-internal"},
    };
    #define MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENUM_STR2VALUE_CNS 2
    static MPD_ENUM_STR2VALUE_STC mpdPhyFirmwareDownloadMethodTypeEnumStr2Value[] = {
        {MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_RAM_E, "RAM"},
        {MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_FLASH_E, "Flash"},
    };
    #define MPD_USX_MODE_TYPE_ENUM_STR2VALUE_CNS 7
    static MPD_ENUM_STR2VALUE_STC mpdUsxModeTypeEnumStr2Value[] = {
        {MPD_USX_MODE_TYPE_NONE_E, "None"},
        {MPD_USX_MODE_TYPE_SXGMII_E, "SXGMII"},
        {MPD_USX_MODE_TYPE_10G_DXGMII_E, "10G_DXGMII"},
        {MPD_USX_MODE_TYPE_20G_DXGMII_E, "20G_DXGMII"},
        {MPD_USX_MODE_TYPE_10G_QXGMII_E, "10G_QXGMII"},
        {MPD_USX_MODE_TYPE_20G_QXGMII_E, "20G_QXGMII"},
        {MPD_USX_MODE_TYPE_OXGMII_E, "OXGMII"},
    };
    #define MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS 3
    static MPD_ENUM_STR2VALUE_STC mpdTransceiverEnumTypeEnumStr2Value[] = {
        {MPD_TRANSCEIVER_ENUM_TYPE_SFP_E, "sfp"},
        {MPD_TRANSCEIVER_ENUM_TYPE_RJ45_E, "RJ45"},
        {MPD_TRANSCEIVER_ENUM_TYPE_COMBO_E, "combo"},
    };
    #define MPD_LED_MODE_ENUM_TYPE_ENUM_STR2VALUE_CNS 2
    static MPD_ENUM_STR2VALUE_STC mpdLedModeEnumTypeEnumStr2Value[] = {
        {MPD_LED_MODE_ENUM_TYPE_SINGLE_E, "single"},
        {MPD_LED_MODE_ENUM_TYPE_MATRIX_E, "matrix"},
    };
    /* aggregate enum conversion structs */
    static MPD_ENUM_IDS_ENUM_STR2VALUE_STC mpdEnumIdsEnumStr2Value[] =     {
    {MPD_ENUM_ID_SPEED_TYPE_E, &mpdSpeedTypeEnumStr2Value[0], MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_OP_CONNECTED_TYPE_E, &mpdOpConnectedTypeEnumStr2Value[0], MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_HOST_LINE_SIDE_TYPE_E, &mpdHostLineSideTypeEnumStr2Value[0], MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_PHY_ENUM_TYPE_E, &mpdPhyEnumTypeEnumStr2Value[0], MPD_PHY_ENUM_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_E, &mpdPhyFirmwareDownloadMethodTypeEnumStr2Value[0], MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_USX_MODE_TYPE_E, &mpdUsxModeTypeEnumStr2Value[0], MPD_USX_MODE_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_TRANSCEIVER_ENUM_TYPE_E, &mpdTransceiverEnumTypeEnumStr2Value[0], MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS},
    {MPD_ENUM_ID_LED_MODE_ENUM_TYPE_E, &mpdLedModeEnumTypeEnumStr2Value[0], MPD_LED_MODE_ENUM_TYPE_ENUM_STR2VALUE_CNS},
    }
;
    /* dynamic definition for array of data */
    static MPD_FEATURE_DATA_STC prvMpdFeaturesData[MPD_FEATURE_ID_LAST_E];
    /* Feature data Array FILLING */
    /* function definition */
    extern MPD_RESULT_ENT prvMpdFeaturesDataHandler (void)
    {
        prvMpdFeaturesData[MPD_FEATURE_ID_MPD_E].feature_id = MPD_FEATURE_ID_MPD_E;
        prvMpdFeaturesData[MPD_FEATURE_ID_MPD_E].is_supported = prvMpdMpdPlatform.mpdFeatureIsSupported;
        prvMpdFeaturesData[MPD_FEATURE_ID_MPD_E].data_PTR = (MPD_FEATURE_DATA_PARAMS_UNT*)&prvMpdMpdPlatform.mpd;
        prvMpdFeaturesData[MPD_FEATURE_ID_MPD_E].data_size = sizeof(prvMpdMpdPlatform.mpd);
        prvMpdFeaturesData[MPD_FEATURE_ID_MPD_E].is_supported_mask = prvMpdMpdPlatform.mpdFeatureIsSupported_mask;

        return MPD_OK_E;
    }
    /* CHOICE function definition */
    /* CHOICE destroy function definition */
    /* function definition */
    extern MPD_RESULT_ENT prvMpdCodeParser (XML_PARSER_NODE_DESCRIPTOR_TYP xml_root)
    {
        /* define generic/temp variables */
        UINT_32 max_size, uint_value;
        UINT_32 i;
        char string_value[PDL_XML_MAX_TAG_LEN + 1];
        
        /* define xml handles and statuses */
        XML_PARSER_RET_CODE_TYP xml_api_status[13];
        XML_PARSER_NODE_DESCRIPTOR_TYP xml_handle[13];
        void * list_entry_handle_PTR[4] = {0};
        void * list_entry_handle_PTR_PTR[4] = {0};
        PRV_PDLIB_DB_TYP list_db_handle_PTR[4] = {0};
        PRV_PDLIB_DB_ATTRIBUTES_STC db_attr;
        MPD_RESULT_ENT sysconf_status;
        UINT_32 list_size[4];
        UINT_32 list_entry_count[4];
        
        (void)list_entry_handle_PTR[0];
        (void)list_entry_handle_PTR_PTR[0];
        (void)sysconf_status;
        /* set endian variable */
        prvMpdIsBe = is_big_endian();
        
        /* init data */
        memset(&prvMpdMpdPlatform, 0, sizeof(prvMpdMpdPlatform));
        /* get root */
        xml_handle[0] = XML_PARSER_ROOT2NODE_CONVERT_MAC(xml_root);
        /* find data node */
        xml_api_status[0] = xmlParserFindChildByName(xml_handle[0], (char *)"data", &xml_handle[1]);
        /* if start */
        if (xml_api_status[0] != XML_PARSER_RET_CODE_OK)
        {
            return MPD_NOT_FOUND_E;
        }
        /* if end */
        xml_handle[0] = xml_handle[1];
        
        /* actual parser code */
        (void)i;
        xml_api_status[0] = xmlParserFindChildByName(xml_handle[0], (char *)"MPD-Platform", &xml_handle[1]);
        /* HANDLE STRUCT */
        if (xml_api_status[0] == XML_PARSER_RET_CODE_OK)
        { /* if start 2 */
            xml_api_status[1] = xmlParserFindChildByName(xml_handle[1], (char *)"Mpd-feature-is-supported", &xml_handle[2]);
            /* if start */
            if (xml_api_status[1] == XML_PARSER_RET_CODE_OK)
            {
                /* HANDLE SIMPLE INTEGER TYPE */
                max_size = sizeof(BOOLEAN);
                uint_value = 0;
                xml_api_status[2] = xmlParserGetValue(xml_handle[2], &max_size, &uint_value);
                /* if start */
                if (xml_api_status[2] == XML_PARSER_RET_CODE_OK)
                {
                    /* if start */
                    if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                    {
                        /* HANDLE ENDIANESS */
                        BOOLEAN x;
                        x  = (BOOLEAN)swap_32(uint_value);
                        uint_value = (UINT_32)x;
                    }
                    /* if end */
                    /* HANDLE SIMPLE INTEGER TYPE */
                    prvMpdMpdPlatform.mpdFeatureIsSupported = (BOOLEAN)uint_value;
                    prvMpdMpdPlatform.mpdFeatureIsSupported_mask |= MPD_FIELD_HAS_VALUE_CNS;
                }
                /* if end */
            }
            /* if end */
            xml_api_status[1] = xmlParserFindChildByName(xml_handle[1], (char *)"MPD", &xml_handle[2]);
            /* HANDLE STRUCT */
            if (xml_api_status[1] == XML_PARSER_RET_CODE_OK)
            { /* if start 1 */
                xml_api_status[2] = xmlParserFindChildByName(xml_handle[2], (char *)"automatic-parsing", &xml_handle[3]);
                /* if start */
                if (xml_api_status[2] == XML_PARSER_RET_CODE_OK)
                {
                    /* HANDLE SIMPLE INTEGER TYPE */
                    max_size = sizeof(BOOLEAN);
                    uint_value = 0;
                    xml_api_status[3] = xmlParserGetValue(xml_handle[3], &max_size, &uint_value);
                    /* if start */
                    if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                    {
                        /* if start */
                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                        {
                            /* HANDLE ENDIANESS */
                            BOOLEAN x;
                            x  = (BOOLEAN)swap_32(uint_value);
                            uint_value = (UINT_32)x;
                        }
                        /* if end */
                        /* HANDLE SIMPLE INTEGER TYPE */
                        prvMpdMpdPlatform.mpd.automaticParsing = (BOOLEAN)uint_value;
                        prvMpdMpdPlatform.mpd.automaticParsing_mask |= MPD_FIELD_HAS_VALUE_CNS;
                    }
                    /* if end */
                }
                /* if end */
                xml_api_status[2] = xmlParserFindChildByName(xml_handle[2], (char *)"disable-on-init", &xml_handle[3]);
                /* if start */
                if (xml_api_status[2] == XML_PARSER_RET_CODE_OK)
                {
                    /* HANDLE SIMPLE INTEGER TYPE */
                    max_size = sizeof(BOOLEAN);
                    uint_value = 0;
                    xml_api_status[3] = xmlParserGetValue(xml_handle[3], &max_size, &uint_value);
                    /* if start */
                    if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                    {
                        /* if start */
                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                        {
                            /* HANDLE ENDIANESS */
                            BOOLEAN x;
                            x  = (BOOLEAN)swap_32(uint_value);
                            uint_value = (UINT_32)x;
                        }
                        /* if end */
                        /* HANDLE SIMPLE INTEGER TYPE */
                        prvMpdMpdPlatform.mpd.disableOnInit = (BOOLEAN)uint_value;
                        prvMpdMpdPlatform.mpd.disableOnInit_mask |= MPD_FIELD_HAS_VALUE_CNS;
                    }
                    /* if end */
                }
                /* if end */
                xml_api_status[2] = xmlParserFindChildByName(xml_handle[2], (char *)"phy-list", &xml_handle[3]);
                /* HANDLE STRUCT */
                if (xml_api_status[2] == XML_PARSER_RET_CODE_OK)
                { /* if start 2 */
                    /* LIST STRUCT CODE START */
                    xml_api_status[2] = xmlParserGetTagCount(xml_handle[2], (char *)"phy-list", &list_size[0]);
                    /* if start */
                    if ((xml_api_status[2] != XML_PARSER_RET_CODE_OK) || (list_size[0] == 0))
                    {
                        return MPD_OP_FAILED_E;
                    }
                    /* if end */
                    list_entry_handle_PTR[0] = prvPdlibOsMalloc(sizeof(MPD_PHY_LIST_PARAMS_STC));
                    /* if start */
                    if (list_entry_handle_PTR[0] == NULL)
                    {
                        return MPD_OUT_OF_CPU_MEM_E;
                    }
                    /* if end */
                    db_attr.listAttributes.keySize = sizeof(MPD_PHY_LIST_KEYS_STC);
                    db_attr.listAttributes.entrySize = sizeof(MPD_PHY_LIST_PARAMS_STC);
                    sysconf_status = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &db_attr, &list_db_handle_PTR[0]);
                    /* if start */
                    if (sysconf_status != MPD_OK_E)
                    {
                        return MPD_OP_FAILED_E;
                    }
                    /* if end */
                    prvMpdMpdPlatform.mpd.phyList_PTR = list_db_handle_PTR[0];
                    list_entry_count[0] = 0;
                    while (xml_api_status[2] == XML_PARSER_RET_CODE_OK)
                    {
                        memset(list_entry_handle_PTR[0], 0, sizeof(MPD_PHY_LIST_PARAMS_STC));
                        
                        xml_api_status[3] = xmlParserIsEqualName(xml_handle[3], (char *)"phy-list");
                        /* if start */
                        if (xml_api_status[3] != XML_PARSER_RET_CODE_OK)
                        {
                            break;
                        }
                        /* if end */
                        list_entry_count[0] ++;
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"phy-number", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE TYPE USING SIMPLE TYPE */
                            /* HANDLE NON-SIMPLE INTEGER TYPE */
                            max_size = sizeof(MPD_PHY_NUMBER_TYPE_TYP);
                            uint_value = 0;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &max_size, &uint_value);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* if start */
                                if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                {
                                    /* HANDLE ENDIANESS */
                                    MPD_PHY_NUMBER_TYPE_TYP x;
                                    x  = (MPD_PHY_NUMBER_TYPE_TYP)swap_32(uint_value);
                                    uint_value = (UINT_32)x;
                                }
                                /* if end */
                                /* Handle integer range */
                                /* if start */
                                if (uint_value > MPD_PHY_NUMBER_TYPE_MAX_VALUE_CNS)
                                {
                                    return MPD_OUT_OF_RANGE_E;
                                }
                                /* if end */
                                /* HANDLE TYPE USING SIMPLE TYPE */
                                /* HANDLE NON-SIMPLE INTEGER TYPE */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).list_keys.phyNumber = (MPD_PHY_NUMBER_TYPE_TYP)uint_value;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"phy-type", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE ENUM */
                            uint_value = PDL_XML_MAX_TAG_LEN;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &uint_value, &string_value[0]);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* handle enum string conversion */
                                /* for loop start */
                                for (i = 0;
                                     i < MPD_PHY_ENUM_TYPE_ENUM_STR2VALUE_CNS;
                                     i++)
                                {
                                    /* if compare string */
                                    /* if start */
                                    if (strcmp(string_value, mpdPhyEnumTypeEnumStr2Value[i].str_value) == 0)
                                    {
                                        uint_value = mpdPhyEnumTypeEnumStr2Value[i].int_value;
                                        break;
                                    }
                                    /* if end */
                                }
                                /* for loop end */
                                /* check value was found if */
                                /* if start */
                                if (i == MPD_PHY_ENUM_TYPE_ENUM_STR2VALUE_CNS)
                                {
                                    return MPD_NOT_FOUND_E;
                                }
                                /* if end */
                                /* HANDLE ENUM */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).phyType = (MPD_PHY_ENUM_TYPE_ENT)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).phyType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"is-oob", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE SIMPLE INTEGER TYPE */
                            max_size = sizeof(BOOLEAN);
                            uint_value = 0;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &max_size, &uint_value);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* if start */
                                if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                {
                                    /* HANDLE ENDIANESS */
                                    BOOLEAN x;
                                    x  = (BOOLEAN)swap_32(uint_value);
                                    uint_value = (UINT_32)x;
                                }
                                /* if end */
                                /* HANDLE SIMPLE INTEGER TYPE */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).isOob = (BOOLEAN)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).isOob_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"oob-device", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE SIMPLE INTEGER TYPE */
                            max_size = sizeof(UINT_8);
                            uint_value = 0;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &max_size, &uint_value);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* if start */
                                if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                {
                                    /* HANDLE ENDIANESS */
                                    UINT_8 x;
                                    x  = (UINT_8)swap_32(uint_value);
                                    uint_value = (UINT_32)x;
                                }
                                /* if end */
                                /* HANDLE SIMPLE INTEGER TYPE */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).oobDevice = (UINT_8)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).oobDevice_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"led-mode", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE ENUM */
                            uint_value = PDL_XML_MAX_TAG_LEN;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &uint_value, &string_value[0]);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* handle enum string conversion */
                                /* for loop start */
                                for (i = 0;
                                     i < MPD_LED_MODE_ENUM_TYPE_ENUM_STR2VALUE_CNS;
                                     i++)
                                {
                                    /* if compare string */
                                    /* if start */
                                    if (strcmp(string_value, mpdLedModeEnumTypeEnumStr2Value[i].str_value) == 0)
                                    {
                                        uint_value = mpdLedModeEnumTypeEnumStr2Value[i].int_value;
                                        break;
                                    }
                                    /* if end */
                                }
                                /* for loop end */
                                /* check value was found if */
                                /* if start */
                                if (i == MPD_LED_MODE_ENUM_TYPE_ENUM_STR2VALUE_CNS)
                                {
                                    return MPD_NOT_FOUND_E;
                                }
                                /* if end */
                                /* HANDLE ENUM */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).ledMode = (MPD_LED_MODE_ENUM_TYPE_ENT)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).ledMode_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"usx-mode", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE ENUM */
                            uint_value = PDL_XML_MAX_TAG_LEN;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &uint_value, &string_value[0]);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* handle enum string conversion */
                                /* for loop start */
                                for (i = 0;
                                     i < MPD_USX_MODE_TYPE_ENUM_STR2VALUE_CNS;
                                     i++)
                                {
                                    /* if compare string */
                                    /* if start */
                                    if (strcmp(string_value, mpdUsxModeTypeEnumStr2Value[i].str_value) == 0)
                                    {
                                        uint_value = mpdUsxModeTypeEnumStr2Value[i].int_value;
                                        break;
                                    }
                                    /* if end */
                                }
                                /* for loop end */
                                /* check value was found if */
                                /* if start */
                                if (i == MPD_USX_MODE_TYPE_ENUM_STR2VALUE_CNS)
                                {
                                    return MPD_NOT_FOUND_E;
                                }
                                /* if end */
                                /* HANDLE ENUM */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).usxMode = (MPD_USX_MODE_TYPE_ENT)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).usxMode_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"phy-firmware-download-method", &xml_handle[4]);
                        /* if start */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        {
                            /* HANDLE ENUM */
                            uint_value = PDL_XML_MAX_TAG_LEN;
                            xml_api_status[4] = xmlParserGetValue(xml_handle[4], &uint_value, &string_value[0]);
                            /* if start */
                            if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                            {
                                /* handle enum string conversion */
                                /* for loop start */
                                for (i = 0;
                                     i < MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENUM_STR2VALUE_CNS;
                                     i++)
                                {
                                    /* if compare string */
                                    /* if start */
                                    if (strcmp(string_value, mpdPhyFirmwareDownloadMethodTypeEnumStr2Value[i].str_value) == 0)
                                    {
                                        uint_value = mpdPhyFirmwareDownloadMethodTypeEnumStr2Value[i].int_value;
                                        break;
                                    }
                                    /* if end */
                                }
                                /* for loop end */
                                /* check value was found if */
                                /* if start */
                                if (i == MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENUM_STR2VALUE_CNS)
                                {
                                    return MPD_NOT_FOUND_E;
                                }
                                /* if end */
                                /* HANDLE ENUM */
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).phyFirmwareDownloadMethod = (MPD_PHY_FIRMWARE_DOWNLOAD_METHOD_TYPE_ENT)uint_value;
                                (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).phyFirmwareDownloadMethod_mask |= MPD_FIELD_HAS_VALUE_CNS;
                            }
                            /* if end */
                        }
                        /* if end */
                        xml_api_status[3] = xmlParserFindChildByName(xml_handle[3], (char *)"port-list", &xml_handle[4]);
                        /* HANDLE STRUCT */
                        if (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                        { /* if start 3 */
                            /* LIST STRUCT CODE START */
                            xml_api_status[3] = xmlParserGetTagCount(xml_handle[3], (char *)"port-list", &list_size[1]);
                            /* if start */
                            if ((xml_api_status[3] != XML_PARSER_RET_CODE_OK) || (list_size[1] == 0))
                            {
                                return MPD_OP_FAILED_E;
                            }
                            /* if end */
                            list_entry_handle_PTR[1] = prvPdlibOsMalloc(sizeof(MPD_PORT_LIST_PARAMS_STC));
                            /* if start */
                            if (list_entry_handle_PTR[1] == NULL)
                            {
                                return MPD_OUT_OF_CPU_MEM_E;
                            }
                            /* if end */
                            db_attr.listAttributes.keySize = sizeof(MPD_PORT_LIST_KEYS_STC);
                            db_attr.listAttributes.entrySize = sizeof(MPD_PORT_LIST_PARAMS_STC);
                            sysconf_status = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &db_attr, &list_db_handle_PTR[1]);
                            /* if start */
                            if (sysconf_status != MPD_OK_E)
                            {
                                return MPD_OP_FAILED_E;
                            }
                            /* if end */
                            (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).portList_PTR = list_db_handle_PTR[1];
                            list_entry_count[1] = 0;
                            while (xml_api_status[3] == XML_PARSER_RET_CODE_OK)
                            {
                                memset(list_entry_handle_PTR[1], 0, sizeof(MPD_PORT_LIST_PARAMS_STC));
                                
                                xml_api_status[4] = xmlParserIsEqualName(xml_handle[4], (char *)"port-list");
                                /* if start */
                                if (xml_api_status[4] != XML_PARSER_RET_CODE_OK)
                                {
                                    break;
                                }
                                /* if end */
                                list_entry_count[1] ++;
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"logical-port", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE SIMPLE INTEGER TYPE */
                                    max_size = sizeof(UINT_8);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            UINT_8 x;
                                            x  = (UINT_8)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).list_keys.logicalPort = (UINT_8)uint_value;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"mdio-address", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE TYPE USING SIMPLE TYPE */
                                    /* HANDLE NON-SIMPLE INTEGER TYPE */
                                    max_size = sizeof(MPD_MDIO_ADDRESS_TYPE_TYP);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            MPD_MDIO_ADDRESS_TYPE_TYP x;
                                            x  = (MPD_MDIO_ADDRESS_TYPE_TYP)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* Handle integer range */
                                        /* if start */
                                        if (uint_value > MPD_MDIO_ADDRESS_TYPE_MAX_VALUE_CNS)
                                        {
                                            return MPD_OUT_OF_RANGE_E;
                                        }
                                        /* if end */
                                        /* HANDLE TYPE USING SIMPLE TYPE */
                                        /* HANDLE NON-SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioAddress = (MPD_MDIO_ADDRESS_TYPE_TYP)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioAddress_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"mdio-device", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE SIMPLE INTEGER TYPE */
                                    max_size = sizeof(UINT_8);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            UINT_8 x;
                                            x  = (UINT_8)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioDevice = (UINT_8)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioDevice_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"mdio-bus", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE SIMPLE INTEGER TYPE */
                                    max_size = sizeof(UINT_8);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            UINT_8 x;
                                            x  = (UINT_8)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioBus = (UINT_8)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mdioBus_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"oob-address", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE TYPE USING SIMPLE TYPE */
                                    /* HANDLE NON-SIMPLE INTEGER TYPE */
                                    max_size = sizeof(MPD_MDIO_ADDRESS_TYPE_TYP);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            MPD_MDIO_ADDRESS_TYPE_TYP x;
                                            x  = (MPD_MDIO_ADDRESS_TYPE_TYP)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* Handle integer range */
                                        /* if start */
                                        if (uint_value > MPD_MDIO_ADDRESS_TYPE_MAX_VALUE_CNS)
                                        {
                                            return MPD_OUT_OF_RANGE_E;
                                        }
                                        /* if end */
                                        /* HANDLE TYPE USING SIMPLE TYPE */
                                        /* HANDLE NON-SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).oobAddress = (MPD_MDIO_ADDRESS_TYPE_TYP)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).oobAddress_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"invert-mdi", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE SIMPLE INTEGER TYPE */
                                    max_size = sizeof(BOOLEAN);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* if start */
                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                        {
                                            /* HANDLE ENDIANESS */
                                            BOOLEAN x;
                                            x  = (BOOLEAN)swap_32(uint_value);
                                            uint_value = (UINT_32)x;
                                        }
                                        /* if end */
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).invertMdi = (BOOLEAN)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).invertMdi_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"vct-offset", &xml_handle[5]);
                                /* if start */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                {
                                    /* HANDLE SIMPLE INTEGER TYPE */
                                    max_size = sizeof(UINT_32);
                                    uint_value = 0;
                                    xml_api_status[5] = xmlParserGetValue(xml_handle[5], &max_size, &uint_value);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).vctOffset = (UINT_32)uint_value;
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).vctOffset_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    }
                                    /* if end */
                                }
                                /* if end */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"mtd-specific-information-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"do-swap-abcd", &xml_handle[6]);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        max_size = sizeof(BOOLEAN);
                                        uint_value = 0;
                                        xml_api_status[6] = xmlParserGetValue(xml_handle[6], &max_size, &uint_value);
                                        /* if start */
                                        if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                        {
                                            /* if start */
                                            if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                            {
                                                /* HANDLE ENDIANESS */
                                                BOOLEAN x;
                                                x  = (BOOLEAN)swap_32(uint_value);
                                                uint_value = (UINT_32)x;
                                            }
                                            /* if end */
                                            /* HANDLE SIMPLE INTEGER TYPE */
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mtdSpecificInformationType.doSwapAbcd = (BOOLEAN)uint_value;
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mtdSpecificInformationType.doSwapAbcd_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                        }
                                        /* if end */
                                    }
                                    /* if end */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"short-reach-serdes", &xml_handle[6]);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE SIMPLE INTEGER TYPE */
                                        max_size = sizeof(BOOLEAN);
                                        uint_value = 0;
                                        xml_api_status[6] = xmlParserGetValue(xml_handle[6], &max_size, &uint_value);
                                        /* if start */
                                        if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                        {
                                            /* if start */
                                            if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                            {
                                                /* HANDLE ENDIANESS */
                                                BOOLEAN x;
                                                x  = (BOOLEAN)swap_32(uint_value);
                                                uint_value = (UINT_32)x;
                                            }
                                            /* if end */
                                            /* HANDLE SIMPLE INTEGER TYPE */
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mtdSpecificInformationType.shortReachSerdes = (BOOLEAN)uint_value;
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mtdSpecificInformationType.shortReachSerdes_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                        }
                                        /* if end */
                                    }
                                    /* if end */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).mtdSpecificInformationType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"combo-port-information-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"transceiver-type", &xml_handle[6]);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE ENUM */
                                        uint_value = PDL_XML_MAX_TAG_LEN;
                                        xml_api_status[6] = xmlParserGetValue(xml_handle[6], &uint_value, &string_value[0]);
                                        /* if start */
                                        if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                        {
                                            /* handle enum string conversion */
                                            /* for loop start */
                                            for (i = 0;
                                                 i < MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS;
                                                 i++)
                                            {
                                                /* if compare string */
                                                /* if start */
                                                if (strcmp(string_value, mpdTransceiverEnumTypeEnumStr2Value[i].str_value) == 0)
                                                {
                                                    uint_value = mpdTransceiverEnumTypeEnumStr2Value[i].int_value;
                                                    break;
                                                }
                                                /* if end */
                                            }
                                            /* for loop end */
                                            /* check value was found if */
                                            /* if start */
                                            if (i == MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS)
                                            {
                                                return MPD_NOT_FOUND_E;
                                            }
                                            /* if end */
                                            /* HANDLE ENUM */
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comboPortInformationType.transceiverType = (MPD_TRANSCEIVER_ENUM_TYPE_ENT)uint_value;
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comboPortInformationType.transceiverType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                        }
                                        /* if end */
                                    }
                                    /* if end */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comboPortInformationType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"fiber-port-information-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"transceiver-type", &xml_handle[6]);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE ENUM */
                                        uint_value = PDL_XML_MAX_TAG_LEN;
                                        xml_api_status[6] = xmlParserGetValue(xml_handle[6], &uint_value, &string_value[0]);
                                        /* if start */
                                        if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                        {
                                            /* handle enum string conversion */
                                            /* for loop start */
                                            for (i = 0;
                                                 i < MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS;
                                                 i++)
                                            {
                                                /* if compare string */
                                                /* if start */
                                                if (strcmp(string_value, mpdTransceiverEnumTypeEnumStr2Value[i].str_value) == 0)
                                                {
                                                    uint_value = mpdTransceiverEnumTypeEnumStr2Value[i].int_value;
                                                    break;
                                                }
                                                /* if end */
                                            }
                                            /* for loop end */
                                            /* check value was found if */
                                            /* if start */
                                            if (i == MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS)
                                            {
                                                return MPD_NOT_FOUND_E;
                                            }
                                            /* if end */
                                            /* HANDLE ENUM */
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).fiberPortInformationType.transceiverType = (MPD_TRANSCEIVER_ENUM_TYPE_ENT)uint_value;
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).fiberPortInformationType.transceiverType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                        }
                                        /* if end */
                                    }
                                    /* if end */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).fiberPortInformationType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"rj45-port-information-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"transceiver-type", &xml_handle[6]);
                                    /* if start */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    {
                                        /* HANDLE ENUM */
                                        uint_value = PDL_XML_MAX_TAG_LEN;
                                        xml_api_status[6] = xmlParserGetValue(xml_handle[6], &uint_value, &string_value[0]);
                                        /* if start */
                                        if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                        {
                                            /* handle enum string conversion */
                                            /* for loop start */
                                            for (i = 0;
                                                 i < MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS;
                                                 i++)
                                            {
                                                /* if compare string */
                                                /* if start */
                                                if (strcmp(string_value, mpdTransceiverEnumTypeEnumStr2Value[i].str_value) == 0)
                                                {
                                                    uint_value = mpdTransceiverEnumTypeEnumStr2Value[i].int_value;
                                                    break;
                                                }
                                                /* if end */
                                            }
                                            /* for loop end */
                                            /* check value was found if */
                                            /* if start */
                                            if (i == MPD_TRANSCEIVER_ENUM_TYPE_ENUM_STR2VALUE_CNS)
                                            {
                                                return MPD_NOT_FOUND_E;
                                            }
                                            /* if end */
                                            /* HANDLE ENUM */
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).rj45PortInformationType.transceiverType = (MPD_TRANSCEIVER_ENUM_TYPE_ENT)uint_value;
                                            (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).rj45PortInformationType.transceiverType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                        }
                                        /* if end */
                                    }
                                    /* if end */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).rj45PortInformationType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"comphy-tx-params-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"comphy-tx-params-list", &xml_handle[6]);
                                    /* HANDLE STRUCT */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    { /* if start 5 */
                                        /* LIST STRUCT CODE START */
                                        xml_api_status[5] = xmlParserGetTagCount(xml_handle[5], (char *)"comphy-tx-params-list", &list_size[2]);
                                        /* if start */
                                        if ((xml_api_status[5] != XML_PARSER_RET_CODE_OK) || (list_size[2] == 0))
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        list_entry_handle_PTR[2] = prvPdlibOsMalloc(sizeof(MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC));
                                        /* if start */
                                        if (list_entry_handle_PTR[2] == NULL)
                                        {
                                            return MPD_OUT_OF_CPU_MEM_E;
                                        }
                                        /* if end */
                                        db_attr.listAttributes.keySize = sizeof(MPD_COMPHY_TX_PARAMS_LIST_KEYS_STC);
                                        db_attr.listAttributes.entrySize = sizeof(MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC);
                                        sysconf_status = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &db_attr, &list_db_handle_PTR[2]);
                                        /* if start */
                                        if (sysconf_status != MPD_OK_E)
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyTxParamsType.comphyTxParamsList_PTR = list_db_handle_PTR[2];
                                        list_entry_count[2] = 0;
                                        while (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                        {
                                            memset(list_entry_handle_PTR[2], 0, sizeof(MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC));
                                            
                                            xml_api_status[6] = xmlParserIsEqualName(xml_handle[6], (char *)"comphy-tx-params-list");
                                            /* if start */
                                            if (xml_api_status[6] != XML_PARSER_RET_CODE_OK)
                                            {
                                                break;
                                            }
                                            /* if end */
                                            list_entry_count[2] ++;
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"speed", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdSpeedTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdSpeedTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.speed = (MPD_SPEED_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"op-connected", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdOpConnectedTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdOpConnectedTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.opConnected = (MPD_OP_CONNECTED_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"side", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdHostLineSideTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdHostLineSideTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.side = (MPD_HOST_LINE_SIDE_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"comphy-tx-params", &xml_handle[7]);
                                            /* HANDLE STRUCT */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            { /* if start 6 */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"preTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.pretap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.pretap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"mainTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.maintap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.maintap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"postTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.posttap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams.posttap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                (* (MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).comphyTxParams_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                            } /* if end 6 */
                                            /* LIST STRUCT CODE STOP */
                                            /* (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyTxParamsType.comphyTxParamsList_PTR = list_db_handle_PTR[2]; */
                                            sysconf_status = prvPdlibDbAdd(list_db_handle_PTR[2], (void *)&(((MPD_COMPHY_TX_PARAMS_LIST_PARAMS_STC*)list_entry_handle_PTR[2])->list_keys), (void *)list_entry_handle_PTR[2], (void **)&list_entry_handle_PTR_PTR[2]);
                                            /* if start */
                                            if (sysconf_status != MPD_OK_E)
                                            {
                                                return MPD_OP_FAILED_E;
                                            }
                                            /* if end */
                                            xml_api_status[5] = xmlParserGetNextSibling(xml_handle[6], & xml_handle[6]);
                                        }
                                        prvPdlibOsFree(list_entry_handle_PTR[2]);
                                        /* if start */
                                        if (list_size[2] != list_entry_count[2])
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyTxParamsType.comphyTxParamsList_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    } /* if end 5 */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyTxParamsType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                xml_api_status[4] = xmlParserFindChildByName(xml_handle[4], (char *)"comphy-voltron-tx-params-type", &xml_handle[5]);
                                /* HANDLE STRUCT */
                                if (xml_api_status[4] == XML_PARSER_RET_CODE_OK)
                                { /* if start 4 */
                                    xml_api_status[5] = xmlParserFindChildByName(xml_handle[5], (char *)"comphy-voltron-tx-params-list", &xml_handle[6]);
                                    /* HANDLE STRUCT */
                                    if (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                    { /* if start 5 */
                                        /* LIST STRUCT CODE START */
                                        xml_api_status[5] = xmlParserGetTagCount(xml_handle[5], (char *)"comphy-voltron-tx-params-list", &list_size[2]);
                                        /* if start */
                                        if ((xml_api_status[5] != XML_PARSER_RET_CODE_OK) || (list_size[2] == 0))
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        list_entry_handle_PTR[2] = prvPdlibOsMalloc(sizeof(MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC));
                                        /* if start */
                                        if (list_entry_handle_PTR[2] == NULL)
                                        {
                                            return MPD_OUT_OF_CPU_MEM_E;
                                        }
                                        /* if end */
                                        db_attr.listAttributes.keySize = sizeof(MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_KEYS_STC);
                                        db_attr.listAttributes.entrySize = sizeof(MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC);
                                        sysconf_status = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &db_attr, &list_db_handle_PTR[2]);
                                        /* if start */
                                        if (sysconf_status != MPD_OK_E)
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR = list_db_handle_PTR[2];
                                        list_entry_count[2] = 0;
                                        while (xml_api_status[5] == XML_PARSER_RET_CODE_OK)
                                        {
                                            memset(list_entry_handle_PTR[2], 0, sizeof(MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC));
                                            
                                            xml_api_status[6] = xmlParserIsEqualName(xml_handle[6], (char *)"comphy-voltron-tx-params-list");
                                            /* if start */
                                            if (xml_api_status[6] != XML_PARSER_RET_CODE_OK)
                                            {
                                                break;
                                            }
                                            /* if end */
                                            list_entry_count[2] ++;
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"speed", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdSpeedTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdSpeedTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_SPEED_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.speed = (MPD_SPEED_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"op-connected", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdOpConnectedTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdOpConnectedTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_OP_CONNECTED_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.opConnected = (MPD_OP_CONNECTED_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"side", &xml_handle[7]);
                                            /* if start */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            {
                                                /* HANDLE ENUM */
                                                uint_value = PDL_XML_MAX_TAG_LEN;
                                                xml_api_status[7] = xmlParserGetValue(xml_handle[7], &uint_value, &string_value[0]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* handle enum string conversion */
                                                    /* for loop start */
                                                    for (i = 0;
                                                         i < MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS;
                                                         i++)
                                                    {
                                                        /* if compare string */
                                                        /* if start */
                                                        if (strcmp(string_value, mpdHostLineSideTypeEnumStr2Value[i].str_value) == 0)
                                                        {
                                                            uint_value = mpdHostLineSideTypeEnumStr2Value[i].int_value;
                                                            break;
                                                        }
                                                        /* if end */
                                                    }
                                                    /* for loop end */
                                                    /* check value was found if */
                                                    /* if start */
                                                    if (i == MPD_HOST_LINE_SIDE_TYPE_ENUM_STR2VALUE_CNS)
                                                    {
                                                        return MPD_NOT_FOUND_E;
                                                    }
                                                    /* if end */
                                                    /* HANDLE ENUM */
                                                    (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).list_keys.side = (MPD_HOST_LINE_SIDE_TYPE_ENT)uint_value;
                                                }
                                                /* if end */
                                            }
                                            /* if end */
                                            xml_api_status[6] = xmlParserFindChildByName(xml_handle[6], (char *)"voltron-tx-params", &xml_handle[7]);
                                            /* HANDLE STRUCT */
                                            if (xml_api_status[6] == XML_PARSER_RET_CODE_OK)
                                            { /* if start 6 */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"preTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.pretap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.pretap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"peakTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.peaktap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.peaktap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"postTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.posttap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.posttap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                xml_api_status[7] = xmlParserFindChildByName(xml_handle[7], (char *)"marginTap", &xml_handle[8]);
                                                /* if start */
                                                if (xml_api_status[7] == XML_PARSER_RET_CODE_OK)
                                                {
                                                    /* HANDLE SIMPLE INTEGER TYPE */
                                                    max_size = sizeof(UINT_8);
                                                    uint_value = 0;
                                                    xml_api_status[8] = xmlParserGetValue(xml_handle[8], &max_size, &uint_value);
                                                    /* if start */
                                                    if (xml_api_status[8] == XML_PARSER_RET_CODE_OK)
                                                    {
                                                        /* if start */
                                                        if (prvMpdIsBe && (max_size < sizeof(UINT_32)))
                                                        {
                                                            /* HANDLE ENDIANESS */
                                                            UINT_8 x;
                                                            x  = (UINT_8)swap_32(uint_value);
                                                            uint_value = (UINT_32)x;
                                                        }
                                                        /* if end */
                                                        /* HANDLE SIMPLE INTEGER TYPE */
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.margintap = (UINT_8)uint_value;
                                                        (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams.margintap_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                                    }
                                                    /* if end */
                                                }
                                                /* if end */
                                                (* (MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC *)list_entry_handle_PTR[2]).voltronTxParams_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                            } /* if end 6 */
                                            /* LIST STRUCT CODE STOP */
                                            /* (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR = list_db_handle_PTR[2]; */
                                            sysconf_status = prvPdlibDbAdd(list_db_handle_PTR[2], (void *)&(((MPD_COMPHY_VOLTRON_TX_PARAMS_LIST_PARAMS_STC*)list_entry_handle_PTR[2])->list_keys), (void *)list_entry_handle_PTR[2], (void **)&list_entry_handle_PTR_PTR[2]);
                                            /* if start */
                                            if (sysconf_status != MPD_OK_E)
                                            {
                                                return MPD_OP_FAILED_E;
                                            }
                                            /* if end */
                                            xml_api_status[5] = xmlParserGetNextSibling(xml_handle[6], & xml_handle[6]);
                                        }
                                        prvPdlibOsFree(list_entry_handle_PTR[2]);
                                        /* if start */
                                        if (list_size[2] != list_entry_count[2])
                                        {
                                            return MPD_OP_FAILED_E;
                                        }
                                        /* if end */
                                        (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyVoltronTxParamsType.comphyVoltronTxParamsList_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                    } /* if end 5 */
                                    (* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR[1]).comphyVoltronTxParamsType_mask |= MPD_FIELD_HAS_VALUE_CNS;
                                } /* if end 4 */
                                /* LIST STRUCT CODE STOP */
                                /* (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).portList_PTR = list_db_handle_PTR[1]; */
                                sysconf_status = prvPdlibDbAdd(list_db_handle_PTR[1], (void *)&(((MPD_PORT_LIST_PARAMS_STC*)list_entry_handle_PTR[1])->list_keys), (void *)list_entry_handle_PTR[1], (void **)&list_entry_handle_PTR_PTR[1]);
                                /* if start */
                                if (sysconf_status != MPD_OK_E)
                                {
                                    return MPD_OP_FAILED_E;
                                }
                                /* if end */
                                xml_api_status[3] = xmlParserGetNextSibling(xml_handle[4], & xml_handle[4]);
                            }
                            prvPdlibOsFree(list_entry_handle_PTR[1]);
                            /* if start */
                            if (list_size[1] != list_entry_count[1])
                            {
                                return MPD_OP_FAILED_E;
                            }
                            /* if end */
                            (* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR[0]).portList_mask |= MPD_FIELD_HAS_VALUE_CNS;
                        } /* if end 3 */
                        /* LIST STRUCT CODE STOP */
                        /* prvMpdMpdPlatform.mpd.phyList_PTR = list_db_handle_PTR[0]; */
                        sysconf_status = prvPdlibDbAdd(list_db_handle_PTR[0], (void *)&(((MPD_PHY_LIST_PARAMS_STC*)list_entry_handle_PTR[0])->list_keys), (void *)list_entry_handle_PTR[0], (void **)&list_entry_handle_PTR_PTR[0]);
                        /* if start */
                        if (sysconf_status != MPD_OK_E)
                        {
                            return MPD_OP_FAILED_E;
                        }
                        /* if end */
                        xml_api_status[2] = xmlParserGetNextSibling(xml_handle[3], & xml_handle[3]);
                    }
                    prvPdlibOsFree(list_entry_handle_PTR[0]);
                    /* if start */
                    if (list_size[0] != list_entry_count[0])
                    {
                        return MPD_OP_FAILED_E;
                    }
                    /* if end */
                    prvMpdMpdPlatform.mpd.phyList_mask |= MPD_FIELD_HAS_VALUE_CNS;
                } /* if end 2 */
                prvMpdMpdPlatform.mpd_mask |= MPD_FIELD_HAS_VALUE_CNS;
            } /* if end 1 */
        } /* if end 2 */
        return MPD_OK_E;
    }
    /* function definition */
    extern MPD_RESULT_ENT prvMpdCodeDestroy (void)
    {
        /* define generic/temp variables */
        
        void * list_entry_handle_PTR_PTR[4] = {0};
        void * list_entry_next_handle_PTR_PTR[4] = {0};
        MPD_RESULT_ENT sysconf_status[4] = { MPD_OK_E };
        
        /* actual destroy code */
        (void)list_entry_handle_PTR_PTR[0];
        (void)list_entry_next_handle_PTR_PTR[0];
        (void)sysconf_status[0];
        sysconf_status[0] = prvPdlibDbGetFirst(prvMpdMpdPlatform.mpd.phyList_PTR, &list_entry_handle_PTR_PTR[0]);
        while (sysconf_status[0] == MPD_OK_E)
        {
            sysconf_status[0] = prvPdlibDbGetNext(prvMpdMpdPlatform.mpd.phyList_PTR, (void *)&((MPD_PHY_LIST_PARAMS_STC*)list_entry_handle_PTR_PTR[0])->list_keys, &list_entry_next_handle_PTR_PTR[0]);
            sysconf_status[1] = prvPdlibDbGetFirst((* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[0]).portList_PTR, &list_entry_handle_PTR_PTR[1]);
            while (sysconf_status[1] == MPD_OK_E)
            {
                sysconf_status[1] = prvPdlibDbGetNext((* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[0]).portList_PTR, (void *)&((MPD_PORT_LIST_PARAMS_STC*)list_entry_handle_PTR_PTR[1])->list_keys, &list_entry_next_handle_PTR_PTR[1]);
                /* if start */
                if ((* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[1]).comphyTxParamsType.comphyTxParamsList_PTR != NULL)
                {
                    prvPdlibDbDestroy((* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[1]).comphyTxParamsType.comphyTxParamsList_PTR);
                }
                /* if end */
                /* if start */
                if ((* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[1]).comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR != NULL)
                {
                    prvPdlibDbDestroy((* (MPD_PORT_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[1]).comphyVoltronTxParamsType.comphyVoltronTxParamsList_PTR);
                }
                /* if end */
                list_entry_handle_PTR_PTR[1] = list_entry_next_handle_PTR_PTR[1];
            }
            /* if start */
            if ((* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[0]).portList_PTR != NULL)
            {
                prvPdlibDbDestroy((* (MPD_PHY_LIST_PARAMS_STC *)list_entry_handle_PTR_PTR[0]).portList_PTR);
            }
            /* if end */
            list_entry_handle_PTR_PTR[0] = list_entry_next_handle_PTR_PTR[0];
        }
        /* if start */
        if (prvMpdMpdPlatform.mpd.phyList_PTR != NULL)
        {
            prvPdlibDbDestroy(prvMpdMpdPlatform.mpd.phyList_PTR);
        }
        /* if end */
        return MPD_OK_E;
    }
    /* Feature data Array get */
    /* function definition */
    extern MPD_RESULT_ENT prvMpdFeaturesDataGet (MPD_FEATURE_DATA_STC ** data_PTR)
    {
        /* if start */
        if (data_PTR == NULL)
        {
            return MPD_OP_FAILED_E;
        }
        /* if end */

        *data_PTR = &prvMpdFeaturesData[0];
        return MPD_OK_E;
    }
    extern MPD_RESULT_ENT mpdInternalDbGet( MPD_MPD_PLATFORM_PARAMS_STC ** data_PTR)
    {
        /* if start */
        if (data_PTR == NULL)
        {
            return MPD_OP_FAILED_E;
        }
        /* if end */

        *data_PTR = &prvMpdMpdPlatform;

        return MPD_OK_E;
    }
    extern MPD_RESULT_ENT mpdEnum2StrConvert( 
        MPD_ENUM_IDS_ENT enum_type,
        UINT_32 int_value,
        char ** str_value_PTR
    )
    {
        UINT_32 i,j;
        for (i = 0; i < sizeof(mpdEnumIdsEnumStr2Value)/sizeof(mpdEnumIdsEnumStr2Value[0]); i ++)
        {
            if (mpdEnumIdsEnumStr2Value[i].enum_id == enum_type)
            {
                for (j = 0; j < mpdEnumIdsEnumStr2Value[i].arr_size; j ++)
                {
                    if (mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j].int_value == int_value)
                    {
                        *str_value_PTR = (char *)mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j].str_value;
                        return MPD_OK_E;
                    }
                }
                return MPD_NOT_FOUND_E;
            }
        }
        return MPD_NOT_FOUND_E;
    }
    extern MPD_RESULT_ENT mpdStr2EnumConvert( 
        MPD_ENUM_IDS_ENT enum_type,
        char * str_value_PTR,
        UINT_32 * int_value_PTR
    )
    {
        UINT_32 i,j;
        for (i = 0; i < sizeof(mpdEnumIdsEnumStr2Value)/sizeof(mpdEnumIdsEnumStr2Value[0]); i ++)
        {
            if (mpdEnumIdsEnumStr2Value[i].enum_id == enum_type)
            {
                for (j = 0; j < mpdEnumIdsEnumStr2Value[i].arr_size; j ++)
                {
                    if (strcmp(mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j].str_value, str_value_PTR) == 0)
                    {
                        *int_value_PTR = mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j].int_value;
                        return MPD_OK_E;
                    }
                }
                return MPD_NOT_FOUND_E;
            }
        }
        return MPD_NOT_FOUND_E;
    }
    extern MPD_RESULT_ENT mpdEnum2StrEntryGet( 
        MPD_ENUM_IDS_ENT enum_type,
        UINT_32 int_value,
        MPD_ENUM_STR2VALUE_STC ** entry_PTR
    )
    {
        UINT_32 i,j;
        for (i = 0; i < sizeof(mpdEnumIdsEnumStr2Value)/sizeof(mpdEnumIdsEnumStr2Value[0]); i ++)
        {
            if (mpdEnumIdsEnumStr2Value[i].enum_id == enum_type)
            {
                for (j = 0; j < mpdEnumIdsEnumStr2Value[i].arr_size; j ++)
                {
                    if (mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j].int_value == int_value)
                    {
                        *entry_PTR = &mpdEnumIdsEnumStr2Value[i].convertArr_PTR[j];
                        return MPD_OK_E;
                    }
                }
            }
        }
        return MPD_NOT_FOUND_E;
    }
