/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdDebug.c
 *  @brief PHY debug functions.
 *
 */
#include  	<mpdPrefix.h>
#include 	<stdio.h>
#include    <mpdTypes.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>

#ifdef MTD_DEBUG
#ifdef MTD_PHY_INCLUDE
#include    <mtdApiTypes.h>
#include    <mtdAPI.h>
#include    <mtdAPIInternal.h>
#include    <mtdApiRegs.h>
#include    <mtdDiagnostics.h>
#include    <mtdDiagnosticsRegDumpData.h>
#include    <mtdEEE.h>
#include    <mtdIntr.h>
#include    <mtdHwSerdesCntl.h>
#include    <serdes/mcesd/mcesdTop.h>
#include    <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include    <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.h>
#include    <mtdUtils.h>
#include    <mtdHXunit.h>
#endif
#ifdef MYD_PHY_INCLUDE
#include    <mydAPI.h>

#endif
#include    <mpdVersion.h>

#define MPD_DEBUG_ARRAY_SIZE_MAC(_arr) (sizeof(_arr) / sizeof(char *))

static const char * prvMpdDebugConvertPhyOpToText_ARR[] = {
	"MPD_OP_CODE_SET_MDIX_E",
	"MPD_OP_CODE_GET_MDIX_E",
	"MPD_OP_CODE_GET_MDIX_ADMIN_E",
	"MPD_OP_CODE_SET_AUTONEG_E",
	"MPD_OP_CODE_GET_AUTONEG_ADMIN_E",
	"MPD_OP_CODE_SET_RESTART_AUTONEG_E",
	"MPD_OP_CODE_SET_DUPLEX_MODE_E",
	"MPD_OP_CODE_SET_SPEED_E",
	"MPD_OP_CODE_SET_VCT_TEST_E",
	"MPD_OP_CODE_GET_EXT_VCT_PARAMS_E",
	"MPD_OP_CODE_GET_CABLE_LEN_E",
	"MPD_OP_CODE_SET_RESET_PHY_E",
	"MPD_OP_CODE_SET_PHY_DISABLE_OPER_E",
	"MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E",
	"MPD_OP_CODE_SET_ADVERTISE_FC_E",
	"MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E",
	"MPD_OP_CODE_SET_POWER_MODULES_E",
	"MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E",
	"MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E",
	"MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E",
	"MPD_OP_CODE_SET_MDIO_ACCESS_E",
	"MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E",
	"MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E",
    "MPD_OP_CODE_GET_EEE_STATUS_E",
	"MPD_OP_CODE_SET_LPI_EXIT_TIME_E",
	"MPD_OP_CODE_SET_LPI_ENTER_TIME_E",
	"MPD_OP_CODE_GET_EEE_CAPABILITY_E",
	"MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E",
	"MPD_OP_CODE_GET_VCT_CAPABILITY_E",
	"MPD_OP_CODE_GET_DTE_STATUS_E",
	"MPD_OP_CODE_SET_DTE_E",
	"MPD_OP_CODE_GET_TEMPERATURE_E",
	"MPD_OP_CODE_GET_REVISION_E",
	"MPD_OP_CODE_GET_AUTONEG_SUPPORT_E",
	"MPD_OP_CODE_SET_SPEED_EXT_E",
	"MPD_OP_CODE_GET_SPEED_EXT_E",
	"MPD_OP_CODE_SET_SERDES_TUNE_E",
	"MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E",
	"MPD_OP_CODE_SET_AUTONEG_MULTISPEED_E",
    "MPD_OP_CODE_GET_PTP_TSQ_E",
    "MPD_OP_CODE_SEND_IDLE_TO_HOST_E",
    "MPD_OP_PHY_CODE_SET_PTP_INIT_E",
    "MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E",
    "MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E",
	"PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E",
	"PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E",
	"PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E",
	"PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E",
	"PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E",
	"PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E",
	"PRV_MPD_OP_CODE_SET_LOOP_BACK_E",
	"PRV_MPD_OP_CODE_GET_VCT_OFFSET_E",
	"PRV_MPD_OP_CODE_INIT_E",
	"PRV_MPD_OP_CODE_SET_ERRATA_E",
	"PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E",
	"PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E",
	"PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E",
	"PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E",
	"PRV_MPD_OP_CODE_GET_LANE_BMP_E",
	"PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E",
	"PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E",
	"PRV_MPD_OP_CODE_GET_I2C_READ_E",
	"PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E",
	"PRV_MPD_OP_CODE_GET_GREEN_READINESS_E",
    "PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E",
    "PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E",
    "PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E"
};

#define PRV_MPD_OP_CODE_TO_TEXT_MAC(_op_ode) \
	((_op_ode < MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyOpToText_ARR))? prvMpdDebugConvertPhyOpToText_ARR[_op_code]:"Illegal Op")

static const char *  prvMpdDebugConvertPhyTypeToText_ARR[MPD_TYPE_NUM_OF_TYPES_E] = {
    "NO PHY",
    "88E1543",
    "88E1545",
    "88E1548",
    "88E1680",
    "88E1680L",
    "88E151x",
    "88E3680",
    "88X32x0",
    "88X33x0",
    "88X20x0",
    "88X2180",
    "88E2540",
    "88X3540",
    "88E1780",
    "88E2580",
    "88E3580",
    "88E1112",
    "88E1781",
    "88E1781In",
    "88X7120",
    "USR0",
    "USR1",
    "USR2",
    "USR3",
    "USR4",
    "USR5",
    "USR6",
    "USR7",
    "USR8",
    "USR9",
    "USR10",
    "USR11",
    "USR12",
    "USR13",
    "USR14",
    "USR15",
    "USR16",
    "USR17",
    "USR18",
    "USR19",
    "USR20",
    "USR21",
    "USR22",
    "USR23",
    "USR24",
    "USR25",
    "USR26",
    "USR27",
    "USR28",
    "USR29",
    "USR30",
    "USR31",
    "USR32"
};

static const char * prvMpdDebugConvertPhyMdiModeToText_ARR[] = {
    "MDI",
    "MDIX",
    "AUTO"
};

static const char * prvMpdDebugConvertPhyMediaTypeToText_ARR[] = {
    "RJ45",
    "FIBER",
    "INVALID"
};

static const char * prvMpdDebugConvertPhyTransceiverTypeToText_ARR[MPD_TRANSCEIVER_MAX_TYPES_E] = {
    "SFP",
    "RJ45",
    "COMBO"
};

static const char *  prvMpdDebugConvertOpModeToText_ARR[MPD_OP_MODE_LAST_E] = {
    "FIBER",
    "DAC",
    "RJ45 SFP 1GBASEX",
	"RJ45 SFP SGMII",
	"SFP+",
    "UNKNOWN"
};

static const char * prvMpdDebugConvertGreenSetToText_ARR[] = {
    "GREEN_NO_SET",
    "GREEN_ENABLE",
    "GREEN_DISABLE"
};

static const char * prvMpdDebugConvertPortSpeedToText_ARR[] = {
    "PORT_SPEED_10",                            /* 0   */
    "PORT_SPEED_100",                           /* 1   */
    "PORT_SPEED_1000",                          /* 2   */
    "PORT_SPEED_10000",                         /* 3   */
    "PORT_SPEED_12000",                         /* 4   */
    "PORT_SPEED_2500",                          /* 5   */
    "PORT_SPEED_5000",                          /* 6   */
    "PORT_SPEED_13600",                         /* 7   */
    "PORT_SPEED_20000",                         /* 8   */
    "PORT_SPEED_40000",                         /* 9   */
    "PORT_SPEED_16000",                         /* 10  */
    "PORT_SPEED_15000",                         /* 11  */
    "PORT_SPEED_75000",                         /* 12  */
    "PORT_SPEED_100000",                        /* 13  */
    "PORT_SPEED_50000",                         /* 14  */
    "PORT_SPEED_140000",                        /* 15  */
    "PORT_SPEED_11800",                         /* 16  */
    "PORT_SPEED_47200",                         /* 17  */
    "PORT_SPEED_22000",                         /* 18  */
    "PORT_SPEED_23600",                         /* 19  */
    "PORT_SPEED_12500",                         /* 20  */
    "PORT_SPEED_25000",                         /* 21  */
    "PORT_SPEED_107000",                        /* 22  */
    "PORT_SPEED_29090",                         /* 23  */
    "PORT_SPEED_200000",                        /* 24  */
    "PORT_SPEED_400000",                        /* 25  */
    "PORT_SPEED_102000",                        /* 26  */
    "PORT_SPEED_52500",                         /* 27  */
    "PORT_SPEED_26700",                         /* 28  */
    "PORT_SPEED_NA_CNS",                        /* 29  */ /* GT_SPEED_NA */
};


static const char * prvMpdDebugConvertPhySpeedToText_ARR[] = {
    "10M",   /* 0   */
    "100M",  /* 1   */
    "1G",    /* 2   */
    "10G",   /* 3   */
    "12G",   /* 4   */
    "2.5G",  /* 5   */
    "5G",    /* 6   */
    "13.6G", /* 7   */
    "20G",   /* 8   */
    "40G",   /* 9   */
    "16G",   /* 10  */
    "15G",   /* 11  */
    "75G",   /* 12  */
    "100G",  /* 13  */
    "50G",   /* 14  */
    "140G",  /* 15  */
    "11.8G", /* 16  */
    "47.2G", /* 17  */
    "22G",   /* 18  */
    "23.6G", /* 19  */
    "12.5G", /* 20  */
    "25G",   /* 21  */
    "107G",  /* 22  */
    "29.09G",/* 23  */
    "200G",  /* 24  */
    "400G",  /* 25  */
    "102G",  /* 26  */
    "53.5G", /* 27  */
    "NA",    /* 28  */
    "NA"     /* 29  */
};

static const char * prvMpdDebugConvertGreenReadinessToText_ARR[] = {
    "GREEN_READINESS_OPRNORMAL_CNS",
    "GREEN_READINESS_FIBER_CNS",
    "GREEN_READINESS_COMBO_FIBER_CNS",
    "GREEN_READINESS_NOT_SUPPORTED_CNS",
};

static const char * prvMpdDebugConvertPhyKindToText_ARR[]= {
    "RJ45",
    "SFP",
    "COMBO",
    "INVALID"
};

static const char * prvMpdDebugConvertEEESpeedToText_ARR[] = {
    "PHY_EEE_SPEED_10M_CNS",
    "PHY_EEE_SPEED_100M_CNS",
    "PHY_EEE_SPEED_1G_CNS",
    "PHY_EEE_SPEED_10G_CNS",
    "PHY_EEE_SPEED_MAX_CNS"
};

static const char * prvMpdDebugConvertEEESCapabilityToText_ARR[] = {
    "BIT_MAP_100BASE_TX_CNS",
    "BIT_MAP_1000BASE_T_CNS",
    "BIT_MAP_10GBASE_T_CNS",
    "BIT_MAP_1000BASE_KX_CNS",
    "BIT_MAP_10GBASE_KX4_CNS",
    "BIT_MAP_10GBASE_KR_CNS",
    "BIT_MAP_2500BASE_T_CNS",
    "BIT_MAP_5GBASE_T_CNS"
};


static const char *  prvMpdDebugConvertMacOnPhyToText_ARR[3] = {
    "NO_BYPASS",
    "BYPASS",
    "NOT INITIALIZED"
};

static const char *  prvMpdDebugConvertUsxTypeToText_ARR[MPD_PHY_USX_MAX_TYPES] = {
    "NO_USX",
    "SXGMII",
    "DXGMII 10G",
    "DXGMII 20G",
    "QXGMII 10G",
    "QXGMII 20G",
    "OXGMII"
};

static PRV_MPD_DEBUG_CONVERT_STC prvDebugConertDb_ARR[MPD_DEBUG_CONVERT_LAST_E] = {
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyTypeToText_ARR),                prvMpdDebugConvertPhyTypeToText_ARR         },  /*  MPD_DEBUG_CONVERT_PHY_TYPE_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyOpToText_ARR),                  prvMpdDebugConvertPhyOpToText_ARR           },  /*  MPD_DEBUG_CONVERT_OP_ID_E                   */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyMdiModeToText_ARR),             prvMpdDebugConvertPhyMdiModeToText_ARR      },  /*  MPD_DEBUG_CONVERT_MDI_MODE_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyMediaTypeToText_ARR),           prvMpdDebugConvertPhyMediaTypeToText_ARR    },  /*  MPD_DEBUG_CONVERT_MEDIA_TYPE_E              */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertGreenSetToText_ARR),               prvMpdDebugConvertGreenSetToText_ARR        },  /*  MPD_DEBUG_CONVERT_GREEN_SET_E               */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPortSpeedToText_ARR),              prvMpdDebugConvertPortSpeedToText_ARR       },  /*  MPD_DEBUG_CONVERT_PORT_SPEED_E              */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertGreenReadinessToText_ARR),         prvMpdDebugConvertGreenReadinessToText_ARR  },  /*  MPD_DEBUG_CONVERT_GREEN_READINESS_E         */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyKindToText_ARR),                prvMpdDebugConvertPhyKindToText_ARR         },  /*  MPD_DEBUG_CONVERT_PHY_KIND_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhySpeedToText_ARR),               prvMpdDebugConvertPhySpeedToText_ARR        },  /*  MPD_DEBUG_CONVERT_PHY_SPEED_E               */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertEEESpeedToText_ARR),               prvMpdDebugConvertEEESpeedToText_ARR        },  /*  MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E           */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertEEESCapabilityToText_ARR),         prvMpdDebugConvertEEESCapabilityToText_ARR  }   /*  MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E      */
};


