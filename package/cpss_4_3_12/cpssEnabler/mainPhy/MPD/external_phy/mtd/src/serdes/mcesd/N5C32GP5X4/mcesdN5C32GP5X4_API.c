/*******************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
5FFP_COMPHY_32G_PIPE5_X4_4PLL
********************************************************************/
#include "mcesdTop.h"
#include "mcesdApiTypes.h"
#include "mcesdUtils.h"
#include "mcesdN5C32GP5X4_Defs.h"
#include "mcesdN5C32GP5X4_API.h"
#include "mcesdN5C32GP5X4_RegRW.h"
#include "mcesdN5C32GP5X4_HwCntl.h"
#include <string.h>

#ifdef N5C32GP5X4

/* Forward internal function prototypes used only in this module */
static MCESD_U32 INT_N5C32GP5X4_ComputeTxEqEmMain(IN MCESD_U32 pre3Cursor, IN MCESD_U32 pre2Cursor, IN MCESD_U32 preCursor, IN MCESD_U32 postCursor, IN MCESD_U32 naCursor);
static MCESD_U32 INT_N5C32GP5X4_Computef0(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane);
static MCESD_STATUS INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, IN MCESD_FIELD_PTR fieldPtr, OUT MCESD_32 *tapValue);
static MCESD_STATUS INT_N5C32GP5X4_CalculateBER(IN MCESD_U32 bitErrorCount, IN MCESD_U64 bitCount, OUT MCESD_U32 *nanoBER);
static MCESD_STATUS INT_N5C32GP5X4_GetDfeF0(IN MCESD_DEV_PTR devPtr, IN MCESD_U8 lane, INOUT MCESD_U32* table);

MCESD_STATUS API_N5C32GP5X4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_FW_MAJOR_VER, 255 /* ignored */, data);
    *major = (MCESD_U8)data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_FW_MINOR_VER, 255 /* ignored */, data);
    *minor = (MCESD_U8)data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_FW_PATCH_VER, 255 /* ignored */, data);
    *patch = (MCESD_U8)data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_FW_BUILD_VER, 255 /* ignored */, data);
    *build = (MCESD_U8)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *tsLocked,
    OUT MCESD_BOOL *rsLocked
)
{
    MCESD_U32 txData, rxData;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_ANA_PLL_TS_LOCK_RD, lane, txData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_ANA_PLL_RS_LOCK_RD, lane, rxData);
    *tsLocked = (MCESD_BOOL)txData;
    *rsLocked = (MCESD_BOOL)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL* txReady,
    OUT MCESD_BOOL* rxReady
)
{
    MCESD_U32 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_TX0, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_RX0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_TX1, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_RX1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_TX2, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_RX2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_TX3, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PLL_READY_RX3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txReady = (MCESD_BOOL)txData;
    *rxReady = (MCESD_BOOL)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 timeout
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT0, 1));
        MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_INITDON0, 1, timeout));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT0, 0));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT1, 1));
        MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_INITDON1, 1, timeout));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT1, 0));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT2, 1));
        MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_INITDON2, 1, timeout));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT2, 0));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT3, 1));
        MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_INITDON3, 1, timeout));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_INIT3, 0));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TXEQ_PARAM param,
    IN MCESD_32 paramValue
)
{
    MCESD_U32 data, position, paramUnsignedValue;
    MCESD_32 paramPol;

    if (paramValue < 0)
    {
        paramUnsignedValue = -1 * paramValue;
        paramPol = 0;       /* Normal Polarity */
    }
    else
    {
        paramUnsignedValue = paramValue;
        paramPol = 1;       /* Inverted Polarity */
    }

    /* Set Polarity */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_TAP_POL_F, lane, 1);
    position = 1 + param;                                           /* N5C32GP5X4_TXEQ_EM_PRE2 1, N5C32GP5X4_TXEQ_EM_PRE 2, ... */
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_TAP_POL, lane, data);
    data = ((data & ~(1 << position)) | (paramPol << position));    /* Modify bit at position with polarity */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_TAP_POL, lane, data);

    switch (param)
    {
    case N5C32GP5X4_TXEQ_EM_PRE2:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C1_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C1, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_PRE:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C2_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C2, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_MAIN:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_POST:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C4_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C4, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_NA:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C5_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C5, lane, paramUnsignedValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 1);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 0);
    
    /* The TX emphasis main is automatically calculated and updated unless user explicitly wants to manually set it */
    if (param != N5C32GP5X4_TXEQ_EM_MAIN)
    {
        MCESD_U32 txEmPre3Data, txEmPre2Data, txEmPreData, txEmMainData, txEmPostData, txEmNAData;

        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C0, lane, txEmPre3Data);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C1, lane, txEmPre2Data);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C2, lane, txEmPreData);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C4, lane, txEmPostData);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C5, lane, txEmNAData);

        /* Calculate Tx emphasis Main based on full_swing setting */
        txEmMainData = INT_N5C32GP5X4_ComputeTxEqEmMain(txEmPre3Data, txEmPre2Data, txEmPreData, txEmPostData, txEmNAData);

        /* Update Tx emphasis Main */
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3_FORCE, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3, lane, txEmMainData);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 1);
        MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 0);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TXEQ_PARAM param,
    OUT MCESD_32 *paramValue
)
{
    MCESD_U32 data, position, paramUnsignedValue;
    MCESD_32 paramSign;

    switch (param)
    {
    case N5C32GP5X4_TXEQ_EM_PRE2:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C1, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_PRE:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C2, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_MAIN:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C3, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_POST:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C4, lane, paramUnsignedValue);
        break;
    case N5C32GP5X4_TXEQ_EM_NA:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_C5, lane, paramUnsignedValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    /* Get Polarity */
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TO_ANA_TX_FIR_POL, lane, data);
    position = 1 + param;       /* N5C32GP5X4_TXEQ_EM_PRE2 1, N5C32GP5X4_TXEQ_EM_PRE 2, ... */
    paramSign = ((data >> position) & 0x1) == 0 ? /* normal */ -1 : /* inverted */ 1;

    *paramValue = paramSign * paramUnsignedValue;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxEqAll
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_32 pre2,
    IN MCESD_32 pre,
    IN MCESD_32 main,
    IN MCESD_32 post
)
{
    MCESD_U32 data, pre2Value, preValue, mainValue, postValue;

    /* Set Polarity */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_TAP_POL_F, lane, 1);
    data = (pre2 >= 0) << 1;
    data += (pre >= 0) << 2;
    data += (main >= 0) << 3;
    data += (post >= 0) << 4;
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_TAP_POL, lane, data);

    /* Calculate absolute values */
    pre2Value = (pre2 < 0) ? -pre2 : pre2;
    preValue = (pre < 0) ? -pre : pre;
    postValue = (post < 0) ? -post : post;
    if (0xFF == main)       /* Defaults main to be readjusted so sum is 63 */
    {
        mainValue = INT_N5C32GP5X4_ComputeTxEqEmMain(0, pre2Value, preValue, postValue, 0);
    }
    else
    {
        mainValue = (main < 0) ? -main : main;
    }

    /* Set all tap values */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 0);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C0_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C1_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C2_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C4_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C5_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C0, lane, 0);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C1, lane, pre2Value);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C2, lane, preValue);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C3, lane, mainValue);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C4, lane, postValue);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_FIR_C5, lane, 0);

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP_FORCE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 1);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case N5C32GP5X4_CTLE_CUR1_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_CURRENT1_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RL1_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL1_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RL1_EXTRA:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL1_EXTRA, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RES1_SEL: 
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RES1_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_CAP1_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_CAP1_SEL_G, lane, ConvertU32ToGrayCode(paramValue));
        break;
    case N5C32GP5X4_CTLE_EN_MIDFREQ:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_EN_CTLE_MID_FREQ, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_CS1_MID:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_CS1_MID, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RS1_MID:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RS1_MID, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_CUR2_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_CURRENT2_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RL2_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL2_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_RL2_TUNE_G:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL2_TUNE_G, lane, ConvertU32ToGrayCode(paramValue));
        break;
    case N5C32GP5X4_CTLE_RES2_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_RES2_SEL, lane, paramValue);
        break;
    case N5C32GP5X4_CTLE_CAP2_SEL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CTLE_CAP2_SEL, lane, paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    MCESD_U32 grayCode;

    switch (param)
    {
    case N5C32GP5X4_CTLE_CUR1_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_CURRENT1_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RL1_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL1_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RL1_EXTRA:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL1_EXTRA, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RES1_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RES1_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_CAP1_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_CAP1_SEL_G, lane, grayCode);
        *paramValue = ConvertGrayCodeToU32(grayCode);
        break;
    case N5C32GP5X4_CTLE_EN_MIDFREQ:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EN_CTLE_MID_FREQ, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_CS1_MID:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_CS1_MID, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RS1_MID:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RS1_MID, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_CUR2_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_CURRENT2_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RL2_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL2_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_RL2_TUNE_G:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RL2_TUNE_G, lane, grayCode);
        *paramValue = ConvertGrayCodeToU32(grayCode);
        break;
    case N5C32GP5X4_CTLE_RES2_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_RES2_SEL, lane, *paramValue);
        break;
    case N5C32GP5X4_CTLE_CAP2_SEL:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_CTLE_CAP2_SEL, lane, *paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_U32 INT_N5C32GP5X4_ComputeTxEqEmMain
