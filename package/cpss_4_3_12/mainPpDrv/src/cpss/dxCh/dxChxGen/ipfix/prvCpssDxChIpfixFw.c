/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvCpssDxChIpfixFw.c
*
* @brief Private CPSS APIs implementation for Ipfix Fw.
*
* @version   1
*********************************************************************************
**/
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixFw.h>


/* set bits in input stream */
static GT_VOID prvCpssDxChIpfixFwBitsSet
(
    IN    GT_U32   offset,
    IN    GT_U32   numBits,
    IN    GT_U32   val,
    INOUT GT_U8    *data
)
{
    GT_U32 ii, byte, bitShift;
    GT_U8 mask = 0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        mask = ~(1 << bitShift);
        data[byte] |= (data[byte] & mask) | (((val >> ii) & 1) << bitShift);
    }
}

/* get bits from input stream */
static GT_U32 prvCpssDxChIpfixFwBitsGet
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBits
)
{
    GT_U32 ii, byte, bitShift, outData=0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        outData  |= ((data[byte] >> bitShift) & 1) << ii;
    }
    return outData;
}

static GT_UINTPTR prvCpssDxChFwChannelGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum
)
{
    return  (GT_UINTPTR)(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[sCpuNum]);
}

/**
* @internal prvCpssDxChScpuIdGet function
* @endinternal
*
* @brief   Retrieve the SCPU ID for IPFIX FW

* @param[in] devNum               - device number
*
* @retval scpu id
*/
static GT_U8 prvCpssDxChScpuIdGet
(
    IN  GT_U8   devNum
)
{
    GT_U8 scpuid;

    for (scpuid = 0; scpuid < PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS; scpuid++)
    {
        /* IPFIX FW channel used for IPC communication should have non-zero
         * value assignment post successful FW load.
         */
        if((GT_UINTPTR)(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->ipcDevCfg[scpuid]) != 0)
        {
            return scpuid;
        }
    }

    /* If IPFIX FW is not loaded return 0xFF to indicate no SCPUID in use */
    return 0xFF;
}

/**
 * @internal prvCpssDxChIpfixFwIpcMessageSend function
 * @endinternal
 *
 * @param[in] devNum   - device number
 * @param[in] devNum   - service cpu number
 * @param[in] ipcMsg   - IPC messae
 *
 * @retval    GT_OK     - on success
 * @retval    GT_FAIL   - on ipc error
 */
