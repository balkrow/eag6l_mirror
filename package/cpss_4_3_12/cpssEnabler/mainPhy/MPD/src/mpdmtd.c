/* ****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdMtd.c
 *  @brief PHY and driver specific PHY implementation.
 *  For all (supported) MTD PHYs
 *
 */
#include  	<mpdPrefix.h>
#include    <mpdTypes.h>
#include    <mpd.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>

#include    <mtdApiTypes.h>
#include    <mtdAPI.h>
#include    <mtdAPIInternal.h>
#include    <mtdApiRegs.h>
#include    <mtdCunit.h>
#include    <mtdDiagnostics.h>
#include    <mtdEEE.h>
#include    <mtdFwDownload.h>
#include    <mtdHunit.h>
#include    <mtdHwCntl.h>
#include    <mtdHXunit.h>
#include    <mtdXgFastRetrain.h>
#include    <mtdXunit.h>
#include    <mtdInitialization.h>
#include    <fwimages/mtdFwDlSlave.h>
#include    <mtdHwSerdesCntl.h>
#include    <serdes/mcesd/mcesdTop.h>
#include    <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include    <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.h>
#include    <mtdUtils.h>

/** DEBUG **/
static const char prvMpdMtdCompName[] = "mtd";
static const char prvMpdMtdPckName[] = "wrap";

static const char prvMpdMtdFwFlagName[] = "fw";
static const char prvMpdMtdFwFlagHelp[] = "debug FW load sequence";

static const char prvMpdMtdVctName[] = "vct";
static const char prvMpdMtdVctHelp[] = "debug VCT sequence";

static const char prvMpdMtdLinkName[] = "link";
static const char prvMpdMtdLinkHelp[] = "debug link change sequence";

static const char prvMpdMtdEeeName[] = "eee";
static const char prvMpdMtdEeeHelp[] = "debug SF EEE";

static const char prvMpdMtdConfigName[] = "config";
static const char prvMpdMtdConfigHelp[] = "debug SF config sequence";

static const char prvMpdMtdMtdPckName[] = "mtd";
static const char prvMpdMtdInfoName[] = "info";
static const char prvMpdMtdInfoHelp[] = "MTD informational";

static const char prvMpdMtdCriticalName[] = "critical";
static const char prvMpdMtdCriticalHelp[] = "MTD critical info";

static const char prvMpdMtdErrorName[] = "error";
static const char prvMpdMtdErrorHelp[] = "MTD Error";


static const PRV_MPD_MTD_TO_MPD_CONVERT_STC prvMpdConvertMtdToMpdSpeed[] = {
    {
        MTD_SPEED_10M_HD,
        MPD_AUTONEG_CAPABILITIES_TENHALF_CNS
    },
    {
        MTD_SPEED_10M_FD,
        MPD_AUTONEG_CAPABILITIES_TENFULL_CNS
    },
    {
        MTD_SPEED_100M_HD,
        MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS
    },
    {
        MTD_SPEED_100M_FD,
        MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS
    },
    {
        MTD_SPEED_1GIG_HD,
        MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS
    },
    {
        MTD_SPEED_1GIG_FD,
        MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS
    },
    {
        MTD_SPEED_10GIG_FD,
        MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS
    },
    {
        MTD_SPEED_2P5GIG_FD,
        MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS
    },
    {
        MTD_SPEED_5GIG_FD,
        MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS
    }
};

static const PRV_MPD_MTD_TO_MPD_CONVERT_STC prvMpdConvertMtdToMpdEeeCapability[] = {
    {
        MTD_EEE_100M,
        MPD_SPEED_CAPABILITY_100M_CNS
    },
    {
        MTD_EEE_1G,
        MPD_SPEED_CAPABILITY_1G_CNS
    },
    {
        MTD_EEE_10G,
        MPD_SPEED_CAPABILITY_10G_CNS
    },
    {
        MTD_EEE_2P5G,
        MPD_SPEED_CAPABILITY_2500M_CNS
    },
    {
        MTD_EEE_5G,
        MPD_SPEED_CAPABILITY_5G_CNS
    }
};

#ifndef PHY_SIMULATION
static const char * prvMpdMtdConvertPhyNameToText_ARR[MPD_TYPE_NUM_OF_TYPES_E] = {
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "PHY-88X32x0",
    "PHY-88X33x0",
    "PHY-88X20X0",
    "PHY-88X2180",
    "PHY-88E2540",
    "PHY-88X3540",
    "NO FW",
    "PHY-88E2580",
    "PHY-88X3580",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW",
    "NO FW"
};
#endif

#ifdef PHY_SIMULATION
#define PRV_MPD_MTD_CALL(_ret,_rel_ifIndex, _status)                                                \
    _status = _ret;                                                                                 \
    PRV_MPD_DEBUG_LOG_PORT_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC(),_rel_ifIndex)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),  \
        "%s returned with status %d line %d",MPD_STR_MAC(_ret), _status, __LINE__ );
#else
#define PRV_MPD_MTD_CALL(_ret,_rel_ifIndex, _status)                                                \
    _status = _ret;                                                                                 \
    PRV_MPD_DEBUG_LOG_PORT_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC(),_rel_ifIndex)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),  \
        "%s returned with status %d line %d",MPD_STR_MAC(_ret), _status, __LINE__ );                    \
    if (_status == MTD_FAIL){                                                                       \
        PRV_MPD_HANDLE_FAILURE_MAC(_rel_ifIndex, MPD_ERROR_SEVERITY_ERROR_E, MPD_STR_MAC(_ret));        \
        return MPD_OP_FAILED_E;                                                                     \
    }
#endif

/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdIsPhyReady
 *
 * DESCRIPTION: check if phy is ready after firmware download (max timeout 3 seconsds)
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdIsPhyReady(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC            *portEntry_PTR,
    PRV_MPD_PHY_READY_SEQUENCE_ENT          phySequence
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT ret_val = MPD_OK_E;
#ifndef PHY_SIMULATION
    MTD_BOOL phyReady = MTD_FALSE, is_tunit_ready;
    UINT_32 rel_ifIndex, i;
    MTD_BOOL appStarted = MTD_FALSE;
    UINT_32 try, max_num_of_tries = 30;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (phySequence == PRV_MPD_PHY_READY_WITH_SW_RESET_E) {
		/* T unit can't be ready in fiber only mode */
		if (portEntry_PTR->runningData_PTR->sfpPresent) {
			portEntry_PTR->runningData_PTR->softwareResetRequired = TRUE;
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
					"rel_ifIndex [%d] SFP present need to skip SW reset, will be done when fiber cable removed",
					rel_ifIndex);
			return ret_val;
		}
    	/* in case we are in prefer fiber mode, the DB might not be updated yet.
			if we are in active media fiber T unit will not be ready for use */
		mtdStatus = mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady);
		if (mtdStatus != MTD_OK) {
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  "rel_ifIndex %d, mtdIsPhyReadyAfterReset FAIL!",
								  rel_ifIndex);
		}
		if (phyReady) {
			mtdStatus = mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 500);
			if (mtdStatus != MTD_OK) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
					"rel_ifIndex %d, mtdSoftwareReset FAIL!",
					rel_ifIndex);
				portEntry_PTR->runningData_PTR->softwareResetRequired = TRUE;
			}
			else {
				portEntry_PTR->runningData_PTR->softwareResetRequired = FALSE;
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
							 "rel_ifIndex [%d] mtdSoftwareReset applied",
							 rel_ifIndex);
				/* might fail if T-unit is in low power mode due to AMD (auto media detect) decision */
				for (i = 0; i < 50; i++) {
					PRV_MPD_MTD_CALL(mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &is_tunit_ready),
									 rel_ifIndex,
									 mtdStatus);
					if (is_tunit_ready == MTD_TRUE) {
						break;
					}
					PRV_MPD_SLEEP_MAC(10);
				}
				if (is_tunit_ready == MTD_FALSE) {
					PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex %d, T Unit is not ready, probably active media is fiber.\n",
						rel_ifIndex);
				}
			}
		}
		else {
			portEntry_PTR->runningData_PTR->softwareResetRequired = TRUE;
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														 "rel_ifIndex [%d] T unit is not ready",
														 rel_ifIndex);
		}
    	return ret_val;
    }

    if (phySequence == PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E) {
        for (try = 1; try <= max_num_of_tries; try++) {
			PRV_MPD_MTD_CALL(mtdDidPhyAppCodeStart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &appStarted),
							 rel_ifIndex,
							 mtdStatus);
			if (appStarted == MTD_TRUE) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														 "rel_ifIndex %d, PHY apps start after FW download, try number %d",
														 rel_ifIndex,
														 try);
				break;
			}

			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													 "rel_ifIndex %d, PHY apps didn't start after FW download, try number %d",
													 rel_ifIndex,
													 try);
			PRV_MPD_SLEEP_MAC(100);
        }
		if (try > max_num_of_tries) {
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													 "rel_ifIndex %d, PHY apps didn't start after FW download, after %d tries",
													 rel_ifIndex,
													 max_num_of_tries);
			PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
									   MPD_ERROR_SEVERITY_ERROR_E,
									   "PHY apps didn't start after FW download\n");
			prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
			ret_val = MPD_OP_FAILED_E;
		}
	}

	/* T unit can't be ready in fiber only mode */
	if (portEntry_PTR->runningData_PTR->sfpPresent) {
		return ret_val;
	}
	for (try = 1; try <= max_num_of_tries; try++) {
		PRV_MPD_MTD_CALL(mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady),
						 rel_ifIndex,
						 mtdStatus);
		if (phyReady == MTD_TRUE) {
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													 "rel_ifIndex %d, PHY ready after reset, try number %d",
													 rel_ifIndex,
													 try);
			break;
		}
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												 "rel_ifIndex %d, PHY not ready after reset, try number %d",
												 rel_ifIndex,
												 try);
		PRV_MPD_SLEEP_MAC(100);
	}
	if (try > max_num_of_tries) {
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												 "rel_ifIndex %d, PHY not ready after reset, after %d tries",
												 rel_ifIndex,
												 max_num_of_tries);
		PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
								   MPD_ERROR_SEVERITY_ERROR_E,
								   "PHY not ready after reset\n");
		prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);

		ret_val = MPD_OP_FAILED_E;
	}
#endif
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(phySequence);
    return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDTEConfig
 *
 * DESCRIPTION: Enable/Disable DTE functionality
 *
 * Note: 1. when DTE feature enable, speed 10M is not supported
 *		 2. It takes about 5 seconds for the PHY to identify the legacy device
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *                  MPD_TYPE_88E3580_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDTEConfig(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{

	MTD_STATUS mtdStatus;
	BOOLEAN enable = params_PTR->phyDteStatus.detect;
	UINT_16 value = (enable)?MTD_BIT_7:0;
	UINT_32 rel_ifIndex;
	MPD_RESULT_ENT ret_val = MPD_OK_E;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

	/* configure DTE admin enable, we be enabled on port admin up when software reset will be applied */
	PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, MTD_T_UNIT_PMA_PMD, 0xC034, MTD_BIT_7, value),
		rel_ifIndex,
		mtdStatus);

	/* required software reset to take affect - if T unit is not ready then it will be done on present notification */
	ret_val = prvMpdMtdIsPhyReady(portEntry_PTR,PRV_MPD_PHY_READY_WITH_SW_RESET_E);
	if (ret_val != MPD_OK_E) {
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												 "rel_ifIndex %d, phy not ready after PRV_MPD_PHY_READY_WITH_SW_RESET_E!",
												 rel_ifIndex);
	}

	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
		"set DTE on rel_ifIndex %d. DTE is %s",
		rel_ifIndex,
		(enable) ? "enabled" : "disabled");


	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetDTEStatus
 *
 * DESCRIPTION: Get DTE Status
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *                  MPD_TYPE_88E3580_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetDTEStatus(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16 value;
	MTD_STATUS mtdStatus;
	UINT_32 rel_ifIndex;

	rel_ifIndex = portEntry_PTR->rel_ifIndex;

		/* enable DTE */
		PRV_MPD_MTD_CALL(   prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, MTD_T_UNIT_PMA_PMD, 0xC035, &value),
			rel_ifIndex,
			mtdStatus);


	params_PTR->phyDteStatus.detect = (value&MTD_BIT_13)?TRUE:FALSE;

	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
	                                                     "rel_ifIndex %d. DTE %s",
	                                                     rel_ifIndex,
														 (params_PTR->phyDteStatus.detect) ? "detected" : "not detected");

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdValidateUsxSpeeds
 *
 * DESCRIPTION: validate speeds appropriate to usx mode.
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *                  MPD_TYPE_88E3580_E
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdValidateUsxSpeeds(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MTD_U16 mtdSpeed
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_PHY_USX_TYPE_ENT usxType;

    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE) {
        return MPD_OK_E;
    }

    switch (usxType) {
        case MPD_PHY_USX_TYPE_OXGMII:
        case MPD_PHY_USX_TYPE_10G_QXGMII:
            /* The appropriate speed is up to MTD_SPEED_2P5GIG_FD */
            if ((mtdSpeed == MTD_SPEED_5GIG_FD) || (mtdSpeed == MTD_SPEED_10GIG_FD)) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d desired speed 0x%x doesn't match usx mode capabilities",
                                                             portEntry_PTR->rel_ifIndex,
                                                             mtdSpeed);
                return MPD_OP_FAILED_E;
            }
            break;
        case MPD_PHY_USX_TYPE_10G_DXGMII:
        case MPD_PHY_USX_TYPE_20G_QXGMII:
            /* The appropriate speed is up to MTD_SPEED_5GIG_FD */
            if (mtdSpeed == MTD_SPEED_10GIG_FD) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d desired speed 0x%x doesn't match usx mode capabilities",
                                                             portEntry_PTR->rel_ifIndex,
                                                             mtdSpeed);
                return MPD_OP_FAILED_E;
            }
            break;
        default:
            break;
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvConvertMpdUsxModeToMtdUsxMode
 *
 * DESCRIPTION: vconvert MPD usx mode to MTD usx mode.
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88E2580_E
 *****************************************************************************/
static MPD_RESULT_ENT prvConvertMpdUsxModeToMtdUsxMode(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    MTD_U16                    * mtdUsxType_PTR
)
{
    MPD_PHY_USX_TYPE_ENT usxType;
    MTD_U16              macType;

    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE) {
        return MPD_OK_E;
    }

    if (mtdUsxType_PTR == NULL) {
        return MPD_OP_FAILED_E;
    }

    switch (usxType) {
        case MPD_PHY_USX_TYPE_SXGMII:
            macType = MTD_MAC_TYPE_SXGMII;
            break;
        case MPD_PHY_USX_TYPE_OXGMII:
            macType = MTD_MAC_TYPE_OXGMII;
            break;
        case MPD_PHY_USX_TYPE_10G_QXGMII:
            macType = MTD_MAC_TYPE_10G_QXGMII;
            break;
        case MPD_PHY_USX_TYPE_10G_DXGMII:
            macType = MTD_MAC_TYPE_10G_DXGMII;
            break;
        case MPD_PHY_USX_TYPE_20G_QXGMII:
            macType = MTD_MAC_TYPE_20G_QXGMII;
            break;
        case MPD_PHY_USX_TYPE_20G_DXGMII:
            macType = MTD_MAC_TYPE_20G_DXGMII;
            break;
        default:
            return MPD_OP_FAILED_E;
            break;
    }

    *mtdUsxType_PTR = macType;

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugApi
 *
 * DESCRIPTION: enable MTD debugs
 *
 *
 *****************************************************************************/
void prvMpdMtdDebugApi(
    MTD_U16 debugLevel,
    char *str
)
{
     MPD_UNUSED_PARAM(debugLevel);
    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())) {
        PRV_MPD_LOGGING_MAC(str);
    }
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInitDebug
 *
 * DESCRIPTION: ALL MTD
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdInitDebug(
    /*     INPUTS:             */
    void
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdPckName,
                           prvMpdMtdFwFlagName,
                           prvMpdMtdFwFlagHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdPckName,
                           prvMpdMtdVctName,
                           prvMpdMtdVctHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdPckName,
                           prvMpdMtdLinkName,
                           prvMpdMtdLinkHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_LINK_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdPckName,
                           prvMpdMtdEeeName,
                           prvMpdMtdEeeHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdPckName,
                           prvMpdMtdConfigName,
                           prvMpdMtdConfigHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdMtdPckName,
                           prvMpdMtdInfoName,
                           prvMpdMtdInfoHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_INFO_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdMtdPckName,
                           prvMpdMtdCriticalName,
                           prvMpdMtdCriticalHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CRITICAL_ACCESS_MAC()));

    PRV_MPD_DEBUG_BIND_MAC(prvMpdMtdCompName,
                           prvMpdMtdMtdPckName,
                           prvMpdMtdErrorName,
                           prvMpdMtdErrorHelp,
                           &(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC()));


#ifdef MTD_DEBUG
#ifdef MTD_DEBUG_FUNC
    mtdSetDbgLogFunc(prvMpdMtdDebugApi);
#endif
#endif
    return MPD_OK_E;

}


