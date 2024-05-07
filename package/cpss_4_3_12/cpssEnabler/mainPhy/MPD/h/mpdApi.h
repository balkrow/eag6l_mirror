/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdApi.h
 * @brief Contain types that are required in order to perform PHY operations
 *
 */

#ifndef MPD_API_H_
#define MPD_API_H_

#include <mpdPrefix.h>
#include <mpdTypes.h>

#define MPD_PORT_NUM_TO_GROUP_MAC(__port)                             (__port / 16)
#define MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(__portGroup)             (1<<__portGroup)
#define MPD_MAX_PORT_NUMBER_CNS  (128)

/** @addtogroup PHYs Supported PHY types
 * @{
 * @enum MPD_TYPE_ENT
 * @brief PHY types supported by MPD
 */

typedef enum {
    /** @brief No PHY, fiber */
    MPD_TYPE_DIRECT_ATTACHED_FIBER_E /*= PDL_PHY_TYPE_direct_attached_fiber_E*/,
    /** @brief 10M/100M/1G              2xSGMII     <-->    QSGMII <--> 2xAMD/F/C  */
    MPD_TYPE_88E1543_E               /*= PDL_PHY_TYPE_alaska_88E1543_E*/,
    /** @brief 10M/100M/1G              QSGMII      <-->    4xC */
    MPD_TYPE_88E1545_E               /*= PDL_PHY_TYPE_alaska_88E1545_E*/,
    /** @brief 10M/100M/1G              QSGMII      <-->    4xAMD/F/C */
    MPD_TYPE_88E1548_E               /*= PDL_PHY_TYPE_alaska_88E1548_E*/,
    /** @brief 10M/100M/1G              2xQSGMII    <-->    8xC */
    MPD_TYPE_88E1680_E               /*= PDL_PHY_TYPE_alaska_88E1680_E*/,
    /** @brief 10M/100M/1G              2xQSGMII    <-->    8xC with LinkCrypt */
    MPD_TYPE_88E1680L_E              /*= PDL_PHY_TYPE_alaska_88E1680L_E*/,
    /** @brief 10M/100M/1G              2xSGMII     <-->    2xC                 88E1512 / 88E1514 */
    MPD_TYPE_88E151x_E               /*= PDL_PHY_TYPE_alaska_88E151X_E*/,
    /** @brief 10M/100M                 2xQSGMII    <-->    8xC */
    MPD_TYPE_88E3680_E               /*= PDL_PHY_TYPE_alaska_88E3680_E*/,
    /** @brief 1G/10G                   2/4xXFI     <-->    2/4xAMD/F/C         88X3220 / 88X3240 */
    MPD_TYPE_88X32x0_E               /*= PDL_PHY_TYPE_alaska_88E32x0_E*/,
    /** @brief 10M/100M/1G/2.5G/5G/10G  USXGMII     <-->    1/4xAMD/F/C         88X3310 / 88X3340 */
    MPD_TYPE_88X33x0_E               /*= PDL_PHY_TYPE_alaska_88E33X0_E*/,
    /** @brief 10M/100M/1G/2.5G/5G      USXGMII     <-->    1/4xAMD/F/C         88E2010 / 88E2040 */
    MPD_TYPE_88X20x0_E               /*= PDL_PHY_TYPE_alaska_88E20X0_E*/,
    /** @brief 10M/100M/1G/2.5G/5G      1/2xUSXGMII <-->    1/8xC               88E2110 / 88E2180 */
    MPD_TYPE_88X2180_E               /*= PDL_PHY_TYPE_alaska_88E2180_E*/,
    /** @brief 10M/100M/1G/2.5G/5G      MP/1xUSXGMII <-->   1/4xC               88E2540 */
    MPD_TYPE_88E2540_E               /*= PDL_PHY_TYPE_alaska_88E2540_E*/,
    /** @brief 10M/100M/1G/2.5G/5G/10G      MP/1/2xUSXGMII <--> 1/8xC           88X3540 */
    MPD_TYPE_88X3540_E               /*= PDL_PHY_TYPE_alaska_88X3540_E*/,
    /** @brief 10M/100M/1G              OUSGMII    <-->    8xC with LinkCrypt */
    MPD_TYPE_88E1780_E               /*= PDL_PHY_TYPE_alaska_88E1780_E*/,
    /** @brief 10M/100M/1G/2.5G/5G      MP/1xUSXGMII <-->   1/4xC               88E2580 */
    MPD_TYPE_88E2580_E               /*= PDL_PHY_TYPE_alaska_88E2580_E*/,
    /** @brief 1G/10G/25G/50G/100G/200G/400G  */
    MPD_TYPE_88X3580_E               /*= PDL_PHY_TYPE_alaska_88X3580_E*/,
    /** @brief combo ports  */
    MPD_TYPE_88E1112_E               /*= PDL_PHY_TYPE_alaska_88E1112_E*/,
	/** @brief 10M/100M/1G              OUSGMII    <-->    8xC with LinkCrypt */
	MPD_TYPE_88E1781_E               /*= PDL_PHY_TYPE_alaska_88E1781_E*/,
	MPD_TYPE_88E1781_internal_E               /*= PDL_PHY_TYPE_alaska_88E1781_internal_E */,
    MPD_TYPE_88X7120_E                          /*  88x7120 */,
    /** @brief 10M/100M/1G/2.5G/5G/10G      MP/1xUSXGMII <-->   1/4xC           88X3580 */
    MPD_TYPE_LAST_SUPPERTED_E = MPD_TYPE_88X7120_E,
    /** @brief User defined reserved, allow application to bind its' own PHY types & driver see \a mpdBindExternalPhyCallback */
    MPD_TYPE_FIRST_USER_DEFINED_E,
    MPD_TYPE_LAST_USER_DEFINED_E = MPD_TYPE_FIRST_USER_DEFINED_E +32,
    MPD_TYPE_NUM_OF_TYPES_E,
    MPD_TYPE_INVALID_E
} MPD_TYPE_ENT;
/** @} */


/** @defgroup port_conf Port configuration
 * @{
 *
 *
 * @addtogroup GET_PORT Get port
 * @{MPD GET PORT
 * - Use \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E in order to get admin configuration of auto-negotiation
 * - Use \ref MPD_OP_CODE_GET_MDIX_ADMIN_E in order to retreive MDIX administrative configuration
 * - Use \ref MPD_OP_CODE_GET_MDIX_E in order to retreive operational MDIX status
 * @remark MPD_OP_CODE_GET_MDIX_E is relevant only when PHY link is up
 * - Use \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E in order to retreive PHY operationla status
 *
 * @}
 @addtogroup SET_PORT Set port
 * @{MPD SET PORT
 * - Use \ref MPD_OP_CODE_SET_AUTONEG_E in order to configure auto-negotiation
 * - Use \ref MPD_OP_CODE_SET_DUPLEX_MODE_E in order to set duplex mode
 * - Use \ref MPD_OP_CODE_SET_SPEED_EXT_E in order to update PHY driver with extended speed params
 * - Use \ref MPD_OP_CODE_SET_MDIX_E in order to set MDIX mode
 * - Use \ref MPD_OP_CODE_SET_ADVERTISE_FC_E in order to configure flow control advertisement
 * - Use \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E in order to shutdown
 *

  @} */
 /** @} */





/** @addtogroup MISC Miscelanious
 * @{
 * @enum MPD_MDIO_ACCESS_TYPE_ENT
 * @brief MDIO access type
 */
typedef enum {
    /** @brief MDIO read */
    MPD_MDIO_ACCESS_READ_E = 0,
    /** @brief MDIO write */
    MPD_MDIO_ACCESS_WRITE_E = 1
} MPD_MDIO_ACCESS_TYPE_ENT;
/** @} */


/** @brief MAX INTERFACE ID NUMBER */
/** Bus Number */
#define MPD_MAX_INTERFACE_ID_NUMBER    2


/** @addtogroup VCT
 * @{
 * @typedef MPD_SPEED_CAPABILITY_TYP
 * @brief speed capability bitmap
 * - MPD_SPEED_CAPABILITY_UNKNOWN_CNS                           <B>  0   </B>
 * - MPD_SPEED_CAPABILITY_10M_CNS                               <B> (1<<1) </B>
 * - MPD_SPEED_CAPABILITY_10M_HD_CNS                            <B> (1<<2)  </B>
 * - MPD_SPEED_CAPABILITY_100M_CNS                              <B> (1<<3)  </B>
 * - MPD_SPEED_CAPABILITY_100M_HD_CNS                           <B> (1<<4)  </B>
 * - MPD_SPEED_CAPABILITY_1G_CNS                                <B> (1<<5)  </B>
 * - MPD_SPEED_CAPABILITY_10G_CNS                               <B> (1<<6)  </B>
 * - MPD_SPEED_CAPABILITY_2500M_CNS                             <B> (1<<7)  </B>
 * - MPD_SPEED_CAPABILITY_5G_CNS                                <B> (1<<8)  </B>
 * - MPD_SPEED_CAPABILITY_12G_CNS                               <B> (1<<9)  </B>
 * - MPD_SPEED_CAPABILITY_16G_CNS                               <B> (1<<10) </B>
 * - MPD_SPEED_CAPABILITY_13600M_CNS                            <B> (1<<11) </B>
 * - MPD_SPEED_CAPABILITY_20G_CNS                               <B> (1<<12) </B>
 * - MPD_SPEED_CAPABILITY_40G_CNS                               <B> (1<<13) </B>
 * - MPD_SPEED_CAPABILITY_100G_CNS                              <B> (1<<14) </B>
 * - MPD_SPEED_CAPABILITY_25G_CNS                               <B> (1<<15) </B>
 * - MPD_SPEED_CAPABILITY_50G_CNS                               <B> (1<<16) </B>
 * - MPD_SPEED_CAPABILITY_24G_CNS                               <B> (1<<17) </B>
 */

typedef UINT_32 MPD_SPEED_CAPABILITY_TYP;
/**@brief Unknown */
#define MPD_SPEED_CAPABILITY_UNKNOWN_CNS                             0x0
/**@brief 10M */
#define MPD_SPEED_CAPABILITY_10M_CNS                                 (1<<1)
/**@brief 10M HD */
#define MPD_SPEED_CAPABILITY_10M_HD_CNS                              (1<<2)
/**@brief 100M */
#define MPD_SPEED_CAPABILITY_100M_CNS                                (1<<3)
/**@brief 100M HD*/
#define MPD_SPEED_CAPABILITY_100M_HD_CNS                             (1<<4)
/**@brief 1G */
#define MPD_SPEED_CAPABILITY_1G_CNS                                  (1<<5)
/**@brief 10G */
#define MPD_SPEED_CAPABILITY_10G_CNS                                 (1<<6)
/**@brief 2500M */
#define MPD_SPEED_CAPABILITY_2500M_CNS                               (1<<7)
/**@brief 5G */
#define MPD_SPEED_CAPABILITY_5G_CNS                                  (1<<8)
/**@brief 12G */
#define MPD_SPEED_CAPABILITY_12G_CNS                                 (1<<9)
/**@brief 16G */
#define MPD_SPEED_CAPABILITY_16G_CNS                                 (1<<10)
/**@brief 13600M */
#define MPD_SPEED_CAPABILITY_13600M_CNS                              (1<<11)
/**@brief 20G */
#define MPD_SPEED_CAPABILITY_20G_CNS                                 (1<<12)
/**@brief 40G */
#define MPD_SPEED_CAPABILITY_40G_CNS                                 (1<<13)
/**@brief 100G */
#define MPD_SPEED_CAPABILITY_100G_CNS                                (1<<14)
/**@brief 25G */
#define MPD_SPEED_CAPABILITY_25G_CNS                                 (1<<15)
/**@brief 50G */
#define MPD_SPEED_CAPABILITY_50G_CNS                                 (1<<16)
/**@brief 24G */
#define MPD_SPEED_CAPABILITY_24G_CNS                                 (1<<17)
/**@brief 200G */
#define MPD_SPEED_CAPABILITY_200G_CNS                                (1<<18)
/** @} */

