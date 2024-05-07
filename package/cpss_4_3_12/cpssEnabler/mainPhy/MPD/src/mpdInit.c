/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdInit.c
 *  @brief PHY and driver initialization related routines
 *
 */


#include     <mpdPrefix.h>
#include     <mpd.h>
#include     <mpdPrv.h>
#include     <mpdInit.h>

#undef __FUNCTION__

static const PRV_MPD_REG_WRITE_INFO_STC PRV_MPD_INIT_SEQ_FOR_88E178x_ARR[] = {
    {0xF8, 4,	0xF00,				 0xF00},
    {0xF8, 23,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x14},
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x3018},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2197},
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0xCB5A},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2093},
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x7088},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2171},
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x311A},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x20C9},
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0xD1BC},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2110},
    /* PCS Reset */
    {0x1E, 26,	0x8000,			         0x8000},
    /* back to page 0 */
    {0, 22,	0,0},
    /* USGMII Regulator setting:
       The USGMII interface may intermittently not be able to receive packets or may encounter CRCs.
       To prevent this behavior, it is recommended to increase the USGMII regulator adjustment.
    */
    {0xF0, 8, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0xB653},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2149},
    {0xF0, 7, 	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x0149},
    /* back to page 0 */
    {0, 22,	0,0},
    /* Gain Settings */
    {0xFF, 17,	0xFF,				 0x44},
    {0xFF, 16,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2140},
    {0xFF, 17,	0xFF,				 0x56},
    {0xFF, 16,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2141},
    {0xFF, 17,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x68C8},
    {0xFF, 16,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2142},
    {0xFF, 17,	0xFF, 				 0x30},
    {0xFF, 16,	PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x215C},
    /* DSP registers */
    {0xFF, 17,  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x700},
    {0xFF, 16,  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2170},
    {0xFF, 17,  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x600},
    {0xFF, 16,  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS, 0x2171},
    /* Enable temperature sensor */
    {6,   27,  0xc000,			         0},
    /* back to page 0 */
    {0,   22, 0,	 0}
};

#define PRV_MPD_INIT_SEQ_FOR_88E178x_LENGTH_CNS (sizeof(PRV_MPD_INIT_SEQ_FOR_88E178x_ARR) / sizeof (PRV_MPD_REG_WRITE_INFO_STC))

static const PRV_MPD_REG_WRITE_INFO_STC PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_ARR[] = {
	/* Disable PCH MCH */
	{26, 0,	0x1,							 0x0}
};

#define PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_LENGTH_CNS (sizeof(PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_ARR) / sizeof (PRV_MPD_REG_WRITE_INFO_STC))

/*
 * Perform a configuration sequence
 * This function is suitable for Alaska 1G PHYs
 * It handles page changes while minimizing such changes
 * Function does not "Set page back to 0", it is caller/sequence responsibility
 * */
static MPD_RESULT_ENT prvMpdPerformConfigSequence (
	/*     INPUTS:             */
	const char 					  * calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
	const PRV_MPD_REG_WRITE_INFO_STC	  * conf_ARR,
	UINT_32							confLength
)
{
	UINT_32 						i;
	UINT_16 						current_page = 0xFFFF, read_value, write_value;
	MPD_OPERATIONS_PARAMS_UNT		phy_params;
	const PRV_MPD_REG_WRITE_INFO_STC *	curr_PTR;

	if (confLength == 0) {
		return MPD_OK_E;
	}
	if (conf_ARR == NULL) {
		return MPD_OP_FAILED_E;
	}
	memset(&phy_params,0,sizeof(phy_params));
	read_value = write_value = 0;
	for (i = 0; i < confLength; i++) {
		curr_PTR = &conf_ARR[i];

		if (current_page != curr_PTR->page) {
			current_page = curr_PTR->page;
			phy_params.internal.phyPageSelect.page = current_page;
			phy_params.internal.phyPageSelect.readPrevPage = FALSE;
			PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC( prvMpdPerformPhyOperation(portEntry_PTR,
											   	   	   	   	   	   	   	   PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
																		   &phy_params),
												calling_func_PTR,
												portEntry_PTR->rel_ifIndex);
		}
		if (curr_PTR->mask == 0) {
			/* don't read - mask is 0 so nothing to write (used for page change at the end of sequence) */
			continue;
		}
		write_value = curr_PTR->value;
		if (curr_PTR->mask != PRV_MPD_MDIO_WRITE_ALL_MASK_CNS) {
			PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC( prvMpdMdioReadRegisterNoPage( calling_func_PTR,
																		  portEntry_PTR,
																		  curr_PTR->address,
																		  &read_value),
													calling_func_PTR,
													portEntry_PTR->rel_ifIndex);

			write_value = (~curr_PTR->mask & read_value) | (curr_PTR->mask & curr_PTR->value);
			/* no need to write same value */
			if (write_value == read_value) {
				continue;
			}
		}
		PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC((prvMpdMdioWriteRegisterNoPage(	calling_func_PTR,
																		portEntry_PTR,
																		curr_PTR->address,
																		write_value)),
											calling_func_PTR,
											portEntry_PTR->rel_ifIndex);

	}
    return MPD_OK_E;
}

/* ***************************************************************************
* FUNCTION NAME: prvMpdTurnOnSignalDetect
*
* DESCRIPTION: Turn on signal_detect bit.
*
*
*****************************************************************************/
static MPD_RESULT_ENT prvMpdTurnOnSignalDetect(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR
)
{
    GT_U16          val;
    MPD_RESULT_ENT  status;

    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    0XFA,
                                    0,
                                    &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* turn on signal detect bit - set FA.0:7=1 */
    val |= (1 << 7);
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0XFA,
                                     0,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}