/*****************************************************************************
* FUNCTION NAME: prvMpdMtdGetMediaType
*
* DESCRIPTION: get the media type select according to fiber present or not
*
*                  MPD_TYPE_88X32x0_E
*
* note: due to a bug in the PHY we do not use copper only mode (on some ports there is no traffic)
*****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMediaType(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MTD_U16 mediaSelect = MTD_MS_CU_ONLY;
    UINT_32 rel_ifIndex;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if ((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) ||
        (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E && portEntry_PTR->runningData_PTR->sfpPresent)) {
        mediaSelect = MTD_MS_FBR_ONLY;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d. Fiber mode",
                                                     rel_ifIndex);
    }
    else {
        mediaSelect = MTD_MS_AUTO_PREFER_FBR;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d. Prefer Fiber mode",
                                                     rel_ifIndex);
    }

    params_PTR->internal.phyMediaSelect.value = mediaSelect;

    return MPD_OK_E;

}

/*****************************************************************************
* FUNCTION NAME: prvMpdMtdGetMediaType_1
*
* DESCRIPTION: get the media type select according to fiber present or not
*
*                  MPD_TYPE_88X33x0_E
*
*****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMediaType_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MTD_U16 mediaSelect = MTD_MS_CU_ONLY;
    UINT_32 rel_ifIndex;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    switch (portEntry_PTR->initData_PTR->transceiverType) {
        case MPD_TRANSCEIVER_SFP_E:
            mediaSelect = MTD_MS_FBR_ONLY;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d. Fiber mode",
                                                         rel_ifIndex);
            break;
        case MPD_TRANSCEIVER_COPPER_E:
            mediaSelect = MTD_MS_CU_ONLY;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d. Copper mode",
                                                         rel_ifIndex);
            break;
        case MPD_TRANSCEIVER_COMBO_E:
            if (portEntry_PTR->runningData_PTR->sfpPresent) {
                mediaSelect = MTD_MS_FBR_ONLY;
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. Fiber mode",
                                                             rel_ifIndex);
            }
            else {
                /* when we work in copper only, the laser is not detected on some gibics */
                mediaSelect = MTD_MS_AUTO_PREFER_FBR;
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. Copper mode",
                                                             rel_ifIndex);
            }
            break;
        default:
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "Unknown transceiver type recieved\n");
            return MPD_OP_FAILED_E;
    }

    params_PTR->internal.phyMediaSelect.value = mediaSelect;

    return MPD_OK_E;

}


 /*****************************************************************************
 * FUNCTION NAME: prvMpdMtdGetAddressForTxParams
 *
 * DESCRIPTION: get address for txTune params
 *
 *****************************************************************************/
 MPD_RESULT_ENT prvMpdMtdGetAddressForTxParams (
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT params,
    /*     OUTPUTS:            */
    UINT_16 *address_PTR
 )
 {
     UINT_16 address;
     if (address_PTR == NULL){
         PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. Null address.",
                                                        portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
     }
     switch (params.phyInternalOperStatus.phySpeed) {
        case MPD_SPEED_10M_E:
        case MPD_SPEED_100M_E:
        case MPD_SPEED_1000M_E:
            address = (params.phyInternalOperStatus.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E) ? 0x8105 : 0x8104;
            break;
        case MPD_SPEED_10000M_E:
            address = 0x8101;
            break;
        case MPD_SPEED_2500M_E:
            address = 0x8103;
            break;
        case MPD_SPEED_5000M_E:
            address = 0x8102;
            break;
        default:
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(    PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. speed is not supported %d.",
                                                             portEntry_PTR->rel_ifIndex, params.phyInternalOperStatus.phySpeed);
            return MPD_NOT_SUPPORTED_E;
     }

     *address_PTR = address;

     return MPD_OK_E;
 }

 /*****************************************************************************
 * FUNCTION NAME: prvMpdMtdValidAndCalculateTxParams
 *
 * DESCRIPTION: calculate and valid tx tune params
 *
 *****************************************************************************/
 MPD_RESULT_ENT prvMpdMtdValidAndCalculateTxParams (
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_SERDES_TX_CONFIG_UNT phyTxParams,
    /*     INPUTS / OUTPUTS:   */
    UINT_16 *mask_PTR,
    UINT_16 *val_PTR
    /*     OUTPUTS:            */
 )
 {
     UINT_8 preTap, mainTap, postTap, val = 0, mask = 0;

     if ((mask_PTR == NULL) || (val_PTR == NULL)){
         PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. NULL pointer.",
                                                        portEntry_PTR->rel_ifIndex);
         return MPD_OP_FAILED_E;
     }

     preTap = phyTxParams.comphy.preTap;
     if (preTap != 0xFF) {
         val |= ((preTap & 0x1F) << 11); /* 15:11 */
         mask |= 0xF800;
     }

     mainTap = phyTxParams.comphy.mainTap;
     if (mainTap != 0xFF) {
         val |= ((mainTap & 0x3F) << 5); /* 10:5  */
         mask |= 0x7E0;
     }

     postTap = phyTxParams.comphy.postTap;
     if (postTap != 0xFF) {
         val |= (postTap & 0x1F);         /* 4:0 */
         mask |= 0x1F;
     }
     if (preTap + postTap + mainTap > 40) {
         PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d. pre %d + post %d + main %d > 40.",
                                                     portEntry_PTR->rel_ifIndex,
                                                     preTap,
                                                     postTap,
                                                     mainTap);
         PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
                                        MPD_ERROR_SEVERITY_WARNING_E,
                                        "Violating recomandtion to keep preTap + postTap + mainTap < 40");
     }

     if (mainTap < 20) {
         PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(   PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. main %d < 20.",
                                                        portEntry_PTR->rel_ifIndex, mainTap);
         return MPD_OP_FAILED_E;
     }

     *val_PTR = val;
     *mask_PTR = mask;

     return MPD_OK_E;
 }


 /*****************************************************************************
 * FUNCTION NAME: prvMpdMtdSetSerdesTune
 *
 * DESCRIPTION: this tx serdes configuration take place immediately and should be applied on link up
 *
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
 MPD_RESULT_ENT prvMpdMtdSetSerdesTune (
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
 )
 {
    UINT_16 val = 0, address = 0, device = 0, mask = 0;
    MPD_OPERATIONS_PARAMS_UNT params;
    MPD_RESULT_ENT status = MPD_OK_E;
    MTD_STATUS mtdStatus;
    MPD_SERDES_TX_CONFIG_UNT phyTxParams;
    MPD_SPEED_ENT  phySpeed;
    MPD_SERDES_TUNE_PARAMS_STC  serdesTuneParams;
    MPD_PHY_HOST_LINE_ENT   phySide;
    BOOLEAN usingInitDb = FALSE;

    memset(&serdesTuneParams, 0, sizeof(MPD_SERDES_TUNE_PARAMS_STC));
    if (params_PTR->phyTune.tuneParams.paramsType != MPD_SERDES_PARAMS_SET_FROM_XML_E){
        if (params_PTR->phyTune.tuneParams.paramsType > MPD_SERDES_PARAMS_TX_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "rel_ifIndex %d, SerDes type %d not supported",
                                                            portEntry_PTR->rel_ifIndex,
                                                            params_PTR->phyTune.tuneParams.paramsType);
            return MPD_NOT_SUPPORTED_E;
        }
        else {
            memcpy(&serdesTuneParams, &params_PTR->phyTune, sizeof(MPD_SERDES_TUNE_PARAMS_STC));
        }
    }
    else {
        usingInitDb = TRUE;
    }

    status = prvMpdPerformPhyOperation(portEntry_PTR, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &params);
    if (status != MPD_OK_E) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(    PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. failed to get internal status.",
                                                        portEntry_PTR->rel_ifIndex);
        return status;
    }

    phySpeed = params.phyInternalOperStatus.phySpeed;
    phySide = (!usingInitDb) ? serdesTuneParams.hostOrLineSide : MPD_PHY_SIDE_LINE_E;
    if (phySide == MPD_PHY_SIDE_LINE_E){
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "Line side for rel_ifIndex %d, phySpeed %d",
                                                        portEntry_PTR->rel_ifIndex, phySpeed);
        if (prvMpdGetTxSerdesParams(portEntry_PTR, MPD_PHY_SIDE_LINE_E, phySpeed, serdesTuneParams, &phyTxParams)) {
            mask = 0; val = 0;
            if (portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[MPD_PHY_SIDE_LINE_E]){
                device = 3;
                address = 0x2A;
            }
            else {
                prvMpdMtdGetAddressForTxParams(portEntry_PTR, params, &address);
                /* configuration on the line side is in offset 0x200 */
                address += 0x200;
                device = 30;
            }
            prvMpdMtdValidAndCalculateTxParams(portEntry_PTR, phyTxParams, &mask, &val);
            if (mask) {
                /* soft reset before change serdes - only in permanent configuration */
                if ((portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[MPD_PHY_SIDE_LINE_E]) && (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E)){
                    PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex), 2000),
                        portEntry_PTR->rel_ifIndex,
                        mtdStatus);
                }

                /* change serdes */
                PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, device, address, mask, val),
                    portEntry_PTR->rel_ifIndex,
                    mtdStatus);
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(    PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d. line side. speed %d %d.%x=0x%x with mask=0x%x,  pre %d,  post %d,  main %d.\n ",
                    portEntry_PTR->rel_ifIndex, params.phyInternalOperStatus.phySpeed, device, address, val, mask,
                    phyTxParams.comphy.preTap,
                    phyTxParams.comphy.postTap,
                    phyTxParams.comphy.mainTap);
            }
        }
    }

    phySide = (!usingInitDb) ? serdesTuneParams.hostOrLineSide : MPD_PHY_SIDE_HOST_E;
    if (phySide == MPD_PHY_SIDE_HOST_E){
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "Host side for rel_ifIndex %d, phySpeed %d",
                                                        portEntry_PTR->rel_ifIndex, phySpeed);
        if (prvMpdGetTxSerdesParams(portEntry_PTR, MPD_PHY_SIDE_HOST_E, phySpeed, serdesTuneParams, &phyTxParams)){
            mask = 0; val = 0;
            if (portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[MPD_PHY_SIDE_HOST_E]){
                device = 3;
                address = 0x29;
            }
            else {
                prvMpdMtdGetAddressForTxParams(portEntry_PTR, params, &address);
                device = 30;
            }
            prvMpdMtdValidAndCalculateTxParams(portEntry_PTR, phyTxParams, &mask, &val);
            if (mask) {
                /* soft reset before change serdes - only in permanent configuration */
                if ((portEntry_PTR->runningData_PTR->isNotImmidiateTxConfigure_ARR[MPD_PHY_SIDE_LINE_E]) &&
                    (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E)){
                    PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex), 2000),
                        portEntry_PTR->rel_ifIndex,
                        mtdStatus);
                }

                /* change serdes */
                PRV_MPD_MTD_CALL(   prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, device, address, mask, val),
                    portEntry_PTR->rel_ifIndex,
                    mtdStatus);
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(    PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d. host side. speed %d %d.%x=0x%x with mask=0x%x,  pre %d,  post %d,  main %d.\n ",
                    portEntry_PTR->rel_ifIndex, portEntry_PTR->runningData_PTR->speed, device, address, val, mask,
                    phyTxParams.comphy.preTap,
                    phyTxParams.comphy.postTap,
                    phyTxParams.comphy.mainTap);
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION NAME: prvMpdMtdSetSerdesTune_1
*
* DESCRIPTION: this tx serdes configuration take place immediately and should be applied on link up
*
*                  MPD_TYPE_88X35x0_E
*                  MPD_TYPE_88X25x0_E
*
*****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetSerdesTune_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16 address = 0;
    MPD_OPERATIONS_PARAMS_UNT params;
    MPD_RESULT_ENT status = MPD_OK_E;
    UINT_32 rel_ifIndex, value;
    UINT_8 pre, peak, post, margin;
    UINT_8 lane = 0;
    MCESD_DEV_PTR pSerdesDev;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (params_PTR->phyTune.tuneParams.paramsType == MPD_SERDES_PARAMS_TX_E) {
        status = prvMpdPerformPhyOperation(portEntry_PTR, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &params);
        if (status != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. failed to get internal status.",
                                                        rel_ifIndex);
            return status;
        }
        if (params.phyInternalOperStatus.isOperStatusUp == FALSE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d. link is down, need to apply configuration on link up.",
                                                         rel_ifIndex);
            return MPD_OP_FAILED_E;
        }

        lane = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 2;

        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),  &pSerdesDev));

        if (pSerdesDev) {
            MTD_ATTEMPT(mtdHwSerdesRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 0x2634,  &value));
            if (value != 0x20E00000) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex [%d]: mdioAddress [%d] value [0x%x] enable serdes", rel_ifIndex, address, value);
                MTD_ATTEMPT(mtdHwSerdesWrite(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 0x2634,  0x20E00000));
            }

            pre   =  params_PTR->phyTune.tuneParams.txTune.comphy_28G.pre;
            if (pre != 0xFF) {
                if (pre > 15) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex [%d]: index [%d] Pre param value [%d] is out of range", rel_ifIndex, pre);
                    return MPD_OP_FAILED_E;
                }
                MCESD_ATTEMPT(API_C28GP4X2_SetTxEqParam(pSerdesDev, lane, C28GP4X2_TXEQ_EM_PRE_CTRL, pre));
            }
            peak  =  params_PTR->phyTune.tuneParams.txTune.comphy_28G.peak;
            if (peak != 0xFF) {
                if (peak > 15) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex [%d]: index [%d] peak param value [%d] is out of range", rel_ifIndex, peak);
                    return MPD_OP_FAILED_E;
                }
                MCESD_ATTEMPT(API_C28GP4X2_SetTxEqParam(pSerdesDev, lane, C28GP4X2_TXEQ_EM_PEAK_CTRL, peak));
            }
            post  =  params_PTR->phyTune.tuneParams.txTune.comphy_28G.post;
            if (post != 0xFF) {
                if (post > 15) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex [%d]: index [%d] post param value [%d] is out of range", rel_ifIndex, post);
                    return MPD_OP_FAILED_E;
                }
                MCESD_ATTEMPT(API_C28GP4X2_SetTxEqParam(pSerdesDev, lane, C28GP4X2_TXEQ_EM_POST_CTRL, post));
            }

            margin = params_PTR->phyTune.tuneParams.txTune.comphy_28G.margin;
            if (margin != 0xFF) {
                if ((margin < 3 && margin > 0) || (margin > 7)) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "rel_ifIndex [%d]: index [%d] margin param value [%d] is out of range", rel_ifIndex, margin);
                    return MPD_OP_FAILED_E;
                }
                MCESD_ATTEMPT(API_C28GP4X2_SetTxEqParam(pSerdesDev, lane, C28GP4X2_TXEQ_MARGIN, margin));
            }

            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex [%d]: serdesLaneNum [%d] pre [0x%x] peak [0x%x] post [0x%x] margin [0x%x]", rel_ifIndex, lane, pre, peak, post, margin);


        }
        else {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d. lane %d serdes is not ready.",
                                                        rel_ifIndex, lane);
        }
    }
    else {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d. RX tune is not supported on this phy.",
                                                    rel_ifIndex);
        return MPD_NOT_SUPPORTED_E;
    }

    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit20x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit20x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_16 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_BOOL inLowPowerMode;
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
        prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
    }

    PRV_MPD_MTD_CALL(mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &inLowPowerMode),
                     rel_ifIndex,
                     mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        for (i = 0; i < 50; i++) {
            PRV_MPD_MTD_CALL(mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &is_tunit_ready),
                             rel_ifIndex,
                             mtdStatus);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE) {
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");
        }

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
         rel_ifIndex,
         (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
         i + 1);
#endif
    }

    /* copper initialization */
    PRV_MPD_MTD_CALL(mtdSetMacInterfaceControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                               MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                               MTD_FALSE, /* TRUE = mac interface always power up */
                                               MTD_MAC_SNOOP_OFF,
                                               0,
                                               MTD_MAC_SPEED_10_GBPS,
                                               MTD_MAX_MAC_SPEED_5G,
                                               MTD_TRUE, MTD_TRUE),
                     rel_ifIndex,
                     mtdStatus);


    PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                          MTD_F2R_OFF,
                                          MTD_MS_CU_ONLY,
                                          MTD_FT_NONE,
                                          MTD_FALSE,
                                          MTD_FALSE,
                                          MTD_TRUE, MTD_TRUE),
                     rel_ifIndex,
                     mtdStatus);


#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                     rel_ifIndex,
                     mtdStatus);

#endif

    if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                         rel_ifIndex,
                         mtdStatus);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit33x0And32x0
 *
 * DESCRIPTION:
 *
 * note: if PHY is strapped to work in fiber mode, init will fail
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit33x0And32x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_16 rel_ifIndex;
    MTD_BOOL inLowPowerMode;
    MTD_STATUS mtdStatus;
    MPD_OPERATIONS_PARAMS_UNT phy_params;
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
        prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
    }

    PRV_MPD_MTD_CALL(mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &inLowPowerMode),
                     rel_ifIndex,
                     mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
                                                     rel_ifIndex,
                                                     (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
                                                     i + 1);
#endif
    }

    if (portEntry_PTR->initData_PTR->shortReachSerdes) {
        /* enable the SERDES short reach mode by using the following setting: 7.800F.10 (hunit_shortReach) = 1 and 7.800F.11 (xunit_shortReach) = 1*/

        PRV_MPD_MTD_CALL(mtdSetSerdesOverrideReach(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_H_UNIT, MTD_TRUE),
                         rel_ifIndex,
                         mtdStatus);

        PRV_MPD_MTD_CALL(mtdSetSerdesOverrideReach(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_X_UNIT, MTD_TRUE),
                         rel_ifIndex,
                         mtdStatus);
    }

    /* copper initialization */
    PRV_MPD_MTD_CALL(mtdSetMacInterfaceControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                               MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                               MTD_FALSE, /* TRUE = mac interface always power up */
                                               MTD_MAC_SNOOP_OFF,
                                               0,
                                               MTD_MAC_SPEED_10_GBPS,
                                               MTD_MAX_MAC_SPEED_10G,
                                               MTD_TRUE, MTD_FALSE),
                     rel_ifIndex,
                     mtdStatus);

    memset(&phy_params, 0, sizeof(phy_params));
    prvMpdPerformPhyOperation(portEntry_PTR, PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E, &phy_params);
    PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                          MTD_F2R_OFF,
                                          phy_params.internal.phyMediaSelect.value,
                                          MTD_FT_10GBASER,
                                          MTD_FALSE,
                                          MTD_FALSE,
                                          MTD_TRUE, MTD_TRUE),
                     rel_ifIndex,
                     mtdStatus);

    PRV_MPD_MTD_CALL(mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                             MTD_ENABLE_NFR_ONLY,
                                             MTD_FR_IDLE,
                                             MTD_FALSE),
                     rel_ifIndex,
                     mtdStatus);

#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                     rel_ifIndex,
                     mtdStatus);
#endif
    /** if downshift supported on the device (might be not supported), Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
     * this configuration requires SW reset but we don't do this here,
     * SW reset will be done when port becomes admin up.
     */
    mtdAutonegSetNbaseTDownshiftControl(
        PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
        0 /* disable downhift */,
        MTD_TRUE /* re-enable all speeds when no energy is detected */,
        MTD_FALSE /* disable downhift */,
        MTD_FALSE /* don't restart autoneg */);

    if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                         rel_ifIndex,
                         mtdStatus);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit2180
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit2180(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_16 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_BOOL inLowPowerMode;
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
        prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
    }

    PRV_MPD_MTD_CALL(mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &inLowPowerMode),
                     rel_ifIndex,
                     mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
         rel_ifIndex,
         (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
         i + 1);
#endif
    }
#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                     rel_ifIndex,
                     mtdStatus);
#endif
    /* copper initialization */
    PRV_MPD_MTD_CALL(mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                     MTD_MAC_TYPE_5BR_SGMII_AN_EN,
                                                     MTD_FALSE, /* TRUE = mac interface always power up */
                                                     MTD_MAC_SPEED_10_MBPS,
                                                     MTD_MAX_MAC_SPEED_5G),
                     rel_ifIndex,
                     mtdStatus);

    PRV_MPD_MTD_CALL(mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                             MTD_ENABLE_NFR_ONLY,
                                             MTD_FR_IDLE,
                                             MTD_TRUE),
                     rel_ifIndex,
                     mtdStatus);


    /** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
     * this configuration requires SW reset but we don't do this here,
     * SW reset will be done when port becomes admin up.
     */
    PRV_MPD_MTD_CALL(mtdAutonegSetNbaseTDownshiftControl(
                         PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                         0 /* disable downhift */,
                         MTD_TRUE /* re-enable all speeds when no energy is detected */,
                         MTD_FALSE /* disable downhift */,
                         MTD_FALSE /* don't restart autoneg */),
                     rel_ifIndex,
                     mtdStatus);

    if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                         rel_ifIndex,
                         mtdStatus);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit35x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit35x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_16 rel_ifIndex;
    MTD_U16 macType, currMacType, currMacLinkDownSpeed, currMacMaxIfSpeed;
    MTD_STATUS mtdStatus;
    MTD_BOOL inLowPowerMode, currMacIfPowerDown, hostCanPowerDown = MTD_FALSE;
    MPD_PHY_USX_TYPE_ENT    usxType;
    BOOLEAN configureInterfaceMode = FALSE;
    MPD_RESULT_ENT mpdStatus;
    char error_msg [256];
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif

    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
        prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
    }

    PRV_MPD_MTD_CALL(mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &inLowPowerMode),
                     rel_ifIndex,
                     mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
         rel_ifIndex,
         (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
         i + 1);
#endif
    }
#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                     rel_ifIndex,
                     mtdStatus);