/** @addtogroup Speed
 * @{
 * @enum MPD_SPEED_ENT
 * @brief Speeds
 */
typedef enum {
	/**@brief 10M   */
    MPD_SPEED_10M_E     = 0,
	/**@brief 100M */
    MPD_SPEED_100M_E    = 1,
	/**@brief 1000M  */
    MPD_SPEED_1000M_E   = 2,
	/**@brief 10000M  */
    MPD_SPEED_10000M_E  = 3,
	/**@brief 2500M  */
    MPD_SPEED_2500M_E   = 5,
	/**@brief 5000M  */
    MPD_SPEED_5000M_E   = 6,
	/**@brief 20000M  */
    MPD_SPEED_20000M_E  = 8,
	/**@brief 40G  */
    MPD_SPEED_40G_E     = 9,
	/**@brief 100G  */
    MPD_SPEED_100G_E    = 13,
	/**@brief 50G */
    MPD_SPEED_50G_E = 14,
	/**@brief 25G */
    MPD_SPEED_25G_E = 21,
	/**@brief 200G  */
    MPD_SPEED_200G_E    = 24,
	/**@brief 400G  */
    MPD_SPEED_400G_E    = 25,
	/**@brief Last  */
    MPD_SPEED_LAST_E    = 29
} MPD_SPEED_ENT;
/** @} */
/** @internal
 * @enum MPD_EEE_ENABLE_MODE_TYP
 * @brief EEE Enable mode
 */
typedef enum {
    /** @internal @brief EEE enabled/disabled on link up/down event, and on admin configuration */
    MPD_EEE_ENABLE_MODE_LINK_CHANGE_E   =   0,
    /** @internal @brief EEE enabled/disabled on admin configuration only */
    MPD_EEE_ENABLE_MODE_ADMIN_ONLY_E    =   1
} MPD_EEE_ENABLE_MODE_ENT;
/** @addtogroup EEE_Green EEE & Green
 * @{
 * @enum MPD_EEE_SPEED_ENT
 * @brief EEE Speeds
 */
typedef enum {
    /**@brief 10M */
    MPD_EEE_SPEED_10M_E   = 0,
    /**@brief 100M */
    MPD_EEE_SPEED_100M_E  = 1,
    /**@brief 1G */
    MPD_EEE_SPEED_1G_E    = 2,
    /**@brief 10G */
    MPD_EEE_SPEED_10G_E   = 3,
    /**@brief 2.5G */
    MPD_EEE_SPEED_2500M_E = 4,
    /**@brief 5G */
    MPD_EEE_SPEED_5G_E    = 5,
    MPD_EEE_SPEED_MAX_E   = 6
} MPD_EEE_SPEED_ENT;
/** @} */

/** @addtogroup Autonegotiation
 * @{
 * @typedef MPD_AUTONEG_CAPABILITIES_TYP
 * @brief Auto Negotiation bitmap values
 * - MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS                         \b 0x0080
 * - MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS                         \b 0x0040
 * - MPD_AUTONEG_CAPABILITIES_TENHALF_CNS                         \b 0x0020
 * - MPD_AUTONEG_CAPABILITIES_TENFULL_CNS                         \b 0x0010
 * - MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS                        \b 0x0008
 * - MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS                        \b 0x0004
 * - MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS                        \b 0x0002
 * - MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS                        \b 0x0001
 * - MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS                       \b 0x2000
 * - MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS                         \b 0x4000
 * - MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS                        \b 0x8000
 * - MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS                        \b 0x10000
 * - MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS                        \b 0x20000
 * - MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS                        \b 0x40000
 * - MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS                       \b 0x80000
 */
/* Must be the same values as MIB_swIfSpeedDuplexAutoNegCapab in lib/mib/mib_l2.c */
typedef UINT_32 MPD_AUTONEG_CAPABILITIES_TYP;
 /** @brief Default - Advertise all speeds that the port supports*/
#define MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS                         0x0080
 /** @brief Unknown */
#define MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS                         0x0040
 /** @brief 10/Half */
#define MPD_AUTONEG_CAPABILITIES_TENHALF_CNS                         0x0020
 /** @brief 10/Full */
#define MPD_AUTONEG_CAPABILITIES_TENFULL_CNS                         0x0010
 /** @brief 100/Half */
#define MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS                        0x0008
/** @brief 100/Full */
#define MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS                        0x0004
/** @brief 1000/Half */
#define MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS                        0x0002
/** @brief 1000/Full */
#define MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS                        0x0001
/** @brief 10G/Full */
#define MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS                        0x8000
/** @brief 5000/Full */
#define MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS                         0x4000
/** @brief 2500/Full */
#define MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS                       0x2000
/** @brief 25G/Full */
#define MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS                        0x1000
/** @brief 40G/Full */
#define MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS                        0x0800
/** @brief 50G/Full */
#define MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS                        0x0400
/** @brief 100G/Full */
#define MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS                       0x0200
/** @brief 200G/Full */
#define MPD_AUTONEG_CAPABILITIES_200G_FULL_CNS                       0x0100

/** @} */

/** @addtogroup EEE_Green EEE & Green
 * @{
 * @enum MPD_GREEN_SET_ENT
 * @brief Green setting
 */
typedef enum {
    /** @brief Do nothing */
    MPD_GREEN_NO_SET_E  = 0,
    /** @brief Enable */
    MPD_GREEN_ENABLE_E  = 1,
    /** @brief Disable */
    MPD_GREEN_DISABLE_E = 2
} MPD_GREEN_SET_ENT;
/**
 * @enum MPD_GREEN_READINESS_TYPE_ENT
 * @brief Green readiness type (ED/SR)
 */
typedef enum {
    /**@brief Short Reach */
    MPD_GREEN_READINESS_TYPE_SR_E = 0,
    /**@brief Energy Detect */
    MPD_GREEN_READINESS_TYPE_ED_E = 1
} MPD_GREEN_READINESS_TYPE_ENT;
/**
 * @enum MPD_GREEN_READINESS_ENT
 * @brief Green readiness value
 */
typedef enum {
    /** @brief Normal */
    MPD_GREEN_READINESS_OPRNORMAL_E = 0,
    /** @brief Fiber */
    MPD_GREEN_READINESS_FIBER_E = 1,
    /** @brief Combo fiber */
    MPD_GREEN_READINESS_COMBO_FIBER_E = 2,
    /** @brief Green (type) not supported */
    MPD_GREEN_READINESS_NOT_SUPPORTED_E = 3,
    /** @brief Green (type) is always enabled */
    MPD_GREEN_READINESS_ALWAYS_ENABLED_E = 4
} MPD_GREEN_READINESS_ENT;
/** @} */

/** @addtogroup VCT
 * @{
 * @enum MPD_VCT_RESULT_ENT
 * @brief Result of VCT Test
 */
/* start with 1 to fit SNMP values - don't change */
typedef enum {
    /**@brief Cable O.K */
    MPD_VCT_RESULT_CABLE_OK_E           = 1,
    /**@brief Indicates that a 2 pair cable is used */
    MPD_VCT_RESULT_2_PAIR_CABLE_E       = 2,
    /**@brief No cable is connected */
    MPD_VCT_RESULT_NO_CABLE_E           = 3,
    /**@brief Open ended cable (not terminated) */
    MPD_VCT_RESULT_OPEN_CABLE_E         = 4,
    /**@brief Short Cable (inter Pair short) */
    MPD_VCT_RESULT_SHORT_CABLE_E        = 5,
    /**@brief Bad Cable not matching other criteria */
    MPD_VCT_RESULT_BAD_CABLE_E          = 6,
    /**@brief Impedence Mismatch */
    MPD_VCT_RESULT_IMPEDANCE_MISMATCH_E = 7,
    /** @brief  short between Tx pair and Rx pair 0 */
    MPD_VCT_RESULT_SHORT_WITH_PAIR0_E   = 8,
     /** @brief  short between Tx pair and Rx pair 1 */
    MPD_VCT_RESULT_SHORT_WITH_PAIR1_E   = 9,
     /** @brief  short between Tx pair and Rx pair 2 */
    MPD_VCT_RESULT_SHORT_WITH_PAIR2_E   = 10,
     /** @brief  short between Tx pair and Rx pair 2 */
    MPD_VCT_RESULT_SHORT_WITH_PAIR3_E   = 11
} MPD_VCT_RESULT_ENT;
/**
 * @enum MPD_VCT_TEST_TYPE_ENT
 * @brief VCT test type.<br>
 * 1)Channel test- transmit signal throw the cable and check the return wave in power and time.<br>
 *   This test gives indication about the quality of the channel(short circuit...)
 * 2)polarity: detect if any of the RJ45 pairs connected to the PHY was inverted.<br>
 * 3)skew: detect skew<br>
 */
/* start with 13 to fit SNMP values - don't change */
typedef enum {
	/**@brief Test channel 1 */
    MPD_VCT_TEST_TYPE_CABLECHANNEL1_E   = 13,
	/**@brief Test channel 2 */
    MPD_VCT_TEST_TYPE_CABLECHANNEL2_E   = 14,
	/**@brief Test channel 3 */
    MPD_VCT_TEST_TYPE_CABLECHANNEL3_E   = 15,
	/**@brief Test channel 4 */
    MPD_VCT_TEST_TYPE_CABLECHANNEL4_E   = 16,
	/**@brief Test polarity in pair 1 */
    MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E  = 17,
	/**@brief Test polarity in pair 2 */
    MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E  = 18,
	/**@brief Test polarity in pair 3 */
    MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E  = 19,
	/**@brief Test polarity in pair 4 */
    MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E  = 20,
	/**@brief Test skew in pair 1 */
    MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E  = 21,
	/**@brief Test skew in pair 2 */
    MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E  = 22,
	/**@brief Test skew in pair 3 */
    MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E  = 23,
	/**@brief Test skew in pair 4 */
    MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E  = 24
} MPD_VCT_TEST_TYPE_ENT;
/** @} */
/** @addtogroup Fiber
 * @{
 * @enum MPD_OP_MODE_ENT
 * @brief Fiber port operation mode
 */
