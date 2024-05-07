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
#include "mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_RegRW.h"
#include "mcesdC12GP41P2V_HwCntl.h"
#include "mcesdC12GP41P2V_API.h"
#include "../mcesdUtils.h"
#if !defined AC5_DEV_SUPPORT && !defined (AC5P_DEV_SUPPORT)
#include <stdio.h>
#endif
#include <string.h>
#include <math.h>

#ifdef C12GP41P2V

/* Forward internal function prototypes used only in this module */
static MCESD_STATUS INT_C12GP41P2V_ProgramNewSpeedsSequence(IN MCESD_DEV_PTR devPtr);
static MCESD_STATUS INT_C12GP41P2V_EOMSetPhase(IN MCESD_DEV_PTR devPtr, IN MCESD_32 softwarePhase, IN MCESD_32 targetPhase, IN MCESD_16 phaseOffset);
static MCESD_STATUS INT_C12GP41P2V_CalculateBER(IN MCESD_U32 bitErrorCount, IN MCESD_U64 bitCount, OUT MCESD_U32* nanoBER);
static MCESD_STATUS INT_C12GP41P2V_SpeedGbpsToMbps(IN E_C12GP41P2V_SERDES_SPEED speed, OUT MCESD_U32* mbps);

MCESD_STATUS API_C12GP41P2V_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *locked
)
{
    MCESD_FIELD pllLock = F_C12GP41P2VR2P0_PLL_LOCK;
    MCESD_U16 pllData;
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllLock, &pllData));
    *locked = (pllData == 1) ? MCESD_TRUE : MCESD_FALSE;
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
)
{
    MCESD_FIELD pllReadyTx = F_C12GP41P2VR2P0_PLL_READY_TX;
    MCESD_FIELD pllReadyRx = F_C12GP41P2VR2P0_PLL_READY_RX;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllReadyTx, &data));
    *txReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &pllReadyRx, &data));
    *rxReady = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCDRLock
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_FIELD cdrLock = F_C12GP41P2VR2P0_CDR_LOCK;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &cdrLock, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_RxInit
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_INIT0, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_RX_INIT_DONE0, 1, 5000));
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_INIT0, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    IN MCESD_U32 paramValue
)
{

    MCESD_FIELD txEmPreEn       = F_C12GP41P2VR2P0_G1_TX_PRE_EN;
    MCESD_FIELD txEmPreCtrl     = F_C12GP41P2VR2P0_G1_TX_PRE;
    MCESD_FIELD txEmPeakEn      = F_C12GP41P2VR2P0_G1_TX_PEAK_EN;
    MCESD_FIELD txEmPeakCtrl    = F_C12GP41P2VR2P0_G1_TX_PEAK;
    MCESD_FIELD txEmPostEn      = F_C12GP41P2VR2P0_G1_TX_POST_EN;
    MCESD_FIELD txEmPostCtrl    = F_C12GP41P2VR2P0_G1_TX_POST;

    switch (param)
    {
    case C12GP41P2V_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPreEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPreCtrl, paramValue));
        break;
    case C12GP41P2V_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPeakEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPeakCtrl, paramValue));
        break;
    case C12GP41P2V_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPostEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmPostCtrl, paramValue));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TXEQ_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD txEmPreCtrl     = F_C12GP41P2VR2P0_G1_TX_PRE;
    MCESD_FIELD txEmPeakCtrl    = F_C12GP41P2VR2P0_G1_TX_PEAK;
    MCESD_FIELD txEmPostCtrl    = F_C12GP41P2VR2P0_G1_TX_POST;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_TXEQ_EM_PRE_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPreCtrl, &data));
        break;
    case C12GP41P2V_TXEQ_EM_PEAK_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPeakCtrl, &data));
        break;
    case C12GP41P2V_TXEQ_EM_POST_CTRL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txEmPostCtrl, &data));
        break;
    default:
        return MCESD_FAIL;  /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD resSel      = F_C12GP41P2VR2P0_RES_SEL;
    MCESD_FIELD resShift    = F_C12GP41P2VR2P0_RES_SHIFT;
    MCESD_FIELD capSel      = F_C12GP41P2VR2P0_CAP_SEL;
    MCESD_FIELD ffeForce    = F_C12GP41P2VR2P0_FFE_SETTING_F;

    switch (param)
    {
    case C12GP41P2V_CTLE_RES_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &resSel, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CTLE_RES_SHIFT:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &resShift, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CTLE_CAP_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &capSel, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD resSel      = F_C12GP41P2VR2P0_ADAPTEDFFE_RES;
    MCESD_FIELD resShift    = F_C12GP41P2VR2P0_RES_SHIFT;
    MCESD_FIELD capSel      = F_C12GP41P2VR2P0_ADAPTEDFFE_CAP;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_CTLE_RES_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &resSel, &data));
        break;
    case C12GP41P2V_CTLE_RES_SHIFT:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &resShift, &data));
        break;
    case C12GP41P2V_CTLE_CAP_SEL:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &capSel, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    MCESD_U16 data;

    switch (tap)
    {
    case C12GP41P2V_DFE_DC_S:
        {
            MCESD_FIELD dcS = F_C12GP41P2VR2P0_DFE_DC_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dcS, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, dcS.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F0_S:
        {
            MCESD_FIELD f0S = F_C12GP41P2VR2P0_DFE_F0_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f0S, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f0S.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F1_S:
        {
            MCESD_FIELD f1S = F_C12GP41P2VR2P0_DFE_F1_S;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f1S, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f1S.totalBits);
        }
        break;
    case C12GP41P2V_DFE_DC_D:
        {
            MCESD_FIELD dcD = F_C12GP41P2VR2P0_DFE_DC_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dcD, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, dcD.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F0_D:
        {
            MCESD_FIELD f0D = F_C12GP41P2VR2P0_DFE_F0_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f0D, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f0D.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F1_D:
        {
            MCESD_FIELD f1D = F_C12GP41P2VR2P0_DFE_F1_D;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f1D, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f1D.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F2:
    {
        MCESD_FIELD f2 = F_C12GP41P2VR2P0_DFE_F2;

        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f2, &data));
        *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f2.totalBits);
    }
        break;
    case C12GP41P2V_DFE_F3:
        {
            MCESD_FIELD f3 = F_C12GP41P2VR2P0_DFE_F3;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f3, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f3.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F4:
        {
            MCESD_FIELD f4 = F_C12GP41P2VR2P0_DFE_F4;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f4, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f4.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F5:
        {
            MCESD_FIELD f5 = F_C12GP41P2VR2P0_DFE_F5;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f5, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f5.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F6:
        {
            MCESD_FIELD f6 = F_C12GP41P2VR2P0_DFE_F6;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f6, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f6.totalBits);
        }
        break;
    case C12GP41P2V_DFE_F7:
        {
            MCESD_FIELD f7 = F_C12GP41P2VR2P0_DFE_F7;

            MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &f7, &data));
            *tapValue = ConvertSignedMagnitudeToI32((MCESD_U32)data, f7.totalBits);
        }
        break;
    default:
        return MCESD_FAIL; /* Unsupported DFE tap */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_IVREF, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_IVREF, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_TX0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_TX0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}


MCESD_STATUS API_C12GP41P2V_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_RX0, state));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_RX0, &data));
    *state = (data == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_IDLE0, state ? 0 : 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_IDLE0, &pinValue));
    *state = (pinValue == 0) ? MCESD_TRUE : MCESD_FALSE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    E_C12GP41P2V_REFFREQ freq;
    E_C12GP41P2V_REFCLK_SEL clkSel;
    E_C12GP41P2V_SERDES_SPEED speed;
    MCESD_FIELD g1PllIcp = F_C12GP41P2VR2P0_G1_PLL_ICP;

    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PU_PLL0, state));

    while (MCESD_TRUE == state)
    {
        /* Speed is C12GP41P2V_SERDES_10P3125G */
        MCESD_ATTEMPT(API_C12GP41P2V_GetTxRxBitRate(devPtr, &speed));
        if (C12GP41P2V_SERDES_10P3125G != speed)
            break;

        MCESD_ATTEMPT(API_C12GP41P2V_GetRefFreq(devPtr, &freq, &clkSel));
        if ((C12GP41P2V_REFFREQ_25MHZ != freq) && (C12GP41P2V_REFFREQ_156P25MHZ != freq))
            break;

        if (C12GP41P2V_REFFREQ_25MHZ == freq)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xC));
            break;
        }

        /* Reference Frequency is C12GP41P2V_REFFREQ_156P25MHZ */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0x7));
        break;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PU_PLL0, &pinValue));
    *state = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_MODE, mode));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_PHYMODE *mode
)
{
    MCESD_U32 pinValue;
    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PHY_MODE, &pinValue));
    *mode = (E_C12GP41P2V_PHYMODE)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_REFFREQ freq,
    IN E_C12GP41P2V_REFCLK_SEL clkSel
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_REF_FREF_SEL, freq));
    MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_REFCLK_SEL, clkSel));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_REFFREQ *freq,
    OUT E_C12GP41P2V_REFCLK_SEL *clkSel
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_REF_FREF_SEL, &pinValue));
    *freq = (E_C12GP41P2V_REFFREQ)pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_REFCLK_SEL, &pinValue));
    *clkSel = (E_C12GP41P2V_REFCLK_SEL)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SERDES_SPEED speed
)
{
    MCESD_FIELD rxCtleDataRate1Force    = F_C12GP41P2VR2P0_RX_CTLE_1_F;
    MCESD_FIELD rxCtleDataRate2Force    = F_C12GP41P2VR2P0_RX_CTLE_2_F;
    MCESD_FIELD rxCtleDataRate3Force    = F_C12GP41P2VR2P0_RX_CTLE_3_F;
    MCESD_FIELD g1RxCtleDataRate1       = F_C12GP41P2VR2P0_G1_RX_CTLE_1;
    MCESD_FIELD g1RxCtleDataRate2       = F_C12GP41P2VR2P0_G1_RX_CTLE_2;
    MCESD_FIELD g1RxCtleDataRate3       = F_C12GP41P2VR2P0_G1_RX_CTLE_3;
    MCESD_FIELD phyGenMax               = F_C12GP41P2VR2P0_PHY_GEN_MAX;
    CPSS_ADDITION_START
    MCESD_FIELD rxDclk2xEnLane          = F_C12GP41P2VR2P0_RXDCLK_2X_EN_LANE;
    MCESD_FIELD txDclk2xEnLane          = F_C12GP41P2VR2P0_TXDCLK_2X_EN_LANE;
    CPSS_ADDITION_END

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x0));

    if (C12GP41P2V_SERDES_11P5625G != speed)
    {
        if (C12GP41P2V_SERDES_10P3125G == speed)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate1, 0x6));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate2, 0x6));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate3, 0x6));
        }
        CPSS_ADDITION_START
        if ((C12GP41P2V_SERDES_1P25G == speed) || (C12GP41P2V_SERDES_3P125G == speed))
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDclk2xEnLane, 0x1));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txDclk2xEnLane, 0x1));
        }
        else
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDclk2xEnLane, 0x0));
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txDclk2xEnLane, 0x0));
        }
        CPSS_ADDITION_END
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_TX0, speed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, speed));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyGenMax, 0x3));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_TX0, C12GP41P2V_SERDES_10P3125G));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, C12GP41P2V_SERDES_10P3125G));
        MCESD_ATTEMPT(INT_C12GP41P2V_ProgramNewSpeedsSequence(devPtr));
    }

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_ProgramNewSpeedsSequence
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD txTxregSpeedtrkClkForce     = F_C12GP41P2VR2P0_TX_CLK_FORCE;
    MCESD_FIELD g1TxTxregSpeedtrkClk        = F_C12GP41P2VR2P0_G1_TX_CLK;
    MCESD_FIELD txTxregSpeedtrkDataForce    = F_C12GP41P2VR2P0_TX_DATA_FORCE;
    MCESD_FIELD g1TxTxregSpeedtrkData       = F_C12GP41P2VR2P0_G1_TX_DATA;
    MCESD_FIELD txSpeedDivForce             = F_C12GP41P2VR2P0_TX_SPDIV_FORCE;
    MCESD_FIELD txSpeedDiv                  = F_C12GP41P2VR2P0_TX_SPEED_DIV;
    MCESD_FIELD txVddCalEnForce             = F_C12GP41P2VR2P0_TX_VDD_CAL_F;
    MCESD_FIELD g1TxVddCalEn                = F_C12GP41P2VR2P0_G1_TX_VDDCALEN;

    MCESD_FIELD rxSpeedDivForce             = F_C12GP41P2VR2P0_RX_SPDIV_FORCE;
    MCESD_FIELD rxSpeedDiv                  = F_C12GP41P2VR2P0_RX_SPEED_DIV;
    MCESD_FIELD rxDtlclkDivForce            = F_C12GP41P2VR2P0_RX_DTLCLK_F;
    MCESD_FIELD g1RxDtlclkDiv               = F_C12GP41P2VR2P0_G1_RX_DTLCLK_D;
    MCESD_FIELD rxIntpiForce                = F_C12GP41P2VR2P0_RX_INTPI_FORCE;
    MCESD_FIELD g1RxIntpi                   = F_C12GP41P2VR2P0_G1_RX_INTPI;
    MCESD_FIELD rxIntprForce                = F_C12GP41P2VR2P0_RX_INTPR_FORCE;
    MCESD_FIELD g1RxIntpr                   = F_C12GP41P2VR2P0_G1_RX_INTPR;
    MCESD_FIELD rxEomIntprForce             = F_C12GP41P2VR2P0_RX_EOM_INTPR_F;
    MCESD_FIELD g1RxEomIntpr                = F_C12GP41P2VR2P0_G1_RXEOM_INTPR;
    MCESD_FIELD rxRxregSpeedtrkClkForce     = F_C12GP41P2VR2P0_RX_CLK_FORCE;
    MCESD_FIELD g1RxRxregSpeedtrkClk        = F_C12GP41P2VR2P0_G1_RX_CLK;
    MCESD_FIELD rxRxregSpeedtrkDataForce    = F_C12GP41P2VR2P0_RX_DATA_FORCE;
    MCESD_FIELD g1RxRxregSpeedtrkData       = F_C12GP41P2VR2P0_G1_RX_DATA;
    MCESD_FIELD rxCkpwCntForce              = F_C12GP41P2VR2P0_RX_CKPW_CNT_F;
    MCESD_FIELD g1RxCkpwCnt                 = F_C12GP41P2VR2P0_G1_RX_CKPW_CNT;
    MCESD_FIELD rxPuAlign90Force            = F_C12GP41P2VR2P0_RX_PU_A90_F;
    MCESD_FIELD g1RxPuAlign90               = F_C12GP41P2VR2P0_G1_RX_PU_A90;
    MCESD_FIELD rxCtleDataRate1Force        = F_C12GP41P2VR2P0_RX_CTLE_1_F;
    MCESD_FIELD g1RxCtleDataRate1           = F_C12GP41P2VR2P0_G1_RX_CTLE_1;
    MCESD_FIELD rxCtleDataRate2Force        = F_C12GP41P2VR2P0_RX_CTLE_2_F;
    MCESD_FIELD g1RxCtleDataRate2           = F_C12GP41P2VR2P0_G1_RX_CTLE_2;
    MCESD_FIELD rxCtleDataRate3Force        = F_C12GP41P2VR2P0_RX_CTLE_3_F;
    MCESD_FIELD g1RxCtleDataRate3           = F_C12GP41P2VR2P0_G1_RX_CTLE_3;
    MCESD_FIELD rxVddCalEnForce             = F_C12GP41P2VR2P0_RX_VDD_CAL_F;
    MCESD_FIELD g1RxVddCalEn                = F_C12GP41P2VR2P0_G1_RX_VDDCALEN;
    MCESD_FIELD rxDccCalEnForce             = F_C12GP41P2VR2P0_RX_DCC_CAL_F;
    MCESD_FIELD g1RxDccCalEn                = F_C12GP41P2VR2P0_G1_RX_DCCCA_EN;
    MCESD_FIELD rxclk25MForce               = F_C12GP41P2VR2P0_RX_25M_FORCE;
    MCESD_FIELD anaRxclk25MCtrl             = F_C12GP41P2VR2P0_RX_25M_CTRL;
    MCESD_FIELD anaRxclk25MDiv1p5En         = F_C12GP41P2VR2P0_RX_25M_DIV1P5;
    MCESD_FIELD anaRxclk25MDiv              = F_C12GP41P2VR2P0_RX_25M_DIV;
    MCESD_FIELD anaRxclk25MFixDivEn         = F_C12GP41P2VR2P0_RX_25M_FIX_DIV;

    MCESD_FIELD phyModeFmReg                = F_C12GP41P2VR2P0_PHY_MODE_FMREG;
    MCESD_FIELD phyMode                     = F_C12GP41P2VR2P0_PHY_MODE;
    MCESD_FIELD refFrefSelFmReg             = F_C12GP41P2VR2P0_FREF_SEL_FMREG;
    MCESD_FIELD refFrefSel                  = F_C12GP41P2VR2P0_REF_FREF_SEL;
    MCESD_FIELD pllRefdivForce              = F_C12GP41P2VR2P0_PLL_REFDIV_F;
    MCESD_FIELD pllRefDiv                   = F_C12GP41P2VR2P0_PLL_REFDIV;
    MCESD_FIELD pllFbdivForce               = F_C12GP41P2VR2P0_PLL_FBDIV_F;
    MCESD_FIELD pllFbdiv                    = F_C12GP41P2VR2P0_PLL_FBDIV;
    MCESD_FIELD pllBwSelForce               = F_C12GP41P2VR2P0_PLL_BW_SEL_F;
    MCESD_FIELD g1PllBwSel                  = F_C12GP41P2VR2P0_G1_PLL_BW_SEL;
    MCESD_FIELD pllLpfC2SelForce            = F_C12GP41P2VR2P0_PLL_LPF_C2_F;
    MCESD_FIELD g1PllLpfC2Sel               = F_C12GP41P2VR2P0_G1_PLL_LPF_C2;
    MCESD_FIELD pllIcpForce                 = F_C12GP41P2VR2P0_PLL_ICP_FORCE;
    MCESD_FIELD g1PllIcp                    = F_C12GP41P2VR2P0_G1_PLL_ICP;
    MCESD_FIELD pllSpeedThreshForce         = F_C12GP41P2VR2P0_PLL_SPEED_F;
    MCESD_FIELD pllSpeedThresh              = F_C12GP41P2VR2P0_PLL_SPEED;
    MCESD_FIELD pllFbdivCalForce            = F_C12GP41P2VR2P0_FBDIV_CAL_F;
    MCESD_FIELD pllFbdivCal                 = F_C12GP41P2VR2P0_FBDIV_CAL;
    MCESD_FIELD txintpiForce                = F_C12GP41P2VR2P0_TXINTPI_FORCE;
    MCESD_FIELD r0Txintpi                   = F_C12GP41P2VR2P0_R0_TXINTPI;
    MCESD_FIELD txintprForce                = F_C12GP41P2VR2P0_TXINTPR_FORCE;
    MCESD_FIELD r0Txintpr                   = F_C12GP41P2VR2P0_R0_TXINTPR;
    MCESD_FIELD txdtxclkDivForce            = F_C12GP41P2VR2P0_TXDTXCLK_DIV_F;
    MCESD_FIELD r0TxdtxclkDiv               = F_C12GP41P2VR2P0_R0_TXDTXCLKDIV;
    MCESD_FIELD pllRegSpeedTrkForce         = F_C12GP41P2VR2P0_PLL_REG_FORCE;
    MCESD_FIELD r0PllRegSpeedTrk            = F_C12GP41P2VR2P0_R0_REG_SPD_TRK;
    MCESD_FIELD initTxfoffsForce            = F_C12GP41P2VR2P0_INIT_TXFOFFS_F;
    MCESD_FIELD initTxfoffs                 = F_C12GP41P2VR2P0_INIT_TXFOFFS;
    MCESD_FIELD initRxfoffsForce            = F_C12GP41P2VR2P0_INIT_RXFOFFS_F;
    MCESD_FIELD initRxfoffs                 = F_C12GP41P2VR2P0_INIT_RXFOFFS;
    E_C12GP41P2V_REFFREQ freq;
    E_C12GP41P2V_REFCLK_SEL clkSel;

    /* TX Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTxregSpeedtrkClkForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxTxregSpeedtrkClk, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTxregSpeedtrkDataForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxTxregSpeedtrkData, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txSpeedDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txSpeedDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txVddCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1TxVddCalEn, 0x1));

    /* RX Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxSpeedDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxSpeedDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDtlclkDivForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxDtlclkDiv, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxIntpiForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxIntpi, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxIntprForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxIntpr, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxEomIntprForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxEomIntpr, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxRxregSpeedtrkClkForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxRxregSpeedtrkClk, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxRxregSpeedtrkDataForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxRxregSpeedtrkData, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCkpwCntForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCkpwCnt, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPuAlign90Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxPuAlign90, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate1Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate1, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate2Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate2, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleDataRate3Force, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxCtleDataRate3, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxVddCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxVddCalEn, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxDccCalEnForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxDccCalEn, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxclk25MForce, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MCtrl, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MDiv1p5En, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MDiv, 0x73));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxclk25MFixDivEn, 0x0));

    /* PLL Programming */
    MCESD_ATTEMPT(API_C12GP41P2V_GetRefFreq(devPtr, &freq, &clkSel));
    if (C12GP41P2V_REFFREQ_25MHZ == freq)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyModeFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyMode, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSelFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSel, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdiv, 0xE7));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllBwSelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllBwSel, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllLpfC2SelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllLpfC2Sel, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllIcpForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xC));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThreshForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThresh, 0xFB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCalForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCal, 0x17));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintpiForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpi, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintprForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpr, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdtxclkDivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0TxdtxclkDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRegSpeedTrkForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0PllRegSpeedTrk, 0x6));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffs, 0x7CB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffs, 0x7CB));
    }
    else if (C12GP41P2V_REFFREQ_156P25MHZ == freq)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyModeFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &phyMode, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSelFmReg, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &refFrefSel, 0xC));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRefDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdiv, 0x25));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllBwSelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllBwSel, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllLpfC2SelForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllLpfC2Sel, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllIcpForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1PllIcp, 0xB));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThreshForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllSpeedThresh, 0xFD));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCalForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllFbdivCal, 0x17));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintpiForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpi, 0x4));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txintprForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0Txintpr, 0x2));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdtxclkDivForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0TxdtxclkDiv, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &pllRegSpeedTrkForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &r0PllRegSpeedTrk, 0x6));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initTxfoffs, 0x0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffsForce, 0x1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &initRxfoffs, 0x0));
    }
    else
    {
        return MCESD_FAIL; /* No sequence for this reference clock */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SERDES_SPEED *speed
)
{
    MCESD_FIELD anaRxclk25MCtrl             = F_C12GP41P2VR2P0_RX_25M_CTRL;
    MCESD_FIELD anaRxclk25MDiv1p5En         = F_C12GP41P2VR2P0_RX_25M_DIV1P5;
    MCESD_FIELD anaRxclk25MDiv              = F_C12GP41P2VR2P0_RX_25M_DIV;
    MCESD_U16 data;
    MCESD_U32 pinValue;

    /* Check ANA_RX_CLK_25M values to see if ProgramNewSpeedsSequence set 11.5625G. */
    while(1)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MCtrl, &data));
        if (0x2 != data)
            break;
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MDiv1p5En, &data));
        if (0x0 != data)
            break;
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxclk25MDiv, &data));
        if (0x73 != data)
            break;
        *speed = C12GP41P2V_SERDES_11P5625G;
        return MCESD_OK;
    }

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_PHY_GEN_RX0, &pinValue));
    *speed = (E_C12GP41P2V_SERDES_SPEED)pinValue;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATABUS_WIDTH width
)
{
    MCESD_FIELD selBits = F_C12GP41P2VR2P0_SEL_BITS;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selBits, width));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATABUS_WIDTH *width
)
{
    MCESD_FIELD selBits = F_C12GP41P2VR2P0_SEL_BITS;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selBits, &data));
    *width = (E_C12GP41P2V_DATABUS_WIDTH)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetAlign90(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 align90
)
{
    MCESD_FIELD align90Force    = F_C12GP41P2VR2P0_ALIGN90_FORCE;
    MCESD_FIELD align90Ext      = F_C12GP41P2VR2P0_ALIGN90_EXT;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &align90Force, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &align90Ext, align90));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetAlign90(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16 *align90
)
{
    MCESD_FIELD align90Ref = F_C12GP41P2VR2P0_ALIGN90_REF;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &align90Ref, &data));
    *align90 = data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    IN S_C12GP41P2V_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C12GP41P2VR2P0_TRX_TRAINTIMER;
    MCESD_FIELD rxTrainTimer        = F_C12GP41P2VR2P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimerEnable = F_C12GP41P2VR2P0_TRX_TRAIN_T_EN;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimer, training->timeout));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trxTrainTimerEnable, training->enable));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxTrainTimer, training->timeout));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTrainingTimeout(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT S_C12GP41P2V_TRAINING_TIMEOUT *training
)
{
    MCESD_FIELD trxTrainTimer       = F_C12GP41P2VR2P0_TRX_TRAINTIMER;
    MCESD_FIELD rxTrainTimer        = F_C12GP41P2VR2P0_RX_TRAIN_TIMER;
    MCESD_FIELD trxTrainTimerEnable = F_C12GP41P2VR2P0_TRX_TRAIN_T_EN;
    MCESD_U16 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimer, &timeoutData));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &trxTrainTimerEnable, &enableData));
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxTrainTimer, &timeoutData));
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = timeoutData;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ExecuteTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_FIELD ffeForce = F_C12GP41P2VR2P0_FFE_SETTING_F;
    MCESD_U32 failed;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 0));

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_PollPin(devPtr, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, 1, 5000));
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failed));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
    }

    return (failed == 0) ? MCESD_OK : MCESD_FAIL;
}