static const char prvMpdDebugComponentName[] = "phy";
static const char prvMpdDebugPckName[] = "drv";

static const char prvMpdDebugTraceFlagName[] = "trace";
static const char prvMpdDebugTraceFlagHelp[] = "Debug Trace for PHY ";

static const char prvMpdDebugConfigVctFlagName[] = "vct";
static const char prvMpdDebugConfigVctFlagHelp[] = "debug PHY vct operations";

static const char prvMpdDebugWriteFlagName[] = "write";
static const char prvMpdDebugWriteFlagHelp[] = "trace register write per port ";

static const char prvMpdDebugReadFlagName[] = "read";
static const char prvMpdDebugReadFlagHelp[] = "trace register read per port ";

static const char prvMpdDebugSfpPckName[] = "sfp";
static const char prvMpdDebugSfpFlagName[] = "state";
static const char prvMpdDebugSfpFlagHelp[] = "debug PHY SFP operations";

static const char prvMpdDebugErrorFlagName[] = "error";
static const char prvMpdDebugErrorFlagHelp[] = "debug PHY config errors";

static const char prvMpdDebugOperationGetFlagName[] = "getoper";
static const char prvMpdDebugOperationGetFlagHelp[] = "debug PHY get operations";

static const char prvMpdDebugOperationSetFlagName[] = "setoper";
static const char prvMpdDebugOperationSetFlagHelp[] = "debug PHY set operations";

static const char prvMpdDebugReducedFlagName[] = "reduced";
static const char prvMpdDebugReducedFlagHelp[] = "reduce the configuration done on the phys";


extern const char * prvMpdDebugConvert(
    /*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   convType,
    UINT_32                 value
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{

    PRV_MPD_DEBUG_CONVERT_STC * deb_entry_PTR;

    if (convType >= MPD_DEBUG_CONVERT_LAST_E) {
        return "badConv";
    }

    deb_entry_PTR = & prvDebugConertDb_ARR[convType];
    if (value >= deb_entry_PTR->num_of_entries) {
        return "badValue";
    }

    return deb_entry_PTR->text_ARR[value];
}

static BOOLEAN prvMpdIs1GPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    BOOLEAN ret = FALSE;

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR) {
        switch (portEntry_PTR->initData_PTR->phyType) {
            case MPD_TYPE_88E1780_E:
            case MPD_TYPE_88E1781_E:
            case MPD_TYPE_88E1781_internal_E:
            case MPD_TYPE_88E1543_E:
            case MPD_TYPE_88E1545_E:
            case MPD_TYPE_88E1548_E:
            case MPD_TYPE_88E1680_E:
            case MPD_TYPE_88E1680L_E:
            case MPD_TYPE_88E151x_E:
            case MPD_TYPE_88E3680_E:
            case MPD_TYPE_88E1112_E:
                ret = TRUE;
                break;
            default:
                ret = FALSE;
        }
    }

    return ret;
}

#ifdef MTD_PHY_INCLUDE

static BOOLEAN prvMpdIsMtdPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    BOOLEAN ret = FALSE;

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR) {
        switch (portEntry_PTR->initData_PTR->phyType) {
            case MPD_TYPE_88X32x0_E:
            case MPD_TYPE_88X33x0_E:
            case MPD_TYPE_88X20x0_E:
            case MPD_TYPE_88X2180_E:
            case MPD_TYPE_88E2540_E:
            case MPD_TYPE_88X3540_E:
            case MPD_TYPE_88E2580_E:
			case MPD_TYPE_88X3580_E:
                ret = TRUE;
                break;
            default:
                ret = FALSE;
        }
    }

    return ret;
}


static BOOLEAN prvMpdIsVoltronDevice(
	/*     INPUTS:             */
	UINT_32    rel_ifIndex
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	)
{
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
	BOOLEAN ret = FALSE;

	portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (portEntry_PTR) {
		ret = MPD_IS_VOLTRON_DEVICE_MAC(portEntry_PTR->initData_PTR->phyType);
	}
	return ret;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdValidateParamValue
 *
 * DESCRIPTION: check if the value of the parameter in API_C28GP4X2_SetTxEqParam function is valid
 *****************************************************************************/
static BOOLEAN prvMpdValidateParamValue(
	/*     INPUTS:             */
	UINT_32                  paramValue,
	MPD_DEBUG_TXEQ_PARAM_E   paramIndex,
	UINT_32                  rel_ifIndex
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	BOOLEAN rc = TRUE;

	switch (paramIndex) {
		case MPD_DEBUG_TXEQ_EM_POST_CTRL_E:
		case MPD_DEBUG_TXEQ_EM_PEAK_CTRL_E:
		case MPD_DEBUG_TXEQ_EM_PRE_CTRL_E:
			if (paramValue > 15) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex [%d]: index [%d] value [%d] is out of range", rel_ifIndex, paramIndex, paramValue);
				rc = FALSE;
			}
			break;
		case MPD_DEBUG_TXEQ_MARGIN_E:
			if ((paramValue < 3 && paramValue > 0) || (paramValue > 7)) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex [%d]: margin value [%d] is out of range", rel_ifIndex, paramValue);
				rc = FALSE;
			}
			break;
		default:
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex [%d]: unknown param [%d]", rel_ifIndex, paramIndex);
			rc = FALSE;
	}
	if (rc == FALSE)
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
	return rc;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdConvertMpdToMtdTxEq
 *
 * DESCRIPTION: convert from mpd to mtd tx eq param
 *****************************************************************************/
