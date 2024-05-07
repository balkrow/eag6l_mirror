/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssHwInit.h
*
* @brief CPSS definitions for HW init.
*
* @version   48
********************************************************************************
*/
#ifndef __prvCpssHwInith
#define __prvCpssHwInith

#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/generic/port/cpssPortTx.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* macro to free an allocated pointer, and set it to NULL */
#if __WORDSIZE != 64
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_U32)ptr) != PRV_CPSS_SW_PTR_ENTRY_UNUSED)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#else
#define FREE_PTR_IF_NOT_NULL_MAC(ptr)       \
        if(ptr && (((GT_UINTPTR)ptr) != 0xFFFFFFFBFFFFFFFB)) \
        {                                   \
            cpssOsFree(ptr);                \
            ptr=NULL;                       \
        }
#endif

/* sip_5_20 convert global txq-dq port number to local txq-dq port number */
#define SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_LOCAL_TXQ_DQ_PORT_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) % PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* sip_5_20 convert global txq-dq port number to DQ unit index */
#define SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) / PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* sip_5_20 convert local txq-dq port number in specific DQ unit to global txq-dq port number */
#define SIP_5_20_LOCAL_TXQ_DQ_PORT_TO_GLOBAL_TXQ_DQ_PORT_MAC(dev,localTxqDqPortIndex,dqIndex) \
    ((localTxqDqPortIndex) + ((dqIndex) * PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp))

/* sip_5_20 access to the DB of 'per port' the txq-Dq unit index */
#define SIP_5_20_TXQ_DQ_UNIT_INDEX_OF_PORT_MAC(dev,_physPort)   \
    PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.mapPhyPortToTxqDqUnitIndex[_physPort]

/* sip_5_20 the max number of global txq-Dq ports */
#define SIP_5_20_TXQ_DQ_MAX_PORTS_MAC(dev) \
    (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp * \
     PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp)

/* sip_5_20 the max number of port in TXDMA */
#define SIP_5_20_TXDMA_MAX_PORTS_MAC(dev) \
    (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.txNumPortsPerDp)


#define LION2_DEVICE_INDICATION_CNS   0xFFFFFFFF
#define BOBK_DEVICE_INDICATION_CNS    0xFFFFFFFE
#define BC3_DEVICE_INDICATION_CNS     0xFFFFFFFD
#define ALDRIN2_DEVICE_INDICATION_CNS 0xFFFFFFFC
#define FALCON_DEVICE_INDICATION_CNS  0xFFFFFFFB
#define HAWK_DEVICE_INDICATION_CNS    0xFFFFFFFA
#define PHOENIX_DEVICE_INDICATION_CNS 0xFFFFFFF9
#define HARRIER_DEVICE_INDICATION_CNS 0xFFFFFFF8
#define IRONMAN_DEVICE_INDICATION_CNS 0xFFFFFFF7

/**
* @internal prvCpssDxChNetIfMultiNetIfNumberGet function
* @endinternal
*
* @brief   private (internal) function to :
*         build the information about all the network interfaces according to phase1 and phase2 params
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - The device number.
*
* @param[out] numOfNetIfsPtr           - (pointer to) the number of network intrfaces according to port mapping
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_APPLICABLE_DEVICE - on non-applicable device
*
*/
GT_STATUS prvCpssDxChNetIfMultiNetIfNumberGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *numOfNetIfsPtr
);
/**
* @internal prvCpssDxChNetIfNumQueuesGet function
* @endinternal
*
* @brief  The function return the number of queue for FROM_CPU,TO_CPU
*           for NON-GDMA devices the number of queues for FROM_CPU,TO_CPU are the same.
*           for     GDMA devices the number of queues for FROM_CPU,TO_CPU could differ from each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @param[out] toCpu_numOfQueuesPtr    - (pointer to) the number of queues that bound for TO_CPU
*                                       ignored if NULL
* @param[out] fromCpu_numOfQueuesPtr  - (pointer to) the number of queues that bound for FROM_CPU
*                                       ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_APPLICABLE_DEVICE - on non-applicable device
*
*/
GT_STATUS prvCpssDxChNetIfNumQueuesGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *toCpu_numOfQueuesPtr,
    OUT GT_U32                      *fromCpu_numOfQueuesPtr
);

/**
* @internal prvCpssPpConfigPortToPhymacObjectBind function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - error
*/
GT_STATUS prvCpssPpConfigPortToPhymacObjectBind
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
);

