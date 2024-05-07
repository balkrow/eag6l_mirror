/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsIronmanPortCfgIf.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#if !defined (IRONMAN_DEV_SUPPORT)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ironman/mvHwsIronmanPortIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>


#if !defined (IRONMAN_DEV_SUPPORT)
extern  GT_BOOL hwsPpHwTraceFlag;
#endif

typedef struct{
    GT_U32 regAddr;
    GT_U32 expectedData;
    GT_U32 mask;
}MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC;

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC ironman_MAC10G_BR_10G_R1_RegDb[] =
{
    {0x988001D4, 0x0f0000ff, 0xFFFFFFFF},   /*ANP.anp_units_RegFile.Port_Control6[0]*/
    {0x98800008, 0x00000000, 0x0000000F},
    {0x98900008, 0x00000001, 0x0000000F},
    {0x98A00008, 0x00000003, 0x0000000F},
    {0x98B00008, 0x00000004, 0x0000000F},
    {0x98C00008, 0x00000002, 0x0000000F},

    {0x98000000, 0x000f0000, 0x000f0000},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9800018c, 0x00000003, 0x00000003},   /*                 mtip_mac_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/

    {0x98000004, 0x00000001, 0x00000001},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.MTIP_Global_Clock_Divider_Control*/
    {0x98000124, 0x00000005, 0x0000000f},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x98000180, 0x11000100, 0xFFFFFFFF},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/

    {0x98003044, 0x00000004, 0xFFFFFFFF},
    {0x98003048, 0x00000000, 0xFFFFFFFF},
    {0x98003108, 0x00082003, 0xFFFFFFFF},
    {0x98003120, 0x000c0007, 0xFFFFFFFF},
    {0x98003080, 0x00000121, 0xFFFFFFFF},
    {0x98003114, 0x00003178, 0xFFFFFFFF},

    {0x980030a8, 0x03780a00, 0xFFFFFFFF},
    {0x98003008, 0x02000803, 0xFFFFFFFF},

    {0x98300000, 0x000f0303, 0xFFFFFFFF},   /*mtip_pcs_ext_units.mtip_pcs_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/
    {0x98301050, 0x00000980, 0xFFFFFFFF},   /*lpcs_RegFile.IF_MODE[0]*/
    {0x98301048, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_0[0]*/
    {0x9830104c, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_1[0]*/
    {0x98301000, 0x00008140, 0xFFFFFFFF},   /*lpcs_RegFile.CONTROL[0]*/
    {0x98302000, 0x0000a040, 0xFFFFFFFF},   /*base_r_pcs_RegFile.CONTROL1[0]*/
    {0x98303000, 0x00008000, 0xFFFFFFFF},   /*fcfec_RegFile.CONTROL[0]*/
};


const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC ironman_MAC10G_BR_OUSGMII_RegDb[] =
{
    {0x988001D4, 0x0f0000ff, 0xFFFFFFFF},   /*ANP.anp_units_RegFile.Port_Control6[0]*/
    {0x98800008, 0x00000000, 0x0000000F},
    {0x98900008, 0x00000001, 0x0000000F},
    {0x98A00008, 0x00000002, 0x0000000F},
    {0x98B00008, 0x00000003, 0x0000000F},
    {0x98C00008, 0x00000004, 0x0000000F},

    {0x98000000, 0x000f0000, 0x000f0000},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9800018c, 0x00000003, 0x00000003},   /*                 mtip_mac_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/

    {0x98000004, 0x00000001, 0x00000001},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.MTIP_Global_Clock_Divider_Control*/
    {0x98000124, 0x00000001, 0x0000000f},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x98000180, 0x11000100, 0xFFFFFFFF},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/

    {0x98003044, 0x00000004, 0xFFFFFFFF},   /*m_mac_ral[0].TX_IPG_LENGTH*/
    {0x98003048, 0x00000000, 0xFFFFFFFF},   /*m_mac_ral[0].CRC_MODE*/
    {0x98003108, 0x00002003, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_COMMAND_CONFIG*/
    {0x98003120, 0x00030002, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_TX_FIFO_SECTIONS*/
    {0x98003020, 0x00000002, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_TX_FIFO_SECTIONS*/
    {0x98003080, 0x00000121, 0xFFFFFFFF},   /*m_mac_ral[0].XIF_MODE*/
    {0x98003114, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_FRM_LENGTH*/
    {0x98003014, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_FRM_LENGTH*/

    {0x980030a8, 0x03780a00, 0xFFFFFFFF},   /*m_mac_ral[0].BR_CONTROL*/
    {0x98003008, 0x02002803, 0xFFFFFFFF},   /*m_mac_ral[0].COMMAND_CONFIG*/

    {0x98300000, 0x000f0303, 0xFFFFFFFF},   /*mtip_pcs_ext_units.mtip_pcs_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/
    {0x98301050, 0x00000909, 0xFFFFFFFF},   /*lpcs_RegFile.IF_MODE[0]*/
    {0x98301048, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_0[0]*/
    {0x9830104c, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_1[0]*/
    {0x98301000, 0x00008140, 0xFFFFFFFF},   /*lpcs_RegFile.CONTROL[0]*/
    {0x98302000, 0x0000a040, 0xFFFFFFFF},   /*base_r_pcs_RegFile.CONTROL1[0]*/
    {0x98303000, 0x00008000, 0xFFFFFFFF},   /*fcfec_RegFile.CONTROL[0]*/

    {0x983013e0, 0x00000cff, 0xFFFFFFFF},   /*lpcs_general_registers_RegFile.GMODE*/
};


const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC ironman_MAC10G_BR_QXGMII_RegDb[] =
{
    {0x988001D4, 0x0f0000ff, 0xFFFFFFFF},   /*ANP.anp_units_RegFile.Port_Control6[0]*/
    {0x98800008, 0x00000001, 0x0000000F},
    {0x98900008, 0x00000000, 0x0000000F},
    {0x98A00008, 0x00000002, 0x0000000F},
    {0x98B00008, 0x00000003, 0x0000000F},
    {0x98C00008, 0x00000004, 0x0000000F},

    {0x98000000, 0x000f0000, 0x000f0000},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9800018c, 0x00000003, 0x00000003},   /*                 mtip_mac_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/

    {0x98000004, 0x00000001, 0x00000001},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.MTIP_Global_Clock_Divider_Control*/
    {0x98000124, 0x00000001, 0x0000000f},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x98000180, 0x01000100, 0xFFFFFFFF},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/

    {0x98003044, 0x00000004, 0xFFFFFFFF},   /*m_mac_ral[0].TX_IPG_LENGTH*/
    {0x98003048, 0x00000000, 0xFFFFFFFF},   /*m_mac_ral[0].CRC_MODE*/
    {0x98003108, 0x00002003, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_COMMAND_CONFIG*/
    {0x98003120, 0x00080007, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_TX_FIFO_SECTIONS*/
    {0x98003020, 0x00000002, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_TX_FIFO_SECTIONS*/
    {0x98003080, 0x00000121, 0xFFFFFFFF},   /*m_mac_ral[0].XIF_MODE*/
    {0x98003114, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_FRM_LENGTH*/
    {0x98003014, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_FRM_LENGTH*/
#if 0
    {0x98003154, 0x0000ffff, 0xFFFFFFFF},
    {0x98003164, 0x00000000, 0xFFFFFFFF},
#endif
    {0x980030a8, 0x03780a00, 0xFFFFFFFF},   /*m_mac_ral[0].BR_CONTROL*/
    {0x98003008, 0x02002803, 0xFFFFFFFF},   /*m_mac_ral[0].COMMAND_CONFIG*/

    {0x98300000, 0x000f0303, 0xFFFFFFFF},   /*mtip_pcs_ext_units.mtip_pcs_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/
#if 0
    {0x98301050, 0x00000909, 0xFFFFFFFF},   /*lpcs_RegFile.IF_MODE[0]*/
#endif
    {0x98301058, 0x00000001, 0xFFFFFFFF},   /*lpcs_RegFile.USXGMII_REP[0]*/
    {0x98301048, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_0[0]*/
    {0x9830104c, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_1[0]*/
    {0x98301000, 0x00008140, 0xFFFFFFFF},   /*lpcs_RegFile.CONTROL[0]*/
    {0x98302000, 0x0000a040, 0xFFFFFFFF},   /*base_r_pcs_RegFile.CONTROL1[0]*/
    {0x98303000, 0x00008000, 0xFFFFFFFF},   /*fcfec_RegFile.CONTROL[0]*/
    {0x98303008, 0x00000004, 0xFFFFFFFF},   /*fcfec_RegFile.PORTS_ENA[0]*/
    {0x9830300c, 0x00001004, 0xFFFFFFFF},   /*fcfec_RegFile.VL_INTVL[0]*/
    {0x98303018, 0x00000000, 0xFFFFFFFF},   /*fcfec_RegFile.VL_INTVL_HI[0]*/

    {0x983013e8, 0x00000004, 0xFFFFFFFF},   /*lpcs_general_registers_RegFile.M0*/
};

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC ironman_MAC10G_BR_QSGMII_RegDb[] =
{
    {0x988001D4, 0x0f0000ff, 0xFFFFFFFF},   /*ANP.anp_units_RegFile.Port_Control6[0]*/
    {0x98800008, 0x00000001, 0x0000000F},
    {0x98900008, 0x00000000, 0x0000000F},
    {0x98A00008, 0x00000002, 0x0000000F},
    {0x98B00008, 0x00000003, 0x0000000F},
    {0x98C00008, 0x00000004, 0x0000000F},

    {0x98000000, 0x000f0000, 0x000f0000},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9800018c, 0x00000003, 0x00000003},   /*                 mtip_mac_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/

    {0x98000004, 0x00000001, 0x00000001},   /*m_ext_global_ral.mtip_mac_global_ext_units_RegFile.MTIP_Global_Clock_Divider_Control*/
    {0x98000124, 0x00000001, 0x0000000f},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x98000180, 0x01000100, 0xFFFFFFFF},   /*                 mtip_mac_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/

    {0x98003044, 0x00000004, 0xFFFFFFFF},   /*m_mac_ral[0].TX_IPG_LENGTH*/
    {0x98003048, 0x00000000, 0xFFFFFFFF},   /*m_mac_ral[0].CRC_MODE*/
    {0x98003108, 0x00002003, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_COMMAND_CONFIG*/
    {0x98003120, 0x00030002, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_TX_FIFO_SECTIONS*/
    {0x98003020, 0x00000002, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_TX_FIFO_SECTIONS*/
    {0x98003080, 0x00000121, 0xFFFFFFFF},   /*m_mac_ral[0].XIF_MODE*/
    {0x98003114, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].EMAC_FRM_LENGTH*/
    {0x98003014, 0x00003178, 0xFFFFFFFF},   /*m_mac_ral[0].PMAC_FRM_LENGTH*/
#if 0
    {0x98003154, 0x0000ffff, 0xFFFFFFFF},
    {0x98003164, 0x00000000, 0xFFFFFFFF},
#endif
    {0x980030a8, 0x03780a00, 0xFFFFFFFF},   /*m_mac_ral[0].BR_CONTROL*/
    {0x98003008, 0x02002803, 0xFFFFFFFF},   /*m_mac_ral[0].COMMAND_CONFIG*/

    {0x98300000, 0x000f0303, 0xFFFFFFFF},   /*mtip_pcs_ext_units.mtip_pcs_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/
    {0x98301050, 0x00000909, 0xFFFFFFFF},   /*lpcs_RegFile.IF_MODE[0]*/
#if 0
    {0x98301058, 0x00000001, 0xFFFFFFFF},   /*lpcs_RegFile.USXGMII_REP[0]*/
#endif
    {0x98301048, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_0[0]*/
    {0x9830104c, 0x00000000, 0xFFFFFFFF},   /*lpcs_RegFile.LINK_TIMER_1[0]*/
    {0x98301000, 0x00008140, 0xFFFFFFFF},   /*lpcs_RegFile.CONTROL[0]*/
    {0x98302000, 0x0000a040, 0xFFFFFFFF},   /*base_r_pcs_RegFile.CONTROL1[0]*/
    {0x98303000, 0x00008000, 0xFFFFFFFF},   /*fcfec_RegFile.CONTROL[0]*/
};

/**
* @internal mvHwsIronmanExtMacClockEnable function
* @endinternal
*
* @brief  EXT MAC Clock enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtMacClockEnable
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData, extField;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(serdesFullConfig);

    regData = (enable == GT_TRUE) ? 1 : 0;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtMacClockEnable ******\n");
    }
#endif

    /* ext_br_manager::mac_clock_enable(uint port_index) */

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        /* set m_RAL_global.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control.cmn_mac_clk_en */
        extField = IRONMAN_MTIP_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_GLOBAL_EXT_UNIT, extField, regData, NULL));

        /* set m_RAL_global.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control.cmn_app_clk_en */
        extField = IRONMAN_MTIP_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_GLOBAL_EXT_UNIT, extField, regData, NULL));
    }

    extField = IRONMAN_MTIP_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_CLK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtMacClockEnable ******\n");
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsIronmanExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - (pointer to) port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsIronmanExtMacClockEnableGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      *enablePtr
)
{
    GT_U32 regData, extField;

    GT_UNUSED_PARAM(portMode);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtMacClockEnableGet ******\n");
    }
#endif

    extField = IRONMAN_MTIP_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_CLK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, &regData, NULL));

    *enablePtr = (regData != 0) ? GT_TRUE : GT_FALSE;

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsIronmanExtMacClockEnableGet ******\n");
        }
#endif
    return GT_OK;
}


/**
* @internal mvHwsIronmanExtMacResetRelease function
* @endinternal
*
* @brief  EXT MAC Reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtMacResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData, extField;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(serdesFullConfig);

    regData = (enable == GT_TRUE) ? 1 : 0;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtMacResetRelease ******\n");
    }
#endif

    /* ext_br_manager::mac_reset_release(uint port_index) */

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        /* set m_RAL_global.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control.cmn_mac_reset_ */
        extField = IRONMAN_MTIP_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_GLOBAL_EXT_UNIT, extField, regData, NULL));

        /* set m_RAL_global.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control.cmn_app_reset_ */
        extField = IRONMAN_MTIP_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_GLOBAL_EXT_UNIT, extField, regData, NULL));
    }

    /* set m_RAL_global.mtip_mac_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control.cmn_mac_clk_en */
    extField = IRONMAN_MTIP_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtMacResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsIronmanExtMacSpeedSet function
* @endinternal
*
* @brief  EXT MAC Speed config
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtMacSpeedSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable
)
{
    GT_U32 regData, extField;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtMacSpeedSet ******\n");
    }
#endif

    switch(portMode)
    {
        case QSGMII:
        case _5G_QUSGMII:
        case SGMII:
        case _100Base_FX:
        case _10G_OUSGMII:
        case _10G_QXGMII:
        case _10G_SXGMII:
        case _5G_SXGMII:
        case _2_5G_SXGMII:
            regData = 1;
            break;

        case _1000Base_X:
        case _1000Base_SX:
        case _2500Base_X:
        case SGMII2_5:
            regData = 3;
            break;

        case _10GBase_KR:
        case _10GBase_SR_LR:
        case _12GBaseR:
        case _5GBaseR:
            regData = 5;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* reset field to default value */
    if(enable == GT_FALSE)
    {
        regData = 0xF;
    }

    /* set m_RAL[port_index].mtip_mac_port_ext_units_RegFile.MTIP_Port_Control1.port_res_speed */
    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL1_PORT_RES_SPEED_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtMacSpeedSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsIronmanExtMacMuxCtrlSet function
* @endinternal
*
* @brief  EXT MAC MUX Control - BR/NON_BR selector
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] isPreemptionEnabled - port preemption status
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtMacMuxCtrlSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           portNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN GT_BOOL                          isPreemptionEnabled
)
{
    GT_U32 regData, extField;

    GT_UNUSED_PARAM(portMode);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtMacMuxCtrlSet ******\n");
    }