static BOOLEAN prvMpdConvertMpdToMtdTxEq(
	/*     INPUTS:             */
	MPD_DEBUG_TXEQ_PARAM_E   paramIndex,

	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	E_C28GP4X2_TXEQ_PARAM  *realParamIndex
)
{
	BOOLEAN rc = TRUE;

	switch (paramIndex) {
		case MPD_DEBUG_TXEQ_EM_POST_CTRL_E:
				*realParamIndex = C28GP4X2_TXEQ_EM_POST_CTRL;
				break;
		case MPD_DEBUG_TXEQ_EM_PEAK_CTRL_E:
				*realParamIndex = C28GP4X2_TXEQ_EM_PEAK_CTRL;
				break;
		case MPD_DEBUG_TXEQ_EM_PRE_CTRL_E:
				*realParamIndex = C28GP4X2_TXEQ_EM_PRE_CTRL;
				break;
		case MPD_DEBUG_TXEQ_MARGIN_E:
			*realParamIndex = C28GP4X2_TXEQ_MARGIN;
			break;
		default:
			PRV_MPD_LOGGING_MAC("un valid index parameter \n");
			rc = FALSE;
	}
	return rc;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPktGenTHUnit
 *
 * DESCRIPTION: send packets from T unit (able to send in both directions - T & H) on 2.5G and higher
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdPktGenTHUnit(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex,
	BOOLEAN sendOnTUnit, /* true - send to T unit, false - send to MAC */
	UINT_16	numPktsToSend /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously, 0 - to stop packet generator */
)
{
	MTD_STATUS mtdStatus;
	MTD_U16 speed;
	MTD_BOOL islinkup, readToClear = MTD_TRUE, generateCRCoff = MTD_FALSE;
	MTD_U16 pktPatternControl = MTD_PKT_RANDOM_BYTE;
	MTD_U32 initialPayload = 0x0;
	MTD_U16 frameLengthControl = MTD_PKT_RAND_LEN0; /* Please refers to definitions in the mtdDiagnostics.h */
	MTD_BOOL randomIPG = MTD_FALSE;
	MTD_U16 ipgDuration = 12;
	MTD_BOOL clearInitCounters = MTD_TRUE;  /* clear counters before start */
	MTD_U16  generatorControl;
	MTD_U16  checkerControl;
	MTD_BOOL enableGenerator = MTD_TRUE;

	if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"received not MTD rel_ifIndex %d\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_SUPPORTED_E;
	}

	mtdStatus = mtdIsBaseTUp(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speed, &islinkup);
	if (mtdStatus != MTD_OK) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdIsBaseTUp with status 0x%x.\n", mtdStatus);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_OP_FAILED_E;
	}

	if (islinkup) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d speed 0x%x link %d. Sending traffic %d on %s unit\n",
				rel_ifIndex, speed, islinkup, numPktsToSend, (sendOnTUnit) ? "T" : "H");
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

		if (!sendOnTUnit) {
			speed = MTD_SPEED_10GIG_FD;
		}
		/* The traffic will start immediately after packet generator is enabled */
		/* Configure Packet generator/checker */
		mtdStatus = mtdTunitConfigurePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speed, readToClear, pktPatternControl,
														 generateCRCoff, initialPayload, frameLengthControl,
														 numPktsToSend, randomIPG, ipgDuration, clearInitCounters);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdTunitConfigurePktGeneratorChecker with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OP_FAILED_E;
		}

		/* The 2.5G/5G/10G packet generator can send data towards the line, MAC or both */
		/* The 10M/100M/1G packet generator can only send data towards the line. */
		generatorControl = (sendOnTUnit) ? MTD_PACKETGEN_TO_LINE : MTD_PACKETGEN_TO_MAC;
		checkerControl = (sendOnTUnit) ? MTD_CHECKER_FROM_LINE : MTD_CHECKER_FROM_MAC;
		/* Note: Calling this function for 1G and below speed will result unpredictable result! */
		if ((speed == MTD_SPEED_10GIG_FD) || (speed == MTD_SPEED_2P5GIG_FD) || (speed == MTD_SPEED_5GIG_FD)) {
			mtdStatus = mtdTunit10GPktGenMuxSelect(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), generatorControl, checkerControl);
			if (mtdStatus != MTD_OK) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdTunit10GPktGenMuxSelect with status 0x%x.\n", mtdStatus);
				PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
				return MPD_OP_FAILED_E;
			}
		}

		enableGenerator = (numPktsToSend) ? MTD_TRUE : MTD_FALSE;
		/* Enable the packet generator and checker */
		mtdStatus = mtdTunitEnablePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speed, enableGenerator, MTD_TRUE);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdTunitEnablePktGeneratorChecker with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OP_FAILED_E;
		}

	} else {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d speed 0x%x link %d. Not sending traffic\n", rel_ifIndex, speed, islinkup);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
	}
	return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPktGenOnXHUnit
 *
 * DESCRIPTION: send packets from H unit (able to send in both directions - X & MAC)
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdPktGenOnXHUnit(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex,
	BOOLEAN sendOnXUnit, /* true - send to X unit, false - send to H unit */
	UINT_16	numPktsToSend /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously, 0 - to stop packet generator */
)
{
	MTD_STATUS mtdStatus;
	MTD_BOOL readToClear = MTD_TRUE, generateCRCoff = MTD_FALSE, dontuseSFDinChecker = MTD_FALSE;
	MTD_U16 pktPatternControl = MTD_PKT_RANDOM_BYTE;
	MTD_U32 initialPayload = 0x0;
	MTD_U16 frameLengthControl = MTD_PKT_RAND_LEN0; /* Please refers to definitions in the mtdDiagnostics.h */
	MTD_BOOL randomIPG = MTD_FALSE;
	MTD_U16 ipgDuration = 3;

	if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"received not MTD rel_ifIndex %d\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_SUPPORTED_E;
	}

	prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d Sending traffic %d on %s unit\n",
			rel_ifIndex, numPktsToSend, (sendOnXUnit) ? "X" : "H");
	PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
	if (numPktsToSend) {
		/* The traffic will start immediately after packet generator is enabled */
		/* Configure Packet generator/checker */
		/* Start generator and checker, no packet is generating */
		mtdStatus = mtdConfigurePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), (sendOnXUnit) ? MTD_X_UNIT : MTD_H_UNIT, readToClear, dontuseSFDinChecker, pktPatternControl,
													generateCRCoff, initialPayload, frameLengthControl, numPktsToSend, randomIPG, ipgDuration);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdConfigurePktGeneratorChecker with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OP_FAILED_E;
		}
		    /* User can also start with preset numPktsToSent, and the traffic will start immediately */
		    /* after calling this function */
		mtdStatus = mtdEnablePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), (sendOnXUnit) ? MTD_X_UNIT : MTD_H_UNIT , MTD_TRUE, MTD_TRUE);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdEnablePktGeneratorChecker with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OP_FAILED_E;
		}
	} else {
		/* Stop packet generator/checker */
		mtdStatus = mtdEnablePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), (sendOnXUnit) ? MTD_X_UNIT : MTD_H_UNIT, MTD_FALSE, MTD_FALSE);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to Stop packet generator/checker with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OP_FAILED_E;
		}
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPktCntChk
 *
 * DESCRIPTION: count packets on X or H unit or T or H unit
 *
 * NOTE: for enabling the counters, need to use API prvMpdMtdPktGenOnXHUnit or prvMpdMtdPktGenTHUnit
 *       if num of packets = 0, enable only the CNT checker.
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdPktCntChk
(
	/*     INPUTS:             */
	UINT_32 rel_ifIndex
)
{
    /* Variables for packet generator */
	MTD_U64 hCounters[2][3];      /* [pkt/byte][tx/rx/err] */
	MTD_U64 xtCounters[2][3];      /* [pkt/byte][tx/rx/err] */
	MTD_U16 pktCntIdx = 0, byteCntIdx = 1;
    MTD_U16 counterIndex;
    MTD_U16 xLinkDropCounter = 0, hLinkDropCounter = 0;
    MTD_U16 speed;
	MTD_BOOL islinkup;
    MTD_STATUS mtdStatus;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    BOOLEAN all = FALSE;
    UINT_32 index = 1;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		all = TRUE;
	}
	else {
		index = rel_ifIndex;
	}

	PRV_MPD_LOGGING_MAC("\n");
	PRV_MPD_LOGGING_MAC(" port #| pkt Cnt on H Unit              | byte Cnt on H Unit             | pkt Cnt on X/T unit            | byte Cnt on X/T unit           | xUnit  | hUnit  \n");
	PRV_MPD_LOGGING_MAC("       |  TX      | RX       | ERR      |  TX      | RX       | ERR      |  TX      | RX       | ERR      |  TX      | RX       | ERR      | DropCnt| DropCnt\n");
	PRV_MPD_LOGGING_MAC("--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

	do {
		if (!prvMpdIsMtdPhyType(index)) {
			continue;
		}
		portEntry_PTR = prvMpdGetPortEntry(index);
		if (portEntry_PTR) {
			/* X unit is not active */
			if (!portEntry_PTR->runningData_PTR->sfpPresent) {
				mtdStatus = mtdIsBaseTUp(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &speed, &islinkup);
				if (mtdStatus != MTD_OK) {
					continue;
				}
			}
		} else {
			continue;
		}

		memset(hCounters, 0, sizeof(MTD_U64));
		memset(xtCounters, 0, sizeof(MTD_U64));
		xLinkDropCounter = 0;
		hLinkDropCounter = 0;
		/* Read to clear counters */
		for (counterIndex = MTD_PKT_GET_TX; counterIndex < (MTD_PKT_GET_ERR+1); counterIndex++)
		{
			mtdStatus = mtdPktGeneratorGetCounter(PRV_MPD_MTD_PORT_OBJECT_MAC(index), MTD_H_UNIT, counterIndex,
												  &hCounters[pktCntIdx][counterIndex], &hCounters[byteCntIdx][counterIndex]);
			if (mtdStatus != MTD_OK) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdPktGeneratorGetCounter with status 0x%x.\n", mtdStatus);
				PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			}
			if (portEntry_PTR->runningData_PTR->sfpPresent) {
				mtdStatus = mtdPktGeneratorGetCounter(PRV_MPD_MTD_PORT_OBJECT_MAC(index), MTD_X_UNIT, counterIndex,
													  &xtCounters[pktCntIdx][counterIndex], &xtCounters[byteCntIdx][counterIndex]);
			} else {
				if (islinkup) {
					mtdStatus = mtdTunitPktGeneratorGetCounter(PRV_MPD_MTD_PORT_OBJECT_MAC(index), speed, counterIndex,
														&xtCounters[pktCntIdx][counterIndex], &xtCounters[byteCntIdx][counterIndex]);
				}
			}
			if (mtdStatus != MTD_OK) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdPktGeneratorGetCounter with status 0x%x.\n", mtdStatus);
				PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			}
		}
		if (!portEntry_PTR->runningData_PTR->sfpPresent) {
			if (islinkup) {
				/* Notes: For the 10M/100M/1G packet generator/checker, this is the only way to clear the counters */
				mtdStatus = mtdTunitPktGeneratorCounterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(index), speed);
				if (mtdStatus != MTD_OK) {
					prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdTunitPktGeneratorCounterReset with status 0x%x.\n", mtdStatus);
					PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
				}
			}
		} else {
			/* Read link drop counter */
			mtdStatus = mtdPktGeneratorCheckerGetLinkDrop(PRV_MPD_MTD_PORT_OBJECT_MAC(index), MTD_X_UNIT, &xLinkDropCounter);
			if (mtdStatus != MTD_OK) {
				prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdPktGeneratorCheckerGetLinkDrop with status 0x%x.\n", mtdStatus);
				PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
				xLinkDropCounter = 0xFFFF;
			}
		}
		/* Read link drop counter */
		mtdStatus = mtdPktGeneratorCheckerGetLinkDrop(PRV_MPD_MTD_PORT_OBJECT_MAC(index), MTD_H_UNIT, &hLinkDropCounter);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Failed to mtdPktGeneratorCheckerGetLinkDrop with status 0x%x.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			hLinkDropCounter = 0xFFFF;
		}

		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), " %-6d|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|%-8u|%-8u\n",
					index,
					hCounters[pktCntIdx][MTD_PKT_GET_TX],
					hCounters[pktCntIdx][MTD_PKT_GET_RX],
					hCounters[pktCntIdx][MTD_PKT_GET_ERR],
					hCounters[byteCntIdx][MTD_PKT_GET_TX],
					hCounters[byteCntIdx][MTD_PKT_GET_RX],
					hCounters[byteCntIdx][MTD_PKT_GET_ERR],

					xtCounters[pktCntIdx][MTD_PKT_GET_TX],
					xtCounters[pktCntIdx][MTD_PKT_GET_RX],
					xtCounters[pktCntIdx][MTD_PKT_GET_ERR],
					xtCounters[byteCntIdx][MTD_PKT_GET_TX],
					xtCounters[byteCntIdx][MTD_PKT_GET_RX],
					xtCounters[byteCntIdx][MTD_PKT_GET_ERR],

					xLinkDropCounter,
					hLinkDropCounter
					);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
	}
	while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);

    return MTD_OK;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugGetEye
 *
 * DESCRIPTION: get serdes eye
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdDebugGetEye(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MTD_U16 eyeArea = 0;
    MTD_U16 eyeWidth = 0;
    MTD_U16 eyeHeight = 0;
    /* provide the arrays for the eye data */
    MTD_U32 eyeDataUp[(MTD_EYEDIAGRAM_NROWS + 1) / 2][MTD_EYEDIAGRAM_NCOLS]  = {{0}};
    MTD_U32 eyeDataDn[(MTD_EYEDIAGRAM_NROWS + 1) / 2][MTD_EYEDIAGRAM_NCOLS] = {{0}};
    UINT_32 lane;
    MTD_STATUS mtdStatus;
    MTD_SERDES_28GX2_EYE_RAW eyeRawData;

    if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"received not MTD rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    for (lane = 0; lane < 2; lane++) {
        if (MTD_IS_X35X0_E2540_DEVICE(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex))) {
            memset(&eyeRawData, 0, sizeof(MTD_SERDES_28GX2_EYE_RAW));
            mtdStatus = mtdSerdes28GX2EyeRawData(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), lane, &eyeRawData);
            if (mtdStatus != MTD_OK) {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Some kind of error occurred in mtdSerdes28GX2EyeRawData status %d.\n", mtdStatus);
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                return MPD_OK_E;
            }
            mtdStatus = mtdSerdes28GX2EyePlotStats(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), &eyeRawData, &eyeWidth, &eyeHeight, &eyeArea);
            if (mtdStatus != MTD_OK) {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Some kind of error occurred in mtdSerdes28GX2EyePlotStats status %d.\n", mtdStatus);
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                return MPD_OK_E;
            }
        } else {
            mtdStatus = mtdGetSerdesEyeStatistics(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), lane, eyeDataUp, eyeDataDn,
                                              &eyeArea, &eyeWidth, &eyeHeight);
            if (mtdStatus != MTD_OK) {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Some kind of error occurred in mtdGetSerdesEyeStatistics status %d.\n", mtdStatus);
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                return MPD_OK_E;
            }
            mtdStatus = mtdSerdesEyePlotChart(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), eyeDataUp, eyeDataDn);
            if (mtdStatus != MTD_OK) {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Some kind of error occurred in mtdSerdesEyePlotChart status %d.\n", mtdStatus);
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                return MPD_OK_E;
            }
        }

        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"GetEye: for lane %d eyeArea:%u eyeWidth:%u eyeHeight:%u\n Note: to draw chart open mtd debug\n",
                     lane, eyeArea, eyeWidth, eyeHeight);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugGetFwDetailedBuildInfo
 *
 * DESCRIPTION: get additional detailed build information about the currently running firmware
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugGetFwDetailedBuildInfo (
	/*     INPUTS:             */
	UINT_32    rel_ifIndex
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
		MTD_STATUS mtdStatus;
		MTD_U16 buildInfo1;
		MTD_U16 buildInfo2;
		MTD_U16 buildInfo3;

		if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"received not MTD rel_ifIndex %d\n", rel_ifIndex);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_NOT_SUPPORTED_E;
		}


		mtdStatus = mtdGetFwDetailedBuildInfo(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), rel_ifIndex,&buildInfo1, &buildInfo2, &buildInfo3);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "Some kind of error occurred in mtdHwXmdioRead in buildInfo status %d.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OK_E;
		}

		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "buildInfo1:%u buildInfo2:%u buildInfo3:%u\n",
			buildInfo1, buildInfo2, buildInfo3);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MTD_OK;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugAPIC28GP4X2GetTxEqParam
 *
 * DESCRIPTION: print the value of the TX equalization parameter on the specified lane.
 *
 *APPLICABLE PHY:
 *                 MPD_TYPE_88E2540_E
 *                 MPD_TYPE_88X3540_E
 *                 MPD_TYPE_88E2580_E
 *                 MPD_TYPE_88X3580_E
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugAPIC28GP4X2GetTxEqParam (
	IN UINT_32                  rel_ifIndex,
	IN UINT_32	                lane
)
{
	MTD_STATUS             mtdStatus;
	MCESD_U32              paramValue;
	const char			  *params_str_arr[] = {"post", "peak", "pre", "margin"};
	UINT_8                 i;
	MCESD_DEV_PTR		   pSerdesDev;

	if (!prvMpdIsVoltronDevice(rel_ifIndex)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not Voltron device rel_ifIndex %d\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_SUPPORTED_E;
	}


	if(lane != 1 && lane != 0) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "lane can be 0 or 1 received %u \n", lane);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_SUPPORTED_E;
	}

	MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),  &pSerdesDev));

	for(i = C28GP4X2_TXEQ_EM_POST_CTRL; i <= C28GP4X2_TXEQ_MARGIN; i++) {
		mtdStatus = API_C28GP4X2_GetTxEqParam(pSerdesDev, lane, i , &paramValue);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                                sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                                "Some kind of error occurred in API_C28GP4X2_GetTxEqParam status %d.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		}
		else{
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                                sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                                "in lane: %d  %s value: %d \n", lane, params_str_arr[i], paramValue);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		}
	}

	return MCESD_OK;
}


/* ***************************************************************************
 * FUNCTION NAME: API_C28GP4X2_SetTxEqParam
 *
 * DESCRIPTION: Sets the TX equalization parameter to a value on the specified lane.
 *
 *APPLICABLE PHY:
 *                 MPD_TYPE_88E2540_E
 *                 MPD_TYPE_88X3540_E
 *                 MPD_TYPE_88E2580_E
 *                 MPD_TYPE_88X3580_E
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugAPIC28GP4X2SetTxEqParam (
	/*     INPUTS:             */
	IN UINT_32					rel_ifIndex,
	IN MPD_DEBUG_TXEQ_PARAM_E   txeq,
	IN UINT_32					lane,
	IN UINT_32					paramValue
)
{
	MTD_STATUS				mtdStatus;
	MCESD_DEV_PTR           pSerdesDev;
	BOOLEAN					isParmValid;
	E_C28GP4X2_TXEQ_PARAM   realParamIndex;
	BOOLEAN                 rc;
	UINT_32                 DataFromMDIO;
	UINT_16					mdioAddress =  prvMpdGetPortEntry(rel_ifIndex)->initData_PTR->mdioInfo.mdioAddress;

	if (!prvMpdIsVoltronDevice(rel_ifIndex)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not Voltron device rel_ifIndex %d\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_SUPPORTED_E;
	}

	MTD_ATTEMPT(mtdHwSerdesRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), PRV_MPD_SERDES_REGISTER_ADDRESS,  &DataFromMDIO));
	if (DataFromMDIO != 0x20E00000) {
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC()) (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													  "rel_ifIndex [%d]: mdioAddress [%d] value [0x%x] enable serdes", rel_ifIndex, mdioAddress, DataFromMDIO);
		MTD_ATTEMPT(mtdHwSerdesWrite(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), PRV_MPD_SERDES_REGISTER_ADDRESS,  0x20E00000));
	}
	MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),  &pSerdesDev));

	rc = prvMpdValidateParamValue(paramValue,txeq, rel_ifIndex);
	if(!rc){
		return MPD_OK_E;
	}
	isParmValid = prvMpdConvertMpdToMtdTxEq(txeq, &realParamIndex);
	if(!isParmValid){
		PRV_MPD_LOGGING_MAC("unknown paramIndex");
		return MPD_OK_E;
	}
	mtdStatus = API_C28GP4X2_SetTxEqParam(pSerdesDev,lane, realParamIndex, paramValue);
		if (mtdStatus != MTD_OK) {
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Some kind of error occurred in API_C28GP4X2_SetTxEqParam status %d.\n", mtdStatus);
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
			return MPD_OK_E;
		}

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugRegDumpHelper
 *
 * DESCRIPTION:
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdDebugRegDumpHelper(
    /*     INPUTS:             */
    UINT_16                      rel_ifIndex,
    MTD_reg_debug_STC           *reg_ARR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_32 i = 0, idx = 0, offset = 0;
    MTD_STATUS mtdStatus;
    MTD_8 *outputBuf;
    MTD_U16 startLocation = 0;

    while (reg_ARR[i].numOfRegs != 0) {
        outputBuf = (MTD_8*) PRV_MPD_ALLOC_MAC(MTD_SIZEOF_OUTPUT * reg_ARR[i].numOfRegs * sizeof(UINT_8));
        if (outputBuf) {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%s\n", reg_ARR[i].description_STR);
            PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

            mtdStatus = mtdCopyRegsToBuf(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                         reg_ARR[i].regs_ARR,
                                         reg_ARR[i].numOfRegs,
                                         outputBuf,
                                         reg_ARR[i].numOfRegs*MTD_SIZEOF_OUTPUT,
                                         &startLocation);

            if (mtdStatus == MTD_OK) {
                offset = 0;
                for (idx = 0; idx < reg_ARR[i].numOfRegs; idx++) {
                    PRV_MPD_LOGGING_MAC((char *)(outputBuf+offset));
                    offset += MTD_SIZEOF_OUTPUT;
                }
            }
            else {
                PRV_MPD_LOGGING_MAC("Some kind of error occurred.");
            }

            PRV_MPD_FREE_MAC(outputBuf);
        }

        i++;
        startLocation = 0;
    }

    return MPD_OK_E;
}