GT_STATUS prvCpssDxChIpfixFwIpcMessageSend
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT     *ipcMsgPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       ipcData[PRV_CPSS_DXCH_IPFIX_FW_IPC_MSG_MAX_SIZE_IN_BYTES_CNS] = {0};
    GT_UINTPTR  fwChannel = 0;
    GT_U32      startOffset = 0;
    GT_U32      i;
    GT_U32      ipcMsgSize = 0;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT  opcode;

    (void) i;

    fwChannel = prvCpssDxChFwChannelGet(devNum, sCpuNum);

    opcode = ipcMsgPtr->opcodeGet.opcode;

    /* opcode */
    ipcData[0] = (GT_U8)opcode;

    switch (opcode)
    {
    /* Entry Add */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_ADD_E:
        /* Flow ID Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(    16, 16, entryAdd.flowId);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* EmIndex Word1[19:0] (20 bits) */
            PRV_IPFIX_FW_BITS_SET(32 + 0, 20, entryAdd.entryParams.emIndex);
        }
        else
        {
            /* First Timestamp Word1[14:0] (15 bits) */
            PRV_IPFIX_FW_BITS_SET(32 + 0, 15, entryAdd.entryParams.firstTs);
            /* First Timestamp Valid Word1[15] (1 bit) */
            PRV_IPFIX_FW_BITS_SET(32 + 15, 1, entryAdd.entryParams.firstTsValid);
        }

        ipcMsgSize = 2 * 4; /* in bytes */
        break;

    /* Entry Delete */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_E:
        /* Flow ID Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(    16, 16, entryDelete.flowId);
        ipcMsgSize = 1 * 4; /* in bytes */

        break;

    /* Global config set */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_SET_E:
        /* ipfix enable Word0[8] (1 bit)  */
        PRV_IPFIX_FW_BITS_SET(8, 1, globalConfigSet.globalCfg.ipfixEnable);
        /* idle aging offload Word0[9] (1 bit)  */
        PRV_IPFIX_FW_BITS_SET(9, 1, globalConfigSet.globalCfg.idleAgingOffload);
        /* long aging offload Word0[10] (1 bit)  */
        PRV_IPFIX_FW_BITS_SET(10, 1, globalConfigSet.globalCfg.longAgingOffload);
        /* counters extension offload Word0[11] (1 bit)  */
        PRV_IPFIX_FW_BITS_SET(11, 1, globalConfigSet.globalCfg.countersExtensionOffload);
        /* timestamps extension offload Word0[12] (1 bit)  */
        PRV_IPFIX_FW_BITS_SET(12, 1, globalConfigSet.globalCfg.timeStampsExtensionOffload);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* first policer stage Word0[15:13] (3 bits) */
            PRV_IPFIX_FW_BITS_SET(13, 3, globalConfigSet.globalCfg.firstPolicerStage);
            /* max in-transit index Word0[31:16] (16-bit) */
            PRV_IPFIX_FW_BITS_SET(16, 16, globalConfigSet.globalCfg.maxInTransitIndex);
        }
        else
        {
            /* max in-transit index Word0[31:16] (16-bit) */
            PRV_IPFIX_FW_BITS_SET(16, 1, globalConfigSet.globalCfg.ipfixEntriesPerFlow);
        }

        startOffset = 32;
        /* idleTimeout Word1[11:0] (12 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 12, globalConfigSet.globalCfg.idleTimeout);
        /* activeTimeout Word1[27:16] (12 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset+16, 12, globalConfigSet.globalCfg.activeTimeout);

        startOffset = 2*32;
        /* scan cycle interval Word2[9:0] (10 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 10, globalConfigSet.globalCfg.scanCycleInterval);
        /* scan loop tiems Word2[25:10] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset+10, 16, globalConfigSet.globalCfg.scanLoopItems);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* second policer stage Word2[28:26] (3 bits) */
            PRV_IPFIX_FW_BITS_SET(startOffset+26, 3, globalConfigSet.globalCfg.secondPolicerStage);
            /* repot new flows Word2[30] (1 bit) */
            PRV_IPFIX_FW_BITS_SET(startOffset+30, 1, globalConfigSet.globalCfg.reportNewFlows);
            /* repot new flows Word2[30] (1 bit) */
            PRV_IPFIX_FW_BITS_SET(startOffset+31, 1, globalConfigSet.globalCfg.hwAutoLearnEnable);

            startOffset = 3 * 32;
            if (ipcMsgPtr->globalConfigSet.globalCfg.hwAutoLearnEnable == GT_TRUE)
            {
                /* max firmware index Word3[15:0] (16 bits) */
                PRV_IPFIX_FW_BITS_SET(startOffset, 16, globalConfigSet.globalCfg.maxFwIndex);
                /* max app ipfix index Word3[31:16] (16 bits) */
                PRV_IPFIX_FW_BITS_SET(startOffset + 16, 16, globalConfigSet.globalCfg.maxAppIndex);
            }
            else
            {
                /* first policer max ipfix index Word3[15:0] (16 bits) */
                PRV_IPFIX_FW_BITS_SET(startOffset, 16, globalConfigSet.globalCfg.firstPolicerMaxIpfixIndex);
                /* second policer max ipfix index Word3[31:16] (16 bits) */
                PRV_IPFIX_FW_BITS_SET(startOffset + 16, 16, globalConfigSet.globalCfg.secondPolicerMaxIpfixIndex);
            }
        }

        /* ipc message size is 4 words */
        ipcMsgSize = 4 * 4; /* in bytes */
        break;

    /* Port Group config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_SET_E:
        startOffset = 32;
        /* first policer stage Word1[2:0] (3 bits)  */
        PRV_IPFIX_FW_BITS_SET(startOffset, 3, portGroupConfigSet.portGroupCfg.firstPolicerStage);
        /* first policer stage Word1[18:16] (3 bits)  */
        PRV_IPFIX_FW_BITS_SET(startOffset + 16, 3, portGroupConfigSet.portGroupCfg.secondPolicerStage);

        startOffset += 32;
        for (i=0; i<4; i++)
        {
            /* first policer max index (16 bits)  */
            PRV_IPFIX_FW_BITS_SET(startOffset, 16, portGroupConfigSet.portGroupCfg.firstPolicerMaxIpfixIndex[i]);
            startOffset += 16;
        }

        for (i=0; i<4; i++)
        {
            /* second policer max index (16 bits)  */
            PRV_IPFIX_FW_BITS_SET(startOffset, 16, portGroupConfigSet.portGroupCfg.secondPolicerMaxIpfixIndex[i]);
            startOffset += 16;
        }

        /* ipc message size is 6 words */
        ipcMsgSize = 6 * 4; /* in bytes */
        break;

    /* export config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_SET_E:
        /* periodic export enable Word0[8] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(8, 1, exportConfigSet.exportCfg.periodicExportEnable);
        /* export last cpu code Word0[10] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(10, 1, exportConfigSet.exportCfg.exportLastCpuCode);
        /* export last packet command Word0[11] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(11, 1, exportConfigSet.exportCfg.exportLastPktCmd);

        /* periodic export interval Word0[29:16] (14 bits) */
        PRV_IPFIX_FW_BITS_SET(16, 14, exportConfigSet.exportCfg.periodicExportInterval);

        startOffset = 32;
        /* data packet mtu Word1[13:0] (14 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 14, exportConfigSet.exportCfg.dataPktMtu);
        /* local queue number Word1[23:16] (8 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset + 16, 8, exportConfigSet.exportCfg.localQueueNum);
        /* MG number Word1[31:24] (8 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset + 24, 8, exportConfigSet.exportCfg.mgNum);

        startOffset = 2*32;
        /* txDsaTag[31:0] Word2[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 32, exportConfigSet.exportCfg.txDsaTag[0]);

        startOffset = 3*32;
        /* txDsaTag[63:32] Word3[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 32, exportConfigSet.exportCfg.txDsaTag[1]);

        startOffset = 4*32;
        /* txDsaTag[95:64] Word4[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 32, exportConfigSet.exportCfg.txDsaTag[2]);

        startOffset = 5*32;
        /* txDsaTag[127:96] Word5[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 32, exportConfigSet.exportCfg.txDsaTag[3]);

        /* ipc message size is 6 words */
        ipcMsgSize = 6 * 4; /* in bytes */

        break;

    /* elephant config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_SET_E:
        /* elephant detection offload Word0[8] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(8, 1, elephantConfigSet.elephantCfg.elephantDetectionOffload);
        /* report mice enable Word0[9] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(9, 1, elephantConfigSet.elephantCfg.reportMiceEnable);
        /* report EM index enable Word0[10] (1 bit) */
        PRV_IPFIX_FW_BITS_SET(10, 1, elephantConfigSet.elephantCfg.reportEmIndexEnable);

        startOffset = 32;
        /* packet count threshold[31:0]  Word1[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 32, elephantConfigSet.elephantCfg.pktCntThreshold[0]);

        startOffset = 2*32;
        /* packet count threshold[47:32] Word2[15:0] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 16, elephantConfigSet.elephantCfg.pktCntThreshold[1]);
        /* byte count threshold[31:0] Word2[31:16] and Word3[15:0] (32 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset + 16, 32, elephantConfigSet.elephantCfg.byteCntThreshold[0]);

        startOffset = 3*32 + 16;
        /* byte count threshold[47:32] Word3[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 16, elephantConfigSet.elephantCfg.byteCntThreshold[1]);

        startOffset = 4*32;
        /* cross count threshold high Word4[7:0] (8 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset, 8, elephantConfigSet.elephantCfg.crossCntThresholdHigh);
        /* cross count threshold low Word4[15:8] (8 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset + 8, 8, elephantConfigSet.elephantCfg.crossCntThresholdLow);
        /* start threshold Word4[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(startOffset + 16, 16, elephantConfigSet.elephantCfg.startThreshold);

        /* ipc message size is 5 words */
        ipcMsgSize = 5 * 4; /* in bytes */
        break;

    /* Data Get */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENABLE_SET_E:
        /* enable set Word0[8] (2 bits) */
        PRV_IPFIX_FW_BITS_SET(8, 1, enableSet.ipfixEnable);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;

    /* Data Get */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E:
        /* Config type Word0[9:8] (2 bits) */
        PRV_IPFIX_FW_BITS_SET(8, 2, configGet.configtype);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;
    /* Data Get */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_E:
        /* Flow ID Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(16, 16, dataGet.flowId);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;

    /* Data Get all*/
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E:
        /* Flow type Word1[1:0] (2 bits) */
        PRV_IPFIX_FW_BITS_SET(32, 2, dataGetAll.flowType);

        /* Max number of flows Word1[31:16] (2 bits) */
        PRV_IPFIX_FW_BITS_SET(32, 2, dataGetAll.maxNumOfFlows);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;

    /* Data Clear */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_E:
        /* Flow ID Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_SET(16, 16, dataClear.flowId);

        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;
    /* Data get all stop */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_STOP_E: /* Fall Through */
    /* Entry Delete All*/
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E:
        /* IPC message size is 1 word */
        ipcMsgSize = 1 * 4; /* in bytes */
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode");
    }

