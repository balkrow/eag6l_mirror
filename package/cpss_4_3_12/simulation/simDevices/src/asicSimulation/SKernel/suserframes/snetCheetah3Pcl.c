/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file snetCheetah3Pcl.c
*
* @brief
* Ingress Policy Engine processing for incoming frame.
*
* @version   51
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Pcl.h>
#include <asicSimulation/SLog/simLog.h>



/* macro to return result of which bits caused the 'NO MATCH' */
#define X_Y_K_FIND_NO_MATCH(x,y,k,mask)  \
    ((~(((~x) & (~k)) | ((~y) & (k)))) & mask )


static GT_BOOL snetCht3DataMaskKeyCompare
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U8 * dataPtr,
    IN GT_U8 * maskPtr,
    IN GT_U8 * keyPtr,
    IN GT_U32 compBits,

    IN GT_U32   indexInTcam,    /* for log info */
    IN GT_U32   bankIndex,      /* for log info */
    IN GT_U32   wordIndex      /* for log info */
);

CHT_PCL_TCAM_COMMON_DATA_STC xcatGlobalPclData =
           {CHT3_PCL_TCAM_BANK_NUMBERS,
            XCAT_PCL_TCAM_BANK_ENTRIES,     CHT3_PCL_TCAM_BANK_ENTRIES_STEP,
            CHT3_PCL_TCAM_BANK_ENTRY_WORDS, CHT3_PCL_TCAM_BANK_ENTRY_WORDS_STEP,
            CHT3_PCL_TCAM_BANK_DATA_BITS,   CHT3_PCL_TCAM_BANK_CTRL_BITS,
            snetCht3DataMaskKeyCompare};

CHT_PCL_TCAM_COMMON_DATA_STC lion2PortGroupGlobalPclData =
           {CHT3_PCL_TCAM_BANK_NUMBERS,
            LION2_PCL_TCAM_BANK_ENTRIES,    CHT3_PCL_TCAM_BANK_ENTRIES_STEP,
            CHT3_PCL_TCAM_BANK_ENTRY_WORDS, CHT3_PCL_TCAM_BANK_ENTRY_WORDS_STEP,
            XCAT2_PCL_TCAM_BANK_DATA_BITS,  XCAT2_PCL_TCAM_BANK_CTRL_BITS,
            snetCht3DataMaskKeyCompare};



/**
* @internal snetCht3IPclActionApply function
* @endinternal
*
* @brief   Apply the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr   - pointer to the PCL action entry.
* @param[in] isFirst                  - is first action apply
*
* @note Policy Tcam Table : The policy TCAM holds 1024 lines
*       of 24 bytes or rule data used for ingress and egress PCL.
*
*/
GT_VOID snetCht3IPclActionApply
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_CHT3_PCL_ACTION_STC             * actionDataPtr,
    IN GT_BOOL                              isFirst
)
{
    DECLARE_FUNC_NAME(snetCht3IPclActionApply);

    SNET_CHT2_PCL_ACTION_STC  * actionCht2DataPtr;
    GT_BOOL saveCntrEn; /* save counter enable status */
    GT_U32  origSstId;/* sstId of the incoming descriptor*/

    actionCht2DataPtr = (SNET_CHT2_PCL_ACTION_STC  *)actionDataPtr;

    if (actionCht2DataPtr->pceRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E)
    {
        /* Route action will be applied later in UC route engine */
        __LOG(("Route action will be applied later in UC route engine"));
        return;
    }

    /* prevent snetCht2IPclActionApply count processing */
    __LOG(("prevent snetCht2IPclActionApply count processing"));
    saveCntrEn = actionDataPtr->baseAction.matchCounterEn;
    actionDataPtr->baseAction.matchCounterEn = GT_FALSE;

    snetCht2IPclActionApply(devObjPtr, descrPtr, actionCht2DataPtr,isFirst);

    /* restore counter-enable state */
    __LOG(("restore counter-enable state"));
    actionDataPtr->baseAction.matchCounterEn = saveCntrEn;

    if(SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceExtEgressIf,
                                    SNET_DST_INTERFACE_LLT_INDEX_E))
    {
        descrPtr->pceRoutLttIdx =
            actionDataPtr->pceExtEgressIf.interfaceInfo.routerLLTIndex;
    }
    else
    if(SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceExtEgressIf,
                                    SNET_DST_INTERFACE_VIRTUAL_ROUTER_E))
    {
        descrPtr->vrfId =
            actionDataPtr->pceExtEgressIf.interfaceInfo.virtualRouterId;
    }

    if (actionDataPtr->srcIdSetEn)
    {
        origSstId = descrPtr->sstId;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* remove the bits that should be set by this logic */
            descrPtr->sstId &= ~descrPtr->pclSrcIdMask;
            /* set the bits that should be set by this logic */
            descrPtr->sstId |= (actionDataPtr->srcId & descrPtr->pclSrcIdMask);

            descrPtr->pclAssignedSstId |= descrPtr->pclSrcIdMask;
            descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;/*just for legacy*/
        }
        else
        {
            descrPtr->sstId = actionDataPtr->srcId;
            descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;
        }

        if(origSstId != descrPtr->sstId)
        {
            __LOG(("SST assignment : sstId changed from [0x%x] to [0x%x] \n",
                origSstId,descrPtr->sstId));
        }

    }

    return ;
}