/*  3.D000.4:0 MDIO Address Strap Value (should match your MDIO address)
	3.D00D.3:0 DSP Revision/Chip Type (3 is X3580)
	3.D00D.6:4 Relative port (Range is 0���3 even on X3580 as upper and lower die are separate)
	3.D00D.9:7 Total ports (-1)
	1.C04E.6:4 Total ports (actual, not  -1, 0 indicates 8 ports)
	1.C04E.9:7 Relative port (Range is 0���7 for X3580, upper die is +4 of 3.D00D.6:4 on this chip)
 */
const MTD_MDIO_ADDR_TYPE prvMpdStrapRegData[] = {
    {31, 0xF000}, /*Mode Configuration*/
    {31, 0xF001}, /*Port Control*/
    {3, 0xD000}, /* phy internal */
	{3, 0xD00D}, /* phy internal */
	{1, 0xC04E}, /* phy internal */
    {1, 0x0000}, /*PMA/PMD Control 1*/
    {1, 0xC050}  /*Boot Status*/
};

const MTD_MDIO_ADDR_TYPE prvMpdLinkFlapRegData[] = {
        {1, 0x1},  /*PMA/PMD Status 1 - T unit*/
        {3, 0x1},  /*PCS Status 1 - T unit*/
        {4, 0x1001}, /*10GBASE-R PCS Status 1 - H unit*/
        {3, 0x16}, /*EEE Wake Error Counter*/
        {3, 0x20}, /*10GBASE-T PCS Status 1*/
        {3, 0x21}, /*10GBASE-T PCS Status 2 - bit error rate counter */
        {1, 0xC056}, /*10G Link Drop Count*/
        {1, 0x93},  /*10GBASE-T Fast Retrain Status and Control*/
        {1,0x0085}, /*10GBASE-T SNR Operating Margin Channel A*/
        {1,0x0086}, /*10GBASE-T SNR Operating Margin Channel B*/
        {1,0x0087}, /*10GBASE-T SNR Operating Margin Channel C*/
        {1,0x0088}, /*10GBASE-T SNR Operating Margin Channel D*/
        {1,0x0089}, /*10GBASE-T Minimum Margin Channel A*/
        {1,0x008A}, /*10GBASE-T Minimum Margin Channel B*/
        {1,0x008B}, /*10GBASE-T Minimum Margin Channel C*/
        {1,0x008C} /*10GBASE-T Minimum Margin Channel D*/
};

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugRegDump
 *
 * DESCRIPTION: print commonly used registers according to dump type (full, strap, link flapping)
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex,
    MPD_DEBUG_REGISTERS_ENT dumpType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    MTD_reg_debug_STC fullReg_ARR[] = {
        {"CUNIT", MTD_CUNIT_NUM_REGS, cUnitRegData}
        , {"F2R", MTD_F2R_NUM_REGS, f2rRegData}
        , {"HUNIT_10GBASER", MTD_HUNIT_10GBASER_NUM_REGS, hUnit10grRegData}
        , {"HUNIT_1000BX", MTD_HUNIT_1000BX_NUM_REGS, hUnit1000bxRegData}
        , {"HUNIT_COMMON", MTD_HUNIT_COMMON_NUM_REGS, hUnitCommonRegData}
        , {"HUNIT_RXAUI", MTD_HUNIT_RXAUI_NUM_REGS, hUnitRxauiRegData}
        , {"TUNIT_MMD1", MTD_TUNIT_MMD1_NUM_REGS, tUnitMmd1RegData}
        , {"TUNIT_MMD3", MTD_TUNIT_MMD3_NUM_REGS, tUnitMmd3RegData}
        , {"TUNIT_MMD3_2", MTD_TUNIT_MMD3_2_NUM_REGS, tUnitMmd3RegData2}
        , {"TUNIT_MMD3_3", MTD_TUNIT_MMD3_3_NUM_REGS, tUnitMmd3RegData3}
        , {"TUNIT_MMD7", MTD_TUNIT_MMD7_NUM_REGS, tUnitMmd7RegData}
        , {"XUNIT_10GBASER", MTD_XUNIT_10GBASER_NUM_REGS, xUnit10grRegData}
        , {"XUNIT_1000BX", MTD_XUNIT_1000BX_NUM_REGS, xUnit1000bxRegData}
        , {"XUNIT_COMMON", MTD_XUNIT_COMMON_NUM_REGS, xUnitCommonRegData}
        , {"", 0, NULL}
    };

    MTD_reg_debug_STC strapReg_ARR[] = {
        {"STRAP", sizeof(prvMpdStrapRegData)/sizeof(MTD_MDIO_ADDR_TYPE), prvMpdStrapRegData}
        , {"", 0, NULL}
    };

    MTD_reg_debug_STC linkFlapReg_ARR[] = {
        {"LinkFlap", sizeof(prvMpdLinkFlapRegData)/sizeof(MTD_MDIO_ADDR_TYPE), prvMpdLinkFlapRegData}
        , {"", 0, NULL}
    };

    if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
        return MPD_NOT_SUPPORTED_E;
    }

    switch (dumpType) {
    case MPD_DEBUG_REGISTERS_FULL_DUMP_E:
        prvMpdMtdDebugRegDumpHelper(rel_ifIndex, fullReg_ARR);
        break;
    case MPD_DEBUG_REGISTERS_STRAP_DUMP_E:
        prvMpdMtdDebugRegDumpHelper(rel_ifIndex, strapReg_ARR);
        break;
    case MPD_DEBUG_REGISTERS_LINK_FLAP_DEBUG_DUMP_E:
        prvMpdMtdDebugRegDumpHelper(rel_ifIndex, linkFlapReg_ARR);
        break;
    default:
        PRV_MPD_LOGGING_MAC("wrong reg dump type received.");
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugRunEchoTest
 *
 * DESCRIPTION: run echo test on RJ45, link partner should be powered down
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdDebugRunEchoTest(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MTD_BOOL testDone = MTD_FALSE;
    MTD_U16 results_ARR[20], i, speed_bits, failCount = 10, offset = 0;

    if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"received not MTD rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speed_bits);
    if (speed_bits & MTD_SPEED_10GIG_FD) {
        mtdStartEchoTest(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_SPEED_10GIG_FD);
    }
    else if (speed_bits & MTD_SPEED_5GIG_FD) {
        mtdStartEchoTest(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_SPEED_5GIG_FD);
    }
    else if (speed_bits & MTD_SPEED_2P5GIG_FD) {
        mtdStartEchoTest(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_SPEED_2P5GIG_FD);
    }
    else {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d speed not supported\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_OP_FAILED_E;
    }

    while (testDone == MTD_FALSE && failCount) {
        mtdIs10GEchoTestDone(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &testDone);
        PRV_MPD_SLEEP_MAC(500);
        failCount--;
    }

    if (!failCount) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d test not completed\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_OP_FAILED_E;
    }

    /* read results and return PHY to normal operation */
    mtdGetEchoTestResults(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), results_ARR);
    for (i=0; i < 20; i++) {
        offset += prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()+offset, sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC())-offset, "1.%x=%x ", (0xC013+i), results_ARR[i]);
    }

    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC() + offset, sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()) - offset,"\n");
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugConvertMtdCapabilitiesToString
 *
 * DESCRIPTION: convert MTD capabilities to string
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *capabilitiesString_PTR
)
{
    if (capabilities == MTD_ADV_NONE) {
        strcat(capabilitiesString_PTR, "none");
    }
    if (capabilities & MTD_SPEED_10M_HD) {
        strcat(capabilitiesString_PTR, "10M_HD ");
    }
    if (capabilities & MTD_SPEED_10M_FD) {
        strcat(capabilitiesString_PTR, "10M_FD ");
    }
    if (capabilities & MTD_SPEED_100M_HD) {
        strcat(capabilitiesString_PTR, "100M_HD ");
    }
    if (capabilities & MTD_SPEED_100M_FD) {
        strcat(capabilitiesString_PTR, "100M_FD ");
    }
    if (capabilities & MTD_SPEED_1GIG_HD) {
        strcat(capabilitiesString_PTR, "1G_HD ");
    }
    if (capabilities & MTD_SPEED_1GIG_FD) {
        strcat(capabilitiesString_PTR, "1G ");
    }
    if (capabilities & MTD_SPEED_2P5GIG_FD) {
        strcat(capabilitiesString_PTR, "2.5G ");
    }
    if (capabilities & MTD_SPEED_5GIG_FD) {
        strcat(capabilitiesString_PTR, "5G ");
    }
    if (capabilities & MTD_SPEED_10GIG_FD) {
        strcat(capabilitiesString_PTR, "10G ");
    }
    strcat(capabilitiesString_PTR, "\n\0");

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugGetInterruptsStatus
 *
 * DESCRIPTION: get interrupts information
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdDebugGetInterruptsStatus(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    MTD_STATUS mtdStatus;
    MTD_BOOL interruptTrig;
    MTD_INTR_STATUS interruptStatus;
    UINT_8 index = 1;
    BOOLEAN all = FALSE;
    static BOOLEAN legend = FALSE;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        all = TRUE;
    }
    else {
        index = rel_ifIndex;
    }

    if (!legend) {
        PRV_MPD_LOGGING_MAC("C-unit interrupt status T-unit interrupt status       H-unit interrupt status           X-unit interrupt status\n");
        PRV_MPD_LOGGING_MAC("----------------------- ----------------------------- --------------------------------- -----------------------\n");
        PRV_MPD_LOGGING_MAC("COMPHY_INTR     (BIT13) SYMBOL_ERR_INTR       (BIT21) == X2/X4 Interrupts               == 5G/10G BaseR Interrupts\n");
        PRV_MPD_LOGGING_MAC("TAI_INTR_BIT    (BIT12) FALSE_CARR_INTR       (BIT20) LINK_UP_TO_DN_INTR        (BIT19) BASER_LOCAL_FAULT_TX_INTR (BIT11)\n");
        PRV_MPD_LOGGING_MAC("PTP_INTR        (BIT11) TX_FIFO_OF_UF_INTR    (BIT17) LINK_DN_TO_UP_INTR        (BIT18) BASER_LOCAL_FAULT_RX_INTR (BIT10)\n");
        PRV_MPD_LOGGING_MAC("I2C_INTR        (BIT10) JABBER_INTR           (BIT16) FAULT_LINE_TO_CORE_INTR   (BIT9 ) BASER_LINK_STATUS_CHG_INTR(BIT2 )\n");
        PRV_MPD_LOGGING_MAC("RMFIFO_INTR     (BIT9 ) AN_ERR_INTR           (BIT15) LN1_ENG_DETECT_CHG_INTR   (BIT5 ) BASER_HIGH_BER_CHG_INTR   (BIT1 )\n");
        PRV_MPD_LOGGING_MAC("WOL_INTR        (BIT8 ) SPEED_CHG_INTR        (BIT14) LN0_ENG_DETECT_CHG_INTR   (BIT4 ) BASER_BLK_LOCK_CHG_INTR   (BIT0 )\n");
        PRV_MPD_LOGGING_MAC("TEMP_SENSOR_INTR(BIT7 ) DX_CHG_INTR           (BIT13) LN1_SYNC_CHG_INTR         (BIT1 ) == 1G/2.5GBASE-X Interrupts\n");
        PRV_MPD_LOGGING_MAC("F2R_MEM_INTR    (BIT6 ) PAGE_RX_INTR          (BIT12) LN0_SYNC_CHG_INTR         (BIT0 ) BASEX_SPEED_CHG_INTR      (BIT14)\n");
        PRV_MPD_LOGGING_MAC("MACSEC_INTR     (BIT5 ) AN_COMPLETED_INTR     (BIT11) == 5G/10G BaseR Interrupts        BASEX_DX_CHG_INTR         (BIT13)\n");
        PRV_MPD_LOGGING_MAC("PROC_INTR       (BIT4 ) LINK_STATUS_CHG_INTR  (BIT10) BASER_LOCAL_FAULT_TX_INTR (BIT11) BASEX_PAGE_RX_INTR        (BIT12)\n");
        PRV_MPD_LOGGING_MAC("GPIO_INTR       (BIT3 ) AN_MDI_CROS_CHG_INTR  (BIT6 ) BASER_LOCAL_FAULT_RX_INTR (BIT10) BASEX_AN_COMPLETED_INTR   (BIT11)\n");
        PRV_MPD_LOGGING_MAC("M_UNIT_INTR     (BIT2 ) DOWNSHIFT_INTR        (BIT5 ) BASER_LINK_STATUS_CHG_INTR(BIT2 ) BASEX_LINK_UP_TO_DN_INTR  (BIT10)\n");
        PRV_MPD_LOGGING_MAC("X_UNIT_INTR     (BIT1 ) ENG_DETECT_INTR       (BIT4 ) BASER_HIGH_BER_CHG_INTR   (BIT1 ) BASEX_LINK_DN_TO_UP_INTR  (BIT9 )\n");
        PRV_MPD_LOGGING_MAC("T_UNIT_INTR     (BIT0 ) FLP_XCHG_NO_LINK_INTR (BIT3 ) BASER_BLK_LOCK_CHG_INTR   (BIT0 ) BASEX_SYMBOL_ERR_INTR     (BIT8 )\n");
        PRV_MPD_LOGGING_MAC("                        POLARITY_CHG_INTR     (BIT1 ) == 1G/2.5GBASE-X Interrupts       BASEX_FALSE_CARR_INTR     (BIT7 )\n");
        PRV_MPD_LOGGING_MAC("                        TUNIT_TEMP_SENSOR_INTR(BIT0 ) BASEX_SPEED_CHG_INTR      (BIT14)\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_DX_CHG_INTR         (BIT13)\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_PAGE_RX_INTR        (BIT12)\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_AN_COMPLETED_INTR   (BIT11)\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_LINK_UP_TO_DN_INTR  (BIT10)\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_LINK_DN_TO_UP_INTR  (BIT9 )\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_SYMBOL_ERR_INTR     (BIT8 )\n");
        PRV_MPD_LOGGING_MAC("                                                      BASEX_FALSE_CARR_INTR     (BIT7 )\n");
        legend = TRUE;
    }

    PRV_MPD_LOGGING_MAC("        |         |          |          |          H UNIT INTERRUPTS                |    X UNIT INTERRUPTS \n");
    PRV_MPD_LOGGING_MAC(" port # |Triggered| C-unit   | T-unit   | baseR    | x2x4     | baseX    | misc     | baseR    | baseX    | misc     |\n");
    PRV_MPD_LOGGING_MAC("----------------------------------------------------------------------------------------------------------------------\n");
    do {

        /* supported for MTD phy types */
        if (!prvMpdIsMtdPhyType(index)) {
            continue;
        }

        mtdStatus = mtdGetPortIntrStatus(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &interruptTrig, &interruptStatus);
        if (mtdStatus == MTD_OK) {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                    sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                    "%-8d|%-9d|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|0x%-8x|\n",
                    index,
                    interruptTrig,
                    interruptStatus.cunitIntr,
                    interruptStatus.tunitIntr,
                    interruptStatus.hunitIntr.baseRIntr,   /* 10/5 GBASE-R */
                    interruptStatus.hunitIntr.x2x4Intr,    /* RXAUI/XAUI */
                    interruptStatus.hunitIntr.baseXIntr,   /* 1000BX/2.5GBX/SGMII */
                    interruptStatus.hunitIntr.miscIntr,
                    interruptStatus.xunitIntr.baseRIntr,   /*10/5 GBASE-R */
                    interruptStatus.xunitIntr.baseXIntr,   /* 1000BX/2.5GB/SGMII */
                    interruptStatus.xunitIntr.miscIntr);

            PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        }
    }
    while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugGetEeeInfo
 *
 * DESCRIPTION: print ports operational status of EEE
 *
 * note: if rel_ifIndex=0 print all ports information
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdDebugGetEeeInfo(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */

)
{
    UINT_8 eeeMode = 0, xgEnter = 0, xgExit = 0, gigEnter = 0, gigExit = 0, mEnter = 0, mExit = 0, ipgLen = 0, index = 1;
    UINT_16 latchedValue = 0, currentValue = 0, wakeErrCount = 0, EEE_support_bits = 0, EEE_advertise_bits = 0, LP_EEE_advertise_bits = 0, EEE_resolved_bits = 0;
    BOOLEAN all = FALSE;
    MTD_BOOL txlpiIndication = MTD_FALSE, rxlpiIndication = MTD_FALSE;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        all = TRUE;
    }
    else {
        index = rel_ifIndex;
    }

    PRV_MPD_LOGGING_MAC("\n");
    PRV_MPD_LOGGING_MAC(" port #|  EEE  | 10G          | 1G           | 100M         | IPG Len | support | advertise | LP advertise | resolved | TX LPI   | RX LPI   | wake Err\n");
    PRV_MPD_LOGGING_MAC("          Mode | Enter | Exit | Enter | Exit | Enter | Exit |           bits    | bits      | bits         | bits     |Indication|Indication| Count   \n");
    PRV_MPD_LOGGING_MAC("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    do {

        /* supported for MTD phy types */
        if (!prvMpdIsMtdPhyType(index)) {
            continue;
        }

        mtdGetEeeBufferConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(index),
                              &eeeMode,
                              &xgEnter,
                              &gigEnter,
                              &mEnter,
                              &xgExit,
                              &gigExit,
                              &mExit,
                              &ipgLen);

        mtdGetLPIStatus(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &latchedValue, &currentValue);
        mtdGetTxLPIIndication(PRV_MPD_MTD_OBJECT_MAC(index), currentValue, &txlpiIndication);
        mtdGetRxLPIIndication(PRV_MPD_MTD_OBJECT_MAC(index), currentValue, &rxlpiIndication);
        mtdWakeErrorCount(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &wakeErrCount);
        mtdGetEEESupported(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &EEE_support_bits);
        mtdGetAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &EEE_advertise_bits);
        mtdGetLP_Advert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &LP_EEE_advertise_bits);
        mtdGetEEEResolution(PRV_MPD_MTD_PORT_OBJECT_MAC(index), &EEE_resolved_bits);

        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                " %-6d| %-5d | %-5d | %-4d | %-5d | %-4d | %-5d | %-4d | %-7d | %-7d | %-9d | %-12d | %-8d | %-8s | %-8s | %-7d |\n",
                index, eeeMode, xgEnter, xgExit, gigEnter, gigExit, mEnter, mExit, ipgLen,
                EEE_support_bits, EEE_advertise_bits, LP_EEE_advertise_bits, EEE_resolved_bits,
                (txlpiIndication) ? "YES" : "NO", (rxlpiIndication) ? "YES" : "NO", wakeErrCount);

        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    }
    while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);

    return MPD_OK_E;

}

