/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/


/************************************************************************
This file contains functions to configure RS-FEC/KR-FEC
and read RS-FEC/KR-FEC status for the Marvell X35X0/E25X0 PHY
************************************************************************/
#include "mtdFeatures.h"
#include "mtdApiRegs.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdHunit.h"
#include "mtdFEC.h"
#include "mtdCunit.h"

#define MTD_RS_FEC_CONTROL              MTD_T_UNIT_PMA_PMD,0xA0C8
#define MTD_RS_FEC_STATUS               MTD_T_UNIT_PMA_PMD,0xA0C9
#define MTD_RS_FEC_CTRL_STATUS7         MTD_T_UNIT_PMA_PMD,0xA10B
#define MTD_RS_FEC_CORR_CW_LOW          MTD_T_UNIT_PMA_PMD,0xA0CA
#define MTD_RS_FEC_CORR_CW_UPPER        MTD_T_UNIT_PMA_PMD,0xA0CB
#define MTD_RS_FEC_UNCORR_CW_LOW        MTD_T_UNIT_PMA_PMD,0xA0CC
#define MTD_RS_FEC_UNCORR_CW_UPPER      MTD_T_UNIT_PMA_PMD,0xA0CD
#define MTD_RS_FEC_SYM_ERR_CTR_LOW      MTD_T_UNIT_PMA_PMD,0xA0D2
#define MTD_RS_FEC_SYM_ERR_CTR_UPPER    MTD_T_UNIT_PMA_PMD,0xA0D3

/*******************************************************************
 Local Function Prototypes
 *******************************************************************/
MTD_STATIC MTD_STATUS mtdErrorCheckRsFec
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

/*******************************************************************
 Local Function Prototypes
 *******************************************************************/
MTD_STATIC MTD_STATUS mtdRsFECGroupEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enable
);

/*******************************************************************************
 MTD_STATUS mtdRsFECEnable
    Enables or disable the RS-FEC
*******************************************************************************/
MTD_STATUS mtdRsFECEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL readToClear,
    IN MTD_U16 enable
)
{
    MTD_U16 temp;

    /* ensure only allow primary port */ 
    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    /* set enable/disable to all the ports in the group */ 
    MTD_ATTEMPT(mtdRsFECGroupEnable(devPtr, port, enable));

    temp = (readToClear == MTD_TRUE) ? 0 : 1;
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CTRL_STATUS7, 0, 1, temp));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdSetRsFecControl
    Configure the RS-FEC for those parameters that are configurable
*******************************************************************************/
MTD_STATUS mtdSetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 bypassIndicationEnable,
    IN MTD_U16 bypassCorrectionEnable
)
{
    MTD_U16 data16Bits;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    if (bypassCorrectionEnable == MTD_ENABLE && bypassIndicationEnable == MTD_ENABLE)
    {
        MTD_DBG_ERROR("mtdSetRsFecControl: Bypass both is an error.\n");
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CONTROL, &data16Bits));
    mtdHwSetRegFieldToWord(data16Bits, bypassCorrectionEnable, 0, 1, &data16Bits);
    mtdHwSetRegFieldToWord(data16Bits, bypassIndicationEnable, 1, 1, &data16Bits);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_RS_FEC_CONTROL, data16Bits));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecControl
    Read the values of the current settings for the RS-FEC
*******************************************************************************/
MTD_STATUS mtdGetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *bypassIndicationEnable,
    OUT MTD_U16 *bypassCorrectionEnable
)
{
    MTD_U16 data16Bits;

    *bypassIndicationEnable = *bypassCorrectionEnable = 0;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CONTROL, &data16Bits));
    mtdHwGetRegFieldFromWord(data16Bits, 0, 1, bypassCorrectionEnable);
    mtdHwGetRegFieldFromWord(data16Bits, 1, 1, bypassIndicationEnable);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecStatus
    Get the high-level status of the RS-FEC
*******************************************************************************/
MTD_STATUS mtdGetRsFecStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *pcsLaneAlignment,
    OUT MTD_U16 *fecLaneAlignment,
    OUT MTD_U16 *latchedRsFecHighErr,
    OUT MTD_U16 *currRsFecHighErr
)
{
    MTD_U16 data16Bits;

    *pcsLaneAlignment = *fecLaneAlignment = *latchedRsFecHighErr = *currRsFecHighErr = 0;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_STATUS, &data16Bits));
    mtdHwGetRegFieldFromWord(data16Bits, 15, 1, pcsLaneAlignment);
    mtdHwGetRegFieldFromWord(data16Bits, 14, 1, fecLaneAlignment);
    mtdHwGetRegFieldFromWord(data16Bits, 2, 1, latchedRsFecHighErr);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_RS_FEC_STATUS, 2, 1, currRsFecHighErr));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdRsFECReset
    Reset the RS-FEC rx block, including RS-FEC counters.
