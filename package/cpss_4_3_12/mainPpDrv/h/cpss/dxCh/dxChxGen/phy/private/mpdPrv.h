/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdPrv.h
 * @brief private (MPD internal) types and declarations
 *
 */

#ifndef PRV_MPD_H_
#define PRV_MPD_H_

#include <cpss/dxCh/dxChxGen/phy/private/mpdPrefix.h>

#include <cpss/dxCh/dxChxGen/phy/private/mpdTypes.h>
#include <cpss/dxCh/dxChxGen/phy/private/mpd.h>
#include <cpss/dxCh/dxChxGen/phy/private/mpdApi.h>


#define MPD_MAXSTR_CNS 1000
#define PRV_MPD_TO_STRING_MAC(_val) #_val

typedef unsigned int    GT_STATUS;
#ifndef PHY_SIMULATION
#define PRV_MPD_CHECK_RETURN_STATUS_MAC(_Status, _rel_ifIndex)\
        PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC(_Status, PRV_MPD_DEBUG_FUNC_NAME_MAC(),_rel_ifIndex);

#define PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC(_Status, _calling_func, _rel_ifIndex)\
    if (_Status != MPD_OK_E) { \
        PRV_MPD_HANDLE_FAILURE_INTERNAL_MAC(_rel_ifIndex, MPD_ERROR_SEVERITY_ERROR_E, _calling_func, PRV_MPD_TO_STRING_MAC(_Status));\
        return _Status; \
    }
#else
#define PRV_MPD_CHECK_RETURN_STATUS_MAC(_Status, _rel_ifIndex)\
    MPD_UNUSED_PARAM(_Status)

#define PRV_MPD_CHECK_RETURN_STATUS_FULL_MAC(_Status, _calling_func, _rel_ifIndex)\
    _Status;\
    MPD_UNUSED_PARAM(_calling_func);\
    MPD_UNUSED_PARAM(_rel_ifIndex)
#endif

/* ******** PORT LIST RELATED *********/

#define PRV_MPD_PORT_LIST_REL_IFINDEX_IS_LEGAL_MAC(relIf_)\
    (((relIf_ > 0) && (relIf_ < ((PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS) * 32)))?TRUE:FALSE)

#define PRV_MPD_PORT_LIST_GET_SHIFT_AND_WORD_MAC(relIf_, shift_, word_)\
    word_ = ((relIf_ - 1) / 32);\
    shift_ = ((relIf_-1) % 32)

#define PRV_MPD_PORT_LIST_WORD_AND_BIT_TO_REL_IFINDEX_MAC(word_, bit_)\
    ((word_ * 32) + bit_ + 1)

/* number of words in ports bitmap  */
#define PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS   (2)
#define PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS  ((MPD_MAX_PORT_NUMBER_CNS + 31)/32)

#define PRV_MPD_VALID_PHY_TYPE(_phy_type)   (_phy_type < MPD_TYPE_NUM_OF_TYPES_E)

#define PRV_MPD_MAX_NUM_OF_PHY_CNS (31)

#define PRV_MPD_MAX_MDIO_ADRESS_CNS (31)

/* used by the xml parser */
#define MPD_FIELD_HAS_VALUE_CNS 1

#define PRV_MPD_XML_TAG_PREFIX_CNS   "mpd"

#define PRV_MPD_PHY_READY_MAX_NUM_RETRIES_CNS (50)
#define PRV_MPD_PHY_NOT_READY_VALUE_CNS       (0xFFFF)
#define PRV_MPD_PHY_NOT_READY_VALUE2_CNS       (0)

typedef struct {
    UINT_32 portsList[PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS];
} PRV_MPD_PORT_LIST_STC;

typedef PRV_MPD_PORT_LIST_STC PRV_MPD_PORT_LIST_TYP;


/* ********* PORT DB *************/
typedef enum {
    PRV_MPD_MAC_ON_PHY_NO_BYPASS_E = FALSE,
    PRV_MPD_MAC_ON_PHY_BYPASS_E    = TRUE,
    PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E
} PRV_MPD_MAC_ON_PHY_ENT;

typedef struct {
    MPD_PHY_USX_TYPE_ENT    usxType;
    UINT_16                 masterMacType;
    UINT_16                 slaveMacType;
} PRV_MPD_USX_MAC_TYPE_STC;

typedef struct {
    /** @brief Real present mode. */
    BOOLEAN                         sfpPresent;
    /** @brief Is combo media type changed. */
    BOOLEAN                         comboMediaTypeChanged;
    /** @brief The configured speed on port. update in set speed operation. */
    MPD_SPEED_ENT                   speed;
    /** @brief The configured duplex mode on port. update in set duplex operation. */
    MPD_DUPLEX_ADMIN_ENT            duplex;
    /** @brief Media type on combo/sfp. */
    MPD_OP_MODE_ENT                 opMode;
    /** @brief mac on phy bypass state. */
    PRV_MPD_MAC_ON_PHY_ENT          macOnPhyState;
    /** @brief Vct information. */
    MPD_PHY_VCT_INFO_STC            vctInfo;
    /** @brief Port Admin mode. */
    MPD_PORT_ADMIN_ENT              adminMode;
    /** @brief AMD disabled the T unit, SW reset was not applied - need to apply in present notification. */
    BOOLEAN                         softwareResetRequired;
    /** @brief PHY FW information. This information may be shared among several ports,
        (for example, ports in same PHY, or same PHY type and and same bus).
        This field is updated internally by MPD. */
    MPD_PHY_FW_PARAMETERS_STC * phyFw_PTR;
    /** @brief Part of PHY fw flash download mode to know which port need to be downloaded. */
    BOOLEAN                     isRepresentative;
    /** @brief since the allocation is mutual to multiple ports, we need to free it once in destroy allocation part */
    BOOLEAN                     traceMtdAllocations;
    /** @brief A flag, using in case of permanent configuration to make sure
        configuration done only once, for each side (host & line [2]). */
    BOOLEAN                     isNotImmidiateTxConfigure_ARR[2];
} PRV_MPD_RUNNING_DB_STC;

/* port entry */
typedef struct {
    UINT_32                         rel_ifIndex;
    MPD_PORT_INIT_DB_STC        *   initData_PTR;
    PRV_MPD_RUNNING_DB_STC      *   runningData_PTR;
} PRV_MPD_PORT_HASH_ENTRY_STC;

typedef struct {
    UINT_32                     numOfMembers;   /* number of entries which hold valid data in porthash*/
    BOOLEAN                     infoReceivedFromXml; /* did we received the ports information from XML or not */
    PRV_MPD_PORT_LIST_TYP       validPorts;     /* indicates which rel_ifIndexes hold valid data (helps speeding up search in get_next_rel_ifIndex */
    MPD_FW_DOWNLOAD_TYPE_ENT    fwDownloadType_ARR[MPD_TYPE_NUM_OF_TYPES_E];    /* assumption - Download type detemined by phy type */
    PRV_MPD_PORT_HASH_ENTRY_STC entries[MPD_MAX_PORT_NUMBER_CNS]; /* array of entries */
} PRV_MPD_GLOBAL_DB_STC;

typedef struct {
    PRV_MPD_PORT_HASH_ENTRY_STC *representativePortEntry_PTR;
    BOOLEAN ptpEnabled;
} PRV_MPD_PHY_DB_STC;

typedef void*  MPD_OPERATION_FUN;

typedef MPD_RESULT_ENT MPD_PORT_OPERATIONS_FUN(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR
    /*     OUTPUTS:            */
);

typedef MPD_RESULT_ENT MPD_PHY_OPERATIONS_FUN(
    /*     INPUTS:             */
    UINT_32   phyNumber,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR
    /*     OUTPUTS:            */
);

typedef union {
    MPD_PORT_OPERATIONS_FUN     *portOperation_PTR;
    MPD_PHY_OPERATIONS_FUN      *phyOperation_PTR;
} PRV_MPD_OPERATION_UNT;