#endif
/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugConvertSpeedToString
 *
 * DESCRIPTION: convert MPD speed to string
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *speedString_PTR
)
{
    char * tmpSpeed_PTR;

    switch (speed) {
        case MPD_SPEED_10M_E   :
            tmpSpeed_PTR = "10M";
            break;
        case MPD_SPEED_100M_E  :
            tmpSpeed_PTR = "100M";
            break;
        case MPD_SPEED_1000M_E :
            tmpSpeed_PTR = "1G";
            break;
        case MPD_SPEED_10000M_E:
            tmpSpeed_PTR = "10G";
            break;
        case MPD_SPEED_2500M_E :
            tmpSpeed_PTR = "2.5G";
            break;
        case MPD_SPEED_5000M_E :
            tmpSpeed_PTR = "5G";
            break;
        case MPD_SPEED_20000M_E:
            tmpSpeed_PTR = "20G";
            break;
        case MPD_SPEED_100G_E  :
            tmpSpeed_PTR = "100G";
            break;
        case MPD_SPEED_50G_E   :
            tmpSpeed_PTR = "50G";
            break;
        case MPD_SPEED_25G_E   :
            tmpSpeed_PTR = "25G";
            break;
        case MPD_SPEED_200G_E  :
            tmpSpeed_PTR = "200G";
            break;
        case MPD_SPEED_400G_E  :
            tmpSpeed_PTR = "400G";
            break;
        case MPD_SPEED_LAST_E  :
        default:
            tmpSpeed_PTR = "INVALID";
    }
    if (speedString_PTR) {
        prvMpdSnprintf(speedString_PTR, sizeof(speedString_PTR), "%s", tmpSpeed_PTR);
    }

    return MPD_OK_E;

}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugPrintPortOperState
 *
 * DESCRIPTION: print ports operational status
 *
 * note: if rel_ifIndex=0 print all ports information
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugPrintPortOperState(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    UINT_32                         index = 1;
    BOOLEAN                         all = FALSE;
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    MPD_OPERATIONS_PARAMS_UNT phy_params;
    const char * mdixModeStr = "N/A", * phyKindStr = "N/A", * mediaTypeStr = "N/A", *phySpeedStr = "N/A";
    MPD_AUTONEG_CAPABILITIES_TYP capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
    BOOLEAN pauseCapable = FALSE, isSfpPresent = FALSE, isOperStatusUp = FALSE;
    MPD_SPEED_CAPABILITY_TYP enableBitmap = MPD_SPEED_CAPABILITY_UNKNOWN_CNS;
    UINT_16     temperature = 0;
    UINT_16         revision = 0;
    MPD_EEE_ENABLE_MODE_ENT  enableMode = MPD_EEE_ENABLE_MODE_LINK_CHANGE_E;
    MPD_RESULT_ENT status = MPD_OK_E;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        all = TRUE;
    }
    else {
        index = rel_ifIndex;
    }

    PRV_MPD_LOGGING_MAC("\n");
    PRV_MPD_LOGGING_MAC(" port #| link | speed  | media   |remote      | mdi/x | pause | phyKind |sfp    |Temperature|EEE       |EEE        |phy     |\n");
    PRV_MPD_LOGGING_MAC("                       | type    |capabilities|        Capable|          present| (Celsius) |Capability|enable mode|revision\n");
    PRV_MPD_LOGGING_MAC("------------------------------------------------------------------------------------------------------------------------------\n");

    memset(&phy_params, 0, sizeof(phy_params));

    do {
        portEntry_PTR = prvMpdGetPortEntry(index);
        if (portEntry_PTR == NULL) {
            continue;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            isOperStatusUp = phy_params.phyInternalOperStatus.isOperStatusUp;
            phySpeedStr = prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, phy_params.phyInternalOperStatus.phySpeed);
            mediaTypeStr = prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E, phy_params.phyInternalOperStatus.mediaType);
        }
        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            capabilities = phy_params.phyRemoteAutoneg.capabilities;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_MDIX_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            mdixModeStr = prvMpdDebugConvert(MPD_DEBUG_CONVERT_MDI_MODE_E, phy_params.phyMdix.mode);
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            pauseCapable = phy_params.phyLinkPartnerFc.pauseCapable;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            phyKindStr = prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_KIND_E, phy_params.phyKindAndMedia.phyKind);
            isSfpPresent = phy_params.phyKindAndMedia.isSfpPresent;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_TEMPERATURE_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            temperature = phy_params.phyTemperature.temperature;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            enableBitmap = phy_params.phyEeeCapabilities.enableBitmap;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            enableMode = phy_params.internal.phyEeeEnableMode.enableMode;
        }

        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           MPD_OP_CODE_GET_REVISION_E,
                                           &phy_params);
        if (status == MPD_OK_E) {
            revision = phy_params.phyRevision.revision;
        }

        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                " %-6d| %-4s | %-6s | %-7s | %-10d | %-5s | %-5s | %-7s | %-5s | %-9d | %-8d | %-9s | 0x%x \n",
                index,
                (isOperStatusUp) ? "up" : "down",
                phySpeedStr,
                mediaTypeStr,
                capabilities,
                mdixModeStr,
                (pauseCapable) ? "yes" : "no",
                phyKindStr,
                (isSfpPresent) ? "yes" : "no",
                temperature,
                enableBitmap,
                (enableMode == MPD_EEE_ENABLE_MODE_LINK_CHANGE_E) ? "link" : "admin",
                revision);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    }
    while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);
#ifdef MTD_PHY_INCLUDE
    prvMpdMtdDebugGetEeeInfo(rel_ifIndex);
    prvMpdMtdDebugGetInterruptsStatus(rel_ifIndex);
#endif
    PRV_MPD_LOGGING_MAC("\n\n");

    return MPD_OK_E;
}


/****************************************************************************
	* FUNCTION NAME: mpdValidateSetPhyProfileParams
	*
	* DESCRIPTION: validate rel_ifIndex, phy-type, mdio-bus, mdio-address before assigning to the phyDb
	*
	* Note: if one of the above params equals -1 (except rel_ifIndex) it will skip on the check
*****************************************************************************/
static BOOLEAN mpdValidateSetPhyProfileParams(
	/*!     INPUTS:  */
	UINT_32         rel_ifIndex,
	MPD_TYPE_ENT	phyType,
	UINT_8			mdioBus,
	UINT_8			mdioAddress
	/*!     INPUTS / OUTPUTS:   */
	/*!     OUTPUTS:            */
	)
{
	BOOLEAN			rc = TRUE;

	if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "rel_ifIndex [%d] is out of range", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		rc = FALSE;
	}

	if ((phyType >= MPD_TYPE_NUM_OF_TYPES_E) && (MPD_TYPE_NUM_OF_TYPES_E != phyType)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "rel_ifIndex %d - received %d which is not valid PHY type\n", rel_ifIndex, phyType);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		rc = FALSE;
	}

	if ((mdioAddress  > PRV_MPD_MAX_MDIO_ADRESS_CNS) && (PRV_MPD_PHY_SKIP_PARAMS_CNS != mdioAddress)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "rel_ifIndex %d - received %d which is not valid MDIO address\n", rel_ifIndex, mdioAddress);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		rc = FALSE;
	}

	if ((phyType == MPD_TYPE_88E1543_E) && (mdioAddress == 0)) {
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "rel_ifIndex %d - MDIO address 0 is not valid for this PHY\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		rc = FALSE;
	}

	if((mdioBus >3) && (PRV_MPD_PHY_SKIP_PARAMS_CNS != mdioBus)){
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                        sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                        "rel_ifIndex %d - Bus validity failed.\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		rc = FALSE;
	}


	return rc;
}