#endif

    /*
        External mux ctrl.
        The mux selects between preemption channel of the BR mac and
        non preemption channel of the nonBR mac.
            0 - preempitive channel of the BR mac
            1 - non preemptive channel of the nonBR mac
    */
    regData = (isPreemptionEnabled == GT_TRUE) ? 0 : 1;

    /* set m_RAL[port_index].mtip_mac_port_ext_units_RegFile.External_Port_Control.mac_mux_ctrl */
    extField = IRONMAN_MTIP_MAC_PORT_EXT_EXTERNAL_PORT_CONTROL_MAC_MUX_CTRL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtMacMuxCtrlSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsIronmanExtPcsClockEnable function
* @endinternal
*
* @brief  EXT unit xpcs clock enable + EXT xpcs release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtPcsClockEnable
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;
    MV_HWS_UNITS_ID unitId = MTI_10G_PCS_EXT_UNIT;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(serdesFullConfig);

    regData = (enable == GT_TRUE) ? 1 : 0;


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtPcsClockEnable ******\n");
    }
#endif

    /* m_ext_manager:pcs_manager:pcs_clock_enable_and_reset_release */

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        /* set shared pcs clk & clk */
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtPcsClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtPcsResetRelease function
* @endinternal
*
* @brief  EXT unit xpcs reset release/enable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtPcsResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;
    MV_HWS_UNITS_ID unitId = MTI_10G_PCS_EXT_UNIT;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(serdesFullConfig);

    regData = (enable == GT_TRUE) ? 1 : 0;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtPcsResetRelease ******\n");
    }