/*$ END OF prvMpdTurnOnSignalDetect */

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetLoopback_2
 *
 * DESCRIPTION: Debug, Set PHY Lookback
 *
 * APPLICABLE PHYS:
 *                  MPD_TYPE_88E3680_E
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdSetLoopback_2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    GT_U16 val, mask, good_vaule;
    BOOLEAN failure = FALSE;
    MPD_RESULT_ENT status;
    if (params_PTR->internal.phyLoopback.enable) {
        /* write 2.21 = 0x1045 default mac interface speed = 100Mbs */
        val = 0x1045;
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         2,
                                         21,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        /* 0.0 = 0xA100 Force 100M Full */
        val = 0xA100;
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         0,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        /* 0.16.10 = '1' - Force link good */
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        16,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        val |= (1 << 10);
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         16,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        /* Check for copper link: 0.17.14:15 = '01', 0.17.13 = '1' 0.17.10 = '1' */
        good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
        mask = good_vaule | (1 << 15);

        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        16,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        if ((val & mask) != good_vaule) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "+++ No Copper link on %d. 0.17 = %#x",
                                                          portEntry_PTR->rel_ifIndex,
                                                          val);
            failure = TRUE;
        }

        /* Check for qsgmii link: 4.17.14:15 = '01', 4.17.13 = '1' 4.17.10 = '1' */
        good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
        mask = good_vaule | (1 << 15);

        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        4,
                                        16,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        if ((val & mask) != good_vaule) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "+++ No QSGMII link on %d. 4.17 = %#x",
                                                          portEntry_PTR->rel_ifIndex,
                                                          val);
            failure = TRUE;
        }

        /* 0.0.14 = '1' Enable Mac Loopback */
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        0,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        val |= (1 << 14);
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         0,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        PRV_MPD_SLEEP_MAC(100);
        /* 0.16.10 = '1' - Disable Force link good */
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        16,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        val &= ~(1 << 10);
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         16,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        /* Check for qsgmii link: 4.17.14:15 = '01', 4.17.13 = '1' 4.17.10 = '1' */
        good_vaule = (1 << 10) | (1 << 13) | (1 << 14);
        mask = good_vaule | (1 << 15);

        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        4,
                                        16,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        if ((val & mask) != good_vaule) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "--- No QSGMII link -- LoopBack %d. 4.17 = %#x",
                                                          portEntry_PTR->rel_ifIndex,
                                                          val);
            failure = TRUE;
        }

        if (failure == FALSE) {
            PRV_MPD_DEBUG_LOG_MAC(PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC())(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                          "Loopback successful in PHY %d",
                                                          portEntry_PTR->rel_ifIndex);
        }
    }
    else {
        /* 0.0.14 = '0' Disable Mac Loopback */
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        0,
                                        &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        val &= ~(1 << 14);
        val |= 0x9340;
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         0,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    }
    return MPD_OK_E;
}

/****************************************************************************
* FUNCTION NAME: prvMpdSetLoopback
*
* DESCRIPTION: Debug, Set PHY Lookback
*
* APPLICABLE PHYS:
*
*
*****************************************************************************/
MPD_RESULT_ENT prvMpdSetLoopback_3(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	MPD_OPERATIONS_PARAMS_UNT *params_PTR
	/*     OUTPUTS:            */
)
{
	UINT_16							 val;
	MPD_RESULT_ENT					 status;
	MPD_LOOPBACK_TYPE_ENT			 loopback_type;
	MPD_LOOPBACK_MODE_TYP			 loopback_mode;
	MPD_TYPE_ENT				     phy_type;
	PRV_MPD_LOOPBACK_PHY_PROFILE_STC *loopback_profile;
	BOOLEAN							 enable_loopback;
	char  git_version_STR [256] = {0};

	loopback_mode = params_PTR->internal.phyLoopback.loopbackMode;
	/*validate loopback type */
	if(loopback_mode >= MPD_LOOPBACK_MODE_LAST_CNS){
		return MPD_OK_E;
	}

	phy_type = portEntry_PTR->initData_PTR->phyType;
	enable_loopback = params_PTR->internal.phyLoopback.enable;

	if(loopback_mode == MPD_LOOPBACK_MODE_PCS_TO_LINE_CNS){
		if(portEntry_PTR->runningData_PTR->sfpPresent == FALSE){
			loopback_type = MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E;
		}
		else{
			loopback_type = MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E;
		}
	}
	else if(loopback_mode == MPD_LOOPBACK_MODE_PCS_TO_MAC_CNS){
		if(portEntry_PTR->runningData_PTR->sfpPresent == FALSE){
			loopback_type = MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E;
		}
		else{
			loopback_type = MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E;
		}
	}
	else{
		loopback_type = loopback_mode;
	}

	loopback_profile = &(PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC())[phy_type][loopback_type];
	/*operation type not available in that PHY */
	if(loopback_profile->bits_to_set == 0){
        prvMpdSnprintf(git_version_STR, sizeof(git_version_STR),"PHY do not supported this mode \n");
		PRV_MPD_LOGGING_MAC(git_version_STR);
		return MPD_OK_E;
	}

	if (enable_loopback) {
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			portEntry_PTR,
			loopback_profile->page,
			loopback_profile->reg,
			&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

		val |= loopback_profile->bits_to_set;
		status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			portEntry_PTR,
			loopback_profile->page,
			loopback_profile->reg,
			PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
			val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	else {
		/* 0.0.14 = '0' Disable Mac Loopback and phy reset*/
		status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			portEntry_PTR,
			loopback_profile->page,
			loopback_profile->reg,
			&val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
		val &= ~(loopback_profile->bits_to_set);
		status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
			portEntry_PTR,
			loopback_profile->page,
			loopback_profile->reg,
			PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
			val);
		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E3680
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E3680(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 val;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
    MPD_RESULT_ENT  status;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_8);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
						 MPD_OP_CODE_GET_DTE_STATUS_E,
						 prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E3680_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E3680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E3680_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));
        entry_PTR->max = 171;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 47;

        return MPD_OK_E;
    }

    /* Disable Gigabit advertisement */
    prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                           portEntry_PTR,
                           PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
                           9,
                           &val);
    /* Zero 0.9.8:9 */
    val &= ~(1 << 8);
    val &= ~(1 << 9);

    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
                            9,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);
    /* write 0.0 = 0xB100 */
    val = 0xB100;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS,
                            0,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);
    /* Matrix LED Mode */
    if (portEntry_PTR->initData_PTR->ledMode == MPD_LED_MODE_MATRIX_E) {
        prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                4,
                                27,
                                (1 << 5),
                                (1 << 5));
    }

    /* QSGMII TX amplitude change (may require additional fine tuning) */
    /* FD.8 = 0xB4F */
    val = 0x0B53;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFD,
                            8,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);
    /* FD.7 = 0x200D */
    val = 0x200D;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFD,
                            7,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);

    /* EEE Initialization */
    /* FF.17 = 0xB030 */
    val = 0xB030;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFF,
                            17,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);
    /* FF.16 = 0x215C */
    val = 0x215C;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFF,
                            16,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);
    /* FC.24 = 0x888C */
    val = 0x888C;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFC,
                            24,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);

    /* FC.25 = 0x888C */
    val = 0x888C;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0xFC,
                            25,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);

    /* set page 0 (X.22 = 0) */
    prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  portEntry_PTR,
                                  PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                  0);
    /* 0.0 = 0x9140 */
    prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                  portEntry_PTR,
                                  0,
                                  0x9140);

    status = prvMpdTurnOnSignalDetect(portEntry_PTR);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E151x
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E151x(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    MPD_RESULT_ENT rc;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_16);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E151x_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_12 | MPD_BIT_8;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = MPD_BIT_12;
		/*inform loopback type is not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E151x_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E151x_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }
    rc  = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  17,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x214B);
    rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  16,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x2144);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  17,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x0C28);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  16,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x2146);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  17,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0xB233);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  16,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x214D);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  17,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0xCC0C);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FF,
								  16,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x2159);

	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0x00FB,
								  7,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0xC00D);
	/* configure PHY mode to SGMII TO COPPER */
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  18,
								  20,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x8201);
	rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  0,
								  PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS,
								  PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
								  0x9140);
	if (rc == MPD_OK_E) {
        return MPD_OK_E;
    }

    return MPD_OP_FAILED_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdErrata33SlowLinkShortCable
 *
 * DESCRIPTION: Errata for PHYs 1545/1543 - slow link on short cable
 *
 * NOTE:    implementation is according to errata 3.3 from RN of PHYs 1545/1543
 *      from 25.04.2012 - slow link with short cable
 *****************************************************************************/