(
    IN MCESD_U32 pre3Cursor,
    IN MCESD_U32 pre2Cursor,
    IN MCESD_U32 preCursor,
    IN MCESD_U32 postCursor,
    IN MCESD_U32 naCursor
)
{
    const MCESD_U32 fullSwing = 15;
    MCESD_U32 sum = pre3Cursor + pre2Cursor + preCursor + postCursor + naCursor;

    if (fullSwing >= sum)
        return fullSwing - sum;
    else
        return 0;
}

MCESD_STATUS API_N5C32GP5X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_EN, lane, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_EN, lane, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_UPDATE_DIS, lane, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_UPDATE_DIS, lane, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
)
{
    switch (tap)
    {
    case N5C32GP5X4_DFE_DC:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_DC_D_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_DC_E:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_DC_E_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F0:
        {
            MCESD_U32 data;

            N5C32GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD((MCESD_FIELD)F_N5C32GP5X4_DFE_F0_D_E_SM), lane, data);
            *tapValue = data * 1000; /* F0 tap is not signed. Unlike other DFE taps, we do not treat it like signed magnitude */
        }
        break;
    case N5C32GP5X4_DFE_F1:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F1_D_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F1P5:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F1P5_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F2:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F2_D_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F3:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F3_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F4:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F4_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F5:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F5_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F6:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F6_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F7:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F7_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F8:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F8_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F9:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F9_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F10:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F10_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F11:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F11_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F12:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F12_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F13:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F13_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F14:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F14_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_F15:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_F15_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF0:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF0_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF1:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF1_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF2:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF2_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF3:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF3_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF4:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF4_E_SM), tapValue));
        break;
    case N5C32GP5X4_DFE_FF5:
        MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes(devPtr, lane, &((MCESD_FIELD)F_N5C32GP5X4_DFE_FF5_E_SM), tapValue));
        break;
    default:
        return MCESD_FAIL; /* Invalid tap */
    }

    return MCESD_OK;
}

MCESD_STATUS INT_N5C32GP5X4_GetDfeTap_ConvertToMilliCodes
(
    IN MCESD_DEV_PTR devPtr, 
    IN MCESD_U8 lane, 
    IN MCESD_FIELD_PTR fieldPtr, 
    OUT MCESD_32 *tapValue
)
{
    MCESD_U32 code;

    MCESD_ATTEMPT(API_N5C32GP5X4_ReadField(devPtr, lane, fieldPtr, &code));
    *tapValue = ConvertSignedMagnitudeToI32(code, fieldPtr->totalBits) * 1000; /* milli-Codes */
    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_BROADCAST, 255 /* ignored*/, state);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_BROADCAST, 255 /* ignored*/, data);
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
)
{
    MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_IVREF, state));