typedef struct {
    PRV_MPD_OPERATION_UNT     operations;
    char                    * funcName_PTR;
} PRV_MPD_FUNC_INFO_ENTRY_STC;


#define PRV_MPD_OP_TO_TEXT_MAC(_phyType, _opType) \
    ((UINT_32)_phyType >= (UINT_32)MPD_TYPE_NUM_OF_TYPES_E)? "Illegal PHY":\
            ((UINT_32)_opType >= (UINT_32)PRV_MPD_NUM_OF_OPS_E)? "Illehal OP":\
                    PRV_MPD_NONSHARED_GLBVAR_MPD_FUNC_INFO_ACCESS_MAC()[_phyType][_opType].funcName_PTR

#define IS_PORT_OPERATION(_op) \
    ((UINT_32)_op < MPD_OP_LAST_PORT_OP_E) || ((UINT_32)_op >= MPD_OP_LAST_PHY_OP_E)

#define IS_PHY_OPERATION(_op) \
((UINT_32)op >= MPD_OP_LAST_PORT_OP_E) && ((UINT_32)op < MPD_OP_LAST_PHY_OP_E)

/**
 * @internal mpdDriverInitDb function
 * @endinternal
 *
 * @brief   bind function
 *          connect betweem PHY type & logical operation to PHY (type specific) implementation
 *
 * @param [in ]     phyType         - The PHY type
 * @param [in ]     opType          - The logical operation
 * @param [in ]     func_PTR        - The implementation of <opType> for <phyType>
 * @param [in ]     func_text_PTR   - Function name, for debug (may be NULL)
 *
 * @return MPD_RESULT_ENT
 */
extern BOOLEAN prvMpdBind(
    IN MPD_TYPE_ENT           phyType,
    IN UINT_32                opType,    /*MPD_OP_CODE_ENT*/
    IN MPD_OPERATION_FUN      func_PTR,
    IN char                 * func_text_PTR
);

extern MPD_RESULT_ENT prvMpdPtpTaiRegisterWrite(
    IN UINT_32 phyNumber,
    IN UINT_8 reg,
    IN UINT_32 mask,
    IN UINT_32 data
);

extern MPD_RESULT_ENT prvMpdPtpTaiRegisterRead(
    IN UINT_32 phyNumber,
    IN UINT_8  reg,
    OUT UINT_32 *data_PTR
);