MCESD_STATUS API_C12GP41P2V_StartTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_FIELD ffeForce = F_C12GP41P2VR2P0_FFE_SETTING_F;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeForce, 0));

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0,  0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0,          1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 1));

    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_UPDATE_DIS0,  0));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_EN0,          1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 1));    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_CheckTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U32 completeData, failedData = 0;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, type));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failedData));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 5));
            MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, type));
        }
    }

    *completed = (completeData == 0) ? MCESD_FALSE : MCESD_TRUE;
    *failed = (failedData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StopTraining(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_TRAINING type
)
{
    MCESD_U32 enableData;

    if (type == C12GP41P2V_TRAINING_TRX)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, &enableData));
        if (1 == enableData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_TX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_DFE_PAT_DIS0,     0));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        }
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, &enableData));
        if (1 == enableData)
        {
            MCESD_ATTEMPT(API_C12GP41P2V_HwSetPinCfg(devPtr, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, 0));
            MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD selMufi = F_C12GP41P2VR2P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C12GP41P2VR2P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C12GP41P2VR2P0_RX_SELMUPI;
    MCESD_FIELD selMupf = F_C12GP41P2VR2P0_RX_SELMUPF;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMufi, &data));
        break;
    case C12GP41P2V_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMuff, &data));
        break;
    case C12GP41P2V_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMupi, &data));
        break;
    case C12GP41P2V_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &selMupf, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD selMufi = F_C12GP41P2VR2P0_RX_SELMUFI;
    MCESD_FIELD selMuff = F_C12GP41P2VR2P0_RX_SELMUFF;
    MCESD_FIELD selMupi = F_C12GP41P2VR2P0_RX_SELMUPI;
    MCESD_FIELD selMupf = F_C12GP41P2VR2P0_RX_SELMUPF;

    switch (param)
    {
    case C12GP41P2V_CDR_SELMUFI:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMufi, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUFF:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMuff, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUPI:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMupi, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_CDR_SELMUPF:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selMupf, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_EN setting
)
{
    MCESD_FIELD slewRateEn = F_C12GP41P2VR2P0_SLEWRATE_EN;

    switch (setting)
    {
    case C12GP41P2V_SR_DISABLE:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewRateEn, 0));
        break;
    case C12GP41P2V_SR_ENABLE:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewRateEn, 3));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSlewRateEnable
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_SLEWRATE_EN *setting
)
{
    MCESD_FIELD slewRateEn = F_C12GP41P2VR2P0_SLEWRATE_EN;
    MCESD_U16 data;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewRateEn, &data));
    *setting = (E_C12GP41P2V_SLEWRATE_EN)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C12GP41P2VR2P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C12GP41P2VR2P0_SLEWCTRL1;

    switch (param)
    {
    case C12GP41P2V_SR_CTRL0:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewCtrl0, (MCESD_U16)paramValue));
        break;
    case C12GP41P2V_SR_CTRL1:
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &slewCtrl1, (MCESD_U16)paramValue));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSlewRateParam
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_SLEWRATE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_FIELD slewCtrl0 = F_C12GP41P2VR2P0_SLEWCTRL0;
    MCESD_FIELD slewCtrl1 = F_C12GP41P2VR2P0_SLEWCTRL1;
    MCESD_U16 data;

    switch (param)
    {
    case C12GP41P2V_SR_CTRL0:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewCtrl0, &data));
        break;
    case C12GP41P2V_SR_CTRL1:
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &slewCtrl1, &data));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    *paramValue = (MCESD_U32)data;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U32 pinValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, C12GP41P2V_PIN_SQ_DETECTED_LPF0, &pinValue));
    *squelched = (pinValue == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_16 threshold
)
{
    MCESD_FIELD sqThresh = F_C12GP41P2VR2P0_SQ_THRESH;

    if ((threshold < C12GP41P2V_SQ_THRESH_MIN) || (threshold > C12GP41P2V_SQ_THRESH_MAX))
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sqThresh, threshold + 0x10));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_16 *threshold
)
{
    MCESD_FIELD sqThresh = F_C12GP41P2VR2P0_SQ_THRESH;
    MCESD_U16 value;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &sqThresh, &value));
    if (value >= 0x10)
        value -= 0x10;

    *threshold = value;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_DATAPATH path
)
{
    MCESD_FIELD digRxToTx   = F_C12GP41P2VR2P0_DIG_RX2TX_EN;
    MCESD_FIELD digTxToRx   = F_C12GP41P2VR2P0_DIG_TX2RX_EN;
    MCESD_FIELD cmnFarend   = F_C12GP41P2VR2P0_CMN_FAREND_EN;
    MCESD_FIELD rxFarend    = F_C12GP41P2VR2P0_RX_FAREND_EN;
    MCESD_FIELD dtlSqDetEn  = F_C12GP41P2VR2P0_DTL_SQ_DET_EN;
    MCESD_FIELD anaRxPu     = F_C12GP41P2VR2P0_ANA_PU_LB;
    MCESD_FIELD anaRxPuDly  = F_C12GP41P2VR2P0_ANA_PU_LB_DLY;

    switch (path)
    {
    case C12GP41P2V_PATH_EXTERNAL:
        /* Enable C12GP41P2V_PATH_EXTERNAL */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        /* Disable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 0));
        /* Disable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 0));
        break;
    case C12GP41P2V_PATH_FAR_END_LB:
        /* Enable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 1));
        /* Disable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 0));
        break;
    case C12GP41P2V_PATH_LOCAL_ANALOG_LB:
        /* Enable C12GP41P2V_PATH_LOCAL_ANALOG_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digRxToTx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &digTxToRx, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dtlSqDetEn, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPu, 1));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &anaRxPuDly, 1));
        /* Disable C12GP41P2V_PATH_FAR_END_LB */
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cmnFarend, 0));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxFarend, 0));
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_DATAPATH *path
)
{
    MCESD_FIELD digRxToTx   = F_C12GP41P2VR2P0_DIG_RX2TX_EN;
    MCESD_FIELD digTxToRx   = F_C12GP41P2VR2P0_DIG_TX2RX_EN;
    MCESD_FIELD cmnFarend   = F_C12GP41P2VR2P0_CMN_FAREND_EN;
    MCESD_FIELD rxFrarend   = F_C12GP41P2VR2P0_RX_FAREND_EN;
    MCESD_FIELD dtlSqDetEn  = F_C12GP41P2VR2P0_DTL_SQ_DET_EN;
    MCESD_FIELD anaRxPu     = F_C12GP41P2VR2P0_ANA_PU_LB;
    MCESD_FIELD anaRxPuDly  = F_C12GP41P2VR2P0_ANA_PU_LB_DLY;
    MCESD_U16 digRxToTxData, digTxToRxData, cmnFarendData, rxFarendData, dtlSqDetEnData, anaRxPuData, anaRxPuDlyData;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &digRxToTx, &digRxToTxData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &digTxToRx, &digTxToRxData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &cmnFarend, &cmnFarendData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxFrarend, &rxFarendData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &dtlSqDetEn, &dtlSqDetEnData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxPu, &anaRxPuData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &anaRxPuDly, &anaRxPuDlyData));

    if ((digRxToTxData == 0) && (digTxToRxData == 0))
    {
        if ((0 == dtlSqDetEnData ) && (1 == anaRxPuData) && (1 == anaRxPuDlyData))
            *path = C12GP41P2V_PATH_LOCAL_ANALOG_LB;
        else
            *path = C12GP41P2V_PATH_EXTERNAL;
    }
    else if ((digRxToTxData == 1) && (digTxToRxData == 0) && (cmnFarendData == 1) && (rxFarendData == 1))
    {
        *path = C12GP41P2V_PATH_FAR_END_LB;
    }
    else
    {
        *path = C12GP41P2V_PATH_UNKNOWN;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_POLARITY txPolarity,
    IN E_C12GP41P2V_POLARITY rxPolarity
)
{
    MCESD_FIELD txdInv = F_C12GP41P2VR2P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C12GP41P2VR2P0_RXD_INV;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txdInv, txPolarity));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxdInv, rxPolarity));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_C12GP41P2V_POLARITY *txPolarity,
    OUT E_C12GP41P2V_POLARITY *rxPolarity
)
{
    MCESD_FIELD txdInv = F_C12GP41P2VR2P0_TXD_INV;
    MCESD_FIELD rxdInv = F_C12GP41P2VR2P0_RXD_INV;
    MCESD_U16 txdInvData, rxdInvData;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txdInv, &txdInvData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxdInv, &rxdInvData));

    *txPolarity = (txdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED;
    *rxPolarity = (rxdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED;
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 errors
)
{
    MCESD_FIELD addErrEn    = F_C12GP41P2VR2P0_ADD_ERR_EN;
    MCESD_FIELD addErrNum   = F_C12GP41P2VR2P0_ADD_ERR_NUM;

    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrNum, errors - 1));

    /* Toggle F_C12GP41P2VR2P0_ADD_ERR_EN 0 -> 1 to trigger error injection */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrEn, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &addErrEn, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PATTERN *txPattern,
    IN S_C12GP41P2V_PATTERN *rxPattern,
    IN E_C12GP41P2V_SATA_LONGSHORT sataLongShort,
    IN E_C12GP41P2V_SATA_INITIAL_DISPARITY sataInitialDisparity,
    IN const char *userPattern,
    IN MCESD_U8 userK
)
{
    MCESD_FIELD txPatternSel        = F_C12GP41P2VR2P0_PT_TX_PAT_SEL;
    MCESD_FIELD rxPatternSel        = F_C12GP41P2VR2P0_PT_RX_PAT_SEL;
    MCESD_FIELD prbsEnc             = F_C12GP41P2VR2P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong            = F_C12GP41P2VR2P0_PT_SATA_LONG;
    MCESD_FIELD startRd             = F_C12GP41P2VR2P0_PT_START_RD;
    MCESD_FIELD userPattern_79_64   = F_C12GP41P2VR2P0_USER_PAT_79_64;
    MCESD_FIELD userPattern_63_48   = F_C12GP41P2VR2P0_USER_PAT_63_48;
    MCESD_FIELD userPattern_47_32   = F_C12GP41P2VR2P0_USER_PAT_47_32;
    MCESD_FIELD userPattern_31_16   = F_C12GP41P2VR2P0_USER_PAT_31_16;
    MCESD_FIELD userPattern_15_0    = F_C12GP41P2VR2P0_USER_PAT_15_0;
    MCESD_FIELD userKChar           = F_C12GP41P2VR2P0_USER_K_CHAR;

    if (txPattern->pattern == C12GP41P2V_PAT_USER)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txPatternSel, (txPattern->enc8B10B == C12GP41P2V_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txPatternSel, txPattern->pattern));
    }

    if (rxPattern->pattern == C12GP41P2V_PAT_USER)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPatternSel, (rxPattern->enc8B10B == C12GP41P2V_ENC_8B10B_DISABLE) ? 1 : 2));
    }
    else
    {
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxPatternSel, rxPattern->pattern));
    }

    if (((txPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (txPattern->pattern <= C12GP41P2V_PAT_PRBS32)) ||
        ((rxPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (rxPattern->pattern <= C12GP41P2V_PAT_PRBS32)))
    {
        if ((txPattern->enc8B10B == C12GP41P2V_ENC_8B10B_ENABLE) || (rxPattern->enc8B10B == C12GP41P2V_ENC_8B10B_ENABLE))
        {
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &prbsEnc, 1));
        }
        else
            MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &prbsEnc, 0));
    }

    if ((sataLongShort == C12GP41P2V_SATA_LONG) || (sataLongShort == C12GP41P2V_SATA_SHORT))
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sataLong, sataLongShort));

    if ((sataInitialDisparity == C12GP41P2V_DISPARITY_NEGATIVE) || (sataInitialDisparity == C12GP41P2V_DISPARITY_POSITIVE))
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &startRd, sataInitialDisparity));

    if (strlen(userPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(userPattern, u8Pattern));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_79_64, MAKEU16FROMU8(u8Pattern[0], u8Pattern[1])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_63_48, MAKEU16FROMU8(u8Pattern[2], u8Pattern[3])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_47_32, MAKEU16FROMU8(u8Pattern[4], u8Pattern[5])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_31_16, MAKEU16FROMU8(u8Pattern[6], u8Pattern[7])));
        MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userPattern_15_0, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9])));
    }

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &userKChar, userK));

    if ((C12GP41P2V_PAT_JITTER_8T == txPattern->pattern) || (C12GP41P2V_PAT_JITTER_4T == txPattern->pattern))
    {
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, C12GP41P2V_DATABUS_32BIT));
    }
    else if ((C12GP41P2V_PAT_JITTER_10T == txPattern->pattern) || (C12GP41P2V_PAT_JITTER_5T == txPattern->pattern))
    {
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, C12GP41P2V_DATABUS_40BIT));
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN *txPattern,
    OUT S_C12GP41P2V_PATTERN *rxPattern,
    OUT E_C12GP41P2V_SATA_LONGSHORT *sataLongShort,
    OUT E_C12GP41P2V_SATA_INITIAL_DISPARITY *sataInitialDisparity,
    OUT char *userPattern,
    OUT MCESD_U8 *userK
)
{
    MCESD_FIELD txPatternSel    = F_C12GP41P2VR2P0_PT_TX_PAT_SEL;
    MCESD_FIELD rxPatternSel    = F_C12GP41P2VR2P0_PT_RX_PAT_SEL;
    MCESD_FIELD prbsEnc         = F_C12GP41P2VR2P0_PT_PRBS_ENC;
    MCESD_FIELD sataLong        = F_C12GP41P2VR2P0_PT_SATA_LONG;
    MCESD_FIELD startRd         = F_C12GP41P2VR2P0_PT_START_RD;
    MCESD_FIELD userKChar       = F_C12GP41P2VR2P0_USER_K_CHAR;
    MCESD_FIELD userPatternFieldArray[] = { F_C12GP41P2VR2P0_USER_PAT_79_64, F_C12GP41P2VR2P0_USER_PAT_63_48, F_C12GP41P2VR2P0_USER_PAT_47_32, F_C12GP41P2VR2P0_USER_PAT_31_16, F_C12GP41P2VR2P0_USER_PAT_15_0 };
    MCESD_32 userPatternFieldArrayCount = sizeof(userPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U16 txPatternSelData, rxPatternSelData, prbsEncData, sataLongData, startRdData, userPatternData, userKData;
    MCESD_U8 u8Pattern[10];

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &txPatternSel, &txPatternSelData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &rxPatternSel, &rxPatternSelData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &prbsEnc, &prbsEncData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &sataLong, &sataLongData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &startRd, &startRdData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &userKChar, &userKData));

    if ((txPatternSelData == 1) || (txPatternSelData == 2))
    {
        txPattern->pattern = C12GP41P2V_PAT_USER;
        txPattern->enc8B10B = (txPatternSelData == 1) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
    }
    else
    {
        txPattern->pattern = (E_C12GP41P2V_PATTERN)txPatternSelData;

        if (txPattern->pattern > C12GP41P2V_PAT_SATA_LTDP)
            txPattern->enc8B10B = C12GP41P2V_ENC_8B10B_ENABLE;
        else if ((txPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (txPattern->pattern <= C12GP41P2V_PAT_PRBS32))
            txPattern->enc8B10B = (prbsEncData == 0) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
        else
            txPattern->enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;
    }

    if ((rxPatternSelData == 1) || (rxPatternSelData == 2))
    {
        rxPattern->pattern = C12GP41P2V_PAT_USER;
        rxPattern->enc8B10B = (rxPatternSelData == 1) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
    }
    else
    {
        rxPattern->pattern = (E_C12GP41P2V_PATTERN)rxPatternSelData;

        if (rxPattern->pattern > C12GP41P2V_PAT_SATA_LTDP)
            rxPattern->enc8B10B = C12GP41P2V_ENC_8B10B_ENABLE;
        else if ((rxPattern->pattern >= C12GP41P2V_PAT_PRBS7) && (rxPattern->pattern <= C12GP41P2V_PAT_PRBS32))
            rxPattern->enc8B10B = (prbsEncData == 0) ? C12GP41P2V_ENC_8B10B_DISABLE : C12GP41P2V_ENC_8B10B_ENABLE;
        else
            rxPattern->enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;
    }

    *sataLongShort = (E_C12GP41P2V_SATA_LONGSHORT)sataLongData;
    *sataInitialDisparity = (E_C12GP41P2V_SATA_INITIAL_DISPARITY)startRdData;

    for (i = 0; i < userPatternFieldArrayCount; i++)
    {
        MCESD_FIELD userPatternField = userPatternFieldArray[i];
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &userPatternField, &userPatternData));
        u8Pattern[i * 2 + 0] = (MCESD_U8)(userPatternData >> 8);
        u8Pattern[i * 2 + 1] = (MCESD_U8)(userPatternData & 0xFF);
    }

    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, userPattern));

    *userK = (MCESD_U8)(userKData & 0xFF);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_PATTERN_STATISTICS *statistics
)
{
    MCESD_FIELD ptPass          = F_C12GP41P2VR2P0_PT_PASS;
    MCESD_FIELD ptLock          = F_C12GP41P2VR2P0_PT_LOCK;
    MCESD_FIELD ptCnt_47_32     = F_C12GP41P2VR2P0_PT_CNT_47_32;
    MCESD_FIELD ptCnt_31_16     = F_C12GP41P2VR2P0_PT_CNT_31_16;
    MCESD_FIELD ptCnt_15_0      = F_C12GP41P2VR2P0_PT_CNT_15_0;
    MCESD_FIELD errCnt_31_16    = F_C12GP41P2VR2P0_ERR_CNT_31_16;
    MCESD_FIELD errCnt_15_0     = F_C12GP41P2VR2P0_ERR_CNT_15_0;
    MCESD_U16 passData, lockData, errData_31_16, errData_15_0, data_47_32, data_31_16, data_15_0;
    E_C12GP41P2V_DATABUS_WIDTH width;
    MCESD_32 widthBit;
    MCESD_U64 frames;

    if (statistics == NULL)
        return MCESD_FAIL;

    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptPass, &passData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptLock, &lockData));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_47_32, &data_47_32));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_31_16, &data_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &ptCnt_15_0, &data_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &errCnt_31_16, &errData_31_16));
    MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, &errCnt_15_0, &errData_15_0));
    MCESD_ATTEMPT(API_C12GP41P2V_GetDataBusWidth(devPtr, &width));

    frames = ((MCESD_U64)data_47_32 << 32) + ((MCESD_U64)data_31_16 << 16) + data_15_0;

    switch (width)
    {
    case C12GP41P2V_DATABUS_16BIT:
        widthBit = 16;
        break;
    case C12GP41P2V_DATABUS_20BIT:
        widthBit = 20;
        break;
    case C12GP41P2V_DATABUS_32BIT:
        widthBit = 32;
        break;
    case C12GP41P2V_DATABUS_40BIT:
        widthBit = 40;
        break;
    default:
        widthBit = 16;
        break;
    }

    statistics->totalBits = frames * widthBit;
    statistics->totalErrorBits = ((MCESD_U64)errData_31_16 << 16) + (MCESD_U64)errData_15_0;
    statistics->pass = (passData == 0) ? MCESD_FALSE : MCESD_TRUE;
    statistics->lock = (lockData == 0) ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptCntRst = F_C12GP41P2VR2P0_PT_CNT_RST;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptCntRst, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C12GP41P2VR2P0_PT_EN;
    MCESD_FIELD ptEnMode        = F_C12GP41P2VR2P0_PT_EN_MODE;
    MCESD_FIELD ptPhyReadyForce = F_C12GP41P2VR2P0_PT_PHYREADY_F;
    MCESD_FIELD ptRst           = F_C12GP41P2VR2P0_PT_RST;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEnMode, 2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 1));

    /* Reset PHY Test */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptRst, 0));

    /* Wait 10 ms for CDR to lock... */
    MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 10));

    /* ...before starting pattern checker */
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 1));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ptEn            = F_C12GP41P2VR2P0_PT_EN;
    MCESD_FIELD ptPhyReadyForce = F_C12GP41P2VR2P0_PT_PHYREADY_F;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptPhyReadyForce, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ptEn, 0));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    OUT S_C12GP41P2V_EOM_SAVED *savedFields,
    OUT MCESD_16 *phaseOffset
)
{
    MCESD_U16 fieldData, i, eomOsStep, cur_phase;

    /* Enable Eye Monitor */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN, 1);

    /* Wait for EOM_READY */
    C12GP41P2V_POLL_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_READY, 1, 3000);

    /* Save Fields */
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_S, fieldData);
    savedFields->normUseS = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_D, fieldData);
    savedFields->normUseD = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_S, fieldData);
    savedFields->dfeF0SSM = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_D, fieldData);
    savedFields->dfeF0DSM = fieldData;

    /* Set Path */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_S, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_D, 0);

    /* Set DFE Samplers */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_SPLR_EN, 2);

    /* Set Sampler Loop Count for DFE */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_LPNUM, 0x3FF);

    /* Select EOM Clock and Phase Control Mode */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_D_EXT, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_D_FORCE, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_S_EXT, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_S_FORCE, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PH_CTRL_EXT, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PH_CTRL_FORCE, 1);

    /* Alignment Calibration */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_CDS_BYPASS, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_ALIGNSTART, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_ALIGNSTART, 0);
    C12GP41P2V_POLL_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_ALIGN_DONE, 1, 3000);

    /* UI Alignment */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_S_EXT, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_D_EXT, savedFields->dfeF0DSM);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 0);

    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_OFFSET, fieldData);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_OFF_EXT, fieldData);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_FORCE, 1);

    /* Save Fields */
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_SYNC_DIV_EN_F, fieldData);
    savedFields->syncDivEnF = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_RESET_BYPASS, fieldData);
    savedFields->resetBypass = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_BYPASS, fieldData);
    savedFields->coarseBypass = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_PHASE_MOVE_DFE, fieldData);
    savedFields->phaseMoveDfe = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_EN, fieldData);
    savedFields->coarseDfeEn = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_F0B_ADAPT_EN, fieldData);
    savedFields->f0bAdaptEn = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_CHECK_BYPASS, fieldData);
    savedFields->checkBypass = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_RESOL_ADJ_EN, fieldData);
    savedFields->resolAdjEn = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_ADJ, fieldData);
    savedFields->coarseDfeAdj = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_FINE_DFE_RESOL, fieldData);
    savedFields->fineDfeResol = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_DATA_EN, fieldData);
    savedFields->adaptDataEn = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_SLICEREN, fieldData);
    savedFields->adaptSliceren = fieldData;
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_CDR_DFE_FORCE, fieldData);
    savedFields->cdrDfeForce = fieldData;

    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SYNC_DIV_EN, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SYNC_DIV_EN_F, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SYNC_DIV_EN, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RESET_BYPASS, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_BYPASS, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PHASE_MOVE_DFE, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_EN, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_F0B_ADAPT_EN, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CHECK_BYPASS, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RESOL_ADJ_EN, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_ADJ, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FINE_DFE_RESOL, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_DATA_EN, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_SLICEREN, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDR_DFE_FORCE, 1);

    /* UI Shift Position Search */
    for (i = 0; i < C12GP41P2V_EOM_UI_MAX_LOOP; i++)
    {
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDS_DONE_ISR, 0);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SCHEME_START_F, 1);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SCHEME_START, 1);
        C12GP41P2V_POLL_FIELD(devPtr, F_C12GP41P2VR2P0_CDS_DONE_ISR, 1, 3000);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SCHEME_START, 0);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDS_DONE_ISR, 0);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_SAV_EYE_CHECK, fieldData);
        if (fieldData == 1)
        {
            MCESD_DBG_INFO("EOM UI alignment done, UI Position = %d\n", i);
            break;
        }
        else if (i == (C12GP41P2V_EOM_UI_MAX_LOOP - 1))
        {
            MCESD_DBG_INFO("EOM UI alignment Failed\n");
            return MCESD_FAIL;
        }

        /* EOM Phase UI Shift Logic */
        eomOsStep = 48;
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_OFFSET, cur_phase);
        while (eomOsStep > 0)
        {
            eomOsStep -= 1;
            cur_phase += 1;
            cur_phase = cur_phase % 32;
            C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_OFF_EXT, cur_phase);
            C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 1);
            C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 0);
        }
    }

    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_OFFSET, fieldData);
    *phaseOffset = fieldData;

    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_CDS_BYPASS, 1);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_EOM_SAVED savedFields,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset
)
{
    /* Restore Fields */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_SYNC_DIV_EN_F, savedFields.syncDivEnF);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RESET_BYPASS, savedFields.resetBypass);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_BYPASS, savedFields.coarseBypass);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PHASE_MOVE_DFE, savedFields.phaseMoveDfe);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_EN, savedFields.coarseDfeEn);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_F0B_ADAPT_EN, savedFields.f0bAdaptEn);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CHECK_BYPASS, savedFields.checkBypass);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_RESOL_ADJ_EN, savedFields.resolAdjEn);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_COARSE_DFE_ADJ, savedFields.coarseDfeAdj);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_FINE_DFE_RESOL, savedFields.fineDfeResol);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_DATA_EN, savedFields.adaptDataEn);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_ADAPT_SLICEREN, savedFields.adaptSliceren);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_CDR_DFE_FORCE, savedFields.cdrDfeForce);

    /* Restore Voltage */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_S_EXT, savedFields.dfeF0SSM);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 0);

    /* Restore Saved Fields*/
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_S, savedFields.normUseS);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_NORM_USE_D, savedFields.normUseD);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_CDS_BYPASS, 0);
    C12GP41P2V_WRITE_FIELD( devPtr, F_C12GP41P2VR2P0_SCHEME_START_F, 0 );

    /* Restore EOM Phase to 0 */
    MCESD_ATTEMPT(INT_C12GP41P2V_EOMSetPhase(devPtr, softwarePhase, 0, phaseOffset));

    /* Disable Phase Offset */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_FORCE, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_EXT, 0);

    /* Restore Phase Control Mode */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PH_CTRL_FORCE, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_PH_CTRL_EXT, 0);

    /* Select Data Clock for Slicer Sampler */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_D_FORCE, 0);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN_S_FORCE, 0);

    /* Fix a bug for PH_CTRL Logci for 16nmffc COMPHY_12G_PIPE4 */
    C12GP41P2V_WRITE_FIELD( devPtr, F_C12GP41P2VR2P0_EOM_EN_E_FORCE, 0);
    C12GP41P2V_WRITE_FIELD( devPtr, F_C12GP41P2VR2P0_PH_CTRL_EXT, 0 );
    C12GP41P2V_WRITE_FIELD( devPtr, F_C12GP41P2VR2P0_PH_CTRL_FORCE, 1 );

    /* Disable EOM Monitor */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_EN, 0);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    IN MCESD_U32 minSamples,
    IN MCESD_32 softwarePhase,
    IN MCESD_16 phaseOffset,
    OUT S_C12GP41P2V_EOM_DATA *measurement
)
{
    MCESD_U16 vldCntPData_15_0, vldCntPData_31_16, vldCntNData_15_0, vldCntNData_31_16;
    MCESD_U16 errCntPData_15_0, errCntPData_31_16, errCntNData_15_0, errCntNData_31_16;
    MCESD_U32 totalVldCntPData, totalVldCntNData;
    MCESD_U32 totalErrCntPData, totalErrCntNData;

    if (measurement == NULL)
        return MCESD_FAIL;

    /* Set phase */
    MCESD_ATTEMPT(INT_C12GP41P2V_EOMSetPhase(devPtr, softwarePhase, phase, phaseOffset));

    /* Set voltage */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_F0_S_EXT, voltage);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_FX_FORCE, 0);

    /* Clear Counters */
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_CNT_CLR, 1);
    C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_CNT_CLR, 0);

    totalVldCntPData = 0;
    totalVldCntNData = 0;
    totalErrCntPData = 0;
    totalErrCntNData = 0;

    while ((totalVldCntPData < minSamples) || (totalVldCntNData < minSamples))
    {
        /* Call DFE to collect statistics */
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_START, 1);
        C12GP41P2V_POLL_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_DONE, 1, 3000);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_START, 0);

        /* Retrieve valid counts */
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_VLDCNT_P_15_0, vldCntPData_15_0);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_VLDCNT_P_31_16, vldCntPData_31_16);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_VLDCNT_N_15_0, vldCntNData_15_0);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_VLDCNT_N_31_16, vldCntNData_31_16);

        /* Retrieve error counts */
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ERRCNT_P_15_0, errCntPData_15_0);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ERRCNT_P_31_16, errCntPData_31_16);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ERRCNT_N_15_0, errCntNData_15_0);
        C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_ERRCNT_N_31_16, errCntNData_31_16);

        totalVldCntPData += MAKEU32FROMU16(vldCntPData_31_16, vldCntPData_15_0);
        totalVldCntNData += MAKEU32FROMU16(vldCntNData_31_16, vldCntNData_15_0);
        totalErrCntPData += MAKEU32FROMU16(errCntPData_31_16, errCntPData_15_0);
        totalErrCntNData += MAKEU32FROMU16(errCntNData_31_16, errCntNData_15_0);
    }

    /* Update data structure */
    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = totalVldCntPData;
    measurement->upperBitErrorCount = totalErrCntPData;
    measurement->lowerBitCount = totalVldCntNData;
    measurement->lowerBitErrorCount = totalErrCntNData;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U16* widthLeft,
    OUT MCESD_U16* widthRight,
    OUT MCESD_U16* heightUpper,
    OUT MCESD_U16* heightLower,
    OUT MCESD_U32* sampleCount
)
{
    MCESD_32 leftEdge, rightEdge, upperEdge, lowerEdge, maxPhase, minPhase, maxVoltage, phase, voltage, phaseMidpoint;
    MCESD_U32 upperBER, lowerBER;
    MCESD_U16 phaseStepCount, voltageStepCount;
    MCESD_32 softwarePhase = 0;
    S_C12GP41P2V_EOM_DATA measurement;
    S_C12GP41P2V_EOM_SAVED savedFields;
    MCESD_16 phaseOffset;

    memset(&measurement, 0x0, sizeof(S_C12GP41P2V_EOM_DATA));

    MCESD_ATTEMPT(API_C12GP41P2V_EOM1UIStepCount(devPtr, &phaseStepCount, &voltageStepCount));

    maxPhase = phaseStepCount / 2;
    minPhase = -phaseStepCount / 2;
    maxVoltage = voltageStepCount;

    /* Set default edge values */
    leftEdge = minPhase;
    rightEdge = maxPhase;
    upperEdge = -maxVoltage;
    lowerEdge = maxVoltage - 1;

    MCESD_ATTEMPT(API_C12GP41P2V_EOMInit(devPtr, &savedFields, &phaseOffset));

    /* Scan Left */
    for (phase = 0; phase < maxPhase; phase++)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, 0, minSamples, softwarePhase, phaseOffset, &measurement));
        softwarePhase = phase;

        if (0 == phase)
        {
            *sampleCount = (MCESD_U32)measurement.upperBitCount;
        }

        /* Stop because no bits were read */
        if ((0 == measurement.upperBitCount) || (0 == measurement.lowerBitCount))
        {
            leftEdge = phase;
            break;
        }

        /* Stop because BER was too high */
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            leftEdge = phase;
            break;
        }
    }

    /* Scan Right */
    for (phase = -1; phase > minPhase; phase--)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, 0, minSamples, softwarePhase, phaseOffset, &measurement));
        softwarePhase = phase;

        /* Stop because no bits were read */
        if ((0 == measurement.upperBitCount) || (0 == measurement.lowerBitCount))
        {
            rightEdge = phase;
            break;
        }

        /* Stop because BER was too high */
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            rightEdge = phase;
            break;
        }
    }

    /* Default if either edge did not update */
    if ((leftEdge == minPhase) || (rightEdge == maxPhase))
    {
        leftEdge = maxPhase;
        rightEdge = minPhase;
    }

    phaseMidpoint = (leftEdge - rightEdge) / 2 + rightEdge;

    /* Scan up and down */
    for (voltage = 0; voltage < maxVoltage; voltage++)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phaseMidpoint, (MCESD_U8)voltage, minSamples, softwarePhase, phaseOffset, &measurement));
        softwarePhase = phaseMidpoint;

        /* Update once */
        while (upperEdge == -maxVoltage)
        {
            /* Update because no bits read */
            if (0 == measurement.upperBitCount)
            {
                upperEdge = voltage;
                break;
            }

            /* Update because BER was too high */
            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
            if (berThreshold < upperBER)
            {
                upperEdge = voltage;
            }
            break;
        }

        /* Update once */
        while (lowerEdge == (maxVoltage - 1))
        {
            /* Update because no bits read */
            if (0 == measurement.lowerBitCount)
            {
                lowerEdge = -voltage;
                break;
            }

            /* Update because BER was too high */
            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
            if (berThreshold < lowerBER)
            {
                lowerEdge = -voltage;
            }
            break;
        }

        /* Stop when both edges are found */
        if ((upperEdge != -maxVoltage) && (lowerEdge != (maxVoltage - 1)))
        {
            break;
        }
    }

    /* Default if either edge did not update */
    if ((upperEdge == -maxVoltage) || (lowerEdge == (maxVoltage - 1)))
    {
        upperEdge = maxVoltage - 1;
        lowerEdge = 1 - maxVoltage;
    }

    *widthLeft = (MCESD_16)((leftEdge == rightEdge) ? 0 : leftEdge);
    *widthRight = (MCESD_16)((leftEdge == rightEdge) ? 0 : -rightEdge);
    if (upperEdge == lowerEdge)
    {
        *heightUpper = 0;
        *heightLower = 0;
    }
    else
    {
        *heightUpper = (MCESD_U16)upperEdge;
        *heightLower = (MCESD_U16)((lowerEdge < 0) ? -lowerEdge : lowerEdge);
    }

    MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));

    return MCESD_OK;
}