#ifdef N5C32GP5X4_ISOLATION
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PU_IVREF_FM_REG, 255 /* ignored*/, 1);
#endif

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_IVREF, &data));
    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_TX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_RX3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE0, state ? 0 : 1));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE1, state ? 0 : 1));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE2, state ? 0 : 1));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE3, state ? 0 : 1));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_IDLE3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (0 == data) ? MCESD_TRUE : MCESD_FALSE;    /* inverted */

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL0, state));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL1, state));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL2, state));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL3, state));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
)
{
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PU_PLL3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *state = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5C32GP5X4_PHYMODE mode
)
{
    MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_MODE, mode));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_N5C32GP5X4_PHYMODE *mode
)
{
    MCESD_U32 data;

    MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_MODE, &data));
    *mode = (E_N5C32GP5X4_PHYMODE)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_REFFREQ txFreq,
    IN E_N5C32GP5X4_REFFREQ rxFreq,
    IN E_N5C32GP5X4_REFCLK_SEL txClkSel,
    IN E_N5C32GP5X4_REFCLK_SEL rxClkSel
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX0, txFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX0, rxFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX0, txClkSel));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX0, rxClkSel));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX1, txFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX1, rxFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX1, txClkSel));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX1, rxClkSel));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX2, txFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX2, rxFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX2, txClkSel));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX2, rxClkSel));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX3, txFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX3, rxFreq));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX3, txClkSel));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX3, rxClkSel));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_REFFREQ *txFreq,
    OUT E_N5C32GP5X4_REFFREQ *rxFreq,
    OUT E_N5C32GP5X4_REFCLK_SEL *txClkSel,
    OUT E_N5C32GP5X4_REFCLK_SEL *rxClkSel
)
{
    MCESD_U32 txFreqData, rxFreqData, txClkSelData, rxClkSelData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX0, &txFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX0, &rxFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX0, &txClkSelData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX0, &rxClkSelData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX1, &txFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX1, &rxFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX1, &txClkSelData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX1, &rxClkSelData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX2, &txFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX2, &rxFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX2, &txClkSelData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX2, &rxClkSelData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_TX3, &txFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFFREF_RX3, &rxFreqData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_TX3, &txClkSelData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_REFCLK_RX3, &rxClkSelData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txFreq = (E_N5C32GP5X4_REFFREQ)txFreqData;
    *rxFreq = (E_N5C32GP5X4_REFFREQ)rxFreqData;
    *txClkSel = (E_N5C32GP5X4_REFCLK_SEL)txClkSelData;
    *rxClkSel = (E_N5C32GP5X4_REFCLK_SEL)rxClkSelData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_SERDES_SPEED txSpeed,
    IN E_N5C32GP5X4_SERDES_SPEED rxSpeed
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX0, txSpeed));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX0, rxSpeed));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX1, txSpeed));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX1, rxSpeed));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX2, txSpeed));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX2, rxSpeed));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX3, txSpeed));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX3, rxSpeed));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_SERDES_SPEED *txSpeed,
    OUT E_N5C32GP5X4_SERDES_SPEED *rxSpeed
)
{
    MCESD_U32 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX0, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX1, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX2, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_TX3, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_PHY_GEN_RX3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txSpeed = (E_N5C32GP5X4_SERDES_SPEED)txData;
    *rxSpeed = (E_N5C32GP5X4_SERDES_SPEED)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_DATABUS_WIDTH txWidth,
    IN E_N5C32GP5X4_DATABUS_WIDTH rxWidth
)
{
    MCESD_U32 txDataSelBits = 0;
    MCESD_U32 rxDataSelBits = 0;

    switch (txWidth)
    {
    case N5C32GP5X4_DATABUS_40BIT:
    {
        txDataSelBits = 0;
    }
    break;
    case N5C32GP5X4_DATABUS_32BIT:
    {
        txDataSelBits = 1;
    }
    break;
    default:
        break;
    }

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_SEL_BITS, lane, txDataSelBits);

    switch (rxWidth)
    {
    case N5C32GP5X4_DATABUS_40BIT:
        {
            rxDataSelBits = 0;
        }
        break;
    case N5C32GP5X4_DATABUS_32BIT:
        {
            rxDataSelBits = 1;
        }
        break;
    default:
        break;
    }

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_SEL_BITS, lane, rxDataSelBits);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_DATABUS_WIDTH *txWidth,
    OUT E_N5C32GP5X4_DATABUS_WIDTH *rxWidth
)
{
    MCESD_U32 txDataSelBits, rxDataSelBits;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TX_SEL_BITS, lane, txDataSelBits);

    if (0 == txDataSelBits)
        *txWidth = N5C32GP5X4_DATABUS_40BIT;
    else
        *txWidth = N5C32GP5X4_DATABUS_32BIT;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_SEL_BITS, lane, rxDataSelBits);

    if (0 == rxDataSelBits)
        *rxWidth = N5C32GP5X4_DATABUS_40BIT;
    else
        *rxWidth = N5C32GP5X4_DATABUS_32BIT;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
)
{
    MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *clockMHz
)
{
    MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_MCU_CLK, clockMHz));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PH_OS_DAT, lane, align90);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_PH_OS_DAT, lane, data);
    *align90 = (MCESD_U16)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type,
    IN S_N5C32GP5X4_TRAIN_TIMEOUT *training
)
{
    if (training == NULL)
        return MCESD_FAIL;

    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TRX_TRAIN_TIMEOUT, lane, training->enable);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TRX_TIMER, lane, training->timeout);
    }
    else
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TRX_TRAIN_TIMEOUT, lane, training->enable);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_TIMER, lane, training->timeout);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type,
    OUT S_N5C32GP5X4_TRAIN_TIMEOUT *training
)
{
    MCESD_U32 enableData, timeoutData;

    if (training == NULL)
        return MCESD_FAIL;

    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TRX_TRAIN_TIMEOUT, lane, enableData);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TRX_TIMER, lane, timeoutData);
    }
    else
    {
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TRX_TRAIN_TIMEOUT, lane, enableData);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_TIMER, lane, timeoutData);
    }

    training->enable = (MCESD_BOOL)enableData;
    training->timeout = (MCESD_U16)timeoutData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type
)
{
    MCESD_U32 failed;

    MCESD_ATTEMPT(API_N5C32GP5X4_StartTraining(devPtr, lane, type));
    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_TX_TRAINCO0, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA0, &failed));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_TX_TRAINCO1, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA1, &failed));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_TX_TRAINCO2, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA2, &failed));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_TX_TRAINCO3, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA3, &failed));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_TRAINCO0, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA0, &failed));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_TRAINCO1, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA1, &failed));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_TRAINCO2, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA2, &failed));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_PollPin(devPtr, N5C32GP5X4_PIN_RX_TRAINCO3, 1, 3000));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA3, &failed));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));

    return (0 == failed) ? MCESD_OK : MCESD_FAIL;
}

MCESD_STATUS API_N5C32GP5X4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_SAT_EN, lane, 0);
    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN0, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN0, 1));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN1, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN2, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN3, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN0, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN0, 1));
            break;
        case 1:

            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN1, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN1, 1));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN2, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN2, 1));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN3, 0));
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN3, 1));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
)
{
    MCESD_U32 completeData, failedData = 0;

    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINCO0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA0, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINCO1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA1, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINCO2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA2, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINCO3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINFA3, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINCO0, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA0, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINCO1, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA1, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINCO2, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA2, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINCO3, &completeData));
            if (1 == completeData)
            {
                MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINFA3, &failedData));
                MCESD_ATTEMPT(API_N5C32GP5X4_StopTraining(devPtr, lane, type));
            }
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    *completed = (MCESD_BOOL)completeData;
    *failed = (MCESD_BOOL)failedData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_TRAINING type
)
{
    if (type == N5C32GP5X4_TRAINING_TRX)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_TRAINEN3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    else
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN0, 0));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN1, 0));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN2, 0));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_TRAINEN3, 0));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5C32GP5X4_TRAINED_EYE_H *trainedEyeHeight
)
{
    MCESD_U32 prevState, f0xData, f0bData, f0n1Data, f0n2Data, f0dData;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_ADAPT_CONT, lane, prevState);

    /* turn off DFE continuous */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_CMD, lane, 0x18);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_ARGS, lane, 0x0);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    
    /* f0b */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_CMD, lane, 0x19);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_ARGS, lane, 0x4000);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    f0bData = INT_N5C32GP5X4_Computef0(devPtr, lane);

    /* fn1 */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_ARGS, lane, 0x8000);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_F0X_SEL, lane, 0x11);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    f0xData = INT_N5C32GP5X4_Computef0(devPtr, lane);
    f0n1Data = f0bData - f0xData;

    /* fn2 */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_F0X_SEL, lane, 0x12);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    f0xData = INT_N5C32GP5X4_Computef0(devPtr, lane);
    f0n2Data = f0bData - f0xData;

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_LOAD_EN, lane, 0);
    
    /* f0d */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_ARGS, lane, 0x20000);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    f0dData = INT_N5C32GP5X4_Computef0(devPtr, lane);

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DFE_LOAD_EN, lane, 1);

    if (prevState)
    {
        /* turn on DFE continuous */
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_CMD, lane, 0x18);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_ARGS, lane, 0x1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_CLI_START, lane, 1);
    }

    trainedEyeHeight->f0b = f0bData;
    trainedEyeHeight->f0n1 = f0n1Data;
    trainedEyeHeight->f0n2 = f0n2Data;
    trainedEyeHeight->f0d = f0dData;

    return MCESD_OK;
}

MCESD_U32 INT_N5C32GP5X4_Computef0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    MCESD_U32 data;
    double sum;

    sum = 0;
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_F0_D_O_2C, lane, data);
    sum += data;
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_F0_S_O_2C, lane, data);
    sum += data;
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_F0_D_E_2C, lane, data);
    sum += data;
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_F0_S_E_2C, lane, data);
    sum += data;

    /* value is returned in milli-units */
    return (MCESD_U32) (sum * 1000 / 4.0);
}