extern MPD_RESULT_ENT prvMpdBindInitFunc(
    /*     INPUTS:             */
    MPD_TYPE_ENT phyType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern char * prvMpdGetPhyOperationName(
    /*     INPUTS:             */
    IN PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    IN UINT_32 op    /* MPD_OP_CODE_ENT */
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdReadLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_32    * data_PTR
);

extern MPD_RESULT_ENT prvMpdWriteLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_16 address,
    UINT_32 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdGetTxSerdesParams(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    MPD_PHY_HOST_LINE_ENT hostOrLineSide,
    MPD_SPEED_ENT  phySpeed,
    MPD_SERDES_TUNE_PARAMS_STC serdesTuneParams,
    /*     INPUTS / OUTPUTS:   */
    MPD_SERDES_TX_CONFIG_UNT * phy_tx_params_PTR
    /*     OUTPUTS: */
);

extern MPD_RESULT_ENT prvMpdInitPortHw(
    UINT_32     rel_ifIndex
);

typedef struct {
    UINT_16 page;
    UINT_16 reg;
    UINT_16 bits_to_set;
} PRV_MPD_LOOPBACK_PHY_PROFILE_STC;

#define  MPD_ID_HIGH_ALASKA_88E1340_CNS      0x0141

#define  MPD_ID_LOW_ALASKA_88E1340_CNS       0x0C90

#define  MPD_ID_MASK_ALASKA_88E1145_CNS      0xFFFFFFF0

/* alaska: Phy */
#define  PRV_MPD_CTRLREG_OFFSET_CNS             0
#define  PRV_MPD_CTRLREG_FULL_DPLX_CNS          0x0100
#define  PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS    0x1000
#define  PRV_MPD_CTRLREG_10BT_CNS               0x0000
#define  PRV_MPD_CTRLREG_100BT_CNS              0x2000
#define  PRV_MPD_CTRLREG_SPEED_1000BT_CNS       0x0040
#define  PRV_MPD_CTRLREG_RESET_BIT_CNS          0x8000
#define  PRV_MPD_CTRLREG_INITLOOP_BIT_CNS       0x4000
#define  PRV_MPD_CTRLREG_POWER_DOWN_CNS         0x0800   /* bit 0.11 */
#define  PRV_MPD_PORT_CNTRL_REG_SPEED_10M_CNS   0x0000
#define  PRV_MPD_PORT_CNTRL_REG_SPEED_100M_CNS  0x2000
#define  PRV_MPD_CTRLREG_EN_AUTO_NEG_CNS        0x1000
#define  PRV_MPD_CTRLREG_AUTO_NEG_CNS           0x1200
#define  PRV_MPD_CTRLREG_RESATRT_AUTO_NEG_CNS   0x0200
#define  PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS   0x0040

#define  PRV_MPD_COPPER_GIG_PHY_IDENTIFIER_CNS   0x3

#define  PRV_MPD_PAGE_SELECT_ADDRESS_CNS        22
#define  PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS    0x00
#define  PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS    0x01
#define  PRV_MPD_PAGE2_ADDRESSREG_OFFSET_CNS    0x02

#define  PRV_MPD_CTRLREG1_OFFSET_CNS             1
#define  PRV_MPD_COPPER_STATUS_REGISTER_CNS      1
#define  PRV_MPD_COPPER_AUTONEG_COMPLETE_CNS     0x20

#define PRV_MPD_DIG_COM_CONTROL_REGISTER_CNS (28)
#define PRV_MPD_RESET_AND_CLOCK_CASCADING_CONTROL_REGISTER_CNS (0)

#define  PRV_MPD_PTP_TAI_INDIRECT_PAGE_CNS          (26)
#define  PRV_MPD_PTP_TAI_INDIRECT_READ_ADDRESS_CNS  (18)
#define  PRV_MPD_PTP_TAI_INDIRECT_WRITE_ADDRESS_CNS (19)
#define  PRV_MPD_PTP_TAI_INDIRECT_DATA_LOW_CNS      (20)
#define  PRV_MPD_PTP_TAI_INDIRECT_DATA_HIGH_CNS     (21)

#define  PRV_MPD_PTP_TAI_PCH_COMMON_CONTROL_REG_CNS  (0)
#define  PRV_MPD_PTP_TAI_PCH_CONTROL_REG_CNS         (6)

#define  PRV_MPD_PTP_TAI_HIGH_SPEED_IO_CAL_REG_CNS   (2)
#define  PRV_MPD_PTP_TAI_REF_CLOCK_REG_CNS           (9)

#define  PRV_MPD_PTP_ETSB_ACCESS_CONTROL_REG_CNS     (23)
#define  PRV_MPD_PTP_ETSB_READ_DATA_0_REG_CNS        (28)
#define  PRV_MPD_PTP_ETSB_READ_DATA_1_REG_CNS        (29)
#define  PRV_MPD_PTP_ETSB_READ_DATA_2_REG_CNS        (30)
#define  PRV_MPD_PTP_ETSB_READ_DATA_3_REG_CNS        (31)
#define  PRV_MPD_PTP_ETSB_DATA_VALID_BIT_CNS     (0x1000)

#define  PRV_MPD_PTP_ETSB_NUM_OF_ENTRIES_CNS         (32)

#define  PRV_MPD_PTP_TAI_WRITE_ALL_MASK_CNS         (0xFFFFFFFF)

#define  PRV_MPD_PTP_TOD_CAPTURED_0_VALID_CNS     (1)
#define  PRV_MPD_PTP_TOD_CAPTURED_1_VALID_CNS     (2)


#define  PRV_MPD_LINK_PARTNER_ABILITY_REG_CNS          5
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100F_CNS  0x0100
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100H_CNS  0x0080
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10F_CNS   0x0040
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10H_CNS   0x0020
#define  PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000H_CNS  0x0040
#define  PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000F_CNS  0x0020

#define  PRV_MPD_1000_BASE_T_STATUS_REG_CNS            10
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000F_CNS 0x0800
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000H_CNS 0x0400

/* Phy Control Register */
#define  PRV_MPD_SPECIFIC_CONTROL_REG_CNS       16
#define  PRV_MPD_SPECIFIC_CONTROL_REG_3_CNS     26
/* Phy Specific Control Register -> MDI/MDIX Crossover */
#define  PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS    0x0060
#define  PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS   0x0000
#define  PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS  0x0020
#define  PRV_MPD_AUTOMATIC_CROSSOVER_CNS        0x0060
/*values for the 3081/3083 phys */
#define  PRV_MPD_FE_MDI_CROSSOVER_MODE_MASK_CNS        0x0030
#define  PRV_MPD_FE_MANUAL_MDIX_CONFIGURATION_CNS      0x0000
#define  PRV_MPD_FE_MANUAL_MDI_CONFIGURATION_CNS       0x0010
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_10_CNS         0x0020
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_11_CNS         0x0030
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_MAIN_BIT_CNS   0x0020

#define  PRV_MPD_SPECIFIC_STATUS_REG_CNS        17
/* PHY_Specific_Status_Reg */
#define  PRV_MPD_DTE_POWER_STATUS_STATUS_CNS    0x0004
#define  PRV_MPD_MDI_CROSSOVER_STATUS_CNS       0x0040
#define  PRV_MPD_PORT_STATUS_LINK_MASK_CNS      0x0400
#define  PRV_MPD_PORT_STATUS_DPLX_MOD_MASK_CNS  0x2000

#define  PRV_MPD_PORT_STATUS_SPEED_AND_DUPLEX_RESOLVED_CNS 0x0800
#define  PRV_MPD_PORT_STATUS_SPEED_10M_CNS      0x0000
#define  PRV_MPD_PORT_STATUS_SPEED_100M_CNS     0x4000
#define  PRV_MPD_PORT_STATUS_SPEED_1000M_CNS    0x8000

/* Extended PHY Specific Control*/
#define  PRV_MPD_AUTNEGADVER_OFFSET_CNS         0x0004
#define  PRV_MPD_ADVERTISE_BITS_CNS             0x01e1
#define  PRV_MPD_ADVERTISE_10_100_CNS           0x01e0
#define  PRV_MPD_FIBER_ADVERTISE_1000H_CNS      0x0040 /* 1000 half  */
#define  PRV_MPD_FIBER_ADVERTISE_CNS            0x0020 /* 1000 full only */
#define  PRV_MPD_ADVERTISE_100_T_FULL_CNS       0x0100
#define  PRV_MPD_ADVERTISE_100_T_HALF_CNS       0x0080
#define  PRV_MPD_ADVERTISE_10_T_FULL_CNS        0x0040
#define  PRV_MPD_ADVERTISE_10_T_HALF_CNS        0x0020

#define  PRV_MPD_CTRLREG_AUTONEG_MASK_CNS       0x1000


#define  PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS  0x9
#define  PRV_MPD_1000BASE_AN_PREFER_MASK_CNS    0x1700
#define  PRV_MPD_ADVERTISE_1000_T_MASK_CNS      0x0300
#define  PRV_MPD_1000BASE_AN_MANUALCONFIG_CNS   0x1000
#define  PRV_MPD_1000BASE_AN_PREFERMASTER_CNS   0x0400
#define  PRV_MPD_ADVERTISE_1000_T_FULL_CNS      0x0200
#define  PRV_MPD_ADVERTISE_1000_T_HALF_CNS      0x0100

#define  PRV_MPD_LINK_UP_CNS                    0xf7ff
#define  PRV_MPD_LINK_DOWN_CNS                  0x0800

/* Giga phy uses register 26 and 27 for the vct activation and results */
#define  PRV_MPD_VCT_2_PAIR_CABLE_CNS                     2
#define  PRV_MPD_VCT_4_PAIR_CABLE_CNS                     4

/*vct status register */
#define  PRV_MPD_VCT_STATUS1_REG_CNS    23
#define  PRV_MPD_VCT_STATUS2_REG_CNS    16

/* Phy 1240 */
#define  PRV_MPD_88E1240_REVISION_S0_CNS                      0x0E30
#define  PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS              22
#define  PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS               0x300

/* Phy 151x */
#define  PRV_MPD_88E151X_PAGE_LED_CTRL_CNS                    0x3
#define  PRV_MPD_88E151X_LED_CTRL_REG_CNS                     16

/* green ethernet energy detect mode */
/*#define  PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS   0x0300* see line 264*/
#define  PRV_MPD_88E1240_ENERGY_DETECT_ON_CNS          0x0300
#define  PRV_MPD_88E1240_ENERGY_DETECT_OFF_CNS         0x0000
#define  PRV_MPD_SR_SPECIFIC_CONTROL_REG_CNS    24
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_CNS   23
#define  PRV_MPD_SR_SPECIFIC_CONTROL2_REG_CNS   25
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_SET_VALUE_CNS    0x2002
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_GET_VALUE_CNS    0x1002
#define  PRV_MPD_88E1240_SHORT_REACH_ON_MASK_CNS           0x0100

/* green ethernet mac interface effect on low power mode */
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS   0x0008
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_OFF_CNS    0x0008
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_ON_CNS     0x0000


#define PRV_MPD_MDIO_WRITE_ALL_MASK_CNS                  0xFFFF
#define PRV_MPD_IGNORE_PAGE_CNS                         0xFFFF
#define PRV_MPD_PORT_GROUP_UNAWARE_MODE_CNS             0xFFFFFFFF

/*Phy 3083 */

/* green ethernet energy detect mode */
#define  PRV_MPD_88E3083_ENERGY_DETECT_MASK_CNS               0x4000
#define  PRV_MPD_88E3083_ENERGY_DETECT_ON_CNS                 0x4000
#define  PRV_MPD_88E3083_ENERGY_DETECT_OFF_CNS                0x0000

/* EEE on phy 88E1540 */
#define  PRV_MPD_88E1540_BUFF_SIZE_KBITS_CNS                160
#define  PRV_MPD_88E1540_MAX_EXIT_LPI_TIMER_VAL             255

/* phy 1543 general control register */
#define  PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS          18

#define  PRV_MPD_88E1543_REVISION_A0_CNS                       0x0EA1
#define  PRV_MPD_88E1543_REVISION_A1_CNS                       0x0EA2

/* this values refer to phy mode configuration page 20 register 6 bits [0..2]
*  see Marvell\AE Alaska\AE 88E1340/88E1322 Auto-Media Detect - MV-S301681-00A 4.1.1
*/
#define PRV_MPD_MODE_SGMII_TO_100_FX_CNS                               3
#define PRV_MPD_MODE_SGMII_TO_AUTO_MEDIA_COPPER_1000_BASEX_CNS         7
#define PRV_MPD_MODE_MASK_CNS                                          7
#define PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS                       20
#define PRV_MPD_PAGE_0_COPPER_CONTROL_CNS                              0
#define PRV_MPD_PAGE_1_FIBER_CONTROL_CNS                               1
#define PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS                  0
#define PRV_MPD_GENERAL_STATUS_REGISTER_PAGE_0_1_CNS                  1

/* ****** 3140 (SolarFlare) ***************/
#define PRV_MPD_3140_EEE_LPI_EXIT_TIME_CNS                              5 /* equal to NETP_green_eee_min_tx_10GBaseT_CNS --> Standard: 4.48  uSec (case 2) */


#ifdef PHY_SIMULATION
#define prvMpd_autoneg_num_of_retries_CNS                1
#else
#define prvMpd_autoneg_num_of_retries_CNS               10
#endif

/* page use to access linkCrypt registers*/
#define PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS 16
/* register to which indirect read address is specified */
#define PRV_MPD_LINKCRYPT_READ_ADDRESS_REG_CNS    0
/* register to which indirect write address is specified */
#define PRV_MPD_LINKCRYPT_WRITE_ADDRESS_REG_CNS   1
/* register to which data low is written/read from */
#define PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS        2
/* register to which data high is written/read from */
#define PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS       3

/* page use to access XMDIO registers*/
#define PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS      0
/* register to which the function (address/data) and to which device */
#define PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS   13
/* register to which the address/data is written to */
#define PRV_MPD_MMD_ADDRESS_DATA_REG_CNS          14

#define PRV_MPD_MMD_DEVICE_MASK_CNS               0x1F
#define PRV_MPD_MMD_FUNCTION_ADDRESS_CNS          0x0000
#define PRV_MPD_MMD_FUNCTION_DATA_CNS             0x4000

#define PRV_MPD_MAX_NUM_OF_SLICES_CNS             4

#define PRV_MPD_1540_SLICE_0_OFFSET_CNS           0x000   /*/< Port 0 registers/memory */
#define PRV_MPD_1540_SLICE_1_OFFSET_CNS           0x800   /*/< Port 1 registers/memory */
#define PRV_MPD_1540_SLICE_2_OFFSET_CNS           0x1000  /*/< Port 2 registers/memory */
#define PRV_MPD_1540_SLICE_3_OFFSET_CNS           0x1800  /*/< Port 3 registers/memory */

#define PRV_MPD_1540_BYPASS_DISABLE_BIT_CNS       0x2000 /* (1<<13) */

/* Definitions for BIST Status Register 1.C00C of 88E32x0/88E33x0*/
#define PRV_MPD_BIST_ABNOMINAL_RESET_POS        (0)
#define PRV_MPD_BIST_CHECKSUM_EXCEEDED_POS  (1)
#define PRV_MPD_BIST_PMA_FAILED_POS             (2)
#define PRV_MPD_BIST_ENX_FAILED_POS             (3)
#define PRV_MPD_BIST_RTOS_FAILED_POS            (4)
#define PRV_MPD_BIST_SW_ERROR_POS           (5)
#define PRV_MPD_BIST_FW_ECC_ERROR_POS       (6)
#define PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_POS (7)
#define PRV_MPD_BIST_RESERVED_POS           (8)
#define PRV_MPD_BIST_DEVICE_NOT_SUPORTED_POS  (9)

#define PRV_MPD_BIST_ABNOMINAL_RESET_MASK           (0x1<<PRV_MPD_BIST_ABNOMINAL_RESET_POS)
#define PRV_MPD_BIST_CHECKSUM_EXCEEDED_MASK         (0x1<<PRV_MPD_BIST_CHECKSUM_EXCEEDED_POS)
#define PRV_MPD_BIST_PMA_FAILED_MASK                (0x1<<PRV_MPD_BIST_PMA_FAILED_POS)
#define PRV_MPD_BIST_ENX_FAILED_MASK                (0x1<<PRV_MPD_BIST_ENX_FAILED_POS)
#define PRV_MPD_BIST_RTOS_FAILED_MASK           (0x1<<PRV_MPD_BIST_RTOS_FAILED_POS)
#define PRV_MPD_BIST_SW_ERROR_MASK              (0x1<<PRV_MPD_BIST_SW_ERROR_POS)
#define PRV_MPD_BIST_FW_ECC_ERROR_MASK          (0x1<<PRV_MPD_BIST_FW_ECC_ERROR_POS)
#define PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_MASK  (0x1<<PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_POS)
#define PRV_MPD_BIST_RESERVED_MASK              (0x1<<PRV_MPD_BIST_RESERVED_POS)
#define PRV_MPD_BIST_DEVICE_NOT_SUPORTED_MASK   (0x1<<PRV_MPD_BIST_DEVICE_NOT_SUPORTED_POS)

#define PRV_MPD_BIST_PASSED_MASK                    (0x3FFF)

/* VCT result */
#define PRV_MPD_vctResult_invalid_CNS            0 /* invalid or incomplete test */
#define PRV_MPD_vctResult_pairOk_CNS             1 /* pair okay, no fault */
#define PRV_MPD_vctResult_pairOpen_CNS           2 /* pair open */
#define PRV_MPD_vctResult_intraPairShort_CNS     3 /* intra pair short */
#define PRV_MPD_vctResult_interPairShort_CNS     4 /* inter pair short */

#define PRV_MPD_master_manual_mask_CNS            0x8000
#define PRV_MPD_master_preferred_mask_CNS         0x2000

#define PRV_MPD_3240_REVISION_NUM_Z2_CNS      1 /* PHY 3240 Revision Z2 number */
#define PRV_MPD_fw_version_num_offset_CNS         0x160 /* offset of version number after the header */
#define PRV_MPD_fw_3240_version_num_offset_CNS    0x120 /* offset of version number after the header */

#define PRV_MPD_MTD_MASTER_PREFERENCE_MASK_CNS      0xA000
#define PRV_MPD_MTD_MASTER_PREFERENCE_MASTER_CNS    0x2000
#define PRV_MPD_MTD_MASTER_PREFERENCE_SLAVE_CNS     0

#define PRV_MPD_MAX_NUM_OF_SERDES_PARAMS 10
#define PRV_MPD_SERDES_REGISTER_ADDRESS 0x2634
#define PRV_MPD_BIND_MAC(PhyType, _op, _func_PTR)         \
        prvMpdBind( PhyType,                                \
                     _op,                                      \
                     _func_PTR,                                \
                     PRV_MPD_TO_STRING_MAC(_func_PTR))


#define PRV_MPD_DEBUG_BIND_MAC(_comp, _pkg, _flag, _help, _flagId)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().debug_bind_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().debug_bind_PTR(_comp, _pkg, _flag, _help, _flagId):\
    FALSE)