#endif

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        /* set shared pcs clk & clk */
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_MAC_CLK_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtPcsResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtSetLaneWidth function
* @endinternal
*
* @brief  Set port lane width (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtSetLaneWidth
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT width
)
{
    GT_STATUS rc;
    GT_U32 regData;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;
    MV_HWS_UNITS_ID unitId = MTI_10G_PCS_EXT_UNIT;

    /* reg_model.m_ext_manager.Configure(conf.port_num, (conf.pcs_port_config.get_lane_width() == 20)); */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtSetLaneWidth ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    regData = (width == _20BIT_ON) ? 1 : 0;
    if (convertIdx.ciderIndexInUnit < 4)
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_CONTROL_SD0_N2_E;
    }
    else
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_CONTROL_SD1_N2_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtSetLaneWidth ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsIronmanExtReset function
* @endinternal
*
* @brief  Reset/unreset EXT SD RX/TX unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
* @param[in] portFullConfig  - the port place in the group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsIronmanExtReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 reset,
    IN MV_HWS_PORT_IN_GROUP_ENT portFullConfig
)
{
    GT_STATUS rc;
    GT_U32 regData = 0, pcsField;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 sdIndex;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtReset ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    regData = (reset == GT_FALSE) ? 1 : 0;

    /* Power down may be executed only for the last port in group */
    if(regData == 0)
    {
        if(portFullConfig != MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            return GT_OK;
        }
    }

    if(regData == 1)
    {
        /* per port (index = 0) PCS unreset for multiplexed ports causes to
           link toggling on other ports in group */
        if((portMode != _10G_OUSGMII) && (portMode != QSGMII) && (portMode != _5G_QUSGMII))
        {
            pcsField = IRONMAN_LPCS_CONTROL_RESET_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 1, NULL));
            pcsField = IRONMAN_BASE_R_PCS_CONTROL1_RESET_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_BASER_UNIT, pcsField, 1, NULL));
            if(portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E)
            {
                pcsField = IRONMAN_PCS_FCFEC_CONTROL_RESET_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_FCFEC_UNIT, pcsField, 1, NULL));
            }
        }
    }

    /* special treatment for 100FX mode */
    if (portMode == _100Base_FX)
    {
        sdIndex = convertIdx.ciderIndexInUnit / 4;

        if(sdIndex == 0)
        {
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD0_RX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD0_TX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_CDR_SD0_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD0_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD0_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
        }
        else
        {
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD1_RX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD1_TX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_CDR_SD1_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD1_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_100FX_SD1_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
        }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtReset ******\n");
    }
#endif
        return GT_OK;
    }

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            sdIndex = 0;
        }
        else
        {
            sdIndex = convertIdx.ciderIndexInUnit / 4;
        }

        if(sdIndex == 0)
        {
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_TX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_RX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
        }
        else
        {
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_TX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_RX_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
            extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtReset ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsIronmanReplicationSet function
* @endinternal
*
* @brief   set usx replication.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] portInitInParamPtr       - PM DB
* @param[in] linkUp                   - port link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanReplicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_STANDARD            portMode,
    IN  MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr,
    IN  GT_BOOL                         linkUp
)
{
    MV_HWS_UNITS_ID         unitId = MTI_10G_PCS_LPCS_PORT_UNIT;
    MV_HWS_IRONMAN_LPCS_UNITS_E fieldNameGmiiRep, fieldNameGmii_2_5;
    GT_U32                  valGmiiRep, valGmii_2_5;
    GT_U32                  portTypeDefaultSpeed = 0, sgmiiSpeed = 2;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_FLOAT32              replication, an_step, desired_timer = 2000000.0;
    GT_U32                  final_timer;
    MV_HWS_PORT_SPEED_ENT   portSpeed = portInitInParamPtr->portSpeed;
    GT_U32                  regData = 0;
    GT_UREG_DATA            data = 0;
    GT_BOOL                 autoNegEnabled = portInitInParamPtr->autoNegEnabled;

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanReplicationSet ******\n");
    }
