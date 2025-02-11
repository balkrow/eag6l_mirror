/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_12G_PIPE4_1P2V
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "../C12GP41P2V/mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_API_Sip6_30.h"
#include "mcesdC12GP41P2V_RegRW_Sip6_30.h"
#include "mcesdC12GP41P2V_HwCntl_Sip6_30.h"
#include "mcesdC12GP41P2V_DeviceInit_Sip6_30.h"

#ifdef C12GP41P2V_SIP6_30

CPSS_ADDITION_START
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvHwsComphySerdesSip6_30.h>
CPSS_ADDITION_END

/* Forward internal function prototypes used only in this module */
static MCESD_STATUS INT_C12GP41P2V_AdditionalInit(IN MCESD_DEV_PTR devPtr);

MCESD_STATUS API_C12GP41P2V_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PowerOn powerOn
)
{
CPSS_ADDITION_START
    MCESD_BOOL powerLane0 = powerOn.powerLaneMask & 0x1;
CPSS_ADDITION_END
    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C12GP41P2V_SetPhyMode(devPtr, C12GP41P2V_PHYMODE_SERDES));

    /* Disable TX Output */
    MCESD_ATTEMPT(API_C12GP41P2V_SetTxOutputEnable(devPtr, MCESD_FALSE));

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerIvRef(devPtr, MCESD_TRUE));

CPSS_ADDITION_START
    /* Make sure PHY PLLs, Transmitter and Receiver power is off */
    /*MCESD_ATTEMPT(API_C12GP41P2V_PowerOffLane(devPtr));*/
CPSS_ADDITION_END

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C12GP41P2V_SetRefFreq(devPtr, powerOn.refFreq, powerOn.refClkSel));

    /* Additional Steps */
CPSS_ADDITION_START
    if (powerLane0)
CPSS_ADDITION_END
        MCESD_ATTEMPT(INT_C12GP41P2V_AdditionalInit(devPtr));

    /* Configure TX and RX bitrates */
CPSS_ADDITION_START
    if (powerLane0)
CPSS_ADDITION_END
        MCESD_ATTEMPT(API_C12GP41P2V_SetTxRxBitRate(devPtr, powerOn.speed));

    /* Set Data Bus Width for all lanes */
CPSS_ADDITION_START
    if (powerLane0)
CPSS_ADDITION_END
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, powerOn.dataBusWidth));

CPSS_ADDITION_START
#if 1
    /* Enable DFE for speeds above 6 Gbps */
    if (powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_SetDfeEnable(devPtr, ( ((powerOn.speed > 12) || (powerOn.speed == 11)) && (powerOn.speed != 18) ) ? MCESD_TRUE : MCESD_FALSE));
#endif
CPSS_ADDITION_END
    /* Power on PHY PLLs, Transmitter and Receiver */
CPSS_ADDITION_START
    if (powerLane0)
CPSS_ADDITION_END
    {
CPSS_ADDITION_START
        if (powerOn.initTx)
            MCESD_ATTEMPT(API_C12GP41P2V_SetPowerTx(devPtr, MCESD_TRUE));
        if (powerOn.initRx)
            MCESD_ATTEMPT(API_C12GP41P2V_SetPowerRx(devPtr, MCESD_TRUE));
        MCESD_ATTEMPT(API_C12GP41P2V_SetPowerPLL(devPtr, MCESD_TRUE));
CPSS_ADDITION_END
   }

    /* Enable TX Output for all lanes */
CPSS_ADDITION_START
    if (powerOn.txOutputEn && powerLane0)
CPSS_ADDITION_END
        MCESD_ATTEMPT(API_C12GP41P2V_SetTxOutputEnable(devPtr, MCESD_TRUE));
CPSS_ADDITION_START
#if 0
    /* Toggle RX_INIT */
    if (powerOn.initRx && powerOn.u.powerLane.powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_RxInit(devPtr));
#endif
CPSS_ADDITION_END

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, C12GP41P2V_TRAINING_RX));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerPLL(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerTx(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerRx(devPtr, MCESD_FALSE));

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_AdditionalInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SQ_DEGLITCH_EN, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SQ_LPF, 0x3FF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SQ_LPF_EN, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RCAL_VTH_RX, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_TXADAPT_GN1_EN, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_TXADAPT_G0_EN, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_TXADAPT_G1_EN, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL10, 0xC);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL11, 0xA);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL12, 0x8);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL13, 0x6);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL14, 0x4);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL15, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL16, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL17, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL20, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL21, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL22, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL23, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL24, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL25, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL26, 0x4);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL27, 0x7);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL30, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL31, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL32, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL33, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL34, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL35, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL36, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL37, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL38, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL39, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3A, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3B, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3C, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3D, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3E, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_RES_SEL3F, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL10, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL11, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL12, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL13, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL14, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL15, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL16, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL17, 0x3);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL18, 0x5);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL19, 0x7);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1A, 0xA);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1B, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1C, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1D, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1E, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL1F, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL20, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL21, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL22, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL23, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL24, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL25, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL26, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL27, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL28, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL29, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2A, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2B, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2C, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2D, 0x5);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2E, 0x9);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FFE_CAP_SEL2F, 0xF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_VICM_SMPLR_SEL, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CTLE_VICM1, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CTLE_VICM2, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_VICM_SMPLR, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_VICM_DFE_CTRL, 0x2);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SAMPLER_VCM, 0x3);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SUMF_TARGET_K, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SUMF_TARGET_C, 0x3F0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EO_BASED, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RX_MAX_DFE_0, 0x0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RX_MAX_DFE_1, 0x3);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDR_MAX_DFE_0, 0x1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDR_MAX_DFE_1, 0x4);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_TX_MAX_DFE, 0x3);

    return MCESD_OK;
}

#endif /* C12GP41P2V_SIP6_30 */