#endif
    /* copper initialization */
    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE) {
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_FALSE;
        macType = MTD_MAC_TYPE_10GR_SGMII_AN_EN;
    }
    else if ((((usxType == MPD_PHY_USX_TYPE_SXGMII) || (usxType == MPD_PHY_USX_TYPE_10G_DXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_DXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 2) == 0) ||
             (((usxType == MPD_PHY_USX_TYPE_10G_QXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_QXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)) {
        configureInterfaceMode = TRUE;
        mpdStatus = prvConvertMpdUsxModeToMtdUsxMode(portEntry_PTR, &macType);
        if (mpdStatus != MPD_OK_E) {
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - Failed to convert MPD usx mode [%d] to MTD usx mode\n", rel_ifIndex, usxType);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }
        hostCanPowerDown = MTD_TRUE;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d , usxType is %d. macType is 0x%x",
         rel_ifIndex,
         usxType,
         macType);
    }
    else if (((usxType == MPD_PHY_USX_TYPE_OXGMII) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)) {
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_TRUE;
        /* checking if the phy is master/slave */
        PRV_MPD_MTD_CALL(mtdGetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         &currMacType,
                                                         &currMacIfPowerDown,
                                                         &currMacLinkDownSpeed,
                                                         &currMacMaxIfSpeed),
                         rel_ifIndex,
                         mtdStatus);
        if (currMacType == MTD_MAC_TYPE_OXGMII) {
            macType = MTD_MAC_TYPE_OXGMII;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
            (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
             "rel_ifIndex %d: is master OXGMII.  usxType is %d. currentMacType is 0x%x",
             rel_ifIndex,
             usxType,
             currMacType);
        }
        else {
            macType = MTD_MAC_TYPE_OXGMII_PARTNER;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
            (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
             "rel_ifIndex %d: is slave OXGMII.  usxType is %d. currentMacType is 0x%x",
             rel_ifIndex,
             usxType,
             currMacType);
        }
    }

    if (configureInterfaceMode) {
        PRV_MPD_MTD_CALL(mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         macType,
                                                         hostCanPowerDown,
                                                         MTD_MAC_SPEED_NOT_APPLICABLE,
                                                         MTD_MAX_MAC_SPEED_NOT_APPLICABLE),
                         rel_ifIndex,
                         mtdStatus);
    }
    PRV_MPD_MTD_CALL(mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                             MTD_ENABLE_NFR_ONLY,
                                             MTD_FR_IDLE,
                                             MTD_FALSE),
                     rel_ifIndex,
                     mtdStatus);

    /** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
     * this configuration requires SW reset but we don't do this here,
     * SW reset will be done when port becomes admin up.
     */
    PRV_MPD_MTD_CALL(mtdAutonegSetNbaseTDownshiftControl(
                         PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                         0 /* disable downhift */,
                         MTD_TRUE /* re-enable all speeds when no energy is detected */,
                         MTD_FALSE /* disable downhift */,
                         MTD_FALSE /* don't restart autoneg */),
                     rel_ifIndex,
                     mtdStatus);

    if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                         rel_ifIndex,
                         mtdStatus);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdInit25x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdInit25x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    UINT_16 rel_ifIndex;
    MTD_U16 macType, currMacType, currMacLinkDownSpeed, currMacMaxIfSpeed;
    MTD_STATUS mtdStatus;
    MTD_BOOL inLowPowerMode, currMacIfPowerDown, hostCanPowerDown = MTD_FALSE;
    MPD_PHY_USX_TYPE_ENT    usxType;
    BOOLEAN configureInterfaceMode = FALSE;
    MPD_RESULT_ENT mpdStatus;
    char error_msg [256];
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif

    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
        prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
    }

    PRV_MPD_MTD_CALL(mtdIsTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &inLowPowerMode),
                     rel_ifIndex,
                     mtdStatus);
    if (inLowPowerMode) {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d: T unit ready after low-power removal [%s], num of tries [%d]",
         rel_ifIndex,
         (is_tunit_ready == MTD_FALSE) ? "FALSE" : "TRUE",
         i + 1);
#endif
    }
#ifndef PHY_SIMULATION
    /* Enable Temperature Sensor*/
    PRV_MPD_MTD_CALL(mtdEnableTemperatureSensor(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                     rel_ifIndex,
                     mtdStatus);
#endif
    /* copper initialization */
    usxType = portEntry_PTR->initData_PTR->usxInfo.usxType;
    if (usxType == MPD_PHY_USX_TYPE_NONE) {
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_FALSE;
        macType = MTD_MAC_TYPE_5BR_SGMII_AN_EN;
    }
    else if ((usxType == MPD_PHY_USX_TYPE_SXGMII) ||
             (((usxType == MPD_PHY_USX_TYPE_10G_DXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_DXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 2) == 0) ||
             (((usxType == MPD_PHY_USX_TYPE_10G_QXGMII) || (usxType == MPD_PHY_USX_TYPE_20G_QXGMII)) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)) {
        configureInterfaceMode = TRUE;
        mpdStatus = prvConvertMpdUsxModeToMtdUsxMode(portEntry_PTR, &macType);
        if (mpdStatus != MPD_OK_E) {
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - Failed to convert MPD usx mode [%d] to MTD usx mode\n", rel_ifIndex, usxType);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }
        hostCanPowerDown = MTD_TRUE;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
        (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
         "rel_ifIndex %d , usxType is %d. macType is 0x%x",
         rel_ifIndex,
         usxType,
         macType);
    }
    else if (((usxType == MPD_PHY_USX_TYPE_OXGMII) && (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % 4) == 0)) {
        configureInterfaceMode = TRUE;
        hostCanPowerDown = MTD_TRUE;
        /* checking if the phy is master/slave */
        PRV_MPD_MTD_CALL(mtdGetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         &currMacType,
                                                         &currMacIfPowerDown,
                                                         &currMacLinkDownSpeed,
                                                         &currMacMaxIfSpeed),
                         rel_ifIndex,
                         mtdStatus);
        if (currMacType == MTD_MAC_TYPE_OXGMII) {
            macType = MTD_MAC_TYPE_OXGMII;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
            (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
             "rel_ifIndex %d: is master OXGMII.  usxType is %d. currentMacType is 0x%x",
             rel_ifIndex,
             usxType,
             currMacType);
        }
        else {
            macType = MTD_MAC_TYPE_OXGMII_PARTNER;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())
            (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
             "rel_ifIndex %d: is slave OXGMII.  usxType is %d. currentMacType is 0x%x",
             rel_ifIndex,
             usxType,
             currMacType);
        }
    }

    if (configureInterfaceMode) {
        PRV_MPD_MTD_CALL(mtdSetMacInterfaceCopperOnlyPhy(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         macType,
                                                         hostCanPowerDown,
                                                         MTD_MAC_SPEED_NOT_APPLICABLE,
                                                         MTD_MAX_MAC_SPEED_NOT_APPLICABLE),
                         rel_ifIndex,
                         mtdStatus);
    }
    PRV_MPD_MTD_CALL(mtdConfigureFastRetrain(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                             MTD_ENABLE_NFR_ONLY,
                                             MTD_FR_IDLE,
                                             MTD_FALSE),
                     rel_ifIndex,
                     mtdStatus);

    /** Disable down-shift feature - prevent link establish on a lower than maximum compatible speed"
     * this configuration requires SW reset but we don't do this here,
     * SW reset will be done when port becomes admin up.
     */
    PRV_MPD_MTD_CALL(mtdAutonegSetNbaseTDownshiftControl(
                         PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                         0 /* disable downhift */,
                         MTD_TRUE /* re-enable all speeds when no energy is detected */,
                         MTD_FALSE /* disable downhift */,
                         MTD_FALSE /* don't restart autoneg */),
                     rel_ifIndex,
                     mtdStatus);

    if (portEntry_PTR->initData_PTR->doSwapAbcd) {
        PRV_MPD_MTD_CALL(prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0xC000, 0x1, 0x1),
                         rel_ifIndex,
                         mtdStatus);
    }

    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdReset32x0
 *
 * DESCRIPTION: Utility, Perform PHY reset for MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/

static MPD_RESULT_ENT prvMpdMtdReset32x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_32 sleep_time_ms
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_BOOL phyReady;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    PRV_MPD_MTD_CALL(mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady),
                     rel_ifIndex,
                     mtdStatus);

    if ((phyReady) && (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E)) {
        PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), sleep_time_ms),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex [%d] call to mtdSoftwareReset",
                                                     rel_ifIndex);
    }
    else {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex [%d] T unit is not ready",
                                                     rel_ifIndex);
    }
    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetSpeed_1
 *
 * DESCRIPTION: Get PHY operational link & speed
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetSpeed_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16 speed_res;
    UINT_32 rel_ifIndex;
    MTD_BOOL dummy, linkStatus;
    MPD_RESULT_ENT ret = MPD_OK_E;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_LAST_E;
    params_PTR->phyInternalOperStatus.duplexMode = TRUE;
    params_PTR->phyInternalOperStatus.isAnCompleted = FALSE;
    params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
#ifdef PHY_SIMULATION
    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
#endif
    params_PTR->phyInternalOperStatus.mediaType = (portEntry_PTR->runningData_PTR->sfpPresent) ? MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E : MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E;

    /* if port is power down we should not check negotiation status -
     * after shutdown and restart phy2180 negotiation status ended with oper status up while the port was in power down */
    if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
        params_PTR->phyInternalOperStatus.isOperStatusUp = FALSE;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_LINK_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                   "rel_ifIndex %d:  admin down state",
                                                   rel_ifIndex);
        return ret;
    }
    else {
        if (portEntry_PTR->runningData_PTR->sfpPresent) { /* handle fiber case */
            PRV_MPD_MTD_CALL(mtdGet10GBRStatus1(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                MTD_X_UNIT, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &linkStatus), rel_ifIndex, mtdStatus);

            if (linkStatus == MTD_TRUE) {
                params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
            }
            else {
                PRV_MPD_MTD_CALL(mtdGet1000BXSGMIIStatus(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         MTD_X_UNIT, &dummy, &dummy, &dummy, &linkStatus), rel_ifIndex, mtdStatus);
                if (linkStatus == MTD_TRUE) {
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
                }
            }

            params_PTR->phyInternalOperStatus.isOperStatusUp = linkStatus;
        }
        /* copper case */
        else {
            PRV_MPD_MTD_CALL(mtdIsBaseTUp(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speed_res, &linkStatus), rel_ifIndex, mtdStatus);
            params_PTR->phyInternalOperStatus.isOperStatusUp = params_PTR->phyInternalOperStatus.isAnCompleted = (linkStatus == MTD_TRUE) ? TRUE : FALSE;
            switch (speed_res) {
                case MTD_SPEED_10M_HD: /* 0x0001 // 10BT half-duplex*/
                case MTD_SPEED_10M_HD_AN_DIS:
                    params_PTR->phyInternalOperStatus.duplexMode = FALSE;
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10M_E;
                    break;
                case MTD_SPEED_10M_FD: /*       0x0002 // 10BT full-duplex*/
                case MTD_SPEED_10M_FD_AN_DIS:
                    params_PTR->phyInternalOperStatus.duplexMode = TRUE;
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10M_E;
                    break;
                case MTD_SPEED_100M_HD: /*  0x0004 // 100BASE-TX half-duplex*/
                case MTD_SPEED_100M_HD_AN_DIS:
                    params_PTR->phyInternalOperStatus.duplexMode = FALSE;
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_100M_E;
                    break;
                case MTD_SPEED_100M_FD: /*  0x0008 // 100BASE-TX full-duplex*/
                case MTD_SPEED_100M_FD_AN_DIS:
                	params_PTR->phyInternalOperStatus.duplexMode = TRUE;
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_100M_E;
                    break;
                case MTD_SPEED_1GIG_HD: /*      0x0010 // 1000BASE-T half-duplex*/
                    params_PTR->phyInternalOperStatus.duplexMode = FALSE;
                    MPD_FALLTHROUGH
                case MTD_SPEED_1GIG_FD: /*      0x0020 // 1000BASE-T full-duplex*/
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_1000M_E;
                    break;
                case MTD_SPEED_10GIG_FD: /*     0x0040 // 10GBASE-T full-duplex*/
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_10000M_E;
                    break;
                case MTD_SPEED_2P5GIG_FD: /*    0x0800 // 2.5GBASE-T full-duplex, 88X33X0 family only*/
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_2500M_E;
                    break;
                case MTD_SPEED_5GIG_FD: /*      0x1000 5Gig */
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_5000M_E;
                    break;
                case MTD_ADV_NONE:
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_LAST_E;
                    break;
                default:
                    params_PTR->phyInternalOperStatus.phySpeed = MPD_SPEED_LAST_E;
                    break;
            }
        /* copper case */
        }
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_LINK_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               "rel_ifIndex %d: media [%s] speed [%#x] duplex [%s] link [%s]",
                                               rel_ifIndex,
                                               (portEntry_PTR->runningData_PTR->sfpPresent) ? "Fiber" : "RJ45",
                                               params_PTR->phyInternalOperStatus.phySpeed,
                                               (params_PTR->phyInternalOperStatus.duplexMode) ? "FULL" : "HALF",
                                               (params_PTR->phyInternalOperStatus.isOperStatusUp) ? "UP" : "DOWN");

    return ret;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetLpAnCapabilities
 *
 * DESCRIPTION: Get link partner auto negotiation capabilities
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetLpAnCapabilities(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 ieeeSpeed, nbSpeed;
    UINT_32 i;
    UINT_32 rel_ifIndex;
    UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdSpeed) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
    MTD_STATUS rc;
    MPD_RESULT_ENT ret = MPD_OK_E;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    params_PTR->phyRemoteAutoneg.capabilities = 0;

    rc = mtdGetLPAdvertisedSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                  &ieeeSpeed,
                                  &nbSpeed);
    if (rc != MTD_OK || ieeeSpeed == MTD_ADV_NONE) {
        params_PTR->phyRemoteAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d:  prvMpdMtdGetLpAnCapabilities return not supported",
                                                    rel_ifIndex);
        ret = MPD_NOT_SUPPORTED_E;
    }

    for (i = 0; i < arr_size; i++) {
        if (ieeeSpeed & prvMpdConvertMtdToMpdSpeed[i].mtd_val) {
            params_PTR->phyRemoteAutoneg.capabilities |= prvMpdConvertMtdToMpdSpeed[i].hal_val;
        }
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d ieeeSpeed 0x%x and returned capabilities 0x%x",
                                                 rel_ifIndex,
                                                 ieeeSpeed,
                                                 params_PTR->phyRemoteAutoneg.capabilities);

    return ret;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetMdixMode
 *
 * DESCRIPTION: Set MDI/X mode (mdi/midx/auto)
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetMdixMode(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    BOOLEAN swReset;
    UINT_16 val;
    UINT_16 sf_mdix_arr[] = {   MTD_FORCE_MDI, /* MPD_MDI_MODE_MEDIA_E  */
                                MTD_FORCE_MDIX, /* MPD_MDIX_MODE_MEDIA_E */
                                MTD_AUTO_MDI_MDIX
                            }; /* MPD_AUTO_MODE_MEDIA_E */
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdStatus = mtdGetMDIXControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                  &val);
    if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d:  mtdGetMDIXControl failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    if ((val & 3) != sf_mdix_arr[params_PTR->phyMdix.mode]) {

        /* if port is UP - restart auto-neg */
    	swReset = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;

        PRV_MPD_MTD_CALL(mtdMDIXControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), sf_mdix_arr[params_PTR->phyMdix.mode], swReset),
                         rel_ifIndex,
                         mtdStatus);

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d config 0x%x swReset %d",
                                                     rel_ifIndex,
                                                     sf_mdix_arr[params_PTR->phyMdix.mode],
													 swReset);
    }
    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetAdvertiseFc
 *
 * DESCRIPTION: Configre flow control advertisement capabilities
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetAdvertiseFc(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 val, forcedSpeed;
    MTD_BOOL isSpeedForced = MTD_FALSE, anRestart = MTD_TRUE;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               7,
                               16,
                               &val);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
    PRV_MPD_MTD_CALL(mtdGetForcedSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &isSpeedForced, &forcedSpeed),
                     rel_ifIndex,
                     mtdStatus);
    if ((isSpeedForced == MTD_TRUE) || (portEntry_PTR->runningData_PTR->adminMode == FALSE)) {
        anRestart = MTD_FALSE;
    }

    /* symetric pause - bit 10 */
    if ((val & 0x400) && params_PTR->phyFc.advertiseFc == FALSE) {

        PRV_MPD_MTD_CALL(mtdSetPauseAdvertisement(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_CLEAR_PAUSE, anRestart),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d clear pause",
                                                     rel_ifIndex);
        if (mtdStatus != MTD_OK) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  mtdSetPauseAdvertisement failed",
                                                        rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
    }
    else if (((val & 0x400) == FALSE) && params_PTR->phyFc.advertiseFc == TRUE) {

        PRV_MPD_MTD_CALL(mtdSetPauseAdvertisement(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_SYM_PAUSE, anRestart),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d config sym pause",
                                                     rel_ifIndex);
        if (mtdStatus != MTD_OK) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d:  mtdSetPauseAdvertisement failed",
                                                        rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
    }

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetMdixAdminMode
 *
 * DESCRIPTION: Get admin (configured) MDI/X mode
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMdixAdminMode(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16 val;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdStatus = mtdGetMDIXControl(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                  &val);
    if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d:  mtdGetMDIXControl failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
    switch (val) {
        case MTD_FORCE_MDI:
            params_PTR->phyMdix.mode = MPD_MDI_MODE_MEDIA_E;
            break;
        case MTD_FORCE_MDIX:
            params_PTR->phyMdix.mode = MPD_MDIX_MODE_MEDIA_E;
            break;
        case MTD_AUTO_MDI_MDIX:
            params_PTR->phyMdix.mode = MPD_AUTO_MODE_MEDIA_E;
            break;
        default:
            return MPD_OP_FAILED_E;

    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetMdixMode
 *
 * DESCRIPTION: Get operational (resolved) MDI/X mode
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetMdixMode(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_8 val;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdStatus = mtdGetMDIXResolution(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                     &val);
    if (mtdStatus != MTD_OK || val == MTD_MDI_MDIX_NOT_RESOLVED) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d:  mtdGetMDIXResolution failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    params_PTR->phyMdix.mode =
        (val == MTD_MDIX_RESOLVED) ? MPD_MDIX_MODE_MEDIA_E : MPD_MDI_MODE_MEDIA_E;

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d result 0x%x return mode %s",
                                                 rel_ifIndex,
                                                 val,
                                                 (params_PTR->phyMdix.mode == MPD_MDIX_MODE_MEDIA_E) ? "MDIX" : "MDI");

    return MPD_OK_E;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetLpAdvFc
 *
 * DESCRIPTION: Get link partner advertised flow control capabilities
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetLpAdvFc(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_8 val;
    UINT_32 rel_ifIndex;
    MPD_RESULT_ENT ret = MPD_OK_E;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    if (mtdGetLPAdvertisedPause(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                &val) == MTD_OK) {
        params_PTR->phyLinkPartnerFc.pauseCapable =
            ((val & MTD_SYM_PAUSE) == 0) ? FALSE : TRUE;

        params_PTR->phyLinkPartnerFc.asymetricRequested =
            (val && (val & ~MTD_SYM_PAUSE) == 0) ? TRUE : FALSE;
    }
    else {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d:  prvMpdMtdGetLpAdvFc return not supported",
                                                    rel_ifIndex);
        ret = MPD_NOT_SUPPORTED_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d result 0x%x return pause capable %s",
                                                 rel_ifIndex,
                                                 val,
                                                 (params_PTR->phyLinkPartnerFc.pauseCapable == FALSE) ? "NO" : "YES");

    return ret;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdRestartAutoNeg
 *
 * DESCRIPTION: Reset auto-negotiation
 *  Reset PHY will loose configuration, it is not exposed to application.
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdRestartAutoNeg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (portEntry_PTR->runningData_PTR->adminMode) {
		PRV_MPD_MTD_CALL(mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
						 rel_ifIndex,
						 mtdStatus);
		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													 "rel_ifIndex %d call to mtdAutonegRestart ",
													 rel_ifIndex);
    } else {
    	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
													 "rel_ifIndex %d SKIP call to mtdAutonegRestart, admin down",
													 rel_ifIndex);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDisable_1
 *
 * DESCRIPTION:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDisable_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus = MTD_FALSE;
    MTD_BOOL enable;
    MTD_BOOL phyReady;
    MPD_PORT_ADMIN_ENT desired;
    MPD_RESULT_ENT ret_val = MPD_OK_E;
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    desired = (params_PTR->phyDisable.forceLinkDown == TRUE) ? MPD_PORT_ADMIN_DOWN_E : MPD_PORT_ADMIN_UP_E;

    if (desired == portEntry_PTR->runningData_PTR->adminMode) {
        /* bug in 88X32x0 do not reconfigure same configuration. */
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, admin %s. current equal desired, skip config.",
                                                     rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");
        return MPD_OK_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, admin %s",
                                                 rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");

    if (desired == MPD_PORT_ADMIN_DOWN_E) {
        ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E);
        if (ret_val != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d, phy not ready!",
                                                         rel_ifIndex);
        }
        /* put in low power mode */
        PRV_MPD_MTD_CALL(mtdPutPhyInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
        /* in case we are in prefer fiber mode, the DB might not be updated yet.
            if we are in active media fiber T unit will not be ready for use */
        mtdStatus = mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady);
        if (mtdStatus != MTD_OK) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  "rel_ifIndex %d, mtdIsPhyReadyAfterReset FAIL!",
                                  rel_ifIndex);
        }
        if (phyReady) {
            mtdStatus = mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 500);
            if (mtdStatus != MTD_OK) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, mtdSoftwareReset FAIL!",
                    rel_ifIndex);
            }