#endif
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /**
     * The function is also called  in prvCpssPortManagerLinkUpCheck
     * in case of link UP
     */
    switch (linkUp)
    {
    case GT_FALSE:
        switch (portMode)
        {
                /*low speed */
            case _100Base_FX:
            case SGMII:
            case _1000Base_X:
            case _1000Base_SX:
            case QSGMII:
            case SGMII2_5:
            case _2500Base_X:
            case _5G_QUSGMII:
            case _10G_OUSGMII:
            case _5GBaseR:
            case HWS_10G_MODE_CASE:
            case _12GBaseR:
            case _12GBase_SR:
                /**
                 * For low speed, the timers must be cleard
                 * and valGmiiRep and valGmii_2_5 set to zero.
                 */
                portTypeDefaultSpeed = 0;
                desired_timer = 0;
                an_step = 12.8;
                /**
                 * For low speed, autonegotiation is configured
                 * separatly
                 */
                autoNegEnabled = GT_FALSE;
                break;

            case _10G_QXGMII:
            case _5G_DXGMII:
            case _2_5G_SXGMII:
                portTypeDefaultSpeed = 2500;
                an_step = 25.6;
                break;

            case _10G_DXGMII:
            case _5G_SXGMII:
                portTypeDefaultSpeed = 5000;
                an_step = 12.8;
                break;

            case _10G_SXGMII:
                portTypeDefaultSpeed = 10000;
                an_step = 6.4;
                break;

            default:
                return GT_NOT_SUPPORTED;
        }

        /**
         * Calculate values for replication and timers for desierd
         * speed:
         *
         * if (Anenable)
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(1);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(0);
         * else
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(Usxgmiirep);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(Usxgmii2_5);
         *
         * final_timer=desired_timer/an_step*2;
         */
        replication = (autoNegEnabled) ? (0x1) : ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        final_timer = (GT_U32)((desired_timer / an_step)*2);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_CONTROL_ANENABLE_E, autoNegEnabled, NULL));

        data = ((autoNegEnabled) ? (final_timer & 0xFFFF) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E, data, NULL));

        data = ((autoNegEnabled) ? ((final_timer >> 16) & 0x1F) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E, data, NULL));

        if (GT_FALSE == autoNegEnabled)
        {
            if (portSpeed == MV_HWS_PORT_SPEED_10M_E)
            {
                sgmiiSpeed = 0;
            }
            else if (portSpeed == MV_HWS_PORT_SPEED_100M_E)
            {
                sgmiiSpeed = 1;
            }
            else
            {
                sgmiiSpeed = 2;
            }

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_speed.set(sgmii_speed); */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_IF_MODE_SGMII_SPEED_E, sgmiiSpeed, NULL));
        }
        else
        {
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_DEV_ABILITY_ABILITY_RSV05_E, 1, NULL));
        }

        break;

    case GT_TRUE:
        switch (portMode)
        {
            case _10G_QXGMII:
            case _5G_DXGMII:
            case _2_5G_SXGMII:
                portTypeDefaultSpeed = 2500;
                break;

            case _10G_DXGMII:
            case _5G_SXGMII:
                portTypeDefaultSpeed = 5000;
                break;

            case _10G_SXGMII:
                portTypeDefaultSpeed = 10000;
                break;

            case SGMII:
            case QSGMII:
                /**
                 * Adjust resolution after AN process completed.
                 *
                 * In SGMII, the speed field is 2bits [11:10] in
                 *
                 * <MTIP_IP>MTIP_IP/<MTIP_IP>
                 * LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
                 *
                 */
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E, &regData, NULL));
                data = (regData & 0x3);

                /**
                 * Set the relevant speed in
                 *
                 * <MTIP_IP>MTIP_IP/<MTIP_IP>
                 * LPCS/LPCS Units %j/PORT<%n>_IF_MODE In bit[3:2]
                 */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_IF_MODE_SGMII_SPEED_E, data, NULL));
                return GT_OK;

            case _5G_QUSGMII:
            case _10G_OUSGMII:
                /**
                 * Adjust resolution after AN process completed.
                 *
                 * In OUSGMII, the speed field is 3bits – [11:9] in
                 *
                 * <MTIP_IP>MTIP_IP/<MTIP_IP>
                 * LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
                 *
                 * TBD for QUSGMII
                 */
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV9_E, &regData, NULL));
                data = (regData & 0x1);
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E, &regData, NULL));
                data |= ((regData & 0x3) << 1);
                /**
                 * Set the relevant speed in
                 *
                 *       <MTIP_IP>MTIP_IP/<MTIP_IP>
                 *       LPCS/LPCS Units %j/PORT<%n>_IF_MODE
                 * In bit[3:2]
                 */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_IF_MODE_SGMII_SPEED_E, data, NULL));

                return GT_OK;

            default:
                return GT_NOT_SUPPORTED;
        }

        /**
         *  Get speed from link partner page in case of AN acknowledge
         */
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV9_E, &regData, NULL));
        data |= (regData & 0x1);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E, &regData, NULL));
        data |= ((regData & 0x3) << 1);
        switch (data)
        {
        case 0:
            portSpeed = MV_HWS_PORT_SPEED_10M_E;
            break;
        case 1:
            portSpeed = MV_HWS_PORT_SPEED_100M_E;
            break;
        case 2:
            portSpeed = MV_HWS_PORT_SPEED_1G_E;
            break;
        case 4:
            portSpeed = MV_HWS_PORT_SPEED_2_5G_E;
            break;
        case 5:
            portSpeed = MV_HWS_PORT_SPEED_5G_E;
            break;
        case 3:
            portSpeed = MV_HWS_PORT_SPEED_10G_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /**
         * Calculate values for replication for confirmed speed
         */
        replication = ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        break;

    default:
        return GT_NOT_SUPPORTED;
    }

    /**
     * The replication values must be set for both the desierd and
     * confirmed speed.
     */
    if (replication == 2.5)
    {
        valGmii_2_5 = 1;
        valGmiiRep = 0;
    }
    else
    {
        valGmii_2_5 = 0;
        valGmiiRep = (GT_U32)replication;
    }

    fieldNameGmiiRep = IRONMAN_LPCS_USXGMII_REP_USXGMIIREP_E;
    fieldNameGmii_2_5 = IRONMAN_LPCS_USXGMII_REP_USXGMII2_5_E;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmiiRep, valGmiiRep, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmii_2_5, valGmii_2_5, NULL));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanReplicationSet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsIronmanReplicationReset function