#define PRV_MPD_IS_VCT_SUPPORTED_MAC(_rel_ifIndex, _supported_PTR)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().vct_supported_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().vct_supported_PTR(_rel_ifIndex, _supported_PTR):\
    FALSE)


#define PRV_MPD_ALLOC_MAC(_alloc_size)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().alloc_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().alloc_PTR(_alloc_size):0)

#define PRV_MPD_REALLOC_MAC(_ptr, _alloc_size)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().realloc_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().realloc_PTR(_ptr, _alloc_size):0)

#define PRV_MPD_CALLOC_MAC(_num_of, Len) prvMpdCalloc(_num_of,Len)

#define PRV_MPD_FREE_MAC(_data)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().free_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().free_PTR(_data):FALSE)


#define PRV_MPD_HANDLE_FAILURE_MAC(_rel_ifIndex, _severity, _error_text)\
    (PRV_MPD_HANDLE_FAILURE_INTERNAL_MAC(_rel_ifIndex, _severity, PRV_MPD_DEBUG_FUNC_NAME_MAC(), _error_text))

#define PRV_MPD_HANDLE_FAILURE_INTERNAL_MAC(_rel_ifIndex, _severity, _func_name, _error_text)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().handle_failure_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().handle_failure_PTR(_rel_ifIndex, _severity, __LINE__, _func_name, _error_text):\
    FALSE)

#define PRV_MPD_LOGGING_MAC(_log_text)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().logging_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().logging_PTR(_log_text):\
    FALSE)

/* don't sleep in simulation */
#ifdef PHY_SIMULATION
#define PRV_MPD_SLEEP_MAC(_time_in_ms)
#else
#define PRV_MPD_SLEEP_MAC(_time_in_ms)  \
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().sleep_PTR != NULL))? \
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().sleep_PTR(_time_in_ms):\
    FALSE)
#endif

#define PRV_MPD_DEBUG_IS_ACTIVE_MAC(_flag) \
    (   (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && \
        (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().is_active_PTR)         && \
        (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().is_active_PTR(_flag)))

#define PRV_MPD_DEBUG_LOG_MAC(_flag) \
    if (    (PRV_MPD_DEBUG_IS_ACTIVE_MAC(_flag) && \
            (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().debug_log_PTR)))   PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().debug_log_PTR

#define PRV_MPD_DEBUG_LOG_PORT_MAC(_flag,_rel_ifIndex) \
    PRV_MPD_DEBUG_LOG_MAC(_flag)

#define PRV_MPD_TRANSCEIVER_ENABLE_MAC(_rel_ifIndex, Enable) \
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().txEnable_PTR != NULL))?\
        PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().txEnable_PTR(_rel_ifIndex, (Enable)):FALSE)