#ifndef ASIC_SIMULATION
    /* Post the AttributesGet IPC message to FW */
    rc = prvCpssGenericSrvCpuIpcMessageSend(fwChannel, 1, ipcData, ipcMsgSize);

#else
    (void)fwChannel;
    (void)ipcMsgSize;
#endif

    return rc;
}

/**
 * @internal prvCpssDxChIpfixFwIpcMessageParse function
 * @endinternal
 *
 * @brief Receive ipc mesage from service cpu
 *
 * @param[in] devNum   - device number
 * @param[in] ipcDataPtr   - (pointer to) IPC message stream read from FW.
 * @param[out] ipcMsg      - parsed IPC messae
 * @param[out] ipcMsgSizePtr - (pointer to) size of ipcData that is parsed.
 *
 * @retval    GT_OK     - on success
 */
static GT_STATUS prvCpssDxChIpfixFwIpcMessageParse
(
    IN  GT_U8                               devNum,
    IN GT_U8                                *ipcDataPtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT *ipcMsgPtr,
    OUT GT_U32                              *ipcMsgSizePtr
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT opcode;
    GT_U32                                startOffset = 0;
    GT_U32                                i;
    GT_U32                                val;

    cpssOsMemSet(ipcMsgPtr, 0, sizeof(CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT));

    CPSS_NULL_PTR_CHECK_MAC(ipcDataPtr);

    opcode = (CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT)ipcDataPtr[0];
    /* opcode */
    ipcMsgPtr->opcodeGet.opcode = opcode;
    startOffset += 8;

    switch (opcode)
    {
    /* Entry Invalidate */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E:
        PRV_IPFIX_FW_BITS_GET(8, 1, entryInvalidate.endReason, CPSS_DXCH_IPFIX_FW_END_REASON_ENT);

        PRV_IPFIX_FW_BITS_GET(16, 16, entryInvalidate.flowId, GT_U32);

        /* IPC message size 4 bytes */
        *ipcMsgSizePtr = 4 * 4; /* in bytes */

        break;

    /* Export Completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E:

        /* numOfExported Word0[31:16] */
        PRV_IPFIX_FW_BITS_GET(16, 16, exportCompletion.numOfExported, GT_32);

        /* last exported flow id Word1[15:0] (16 bits) */
        startOffset = 1*32 + 0;
        PRV_IPFIX_FW_BITS_GET(startOffset, 16, exportCompletion.lastExportedFlowId, GT_U32);

        /* last packet number. Word1[31:16] (16 bits) */
        startOffset = 1*32 + 16;
        PRV_IPFIX_FW_BITS_GET(startOffset, 16, exportCompletion.lastPktNum, GT_U32);

        /* TOD in 4ms units for last sampled packet. Word2[31:0] and Word3[23:0] (56 bits)*/
        startOffset = 2*32;
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, exportCompletion.lastTod[0], GT_U32);
        startOffset = 3*32;
        PRV_IPFIX_FW_BITS_GET(startOffset, 24, exportCompletion.lastTod[1], GT_U32);

        /* IPC message size 4 words */
        *ipcMsgSizePtr = 4 * 4; /* in bytes */

        break;

    /* Global config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E:
        /* ipfix enable Word0[8] (1 bit)  */
        PRV_IPFIX_FW_BITS_GET(8, 1, globalConfigReturn.globalCfg.ipfixEnable, GT_BOOL);
        /* idle aging offload Word0[9] (1 bit)  */
        PRV_IPFIX_FW_BITS_GET(9, 1, globalConfigReturn.globalCfg.idleAgingOffload, GT_BOOL);
        /* long aging offload Word0[10] (1 bit)  */
        PRV_IPFIX_FW_BITS_GET(10, 1, globalConfigReturn.globalCfg.longAgingOffload, GT_BOOL);
        /* counters extension offload Word0[11] (1 bit)  */
        PRV_IPFIX_FW_BITS_GET(11, 1, globalConfigReturn.globalCfg.countersExtensionOffload, GT_BOOL);
        /* timestamps extension offload Word0[12] (1 bit)  */
        PRV_IPFIX_FW_BITS_GET(12, 1, globalConfigReturn.globalCfg.timeStampsExtensionOffload, GT_BOOL);
        /* first policer stage Word0[15:13] (3 bits) */
        PRV_IPFIX_FW_BITS_GET(13, 3, globalConfigReturn.globalCfg.firstPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
        /* max in-transit index Word0[31:16] (16-bit) */
        PRV_IPFIX_FW_BITS_GET(16, 16, globalConfigReturn.globalCfg.maxInTransitIndex, GT_U32);

        startOffset = 32;
        /* idleTimeout Word1[11:0] (12 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 12, globalConfigReturn.globalCfg.idleTimeout, GT_U32);
        /* activeTimeout Word1[27:16] (12 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+16, 12, globalConfigReturn.globalCfg.activeTimeout, GT_U32);

        startOffset = 2*32;
        /* scan cycle interval Word2[9:0] (10 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 10, globalConfigReturn.globalCfg.scanCycleInterval, GT_U32);
        /* scan loop tiems Word2[25:10] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+10, 16, globalConfigReturn.globalCfg.scanLoopItems, GT_U32);
        /* second policer stage Word2[28:26] (3 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+26, 3, globalConfigReturn.globalCfg.secondPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
        /* repot new flows Word2[30] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(startOffset+30, 1, globalConfigReturn.globalCfg.reportNewFlows, GT_BOOL);
        /* hw Auto learn enable Word2[31] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(startOffset+31, 1, globalConfigReturn.globalCfg.hwAutoLearnEnable, GT_BOOL);

        startOffset = 3*32;
        /* first policer max ipfix index or max FW index Word3[15:0] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 16, globalConfigReturn.globalCfg.maxFwIndex, GT_U32);
        /* second policer max ipfix index or max App index Word3[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 16, 16, globalConfigReturn.globalCfg.maxAppIndex, GT_U32);

        /* ipc message size is 4 words */
        *ipcMsgSizePtr = 4 * 4; /* in bytes */
        break;

    /* Port Group config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E:
        startOffset = 32;
        /* first policer stage Word1[2:0] (3 bits)  */
        PRV_IPFIX_FW_BITS_GET(startOffset, 3, portGroupConfigReturn.portGroupCfg.firstPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
        /* first policer stage Word1[18:16] (3 bits)  */
        PRV_IPFIX_FW_BITS_GET(startOffset + 16, 3, portGroupConfigReturn.portGroupCfg.secondPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);

        startOffset += 32;
        for (i=0; i<4; i++)
        {
            /* first policer max index (16 bits)  */
            PRV_IPFIX_FW_BITS_GET(startOffset, 16, portGroupConfigReturn.portGroupCfg.firstPolicerMaxIpfixIndex[i], GT_U32);
            startOffset += 16;
        }

        for (i=0; i<4; i++)
        {
            /* second policer max index (16 bits)  */
            PRV_IPFIX_FW_BITS_GET(startOffset, 16, portGroupConfigReturn.portGroupCfg.secondPolicerMaxIpfixIndex[i], GT_U32);
            startOffset += 16;
        }

        /* ipc message size is 5 words */
        *ipcMsgSizePtr = 6 * 4; /* in bytes */
        break;


    /* export config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E:
        /* periodic export enable Word0[8] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(8, 1, exportConfigReturn.exportCfg.periodicExportEnable, GT_BOOL);
        /* export last cpu code Word0[10] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(10, 1, exportConfigReturn.exportCfg.exportLastCpuCode, GT_BOOL);
        /* export last packet command Word0[11] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(11, 1, exportConfigReturn.exportCfg.exportLastPktCmd, GT_BOOL);

        /* periodic export interval Word0[29:16] (14 bits) */
        PRV_IPFIX_FW_BITS_GET(16, 14, exportConfigReturn.exportCfg.periodicExportInterval, GT_U32);

        startOffset = 32;
        /* data packet mtu Word1[13:0] (14 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 14, exportConfigReturn.exportCfg.dataPktMtu, GT_U32);
        /* local queue number Word1[23:16] (8 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 16, 8, exportConfigReturn.exportCfg.localQueueNum, GT_U32);
        /* MG number Word1[31:24] (8 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 24, 8, exportConfigReturn.exportCfg.mgNum, GT_U32);

        startOffset = 2*32;
        /* txDsaTag[31:0] Word2[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, exportConfigReturn.exportCfg.txDsaTag[0], GT_U32);

        startOffset = 3*32;
        /* txDsaTag[63:32] Word3[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, exportConfigReturn.exportCfg.txDsaTag[1], GT_U32);

        startOffset = 4*32;
        /* txDsaTag[95:64] Word4[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, exportConfigReturn.exportCfg.txDsaTag[2], GT_U32);

        startOffset = 5*32;
        /* txDsaTag[127:96] Word5[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, exportConfigReturn.exportCfg.txDsaTag[3], GT_U32);

        /* ipc message size is 6 words */
        *ipcMsgSizePtr = 6 * 4; /* in bytes */

        break;

    /* elephant config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E:
        /* elephant detection offload Word0[8] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(8, 1, elephantConfigReturn.elephantCfg.elephantDetectionOffload, GT_BOOL);
        /* report mice enable Word0[9] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(9, 1, elephantConfigReturn.elephantCfg.reportMiceEnable, GT_BOOL);
        /* report EM index enable Word0[10] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(10, 1, elephantConfigReturn.elephantCfg.reportEmIndexEnable, GT_BOOL);

        startOffset = 32;
        /* packet count threshold[31:0]  Word1[31:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 32, elephantConfigReturn.elephantCfg.pktCntThreshold[0], GT_U32);

        startOffset = 2*32;
        /* packet count threshold[47:32] Word2[15:0] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 16, elephantConfigReturn.elephantCfg.pktCntThreshold[1], GT_U32);
        /* byte count threshold[31:0] Word2[31:16] and Word3[15:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 16, 32, elephantConfigReturn.elephantCfg.byteCntThreshold[0], GT_U32);

        startOffset = 3*32 + 16;
        /* byte count threshold[47:32] Word3[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 16, elephantConfigReturn.elephantCfg.byteCntThreshold[1], GT_U32);

        startOffset = 4*32;
        /* cross count threshold high Word4[7:0] (8 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 8, elephantConfigReturn.elephantCfg.crossCntThresholdHigh, GT_U32);
        /* cross count threshold low Word4[15:8] (8 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 8, 8, elephantConfigReturn.elephantCfg.crossCntThresholdLow, GT_U32);
        /* start threshold Word4[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 16, 16, elephantConfigReturn.elephantCfg.startThreshold, GT_U32);

        /* ipc message size is 5 words */
        *ipcMsgSizePtr = 5 * 4; /* in bytes */
        break;

    /* entry removed */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_REMOVED_E:
        /* last cpu code Word0[15:8] (8-bits) */
        PRV_IPFIX_FW_BITS_GET(8, 8, entryRemoved.lastCpuCode, GT_U32);
        /* Flow ID Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(16, 16, entryRemoved.flowId, GT_U32);

        startOffset = 32;
        /* first time stamp Word1[17:0] (18-bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 18, entryRemoved.firstTs, GT_U32);
        /* last time stamp Word1[31:18] and Word2[3:0] (18-bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+18, 18, entryRemoved.lastTs, GT_U32);

        startOffset = 2*32;
        /* Packet Count[31:0] Word2[31:4] and Word3[3:0] (32-bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+4, 32, entryRemoved.packetCount[0], GT_U32);

        startOffset = 3*32;
        /* Packet Count[39:32] Word3[11:4] (8 bits)*/
        PRV_IPFIX_FW_BITS_GET(startOffset, 8, entryRemoved.packetCount[1], GT_U32);
        /* drop Count[31:0] Word3[31:12] and Word4[11:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset + 12, 32, entryRemoved.dropCount[0], GT_U32);

        startOffset = 4*32;
        /* drop Count[39:32] Word4[19:12] (8 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+12, 8, entryRemoved.dropCount[1], GT_U32);
        /* byte Count[31:0] Word4[31:20] and Word5[19:0] (32 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+20, 32, entryRemoved.byteCount[0], GT_U32);

        startOffset = 5*32;
        /* byte count[45:32] Word5[31:20] and Word6[1:0] (14 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset+20, 14, entryRemoved.byteCount[1], GT_U32);

        startOffset = 6*32;
        /* Reserved */

        /* ipc message size is 7 words */
        *ipcMsgSizePtr = 7 * 4; /* in bytes */
        break;

    /* elephant set */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_SET_E:
        /* elephant set / invalidate Word0[8] (1 bit) */
        PRV_IPFIX_FW_BITS_GET(8, 1, elephantSet.state, GT_BOOL);
        /* flowId  Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(16, 16, elephantSet.flowId, GT_U32);

        startOffset = 1*32;
        /* emIndex Word0[19:0] (20 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 20, elephantSet.emIndex, GT_U32);

        /* ipc message size is 7 words */
        *ipcMsgSizePtr = 2 * 4; /* in bytes */
        break;

    /* delete all completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E:
        /* number of deleted  Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(16, 16, deleteAllCompletion.numOfDeleted, GT_U32);

        /* ipc message size is 1 words */
        *ipcMsgSizePtr = 1 * 4; /* in bytes */
        break;

    /* data clear all completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_ALL_COMPLETION_E:
        /* ipc message size is 1 words */
        *ipcMsgSizePtr = 1 * 4; /* in bytes */
        break;

    /* entry learned */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_LEARNED_E:
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode: %d", opcode);
            break;
        }
        /* keySize Word[9:8] */
        /* [TBD] Need to define enum */
        PRV_IPFIX_FW_BITS_GET(8, 2, entryLearned.keySize, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);
        /* flowId  Word0[31:16] (16 bits) */
        PRV_IPFIX_FW_BITS_GET(16, 16, entryLearned.flowId, GT_U32);

        startOffset = 1*32;
        /* emIndex Word1[19:0] (20 bits) */
        PRV_IPFIX_FW_BITS_GET(startOffset, 20, entryLearned.emIndex, GT_U32);

        for (i=0; i<12; i++)
        {
            startOffset = (i+1)*32 + 24;
            /* flowKey Word1[31:24] and Work2[23:0] (32 bits) */
            PRV_IPFIX_FW_BITS_GET(startOffset, 32, entryLearned.flowKey[i], GT_U32);
        }

        /* ipc message size is 14 words */
        *ipcMsgSizePtr = 14 * 4; /* in bytes */
        break;

    /* config error */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E:
        /* return Code Word0[14:8] (7 bits)*/
        val = prvCpssDxChIpfixFwBitsGet(ipcDataPtr, 8, 7);
        ipcMsgPtr->configError.errType = CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NONE_E;
        for (i=0; i<=CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_SCAN_CYCLE_ERROR_E; i++)
        {
            if (val & (1<<i))
            {
                ipcMsgPtr->configError.errType = (CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT)i;
                break;
            }
        }

        /* ipc message size is 1 word */
        *ipcMsgSizePtr = 1 * 4; /* in bytes */

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode");
    }

    return rc;
}