static MCESD_STATUS INT_C12GP41P2V_EOMSetPhase
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_32 softwarePhase,
    IN MCESD_32 targetPhase,
    IN MCESD_16 phaseOffset
)
{
    MCESD_U16 step, rawPhase;
    MCESD_16 phase, difference;

    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_EXT, rawPhase);

    phase = softwarePhase;

    if (phase == targetPhase)
        return MCESD_OK;
    else if (phase > targetPhase)
        step = -2;
    else
        step = 2;

    do
    {
        difference = phase - targetPhase;
        if (difference < 0)
            difference *= -1;

        if (difference <= 2)
            phase = (MCESD_16)targetPhase;
        else
            phase += step;

        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_EXT, ((MCESD_U16)(phase + phaseOffset) % 32));
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 1);
        C12GP41P2V_WRITE_FIELD(devPtr, F_C12GP41P2VR2P0_EOM_PH_VALID, 0);
    } while (phase != targetPhase);

    return MCESD_OK;
}

static MCESD_STATUS INT_C12GP41P2V_CalculateBER
(
    IN MCESD_U32 bitErrorCount,
    IN MCESD_U64 bitCount,
    OUT MCESD_U32* nanoBER
)
{
    MCESD_U64 adjustedBitErrorCount = (MCESD_U64)bitErrorCount * 0x3B9ACA00;   /* 0x3B9ACA00 == 1E+9 */
    *nanoBER = (MCESD_U32)(adjustedBitErrorCount / ((MCESD_U64)bitCount));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMConvertWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 width,
    IN MCESD_U16 heightUpper,
    IN MCESD_U16 heightLower,
    OUT MCESD_U32* widthmUI,
    OUT MCESD_U32* heightmVUpper,
    OUT MCESD_U32* heightmVLower
)
{
    MCESD_U16 phaseStepCount, voltageStepCount, data;

    *widthmUI = 0;
    *heightmVUpper = 0;
    *heightmVLower = 0;

    if (0 == width)
        return MCESD_FAIL; /* Division by 0 Error */

    /* Convert width */
    MCESD_ATTEMPT(API_C12GP41P2V_EOM1UIStepCount(devPtr, &phaseStepCount, &voltageStepCount));
    *widthmUI = (MCESD_U32)width * 1000 / phaseStepCount;

    /* Convert height */
    C12GP41P2V_READ_FIELD(devPtr, F_C12GP41P2VR2P0_DFE_RES_F0, data);
    *heightmVUpper = heightUpper * (data + 2);
    *heightmVLower = heightLower * (data + 2);

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U16* phaseStepCount,
    OUT MCESD_U16* voltageStepCount
)
{
    E_C12GP41P2V_SERDES_SPEED speed;
    MCESD_U32 mbps;

    MCESD_ATTEMPT(API_C12GP41P2V_GetTxRxBitRate(devPtr, &speed));
    MCESD_ATTEMPT(INT_C12GP41P2V_SpeedGbpsToMbps(speed, &mbps));

    if (mbps >= 8000)       /* 8 Gbps <= speed */
    {
        *phaseStepCount = 128;
    }
    else if (mbps >= 4000)  /* 4 Gbps <= speed < 8 Gbps */
    {
        *phaseStepCount = 256;
    }
    else if (mbps >= 2000)  /* 2 Gbps <= speed < 4 Gbps */
    {
        *phaseStepCount = 512;
    }
    else                    /* 0 Gbps <= speed < 2 Gbps */
    {
        *phaseStepCount = 1024;
    }

    *voltageStepCount = 64;

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_SpeedGbpsToMbps
(
    IN E_C12GP41P2V_SERDES_SPEED speed,
    OUT MCESD_U32* mbps
)
{
    *mbps = 0;

    switch (speed)
    {
    case C12GP41P2V_SERDES_1P125G:          /* 1.125 Gbps */
        *mbps = 1125;
        break;
    case C12GP41P2V_SERDES_1P25G:           /* 1.25 Gbps */
        *mbps = 1250;
        break;
    case C12GP41P2V_SERDES_1P5G:            /* 1.5 Gbps */
        *mbps = 1500;
        break;
    case C12GP41P2V_SERDES_2P5G:            /* 2.5 Gbps */
        *mbps = 2500;
        break;
    case C12GP41P2V_SERDES_2P578125G:       /* 2.578125 Gbps */
        *mbps = 2578;
        break;
    case C12GP41P2V_SERDES_3G:              /* 3.0 Gbps */
        *mbps = 3000;
        break;
    case C12GP41P2V_SERDES_3P125G:          /* 3.125 Gbps */
        *mbps = 3125;
        break;
    case C12GP41P2V_SERDES_3P75G:           /* 3.75 Gbps */
        *mbps = 3750;
        break;
    case C12GP41P2V_SERDES_4G:              /* 4.0 Gbps */
        *mbps = 4000;
        break;
    case C12GP41P2V_SERDES_4P25G:           /* 4.25 Gbps */
        *mbps = 4250;
        break;
    case C12GP41P2V_SERDES_5G:              /* 5.0 Gbps */
        *mbps = 5000;
        break;
    case C12GP41P2V_SERDES_5P15625G:        /* 5.15625 Gbps */
        *mbps = 5156;
        break;
    case C12GP41P2V_SERDES_6G:              /* 6.0 Gbps */
        *mbps = 6000;
        break;
    case C12GP41P2V_SERDES_6P25G:           /* 6.25 Gbps */
        *mbps = 6250;
        break;
    case C12GP41P2V_SERDES_7P5G:            /* 7.5 Gbps */
        *mbps = 7500;
        break;
    case C12GP41P2V_SERDES_10G:             /* 10.0 Gbps */
        *mbps = 10000;
        break;
    case C12GP41P2V_SERDES_10P3125G:        /* 10.3125 Gbps */
        *mbps = 10312;
        break;
    case C12GP41P2V_SERDES_11P5625G:        /* 11.5625 Gbps */
        *mbps = 11562;
        break;
    case C12GP41P2V_SERDES_12P375G:         /* 12.375 Gbps */
        *mbps = 12375;
        break;
    case C12GP41P2V_SERDES_12P5G:           /* 12.5 Gbps */
        *mbps = 12500;
        break;
    default:
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
CPSS_ADDITION_START
    IN MCESD_U32 phaseStepSize,
    IN MCESD_U32 voltageStepSize,
CPSS_ADDITION_END
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    INOUT S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr
)
{
    S_C12GP41P2V_EOM_DATA measurement;
    MCESD_U32 leftEdgeIdx, rightEdgeIdx, upperEdgeIdx, lowerEdgeIdx, phaseCenterIdx, voltageCenterIdx, upperBer, lowerBer;
    MCESD_32 upperEdge, lowerEdge, leftMaxEdge, rightMaxEdge, upperMaxEdge, lowerMaxEdge, phase, voltage, upperVoltage, lowerVoltage, leftInnerEdge, rightInnerEdge;
    MCESD_U16 phaseStepCount, voltageStepCount;
    MCESD_32 softwarePhase = 0;
    MCESD_16 phaseOffset;
    S_C12GP41P2V_EOM_SAVED savedFields;

    memset(&measurement, 0x0, sizeof(S_C12GP41P2V_EOM_DATA));

    MCESD_ATTEMPT(API_C12GP41P2V_EOM1UIStepCount(devPtr, &phaseStepCount, &voltageStepCount));

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    eyeRawDataPtr->oneUIwidth = phaseStepCount;

    phaseCenterIdx = C12GP41P2V_EYE_MAX_PHASE_LEVEL / 2;
    voltageCenterIdx = voltageStepCount - 1;
    leftEdgeIdx = phaseCenterIdx - (phaseStepCount / 2);
    rightEdgeIdx = phaseCenterIdx + (phaseStepCount / 2) - 1;
    upperEdgeIdx = voltageCenterIdx - (voltageStepCount - 1);
    lowerEdgeIdx = voltageCenterIdx + (voltageStepCount - 1);

    leftMaxEdge = phaseStepCount / 2;
    rightMaxEdge = 1 - (phaseStepCount / 2);
    upperMaxEdge = voltageStepCount - 1;
    lowerMaxEdge = 1 - voltageStepCount;
    upperEdge = 0;
    lowerEdge = 0;

    MCESD_ATTEMPT(API_C12GP41P2V_EOMInit(devPtr, &savedFields, &phaseOffset));

    /* Check (0, 0) */
    MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, 0, 0, minSamples, softwarePhase, phaseOffset, &measurement));
    eyeRawDataPtr->sampleCount = (MCESD_U32)measurement.upperBitCount;

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
    MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
    if ((berThreshold < upperBer) || (berThreshold < lowerBer))
    {
        MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMGetEyeData: BER > berThreshold at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Left Sweep from Phase 0 with Voltage Sweep */
    leftInnerEdge = leftMaxEdge;
CPSS_ADDITION_START
    for (phase = 0; phase <= leftMaxEdge; phase += phaseStepSize)
CPSS_ADDITION_END
    {
        upperVoltage = lowerMaxEdge;
        lowerVoltage = upperMaxEdge;
CPSS_ADDITION_START
        for (voltage = 0; voltage <= upperMaxEdge; voltage += voltageStepSize)
CPSS_ADDITION_END
        {
            MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, minSamples, softwarePhase, phaseOffset, &measurement));
            softwarePhase = phase;

            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx + voltage] = measurement.lowerBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx - voltage] = measurement.upperBitErrorCount;

            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
            if ((berThreshold < upperBer) || (berThreshold < lowerBer))
            {
                if ((upperVoltage == lowerMaxEdge) && ((0 == measurement.upperBitCount) || (berThreshold < upperBer)))
                    upperVoltage = voltage; /* found upper edge */

                if ((lowerVoltage == upperMaxEdge) && ((0 == measurement.lowerBitCount) || (berThreshold < lowerBer)))
                    lowerVoltage = -voltage; /* found lower edge */
            }

            if ((phase < leftInnerEdge) && ((0 == upperVoltage) || (0 == lowerVoltage)))
                leftInnerEdge = phase;

            if (((upperVoltage != lowerMaxEdge) && (lowerVoltage != upperMaxEdge)))
                break;
        }

        if (upperVoltage > upperEdge)
            upperEdge = upperVoltage;

        if (lowerVoltage < lowerEdge)
            lowerEdge = lowerVoltage;

        /* Stop when (phase, 0) exceeds BER threshold */
        if ((upperVoltage == 0) && (lowerVoltage == 0))
        {
            leftEdgeIdx = phaseCenterIdx - phase;
            break;
        }
    }

    /* Right Sweep from Phase -1 with Voltage Sweep */
    rightInnerEdge = rightMaxEdge;