static MPD_RESULT_ENT prvMpdErrata33SlowLinkShortCable(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{

    MPD_RESULT_ENT status;
    UINT_16 phyId;
    BOOLEAN config = FALSE;
    switch ((MPD_TYPE_ENT) portEntry_PTR->initData_PTR->phyType) {
        case MPD_TYPE_88E1543_E:
        case MPD_TYPE_88E1545_E:
            status = prvMpdMdioReadRegisterNoPage((char*) PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                  portEntry_PTR,
                                                  3,
                                                  &phyId);
            PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

            if (phyId == PRV_MPD_88E1543_REVISION_A0_CNS) {
                config = TRUE;
            }

            break;

        default:
            return MPD_OK_E;
    }

    if (config == TRUE) { /* revision number is A0*/
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         250,
                                         25,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         0);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    }

    return MPD_OK_E;
}

/* ***************************************************************************
* FUNCTION NAME: prvMpdErrata3_1revC2
*
* DESCRIPTION:  Errata for PHYs 1112 - Forced 100 Mbps copper interface while there is Fiber link because
*                                      inserting the copper media might cause flapping on the fiber side.
*
* NOTE:
 *****************************************************************************/

static MPD_RESULT_ENT prvMpdErrata3_1revC2(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT status;
    GT_U16 val, val_reg29, val_reg30;

    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    0,
                                    PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                    &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    if (portEntry_PTR->runningData_PTR->sfpPresent) {
        /* disable copper receiver */
        val_reg29 = 0x001F;
        val_reg30 = 0x8FF0;
        /* disable copper transmitter - 16_0.3 = '1' */
        val |= 0x8;
    }
    else {
        /* enable copper receiver */
        val_reg29 = 0x001F;
        val_reg30 = 0;
        /* enable copper transmitter - 16_0.3 = '0' */
        val &= 0xFFF7;
    }
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0,
                                     PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0,
                                     29,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val_reg29);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0,
                                     30,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val_reg30);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdSetErrata
 *
 * DESCRIPTION: implement phy erratas
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdSetErrata(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    MPD_RESULT_ENT status;

    switch (params_PTR->internal.phyErrata.erratasBitmap) {
        case MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS:
            /* call to WA for errata 3.3 from RN of PHYs 1545/1543 from 25.04.2012 - slow link with short cable */
            status = prvMpdErrata33SlowLinkShortCable(portEntry_PTR);
            PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
            break;
        case MPD_ERRATA_3_1_REV_C2_CNS:
            /* call to WA for errata 3.1 from RN of PHYs 1112 - Forced 100 Mbps copper interface while there is Fiber link */
            status = prvMpdErrata3_1revC2(portEntry_PTR);
            PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
            break;
        default:
            PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                       MPD_ERROR_SEVERITY_FATAL_E,
                                       "unknown errata type received");
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpd154xFiberPowerWA
 *
 * DESCRIPTION: Workaround to avoid jitter issue by disabling the power saving feature
 * Powering down/up fiber interface (Reg 0_1.11 = ‘1’) or changing the operating mode to/from fiber mode powers down
 * certain blocks for the purpose of power saving (the saved power is negligible). This event may introduce jitter on the
 * QSGMII interface and may translate as error on other ports in the same device. In some cases, it may drop the
 * QSGMII link temporarily. This issue is applicable to all modes that use fiber interface (1000BASE-X, 100BASE-FX,
 * and SGMII interface) including auto-media modes. In auto-media application, whenever the copper interface links up,
 * the fiber interface will be powered down automatically and may trigger this issue.
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpd154xFiberPowerWA(
		IN PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR
)
{
	MPD_RESULT_ENT status;

	if (	portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E ||
			portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {

		/* set 16.9 = 0 */
		status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS,
								PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
								0x200,
								0);

		PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	}
	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1543
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1543(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 phyId, val;
    MPD_RESULT_ENT status;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
    PRV_MPD_PORT_HASH_ENTRY_STC innerEntry;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
                         prvMpdSetComboMediaType_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_11);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E,
                         prvMpdEnableFiberPortStatus_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdSfpPresentNotification);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E,
                         prvMpdSetFastLinkDownEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_ERRATA_E,
                         prvMpdSetErrata);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
						 MPD_OP_CODE_GET_DTE_STATUS_E,
						 prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1543_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_12 | MPD_BIT_8;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = MPD_BIT_12;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1543_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_12;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1543_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                           0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    status = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          portEntry_PTR,
                                          3,
                                          &phyId);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* Dual port */
    /* set port 0/2 sgmii -> qsgmii
     set loopback port 0/2 port 1/3 cross  */

    /* get phy port 0/2 smi address */
    if (portEntry_PTR->initData_PTR->mdioInfo.mdioAddress == 0) {
        PRV_MPD_HANDLE_FAILURE_MAC(portEntry_PTR->rel_ifIndex,
                                   MPD_ERROR_SEVERITY_FATAL_E,
                                   "Failed to calculate internal mdio address");
        return MPD_OP_FAILED_E;
    }

    memcpy(&innerEntry, portEntry_PTR,sizeof(innerEntry));
    /* The internal address is calculated by offset -1 of the main phy address */
    innerEntry.initData_PTR->mdioInfo.mdioAddress = portEntry_PTR->initData_PTR->mdioInfo.mdioAddress - 1;

    status = prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                 &innerEntry,
                                 PRV_MPD_IGNORE_PAGE_CNS,
                                 PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                 0x12);
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  0x14,
                                  0x8205);
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                  1);
    /* 1000base-x interface must be powered up */
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  0,
                                  0x9140);
    /* qsgmii interface must be powered up manually */
    /* page 4, register 26, unset bit 11 */
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                  4);
    status |= prvMpdMdioWrapRead(portEntry_PTR->rel_ifIndex,
                                 &innerEntry,
                                 PRV_MPD_IGNORE_PAGE_CNS,
                                 26,
                                 &val);
    val = val & 0xF7FF;
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  26,
                                  val);
    /* set crossover port 0 to 1 and 2 to 3 and set qsgmii loopback enable + bit 9-13 reserved from projects */
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                  4);
    if (phyId == PRV_MPD_88E1543_REVISION_A0_CNS || /* revision number is A0*/
        phyId == PRV_MPD_88E1543_REVISION_A1_CNS) { /* revision number is A1*/
        status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                      &innerEntry,
                                      PRV_MPD_IGNORE_PAGE_CNS,
                                      0x1B,
                                      0x7D03);
    }
    else {
        status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
                                      &innerEntry,
                                      PRV_MPD_IGNORE_PAGE_CNS,
                                      0x1B,
                                      0x7C03);
    }
    /* reset qsgmii */
    status |= prvMpdMdioWrapWrite(portEntry_PTR->rel_ifIndex,
    							  &innerEntry,
                                  PRV_MPD_IGNORE_PAGE_CNS,
                                  0,
                                  0x9140);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* set port 1/3 qsgmii to auto detect */
    if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
        /* set port 1/3 qsgmii to auto detect */
        /* table 204, page 18, register 20 bits 0:2 - value 111 binary + bit 6 for auto-media detect copper/100BASE-FX+ bit 15 for reset + bit 9 reserved - from projects */
        status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               portEntry_PTR,
                                               PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                               0x12);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0x14,
                                                0x8267);
        /* reset qsgmii */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                4);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
        /* reset fiber */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                1);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
        /* reset copper */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
    }
    /* copper port
     sgmii to copper */
    else if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COPPER_E) {
        /* set system mode sgmii interface mode sgmii */
        /* table 204, page 18, register 20 bits 0:2 - value 001 binary + bit 15 for reset + bit 9 reserved - from projects */
        /* status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS, 0x12);*/
        /* status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 0x14, 0x8201);*/
        /* reset copper */
        /* status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS, 0);*/
        /* status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(), portEntry_PTR, 0, 0x9140);*/

        /* set port 1/3 qsgmii to auto detect */
        /* table 204, page 18, register 20 bits 0:2 - value 111 binary + bit 15 for reset + bit 9 reserved - from projects */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                0x12);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0x14,
                                                0x8200);
        /* reset qsgmii */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                4);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
        /* reset copper */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
    }
    /* sfp */
    else {
        /* set port 1/3 qsgmii to auto detect */
        /* table 204, page 18, register 20 bits 0:2 - value 111 binary + bit 15 for reset + bit 9 reserved - from projects */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                0x12);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0x14,
                                                0x8207);
        /* reset qsgmii */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                4);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
        /* reset fiber */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                1);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
        /* reset copper */
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS,
                                                0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                0,
                                                0x9140);
    }

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    if (phyId == PRV_MPD_88E1543_REVISION_A0_CNS || /* revision number is A0*/
        phyId == PRV_MPD_88E1543_REVISION_A1_CNS) { /* revision number is A1*/
        status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               portEntry_PTR,
                                               PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                               0x00FF);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2148);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2144);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0C28);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2146);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xB233);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214D);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xDC0C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2159);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                7,
                                                0xC00D);
    }
    else {
        /* PHY errata 3.1 form 1543 release notes */
        status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               portEntry_PTR,
                                               PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                               0x0002);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x6008);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x0012);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x111E);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                2,
                                                0x111E);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FA);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                8,
                                                0x0010);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x4099);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                3,
                                                0x1120);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                11,
                                                0x113C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                14,
                                                0x8100);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                15,
                                                0x112A);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FC);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x20B0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FF);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2000);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x4444);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2140);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8064);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2141);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0108);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2144);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0F16);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2146);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8C44);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214B);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0F90);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xBA33);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214D);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x39AA);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214F);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8433);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2151);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2010);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2152);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x99EB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2153);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2F3B);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2154);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x584E);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2156);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x1223);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2158);
    }

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* initialize preferred media to prefer fiber */
    val = 0;
    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
                                    PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
                                    &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* Zero mode field */
    val &= ~PRV_MPD_MODE_MASK_CNS;

    if ((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) ||
        (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E)) {
        val &= ~(1 << 4);
        val |= (1 << 5);
    }
    else {
        val |= (1 << 4);
        val &= ~(1 << 5);
    }
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
                                     PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    status = prvMpdTurnOnSignalDetect(portEntry_PTR);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* change IPG to 0xC instead of default 6 */
    if (prvMpdEeeIpg_1(portEntry_PTR) != MPD_OK_E) {
        return MPD_OP_FAILED_E;
    }

    status = prvMpd154xFiberPowerWA(portEntry_PTR);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1545
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1545(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 val, phyId;
    MPD_RESULT_ENT status = MPD_OK_E;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
                         prvMpdSetComboMediaType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_11);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E,
                         prvMpdSetFastLinkDownEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_ERRATA_E,
                         prvMpdSetErrata);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
						 MPD_OP_CODE_GET_DTE_STATUS_E,
						 prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1545_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_12 | MPD_BIT_8;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = MPD_BIT_12;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1545_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_12;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1545_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    status = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                          portEntry_PTR,
                                          3,
                                          &phyId);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* set full duplex bit in PHY */
    prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                           portEntry_PTR,
                           0,
                           0,
                           &val);
    val |= PRV_MPD_CTRLREG_FULL_DPLX_CNS;
    prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                            portEntry_PTR,
                            0,
                            0,
                            PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                            val);

    if (phyId == PRV_MPD_88E1543_REVISION_A0_CNS) { /* revision number is A0*/
        status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               portEntry_PTR,
                                               PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                               0x00FF);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2148);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2144);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0C28);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2146);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xB233);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214D);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xDC0C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2159);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                7,
                                                0xC00D);
    }
    if (phyId == PRV_MPD_88E1543_REVISION_A1_CNS) { /* revision number is A1*/
        /* not PHY initialization is required */
    }
    else { /* Z0 */
        /* PHY errata 3.1 form 1545 release notes */
        status = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                               portEntry_PTR,
                                               PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                               0x0002);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x6008);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x0012);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x111E);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                2,
                                                0x111E);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FA);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                8,
                                                0x0010);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x4099);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                3,
                                                0x1120);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                11,
                                                0x113C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                14,
                                                0x8100);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                15,
                                                0x112A);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FC);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                1,
                                                0x20B0);

        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                                0x00FF);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2000);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x4444);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2140);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8064);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2141);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0108);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2144);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0F16);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2146);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8C44);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214B);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x0F90);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214C);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0xBA33);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214D);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x39AA);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x214F);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x8433);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2151);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2010);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2152);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x99EB);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2153);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x2F3B);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2154);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x584E);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2156);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                17,
                                                0x1223);
        status |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                portEntry_PTR,
                                                16,
                                                0x2158);
    }

    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* initialize preferred media to prefer fiber */
    val = 0;
    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
                                    PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
                                    &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* Zero mode field */
    val &= ~PRV_MPD_MODE_MASK_CNS;

    if ((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) ||
        (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E)) {
        val &= ~(1 << 4);
        val |= (1 << 5);
    }
    else {
        val |= (1 << 4);
        val &= ~(1 << 5);
    }
    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS,
                                     PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    /* change IPG to 0xC instead of default 6 */
    status |= prvMpdEeeIpg_1(portEntry_PTR);

    /* configure general control register - fix Bugs00149620 */
    status |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                      portEntry_PTR,
                                      PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                      0,
                                      16,
                                      0x3060);

	status = prvMpd154xFiberPowerWA(portEntry_PTR);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1548
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1548(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_9);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdSfpPresentNotification_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
						 MPD_OP_CODE_GET_DTE_STATUS_E,
						 prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1548_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_12 | MPD_BIT_8;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].page = 1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = MPD_BIT_12;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 16;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1548_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_12;
		return MPD_OK_E;
    }

	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								  0xFF);

	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  17,
								  0x2148);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  16,
								  0x2144);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  17,
								  0x0C28);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  16,
								  0x2146);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  17,
								  0xB233);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  16,
								  0x214D);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  17,
								  0xDC0C);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  16,
								  0x2159);

	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								  0xFB);
	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  7,
								  0xC00D);

	prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								  portEntry_PTR,
								  PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								  1);

	status = prvMpd154xFiberPowerWA(portEntry_PTR);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1680
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1680(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    UINT_16 val;
    GT_U16 val_reg3;
    MPD_RESULT_ENT rc;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_14);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdGetInternalOperStatus_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1680_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    rc = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                       portEntry_PTR,
                                       PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                       0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
    rc = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                      portEntry_PTR,
                                      3,
                                      &val_reg3);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    if (val_reg3 == 0x0ED2) {

        /* revision A1 init sequence */
        rc = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           0x00FC);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            24,
                                            0x9555);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            25,
                                            0x9575);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0x00FF);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0xDC0C);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x2159);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0x0088);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x215D);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0x8906);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x215B);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0x2148);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x2144);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0x8034);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x2141);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0x00FB);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            8,
                                            0x3606);
        /* QSGMII TX Amplitude change */
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0x00FD);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            8,
                                            0x0B53); /* default: 0x0B4F */
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            7,
                                            0x200D);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0);

        /* Power UP the PHY */
        rc |= prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0,
                                     16,
                                     &val);
        val &= ~(1 << 2);
        rc |= prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                      portEntry_PTR,
                                      0,
                                      16,
                                      PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                      val);

        rc |= prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     0,
                                     0,
                                     &val);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

        /* add all 1680 ports to port list.
         ** required for QSGMII WA see 88E1680 RN from 6.4.12 - for A1 only */
        prvMpdPortListAdd(portEntry_PTR->rel_ifIndex,
                          &PRV_MPD_SHARED_GLBVAR_88E1680_PSET_ACCESS_MAC());
    }
    else {
        /* revision A2 init sequence */
        /* Matrix LED Mode */
        if (portEntry_PTR->initData_PTR->ledMode == MPD_LED_MODE_MATRIX_E) {
            prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    4,
                                    27,
                                    (1 << 5),
                                    (1 << 5));
        }

        /* QSGMII TX Amplitude change */
        rc = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                           portEntry_PTR,
                                           PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                           0x00FD);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            8,
                                            0x0B53); /* default: 0x0B4F */
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            7,
                                            0x200D);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0);
        /* EEE initialization */
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                            0x00FF);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            17,
                                            0xB030);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            16,
                                            0x215C);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            22,
                                            0x00FC);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            24,
                                            0x888C);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            25,
                                            0x888C);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            22,
                                            0);
        rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                            portEntry_PTR,
                                            0,
                                            0x9940);

        PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    }

    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    rc = prvMpdMdioReadRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                      portEntry_PTR,
                                      16,
                                      &val);
    val &= 0xFFFB;
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        16,
                                        val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    rc = prvMpdTurnOnSignalDetect(portEntry_PTR);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

   return MPD_OK_E;
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1680L
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdInit88E1680L(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT rc;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_14);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E, MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E, prvMpdSetSpecificFeatures_4);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdGetInternalOperStatus_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1680L_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 4;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  = 21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1680L_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1680L_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;
        return MPD_OK_E;
    }

    rc = prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                       portEntry_PTR,
                                       PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                       0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    /* Matrix LED Mode */

    if (portEntry_PTR->initData_PTR->ledMode == MPD_LED_MODE_MATRIX_E) {
        prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                portEntry_PTR,
                                4,
                                27,
                                (1 << 5),
                                (1 << 5));
    }

    /* QSGMII TX Amplitude change */
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                        0x00FD);
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        8,
                                        0x0B53); /* default: 0x0B4F */
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        7,
                                        0x200D);
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                        0);
    /* EEE initialization */
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                        0x00FF);
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        17,
                                        0xB030);
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        16,
                                        0x215C);
    rc |= prvMpdMdioWriteRegisterNoPage(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
                                        0);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    rc = prvMpdTurnOnSignalDetect(portEntry_PTR);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    prvMpd1540BypassSet(portEntry_PTR,
                        TRUE);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1780
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1780(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT rc;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_14);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdGetInternalOperStatus_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdSetSerdesTune);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
						 PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
						 prvMpdMtdSetCheckLinkUp_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1780_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg   =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set  = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg =  21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 28;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_4;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1780_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1780_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    rc = prvMpdPerformConfigSequence( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
    							 	  portEntry_PTR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_ARR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_LENGTH_CNS);

    PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1781
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1781(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_RESULT_ENT rc;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_14);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdGetInternalOperStatus_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdSetSerdesTune);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_CODE_GET_PTP_TSQ_E,
                         prvMpdPtpGetTsq);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_PHY_CODE_SET_PTP_INIT_E,
                         prvMpdPtpInit);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E,
                         prvMpdPtpCaptureFRC);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
                         MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E,
                         prvMpdPtpGetCapturedFRC);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
						 PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
						 prvMpdMtdSetCheckLinkUp_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg   =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set  = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg =  21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 28;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_4;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1781_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    rc = prvMpdPerformConfigSequence( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									  portEntry_PTR,
									  PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_ARR,
									  PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_LENGTH_CNS);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    rc = prvMpdPerformConfigSequence( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									  portEntry_PTR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_ARR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_LENGTH_CNS);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1781_internal
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1781_internal(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
	MPD_UNUSED_PARAM(portEntry_PTR);

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_7);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_6);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_14);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdSetEeeAdvertize);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdSetEeeMasterEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdGetEeeStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdSetLpiExitTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdSetLpiEnterTime_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdGetEeeCapability);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdGetInternalOperStatus_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
                         prvMpdGetVctOffset_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdGetDteStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdGetTemperature_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdSetSerdesTune);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults1);
        /*PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_CODE_GET_PTP_TSQ_E,
                         prvMpdPtpGetTsq);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_PHY_CODE_SET_PTP_INIT_E,
                         prvMpdPtpInit);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E,
                         prvMpdPtpCaptureFRC);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
                         MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E,
                         prvMpdPtpGetCapturedFRC);*/
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
						 PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
						 prvMpdMtdSetCheckLinkUp_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1781_internal_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdSetDteConfig);


		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg   =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set  = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page = 2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg =  21;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].reg  = 28;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = MPD_BIT_4;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1781_internal_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_10M_E] = 142;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 135;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_10M_E] = 171;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 144;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }
/*
    rc = prvMpdPerformConfigSequence( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									  portEntry_PTR,
									  PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_ARR,
									  PRV_MPD_INIT_EXT_SEQ_FOR_88E1781_LENGTH_CNS);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);

    rc = prvMpdPerformConfigSequence( PRV_MPD_DEBUG_FUNC_NAME_MAC(),
									  portEntry_PTR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_ARR,
									  PRV_MPD_INIT_SEQ_FOR_88E178x_LENGTH_CNS);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(rc, portEntry_PTR->rel_ifIndex);
*/
    return MPD_OK_E;
}



/* ***************************************************************************
 * FUNCTION NAME: prvMpdInitdirectAttachedFiber
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInitdirectAttachedFiber(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(portEntry_PTR);
    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_DIRECT_ATTACHED_FIBER_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_9);
        PRV_MPD_BIND_MAC(MPD_TYPE_DIRECT_ATTACHED_FIBER_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindSfp);
        PRV_MPD_BIND_MAC(MPD_TYPE_DIRECT_ATTACHED_FIBER_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdSfpPresentNotification);
        PRV_MPD_BIND_MAC(MPD_TYPE_DIRECT_ATTACHED_FIBER_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_8);
        return MPD_OK_E;
    }

    return MPD_OK_E;
}

#ifdef MTD_PHY_INCLUDE

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X32x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X32x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdMtdSetPresentNotification);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdGetEeeMaxTxVal_3/*TBD-KC*/);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X32x0_E,
                         PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E,
                         prvMpdMtdGetMediaType);

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X32x0_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit33x0And32x0(portEntry_PTR,
                                        NULL);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X20x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X20x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;
    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdMtdSetPresentNotification);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X20x0_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);
        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X20x0_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit20x0(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X33x0
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X33x0(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdMtdSetPresentNotification);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E,
                         prvMpdMtdGetMediaType_1);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X33x0_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);
        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X33x0_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit33x0And32x0(portEntry_PTR,
                                        NULL);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X2180
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X2180(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
						 MPD_OP_CODE_GET_DTE_STATUS_E,
						 prvMpdMtdGetDTEStatus);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X2180_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);
        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X2180_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit2180(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E2540
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E2540(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune_1);
		PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
                         MPD_OP_CODE_SEND_IDLE_TO_HOST_E,
                         prvMpdMtdSendIdleToHost);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2540_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E2540_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit25x0(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X3540
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X3540(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune_1);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3540_E,
                         MPD_OP_CODE_SEND_IDLE_TO_HOST_E,
                         prvMpdMtdSendIdleToHost);
        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X3540_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {

        return prvMpdMtdInit35x0(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;
}



/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E2580
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E2580(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune_1);
		PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
                         MPD_OP_CODE_SEND_IDLE_TO_HOST_E,
                         prvMpdMtdSendIdleToHost);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E2580_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);
        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E2580_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {
        return prvMpdMtdInit25x0(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X3580
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X3580(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdMtdGetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdMtdSetAutoNeg_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdMtdGetAutonegAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdMtdGetAutonegSupport);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdMtdSetDuplexMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdMtdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdMtdSetMdixMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdMtdSetVctTest);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdMtdGetCableLenghNoBreakLink);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdMtdRestartAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMtdSetDisable_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdMtdGetLpAnCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdMtdSetAdvertiseFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdMtdGetLpAdvFc);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdMtdSetPowerModules);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
                         prvMpdGetCableLenNoRange_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMtdPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMtdFwDownload_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMtdPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindCopper);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
                         prvMpdMtdSetEeeAdvertise_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
                         prvMpdMtdSetEeeLegacyEnable_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_EEE_STATUS_E,
                         prvMpdMtdGetEeeStatus_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
                         prvMpdMtdSetEeeLpiExitTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
                         prvMpdMtdSetEeeLpiEnterTimer_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_EEE_CAPABILITY_E,
                         prvMpdMtdGetEeeCapabilities);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
                         prvMpdMtdGetEeeConfig_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMtdGetSpeed_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
                         prvMpdGetEeeGetEnableMode_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                         prvMpdGetVctCapability_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
                         prvMpdMtdSetCheckLinkUp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMtdGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdMtdGetMdixAdminMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMtdSetSerdesTune_1);
		PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
						 PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
						 prvMpdMtdSetNearEndLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_GET_DTE_STATUS_E,
                         prvMpdMtdGetDTEStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
                         MPD_OP_CODE_SEND_IDLE_TO_HOST_E,
                         prvMpdMtdSendIdleToHost);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X3580_E,
						 MPD_OP_CODE_SET_DTE_E,
						 prvMpdMtdSetDTEConfig);

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88X3580_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 3155;
        entry_PTR->up.sr_on[MPD_SPEED_100M_E] = 1500;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_2500M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_5000M_E] = 1880;
        entry_PTR->up.sr_on[MPD_SPEED_10000M_E] = 2850;
        entry_PTR->up.sr_off[MPD_SPEED_100M_E] = 1600;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_2500M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_5000M_E] = 1980;
        entry_PTR->up.sr_off[MPD_SPEED_10000M_E] = 3155;
        entry_PTR->down.ed_off = 1134;
        entry_PTR->down.ed_on = 1000;
    }
    else {

        return prvMpdMtdInit35x0(portEntry_PTR,
                                 NULL);
    }
    return MPD_OK_E;
}