typedef enum {
    /** @brief  SFP */
    MPD_OP_MODE_FIBER_E                                     =  0,
    /** @brief  Direct attached/ DAC cable */
    MPD_OP_MODE_DIRECT_ATTACH_E                             =  1,
    /** @brief  Copper SFP 1000Base-T/10GBase-T in GBIC mode (1000BASE-X to 1000BASE-T SFP)*/
    MPD_OP_MODE_COPPER_SFP_1000BASE_X_MODE_E                =  2,
	/** @brief  Copper SFP 1000Base-T/10GBase-T in SGMII mode (SGMII to Copper (3-speed)) */
	MPD_OP_MODE_COPPER_SFP_SGMII_MODE_E                     =  3,
	/** @brief  SFP+ */
	MPD_OP_MODE_FIBER_SFP_PLUSE_E                           =  4,
    /** @brief  In case not preset or failed to identify */
    MPD_OP_MODE_UNKNOWN_E                                   =  5,
    MPD_OP_MODE_LAST_E                                      =  6
} MPD_OP_MODE_ENT;
/**
 * @typedef MPD_TYPE_BITMAP_TYP
 * @brief Port type
 */
typedef UINT_32 MPD_TYPE_BITMAP_TYP;
#define MPD_TYPE_BITMAP_10BASETX_RESERVED_CNS       0x0001  /* not in use */
#define MPD_TYPE_BITMAP_100BASETX_CNS               0x0002
#define MPD_TYPE_BITMAP_1000BASET_CNS               0x0004
#define MPD_TYPE_BITMAP_10GBASET_CNS                0x0008
#define MPD_TYPE_BITMAP_1000BASEKX_CNS              0x0010
#define MPD_TYPE_BITMAP_10GBASEKX4_CNS              0x0020
#define MPD_TYPE_BITMAP_10GBASEKR_CNS               0x0040
#define MPD_TYPE_BITMAP_2500BASET_CNS               0x0080
#define MPD_TYPE_BITMAP_5GBASET_CNS                 0x0100

/** @} */
/** @internal
 * @typedef MPD_ERRATA_TYP
 * @brief Errata
 */
typedef UINT_32 MPD_ERRATA_TYP;
/** @internal @brief Some oarts may have slow link issue with short cable\n
 * write 250.25 = 0 after soft-reset/power up and before auto-negotiation process is complete
 */
#define MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS (1 << 0)
/** Forced 100 Mbps copper interface while there is Fiber link */
#define MPD_ERRATA_3_1_REV_C2_CNS (1 << 1)

/**
 * @enum    MPD_FW_DOWNLOAD_TYPE_ENT
 *
 * @brief   phy download type
 */
typedef enum {
    MPD_FW_DOWNLOAD_TYPE_NONE_E,    /*= PDL_PHY_DOWNLOAD_TYPE_NONE_E,*/              /* no download          */
    MPD_FW_DOWNLOAD_TYPE_RAM_E,     /*= PDL_PHY_DOWNLOAD_TYPE_RAM_E, */              /* ram download         */
    MPD_FW_DOWNLOAD_TYPE_FLASH_E,   /*= PDL_PHY_DOWNLOAD_TYPE_FLASH_E,*/             /* flash download       */
    MPD_FW_DOWNLOAD_TYPE_LAST_E     /*= PDL_PHY_DOWNLOAD_TYPE_LAST_E*/
} MPD_FW_DOWNLOAD_TYPE_ENT;

/**
* @addtogroup API
* @{Logical operations definition
*/
/**
 *  @enum MPD_OP_CODE_ENT
 *  @brief operation to be performed on PHY
 */
typedef enum {
    /** @brief Set MDI/X mode. See \ref MPD_MDIX_MODE_PARAMS_STC \ref set_mdix "go to example" */
    MPD_OP_CODE_SET_MDIX_E,
    /** @brief Get MDI/X operational status. See \ref MPD_MDIX_MODE_PARAMS_STC \ref get_mdix "go to example" */
    MPD_OP_CODE_GET_MDIX_E,
    /** @brief Get MDI/X admin configuration. See \ref MPD_MDIX_MODE_PARAMS_STC \ref get_mdix_admin "go to example"*/
    MPD_OP_CODE_GET_MDIX_ADMIN_E,
    /** @brief Set auto-negotiation parameters. See \ref MPD_AUTONEG_PARAMS_STC \ref set_auto_neg "go to example" */
    MPD_OP_CODE_SET_AUTONEG_E,
    /** @brief Get auto-negotiation admin configuration. See \ref MPD_AUTONEG_PARAMS_STC \ref get_auto_neg_admin "go to example" */
    MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
    /** @brief Restart auto-negotiation on copper port \ref set_restart_auto_neg "go to example" */
    MPD_OP_CODE_SET_RESTART_AUTONEG_E,
    /** @brief Set duplex mode (Half/Full). See \ref MPD_DUPLEX_MODE_PARAMS_STC \ref set_duplex_mode "go to example" */
    MPD_OP_CODE_SET_DUPLEX_MODE_E,
    /** @brief Set speed. See \ref MPD_SPEED_PARAMS_STC  */
    MPD_OP_CODE_SET_SPEED_E,
    /** @brief Execute Basic VCT (TDR) test (get status and cable length). See \ref MPD_VCT_PARAMS_STC \ref set_vct_test "go to example" */
    MPD_OP_CODE_SET_VCT_TEST_E,
    /** @brief Execute advanced VCT (TDR & DSP) test (get polarity, sqew, impedance per pair). See \ref MPD_EXT_VCT_PARAMS_STC \ref get_ext_vct_params "go to example" */
    MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
    /** @brief Execute advanced VCT (TDR & DSP) test and get cable length (only). See \ref MPD_CABLE_LEN_PARAMS_STC \ref get_cable_len "go to example" */
    MPD_OP_CODE_GET_CABLE_LEN_E,
    /** @brief Perform (Soft) reset \ref set_reset_phy "go to example" */
    MPD_OP_CODE_SET_RESET_PHY_E,
    /** @brief Admin disable (shutdown). See \ref MPD_PHY_DISABLE_PARAMS_STC  \ref set_phy_disable_oper "go to example" */
    MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
    /** @brief Get link partner (advertised) auto-negotiation capabilities. See \ref MPD_AUTONEG_CAPABILITIES_PARAMS_STC \ref get_auto_neg_remote_cap "go to example" */
    MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
    /** @brief Advertise flow control capabilities. See \ref MPD_ADVERTISE_FC_PARAMS_STC \ref set_advertise_fc "go to example" */
    MPD_OP_CODE_SET_ADVERTISE_FC_E,
    /** @brief Get link partner (advertised) flow control capabilities. See \ref MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC \ref get_link_partner "go to example"*/
    MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
    /** @brief Configure Green (ED & SR). See \ref MPD_POWER_MODULES_PARAMS_STC \ref set_power_modules "go to example" */
    MPD_OP_CODE_SET_POWER_MODULES_E,
    /** @brief Execute VCT (DSP) and get accurate cable length. See \ref MPD_CABLE_LEN_NO_RANGE_PARAMS_STC \ref get_cable_len_no_range "go to example" */
    MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
    /** @brief Get PHY kind and active media (copper/fiber). See \ref MPD_KIND_AND_MEDIA_PARAMS_STC \ref get_phy_kind_media "go to example" */
    MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
    /** @brief Set SFP present. See \ref MPD_PRESENT_NOTIFICATION_PARAMS_STC \ref set_sfp_present "go to example" */
    MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
    /** @brief Perform MDIO read/write. See \ref MPD_MDIO_ACCESS_PARAMS_STC \ref set_mdio_access "go to example" */
    MPD_OP_CODE_SET_MDIO_ACCESS_E,
    /** @brief Advertise EEE capabilities. See \ref MPD_EEE_ADVERTISE_PARAMS_STC \ref set_eee_adv_cap "go to example" */
    MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
    /** @brief Enable/Disable EEE. See \ref MPD_EEE_MASTER_ENABLE_PARAMS_STC \ref set_eee_master_enable "go to example" */
    MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
    /** @brief Get EEE remote (Advertised) status and local status. See \ref MPD_EEE_STATUS_PARAMS_STC \ref get_eee_status "go to example" */
    MPD_OP_CODE_GET_EEE_STATUS_E,
    /** @brief EEE, set LPI Exit time. See \ref MPD_EEE_LPI_TIME_PARAMS_STC \ref set_LPI_exit_time "go to example" */
    MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
    /** @brief EEE, set LPI Enter time. See \ref MPD_EEE_LPI_TIME_PARAMS_STC \ref set_LPI_enter_time "go to example" */
    MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
    /** @brief Get (local) EEE capability. See \ref MPD_EEE_CAPABILITIES_PARAMS_STC \ref get_EEE_capability "go to example" */
    MPD_OP_CODE_GET_EEE_CAPABILITY_E,
    /** @brief Get PHY status (link & speed). See \ref MPD_INTERNAL_OPER_STATUS_STC \ref get_internal_oper_status "go to example" */
    MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
    /** @brief Get VCT capability - Relevant to DSP(passive test). See \ref MPD_VCT_CAPABILITY_PARAMS_STC \ref get_vct_cap "go to example" */
    MPD_OP_CODE_GET_VCT_CAPABILITY_E,
    /** @brief Get Data Terminal Equipment status\n
     * Relevant for devices that support DTE power function, the DTE power function is used to detect
     * if a link partner requires power supplied by the POE PSE device
     * See \ref MPD_DTE_STATUS_PARAMS_STC \ref get_dte_status "go to example" */
    MPD_OP_CODE_GET_DTE_STATUS_E,
	/**  @brief set DTE configuration. See \ref MPD_DTE_STATUS_PARAMS_STC  */
	MPD_OP_CODE_SET_DTE_E,
    /** @brief Get PHY temperature. See \ref MPD_TEMPERATURE_PARAMS_STC \ref get_phy_temperatura "go to example" */
    MPD_OP_CODE_GET_TEMPERATURE_E,
    /** @brief Get PHY revision. See \ref MPD_REVISION_PARAMS_STC \ref get_phy_revision "go to example"*/
    MPD_OP_CODE_GET_REVISION_E,
    /** @brief Get supported speed & duplex modes. See \ref MPD_AUTONEG_CAPABILITIES_PARAMS_STC \ref get_auto_neg_support "go to example" */
    MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
    /** @brief Set extended speed configurations. See \ref MPD_SPEED_EXT_PARAMS_STC  */
    MPD_OP_CODE_SET_SPEED_EXT_E,
    /** @brief Get extended speed configurations. See \ref MPD_SPEED_EXT_PARAMS_STC  */
    MPD_OP_CODE_GET_SPEED_EXT_E,
    /** @brief Set Serdes rx, tx tune params. See \ref MPD_SERDES_TUNE_STC \ref set_serdes_tune "go to example" */
    MPD_OP_CODE_SET_SERDES_TUNE_E,
    /** @brief Set Serdes Lane Rx/Tx polarity.See \ref MPD_SERDES_LANE_POLARITY_PARAMS_STC */
    MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E,
    /** @brief Multispeed AP advertise*/
    MPD_OP_CODE_SET_AUTONEG_MULTISPEED_E,
     /**  @brief get ETSB (TSQ) info find the first valid entry of the requested port and signature. See \ref MPD_PTP_TSQ_INFO_PARAMS_STC  */
    MPD_OP_CODE_GET_PTP_TSQ_E,
    /** @brief Send IDLE'S to host using packet generator. Sending the idles cause the mac to link up.
      * See \ref MPD_SERDES_LANE_POLARITY_PARAMS_STC */
    MPD_OP_CODE_SEND_IDLE_TO_HOST_E,

    MPD_OP_LAST_PORT_OP_E,
    /*----- operations on phyNumber ----- */
    /** @brief init ptp with clock delay time. See \ref MPD_PTP_INIT_PARAMS_STC  */
    MPD_OP_PHY_CODE_SET_PTP_INIT_E = MPD_OP_LAST_PORT_OP_E,
    /** @brief start FRC (Free Running Counter) capture. */
    MPD_OP_PHY_CODE_SET_PTP_TOD_CAPTURE_E,
    /** get captured FRC (Free Running Counter) see \ref MPD_PTP_CAPTURED_FRC_INFO_PARAMS_STC */
    MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E,

    MPD_OP_LAST_PHY_OP_E
} MPD_OP_CODE_ENT;
/** @} */

