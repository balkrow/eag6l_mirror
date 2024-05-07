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
* @file snetCheetah2Pcl.c
*
* @brief Cheetah2 Asic Simulation .
* Ingress Policy Engine processing for incoming frame.
* Source Code file.
*
* @version   31
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Pcl.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SLog/simLog.h>


/**
* @internal snetCht2IPclActionApply function
* @endinternal
*
* @brief   Apply the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr   - pointer to the PCL action entry.
* @param[in] isFirst                  - is first action apply
*
* @note C.12.13   Policy Tcam Table : The policy TCAM holds 1024 lines
*       of 24 bytes or rule data used for ingress and egress PCL.
*
*/
GT_VOID snetCht2IPclActionApply
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_CHT2_PCL_ACTION_STC             * actionDataPtr,
    IN GT_BOOL                              isFirst
)
{
    DECLARE_FUNC_NAME(snetCht2IPclActionApply);



    GT_BOOL updateVid = GT_FALSE; /* update eVid flag */

    if (actionDataPtr->pceRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E)
    {
        /* Route action will be applied later in UC route engine */
        __LOG(("Route action will be applied later in UC route engine"));
        return;
    }

    /* Remarking the QOS profile . relevant only is Qos precedence of the
       previous Qos Assignment mechanism is soft */
    if (descrPtr->qosProfilePrecedence == SKERNEL_QOS_PROF_PRECED_SOFT)
    {
        /* Enables the modification of the packet 802.1q User Priority field */
        __LOG(("Enables the modification of the packet 802.1q User Priority field"));
        descrPtr->modifyUp = (actionDataPtr->pceModifyUp == 1) ? 1 :
                             (actionDataPtr->pceModifyUp == 2) ? 0 : descrPtr->modifyUp;

        /* Enables the modification of the packet 802.1q dscp field */
        __LOG(("Enables the modification of the packet 802.1q dscp field"));
        descrPtr->modifyDscp = (actionDataPtr->pceModifyDscp == 1) ? 1 :
                               (actionDataPtr->pceModifyDscp == 2) ? 0 : descrPtr->modifyDscp;

        /* Enables the modification of the qos profile index */
        __LOG(("Enables the modification of the qos profile index"));
        if(actionDataPtr->pceQosProfileMarkingEn == GT_TRUE)
        {
            descrPtr->qos.qosProfile = actionDataPtr->pceQosProfile;
        }
        descrPtr->qosProfilePrecedence =  actionDataPtr->pceQosPrecedence ;
    }

    /* enables the mirroring of the packet to the ingress analyzer port */
    __LOG(("enables the mirroring of the packet to the ingress analyzer port"));
    if(actionDataPtr->mirrorToIngressAnalyzerPort)
    {
        descrPtr->rxSniff = 1;
    }

    /* Select mirror analyzer index */
    __LOG(("Select mirror analyzer index"));
    snetXcatIngressMirrorAnalyzerIndexSelect(devObjPtr, descrPtr,
        actionDataPtr->mirrorToIngressAnalyzerPort);


    /* resolve packet command and CPU code */
    __LOG(("resolve packet command and CPU code"));
    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              actionDataPtr->fwdCmd,
                                              descrPtr->cpuCode,
                                              actionDataPtr->pceCpuCode,
                                              SNET_CHEETAH_ENGINE_UNIT_PCL_E,
                                              isFirst);


    /*  VID re-assignment is relevant only is the VID precedence , set by the
        previous VID assignment mechanism is soft */
    if  (descrPtr->preserveVid == 0)
    {
        descrPtr->preserveVid = actionDataPtr->pceVidPrecedence;
        /* PCE_VID is assigned for all packets */
        __LOG(("PCE_VID is assigned for all packets"));
        if (actionDataPtr->pceVlanCmd == 3)
        {
            updateVid = GT_TRUE;
        }
        else if ((actionDataPtr->pceVlanCmd == 2) &&
                 (descrPtr->origSrcTagged == 1))
        {
            updateVid = GT_TRUE;
        }
        else if ((actionDataPtr->pceVlanCmd == 1) &&
                 (descrPtr->origSrcTagged == 0))
        {
            updateVid = GT_TRUE;
        }
        if(updateVid == GT_TRUE)
        {
            descrPtr->eVid = actionDataPtr->pceVid;
            descrPtr->vidModified = 1;
        }
    }

    /* Nested Vid  */
    if(descrPtr->nestedVlanAccessPort == 0)
    {
        /*When <EnNestedVLAN> is set, this rule matching flow is defined as a Nested VLAN Access Flow.*/
        __LOG(("When <EnNestedVLAN> is set, this rule matching flow is defined as a Nested VLAN Access Flow."));
        descrPtr->nestedVlanAccessPort = actionDataPtr->enNestedVlan;
        if(actionDataPtr->enNestedVlan)
        {
            /* When a packet received on an access flow is transmitted via a
               tagged port or a cascading port, a VLAN tag is added to the packet
               (on top of the existing VLAN tag, if any). The VID field is the
               VID assigned to the packet as a result of all VLAN assignment
               algorithms. */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;
        }
    }


    /* when set to 1 , this rule is bound to the policer indexed    *
     *   by actionDataPtr->policerIndex                              */
    descrPtr->policerEn = actionDataPtr->policerEn ;

    if(descrPtr->policerEn == 1)
    {
        descrPtr->policerPtr = actionDataPtr->policerIndex;
    }

    /* copy the redirection command */
    __LOG(("copy the redirection command"));
    if (actionDataPtr->pceRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        descrPtr->useVidx =
            SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf, SNET_DST_INTERFACE_VIDX_E);
        if(descrPtr->useVidx)
        {
            descrPtr->eVidx = actionDataPtr->pceEgressIf.interfaceInfo.vidx;
        }
        else
        {
            descrPtr->targetIsTrunk =
                SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf, SNET_DST_INTERFACE_TRUNK_E);

            if(descrPtr->targetIsTrunk)
            {
                descrPtr->trgTrunkId = actionDataPtr->pceEgressIf.interfaceInfo.trunkId;
            }
            else
            {
                descrPtr->trgDev = actionDataPtr->pceEgressIf.interfaceInfo.devPort.devNum;
                descrPtr->trgEPort = actionDataPtr->pceEgressIf.interfaceInfo.devPort.port;
            }
        }
        if (actionDataPtr->tunnelStart == 1)
        {/* indicate this action is a T.S */
            __LOG(("indicate this action is a T.S"));
            descrPtr->tunnelStart = GT_TRUE;
            descrPtr->tunnelPtr = actionDataPtr->tunnelPtr;
            descrPtr->tunnelStartPassengerType  = actionDataPtr->tunnelType;
        }
        descrPtr->VntL2Echo = actionDataPtr->VNTL2Echo;
    }

    return ;
}


/**
* @internal snetCht2IPclActionGet function
* @endinternal
*
* @brief   Get the action entry from the policy action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action table .
*
* @note C.12.13   Policy Tcam Table : The policy TCAM holds 1024 lines
*       of 24 bytes or rule data used for ingress and egress PCL.
*
*/
extern GT_VOID snetCht2IPclActionGet
(
    IN SKERNEL_DEVICE_OBJECT            *   devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *   descrPtr,
    IN GT_U32                               matchIndex,
    OUT SNET_CHT2_PCL_ACTION_STC        *   actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetCht2IPclActionGet);

    GT_U32  tblAddr;
    GT_U32  * actionEntryDataPtr;
    GT_BIT enable;

    tblAddr = SMEM_CHT_PCL_ACTION_TBL_MEM(devObjPtr, matchIndex) ;
    actionEntryDataPtr = smemMemGet(devObjPtr,tblAddr);

    actionDataPtr->pceRedirectCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],0,2);

    descrPtr->pclRedirectCmd = actionDataPtr->pceRedirectCmd;

    if (actionDataPtr->pceRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E)
    {
        return;
    }

    /* Read word 0 from the action table entry */
    __LOG(("Read word 0 from the action table entry"));
    actionDataPtr->matchCounterIndex = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],26,5);
    actionDataPtr->matchCounterEn = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],25,1);
    actionDataPtr->pceModifyUp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],23,2);
    actionDataPtr->pceModifyDscp = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],21,2);
    actionDataPtr->pceQosProfile  = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],14,7);
    actionDataPtr->pceQosProfileMarkingEn  = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],13,1);
    actionDataPtr->pceQosPrecedence = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],12,1);
    actionDataPtr->mirrorToIngressAnalyzerPort = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],11,1);
    actionDataPtr->pceCpuCode = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],3,8);
    actionDataPtr->fwdCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[0],0,3);

    /* Read word 1 from the action table entry */
    __LOG(("Read word 1 from the action table entry"));
    actionDataPtr->pceVid = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],19,12);
    actionDataPtr->pceVlanCmd = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],17,2);
    actionDataPtr->enNestedVlan = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],16,1);
    actionDataPtr->pceVidPrecedence = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],15,1);
    enable = SMEM_U32_GET_FIELD(actionEntryDataPtr[1],14,1);
    SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf,
                                 SNET_DST_INTERFACE_VIDX_E, enable);
    if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf,
                                     SNET_DST_INTERFACE_VIDX_E))
    {
        actionDataPtr->pceEgressIf.interfaceInfo.vidx =
            (GT_U16)SMEM_U32_GET_FIELD(actionEntryDataPtr[1],2,12);
    }
    else
    {
        enable = (GT_U8)SMEM_U32_GET_FIELD(actionEntryDataPtr[1],2,1);
        SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf,
                                     SNET_DST_INTERFACE_TRUNK_E, enable);
        if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf,
                                         SNET_DST_INTERFACE_TRUNK_E))
        {
            actionDataPtr->pceEgressIf.interfaceInfo.trunkId =
                    (GT_U8)SMEM_U32_GET_FIELD(actionEntryDataPtr[1],3,7);
        }
        else
        {
            GT_U32  tmpVal=1;
            SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->pceEgressIf,
                                         SNET_DST_INTERFACE_PORT_E, tmpVal);

            actionDataPtr->pceEgressIf.interfaceInfo.devPort.port =
                    (GT_U8)SMEM_U32_GET_FIELD(actionEntryDataPtr[1],3,6);
            actionDataPtr->pceEgressIf.interfaceInfo.devPort.devNum =
                    (GT_U8)SMEM_U32_GET_FIELD(actionEntryDataPtr[1],9,5);
        }
    }

    /* Read word 2 from the action table entry */
    __LOG(("Read word 2 from the action table entry"));
    actionDataPtr->tunnelType  = SMEM_U32_GET_FIELD(actionEntryDataPtr[2],21,1);
    actionDataPtr->tunnelPtr   = SMEM_U32_GET_FIELD(actionEntryDataPtr[2],11,10);
    actionDataPtr->tunnelStart = SMEM_U32_GET_FIELD(actionEntryDataPtr[2],10,1);
    actionDataPtr->VNTL2Echo   = SMEM_U32_GET_FIELD(actionEntryDataPtr[2],9,1);
    actionDataPtr->policerIndex= SMEM_U32_GET_FIELD(actionEntryDataPtr[2],1,9);
    actionDataPtr->policerEn   = SMEM_U32_GET_FIELD(actionEntryDataPtr[2],0,1);

    return ;
}


/**
* @internal snetCht2SrvPclTcamLookUp function
* @endinternal
*
* @brief   Tcam lookup .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] pclKeyPtr                - pointer to PCL key .
*
* @param[out] matchIndexPtr            - pointer to the matching index.
*                                      RETURN:
*                                      COMMENTS:
*                                      C.12.13     Policy Tcam Table : The policy TCAM holds 1024 lines
*                                      of 24 bytes or rule data used for ingress and egress PCL.
*
* @note C.12.13   Policy Tcam Table : The policy TCAM holds 1024 lines
*       of 24 bytes or rule data used for ingress and egress PCL.
*
*/
extern GT_VOID snetCht2SrvPclTcamLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr  ,
    IN SNET_CHT_POLICY_KEY_STC    * pclKeyPtr,
    OUT GT_U32 *  matchIndexPtr
)
{
    DECLARE_FUNC_NAME(snetCht2SrvPclTcamLookUp);

    GT_U32   maxEntries;  /* 512 entries for extended PCE , 1024 for standard*/
    GT_U32   entryIndex;  /* Index to the TCAM rule */
    GT_U32   wordIndex;   /* Index to the word in the TCAM rule */
    GT_U32 * policyTcamEntryDataPtr; /* pointer to policy TCAM data entry  */
    GT_U32 * policyTcamEntryCtrlPtr; /* pointer to policy TCAM ctrl entry  */
    GT_U32 * policyTcamEntryDataMaskPtr; /* pointer to policy TCAM data mask entry */
    GT_U32 * policyTcamEntryCtrlMaskPtr; /* pointer to policy TCAM ctrl mask entry  */
    GT_U32   compModeData; /* compare mode bit in the tcam rule */
    GT_U32 * pclSearchKeyPtr; /* pointer to pcl tcam search0 key */
    GT_U32 * pclSearchKeyPtr1; /* pointer to pcl tcam search1 key */
    GT_U32   keySize;/* keysize . 5 words for standard key and 10 for extended*/
    GT_U32   policyTcamData32Bits;/* policy Tcam data 32 lower bits   */
    GT_U32   policyTcamData16Bits;/* policy Tcam data 16 upper bits   */
    GT_U32   pclSearchKey16Bits ;/* pcl tcam search pcl 32 lower bits   */
    GT_U32   pclSearchKey32Bits ; /* pcl tcam search pcl 16 lower bits   */
    GT_U32  *policyTcamEntryDataSecondHalfPtr;/* pointer to middle of data tcam table */
    GT_U32  *policyTcamEntryCtrlSecondHalfPtr;/* pointer to middle of ctrl tcam table */
    GT_U32  *policyTcamEntryDataMaskSecondHalfPtr;/* pointer to middle of data tcam mask table */
    GT_U32  *policyTcamEntryCtrlMaskSecondHalfPtr;/* pointer to middle of ctrl tcam mask table */
    GT_U32  *dataPtr;
    GT_U32  *dataMaskPtr;
    GT_U32  *ctrlPtr;
    GT_U32  *ctrlMaskPtr;
    GT_U32  memTcamAddr;

    if (pclKeyPtr->pclKeyFormat == CHT_PCL_KEY_EXTENDED_E)
    {
        maxEntries = SNET_CHT2_TCAM_1024_EXTENDED_RULES_CNS ;
        pclSearchKeyPtr1 = (GT_U32*)&(pclKeyPtr->key.extended);
        keySize = 10 ;
    }
    else
    {
        maxEntries = SNET_CHT2_TCAM_512_STANDARD_RULES_CNS  ;
        pclSearchKeyPtr1 = (GT_U32*)&(pclKeyPtr->key.regular);
        keySize = 5 ;
    }

    /* initialize the matchIndexPtr */
    *matchIndexPtr = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;

    memTcamAddr = SMEM_CHT_PCL_TCAM_TBL_MEM(devObjPtr, 0, 0);

    /*  Get pointer to Tcam data entry */
    policyTcamEntryDataPtr = smemMemGet(devObjPtr, memTcamAddr);

    /*  Get pointer to Tcam control entry */
    policyTcamEntryCtrlPtr = smemMemGet(devObjPtr, memTcamAddr + 0x4);

    /*  Get pointer to Tcam data mask entry */
    policyTcamEntryDataMaskPtr = smemMemGet(devObjPtr, memTcamAddr + 0x8);

    /*  Get pointer to Tcam control mask entry */
    policyTcamEntryCtrlMaskPtr = smemMemGet(devObjPtr, memTcamAddr + 0xc);

    /*  Get pointer to Tcam data entry second half table */
    policyTcamEntryDataSecondHalfPtr =  policyTcamEntryDataPtr +
            CHT2_PCL_TCAM_OFFSET_TO_SECOND_HALF_TABLE_IN_WORD_CNS ;

    /*  Get pointer to Tcam ctrl entry second half table */
    policyTcamEntryCtrlSecondHalfPtr = policyTcamEntryCtrlPtr +
            CHT2_PCL_TCAM_OFFSET_TO_SECOND_HALF_TABLE_IN_WORD_CNS ;

    /*  Get pointer to Tcam data mask entry second half table */
    policyTcamEntryDataMaskSecondHalfPtr =  policyTcamEntryDataMaskPtr +
            CHT2_PCL_TCAM_OFFSET_TO_SECOND_HALF_TABLE_IN_WORD_CNS ;

    /*  Get pointer to Tcam ctrl mask entry second half table */
    policyTcamEntryCtrlMaskSecondHalfPtr =  policyTcamEntryCtrlMaskPtr +
            CHT2_PCL_TCAM_OFFSET_TO_SECOND_HALF_TABLE_IN_WORD_CNS ;

    for (entryIndex = 0 ; entryIndex <  maxEntries;  ++entryIndex,
            policyTcamEntryDataPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryCtrlPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryDataMaskPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryCtrlMaskPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryDataSecondHalfPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryCtrlSecondHalfPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryDataMaskSecondHalfPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS,
            policyTcamEntryCtrlMaskSecondHalfPtr += CHT_PCL2_TCAM_ENTRY_WIDTH_CNS
        )
    {
        pclSearchKeyPtr = pclSearchKeyPtr1;
        dataPtr = policyTcamEntryDataPtr;
        ctrlPtr = policyTcamEntryCtrlPtr;
        dataMaskPtr = policyTcamEntryDataMaskPtr;
        ctrlMaskPtr = policyTcamEntryCtrlMaskPtr;

        for (wordIndex = 0 ; wordIndex <  keySize  ; ++wordIndex,
                                dataPtr += CHT_PCL2_TCAM_WORDS_WIDTH_CNS,
                                ctrlPtr += CHT_PCL2_TCAM_WORDS_WIDTH_CNS,
                                dataMaskPtr += CHT_PCL2_TCAM_WORDS_WIDTH_CNS,
                                ctrlMaskPtr += CHT_PCL2_TCAM_WORDS_WIDTH_CNS
            )
        {
            if (wordIndex == 5)
            {
                dataPtr =  policyTcamEntryDataSecondHalfPtr;
                ctrlPtr =  policyTcamEntryCtrlSecondHalfPtr;
                dataMaskPtr = policyTcamEntryDataMaskSecondHalfPtr;
                ctrlMaskPtr =  policyTcamEntryCtrlMaskSecondHalfPtr;
            }

            /* check validity of entry */
            __LOG(("check validity of entry"));
            if (( ((ctrlPtr[0] >> 0x11) & 0x1) &
                  ((ctrlMaskPtr[0] >> 0x11) & 0x1)) == 0x0)
            {
                break; /* not valid entry  - go to the next entry */
            }
            /* check the compmode */
            __LOG(("check the compmode"));
            compModeData =  ( ((ctrlPtr[0] >> 0x12) & 0x3 ) &
                               ((ctrlMaskPtr[0] >> 0x12) & 0x3) );
            if ((( compModeData == 1 ) &&  ( pclKeyPtr->pclKeyFormat == CHT_PCL_KEY_EXTENDED_E )) ||
                (( compModeData == 2 ) &&  ( pclKeyPtr->pclKeyFormat == CHT_PCL_KEY_REGULAR_E)))
            {
                break; /* compmode is not as expected - go to the next entry */
            }

            if ( (wordIndex == 1) || (wordIndex == 6) )
            {
                /* calculate 48 bits for word1 */
                __LOG(("calculate 48 bits for word1"));
                policyTcamData16Bits = (SMEM_U32_GET_FIELD(ctrlPtr[0],0,15) &
                                       SMEM_U32_GET_FIELD(ctrlMaskPtr[0],0,15));
                policyTcamData32Bits = (dataPtr[0] & dataMaskPtr[0]);

                /* calculate 48 bits for word1 from the PCL search key */
                __LOG(("calculate 48 bits for word1 from the PCL search key"));
                pclSearchKey16Bits = SMEM_U32_GET_FIELD(pclSearchKeyPtr[1],0,15);
                pclSearchKey32Bits = pclSearchKeyPtr[0];
                pclSearchKeyPtr++;

                pclSearchKey16Bits &= SMEM_U32_GET_FIELD(ctrlMaskPtr[0],0,15);
                pclSearchKey32Bits &= dataMaskPtr[0];
            }
            else if ( (wordIndex == 2) || (wordIndex ==7) )
            {
                /* calculate 48 bits for word2 */
                __LOG(("calculate 48 bits for word2"));
                policyTcamData16Bits = (SMEM_U32_GET_FIELD(ctrlPtr[0],0,15) &
                                        SMEM_U32_GET_FIELD(ctrlMaskPtr[0],0,15));
                policyTcamData32Bits = (dataPtr[0] & dataMaskPtr[0]);

                /* calculate 48 bits for word2 from the PCL search key */
                __LOG(("calculate 48 bits for word2 from the PCL search key"));
                pclSearchKey32Bits = SMEM_U32_GET_FIELD(pclSearchKeyPtr[0],16,16);
                pclSearchKey32Bits |= SMEM_U32_GET_FIELD(pclSearchKeyPtr[1],0,16) << 16;
                pclSearchKey16Bits = SMEM_U32_GET_FIELD(pclSearchKeyPtr[1],16,16);
                pclSearchKeyPtr += 2;

                pclSearchKey16Bits &= SMEM_U32_GET_FIELD(ctrlMaskPtr[0],0,15);
                pclSearchKey32Bits &= dataMaskPtr[0];
            }
            else
            {
                /* 16 extended bits for word1-2 are zero */
                __LOG(("16 extended bits for word1-2 are zero"));
                pclSearchKey16Bits = 0;
                policyTcamData16Bits = 0;

                /* calculate 32 bits for word 0 , word3 and word4 */
                __LOG(("calculate 32 bits for word 0 , word3 and word4"));
                policyTcamData32Bits = (dataPtr[0] & dataMaskPtr[0]);
                pclSearchKey32Bits = pclSearchKeyPtr[0];
                pclSearchKeyPtr++;

                pclSearchKey32Bits &= dataMaskPtr[0];
            }

            /* lookup in TCAM */
            __LOG(("lookup in TCAM"));
            if ((policyTcamData16Bits != pclSearchKey16Bits) ||
                (policyTcamData32Bits != pclSearchKey32Bits))
                break;

        }

        if (wordIndex == keySize)
        {
            *matchIndexPtr = entryIndex;
            break ;
        }
    }

    return ;
}