* @endinternal
*
* @brief   reset usx replication.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsIronmanReplicationReset
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_STANDARD            portMode
)
{
    MV_HWS_UNITS_ID         unitId = MTI_10G_PCS_LPCS_PORT_UNIT;
    MV_HWS_IRONMAN_LPCS_UNITS_E fieldNameGmiiRep, fieldNameGmii_2_5;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanReplicationReset ******\n");
    }
#endif
    fieldNameGmiiRep = IRONMAN_LPCS_USXGMII_REP_USXGMIIREP_E;
    fieldNameGmii_2_5 = IRONMAN_LPCS_USXGMII_REP_USXGMII2_5_E;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmiiRep,  0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmii_2_5, 0, NULL));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanReplicationReset ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsIronmanExtFXEnable function
* @endinternal
*
* @brief   set config for 100FX port mode.
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical number
* @param[in] portMode              - port mode
* @param[in] enable                - enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsIronmanExtFXEnable
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_BOOL                  enable
)
{
    GT_U32 regData;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanExtFXEnable ******\n");
    }
#endif

    if(portMode != _100Base_FX)
    {
        return GT_OK;
    }

    regData = (enable == GT_TRUE) ? 1 : 0;
    extField = IRONMAN_MTIP_PCS_EXT_UNITS_CONTROL_P0_100FX_MODE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, regData, NULL));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanExtFXEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsIronmanUsxmCfg function
* @endinternal
*
* @brief   config usxm.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] enable                   - enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsIronmanUsxmCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                  enable
)
{
    MV_HWS_UNITS_ID unitId = MTI_10G_PCS_FCFEC_UNIT;
    GT_U32          fieldName, markerInterval=0, regData;
    GT_U32          activePorts = 0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx));
    CHECK_STATUS(mvHwsMarkerIntervalCalc(devNum, portMode, FEC_OFF, &markerInterval));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanUsxmCfg ******\n");
    }
#endif

    markerInterval /= 4;
    activePorts = 4;

    /* int_manager::ConfigureUSXMlane */
    unitId = MTI_10G_PCS_FCFEC_UNIT;
    fieldName = IRONMAN_PCS_FCFEC_PORTS_ENA_NUMPORTS_E;
    regData = (enable == GT_TRUE) ? activePorts : 0;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, activePorts, NULL));

    fieldName = IRONMAN_PCS_FCFEC_VL_INTVL_MARKER_COUNTER_E;
    regData = (enable == GT_TRUE) ? (markerInterval & 0xFFFF) : 0x1004;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, regData, NULL));
    fieldName = IRONMAN_PCS_FCFEC_VL_INTVL_HI_MARKER_COUNTERHI_E;
    regData = (enable == GT_TRUE) ? ((markerInterval >> 16) & 0xFF) : 0;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, regData, NULL));

    if (convertIdx.ciderIndexInUnit < 4)
    {
        fieldName = IRONMAN_LPCS_GENERAL_M0_PORTSENABLE_E;
    }
    else
    {
        fieldName = IRONMAN_LPCS_GENERAL_M1_PORTSENABLE_E;
    }
    unitId = MTI_10G_PCS_LPCS_GENERAL_UNIT;
    regData = (enable == GT_TRUE) ? activePorts : 0;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, regData, NULL));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanUsxmCfg ******\n");
    }
#endif
    return GT_OK;
}