/**
 * @internal prvCpssDxChIpfixFwIpcMsgFetchAndParse function
 * @endinternal
 *
 * @brief  Fetch and parse IPC messages
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P.
 *
 * @param[in]  devNum              - device number
 * @param[in]  sCpuNum             - service CPU number
 * @param[in]  numOfMsgs           - number of messages to be fetched.
 * @param[out] numOfMsgsFetchedPtr - (pointer to) number of messages fetched.
 * @param[out] pendingMsgBuffSizePtr - (pointer to) Size in Bytes of the number of remaining messages
 *                                   in Tx IPC queue, which are pending to be fetched
 * @param[out] ipcMsgArrPtr        - (pointer to) fetched messages array.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 */
GT_STATUS prvCpssDxChIpfixFwIpcMsgFetchAndParse
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum,
    IN  GT_U32    numOfMsgs,
    OUT GT_U32    *numOfMsgsFetchedPtr,
    OUT GT_U32    *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   *ipcMsgArrPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       ipcData[PRV_CPSS_DXCH_IPFIX_FW_IPC_BUFF_MAX_SIZE_IN_BYTES_CNS] = {0};
    GT_U8       *ipcMsgStartPtr;
    GT_UINTPTR  fwChannel;
    GT_U32      i;
    GT_U32      ipcHwMsgSize[PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS];
    GT_U32      ipcMsgSize[PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS];
    GT_U32      ipcHwMsgTotalSize   = 0;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT     ipcMsg;
    GT_U32      loopCount = 0;
    GT_U32      maxLoopCount = 2;

    fwChannel = prvCpssDxChFwChannelGet(devNum, sCpuNum);
    if (!fwChannel)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FW channel error");
    }

    cpssOsMemSet(ipcHwMsgSize, 0, sizeof(ipcHwMsgSize));
    cpssOsMemSet(ipcMsgSize, 0, sizeof(ipcMsgSize));

    /* Read all IPC memory and store it in temporary buffer */
    ipcHwMsgTotalSize = 0;
    ipcMsgStartPtr  = &ipcData[0];
    for (i = 0; i < numOfMsgs; i++)
    {
#ifndef ASIC_SIMULATION
        loopCount = 0;
        do {
            rc = prvCpssGenericSrvCpuIpcMessageRecv(fwChannel, 1, ipcMsgStartPtr, &ipcHwMsgSize[i]);
            loopCount++;
            if (loopCount >= maxLoopCount)
            {
                break;
            }
        } while (rc == GT_NO_MORE);
#else
        (void)loopCount;
        (void)maxLoopCount;
        (void)fwChannel;
        return GT_OK;
#endif
        if (rc == GT_NO_MORE)
        {
             rc = GT_OK;
             break;
        }
        else if (rc != GT_OK)
        {
            return rc;
        }

        ipcHwMsgTotalSize += ipcHwMsgSize[i];
        if (ipcHwMsgTotalSize > PRV_CPSS_DXCH_IPFIX_FW_IPC_BUFF_MAX_SIZE_IN_BYTES_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                      "Total IPC messages buffer exceeds max size");
        }

        ipcMsgStartPtr += ipcHwMsgSize[i];
    }

    /* Parse each IPC message and issue events */
    ipcMsgStartPtr   = &ipcData[0];
    *numOfMsgsFetchedPtr = 0;
    i = 0;
    while (*numOfMsgsFetchedPtr < numOfMsgs) {
        /* If the IPC memory is empty, complete the API by returning 0 to the App.
           Otherwise, fetch the next pending IPC message */
        rc = prvCpssDxChIpfixFwIpcMessageParse(devNum, ipcMsgStartPtr, &ipcMsg, &ipcMsgSize[i]);
        if (rc == GT_BAD_VALUE)
        {
            break;
        }
        else if (rc != GT_OK)
        {
            return rc;
        }

        if (*numOfMsgsFetchedPtr < numOfMsgs)
        {
            cpssOsMemCpy(&ipcMsgArrPtr[*numOfMsgsFetchedPtr], &ipcMsg, sizeof(ipcMsg));
            (*numOfMsgsFetchedPtr)++;
        }

        ipcMsgStartPtr += ipcHwMsgSize[i];
        if ((ipcMsgStartPtr - &ipcData[0]) >=  PRV_CPSS_DXCH_IPFIX_FW_IPC_BUFF_MAX_SIZE_IN_BYTES_CNS)
        {
            break;
        }
        i++;
    }

    *pendingMsgBuffSizePtr = 0;

    return GT_OK;
}

/**
* @internal ipfixEmSemaLock function
* @endinternal
*
* @brief   Aquiring the HW semaphore lock for EM access.

* @param[in] devNum               - device number
* @param[in] semaNum              - semaphore number
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_BOOL ipfixEmSemaLock
(
    IN  GT_U8   devNum,
    IN  GT_U32  semaNum
)
{
    GT_U32      tmp = 0;
    /* For host the cpuId value will be 0 */
    GT_U32      cpuId = HW_SEMA_HCPU_ID;
    GT_U32      baseAddr = 0x0;
    GT_U8       unitId;
    GT_BOOL     isError;
    GT_UINTPTR  fwChannel = 0;
    GT_U8       sCpuId;

    sCpuId = prvCpssDxChScpuIdGet(devNum);
    if(sCpuId == 0xFF)
    {
        return GT_OK;
    }

    /* Retrieve object ID for IPC operations with this SCPU */
    fwChannel = prvCpssDxChFwChannelGet(devNum, sCpuId);

    /* IPFIX FW is not loaded */
    if (!fwChannel)
    {
        return GT_OK;
    }

    if(semaNum > MAX_HW_SEMA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Invalid semaphore number %d\n", semaNum);
    }

    /* Get MG unit base address*/
    unitId   = (sCpuId == 0) ? PRV_CPSS_DXCH_UNIT_MG_E : PRV_CPSS_DXCH_UNIT_MG_0_1_E + sCpuId - 1;
    baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitId, &isError);
    if(isError)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "SCPUID %d not supported \n", sCpuId);
    }
    baseAddr += HW_SEMA_REG_BASE_SWITCH_MG;

    do
    {
        prvCpssHwPpWriteRegister(devNum, baseAddr + semaNum*4, cpuId);
        prvCpssHwPpReadRegister(devNum, baseAddr + semaNum*4, &tmp);

    } while (((tmp & 0xFF) != cpuId) && ((tmp & 0xFF) != ((cpuId == 0) ? 1 : cpuId)));

    return GT_TRUE;
}

/**
* @internal ipfixEmSemaUnlock function
* @endinternal
*
* @brief   Releasing the HW semaphore lock for EM access.

* @param[in] devNum               - device number
* @param[in] semaNum              - semaphore number
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_BOOL ipfixEmSemaUnlock
(
    IN  GT_U8   devNum,
    IN  GT_U32  semaNum
)
{
    GT_U32      baseAddr = 0x0;
    GT_U8       unitId;
    GT_BOOL     isError;
    GT_UINTPTR  fwChannel = 0;
    GT_U8       sCpuId;

    sCpuId = prvCpssDxChScpuIdGet(devNum);
    if(sCpuId == 0xFF)
    {
        return GT_OK;
    }

    /* Retrieve object ID for IPC operations with this SCPU */
    fwChannel = prvCpssDxChFwChannelGet(devNum, sCpuId);

    /* IPFIX FW is not loaded */
    if (!fwChannel)
    {
        return GT_OK;
    }

    if (semaNum > MAX_HW_SEMA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Invalid semaphore number %d\n", semaNum);
    }

    /* Get MG unit base address*/
    unitId   = (sCpuId == 0) ? PRV_CPSS_DXCH_UNIT_MG_E : PRV_CPSS_DXCH_UNIT_MG_0_1_E + sCpuId - 1;
    baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitId, &isError);
    if(isError)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "SCPUID %d not supported \n", sCpuId);
    }
    baseAddr += HW_SEMA_REG_BASE_SWITCH_MG;

    prvCpssHwPpWriteRegister(devNum, baseAddr + semaNum*4, 0xFF);

    return GT_TRUE;
}