MCESD_STATUS API_N5C32GP5X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
)
{
    switch (param)
    {
    case N5C32GP5X4_CDR_SELMUFI:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_SELMUFI, lane, *paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUFF:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_SELMUFF, lane, *paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUPI:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_REG_SELMUPI, lane, *paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUPF:
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_REG_SELMUPF, lane, *paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
)
{
    switch (param)
    {
    case N5C32GP5X4_CDR_SELMUFI:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_SELMUFI, lane, paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUFF:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_SELMUFF, lane, paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUPI:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_REG_SELMUPI, lane, paramValue);
        break;
    case N5C32GP5X4_CDR_SELMUPF:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_REG_SELMUPF, lane, paramValue);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
)
{
    MCESD_U32 data;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_SQ_DET_LPF0, &data));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_SQ_DET_LPF1, &data));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_SQ_DET_LPF2, &data));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_SQ_DET_LPF3, &data));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *squelched = (MCESD_BOOL)data;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
)
{
    if (threshold > N5C32GP5X4_SQ_THRESH_MAX)
        return MCESD_FAIL;

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_SQ_CAL_INDV_EXT_EN, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_SQ_CAL_RESULT_EXT, lane, threshold + 0x20);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
)
{
    MCESD_U32 data;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_SQ_CAL_RESULT_RD, lane, data);
    *threshold = (MCESD_16)data - 0x20;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_DATAPATH path
)
{
    switch (path)
    {
    case N5C32GP5X4_PATH_NEAR_END_LB:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DTL_CLAMPING_SEL, lane, 7);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_RX2TX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_A_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_TX2RX_LPBK, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN, lane, 0);
        break;
    case N5C32GP5X4_PATH_LOCAL_LB:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DTL_CLAMPING_SEL, lane, 7);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_RX2TX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_A_TX2RX_LPBK, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PU_LB, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN, lane, 0);
        break;
    case N5C32GP5X4_PATH_EXTERNAL:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DTL_CLAMPING_SEL, lane, 7);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_RX2TX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_A_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PU_LB, lane, 0);
        break;
    case N5C32GP5X4_PATH_FAR_END_LB:
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_DTL_CLAMPING_SEL, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_R2T_NO_STOP, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_R2T_RD_START, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_FOFFSET_DISABLE, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_RX2TX_LPBK, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_A_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_D_TX2RX_LPBK, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PU_LB, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN, lane, 0);
        break;
    default:
        return MCESD_FAIL; /* Unsupported parameter */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_DATAPATH *path
)
{
    MCESD_U32 digRxToTxData, anaTxToRxData, digTxToRxData;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_D_RX2TX_LPBK, lane, digRxToTxData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_A_TX2RX_LPBK, lane, anaTxToRxData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_D_TX2RX_LPBK, lane, digTxToRxData);

    if (1 == digTxToRxData)
        *path = N5C32GP5X4_PATH_NEAR_END_LB;
    else if (1 == digRxToTxData)
        *path = N5C32GP5X4_PATH_FAR_END_LB;
    else if (1 == anaTxToRxData)
        *path = N5C32GP5X4_PATH_LOCAL_LB;
    else
        *path = N5C32GP5X4_PATH_EXTERNAL;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
)
{
    MCESD_U32 data;
    MCESD_32 codeValue;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TSEN_ADC_DATA, 255 /* ignored */, data);
    codeValue = ConvertTwosComplementToI32(data, 12);
    *temperature = (MCESD_32)((codeValue * N5C32GP5X4_TSENE_GAIN) + N5C32GP5X4_TSENE_OFFSET);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_POLARITY txPolarity,
    IN E_N5C32GP5X4_POLARITY rxPolarity
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TXD_INV, lane, txPolarity);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RXD_INV, lane, rxPolarity);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_POLARITY *txPolarity,
    OUT E_N5C32GP5X4_POLARITY *rxPolarity
)
{
    MCESD_U32 txData, rxData;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_TXD_INV, lane, txData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RXD_INV, lane, rxData);

    *txPolarity = (E_N5C32GP5X4_POLARITY)txData;
    *rxPolarity = (E_N5C32GP5X4_POLARITY)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
)
{
    if ((errors < 1) || (errors > 8))
        return MCESD_FAIL;

    /* 0 = 1 error, 1 = 2 errors, ... 7 = 8 errors */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADD_ERR_NUM, lane, errors - 1);

    /* Toggle F_N5C32GP5X4_ADD_ERR_EN 0 -> 1 to trigger error injection */
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADD_ERR_EN, lane, 0);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADD_ERR_EN, lane, 1);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADD_ERR_EN, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_PATTERN txPattern,
    IN E_N5C32GP5X4_PATTERN rxPattern,
    IN const char *txUserPattern,
    IN const char *rxUserPattern
)
{
    E_N5C32GP5X4_DATABUS_WIDTH txWidth, rxWidth;

    if (N5C32GP5X4_PAT_USER == txPattern)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_PATTERN_SEL, lane, 1);
    }
    else
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_PATTERN_SEL, lane, txPattern);
    }

    if (strlen(txUserPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(txUserPattern, u8Pattern));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP_7948, lane, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3]));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP_4716, lane, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7]));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_UP_1500, lane, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9]));
    }

    if (N5C32GP5X4_PAT_USER == rxPattern)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_PATTERN_SEL, lane, 1);
    }
    else
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_PATTERN_SEL, lane, rxPattern);
    }

    if (strlen(rxUserPattern) > 0)
    {
        MCESD_U8 u8Pattern[10];

        MCESD_ATTEMPT(PatternStringToU8Array(rxUserPattern, u8Pattern));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_UP_7948, lane, MAKEU32FROMU8(u8Pattern[0], u8Pattern[1], u8Pattern[2], u8Pattern[3]));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_UP_4716, lane, MAKEU32FROMU8(u8Pattern[4], u8Pattern[5], u8Pattern[6], u8Pattern[7]));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_UP_1500, lane, MAKEU16FROMU8(u8Pattern[8], u8Pattern[9]));
    }

    MCESD_ATTEMPT(API_N5C32GP5X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));
    
    if ((N5C32GP5X4_PAT_JITTER_8T == txPattern) || (N5C32GP5X4_PAT_JITTER_4T == txPattern))
        txWidth = N5C32GP5X4_DATABUS_32BIT;
    else if ((N5C32GP5X4_PAT_JITTER_10T == txPattern) || (N5C32GP5X4_PAT_JITTER_5T == txPattern))
        txWidth = N5C32GP5X4_DATABUS_40BIT;

    if ((N5C32GP5X4_PAT_JITTER_8T == rxPattern) || (N5C32GP5X4_PAT_JITTER_4T == rxPattern))
        rxWidth = N5C32GP5X4_DATABUS_32BIT;
    else if ((N5C32GP5X4_PAT_JITTER_10T == rxPattern) || (N5C32GP5X4_PAT_JITTER_5T == rxPattern))
        rxWidth = N5C32GP5X4_DATABUS_40BIT;
    
    MCESD_ATTEMPT(API_N5C32GP5X4_SetDataBusWidth(devPtr, lane, txWidth, rxWidth));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_PATTERN *txPattern,
    OUT E_N5C32GP5X4_PATTERN *rxPattern,
    OUT char *txUserPattern,
    OUT char *rxUserPattern
)
{
    MCESD_FIELD txUserPatternFieldArray[] = { F_N5C32GP5X4_TX_UP_7948 , F_N5C32GP5X4_TX_UP_4716 , F_N5C32GP5X4_TX_UP_1500 };
    MCESD_FIELD rxUserPatternFieldArray[] = { F_N5C32GP5X4_RX_UP_7948 , F_N5C32GP5X4_RX_UP_4716 , F_N5C32GP5X4_RX_UP_1500 };
    MCESD_32 txUserPatternFieldArrayCount = sizeof(txUserPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 rxUserPatternFieldArrayCount = sizeof(rxUserPatternFieldArray) / sizeof(MCESD_FIELD);
    MCESD_32 i;
    MCESD_U32 txPatternSelData, rxPatternSelData, txUserPatternData, rxUserPatternData;
    MCESD_U8 u8Pattern[10];

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_PT_TX_PATTERN_SEL, lane, txPatternSelData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_PT_RX_PATTERN_SEL, lane, rxPatternSelData);

    *txPattern = (1 == txPatternSelData) ? N5C32GP5X4_PAT_USER : (E_N5C32GP5X4_PATTERN)txPatternSelData;
    *rxPattern = (1 == rxPatternSelData) ? N5C32GP5X4_PAT_USER : (E_N5C32GP5X4_PATTERN)rxPatternSelData;

    for (i = 0; i < txUserPatternFieldArrayCount; i++)
    {
        N5C32GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD(txUserPatternFieldArray[i]), lane, txUserPatternData);
        if (i != txUserPatternFieldArrayCount - 1)
        {
            u8Pattern[i * 4 + 0] = (MCESD_U8)(txUserPatternData >> 24);
            u8Pattern[i * 4 + 1] = (MCESD_U8)((txUserPatternData >> 16) & 0xFF);
            u8Pattern[i * 4 + 2] = (MCESD_U8)((txUserPatternData >> 8) & 0xFF);
            u8Pattern[i * 4 + 3] = (MCESD_U8)(txUserPatternData & 0xFF);
        }
        else
        {
            u8Pattern[i * 4 + 0] = (MCESD_8)(txUserPatternData >> 8);
            u8Pattern[i * 4 + 1] = (MCESD_8)(txUserPatternData & 0xFF);
        }
    }
    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, txUserPattern));

    for (i = 0; i < rxUserPatternFieldArrayCount; i++)
    {
        N5C32GP5X4_READ_FIELD(devPtr, EXTRACT_FIELD(rxUserPatternFieldArray[i]), lane, rxUserPatternData);
        if (i != rxUserPatternFieldArrayCount - 1)
        {
            u8Pattern[i * 4 + 0] = (MCESD_U8)(rxUserPatternData >> 24);
            u8Pattern[i * 4 + 1] = (MCESD_U8)((rxUserPatternData >> 16) & 0xFF);
            u8Pattern[i * 4 + 2] = (MCESD_U8)((rxUserPatternData >> 8) & 0xFF);
            u8Pattern[i * 4 + 3] = (MCESD_U8)(rxUserPatternData & 0xFF);
        }
        else
        {
            u8Pattern[i * 4 + 0] = (MCESD_8)(rxUserPatternData >> 8);
            u8Pattern[i * 4 + 1] = (MCESD_8)(rxUserPatternData & 0xFF);
        }
    }
    MCESD_ATTEMPT(GenerateStringFromU8Array(u8Pattern, rxUserPattern));

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_GRAY_CODE txGrayCode,
    IN E_N5C32GP5X4_GRAY_CODE rxGrayCode
)
{
    if (N5C32GP5X4_GRAY_NOT_USED != txGrayCode)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE0, txGrayCode));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE1, txGrayCode));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE2, txGrayCode));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE3, txGrayCode));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    if (N5C32GP5X4_GRAY_NOT_USED != rxGrayCode)
    {
        switch (lane)
        {
        case 0:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE0, rxGrayCode));
            break;
        case 1:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE1, rxGrayCode));
            break;
        case 2:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE2, rxGrayCode));
            break;
        case 3:
            MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE3, rxGrayCode));
            break;
        default:
            return MCESD_FAIL; /* Invalid lane */
        }
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5C32GP5X4_GRAY_CODE *txGrayCode,
    OUT E_N5C32GP5X4_GRAY_CODE *rxGrayCode
)
{
    MCESD_U32 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE0, &txData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE1, &txData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE2, &txData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_GRAY_CODE3, &txData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_GRAY_CODE3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txGrayCode = (E_N5C32GP5X4_GRAY_CODE)txData;
    *rxGrayCode = (E_N5C32GP5X4_GRAY_CODE)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_SetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txState,
    IN MCESD_BOOL rxState
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE0, txState));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE1, txState));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE2, txState));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE3, txState));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE0, rxState));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE1, rxState));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE2, rxState));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE3, rxState));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
)
{
    MCESD_U32 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE0, &txData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE1, &txData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE2, &txData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_TX_PRE_CODE3, &txData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RX_PRE_CODE3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txState = (MCESD_BOOL)txData;
    *rxState = (MCESD_BOOL)rxData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5C32GP5X4_PATTERN_STATS *statistics
)
{
    E_N5C32GP5X4_DATABUS_WIDTH txWidth, rxWidth;
    MCESD_U64 frames;
    MCESD_U32 passData, lockData, errData_47_32, errData_31_0, data_47_32, data_31_0;
    MCESD_32 dataBus;

    if (statistics == NULL)
        return MCESD_FAIL;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_PT_RX_PASS, lane, passData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_PT_RX_LOCK, lane, lockData);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_CNT_4732, lane, data_47_32);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_CNT_3100, lane, data_31_0);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_ERR_4732, lane, errData_47_32);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RX_ERR_3100, lane, errData_31_0);
    MCESD_ATTEMPT(API_N5C32GP5X4_GetDataBusWidth(devPtr, lane, &txWidth, &rxWidth));

    frames = ((MCESD_U64)data_47_32 << 32) + data_31_0;

    if (N5C32GP5X4_DATABUS_32BIT == rxWidth)
        dataBus = 32;
    else
        dataBus = 40;

    statistics->totalBits = frames * dataBus;
    statistics->totalErrorBits = ((MCESD_U64)errData_47_32 << 32) + errData_31_0;
    statistics->pass = (MCESD_BOOL)passData;
    statistics->lock = (MCESD_BOOL)lockData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_CNT_RST, lane, 0);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_CNT_RST, lane, 1);
    MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 1));
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_CNT_RST, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_PHYTEST type
)
{
    if (N5C32GP5X4_PHYTEST_TX == type)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_PHYREADY, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN_MODE, lane, 2);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_RST, lane, 0);
        MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_RST, lane, 1);
        MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_RST, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_PHYREADY, lane, 1);
    }

    if (N5C32GP5X4_PHYTEST_RX == type)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_PHYREADY, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_EN_MODE, lane, 2);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_EN, lane, 1);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_RST, lane, 0);
        MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_RST, lane, 1);
        MCESD_ATTEMPT(API_N5C32GP5X4_Wait(devPtr, 5));
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_RST, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_PHYREADY, lane, 1);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5C32GP5X4_PHYTEST type
)
{
    if (N5C32GP5X4_PHYTEST_RX == type)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_RX_EN, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_RX_PHYREADY, lane, 0);
    }

    if (N5C32GP5X4_PHYTEST_TX == type)
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_PT_TX_EN, lane, 0);
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_TX_PHYREADY, lane, 0);
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_PATH_SEL, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_DFEADAPT, lane, 0xF);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADAPT_EVEN_ENABLE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ADAPT_ODD_ENABLE, lane, 1);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_EN, lane, 1);
    N5C32GP5X4_POLL_FIELD(devPtr, F_N5C32GP5X4_EOM_READY, lane, 1, 15000);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_LPNUM, lane, 0xFF);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_EN, lane, 0);

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    IN MCESD_U32 minSamples,
    OUT S_N5C32GP5X4_EOM_DATA *measurement
)
{
    MCESD_U32 vldCntPData_39_32, vldCntPData_31_00, vldCntNData_39_32, vldCntNData_31_00, errCntPData_31_00, errCntNData_31_00, totalErrCntPData, totalErrCntNData;
    MCESD_U64 totalVldCntPData, totalVldCntNData;

    if (NULL == measurement)
        return MCESD_FAIL;

    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_PHASE, lane, phase);
    N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_ESM_VOLTAGE, lane, voltage);

    totalVldCntPData = 0;
    totalVldCntNData = 0;
    totalErrCntPData = 0;
    totalErrCntNData = 0;

    while ((totalVldCntPData < minSamples) || (totalVldCntNData < minSamples))
    {
        N5C32GP5X4_WRITE_FIELD(devPtr, F_N5C32GP5X4_EOM_DFE_CALL, lane, 1);
        N5C32GP5X4_POLL_FIELD(devPtr, F_N5C32GP5X4_EOM_DFE_CALL, lane, 0, 1000);

        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_VC_P_3932, lane, vldCntPData_39_32);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_VC_P_3100, lane, vldCntPData_31_00);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_VC_N_3932, lane, vldCntNData_39_32);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_VC_N_3100, lane, vldCntNData_31_00);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_ERR_CNT_P, lane, errCntPData_31_00);
        N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_EOM_ERR_CNT_N, lane, errCntNData_31_00);

        totalVldCntPData += MAKEU64FROMU32((MCESD_U64)vldCntPData_39_32, vldCntPData_31_00);
        totalVldCntNData += MAKEU64FROMU32((MCESD_U64)vldCntNData_39_32, vldCntNData_31_00);
        totalErrCntPData += errCntPData_31_00;
        totalErrCntNData += errCntNData_31_00;
    }

    measurement->phase = phase;
    measurement->voltage = voltage;
    measurement->upperBitCount = totalVldCntPData;
    measurement->upperBitErrorCount = totalErrCntPData;
    measurement->lowerBitCount = totalVldCntNData;
    measurement->lowerBitErrorCount = totalErrCntNData;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
)
{
    MCESD_U32 rxSpeedDiv, rxSpeedDiv2p5En;

    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RXSPEED_DIV, lane, rxSpeedDiv);
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_RXSPEED_DIV_2P5_EN, lane, rxSpeedDiv2p5En);

    switch (rxSpeedDiv)
    {
    case 0:
        *phaseStepCount = (1 == rxSpeedDiv2p5En) ? 160 : 128;
        break;
    case 4:
        *phaseStepCount = (1 == rxSpeedDiv2p5En) ? 320 : 256;
        break;
    case 5:
        *phaseStepCount = (1 == rxSpeedDiv2p5En) ? 640 : 512;
        break;
    case 6:
        *phaseStepCount = (1 == rxSpeedDiv2p5En) ? 1280 : 1024;
        break;
    case 7:
        *phaseStepCount = (1 == rxSpeedDiv2p5En) ? 2560 : 2048;
        break;
    default:
        return MCESD_FAIL;
    }

    *voltageStepCount = 64;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *heightUpper,
    OUT MCESD_U16 *heightLower,
    OUT MCESD_U32 *sampleCount
)
{
    MCESD_32 leftEdge, rightEdge, upperEdge, lowerEdge, maxPhase, minPhase, maxVoltage, phase, voltage, phaseMidpoint;
    MCESD_U32 upperBER, lowerBER;
    MCESD_U16 phaseStepCount, voltageStepCount;
    S_N5C32GP5X4_EOM_DATA measurement;

    MCESD_ATTEMPT(API_N5C32GP5X4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));

    maxPhase = phaseStepCount / 2;
    minPhase = -phaseStepCount / 2;
    maxVoltage = voltageStepCount;

    /* Set default edge values */
    leftEdge = minPhase;
    rightEdge = maxPhase;
    upperEdge = -maxVoltage;
    lowerEdge = maxVoltage - 1;

    MCESD_ATTEMPT(API_N5C32GP5X4_EOMInit(devPtr, lane));

    /* Scan Left */
    for (phase = 0; phase < maxPhase; phase++)
    {
        MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phase, 0, minSamples, &measurement));

        if (0 == phase)
        {
            *sampleCount = (MCESD_U32)measurement.upperBitCount;
        }

        /* Stop when no bits read or error */
        if ((0 == measurement.upperBitCount) || (0 == measurement.lowerBitCount))
        {
            leftEdge = phase;
            break;
        }

        /* Stop because BER was too high */
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            leftEdge = phase;
            break;
        }
    }

    /* Scan Right */
    for (phase = -1; phase > minPhase; phase--)
    {
        MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phase, 0, minSamples, &measurement));

        /* Stop when no bits read or error */
        if ((0 == measurement.upperBitCount) || (0 == measurement.lowerBitCount))
        {
            rightEdge = phase;
            break;
        }

        /* Stop because BER was too high */
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
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
        MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phaseMidpoint, (MCESD_U8)voltage, minSamples, &measurement));

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
            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBER));
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
            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBER));
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

    *width = (MCESD_16)((leftEdge == rightEdge) ? 0 : leftEdge - rightEdge);
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

    MCESD_ATTEMPT(API_N5C32GP5X4_EOMFinalize(devPtr, lane));

    return MCESD_OK;
}