#define PRV_MPD_IS_INFO_VALID_MAC(_data) \
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().isInfoValid_PTR != NULL))?\
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().isInfoValid_PTR(_data):TRUE)

#define PRV_MPD_GET_FW_FILES_MAC(_phyType, _main) \
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().getFwFiles_PTR != NULL))?\
        PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().getFwFiles_PTR(_phyType, _main):FALSE)

#define PRV_MPD_SMI_AUTONEG_DISABLE_MAC(_rel_ifIndex, _disable, _prev_state_PTR)\
    (((PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().initialized == TRUE) && (PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().smiAn_disable_PTR != NULL))?\
    PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC().smiAn_disable_PTR((_rel_ifIndex),(_disable), (_prev_state_PTR)): FALSE)

#define MPD_MDIO_ACCESS_DONT_SET_PAGE_CNS   (0xFFFF)

#define PRV_MPD_MDIO_WRITE_MAC(_rel_ifIndex, _deviceOrPage, _address, _value)\
        prvMpdMdioWrapWrite(_rel_ifIndex, prvMpdGetPortEntry(_rel_ifIndex), _deviceOrPage, _address, _value)

#define PRV_MPD_MDIO_READ_MAC(_rel_ifIndex, _deviceOrPage, _address, _value)\
        prvMpdMdioWrapRead(_rel_ifIndex, prvMpdGetPortEntry(_rel_ifIndex), _deviceOrPage, _address, _value)

#define PRV_MPD_DEBUG_FUNC_NAME_MAC() ((const char *)__FUNCTION__)
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define MPD_STR_MAC(x) #x
#define MPD_CONST_TO_STR_MAC(x) MPD_STR_MAC(x)


#if BYTE_ORDER == LITTLE_ENDIAN_MEM_FORMAT
#define PRV_MPD_SWAP16_MAC(val) \
    (UINT_16)(((UINT_16)((val) & 0xFF) << 8) + ((UINT_16)((val) & 0xFF00) >> 8))
#else
#define PRV_MPD_SWAP16_MAC(val)   (val)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN_MEM_FORMAT
#define PRV_MPD_SWAP32_MAC(val)   (UINT_32 ) ( (((val) & 0xFF) << 24) | (((val) & 0xFF00) << 8) | (((val) & 0xFF0000) >> 8) | (((val) & 0xFF000000) >> 24) )
#else
#define PRV_MPD_SWAP32_MAC(val)   (val)
#endif


#define MPD_IS_VOLTRON_DEVICE_MAC(_phyType) \
    ((_phyType == MPD_TYPE_88E2540_E) || (_phyType == MPD_TYPE_88X3540_E) || (_phyType == MPD_TYPE_88E2580_E) || (_phyType == MPD_TYPE_88X3580_E))


typedef enum {
    PRV_MPD_SPECIFIC_FEATURE_FW_LOAD_SF_E,
    PRV_MPD_SPECIFIC_FEATURE_WA_1680_E,
    PRV_MPD_SPECIFIC_FEATURE_WA_178x_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_1_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_2_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_3_E,
    /* must be last */
    PRV_MPD_SPECIFIC_FEATURE_LAST_E
} PRV_MPD_SPECIFIC_FEATURE_ENT;

typedef void PRV_MPD_SPECIFIC_FEATURE_FUN(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

typedef struct {
    BOOLEAN supported;
    PRV_MPD_SPECIFIC_FEATURE_FUN *preInitFeatureExecuteCallback_PTR;
    PRV_MPD_SPECIFIC_FEATURE_FUN *postInitFeatureExecuteCallback_PTR;
} PRV_MPD_SPECIFIC_FEATURE_STC;

typedef struct {
    MPD_SPEED_CAPABILITY_TYP    ifSpeedCapabBit;
    MPD_TYPE_BITMAP_TYP         phyTypeBit;
} PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC;


typedef enum {
    PRV_MPD_PHY_READY_AFTER_RESET_E,
    PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E,
    PRV_MPD_PHY_READY_WITH_SW_RESET_E
} PRV_MPD_PHY_READY_SEQUENCE_ENT;

#define PRV_MPD_NUM_OF_EXTERNAL_PHY_TYPES_SUPPORTED_CNS (MPD_TYPE_NUM_OF_TYPES_E)


typedef struct {
    UINT_32         max; /* maximum consumption - must be one of the options  */
    struct {
        UINT_32 sr_on[MPD_SPEED_LAST_E];
        UINT_32 sr_off[MPD_SPEED_LAST_E];
    } up; /* power consumption when port is up */

    struct {
        UINT_32 ed_on;
        UINT_32 ed_off;
    } down; /* power consumption when port is down */
} PRV_MPD_POWER_CONSUMPTION_STC;


/*extern PRV_MPD_PORT_LIST_TYP          prvMpdDebugWriteDisabledPortList;*/
extern PRV_MPD_PORT_LIST_TYP            prvMpdActivePorts;

#define PRV_MPD_fw_version_index_major_CNS        0
#define PRV_MPD_fw_version_index_minor_CNS        1
#define PRV_MPD_fw_version_index_inc_CNS          2
#define PRV_MPD_fw_version_index_test_CNS         3
#define PRV_MPD_fw_version_size_CNS               4

typedef struct {
    UINT_8       mdioDev;   /* pp device number */
    UINT_8       mdioBus;   /* bus id */
    MPD_TYPE_ENT phyType; /* phy type */
} PRV_MPD_APP_DATA_STC;

#define PRV_MPD_MTD_OBJECT_MAC(_rel_ifindex)         PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()[_rel_ifindex]->mtdObject_PTR
#define PRV_MPD_MTD_PORT_OBJECT_MAC(_rel_ifindex)    PRV_MPD_MTD_OBJECT_MAC(_rel_ifindex), prvMpdGetPortEntry(_rel_ifindex)->initData_PTR->mdioInfo.mdioAddress
#define PRV_MPD_MTD_ASSERT_REL_IFINDEX(_rel_ifIndex)                    \
        if (_rel_ifIndex > MPD_MAX_PORT_NUMBER_CNS) {                   \
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,                    \
                                        MPD_ERROR_SEVERITY_ERROR_E,     \
                                        "Illegal rel_ifIndex");         \
            return MPD_OP_FAILED_E;                                     \
        }


typedef struct {
    UINT_16 mtd_val;
    UINT_32 hal_val;
} PRV_MPD_MTD_TO_MPD_CONVERT_STC;

#define PRV_MPD_GAIN_COF_BITS1_CNS      3
#define PRV_MPD_GAIN_COF_BITS2_CNS      7

#define PRV_MPD_MAX_GAINS_NUMBER_CNS    4

typedef struct {
    UINT_16 gainCoefficient;
    UINT_16 maxVal;
    double  gainVars [PRV_MPD_MAX_GAINS_NUMBER_CNS * 2];
} PRV_MPD_VCT_EXT_GAIN_INFO_STC;

#define PRV_MPD_STUB_CREATE_MAC(__func_name) \
    extern MPD_RESULT_ENT __func_name (   \
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR, \
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR   \
)

#define PRV_MPD_STUB_PHY_CREATE_MAC(__func_name) \
    extern MPD_RESULT_ENT __func_name (   \
    UINT_32 phyNumber, \
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR   \
)

PRV_MPD_STUB_CREATE_MAC(prvMpdMydPreFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydPostFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetPortMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetSerdesTune);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetSerdesLanePolarity);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetAutoNeg);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortLaneBmp);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortLinkStatus);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetDisable);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydInit7120);
PRV_MPD_STUB_CREATE_MAC(prvMpdInit88X7120);