/**
* @internal snetCht3DataMaskKeyCompare function
* @endinternal
*
* @brief   Compare TCAM data with PCL key data
*/
static GT_BOOL snetCht3DataMaskKeyCompare
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U8 * dataPtr,
    IN GT_U8 * maskPtr,
    IN GT_U8 * keyPtr,
    IN GT_U32 compBits,

    IN GT_U32   indexInTcam,    /* for log info */
    IN GT_U32   bankIndex,      /* for log info */
    IN GT_U32   wordIndex      /* for log info */
)
{

    GT_U32 index;                   /* Byte index */
    GT_U32 bytes = compBits / 8;    /* Number of bytes to be compared */
    GT_U32 bits = compBits % 8;     /* Number of reminded bits to be compared */
    GT_U32 result;              /* Compare result status */
    GT_U32  bitMask;                /* mask for needed bits */
    GT_U32  ii;
    GT_U32  globalBitIndexForLog;
    GT_U32  x,y,k;
    GT_U32  byteIndex;
    GT_U32  bitIndex;
    GT_BOOL noMatch = GT_FALSE;
    GT_U32  startBit = 0;
    /*GT_U32  bankWidth;*/
    GT_U32  firstIndexBits;

    if(compBits == 52)/*xcat2,lion2*/
    {
        firstIndexBits = 50;
    }
    else
    {
        firstIndexBits = compBits;
    }

    if (wordIndex == 0)
    {
        startBit = compBits - firstIndexBits;
    }
    else
    {
        startBit = 0;
    }

    /* allow first line to be with different length then other lines */
    /* bankWidth = (compBits * 3) + firstIndexBits; */ /* 206 bits */

    if(wordIndex > 0)
    {
        /* allow first line to be with different length then other lines */
        globalBitIndexForLog = (wordIndex-1) * compBits + firstIndexBits;
    }
    else
    {
        globalBitIndexForLog = 0;
    }

    globalBitIndexForLog += (compBits == 52) ?
        (206 * devObjPtr->pclTcamInfoPtr->segmentIndex) : /* xcat2/Lion2*/
        (192 * devObjPtr->pclTcamInfoPtr->segmentIndex) ; /*ch1,2,3,xcat1,3*/

    /*globalBitIndexForLog += (bankIndex * bankWidth);*/

    for (index = 0; index < bytes; index++)
    {
        result = CH3_TCAM_X_Y_K_MATCH(dataPtr[index],maskPtr[index],keyPtr[index],0xff);
        if (result == 0)
        {

            if(compBits == 52 && /*xcat2,lion2*/
                globalBitIndexForLog == 0 &&
                index == 0 &&
                0 == CH3_TCAM_X_Y_K_MATCH(dataPtr[index],maskPtr[index],keyPtr[index],0x03))
            {
                /* no LOG indication needed , since the entry not valid for current lookup */
                return GT_FALSE;
            }

            noMatch = GT_TRUE;
            goto exit_func_lbl;
        }
    }

    if (bits)
    {
        bitMask = ((1 << bits) - 1);
        result = CH3_TCAM_X_Y_K_MATCH(dataPtr[index],maskPtr[index],keyPtr[index],bitMask);
        if (result == 0)
        {
            if(bytes == 0 && /* legacy devices(non xcat2,lion2) */
                globalBitIndexForLog == 0)
            {
                /* no LOG indication needed , since the entry not valid for current lookup */
                return GT_FALSE;
            }


            noMatch = GT_TRUE;
            goto exit_func_lbl;
        }
    }

exit_func_lbl:
    if(simLogIsOpenFlag && noMatch == GT_TRUE && (compBits > 8))
    {
        __LOG_NO_LOCATION_META_DATA(("indexInTcam[%d] wordIndex [%d] bankIndex[%d] \n",
            indexInTcam , wordIndex , bankIndex));

        __LOG_NO_LOCATION_META_DATA(("NOTE: in terms of CPSS the 'no match' cpssCurrentIndex [0x%x]([%d]) \n",
            indexInTcam,indexInTcam));

        /* analyze the bits that are 'NO match' */
        __LOG_NO_LOCATION_META_DATA(("analyze the GLOBAL bits that are 'NO match' \n"));
        __LOG_NO_LOCATION_META_DATA(("the global index is in terms of the FS that describes the TTI/PCL key \n"));

        /* the global index is in terms of the FS that describes the TTI/PCL key */

        __LOG_NO_LOCATION_META_DATA(("Non matched Bits:"));
        for(ii = startBit ; ii < compBits ; ii++ , globalBitIndexForLog++)
        {
            byteIndex = ii / 8;
            bitIndex = ii % 8;

            x = SMEM_U32_GET_FIELD(dataPtr[byteIndex],bitIndex,1);
            y = SMEM_U32_GET_FIELD(maskPtr[byteIndex],bitIndex,1);
            k = SMEM_U32_GET_FIELD(keyPtr[byteIndex] ,bitIndex,1);

            if(X_Y_K_FIND_NO_MATCH(x,y,k,1))
            {
                __LOG_NO_LOCATION_META_DATA(("%d,",
                    globalBitIndexForLog));
            }
        }
        __LOG_NO_LOCATION_META_DATA((". \n End of not matched Bits  \n\n"));
    }

    if (noMatch == GT_TRUE)
    {
        return GT_FALSE;
    }

    if(simLogIsOpenFlag && (compBits > 8))
    {
        GT_BOOL firstHit = GT_TRUE;

        for(ii = startBit ; ii < compBits ; ii++ , globalBitIndexForLog++)
        {
            byteIndex = ii / 8;
            bitIndex = ii % 8;

            x = SMEM_U32_GET_FIELD(dataPtr[byteIndex],bitIndex,1);
            y = SMEM_U32_GET_FIELD(maskPtr[byteIndex],bitIndex,1);

            if(x != y)/* indication that key match 0 or 1 ant not ignored ! */
            {
                if(firstHit == GT_TRUE)
                {
                    firstHit = GT_FALSE;
                    /* explain the bits that where 'exact match' : */
                    __LOG_NO_LOCATION_META_DATA(("indexInTcam[%d] wordIndex [%d] bankIndex[%d] \n",
                        indexInTcam , wordIndex , bankIndex));

                    __LOG_NO_LOCATION_META_DATA(("NOTE: in terms of CPSS the 'match' cpssCurrentIndex [0x%x]([%d]) \n",
                        indexInTcam,indexInTcam));
                    __LOG_NO_LOCATION_META_DATA(("the global index is in terms of the FS that describes the TTI/PCL key \n"));

                    /* the global index is in terms of the FS that describes the TTI/PCL key */

                    __LOG_NO_LOCATION_META_DATA(("exact matched Bits: (not don't care)"));
                }
                __LOG_NO_LOCATION_META_DATA(("%d,",
                    globalBitIndexForLog));
            }
        }

        if(firstHit == GT_FALSE)
        {
            __LOG_NO_LOCATION_META_DATA((". \n End of matched Bits  \n\n"));
        }
        else
        {
            /* all bits are 'don't care' */
            __LOG_NO_LOCATION_META_DATA(("\n all bits in the bank[%d] are 'don't care' (start bit[%d],end bit[%d])  \n\n",
                wordIndex,
                globalBitIndexForLog - (compBits - startBit),
                globalBitIndexForLog - 1));
        }

    }

    return GT_TRUE;
}