MCESD_STATUS INT_N5C32GP5X4_CalculateBER
(
    IN MCESD_U32 bitErrorCount,
    IN MCESD_U64 bitCount,
    OUT MCESD_U32 *nanoBER
)
{
    MCESD_U64 adjustedBitErrorCount = (MCESD_U64)bitErrorCount * 0x3B9ACA00;   /* 0x3B9ACA00 == 1E+9 */
    *nanoBER = (MCESD_U32)(adjustedBitErrorCount / ((MCESD_U64)bitCount));
    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMConvertWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 width,
    IN MCESD_U16 heightUpper,
    IN MCESD_U16 heightLower,
    OUT MCESD_U32* widthmUI,
    OUT MCESD_U32* height10uVUpper,
    OUT MCESD_U32* height10uVLower
)
{
    *widthmUI = 0;
    *height10uVUpper = 0;
    *height10uVLower = 0;

#ifdef N5C32GP5X4_DFE_MILLIVOLTS 
    MCESD_U16 phaseStepCount, voltageStepCount;
    MCESD_U32 table[64];

    if (0 == width)
        return MCESD_FAIL; /* Division by 0 Error */

    if((heightUpper > 63) || (heightLower > 63))
    {
        MCESD_DBG_ERROR( "API_N5C32GP5X4_EOMConvertWidthHeight: Height out of range (0-63)\n" );
        return MCESD_FAIL;
    }

    /* Convert width */
    MCESD_ATTEMPT(API_N5C32GP5X4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));
    *widthmUI = (MCESD_U32)width * 1000 / phaseStepCount;

    /* Convert height */
    MCESD_ATTEMPT(INT_N5C32GP5X4_GetDfeF0(devPtr, lane, table));
    *height10uVUpper = table[heightUpper];
    *height10uVLower = table[heightLower];

    return MCESD_OK;