PRV_MPD_STUB_CREATE_MAC(prvMpdGetMdixOperType_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetMdixAdminMode_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetMdixAdminMode_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetVctTest_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetVctTest_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetExtVctParams_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLen_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdResetPhy_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdResetPhy_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetLinkPartnerPauseCapable_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetLinkPartnerPauseCapable_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenConsumption);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLenNoRange_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationRemoteCapabilities_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationRemoteCapabilities_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdAdvertiseFc_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetComboMediaType_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDuplexMode_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDuplexMode_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindCopper);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindSfp);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindByType);
PRV_MPD_STUB_CREATE_MAC(prvMpdSfpPresentNotification);
PRV_MPD_STUB_CREATE_MAC(prvMpdSfpPresentNotification2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMdioAccess);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetEeeAdvertize);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeStatus);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeCapability);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetPowerModules_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdEnableFiberPortStatus_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetComboMediaType_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetComboMediaType_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetEeeMasterEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLpiExitTime_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLpiEnterTime_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeMaxTxVal_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_9);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_11);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_16);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpecificFeatures_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpecificFeatures_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetVctTest_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpecificFeatures_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_14);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetFastLinkDownEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeGetEnableMode_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeGetEnableMode_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLenNoRange_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetInternalOperStatus_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetInternalOperStatus_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSfpPresentNotification_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeMaxTxVal_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDuplexMode_8);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetMdixAdminMode_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdResetPhy_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLoopback_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyIdentifier);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdMydGetPhyIdentifier);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetPhyPageSelect);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLoopback_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_8);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctOffset_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_9);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetDteStatus_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetTemperature_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMdixMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDuplexMode_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetMdixMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetVctTest);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetCableLenghNoBreakLink);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdPreFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdFwDownload_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdPostFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiEnterTimer_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeAdvertise_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLegacyEnable_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeStatus_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetNearEndLoopback_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetNearEndLoopback_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiExitTimer_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetPowerModules);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetLpAnCapabilities);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetAdvertiseFc);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetLpAdvFc);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdRestartAutoNeg);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetPresentNotification);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetSpeed_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetSpeed_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetAutoNeg_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDisable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDisable_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDisable_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeCapabilities);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeStatus_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeAdvertise_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLegacyEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeConfig_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiExitTimer_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiEnterTimer_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetCheckLinkUp);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetCheckLinkUp_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit33x0And32x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit20x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit2180);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit35x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit25x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetMdixAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMdixAdminMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdRestartAutoNegotiation_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationAdmin_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetAutonegAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetAutonegSupport);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationSupport_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationSupport_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetSerdesTune);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetSerdesTune_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMediaType);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMediaType_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSerdesTune);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctEnableTest1);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctEnableTest2);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctTdrCollectResults1);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctTdrCollectResults2);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctDspGetResults1);
PRV_MPD_STUB_CREATE_MAC(prvMpdVctDspGetResults2);
PRV_MPD_STUB_CREATE_MAC(prvMpdPtpGetTsq);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetDTEStatus);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSendIdleToHost);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDTEConfig);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDteConfig);
PRV_MPD_STUB_PHY_CREATE_MAC(prvMpdPtpInit);
PRV_MPD_STUB_PHY_CREATE_MAC(prvMpdPtpCaptureFRC);
PRV_MPD_STUB_PHY_CREATE_MAC(prvMpdPtpGetCapturedFRC);


/* *****************************************/
/* Private PHY operations */

typedef enum {
    /** @internal @brief Set combo media type */
    PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E = MPD_OP_LAST_PHY_OP_E,
    /** @internal @brief Enable fiber part in PHY */
    PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E,
    /** @internal @brief Pre firmware download */
    PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
    /** @internal @brief Download FW to PHY */
    PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
    /** @internal @brief Post FW download */
    PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
    /** @internal @brief Set PHY specific feature hooks (internal PHY operations) */
    PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
    /** @internal @brief Set PHY (towards host) loopback */
    PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
    /** @internal @brief Debug - run VCT when no cable is connected */
    PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
    /** @internal @brief Initialize PHY */
    PRV_MPD_OP_CODE_INIT_E,
    /** @internal @brief Execure Errata */
    PRV_MPD_OP_CODE_SET_ERRATA_E,
    /** @internal @brief Enable fast link down */
    PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E,
    /** @internal @brief Get max EEE tx exit time */
    PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
    /** @internal @brief Get EEE enable mode (link change / admin) */
    PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
    /**  @internal @brief Verify Link & Speed match negotiation capabilities (confgured)\n
     *  Restart auto-neg in case of missmatch
     */
    PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
    /** @internal @brief get the number of lanes for configured mode */
    PRV_MPD_OP_CODE_GET_LANE_BMP_E,
    /** @internal @brief used for PHYs which support page select */
    PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
    /** @internal @brief used for combo PHYs to decide which media select to choose */
    PRV_MPD_OP_CODE_GET_PHY_COMBO_MEDIA_TYPE_E,

    /** @internal @brief Get power consumption values (DB based, see prvMpdUpdatePhyPowerConsumption).  */
    PRV_MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
    /** @internal @brief Perform I2C read (for supporting PHY, when I2C is connected to PHY). */
    PRV_MPD_OP_CODE_GET_I2C_READ_E,
    /** @internal @brief Get green operational support. See \ref PRV_MPD_GREEN_READINESS_PARAMS_STC   */
    PRV_MPD_OP_CODE_GET_GREEN_READINESS_E,
    /** @internal @brief enable vct test. */
    PRV_MPD_OP_CODE_SET_VCT_ENABLE_TEST_E,
    /** @internal @brief collect vct tdr results. */
    PRV_MPD_OP_CODE_GET_VCT_TDR_RESULTS_E,
    /** @internal @brief Get vct dsp results. */
    PRV_MPD_OP_CODE_GET_VCT_DSP_RESULTS_E,

    PRV_MPD_NUM_OF_OPS_E
} PRV_MPD_OP_CODE_ENT;

typedef enum {
    MPD_LOOPBACK_TYPE_PCS_TO_MAC_COOPER_E,
    MPD_LOOPBACK_TYPE_PCS_TO_MAC_FIBER_E,
    MPD_LOOPBACK_TYPE_PCS_TO_LINE_COPPER_E,
    MPD_LOOPBACK_TYPE_PCS_TO_LINE_FIBER_E,
    MPD_LOOPBACK_TYPE_SERDES_TO_MAC_E,
    MPD_LOOPBACK_TYPE_SERDES_TO_LINE_E,

    /*have to be the last Enum*/
    MPD_LOOPBACK_TYPE_NUMBER_OF_TYPES_E

} MPD_LOOPBACK_TYPE_ENT;

/**
 * @struct PRV_MPD_GLOBAL_SHARED_DB_STC
 * @brief Global Shared DB which can be shared across applications when MPD is used in Shared Lib mode.
 */
typedef struct {

    /** @brief MPD Global DB. Contains INIT and Running Data of all ports. */
    PRV_MPD_GLOBAL_DB_STC               * prvMpdSharedMpdGlobalDb_PTR;

    /** @brief Contains per PHY representative Port entry. Used for accessing TAI registers */
    PRV_MPD_PHY_DB_STC                    prvMpdSharedPhyDbARR[PRV_MPD_MAX_NUM_OF_PHY_CNS];

    /** @brief Port SET to accumulate all 1680 ports.
     *  Used for configuration of QSGMII WA required for 1680 */
    PRV_MPD_PORT_LIST_TYP                 prvMpdShared88E1680PortList;

    /** @brief Port SET to accumulate all 1780 Ports.
     *  Used for configuration of clock cascading required for 178x */
    PRV_MPD_PORT_LIST_TYP                 prvMpdShared88E1780PortList;

    /** @brief Per PHY Power consumption values when Port is Up/Down */
    PRV_MPD_POWER_CONSUMPTION_STC         prvMpdSharedPowerConsumptionPerPhy[MPD_TYPE_NUM_OF_TYPES_E];

    /** @brief This array will hold rel_ifIndex per <dev, mdioBus, mdioAddress> */
    UINT_32                               prvMpdSharedFindPortEntryAssist_ARR[PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS][MPD_MAX_INTERFACE_ID_NUMBER][PRV_MPD_MAX_MDIO_ADRESS_CNS+1];

} PRV_MPD_GLOBAL_SHARED_DB_STC;