#ifndef PHY_SIMULATION
            else {
                 /* mtdStatus = MTD_OK - check t unit is ready  */
                for (i = 0; i < 50; i++) {
                    PRV_MPD_MTD_CALL(mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &is_tunit_ready),
                                     rel_ifIndex,
                                     mtdStatus);
                    if (is_tunit_ready == MTD_TRUE) {
                        break;
                    }
                    PRV_MPD_SLEEP_MAC(10);
                }
                if (is_tunit_ready == MTD_FALSE) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                        "rel_ifIndex %d, T Unit is not ready, probably active media is fiber.\n",
                        rel_ifIndex);
                }
            }
#endif
        }
        else {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex [%d] T unit is not ready",
                                                         rel_ifIndex);
        }
    }

    if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_COPPER_E) {
        /* Disable/ enable Fiber transceiver through application */
        enable = (params_PTR->phyDisable.forceLinkDown == FALSE) ? TRUE : FALSE;
        PRV_MPD_TRANSCEIVER_ENABLE_MAC(portEntry_PTR->rel_ifIndex,
                                       enable);
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDisable_2
 *
 * DESCRIPTION:
 *                  MPD_TYPE_88X33x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDisable_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus = MTD_FALSE;
    MTD_BOOL enable;
    MPD_PORT_ADMIN_ENT desired;
	MTD_U16 frameToRegister, mediaSelect, fiberType;
	MTD_BOOL npMediaEnergyDetect, maxPowerTunitAMDetect;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	MPD_RESULT_ENT ret_val = MPD_OK_E;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    desired = (params_PTR->phyDisable.forceLinkDown == TRUE) ? MPD_PORT_ADMIN_DOWN_E : MPD_PORT_ADMIN_UP_E;

    if (desired == portEntry_PTR->runningData_PTR->adminMode) {
        /* bug in 88X33x0 do not reconfigure same configuration. */
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, admin %s. current equal desired, skip config.",
                                                     rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");
        return MPD_OK_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, admin %s",
                                                 rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");
    /* get configured information */
    PRV_MPD_MTD_CALL(mtdGetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
        			&frameToRegister, &mediaSelect, &fiberType, &npMediaEnergyDetect, &maxPowerTunitAMDetect),
        					 rel_ifIndex,
        					 mtdStatus);

    if (desired == MPD_PORT_ADMIN_DOWN_E) {
    	if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
			/* move to fiber only mode, so T unit will be powered down */
			PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_F2R_OFF, MTD_MS_FBR_ONLY,
					fiberType, MTD_FALSE, MTD_FALSE, MTD_FALSE, MTD_FALSE),
							 rel_ifIndex,
							 mtdStatus);
    	}
        /* put in low power mode */
        PRV_MPD_MTD_CALL(mtdPutPhyInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemovePhyLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
        if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
			memset(&phy_params, 0, sizeof(phy_params));
			prvMpdPerformPhyOperation(portEntry_PTR, PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E, &phy_params);
			/* move to appropriate media mode according to present status */
			PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_F2R_OFF, phy_params.internal.phyMediaSelect.value,
							fiberType, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_TRUE),
							rel_ifIndex,
							mtdStatus);
        }
        /* perform software reset - if T unit is not ready then it will be done on present notification */
        ret_val = prvMpdMtdIsPhyReady(portEntry_PTR,PRV_MPD_PHY_READY_WITH_SW_RESET_E);
		if (ret_val != MPD_OK_E) {
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														 "rel_ifIndex %d, phy not ready after PRV_MPD_PHY_READY_WITH_SW_RESET_E!",
														 rel_ifIndex);
		}
    }


    if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_COPPER_E) {
        /* Disable/ enable Fiber transceiver through application */
        enable = (params_PTR->phyDisable.forceLinkDown == FALSE) ? TRUE : FALSE;
        PRV_MPD_TRANSCEIVER_ENABLE_MAC(portEntry_PTR->rel_ifIndex,
                                       enable);
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDisable_3
 *
 * DESCRIPTION:
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDisable_3(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_PORT_ADMIN_ENT desired;
    MPD_RESULT_ENT ret_val = MPD_OK_E;
#ifndef PHY_SIMULATION
    MTD_BOOL is_tunit_ready;
    UINT_32 i;
#endif
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    desired = (params_PTR->phyDisable.forceLinkDown == TRUE) ? MPD_PORT_ADMIN_DOWN_E : MPD_PORT_ADMIN_UP_E;

    if (desired == portEntry_PTR->runningData_PTR->adminMode) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, admin %s. current equal desired, skip config.",
                                                     rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");
        return MPD_OK_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, admin %s",
                                                 rel_ifIndex, (desired == MPD_PORT_ADMIN_DOWN_E) ? "DOWN" : "UP");

    if (desired == MPD_PORT_ADMIN_DOWN_E) {
        /* put in low power mode */
        ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E);
        if (ret_val != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d, phy not ready!",
                                                         rel_ifIndex);
        }
        PRV_MPD_MTD_CALL(mtdPutTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        /* remove from low-power mode */
        PRV_MPD_MTD_CALL(mtdRemoveTunitLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);

        PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 500),
                         rel_ifIndex,
                         mtdStatus);

#ifndef PHY_SIMULATION
        /* check t unit is ready  */
        for (i = 0; i < 50; i++) {
            mtdIsTunitResponsive(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &is_tunit_ready);
            if (is_tunit_ready == MTD_TRUE) {
                break;
            }
            PRV_MPD_SLEEP_MAC(20);
        }
        if (is_tunit_ready == MTD_FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "T Unit is not ready after 1 Sec\n");
#endif
    }

#ifndef PHY_SIMULATION
    return mtdStatus == MTD_OK ? MPD_OK_E : MPD_OP_FAILED_E;
#else
	return MPD_OK_E;
#endif

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdVctUtility
 *
 * DESCRIPTION: Utility, Set SolarFlare VCT test - utility used for test2 and test3
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdVctUtility(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_8 terminitationCode[4] = { 0,
                                    0,
                                    0,
                                    0
                                  }, length[4] = {  0,
                                                    0,
                                                    0,
                                                    0
                                                 };
    UINT_8 max_termination_code = 0;
    MTD_BOOL testDone = MTD_FALSE;
    UINT_32 i, num_of_tries = 60, num_of_pairs = 4,
               len = 0;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus = MTD_FAIL;
    MPD_RESULT_ENT ret_val = MPD_OK_E;
    MPD_VCT_RESULT_ENT vct_res_convert_arr[] = {    MPD_VCT_RESULT_BAD_CABLE_E,
                                                    MPD_VCT_RESULT_CABLE_OK_E,
                                                    MPD_VCT_RESULT_OPEN_CABLE_E,
                                                    MPD_VCT_RESULT_SHORT_CABLE_E,
                                                    MPD_VCT_RESULT_SHORT_CABLE_E
                                               };

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    /* Perform VCT test - then restore the WA  */
    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "start VCT test on rel_ifIndex %d",
                                              rel_ifIndex);
    /* start VCT test - TDR */
    PRV_MPD_MTD_CALL(mtdRun_CableDiagnostics(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_TRUE/*break link*/, MTD_FALSE/*disableInterPairShort*/),
                     rel_ifIndex,
                     mtdStatus);

    /* sleep and check up to 5 seconds */
    for (i = 0; i < num_of_tries; i++) {
        /* sleep 100 ms and check whether the test has finished */
        PRV_MPD_SLEEP_MAC(100);
        mtdStatus = mtdGet_CableDiagnostics_Status(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                   &testDone);
        if ((mtdStatus == MTD_OK) && (testDone == MTD_TRUE)) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "Exit in mtdGet_CableDiagnostics_Status. rel_ifIndex %d, i = %d",
                                                      rel_ifIndex,
                                                      i);
            break;
        }
    }

    /* test succeeded */
    if ((i < num_of_tries) && (testDone == MTD_TRUE)) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "VCT test ended on rel_ifIndex %d, i=%d, test duration: %d[ms]",
                                                  rel_ifIndex,
                                                  i,
                                                  (i + 1) * 100);

        PRV_MPD_MTD_CALL(mtdGet_CableDiagnostics_Results(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         terminitationCode,
                                                         length), rel_ifIndex, mtdStatus);

        for (i = 0; i < num_of_pairs; i++) {
            if (length[i] >= 0xF0) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "rel_ifIndex [%d], pair No. [%d], Length=[%d], override with 1 terminitationCode=[%d]",
                                                          rel_ifIndex,
                                                          i + 1,
                                                          length[i],
                                                          terminitationCode[i]);
                length[i] = 0;

            }
            else {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "rel_ifIndex [%d], pair No. [%d], Length=[%d], terminitationCode=[%d]",
                                                          rel_ifIndex,
                                                          i + 1,
                                                          length[i],
                                                          terminitationCode[i]);
            }
            len += length[i];
            if (max_termination_code < terminitationCode[i]) {
                max_termination_code = terminitationCode[i];
            }
        }

        /* average of all pairs length */
        params_PTR->phyVct.cableLength = len / (num_of_pairs);

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d, i=%d, Total Length: %d, Avg Len: %d",
                                                  rel_ifIndex,
                                                  i,
                                                  len,
                                                  params_PTR->phyVct.cableLength);

        if (max_termination_code > PRV_MPD_vctResult_interPairShort_CNS) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d. phy_op_failed");

            ret_val = MPD_OP_FAILED_E;
        }

        params_PTR->phyVct.testResult = vct_res_convert_arr[max_termination_code];
    }
    else { /* test failed */
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "VCT test failed on rel_ifIndex %d",
                                                  rel_ifIndex);

        ret_val = MPD_OP_FAILED_E; /* VCT failed */
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "End of VCT test on rel_ifIndex %d",
                                              rel_ifIndex);

    /* END WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
    return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetVctTest
 *
 * DESCRIPTION: Set SolarFlare VCT test
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetVctTest(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    BOOLEAN energyDetetctEnabled = FALSE, exitReset = FALSE;
    UINT_16 val = 0;
    UINT_32 i, num_of_tries = 60;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus = MTD_FAIL;
    MPD_RESULT_ENT ret_val = MPD_OK_E;
    MTD_BOOL phyReady = MTD_FALSE;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    /* if port is down, VCT should not run */
    if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d admin is Down - VCT should not run",
                                                  rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "START VCT WA - disable energy detect on rel_ifIndex %d",
                                              rel_ifIndex);

    /* START WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
    /* disable energy detect - set 3.8000.9:8 to 00 */
    PRV_MPD_MTD_CALL(prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 3, 0x8000, &val),
                     rel_ifIndex,
                     mtdStatus);
    energyDetetctEnabled = ((val >> 8) & 0x3) ? TRUE : FALSE;

    if (energyDetetctEnabled) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 0),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "WA - rel_ifIndex %d energy detect disabled",
                                                  rel_ifIndex);

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "WA - rel_ifIndex %d mtdSoftwareReset",
                                                  rel_ifIndex);
        /* software reset to PHY - set 1.0000.15 to 1 (soft reset) and wait until 1.0000.15 self clears */
        PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex), 2000),
                        portEntry_PTR->rel_ifIndex,
                        mtdStatus);

        /* sleep and check up to 5 seconds */
        for (i = 0; i < num_of_tries; i++) {
            /* sleep 100 ms and check whether the test has finished */
            PRV_MPD_SLEEP_MAC(100);
            PRV_MPD_MTD_CALL(mtdIsPhyReadyAfterReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &phyReady),
                             rel_ifIndex,
                             mtdStatus);
            if (phyReady == MTD_TRUE) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "Exit after reset. rel_ifIndex %d, i = %d",
                                                          rel_ifIndex,
                                                          i);
                exitReset = TRUE;
                break;
            }
        }
        if (exitReset == FALSE)
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "prvMpdMtdSetVctTest - PHY not exit from RESET");
    }

    /* run VCT test */
    ret_val = prvMpdMtdVctUtility(portEntry_PTR,
                                  params_PTR);
    if (ret_val != MPD_OK_E) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "VCT test result:  rel_ifIndex %d, ret_val = %d",
                                                  rel_ifIndex,
                                                  ret_val);
    }
    else {
        /* if length is greater than 110 - rerun test */
        if (params_PTR->phyVct.cableLength > 110 ||
            (params_PTR->phyVct.cableLength == 0 && params_PTR->phyVct.testResult == MPD_VCT_RESULT_CABLE_OK_E)) {
            prvMpdMtdVctUtility(portEntry_PTR,
                                params_PTR);
        }
    }

    /* restore WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */
    if (energyDetetctEnabled) {

        /* restore energy detect - set 3.8000.9:8 back to 2b?11 */
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 3),
                         rel_ifIndex,
                         mtdStatus);

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "WA - rel_ifIndex %d mtdSoftwareReset",
                                                          rel_ifIndex);
        /* software reset to PHY - set 1.0000.15 to 1 (soft reset) and wait until 1.0000.15 self clears */
        PRV_MPD_MTD_CALL(mtdSoftwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex), 2000),
                portEntry_PTR->rel_ifIndex,
                mtdStatus);
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "RESTORE VCT WA - DONE- on rel_ifIndex %d",
                                              rel_ifIndex);
    /* END WA for PHYs 20X0 and 33X0 - for VCT test - after running the test - no link up on port */

    return ret_val;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetCableLenghNoBreakLink
 *
 * DESCRIPTION: Get cable length without breaking the link
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetCableLenghNoBreakLink(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_8 terminitationCode[4] = { 0,
                                    0,
                                    0,
                                    0
                                  }, length[4] = {  0,
                                                    0,
                                                    0,
                                                    0
                                                 };
    MTD_BOOL testDone = MTD_FALSE;
    UINT_32 rel_ifIndex;
    UINT_32 i, num_of_pairs = 4, len = 0;
    UINT_32 num_of_tries = 100;
    MTD_STATUS mtdStatus;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "Get passive cable length on rel_ifIndex %d",
                                              rel_ifIndex);

    /* start VCT test - DSP */
    PRV_MPD_MTD_CALL(mtdRun_CableDiagnostics(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_FALSE/*break link*/, MTD_TRUE/*disableInterPairShort*/),
                     rel_ifIndex,
                     mtdStatus);



    for (i = 0; i < num_of_tries; i++) {
        mtdStatus = mtdGet_CableDiagnostics_Status(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                   &testDone);
        if ((mtdStatus == MTD_OK) && (testDone == MTD_TRUE)) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "Exit in mtdGet_CableDiagnostics_Status. rel_ifIndex %d, i = %d",
                                                      rel_ifIndex,
                                                      i);
            break;
        }
        /* sleep 100 ms and check whether the test has finished */
        PRV_MPD_SLEEP_MAC(20);
    }

    if ((i < num_of_tries) && (testDone == MTD_TRUE)) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d, i=%d, test duration: %d[ms]",
                                                  rel_ifIndex,
                                                  i,
                                                  i * 20);

        PRV_MPD_MTD_CALL(mtdGet_CableDiagnostics_Results(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                         terminitationCode,
                                                         length), rel_ifIndex, mtdStatus);

        for (i = 0; i < num_of_pairs; i++) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d, pair No. %d, Length=%d, terminitationCode=%d",
                                                      rel_ifIndex,
                                                      i + 1,
                                                      length[i],
                                                      terminitationCode[i]);

            len += length[i];
            /* in case a pair is not OK - break and return this result */
            if (terminitationCode[i] != PRV_MPD_vctResult_pairOk_CNS) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "rel_ifIndex %d, pair No. %d, pair is not OK",
                                                          rel_ifIndex,
                                                          i + 1);

                return MPD_NOT_SUPPORTED_E;
            }
        }

        /* average of 4 pairs length */
        len /= num_of_pairs;

        if (len <= 50) {
            params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_LESS_THAN_50M_E;
        }
        else if (len <= 80) {
            params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_50M_80M_E;
        }
        else if (len <= 110) {
            params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_80M_110M_E;
        }
        else if (len <= 140) {
            params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_110M_140M_E;
        }
        else {
            params_PTR->phyCableLen.cableLength = MPD_CABLE_LENGTH_MORE_THAN_140M_E;
        }
        params_PTR->phyCableLen.accurateLength = len;

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d, Length=%d, phy_cableLen.accurateLength=%d",
                                                  rel_ifIndex,
                                                  len,
                                                  params_PTR->phyCableLen.accurateLength);
    }
    else {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "VCT-DSP (no-break link) test failed on rel_ifIndex %d",
                                                  rel_ifIndex);

        return MPD_OP_FAILED_E;
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdUpdateParallelList
 *
 * DESCRIPTION: Update parallel list to download fw into the flash/ram:
 *              To flash - The number of ports to download should be one per phy.
 *              To ram - The number of ports to download should be all the ports in the same phyType,InterfaceId, device \
 *                       because it done per port.
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X2580_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdUpdateParallelList(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_FW_DOWNLOAD_TYPE_ENT    fwDownloadType,
    /*     INPUTS / OUTPUTS:   */
    BOOLEAN *repPerPhyNumber_ARR
    /*     OUTPUTS:            */
)
{
    UINT_32 phyNumber;
    BOOLEAN isRepresentative = FALSE;
    MPD_TYPE_ENT    phyType;

    phyType = portEntry_PTR->initData_PTR->phyType;
    if (phyType >=  MPD_TYPE_NUM_OF_TYPES_E) {
        PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                   MPD_ERROR_SEVERITY_ERROR_E,
                                   "prvMpdMtdUpdateParallelList - Illegal PHY type");
        return MPD_OP_FAILED_E;
    }
    phyNumber = portEntry_PTR->initData_PTR->phyNumber;
    if (PRV_MPD_NONSHARED_GLBVAR_MPDMTD_REP_MODULO_ACCESS_MAC()[phyType] != 0) {
        if (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress % PRV_MPD_NONSHARED_GLBVAR_MPDMTD_REP_MODULO_ACCESS_MAC()[phyType] == 0) {
            isRepresentative = TRUE;
        }
    }
    else if (repPerPhyNumber_ARR[phyNumber] == FALSE) {
        isRepresentative = TRUE;
        repPerPhyNumber_ARR[phyNumber] = TRUE;
    }

    if (fwDownloadType == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        isRepresentative = TRUE;
    }
    if (isRepresentative) {
        portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports] =
            portEntry_PTR->initData_PTR->mdioInfo.mdioAddress;
        portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports++;
        if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
            PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       "prvMpdMtdUpdateParallelList - too many ports to download to");
            return MPD_OP_FAILED_E;

        }
        portEntry_PTR->runningData_PTR->isRepresentative = TRUE;
    }

    return MPD_OK_E;
}


/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdPrintFwVersion
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdPrintFwVersion(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    BOOLEAN    beforeFwDownload
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
#ifndef PHY_SIMULATION
    MPD_TYPE_ENT   phyType;
    UINT_8         major, minor, inc, test;
    char           fw_msg [256] = {0};
    MTD_STATUS     mtdStatus = MTD_OK;

    phyType = portEntry_PTR->initData_PTR->phyType;
    if (phyType >= MPD_TYPE_NUM_OF_TYPES_E) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, invalid PHY type %d",
                                                 portEntry_PTR->rel_ifIndex,
												 phyType);
        return MPD_OP_FAILED_E;
    }
    mtdStatus = mtdGetFirmwareVersion(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex),
                                      &major,
                                      &minor,
                                      &inc,
                                      &test);


    if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, mtdGetFirmwareVersion -FAIL!",
                                                 portEntry_PTR->rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    if (beforeFwDownload) {
        if (!PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()[phyType]) {
            prvMpdSnprintf(fw_msg, sizeof(fw_msg), "\n%s - FW Version: %u.%u.%u.%u\n",
                    prvMpdMtdConvertPhyNameToText_ARR[phyType],
                    major,
                    minor,
                    inc,
                    test);
            PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()[phyType] = TRUE;
        }
    }
    else if (PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()[phyType]) {
        prvMpdSnprintf(fw_msg, sizeof(fw_msg), "\n%s - FW Version: %u.%u.%u.%u\n",
                prvMpdMtdConvertPhyNameToText_ARR[phyType],
                major,
                minor,
                inc,
                test);
        PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()[phyType] = FALSE;
    }

    PRV_MPD_LOGGING_MAC(fw_msg);