/** @addtogroup Autonegotiation
 * @{
 *  @enum MPD_AUTO_NEGOTIATION_ENT
 *  @brief Negotiation Enable/Disable
 */
typedef enum {
	/** @brief Enable Automatic negotiation mode */
    MPD_AUTO_NEGOTIATION_ENABLE_E,
	/** @brief Disable Automatic negotiation mode */
    MPD_AUTO_NEGOTIATION_DISABLE_E
} MPD_AUTO_NEGOTIATION_ENT;
/** @} */

/** @addtogroup Duplex
 * @{
 *  @enum MPD_DUPLEX_ADMIN_ENT
 *  @brief Duplex mode (Half/Full)
 */
typedef enum {
	/** @brief Half Duplex mode */
    MPD_DUPLEX_ADMIN_MODE_HALF_E,
	/** @brief Full Duplex mode */
    MPD_DUPLEX_ADMIN_MODE_FULL_E
} MPD_DUPLEX_ADMIN_ENT;
/** @} */

/** @addtogroup MDIX
 * @{
 *  @enum MPD_MDIX_MODE_TYPE_ENT
 *  @brief MDI/X mode
 */
typedef enum {
	/** @brief MDI mode */
    MPD_MDI_MODE_MEDIA_E,
	/** @brief MDIX mode */
    MPD_MDIX_MODE_MEDIA_E,
	/** @brief Auto mode */
    MPD_AUTO_MODE_MEDIA_E
} MPD_MDIX_MODE_TYPE_ENT;
/** @} */

/** @addtogroup Fiber
 * @{
 *  @enum MPD_KIND_ENT
 *  @brief PHY Kind
 */
typedef enum {
	/** @brief Copper  */
    MPD_KIND_COPPER_E = 0,
	/** @brief SFP  */
    MPD_KIND_SFP_E = 1,
	/** @brief Combo */
    MPD_KIND_COMBO_E = 2,
	/** @brief Invalid mode */
    MPD_KIND_INVALID_E = 3
} MPD_KIND_ENT;
/** @} */

/** @addtogroup Admin
 * @{
 *  @enum MPD_PORT_ADMIN_ENT
 *  @brief Admin status
 */
typedef enum {
	/** @brief Admin down */
    MPD_PORT_ADMIN_DOWN_E,
	/** @brief Admin up */
    MPD_PORT_ADMIN_UP_E,
	/** @brief Unknown */
    MPD_PORT_ADMIN_UNKNOWN_E
} MPD_PORT_ADMIN_ENT;
/** @} */

/** @addtogroup Autonegotiation
 * @{
 *  @enum MPD_AUTONEGPREFERENCE_ENT
 *  @brief Auto-negotiation preference (master/slave)
 */
typedef enum {
	/** @brief Auto-negotiation master mode */
    MPD_AUTONEGPREFERENCE_MASTER_E  = 0,
	/** @brief Auto-negotiation slave mode */
    MPD_AUTONEGPREFERENCE_SLAVE_E   = 1,
	/** @brief Auto-negotiation Unknown */
    MPD_AUTONEGPREFERENCE_UNKNOWN_E = 2
} MPD_AUTONEGPREFERENCE_ENT;
/** @} */
/** @addtogroup Fiber
 * @{
 *  @enum MPD_COMFIG_PHY_MEDIA_TYPE_ENT
 *  @brief Active media
 */
typedef enum {
	/** @brief Copper is an Active Media */
    MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E,
	/** @brief Fibber is an Active Media */
    MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E,
	/** @brief Invalid Media */
    MPD_COMFIG_PHY_MEDIA_TYPE_INVALID_E
} MPD_COMFIG_PHY_MEDIA_TYPE_ENT;
/** @} */

/** @addtogroup VCT
 * @{
 *  @enum MPD_CABLE_LENGTH_ENT
 *  @brief Cable length ranges (Result of DSP)
 */
/* the enum start with 1 to fit SNMP values */
typedef enum {
	/** @brief cable length range less then 50 meters */
    MPD_CABLE_LENGTH_LESS_THAN_50M_E = 1,
	/** @brief cable length range between 50 to 80 meters */
    MPD_CABLE_LENGTH_50M_80M_E,
	/** @brief cable length range between 80 to 110 meters */
    MPD_CABLE_LENGTH_80M_110M_E,
	/** @brief cable length range between 110 to 140 meters */
    MPD_CABLE_LENGTH_110M_140M_E,
	/** @brief cable length range more than 140 meters */
    MPD_CABLE_LENGTH_MORE_THAN_140M_E,
	/** @brief unknown cable length  */
    MPD_CABLE_LENGTH_UNKNOWN_E,
} MPD_CABLE_LENGTH_ENT;
/** @} */

/** @internal
 *  @enum MPD_GIG_PHY_LED_OPERATION_ENT
 *  @brief Turn 1G Led On/Off
 */
typedef enum {
	/** @brief Turn 1G Led On  */
    MPD_GIG_PHY_LED_ON_E,
	/** @brief Turn 1G Led Off  */
    MPD_GIG_PHY_LED_OFF_E,
	/** @brief Led Unknown*/
    MPD_GIG_PHY_LED_UNKNOWN_E
} MPD_GIG_PHY_LED_OPERATION_ENT;




/**
  * @brief maximum number of pairs in cable. Mostly: (1,2) (3,6) (4,5) (7,8)
  */
#define MPD_VCT_MDI_PAIR_NUM_CNS    4

/**
 * @brief number of channel pairs in cable: A/B and C/D
 */
#define MPD_VCT_CHANNEL_PAIR_NUM_CNS 2

/**
 * @brief Amplitude threshold for cables shorter then 110m
 */
#define MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_CNS 40

/**
 * @brief Amplitude threshold for cables longer then 110m
 */
#define MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_FOR_110M_CABLE_CNS 20

/**
 * @brief 110m minimum possible measured length due to 10% possible deviation.
 * (110 * 90% = 99 > 98).
 */

#define  MPD_VCT_MIN_110M_CABLE_LEN_DUE_TO_DEVIATION_CNS 98

/**
 * @enum MPD_VCT_PAIR_SWAP_ENT
 *
 * @brief Enumeration for pair swap
 */
typedef enum {

    /** @brief channel A on MDI[0] and B on MDI[1]
     *  or channel C on MDI[3] and D on MDI[4]
     */
    MPD_VCT_CABLE_SWAP_STRAIGHT_E,

    /** @brief channel B on MDI[0] and A on MDI[1]
     *  or channel D on MDI[3] and C on MDI[4]
     */
    MPD_VCT_CABLE_SWAP_CROSSOVER_E,

    /**
     * @brief in FE just two first channels are checked
     */
    MPD_VCT_NOT_APPLICABLE_SWAP_E

} MPD_VCT_PAIR_SWAP_ENT;

/**
 * @enum MPD_VCT_POLARITY_SWAP_ENT
 *
 * @brief Enumeration for pair polarity swap
 */
typedef enum {

    /**
     * @brief good polarity
     */
    MPD_VCT_POSITIVE_POLARITY_E,

    /**
     * @brief reversed polarity
     */
    MPD_VCT_NEGATIVE_POLARITY_E,

    /**
     * @brief in FE there just one value for all the cable then just first pair is applicable
     */
    MPD_VCT_NOT_APPLICABLE_POLARITY_E

} MPD_VCT_POLARITY_SWAP_ENT;

/**
 * @struct MPD_VCT_PAIR_SKEW_STC
 *
 * @brief pair skew values.
*/
typedef struct {

    /** @brief whether results are valid
     *  (not valid for FE).
     */
    BOOLEAN isValid;

    /**
     * @brief
     */
    UINT_32 skew[MPD_VCT_MDI_PAIR_NUM_CNS];

} MPD_VCT_PAIR_SKEW_STC;

/**
* @struct MPD_VCT_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diagnostic status per MDI pair/channel
 * pair.
*/
typedef struct {

    /** @brief if results are valid (if not, maybe there is no
     *  gigabit link for GE or 100M link for FE).
     */
    BOOLEAN isValid;

    MPD_VCT_PAIR_SWAP_ENT pairSwap[MPD_VCT_CHANNEL_PAIR_NUM_CNS];

    MPD_VCT_POLARITY_SWAP_ENT pairPolarity[MPD_VCT_MDI_PAIR_NUM_CNS];

    /** @brief the skew among the four pairs of the cable
     *  (delay between pairs in n-Seconds)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  in this case:
     *  swap - just pairSwap[0] relevant
     *  polarity - just pairPolarity[0] and pairPolarity[1] are relevant
     *  skew - will be not relevant
     */
    MPD_VCT_PAIR_SKEW_STC pairSkew;

} MPD_VCT_EXTENDED_STATUS_STC;

/**
 * @struct MPD_VCT_ACCURATE_CABLE_LEN_STC
 *
 * @brief accurate cable length for each MDI pair.
 */
typedef struct {

    BOOLEAN isValid[MPD_VCT_MDI_PAIR_NUM_CNS];

    UINT_16 cableLen[MPD_VCT_MDI_PAIR_NUM_CNS];

} MPD_VCT_ACCURATE_CABLE_LEN_STC;

/**
 * @struct MPD_VCT_CABLE_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diag. status per MDI pair/channel pair.
 */
typedef struct {
    /**
     * @brief extended VCT cable status.
     */
    MPD_VCT_EXTENDED_STATUS_STC vctExtendedCableStatus;

    /** @brief accurate cable length.
     *  accurateCableLen - not relevant
     */
    MPD_VCT_ACCURATE_CABLE_LEN_STC accurateCableLen;

} MPD_VCT_CABLE_EXTENDED_STATUS_STC;


/**
 * @struct MPD_VCT_STATUS_STC
 *
 * @brief Structure holding VCT results
 */
typedef struct {

    /**
     * @brief VCT test result.
     */
    MPD_VCT_RESULT_ENT testResult;

    /**
     * @brief for cable failure the estimate fault distance in meters.
     */
    UINT_8 errCableLen;

} MPD_VCT_STATUS_STC;