/****************************************************************************
	* FUNCTION NAME: prvMpdDebugSetPhyData
	*
	* DESCRIPTION: set PHY-type, mdio-address, mdio-bus, mdio-dev
	*
	* Note: if one of the above params equals -1 the function will not update it
	*		can initialize PHY driver and HW of the given rel_ifIndex if doInitSequence is set
*****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugUpdatePortDb(

	/*     INPUTS:             */
	UINT_32			rel_ifIndex,
	MPD_TYPE_ENT	phyType,
	UINT_8			mdioBus,
	UINT_8			mdioDev,
	UINT_8			mdioAddress,
	BOOLEAN			doInitSequence

	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	PRV_MPD_PORT_HASH_ENTRY_STC    *portEntry_PTR;
	BOOLEAN							changed = FALSE;
	MPD_RESULT_ENT					rc;

	if(mpdValidateSetPhyProfileParams(rel_ifIndex, phyType, mdioBus, mdioAddress) == FALSE){
		return MPD_OP_FAILED_E;
	}

	portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if(portEntry_PTR == NULL){
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"failed to get real_ifindex %d PHY", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		return MPD_NOT_FOUND_E;
	}

	if(PRV_MPD_PHY_SKIP_PARAMS_CNS != mdioAddress){
		portEntry_PTR->initData_PTR->mdioInfo.mdioAddress = mdioAddress;
		changed = TRUE;
	}
	if(PRV_MPD_PHY_SKIP_PARAMS_CNS != mdioBus){
		portEntry_PTR->initData_PTR->mdioInfo.mdioBus     = mdioBus;
		changed = TRUE;
	}
	if(PRV_MPD_PHY_SKIP_PARAMS_CNS != mdioDev){
		portEntry_PTR->initData_PTR->mdioInfo.mdioDev     = mdioDev;
		changed = TRUE;
	}
	if(PRV_MPD_PHY_SKIP_PARAMS_CNS != phyType){
		portEntry_PTR->initData_PTR->phyType              = phyType;
		changed = TRUE;
	}

	prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"real_ifindex %d PHY %s", rel_ifIndex,changed? "changed successfully":"didnt change");
	PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

	if(doInitSequence){
		prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Perform init sequence for real_ifindex %d\n", rel_ifIndex);
		PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

		rc = prvMpdInitPortHw(rel_ifIndex);

		if(rc != MPD_OK_E){
			prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Error:didnt init sequence\n");
			PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
		}
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugPrintPortDb
 *
 * DESCRIPTION: print ports DB
 *
 * note: if rel_ifIndex=0 print all ports information
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugPrintPortDb(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex,
    BOOLEAN    fullDb
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    UINT_32                         index = 1, indexParallel = 0, i = 0, offset = 0;
    BOOLEAN                         all = FALSE;
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    char parallelListArr[100] = {0};
#ifdef MTD_PHY_INCLUDE
    UINT_8 numPorts = 0, thisPort = 0;
    MTD_DEVICE_ID phyMtdRev = 0;
    char fwVersion[20];
    UINT_8                          major = 0, minor = 0, inc = 0, test = 0;
    MTD_STATUS                      mtdStatus = MTD_OK;
#else
    INT_32 phyMtdRev = 0;
#endif
    static BOOLEAN                  legend = FALSE;

    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        all = TRUE;
    }
    else {
        index = rel_ifIndex;
    }

    if (!legend) {
        PRV_MPD_LOGGING_MAC("\n");
        PRV_MPD_LOGGING_MAC(" port #->Rel_ifIndex.\n phy #->Phy number, represent the phy number which the port belong to.\n");
        PRV_MPD_LOGGING_MAC(" addr->MDIO address.\n busId->MDIO bus ID.\n isPrimary->Representative port.\n");
        PRV_MPD_LOGGING_MAC(" vct_offset->vct offset to get cable length.\n macOnPhyState->mac on phy bypass state.\n adminMode->Port Admin mode.\n");
        PRV_MPD_LOGGING_MAC(" usx type->PHY USX information.\n disableOnInit->if true PHY should be disabled on init.\n");
        PRV_MPD_LOGGING_MAC(" invertMdi->Is invert (mdi->mdix/mdix->mdi) required.\n doSwapAbcd->Swap pairs ABCD to DCBA.\n");
        PRV_MPD_LOGGING_MAC(" shortReachSerdes->Enable the SERDES short reach mode.\n opMode->Fiber port operation mode.\n");
        legend = TRUE;
    }

    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "\nMPD version:%s\n", MPD_CONST_TO_STR_MAC(MPD_GIT_VERSION));
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
#ifdef MTD_PHY_INCLUDE
    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "MTD version:%d.%d\n", MTD_API_MAJOR_VERSION, MTD_API_MINOR_VERSION);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
#endif

#ifdef MYD_PHY_INCLUDE
    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "MYD version:%d.%d\n", MYD_API_MAJOR_VERSION, MYD_API_MINOR_VERSION);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
#endif

    PRV_MPD_LOGGING_MAC("\n");
    PRV_MPD_LOGGING_MAC(" port #|  phy type  |  phyMtdRev | phy # | addr | dev | busId | Admin | speed | sfp present |    op mode   |  usx type  | transceiver| Duplex\n");
    PRV_MPD_LOGGING_MAC("                                                                                                               type\n");
    PRV_MPD_LOGGING_MAC("---------------------------------------------------------------------------------------------------------------------------------------------\n");
    do {
        portEntry_PTR = prvMpdGetPortEntry(index);
        if (portEntry_PTR == NULL) {
            continue;
        }
#ifdef MTD_PHY_INCLUDE
        if (prvMpdIsMtdPhyType(index)) {
            mtdGetPhyRevision(PRV_MPD_MTD_PORT_OBJECT_MAC(index),
                              &phyMtdRev,
                              &numPorts,
                              &thisPort);
        }
        else{
            phyMtdRev = 0;
        }
#endif
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(),
                sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),
                " %-6d| %-10s | %-10d |%-6d | %-4d | %-3d | %-5d | %-5s | %-5s | %-11s |%-14s| %-10s | %-11s| %-6s\n",
                index,
                (portEntry_PTR->initData_PTR->phyType < MPD_TYPE_NUM_OF_TYPES_E) ? prvMpdDebugConvertPhyTypeToText_ARR[portEntry_PTR->initData_PTR->phyType] : "INVALID",
                phyMtdRev,
                portEntry_PTR->initData_PTR->phyNumber,
                portEntry_PTR->initData_PTR->mdioInfo.mdioAddress,
                portEntry_PTR->initData_PTR->mdioInfo.mdioDev,
                portEntry_PTR->initData_PTR->mdioInfo.mdioBus,
                (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? "UP" : "DOWN",
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, portEntry_PTR->runningData_PTR->speed),
                portEntry_PTR->runningData_PTR->sfpPresent?"TRUE" : "FALSE",
                (portEntry_PTR->runningData_PTR->opMode < MPD_OP_MODE_LAST_E) ? prvMpdDebugConvertOpModeToText_ARR[portEntry_PTR->runningData_PTR->opMode] : "INVALID",
                (portEntry_PTR->initData_PTR->usxInfo.usxType < MPD_PHY_USX_MAX_TYPES) ? prvMpdDebugConvertUsxTypeToText_ARR[portEntry_PTR->initData_PTR->usxInfo.usxType] : "INVALID",
                (portEntry_PTR->initData_PTR->transceiverType < MPD_TRANSCEIVER_MAX_TYPES_E) ? prvMpdDebugConvertPhyTransceiverTypeToText_ARR[portEntry_PTR->initData_PTR->transceiverType] : "INVALID",
                (portEntry_PTR->runningData_PTR->duplex == MPD_DUPLEX_ADMIN_MODE_FULL_E) ? "FULL" : "HALF");
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    }
    while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);

    PRV_MPD_LOGGING_MAC("\n\n");

    if (fullDb == TRUE) {
        if (all == TRUE) {
            index = 1;
            indexParallel = 0;
        }
        else {
            index = rel_ifIndex;
            indexParallel = rel_ifIndex;
        }
        PRV_MPD_LOGGING_MAC(" port #| Invert | Swap | Disable |      mac on      |  vct   | ShortReach |\n");
        PRV_MPD_LOGGING_MAC("       |   Mdi  | Abcd | On Init |    phy state     | offset |  Serdes    |\n");
        PRV_MPD_LOGGING_MAC("---------------------------------------------------------------------------\n");
        do {
            portEntry_PTR = prvMpdGetPortEntry(index);
            if (portEntry_PTR == NULL) {
                continue;
            }
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "% -6d| %-5s | %-5s | %-7s | %-16s | %-6d | %-10s \n",
                    index,
                    (portEntry_PTR->initData_PTR->invertMdi) ? "TRUE" : "FALSE",
                    (portEntry_PTR->initData_PTR->doSwapAbcd) ? "TRUE" : "FALSE",
                    (portEntry_PTR->initData_PTR->disableOnInit) ? "TRUE" : "FALSE",
                    (portEntry_PTR->runningData_PTR->macOnPhyState <= PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E) ? prvMpdDebugConvertMacOnPhyToText_ARR[portEntry_PTR->runningData_PTR->macOnPhyState] : "INVALID",
                    portEntry_PTR->initData_PTR->vctOffset,
                    (portEntry_PTR->initData_PTR->shortReachSerdes) ? "TRUE" : "FALSE");
            PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        }
        while (all && index++ < MPD_MAX_PORT_NUMBER_CNS);

        PRV_MPD_LOGGING_MAC("\n\n");
        PRV_MPD_LOGGING_MAC(" *** FW db ***\n");
        PRV_MPD_LOGGING_MAC(" port #| isPrimary | Downloaded |   Version    |                                       Parallel list - mdioAddress                                    |\n");
        PRV_MPD_LOGGING_MAC("-------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        do {
            portEntry_PTR = prvMpdGetPortEntry(indexParallel);
            if (portEntry_PTR == NULL) {
                continue;
            }
            offset = 0;
            if (portEntry_PTR->runningData_PTR->phyFw_PTR != NULL) {
                for (i = 0; i<portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports; i++) {
                    offset += prvMpdSnprintf(parallelListArr+offset, sizeof(parallelListArr)-offset, "%d, ", portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[i]);
                }
#ifdef MTD_PHY_INCLUDE
                mtdStatus = mtdGetFirmwareVersion(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex),
                                                  &major,
                                                  &minor,
                                                  &inc,
                                                  &test);
                if (mtdStatus == MTD_OK) {
                    prvMpdSnprintf(fwVersion, sizeof(fwVersion), "%u.%u.%u.%u", major, minor, inc, test);

                    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "% -6d| %-9s | %-10s | %-12s | %-100s |\n",
                            indexParallel,
                            (portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE",
                            (portEntry_PTR->runningData_PTR->phyFw_PTR != NULL) ? (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded) ? "TRUE" : "FALSE" : "NO FW",
                            fwVersion,
                            parallelListArr);
                    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                }
#endif
            }
        }
        while (all && indexParallel++ < MPD_MAX_PORT_NUMBER_CNS);
    }

    PRV_MPD_LOGGING_MAC("\n\n");

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl45WriteRegister
 *
 * DESCRIPTION: relevant to 10G PHYs (Clause45)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl45WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    MPD_RESULT_ENT status = MPD_OK_E;

    if (prvMpdIs1GPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not 10G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);

    if (portEntry_PTR != NULL) {
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, device, address, PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, value);
        if (status == MPD_OK_E) {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d %d.0x%x=0x%x\n", rel_ifIndex, device, address, value);
        }
        else {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d Failed Cl45 write operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    }
    else {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl45ReadRegister
 *
 * DESCRIPTION: relevant to 10G PHYs (Clause45)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl45ReadRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    UINT_16 value;
    MPD_RESULT_ENT status = MPD_OK_E;

    if (prvMpdIs1GPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not 10G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR != NULL) {
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, device, address, &value);
        if (status == MPD_OK_E) {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d %d.0x%x=0x%x\n", rel_ifIndex, device, address, value);
        }
        else {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d Failed Cl45 read operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    }
    else {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return status;
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl22WriteRegister
 *
 * DESCRIPTION: relevant to 1G PHYs (Clause22)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl22WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    MPD_RESULT_ENT status = MPD_OK_E;
    BOOLEAN prev_state;

    if (!prvMpdIs1GPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not 1G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);

    if (portEntry_PTR != NULL) {
        /* 1G phy which support page select we need to disable PPU */
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, TRUE, &prev_state);
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, page, address, PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, value);
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, prev_state, &prev_state);
        if (status == MPD_OK_E) {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d 0x%x=0x%x\n", rel_ifIndex, address, value);
        }
        else {
            prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d Failed write operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    }
    else {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl22ReadRegister
 *
 * DESCRIPTION: relevant to 1G PHYs (Clause22)
 *
 * note: if readAll=TRUE print all registers of the current page
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl22ReadRegister(
    /*     INPUTS:             */
    BOOLEAN readAll,
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    UINT_16 value, traversePage = 0;
    BOOLEAN prev_state, readAllPages = FALSE;
    MPD_RESULT_ENT status = MPD_OP_FAILED_E;
    MPD_OPERATIONS_PARAMS_UNT phy_params;

    if (!prvMpdIs1GPhyType(rel_ifIndex)) {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()), "received not 1G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
        return MPD_NOT_SUPPORTED_E;
    }

    if (page > 0xFF) {
        readAllPages = TRUE;
    } else {
        traversePage = page;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR != NULL) {
        /* 1G phy which support page select we need to disable PPU */
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, TRUE, &prev_state);
        if (readAll) {
            do {
                /* moving to new page */
                phy_params.internal.phyPageSelect.page = traversePage;
                phy_params.internal.phyPageSelect.readPrevPage = TRUE;
                status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                   PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                   &phy_params);
                if (status == MPD_OP_FAILED_E) {
                    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d failed to set page %d\n", rel_ifIndex, traversePage);
                    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                    return MPD_OP_FAILED_E;
                }

                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"Page:  Offset RegVal Description [rel_ifIndex:%d]\n", rel_ifIndex);
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                PRV_MPD_LOGGING_MAC("-------------------------------------------------\n");
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 0, &value);
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 0, value, (traversePage <= 1) ? "Control Register" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 1, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 1, value, (traversePage <= 1) ? "Status Register" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 2, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 2, value, (traversePage <= 1) ? "PHY Identifier 1" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 3, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 3, value, (traversePage <= 1) ? "PHY Identifier 2" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 4, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 4, value, (traversePage <= 1) ? "Auto-Neg Advertisement" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 5, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 5, value, (traversePage <= 1) ? "LP Ability" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 6, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 6, value, (traversePage <= 1) ? "Auto-Neg Expansion" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 7, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 7, value, (traversePage <= 1) ? "Next Page Transmit" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 8, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 8, value, (traversePage <= 1) ? "LP Next Page" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 9, &value);   										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 9, value, (traversePage <= 1) ? "1000BASE-T Control" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 10, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 10, value, (traversePage <= 1) ? "1000BASE-T Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 11, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 11, value, (traversePage <= 1) ? "Reserved" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 12, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 12, value, (traversePage <= 1) ? "Reserved" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 13, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 13, value, (traversePage <= 1) ? "MMD Access Control" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 14, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 14, value, (traversePage <= 1) ? "MMD Access Address" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 15, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 15, value, (traversePage <= 1) ? "Extended Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 16, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 16, value, (traversePage <= 1) ? "PHY Specific Control" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 17, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 17, value, (traversePage <= 1) ? "PHY Specific Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 18, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 18, value, (traversePage <= 1) ? "Interrupt Enable" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 19, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 19, value, (traversePage <= 1) ? "Interrupt Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 20, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 20, value, (traversePage <= 1) ? "Extended PHY Specific" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 21, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 21, value, (traversePage <= 1) ? "Receive Error Count" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 22, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 22, value, "Page address");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 23, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 23, value, (traversePage <= 1) ? "Global Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 24, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 24, value, (traversePage <= 1) ? "LED Control" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 25, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 25, value, (traversePage <= 1) ? "Manual Led Override" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 26, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 26, value, (traversePage <= 1) ? "VCT Control" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 27, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 27, value, (traversePage <= 1) ? "VCT Status" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 28, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 28, value, (traversePage <= 1) ? "Specific Control 2" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 29, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 29, value, (traversePage <= 1) ? "Reserved" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 30, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 30, value, (traversePage <= 1) ? "Reserved" : "NA");
                PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());   																		  
                prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS, 31, &value);  										  
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"%2d :  %2d    0x%4x   %s\n",traversePage , 31, value, (traversePage <= 1) ? "Reserved" : "NA");

                /* moving to old page */
                phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
                phy_params.internal.phyPageSelect.readPrevPage = FALSE;
                status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                   PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                   &phy_params);
                if (status == MPD_OP_FAILED_E) {
                    prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d failed to set page %d\n", rel_ifIndex, phy_params.internal.phyPageSelect.prevPage);
                    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
                    return MPD_OP_FAILED_E;
                }
                traversePage++;
            } while (readAllPages && (traversePage <= 0xff));
        }
        else {
            status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, traversePage, address, &value);
            if (status == MPD_OK_E) {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d 0x%x=0x%x\n", rel_ifIndex, address, value);
            }
            else {
                prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d Failed read operation\n", rel_ifIndex);
                status = MPD_OP_FAILED_E;
            }
        }
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, prev_state, &prev_state);
    }
    else {
        prvMpdSnprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), sizeof(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()),"rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return status;
}