#else
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(beforeFwDownload);
#endif
    return MTD_OK;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPreFwDownload
 *
 * DESCRIPTION: Pre FW download
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdPreFwDownload(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_8 major = 0, minor = 0, inc = 0, test = 0;
    UINT_32                  rel_ifIndex;
    PRV_MPD_sw_version_UNT fwPhy_version;
    MTD_STATUS mtdStatus = MTD_OK;
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    memset(&fwPhy_version, 0, sizeof(fwPhy_version));
    /* need to wait till phy is ready before reading phy fw version */
    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_FLASH_E) {
        if (prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E) != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, prvMpdMtdIsPhyReadyAfterFWDownload - FAIL, PHY IS NOT READY!",
                                                     rel_ifIndex);
        }
        /* get current FW version - only in flash, otherwise all version numbers are 0 */
        mtdStatus = mtdGetFirmwareVersion(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                          &major,
                                          &minor,
                                          &inc,
                                          &test);
        if (mtdStatus != MTD_OK) {
#ifndef PHY_SIMULATION
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, mtdGetFirmwareVersion -FAIL!",
                                                     rel_ifIndex);

            if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
                prvMpdMtdDebugRegDump(rel_ifIndex, MPD_DEBUG_REGISTERS_STRAP_DUMP_E);
            }

            PRV_MPD_MTD_CALL(mtdChipHardwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_SLEEP_MAC(100);
#endif
        }
        /* print current FW version */
        prvMpdMtdPrintFwVersion(portEntry_PTR, TRUE);

        fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS] = (UINT_8) major;
        fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS] = (UINT_8) minor;
        fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS] = (UINT_8) inc;
        fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS] = (UINT_8) test;

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, current FW version on PHY: major = %d, minor = %d inc = %d test = %d, in dWord = 0x%x",
                                                 rel_ifIndex,
                                                 fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS],
                                                 fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS],
                                                 fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS],
                                                 fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS],
                                                 fwPhy_version.ver_num);
    }
    else {   /* RAM download */
        /* HW reset on the representative port of the phy - this is done to cover the case in which the device has no reset circuit */
        if (portEntry_PTR->runningData_PTR->isRepresentative) {
#ifndef PHY_SIMULATION
            PRV_MPD_MTD_CALL(mtdChipHardwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_SLEEP_MAC(100);
            /* to print the downloaded FW in RAM download mode */
            PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()[portEntry_PTR->initData_PTR->phyType] = TRUE;
#endif
        }
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                             "FW version from Application: major = %d, minor = %d inc = %d test = %d, in dWord = 0x%x",
                                             PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS],
                                             PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS],
                                             PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS],
                                             PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS],
                                             PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_num);

    if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE) &&
        (fwPhy_version.ver_num != PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_num)) {
        portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired = TRUE;
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, Phy required fw download",
                                                 rel_ifIndex);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownloadByType
 *
 * DESCRIPTION: Help to apply FW download
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdMtdFwDownloadByType(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_FW_DOWNLOAD_TYPE_ENT downloadType,
    MTD_U16 portsToDownload[],
    UINT_16 numOfPortsToDownload
)
{

    UINT_16 error_code = 0, reg_val;
    UINT_16 error_code_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS] = { 0 };
    char error_msg [256];
    UINT_32 rel_ifIndex;
    UINT_32 idx;
    UINT_16 mdio_addr = 0;
    MTD_STATUS ret_val = MTD_OK;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    if (numOfPortsToDownload > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_ERROR_E,
                                   "Too many ports to download to");
        return MPD_OP_FAILED_E;
    }
    if (downloadType == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        if ((PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[portEntry_PTR->rel_ifIndex]->fw_main.data_PTR == NULL) ||
            (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[portEntry_PTR->rel_ifIndex]->fw_main.dataSize == 0)) {
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       "No fw to download on ram");
            return MPD_OP_FAILED_E;
        }
        ret_val = mtdParallelUpdateRamImage(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex),
                                            portsToDownload,
                                            PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR,
                                            PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.dataSize,
                                            numOfPortsToDownload,
                                            error_code_ports,
                                            &error_code);
        if (ret_val != MTD_OK) {
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 1, 0XC050, &reg_val);
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d: RAM download. num of parallel ports= %d, error_code = 0X%X, reg 0XC050 = 0X%X\n",
                    rel_ifIndex,
                    numOfPortsToDownload,
                    error_code,
                    reg_val);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       error_msg);
        }
        else {
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded = TRUE;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, downloaded to %d ports on PHY's ram",
                                                     rel_ifIndex,
                                                     numOfPortsToDownload);
        }
    }
    else {
        ret_val = mtdParallelUpdateFlash(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex),
                                         portsToDownload,
                                         PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR,
                                         PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.dataSize,
                                         numOfPortsToDownload,
                                         error_code_ports,
                                         &error_code);
        if (ret_val != MTD_OK) {
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d: Flash download. error_code = 0X%X\n",
                    rel_ifIndex,
                    error_code);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_ERROR_E,
                                       error_msg);
        }
        else {
            portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded = TRUE;
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, downloaded to %d ports on PHY's flash, ret_val = 0x%x error_code = 0x%x",
                                                     rel_ifIndex,
                                                     numOfPortsToDownload,
                                                     ret_val,
                                                     error_code);
            for (idx = 0; idx < numOfPortsToDownload; idx++) {
                mdio_addr = portsToDownload[idx];
                /* for 88e2580 & 88x3580 needed to be performed only on one port of the phy */
                if ((portEntry_PTR->initData_PTR->phyType == MPD_TYPE_88E2580_E) || (portEntry_PTR->initData_PTR->phyType == MPD_TYPE_88X3580_E)) {
                	if (mdio_addr % 8 != 0) {
                		continue;
                	}
                }
				/* this call Trigger the FW loading from the flash to the ram */
				PRV_MPD_MTD_CALL(mtdChipHardwareReset(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), mdio_addr),
								 rel_ifIndex,
								 ret_val);

				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														 "rel_ifIndex %d: mtdChipHardwareReset - mdio_addr 0x%x",
														 rel_ifIndex, mdio_addr);

            }
        }

    }

    return ret_val;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownload
 *
 * DESCRIPTION: FW download
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdFwDownload(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    static BOOLEAN write_msg = TRUE;
    UINT_32 rel_ifIndex;
    MTD_STATUS ret_val = MTD_OK;
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, no need to download FW",
                                                 rel_ifIndex);
        return TRUE;
    }

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex, MPD_ERROR_SEVERITY_FATAL_E, "num of parallel ports exceed maximum supported");
        return MPD_OP_FAILED_E;
    }
    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "\nThe 10G PHY firmware is being downloaded to RAM. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d: download FW to RAM. representative %s",
                                                 rel_ifIndex,
                                                 (portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE");
        if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) {
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                                                MPD_FW_DOWNLOAD_TYPE_RAM_E,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MTD_OK) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to RAM. FAILED",
                                                         rel_ifIndex);
            }
        }
    }
    else {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "\nThe 10G PHY firmware is being updated. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) && (portEntry_PTR->runningData_PTR->isRepresentative)) {
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                                                MPD_FW_DOWNLOAD_TYPE_FLASH_E,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MTD_OK) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to FLASH. FAILED",
                                                         rel_ifIndex);
            }
        }
    }
#ifdef PHY_SIMULATION
    ret_val = MTD_OK;
#endif

    return (ret_val == MTD_OK) ? MPD_OK_E : MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdFwDownload_1
 *
 * DESCRIPTION: FW download
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88E2580_E
 *                  MPD_TYPE_88X3580_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdFwDownload_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    static BOOLEAN write_msg = TRUE;
    UINT_32 rel_ifIndex, idx, lower_idx = 0, upper_idx = 0;
    MPD_RESULT_ENT ret_val = MTD_OK;
    UINT_16 mdio_addr = 0;
    UINT_16 parallel_list_upper_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS];
    UINT_16 parallel_list_lower_ports[MPD_MAX_PORTS_TO_DOWNLOAD_CNS];
    MTD_STATUS mtdStatus;
    UINT_16 val, relativePort = 0;

    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloadRequired == FALSE) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, no need to download FW",
                                                 rel_ifIndex);
        mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
								   1,
								   0xC04E,
								   &val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
		/* relative port in the phy, Range is 0-7 (bits 7:9) */
		relativePort = val & 0x380;
		/* checking if the phy is master/slave */
		if (portEntry_PTR->runningData_PTR->isRepresentative && (relativePort <= 3)) {
			/* errata for 88e2580 & 88x3580 needed to sync the clocks of 2 internal phys, needed to be performed only on one port of the phy */
			/* need to be done on the master phy, otherwise will cause to reset (or worse) on wrong mdio address */
			PRV_MPD_MTD_CALL(mtdChipHardwareReset(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
											 rel_ifIndex,
											 ret_val);

			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
												 "rel_ifIndex %d, mtdChipHardwareReset",
												 rel_ifIndex);

		}
        return TRUE;
    }

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports > MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex, MPD_ERROR_SEVERITY_FATAL_E, "num of parallel ports exceed maximum supported");
        return MPD_OP_FAILED_E;
    }
    if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[portEntry_PTR->initData_PTR->phyType] == MPD_FW_DOWNLOAD_TYPE_RAM_E) {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "\nThe 10G PHY firmware is being downloaded to RAM. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d: download FW to RAM. representative %s",
                                                 rel_ifIndex,
                                                 (portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE");

        if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) {
            /* Special sequence for X3580/E2580 starts here .... needed only for RAM download due to HW issue */
            for (idx = 0; idx < portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports; idx++) {
                mdio_addr = portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[idx];
                if (mdio_addr % 8 >= 4) {
                    parallel_list_upper_ports[upper_idx] = mdio_addr;
                    upper_idx++;
                }
                else {
                    parallel_list_lower_ports[lower_idx] = mdio_addr;
                    lower_idx++;
                }
            }

            /* Chip reset upper ports in each PHY */
            for (idx = 0; idx < upper_idx; idx++) {
                mdio_addr = parallel_list_upper_ports[idx];
                if (mdio_addr % 4 == 0) {
                    PRV_MPD_MTD_CALL(mtdHwXmdioWrite(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), mdio_addr, 31, 0xF001, (1<<14)),
                                     rel_ifIndex,
                                     ret_val);
                    /* Very important, delay needed after this or will fail */
                    PRV_MPD_SLEEP_MAC(10); /* Wait 10ms before any MDIO access */
                }
            }
            if (upper_idx >= MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
                return MPD_OP_FAILED_E;
            }
            /* download FW to upper ports */
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                                                MPD_FW_DOWNLOAD_TYPE_RAM_E,
                                                parallel_list_upper_ports,
                                                upper_idx);

            if (ret_val == MPD_OK_E) {
                /* Chip reset lower ports in each PHY */
                for (idx = 0; idx < lower_idx; idx++) {
                    mdio_addr = parallel_list_lower_ports[idx];
                    if (mdio_addr % 4 == 0) {
                        /* Special sequence for X3580/E2580 starts here .... */
                        /* Chip reset upper ports */
                        PRV_MPD_MTD_CALL(mtdHwXmdioWrite(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex), mdio_addr, 31, 0xF001, (1<<14)),
                                         rel_ifIndex,
                                         ret_val);
                        /* Very important, delay needed after this or will fail */
                        PRV_MPD_SLEEP_MAC(10); /* Wait 10ms before any MDIO access */
                    }
                }
                if (lower_idx >= MPD_MAX_PORTS_TO_DOWNLOAD_CNS) {
                    return MPD_OP_FAILED_E;
                }
                /* download FW to lower ports */
                ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                                                    MPD_FW_DOWNLOAD_TYPE_RAM_E,
                                                    parallel_list_lower_ports,
                                                    lower_idx);
                if (ret_val!= MPD_OK_E) {
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d, Failed to downloaded on lower ports on PHY's ram",
                                                             rel_ifIndex);
                }
            }
            else {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d, Failed to downloaded on upper ports on PHY's ram",
                                                         rel_ifIndex);
            }
        }
    }
    else {
        if (write_msg == TRUE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "\nThe 10G PHY firmware is being updated. This process might take a few minutes. The ports are not operational during this period");
            write_msg = FALSE;
        }
        if ((portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) && (portEntry_PTR->runningData_PTR->isRepresentative)) {
            ret_val = prvMpdMtdFwDownloadByType(portEntry_PTR,
                                                MPD_FW_DOWNLOAD_TYPE_FLASH_E,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list,
                                                portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports);
            if (ret_val != MPD_OK_E) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d: prvMpdMtdFwDownloadByType - download FW to FLASH. FAILED",
                                                         rel_ifIndex);
            }
        }
    }
#ifdef PHY_SIMULATION
    ret_val = MTD_OK;
#endif

    return (ret_val == MTD_OK) ? MPD_OK_E : MPD_OP_FAILED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdPostFwDownload
 *
 * DESCRIPTION: Post FW download
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdPostFwDownload(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32 rel_ifIndex;
#ifndef PHY_SIMULATION
    PRV_MPD_sw_version_UNT fwPhy_version;
    UINT_8 major = 0, minor = 0, inc = 0, test = 0;
#endif
    MPD_RESULT_ENT ret_val = MPD_OK_E;
    MPD_UNUSED_PARAM(params_PTR);

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR != NULL) {
        PRV_MPD_FREE_MAC(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR);
        PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR = NULL;
    }

    if (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded == FALSE) {
        ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_RESET_E);
        if (ret_val != MPD_OK_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d, phy not ready!",
                                                     rel_ifIndex);
        }
        return MPD_OK_E;
    }

    ret_val = prvMpdMtdIsPhyReady(portEntry_PTR, PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E);
    if (ret_val == MPD_OK_E) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d, Post download succeeded!",
                                                 rel_ifIndex);
    }
    else {
        return ret_val;
    }

#ifndef PHY_SIMULATION
    memset(&fwPhy_version, 0, sizeof(fwPhy_version));
    mtdGetFirmwareVersion(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                          &major,
                          &minor,
                          &inc,
                          &test);

    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS] = (UINT_8) major;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS] = (UINT_8) minor;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS] = (UINT_8) inc;
    fwPhy_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS] = (UINT_8) test;

    /* print current FW version */
    prvMpdMtdPrintFwVersion(portEntry_PTR, FALSE);

    if (fwPhy_version.ver_num != PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_version.ver_num) {
        PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                   MPD_ERROR_SEVERITY_ERROR_E,
                                   "The fw version that load to flash is not one that running \n");
    }
#endif

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeCapabilities
 *
 * DESCRIPTION: Get EEE capabilities
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeCapabilities(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_16 mtd_eee_supported_bits;
    UINT_32 i;
    UINT_32 rel_ifIndex;
    UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
#ifndef PHY_SIMULATION
    MTD_STATUS ret_val;
#endif

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

#ifdef PHY_SIMULATION
    mtd_eee_supported_bits = 0xE; /* set bits 1-3 --> 100m, 1g, 10g */
#else
    ret_val = mtdGetEEESupported(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                 &mtd_eee_supported_bits);
    if (ret_val != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: mtdGetEEESupported failed",
                                                    rel_ifIndex);

        return MPD_OP_FAILED_E;
    }
#endif

    for (i = 0; i < arr_size; i++) {
        if (mtd_eee_supported_bits & prvMpdConvertMtdToMpdEeeCapability[i].mtd_val) {
            params_PTR->phyEeeCapabilities.enableBitmap |= prvMpdConvertMtdToMpdEeeCapability[i].hal_val;
        }
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d read capabilities 0x%x returned capabilities 0x%x",
                                              rel_ifIndex,
                                              mtd_eee_supported_bits,
                                              params_PTR->phyEeeCapabilities.enableBitmap);

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiEnterTimer_2
 *
 * DESCRIPTION: Set EEE LPI Enter timer
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiEnterTimer_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);

    return MPD_OK_E;
    /* OriB, 19.02.17: according to PHY team instructions - we don't need this configuration (Ts is HW parameter); leave it with PHYs defaults */
#if 0
    UINT_8 numPorts, thisPort;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_DEVICE_ID phyRev;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdGetPhyRevision(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                      &phyRev,
                      &numPorts,
                      &thisPort);
    if (phyRev == MTD_REV_3240P_Z2) {
        return MPD_OK_E;
    }

    if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF004, 8, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF004, 0, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 8, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        return MPD_NOT_SUPPORTED_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d set lpi_enter_time %d to speed 0x%x",
                                              rel_ifIndex,
                                              params_PTR->phyEeeLpiTime.time_us,
                                              params_PTR->phyEeeLpiTime.speed);

    return MPD_OK_E;
#endif
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeAdvertise_2
 *
 * DESCRIPTION: Set EEE advertisiement
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeAdvertise_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_8 numPorts, thisPort;
    UINT_16 current_adv, input_adv, i;
#ifndef PHY_SIMULATION
    BOOLEAN restartAn;
#endif
    UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_DEVICE_ID phyRev;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdGetPhyRevision(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                      &phyRev,
                      &numPorts,
                      &thisPort);
    if (phyRev == MTD_REV_3240P_Z2) {
        return MPD_OK_E;
    }

    for (i = 0, input_adv = 0; i < arr_size; i++) {
        if (params_PTR->phyEeeAdvertize.speedBitmap & prvMpdConvertMtdToMpdEeeCapability[i].hal_val) {
            input_adv |= prvMpdConvertMtdToMpdEeeCapability[i].mtd_val;
        }
    }

    /* supports 100M, 1G and 10G */
    if (input_adv & ~(MTD_EEE_ALL)) {
        return MPD_NOT_SUPPORTED_E;
    }

#ifndef PHY_SIMULATION
    /* if port is UP - restart auto-neg */
    restartAn = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;
#endif
    /* get the current advertisement */
    mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               7,
                               0x003C,
                               &current_adv);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);

#ifndef PHY_SIMULATION
    if ((params_PTR->phyEeeAdvertize.advEnable == TRUE) && (current_adv != input_adv)) {
        PRV_MPD_MTD_CALL(mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), input_adv, restartAn),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d advertise EEE on speeds 0x%x and restart AN - %s",
                                                  rel_ifIndex,
                                                  params_PTR->phyEeeAdvertize.speedBitmap,
                                                  (restartAn == FALSE) ? "NO" : "YES");
    }
    else if ((params_PTR->phyEeeAdvertize.advEnable == FALSE) && (current_adv != 0)) {
        PRV_MPD_MTD_CALL(mtdHwXmdioWrite(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 7, 0x003C, 0),
                         rel_ifIndex,
                         mtdStatus);
        if (restartAn) {
            PRV_MPD_MTD_CALL(mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                             rel_ifIndex,
                             mtdStatus);
        }
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d not advertise EEE on any speed restart AN - %s",
                                                  rel_ifIndex,
                                                  (restartAn == FALSE) ? "NO" : "YES");
    }
#endif

    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeAdvertise_1
 *
 * DESCRIPTION: Set EEE advertisiement
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeAdvertise_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    MTD_U16 mtd_eee_advertise_bits = 0, current_adv;
#ifndef PHY_SIMULATION
    MTD_BOOL anRestart;
#endif
    UINT_32 i;
    UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (params_PTR->phyEeeAdvertize.advEnable == TRUE) {
        for (i = 0, mtd_eee_advertise_bits = 0; i < arr_size; i++) {
            if (params_PTR->phyEeeAdvertize.speedBitmap & prvMpdConvertMtdToMpdEeeCapability[i].hal_val) {
                mtd_eee_advertise_bits |= prvMpdConvertMtdToMpdEeeCapability[i].mtd_val;
            }
        }
    }

#ifndef PHY_SIMULATION
    anRestart = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? TRUE : FALSE;