#else
    return MCESD_FAIL;
#endif
}

MCESD_STATUS INT_N5C32GP5X4_GetDfeF0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    INOUT MCESD_U32* table
)
{
#ifdef N5C32GP5X4_DFE_MILLIVOLTS
    MCESD_U32 i, dcShift, resData, avddHiLo;
    
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_F0_DC_SHIFT, lane, dcShift);
    if (dcShift > 1)
    {
        MCESD_DBG_ERROR("INT_N5C32GP5X4_GetDfeF0: DC Shift can only be 0 or 1\n");
        return MCESD_FAIL;
    }
    N5C32GP5X4_READ_FIELD(devPtr, F_N5C32GP5X4_DFE_RES_F0, lane, resData);
    MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_AVDD_SEL, &avddHiLo));
    avddHiLo = (avddHiLo > 1) ? 1 : 0;

    for (i = 0; i < 64; i++)
    {
        table[i] = N5C32GP5X4_DFE_F0_TABLE[avddHiLo][dcShift][resData][i];
    }

    return MCESD_OK;
#else
    return MCESD_FAIL;
#endif
}

MCESD_STATUS API_N5C32GP5X4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    IN MCESD_BOOL eomStatsMode,
    INOUT S_N5C32GP5X4_EYE_RAW_PTR eyeRawDataPtr
)
{
    S_N5C32GP5X4_EOM_DATA measurement;
    MCESD_U32 leftEdgeIdx, rightEdgeIdx, upperEdgeIdx, lowerEdgeIdx, phaseCenterIdx, voltageCenterIdx, upperBer, lowerBer;
    MCESD_32 upperEdge, lowerEdge, leftMaxEdge, rightMaxEdge, upperMaxEdge, lowerMaxEdge, phase, voltage, upperVoltage, lowerVoltage, leftInnerEdge, rightInnerEdge;
    MCESD_U16 phaseStepCount, voltageStepCount;

    MCESD_ATTEMPT(API_N5C32GP5X4_EOM1UIStepCount(devPtr, lane, &phaseStepCount, &voltageStepCount));

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_N5C32GP5X4_EOMGetEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    eyeRawDataPtr->oneUIwidth = phaseStepCount;

    phaseCenterIdx = N5C32GP5X4_EYE_MAX_PHASE_LEVEL / 2;
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

    MCESD_ATTEMPT(API_N5C32GP5X4_EOMInit(devPtr, lane));

    /* Check (0, 0) */
    MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, 0, 0, minSamples, &measurement));
    eyeRawDataPtr->sampleCount = (MCESD_U32)measurement.upperBitCount;

    if (0 == measurement.upperBitCount || 0 == measurement.lowerBitCount)
    {
        MCESD_ATTEMPT(API_N5C32GP5X4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_N5C32GP5X4_EOMGetEyeData: Total Bit Count == 0 at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
    MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
    if ((berThreshold < upperBer) || (berThreshold < lowerBer))
    {
        MCESD_ATTEMPT(API_N5C32GP5X4_EOMFinalize(devPtr, lane));
        MCESD_DBG_ERROR("API_N5C32GP5X4_EOMGetEyeData: BER > berThreshold at (Phase: 0, Voltage: 0)\n");
        return MCESD_FAIL;
    }

    /* Left Sweep from Phase 0 with Voltage Sweep */
    leftInnerEdge = leftMaxEdge;
    for (phase = 0; phase <= leftMaxEdge; phase++)
    {
        upperVoltage = lowerMaxEdge;
        lowerVoltage = upperMaxEdge;
        for (voltage = 0; voltage <= upperMaxEdge; voltage++)
        {
            MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phase, (MCESD_U8)voltage, minSamples, &measurement));

            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx + voltage] = measurement.lowerBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx - voltage] = measurement.upperBitErrorCount;

            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
            if ((berThreshold < upperBer) || (berThreshold < lowerBer))
            {
                if ((upperVoltage == lowerMaxEdge) && ((0 == measurement.upperBitCount) || (berThreshold < upperBer)))
                    upperVoltage = voltage; /* found upper edge */

                if ((lowerVoltage == upperMaxEdge) && ((0 == measurement.lowerBitCount) || (berThreshold < lowerBer)))
                    lowerVoltage = -voltage; /* found lower edge */
            }

            if ((phase < leftInnerEdge) && ((0 == upperVoltage) || (0 == lowerVoltage)))
                leftInnerEdge = phase;

            /* Measure only voltage 0 in eomStatsMode */
            if (eomStatsMode)
                break;

            if (((upperVoltage != lowerMaxEdge) && (lowerVoltage != upperMaxEdge)))
                break;
        }

        /* Measure full UI in eomStatsMode */
        if (eomStatsMode)
            continue;

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
    for (phase = -1; phase >= rightMaxEdge; phase--)
    {
        upperVoltage = lowerMaxEdge;
        lowerVoltage = upperMaxEdge;
        for (voltage = 0; voltage <= upperMaxEdge; voltage++)
        {
            MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phase, (MCESD_U8)voltage, minSamples, &measurement));

            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx + voltage] = measurement.lowerBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx - voltage] = measurement.upperBitErrorCount;

            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.upperBitErrorCount, measurement.upperBitCount, &upperBer));
            MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(measurement.lowerBitErrorCount, measurement.lowerBitCount, &lowerBer));
            if ((berThreshold < upperBer) || (berThreshold < lowerBer))
            {
                if ((upperVoltage == lowerMaxEdge) && ((0 == measurement.upperBitCount) || (berThreshold < upperBer)))
                    upperVoltage = voltage; /* found upper edge */

                if ((lowerVoltage == upperMaxEdge) && ((0 == measurement.lowerBitCount) || (berThreshold < lowerBer)))
                    lowerVoltage = -voltage; /* found lower edge */
            }

            if ((phase > rightInnerEdge) && ((0 == upperVoltage) || (0 == lowerVoltage)))
                rightInnerEdge = phase;

            /* Measure only voltage 0 in eomStatsMode */
            if (eomStatsMode)
                break;

            if (((upperVoltage != lowerMaxEdge) && (lowerVoltage != upperMaxEdge)))
                break;
        }

        /* Measure full UI in eomStatsMode */
        if (eomStatsMode)
            continue;

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
    if (eomStatsMode)
    {
        /* Full Voltage Sweep at Phase Midpoint */
        phase = (leftInnerEdge - rightInnerEdge) / 2 + rightInnerEdge;
        for (voltage = 0; voltage <= upperMaxEdge; voltage++)
        {
            MCESD_ATTEMPT(API_N5C32GP5X4_EOMMeasPoint(devPtr, lane, phase, (MCESD_U8)voltage, minSamples, &measurement));

            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx + voltage] = measurement.lowerBitErrorCount;
            eyeRawDataPtr->eyeRawData[phaseCenterIdx - phase][voltageCenterIdx - voltage] = measurement.upperBitErrorCount;
        }
        upperEdgeIdx = voltageCenterIdx - upperMaxEdge;
        lowerEdgeIdx = voltageCenterIdx - lowerMaxEdge;
        leftEdgeIdx = phaseCenterIdx - leftMaxEdge;
        rightEdgeIdx = phaseCenterIdx - rightMaxEdge;
    }
    else
    {
        upperEdgeIdx = voltageCenterIdx - upperEdge;
        lowerEdgeIdx = voltageCenterIdx - lowerEdge;
    }

    MCESD_ATTEMPT(API_N5C32GP5X4_EOMFinalize(devPtr, lane));

    eyeRawDataPtr->leftEdge = leftEdgeIdx;
    eyeRawDataPtr->rightEdge = rightEdgeIdx;
    eyeRawDataPtr->upperEdge = upperEdgeIdx;
    eyeRawDataPtr->lowerEdge = lowerEdgeIdx;

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_EOMPlotEyeData
(
    IN S_N5C32GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 phaseStepSize,
    IN MCESD_U32 voltageStepSize,
    IN MCESD_U32 berThreshold,
    IN MCESD_U32 berThresholdMax
)
{
    MCESD_U32 *mVTable = NULL;

    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_N5C32GP5X4_EOMPlotEyeData: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    MCESD_ATTEMPT(plotEyeData(eyeRawDataPtr->eyeRawData[0], eyeRawDataPtr->leftEdge, eyeRawDataPtr->rightEdge, N5C32GP5X4_EYE_MAX_PHASE_LEVEL / 2, phaseStepSize,
        eyeRawDataPtr->upperEdge, eyeRawDataPtr->lowerEdge, N5C32GP5X4_EYE_MAX_VOLT_STEPS - 1, voltageStepSize, eyeRawDataPtr->sampleCount, (N5C32GP5X4_EYE_MAX_VOLT_STEPS * 2) - 1,
        berThreshold, berThresholdMax, mVTable, 1000));

    return MCESD_OK;
}