/** @}*/

extern PRV_MPD_GLOBAL_SHARED_DB_STC       * prvMpdSharedGlobalDb_PTR;
#define PRV_MPD_SHARED_GLBVAR_ACCESS_MAC()                      (prvMpdSharedGlobalDb_PTR)
#define PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()            (prvMpdSharedGlobalDb_PTR->prvMpdSharedMpdGlobalDb_PTR)
#define PRV_MPD_SHARED_GLBVAR_PHY_DB_ACCESS_MAC()               (prvMpdSharedGlobalDb_PTR->prvMpdSharedPhyDbARR)
#define PRV_MPD_SHARED_GLBVAR_88E1680_PSET_ACCESS_MAC()         (prvMpdSharedGlobalDb_PTR->prvMpdShared88E1680PortList)
#define PRV_MPD_SHARED_GLBVAR_88E1780_PSET_ACCESS_MAC()         (prvMpdSharedGlobalDb_PTR->prvMpdShared88E1780PortList)
#define PRV_MPD_SHARED_GLBVAR_PWR_PER_PHY_ACCESS_MAC()          (prvMpdSharedGlobalDb_PTR->prvMpdSharedPowerConsumptionPerPhy)
#define PRV_MPD_SHARED_GLBVAR_PORTENTRY_ASSIST_ACCESS_MAC()     (prvMpdSharedGlobalDb_PTR->prvMpdSharedFindPortEntryAssist_ARR)
#define PRV_MPD_SHARED_GLBVAR_MTD_PORT_DB_ACCESS_MAC()          (prvMpdSharedGlobalDb_PTR->prvMpdSharedMtdPortDb_ARR)
#define PRV_MPD_SHARED_GLBVAR_MYD_PORT_DB_ACCESS_MAC()          (prvMpdSharedGlobalDb_PTR->prvMpdSharedMydPortDb_ARR)

/**
 * @struct PRV_MPD_NON_SHARED_GLBVAR_MPD_DB_STC
 * @brief STC containing all Global variables used in mpd.c
 */
typedef struct {
    /** @brief Debug Flag: debug PHY vct operations */
    UINT_32                                 prvMpdDebugConfigVctFlagId;

    /** @brief Debug Flag: Debug Trace for PHY */
    UINT_32                                 prvMpdDebugTraceFlagId;

    /** @brief Debug Flag: trace register write per port */
    UINT_32                                 prvMpdDebugWriteFlagId;

    /** @brief Debug Flag: trace register read per port */
    UINT_32                                 prvMpdDebugReadFlagId;

    /** @brief Debug Flag: debug PHY config errors */
    UINT_32                                 prvMpdDebugErrorFlagId;

    /** @brief Debug Flag: debug PHY SFP operations */
    UINT_32                                 prvMpdDebugSfpFlagId;

    /** @brief Debug Flag: debug PHY get operations */
    UINT_32                                 prvMpdDebugOperationGetFlagId;

    /** @brief Debug Flag: debug PHY set operations */
    UINT_32                                 prvMpdDebugOperationSetFlagId;

    /** @brief OS and External Services Callbacks supplied to MPD by the Application */
    MPD_CALLBACKS_STC                       prvMpdCallBacks;

    /** @brief PHY specific Pre and Post Init Callbacks */
    PRV_MPD_SPECIFIC_FEATURE_STC            prvMpdSpecificFeature_ARR[PRV_MPD_SPECIFIC_FEATURE_LAST_E];

    /** @brief PHY specific per MPD operation Callback. Bound by MPD */
    PRV_MPD_FUNC_INFO_ENTRY_STC          ** prvMpdFuncDb_ARR;

    /** @brief Debug Flag: debug reduced configuration sequence */
    UINT_32                                 prvMpdReducedConfigFlagId;

    /** @brief Skip the operations which marked in TRUE */
    BOOLEAN                                 prvMpdSkipOperation_ARR[PRV_MPD_NUM_OF_OPS_E];

} PRV_MPD_NON_SHARED_GLBVAR_MPD_DB_STC;

typedef struct {

    /** @brief An inner table that initializes at run time and contains what page, register and bit, need to change for setting loopback for each loopback-type
     *  @note : relevant to 1G phys and working here with internal loopback type for active media type (fiber/copper)
     */
    PRV_MPD_LOOPBACK_PHY_PROFILE_STC        prvMpdLoopbackPhyProfileARR[MPD_TYPE_NUM_OF_TYPES_E][MPD_LOOPBACK_TYPE_NUMBER_OF_TYPES_E];

} PRV_MPD_NON_SHARED_GLBVAR_MPDINIT_DB_STC;

#ifdef MPD_XML_INIT
typedef struct {
    /** @brief representing the XML parser root descriptor type */
    void                            * mpdInitxmlRootId;
} PRV_MPD_NON_SHARED_GLBVAR_MPDUTILS_DB_STC;
#endif

typedef struct {
    /** @brief global non-shared string, used for debug */
    char                            prvMpdDebugString[256];
} PRV_MPD_NON_SHARED_GLBVAR_MPDDEBUG_DB_STC;

typedef struct {
    /** @brief   The mpd prv database feature array[ MPD_FEATURE_ID_LAST_E ] */
    void                          * prvMpdFeatureDataDbPtr;
} PRV_MPD_NON_SHARED_GLBVAR_MPDPARSER_DB_STC;

/**
 * @struct PRV_MPD_GLOBAL_NON_SHARED_DB_STC
 * @brief Global DB containing all Non-shared variables which are mutable (unique to each application
 *        using MPD.
 */
typedef struct {
    /** @brief Core MPD non-shared DB */
    PRV_MPD_NON_SHARED_GLBVAR_MPD_DB_STC            prvMpdNonSharedMpdDb;

    /** @brief MPD Init non-shared DB */
    PRV_MPD_NON_SHARED_GLBVAR_MPDINIT_DB_STC        prvMpdNonSharedMpdInitDb;

#ifdef MPD_XML_INIT
    /** @brief MPD Utils non-shared DB */
    PRV_MPD_NON_SHARED_GLBVAR_MPDUTILS_DB_STC       prvMpdNonSharedMpdUtilsDb;
#endif

    /** @brief MPD Debug Non-Shared DB */
    PRV_MPD_NON_SHARED_GLBVAR_MPDDEBUG_DB_STC       prvMpdNonSharedMpdDebugDb;

    /** @brief MPD Parser Non-Shared DB */
    PRV_MPD_NON_SHARED_GLBVAR_MPDPARSER_DB_STC      prvMpdNonSharedMpdParserDb;

} PRV_MPD_GLOBAL_NON_SHARED_DB_STC;

extern PRV_MPD_GLOBAL_NON_SHARED_DB_STC           * prvMpdGlobalNonSharedDb_PTR;
#define PRV_MPD_NONSHARED_GLBVAR_ACCESS_MAC()                       (prvMpdGlobalNonSharedDb_PTR)

#define PRV_MPD_NONSHARED_GLBVAR_MPD_ACCESS_MAC()                   (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_VCT_CFG_ACCESS_MAC()       (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugConfigVctFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_TRACE_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugTraceFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_WRITE_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugWriteFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_READ_ACCESS_MAC()          (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugReadFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_ERROR_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugErrorFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SFP_ACCESS_MAC()           (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugSfpFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYGET_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugOperationGetFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_PHYSET_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdDebugOperationSetFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_CB_ACCESS_MAC()                (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdCallBacks)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_PHY_SPECIFIC_ARR_ACCESS_MAC()  (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdSpecificFeature_ARR)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_FUNC_INFO_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdFuncDb_ARR)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_REDUCDED_CONFIG_MAC()      (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdReducedConfigFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPD_DBG_SKIP_OPER_MAC()            (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDb.prvMpdSkipOperation_ARR)