#endif
    PRV_MPD_MTD_CALL(mtdGetAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                      &current_adv), rel_ifIndex, mtdStatus);

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())
    (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
     "rel_ifIndex %d: advertise [%s], input hal-speeds [%#x], current phy config [%#x], admin mode [%s]",
     rel_ifIndex,
     (params_PTR->phyEeeAdvertize.advEnable == TRUE) ? "ENABLE" : "DISBALE",
     params_PTR->phyEeeAdvertize.speedBitmap,
     current_adv,
     (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? "UP" : "DOWN");

#ifndef PHY_SIMULATION
    if ((params_PTR->phyEeeAdvertize.advEnable == TRUE) && (current_adv != mtd_eee_advertise_bits)) {
    	mtdStatus = mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                mtd_eee_advertise_bits,
                                anRestart);
        if (mtdStatus != MTD_OK) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdAdvert_EEE failed",
                                                        rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
    }
    else if ((params_PTR->phyEeeAdvertize.advEnable == FALSE) && (current_adv != MTD_EEE_NONE)) {
    	mtdStatus = mtdAdvert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                MTD_EEE_NONE,
                                anRestart);
        if (mtdStatus != MTD_OK) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "rel_ifIndex %d: mtdAdvert_EEE failed",
                                                        rel_ifIndex);
            return MPD_OP_FAILED_E;
        }
    }
#endif

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLegacyEnable_2
 *
 * DESCRIPTION: Set EEE legacy (master) enable
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLegacyEnable_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_8 numPorts, thisPort;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_DEVICE_ID phyRev;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdGetPhyRevision(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                      &phyRev,
                      &numPorts,
                      &thisPort);
    if (phyRev == MTD_REV_3240P_Z2) {
        return MPD_OK_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())
    (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
     "rel_ifIndex %d: Set EEE legacy %s",
     rel_ifIndex,
     (params_PTR->phyEeeMasterEnable.masterEnable == TRUE) ? "Enable" : "Disable");

    if (params_PTR->phyEeeMasterEnable.masterEnable == TRUE) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 0, 2, (MTD_U16)3),
                         rel_ifIndex,
                         mtdStatus);
        PRV_MPD_MTD_CALL(mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0xC033, 0, 2, (MTD_U16)0),
                         rel_ifIndex,
                         mtdStatus);
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeConfig
 *
 * DESCRIPTION:  Utility, Set EEE configuration
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeConfig(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OP_CODE_ENT phyOperation,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    MTD_U8 eeeMode, xg_enter, xg_exit, g_enter, g_exit, m_enter, m_exit, ipgLen;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_BOOL restart_auto_neg = MTD_FALSE, reconfigure = MTD_FALSE;
    MTD_U16 speedVal = 0;
    MTD_BOOL speedIsForced;


    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    /* read current configuration */
    mtdStatus = mtdGetEeeBufferConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                    &eeeMode,
                                    &xg_enter,
                                    &g_enter,
                                    &m_enter,
                                    &xg_exit,
                                    &g_exit,
                                    &m_exit,
                                    &ipgLen);
    if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: mtdGetEeeBufferConfig failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC()) (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
    		"rel_ifIndex %d, EEE oper %d. current configuration: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d",
			rel_ifIndex,
			phyOperation,
			(eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
			xg_enter,
			xg_exit,
			g_enter,
			g_exit,
			m_enter,
			m_exit,
			ipgLen);

    switch (phyOperation) {
        case MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E:
        	if (((eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) && !params_PTR->phyEeeMasterEnable.masterEnable) ||
				((eeeMode != MTD_EEE_MODE_ENABLE_WITH_LEGACY) && params_PTR->phyEeeMasterEnable.masterEnable)) {
        		reconfigure = MTD_TRUE;
        		PRV_MPD_MTD_CALL(mtdGetForcedSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speedIsForced, &speedVal),
        				rel_ifIndex,
						mtdStatus);
				restart_auto_neg = ((speedIsForced == MTD_TRUE) || (portEntry_PTR->runningData_PTR->adminMode == FALSE)) ? MTD_FALSE : MTD_TRUE;
        	}
            eeeMode = (params_PTR->phyEeeMasterEnable.masterEnable == TRUE) ? MTD_EEE_MODE_ENABLE_WITH_LEGACY : MTD_EEE_MODE_DISABLE;

            break;

        case MPD_OP_CODE_SET_LPI_ENTER_TIME_E:
            if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
            	if (m_enter != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					m_enter = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
            	if (g_enter != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					g_enter = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
            	if (xg_enter != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					xg_enter = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else {
                return MPD_NOT_SUPPORTED_E;
            }
            break;

        case MPD_OP_CODE_SET_LPI_EXIT_TIME_E:
            if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
            	if (m_exit != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					m_exit = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
            	if (g_exit != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					g_exit = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
            	if (xg_exit != params_PTR->phyEeeLpiTime.time_us) {
					reconfigure = MTD_TRUE;
					xg_exit = params_PTR->phyEeeLpiTime.time_us;
				}
            }
            else {
                return MPD_NOT_SUPPORTED_E;
            }
            break;

        default:
            return MPD_NOT_SUPPORTED_E;
            break;
    }

    if (reconfigure) {
		/* configure new configuration */
    	mtdStatus = mtdEeeBufferConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
									 eeeMode,
									 xg_enter,
									 g_enter,
									 m_enter,
									 xg_exit,
									 g_exit,
									 m_exit,
									 ipgLen,
									 restart_auto_neg);

		PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC()) (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			 "rel_ifIndex %d: new configuration: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d, ret_val: %d. restartAutoNeg %d",
			 rel_ifIndex,
			 (eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
			 xg_enter,
			 xg_exit,
			 g_enter,
			 g_exit,
			 m_enter,
			 m_exit,
			 ipgLen,
			 mtdStatus,
			 restart_auto_neg);

		if (mtdStatus != MTD_OK) {
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														"rel_ifIndex %d: mtdGetEeeBufferConfig failed",
														rel_ifIndex);
			return MPD_OP_FAILED_E;
		}
    }

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiEnterTimer_1
 *
 * DESCRIPTION: PHY 33x0, 20x0
 *
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiEnterTimer_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    /* OriB, 19.02.17: according to PHY team instructions - we don't need this configuration (Ts is HW parameter); leave it with PHYs defaults
     return prvMpdMtdSetEeeConfig(rel_ifIndex, MPD_OP_CODE_SET_LPI_ENTER_TIME_E, params_PTR);*/
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLegacyEnable_1
 *
 * DESCRIPTION: PHY 33x0
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLegacyEnable_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    return prvMpdMtdSetEeeConfig(portEntry_PTR,
                                 MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                                 params_PTR);
}

/* ***************************************************************************
  * FUNCTION NAME: prvMpdMtdGetEeeStatus_2
 *
 * DESCRIPTION: Get EEE remote status
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdGetEeeStatus_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

#ifndef PHY_SIMULATION
	UINT_16 local_status_bit;
#endif
    UINT_16 val, temp;
    UINT_32 bit_map_ARR[4] = {  0,
                                MPD_SPEED_CAPABILITY_100M_CNS,
                                MPD_SPEED_CAPABILITY_1G_CNS,
                                MPD_SPEED_CAPABILITY_10G_CNS
                             };
    UINT_32 rel_ifIndex, i;
    MTD_STATUS  mtdStatus = MPD_OK_E;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

#ifdef PHY_SIMULATION
    val = 8; /* WM - remote advertise EEE 10G capability */
    params_PTR->phyEeeStatus.localEnable = FALSE;
#else
    mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               7,
                               61,
                               &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
    mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                    31,
                                                    0xF003,
                                                    1,
                                                    1,
                                                    &local_status_bit);
    if (local_status_bit) {
        params_PTR->phyEeeStatus.localEnable = TRUE;
    }
    else
    {
        params_PTR->phyEeeStatus.localEnable = FALSE;
    }
#endif
    for (i = 1; i < 4; i++) {
        temp = val & (1 << i);
        if (temp != 0) {
            params_PTR->phyEeeStatus.enableBitmap |= bit_map_ARR[i];
        }
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d result 0x%x returned value 0x%x, local %s",
                                              rel_ifIndex,
                                              val,
                                              params_PTR->phyEeeStatus.enableBitmap,
                                              params_PTR->phyEeeStatus.localEnable?"Enable":"Disable");

    return mtdStatus;

}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeStatus_1
 *
 * DESCRIPTION: PHY 33x0
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeStatus_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

#ifndef PHY_SIMULATION
	UINT_16 local_status_bit;
#endif
    UINT_16 eee_lp_advert;
    UINT_32 i;
    UINT_32 arr_size = sizeof(prvMpdConvertMtdToMpdEeeCapability) / sizeof(PRV_MPD_MTD_TO_MPD_CONVERT_STC);
    UINT_32 rel_ifIndex;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    params_PTR->phyEeeStatus.enableBitmap = 0;
    if (mtdGetLP_Advert_EEE(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            &eee_lp_advert) != MTD_OK) {

        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: mtdGetLP_Advert_EEE failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }

    for (i = 0; i < arr_size; i++) {
        if (eee_lp_advert & prvMpdConvertMtdToMpdEeeCapability[i].mtd_val) {
            params_PTR->phyEeeStatus.enableBitmap |= prvMpdConvertMtdToMpdEeeCapability[i].hal_val;
        }
    }

#ifdef PHY_SIMULATION
    params_PTR->phyEeeStatus.localEnable = FALSE;
#else
    mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                        31,
                        0xF003,
                        1,
                        1,
                        &local_status_bit);
    if (local_status_bit) {
        params_PTR->phyEeeStatus.localEnable = TRUE;
    }
    else
    {
        params_PTR->phyEeeStatus.localEnable = FALSE;
    }
#endif

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d result 0x%x returned value 0x%x, local %s",
                                              rel_ifIndex,
                                              eee_lp_advert,
                                              params_PTR->phyEeeStatus.enableBitmap,
                                              params_PTR->phyEeeStatus.localEnable?"Enable":"Disable");

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetEeeConfig_1
 *
 * DESCRIPTION: Get EEE configuration from MTD driver.
 *               PHYs: 3220, 33x0
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetEeeConfig_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    MTD_U8 eeeMode, xg_enter, xg_exit, g_enter, g_exit, m_enter, m_exit, ipgLen;
    UINT_32 rel_ifIndex = portEntry_PTR->rel_ifIndex;
    MTD_STATUS rc;

#ifdef PHY_SIMULATION
    eeeMode=MTD_EEE_MODE_ENABLE_WITH_LEGACY;
    xg_enter=0x1A;
    xg_exit=8;
    g_enter=0x12;
    g_exit=0x12;
    m_enter=0x20;
    m_exit=0x20;
    ipgLen=0xc;
    rc = MTD_OK;
#else
    rc = mtdGetEeeBufferConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               &eeeMode,
                               &xg_enter,
                               &g_enter,
                               &m_enter,
                               &xg_exit,
                               &g_exit,
                               &m_exit,
                               &ipgLen);
#endif
    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())
    (PRV_MPD_DEBUG_FUNC_NAME_MAC(),
     "rel_ifIndex %d: eeeMode %s, xg_enter %d, xg_exit %d, g_enter %d, g_exit %d, m_enter %d, m_exit %d, ipgLen %d",
     rel_ifIndex,
     (eeeMode == MTD_EEE_MODE_ENABLE_WITH_LEGACY) ? "ENABLE-LEGACY" : "DISABLE",
     xg_enter,
     xg_exit,
     g_enter,
     g_exit,
     m_enter,
     m_exit,
     ipgLen);

    if (rc != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: mtdGetEeeBufferConfig failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_100M_E] = m_exit;
    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_1G_E] = g_exit;
    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_10G_E] = xg_exit;
    /* PATCH!!! - need values from PHY driver!!! */
    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_2500M_E] = xg_exit;
    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[MPD_EEE_SPEED_5G_E] = xg_exit;

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetNearEndLoopback_2
 *
 * DESCRIPTION: PHY 3240
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetNearEndLoopback_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_32					rel_ifIndex;

    rel_ifIndex  = portEntry_PTR->rel_ifIndex;
    if (params_PTR->internal.phyLoopback.enable) {
        mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            3,
                            0,
                            14,
                            1,
                            1);
    }
    else {
        prvMpdMtdReset32x0(portEntry_PTR,
                           50);
    }
    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetNearEndLoopback_3
 *
 * DESCRIPTION: PHY 3240
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetNearEndLoopback_3(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
	UINT_32					rel_ifIndex;
	MPD_LOOPBACK_MODE_TYP	lobback_mode;
	BOOLEAN					enable_loopback;
	UINT_16					speed, loopBackSpeed;
	MTD_STATUS				rc;
	/*blocking ingress data from H-unit*/
	BOOLEAN					passThrough = FALSE;
	MTD_BOOL				linkUp;
	char					error_msg [256];

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
	lobback_mode = params_PTR->internal.phyLoopback.loopbackMode;
	enable_loopback = params_PTR->internal.phyLoopback.enable;

    switch(lobback_mode){
		case MPD_LOOPBACK_MODE_DEEP_MAC_CNS:
			if(portEntry_PTR->runningData_PTR->sfpPresent){
				rc = mtdSetHXunitMacLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_X_UNIT, passThrough, enable_loopback);
			}
			else{
				PRV_MPD_MTD_CALL(mtdIsBaseTUp(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speed, &linkUp),rel_ifIndex, rc);

				if(linkUp == FALSE){
                    prvMpdSnprintf(error_msg, sizeof(error_msg), "loopback in direction to line require link up \n");
					PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
						MPD_ERROR_SEVERITY_MINOR_E,
						error_msg);
					return MTD_FAIL;
				}
				rc = mtdSetTunitDeepMacLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speed, enable_loopback);
			}
			break;
		case MPD_LOOPBACK_MODE_SHALLOW_LINE_CNS:
			if(portEntry_PTR->runningData_PTR->sfpPresent){
				rc = mtdSetHXunitMacLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_X_UNIT, passThrough, enable_loopback);
			}
			else{
				PRV_MPD_MTD_CALL(mtdIsBaseTUp(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speed, &linkUp),rel_ifIndex, rc);

				if(linkUp == FALSE){
                    prvMpdSnprintf(error_msg, sizeof(error_msg), "linked is down could could not get speed \n");
					PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
						MPD_ERROR_SEVERITY_MINOR_E,
						error_msg);
					return MTD_FAIL;
				}

				loopBackSpeed = (speed <= MTD_SPEED_1GIG_FD) ? MTD_1G_BELOW_LB_SPEED : MTD_2P5G_ABOVE_LB_SPEED;
				rc = mtdSetTunitShallowLineLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), loopBackSpeed, enable_loopback);
			}
			break;
		case MPD_LOOPBACK_MODE_SHALLOW_MAC_CNS:
			rc = mtdSetHXunitMacLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_H_UNIT, passThrough, enable_loopback);
			break;
		case MPD_LOOPBACK_MODE_DEEP_LINE_CNS:
			rc = mtdSetHXunitLineLoopback(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_H_UNIT, passThrough, enable_loopback);
			break;
		default:
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
				"rel_ifIndex %d: loopback operation failed",
				rel_ifIndex);
			return MPD_OP_FAILED_E;
	}

    return rc;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetDuplexMode_2
 *
 * DESCRIPTION: Set Duplex mode
 *          Relevant only if AN is disabled in 10M or 100M
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetDuplexMode_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    char *duplex_text = "FULL";
    UINT_16 speed_val, new_val;
    MTD_BOOL speedIsForced;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_RESULT_ENT ret = MPD_OK_E;
    BOOLEAN adminDown;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (portEntry_PTR->runningData_PTR->sfpPresent) {
    	PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
    							"rel_ifIndex %d port in fiber mode, skip configuration",
    							rel_ifIndex);
    	return ret;
    }

    /* check if works in 100M - maybe need to enable parallel detect and disable it in other speeds */
    mtdGetForcedSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                      &speedIsForced,
                      &speed_val);

    if (speedIsForced == MTD_TRUE) {
        if ((((speed_val == MTD_SPEED_10M_HD_AN_DIS) || (speed_val == MTD_SPEED_100M_HD_AN_DIS)) && (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E)) ||
            (((speed_val == MTD_SPEED_10M_FD_AN_DIS) || (speed_val == MTD_SPEED_100M_FD_AN_DIS)) && (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_HALF_E))) {

			adminDown = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) ? TRUE : FALSE;
			if (adminDown) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex %d remove from low-power mode to configure forceMode",
						rel_ifIndex);
				/* remove from low-power mode */
				PRV_MPD_MTD_CALL(mtdRemoveTunitLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
								 rel_ifIndex,
								 mtdStatus);
			}
			new_val = speed_val;
			if (params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E) {
				new_val = new_val << 1;
			}
			else {
				new_val = new_val >> 1;
				duplex_text = "HALF";
			}

			PRV_MPD_MTD_CALL(mtdForceSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), new_val),
							 rel_ifIndex,
							 mtdStatus);
			PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
														 "rel_ifIndex %d config %s DUPLEX",
														 rel_ifIndex,
														 duplex_text);
			if (adminDown) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex %d return to low-power mode after configure forceMode",
						rel_ifIndex);
				PRV_MPD_MTD_CALL(mtdPutTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
																	 rel_ifIndex,
																	 mtdStatus);
			}
		}

    }

    return ret;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiExitTimer_2
 *
 * DESCRIPTION: Set EEE LPI Exit timer
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiExitTimer_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_8 numPorts, thisPort;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MTD_DEVICE_ID phyRev;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    mtdGetPhyRevision(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                      &phyRev,
                      &numPorts,
                      &thisPort);
    if (phyRev == MTD_REV_3240P_Z2) {
        return MPD_OK_E;
    }

    if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_100M_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF006, 8, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_1G_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF006, 0, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else if (params_PTR->phyEeeLpiTime.speed == MPD_EEE_SPEED_10G_E) {
        PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 31, 0xF005, 8, 8, params_PTR->phyEeeLpiTime.time_us),
                         rel_ifIndex,
                         mtdStatus);
    }
    else {
        return MPD_NOT_SUPPORTED_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d set lpi_exit_time %d to speed 0x%x",
                                              rel_ifIndex,
                                              params_PTR->phyEeeLpiTime.time_us,
                                              params_PTR->phyEeeLpiTime.speed);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetEeeLpiExitTimer_1
 *
 * DESCRIPTION: Set EEE LPI Exit timer
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetEeeLpiExitTimer_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    return prvMpdMtdSetEeeConfig(portEntry_PTR,
                                 MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                                 params_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetPowerModules
 *
 * DESCRIPTION: Enable Green Ethernet Short Reach
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetPowerModules(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    /*ED treatment*/
    if (params_PTR->phyPowerModules.energyDetetct != MPD_GREEN_NO_SET_E) {
        if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_ENABLE_E) {

            PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 3),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d Energy detect enable",
                                                      rel_ifIndex);
        } /* enable ED */
        else if (params_PTR->phyPowerModules.energyDetetct == MPD_GREEN_DISABLE_E) {
            PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0x8000, 8, 2, 0),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d Energy detect disable",
                                                      rel_ifIndex);
        } /* disable ED */
        if (params_PTR->phyPowerModules.performPhyReset) {
            prvMpdMtdReset32x0(portEntry_PTR,
                               50);
        }
    } /* ED */

    /*SR treatment*/
    if (params_PTR->phyPowerModules.shortReach != MPD_GREEN_NO_SET_E) {
        if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_ENABLE_E) {
            PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0083, 0, 1, 1),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d Short reach enable",
                                                      rel_ifIndex);
        } /* enable SR */
        else if (params_PTR->phyPowerModules.shortReach == MPD_GREEN_DISABLE_E) {
            PRV_MPD_MTD_CALL(mtdHwSetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 1, 0x0083, 0, 1, 0),
                             rel_ifIndex,
                             mtdStatus);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                      "rel_ifIndex %d Short reach disable",
                                                      rel_ifIndex);
        } /* disable SR */
    } /* SR */

    return MPD_OK_E;
}