#ifdef MCESD_EOM_STATS
MCESD_STATUS API_N5C32GP5X4_EOMGetStats
(
    IN S_N5C32GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN S_N5C32GP5X4_EYE_BUFFER_PTR eyeBufferDataPtr,
    INOUT S_EOM_STATS_EYE_DIM *estimateEyeDimPtr,
    IN MCESD_U16 estimateEyeDimCount,
    OUT S_EOM_STATS_EYE_AMP *amplitudeStats
)
{
    if (!eyeRawDataPtr)
    {
        MCESD_DBG_ERROR("API_N5C32GP5X4_EOMGetStats: eyeRawDataPtr is NULL\n");
        return MCESD_FAIL;
    }

    /* Calculate Q and SNR */
    MCESD_ATTEMPT(INT_EOM_STATS_GetAmplitudeStats(eyeRawDataPtr->eyeRawData[0], eyeBufferDataPtr->bufferData[0],
        eyeRawDataPtr->relativeCenter, N5C32GP5X4_EYE_MAX_PHASE_LEVEL,
        eyeRawDataPtr->upperEdge, eyeRawDataPtr->lowerEdge, N5C32GP5X4_EYE_MAX_VOLT_STEPS - 1, (N5C32GP5X4_EYE_MAX_VOLT_STEPS * 2) - 1,
        amplitudeStats));

    /* Estimate Height */
    MCESD_ATTEMPT(INT_EOM_STATS_EstimateDimension(eyeRawDataPtr->eyeRawData[0], eyeBufferDataPtr->bufferData[0],
        eyeRawDataPtr->leftEdge, eyeRawDataPtr->rightEdge, eyeRawDataPtr->relativeCenter, N5C32GP5X4_EYE_MAX_PHASE_LEVEL,
        eyeRawDataPtr->upperEdge, eyeRawDataPtr->lowerEdge, N5C32GP5X4_EYE_MAX_VOLT_STEPS - 1, (N5C32GP5X4_EYE_MAX_VOLT_STEPS * 2) - 1,
        eyeRawDataPtr->sampleCount, estimateEyeDimPtr, estimateEyeDimCount, MCESD_FALSE));

    /* Estimate Width */
    MCESD_ATTEMPT(INT_EOM_STATS_EstimateDimension(eyeRawDataPtr->eyeRawData[0], eyeBufferDataPtr->bufferData[0],
        eyeRawDataPtr->leftEdge, eyeRawDataPtr->rightEdge, eyeRawDataPtr->relativeCenter, N5C32GP5X4_EYE_MAX_PHASE_LEVEL,
        eyeRawDataPtr->upperEdge, eyeRawDataPtr->lowerEdge, N5C32GP5X4_EYE_MAX_VOLT_STEPS - 1, (N5C32GP5X4_EYE_MAX_VOLT_STEPS * 2) - 1,
        eyeRawDataPtr->sampleCount, estimateEyeDimPtr, estimateEyeDimCount, MCESD_TRUE));

    return MCESD_OK;
}
#endif

