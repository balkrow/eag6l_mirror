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
* @file snetCheetah2EPcl.c
*
* @brief Cheetah2 Asic Simulation .
* Egress Policy Engine processing for outgoing frame.
* Source Code file.
*
* @version   22
********************************************************************************
*/

#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2EPcl.h>
#include <asicSimulation/SLog/simLog.h>

/* number of compares to be done on Layer 4 ports */
#define SNET_CHT2_L4_TCP_PORT_COMP_RANGE_INDEX_CNS      (0x8)

/**
* @internal snetCht2EPclActionApply function
* @endinternal
*
* @brief   Apply the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr   - pointer to the ePCL action entry.
*
* @note C.12.14 - Egress PCL action table and Policy TCAM access control
*       Registers
*
*/
GT_VOID snetCht2EPclActionApply
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_CHT2_EPCL_ACTION_STC            * actionDataPtr
)
{
    /* For Ch3 matching counter update is done as part of the CNC code */
    /* and not here */

    /* Remarking the QOS profile .                                       */
    /* Enables the modification of the packet 802.1q User Priority field */
    descrPtr->modifyUp = actionDataPtr->pceModifyUp;
    if (actionDataPtr->pceModifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E) /* 0 - no modification */
    {                                    /* 2 ,3 are reserved values */
       /*ModifyOuterTag : EPCL Action entry <Tag0 VID> is assigned to the outer tag of the packet.
          If the packet is tunneled, this refers to the tunnel header tag.*/

       descrPtr->up = actionDataPtr->epceUp;
    }

    /* Enables the modification of the packet 802.1q DSCP field */
    if (actionDataPtr->pceModifyDscp == 1) /* 0 - no modification */
    {                                      /* 2 ,3 are reserved values */
       descrPtr->dscp = actionDataPtr->epceDscp;
       descrPtr->modifyDscp = 1;
    }

    descrPtr->epclAction.modifyUp = actionDataPtr->pceModifyUp;
    descrPtr->epclAction.modifyDscp = actionDataPtr->pceModifyDscp;
    descrPtr->epclAction.up = actionDataPtr->epceUp;
    descrPtr->epclAction.dscp = actionDataPtr->epceDscp;
    descrPtr->epclAction.drop = actionDataPtr->fwdCmd;
    descrPtr->epclAction.modifyVid0 = actionDataPtr->epceVlanCmd;
    descrPtr->epclAction.vid0 = actionDataPtr->epceVid;

    if(devObjPtr->ePclKeyFormatVersion >= 2)
    {
        /*Tag1 VLAN Command*/
        descrPtr->epclAction.vlan1Cmd = actionDataPtr->vlan1Cmd;
        /*Enable Modify UP1*/
        descrPtr->epclAction.modifyUp1 = actionDataPtr->modifyUp1;
        /*Tag1 VID*/
        descrPtr->epclAction.vid1 = actionDataPtr->vid1;
        /*UP1*/
        descrPtr->epclAction.up1 =  actionDataPtr->up1;
    }



    return ;
}

/**
* @internal snetCht2EPclActionGet function
* @endinternal
*
* @brief   Get the action entry from the policy action table.
*         The function used for CH3 and above.
*         For CH2 used another function due to FEr#1087.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action table .
*
* @note C.12.12 - The policy Engine maintains an 1024 entries table, corresponding
*       to the 1024 rules that may be defined in the TCAM lookup
*       structure.
*
*/
GT_VOID snetCht2EPclActionGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr  ,
    IN GT_U32                             matchIndex,
    OUT SNET_CHT2_EPCL_ACTION_STC       * actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetCht2EPclActionGet);

    GT_U32  tblAddr;
    GT_U32  * actionEntryDataPtr;

    tblAddr = SMEM_CHT_PCL_ACTION_TBL_MEM(devObjPtr, matchIndex) ;
    actionEntryDataPtr = smemMemGet(devObjPtr,tblAddr);

    /* Read word 0 from the action table entry */
    __LOG(("Read word 0 from the action table entry"));
    actionDataPtr->matchCounterIndex = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],26,6);
    actionDataPtr->matchCounterEn = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],25,1);
    actionDataPtr->pceModifyUp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],23,2);
    actionDataPtr->pceModifyDscp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],21,2);
    actionDataPtr->epceUp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],18,3);
    actionDataPtr->epceDscp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],12,6);
    actionDataPtr->fwdCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],0,3);

    /* Read word 1 from the action table entry */
    __LOG(("Read word 1 from the action table entry"));
    actionDataPtr->epceVid = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],19,12);
    actionDataPtr->epceVlanCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],17,2);

    return ;
}

/**
* @internal snetCht2EPclActionGetWithCh2ErrataVlanAndConter function
* @endinternal
*
* @brief   Get the action entry from the policy action table.
*         Supports FEr#1087 - CH2 only
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action table .
*
* @note Wrong HW format described below:
*       Policy action entry world0:
*       Bit[30], MatchCounterEn
*       Bits[29:25], MatchCounterIndex
*       Policy action entry world1:
*       Bits[30:20], PCE_VID[10:0]
*       Bits[19:18], PCE_VLANCmd
*       Policy action entry world2:
*       Bit[0], PCE_VID[11]
*
*/
GT_VOID snetCht2EPclActionGetWithCh2ErrataVlanAndConter
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr  ,
    IN GT_U32                             matchIndex,
    OUT SNET_CHT2_EPCL_ACTION_STC       * actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetCht2EPclActionGetWithCh2ErrataVlanAndConter);

    GT_U32  tblAddr;
    GT_U32  * actionEntryDataPtr;

    tblAddr = SMEM_CHT_PCL_ACTION_TBL_MEM(devObjPtr, matchIndex) ;
    actionEntryDataPtr = smemMemGet(devObjPtr,tblAddr);

    /* Read word 0 from the action table entry */
    __LOG(("Read word 0 from the action table entry"));
    actionDataPtr->matchCounterIndex = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],25,5);
    actionDataPtr->matchCounterEn = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],30,1);
    actionDataPtr->pceModifyUp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],23,2);
    actionDataPtr->pceModifyDscp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],21,2);
    actionDataPtr->epceUp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],18,3);
    actionDataPtr->epceDscp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],12,6);
    actionDataPtr->fwdCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],0,3);

    /* Read word 1 from the action table entry */
    __LOG(("Read word 1 from the action table entry"));
    actionDataPtr->epceVid =
        SMEM_U32_GET_FIELD(actionEntryDataPtr[1],20,11)
        | ((SMEM_U32_GET_FIELD(actionEntryDataPtr[2],1,1)) << 11);
    actionDataPtr->epceVlanCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],18,2);

    return ;
}