/**
 * @struct MPD_VCT_TDR_CABLE_STC
 *
 * @brief virtual cable diagnostic status per MDI pair.
*/
typedef struct {
    /**
     * @brief structure holding vct results per pair
     */
    MPD_VCT_STATUS_STC cableStatus[MPD_VCT_MDI_PAIR_NUM_CNS];
    /** @brief type of phy (100M phy or Gigabit phy)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  One is RX Pair (cableStatus[0] or cableLen[0]) and
     *  the other is TX Pair (cableStatus[1] or cableLen[1]).
     */
} MPD_VCT_TDR_CABLE_STC;

/**
* @addtogroup MDIX
* @{MPD API
*
* @struct  MPD_MDIX_MODE_PARAMS_STC
* @brief Used for
* - \ref MPD_OP_CODE_GET_MDIX_E
* - \ref MPD_OP_CODE_SET_MDIX_E
* - \ref MPD_OP_CODE_GET_MDIX_ADMIN_E
*/
typedef struct {
    /** @brief [in,out] mode - the MDI/X mode */
    MPD_MDIX_MODE_TYPE_ENT mode;
} MPD_MDIX_MODE_PARAMS_STC;
/** @} */


/** @addtogroup Autonegotiation
 * @{MPD API

 * - Use \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E in order to retrieve link partner capabilities
 * - Use \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E in order to retrieve link partner flow control advertised capabilities
 * - Use \ref MPD_OP_CODE_SET_RESTART_AUTONEG_E in order to manually trigger restart auto-negotiation on copper port
 * @struct  MPD_AUTONEG_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_AUTONEG_E
 * - \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E
 */
typedef struct {
    /**
     * @brief [in] enable - Auto negotiation enable/disable
     */
    MPD_AUTO_NEGOTIATION_ENT     enable;
    /**
     * @brief [in] capabilities - Auto negotiation advertised (speed & duplex) capabilities
     * @remark Relevant when \a enable is TRUE
     */
    MPD_AUTONEG_CAPABILITIES_TYP capabilities;
    /**
     * @brief [in] masterSlave - Auto-negotiation Master/Slave preference (not force)
     * @remark Relevant when \a enable is TRUE
     */
    MPD_AUTONEGPREFERENCE_ENT    masterSlave;
}  MPD_AUTONEG_PARAMS_STC;
/** @} */

/** @addtogroup Duplex
 * @{MPD API Set Duplex mode
 * @remark Half duplex is supported only for 10M & 100M speeds
 * @remark In order to force duplex, first disable auto-negotiation using MPD_OP_CODE_SET_AUTONEG_E
 * @struct  MPD_DUPLEX_MODE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_DUPLEX_MODE_E
 */
typedef struct {
    /**
     * @brief [in] mode - Duplex mode (HALF/FULL)
     */
    MPD_DUPLEX_ADMIN_ENT mode;
} MPD_DUPLEX_MODE_PARAMS_STC;
/** @} */

/** @addtogroup VCT
 * @{MPD API Virtual cable test operations
 * - Use \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E in order to get for which speeds VCT is supported
 * - Use \ref MPD_OP_CODE_SET_VCT_TEST_E in order to execute basic TDR, retreive result and cable length
 * - Use \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E in order to execute advanced VCT (TDR & DSP) test (get polarity, sqew, impedance per pair)
 * - Use \ref MPD_OP_CODE_GET_CABLE_LEN_E in order to execute advanced VCT (TDR & DSP) test and get cable length (only)
 * @struct  MPD_VCT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_VCT_TEST_E
 */
typedef struct {
    /**
     * @brief [out] testResult - VCT Test result
     */
    MPD_VCT_RESULT_ENT    testResult;
    /**
     * @brief [out] cableLength - Cable length in meters
     */
    UINT_32               cableLength;
} MPD_VCT_PARAMS_STC;

/**
 * @struct  MPD_EXT_VCT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E
 */
typedef struct {
    /**
     * @brief [in] testType - VCT Test data that is queried
     */
    MPD_VCT_TEST_TYPE_ENT   testType;
    /**
     * @brief [out] result - Result according to test type
     */
    UINT_32                 result;
} MPD_EXT_VCT_PARAMS_STC;

/**
 * @struct  MPD_VCT_CAPABILITY_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E
 */
typedef struct {
    /**
     * @brief [out] vctSupportedSpeedsBitmap - Bit map of speeds that support VCT
     */
    MPD_SPEED_CAPABILITY_TYP vctSupportedSpeedsBitmap;
} MPD_VCT_CAPABILITY_PARAMS_STC;

/**
 * @struct  MPD_CABLE_LEN_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_CABLE_LEN_E
 */
typedef struct {
    /**
     * @brief [out] cableLength - When running DSP with link up without breaking link, VCT measured length in low resulution
     */
    MPD_CABLE_LENGTH_ENT  cableLength;
    /**
     * @brief [out] accurateLength - Accurate cable length, when available
     */
    UINT_32               accurateLength;
} MPD_CABLE_LEN_PARAMS_STC;
/**
 * @struct  MPD_CABLE_LEN_NO_RANGE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E
 */
typedef struct {
    /**
     * @brief [out] cableLen - Cable length in meters
     */
    UINT_32 cableLen;
} MPD_CABLE_LEN_NO_RANGE_PARAMS_STC;
/** @} */


/** @addtogroup Autonegotiation
 * @{
 * @struct  MPD_AUTONEG_CAPABILITIES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E
 * - \ref MPD_OP_CODE_GET_AUTONEG_SUPPORT_E
 */
typedef struct {
    /**
     * @brief [out] capabilities -
     * When calling MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E: Link partner (detected) negotiation capabilities
     * When calling MPD_OP_CODE_GET_AUTONEG_SUPPORT_E: Local supported negotiated capabilities
     */
    MPD_AUTONEG_CAPABILITIES_TYP capabilities;
}  MPD_AUTONEG_CAPABILITIES_PARAMS_STC;

/**
 * @struct  MPD_ADVERTISE_FC_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_ADVERTISE_FC_E
 */
typedef struct {
    /**
     * @brief [in] advertiseFc - Advertise Flow control (requires auto-negotiation enabled)
     */
    BOOLEAN advertiseFc;
} MPD_ADVERTISE_FC_PARAMS_STC;

/**
 * @struct  MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E
 */
typedef struct {
    /**
     * @brief [out] pauseCapable - Link partner (negotiated) pause (FC) ability
     */
    BOOLEAN pauseCapable;
    /**
     * @brief [out] asymetricRequested - Link partner (negotiated) requests asymetric pause
     */
    BOOLEAN asymetricRequested;
} MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC;
/** @} */

/** @addtogroup Admin
 * @{MPD API Set PHY administrative mode
 * @struct  MPD_PHY_DISABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E
 */
typedef struct {
    /**
     * @brief [in] forceLinkDown - Force link down or unforce link down
     */
    BOOLEAN forceLinkDown;
} MPD_PHY_DISABLE_PARAMS_STC;
/** @} */

/** @addtogroup EEE_Green EEE & Green
 * @{MPD API Green related configuration
 * - Use \ref MPD_OP_CODE_SET_POWER_MODULES_E in order to enable or disable
 * - Energy Detect (ED) feature
 * - Short Reach (SR) feature
 * @remark Power consumption valuse are not measured, these are calculated based on static beforhand measurements
 * @struct MPD_POWER_MODULES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_POWER_MODULES_E
 */
typedef struct {
    /**
     * @brief [in] energyDetetct - Enery Detect configuration to apply
     */
    MPD_GREEN_SET_ENT energyDetetct;
    /**
     * @brief [in] shortReach - Short Reach configuration to apply
     */
    MPD_GREEN_SET_ENT shortReach;
    /**
     * @brief [in] performPhyReset - Perform PHY reset
     */
    BOOLEAN           performPhyReset;
} MPD_POWER_MODULES_PARAMS_STC;


/** @} */


/**
 * @struct MPD_MDIO_ACCESS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_MDIO_ACCESS_E
 */
typedef struct {
    /**
     * @brief [in] type - Access type (read/write)
     */
    MPD_MDIO_ACCESS_TYPE_ENT             type;
    /**
     * @brief [in] device - Device Or Page within PHY
     */
    UINT_16                              deviceOrPage;
    /**
     * @brief [in] address - Register address/offset
     */
    UINT_16                              address;
    /**
     * @brief [in] mask - bits to write/ read
     */
    UINT_16                              mask;
    /**
     * @brief [in,out] data - Register Value
     */
    UINT_16                              data;
} MPD_MDIO_ACCESS_PARAMS_STC;

/** @} */

/** @addtogroup EEE_Green EEE & Green
 * @{MPD API  Energy Efficient Ethernet, 802.3az
 * - Use \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E in order to configure EEE advertisement capabilities
 * - Use \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E in order to enable and disable EEE
 * - Use \ref MPD_OP_CODE_GET_EEE_STATUS_E in order to get remote (Advertised) status and local status
 * - Use \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E in order to configure Low Power Idle (LPI) exit timer
 * - Use \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E in order to configure Low Power Idle (LPI) enter timer
 * - Use \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E in order to get local EEE capability
 * @struct MPD_EEE_ADVERTISE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E
 */
typedef struct {
    /**
     * @brief [in] speedBitmap - Speeds on which to advertise EEE
     */
    MPD_SPEED_CAPABILITY_TYP    speedBitmap;
    /**
     * @brief [in] advEnable - Advertise EEE
     */
    BOOLEAN                     advEnable;
} MPD_EEE_ADVERTISE_PARAMS_STC;

/**
 * @struct MPD_EEE_MASTER_ENABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E
 */
typedef struct {
    /**
     * @brief [in] masterEnable - EEE Admin mode
     */
    BOOLEAN     masterEnable;
} MPD_EEE_MASTER_ENABLE_PARAMS_STC;

/**
 * @struct MPD_EEE_CAPABILITIES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E
 */
typedef struct {
    /**
     * @brief [out] enableBitmap - EEE ability speed bitmap
     */
    MPD_SPEED_CAPABILITY_TYP enableBitmap;
} MPD_EEE_CAPABILITIES_PARAMS_STC;

/**
 * @struct MPD_EEE_STATUS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_EEE_STATUS_E
 */
typedef struct {
    /**
     * @brief [out] enableBitmap - EEE ability speed bitmap for remote (Advertised) status
     */
    MPD_SPEED_CAPABILITY_TYP enableBitmap;
    /**
     * @brief [out] localEnable - local operative EEE status
     */
    BOOLEAN                  localEnable;
} MPD_EEE_STATUS_PARAMS_STC;

/**
 * @struct MPD_EEE_LPI_TIME_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E
 * - \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E
 */
typedef struct {
    /**
     * @brief [in] speed - speed on which to configure
     */
    MPD_EEE_SPEED_ENT    speed;
    /**
     * @brief [in] time_us - time in microseconds
     */
    UINT_16              time_us;
} MPD_EEE_LPI_TIME_PARAMS_STC;

/** @} */

/** @addtogroup MISC Miscelanious
 * @{MPD API - miscelanious
 * @struct MPD_DTE_STATUS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_DTE_STATUS_E
 * - \ref MPD_OP_CODE_SET_DTE_E
 */
typedef struct {
    /**
     * @brief [in] detect - TRUE to enable DTE, FALSE to disable
     * @brief [out] detect - Is DTE detected
     */
    BOOLEAN     detect;
} MPD_DTE_STATUS_PARAMS_STC;


/**
 * @struct MPD_TEMPERATURE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_TEMPERATURE_E
 */
typedef struct {
    /**
     * @brief [out] temperature - in celcius
     */
    UINT_16     temperature;
} MPD_TEMPERATURE_PARAMS_STC;

/**
 * @struct MPD_REVISION_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_REVISION_E
 */
typedef struct {
    /**
     * @brief [out] revision - PHY revision
     */
    UINT_16         revision;
    /**
     * @brief [out] phyType - PHY Type
     */
    MPD_TYPE_ENT    phyType;
} MPD_REVISION_PARAMS_STC;
/** @} */

/** @addtogroup Speed
 * @{MPD API Configure PHY speed
 * - Use \ref MPD_OP_CODE_SET_SPEED_E in order to configure PHY speed
 * @remark speed 10M & 100M are forced with negotiation disabled.
 * @remark 1G and higher speeds are working with negotiation enabled and advertising one speed
 * @struct MPD_SPEED_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SPEED_E
 */
typedef struct {
    /**
     * @brief [in] speed - PHY speed to configure
     */
    MPD_SPEED_ENT                   speed;

} MPD_SPEED_PARAMS_STC;
/** @} */

/** @addtogroup Fiber
 * @{MPD API Fiber related APIs
 * - Use \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E in order to update PHY driver with SFP present (LoS) status
 * - Use \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E in order to get operational media
 * @struct MPD_PRESENT_NOTIFICATION_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E
 */
typedef struct {
    /**
     * @brief [in] isSfpPresent - Is SFP signal (LOS) present
     */
    BOOLEAN             isSfpPresent;
    /**
     * @brief [in] opMode - type of media detected
     */
    MPD_OP_MODE_ENT     opMode;
} MPD_PRESENT_NOTIFICATION_PARAMS_STC;


/**
 * @struct MPD_KIND_AND_MEDIA_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E
 */
typedef struct {
    /**
     * @brief [out] phyKind - Copper/Fiber/Combo
     */
    MPD_KIND_ENT                    phyKind;
    /**
     * @brief [out] isSfpPresent - Is SFP signal present (DB)
     */
    BOOLEAN                         isSfpPresent;
    /**
     * @brief [out] mediaType - Active Media
     */
    MPD_COMFIG_PHY_MEDIA_TYPE_ENT   mediaType;
} MPD_KIND_AND_MEDIA_PARAMS_STC;
/** @} */

/** @addtogroup OperationalStatus Operational Status
 * @{MPD API Get operational status
 * @struct MPD_INTERNAL_OPER_STATUS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E
 */
typedef struct {
    /**
     * @brief [out] isOperStatusUp - Is PHY operational UP
     * @note All following fields are relevant only when this is TRUE
     */
    BOOLEAN                         isOperStatusUp;
    /**
     * @brief [out] phySpeed - PHY operational speed
     */
    MPD_SPEED_ENT                   phySpeed;
    /**
     * @brief [out] phySpeed - PHY operational duplex TRUE - full duplex, FALSE - half duplex
     */
    BOOLEAN                         duplexMode;
    /**
     * @brief [out] phySpeed - Active media type
     */
    MPD_COMFIG_PHY_MEDIA_TYPE_ENT   mediaType;
    /**
     * @brief [out] phySpeed - Is negotiation complete
     */
    BOOLEAN                         isAnCompleted;
} MPD_INTERNAL_OPER_STATUS_STC;
/** @} */

/** @addtogroup FecMode
 * @{
 *      @enum MPD_FEC_MODE_SUPPORTED_ENT
 *  @brief Forward Error Correction modes
 */
typedef enum {

    /** FC-FEC enabled on port */
    MPD_FEC_MODE_ENABLED_E,

    /** FEC disabled on port */
    MPD_FEC_MODE_DISABLED_E,

    /** @brief Reed-Solomon (528,514) FEC mode
     */
    MPD_RS_FEC_MODE_ENABLED_E,

    /** @brief Used only for AP advertisment. Both FC and RS fec supported.
     */
    MPD_BOTH_FEC_MODE_ENABLED_E,

    /** @brief Reed-Solomon (544,514) FEC mode
     */
    MPD_RS_FEC_544_514_MODE_ENABLED_E,

    MPD_FEC_MODE_LAST_E
} MPD_FEC_MODE_SUPPORTED_ENT;
/** @} */

/** @addtogroup Speed_Extended
 * @{
 *      @enum MPD_INTERFACE_MODE_ENT
 *      @brief Interface mode
 */

typedef enum {
    MPD_INTERFACE_MODE_KR_E     = 16,
    MPD_INTERFACE_MODE_SR_LR_E  = 20,
    MPD_INTERFACE_MODE_KR2_E    = 27,
    MPD_INTERFACE_MODE_KR4_E    = 28,
    MPD_INTERFACE_MODE_SR_LR2_E = 29,
    MPD_INTERFACE_MODE_SR_LR4_E = 30,
    MPD_INTERFACE_MODE_CR_E     = 36,
    MPD_INTERFACE_MODE_CR2_E    = 37,
    MPD_INTERFACE_MODE_CR4_E    = 38,
    MPD_INTERFACE_MODE_KR8_E    = 41,
    MPD_INTERFACE_MODE_CR8_E    = 42,
    MPD_INTERFACE_MODE_SR_LR8_E = 43,
    MPD_INTERFACE_MODE_NA_E     = 56
} MPD_INTERFACE_MODE_ENT;

/**
 * @struct MPD_SPEED_PHY_SIDE_PARAMS_STC
 * @brief Used for Speed Extended configuration at line side and host side of PHY
 */
typedef struct {
    /**
     * @brief [in] apEnable - Port auto-neg enable
     */
    BOOLEAN                         apEnable;
    /**
     * @brief [in] speed - Port speed
     */
    MPD_SPEED_ENT                   speed;
    /**
     * @brief [in] ifMode - Interface mode
     */
    MPD_INTERFACE_MODE_ENT          ifMode;
    /**
     * @brief [in] fecMode - Port FEC mode at host/line side
     */
    MPD_FEC_MODE_SUPPORTED_ENT      fecMode;
} MPD_SPEED_PHY_SIDE_PARAMS_STC;

/**
 * MPD API Speed related APIs
 * - Use \ref MPD_OP_CODE_GET_SPEED_EXT_E in order to get speed and fec mode
 * @struct MPD_SPEED_EXT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SPEED_EXT_E
 * - \ref MPD_OP_CODE_GET_SPEED_EXT_E
 */
typedef struct {
    /**
     * @brief [in] isRetimerMode - ie retimer mode.
     */
    BOOLEAN                         isRetimerMode;

    /**
    * @brief [in] hostSide - Phy hostSide Port Params.
           */

    MPD_SPEED_PHY_SIDE_PARAMS_STC   hostSide;
    /**
     * @brief [in] lineSide - Phy lineSide Port Params.
     */
    MPD_SPEED_PHY_SIDE_PARAMS_STC   lineSide;
} MPD_SPEED_EXT_PARAMS_STC;

/**
 * @enum MPD_SERDES_CONFIG_ENT
 * @brief Used for serdes tune - is it permanent configuration or not.
 */
typedef enum {
    /*@brief Different tune params per speed/omMpde -> configure when speed/opMode changes. */
    MPD_SERDES_CONFIG_IMMIDIATE_E,
    /*@brief Same tune params to all speed & opMode -> configure only once during init sequence. */
    MPD_SERDES_CONFIG_PERMANENT_E
}MPD_SERDES_CONFIG_ENT;

/**
 * @enum MPD_PHY_HOST_LINE_ENT
 * @brief Used for Speed Extended configuration at line side and host side of PHY
 */
typedef enum {
    /*@brief host side */
    MPD_PHY_SIDE_HOST_E,
    /*@brief line side */
    MPD_PHY_SIDE_LINE_E,
    MPD_PHY_SIDE_LAST_E
} MPD_PHY_HOST_LINE_ENT;
/** @} */

/** @addtogroup Serdes_Tune
 * @{
 *      @enum MPD_SERDES_PARAMS_TYPE_ENT
 *      @brief Port Serdes Rx/Tx params type.
 */
typedef enum {
    /*@brief Serdes params set from xml */
    MPD_SERDES_PARAMS_SET_FROM_XML_E,
    /*@brief Serdes tx side params */
    MPD_SERDES_PARAMS_TX_E,
    /*@brief Serdes rx side params */
    MPD_SERDES_PARAMS_RX_E,
    /*@brief Serdes tx and rx side params */
    MPD_SERDES_PARAMS_BOTH_E,
    MPD_SERDES_PARAMS_LAST_E
} MPD_SERDES_PARAMS_TYPE_ENT;

/**
 * @union MPD_SERDES_TX_CONFIG_UNT
 * @brief relevant to MTD (comphy)/ MYD (avago) PHYs \n
 * defines the TX serdes tuning parameters
 * comphy_28G - 2540,3540, 2580, 3580
 * comphy - 32x0, 33x0, 21x0, 20x0
 * avago - 7120
 */
typedef union {
    /* main + pre + post <= 40*/
    struct {
        /**
        * @brief [in] pre - pre cursor
        */
        UINT_8      preTap;    /* 15:11 */
        /**
        * @brief [in] main/TX_AMP in 88E1780 dataSheet.
        */
        UINT_8      mainTap;   /* 10:5  */
        /**
        * @brief [in] post - post cursor
        */
        UINT_8      postTap;   /* 4:0   */
    } comphy;
    struct {
        /**
        * @brief [in] pre - pre cursor
        */
        UINT_8      pre;
        /**
        * @brief [in] peak
        */
        UINT_8      peak;
        /**
        * @brief [in] post - post cursor
        */
        UINT_8      post;
        /**
        * @brief [in] margin
        */
        UINT_8      margin;
    } comphy_28G;
    struct {
        /**
          * @brief [in] atten - attenuation
          */
        UINT_16  atten;
        /**
          * @brief [in] post - post cursor
          */
        UINT_16  post;
        /**
          * @brief [in] pre - pre cursor
          */
        UINT_16  pre;
        /**
          * @brief [in] pre2 - pre cursor2
          */
        UINT_16  pre2;
        /**
          * @brief [in] pre3 - pre cursor3
          */
        UINT_16  pre3;
    } avago;
} MPD_SERDES_TX_CONFIG_UNT;

/**
 *      @struct MPD_SERDES_RX_CONFIG_STC
 *      @brief Port Serdes Rx params .
 */
typedef struct {
    /*@brief dc  */
    UINT_16   dc;

    /*@brief low-frequency  */
    UINT_16   lf;

    /*@brief high-frequency  */
    UINT_16   hf;

    /*@brief bandwidth  */
    UINT_16   bw;

} MPD_SERDES_RX_CONFIG_STC;

/**
 *      @struct MPD_SERDES_TUNE_STC
 *      @brief Port Serdes Tune params .
 */