MCESD_STATUS API_N5C32GP5X4_EyeDataDimensions
(
    IN S_N5C32GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U32 *width,
    OUT MCESD_U32 *heightUpper,
    OUT MCESD_U32 *heightLower
)
{
    MCESD_U32 upperBER, lowerBER, phaseIndex, phaseCenter, leftIndex, rightIndex, voltageCenter, upperIndex, lowerIndex;
    MCESD_32 voltageIndex;
    phaseCenter = N5C32GP5X4_EYE_MAX_PHASE_LEVEL / 2;
    leftIndex = eyeRawDataPtr->leftEdge;
    rightIndex = eyeRawDataPtr->rightEdge;
    voltageCenter = N5C32GP5X4_EYE_MAX_VOLT_STEPS - 1;
    upperIndex = eyeRawDataPtr->upperEdge;
    lowerIndex = eyeRawDataPtr->lowerEdge;

    /* Sweep from Center Phase to Left Edge */
    for (phaseIndex = phaseCenter; phaseIndex >= eyeRawDataPtr->leftEdge; phaseIndex--)
    {
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter], eyeRawDataPtr->sampleCount, &upperBER));
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter + 1], eyeRawDataPtr->sampleCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            leftIndex = phaseIndex;
            break;
        }
    }

    /* Sweep from Center Phase to Right Edge */
    for (phaseIndex = phaseCenter; phaseIndex <= eyeRawDataPtr->rightEdge; phaseIndex++)
    {
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter], eyeRawDataPtr->sampleCount, &upperBER));
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageCenter + 1], eyeRawDataPtr->sampleCount, &lowerBER));
        if ((berThreshold < upperBER) || (berThreshold < lowerBER))
        {
            rightIndex = phaseIndex;
            break;
        }
    }

    phaseIndex = leftIndex + (rightIndex - leftIndex) / 2;

    /* Sweep from Center Voltage to Upper Edge */
    for (voltageIndex = voltageCenter; voltageIndex >= ((MCESD_32) eyeRawDataPtr->upperEdge); voltageIndex--)
    {
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex], eyeRawDataPtr->sampleCount, &upperBER));
        if (berThreshold < upperBER)
        {
            upperIndex = voltageIndex;
            break;
        }
    }

    /* Sweep from Center Voltage to Lower Edge */
    for (voltageIndex = voltageCenter; voltageIndex <= ((MCESD_32) eyeRawDataPtr->lowerEdge); voltageIndex++)
    {
        MCESD_ATTEMPT(INT_N5C32GP5X4_CalculateBER(eyeRawDataPtr->eyeRawData[phaseIndex][voltageIndex], eyeRawDataPtr->sampleCount, &lowerBER));
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

MCESD_STATUS API_N5C32GP5X4_ExecuteCDS
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
)
{
    (void)devPtr;
    (void)lane;
    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_AssertTxRxCoreReset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txReset,
    IN MCESD_BOOL rxReset
)
{
    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_TX0, txReset));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_RX0, rxReset));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_TX1, txReset));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_RX1, rxReset));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_TX2, txReset));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_RX2, rxReset));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_TX3, txReset));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwSetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_RX3, rxReset));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_GetResetCoreAckTxRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReset,
    OUT MCESD_BOOL *rxReset
)
{
    MCESD_U32 txData, rxData;

    switch (lane)
    {
    case 0:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_TX0, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_RX0, &rxData));
        break;
    case 1:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_TX1, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_RX1, &rxData));
        break;
    case 2:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_TX2, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_RX2, &rxData));
        break;
    case 3:
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_TX3, &txData));
        MCESD_ATTEMPT(API_N5C32GP5X4_HwGetPinCfg(devPtr, N5C32GP5X4_PIN_RESET_ACK_RX3, &rxData));
        break;
    default:
        return MCESD_FAIL; /* Invalid lane */
    }

    *txReset = txData == 0 ? MCESD_FALSE : MCESD_TRUE;
    *rxReset = rxData == 0 ? MCESD_FALSE : MCESD_TRUE;

    return MCESD_OK;
}

#endif /* N5C32GP5X4 */