/*****************************************************************************
 * FUNCTION NAME: prvMpdMtdUpdateCUnitMediaAndSpeed
 *
 * DESCRIPTION:
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X32x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdUpdateCUnitMediaAndSpeed(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_32                      speed
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */

)
{
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus, swReset;
    MTD_U16 frameToRegister, mediaSelect, fiberType;
    MTD_BOOL npMediaEnergyDetect, maxPowerTunitAMDetect;
    UINT_32 fiberSpeed = MTD_FT_1000BASEX;
    BOOLEAN configure = FALSE;
    UINT_16 val = 0;
    MPD_OPERATIONS_PARAMS_UNT phy_params;
    MPD_RESULT_ENT ret_val = MPD_OK_E;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    /* check if we have X-UNIT */
    if (MTD_HAS_X_UNIT(PRV_MPD_MTD_OBJECT_MAC(rel_ifIndex))) {
        if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_COMBO_E) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d. Skip, not a combo port",
                                                         rel_ifIndex);
            return MPD_OK_E;
        }

        memset(&phy_params, 0, sizeof(phy_params));
        prvMpdPerformPhyOperation(portEntry_PTR, PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E, &phy_params);

        /* get current media */
        PRV_MPD_MTD_CALL(mtdGetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                              &frameToRegister, &mediaSelect, &fiberType, &npMediaEnergyDetect, &maxPowerTunitAMDetect),
                         rel_ifIndex,
                         mtdStatus);

        /* check if Fiber is present */
        if (portEntry_PTR->runningData_PTR->sfpPresent) {
            switch (speed) {
                case MPD_SPEED_1000M_E:
                    fiberSpeed = MTD_FT_1000BASEX;
                    break;
                case MPD_SPEED_2500M_E:
                    fiberSpeed = MTD_FT_2500BASEX;
                    break;
                case MPD_SPEED_5000M_E:
                    fiberSpeed = MTD_FT_5GBASER;
                    break;
                case MPD_SPEED_10000M_E:
                    fiberSpeed = MTD_FT_10GBASER;
                    break;
                default:
                    fiberSpeed = MTD_FT_NONE;
            }

            /* if desired speed was not selected, update media type will occur in set speed operation */
            if (fiberSpeed == MTD_FT_NONE) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. No valid speed received %d",
                                                             rel_ifIndex, speed);
                return MPD_OK_E;
            }

            mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0xF002, &val);
            if (val == 0) {
                /* 7.800F.7:4 has the correct value and only 3.F002 is incorrect.
                    And when we run rerunSerdesInitialization, it syncs up the firmware with the change to 3.F002.*/
                PRV_MPD_MTD_CALL(mtdHwXmdioWrite(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), 3, 0xF002, 7),
                                 rel_ifIndex,
                                 mtdStatus);
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. Apply WA 3.F002=7",
                                                             rel_ifIndex);
                configure = TRUE;
            }

            /* configured media type & speed are as desired */
            if ((mediaSelect == phy_params.internal.phyMediaSelect.value) && (fiberSpeed == fiberType)) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. No change (fiber mode), speed %d",
                                                             rel_ifIndex, fiberSpeed);
            }
            else {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. Apply fiber mode, using new speed %d",
                                                             rel_ifIndex, fiberSpeed);
                configure = TRUE;
            }
        }
        else {   /* if Fiber is not present, desired media is Copper */
            /* configured media is copper */
            if (mediaSelect == phy_params.internal.phyMediaSelect.value) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. No change (copper mode)",
                                                             rel_ifIndex);

            }
            else {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d. Apply copper mode",
                                                             rel_ifIndex);

                configure = TRUE;
            }
        }

        if (configure) {
            /* if admin down sw reset  = false,*/
            swReset = (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? MTD_TRUE : MTD_FALSE;
            PRV_MPD_MTD_CALL(mtdSetCunitTopConfig(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), MTD_F2R_OFF, phy_params.internal.phyMediaSelect.value,
                                                  fiberSpeed, MTD_FALSE, MTD_FALSE, swReset, MTD_TRUE),
                             rel_ifIndex,
                             mtdStatus);
        }
        /* we skipped SW reset in admin up due to AMD powering down the T unit */
        if (portEntry_PTR->runningData_PTR->softwareResetRequired) {
        	if (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_DOWN_E) {
				portEntry_PTR->runningData_PTR->softwareResetRequired = FALSE;
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex [%d] moved back to admin down, skip SW reset as it will be done in admin up",
						rel_ifIndex);
			} else {
				ret_val = prvMpdMtdIsPhyReady(portEntry_PTR,PRV_MPD_PHY_READY_WITH_SW_RESET_E);
				if (ret_val != MPD_OK_E) {
					PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															 "rel_ifIndex %d, phy not ready after PRV_MPD_PHY_READY_WITH_SW_RESET_E!",
															 rel_ifIndex);
				}
			}
        }
    }

    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetSpeed_5
 *
 * DESCRIPTION: work with one speed
 *
 * note: speed 10M, 100M are forced without negotiation
 *       higher speeds are working with negotiation enabled and advertising one speed
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X32x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetSpeed_5(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 speedVal = 0, autoNegEnabled;
    UINT_16 supportedCapabilities = 0;
    MTD_BOOL speedIsForced = MTD_FALSE, config = FALSE;
    BOOLEAN restartAutoneg;
    UINT_32 speedConf = 0;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_RESULT_ENT ret = MPD_OK_E;
    char error_msg [256];

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    restartAutoneg =
        ((portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) && (!portEntry_PTR->runningData_PTR->sfpPresent)) ? TRUE : FALSE;

    switch (params_PTR->phySpeed.speed) {
        case MPD_SPEED_10M_E:
        case MPD_SPEED_100M_E:
        	/* calculate which speed we desire to configure */
        	if (portEntry_PTR->runningData_PTR->duplex == MPD_DUPLEX_ADMIN_MODE_FULL_E) {
				speedConf = (params_PTR->phySpeed.speed == MPD_SPEED_10M_E) ? MTD_SPEED_10M_FD_AN_DIS : MTD_SPEED_100M_FD_AN_DIS;
			}
			else {
				speedConf = (params_PTR->phySpeed.speed == MPD_SPEED_10M_E) ? MTD_SPEED_10M_HD_AN_DIS : MTD_SPEED_100M_HD_AN_DIS;
			}
			mtdGetForcedSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
							  &speedIsForced,
							  &speedVal);
			/* check if there is a change */
			if (speedIsForced && (speedVal == speedConf)) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
										 "rel_ifIndex %d, forced speed %s. current equal desired skip config.",
										 rel_ifIndex,
										 prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
				return MPD_OK_E;
			}
			/* if port in low power mode we need to remove before applying force mode */
			if (!restartAutoneg) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex %d remove from low-power mode to configure forceMode",
						rel_ifIndex);
				/* remove from low-power mode */
				PRV_MPD_MTD_CALL(mtdRemoveTunitLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
								 rel_ifIndex,
								 mtdStatus);
			}

			PRV_MPD_MTD_CALL(mtdForceSpeed(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speedConf),
										 rel_ifIndex,
										 mtdStatus);
            /* need to return the PHY to shutdown */
			if (!restartAutoneg) {
				PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
						"rel_ifIndex %d return to low-power mode after configure forceMode",
						rel_ifIndex);
				PRV_MPD_MTD_CALL(mtdPutTunitInLowPowerMode(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex)),
																	 rel_ifIndex,
																	 mtdStatus);
			}
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d, restartAutoneg - %s, MTD 0x%x, MPD %d speed %s",
                                                         rel_ifIndex,
                                                         (restartAutoneg) ? "YES" : "No",
                                                         speedConf,
                                                         params_PTR->phySpeed.speed,
                                                         prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));

            break;
        case MPD_SPEED_1000M_E:
            speedConf = MTD_SPEED_1GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_2500M_E:
            speedConf = MTD_SPEED_2P5GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_5000M_E:
            speedConf = MTD_SPEED_5GIG_FD;
            config = TRUE;
            break;
        case MPD_SPEED_10000M_E:
            speedConf = MTD_SPEED_10GIG_FD;
            config = TRUE;
            break;

        default:
            return MPD_NOT_SUPPORTED_E;
    }
    if (config == TRUE) {
        /* get all available supported speeds in the PHY */
        PRV_MPD_MTD_CALL(mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &supportedCapabilities),
                         rel_ifIndex,
                         mtdStatus);
        /* validate desired speed is supported */
        if ((supportedCapabilities & speedConf) != speedConf) {
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - some desired capabilities [%d] are not supported [%d]\n", portEntry_PTR->rel_ifIndex, speedConf, supportedCapabilities);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_MINOR_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }

        ret = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, speedConf);
        if (ret != MPD_OK_E) {
            prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - desired MTD speed %d doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex, speedConf);
            PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                       MPD_ERROR_SEVERITY_MINOR_E,
                                       error_msg);
            return MPD_OP_FAILED_E;
        }
        /* get auto-negotiation value and advertise capabilities */
        mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            7,
                            0,
                            12,
                            1,
                            &autoNegEnabled);
        mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),  &speedVal);
        if ((autoNegEnabled == FALSE) || (speedVal != speedConf) || (portEntry_PTR->runningData_PTR->sfpPresent)) {
            prvMpdMtdUpdateCUnitMediaAndSpeed(portEntry_PTR, params_PTR->phySpeed.speed);
            if (!portEntry_PTR->runningData_PTR->sfpPresent){
                PRV_MPD_MTD_CALL(mtdEnableSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), speedConf, restartAutoneg),
                    rel_ifIndex,
                    mtdStatus);

            }
            if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
                PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                             "rel_ifIndex %d restartAutoneg [%s] MTD [%d] MPD [%d] speed [%s]",
                                                             rel_ifIndex,
                                                             (restartAutoneg) ? "YES" : "No",
                                                             speedConf,
                                                             params_PTR->phySpeed.speed,
                                                             prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
            }
        }
        else {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d, speed %s. current equal desired or autoNegEnabled is %d, skip config.",
                                                         rel_ifIndex,
                                                         prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed),
                                                         autoNegEnabled);
        }
    }


    return ret;
}

static MPD_RESULT_ENT prvMpdMtdGetHighestSpeed(
    /*     INPUTS:             */
    UINT_16 negCapabilities,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_32 * speed_PTR
)
{
    UINT_32 speed = MPD_SPEED_LAST_E;

    if (!speed_PTR) {
        return MPD_OP_FAILED_E;
    }

    if (negCapabilities & MPD_AUTONEG_CAPABILITIES_200G_FULL_CNS) {
        speed = MPD_SPEED_200G_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS) {
        speed = MPD_SPEED_100G_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS) {
        speed = MPD_SPEED_50G_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS) {
        speed = MPD_SPEED_40G_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS) {
        speed = MPD_SPEED_25G_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS) {
        speed = MPD_SPEED_10000M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS) {
        speed = MPD_SPEED_5000M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS) {
        speed = MPD_SPEED_2500M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
        speed = MPD_SPEED_1000M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
        speed = MPD_SPEED_100M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
        speed = MPD_SPEED_100M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
        speed = MPD_SPEED_10M_E;
    }
    else if (negCapabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
        speed = MPD_SPEED_10M_E;
    }

    * speed_PTR = speed;

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetAutoNeg_5
 *
 * DESCRIPTION: PHY 2180
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetAutoNeg_5(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    typedef MTD_STATUS masterSlave_func(
        MTD_DEV_PTR,
        UINT_16,
        MTD_BOOL);

    masterSlave_func *func_PTR = NULL;
    UINT_16 val = 0, master_stat;
    UINT_16 capabilities = 0, supportedCapabilities = 0;
    UINT_16 autoNegEnabled;
    BOOLEAN configNeeded = FALSE;
    BOOLEAN set_master = FALSE, restartAn = FALSE, restartAnSlaveMaster = FALSE;
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus;
    MPD_RESULT_ENT status;
    UINT_32 speed = MPD_SPEED_LAST_E;
    char prvMpdCapabilitiesString[256];
    char error_msg [256];

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    if (params_PTR->phyAutoneg.enable == MPD_AUTO_NEGOTIATION_ENABLE_E) {

        /* get all available supported speeds in the PHY */
        PRV_MPD_MTD_CALL(mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &supportedCapabilities),
                         rel_ifIndex,
                         mtdStatus);

        if ((params_PTR->phyAutoneg.capabilities == 0) ||
            (params_PTR->phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS) ||
            (params_PTR->phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)) {
            capabilities = supportedCapabilities;
            /* 1G Half Duplex is not supported */
            capabilities &= ~MTD_SPEED_1GIG_HD;

            if (capabilities & MTD_SPEED_5GIG_FD) {
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_5GIG_FD);
                if (status != MPD_OK_E) {
                    capabilities &= ~MTD_SPEED_5GIG_FD;
                }
            }
            if (capabilities & MTD_SPEED_10GIG_FD) {
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_10GIG_FD);
                if (status != MPD_OK_E) {
                    capabilities &= ~MTD_SPEED_10GIG_FD;
                }
            }
        }
        else {
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS) {
                capabilities |= MTD_SPEED_10M_HD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS) {
                capabilities |= MTD_SPEED_10M_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS) {
                capabilities |= MTD_SPEED_100M_HD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS) {
                capabilities |= MTD_SPEED_100M_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS) {
                capabilities |= MTD_SPEED_1GIG_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS) {
                capabilities |= MTD_SPEED_2P5GIG_FD;
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS) {
                capabilities |= MTD_SPEED_5GIG_FD;
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_5GIG_FD);
                if (status != MPD_OK_E) {
                    prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - desired speed 5G doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex);
                    PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                               MPD_ERROR_SEVERITY_MINOR_E,
                                               error_msg);
                    return MPD_OP_FAILED_E;
                }
            }
            if (params_PTR->phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS) {
                capabilities |= MTD_SPEED_10GIG_FD;
                status = prvMpdMtdValidateUsxSpeeds(portEntry_PTR, MTD_SPEED_10GIG_FD);
                if (status != MPD_OK_E) {
                    prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - desired speed 10G doesn't match usx mode capabilities\n", portEntry_PTR->rel_ifIndex);
                    PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                               MPD_ERROR_SEVERITY_MINOR_E,
                                               error_msg);
                    return MPD_OP_FAILED_E;
                }
            }
            /* validate desired capabilities are supported */
            if ((supportedCapabilities & capabilities) != capabilities) {
                prvMpdSnprintf(error_msg, sizeof(error_msg), "rel_ifIndex %d - some desired capabilities [%d] are not supported [%d]\n", portEntry_PTR->rel_ifIndex, capabilities, supportedCapabilities);
                PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                           MPD_ERROR_SEVERITY_MINOR_E,
                                           error_msg);
                return MPD_OP_FAILED_E;
            }
        }

        /* read master-slave preference status */
        mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                       7,
                       32,
                       &master_stat);
        /* check if need to set master/slave preferred */
        if (((master_stat & PRV_MPD_master_manual_mask_CNS) != 0) ||
            ((master_stat & PRV_MPD_master_preferred_mask_CNS) == 0 &&
             params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ||
            ((master_stat & PRV_MPD_master_preferred_mask_CNS) != 0 &&
             params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_SLAVE_E)) {
            set_master = TRUE;
            func_PTR = (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ?
                       mtdAutonegSetMasterPreference : mtdAutonegSetSlavePreference;
        }
    }
    else {
        /* possible for 10M or 100M forced speed only - the configuration will be done in prvMpdMtdSetSpeed_5 */
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d autoNeg disabled - configuration is done in set speed operation",
                                                     rel_ifIndex);
        return MPD_OK_E;
    }

    /* check the speeds that currently been advertised */
    mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &val);

    /* if port is UP and we are in copper mode - restart auto-neg */
    restartAn = ((portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) && (!portEntry_PTR->runningData_PTR->sfpPresent)) ? TRUE : FALSE;

    if (set_master) {
        restartAnSlaveMaster = (restartAn && (val == capabilities)) ? TRUE : FALSE;
        /* call to master/salve preferred function + restart AN */
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "rel_ifIndex %d master-salve preferred (%s) function restart AN %d",
                                                     rel_ifIndex,
                                                     (params_PTR->phyAutoneg.masterSlave == MPD_AUTONEGPREFERENCE_MASTER_E) ? "Master" : "Slave",
                                                     restartAnSlaveMaster);
        PRV_MPD_MTD_CALL((func_PTR)(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), restartAnSlaveMaster),
                         rel_ifIndex,
                         mtdStatus);
    }
    /* check if configuration needed - capabilities has been changed or moving from forced mode */
    if (val != capabilities){
        configNeeded = TRUE;
    }
    else {
        mtdHwGetPhyRegField(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                            7,
                            0,
                            12,
                            1,
                            &autoNegEnabled);
        if (autoNegEnabled == 0){
             configNeeded = TRUE;
        }
    }

    if (configNeeded) {
        prvMpdMtdGetHighestSpeed(capabilities, &speed);
        prvMpdMtdUpdateCUnitMediaAndSpeed(portEntry_PTR, speed);
        if (!portEntry_PTR->runningData_PTR->sfpPresent){
            PRV_MPD_MTD_CALL(mtdEnableSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), capabilities, restartAn),
                            rel_ifIndex,
                            mtdStatus);
        }
        if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
            prvMpdCapabilitiesString[0] = '\0';
            prvMpdDebugConvertMtdCapabilitiesToString(capabilities, prvMpdCapabilitiesString);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d MPD 0x%x MTD 0x%x capabilities %s. previous MTD speeds 0x%x. restart AN %d",
                                                         rel_ifIndex,
                                                         params_PTR->phyAutoneg.capabilities,
                                                         capabilities,
                                                         prvMpdCapabilitiesString,
                                                         val,
                                                         restartAn);
        }

    }
    else {
        if (PRV_MPD_DEBUG_IS_ACTIVE_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())) {
            prvMpdCapabilitiesString[0] = '\0';
            prvMpdDebugConvertMtdCapabilitiesToString(capabilities, prvMpdCapabilitiesString);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "rel_ifIndex %d capabilities %s. current equal desired, skip config",
                                                         rel_ifIndex,
                                                         prvMpdCapabilitiesString);
        }
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdUtilConvertSpeedTypes
 *
 * DESCRIPTION: Convert speed types from MTD format to MPD format
 *
 *****************************************************************************/