*******************************************************************************/
MTD_STATUS mtdRsFECReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x3));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x2));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x0));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecCorrectedCwCntr
    Read the RS-FEC Corrected Code Word Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
)
{
    MTD_U16 correctedCodeWordLower, correctedCodeWordUpper;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CORR_CW_LOW, &correctedCodeWordLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CORR_CW_UPPER, &correctedCodeWordUpper));

    *codeWordCounter = correctedCodeWordLower;
    *codeWordCounter |= (((MTD_U32)(correctedCodeWordUpper))<<16);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
    Read the RS-FEC Uncorrected Code Word Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
)
{
    MTD_U16 uncorrectedCodeWordLower, uncorrectedCodeWordUpper;

    *codeWordCounter = 0;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_UNCORR_CW_LOW, &uncorrectedCodeWordLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_UNCORR_CW_UPPER, &uncorrectedCodeWordUpper));

    *codeWordCounter = uncorrectedCodeWordLower;
    *codeWordCounter |= (((MTD_U32)(uncorrectedCodeWordUpper))<<16);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecSymbolErrorCntr
    Read the RS-FEC Symbol Error Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecSymbolErrorCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *errorCounter
)
{
    MTD_U16 symbolErrorCtrLower, symbolErrorCtrUpper;

    *errorCounter = 0;

    MTD_ATTEMPT(mtdErrorCheckRsFec(devPtr, port));

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_SYM_ERR_CTR_LOW, &symbolErrorCtrLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_SYM_ERR_CTR_UPPER, &symbolErrorCtrUpper));

    *errorCounter = symbolErrorCtrLower;
    *errorCounter |= (((MTD_U32)(symbolErrorCtrUpper))<<16);

    return MTD_OK;
}

/*******************************************************************
 Local Functions
 *******************************************************************/

/* Performs parameter checks and also
   checks for RS-FEC on right mac type and also primary port
   NOTE: on the read counter functions, may want to comment out the
   error checking to reduce the delay in reading the counters.
   Checking the mactype, for example, involves quite a bit
   of MDIO read activity */
MTD_STATIC MTD_STATUS mtdErrorCheckRsFec
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_U16 macType, macLinkDownSpeed, macMaxIfSpeed;
    MTD_BOOL macIfPowerDown;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdErrorCheckRsFec: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr) && !MTD_IS_X3680_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdErrorCheckRsFec: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdGetMacInterfaceCopperOnlyPhy(devPtr, port, &macType, &macIfPowerDown, &macLinkDownSpeed, &macMaxIfSpeed));

    /* Check mac type and on primary port */
    if (macType == MTD_MAC_TYPE_20G_DXGMII)
    {
        if (port%2)
        {
            MTD_DBG_ERROR("mtdErrorCheckRsFec: RS-FEC is on primary port only. \n");
            return MTD_FAIL;
        }
    }
    else if (macType == MTD_MAC_TYPE_20G_QXGMII)
    {
        if (port%4)
        {
            MTD_DBG_ERROR("mtdErrorCheckRsFec: RS-FEC is on primary port only. \n");
            return MTD_FAIL;
        }
    }
    else if (macType == MTD_MAC_TYPE_OXGMII || macType == MTD_MAC_TYPE_OXGMII_PARTNER)
    {
        if (port%8)
        {
            MTD_DBG_ERROR("mtdErrorCheckRsFec: RS-FEC is on primary port only. \n");
            return MTD_FAIL;
        }
    }
    else
    {
        MTD_DBG_ERROR("mtdErrorCheckRsFec: RS-FEC is not supported on this mac type. \n");
        return MTD_FAIL;
    }

    return MTD_OK;

}


/*******************************************************************
 Local Functions
 *******************************************************************/
/*
    set RE-FEC enable/disable to all the ports in a group
*/

MTD_STATIC MTD_STATUS mtdRsFECGroupEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enable
)
{
    MTD_U16 macType, macLinkDownSpeed, macMaxIfSpeed, idx;
    MTD_BOOL macIfPowerDown;
    MTD_U16 portNumInGroup = 0;

    MTD_ATTEMPT(mtdGetMacInterfaceCopperOnlyPhy(devPtr, port, &macType, &macIfPowerDown, &macLinkDownSpeed, &macMaxIfSpeed));

    if (macType == MTD_MAC_TYPE_20G_DXGMII)
    {
        portNumInGroup = 2;
    }
    else if (macType == MTD_MAC_TYPE_20G_QXGMII)
    {
        portNumInGroup = 4;
    }
    else if (macType == MTD_MAC_TYPE_OXGMII)
    {
       if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
       {
            MTD_DBG_ERROR("mtdRsFECGroupEnable: RS-FEC can't be set/unset for X35X0/E25X0 in OXGMII mode. \n");
            return MTD_FAIL;
       }
       else
       {
            portNumInGroup = 8;
       }
    }

    /* enable/disable RS-FEC in each port of the group */ 
    for (idx = 0; idx < portNumInGroup; idx++)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port + idx, MTD_CUNIT_PORT_CTRL2, 13, 2, ((1<<1)|enable)));
    }

    /* c unit need to apply a sw reset to take effect, only need to apply to the primary port of the group  */ 
    MTD_ATTEMPT(mtdCunitSwReset(devPtr, port));

    return MTD_OK;
}