/*****************************************************************************
* FUNCTION NAME: prvMpdPtpGetDebugCounters
*
* DESCRIPTION: get debug counters:
*           PulseInCounter - The pulse trigger comes from TOD_CLK_IN or TOD_TRIG_IN based on register selection.
*           ClockInCounter - The alignment clock comes from TOD_CLK_IN or TOD_TRIG_IN
*           both counters should be clear on read
*
* APPLICABLE PHYS:
*                  MPD_TYPE_88E1781_E
*****************************************************************************/
extern MPD_RESULT_ENT prvMpdPtpGetDebugCounters(
    IN UINT_32 phyNumber,
    OUT PRV_MPD_PTP_COUNTERS_STC *counters_PTR
)
{
    MPD_RESULT_ENT status;
    UINT_32 data;

    status =  prvMpdPtpSupport(phyNumber);
    if (status != MPD_OK_E){
        return status;
    }
    if (counters_PTR == NULL) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "counters_PTR on phyNumber %d NULL pointer",
                                                      phyNumber);
        return MPD_OP_FAILED_E;
    }

    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE2, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    data &= 0xFF;
    counters_PTR->pulseInCounter = data;
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEE, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    counters_PTR->clockInCounter = data;
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), "phyNumber: %d, pulseInCounter: 0x%x, clockInCounter: 0x%x. \n", phyNumber, counters_PTR->pulseInCounter, counters_PTR->clockInCounter);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    /* clear registers after read */
    status = prvMpdPtpTaiRegisterWrite(phyNumber, 0xE2, 0XFF, 0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    status = prvMpdPtpTaiRegisterWrite(phyNumber, 0xEE, PRV_MPD_PTP_TAI_WRITE_ALL_MASK_CNS, 0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    return status;
}


/*****************************************************************************
* FUNCTION NAME: mpdDebugPtpTaiRegisterReadAll
*
* DESCRIPTION: print all ptp registers.
*
* APPLICABLE PHYS:
*                  MPD_TYPE_88E1781_E
*****************************************************************************/
extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterReadAll(
    IN UINT_32 phyNumber
)
{
    MPD_RESULT_ENT status;
    UINT_32 data;

    status =  prvMpdPtpSupport(phyNumber);
    if (status != MPD_OK_E){
        return status;
    }

    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), "TaiRegisterName         RegOffset     RegVal  [phy number:%d]\n", phyNumber);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    PRV_MPD_LOGGING_MAC("-------------------------      ---------   ---------\n");
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC0, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " TOD_0                  :  0x%x     :  0x%x \n", 0xC0, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC1, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " TOD_1                  :  0x%x     :  0x%x \n", 0xC1, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC2, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " TOD_2                  :  0x%x     :  0x%x \n", 0xC2, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC3, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " TOD_3                  :  0x%x     :  0x%x \n", 0xC3, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC4, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " FRC_0                  :  0x%x     :  0x%x \n", 0xC4, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC5, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " FRC_1                  :  0x%x     :  0x%x \n", 0xC5, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC6, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " FRC_2_lower            :  0x%x     :  0x%x \n", 0xC6, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC7, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " FRC_3                  :  0x%x     :  0x%x \n", 0xC7, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC8, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Drift_adj_cfg lower    :  0x%x     :  0x%x \n", 0xC8, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xC9, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_step_ns            :  0x%x     :  0x%x \n", 0xC9, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCA, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_step_fs            :  0x%x     :  0x%x \n", 0xCA, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCB, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cfg_gen            :  0x%x     :  0x%x \n", 0xCB, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCC, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Pulse_delay            :  0x%x     :  0x%x \n", 0xCC, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCD, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_tod_0         :  0x%x     :  0x%x \n", 0xCD, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCE, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_tod_1         :  0x%x     :  0x%x \n", 0xCE, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xCF, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_tod_2         :  0x%x     :  0x%x \n", 0xCF, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD0, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_tod_3         :  0x%x     :  0x%x \n", 0xD0, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD1, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_mask_0        :  0x%x     :  0x%x \n", 0xD1, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD2, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_mask_1        :  0x%x     :  0x%x \n", 0xD2, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD3, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_mask_2        :  0x%x     :  0x%x \n", 0xD3, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD4, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Trig_gen_mask_3        :  0x%x     :  0x%x \n", 0xD4, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD5, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_load_0             :  0x%x     :  0x%x \n", 0xD5, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD6, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_load_1             :  0x%x     :  0x%x \n", 0xD6, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD7, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_load_2             :  0x%x     :  0x%x \n", 0xD7, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD8, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_load_3             :  0x%x     :  0x%x \n", 0xD8, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xD9, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap0_0             :  0x%x     :  0x%x \n", 0xD9, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDA, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap0_1             :  0x%x     :  0x%x \n", 0xDA, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDB, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap0_2             :  0x%x     :  0x%x \n", 0xDB, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDC, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap0_3             :  0x%x     :  0x%x \n", 0xDC, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDD, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap1_0             :  0x%x     :  0x%x \n", 0xDD, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDE, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap1_1             :  0x%x     :  0x%x \n", 0xDE, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xDF, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap1_2             :  0x%x     :  0x%x \n", 0xDF, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE0, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap1_3             :  0x%x     :  0x%x \n", 0xE0, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE1, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_cap_cfg            :  0x%x     :  0x%x \n", 0xE1, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE2, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " pulse_in_cnt           :  0x%x     :  0x%x \n", 0xE2, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE3, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_func_cfg           :  0x%x     :  0x%x \n", 0xE3, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE4, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Pclk_cfg               :  0x%x     :  0x%x \n", 0xE4, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE5, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Pclk_cnt_0             :  0x%x     :  0x%x \n", 0xE5, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE6, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Pclk_cnt_1             :  0x%x     :  0x%x \n", 0xE6, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE7, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Drift_thr_cfg lower    :  0x%x     :  0x%x \n", 0xE7, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE8, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_0              :  0x%x     :  0x%x \n", 0xE8, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xE9, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_1              :  0x%x     :  0x%x \n", 0xE9, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEA, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_2_lower        :  0x%x     :  0x%x \n", 0xEA, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEB, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_3              :  0x%x     :  0x%x \n", 0xEB, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEC, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Clock_cyc              :  0x%x     :  0x%x \n", 0xEC, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xED, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Clk_delay              :  0x%x     :  0x%x \n", 0xED, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEE, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Clockk_in_cnt          :  0x%x     :  0x%x \n", 0xEE, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xEF, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " intr_set               :  0x%x     :  0x%x \n", 0xEF, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF0, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " intr_msk               :  0x%x     :  0x%x \n", 0xF0, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF1, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Old_tod_0              :  0x%x     :  0x%x \n", 0xF1, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF2, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Old_tod_1              :  0x%x     :  0x%x \n", 0xF2, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF3, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Old_tod_2              :  0x%x     :  0x%x \n", 0xF3, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF4, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Old_tod_3              :  0x%x     :  0x%x \n", 0xF4, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF5, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Drift_calc_lower       :  0x%x     :  0x%x \n", 0xF5, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF6, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Drift_mcand            :  0x%x     :  0x%x \n", 0xF6, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF7, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " drift_rslt             :  0x%x     :  0x%x \n", 0xF7, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF8, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Time_upd_cnt           :  0x%x     :  0x%x \n", 0xF8, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xF9, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Nsec_latency_eg        :  0x%x     :  0x%x \n", 0xF9, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFA, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Nsec_latency_ig        :  0x%x     :  0x%x \n", 0xFA, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFB, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_delayed_0      :  0x%x     :  0x%x \n", 0xFB, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFC, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_delayed_1      :  0x%x     :  0x%x \n", 0xFC, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFD, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_delayed_2      :  0x%x     :  0x%x \n", 0xFD, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFE, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Next_rt_delayed_3      :  0x%x     :  0x%x \n", 0xFE, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    status = prvMpdPtpTaiRegisterRead(phyNumber, 0xFF, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " Tod_step_patch 1       :  0x%x     :  0x%x \n", 0xFF, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());

    return status;
}


/*****************************************************************************
* FUNCTION NAME: mpdDebugPtpTaiRegisterRead
*
* DESCRIPTION: print ptp tai register.
*
* APPLICABLE PHYS:
*                  MPD_TYPE_88E1781_E
*****************************************************************************/
extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterRead(
    IN UINT_32 phyNumber,
    IN UINT_8  reg
)
{
    MPD_RESULT_ENT status;
    UINT_32 data;

    status =  prvMpdPtpSupport(phyNumber);
    if (status != MPD_OK_E){
        return status;
    }
    status = prvMpdPtpTaiRegisterRead(phyNumber, reg, &data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    sprintf(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC(), " PhyNumber %d, reg 0x%x : 0x%x \n", phyNumber, reg, data);
    PRV_MPD_LOGGING_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC());
    return status;
}


/*****************************************************************************
* FUNCTION NAME: mpdDebugPtpTaiRegisterWrite
*
* DESCRIPTION: set ptp tai register.
*
* APPLICABLE PHYS:
*                  MPD_TYPE_88E1781_E
*****************************************************************************/
extern MPD_RESULT_ENT mpdDebugPtpTaiRegisterWrite(
    IN UINT_32 phyNumber,
    IN UINT_8  reg,
    IN UINT_32  mask,
    IN UINT_32  data
)
{
    MPD_RESULT_ENT status;

    status =  prvMpdPtpSupport(phyNumber);
    if (status != MPD_OK_E){
        return status;
    }
    status = prvMpdPtpTaiRegisterWrite(phyNumber, reg, mask, data);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, phyNumber);
    return status;
}