CPSS_ADDITION_START
    for (phase = -1; phase >= rightMaxEdge; phase -= phaseStepSize)
CPSS_ADDITION_END
    {
        upperVoltage = lowerMaxEdge;
        lowerVoltage = upperMaxEdge;
CPSS_ADDITION_START
        for (voltage = 0; voltage <= upperMaxEdge; voltage += voltageStepSize)
CPSS_ADDITION_END
        {
            MCESD_ATTEMPT(API_C12GP41P2V_EOMMeasPoint(devPtr, phase, (MCESD_U8)voltage, minSamples, softwarePhase, phaseOffset, &measurement));
            softwarePhase = phase;

            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx + voltage] = measurement.lowerBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx - voltage] = measurement.upperBitErrorCount;

            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
            MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
            if ((berThreshold < upperBer) || (berThreshold < lowerBer))
            {
                if ((upperVoltage == lowerMaxEdge) && ((0 == measurement.upperBitCount) || (berThreshold < upperBer)))
                    upperVoltage = voltage; /* found upper edge */

                if ((lowerVoltage == upperMaxEdge) && ((0 == measurement.lowerBitCount) || (berThreshold < lowerBer)))
                    lowerVoltage = -voltage; /* found lower edge */
            }

            if ((phase > rightInnerEdge) && ((0 == upperVoltage) || (0 == lowerVoltage)))
                rightInnerEdge = phase;

            if (((upperVoltage != lowerMaxEdge) && (lowerVoltage != upperMaxEdge)))
                break;
        }

        if (upperVoltage > upperEdge)
            upperEdge = upperVoltage;

        if (lowerVoltage < lowerEdge)
            lowerEdge = lowerVoltage;

        /* Stop when (phase, 0) exceeds BER threshold */
        if ((upperVoltage == 0) && (lowerVoltage == 0))
        {
            rightEdgeIdx = phaseCenterIdx - phase;
            break;
        }
    }

    eyeRawDataPtr->relativeCenter = phaseCenterIdx - ((leftInnerEdge - rightInnerEdge) / 2 + rightInnerEdge);
    upperEdgeIdx = voltageCenterIdx - upperEdge;
    lowerEdgeIdx = voltageCenterIdx - lowerEdge;

    MCESD_ATTEMPT(API_C12GP41P2V_EOMFinalize(devPtr, savedFields, softwarePhase, phaseOffset));

    eyeRawDataPtr->leftEdge = leftEdgeIdx;
    eyeRawDataPtr->rightEdge = rightEdgeIdx;
    eyeRawDataPtr->upperEdge = upperEdgeIdx;
    eyeRawDataPtr->lowerEdge = lowerEdgeIdx;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EOMPlotEyeData