#endif


/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88E1112
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88E1112(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    UINT_16 val;
    MPD_RESULT_ENT status;
    MPD_OPERATIONS_PARAMS_UNT phy_params;
    PRV_MPD_POWER_CONSUMPTION_STC *entry_PTR;

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_MDIX_E,
                         prvMpdGetMdixOperType_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_MDIX_ADMIN_E,
                         prvMpdGetMdixAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_MDIX_E,
                         prvMpdSetMdixAdminMode_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_POWER_MODULES_E,
                         prvMpdSetPowerModules_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
                         prvMpdGetGreenReadiness_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
                         prvMpdGetGreenConsumption);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_VCT_TEST_E,
                         prvMpdSetVctTest_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
                         prvMpdGetExtVctParams_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_CABLE_LEN_E,
                         prvMpdGetCableLen_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_AUTONEG_E,
                         prvMpdSetAutoNegotiation_6);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                         prvMpdGetAutoNegotiationAdmin_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                         prvMpdGetAutoNegotiationSupport_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                         prvMpdRestartAutoNegotiation_1);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_DUPLEX_MODE_E,
                         prvMpdSetDuplexMode_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_SPEED_E,
                         prvMpdSetSpeed_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
                         prvMpdSetComboMediaType_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_RESET_PHY_E,
                         prvMpdResetPhy_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdDisableOperation_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E,
                         prvMpdEnableFiberPortStatus_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                         prvMpdGetAutoNegotiationRemoteCapabilities_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_ADVERTISE_FC_E,
                         prvMpdAdvertiseFc_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                         prvMpdGetLinkPartnerPauseCapable_5);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
                         prvMpdGetPhyKindByType);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
                         prvMpdSfpPresentNotification2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_SET_MDIO_ACCESS_E,
                         prvMpdMdioAccess);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_ERRATA_E,
                         prvMpdSetErrata);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
                         prvMpdSetLoopback_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                         prvMpdSetPhyPageSelect);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                        MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                        prvMpdGetInternalOperStatus_4);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                        MPD_OP_CODE_GET_VCT_CAPABILITY_E,
                        prvMpdGetVctCapability_2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdGetPhyIdentifier);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
                         prvMpdVctEnableTest2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
                         prvMpdVctTdrCollectResults2);
        PRV_MPD_BIND_MAC(MPD_TYPE_88E1112_E,
                         PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,
                         prvMpdVctDspGetResults2);

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].page = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].reg  = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].page =  1;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].reg  =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E].bits_to_set = MPD_BIT_14;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].page = 30;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].reg  = 26;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E].bits_to_set = MPD_BIT_9;

		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].page =  2;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].reg  =  0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E].bits_to_set = MPD_BIT_14;
		/*inform loopback types are not enable in this PHY */
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E].bits_to_set = 0;
		PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()[MPD_TYPE_88E1112_E][MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E].bits_to_set = 0;

        entry_PTR = &(PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()[MPD_TYPE_88E1112_E]);
        memset(entry_PTR,
               0,
               sizeof(PRV_MPD_POWER_CONSUMPTION_STC));

        entry_PTR->max = 402;
        entry_PTR->up.sr_on[MPD_SPEED_1000M_E] = 385;
        entry_PTR->up.sr_off[MPD_SPEED_1000M_E] = 402;
        entry_PTR->down.ed_off = 76;
        entry_PTR->down.ed_on = 37;

        return MPD_OK_E;
    }

    if (portEntry_PTR->initData_PTR->transceiverType != MPD_TRANSCEIVER_SFP_E) {
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        1,
                                        PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                        &val);
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }
        val |= 0x0200; /* set bit 9 - SIGDET polarity */
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         1,
                                         PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         (GT_U16) val);
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }
    }

    /* initialize preferred media to prefer fiber */
    val = 0;
    /* Get value of the general control register */
    status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    2,
                                    PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                    &val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

    if ((portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) ||
        (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E)) {
        val |= (1 << 10);
        val &= ~(1 << 11);
    }
    else {
        val |= (1 << 11);
        val &= ~(1 << 10);
    }

    status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                     portEntry_PTR,
                                     2,
                                     PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                     PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                     val);
    PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);


    if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_COMBO_E) {
        /* start with active mode copper */
        phy_params.internal.phyFiberParams.fiberPresent = FALSE;
        phy_params.internal.phyFiberParams.phySpeed = portEntry_PTR->runningData_PTR->speed;
        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                        PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E,
                                        &phy_params);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
        /* call to WA for errata 3.1 */
        phy_params.internal.phyErrata.erratasBitmap = MPD_ERRATA_3_1_REV_C2_CNS;
        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                           PRV_MPD_OP_CODE_SET_ERRATA_E,
                                           &phy_params);
        PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
    } else {
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                    portEntry_PTR,
                                    2,
                                    PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                    &val);

        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }

        val &= 0xFC7F; /* unset bits 7-9 */
        if (portEntry_PTR->initData_PTR->transceiverType == MPD_TRANSCEIVER_SFP_E) {
            val |= 0x0380;
        }
        else {
            val |= 0x0100;
        }
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         2,
                                         PRV_MPD_SPECIFIC_CONTROL_REG_CNS,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         (GT_U16) val);
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                        portEntry_PTR,
                                        0,
                                        PRV_MPD_CTRLREG_OFFSET_CNS,
                                        &val);
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }

        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                         portEntry_PTR,
                                         0,
                                         PRV_MPD_CTRLREG_OFFSET_CNS,
                                         PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
                                         (GT_U16)(val | PRV_MPD_CTRLREG_RESET_BIT_CNS));
        if (status != MPD_OK_E) {
            return MPD_OP_FAILED_E;
        }
    }

    return MPD_OK_E;

}

#ifdef MYD_PHY_INCLUDE
/* ***************************************************************************
 * FUNCTION NAME: prvMpdInit88X7120
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdInit88X7120(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{

    MPD_UNUSED_PARAM(portEntry_PTR);

    if (params_PTR->internal.phyInit.initializeDb) {
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
                         prvMpdSetSpecificFeatures_3);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
                         prvMpdMydPreFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
                         prvMpdMydFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
                         prvMpdMydPostFwDownload);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_SET_SPEED_EXT_E,
                         prvMpdMydSetPortMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_GET_SPEED_EXT_E,
                         prvMpdMydGetPortMode);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_SET_SERDES_TUNE_E,
                         prvMpdMydSetSerdesTune);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E,
                         prvMpdMydSetSerdesLanePolarity);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_SET_AUTONEG_MULTISPEED_E,
                         prvMpdMydSetAutoNeg);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         PRV_MPD_OP_CODE_GET_LANE_BMP_E,
                         prvMpdMydGetPortLaneBmp);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                         prvMpdMydGetPortLinkStatus);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                         prvMpdMydSetDisable);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_GET_TEMPERATURE_E,
                         prvMpdMydGetTemperature);
        PRV_MPD_BIND_MAC(MPD_TYPE_88X7120_E,
                         MPD_OP_CODE_GET_REVISION_E,
                         prvMpdMtdMydGetPhyIdentifier);
    }
    else {
        return prvMpdMydInit7120(portEntry_PTR,
                                 NULL);
    }

    return MPD_OK_E;
}
#endif
MPD_RESULT_ENT prvMpdMtdMydInitStub(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT *params_PTR
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(params_PTR);

    return MPD_NOT_SUPPORTED_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdBindInitFunc
 *
 * DESCRIPTION: bind init function according to supported driver phy type
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdBindInitFunc(
    /*     INPUTS:             */
    MPD_TYPE_ENT phyType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
#define set_func_name_name_MAC(_func) func_PTR = (MPD_PORT_OPERATIONS_FUN*)_func; name_PTR = PRV_MPD_TO_STRING_MAC(_func);

    char *				name_PTR = NULL;
    MPD_PORT_OPERATIONS_FUN *func_PTR = NULL;
    /* Driver supported types */
    switch (phyType) {
        case MPD_TYPE_DIRECT_ATTACHED_FIBER_E:
        	set_func_name_name_MAC(prvMpdInitdirectAttachedFiber);
            break;
        case MPD_TYPE_88E1543_E:
        	set_func_name_name_MAC(prvMpdInit88E1543);
            break;
        case MPD_TYPE_88E1545_E:
        	set_func_name_name_MAC(prvMpdInit88E1545);
            break;
        case MPD_TYPE_88E1548_E:
        	set_func_name_name_MAC(prvMpdInit88E1548);
            break;
        case MPD_TYPE_88E1680_E:
        	set_func_name_name_MAC(prvMpdInit88E1680);
            break;
        case MPD_TYPE_88E1680L_E:
        	set_func_name_name_MAC(prvMpdInit88E1680L);
            break;
        case MPD_TYPE_88E151x_E:
        	set_func_name_name_MAC(prvMpdInit88E151x);
            break;
        case MPD_TYPE_88E3680_E:
        	set_func_name_name_MAC(prvMpdInit88E3680);
            break;
        case MPD_TYPE_88E1780_E:
        	set_func_name_name_MAC(prvMpdInit88E1780);
            break;
        case MPD_TYPE_88E1781_E:
            set_func_name_name_MAC(prvMpdInit88E1781);
            break;
        case MPD_TYPE_88E1112_E:
        	set_func_name_name_MAC(prvMpdInit88E1112);
            break;
        case MPD_TYPE_88E1781_internal_E:
            set_func_name_name_MAC(prvMpdInit88E1781_internal);
            break;
#ifdef MTD_PHY_INCLUDE
        case MPD_TYPE_88X32x0_E:
        	set_func_name_name_MAC(prvMpdInit88X32x0);
            break;
        case MPD_TYPE_88X33x0_E:
        	set_func_name_name_MAC(prvMpdInit88X33x0);
            break;
        case MPD_TYPE_88X20x0_E:
        	set_func_name_name_MAC(prvMpdInit88X20x0);
            break;
        case MPD_TYPE_88X2180_E:
        	set_func_name_name_MAC(prvMpdInit88X2180);
            break;
        case MPD_TYPE_88E2540_E:
        	set_func_name_name_MAC(prvMpdInit88E2540);
            break;
        case MPD_TYPE_88X3540_E:
        	set_func_name_name_MAC(prvMpdInit88X3540);
            break;
        case MPD_TYPE_88E2580_E:
        	set_func_name_name_MAC(prvMpdInit88E2580);
            break;
        case MPD_TYPE_88X3580_E:
        	set_func_name_name_MAC(prvMpdInit88X3580);
            break;
#else
        case MPD_TYPE_88X32x0_E:
        case MPD_TYPE_88X33x0_E:
        case MPD_TYPE_88X20x0_E:
        case MPD_TYPE_88X2180_E:
        case MPD_TYPE_88E2540_E:
        case MPD_TYPE_88X3540_E:
        case MPD_TYPE_88E2580_E:
        case MPD_TYPE_88X3580_E:
        	set_func_name_name_MAC(prvMpdMtdMydInitStub);
            break;
#endif
#ifdef MYD_PHY_INCLUDE
        case MPD_TYPE_88X7120_E:
        	set_func_name_name_MAC(prvMpdInit88X7120);
            break;
#else
        case MPD_TYPE_88X7120_E:
        	set_func_name_name_MAC(prvMpdMtdMydInitStub);
            break;
#endif
        default:
            break;
    }

    if (func_PTR) {
        PRV_MPD_BIND_MAC(phyType,
                         PRV_MPD_OP_CODE_INIT_E,
                         func_PTR);
        PRV_MPD_NONSHARED_GLBVAR_MPD_FUNC_INFO_ACCESS_MAC()[phyType][PRV_MPD_OP_CODE_INIT_E].funcName_PTR = name_PTR;
    }
    return MPD_OK_E;
}