typedef struct {
    /** @brief paramsType -
     *  tx/rx/both
     */
    MPD_SERDES_PARAMS_TYPE_ENT paramsType;
    /** @brief txTune -
         *  TX parameters
         */
    MPD_SERDES_TX_CONFIG_UNT txTune;
    /** @brief rxTune -
     *  RX parameters, relevant to MYD
     */
    MPD_SERDES_RX_CONFIG_STC rxTune;
} MPD_SERDES_TUNE_STC;

/**
 * MPD API Serdes rx/tx tune  related APIs
 * - Use \ref MPD_OP_CODE_SET_SERDES_TUNE_E in order to update PHY driver with serdes rx/tx params
 * @struct MPD_SERDES_TUNE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SERDES_TUNE_E
 */
typedef struct {
    /** @brief hostOrLineSide -
     * PHY side to configure
     */
    MPD_PHY_HOST_LINE_ENT            hostOrLineSide;
    /** @brief lanesBmp -
     * Port lane number, relevant to MYD
     */
    UINT_32                           lanesBmp;
    /** @brief tuneParams -
     * Serdes Tune params
     */
    MPD_SERDES_TUNE_STC              tuneParams;
    /** @brief txConfig -
    * config immidiate or pernament
    */
    MPD_SERDES_CONFIG_ENT            txConfig;
}MPD_SERDES_TUNE_PARAMS_STC;

/**
 *      @struct MPD_SERDES_CONFIG_PARAMS_STC
 *      @brief Port Serdes Tune params.
 */
typedef struct {
    /*@brief phy speed */
    MPD_SPEED_ENT               speed;

    /*@brief fiber. direct attach, copper_sfp */
    MPD_OP_MODE_ENT             opMode;

    /*@brief serdes override information */
    MPD_SERDES_TUNE_PARAMS_STC    serdesTuneParams;
}MPD_SERDES_CONFIG_PARAMS_STC;


/** @} */

/** @addtogroup Serdes_Polarity
 * @{ MPD API Serdes rx/tx tune  related APIs
 * - Use \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E in order to update PHY driver with serdes rx/tx polarity
 * @struct MPD_SERDES_LANE_POLARITY_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E
 */
typedef struct {
    /** @brief hostOrLineSide -
     * PHY side to configure
     */
    MPD_PHY_HOST_LINE_ENT            hostOrLineSide;
    /** @brief laneNum -
     * Port Lane number
     */
    UINT_32                          laneNum;
    /** @brief invertTx -
     * Port Lane polarity invert at Tx
     */
    BOOLEAN                          invertTx;
    /** @brief invertRx -
     * Port Lane polarity invert at Rx
     */
    BOOLEAN                          invertRx;

} MPD_SERDES_LANE_POLARITY_PARAMS_STC;

/** @} */


/** @addtogroup MISC Miscelanious
 * @{
 * @enum MPD_PTP_LOAD_REASON_ENT
 * @brief first application should give the load time for pulse
 *        and in the next step the load time for pps when the
 *        pulse is already synced
 */
typedef enum {
    /** @brief when pulse received what to load */
    MPD_PTP_LOAD_TIME_FOR_PULSE_E = 0,
    /** @brief load time for PPS */
    MPD_PTP_LOAD_TIME_FOR_PPS_E = 1
} MPD_PTP_LOAD_REASON_ENT;
/** @} */

/**
* @struct MPD_PTP_TOD_COUNT_STC
 *
 * @brief Structure for TOD Counter.
*/
typedef struct{
    /** nanosecond value of the time stamping TOD counter */
    UINT_32 nanoSeconds;
    /** @brief second low value of the time stamping TOD counter */
    UINT_32 lowSeconds;
    /** @brief second high value of the time stamping TOD counter */
    UINT_32 highSeconds;
    /** @brief fractional nanosecond part of the value. */
    UINT_32 fractionalNanoSeconds;

} MPD_PTP_TOD_COUNT_STC;

/** @addtogroup PTP PTP
* @{
 * @struct MPD_PTP_INIT_PARAMS_STC,
 * @brief used for
 * - \ref MPD_OP_PHY_CODE_SET_PTP_INIT_E
 */
typedef struct {
    /**  @brief reason -
    * syncing pulse or starting pps
    */
    MPD_PTP_LOAD_REASON_ENT reason;
    /**  @brief loadTime -
    * load time when pulse or pps received
    */
    MPD_PTP_TOD_COUNT_STC loadTime;
}  MPD_PTP_INIT_PARAMS_STC;

/**
 * @struct MPD_PTP_CAPTURED_FRC_STC
 */
typedef struct {
    /**  @brief valid -
    * reg value is valid
    */
    BOOLEAN   valid;
    /**  @brief fractionalNanoseconds
    */
    UINT_32   fractionalNanoseconds;
    /**  @brief nanoseconds
    */
    UINT_32   nanoseconds;
    /**  @brief lowSeconds
    */
    UINT_32   lowSeconds;
    /**  @brief highSeconds
    */
    UINT_16   highSeconds;
} MPD_PTP_CAPTURED_FRC_STC;

/**
 * @struct MPD_PTP_CAPTURED_FRC_INFO_PARAMS_STC
 * @brief - \ref MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E
 */
typedef struct {
    /**  @brief todCapture0
    */
    MPD_PTP_CAPTURED_FRC_STC   todCapture0;
    /**  @brief todCapture1
    */
    MPD_PTP_CAPTURED_FRC_STC   todCapture1;
} MPD_PTP_CAPTURED_FRC_INFO_PARAMS_STC;

/**
 * @struct MPD_PTP_TSQ_INFO_PARAMS_STC
 * @brief - \ref MPD_OP_CODE_GET_PTP_TSQ_E
 */
typedef struct {
    /**  @brief signature -
    * input parameter which identifies the desired packet sent
    */
    UINT_16 signature;
     /**  @brief timestamp -
     * output parameter holding the time stamp of the desired packet
     */
    UINT_32 timestamp;
} MPD_PTP_TSQ_INFO_PARAMS_STC;

/**
 * @struct MPD_SEND_IDLE_TO_HOST_PARAMS_STC
 * @brief - \ref MPD_OP_CODE_SEND_IDLE_TO_HOST_E
 */
typedef struct {
    /**
    * @brief [in] enable/disable sending Idles to Host
    */
    BOOLEAN sendIdle;
} MPD_SEND_IDLE_TO_HOST_PARAMS_STC;

/** @} */

/* ******************* INTERNAL ****************/
/** @internal
 * @struct PRV_MPD_INITIALIZE_PHY_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_INIT_E
 */
typedef struct {
    /**
     * @brief [in] initializeDb - DB or HW initialization
     */
    BOOLEAN initializeDb;
} PRV_MPD_INITIALIZE_PHY_STC;

/** @internal
 * @struct PRV_MPD_FIBER_MEDIA_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E
 * - PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E
 * @warning Internal - do not use
 */
typedef struct {
    /*  INPUT  */
    BOOLEAN                         fiberPresent;
    BOOLEAN                         comboModeWa;
    MPD_SPEED_ENT                   phySpeed;/* todo - move to application */
    /*  INPUT / OUTPUT  */
    MPD_COMFIG_PHY_MEDIA_TYPE_ENT   mediaType;
} PRV_MPD_FIBER_MEDIA_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_LOOP_BACK_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_LOOP_BACK_E
 */
typedef struct {
    /** @internal
     * @brief [in] enable - enable loopback     */
    BOOLEAN					enable;
	/* @brief [in] loopbackMode - loop location in the phy */
	MPD_LOOPBACK_MODE_TYP	loopbackMode;
} PRV_MPD_LOOP_BACK_STC;

/** @internal
 * @struct  PRV_MPD_VCT_OFFSET_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_VCT_OFFSET_E
 * @warning - internal, do not use
 */
typedef struct {
    BOOLEAN do_set;
    UINT_32 offset;
} PRV_MPD_VCT_OFFSET_PARAMS_STC;

/** @internal
 * @struct MPD_ERRATAS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_ERRATA_E
 * @warning Internal
 */
typedef struct {
    MPD_ERRATA_TYP erratasBitmap;
} PRV_MPD_ERRATAS_PARAMS_STC;

/** @internal
 * @struct MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E
 */
typedef struct {
    /**
     * @brief [in] enable - enable fast link down
     */
    BOOLEAN     enable;
} PRV_MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC;

/** @internal
 * @struct MPD_EEE_MAX_TX_VAL_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E
 */
typedef struct {
    /**
     * @brief [out] maxTxVal_ARR - Maximum LPI Exit time per speed
     */
    UINT_16     maxTxVal_ARR[MPD_EEE_SPEED_MAX_E];
} PRV_MPD_EEE_MAX_TX_VAL_PARAMS_STC;

/** @internal
 * @struct MPD_EEE_ENABLE_MODE_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E
 */
typedef struct {
    /**
     * @brief [out] enableMode - EEE enable mode
     */
    MPD_EEE_ENABLE_MODE_ENT  enableMode;
} PRV_MPD_EEE_ENABLE_MODE_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E
 */
typedef struct {
    /**
     * @brief [in] phy_validStatus - PHY valid link status
     */
    BOOLEAN     phyValidStatus;
} PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_PORT_LANE_BMP_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_LANE_BMP_E
 */
typedef struct {
    UINT_32 hostSideLanesBmp;
    UINT_32 lineSideLanesBmp;

} PRV_MPD_PORT_LANE_BMP_STC;

/** @internal
 * @struct  PRV_MPD_PAGE_SELECT_PARAMS_STC
 * @brief Used for PHYs which support page select
 * - PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E
 * @warning - internal, do not use
 */
typedef struct {
    UINT_16   page;
    UINT_16   prevPage;
    BOOLEAN   readPrevPage;
} PRV_MPD_PAGE_SELECT_PARAMS_STC;

/** @internal
 * @struct  PRV_MPD_MEDIA_SELECT_PARAMS_STC
 * @brief Used for Combo PHYs which support media select
 * - PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E
 */
typedef struct {
    UINT_16  value;
} PRV_MPD_MEDIA_SELECT_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_GREEN_POW_CONSUMPTION_PARAMS_STC
 * @brief Used for
 * - \ref PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E
 */
typedef struct {
    /**
     * @brief [in] energyDetetct - ED status
     */
    MPD_GREEN_SET_ENT   energyDetetct;
    /**
     * @brief [in] shortReach - SR Status
     */
    MPD_GREEN_SET_ENT   shortReach;
    /**
     * @brief [in] portUp - is port up
     */
    BOOLEAN             portUp;
    /**
     * @brief [in] portSpeed - port speed
     */
    MPD_SPEED_ENT       portSpeed;
    /**
     * @brief [in] getMax - if TRUE - return max power consumption
     */
    BOOLEAN             getMax;
    /**
     * @brief [out] greenConsumption - PHY power consumption in mW
     */
    UINT_32             greenConsumption;
} PRV_MPD_GREEN_POW_CONSUMPTION_PARAMS_STC;


/**
 * @internal
 * @struct PRV_MPD_I2C_READ_PARAMS_STC
 * @brief Used for
 * - \ref PRV_MPD_OP_CODE_GET_I2C_READ_E
 */