/*****************************************************************************
* FUNCTION NAME: prvMpdDebugPerformPhyOperation
*
* DESCRIPTION:
*
*****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{

    BOOLEAN     debug_get, debug_set;
    PRV_MPD_OP_CODE_ENT prv_op_code;

    if (!PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC()) && !PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())) {
        return MPD_OK_E;
    }

    debug_set = before;
    debug_get = before?FALSE:TRUE;
    switch (op) {
        case MPD_OP_CODE_GET_MDIX_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_MDI_MODE_E, params_PTR->phyMdix.mode));
            }
            break;
        case MPD_OP_CODE_SET_AUTONEG_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. params: autoneg %s, capabilities (bitmap) %#x, set as %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyAutoneg.enable==MPD_AUTO_NEGOTIATION_ENABLE_E?"Enable":"Disable",
                                                                     params_PTR->phyAutoneg.capabilities,
                                                                     params_PTR->phyAutoneg.masterSlave==MPD_AUTONEGPREFERENCE_MASTER_E?"Master":"Slave");
            }
            break;
        case MPD_OP_CODE_SET_DUPLEX_MODE_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. params: duplex %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E?"FULL":"HALF");
            }
            break;
        case MPD_OP_CODE_SET_SPEED_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. params: speed %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
            }
            break;
        case MPD_OP_CODE_SET_MDIX_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. params: mdi/x mode %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_MDI_MODE_E, params_PTR->phyMdix.mode));
            }
            break;
        case MPD_OP_CODE_SET_VCT_TEST_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: result %d, cable length %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyVct.testResult,
                                                                     params_PTR->phyVct.cableLength);
            }
            break;
        case MPD_OP_CODE_GET_EXT_VCT_PARAMS_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: test type %d, result %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyExtVct.testType,
                                                                     params_PTR->phyExtVct.result);
            }
            break;
        case MPD_OP_CODE_GET_CABLE_LEN_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: cable length (enum) %d accurate length %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyCableLen.cableLength,
                                                                     params_PTR->phyCableLen.accurateLength);
            }
            break;
        case MPD_OP_CODE_SET_PHY_DISABLE_OPER_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. force link down %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyDisable.forceLinkDown);
            }
            break;
        case MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: capabilities (bitmap) %#x",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyRemoteAutoneg.capabilities);
            }
            break;
        case MPD_OP_CODE_SET_ADVERTISE_FC_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. advertise FC %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyFc.advertiseFc);
            }
            break;
        case MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyLinkPartnerFc.pauseCapable?"Capable":"Not Capable");
            }
            break;

        case MPD_OP_CODE_SET_POWER_MODULES_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. ED %s, SR %s, perform PHY reset %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerModules.energyDetetct),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerModules.shortReach),
                                                                     params_PTR->phyPowerModules.performPhyReset);
            }
            break;
        case MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyCableLenNoRange.cableLen);
            }
            break;
        case MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: kind: %s, SFP: %s, media type: %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_KIND_E, params_PTR->phyKindAndMedia.phyKind),
                                                                     params_PTR->phyKindAndMedia.isSfpPresent?"Present":"Not Present",
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E, params_PTR->phyKindAndMedia.mediaType));
            }
            break;
        case MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. SFP is %s. op_mode %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phySfpPresentNotification.isSfpPresent?"Present":"Not Present",
                                                                     params_PTR->phySfpPresentNotification.opMode);
            }
            break;
        case MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. speed %s, advertise %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E, params_PTR->phyEeeAdvertize.speedBitmap),
                                                                     params_PTR->phyEeeAdvertize.advEnable?"Enable":"Disable");
            }
            break;
        case MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. - %s",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyEeeMasterEnable.masterEnable?"Enable":"Disable");
            }
            break;
        case MPD_OP_CODE_GET_EEE_CAPABILITY_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: (bitmap) %#x",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyEeeCapabilities.enableBitmap);
            }
            break;
        case MPD_OP_CODE_GET_EEE_STATUS_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                    "rel_ifIndex %d, op:%s, phy type %s. returned: remote (bitmap) %#x, local %s",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    params_PTR->phyEeeStatus.enableBitmap,
                    params_PTR->phyEeeStatus.localEnable?"Enable":"Disable");
            }
            break;
        case MPD_OP_CODE_SET_LPI_EXIT_TIME_E:
        case MPD_OP_CODE_SET_LPI_ENTER_TIME_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. speed %s time_us %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E, params_PTR->phyEeeLpiTime.speed),
                                                                     params_PTR->phyEeeLpiTime.time_us);
            }
            break;
        case MPD_OP_CODE_SET_RESET_PHY_E:
            /* no parameters */
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s.",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType));
            }
            break;
        case MPD_OP_CODE_GET_TEMPERATURE_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                     "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
                                                                     portEntry_PTR->rel_ifIndex,
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                     prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                     params_PTR->phyTemperature.temperature);
            }
            break;
        case MPD_OP_CODE_GET_DTE_STATUS_E:
        	if (debug_get) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 params_PTR->phyDteStatus.detect);
        	}
            break;
        case MPD_OP_CODE_SET_DTE_E:
        	if (debug_set) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s. detect: %d",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 params_PTR->phyDteStatus.detect);
        	}
			break;
        case MPD_OP_CODE_GET_REVISION_E:
        	if (debug_get) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s. revision: %d",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 params_PTR->phyRevision.revision);
        	}
            break;
        case MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E:
        	if (debug_get) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s. phySpeed %s isOperStatusUp %s mediaType %s",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phyInternalOperStatus.phySpeed),
																	 (params_PTR->phyInternalOperStatus.isOperStatusUp) ? "up" : "down",
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E, params_PTR->phyInternalOperStatus.mediaType));
        	}
            break;
        case MPD_OP_PHY_CODE_SET_PTP_INIT_E:
        	if (debug_set) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s, phy Number %d. reason: %s time: frac %d nanoSeconds %d second low %d high %d",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 portEntry_PTR->initData_PTR->phyNumber,
																	 (params_PTR->phyPtpInit.reason == MPD_PTP_LOAD_TIME_FOR_PULSE_E) ? "pulse" : "pps",
																	 params_PTR->phyPtpInit.loadTime.fractionalNanoSeconds,
																	 params_PTR->phyPtpInit.loadTime.nanoSeconds,
																	 params_PTR->phyPtpInit.loadTime.lowSeconds,
																	 params_PTR->phyPtpInit.loadTime.highSeconds);
        	}
            break;
        case MPD_OP_CODE_GET_PTP_TSQ_E:
        	if (debug_get) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																	 "rel_ifIndex %d, op:%s, phy type %s. signature: 0x%x, tineStamp: 0x%x",
																	 portEntry_PTR->rel_ifIndex,
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																	 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																	 params_PTR->phyPtpTsq.signature,
																	 params_PTR->phyPtpTsq.timestamp);
        	}
            break;
        case MPD_OP_CODE_SET_MDIO_ACCESS_E:
        case MPD_OP_CODE_GET_VCT_CAPABILITY_E:
        case MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E:
        case MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E:
            break;
        case MPD_OP_CODE_SEND_IDLE_TO_HOST_E:
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                 "rel_ifIndex %d, op:%s, phy type %s. %s idles",
                                                                 portEntry_PTR->rel_ifIndex,
                                                                 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                 (params_PTR->phySendIdleToHost.sendIdle)? "Send" : "Stop");

            break;
        default:
            prv_op_code = (PRV_MPD_OP_CODE_ENT)op;
            switch (prv_op_code) {
			case PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E:
				if (debug_get) {
					PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																		 "rel_ifIndex %d, op:%s, phy type %s. input params: ED %s, SR %s, port is %s, speed is %s, returned: green consumption = %dMw",
																		 portEntry_PTR->rel_ifIndex,
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->internal.phyPowerConsumptions.energyDetetct),
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->internal.phyPowerConsumptions.shortReach),
																		 (params_PTR->internal.phyPowerConsumptions.portUp)?"Up":"Down",
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PORT_SPEED_E, params_PTR->internal.phyPowerConsumptions.portSpeed),
																		 params_PTR->internal.phyPowerConsumptions.greenConsumption);
				}
				break;
			case PRV_MPD_OP_CODE_GET_GREEN_READINESS_E:
				if (debug_get) {
					PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
																		 "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
																		 portEntry_PTR->rel_ifIndex,
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																		 prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_READINESS_E, params_PTR->internal.phyGreen.readiness));
				}
            break;
			case PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E:
                    if (debug_get) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. returned: %d, %d, %d, %d",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[0],
                                                                             params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[1],
                                                                             params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[2],
                                                                             params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[3]);
                    }
                    break;
                case PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E:
                case PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E:
                    if (debug_set) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. params: fiber is %s, phySpeed is %s, media_type is %s",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             params_PTR->internal.phyFiberParams.fiberPresent?"Present":"Not Present",
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PORT_SPEED_E, params_PTR->internal.phyFiberParams.phySpeed),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E, params_PTR->internal.phyFiberParams.mediaType));
                    }
                    break;
                case PRV_MPD_OP_CODE_SET_LOOP_BACK_E:
                    if (debug_set) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                            "rel_ifIndex %d, op:%s, phy type %s. params: enable: %s, loopback mode is %d.",
                            portEntry_PTR->rel_ifIndex,
                            prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                            prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                            params_PTR->internal.phyLoopback.enable? "TRUE":"FALSE",
                            params_PTR->internal.phyLoopback.loopbackMode);
                    }
                    break;
                case PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E:
                case PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E:
                case PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E:
                case PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E:
                case PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E:
                case PRV_MPD_OP_CODE_SET_ERRATA_E:
                case PRV_MPD_OP_CODE_GET_LANE_BMP_E:
                case PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E:
                case PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E:
                case PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E:
                case PRV_MPD_NUM_OF_OPS_E:
				case PRV_MPD_OP_CODE_GET_I2C_READ_E:
                case PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E:
                    /* no debug*/
                    break;
                case PRV_MPD_OP_CODE_INIT_E:
                    if (debug_set) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s %s",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
																			 params_PTR->internal.phyInit.initializeDb?"Initialize DB":"HW init");
                    }
                    break;
                case PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E:
                    if (debug_set) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. enable: %s",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             params_PTR->internal.phyFastLinkDown.enable);
                    }
                    break;
                case PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E:
                    if (debug_get) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. EEE enable mode %s",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             (params_PTR->internal.phyEeeEnableMode.enableMode == MPD_EEE_ENABLE_MODE_LINK_CHANGE_E)?"On Link Change event":"On Admin only");
                    }
                    break;
                case PRV_MPD_OP_CODE_GET_VCT_OFFSET_E:
                    if (debug_get) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. do_set %d",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             params_PTR->internal.phyVctOffset.do_set);
                    }
                    break;
                case PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E:
                    if (debug_get) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC())(PRV_MPD_OP_TO_TEXT_MAC(portEntry_PTR->initData_PTR->phyType, op),
                                                                             "rel_ifIndex %d, op:%s, phy type %s. media select %d",
                                                                             portEntry_PTR->rel_ifIndex,
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                                                                             params_PTR->internal.phyMediaSelect.value);
                    }
                    break;
            }

    }
    return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdDebugInit(
    void
)
{
	/* bind debug */

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugWriteFlagName,
						   prvMpdDebugWriteFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugReadFlagName,
						   prvMpdDebugReadFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_READ_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugConfigVctFlagName,
						   prvMpdDebugConfigVctFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_VCT_CFG_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugSfpPckName,
						   prvMpdDebugSfpFlagName,
						   prvMpdDebugSfpFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SFP_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugErrorFlagName,
						   prvMpdDebugErrorFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugTraceFlagName,
						   prvMpdDebugTraceFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugOperationGetFlagName,
						   prvMpdDebugOperationGetFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugOperationSetFlagName,
						   prvMpdDebugOperationSetFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC()));

	PRV_MPD_DEBUG_BIND_MAC(prvMpdDebugComponentName,
						   prvMpdDebugPckName,
						   prvMpdDebugReducedFlagName,
						   prvMpdDebugReducedFlagHelp,
						   &(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_REDUCDED_CONFIG_MAC()));

	if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_REDUCDED_CONFIG_MAC())) {
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_RESET_PHY_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_PHY_DISABLE_OPER_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_ADVERTISE_FC_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_POWER_MODULES_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_LPI_EXIT_TIME_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_LPI_ENTER_TIME_E] = TRUE;
		PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()[MPD_OP_CODE_SET_DTE_E] = TRUE;
	}

    return MPD_OK_E;
}


#else

extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *speedString_PTR
)
{
    MPD_UNUSED_PARAM(speed);
    MPD_UNUSED_PARAM(speedString_PTR);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *capabilitiesString_PTR
)
{
    MPD_UNUSED_PARAM(capabilities);
    MPD_UNUSED_PARAM(capabilitiesString_PTR);
    return MPD_OK_E;
}
extern MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(op);
    MPD_UNUSED_PARAM(params_PTR);
    MPD_UNUSED_PARAM(before);
    return MPD_OK_E;
}
extern const char * prvMpdDebugConvert(
    /*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   conv_type,
    UINT_32                 value
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(conv_type);
    MPD_UNUSED_PARAM(value);
    return "";
}

extern MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex,
    MPD_DEBUG_REGISTERS_ENT dumpType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(dumpType);
    return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdMtdDebugGetEye(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdMtdDebugGetInterruptsStatus(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdDebugInit(
    void
)
{
	return MPD_OK_E;
}

#endif
