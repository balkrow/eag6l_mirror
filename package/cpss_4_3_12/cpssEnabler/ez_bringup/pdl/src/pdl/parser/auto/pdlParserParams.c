/* -------------------------------------------------------------------------------------------------------
   Generated from MTS-Platform-Legacy-Yang-Module based on revision 2023-05-16:3.60 on 04/06/2023 16:40:45
   ------------------------------------------------------------------------------------------------------- */
#include <pdl/common/pdlTypes.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdlib/xml/private/prvXmlParserBuilder.h>
#include <pdl/parser/pdlParser.h>
#include <inf/sysconfinf/exp/sysconfinf.h>
#include <mngcnf/sysconf/main/inc/sysconf_prv.h>
    #undef PDL_CHECK_STATUS
    #define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlAutoParserDebugFlag)

    #ifdef RETURN_WRAPPER_MAC
    #undef RETURN_WRAPPER_MAC
    #endif
    #define RETURN_WRAPPER_MAC(_ret_code) PDL_CHECK_STATUS(_ret_code)

    /* HPRM STRUCT FILLING */
    /* function definition */
    extern PDL_STATUS pdlHprmHandler (void ** hash_db_PTR)
    {
        PDL_MTS_PLATFORM_PARAMS_STC * project_data_db_PTR;
        VHASHG_status_ENT hash_status;
        PDL_STATUS sysconf_status;
        PDL_FEATURE_HPRM_DATA_STC hash_entry;
        PDL_ENUM_STR2VALUE_STC * enum_str2value_entry_PTR;
        
        sysconf_status = pdlInternalDbGet ( &project_data_db_PTR);
        /* if start */
        if (sysconf_status != PDL_OK)
        {
            OSSYSG_fatal_error("SYSCONFG_hprm_handler - project profile fail to get project data DB");
        }
        /* if end */
        
        hash_status = VHASHG_new(NULL, TRUE, OSMEMG_type_perm_E, hash_db_PTR);
        /* if start */
        if (hash_status != VHASHG_status_ok_E)
        {
            OSSYSG_fatal_error("SYSCONFG_hprm_handler - project profile fail to create hprms DB");
        }
        /* if end */
        hash_status = VHASHG_set_sorting(*hash_db_PTR, VHASHG_sort_type_strcmp_E);
        /* if start */
        if (hash_status != VHASHG_status_ok_E)
        {
            OSSYSG_fatal_error("SYSCONFG_hprm_handler - project profile fail to set hprms DB sort type");
        }
        /* if end */
        hash_status = VHASHG_set_maximal_key_size(*hash_db_PTR, SYSCONFP_max_key_size_CNS);
        /* if start */
        if (hash_status != VHASHG_status_ok_E)
        {
            OSSYSG_fatal_error("SYSCONFG_hprm_handler - project profile fail to set hprms DB maximal key size");
        }
        /* if end */
        /* List is not supported - skip it */
        /* project_data_db_PTR->networkPorts.frontPanelGroupList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.gpioblinkratecontrol.ledGpiorateGroupType.ledGpioRateControlList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portsleds.portledsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.ledstreamleds.ledstreamportsleds.ledstreamportledsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.ledstreamleds.ledstreamInterfaceList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackledList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->buttons.buttonsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->mppGpioPins.mppDeviceList */
        /* Union is not supported - skip it */
        /* project_data_db_PTR->backToBackInformation */
        /* List is not supported - skip it */
        /* project_data_db_PTR->cpu.cpuSdmaPortMapping.cpuSdmaPortList */
        /* Union is not supported - skip it */
        /* project_data_db_PTR->cpu.cpuInfoType */
        /* List is not supported - skip it */
        /* project_data_db_PTR->serdes.packetProcessorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->sensors.sensorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->externalDriver.externalDriverList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pdportList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.poePowerBanks.poebankList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.pseports.pseList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.pseports.pseportList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->phys.phyList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.statusList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->fans.swFanControllers.thresholdList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->fans.fanControllerList */
        /* fill hash entry */
        if ((project_data_db_PTR->powerFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->powerFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "powerFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpuSdmaPortMappingIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpuSdmaPortMappingIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpuSdmaPortMappingIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->negotiationFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->negotiationFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "negotiationFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpldFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpldFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpldFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->mpdExternalFile.mpdExternalFileName_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)project_data_db_PTR->mpdExternalFile.mpdExternalFileName;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_displayString_CNS;
            hash_entry.data_size = strlen(/* HANDLE SIMPLE TYPE */project_data_db_PTR->mpdExternalFile.mpdExternalFileName) + 1;
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "mpdExternalFile.mpdExternalFileName",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->buttonFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->buttonFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "buttonFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->networkPorts.frontPanelGroupList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.systemled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.gpioblinkratecontrol.ledGpiorateGroupType.ledGpioRateControlList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.gpioblinkrateconrtolIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.gpioblinkrateconrtolIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.gpioblinkrateconrtolIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portsleds.portledsSupportedType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_PORT_LEDS_SUPPORTED_TYPE_E, project_data_db_PTR->leds.portsleds.portledsSupportedType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portsleds.portledsSupportedType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portsleds.portledsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.ledstreamleds.ledstreamportsleds.ledstreamportledsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.ledstreamleds.ledstreamInterfaceList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobledstateIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobledstateIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobledstateIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.portledstateled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackledList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress** */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType** */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId** */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.stackleds.stackLedController.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.stackleds.stackLedController.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.cloudmgmtled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.cloudmgmtled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.systemledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.systemledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.systemledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.poeled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.rpsled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.rpsled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.speedled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.speedled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.locatorled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.locatorled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.poeledIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.poeledIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.poeledIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.ledstreamIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.ledstreamIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.ledstreamIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledstateIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledstateIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledstateIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobledsSupportedType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_PORT_LEDS_SUPPORTED_TYPE_E, project_data_db_PTR->leds.oobleds.oobledsSupportedType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobledsSupportedType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.panelGroupNumber_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.panelGroupNumber;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_PANEL_GROUP_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.panelGroupNumber",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.panelPortNumber_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.panelPortNumber;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_PANEL_PORT_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.panelPortNumber",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyOffExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyOffExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyOffExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_E, project_data_db_PTR->leds.oobleds.oobsingleled.ledPpGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobsingleled.ledPpGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyOffExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyOffExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyOffExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_E, project_data_db_PTR->leds.oobleds.oobleftled.ledPpGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobleftled.ledPpGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyInitValuesExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyInitList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyOffExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyOffExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyOffExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledPpI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpGpioColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpValues.ledPpOffValue.functionCallGroupType.functionCallList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledPpValues.ledPpInitValue.functionCallGroupType.functionCallList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_E, project_data_db_PTR->leds.oobleds.oobrightled.ledPpGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.oobleds.oobrightled.ledPpGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2CColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.externalDriverId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.externalDriverId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_EXTERNAL_DRIVER_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.externalDriverId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateExist",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2COffValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_BUS_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_ACCESS_TYPE_E, project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_OFFSET_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_I2C_TRANSACTION_TYPE_E, project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_MASK_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CInitValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledExtdrvI2C.ledI2CAlternateValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterColorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledExtdrvGpioColorList */
        /* fill hash entry */
        if ((project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledInterfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_E, project_data_db_PTR->leds.fanled.ledExtdrvGroupType.ledInterfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.fanled.ledExtdrvGroupType.ledInterfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->leds.portledsIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->leds.portledsIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "leds.portledsIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->sensorFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->sensorFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "sensorFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->buttons.buttonsList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->mppGpioPins.mppDeviceList */
        /* fill hash entry */
        if ((project_data_db_PTR->backToBackIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->backToBackIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "backToBackIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->stackFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->stackFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "stackFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* Union is not supported - skip it */
        /* project_data_db_PTR->backToBackInformation */
        /* fill hash entry */
        if ((project_data_db_PTR->poeFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poeFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poeFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->serdesFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->serdesFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "serdesFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->cpu.cpuSdmaPortMapping.cpuSdmaPortList */
        /* Union is not supported - skip it */
        /* project_data_db_PTR->cpu.cpuInfoType */
        /* List is not supported - skip it */
        /* project_data_db_PTR->serdes.packetProcessorList */
        /* fill hash entry */
        if ((project_data_db_PTR->gbicIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->gbicIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "gbicIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->ledFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->ledFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "ledFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->isLogicalPortRequired_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->isLogicalPortRequired;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "isLogicalPortRequired",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->sensors.sensorToShow_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->sensors.sensorToShow;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "sensors.sensorToShow",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->sensors.sensorList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->externalDriver.externalDriverList */
        /* fill hash entry */
        if ((project_data_db_PTR->cpuFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpuFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpuFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpuInfoFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpuInfoFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpuInfoFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->packetProcessorIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->packetProcessorIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "packetProcessorIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.isIsrRegAddressUsed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.isIsrRegAddressUsed;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.isIsrRegAddressUsed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.poePoweredPdPortsDefaultRequestValueMw_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.poePoweredPdPortsDefaultRequestValueMw;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.poePoweredPdPortsDefaultRequestValueMw",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.poePoweredPdNumOfPorts_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.poePoweredPdNumOfPorts;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.poePoweredPdNumOfPorts",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.poePdPowerNegotiationSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.poePdPowerNegotiationSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.poePdPowerNegotiationSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pdportList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset** */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId** */
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_INTERFACE_TYPE_E, project_data_db_PTR->poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.isrregaddress.mask_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.isrregaddress.mask;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.isrregaddress.mask",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.isrregaddress.reg_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.isrregaddress.reg;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.isrregaddress.reg",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.isrregaddress.goodValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.isrregaddress.goodValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.isrregaddress.goodValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poepd.poePdPowerManagementSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poepd.poePdPowerManagementSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poepd.poePdPowerManagementSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_SMI_XSMI_TYPE_TYPE_E, project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_PHY_POSITION_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_SMI_XSMI_ADDRESS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_PHY_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_SMI_XSMI_INTERFACE_ID_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_PP_DEVICE_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.cpuPortNumber_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.cpuPortNumber;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_CPU_PORT_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.cpuPortNumber",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.ethId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.ethId;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.ethId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.logicalPort_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.logicalPort;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.logicalPort",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.oobDevice_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->oobPort.oobDevice;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.oobDevice",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->oobPort.speed_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_SPEED_TYPE_E, project_data_db_PTR->oobPort.speed, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "oobPort.speed",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->mpdExternalFileSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->mpdExternalFileSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "mpdExternalFileSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poePdIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poePdIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poePdIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->networkPortsFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->networkPortsFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "networkPortsFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->phyFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->phyFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "phyFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.fwFileName_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)project_data_db_PTR->poe.fwFileName;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_displayString_CNS;
            hash_entry.data_size = strlen(/* HANDLE SIMPLE TYPE */project_data_db_PTR->poe.fwFileName) + 1;
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.fwFileName",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.dteSupport_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poe.dteSupport;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.dteSupport",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.pseportsAreSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poe.pseportsAreSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.pseportsAreSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.hostSerialChannelId_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_HOST_SERIAL_CHANNEL_ID_E, project_data_db_PTR->poe.hostSerialChannelId, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.hostSerialChannelId",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.class0MappingMethod_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_MAPPING_METHOD_TYPE_E, project_data_db_PTR->poe.class0MappingMethod, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.class0MappingMethod",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.poePoweredPdPortsDefaultRequestValueMw_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->poe.poePoweredPdPortsDefaultRequestValueMw;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_32);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.poePoweredPdPortsDefaultRequestValueMw",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.poeCommunicationTypeValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_COMMUNICATION_TYPE_E, project_data_db_PTR->poe.poeCommunicationTypeValue, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.poeCommunicationTypeValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.pseFwFileName_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)project_data_db_PTR->poe.pseFwFileName;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_displayString_CNS;
            hash_entry.data_size = strlen(/* HANDLE SIMPLE TYPE */project_data_db_PTR->poe.pseFwFileName) + 1;
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.pseFwFileName",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.poeHwTypeValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_HARDWARE_TYPE_E, project_data_db_PTR->poe.poeHwTypeValue, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.poeHwTypeValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.poePowerBanks.poebankList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.pseports.pseList */
        /* fill hash entry */
        if ((project_data_db_PTR->poe.pseports.pseInformation_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_PSE_TYPE_E, project_data_db_PTR->poe.pseports.pseInformation, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.pseports.pseInformation",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->poe.pseports.pseportList */
        /* fill hash entry */
        if ((project_data_db_PTR->poe.poeVendor_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_VENDOR_TYPE_E, project_data_db_PTR->poe.poeVendor, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.poeVendor",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->poe.mcuType_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_POE_MCU_TYPE_E, project_data_db_PTR->poe.mcuType, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "poe.mcuType",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->negotiation.negotiationMode_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_NEGOTIATION_MODE_TYPE_E, project_data_db_PTR->negotiation.negotiationMode, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "negotiation.negotiationMode",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->boardDescriptionFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->boardDescriptionFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "boardDescriptionFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->externalDriverFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->externalDriverFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "externalDriverFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->phys.phyList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* fill hash entry */
        if ((project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset** */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId** */
        /* fill hash entry */
        if ((project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_INTERFACE_TYPE_E, project_data_db_PTR->power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "power.ps.psActiveinfo.powerConnectionReadOnlyGroupType.interfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.statusList */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyGpioGroupList */
        /* fill hash entry */
        if ((project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_I2C_VALUE_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.activeValue",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBusId** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CAccess** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2COffset** */
        /* List is not supported - skip it */
        /* project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.muxGroupType.muxList */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CTransactionType** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CMask** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.isI2CChannelUsed** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CBaseAddress** */
        /* fill hash entry */
        /*    Parameter name is too long - can't support it: */
        /* ** power.rps.connectedinfo.powerConnectionReadOnlyGroupType.powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType.i2CChannelId** */
        /* fill hash entry */
        if ((project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.interfaceSelect_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            /* HANDLE ENUM */
            /* if start */
            if (PDL_OK != pdlEnum2StrEntryGet(PDL_ENUM_ID_INTERFACE_TYPE_E, project_data_db_PTR->power.rps.connectedinfo.powerConnectionReadOnlyGroupType.interfaceSelect, &enum_str2value_entry_PTR))
            {
                RETURN_WRAPPER_MAC(PDL_ERROR);
            }
            /* if end */
            hash_entry.data_PTR = (void *)enum_str2value_entry_PTR;
            hash_entry.type = /* HANDLE ENUM */MIB_rlHostParamType_enum_CNS;
            hash_entry.data_size = sizeof(/* HANDLE ENUM */PDL_ENUM_STR2VALUE_STC);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "power.rps.connectedinfo.powerConnectionReadOnlyGroupType.interfaceSelect",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->power.rpsIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->power.rpsIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "power.rpsIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->boardInformation.boardDescription_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)project_data_db_PTR->boardInformation.boardDescription;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_displayString_CNS;
            hash_entry.data_size = strlen(/* HANDLE SIMPLE TYPE */project_data_db_PTR->boardInformation.boardDescription) + 1;
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "boardInformation.boardDescription",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->packetProcessors.numberOfPps_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->packetProcessors.numberOfPps;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_NUMBER_OF_PPS_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "packetProcessors.numberOfPps",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->mppPinFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->mppPinFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "mppPinFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->fans.swFanControllers.thresholdList */
        /* fill hash entry */
        if ((project_data_db_PTR->fans.fancontrollerIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->fans.fancontrollerIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "fans.fancontrollerIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* List is not supported - skip it */
        /* project_data_db_PTR->fans.fanControllerList */
        /* fill hash entry */
        if ((project_data_db_PTR->fans.swfancontrollerIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->fans.swfancontrollerIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "fans.swfancontrollerIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->stackInfo.placeHolder_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->stackInfo.placeHolder;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "stackInfo.placeHolder",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->fanFeatureIsSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->fanFeatureIsSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "fanFeatureIsSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgrade.gpioTdo_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgrade.gpioTdo;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgrade.gpioTdo",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgrade.gpioTms_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgrade.gpioTms;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgrade.gpioTms",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgrade.gpioTck_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgrade.gpioTck;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgrade.gpioTck",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgrade.cpldPinsDeviceNum_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgrade.cpldPinsDeviceNum;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE TYPE USING SIMPLE TYPE */PDL_MPP_DEVICE_NUMBER_TYPE_TYP);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgrade.cpldPinsDeviceNum",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgrade.gpioTdi_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgrade.gpioTdi;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_uint_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */UINT_8);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgrade.gpioTdi",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        /* fill hash entry */
        if ((project_data_db_PTR->cpld.onlineUpgradeSupported_mask) & PDL_FIELD_HAS_VALUE_CNS)
        {
            hash_entry.data_PTR = (void *)&project_data_db_PTR->cpld.onlineUpgradeSupported;
            hash_entry.type = /* HANDLE TYPE USING SIMPLE TYPE */MIB_rlHostParamType_truthValue_CNS;
            hash_entry.data_size = sizeof(/* HANDLE SIMPLE TYPE */BOOLEAN);
            hash_status = VHASHG_set_by_string_key(
                *hash_db_PTR,
                "cpld.onlineUpgradeSupported",
                sizeof(PDL_FEATURE_HPRM_DATA_STC),
                &hash_entry,
                NULL);
            /* if start */
            if (hash_status != VHASHG_status_ok_E)
            {
                RETURN_WRAPPER_MAC(PDL_NO_SUCH);
            }
            /* if end */
        }
        return PDL_OK;
    }