static void prvMpdMtdUtilConvertSpeedTypes(
    /*     INPUTS:             */
    UINT_16 speeds,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    MPD_AUTONEG_CAPABILITIES_TYP *capabilities_PTR
)
{
    (*capabilities_PTR) = 0;
    if (speeds & MTD_SPEED_10M_HD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_TENHALF_CNS;
    }
    if (speeds & MTD_SPEED_10M_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_TENFULL_CNS;
    }
    if (speeds & MTD_SPEED_100M_HD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
    }
    if (speeds & MTD_SPEED_100M_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS;
    }
    if (speeds & MTD_SPEED_1GIG_HD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS;
    }
    if (speeds & MTD_SPEED_1GIG_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS;
    }
    if (speeds & MTD_SPEED_10GIG_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS;
    }
    if (speeds & MTD_SPEED_2P5GIG_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS;
    }
    if (speeds & MTD_SPEED_5GIG_FD) {
        (*capabilities_PTR) |= MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS;
    }
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetAutonegSupport
 *
 * DESCRIPTION: Get supported auto-negotiation capabilities
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetAutonegSupport(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16     speedsVal;
    UINT_32     rel_ifIndex;
    MTD_STATUS  mtdStatus;
    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    PRV_MPD_MTD_CALL(mtdGetSpeedsAvailable(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speedsVal),
                     rel_ifIndex,
                     mtdStatus);

    prvMpdMtdUtilConvertSpeedTypes(speedsVal, &params_PTR->PhyAutonegCapabilities.capabilities);
    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetAutonegAdmin_1
 *
 * DESCRIPTION: Get admin (configured) auto-negotiation
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdGetAutonegAdmin_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_32     rel_ifIndex;
    UINT_16     masterPrefer, autoNegReg, speedsVal;
    MTD_STATUS  mtdStatus;

    params_PTR->phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS;
    params_PTR->phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;
    params_PTR->phyAutoneg.enable       = MPD_AUTO_NEGOTIATION_DISABLE_E;

    if (((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) &&
         (portEntry_PTR->runningData_PTR->sfpPresent == TRUE)) ||
        (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E)) {
        return MPD_NOT_SUPPORTED_E;
    }

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    /* check if auto neg enabled - dev 7, reg 0, bit 12 */
    mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               7,
                               0,
                               &autoNegReg);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);
    if ((autoNegReg & 0x1000) == 0) {
        return MPD_OK_E;
    }

    mtdStatus = mtdHwXmdioRead(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                               7,
                               32,
                               &masterPrefer);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(mtdStatus, portEntry_PTR->rel_ifIndex);

    masterPrefer &= PRV_MPD_MTD_MASTER_PREFERENCE_MASK_CNS;
    if (masterPrefer == PRV_MPD_MTD_MASTER_PREFERENCE_MASTER_CNS) {
        params_PTR->phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_MASTER_E;
    }
    if (masterPrefer == PRV_MPD_MTD_MASTER_PREFERENCE_SLAVE_CNS) {
        params_PTR->phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_SLAVE_E;
    }

    mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &speedsVal);

    prvMpdMtdUtilConvertSpeedTypes(speedsVal, &params_PTR->phyAutoneg.capabilities);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetPresentNotification
 *
 * DESCRIPTION: Set to copper media type for combo PHY
 *
 * APPLICABLE PHY:
 *              MPD_TYPE_88X32x0_E
 *              MPD_TYPE_88X33x0_E
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetPresentNotification(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    portEntry_PTR->runningData_PTR->sfpPresent = params_PTR->phySfpPresentNotification.isSfpPresent;
    if (params_PTR->phySfpPresentNotification.isSfpPresent) {
        portEntry_PTR->runningData_PTR->opMode = params_PTR->phySfpPresentNotification.opMode;
    }
    else {
        portEntry_PTR->runningData_PTR->opMode = MPD_OP_MODE_UNKNOWN_E;
    }

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d opMode %d isPresent %d speed %d",
                                                 portEntry_PTR->rel_ifIndex,
                                                 portEntry_PTR->runningData_PTR->opMode,
                                                 portEntry_PTR->runningData_PTR->sfpPresent,
                                                 portEntry_PTR->runningData_PTR->speed);

    prvMpdMtdUpdateCUnitMediaAndSpeed(portEntry_PTR, portEntry_PTR->runningData_PTR->speed);

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdSetCheckLinkUp
 *
 * DESCRIPTION:     MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSetCheckLinkUp(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 local, ieeeSpeed, nbSpeed, speed_res, lpAnd_local;
    UINT_32 retries = 0;
    UINT_32 rel_ifIndex;
    MTD_STATUS rc;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;
    params_PTR->internal.phyLinkStatus.phyValidStatus = TRUE;
#ifdef PHY_SIMULATION
	return MPD_OK_E;
#endif
    /* check the speeds that currently been advertised */
    rc = mtdGetSpeedsEnabled(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                             &local);
    if (rc != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "mtdGetSpeedsEnabled on rel_ifIndex %d failed",
                                                     rel_ifIndex);
        return MPD_OK_E;
    }
    rc = mtdGetLPAdvertisedSpeeds(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                  &ieeeSpeed,
                                  &nbSpeed);
    if (rc != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "mtdGetLPAdvertisedSpeeds on rel_ifIndex %d failed",
                                                     rel_ifIndex);
        return MPD_OK_E;
    }
    rc = MTD_FAIL;
    while (rc != MTD_OK && retries < 20) {
        rc = mtdGetAutonegSpeedDuplexResolution(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                                                &speed_res);
        retries++;
        if (rc != MTD_OK) {
            PRV_MPD_SLEEP_MAC(1);
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                         "mtdGetAutonegSpeedDuplexResolution on rel_ifIndex %d failed retry %d",
                                                         rel_ifIndex,
                                                         retries);
        }
    }

    if (rc != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "mtdGetLPAdvertisedSpeeds on rel_ifIndex %d failed",
                                                     rel_ifIndex);
        return MPD_OK_E;
    }

    lpAnd_local = local & ieeeSpeed;

    if (((lpAnd_local & ~speed_res) > speed_res) && speed_res != MTD_SPEED_10GIG_FD) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                     "%d Resolved to NOT highest possible speed. lpAnd_local[%#x] resolved[%#x]",
                                                     rel_ifIndex,
                                                     lpAnd_local,
                                                     speed_res);

        params_PTR->internal.phyLinkStatus.phyValidStatus = FALSE;
        mtdAutonegRestart(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex));
    }
    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                 "rel_ifIndex %d local[%#x] remote_ieeeSpeed[%#x] lpAnd_local[%#x] resolved[%#x]",
                                                 rel_ifIndex,
                                                 local,
                                                 ieeeSpeed,
                                                 lpAnd_local,
                                                 speed_res);

    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdGetTemperature
 *
 * DESCRIPTION: Get temperature from PHY temperature sensoe
 *
 * APPLICABLE PHY:
 *              MPD_TYPE_88X32x0_E
 *              MPD_TYPE_88X33x0_E
 *              MPD_TYPE_88X20x0_E
 *              MPD_TYPE_88X2180_E
 *              MPD_TYPE_88E2540_E
 *              MPD_TYPE_88X3540_E
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdMtdGetTemperature(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    MTD_S16 temperature = 50;
#ifndef PHY_SIMULATION
    UINT_32 rel_ifIndex;
    MTD_STATUS mtdStatus = MTD_OK;

    rel_ifIndex = portEntry_PTR->rel_ifIndex;

    PRV_MPD_MTD_CALL(mtdReadTemperature(PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex), &temperature),
                     rel_ifIndex,
                     mtdStatus);
    if (mtdStatus != MTD_OK) {
        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                    "rel_ifIndex %d: mtdReadTemperature failed",
                                                    rel_ifIndex);
        return MPD_OP_FAILED_E;
    }
    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_INFO_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               "rel_ifIndex %d temperature is 0x%x",
                                               rel_ifIndex,
                                               temperature);
#endif
    MPD_UNUSED_PARAM(portEntry_PTR);

    params_PTR->phyTemperature.temperature = temperature;
    return MPD_OK_E;

}


/*****************************************************************************
* FUNCTION NAME: prvMpdMtdSendIdleToHost
*
* DESCRIPTION: This function send idle from H unit to Host.
*
* APPLICABLE PHY:
*              MPD_TYPE_88X2580_E
*              MPD_TYPE_88X3580_E
*              MPD_TYPE_88E2540_E
*              MPD_TYPE_88X3540_E
*****************************************************************************/
MPD_RESULT_ENT prvMpdMtdSendIdleToHost(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC* portEntry_PTR,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:             */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
)
{
    MTD_STATUS mtdStatus = MTD_OK;
    MTD_BOOL packetGeneratorEnable;

    packetGeneratorEnable = (params_PTR->phySendIdleToHost.sendIdle) ? MTD_TRUE : MTD_FALSE;

    PRV_MPD_MTD_CALL(mtdEnablePktGeneratorChecker(PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex), MTD_H_UNIT , packetGeneratorEnable, MTD_FALSE),
        portEntry_PTR->rel_ifIndex,
        mtdStatus);

    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  "rel_ifIndex %d, %s idle packets",
                                                  portEntry_PTR->rel_ifIndex,
                                                  (params_PTR->phySendIdleToHost.sendIdle) ? "Start" : "Stop");

    return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdMdioReadWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdMdioReadWrap(
    /*     INPUTS:             */
    MTD_DEV *dev_PTR,
    UINT_16 phy_addr,
    UINT_16 dev,
    UINT_16 reg,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *value_PTR
)
{
    PRV_MPD_APP_DATA_STC * appData_PTR;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;


    MPD_UNUSED_PARAM(phy_addr);
    appData_PTR = (PRV_MPD_APP_DATA_STC *) dev_PTR->appData;
    portEntry_PTR = prvMpdFindPortEntry(appData_PTR, phy_addr);
    if (portEntry_PTR == NULL) {
        PRV_MPD_HANDLE_FAILURE_MAC(0,
                                   MPD_ERROR_SEVERITY_ERROR_E,
                                   "couldn't find entry\n");
        return MTD_FAIL;
    }
    return prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  portEntry_PTR,
                                  dev,
                                  reg,
                                  value_PTR);

}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdWaitWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdWaitWrap(
    IN  MTD_DEV_PTR dev,
    IN  MTD_UINT milliSeconds
)
{
    MPD_UNUSED_PARAM(dev);
    MPD_UNUSED_PARAM(milliSeconds);
    PRV_MPD_SLEEP_MAC(milliSeconds);
    return MTD_OK;
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdMdioWriteWrap
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MTD_STATUS mpdMtdMdioWriteWrap(
    /*     INPUTS:             */
    MTD_DEV *dev_PTR,
    UINT_16 phy_addr,
    UINT_16 dev,
    UINT_16 reg,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_APP_DATA_STC * appData_PTR;
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;

    appData_PTR = (PRV_MPD_APP_DATA_STC *) dev_PTR->appData;
    portEntry_PTR = prvMpdFindPortEntry(appData_PTR, phy_addr);
    if (portEntry_PTR == NULL) {
        PRV_MPD_HANDLE_FAILURE_MAC(0,
                                   MPD_ERROR_SEVERITY_ERROR_E,
                                   "couldn't find entry\n");
        return MTD_FAIL;
    }

    return prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                   portEntry_PTR,
                                   dev,
                                   reg,
                                   PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                   value);

}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdInit
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/

extern MPD_RESULT_ENT mpdMtdInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP *fw_portList_PTR,
    MPD_TYPE_ENT *phyType_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    MPD_TYPE_ENT phyType;
    UINT_32 rel_ifIndex;
    PRV_MPD_MTD_OBJECT_INFO_STC phys_obj_info [MPD_TYPE_NUM_OF_TYPES_E][MPD_MAX_INTERFACE_ID_NUMBER][PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS];
    PRV_MPD_APP_DATA_STC * appData_PTR;
    UINT_8  dev, xsmiInterface;
#ifndef PHY_SIMULATION
    UINT_16 err_code, reg_val;
    UINT_32 try = 0;
    MTD_STATUS mtdStat;
#endif

    prvMpdMtdInitDebug();

    /* fill PHY attributes */
    memset(phys_obj_info,
           0,
           sizeof(phys_obj_info));

    for (rel_ifIndex = 0; prvMpdPortListGetNext(fw_portList_PTR,
                                                &rel_ifIndex) == TRUE;) {
        phyType = phyType_PTR[rel_ifIndex];

        /* todo - call init external driver or re-design */
        if (phyType == MPD_TYPE_INVALID_E) {
            /* check whether this is a user defined */
            if (phyType_PTR[rel_ifIndex] >= MPD_TYPE_FIRST_USER_DEFINED_E &&
                phyType_PTR[rel_ifIndex] <= MPD_TYPE_LAST_USER_DEFINED_E) {
                phyType = phyType_PTR[rel_ifIndex];
            }
        }

        portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
        xsmiInterface = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
        dev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
        if (PRV_MPD_VALID_PHY_TYPE(phyType)) {
            if (phys_obj_info[phyType][xsmiInterface][dev].exist == FALSE) {
                /* handle MTD PHYs family (10G copper PHYs) object initialization */
                phys_obj_info[phyType][xsmiInterface][dev].exist = TRUE;
                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR = (PRV_MPD_MTD_OBJECT_STC*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_MTD_OBJECT_STC));
                if (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(0,
                                               MPD_ERROR_SEVERITY_FATAL_E,
                                               PRV_MPD_DEBUG_FUNC_NAME_MAC());
                }
                memset(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR,
                       0,
                       sizeof(PRV_MPD_MTD_OBJECT_STC));
                phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR = (MPD_PHY_FW_PARAMETERS_STC*) PRV_MPD_ALLOC_MAC(sizeof(MPD_PHY_FW_PARAMETERS_STC));
                if (phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(0,
                                               MPD_ERROR_SEVERITY_FATAL_E,
                                               PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset(phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR,
                       0,
                       sizeof(MPD_PHY_FW_PARAMETERS_STC));
				/* since the allocation is mutual to multiple ports, we need to free it once in destroy allocation part */
                portEntry_PTR->runningData_PTR->traceMtdAllocations = TRUE;

                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR = (MTD_DEV*) PRV_MPD_ALLOC_MAC(sizeof(MTD_DEV));
                if (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(0,
                                               MPD_ERROR_SEVERITY_FATAL_E,
                                               PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR,
                       0,
                       sizeof(MTD_DEV));

                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData = (void*) PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_APP_DATA_STC));
                if (phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData == NULL) {
                    PRV_MPD_HANDLE_FAILURE_MAC(0,
                                               MPD_ERROR_SEVERITY_FATAL_E,
                                               PRV_MPD_DEBUG_FUNC_NAME_MAC());
                    return MPD_OP_FAILED_E;
                }
                memset(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData,
                       0,
                       sizeof(PRV_MPD_APP_DATA_STC));

                appData_PTR = (PRV_MPD_APP_DATA_STC *) phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR->appData;
                appData_PTR->mdioDev = portEntry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
                appData_PTR->mdioBus = portEntry_PTR->initData_PTR->mdioInfo.mdioBus;
                appData_PTR->phyType = phyType;

	#ifndef PHY_SIMULATION
                if (PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[phyType] == MPD_FW_DOWNLOAD_TYPE_FLASH_E) {
					/* We can't call mtdLoadDriver until the firmware is loaded from FLASH and running.
					   In General, we should always wait until 1.0.15 is 0 before you do anything with the PHY,
					   This is the signal that the phy is out of reset and the MDIO registers can be accessed. */
					do {
						try++;
						prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, MTD_TUNIT_IEEE_PMA_CTRL1, &reg_val);
						PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																	  "rel_ifIndex %d. reading bit 1.0.15, try number: %d.\n",
																	  portEntry_PTR->rel_ifIndex, try);
						PRV_MPD_SLEEP_MAC(40);
					} while ((reg_val & 0X8000) && (try < PRV_MPD_PHY_READY_MAX_NUM_RETRIES_CNS));

					if (try >= PRV_MPD_PHY_READY_MAX_NUM_RETRIES_CNS){
						PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
												   MPD_ERROR_SEVERITY_FATAL_E,
												   "Phy is not out of reset, MDIO registers can't be accessed!");
						return MPD_OP_FAILED_E;
					}
                }


                try = 1;
                while (try < PRV_MPD_PHY_READY_MAX_NUM_RETRIES_CNS) {
                    mtdStat = mtdLoadDriver(mpdMtdMdioReadWrap,
                                            mpdMtdMdioWriteWrap,
                                            mpdMtdWaitWrap,
                                            (UINT_16) portEntry_PTR->initData_PTR->mdioInfo.mdioAddress,
                                            FALSE,
                                            phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->mtdObject_PTR,
                                            &err_code);
                    if (mtdStat == MTD_OK) {
                        PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                              "rel_ifIndex %d. mtdLoadDriver succeed try: %d",
                                              portEntry_PTR->rel_ifIndex, try);
                        break;
                    }
                    PRV_MPD_SLEEP_MAC(1);
                    try++;
                }
                if (try >= PRV_MPD_PHY_READY_MAX_NUM_RETRIES_CNS){
                    PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          "rel_ifIndex %d. mtdLoadDriver failed - err_code %d.\n",
                                          portEntry_PTR->rel_ifIndex, err_code);
                    PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
                                               MPD_ERROR_SEVERITY_FATAL_E,
                                               "Failed to load mtd driver!");
                    return MPD_OP_FAILED_E;
                }
#else
                switch (phyType) {
                    case MPD_TYPE_88X33x0_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3340_A1;
                        break;
                    case MPD_TYPE_88X32x0_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3240_A1;
                        break;
                    case MPD_TYPE_88X20x0_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2040_A1;
                        break;
                    case MPD_TYPE_88X2180_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2180_A0;
                        break;
                    case MPD_TYPE_88E2540_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2540_B0;
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 4;
                        break;
                    case MPD_TYPE_88X3540_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_X3540_B0;
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 4;
                        break;
                    case MPD_TYPE_88E2580_E:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_E2580_A0;
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 8;
                        break;
                    case MPD_TYPE_88X3580_E:
                    	phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_X3580_A0;
                    	phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->numPortsPerDevice = 8;
                    	break;
                    default:
                        phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->deviceId = MTD_REV_3340_A1;
                        break;
                }
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->fmtdReadMdio = mpdMtdMdioReadWrap;
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->fmtdWriteMdio = mpdMtdMdioWriteWrap;
                phys_obj_info[phyType][appData_PTR->mdioBus][dev].obj_PTR->mtdObject_PTR->devEnabled = MTD_TRUE;
#endif
                phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->phyType = phyType;
                /* Depending on the part, there may be one flash per 1 port, 2 ports, or 4 ports.
    			   Only the first port on each device needs to be downloaded.
    			   2580 & 3580 constructed from two devices */
                PRV_MPD_NONSHARED_GLBVAR_MPDMTD_REP_MODULO_ACCESS_MAC()[phyType] = 4;

#ifndef PHY_SIMULATION
                /* get PHY FW files */
                if (PRV_MPD_GET_FW_FILES_MAC(phyType,
                                             &phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main)) {
                    mtdGetTunitFwVersionFromImage(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main.data_PTR,
                                                  phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_main.dataSize,
                                                  &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_major_CNS]),
                                                  &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_minor_CNS]),
                                                  &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_inc_CNS]),
                                                  &(phys_obj_info[phyType][xsmiInterface][dev].obj_PTR->fw_version.ver_bytes[PRV_MPD_fw_version_index_test_CNS]));

                }
#endif
            }

            PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex] = phys_obj_info[phyType][xsmiInterface][dev].obj_PTR;
            portEntry_PTR->runningData_PTR->phyFw_PTR = phys_obj_info[phyType][xsmiInterface][dev].phyFw_PTR;
            prvMpdMtdUpdateParallelList(portEntry_PTR,
                                        PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[phyType],
                                        phys_obj_info[phyType][xsmiInterface][dev].representativePerPhyNumber_ARR);
        }
        else {
            PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex] = NULL;
            portEntry_PTR->runningData_PTR->phyFw_PTR = NULL;
        }
    }

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: mpdMtdGetPortObject
 *
 * DESCRIPTION:	get MTD port object.
 *	this allows application to directly call MTD
 *
 *****************************************************************************/
extern MPD_RESULT_ENT mpdMtdGetPortObject(
	/*     INPUTS:             */
	UINT_32 		rel_ifIndex,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
	OUT void	** port_object_PTR,
	OUT UINT_8   * mdio_address
)
{
	MTD_DEV * obj_PTR;
	PRV_MPD_MTD_ASSERT_REL_IFINDEX(rel_ifIndex);
	obj_PTR = PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR;
	if (obj_PTR == NULL) {
		return MPD_NOT_FOUND_E;
	}
	*(MTD_DEV**)port_object_PTR = obj_PTR;

	*mdio_address =  prvMpdGetPortEntry(rel_ifIndex)->initData_PTR->mdioInfo.mdioAddress;

	return MPD_OK_E;

}


/* ***************************************************************************
 * FUNCTION NAME: mpdMtdDriverDestroy
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT mpdMtdDriverDestroy(
    /*     INPUTS:             */
    void
)
{
    UINT_32 rel_ifIndex;
#ifndef PHY_SIMULATION
    MTD_STATUS mtdStat;
#endif


	for (rel_ifIndex = 0; rel_ifIndex < MPD_MAX_PORT_NUMBER_CNS; rel_ifIndex++) {
		if ((PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex] != NULL) &&
							PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->entries[rel_ifIndex].runningData_PTR->traceMtdAllocations) {
			if (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR != NULL) {
#ifndef PHY_SIMULATION
				mtdStat = mtdUnloadDriver(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR);
				if (mtdStat != MTD_OK) {
					PRV_MPD_HANDLE_FAILURE_MAC(rel_ifIndex,
											   MPD_ERROR_SEVERITY_FATAL_E,
											   "failed to unload mtd driver (mtdUnloadDriver)");
					return MPD_OP_FAILED_E;
				}
#endif
				if (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR->appData != NULL) {
					PRV_MPD_FREE_MAC(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR->appData);
				}
				PRV_MPD_FREE_MAC(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR);
				PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->mtdObject_PTR = NULL;
			}
			if (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR != NULL) {
				PRV_MPD_FREE_MAC(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]->fw_main.data_PTR);
			}
			PRV_MPD_FREE_MAC(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[rel_ifIndex]);
		}
	}
	memset (PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC(), 0, sizeof(PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()));

    return MPD_OK_E;

}