/**
* @internal prvCpssDxChHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of unit.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      there are some special 'devNum' :
*                                      see LION2_DEVICE_INDICATION_CNS ,
*                                      BOBK_DEVICE_INDICATION_CNS ,
*                                      BC3_DEVICE_INDICATION_CNS
* @param[in] unitId                   - the ID of the address space unit.
*
* @param[out] errorPtr                 - (pointer to) indication that function did error.
*                                        GT_TRUE denotes error
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssDxChHwUnitBaseAddrGet
(
    IN GT_U32                   devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_BOOL                 *errorPtr
);

/**
* @internal prvCpssDxChHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
);


/**
* @internal convertRegAddrToNewUnitsBaseAddr function
* @endinternal
*
* @brief   the assumption is that the 'old address' is based on Lion2 units base addresses !
*         so need to convert it to address space of current device
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in,out] oldNewRegPtr             - pointer to the 'old' reg address (from Lion2)
* @param[in,out] oldNewRegPtr             - pointer to the 'new' reg address - after aligned to the current device.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS convertRegAddrToNewUnitsBaseAddr
(
    IN GT_U8    devNum,
    INOUT GT_U32 *oldNewRegPtr
);


/**
* @internal prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet function
* @endinternal
*
* @brief   get the DMA index that is reserved for the CPU port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; xCat2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] txqPortNumber            - sip5 : The global txq port number of the cpu port
*                                       sip6 : The global DMA port number of the cpu port
*
* @param[out] indexPtr                 - pointer to the index
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  txqPortNumber,
    OUT GT_U32  *indexPtr
);


/**
* @internal prvCpssDxChHwPpSdmaCpuPortReservedMacIdxGet function
* @endinternal
*
* @brief   get the index that is reserved for the CPU port for 'link status' filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; xCat2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] portNum                  - The Physical port number of the cpu port
*
* @param[out] indexPtr                 - pointer to the index
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpSdmaCpuPortReservedMacIdxGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *indexPtr
);


/**
* @internal prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
);

/**
* @internal prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
);

/**
* @internal prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
);

/**
* @internal prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*                                       if 'current' is GT_NA (0xFFFFFFFF) --> meaning need to 'get first'
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
);

/**
* @internal prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert function
* @endinternal
*
* @brief   convert the physical port number in device to port group index.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] physicalPortNum          - the physical port number.
*
* @param[out] portGroupIdPtr           - (pointer to) the port group of the port
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    physicalPortNum,
    OUT GT_U32                 *portGroupIdPtr
);

/**
* @internal prvCpssDxChHwEgfQagTcDpMapperByParamsSet function
* @endinternal
*
* @brief   (sip5_20) set the EGF QAG 'TC,DP' mapping table , according to parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*                                      next parameters are part of the key , which define the 'index' to the table
* @param[in] origTc                   - orig TC (traffic class).
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] origDp                   - orig DP (Drop precedence).
* @param[in] packetCommand            - packet command (on of CPSS_DXCH_NET_DSA_CMD_ENT)
* @param[in] isMultiDestination       - GT_TRUE - multi-destination
*                                      GT_FALSE - single destination
* @param[in] targetPortTcProfile      - Target Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] sourcePortTcProfile      - Source Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
*                                      next parameters are part of the 'data' to set according to the key
* @param[in] newTxqDp                 - the DP to assign in the TXQ
* @param[in] newTxqTc                 - the TC to assign in the TXQ
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] newTxqPfcTc              - the PFC TC to assign in the TXQ
*                                      (APPLICABLE RANGES: 0..7)
* @param[in]  mcastPriority            -defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in]  dpForRx                 - defines whether on congestion packets will be dropped from RX context queue or
*                                       queue will back pressure the control pipe.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in]  preemptiveTc            -defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChHwEgfQagTcDpMapperByParamsSet
(
    IN  GT_U8   devNum,
    /* key */
    IN  GT_U32  origTc,
    IN  CPSS_DP_LEVEL_ENT   origDp,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT  packetCommand,
    IN  GT_BOOL   isMultiDestination,
    IN  GT_U32  targetPortTcProfile,
    IN  GT_U32  sourcePortTcProfile,
    /* data */
    IN  GT_U32  newTxqDp,
    IN  GT_U32  newTxqTc,
    IN  GT_U32  newTxqPfcTc,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     mcastPriority,
    IN  CPSS_DP_FOR_RX_ENT                       dpForRx,
    IN  GT_BOOL                                  preemptiveTc
);

/**
* @internal prvCpssDxChHwEgfQagTcDpMapperByParamsGet function
* @endinternal
*
* @brief   (sip5_20) Get the EGF QAG 'TC,DP' mapping table entry, according to parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number.
*                                      next parameters are part of the key , which define the 'index' to the table
* @param[in] origTc                   - orig TC (traffic class).
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] origDp                   - orig DP (Drop precedence).
* @param[in] packetCommand            - packet command (on of CPSS_DXCH_NET_DSA_CMD_ENT)
* @param[in] isMultiDestination       - GT_TRUE - multi-destination
*                                      GT_FALSE - single destination
* @param[in] targetPortTcProfile      - Target Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] sourcePortTcProfile      - Source Port TC Profile
*                                      (APPLICABLE RANGES: 0..3)
*                                      next parameters are 'output' to get according to the key
* @param[in] txqDpPtr                 - (pointer to) the DP assigned to the TXQ
* @param[in] txqTcPtr                 - (pointer to) the TC to assigned to the TXQ
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] txqPfcTcPtr              - (pointer to) the PFC TC assigned to the TXQ
*                                      (APPLICABLE RANGES: 0..7)
*                                       GT_OK on success
* @param[out]  mcastPriorityPtr            -(pointer to)defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[out]  dpForRx                 -(pointer to) Drop precedence for RX context queue.
* @param[in]  preemptiveTcPtr            -(pointer to)defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).

*/
GT_STATUS prvCpssDxChHwEgfQagTcDpMapperByParamsGet
(
    IN  GT_U8   devNum,
    /* key */
    IN  GT_U32  origTc,
    IN  CPSS_DP_LEVEL_ENT   origDp,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT  packetCommand,
    IN  GT_BOOL   isMultiDestination,
    IN  GT_U32  targetPortTcProfile,
    IN  GT_U32  sourcePortTcProfile,
    /* output parameters */
    OUT  CPSS_DP_LEVEL_ENT  * txqDpPtr,
    OUT  GT_U32  * txqTcPtr,
    OUT  GT_U32  * txqPfcTcPtr,
    OUT  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT       *mcastPriorityPtr,
    OUT  CPSS_DP_FOR_RX_ENT                         *dpForRxPtr,
    OUT  GT_BOOL                                    *preemptiveTcPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssHwInith */


