
/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/* @file mpdExamples.h

*/


/** @page Examples Examples
	@{
		@section MDIX MDIX
			The PHY automatically determines whether or not it needs to cross over between pairs, so that an external crossover cable is not required. <br>
			If the PHY interoperates with a device that cannot automatically correct for crossover, the PHY makes the necessary adjustment prior to
			commencing Auto-Negotiation. <br>
			If the PHY interoperates with a device that implements MDI/MDIX crossover, a random algorithm as described in IEEE 802.3 clause 40.4.4 determines which device<br>
			performs the crossover.<br>
			@subsection set_mdix Set MDIX
				<b>The Auto MDI/MDIX crossover function can be configured using this command.</b> <br>
				@code
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					phy_params.phyMdix.mode = MPD_AUTO_MODE_MEDIA_E;
					rel_ifIndex = 3;
					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_MDIX_E, &phy_params);

					ASSERT(ret_val);
				@endcode

			@subsection get_mdix Get MDIX
				<b>Get MDI/X operational current status (MDI, MDIX).</b>
				@code
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;
					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_MDIX_E, &phy_params);

					ASSERT(ret_val);

						if(phy_params.phyMdix.mode == MPD_MDI_MODE_MEDIA_E)
							printf("In MDI mode ");
						else if(phy_params.phyMdix.mode == MPD_MDIX_MODE_MEDIA_E)
							printf("In MDIX mode ");
				@endcode

		@subsection get_mdix_admin Get MDIX Administrator
			<b>Get MDI/X mode that the Administrator configured (MDI,MDIX,AUTO).</b>
			@code{.c}
				MPD_OPERATIONS_PARAMS_UNT               phy_params;
				MPD_RESULT_ENT                          ret_val;
				memset(& phy_params, 0, sizeof(phy_params));
				rel_ifIndex = 3;

				ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_MDIX_ADMIN_E, &phy_params);

				ASSERT(ret_val);

				switch(phy_params.phyMdix.mode){
					case MPD_MDI_MODE_MEDIA_E:
						printf("mdix Admin mode is: MDI\n");
						break;

					case MPD_MDIX_MODE_MEDIA_E:
						printf("mdix Admin mode is: MDIX\n");
						break;

					case MPD_AUTO_MODE_MEDIA_E:
						printf("mdix Admin mode is: AUTO\n");
						break;

					default:
						printf("error! could not get the Media \n");
				}
			@endcode

		@section AUTONEG Autonegotiation
			Auto-Negotiation is a signaling mechanism by which two connected devices choose common transmission parameters, such as speed, duplex mode, and flow control.
			@subsection set_auto_neg Set AutoNeg
				<b>This example show how to setup auto-negotiation mechanism.<br></b>
				The 'capabilities' parameter determine what speeds the PHY will advertise when this port will start the negotiation.
				@code
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;
					phy_params.phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS;
					phy_params.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_MASTER_E;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &phy_params);
					ASSERT(ret_val);
				@endcode

			@subsection get_auto_neg_remote_cap Get AutoNeg Remote Capabilities
				<b>Get link partner (advertised) auto-negotiation speed capabilities.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT                    phy_params;
					MPD_RESULT_ENT                               ret_val;
					MPD_AUTONEG_CAPABILITIES_TYP                 capabilities;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E, &phy_params);

					ASSERT(ret_val);

					if (phy_params.phyRemoteAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)
						printf("autoneg remote capabilities are default\n");

					else
					{
						if (phy_params.phyRemoteAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS)
							printf("autoneg remote capabilities are unknown\n");

						else
						{
							printf("autoneg remote capabilities are ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
							printf("10MH ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
							printf("10MF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
							printf("100MH ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
							printf("100MF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
							printf("1GH ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
							printf("1GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
							printf("2.5GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
							printf("5GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
							printf("10GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
							printf("25GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
							printf("40GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
							printf("50GF ");
							if (phy_params.phyRemoteAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
							printf("100GF ");
							printf("\n");
						}
					}
				@endcode

			@subsection get_auto_neg_admin Get AutoNeg Administrator
				<b>Get auto-negotiation Administrator speed capabilities configuration.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT               phy_params;
					MPD_RESULT_ENT                          ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_ADMIN_E, &phy_params);

					ASSERT(ret_val);

					if (phy_params.phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS)
					{
						printf("autoneg admin capabilities are default\n");
					}
					else
						if (phy_params.phyAutoneg.capabilities == MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS)
						{
							printf("autoneg admin capabilities are unknown\n");
						}
						else
						{
							printf("autoneg admin capabilities are ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
								printf("10MH ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
								printf("10MF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
								printf("100MH ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
								printf("100MF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
								printf("1GH ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
								printf("1GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
								printf("2.5GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
								printf("5GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
								printf("10GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
								printf("25GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
								printf("40GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
								printf("50GF ");
							if (phy_params.phyAutoneg.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
								printf("100GF ");
							printf("\n");
						}
				@endcode

			@subsection set_restart_auto_neg Restart AutoNeg
				<b>Restart auto-negotiation on copper port.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT               phy_params;
					MPD_RESULT_ENT                          ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_RESTART_AUTONEG_E, &phy_params);

					ASSERT(ret_val);
				@endcode

			@subsection get_auto_neg_support Get AutoNeg Support
			<b>	Get supported speed & duplex modes.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT               phy_params;
					MPD_RESULT_ENT                          ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_AUTONEG_SUPPORT_E, &phy_params);

					ASSERT(ret_val);

					printf("get autoneg support operation on relIfindex %d: PhyAutoneg capabilities capabilities are\n", rel_ifIndex);
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_TENHALF_CNS)
						printf("10MH ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_TENFULL_CNS)
						printf("10MF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS)
						printf("100MH ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS)
						printf("100MF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS)
						printf("1GH ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS)
						printf("1GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS)
						printf("2.5GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS)
						printf("5GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS)
						printf("10GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS)
						printf("25GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS)
						printf("40GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS)
						printf("50GF ");
					if (phy_params.PhyAutonegCapabilities.capabilities & MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS)
						printf("100GF ");

					printf("\n");

				@endcode

		@section DUPLEX_MODE Duplex Mode
			The transmission mode of a communication channel determines the direction of data can be sent.<br>
			The system supports two kinds of modes:<br>
			    <b>Full-Duplex</b>: Simultaneous transmission of data is enabled between connected systems.
				<b>Half-Duplex</b>: These channels enable bidirectional communications,<br>
				                   but systems at either end of the connection must take turns sending over the communication medium.
			@subsection set_duplex_mode Set Duplex Mode
				<b>Set duplex mode (Half/Full).</b>
				@code
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));

					rel_ifIndex = 3;
					phy_params.phyDuplex.mode = MPD_DUPLEX_ADMIN_MODE_FULL_E;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_DUPLEX_MODE_E, &phy_params);

					ASSERT(ret_val);
				@endcode

		@section VCT VCT
			The device Virtual Cable Tester feature uses Time Domain Reflectometry (TDR) to determine the <br>
			quality of the cables, shorts, cable impedance mismatch, bad connectors, termination mismatch, <br>
			and bad magnetics. The device transmits a signal of known amplitude (+1V) down each of the four <br>
			pairs of an attached cable. It will conduct the cable diagnostic test on each pair, testing the <br>
			MDI_0_0P/N, MDI_0_1P/N, MDI_0_2P/N, and MDI_0_3P/N pairs sequentially. The transmitted <br>
			signal will continue down the cable until it reflects off of a cable imperfection. <br>

			<b>VCT do NOT support speed 10M and cables length under 3 meters</b>
			@subsection set_vct_test Set VCT Test
				<b>Execute Basic VCT (TDR) test (get status and cable length).</b>
				@code
					MPD_OPERATIONS_PARAMS_UNT        phy_params;
					MPD_RESULT_ENT                   ret_val;
					MPD_CABLE_LENGTH_ENT             cable_length;
					MPD_VCT_RESULT_ENT               test_result;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_VCT_TEST_E, &phy_params);

					ASSERT(ret_val);

					if (phy_params.phyVct.testResult == MPD_VCT_RESULT_CABLE_OK_E)
						printf("vct test result is ok\n");
					else if (phy_params.phyVct.testResult == MPD_VCT_RESULT_2_PAIR_CABLE_E)
							printf("vct test result is 2 pair cable\n");
					else if (phy_params.phyVct.testResult == MPD_VCT_RESULT_NO_CABLE_E)
						printf("vct test result is no cable\n");
					else if (phy_params.phyVct.testResult == MPD_VCT_RESULT_OPEN_CABLE_E)
						printf("vct test result is open cable\n");
					else if (phy_params.phyVct.testResult == MPD_VCT_RESULT_SHORT_CABLE_E)
						printf("vct test result is short cable\n");
					else if (phy_params.phyVct.testResult == MPD_VCT_RESULT_BAD_CABLE_E)
						printf("vct test result is bad cable\n");
					else
						printf("vct test result is impedance misMmatch\n");

					printf("vct cable length is %d\n", phy_params.phyVct.cableLength);

				@endcode

			@subsection get_ext_vct_params Get EXT VCT Params
				The Advanced VCT has 4 modes of operation. The first <br>
				mode returns the peak with the maximum amplitude that is above a certain threshold. The second <br>
				mode returns the first peak detected that is above a certain threshold. The third mode measures the <br>
				systematic offset at the receiver. The fourth mode measures the amplitude seen at a certain <br>
				specified distance. <br>

				<b>Here we can see how to execute advanced VCT (TDR & DSP) test (get polarity, sqew, impedance per pair).</b>
				@code
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;
					phy_params.phyExtVct.testType = MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EXT_VCT_PARAMS_E, &phy_params);

					ASSERT(ret_val);

					 printf("test result is %d\n", phy_params.phyExtVct.result);
				@endcode

			@subsection get_cable_len Get Cable Length
				<b>This operation uses DSP to measure cable length.</b> Because of its limits, it returns a range of cable lengths instead of exact measurements.<br>
				Moreover, <b>this feature has a inaccuracy of 15 meters</b>,so in the edge of the range it can returns wrong answer. <br>
				For example, it is possible that for a cable that is 55 meters this operation can returns MPD_CABLE_LENGTH_LESS_THAN_50M_E and not MPD_CABLE_LENGTH_50M_80M_E as expected.

				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT             phy_params;
					MPD_RESULT_ENT                        ret_val;
					MPD_CABLE_LENGTH_ENT				  cable_length;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_CABLE_LEN_E, &phy_params);

					ASSERT(ret_val);

					if (phy_params.phyCableLen.cableLength == MPD_CABLE_LENGTH_LESS_THAN_50M_E)
						printf("cable length is less than 50 meter\n");

					else if (phy_params.phyCableLen.cableLength == MPD_CABLE_LENGTH_50M_80M_E)
						printf("cable length is 50-80 meter\n");

					else if (phy_params.phyCableLen.cableLength == MPD_CABLE_LENGTH_80M_110M_E)
						printf("cable length is 80-110 meter\n");

					else if (phy_params.phyCableLen.cableLength == MPD_CABLE_LENGTH_110M_140M_E)
						printf("cable length is 110-140 meter\n");

					else if (phy_params.phyCableLen.cableLength == MPD_CABLE_LENGTH_MORE_THAN_140M_E)
						printf("cable length is more than 140 meter\n");

					else
						printf("cable length is unknown\n");

				@endcode

			@subsection get_cable_len_no_range Get Cable Length No Range.
				<b>Execute VCT (DSP) and get accurate cable length.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					UINT_16                    length;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E, &phy_params);

					ASSERT(ret_val);

					printf("accurate length is %d\n",phy_params.phyCableLenNoRange.cableLen);

				@endcode

			@subsection get_vct_cap Get VCT Capability
				VCT is supported on the following speeds: 100M, 2500M, 1G, 5G, 10G. <br>
				<b>Get VCT speed capability.</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT               phy_params;
					MPD_VCT_CAPABILITY_PARAMS_STC           phy_vct_params;
					MPD_RESULT_ENT                          ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_VCT_CAPABILITY_E, &phy_params);

					ASSERT(ret_val);

				printf("get vct capability operation on relIfindex %d: supported speeds are:\n", rel_ifIndex);

				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10M_CNS){
					printf("10M ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10M_HD_CNS){
					printf("10M_HD ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100M_CNS){
					printf("100M ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100M_HD_CNS){
					printf("100M_HD ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_1G_CNS){
					printf("1G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_10G_CNS){
					printf("10G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_2500M_CNS){
					printf("2500M ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_5G_CNS){
					printf("5G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_12G_CNS){
					printf("12G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_16G_CNS){
					printf("16G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_13600M_CNS){
					printf("13600M ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_20G_CNS){
					printf("20G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_40G_CNS){
					printf("40G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_100G_CNS){
					printf("100G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_25G_CNS){
					printf("25G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_50G_CNS){
					printf("50G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_24G_CNS){
					printf("24G ");
				}
				if (phy_params.phyVctCapab.vctSupportedSpeedsBitmap & MPD_SPEED_CAPABILITY_200G_CNS){
					printf("200G ");
				}
				@endcode

		@section Miscellaneous Miscellaneous
			@subsection set_reset_phy Reset Phy
				<b>Perform (Soft) reset</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_RESET_PHY_E, &phy_params);

					ASSERT(ret_val);
				@endcode

			@subsection set_phy_disable_oper Disable PHY Operation
				<b>Admin disable (shutdown).</b>
				@code{.c}
					MPD_OPERATIONS_PARAMS_UNT  phy_params;
					MPD_RESULT_ENT             ret_val;
					memset(& phy_params, 0, sizeof(phy_params));
					rel_ifIndex = 3;
					phy_params.phyDisable.forceLinkDown = TRUE;

					ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_PHY_DISABLE_OPER_E, &phy_params);

					ASSERT(ret_val);
				@endcode

				@subsection get_phy_kind_media Get PHY Kind And Media
					<b>Get PHY kind and active media (copper/fiber).</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT       phy_params;
						MPD_KIND_AND_MEDIA_PARAMS_STC   phy_kind_and_media;
						MPD_RESULT_ENT					ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;
						phy_params.phyDisable.forceLinkDown = TRUE;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E, &phy_params);

						ASSERT(ret_val);

						printf("get kind and media operation on relIfindex %d:\n phyKind is:", rel_ifIndex);

						if (phy_params.phyKindAndMedia.phyKind == MPD_KIND_COPPER_E){
							printf("COPPER\n");
						}
						else if (phy_params.phyKindAndMedia.phyKind == MPD_KIND_SFP_E){
							printf("SFP\n");
						}
						else if (phy_params.phyKindAndMedia.phyKind == MPD_KIND_COMBO_E){
							printf("COMBO\n");
						}
						else {
							printf("INVALID\n");
						}

						if (phy_params.phyKindAndMedia.isSfpPresent){
							printf("Sfp Present:TRUE\n");
						}
						else {
							printf("Sfp Present:FALSE\n");
						}

						printf("Active media:\n");
						if (phy_params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E){
							printf("COPPER\n");
						}
						if (phy_params.phyKindAndMedia.mediaType == MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E){
							printf("FIBER\n");
						}
					@endcode

				@subsection get_phy_temperatura Get PHY Temperature
					<b>This example shows how to get the temperature (in Celsius degrees). </b> <br>
					The result can be read back on any port as long as the port is not disabled. <br>

					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT               phy_params;
						MPD_RESULT_ENT                          ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_TEMPERATURE_E, &phy_params);

						ASSERT(ret_val);

						printf("temperature is %d\n", phy_params.phyTemperature.temperature);

					@endcode

				@subsection get_phy_revision Get PHY Revision
					<b>Get PHY revision.</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT               phy_params;
						MPD_REVISION_PARAMS_STC					phy_revision;
						MPD_RESULT_ENT                          ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_REVISION_E, &phy_params);

						ASSERT(ret_val);

						switch (phy_params.phyRevision.phyType)
						{
							case MPD_TYPE_88E1543_E:
								printf("relIfindex %d phyType is 1543\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E1545_E:
								printf("relIfindex %d phyType is 1545\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E1548_E:
								printf("relIfindex %d phyType is 1548\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E1680_E:
								printf("relIfindex %d phyType is 1680\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E1680L_E:
								printf("relIfindex %d phyType is 1680L\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E151x_E:
								printf("relIfindex %d phyType is 151x\n", rel_ifIndex);
								break;
							case MPD_TYPE_88E3680_E:
								printf("relIfindex %d phyType is 3680\n", rel_ifIndex);
								break;
							case MPD_TYPE_88X32x0_E:
								printf("relIfindex %d phyType is 32x0\n", rel_ifIndex);
								break;
							case MPD_TYPE_88X33x0_E:
								printf("relIfindex %d phyType is 33x0\n", rel_ifIndex);
								break;
							case MPD_TYPE_88X20x0_E:
								printf("relIfindex %d phyType is 1543\n", rel_ifIndex);
								break;
							case MPD_TYPE_88X2180_E:
								printf("relIfindex %d phyType is 2180\n", rel_ifIndex);
								break;
							default:
								printf("relIfindex %d has no PHY\n", rel_ifIndex);
						}
						printf("revision is %d\n", phy_params.phyRevision.revision);
					@endcode

				@subsection get_internal_oper_status Get PHY Internal Operation Status
					<b>Get PHY status (link & speed).</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT               phy_params;
						MPD_RESULT_ENT                          ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E, &phy_params);

						ASSERT(ret_val);

						if (phy_params.phyInternalOperStatus.isOperStatusUp == FALSE)
							printf("relIfindex %d internal oper status is NOT UP\n", rel_ifIndex);

						else{
							printf("relIfindex %d internal oper status is UP\n", rel_ifIndex);

							if (phy_params.phyInternalOperStatus.phySpeed == MPD_SPEED_LAST_E)
								printf("speed in NA\n");

							else
							{
								switch (phy_params.phyInternalOperStatus.phySpeed)
								{
									case MPD_SPEED_10M_E:
										printf("speed is 10M\n");
										break;
									case MPD_SPEED_100M_E:
										printf("speed is 100M\n");
										break;
									case MPD_SPEED_1000M_E:
										printf("speed is 1000M\n");
										break;
									case MPD_SPEED_10000M_E:
										printf("speed is 10000M\n");
										break;
									case MPD_SPEED_2500M_E:
										printf("speed is 2500M\n");
										break;
									case MPD_SPEED_5000M_E:
										printf("speed is 5000M\n");
										break;
									case MPD_SPEED_20000M_E:
										printf("speed is 20000M\n");
										break;
									default:
										printf("speed support should be added\n");
								}
								if (phy_params.phyInternalOperStatus.duplexMode == TRUE)
									printf("duplex mode is full\n");

								else
									printf("duplex mode is half\n");

							}

							switch (phy_params.phyInternalOperStatus.mediaType){
								case MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E:
									printf("media type is copper\n");
									break;
								case MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E:
									printf("media type is fiber\n");
									break;
								default:
									printf("media type is invalid\n");
							}

							if (phy_params.phyInternalOperStatus.isAnCompleted == TRUE)
								`printf("Negotiation completed!\n");

							else
								printf("Negotiation was NOT completed!\n");
						}
					@endcode

			@section FLOW_CONTROL Flow Control
				Flow-Control is the process of managing the rate of data transmission between two nodes to prevent a fast sender from overwhelming a slow receiver.<br>
				It provides a mechanism for the receiver to control the transmission speed, so that the receiving node is not overwhelmed with data from transmitting node.<br>
				<b>The PHY responsibility is to advertise to the link partner if it support Flow-Control.</b>
				@subsection set_advertise_fc Set Advertise FlowControl
					<b>Advertise flow control capabilities.</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT               phy_params;
						MPD_RESULT_ENT                          ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;
						phy_params.phyFc.advertiseFc = FALSE;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_ADVERTISE_FC_E, &phy_params);

						ASSERT(ret_val);
					@endcode

				@subsection get_link_partner Get Link Partner Pause Capable
					<b>Get link partner (advertised) flow control capabilities.</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT               phy_params;
						MPD_RESULT_ENT                          ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;
						phy_params.phyFc.advertiseFc = FALSE;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E, &phy_params);

						ASSERT(ret_val);

						if (phy_params.phyLinkPartnerFc.pauseCapable == TRUE)
							printf("link partner is pause capable\n");

						else
							printf("link partner is not pause capable\n");

					@endcode

			@section GREEN Green
				Mechanism to save power consumption. Operate two methods: <br>
				<b>Energy-Detect Mode</b> - the PHY has two Energy-Detect modes, our system uses only Energy-Detect+TM(mode2).<br>
				In mode 2, the PHY sends out a single 10 Mbps NLP (Normal Link Pulse) every one second()<br>
				On link down, the port moves into inactive mode, <br>
				saving power while keeping the Administrative status of the port Up. Recovery from this mode to full operational mode is fast. <br>
				<b>Short-Reach Mode</b> - Cable length is analyzed and the power usage is adjusted for various cable lengths.   <br>
 				@subsection set_power_modules Set Power Modules
					<b>In this example can be seen how to enable/disable the PHY to use GREEN methods (Energy Detect & Short Reach).</b>
					@code{.c}
						MPD_OPERATIONS_PARAMS_UNT  phy_params;
						MPD_RESULT_ENT             ret_val;
						memset(& phy_params, 0, sizeof(phy_params));
						rel_ifIndex = 3;
						phy_params.phyPowerModules.energyDetetct   = MPD_GREEN_ENABLE_E;
						phy_params.phyPowerModules.shortReach      = MPD_GREEN_DISABLE_E;
						phy_params.phyPowerModules.performPhyReset = FALSE;

						ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_POWER_MODULES_E, &phy_params);

						ASSERT(ret_val);
					@endcode

				@section SFP SFP
				 Small Form-factor Pluggable (SFP) is a compact, hot-pluggable network interface module.<br>
				 SFP interface on networking hardware is a modular slot for a media-specific transceiver in order to connect a fiber or copper cables.<br>
				 The advantage of using SFPs compared to fixed interfaces (e.g. modular connectors in Ethernet switches)<br>
				 is that individual ports can be equipped with any suitable type of transceiver as needed.<br>
					@subsection set_sfp_present Set SFP Present
						<b>This example shows how application can update the media that connect to the PHY.</b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT  phy_params;
							MPD_RESULT_ENT             ret_val;
							memset(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;
							phy_params.phySfpPresentNotification.isSfpPresent = TRUE;
							phy_params.phySfpPresentNotification.opMode       = MPD_OP_MODE_COPPER_SFP_GBIC_MODE_E;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E, &phy_params);

							ASSERT(ret_val);
						@endcode

				@section MDIO MDIO
					Management Data Input/Output, or MDIO, is a 2-wire serial bus that is used to manage PHYs or physical layer devices in media access controllers (MACs)<br>
					in Gigabit Ethernet equipment. The management of these PHYs is based on the access and modification of their various registers.
					@subsection set_mdio_access Set MDIO Access
						<b>Example of how to perform MDIO read/write operation. </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT  phy_params;
							MPD_RESULT_ENT             ret_val;
							memset(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;
							phy_params.phyMdioAccess.type         = MPD_MDIO_ACCESS_READ_E;
							phy_params.phyMdioAccess.deviceOrPage = 1;
							phy_params.phyMdioAccess.address      = 220;
							phy_params.phyMdioAccess.mask         = 255;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_MDIO_ACCESS_E, &phy_params);

							ASSERT(ret_val);

							printf("Register Value: %d \n", phy_params.phyMdioAccess.data);
						@endcode

				@section EEE EEE
					Energy Efficient Ethernet Low Power Modes
					@subsection set_eee_adv_cap Set EEE Advertised Capabilities
						<b>set EEE speed and enable/disable Advertise. </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT  phy_params;
							MPD_RESULT_ENT            ret_val;
							memset(& phy_params, 0, sizeof(phy_params));

							phy_params.phyEeeAdvertize.speedBitmap = MPD_SPEED_CAPABILITY_10M_CNS;
							phy_params.phyEeeAdvertize.advEnable   = TRUE;
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E, &phy_params);

							ASSERT(ret_val);

						@endcode

					@subsection set_eee_master_enable Set EEE Master Enable
						Master(or Legacy) mode of operation is used in systems where the external MAC is not EEE capable. <br>
						The	MAC and the system are completely transparent to the EEE operation. Enter and exit out of EEE <br>
						operation or Low Power Idle (LPI) mode is completely managed and controlled by the <br>
						PHY alone.<br>
						<b>In the example below it can seen how to enable/disable this mode.  </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT            phy_params;
							MPD_RESULT_ENT                       ret_val;
							MPD_EEE_MASTER_ENABLE_PARAMS_STC     master_enable;
							memset(& phy_params, 0, sizeof(phy_params));
							master_enable.masterEnable = TRUE;
							phy_params.phyEeeMasterEnable.masterEnable = eee_enable.enable;
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E, &phy_params);

							ASSERT(ret_val);

						@endcode

					@subsection get_eee_remote_status Get EEE Remot Status
						<b>Get EEE remote (Advertised) status.</b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT       phy_params;
							BOOLEAN						    eee_rem_status;
							MPD_SPEED_CAPABILITY_TYP		enableBitmap;
							MPD_RESULT_ENT                  ret_val;
							memset(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EEE_CAPABILITY_E, &phy_params);
							ASSERT(ret_val);
							enableBitmap = phy_params.phyEeeCapabilities.enableBitmap;

							memset(& phy_params, 0, sizeof(phy_params));
							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E, &phy_params);
							ASSERT(ret_val);

							if (ret_val == MPD_OK_E)
							{
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_UNKNOWN_CNS)
									eee_rem_status = FALSE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_HD_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_HD_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_1G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_2500M_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_5G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_12G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_16G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_13600M_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_20G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_40G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_25G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_50G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_24G_CNS)
									eee_rem_status = TRUE;
								else if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_200G_CNS)
									eee_rem_status = TRUE;
								else
									eee_rem_status = FALSE;
							}
								if(eee_rem_status)
									printf("link partner uses EEE\n");
								else
									printf("link partner is NOT uses EEE\n");

							}
						@endcode

					@subsection set_LPI_exit_time Set LPI Exit Time
						Once the low power mode is disabled, either because of a wake signal received from the link partner or data received from the MAC,<br>
						the PHY waits for an exit timer. <br>
						to expire before it resumes normal operation. The exit timer tracks the transition from the low power <br>
						idle mode to normal operation. <br>
						This timer effects only when master mode is enable <br>
						<b>Here is an example to how set this timer. </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT          phy_params;
							UINT_16                            lpi_tx_timeOut = {30,17,14,8,13,18};
							MPD_RESULT_ENT                     ret_val;
							UINT_8                             speed_index;
							UINT_32							   arr_len = sizeof(lpi_tx_timeOut) / size(lpi_tx_timeOut[0]);
							memset(& phy_params, 0, sizeof(phy_params));

							for(speed_index = 0; speed_index < arr_len; i++){
								phy_params.phyEeeLpiTime.speed   = speed_index;
								phy_params.phyEeeLpiTime.time_us = lpi_tx_timeOut[speed_index];
							}
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_LPI_EXIT_TIME_E, &phy_params);

							ASSERT(ret_val);

						@endcode

					@subsection set_LPI_enter_time Set LPI Enter Time
						The entry timer tracks the amount of time without a data transaction between the MAC and the PHY.<br>
						Once the entry timer expires the PHY begins to transmit, the low power idles to save power. <br>
						This timer effects only when master mode is enable <br>
						<b>Here is an example to how set this timer. </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT          phy_params;
							UINT_8							   speed_index;
							UINT_16							   lpi_tx_timeOut = {30,17,14,8,13,18};
							UINT_32							   arr_len = sizeof(lpi_tx_timeOut) / size(lpi_tx_timeOut[0]);
							MPD_RESULT_ENT                     ret_val;
							memset(& phy_params, 0, sizeof(phy_params));

							for(speed_index = 0; speed_index < arr_len; i++){
								phy_params.phyEeeLpiTime.speed   = speed_index;
								phy_params.phyEeeLpiTime.time_us = lpi_tx_timeOut[speed_index];
							}
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_LPI_ENTER_TIME_E, &phy_params);

							ASSERT(ret_val);
						@endcode

					@subsection get_EEE_capability Get EEE Capability
						<b>Get (local) EEE capability.</b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT          phy_params;
							BOXG_speed_capability_TYP          port_capab;
							MPD_SPEED_CAPABILITY_TYP		   support_bitmap;
							MPD_RESULT_ENT                     ret_val;
							memset(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_EEE_CAPABILITY_E, &phy_params);

							ASSERT(ret_val);
							if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_UNKNOWN_CNS)
								printf("UNKNOWN");
							else{
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_CNS)
									printf("10M ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10M_HD_CNS)
									printf("10M HD");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_CNS)
									printf("100M ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100M_HD_CNS)
									printf("10M HD");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_1G_CNS)
									printf("1G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_10G_CNS)
									printf("10G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_2500M_CNS)
									printf("2500M");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_5G_CNS)
									printf("5G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_12G_CNS)
									printf("12G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_16G_CNS)
									printf("16G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_13600M_CNS)
									printf("13600M");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_20G_CNS)
									printf("20G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_40G_CNS)
									printf("40G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_100G_CNS)
									printf("100G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_25G_CNS)
									printf("256G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_50G_CNS)
									printf("50G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_24G_CNS)
									printf("24G ");
								if (phy_params.phyEeeCapabilities.enableBitmap & MPD_SPEED_CAPABILITY_200G_CNS)
									printf("200G ");
							}
						@endcode

				@section DTE DTE
					Data Terminal Equipment (DTE) power function. The DTE power function is <br>
					used to detect if a link partner requires power supplied by the device.

					@subsection get_dte_status Get DTE Status
						The device will first monitor for any activity transmitted by the link partner. <br>
						If the link partner is active, then the link partner has power and power from the device is not required. <br>
						If there is no activity coming from the link partner, DTE power engages, and special pulses are sent <br>
						to detect if the link partner requires DTE power. <br>
						The DTE power status immediately comes up as soon as it is detected <br>
						that a device (link partner) requires DTE power.<br>
						Relevant for devices that support DTE power function, the DTE power function is used to detect if a link partner requires power supplied by the POE PSE device. <br>
						<b>The example shows how to detect a link partner that need power supplies . </b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT               phy_params;
							MPD_RESULT_ENT                          ret_val;
							memset(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;

							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_GET_DTE_STATUS_E, &phy_params);

							ASSERT(ret_val);

							if (phy_params.phyDteStatus.dteDetetcted == TRUE)
								printf("DTE status is detected\n");

							else
								printf("DTE status is not detected\n");
						@endcode

				@section SERDES SERDES
					@subsection set_serdes_tune Set SERDES Tune
						<b>Set Serdes rx, tx tune params.</b>
						@code{.c}
							MPD_OPERATIONS_PARAMS_UNT               phy_params;
							MPD_RESULT_ENT                          ret_val;
							cpssOsMemSet(& phy_params, 0, sizeof(phy_params));
							rel_ifIndex = 3;

							phy_params.phyTune.tuneParams.txTune.comphy.preTap		= 10;
							phy_params.phyTune.tuneParams.txTune.comphy.mainTap 	= 10;
							phy_params.phyTune.tuneParams.txTune.comphy.postTap		= 10;
							phy_params.phyTune.txConfig								= MPD_SERDES_CONFIG_PERMANENT_E;
							phy_params.phyTune.lanesBmp								= 5;
							phy_params.phyTune.hostOrLineSide						= MPD_PHY_SIDE_HOST_E;
							ret_val = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_SERDES_TUNE_E, &phy_params);

							ASSERT(ret_val);
						@endcode
	@}
			*/

