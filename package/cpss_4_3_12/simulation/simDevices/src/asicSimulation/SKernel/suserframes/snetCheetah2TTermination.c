/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetCheetah2TT.c
*
* DESCRIPTION:
*       Cheetah2 Asic Simulation .
*       Egress Policy Engine processing for outgoing frame.
*       Source Code file.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TTermination.h>
#include <asicSimulation/SLog/simLog.h>

/**
* @internal snetCht2TTIPv4ExceptionCheck function
* @endinternal
*
* @brief   IPv4 tunnels and MPLS tunnels have separate T.T trigger
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*
* @note 13.4.3 page 358
*
*/
GT_BOOL snetCht2TTIPv4ExceptionCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STCT_PTR      internalTtiInfoPtr

)
{
    DECLARE_FUNC_NAME(snetCht2TTIPv4ExceptionCheck);


    GT_BOOL status = GT_TRUE;  /* return code */
    GT_U32 fldVal;              /* register field's value */
    GT_U32 startBit;            /* start bit in the register */
    GT_U32 cpuCode;             /*cpu code*/
    GT_BIT ipHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError :
                                           descrPtr->ipHeaderError;
    GT_BIT ipTtiHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError :
                                           descrPtr->ipTtiHeaderError;


    if (ipHeaderError ||  /* ip header error */
        ipTtiHeaderError) /* or extra TTI Ip header Error*/
    {
        status = GT_FALSE;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(ipHeaderError)
            {
                __LOG(("IPv4 TTI Header Error Command \n"));
                startBit = 0;
                cpuCode = SNET_CHT_IPV4_TT_HEADER_ERROR;
            }
            else /*ipTtiHeaderError*/
            {
                __LOG(("IPv4 TTI SIP Address Error Command \n"));
                startBit = 9;
                /* get the CPU code from the register */
                smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_0_REG(devObjPtr) , 0, 8, &fldVal);

                cpuCode = fldVal;
            }

            smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_COMMANDS_REG(devObjPtr) , startBit, 3, &fldVal);

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      fldVal,
                                                      descrPtr->cpuCode,
                                                      cpuCode,
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_FALSE);

            return (descrPtr->packetCmd > SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E) ?
                    GT_FALSE :
                    GT_TRUE;
        }

        smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr) , 4, 1, &fldVal);

        if (fldVal == 1)
        {
            descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_HARD_DROP_E;
        }
        else
        {
            descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_TRAP_E;
            descrPtr->cpuCode = SNET_CHT_IPV4_TT_HEADER_ERROR;

        }
    }

    return   status;

}