(
    IN S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 phaseStepSize,
    IN MCESD_U32 voltageStepSize,
    IN MCESD_U32 berThreshold,
    IN MCESD_U32 berThresholdMax
)
{
    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_C12GP41P2V_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(plotEyeData(eyeRawDataPtr->eyeRawData[0], eyeRawDataPtr->leftEdge, eyeRawDataPtr->rightEdge, C12GP41P2V_EYE_MAX_PHASE_LEVEL / 2, phaseStepSize,
        eyeRawDataPtr->upperEdge, eyeRawDataPtr->lowerEdge, C12GP41P2V_EYE_MAX_VOLT_STEPS - 1, voltageStepSize, eyeRawDataPtr->sampleCount, (C12GP41P2V_EYE_MAX_VOLT_STEPS * 2) - 1,
        berThreshold, berThresholdMax, NULL, 1));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_EyeDataDimensions
(
    IN S_C12GP41P2V_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U32* width,
    OUT MCESD_U32* heightUpper,
    OUT MCESD_U32* heightLower
)
{
    MCESD_U32 upperBER, lowerBER, phaseIndex, phaseCenter, leftIndex, rightIndex, voltageCenter, upperIndex, lowerIndex;
    MCESD_32 voltageIndex;
    phaseCenter = C12GP41P2V_EYE_MAX_PHASE_LEVEL / 2;
    leftIndex = eyeRawDataPtr->leftEdge;
    rightIndex = eyeRawDataPtr->rightEdge;
    voltageCenter = C12GP41P2V_EYE_MAX_VOLT_STEPS - 1;
    upperIndex = eyeRawDataPtr->upperEdge;
    lowerIndex = eyeRawDataPtr->lowerEdge;

    /* Sweep from Center Phase to Left Edge */
    for (phaseIndex = phaseCenter; phaseIndex >= (MCESD_U32)eyeRawDataPtr->leftEdge; phaseIndex--)
    {
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter], eyeRawDataPtr->sampleCount, &upperBER));
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter + 1], eyeRawDataPtr->sampleCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            leftIndex = phaseIndex;
            break;
        }
    }

    /* Sweep from Center Phase to Right Edge */
    for (phaseIndex = phaseCenter; phaseIndex <= (MCESD_U32)eyeRawDataPtr->rightEdge; phaseIndex++)
    {
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter], eyeRawDataPtr->sampleCount, &upperBER));
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter + 1], eyeRawDataPtr->sampleCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            rightIndex = phaseIndex;
            break;
        }
    }

    phaseIndex = leftIndex + (rightIndex - leftIndex) / 2;

    /* Sweep from Center Voltage to Upper Edge */
    for (voltageIndex = voltageCenter; voltageIndex >= ((MCESD_32)eyeRawDataPtr->upperEdge); voltageIndex--)
    {
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex], eyeRawDataPtr->sampleCount, &upperBER));
        if (berThreshold < upperBER)
        {
            upperIndex = voltageIndex;
            break;
        }
    }

    /* Sweep from Center Voltage to Lower Edge */
    for (voltageIndex = voltageCenter; voltageIndex <= ((MCESD_32)eyeRawDataPtr->lowerEdge); voltageIndex++)
    {
        MCESD_ATTEMPT(INT_C12GP41P2V_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex], eyeRawDataPtr->sampleCount, &lowerBER));
        if (berThreshold < lowerBER)
        {
            lowerIndex = voltageIndex;
            break;
        }
    }

    *width = rightIndex - leftIndex;
    *heightUpper = voltageCenter - upperIndex;
    *heightLower = lowerIndex - voltageCenter;

    return MCESD_OK;
}
#endif /* C12GP41P2V */