/**
* @internal mvHwsIronmanEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portInitInParamPtr     - port info struct
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsIronmanEthPortPowerUp
(
    IN GT_U8                            devNum,
    IN GT_U32                           phyPortNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS    *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_IN_GROUP_ENT stateInSerdesGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_IN_GROUP_ENT stateInPortGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS portAttributes;
    MV_HWS_PORT_PCS_TYPE    pcsType;

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("\n****** Start of mvHwsIronmanEthPortPowerUp for portNum=%d ******\n", phyPortNum);
    }
#endif

    HWS_NULL_PTR_CHECK_MAC(portInitInParamPtr);
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

     /* should be done before mvHwsIronmanExtMacClockEnable() that enables clocks */
    CHECK_STATUS(mvHwsExtIfFirstInPortGroupCheck(devNum, phyPortNum, portMode, &stateInPortGroup));
    CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum, phyPortNum, portMode, &stateInSerdesGroup));

    CHECK_STATUS(mvHwsIronmanExtMacClockEnable(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtMacResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtMacSpeedSet(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsIronmanExtMacMuxCtrlSet(devNum, phyPortNum, portMode, portInitInParamPtr->isPreemptionEnabled));

    pcsType = curPortParams.portPcsType;
    CHECK_STATUS(mvHwsPcsRemoteFaultSelectSet(devNum, phyPortNum, pcsType, portMode, GT_TRUE));

    portAttributes.preemptionEnable = portInitInParamPtr->isPreemptionEnabled;
    portAttributes.portSpeed = portInitInParamPtr->portSpeed;
    portAttributes.duplexMode = portInitInParamPtr->duplexMode;

    CHECK_STATUS(mvHwsMacModeCfg(devNum, 0, phyPortNum, portMode, &portAttributes));

    CHECK_STATUS(mvHwsIronmanExtPcsClockEnable(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtPcsResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtSetLaneWidth(devNum, phyPortNum, portMode, curPortParams.serdes10BitStatus));
    CHECK_STATUS(mvHwsIronmanExtFXEnable(devNum, phyPortNum, portMode, GT_TRUE));

    /* For multi-USX modes (DX- QX- OX-) USM Multiplexer should be set only for the first port in SD group */
    if ((portMode == _10G_QXGMII) && (stateInSerdesGroup == MV_HWS_PORT_IN_GROUP_FIRST_E))
    {
        CHECK_STATUS(mvHwsIronmanUsxmCfg(devNum, 0, phyPortNum, portMode, GT_TRUE));
    }

    CHECK_STATUS(mvHwsPcsModeCfg(devNum, 0, phyPortNum, portMode, &portAttributes));
    CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, phyPortNum, portMode));

    CHECK_STATUS(mvHwsIronmanReplicationSet(devNum, 0, phyPortNum, portMode, portInitInParamPtr, GT_FALSE));

    /* Un-Reset mtip */
    CHECK_STATUS(mvHwsIronmanExtReset(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsIronmanEthPortPowerUp for portNum=%d, portMode = %d ******\n\n", phyPortNum, portMode);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsIronmanEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] stateInPortGroup         - first/existing/last int port group
* @param[in] stateInSerdesGroup       - first/existing/last int serdes group
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanEthPortPowerDown
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_IN_GROUP_ENT    stateInPortGroup,
    MV_HWS_PORT_IN_GROUP_ENT    stateInSerdesGroup
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_RESET pcsReset = POWER_DOWN;
    MV_HWS_PORT_ACTION action = PORT_POWER_DOWN;
    MV_HWS_PORT_PCS_TYPE    pcsType;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("******mvHwsIronmanEthPortPowerDown: port %d delete mode %d  port group %d serdes group %d ******\n", phyPortNum, portMode,stateInPortGroup,stateInSerdesGroup);
    }
#endif

    if(mvHwsUsxModeCheck(devNum, phyPortNum, portMode))
    {
        if(stateInSerdesGroup != MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            pcsReset = PARTIAL_POWER_DOWN;
            action = PORT_PARTIAL_POWER_DOWN;
        }
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsIronmanExtMacSpeedSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsIronmanExtMacMuxCtrlSet(devNum, phyPortNum, portMode, GT_FALSE));

    pcsType = curPortParams.portPcsType;
    CHECK_STATUS(mvHwsPcsRemoteFaultSelectSet(devNum, phyPortNum, pcsType, portMode, GT_FALSE));

    CHECK_STATUS(mvHwsIronmanReplicationReset(devNum, 0, phyPortNum, portMode));
    if ((portMode == _10G_QXGMII) && (stateInSerdesGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        CHECK_STATUS(mvHwsIronmanUsxmCfg(devNum, 0, phyPortNum, portMode, GT_FALSE));
    }
    CHECK_STATUS(mvHwsIronmanExtFXEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsIronmanExtReset(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup));
    CHECK_STATUS(mvHwsPortStopCfg(devNum, 0, phyPortNum, portMode, action, &(curLanesList[0]), pcsReset, POWER_DOWN));
    CHECK_STATUS(mvHwsIronmanExtPcsResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsIronmanExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsIronmanEthPortPowerDown for portNum=%d, portMode = %d ******\n\n", phyPortNum, portMode);
    }
#endif

    return GT_OK;
}

#ifndef  MV_HWS_FREE_RTOS
GT_STATUS mvHwsIronmanPunktCheck(GT_U32 portType)
{
    GT_U32 i;
    GT_U32 iterNum;
    GT_U32 regData;
    const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC *regPtr;

    if(portType == 0)
    {
        hwsOsPrintf(" 1 - MTI_10G_MAC_BR - 10G_R1         \n");
        hwsOsPrintf(" 2 - MTI_10G_MAC_BR - 10G_OUSGMII    \n");
        hwsOsPrintf(" 3 - MTI_10G_MAC_BR - USX 10G_QXGMII \n");
        hwsOsPrintf(" 4 -     \n");
        hwsOsPrintf(" 5 -     \n");
        hwsOsPrintf(" 6 -     \n");
        hwsOsPrintf(" 7 -     \n");
        hwsOsPrintf(" 8 -     \n");
        hwsOsPrintf(" 9 -     \n");
        hwsOsPrintf(" 10-     \n");
        hwsOsPrintf(" 11-     \n");
        hwsOsPrintf(" 12-     \n");
        hwsOsPrintf(" 13-     \n");
        hwsOsPrintf("\n");
        return GT_OK;
    }
    else if(portType == 1) /* MTI10BR 10G_R1 port */
    {
        iterNum = sizeof(ironman_MAC10G_BR_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = ironman_MAC10G_BR_10G_R1_RegDb;
    }
    else if(portType == 2) /* USX 10G OUSGMII port*/
    {
      iterNum = sizeof(ironman_MAC10G_BR_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
      regPtr = ironman_MAC10G_BR_OUSGMII_RegDb;
    }
    else if(portType == 3) /* USX 10G_QXGMII port*/
    {
        iterNum = sizeof(ironman_MAC10G_BR_QXGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = ironman_MAC10G_BR_QXGMII_RegDb;
    }
    else if(portType == 4) /* USX 5G_QSGMII */
    {
        iterNum = sizeof(ironman_MAC10G_BR_QSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = ironman_MAC10G_BR_QSGMII_RegDb;
    }
    else if(portType == 5) /* CPU 10G port*/
    {
/*      iterNum = sizeof(phoenixCpu_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixCpu_10G_R1_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 6) /* MTI100 50G_R2 */
    {
/*      iterNum = sizeof(phoenixMti100_50G_R2_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_50G_R2_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 7) /* MTI100 100G_R4 port */
    {
/*      iterNum = sizeof(phoenixMti100_100G_R4_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_100G_R4_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 8) /* MTI100 SGMII*/
    {
/*      iterNum = sizeof(phoenixMti100_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_SGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 9) /* MTIUSX SGMII*/
    {
/*      iterNum = sizeof(phoenixMtiUSX_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUSX_SGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 10) /* MTI100 1000BaseX*/
    {
/*      iterNum = sizeof(phoenixMti100_1000BaseX_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_1000BaseX_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 11) /* MTI100 10G_R1*/
    {
/*      iterNum = sizeof(ironman_MAC10G_BR_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = ironman_MAC10G_BR_10G_R1_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 12) /* MTI USX 10G OUSGMII */
    {
/*      iterNum = sizeof(phoenixMtiUsx_10G_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUsx_10G_OUSGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 13) /* USX 10G port*/
    {
/*      iterNum = sizeof(phoenixMtiUsx_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUsx_10G_R1_RegDb;                                                              */
        return GT_FAIL;
    }
    else
    {
        hwsOsPrintf("ILLEGAL PORT TYPE\n");
        return GT_FAIL;
    }
    hwsOsPrintf("iterNum = %d\n\n", iterNum);

    for(i = 0; i < iterNum; i++)
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(0, 0, regPtr[i].regAddr, &regData, 0xFFFFFFFF));
        if ((regData & regPtr[i].mask) != (regPtr[i].expectedData & regPtr[i].mask))
        {
            hwsOsPrintf("Found mismatch: iter = %d, regAddr = 0x%x, data = 0x%x, expectedData = 0x%x\n", i, regPtr[i].regAddr, (regData& regPtr[i].mask), (regPtr[i].expectedData& regPtr[i].mask));
        }
    }

    return GT_OK;
}

GT_STATUS mvHwsIronmanPunktUnitBaseGet
(
    GT_U32                  macPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          unitAddr = 0;
    GT_U32          localUnitNum = 0;
    GT_U32          unitIndex = 0;
    GT_U32 i;

    rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, portMode, &convertIdx);
    if(rc != GT_OK)
    {
        return rc;
    }
    unitId = MTI_10G_MAC_BR_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_MAC_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_MAC_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


    unitId = MTI_10G_MAC_NON_BR_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_MAC_NON_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_MAC_NON_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = MTI_10G_GLOBAL_EXT_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_GLOBAL_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_GLOBAL_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = MTI_10G_PORT_EXT_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_PORT_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_PORT_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


    unitId = ANP_USX_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("ANP_USX_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("ANP_USX_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = ANP_CPU_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("ANP_CPU_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("ANP_CPU_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


#if 0
    switch (portMode)
    {
        case _10GBase_KR:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10GBase_KR /*NON SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MTI_10G_MAC_BR_E;
            break;

        case _2_5G_SXGMII:
        case _5G_SXGMII:
        case _10G_SXGMII:
        case _5G_DXGMII:
        case _10G_DXGMII:
        case _20G_DXGMII:
        case _5G_QUSGMII:
        case _10G_QXGMII:
        case _20G_QXGMII:
        case _10G_OUSGMII:
        case _20G_OXGMII:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10G_OUSGMII /*USX PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_USX_UNIT;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }
    if(mvHwsMtipIsReducedPort(0,macPortNum) == GT_TRUE)
    {
        unitId = MIF_CPU_UNIT;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK))
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    osPrintf("MIF UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    if (mvHwsUsxModeCheck(0, macPortNum, portMode))
    {
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            unitId = ANP_USX_O_UNIT;
        }
        else
        {
            unitId = ANP_USX_UNIT;
        }
        unitId1 = AN_USX_UNIT;
    }
    else if(mvHwsMtipIsReducedPort(0,macPortNum) == GT_TRUE)
    {
        unitId = ANP_CPU_UNIT;
        unitId1 = AN_CPU_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitId1 = AN_400_UNIT;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
    {
        return GT_FAIL;
    }

    osPrintf("ANP UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    rc = mvUnitExtInfoGet(0, unitId1, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
    {
        return GT_FAIL;
    }
#endif


    return GT_OK;
}

#endif

/**
* @internal mvHwsWindowSizeIndicationGet function
* @endinternal
*
* @brief  Get window size (indication of EOM width for 100FX only for Ironman only)
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[out] regData        - (pointer to) EOM width 100FX
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsWindowSizeIndicationGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum,
    OUT GT_U32 *regData
)
{
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E pcsField;
    GT_U32 combinedValue, tempData;

    if (regData == NULL)
    {
        return GT_BAD_PTR;
    }

    /* reset sticky windows indication (bit 2) */
    pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_RX_CONTROL3_SD0_RESERVED_INPUT_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, pcsField, regData, NULL));
    combinedValue = (*regData & 0xFFFFFFFB) | ((0x1 << 2));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, pcsField, combinedValue, NULL));
    hwsOsTimerWkFuncPtr(1);
    combinedValue = (*regData) & 0xFFFFFFFB;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, pcsField, combinedValue, NULL));
    hwsOsTimerWkFuncPtr(100);

    pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_STATUS2_SD0_RESERVED_OUT_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, pcsField, &tempData, NULL));
    *regData = tempData & 0xFF;

    return GT_OK;
}