typedef struct {
    /**
     * @brief [in] address - I2C address
     */
    UINT_8 address;
    /**
     * @brief [in] offset - Register offset
     */
    UINT_8 offset;
    /**
     * @brief [in] length - Read length (1..4)
     */
    UINT_8 length;
    /**
     * @brief [out] value - Read value
     */
    UINT_8 value[4];
} PRV_MPD_I2C_READ_PARAMS_STC;

/**
 * @internal
 * @struct PRV_MPD_GREEN_READINESS_PARAMS_STC
 * @brief Used for
 * - \ref PRV_MPD_OP_CODE_GET_GREEN_READINESS_E
 */
typedef struct {
    /**
     * @brief [in] type - Green readiness (support) query type
     */
    MPD_GREEN_READINESS_TYPE_ENT type;
    /**
     * @brief [out] readiness - Support type
     */
    MPD_GREEN_READINESS_ENT      readiness;
    /**
     * @brief [out] srSpeeds - Bit map of speeds that the PHY support SR
     */
    MPD_SPEED_CAPABILITY_TYP     srSpeeds;
} PRV_MPD_GREEN_READINESS_PARAMS_STC;

typedef struct {
    /**
    * @brief [out] vct tdr test results (status | cable len)
    */
    MPD_VCT_TDR_CABLE_STC vctTdrCable;
} PRV_MPD_VCT_TDR_COLLECT_RESULTS_PARAMS_STC;

typedef struct {
    /**
    * @brief [out] vct dsp test results (cable len)
    */
    MPD_VCT_CABLE_EXTENDED_STATUS_STC extendedCableStatus;
} PRV_MPD_VCT_DSP_GET_RESULTS_PARAMS_STC;


/* ******************* END INTERNAL ****************/

/**
* @addtogroup API API
* @{MPD API, all information required in order to perform PHY operations*
 * @union MPD_OPERATIONS_PARAMS_UNT
 * @brief Data union for logical operations
*/

typedef union {
    /**
     * - \ref MPD_OP_CODE_GET_MDIX_E
     * - \ref MPD_OP_CODE_SET_MDIX_E
     * - \ref MPD_OP_CODE_GET_MDIX_ADMIN_E
     */
    MPD_MDIX_MODE_PARAMS_STC                    phyMdix;
    /**
     * - \ref MPD_OP_CODE_SET_AUTONEG_E
     * - \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E
     */
    MPD_AUTONEG_PARAMS_STC                      phyAutoneg;
    /**
     * - \ref MPD_OP_CODE_SET_DUPLEX_MODE_E
     */
    MPD_DUPLEX_MODE_PARAMS_STC                  phyDuplex;
    /**
     * - \ref MPD_OP_CODE_SET_SPEED_E
     */
    MPD_SPEED_PARAMS_STC                        phySpeed;
    /**
     * - \ref MPD_OP_CODE_SET_VCT_TEST_E
     */
    MPD_VCT_PARAMS_STC                          phyVct;
    /**
     * - \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E
     */
    MPD_EXT_VCT_PARAMS_STC                      phyExtVct;
    /**
     * - \ref MPD_OP_CODE_GET_CABLE_LEN_E
     */
    MPD_CABLE_LEN_PARAMS_STC                    phyCableLen;
    /**
     * - \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E
     */
    MPD_PHY_DISABLE_PARAMS_STC                  phyDisable;
    /**
     * - \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E
     */
    MPD_AUTONEG_CAPABILITIES_PARAMS_STC         phyRemoteAutoneg;
    /**
     * - \ref MPD_OP_CODE_GET_AUTONEG_SUPPORT_E
     */
    MPD_AUTONEG_CAPABILITIES_PARAMS_STC         PhyAutonegCapabilities;
    /**
     * - \ref MPD_OP_CODE_SET_ADVERTISE_FC_E
     */
    MPD_ADVERTISE_FC_PARAMS_STC                 phyFc;
    /**
     * - \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E
     */
    MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC        phyLinkPartnerFc;
    /**
     * - \ref MPD_OP_CODE_SET_POWER_MODULES_E
     */
    MPD_POWER_MODULES_PARAMS_STC                phyPowerModules;
    /**
     * - \ref MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E
     */
    MPD_CABLE_LEN_NO_RANGE_PARAMS_STC           phyCableLenNoRange;
    /**
     * - \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E
     */
    MPD_KIND_AND_MEDIA_PARAMS_STC               phyKindAndMedia;
    /**
     * - \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E
     */
    MPD_PRESENT_NOTIFICATION_PARAMS_STC         phySfpPresentNotification;
    /**
     * - \ref MPD_OP_CODE_SET_MDIO_ACCESS_E
     */
    MPD_MDIO_ACCESS_PARAMS_STC                  phyMdioAccess;
    /**
     * - \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E
     */
    MPD_EEE_ADVERTISE_PARAMS_STC                phyEeeAdvertize;
    /**
     * - \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E
     */
    MPD_EEE_MASTER_ENABLE_PARAMS_STC            phyEeeMasterEnable;
    /**
     * - \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E
     */
    MPD_EEE_CAPABILITIES_PARAMS_STC             phyEeeCapabilities;
    /**
     * - \ref MPD_OP_CODE_GET_EEE_STATUS_E
     */
    MPD_EEE_STATUS_PARAMS_STC                   phyEeeStatus;
    /**
     * - \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E
     * - \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E
     */
    MPD_EEE_LPI_TIME_PARAMS_STC                 phyEeeLpiTime;
    /**
     * - \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E
     */
    MPD_INTERNAL_OPER_STATUS_STC                phyInternalOperStatus;
    /**
     * - \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E
     */
    MPD_VCT_CAPABILITY_PARAMS_STC               phyVctCapab;
    /**
     * - \ref MPD_OP_CODE_GET_DTE_STATUS_E
     * - \ref MPD_OP_CODE_SET_DTE_E
     */
    MPD_DTE_STATUS_PARAMS_STC                   phyDteStatus;
    /**
     * - \ref MPD_OP_CODE_GET_TEMPERATURE_E
     */
    MPD_TEMPERATURE_PARAMS_STC                  phyTemperature;
    /**
     * - \ref MPD_OP_CODE_GET_REVISION_E
     */
    MPD_REVISION_PARAMS_STC         phyRevision;
    /**
     * - \ref MPD_OP_CODE_SET_SPEED_EXT_E
     * - \ref MPD_OP_CODE_GET_SPEED_EXT_E
     */
    MPD_SPEED_EXT_PARAMS_STC                    phySpeedExt;
    /**
     * - \ref MPD_OP_CODE_SET_SERDES_TUNE_E
     */
    MPD_SERDES_TUNE_PARAMS_STC                  phyTune;
    /**
     * - \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E
     */
    MPD_SERDES_LANE_POLARITY_PARAMS_STC         phySerdesPolarity;
    /**
     * - \ref MPD_OP_PHY_CODE_SET_PTP_INIT_E
     */
    MPD_PTP_INIT_PARAMS_STC                     phyPtpInit;
    /**
     * - \ref MPD_OP_PHY_CODE_GET_PTP_TOD_CAPTURE_VALUE_E
     */
    MPD_PTP_CAPTURED_FRC_INFO_PARAMS_STC        phyPtpCapturedFrc;
    /**
     * - \ref MPD_OP_CODE_GET_PTP_TSQ_E
     */
    MPD_PTP_TSQ_INFO_PARAMS_STC                 phyPtpTsq;
    /**
     * - \ref MPD_OP_CODE_SEND_IDLE_TO_HOST_E
     */
    MPD_SEND_IDLE_TO_HOST_PARAMS_STC            phySendIdleToHost;
    /**
     * @internal
     * @brief - internal operations, implemented per PHY
     *  but we expect these to be called only from withing MPD
     */
    union {
        /**
         * @internal
         * @brief - Peform PHY initialization sequence.
         * Called per port
         */
        PRV_MPD_INITIALIZE_PHY_STC                  phyInit;
        PRV_MPD_FIBER_MEDIA_PARAMS_STC              phyFiberParams;
        PRV_MPD_VCT_OFFSET_PARAMS_STC               phyVctOffset;
        PRV_MPD_LOOP_BACK_STC                       phyLoopback;
        PRV_MPD_ERRATAS_PARAMS_STC                  phyErrata;
        PRV_MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC    phyFastLinkDown;
        PRV_MPD_EEE_MAX_TX_VAL_PARAMS_STC           phyEeeMaxTxVal;
        PRV_MPD_EEE_ENABLE_MODE_PARAMS_STC          phyEeeEnableMode;
        PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC      phyLinkStatus;
        PRV_MPD_PORT_LANE_BMP_STC                   phyLaneBmp;
        PRV_MPD_PAGE_SELECT_PARAMS_STC              phyPageSelect;
        PRV_MPD_MEDIA_SELECT_PARAMS_STC             phyMediaSelect;
		PRV_MPD_GREEN_POW_CONSUMPTION_PARAMS_STC    phyPowerConsumptions;
		PRV_MPD_I2C_READ_PARAMS_STC                 phyI2cRead;
		PRV_MPD_GREEN_READINESS_PARAMS_STC          phyGreen;
        PRV_MPD_VCT_TDR_COLLECT_RESULTS_PARAMS_STC  phyVctTdrCollectResults;
        PRV_MPD_VCT_DSP_GET_RESULTS_PARAMS_STC      phyVctDspGetResults;
    } internal;
} MPD_OPERATIONS_PARAMS_UNT;


/**
 * @brief Perform PHY operation on port
 * @param [in]      rel_ifIndex - The port on which to perform operation
 * @param [in]      op          - The logical operation
 * @param [in,out]  params_PTR  - Data for operation (Set & Get)
 * @return MPD_RESULT_ENT
 */

extern MPD_RESULT_ENT mpdPerformPhyOperation(
    IN UINT_32                      rel_ifIndex,
    IN MPD_OP_CODE_ENT              op,
    INOUT MPD_OPERATIONS_PARAMS_UNT * params_PTR
);

/**
 * @brief Perform PHY operation on PHY
 * @param [in]      phyNumber 	- The PHY on which to perform operation
 * @param [in]      op          - The logical operation
 * @param [in,out]  params_PTR  - Data for operation (Set & Get)
 * @return MPD_RESULT_ENT
 */

extern MPD_RESULT_ENT mpdPerformOperationOnPhy(
    IN UINT_32 phyNumber,
    IN MPD_OP_CODE_ENT op,
    INOUT MPD_OPERATIONS_PARAMS_UNT *params_PTR
);

/**
 * @brief Get MTD port object \br
 * Allows application to directly call MTD \br
 * Supported only on MTD PHYs
 * @param [in]      rel_ifIndex 		- The port
 * @param [out]     port_object_PTR 	- Pointer to object of type (MTD_DEV **)
 * @note This function returns pointer to actual Object, not a copy
 * @return MPD_RESULT_ENT
 *****************************************************************************/
extern MPD_RESULT_ENT mpdMtdGetPortObject(
	IN UINT_32 		rel_ifIndex,
	OUT void	** port_object_PTR,
	OUT UINT_8	 * mdio_address
);

/**
 *
 * @brief Clear and free all MPD allocated resources
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdDriverDestroy (
	void
);

/** @}*/

#endif /* MPD_API_H_ */