#define PRV_MPD_NONSHARED_GLBVAR_MPDINIT_ACCESS_MAC()               (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdInitDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDINIT_LOOPBACK_ARR_ACCESS_MAC()  (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdInitDb.prvMpdLoopbackPhyProfileARR)

#define PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_ACCESS_MAC()              (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdUtilsDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_XMLROOT_ACCESS_MAC()      ((XML_PARSER_ROOT_DESCRIPTOR_TYP)(prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdUtilsDb.mpdInitxmlRootId))
#define PRV_MPD_NONSHARED_GLBVAR_MPDUTILS_XMLROOT_VOID_ACCESS_MAC() (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdUtilsDb.mpdInitxmlRootId)

#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_ACCESS_MAC()                (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_FW_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdFwFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CFG_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdConfigFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_VCT_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdVctFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_EEE_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdEeeFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_ERR_ACCESS_MAC()        (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdErrorFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_LINK_ACCESS_MAC()       (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdLinkFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_INFO_ACCESS_MAC()       (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdInfoFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_DBG_CRITICAL_ACCESS_MAC()   (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdCriticalFlagId)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_REP_MODULO_ACCESS_MAC()     (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdRepresentativeModulo_ARR)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMTD_PRINT_PHY_FWVER_ACCESS_MAC()    (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMtdDb.prvMpdMtdPrintPhyFw_ARR)

#define PRV_MPD_NONSHARED_GLBVAR_MPDMYD_ACCESS_MAC()                (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMydDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDMYD_DOWNLOAD_PSET_ACCESS_MAC()  (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdMydDb.prvMpdMydDownloadModeEnableSet)

#define PRV_MPD_NONSHARED_GLBVAR_MPDDBG_ACCESS_MAC()                (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDebugDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDDBG_DBGSTR_ACCESS_MAC()         (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdDebugDb.prvMpdDebugString)

#define PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_ACCESS_MAC()             (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdParserDb)
#define PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_ACCESS_MAC()  ((MPD_FEATURE_DATA_STC *)(prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdParserDb.prvMpdFeatureDataDbPtr))
#define PRV_MPD_NONSHARED_GLBVAR_MPDPARSER_FEATURE_DB_VOID_ACCESS_MAC() (prvMpdGlobalNonSharedDb_PTR->prvMpdNonSharedMpdParserDb.prvMpdFeatureDataDbPtr)

/* the following operations don't have parameters
 * MPD_OP_CODE_SET_RESET_PHY_E,
 * MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
 * MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E
 * MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E
 */

/* *****************************************/

/**
 *
 * @brief  Initialize the Global Shared DB Structure
 * @note   This is not part of Shared lib solution but needed for CPSS global variable handling mechanism
 *
 * Sequence for CPSS Shared Lib MPD INIT:
 *       Primary Application (Full Init): prvMpdDriverInitSharedDb --> prvMpdDriverInitNonSharedDb --> mpdDriverInitDb --> mpdPortDbUpdate --> mpdDriverInitHw
 *       Non-Primary Application (Reduced Init): prvMpdDriverInitNonSharedDb --> mpdDriverInitDb --> mpdSharedLibReducedInit
 *
 * @param [in]      sharedMemAllocFunc - Callback to dynamically allocate from the Shared memory.
 *                                       Memory allocated by this callback should be shared
 *                                       between different applications and protected.
 * @param [in,out]  mpdSharedDbPtr - Pointer to the MPD Global Shared DB
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT prvMpdDriverInitSharedDb (
    IN MPD_ALLOC_FUNC       * sharedMemAllocFunc,
    INOUT void              ** mpdSharedDbPtr
);

/**
 *
 * @brief  Initialize the Global Non-Shared DB and provide pointer to caller
 * @note   This is not part of Shared lib solution but needed for CPSS global variable handling mechanism
 * @param [in,out]  mpdNonSharedDbPtr - Pointer to the MPD Global Non-Shared DB
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT prvMpdDriverInitNonSharedDb (
    INOUT void              ** mpdNonSharedDbPtr
);

extern MPD_RESULT_ENT mpdMtdDriverDestroy(
    /*     INPUTS:             */
    void
);

extern MPD_RESULT_ENT mpdMtdInit(
    /*     INPUTS:             */
    IN PRV_MPD_PORT_LIST_TYP   * fw_port_list_PTR,
    IN MPD_TYPE_ENT         * phyType_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDriverInitHw (
    /*     INPUTS:             */
    BOOLEAN isHwInitNeeded
);

extern MPD_RESULT_ENT prvMpdMdioReadRegister(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 device,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *value_PTR
);
extern MPD_RESULT_ENT prvMpdMdioReadRegisterNoPage(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16 *value_PTR
);

extern MPD_RESULT_ENT prvMpdMdioWriteRegisterNoPage(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioWriteRegister(
    /*     INPUTS:             */
    const char *calling_func_PTR,
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
    UINT_16 device,
    UINT_16 address,
    UINT_16 mask,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpd1540BypassSet(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    BOOLEAN bypass /* TRUE - bypass mode, FALSE - mac on PHY mode */
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdEeeIpg_1(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern void* prvMpdCalloc(
    /*     INPUTS:             */
    UINT_32                 numOfObjects,
    UINT_32                 len
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdPortHashCreate(
    /*     INPUTS:             */
    void
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListRemove(
    /*     INPUTS:             */
    UINT_32                         rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern void prvMpdPortListClear(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListAdd(
    /*     INPUTS:             */
    UINT_32                         rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdGetPortEntry(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListIsMember(
    /*     INPUTS:             */
    UINT_32                 rel_ifIndex,
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListGetNext(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR,
    /*     INPUTS / OUTPUTS:   */
    UINT_32    * relative_ifIndex_PTR
    /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListIsEmpty(
    /*!     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

extern UINT_32 prvMpdPortListNumOfMembers(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);


extern MPD_RESULT_ENT mpdBindExternalPhyCallback(
    /*     INPUTS:             */
    MPD_TYPE_ENT           phyType,
    MPD_OP_CODE_ENT        op,
    MPD_OPERATION_FUN   * func_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdXmdioWriteReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_8  device,
    UINT_16 address,
    UINT_16 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdXmdioReadReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_8  device,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16    * data_PTR
);

extern MPD_RESULT_ENT prvMpdPerformPhyOperation(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_32 op, /* MPD_OP_CODE_ENT */
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR
    /*     OUTPUTS:            */
);

extern void prvMpdFwLoadSequence(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * fw_port_list_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdFindPortEntry(
    /*     INPUTS:             */
    PRV_MPD_APP_DATA_STC    * app_data_PTR,
    UINT_16                 mdio_address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT mpdMtdInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP     * fw_portList_PTR,
    MPD_TYPE_ENT           * phyType_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);
/*$ END OF mpdMtdInit */


extern MPD_RESULT_ENT prvMpdUpdatePhyPowerConsumption(
    /*     INPUTS:             */
    MPD_TYPE_ENT    phyType,
    PRV_MPD_POWER_CONSUMPTION_STC *powerConsumption_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioWrapWrite(
    IN  UINT_32                         rel_ifIndex,
    IN  PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    IN  UINT_16                         deviceOrPage,
    IN  UINT_16                         address,
    IN  UINT_16                         value
);

extern MPD_RESULT_ENT prvMpdMdioWrapRead(
    IN  UINT_32                         rel_ifIndex,
    IN  PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    IN  UINT_16                         deviceOrPage,
    IN  UINT_16                         address,
    OUT UINT_16                       * value_PTR
);


extern int prvMpdSnprintf (
    /*!     INPUTS:             */
    char                *str,
    UINT_32               size,
    const char          * format,
    ...
    /*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdPtpSupport(
    IN UINT_32 phyNumber
);

#endif /* PRV_MPD_H_ */


