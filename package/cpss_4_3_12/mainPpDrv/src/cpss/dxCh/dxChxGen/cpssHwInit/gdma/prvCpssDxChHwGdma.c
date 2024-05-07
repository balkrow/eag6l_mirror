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
* @file prvCpssDxChHwGdma.c
*
* @brief This file implement GDMA unit of the AAS.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfGdma.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdmaNetIfFunc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdmaMsg.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>
#include <cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>

enum {OWNER_CPU=0,OWNER_HW=1};

/* check collide with TO_CPU/FROM_CPU/FDB/CNC/IPE already used queues */
#define CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,_collidName)   \
    if(_curr_bmpOfQueues & gdmaInfoPtr->gdmaUsedBmp_##_collidName.globalGdmaUnits[gdmaUnitId].usedQueuesBmp)  \
    {                                                                                                          \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                                                            \
            "cpuPortNumber[%d] [%s] queues [%d..%d] overlap queues local [%s] queues bmp[0x%8.8x] in global GDMA unit[%d]\n",\
            cpuPortNumber,                                                                  \
            #_clientName,                                                                   \
            _firstQueue,                                                                    \
            _firstQueue+_numQueues-1,                                                       \
            #_collidName,                                                                   \
            gdmaInfoPtr->gdmaUsedBmp_##_collidName.globalGdmaUnits[gdmaUnitId].usedQueuesBmp,    \
            gdmaUnitId);                                                                    \
    }

/* check collide with TO_CPU/FROM_CPU/FDB/CNC/IPE already used queues */
#define CHECK_OVERLAP(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName)                     \
    /* check collide with TO_CPU already used queues */                                         \
    CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,TO_CPU);    \
    /* check collide with FROM_CPU already used queues */                                       \
    CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,FROM_CPU);  \
    /* check collide with FDB already used queues */                                            \
    CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,FDB);       \
    /* check collide with CNC already used queues */                                            \
    CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,CNC);       \
    /* check collide with IPE already used queues */                                            \
    CHECK_OVERLAP_PER_COLLIDER(_curr_bmpOfQueues,_firstQueue,_numQueues,_clientName,IPE)

/* 16 byte alignment -- TO_CPU works in 16B alignment  */
#define GDMA_RX_BUFF_ALIGN   (16)
#define GDMA_TX_BUFF_ALIGN   (16)


static GT_STATUS gdmaTxGeneratorBuffListInit
(
    IN  GT_U8       devNum
);

/**
* @internal prvCpssHwPpGdmaReadReg function
* @endinternal
*
* @brief   GDMA register : Read a register value from the GDMA unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaReadReg /*like prvCpssHwPpMgReadReg*/
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    OUT GT_U32  *data
)
{
    /* calling the 'non-Drv' for the gdmaNum[0..max] */
    /* with port group == 'gdmaNum' */
    return prvCpssHwPpPortGroupReadRegister(devNum,gdmaNum,regAddr_gdma0,data);
}
/**
* @internal prvCpssHwPpGdmaWriteReg function
* @endinternal
*
* @brief   GDMA register : Write to the GDMA unit given register.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaWriteReg /*like prvCpssHwPpMgWriteReg*/
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  data
)
{
    /* calling the 'non-Drv' for the gdmaNum[0..max] */
    /* with port group == 'gdmaNum' */
    return prvCpssHwPpPortGroupWriteRegister(devNum,gdmaNum,regAddr_gdma0,data);
}

/**
* @internal prvCpssHwPpGdmaGetRegField function
* @endinternal
*
* @brief   GDMA register : Read a selected register field from the GDMA Unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaGetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *fieldData
)
{
    /* calling the 'non-Drv' for the gdmaNum[0..max] */
    /* with port group == 'gdmaNum' */
    return prvCpssHwPpPortGroupGetRegField(devNum,gdmaNum,regAddr_gdma0,fieldOffset,fieldLength,fieldData);
}

/**
* @internal prvCpssHwPpGdmaSetRegField function
* @endinternal
*
* @brief   GDMA register : Write value to selected register field of the GDMA Unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssHwPpGdmaSetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  fieldData
)
{
    /* calling the 'non-Drv' for the gdmaNum[0..max] */
    /* with port group == 'gdmaNum' */
    return prvCpssHwPpPortGroupSetRegField(devNum,gdmaNum,regAddr_gdma0,fieldOffset,fieldLength,fieldData);
}

/**
* @internal prvCpssHwPpGdmaRegBusyWaitByMask function
* @endinternal
*
* @brief   GDMA register : function do 'Busy wait' on specific mask of the register.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] regMask                  - the mask of the register that we wait for to clear
* @param[in] isRevert                 - indication that we wait for 'set' or wait for 'clear'
*                                       GT_TRUE  : wait for 'set'   (value 1 in the bits)
*                                       GT_FALSE : wait for 'clear' (value 0 in the bits)
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaRegBusyWaitByMask
(
    IN GT_U8                            devNum,
    IN GT_U32                           gdmaNum,
    IN GT_U32                           regAddr_gdma0,
    IN GT_U32                           regMask,
    IN GT_BOOL                          isRevert,
    IN GT_BOOL                          onlyCheck
)
{
    GT_STATUS   rc;
    GT_U32  busyIterator; /* busy wait iterator */
    GT_U32  regValue;     /* current register value*/

    busyIterator = 0;
    /* use "busy" wait */
    do
    {
        /* query the needed bit in the needed register in the specified port group*/
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaNum, regAddr_gdma0, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* check that number of iteration not over the limit */
        PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,(busyIterator++));

        if((isRevert == GT_FALSE) && (regValue & regMask) && (onlyCheck == GT_FALSE))
        {
            /* continue the loop */
        }
        else
        if((isRevert == GT_TRUE) && (0 == (regValue & regMask)) && (onlyCheck == GT_FALSE))
        {
            /* continue the loop */
        }
        else
        {
            break;
        }
    }
    while(1);

    if((isRevert == GT_FALSE) && (regValue & regMask))
    {
        /* the case of (onlyCheck == GT_FALSE) would have already returned GT_TIMEOUT
            from the macro of PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS */
        return /* do not register as error (up to the caller to register error if needed) */ GT_BAD_STATE;
    }

    if((isRevert == GT_TRUE) && (0 == (regValue & regMask)))
    {
        /* the case of (onlyCheck == GT_FALSE) would have already returned GT_TIMEOUT
            from the macro of PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS */
        return /* do not register as error (up to the caller to register error if needed) */ GT_BAD_STATE;
    }

    return GT_OK;
}

/**
* @internal prvCpssHwPpGdmaWriteRegBitMask function
* @endinternal
*
* @brief  GDMA register :  Writes the unmasked bits of a register.
*         - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpGdmaWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   gdmaNum,
    IN GT_U32   regAddr_gdma0,
    IN GT_U32   mask,
    IN GT_U32   value
)
{
    GT_STATUS   rc;
    GT_U32  origValue,newValue;

    if(mask != 0xFFFFFFFF)
    {
        rc = prvCpssHwPpGdmaReadReg(devNum,gdmaNum,regAddr_gdma0,&origValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        newValue = (origValue & ~(mask)) | (value & mask);
    }
    else
    {
        newValue = value;
    }

    return prvCpssHwPpGdmaWriteReg(devNum,gdmaNum,regAddr_gdma0,newValue);
}

/**
* @internal prvCpssDxChNetIfGdmaCpuPortBindToQueuesSet function
* @endinternal
*
* @brief   Bind CPU physical port to the number of GDMA queues that it need to use.
*           number of queues for 'TO_CPU'   (Rx direction)
*           number of queues for 'FROM_CPU' (Tx direction)
*           NOTE:
*               1. Those are NOT the 'TXQ queues' that the port give in port mapping <txqPortNumber>
*                  Those are GDMA queues.
*               2. must be called after 'port mapping' and before 'phase 2 init'
*               so can be considered as need to be called right after 'port mapping'
*               3. after calling this API for all ports of type CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E
*                  and giving the info to CPSS about number of GDMA queues needed
*                  the user can call to cpssDxChNetIfGdmaCpuPortsInfoGet , to get the 'global GDMA queues' that each port get.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - Device number.
* @param[in] cpuPortNumber         - The CPU physical port number.
*                                    A physical port that used CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E
*                                    in the 'port mapping' API
* @param[in] cpuPortInfoPtr        - (pointer to) The CPU port info
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad physical port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - the numOfQueuesToCpu or numOfQueuesFromCpu more than the port supports
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChNetIfGdmaCpuPortBindToQueuesSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cpuPortNumber,
    IN PRV_CPSS_DXCH_NETIF_GDMA_BIND_CPU_PORT_INFO_STC    *cpuPortInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  orig_numOfQueuesToCpu  ;
    GT_U32  orig_numOfQueuesFromCpu;
    GT_U32  dmaNum;
    GT_U32  ii,netIfNum,gdmaUnitId;
    GT_BOOL isCpuPort;
    PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_INFO_STC        *legacy_cpuPortInfoPtr;
    PRV_CPSS_DXCH_GDMA_CPU_PORT_NETIF_INFO_STC        *cpuPortPtr;
    PRV_CPSS_DXCH_GDMA_INFO_STC                       *gdmaInfoPtr;
    GT_U32  curr_numOfQueues,curr_bmpOfQueues;
    GT_U32  numGdmaUnits_FROM_CPU = 0,numGdmaUnits_TO_CPU = 0;
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *allCpuPortsInfoPtr;
    CPSS_DXCH_NETIF_GDMA_TO_CPU_INFO_STC    *toCpuInfoPtr;
    CPSS_DXCH_NETIF_GDMA_FROM_CPU_INFO_STC  *fromCpuInfoPtr;
    CPSS_DXCH_NETIF_GDMA_TO_CPU_PORT_INFO_STC    *toCpuPortInfoPtr = NULL;
    CPSS_DXCH_NETIF_GDMA_FROM_CPU_PORT_INFO_STC  *fromCpuPortInfoPtr;
    GT_U32  firstQueueToCpu;
    GT_U32  numOfQueuesToCpu;
    GT_U32  firstQueueFromCpu;
    GT_U32  numOfQueuesFromCpu;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E   | CPSS_AC5_E    | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E    | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E |
        CPSS_FALCON_E  | CPSS_AC5P_E   | CPSS_AC5X_E   | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(cpuPortInfoPtr);

    allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;
    toCpuInfoPtr   = &allCpuPortsInfoPtr->toCpuInfo;
    fromCpuInfoPtr = &allCpuPortsInfoPtr->fromCpuInfo;

    firstQueueToCpu    = cpuPortInfoPtr->firstQueueToCpu;
    numOfQueuesToCpu   = cpuPortInfoPtr->numOfQueuesToCpu;
    firstQueueFromCpu  = cpuPortInfoPtr->firstQueueFromCpu;
    numOfQueuesFromCpu = cpuPortInfoPtr->numOfQueuesFromCpu;

    orig_numOfQueuesToCpu   = numOfQueuesToCpu  ;
    orig_numOfQueuesFromCpu = numOfQueuesFromCpu;

    if(orig_numOfQueuesFromCpu == 0 && orig_numOfQueuesToCpu == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfQueuesToCpu is ZERO and numOfQueuesFromCpu is ZERO (not allowed)");
    }

    rc = prvCpssDxChPortPhysicalPortIsCpu(devNum, cpuPortNumber, &isCpuPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isCpuPort == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "cpuPortNumber[%d] is not 'CPU GDMA' port : was not mapped as 'CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E'",
            cpuPortNumber);
    }

    /* convert the cpuPortNumber to dmaNum  */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, cpuPortNumber,
        PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, &dmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* find the cpu port that uses this dmaNum */
    for (ii = 0; ii < CPSS_MAX_SDMA_CPU_PORTS_CNS; ii++)
    {
        PRV_CPSS_DXCH_NETIF_SKIP_NON_EXISTS_NETIF_NUM_MAC(devNum,ii);
        if(dmaNum == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[ii].dmaNum)
        {
            break;
        }
    }

    if(ii == CPSS_MAX_SDMA_CPU_PORTS_CNS)
    {
        /* not found ??? should not happen --> 'bad state*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "cpuPortNumber[%d] is not found in PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[] DB",
            cpuPortNumber);
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    netIfNum = ii; /*index to hwInfo.cpuPortInfo.info[] */
    legacy_cpuPortInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[netIfNum];
    cpuPortPtr = &gdmaInfoPtr->cpuPortsArr[netIfNum];

    for (ii = 0; ii < cpuPortPtr->numGdmaUnitsUsed &&
                 ii < PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS;
        ii++)
    {
        gdmaUnitId = cpuPortPtr->gdmaUnitInfo[ii].gdmaUnitId;

        curr_numOfQueues = MIN(numOfQueuesToCpu,
            gdmaInfoPtr->gdmaNumQueues_TO_CPU[gdmaUnitId % PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS]);

        if(curr_numOfQueues)
        {
            curr_bmpOfQueues = BIT_MASK_MAC(curr_numOfQueues);

            /* check collide with TO_CPU/FROM_CPU/FDB/CNC/IPE already used queues */
            CHECK_OVERLAP(curr_bmpOfQueues,firstQueueToCpu,curr_numOfQueues,TO_CPU);

            /*cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_TO_CPU.gdmaLocalFirstQueue = 0 --> already set as 0 */
            cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_TO_CPU.gdmaGlobalQueue = firstQueueToCpu;
            cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_TO_CPU.numOfQueues     = curr_numOfQueues;

            /* update the actual BMP used */
            gdmaInfoPtr->gdmaUsedBmp_TO_CPU.globalGdmaUnits[gdmaUnitId].usedQueuesBmp = curr_bmpOfQueues;

            if(numGdmaUnits_TO_CPU == 0)
            {
                if((toCpuInfoPtr->numOfValidPorts + 1) >=
                    (CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_PORTS_CNS/PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "The cpuPortNumber[%d] can't hold 'TO_CPU' , as other [%d] ports already use the 'TO_CPU'",
                        cpuPortNumber,
                        CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_PORTS_CNS/PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                }

                toCpuPortInfoPtr = &toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts];

                if(toCpuInfoPtr->numOfValidPorts > 0 && toCpuInfoPtr->numOfValidPorts <= CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_PORTS_CNS)
                {
                    /* check that firstQueueToCpu is continues to the previous call {firstQueue+numOfQueuesToCpu} */
                    if(firstQueueToCpu !=
                        toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts - 1].firstQueue +
                        toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts - 1].numOfQueues)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                            "The cpuPortNumber[%d] firstQueueToCpu[%d] not continues to last queue of 'cpu port' [%d] queue [%d] ",
                            cpuPortNumber,
                            firstQueueToCpu,
                            toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts - 1].cpuPortNumber,
                            toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts - 1].firstQueue +
                            toCpuInfoPtr->portsInfo[toCpuInfoPtr->numOfValidPorts - 1].numOfQueues -1);
                    }
                }

                toCpuPortInfoPtr->cpuPortNumber = cpuPortNumber;
                toCpuPortInfoPtr->firstQueue    = firstQueueToCpu;
                toCpuPortInfoPtr->numOfQueues   = numOfQueuesToCpu;

                toCpuInfoPtr->numOfValidPorts++;
            }

            if(toCpuPortInfoPtr)
            {
                toCpuPortInfoPtr->gdmaUnitInfoArr[toCpuPortInfoPtr->numOfGdmaUnits].gdmaUnitId = gdmaUnitId;
                toCpuPortInfoPtr->gdmaUnitInfoArr[toCpuPortInfoPtr->numOfGdmaUnits].gdmaFirstQueue = 0;
                toCpuPortInfoPtr->gdmaUnitInfoArr[toCpuPortInfoPtr->numOfGdmaUnits].gdmaNumOfQueues = curr_numOfQueues;
                toCpuPortInfoPtr->numOfGdmaUnits++;
            }


            numGdmaUnits_TO_CPU++;
            cpuPortPtr->index_gdmaUnitInfo_TO_CPU = ii;

            if(curr_numOfQueues > numOfQueuesToCpu)
            {
                tooManyQueues_TO_CPU_lbl:
                /* the '-=' operation will cause the numOfQueuesToCpu to be 'negative'
                   this means that the caller ask for too many queues for this port
                 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "cpuPortNumber[%d] not support numOfQueuesToCpu[%d] \n",
                    cpuPortNumber,
                    orig_numOfQueuesToCpu);
            }

            firstQueueToCpu  += curr_numOfQueues;
            numOfQueuesToCpu -= curr_numOfQueues;
        }

        curr_numOfQueues = MIN(numOfQueuesFromCpu,
            gdmaInfoPtr->gdmaNumQueues_FROM_CPU[gdmaUnitId % PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS]);
        if(curr_numOfQueues)
        {
            /* for from CPU use queues 'in reverse' from queue : 31... */
            curr_bmpOfQueues = BIT_MASK_MAC(curr_numOfQueues) << (32-curr_numOfQueues);

            /* check collide with TO_CPU/FROM_CPU/FDB/CNC/IPE already used queues */
            CHECK_OVERLAP(curr_bmpOfQueues,firstQueueFromCpu,curr_numOfQueues,FROM_CPU);

            /* for from CPU use queues 'in reverse' , for example 7 queues from 25..31 ,
                and not like the TO_CPU is doing 0..6 */
            cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_FROM_CPU.gdmaLocalFirstQueue = (32-curr_numOfQueues);
            cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_FROM_CPU.gdmaGlobalQueue = firstQueueFromCpu;
            cpuPortPtr->gdmaUnitInfo[ii].gdmaQueueInfo_FROM_CPU.numOfQueues     = curr_numOfQueues;

            /* update the actual BMP used */
            gdmaInfoPtr->gdmaUsedBmp_FROM_CPU.globalGdmaUnits[gdmaUnitId].usedQueuesBmp = curr_bmpOfQueues;

            if(numGdmaUnits_FROM_CPU == 0)
            {
                if((fromCpuInfoPtr->numOfValidPorts + 1) >=
                    (CPSS_DXCH_NETIF_GDMA_FROM_CPU_MAX_PORTS_CNS/PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "The cpuPortNumber[%d] can't hold 'FROM_CPU' , as other [%d] ports already use the 'FROM_CPU'",
                        cpuPortNumber,
                        CPSS_DXCH_NETIF_GDMA_FROM_CPU_MAX_PORTS_CNS/PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                }

                fromCpuPortInfoPtr = &fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts];

                if(fromCpuInfoPtr->numOfValidPorts != 0)
                {
                    /* check that firstQueueFromCpu is continues to the previous call {firstQueue+numOfQueuesFromCpu} */
                    if(firstQueueFromCpu !=
                        fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts - 1].firstQueue +
                        fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts - 1].numOfQueues)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                            "The cpuPortNumber[%d] firstQueueFromCpu[%d] not continues to last queue of 'cpu port' [%d] queue [%d] ",
                            cpuPortNumber,
                            firstQueueFromCpu,
                            fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts - 1].cpuPortNumber,
                            fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts - 1].firstQueue +
                            fromCpuInfoPtr->portsInfo[fromCpuInfoPtr->numOfValidPorts - 1].numOfQueues -1);
                    }
                }

                fromCpuPortInfoPtr->cpuPortNumber = cpuPortNumber;
                fromCpuPortInfoPtr->firstQueue    = firstQueueFromCpu;
                fromCpuPortInfoPtr->numOfQueues   = numOfQueuesFromCpu;
                fromCpuPortInfoPtr->gdmaUnitId    = gdmaUnitId;
                fromCpuPortInfoPtr->gdmaFirstQueue= 32-curr_numOfQueues;

                fromCpuInfoPtr->numOfValidPorts++;
            }

            /* update index to the only GDMA unit to serve this port */
            numGdmaUnits_FROM_CPU++;

            cpuPortPtr->index_gdmaUnitInfo_FROM_CPU = ii;

            if(curr_numOfQueues > numOfQueuesFromCpu)
            {
                tooManyQueues_FROM_CPU_lbl:
                /* the '-=' operation will cause the numOfQueuesFromCpu to be 'negative'
                   this means that the caller ask for too many queues for this port
                 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "cpuPortNumber[%d] not support numOfQueuesFromCpu[%d] \n",
                    cpuPortNumber,
                    orig_numOfQueuesFromCpu);
            }

            firstQueueFromCpu  += curr_numOfQueues;
            numOfQueuesFromCpu -= curr_numOfQueues;
        }
    }

    if(numOfQueuesToCpu != 0)
    {
        goto tooManyQueues_TO_CPU_lbl;
    }
    if(numOfQueuesFromCpu != 0)
    {
        goto tooManyQueues_FROM_CPU_lbl;
    }

    /* check indication for single GDMA unit to serve this port */
    if(numGdmaUnits_TO_CPU == 1)
    {
        /* index to the only GDMA unit to serve this port */
        /* was already set */
    }
    else
    if(numGdmaUnits_FROM_CPU > 1)
    {
        /* indication that we NOT have single GDMA unit to use for the TO_CPU */
        cpuPortPtr->index_gdmaUnitInfo_TO_CPU = (GT_NA-1);
    }
    else
    {
        /* keep the GT_NA that set by prvCpssAasInitParamsSet (...) */
    }


    /*
        so we need to fix :
        1. firstSwQueueIndex --> GT_NA
    */
    legacy_cpuPortInfoPtr->firstSwQueueIndex = 0xFF;/* not to be used by GDMA */

    return GT_OK;
}

/**
* @internal gdmaRingEnableGet function
* @endinternal
*
* @brief   get indication if the rin is enabled/disabled.
*          NOTE: the result depend also on 'idle' bit
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number .
* @param[in] gdmaUnitId               - The global GDMA unit
* @param[in] localRingId              - The local queue/ring number
* @param[out] enablePtr               - (pointer to) is the queue enabled/disabled.
*                                       GT_TRUE  : the ring is enabled.
*                                       GT_FALSE : the ring is disabled (and idle) .
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - HW error
*/
static GT_STATUS gdmaRingEnableGet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   ringId,
    IN GT_BOOL                                  *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      fieldValue;

    /* Get <Ring Enable> , <Ring Idle>*/
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[ringId].queueEnable;
    rc = prvCpssHwPpGdmaGetRegField(devNum,gdmaUnitId,regAddr,0,2,&fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fieldValue & 0x1) /* check bit 0 : <Ring Enable>*/
    {
        /* the queue is enabled */
        *enablePtr = GT_TRUE;
    }
    else
    {
        if(fieldValue & 0x2) /* check bit 1 : <Ring Idle> */
        {
            /* 'idle' = TRUE */

            /* the queue is enabled */
            *enablePtr = GT_FALSE;
        }
        else
        {
            /* 'busy' = TRUE */

            /* probably under disabling operation ... need to wait till disabled */
            *enablePtr = GT_TRUE;
        }
    }

    return GT_OK;
}

/**
* @internal gdmaRingEnableDisable function
* @endinternal
*
* @brief   enable/disable the ring and
*           for disable we can wait for it to come down.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number .
* @param[in] gdmaUnitId               - The global GDMA unit
* @param[in] localRingId              - The local queue/ring number
* @param[in] enable                   - GT_TRUE  : need to enable the queue/ring .
*                                       GT_FALSE : need to disable the queue/ring .
* @param[in] waitForDisable           - when 'disable' , this state if need to
*                                       wait till the queue/ring  is 'idle'
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - HW error
*/
static GT_STATUS gdmaRingEnableDisable
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   ringId,
    IN GT_BOOL                                  enable,
    IN GT_BOOL                                  waitForDisable
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;

    /* set <Ring Enable>*/
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[ringId].queueEnable;
    rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,1,BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    if(enable == GT_FALSE &&
       waitForDisable == GT_TRUE)
    {
        /* wait for the queue to be disabled */

        /*<Ring Idle> : busy is 0 --> we need to wait for '1' --> 'revert' */
        rc = prvCpssHwPpGdmaRegBusyWaitByMask(devNum,gdmaUnitId,regAddr,
            BIT_1,GT_TRUE/*revert*/,GT_FALSE/*wait for idle*/);
    }

    return GT_OK;
}


/* pointer aligned by += diff_to_align */
#define ALIGN_PTR(_ptr,_alignment,_diff)         \
    _diff = (GT_UINTPTR)(_ptr) % (_alignment);   \
    if(_diff)                                    \
    {                                            \
        _ptr = (void*)((GT_CHAR*)(_ptr) + ((_alignment) - _diff));\
    }

/* value aligned by -= diff_to_align */
#define ALIGN_SIZE_FOR_PTR(_value,_diff)    \
    _value -= _diff

/* value aligned by -= diff_to_align */
#define ALIGN_VALUE(_value,_alignment,_diff)     \
    _diff = (_value) % (_alignment);             \
    if(_diff)                                    \
    {                                            \
        _value -= (_alignment) - _diff;          \
    }

/* do malloc + memset */
static void* memCalloc(IN GT_U32 sizeInBytes)
{
    void* pointer;
    pointer = cpssOsMalloc(sizeInBytes);
    if( NULL != pointer)
    {
        cpssOsMemSet(pointer,0,sizeInBytes);
    }
    return pointer;
}

/*
    allocate the buffersNodesArray[] according to buffersNodesNumElements and

    init the link list of :
        freeBuffers_startLinkdedList
        freeBuffers_lastLinkdedList
    with DMA info
*/
static GT_STATUS rxInitBuffersLinkList
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr,
    IN CPSS_RX_BUFF_MALLOC_FUNC                 buffMallocFunc,
    IN GT_U8                                    *static_buffMemPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numElements;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC *currNodePtr;
    GT_U8*  currBuffPtr;
    GT_U32  buffSize = buffersInfoPtr->perQueue_bufferNumOfBytes;
    GT_U32  headerOffset = buffersInfoPtr->headerOffset;
    GT_UINTPTR  phyAddr;/*DMA address*/

    buffersInfoPtr->buffersNodesArray =
        memCalloc(buffersInfoPtr->buffersNodesNumElements *
            sizeof(PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC));
    if( NULL == buffersInfoPtr->buffersNodesArray)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : buffersNodesArray");
    }
    numElements = buffersInfoPtr->buffersNodesNumElements;

    currNodePtr = &buffersInfoPtr->buffersNodesArray[0];

    if(buffMallocFunc)
    {
        currBuffPtr = NULL;
    }
    else
    {
        currBuffPtr = static_buffMemPtr;

        if((buffSize % GDMA_RX_BUFF_ALIGN) != 0)
        {
            buffSize += (GDMA_RX_BUFF_ALIGN -(buffSize % GDMA_RX_BUFF_ALIGN));
        }
    }


    for(ii = 0 ; ii < numElements ; ii++ , currNodePtr++)
    {
        if(buffMallocFunc)
        {
            currBuffPtr = buffMallocFunc(buffSize,GDMA_RX_BUFF_ALIGN);
            if(currBuffPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    "dynamic alloc by 'call back' failed to allocate DMA [%d] bytes",
                    buffSize);
            }

            currBuffPtr += headerOffset;
        }
        else
        {
            if(ii == 0)
            {
                currBuffPtr += headerOffset;
            }
            else
            {
                currBuffPtr += buffSize;
            }
        }

        rc = cpssOsVirt2Phy((GT_UINTPTR)currBuffPtr, &phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr);

        /* fill the phyAddr into node_bufferDmaAddr_low,node_bufferDmaAddr_high */
        PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(phyAddr,
            currNodePtr->node_bufferDmaAddr_low,
            currNodePtr->node_bufferDmaAddr_high);

        currNodePtr->node_bufferDmaAddr_high += PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh;

        if(buffersInfoPtr->perQueue_lastBufferNumOfBytes &&
           (ii == (numElements - 1)))
        {
            currNodePtr->node_bufferNumOfBytes      =  buffersInfoPtr->perQueue_lastBufferNumOfBytes;
        }
        else
        {
            currNodePtr->node_bufferNumOfBytes      =  buffersInfoPtr->perQueue_bufferNumOfBytes;
        }

        /* point to the next node */
        currNodePtr->node_nextPtr = currNodePtr+1;
    }
    /* terminate the list on the last node */
    currNodePtr = &buffersInfoPtr->buffersNodesArray[numElements-1];
    currNodePtr->node_nextPtr = NULL;/* no next */


    /* first node - start of list */
    buffersInfoPtr->freeBuffers_startLinkdedList = &buffersInfoPtr->buffersNodesArray[0];

    /* last node - start of list */
    buffersInfoPtr->freeBuffers_lastLinkdedList  = &buffersInfoPtr->buffersNodesArray[numElements-1];

    /* the other list hold nothing ... yet */
    buffersInfoPtr->freeNodes_startLinkdedList = NULL;
    buffersInfoPtr->freeNodes_lastLinkdedList  = NULL;

    return GT_OK;
}

/* 1. move the first node from freeBuffers_startLinkdedList to be last in freeNodes_lastLinkdedList
   2. invalidate the content of the moved node
*/
static GT_STATUS moveBufferNodeToFreeNodeList(
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr
)
{
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *currNodePtr;
    if(!buffersInfoPtr->freeBuffers_startLinkdedList)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "freeBuffers_startLinkdedList is NULL");
    }

    /* remove the node from : freeBuffers_startLinkdedList ,
       check if need to update also freeBuffers_lastLinkdedList */
    currNodePtr = buffersInfoPtr->freeBuffers_startLinkdedList;
    buffersInfoPtr->freeBuffers_startLinkdedList = currNodePtr->node_nextPtr;
    if(currNodePtr == buffersInfoPtr->freeBuffers_lastLinkdedList)
    {
        buffersInfoPtr->freeBuffers_lastLinkdedList = NULL;
    }

    currNodePtr->node_nextPtr            = NULL ;/* terminate the node */
    currNodePtr->node_bufferDmaAddr_low  = GT_NA;/* indication that not valid */
    currNodePtr->node_bufferDmaAddr_high = GT_NA;/* indication that not valid */

    /* add it to the freeNodes_lastLinkdedList
           check if need to update also freeNodes_startLinkdedList */
    if(buffersInfoPtr->freeNodes_lastLinkdedList == NULL)
    {
        buffersInfoPtr->freeNodes_startLinkdedList = currNodePtr;
    }
    else
    {
        buffersInfoPtr->freeNodes_lastLinkdedList->node_nextPtr = currNodePtr;
    }

    buffersInfoPtr->freeNodes_lastLinkdedList = currNodePtr;


    return GT_OK;
}

/* set the DMA address of base of descriptors , and the number of descriptors

    convert virtual address virtAddrOfDescBase to DMA address and write it to:
        sgdChainBaseAddressLow ,  sgdChainBaseAddressHigh

    and write the numOfDesc to : sgdChainSize
*/
static GT_STATUS    setDmaAddrOfDescBaseToHw(
    IN GT_U8                    devNum,
    IN GT_U32                   gdmaUnitId,
    IN GT_U32                   localRxQueue,
    IN GT_UINTPTR               virtAddrOfDescBase,
    IN GT_U32                   numOfDesc
)
{
    GT_STATUS   rc;
    GT_UINTPTR  phyAddr;/*DMA address*/
    GT_U32  addrLow,addrHigh;
    GT_U32  regAddr;

    /* convert the virtual address of the start array of descriptors */
    /* and write it to the proper registers                          */
    rc = cpssOsVirt2Phy(virtAddrOfDescBase, &phyAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr);

    /* fill the phyAddr into node_bufferDmaAddr_low,node_bufferDmaAddr_high */
    PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(phyAddr,addrLow,addrHigh);

    addrHigh += PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[localRxQueue].sgdChainBaseAddressLow;
    rc = prvCpssHwPpGdmaWriteReg(devNum,gdmaUnitId,regAddr,addrLow);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[localRxQueue].sgdChainBaseAddressHigh;
    rc = prvCpssHwPpGdmaWriteReg(devNum,gdmaUnitId,regAddr,addrHigh);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the number of descriptor */
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[localRxQueue].sgdChainSize;
    rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,16,numOfDesc);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* add 'number of descriptors' that HW can start use (regardless to <OWN> bit)

    called during init stage with numOfDesc of full queue ,
    or during 'Rx packet free' in mode of : CPSS_RX_BUFF_NO_ALLOC_E
        for numOfDesc = 1 , till all descriptors are bound to buffers

    NOTE:  if numOfDesc==0 , we not update the HW.
*/
static GT_STATUS    addDescNumToHw(
    IN GT_U8                    devNum,
    IN GT_U32                   gdmaUnitId,
    IN GT_U32                   localRxQueue,
    IN GT_U32                   numOfDesc
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  ii;

    /* we can update the HW about the number of waiting descriptors */
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[localRxQueue].addGDMASGDOwn;
    for(ii = 0 ; ii < (numOfDesc / 256) ; ii++)
    {
        rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,8,
            0/* value 0 means '256' descriptors */ );
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(numOfDesc % 256)
    {
        /* write the '%256' value to the register */
        rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,8,numOfDesc % 256);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*
    get from the link list of :
        freeBuffers_startLinkdedList
        freeBuffers_lastLinkdedList
    nodes to give info for descriptor that need 'new info' about buffer and <ONW>=HW

    those nodes will move to the link list of :
        freeNodes_startLinkdedList
        freeNodes_lastLinkdedList
*/
static GT_STATUS gdmaRxDescIsReadyForNewInfo
(
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC       *buffersInfoPtr,
    IN PRV_CPSS_GDMA_DESC_STC                                   *currDescPtr,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC   *currNodePtr
)
{
    GT_STATUS   rc;
    GT_U32  phyAddr_2_LSBits;  /*  2 bits to write to word0 of the descriptor */
    GT_U32  phyAddr_32_MSBits; /* 32 bits to write to word1 of the descriptor */
    GT_U32  phyAddr_14_LSBits; /* 14 bits to write to word3 of the descriptor */
    GT_U32  word0;

    if(currNodePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "freeBuffers_startLinkdedList is NULL");
    }
    else
    {
        /* get info from node_bufferDmaAddr_low,node_bufferDmaAddr_high
           and split it to phyAddr_2_LSBits,phyAddr_32_MSBits,phyAddr_14_LSBits
        */
        PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(
            /* IN */
            currNodePtr->node_bufferDmaAddr_low,
            currNodePtr->node_bufferDmaAddr_high,

            /* OUT */
            phyAddr_2_LSBits,
            phyAddr_32_MSBits,
            phyAddr_14_LSBits);

        word0 = OWNER_HW | /* <OWN> = GDMA (HW) */
                (currNodePtr->node_bufferNumOfBytes-1) << 14 |
                phyAddr_2_LSBits << 30;

        /* we are done with the node in freeBuffers_startLinkdedList (currNodePtr) */
        /* so we need to move it to freeNodes_lastLinkdedList and reset it's DMA info */
        rc = moveBufferNodeToFreeNodeList(buffersInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        currDescPtr->word0 = CPSS_32BIT_LE(word0);
        currDescPtr->word1 = CPSS_32BIT_LE(phyAddr_32_MSBits);
        currDescPtr->word2 = 0;/* !!! not in use !!! hold only reserved fields */
        currDescPtr->word3 = CPSS_32BIT_LE(phyAddr_14_LSBits);
    }

    /* make sure that the descriptor fully written */
    GT_SYNC;

    return GT_OK;
}

/*
    get from the link list of :
        freeBuffers_startLinkdedList
        freeBuffers_lastLinkdedList
    nodes to give the array of descriptors that need 'info' about buffer and <ONW>=HW

    those nodes will move to the link list of :
        freeNodes_startLinkdedList
        freeNodes_lastLinkdedList

    update the HW about those descriptors that are ready !!!
*/
static GT_STATUS rxInitDescriptorArr
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   localRxQueue,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numDesc;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC *currNodePtr;
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr;
    /* we know that we have less descriptors (<=) than buffers , so we can loop
       on number of descriptors , also in the array of buffer nodes */
    numDesc = decriptorsInfoPtr->toCpu_numOfDescriptors;

    currNodePtr = &buffersInfoPtr->buffersNodesArray[0];
    currDescPtr = decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress;

    for(ii = 0 ; ii < numDesc ; ii++ , currNodePtr++ , currDescPtr++)
    {
        rc = gdmaRxDescIsReadyForNewInfo(buffersInfoPtr,currDescPtr,currNodePtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* set the DMA address of base of descriptors , according to the virtual addr */
    rc = setDmaAddrOfDescBaseToHw(devNum,gdmaUnitId,localRxQueue,
        (GT_UINTPTR)decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress,
        numDesc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* we can update the HW about the number of waiting descriptors */
    /* add 'number of descriptors' that HW can start use (regardless to <OWN> bit) */
    rc = addDescNumToHw(devNum,gdmaUnitId,localRxQueue,numDesc);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

typedef enum{
 GDMA_RING_TYPE_MEM2MEM_E            = 0x0,
 GDMA_RING_TYPE_TO_CPU_PKT_MODE_E    = 0x1,
 GDMA_RING_TYPE_TO_CPU_MSG_MODE_E    = 0x2,
 GDMA_RING_TYPE_PDI2PDI_MSG2PKT_E    = 0x3,
 GDMA_RING_TYPE_PDI2PDI_LPBK_E       = 0x4,
 GDMA_RING_TYPE_FROM_CPU_E           = 0x5,
 GDMA_RING_TYPE_FROM_CPU_PKT_GEN_E   = 0x6
}GDMA_RING_TYPE_ENT;

typedef enum{DROP=0,WAIT=1}LOR_MODE;

/**
* @internal gdmaInitRingType function
* @endinternal
*
* @brief  GDMA : set the ring type to it's purpose
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
* @param[in] gdmaUnitId               - The global GDMA unit
* @param[in] localRingId              - The local queue/ring number
* @param[in] ringType                 - The ring type
* @param[in] sgdRecycleChain          - Whether to restart the GDMA chain when ends
* @param[in] sgdExtendMode            - Defines the size mode of the SGD
*                                       (relevant in FROM/TO CPU rings, for MEM2MEM rings extended format is always used)
* @param[in] lorMode                  - Lack of Resources mode (DROP/WAIT)
*
* @retval GT_OK                    - on success.
* @retval GT_INIT_ERROR            - on init error
*/
static GT_STATUS gdmaInitRingType
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   localRingId,
    IN GDMA_RING_TYPE_ENT                       ringType,
    IN GT_BOOL                                  sgdRecycleChain,
    IN GT_BOOL                                  sgdExtendMode,
    IN GT_U32                                   rxBufferByteCnt,
    IN LOR_MODE                                 lorMode /*DROP/WAIT*/
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      fieldValue;

    fieldValue = (GT_U32)ringType |          /*Ring Type*/
        BOOL2BIT_MAC(sgdRecycleChain) << 4 | /*SGD Recycle Chain*/
        BOOL2BIT_MAC(sgdExtendMode)   << 5 | /*SGD Extend mode*/
        lorMode << 6;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRingId].queueControl;

    /*RX buffer Byte Cnt*/

    rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,7,fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(rxBufferByteCnt == 0)
    {
        fieldValue = 0;
    }
    else
    {
        fieldValue = rxBufferByteCnt/*RX buffer Byte Cnt*/ |
            BIT_16/*RX Buffer Byte Count Mode*/;
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRingId].rxBufferByteCount;
    rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,17,fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(GDMA_RING_TYPE_FROM_CPU_E == ringType)
    {
        /* set the <PDI enable> to 'enable' */
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).pdiMasterRegs.pdiMasterControl;
        rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,1,1);
        if(rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).generalRegs.gdmaMiscConfig;
        /* <PDI Master word swap enable> , <PDI Master byte swap enable> */
        rc = prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr, 0, 2, 3);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal gdmaRxInitQueue function
* @endinternal
*
* @brief   This function initializes the Core Rx module, by allocating the cyclic
*         Rx descriptors Array , and the rx buffers. -- GDMA
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
* @param[in] systemRecoveryProcess    - The system HA mode
* @param[in] globalQueueNum           - The global queue number (not local in the context of netIfNum)
* @param[in] gdmaUnitId               - The global GDMA unit Id
* @param[in] localRxQueue             - The local queue number in the GDMA unit
* @param[in] phase2InitInfoPtr        - (pointer to) info from phase 2 init about this queue
*
* @param[out] gdmaPerQueue_toCpu_Ptr  - (pointer to) the queue info that need to be filled for later use
*
* @retval GT_OK                    - on success.
* @retval GT_INIT_ERROR            - on init error
*/
static GT_STATUS gdmaRxInitQueue
(
    IN GT_U8                                    devNum,
    IN CPSS_SYSTEM_RECOVERY_PROCESS_ENT         systemRecoveryProcess,
    IN GT_U32                                   globalQueueNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   localRxQueue,
    IN CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC      *phase2InitInfoPtr,
    OUT PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Ptr
)
{
    GT_STATUS rc;
    GT_U32 numOfRxDesc;         /* Number of Rx desc. that may be       */
                                /* allocated from the given block.      */
    /* The following vars. will hold the data from phase2InitInfoPtr, and only  */
    /* some of them will be used, depending on the allocation method.   */
    GT_U32 buffBlockSize = 0;   /* The static buffer block size (bytes) */
    GT_U8 *buffBlock = NULL;    /* A pointer to the static buffers block*/
    CPSS_RX_BUFF_ALLOC_METHOD_ENT allocMethod = phase2InitInfoPtr->buffAllocMethod;
    CPSS_RX_BUFF_MALLOC_FUNC buffMallocFunc = NULL;/* Function for allocating the buffers. */
    GT_U32         headerOffset = phase2InitInfoPtr->buffHeaderOffset;
    GT_U32         buffSize = phase2InitInfoPtr->buffSize;
    GT_U32         diffToAlign;/* number of bytes that needed for alignment */

    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    GT_U32 buffersNodesNumElements; /*number of needed buffers */

    buffersInfoPtr    = &gdmaPerQueue_toCpu_Ptr->toCpu_buffers;
    decriptorsInfoPtr = &gdmaPerQueue_toCpu_Ptr->toCpu_descriptors;

    if(globalQueueNum >= NUM_OF_SDMA_QUEUES)
    {
        /* add check for klockwork */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* save space before user header for internal packets indication */
    buffSize     += PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS;
    headerOffset += PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS;

    /* Set the descBlockPtr to point to an aligned start address. */
    ALIGN_PTR(phase2InitInfoPtr->descMemPtr,PRV_CPSS_GDMA_DESC_ALIGN,diffToAlign);
    ALIGN_SIZE_FOR_PTR(phase2InitInfoPtr->descMemSize,diffToAlign);
    /* Number of Rx descriptors is calculated according to the  */
    /* (fixed) size of the given Rx Descriptors block.                  */
    numOfRxDesc = phase2InitInfoPtr->descMemSize / PRV_CPSS_GDMA_DESC_ALIGN;
    if(numOfRxDesc == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "TO_CPU : globalQueueNum[%d] : num of descriptors is ZERO ?!",
            globalQueueNum);
    }
    if(numOfRxDesc >= BIT_16)/*HW limited to 16 bits*/
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "TO_CPU : globalQueueNum[%d] : num of descriptors is [%d] >= 64K",
            globalQueueNum,
            numOfRxDesc);
    }

    /* disable queue , and wait for it to come down */
    rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localRxQueue,
            GT_FALSE/*disable*/,
            GT_TRUE/*waitForDisable*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set the ring to mode 'packets TO_CPU' */
    rc = gdmaInitRingType(devNum,gdmaUnitId,localRxQueue,
        GDMA_RING_TYPE_TO_CPU_PKT_MODE_E,
        GT_TRUE/*sgdRecycleChain*/,
        GT_TRUE/*sgdExtendMode*/,
        ((allocMethod == CPSS_RX_BUFF_NO_ALLOC_E) ? 0 :
          phase2InitInfoPtr->buffSize - 1)/*rxBufferByteCnt*/,
          DROP/*lorMode*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    gdmaPerQueue_toCpu_Ptr->buffersInCachedMem = phase2InitInfoPtr->buffersInCachedMem;


    decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress =
        (void*)phase2InitInfoPtr->descMemPtr;
    decriptorsInfoPtr->toCpu_numOfDescriptors = numOfRxDesc;

    decriptorsInfoPtr->toCpu_currDescriptorIndex = 0;

    if(allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        buffersInfoPtr->buffersNodesNumElements      = 0;
        buffersInfoPtr->buffersNodesArray            = NULL;
        buffersInfoPtr->freeBuffers_startLinkdedList = NULL;
        buffersInfoPtr->freeBuffers_lastLinkdedList  = NULL;
        buffersInfoPtr->freeNodes_startLinkdedList   = NULL;
        buffersInfoPtr->freeNodes_lastLinkdedList    = NULL;
        /*
         *  Each packet with it's own size.
         *   see : PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC::node_bufferNumOfBytes
        */

        buffersInfoPtr->perQueue_bufferNumOfBytes    = GT_NA; /* so the parameter must be used !!! */
        /*
         *   The application is the one to return buffer of 'rx packet free'
         *   without manipulations by the CPSS
         *   see function internal_cpssDxChNetIfRxBufFreeWithSize
        */
        buffersInfoPtr->headerOffset = GT_NA; /* so the parameter must not be used !!! */

        /* Do not enable the queues in CPSS_RX_BUFF_NO_ALLOC_E mode. */
        /* Application responsibility to enable the queue after all buffers attached. */
        return GT_OK;
    }

    if((headerOffset % GDMA_RX_BUFF_ALIGN) != 0)
    {
        headerOffset += (GDMA_RX_BUFF_ALIGN -(headerOffset % GDMA_RX_BUFF_ALIGN));
    }

    if(allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
    {
        buffBlock = (GT_U8*)phase2InitInfoPtr->memData.staticAlloc.buffMemPtr;
        buffBlockSize = phase2InitInfoPtr->memData.staticAlloc.buffMemSize;

        /* Set the buffers block to point to a properly aligned block. */
        ALIGN_PTR(buffBlock,GDMA_RX_BUFF_ALIGN,diffToAlign);
        ALIGN_SIZE_FOR_PTR(buffBlockSize,diffToAlign);

        buffersNodesNumElements = buffBlockSize / buffSize;
    }
    else
    {
        buffMallocFunc = phase2InitInfoPtr->memData.dynamicAlloc.buffMallocFunc;
        /* we need to use numOfRxBuff that was not used by CPSS till now */
        buffersNodesNumElements = phase2InitInfoPtr->numOfRxBuff;
    }

    /* save the value from application + 4 bytes from PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS */
    buffersInfoPtr->headerOffset = headerOffset;

#if 1
    if(buffersNodesNumElements < 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "TO_CPU : globalQueueNum[%d] : can't use less than 2 buffers ",
            globalQueueNum);
    }
    if((numOfRxDesc * 2) > buffersNodesNumElements)
    {
        /* we need at least 2 times the number of buffers ,than the number of descriptors.
           so when 'rx packet get' API serves the packet , there will be free buffer for it.
        */
        numOfRxDesc = buffersNodesNumElements / 2;
        /* update also the already saved value */
        decriptorsInfoPtr->toCpu_numOfDescriptors = numOfRxDesc;
    }
#else /*0*/
    /* we need the number of buffers to NOT be less than the number of descriptors */
    /* as it will cause holes in the cyclic ring */
    if(numOfRxDesc > buffersNodesNumElements)
    {
        if((numOfRxDesc - 1) == buffersNodesNumElements &&
           allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E &&
           numOfRxDesc > 5)
        {
            /* probably the calculation lost buffer due to the 4 bytes from
               PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS per buffer that
               application not considered */
            /* so in order to not fail the application , lets reduce the number
               of descriptors to use , by one to equal to number of buffers */
            numOfRxDesc = buffersNodesNumElements;

            /* update also the already saved value */
            decriptorsInfoPtr->toCpu_numOfDescriptors = numOfRxDesc;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "TO_CPU : globalQueueNum[%d] : num of descriptors[%d] need to be <= num of buffers [%d] ",
                globalQueueNum,
                numOfRxDesc,
                buffersNodesNumElements);
        }
    }
#endif /*0*/

    /* use phase2InitInfoPtr->buffSize and not 'buffSize' , because 'buffSize'
       hold additional 4 bytes for manipulations , that not needed when we write
       to the descriptor the buffer side */
    buffersInfoPtr->perQueue_bufferNumOfBytes    = phase2InitInfoPtr->buffSize;

    /* the GDMA need to have buffers >= 64 bytes */
    if(phase2InitInfoPtr->buffSize < 64)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR,
            "phase2InitInfoPtr->buffSize[%d] for TO_CPU global queue [%d] must be >= 64",
            buffSize,
            globalQueueNum);
    }

    if(buffSize >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR,
            "phase2InitInfoPtr->buffSize[%d] for TO_CPU global queue [%d] must be < [%d]",
            buffSize,
            globalQueueNum,
            (BIT_16 - PRV_CPSS_DXCH_NET_INTERNAL_RX_PACKET_INFO_SIZE_CNS));
    }

    buffersInfoPtr->buffersNodesNumElements = buffersNodesNumElements;

    /*
        allocate the buffersNodesArray[] according to buffersNodesNumElements and

        init the link list of :
            freeBuffers_startLinkdedList
            freeBuffers_lastLinkdedList
        with DMA info
    */
    rc = rxInitBuffersLinkList(devNum,
        buffersInfoPtr,
        buffMallocFunc, /* for dynamic mode */
        buffBlock);     /* for static mode  */
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
        get from the link list of :
            freeBuffers_startLinkdedList
            freeBuffers_lastLinkdedList
        nodes to give the array of descriptors

        those nodes will mode to the link list of :
            freeNodes_startLinkdedList
            freeNodes_lastLinkdedList

        update the HW about those descriptors that are ready !!!
    */
    rc = rxInitDescriptorArr(devNum,
        gdmaUnitId,
        localRxQueue,
        decriptorsInfoPtr,
        buffersInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* if rxsdma init is going under recovery process as PP is disabled to write CPU memory, enable Rx SDMA queues just  */
    /* after PP would be able to access CPU memory(in catch up stage)- otherwise syncronization may be lost between      */
    /* PP and descriptors                                                                                                */
    if (systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /* Enable the Rx SDMA Queue */
        rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localRxQueue,
                GT_TRUE/*enable*/,
                GT_TRUE/*dont care for 'enable*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal gdmaPdiSlaveEnable function
* @endinternal
*
* @brief  enable the PDI slave for TO_CPU,MSG_TO_CPU(FDB,IPE,CNC0..3)
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the Rx unit for.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
static GT_STATUS gdmaPdiSlaveEnable
(
    IN GT_U8        devNum,
    IN GT_U32       gdmaUnitId,
    IN GT_U32       pdiIndex
)
{
    GT_U32  regAddr;

    /* enable the PDI slave interface for this GDMA for PDI interface in the unit */
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).pdiSlaveRegs[pdiIndex].pdiSlaveControl;
    return prvCpssHwPpGdmaSetRegField(devNum,gdmaUnitId,regAddr,0,1,1);
}

/**
* @internal gdmaDispatcherInit function
* @endinternal
*
* @brief  init the GDMA dispatcher unit that needed for 'TO_CPU'
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the Rx unit for.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
static GT_STATUS gdmaDispatcherInit
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    PRV_CPSS_DXCH_GDMA_INFO_STC                       *gdmaInfoPtr;
    GT_U32  gdmaUnitId;
    GT_U32  ii;

    /*<GDMA PDI Enable>*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->CNM.
        GDMA_Dispatcher.gdmaDispatcherControl;
    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,1,1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* <Dispatcher Slave Port Assign Mode> : set to legacy mode , to not use the
       CPU code table , till the application have API to control this bit
       (and set the CPU code table in this unit) */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,1,1,0);
    if(rc != GT_OK)
    {
        return rc;
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    for(ii = 0 ; ii < PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS; ii++)
    {
        gdmaUnitId = gdmaInfoPtr->gdmaDispatcherToUnitmap[ii];
        if(gdmaUnitId == GT_NA)
        {
            continue;
        }

        /* enable the PDI slave interface for this GDMA as PDI interface 0 in the unit */
        rc = gdmaPdiSlaveEnable(devNum,gdmaUnitId,0/*PDI interface 0*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

extern GT_STATUS prvCpssPpDrvIntDefDxChAasBindGdmaInterruptsToPortMappingDependedEvents
(
    IN  GT_U8  devNum,
    IN  GT_U32 gdmaUnit,
    IN  GT_U32 rxQueue0StartRing,
    IN  GT_U32 numOfQueues
);

/**
* @internal prvCpssDxChGdmaNetIfRxInit function
* @endinternal
*
* @brief   This function initializes the GDMA for TO_CPU , by allocating the array
*         of TO_CPU descriptors , and the TO_CPU buffers
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxInit
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_U32      totalNumOfQueues;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Arr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Ptr;
    CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC *phase2InitInfoPtr;
    GT_U32      ii,gdmaIterator,gdmaNumOfQueues;
    GT_U32      netIfNum;
    GT_U32      gdmaUnitId,gdmaFirstQueue;
    GT_U32      firstQueue,numOfQueues;
    GT_U32      localRxQueue,globalRxQueue;
    /* indexes into 'legacy' DB of :
       moduleCfgPtr->multiNetIfCfg.rxSdmaQueuesConfig[legacy_cpuPortIndex][legacy_queueIndex]*/
    GT_U32      legacy_cpuPortIndex,legacy_queueIndex;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC * moduleCfgPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg); /* pointer to the module configure of the PP's database*/
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /*holds system recovery info*/
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *allCpuPortsInfoPtr;
    GT_U32      wa_gdmaFirstQueue_toCpu;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* init the GDMA-dispatcher sub-unit that dispatch 'TO_CPU' to several
       GDMA units : to allow more than 32 TO_CPU queues */
    rc = gdmaDispatcherInit(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get info about all CPU ports that need Rx/Tx queues */
    allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;

    if(allCpuPortsInfoPtr->toCpuInfo.numOfValidPorts == 0)
    {
        /* nothing more to do */
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr)
    {
        /* nothing more to do , already initialized */
        return GT_OK;
    }

    netIfNum = allCpuPortsInfoPtr->toCpuInfo.numOfValidPorts - 1;
    /* calc the number of queues needed for the 'TO_CPU' */
    totalNumOfQueues = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].firstQueue +
                       allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].numOfQueues;

    PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_toCpu = totalNumOfQueues;

    gdmaPerQueue_toCpu_Arr = memCalloc(totalNumOfQueues *
            sizeof(PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC));
    if( NULL == gdmaPerQueue_toCpu_Arr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : gdmaPerQueue_toCpu_Arr");
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr = gdmaPerQueue_toCpu_Arr;

    gdmaPerQueue_toCpu_Ptr = &gdmaPerQueue_toCpu_Arr[0];

    /* init the 'to_cpu' part of queues */
    for(netIfNum = 0; netIfNum < allCpuPortsInfoPtr->toCpuInfo.numOfValidPorts ; netIfNum++ )
    {
        firstQueue     = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].firstQueue;
        numOfQueues    = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].numOfQueues;
        globalRxQueue  = firstQueue;

        {
            CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
            GT_PHYSICAL_PORT_NUM cpuPortNumber = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].cpuPortNumber;

            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum,cpuPortNumber,&portMapShadow);
            if(rc != GT_OK)
            {
                return rc;
            }
            CPSS_TBD_BOOKMARK_AAS
            /* next needed when <Dispatcher Slave Port Assign Mode> : set to legacy mode */
            wa_gdmaFirstQueue_toCpu = (portMapShadow.extPortMap.txqInfo.secondarySchFirstQueueIndex & 0xF); /* 0 or 8 */
        }

        for(gdmaIterator = 0 ;
            gdmaIterator < allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].numOfGdmaUnits ;
            gdmaIterator++)
        {
            gdmaUnitId     = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].gdmaUnitInfoArr[gdmaIterator].gdmaUnitId;
            gdmaFirstQueue = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].gdmaUnitInfoArr[gdmaIterator].gdmaFirstQueue;
            gdmaNumOfQueues = allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].gdmaUnitInfoArr[gdmaIterator].gdmaNumOfQueues;

            localRxQueue = gdmaFirstQueue;
            if(gdmaIterator == 0 && wa_gdmaFirstQueue_toCpu != 0)
            {
                localRxQueue += wa_gdmaFirstQueue_toCpu;
                /* update the interrupts unified event to connect the queues to rings with the proper offset */
                rc = prvCpssPpDrvIntDefDxChAasBindGdmaInterruptsToPortMappingDependedEvents(
                    devNum,gdmaUnitId,wa_gdmaFirstQueue_toCpu,gdmaNumOfQueues);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            for(ii = 0 ;
                globalRxQueue < (firstQueue +  numOfQueues) &&
                ii < gdmaNumOfQueues;
                ii ++, globalRxQueue++,localRxQueue++ , gdmaPerQueue_toCpu_Ptr++)
            {
                gdmaPerQueue_toCpu_Ptr->gdmaUnitId   = gdmaUnitId;
                gdmaPerQueue_toCpu_Ptr->localRxQueue = localRxQueue % 32;

                legacy_cpuPortIndex = globalRxQueue / CPSS_MAX_RX_QUEUE_CNS;
                legacy_queueIndex   = globalRxQueue % CPSS_MAX_RX_QUEUE_CNS;

                phase2InitInfoPtr = &moduleCfgPtr->multiNetIfCfg.rxSdmaQueuesConfig[legacy_cpuPortIndex][legacy_queueIndex];

                if(phase2InitInfoPtr->descMemSize != 0)
                {
                    rc = gdmaRxInitQueue(devNum,
                        tempSystemRecovery_Info.systemRecoveryProcess,
                        globalRxQueue,
                        gdmaUnitId,localRxQueue % 32,
                        phase2InitInfoPtr,
                        /*OUT*/gdmaPerQueue_toCpu_Ptr);
                    if (GT_OK != rc)
                    {
                        return rc;
                    }
                }
                PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[globalRxQueue] =
                    phase2InitInfoPtr->buffAllocMethod;
            }
        }/*gdmaIterator*/
    }

    return GT_OK;
}


/* 1. move the first node from freeNodes_startLinkdedList to be last in freeBuffers_lastLinkdedList
   2. and set the value of the buffer DMA address and buffer size
*/
static GT_STATUS moveFreeNodeToBufferNodeList(
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr,
    IN GT_U32 node_bufferDmaAddr_low,
    IN GT_U32 node_bufferDmaAddr_high,
    IN GT_U32  node_bufferNumOfBytes,
    IN GT_BOOL  allowAddNewNode
)
{
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *currNodePtr;

    if(NULL == buffersInfoPtr->freeNodes_startLinkdedList)
    {
        if(allowAddNewNode == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "freeNodes_startLinkdedList is NULL");
        }

        /* allocMethod == CPSS_RX_BUFF_NO_ALLOC_E */

        /* allocate the new node , as we not did it during gdmaRxInitQueue(...) */
        currNodePtr = memCalloc(sizeof(PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC));
        if( NULL == currNodePtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : currNodePtr");
        }
    }
    else
    {
        /* remove the node from : freeNodes_startLinkdedList ,
           check if need to update also freeNodes_lastLinkdedList */
        currNodePtr = buffersInfoPtr->freeNodes_startLinkdedList;
        buffersInfoPtr->freeNodes_startLinkdedList = currNodePtr->node_nextPtr;
        if(currNodePtr == buffersInfoPtr->freeNodes_lastLinkdedList)
        {
            buffersInfoPtr->freeNodes_lastLinkdedList = NULL;
        }
    }

    currNodePtr->node_nextPtr            = NULL ;/* terminate the node */
    currNodePtr->node_bufferDmaAddr_low  = node_bufferDmaAddr_low;
    currNodePtr->node_bufferDmaAddr_high = node_bufferDmaAddr_high;
    currNodePtr->node_bufferNumOfBytes   = node_bufferNumOfBytes;

    /* add it to the freeBuffers_lastLinkdedList
           check if need to update also freeBuffers_startLinkdedList */
    if(buffersInfoPtr->freeBuffers_lastLinkdedList == NULL)
    {
        buffersInfoPtr->freeBuffers_startLinkdedList = currNodePtr;
    }
    else
    {
        buffersInfoPtr->freeBuffers_lastLinkdedList->node_nextPtr = currNodePtr;
    }

    buffersInfoPtr->freeBuffers_lastLinkdedList = currNodePtr;

    return GT_OK;
}

/* we need to update the HW about another descriptor that can be
   bound to this buffer */
static GT_STATUS   addDescWithBuffer_from_CPSS_RX_BUFF_NO_ALLOC_E(
    IN GT_U8                    devNum,
    IN GT_U32                   gdmaUnitId,
    IN GT_U32                   localRxQueue,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr,
    IN PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC     *buffersInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  phyAddr_2_LSBits;  /*  2 bits to write to word0 of the descriptor */
    GT_U32  phyAddr_32_MSBits; /* 32 bits to write to word1 of the descriptor */
    GT_U32  phyAddr_14_LSBits; /* 14 bits to write to word3 of the descriptor */
    GT_U32  word0;
    PRV_CPSS_GDMA_DESC_STC   *currDescPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC *currNodePtr;

    currDescPtr = decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress +
                  decriptorsInfoPtr->toCpu_numOfBuffers_for_CPSS_RX_BUFF_NO_ALLOC_E;

    decriptorsInfoPtr->toCpu_numOfBuffers_for_CPSS_RX_BUFF_NO_ALLOC_E ++;

    /* we just added node to it during : moveFreeNodeToBufferNodeList(...) */
    currNodePtr = buffersInfoPtr->freeBuffers_lastLinkdedList;

    /* get info from node_bufferDmaAddr_low,node_bufferDmaAddr_high
       and split it to phyAddr_2_LSBits,phyAddr_32_MSBits,phyAddr_14_LSBits
    */
    PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(
        /* IN */
        currNodePtr->node_bufferDmaAddr_low,
        currNodePtr->node_bufferDmaAddr_high,

        /* OUT */
        phyAddr_2_LSBits,
        phyAddr_32_MSBits,
        phyAddr_14_LSBits);

    word0 = OWNER_HW | /* <OWN> = GDMA (HW) */
            (currNodePtr->node_bufferNumOfBytes - 1) << 14 |
            phyAddr_2_LSBits << 30 ;

    /* we are done with the node in freeBuffers_startLinkdedList (currNodePtr) */
    /* so we need to move it to freeNodes_lastLinkdedList and reset it's DMA info */
    rc = moveBufferNodeToFreeNodeList(buffersInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    currDescPtr->word0 = CPSS_32BIT_LE(word0);
    currDescPtr->word1 = CPSS_32BIT_LE(phyAddr_32_MSBits);
    currDescPtr->word2 = 0;/* !!! not in use !!! hold only reserved fields */
    currDescPtr->word3 = CPSS_32BIT_LE(phyAddr_14_LSBits);

    /* make sure that the descriptor fully written */
    GT_SYNC;

    /* we can update the HW about the number of waiting descriptors */
    /* add 'single descriptor' that HW can start use (regardless to <OWN> bit) */
    rc = addDescNumToHw(devNum,gdmaUnitId,localRxQueue,1);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfRxBufFreeWithSize function
* @endinternal
*
* @brief  GDMA : Free a list of buffers, that where previously passed to the application
*         by the 'Rx Packet Get' API. -- GDMA relate
*
*         NOTE: in CPSS_RX_BUFF_DYNAMIC_ALLOC_E/CPSS_RX_BUFF_STATIC_ALLOC_E mode :
*               the function NOT do any read/write to HW
*               the function NOT do any read/write to descriptors/buffers memory in DRAM
*               the function only save the info about the released buffer into the link list
*                   that wait for descriptor(s) to need to bind to buffer.
*               in CPSS_RX_BUFF_NO_ALLOC_E mode:
*                   if the amount of buffers returned by this function (in total by all calls to it)
*                   is less than the number of descriptors that allocated for this queue then
*                       the function will update 'empty' descriptor , with this buffer info
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number through which these buffers where received.
* @param[in] globalRxQueue            - The Rx queue number through which these buffers where received.
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes , to set to the free descriptor
*                                      if this parameter is NULL --> we ignore it.
* @param[in] buffListLen              - Length of rxBufList.
*
* @retval GT_OK on success, or
* @retval GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChGdmaNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    globalRxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_STATUS rc;
    GT_U32  ii;

    GT_U32  gdmaUnitId,localRxQueue;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Ptr;
    GT_U32 bufferDmaAddr_low,bufferDmaAddr_high,bufferNumOfBytes;
    GT_UINTPTR  phyAddr;/*DMA address*/
    GT_BOOL allowAddNewNode;

    /*  check of :
        PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,rxQueue);
        was already done by the caller
    */
    gdmaPerQueue_toCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr[globalRxQueue];

    buffersInfoPtr    = &gdmaPerQueue_toCpu_Ptr->toCpu_buffers;
    decriptorsInfoPtr = &gdmaPerQueue_toCpu_Ptr->toCpu_descriptors;

    if(rxBuffSizeList &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[globalRxQueue] == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        /*allocMethod == CPSS_RX_BUFF_NO_ALLOC_E*/
        allowAddNewNode = GT_TRUE;
    }
    else
    {
        allowAddNewNode = GT_FALSE;
    }


    /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
    PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
        gdmaUnitId,localRxQueue);

    for(ii = 0; ii < buffListLen; ii++)
    {
        rc = cpssOsVirt2Phy((GT_UINTPTR)(rxBuffList[ii]),&phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr);

        /* fill the phyAddr into node_bufferDmaAddr_low,node_bufferDmaAddr_high */
        PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(phyAddr,
            bufferDmaAddr_low,
            bufferDmaAddr_high);

        bufferDmaAddr_high += PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh;

        if(rxBuffSizeList == NULL)
        {
            bufferNumOfBytes =  buffersInfoPtr->perQueue_bufferNumOfBytes;
        }
        else
        {
            bufferNumOfBytes = rxBuffSizeList[ii];
        }

        /* 1. move the first node from freeNodes_startLinkdedList to be last in freeBuffers_lastLinkdedList
           2. and set the value of the buffer DMA address and buffer size
        */
        rc = moveFreeNodeToBufferNodeList(buffersInfoPtr,bufferDmaAddr_low,bufferDmaAddr_high,bufferNumOfBytes,
            allowAddNewNode);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(allowAddNewNode && /* only in mode : allocMethod == CPSS_RX_BUFF_NO_ALLOC_E */
           (decriptorsInfoPtr->toCpu_numOfDescriptors >
            decriptorsInfoPtr->toCpu_numOfBuffers_for_CPSS_RX_BUFF_NO_ALLOC_E))
        {
            /* we need to update the HW about another descriptor that can be
               bound to this buffer */
            rc = addDescWithBuffer_from_CPSS_RX_BUFF_NO_ALLOC_E(
                devNum,
                gdmaUnitId,
                localRxQueue,
                decriptorsInfoPtr,
                buffersInfoPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfRxPacketGet function
* @endinternal
*
* @brief  GDMA :  This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - Device number.
* @param[in] globalRxQueue         - The Rx queue number through which these packets received.
* @param[in] numOfBuffPtr          - (pointer to) Num of buffs in packetBuffsArrPtr that application want to get (at max).
*
* @param[in] numOfBuffPtr          - (pointer to) Num of buffs in packetBuffsArrPtr that CPSS bind for the packet.
* @param[out] packetBuffsArrPtr[]  - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]         - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                globalRxQueue,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
)
{
    GT_STATUS rc,rc1;
    GT_U32  ii;

    GT_U32  gdmaUnitId,localRxQueue;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC      *buffersInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC              *gdmaPerQueue_toCpu_Ptr;
    GT_UINTPTR  phyAddr;/*DMA address*/
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */
    GT_U32  descIndex;
    GT_U32  ownBit,failure_code,isLast,isFirst,buffLen;
    GT_U8   *buffPtr;
    GT_U32  IN_numOfBuff = *numOfBuffPtr;
    GT_U32  didError;/* error indication in one of the descriptors*/
    GT_U32  errorMessageDone = 0;
    GT_U32  numOfDescriptorsHandled = 0;

    #ifdef CPU_BE
    PRV_CPSS_GDMA_DESC_STC  currDesc_BE;
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr_afterSwap = &currDesc_BE;
    #else
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr_afterSwap;/* set in runtime to currDescPtr */
    #endif  /*CPU_BE*/
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr;

    /*  check of :
        PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,rxQueue);
        was already done by the caller
    */
    /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
    PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
        gdmaUnitId,localRxQueue);

    gdmaPerQueue_toCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr[globalRxQueue];

    buffersInfoPtr    = &gdmaPerQueue_toCpu_Ptr->toCpu_buffers;
    decriptorsInfoPtr = &gdmaPerQueue_toCpu_Ptr->toCpu_descriptors;

    /* label to start packet again  */
retryGetPacketFromStart_lbl:
    didError = 0;
    rc1 = GT_OK;
    *numOfBuffPtr = IN_numOfBuff;

    descIndex   =  decriptorsInfoPtr->toCpu_currDescriptorIndex;
    currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[descIndex];

    *numOfBuffPtr = 0;

    for(ii = 0;
        ii < decriptorsInfoPtr->toCpu_numOfDescriptors;
        ii ++, currDescPtr++, descIndex++)
    {
        if(descIndex == decriptorsInfoPtr->toCpu_numOfDescriptors)
        {
            /* running in cyclic mode */
            descIndex   = 0;
            currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[descIndex];
        }

        #ifdef CPU_BE
            /* we need to swap the values in order to treat them as 'little endian' */
            /* therefore we 'copy' them to local variable */
            currDesc_BE.word0 = CPSS_32BIT_LE(currDescPtr->word0);
            currDesc_BE.word1 = CPSS_32BIT_LE(currDescPtr->word1);
            /*currDescPtr->word2 = CPSS_32BIT_LE(currDescPtr->word2); word not in use */
            currDesc_BE.word3 = CPSS_32BIT_LE(currDescPtr->word3);
            /*currDescPtr_afterSwap = &currDesc_BE;*/
        #else
            /* no SWAP needed */
            /* therefore we NOT 'copy' them to local variable */
            /* so just point to the current descriptor        */
            currDescPtr_afterSwap = currDescPtr;
        #endif /*CPU_BE*/

        ownBit = GDMA_DESC_GET_OWN_BIT(currDescPtr_afterSwap);

        if(OWNER_HW == ownBit)
        {
            if(ii == 0)
            {
                return /* it's not error for log */ GT_NO_MORE;
            }

            rc1 = GT_HW_ERROR;
            CPSS_LOG_ERROR_MAC(
                "to cpu : globalRxQueue[%d] Got <OWN> == 'HW' in the middle of the packet",
                globalRxQueue);
            break;
        }

        failure_code = GDMA_DESC_RX_GET_FAILURE_CODE_FIELD(currDescPtr_afterSwap);
        if(failure_code)
        {
            didError++;
        }

        isFirst = GDMA_DESC_GET_FIRST_BIT(currDescPtr_afterSwap);
        if(isFirst && ii != 0)
        {
            /*Got <FIRST> == 1 in the middle of the packet */

            /* we got to the next packet , meaning that last one was not terminated properly */
            /* meaning that the buffers that we already gave the application are not valid   */
            goto retryGetPacketFromStart_lbl;
        }

        if(!isFirst && ii == 0)
        {
            /*Got <FIRST> == 0 in the start of the packet */
            didError++;
        }

        /* get the DMA buff address */
        GDMA_DESC_GET_BUFF_ADDR_48_BITS_FIELD(/*IN*/currDescPtr_afterSwap,/*OUT*/phyAddr);

        #if __WORDSIZE == 64  /* 64 bits compilation */
            phyAddr -= (((GT_UINTPTR)PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh/*0x2*/) << 32);
        #endif

        rc = perDev_cpssOsPhy2Virt(devNum,phyAddr, &buff_virtAddr);
        if (rc != GT_OK)
        {
            /* we can't get over this error ... we will exist the function
               without giving valid descriptor to HW ... this is 'killer' */
            rc1 = rc;
            CPSS_LOG_ERROR_MAC(
                "to cpu : globalRxQueue[%d] : perDev_cpssOsPhy2Virt device[%d] failed to convert buffer phyAddr[%p] to virtual addr ",
                globalRxQueue,
                devNum,
                phyAddr);
            break;
        }

        buffLen = GDMA_DESC_GET_BYTE_COUNT_FIELD_MAC(currDescPtr_afterSwap);
        buffLen ++; /* the HW hold value 1 less than actual */
        buffPtr = (GT_U8*)buff_virtAddr;

        if((*numOfBuffPtr) == IN_numOfBuff)
        {
            rc1 = GT_BAD_SIZE;
            if(errorMessageDone == 0)
            {
                CPSS_LOG_ERROR_MAC(
                    "to cpu : globalRxQueue[%d] Got packet that need more than [%d] buffers that caller gave",
                    globalRxQueue,
                    IN_numOfBuff);
            }
            errorMessageDone = 1;
        }
        else
        {
            buffLenArr[ii]        = buffLen;
            packetBuffsArrPtr[ii] = buffPtr;
            (*numOfBuffPtr)++;
        }

        /* Invalidate data cache for cached buffer */
        if(GT_TRUE == gdmaPerQueue_toCpu_Ptr->buffersInCachedMem)
        {
            /* invalidate data cache */
            cpssExtDrvMgmtCacheInvalidate(CPSS_MGMT_DATA_CACHE_E, buffPtr, buffLen);
        }

        /* get the info from currDescPtr_afterSwap */
        /* before we update the current descriptor with new info in
           gdmaRxDescIsReadyForNewInfo(...) */
        isLast  = GDMA_DESC_GET_LAST_BIT(currDescPtr_afterSwap);

        if(NULL == buffersInfoPtr->freeBuffers_startLinkdedList)
        {
            /* we don't have free buffer to attach the current descriptor
               as the current buffer need to 'go to the application'.
               and without such free buffer , we can not increment the HW about
               the free descriptor
            */
            rc1 = GT_EMPTY;/* !!!GT_EMPTY!!! */
            CPSS_LOG_ERROR_MAC(
                "to cpu : globalRxQueue[%d] have no free buffer to attach to the descriptor (application to free some buffers first)",
                globalRxQueue);
            break;
        }

        /* give the descriptor a new info to point to new buffer */
        rc = gdmaRxDescIsReadyForNewInfo(buffersInfoPtr,
            currDescPtr,
            buffersInfoPtr->freeBuffers_startLinkdedList);
        if(rc != GT_OK)
        {
            rc1 = rc;
            break;
        }

        /* we are done with this descriptor */
        decriptorsInfoPtr->toCpu_currDescriptorIndex++;
        if(decriptorsInfoPtr->toCpu_currDescriptorIndex ==
           decriptorsInfoPtr->toCpu_numOfDescriptors)
        {
            /* running in cyclic mode */
            decriptorsInfoPtr->toCpu_currDescriptorIndex = 0;
        }

        numOfDescriptorsHandled++;
        if(isLast)
        {
            /* NOTE : if didError != 0 then we will restart the loop on descriptors ,
               but we allow to call addDescNumToHw(...) on the descriptors that already finished  */
            break;
        }
    }

    /* we can update the HW about the number of descriptors that we handled
       meaning descriptors that hold new buffer info (dma addr,size)
    */
    rc = addDescNumToHw(devNum,gdmaUnitId,localRxQueue,numOfDescriptorsHandled);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(rc1 != GT_OK)
    {
        /* the 'meaningful' error string was done above by : CPSS_LOG_ERROR_MAC(...) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, /*use rc1 that is before call to
            gdmaRxReuseAllDescriptorsTillNextValidPacket(...) */
            "to cpu : globalRxQueue[%d]",
            globalRxQueue);
    }

    if(ii == decriptorsInfoPtr->toCpu_numOfDescriptors)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
            "to cpu : globalRxQueue[%d] : all the [%d] descriptors in the queue are not enough for the packet !",
            globalRxQueue);
    }

    if(didError)
    {
        /* we already used this value on gdmaRxReuseAllDescriptorsTillNextValidPacket(...)
           so need to reset it */
        numOfDescriptorsHandled = 0;
        goto retryGetPacketFromStart_lbl;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChGdmaNetIfRxCountersGet function
* @endinternal
*
* @brief  GDMA : Get the Rx packet counters for the specific queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalRxQueue            - The Rx queue number through which these packets received.
*
* @param[out] rxCountersPtr           - (pointer to) rx counters on this queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counters are reset on every read.
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      globalRxQueue,
    OUT   CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  dummy;
    GT_U32  gdmaUnitId,localRxQueue;

    /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
    PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
        gdmaUnitId,localRxQueue);

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].stat_byte_counter[0];
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&rxCountersPtr->rxInOctets);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].stat_byte_counter[1];
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&dummy);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].stat_counter[0];
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&rxCountersPtr->rxInPkts);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].stat_counter[1];
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&dummy);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfRxErrorCountersGet function
* @endinternal
*
* @brief  GDMA : Get the Rx Error packet counters for all the queues.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
*
* @param[out] rxErrCountPtr        - (pointer to) The total number of Rx resource errors on
*                                    the device for all the queues.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxErrorCountersGet
(
    IN GT_U8    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC  *rxErrCountPtr
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  dummy;
    GT_U32  gdmaUnitId,localRxQueue;
    GT_U8   globalRxQueue;
    GT_U32  totalNumOfQueues = PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_toCpu;

    for(globalRxQueue = 0 ;globalRxQueue < totalNumOfQueues ;globalRxQueue++)
    {
        /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
        PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
            gdmaUnitId,localRxQueue);

        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].RX_Drop_counter[0];
        rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&rxErrCountPtr->counterArray[globalRxQueue]);
        if(rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[localRxQueue].RX_Drop_counter[1];
        rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    for(/* continue */ ;globalRxQueue < NUM_OF_SDMA_QUEUES ;globalRxQueue++)
    {
        rxErrCountPtr->counterArray[globalRxQueue] = 0;
    }

    return GT_OK;
}


static GT_BOOL  aas_supportDuplicate_gdma_Addr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perGdmaAddrArr[]=
    {
          {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.generalRegs.gdmaBuffersSize                  ), 1}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.generalRegs.gdmaMiscConfig                   ), 1}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.generalRegs.axiTotalOutstandingControlReg    ), 1}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.generalRegs.gdmaSWResetReg                   ), 1}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.generalRegs.gdmaPayloadBufferThreshold       ), 1}
         ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(CNM.GDMA.multicastProfileRegs/*[mc_pr]*/)}
         ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(CNM.GDMA.pdiSlaveRegs/*[pdi]*/          )}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.pdiMasterRegs.pdiMasterControl                 ), 1}
         ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(CNM.GDMA.interruptAndDebugRegs.payloadBufferMaxFillLevel), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perGdmaAddrArr,regAddr);
}
/**
* @internal aas_gdmaUnitDuplicatedMultiPortGroupsGet function
* @endinternal
*
* @brief   GDMA : check if register address is duplicated in AAS device.
*           or the access to register in specific GDMA unit , but the address from unit [0]
*           need to be manipulated the address in the target GDMA unit
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
*
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL aas_gdmaUnitDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr_gdma0,
    INOUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    INOUT GT_U32                  *additionalBaseAddrPtr,
    INOUT GT_U32                  *dupIndexPtr,
    OUT   GT_BOOL                  *dupWasDonePtr,
    OUT   GT_BOOL                  *unitPerGdmaPtr
)
{
    GT_U32 relativeRegAddr;
    PRV_CPSS_DXCH_UNIT_ENT  convertedUnitId;

    if(portGroupId == 0)
    {
        /* the CPSS layer want to access only this GDMA unit */
        return GT_FALSE;
    }

    *unitPerGdmaPtr = GT_TRUE;

    if (portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        if(GT_TRUE == aas_supportDuplicate_gdma_Addr(devNum, regAddr_gdma0))
        {
            /* the CPSS layer want to duplicate to all GDMA units */
            /* duplication will be done by the caller             */
            return GT_TRUE;
        }

        /* not allow duplicate */
        return GT_FALSE;
    }
    /* the CPSS layer want to access the specific 'portGroupId' GDMA unit */

    relativeRegAddr = regAddr_gdma0 & (GDMA_UNIT_SIZE-1);
    convertedUnitId =
        PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS * (portGroupId / PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS) +
        PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E + (portGroupId % PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS) ;

    additionalBaseAddrPtr[(*dupIndexPtr)++] =
        prvCpssDxChHwUnitBaseAddrGet(devNum,convertedUnitId,NULL) +
        relativeRegAddr;/* baseOfUnit + 'relative' offset */

    additionalRegDupPtr->portGroupsArr[0] = portGroupId; /*per GDMA*/

    *dupWasDonePtr = GT_TRUE;

    return GT_TRUE;
}

/**
* @internal prvCpssDxChGdmaNetIfRxQueueEnableSet function
* @endinternal
*
* @brief  GDMA :  Set (enable/disable) the specified traffic class queue for RX
*         packets in CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] globalRxQueue         - The Rx queue number through which these packets received.
*
* @param[in] enable                - GT_TRUE, enable queue
*                                    GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfRxQueueEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalRxQueue,
    IN  GT_BOOL                         enable
)
{
    GT_U32  gdmaUnitId,localRxQueue;

    /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
    PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
        gdmaUnitId,localRxQueue);

    /* enable/disable queue , and for disable also wait for it to come down */
    return gdmaRingEnableDisable(devNum,gdmaUnitId,localRxQueue,
            enable/*enable/disable*/,
            GT_TRUE/*waitForDisable*/);
}

/**
* @internal prvCpssDxChGdmaNetIfRxQueueEnableGet function
* @endinternal
*
* @brief  GDMA :  Get status of the specified traffic class queue for RX
*         packets in CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] globalRxQueue            - The Rx queue number through which these packets received.
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChGdmaNetIfRxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalRxQueue,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_U32  gdmaUnitId,localRxQueue;

    /* convert globalRxQueue to : gdmaUnitId,localRxQueue */
    PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalRxQueue,
        gdmaUnitId,localRxQueue);

    return gdmaRingEnableGet(devNum,gdmaUnitId,localRxQueue,enablePtr);
}

/**
* @internal prvCpssDxChGdmaNetIfPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief  GDMA : Enables/Disable pre-pending a two-byte header to all packets forwarded
*         to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Two-byte header is pre-pended to packets
*                                      forwarded to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      forward to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  fieldVal;

    /*<DSA Slave Port Type Offset>,<CPU Code Offset>*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->CNM.
        GDMA_Dispatcher.dsaFieldOffsets;

    fieldVal = 0xc | 0x13 << 5;
    if(enable == GT_TRUE)
    {
        /* add 2 bytes to <DSA Slave Port Type Offset>,
           and 2 bytes to <CPU Code Offset>
        */
        fieldVal += 0x2 | 0x2 << 5;
    }

    /* set the 10 bits of the 2 fields */
    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,10,fieldVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal gdmaTxInitQueue function
* @endinternal
*
* @brief   This function initializes the Core Tx module, by allocating the cyclic
*         Tx descriptors Array (each with additional eDSA words) -- GDMA
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number to init the Tx unit for.
* @param[in] systemRecoveryProcess    - The system HA mode
* @param[in] gdmaUnitId               - The global GDMA unit Id
* @param[in] localTxQueue             - The local queue number in the GDMA unit
* @param[in] phase2InitInfoPtr        - (pointer to) info from phase 2 init about this queue
*
* @param[out] gdmaPerQueue_fromCpu_Ptr  - (pointer to) the queue info that need to be filled for later use
*
* @retval GT_OK                    - on success.
* @retval GT_INIT_ERROR            - on init error
*/
static GT_STATUS gdmaTxInitQueue
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   gdmaUnitId,
    IN GT_U32                                   localTxQueue,
    IN CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC      *phase2InitInfoPtr,
    OUT PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr
)
{
    GT_STATUS   rc;
    GT_U32  diffToAlign;/* number of bytes that needed for alignment */
    GT_U32  numOfTxDesc;/* number of Tx descriptors */
    GT_U8   *descBlockPtr;
    GT_U32  descBlockSize;
    GT_U32  sizePerDesc;/* size in bytes of descriptor + eDsa  */
    CPSS_TX_BUFF_MALLOC_FUNC buffAndDescMallocFunc;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC *decriptorsInfoPtr;

    /* disable queue if needed , and wait for it to come down */
    rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localTxQueue,
            GT_FALSE/*disable*/,
            GT_TRUE/*waitForDisable*/);

    sizePerDesc = sizeof(PRV_CPSS_GDMA_DESC_STC)+sizeof(PRV_CPSS_GDMA_TX_EDSA_STC);

    /* malloc: for quick sw shadow control to descriptors and buffers in the HW
     * The TxGenerator has pre-allocated buffers in the area associated
     * each to other later on by the gdmaTxGeneratorBuffListInit
     */
    if( CPSS_TX_BUFF_STATIC_ALLOC_E == phase2InitInfoPtr->buffAndDescAllocMethod )
    {
        /* Set the descBlockPtr to point to an aligned start address. */
        ALIGN_PTR(phase2InitInfoPtr->memData.staticAlloc.buffAndDescMemPtr,PRV_CPSS_GDMA_DESC_ALIGN,diffToAlign);
        ALIGN_SIZE_FOR_PTR(phase2InitInfoPtr->memData.staticAlloc.buffAndDescMemSize,diffToAlign);

        /* number of elements of type : 'HW TX descriptor + eDSA buffer' */
        numOfTxDesc = phase2InitInfoPtr->memData.staticAlloc.buffAndDescMemSize /
            sizePerDesc;

        if(numOfTxDesc > phase2InitInfoPtr->numOfTxDesc)
        {
            /* buffAndDescMemPtr is allocated with (sizePerDesc + BUFFER)
             * (for example - TxGenerator case). Use min num of descriptors.
             */
            numOfTxDesc = phase2InitInfoPtr->numOfTxDesc;

            if ((phase2InitInfoPtr->queueMode == CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E) &&
                (numOfTxDesc * (sizeof(PRV_CPSS_GDMA_DESC_STC) + phase2InitInfoPtr->buffSize) >
                 phase2InitInfoPtr->memData.staticAlloc.buffAndDescMemSize))
            {
                /* buffAndDescMemSize has been reduced by allocation alignment,
                 * not enough memory for last buffer.
                 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        descBlockPtr  = (GT_U8*)phase2InitInfoPtr->memData.staticAlloc.buffAndDescMemPtr;
        descBlockSize = sizePerDesc * numOfTxDesc ;
    }
    else /* CPSS_TX_BUFF_DYNAMIC_ALLOC_E */
    {
        numOfTxDesc = phase2InitInfoPtr->numOfTxDesc;
        descBlockSize = sizePerDesc * numOfTxDesc ;

        buffAndDescMallocFunc = phase2InitInfoPtr->memData.dynamicAlloc.buffAndDescMallocFunc;

        descBlockPtr = buffAndDescMallocFunc(descBlockSize, PRV_CPSS_GDMA_DESC_ALIGN);
        if(descBlockPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                "dynamic alloc by 'call back' failed to allocate DMA [%d] bytes",
                descBlockSize);
        }
    }

    /* malloc: for Tx-End interrupt/event notification to appDemo */
    if (gdmaPerQueue_fromCpu_Ptr->txEndFifo_poolId == 0)
    {
        rc = cpssBmPoolCreate(sizeof(PRV_CPSS_SW_TX_FREE_DATA_STC),
                             CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
                             numOfTxDesc,
                             &gdmaPerQueue_fromCpu_Ptr->txEndFifo_poolId);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "Failed to create Tx End FIFO");
        }
    }
    else
    {
        rc = cpssBmPoolReCreate(gdmaPerQueue_fromCpu_Ptr->txEndFifo_poolId);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "Failed to Re-create the Tx End FIFO");
        }
    }

    decriptorsInfoPtr = &gdmaPerQueue_fromCpu_Ptr->fromCpu_descriptors;

    /* bind the virtual address to look at the DMA memory */
    decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress = (void*)descBlockPtr;

    decriptorsInfoPtr->fromCpu_numOfDescriptors      = numOfTxDesc;
    /* all descriptors are free */
    decriptorsInfoPtr->fromCpu_free_numOfDescriptors = numOfTxDesc;
    /* Mem-area begins with SGD, the 'offset_toEdsa' points to data/buffers DMA area */
    decriptorsInfoPtr->fromCpu_offset_toEdsa = numOfTxDesc * sizeof(PRV_CPSS_GDMA_DESC_STC);

    /* allocate extra info that needed per descriptor */
    decriptorsInfoPtr->fromCpu_descExtInfoArr =
        memCalloc(numOfTxDesc *
            sizeof(PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC));
    if( NULL == decriptorsInfoPtr->fromCpu_descExtInfoArr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : fromCpu_descExtInfoArr");
    }

    /* Set the ring to mode 'packets FROM_CPU' */
    rc = gdmaInitRingType(devNum,gdmaUnitId,localTxQueue,
        ((phase2InitInfoPtr->queueMode == CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E) ?
            GDMA_RING_TYPE_FROM_CPU_PKT_GEN_E : GDMA_RING_TYPE_FROM_CPU_E),
        GT_TRUE/*sgdRecycleChain*/,
        GT_TRUE/*sgdExtendMode*/,
        0/*rxBufferByteCnt -- not relevant to this queue*/,
        DROP/*lorMode*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set the DMA address of base of descriptors, according to the virtual addr. Set Ring sgdChainSize */
    rc = setDmaAddrOfDescBaseToHw(devNum,gdmaUnitId,localTxQueue,
        (GT_UINTPTR)decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress,
        numOfTxDesc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (phase2InitInfoPtr->queueMode == CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E)
    {
        /* Do not enable TxGenerator with empty ring. It would be on add-packet */
        return GT_OK;
    }

    /* there is nothing to init in the HW , in term of descriptors , as we not
       have any 'FROM_CPU' packets */

    /* Enable the Tx GDMA Queue */
    rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localTxQueue,
            GT_TRUE/*enable*/,
            GT_TRUE/*dont care for 'enable*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfTxInit function
* @endinternal
*
* @brief   This function initializes the GDMA for FROM_CPU , by allocating the array
*         of FROM_CPU descriptors , and the 'eDSA' buffers
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - The device number to init the Tx unit for.
*    GT_OK on success, or
*    GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChGdmaNetIfTxInit
(
    IN GT_U8                    devNum
)
{
    GT_STATUS   rc;
    GT_U32      totalNumOfQueues;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Arr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr;
    CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC *phase2InitInfoPtr;
    GT_U32      netIfNum;
    GT_U32      gdmaUnitId,gdmaFirstQueue;
    GT_U32      firstQueue,numOfQueues;
    GT_U32      localTxQueue,globalTxQueue;
    /* indexes into 'legacy' DB of :
       moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[legacy_cpuPortIndex][legacy_queueIndex]*/
    GT_U32      legacy_cpuPortIndex,legacy_queueIndex;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC * moduleCfgPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg); /* pointer to the module configure of the PP's database*/
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *allCpuPortsInfoPtr;

    /* get info about all CPU ports that need Rx/Tx queues */
    allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;

    if(allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts == 0)
    {
        /* nothing more to do */
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr)
    {
        /* nothing more to do , already initialized */
        return GT_OK;
    }

    netIfNum = allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts - 1;
    /* calc the number of queues needed for the 'FROM_CPU' */
    totalNumOfQueues = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].firstQueue +
                       allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].numOfQueues;

    PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_fromCpu = totalNumOfQueues;

    gdmaPerQueue_fromCpu_Arr = memCalloc(totalNumOfQueues *
            sizeof(PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC));

    if( NULL == gdmaPerQueue_fromCpu_Arr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : gdmaPerQueue_fromCpu_Arr");
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr = gdmaPerQueue_fromCpu_Arr;

    gdmaPerQueue_fromCpu_Ptr = &gdmaPerQueue_fromCpu_Arr[0];

    /* init the 'from_cpu' part of queues */
    for(netIfNum = 0; netIfNum < allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts ; netIfNum++ )
    {
        firstQueue     = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].firstQueue;
        numOfQueues    = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].numOfQueues;
        gdmaUnitId     = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].gdmaUnitId;
        gdmaFirstQueue = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].gdmaFirstQueue;

        localTxQueue = gdmaFirstQueue;
        for(globalTxQueue = firstQueue;
            globalTxQueue < (firstQueue +  numOfQueues);
            globalTxQueue++,localTxQueue++ , gdmaPerQueue_fromCpu_Ptr++)
        {
            gdmaPerQueue_fromCpu_Ptr->gdmaUnitId   = gdmaUnitId;
            gdmaPerQueue_fromCpu_Ptr->localTxQueue = localTxQueue;

            /* Convert Global[0..31] into legacy [netIf=4][QUEUE_MAX=8] */
            legacy_cpuPortIndex = globalTxQueue / CPSS_MAX_TX_QUEUE_CNS;
            legacy_queueIndex   = globalTxQueue % CPSS_MAX_TX_QUEUE_CNS;

            phase2InitInfoPtr = &moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[legacy_cpuPortIndex][legacy_queueIndex];
            gdmaPerQueue_fromCpu_Ptr->queueMode = phase2InitInfoPtr->queueMode;

            if (phase2InitInfoPtr->numOfTxDesc == 0)
                continue;

            if ((phase2InitInfoPtr->queueMode == CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E) ||
                (phase2InitInfoPtr->queueMode == CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E))
            {
                rc = gdmaTxInitQueue(devNum,
                    gdmaUnitId,localTxQueue,
                    phase2InitInfoPtr,
                    /*OUT*/gdmaPerQueue_fromCpu_Ptr);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    /* TxGenerator is initialized with SGD Descriptors+Buffers common area allocation.
     * The buffers are associated with SGD by local API gdmaTxGeneratorBuffListInit().
     * At runtime an application's packet is copied into an appropriated associated Buffer
     * (on PacketRemove/PacketAdd buffers are swapped between SGDs - association changed).
     * The GdmaTxGenerator is working with "legacy" Sdma pointer/index structures
     * initializad by the same API.
     */
    rc = gdmaTxGeneratorBuffListInit(devNum);

    return rc;
}

/**
* @internal prvCpssDxChGdmaNetIfTxPacketSend function
* @endinternal
*
* @brief   GDMA : This function receives packet buffers & parameters from the different
*         core Tx functions. Prepares them for the transmit operation, and
*         enqueues the prepared descriptors to the PP's tx queues. -- GDMA relate
*         function activates Tx GDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            - The internal packet params to be set into the packet descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
* @param[in] synchronicSend           - sent the packets synchronic (not return until the
*                                      buffers can be free)
*                                      GT_TRUE - send synchronic
*                                           the free of buffers done from this function
*                                      GT_FALSE - send asynchronous (not wait for buffers to be free)
*                                           the free of buffers done from 'Tx ended' ISR context
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_ABORTED               - if command aborted (during shutDown operation)
* @retval GT_HW_ERROR              - when synchronicSend = GT_TRUE and after transmission
*                                       the last descriptor own bit wasn't changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       buffer data can occupied up to the maximum
*                                       number of descriptors defined or queue assigned for
*                                       Packet Generator, unavailable for regular transmission.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*
* @note 1. Each buffer should be at least 8 bytes long.
*       2. first buffer must be at least 24 bytes for tagged packet,
*          20 for untagged packet.
*       3. support ALL DSA tag formats regulate / extended / eDSA
*       4. Packet's length should include 4 bytes for CRC. (legacy from previous devices)
*
*       logic based on dxChNetIfSdmaTxPacketSend(...)
*
*/
GT_STATUS prvCpssDxChGdmaNetIfTxPacketSend
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *buffList[],
    IN GT_U32                            buffLenList[],
    OUT GT_U32                           numOfBufs,
    IN GT_BOOL                           synchronicSend
)
{
    GT_STATUS rc;
    GT_U32      gdmaUnitId;
    GT_U32      localTxQueue,globalTxQueue;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC *decriptorsInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *currDescExtInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *lastDesccExtInfoPtr_ofPacket = NULL;
    PRV_CPSS_GDMA_DESC_STC              *currDescPtr;
    PRV_CPSS_GDMA_DESC_STC              *lastDescPtr_ofPacket = NULL;
    GT_U32      descIndex;
    GT_U32      word0;          /* The first word of the Tx descriptor. */
    GT_UINTPTR  phyAddr; /*DMA     address*/
    GT_UINTPTR  virtAddr;/*virtual address*/
    GT_U32      bufferDmaAddr_low;
    GT_U32      bufferDmaAddr_high;
    GT_U32      phyAddr_2_LSBits;  /*  2 bits to write to word0 of the descriptor */
    GT_U32      phyAddr_32_MSBits; /* 32 bits to write to word1 of the descriptor */
    GT_U32      phyAddr_14_LSBits; /* 14 bits to write to word3 of the descriptor */
    PRV_CPSS_GDMA_TX_EDSA_STC   *edsaInfoPtr; /* pointer to the eDSA bytes of the eDSA (STC format)*/
    GT_U8       *edsaBytesPtr;/* pointer to the eDSA bytes of the eDSA */
    GT_U32      ii;

    GT_U8       numOfAddedBuffs;
    GT_U32      dataOffset = 0;       /* offset from start of packet till after MAC SA,DA + vlan tag (packetParamsPtr->packetIsTagged) */
    GT_U32      dsaTagNumOfBytes = 0; /* number of DSA tag size : 4/8/16 */
    GT_U32      numOfBufs_updated = numOfBufs;
    GT_U32      numOfBufsFinal;
    GT_U32      buffLen;    /* length of current buffer */
    GT_U32      buff0Len;   /* length of 0-th buffer */
    GT_U32      packetLenDec; /* length of packet decrement */

    if(numOfBufs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"numOfBufs is ZERO");
    }

    /* For AAS like Ironman packet should be sent without CRC          */
    /* Assumed that the last 4 bytes of packet passed by caller is CRC */
    packetLenDec = 4; /* size of CRC */
    for (ii = numOfBufs_updated; (ii > 0); ii--)
    {
        if ((buffLenList[ii-1] > packetLenDec))
        {
            break;
        }
        numOfBufs_updated --;
        packetLenDec -= buffLenList[ii-1];
    }

    if (numOfBufs_updated == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "prvCpssDxChGdmaNetIfTxPacketSend :  too small packet");
    }

    buff0Len = buffLenList[0];
    if ((numOfBufs_updated == 1) && (packetLenDec > 0))
    {
        /* no CRC for Ironman */
        buff0Len -= packetLenDec;
    }

    /* Set tx parameters */
    globalTxQueue = pcktParamsPtr->sdmaInfo.txQueue;
    if(globalTxQueue >= PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_fromCpu)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The (global) queue [%d] is >= [%d] max 'FROM_CPU' queues",
            globalTxQueue,
            PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_fromCpu);
    }

    gdmaPerQueue_fromCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[globalTxQueue];
    gdmaUnitId   = gdmaPerQueue_fromCpu_Ptr->gdmaUnitId;
    localTxQueue = gdmaPerQueue_fromCpu_Ptr->localTxQueue;

    decriptorsInfoPtr = &gdmaPerQueue_fromCpu_Ptr->fromCpu_descriptors;

    /* Queue is assigned for Packet Generator */
    if(CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E ==
        gdmaPerQueue_fromCpu_Ptr->queueMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The (global) queue [%d] is associated with 'Packet Generator'",
            globalTxQueue);
    }

    rc = prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet(pcktParamsPtr,
                                                         &dsaTagNumOfBytes,
                                                         &dataOffset);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(buff0Len <= dataOffset/*mac SA+DA+(optional)vlanTag*/)
    {
        if(buff0Len < dataOffset/*mac SA+DA*/)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "too few bytes in first buffer [%d] need to be at least [%d] for %s packet",
                buff0Len, dataOffset,
                pcktParamsPtr->packetIsTagged ? "tagged":"untagged");
        }

        /* the MAC SA+DA already split from the rest of the packet */
        /* so need only single buffer for the DSA */
        numOfAddedBuffs = 1;
    }
    else
    {
        /* Number of buffers added : 1 for the DSA Tag, 1 for split data after
           the Source MAC Address and Destination MAC Address */
        numOfAddedBuffs = 2;
    }

    numOfBufsFinal = numOfBufs_updated + numOfAddedBuffs;

    if(numOfBufsFinal >
        decriptorsInfoPtr->fromCpu_numOfDescriptors)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The Tx packet requires [%d] descriptors , but the queue have only [%d] descriptors (free+used)",
            numOfBufsFinal ,
            decriptorsInfoPtr->fromCpu_numOfDescriptors);
    }

    /* check available buffers in pool */
    if (synchronicSend == GT_FALSE &&
        cpssBmPoolBufFreeCntGet(gdmaPerQueue_fromCpu_Ptr->txEndFifo_poolId) < 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
            "No available buffers in txEndFifo_poolId (need to call cpssDxChNetIfTxBufferQueueGet(...))");
    }

    if(numOfBufsFinal >
        decriptorsInfoPtr->fromCpu_free_numOfDescriptors)
    {
        /* Check NOT have enough free descriptors needed for the packet , but we
           can try to force a check if the device finished with them (and for some
           reason the ISR function did not free them yet)
        */
        if(PRV_CPSS_PP_MAC(devNum)->useIsr == GT_FALSE)
        {
            /* the device was NOT bound to the ISR ,
               and the EVENTs are not handled by the cpss */

            /* return 'Empty' to tell the application , to take care of the
               descriptors , because we have not enough descriptor to do the 'TX'
            */
            /* note : we must not call the
                prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle , because this
                function wants to use the EVENTs mechanism of the CPSS, by
                releasing the descriptors but also sending the 'cookie' info to
                the events queue handling
            */

            /* Application should call function cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
               to release the descriptors ,and get the 'cookie' indication */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY,
                "(useIsr == GT_FALSE) : The Tx packet requires [%d] descriptors , but the queue have only [%d] free descriptors",
                numOfBufsFinal ,
                decriptorsInfoPtr->fromCpu_free_numOfDescriptors);

        }

        /* Call prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle to free buffers.
        Must be under interrupt disabled */
        PRV_CPSS_INT_SCAN_LOCK();
        rc = prvCpssDxChGdmaNetIfTxBuffQueueEvIsrHandle(devNum, (GT_U8)globalTxQueue);
        PRV_CPSS_INT_SCAN_UNLOCK();
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Check again if there are enough descriptors for this packet. */
        if(numOfBufsFinal >
            decriptorsInfoPtr->fromCpu_free_numOfDescriptors)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY,
                "The Tx packet requires [%d] descriptors , but the queue have only [%d] free descriptors",
                numOfBufsFinal ,
                decriptorsInfoPtr->fromCpu_free_numOfDescriptors);
        }
    }

    descIndex   =  decriptorsInfoPtr->fromCpu_currDescriptorIndex;
    currDescPtr = &decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress[descIndex];
    currDescExtInfoPtr = &decriptorsInfoPtr->fromCpu_descExtInfoArr[descIndex];

    for(ii = 0 ; ii < numOfBufsFinal ;
        ii++ , descIndex++, currDescPtr++, currDescExtInfoPtr++)
    {
        if(descIndex ==
           decriptorsInfoPtr->fromCpu_numOfDescriptors)
        {
            /* wrap around */
            descIndex = 0;
            currDescExtInfoPtr = &decriptorsInfoPtr->fromCpu_descExtInfoArr[descIndex];
            currDescPtr        = decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress;
        }

        /* info in the CPU memory : save it for ALL descriptors of the packet
           not only for the first descriptor */
        if(synchronicSend == GT_FALSE)
        {
            currDescExtInfoPtr->userData  = pcktParamsPtr->cookie;
            currDescExtInfoPtr->evReqHndl = pcktParamsPtr->sdmaInfo.evReqHndl;
        }
        else
        {
            /* info not relevant to sync sent , so ignore the info from the caller */
            currDescExtInfoPtr->userData  = NULL;
            currDescExtInfoPtr->evReqHndl = 0;
        }
        currDescExtInfoPtr->wasFreeByIsr = GT_FALSE;

        /* calculate the virtAddr , buffLen of current part of the packet */
        if(ii == 0)
        {
            /* get DMA addr of virtual addr of first buffer */
            virtAddr = (GT_UINTPTR)buffList[0];

            buffLen  = PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS;/*12*//*only the MacDa + MacSa bytes */
        }
        else
        if(ii == 1) /* DSA tag */
        {
            /* the descriptor hold place for DSA 16 bytes , with fixed offset from the descriptor */
            virtAddr = decriptorsInfoPtr->fromCpu_offset_toEdsa + (GT_UINTPTR)(currDescPtr);
            buffLen  = dsaTagNumOfBytes;

            edsaInfoPtr = (PRV_CPSS_GDMA_TX_EDSA_STC*)virtAddr;
            edsaBytesPtr = &edsaInfoPtr->edsa_bytes[0];

            /* Build the DSA Tag info into the 'short buffer' of the descriptor */
            rc = cpssDxChNetIfDsaTagBuild(devNum,
                                             &pcktParamsPtr->dsaParam,
                                             edsaBytesPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            if(numOfAddedBuffs == 1)
            {
                /* first buffer not needed to split for adding the eDSA*/
                virtAddr = (GT_UINTPTR)buffList[ii-1];
                buffLen  = buffLenList[ii-1];
            }
            else
            {
                /* first buffer needed to split for adding the eDSA*/
                if(ii == 2)
                {
                    virtAddr = (GT_UINTPTR)buffList[0] + (GT_UINTPTR)dataOffset;
                    buffLen  = buff0Len - dataOffset;
                }
                else /*(ii is 3,4...) */
                {
                    virtAddr = (GT_UINTPTR)buffList[ii-2];
                    buffLen  = buffLenList[ii-2];
                }
            }
        }

        rc = cpssOsVirt2Phy(virtAddr, &phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr);

        PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(phyAddr,
            bufferDmaAddr_low,
            bufferDmaAddr_high);

        bufferDmaAddr_high += PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh;

        PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(
            /* IN */
            bufferDmaAddr_low,
            bufferDmaAddr_high,

            /* OUT */
            phyAddr_2_LSBits,
            phyAddr_32_MSBits,
            phyAddr_14_LSBits);

        word0 = OWNER_HW | /* <OWN> = GDMA (HW) */
                BIT_5 | /* valid bit (not exists in 'TO_CPU') */
                ((buffLen - 1) & 0xFFFF) << 14 |
                phyAddr_2_LSBits << 30;

        if(ii == 0)
        {
            word0 |= BIT_6;/* <F> bit (first bit) */
        }

        if(ii == (numOfBufsFinal - 1))
        {
            word0 |= BIT_7;/* <L> bit (last bit) */
            lastDescPtr_ofPacket = currDescPtr;
            lastDesccExtInfoPtr_ofPacket = currDescExtInfoPtr;
        }

        currDescPtr->word0 = CPSS_32BIT_LE(word0);
        currDescPtr->word1 = CPSS_32BIT_LE(phyAddr_32_MSBits);
        currDescPtr->word2 = 0;/* !!! not in use !!! hold only reserved fields */
        currDescPtr->word3 = CPSS_32BIT_LE(phyAddr_14_LSBits);
    }

    GT_SYNC;

    /* protect modification of fromCpu_free_numOfDescriptors to avoid
     * race condition with prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle()
     * running in interrupt scan thread */
    PRV_CPSS_INT_SCAN_LOCK();
    decriptorsInfoPtr->fromCpu_currDescriptorIndex    = descIndex;
    decriptorsInfoPtr->fromCpu_free_numOfDescriptors -= numOfBufsFinal;
    PRV_CPSS_INT_SCAN_UNLOCK();

    /* we can update the HW about the number of waiting descriptors */
    /* add 'number of descriptors' that HW can start use (regardless to <OWN> bit) */
    rc = addDescNumToHw(devNum,gdmaUnitId,localTxQueue,numOfBufsFinal);
    if(rc != GT_OK)
    {
        goto exitCleanly_lbl;
    }

    if(synchronicSend == GT_TRUE)
    {
        GT_U32 loopIndex;
        GT_U32 doSleep;
        GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */

        if(lastDescPtr_ofPacket == NULL)
        {
            CPSS_LOG_ERROR_MAC("wrong management of 'last descriptor for the packet' ");
            rc = GT_BAD_STATE;
            goto exitCleanly_lbl;
        }

        if(0 == CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
        {
            /* DMA procedure may take a long time for Jumbo packets.
              E.g. the TX of 10240 Byte packet may take 411 micro seconds for PEX Gen 1.1.
              PEX Gen 1.1 haz 250 MBytes per second maximal throughput. The TX of
              10240 bytes minimal time will take:
               (10240 + 16 (eDSA size) + 16 (descriptor size)) / 250 M = 41.088 microseconds
              The ARM v7 CPU with 1.3 Giga Core Clock and DDR3-1333 performs
              approximately 15 millions per second loops of OWN Bit check algorithm.
              It's required 41.088 * 15 = 613 loops for ARM CPU and maximal PEX 1.1 throughput.
              Choose approx 500 times more loops to be on safe side for high end CPUs - 312000.
              The "numOfBufs + numOfAddedBuffs"  is 3 or more.
              Use constant 104000 to get 312000 loop count.*/

            /* PCIe gen3 is X3 faster than gen1 , gen4 is X2 faster than gen3
               so we can use 1/2 the value of 104000 , but keep it to be on the
               safe side for high end CPUs */
            loopIndex = numOfBufsFinal * 104000;
            doSleep = 0;
        }
        else
        {
            /* ASIM on single image mode should also allow WM more time to handle
               the Tx descriptor from the CPU , before declaring fail */
            doSleep = 1;
            loopIndex = numOfBufsFinal * 500;
        }

        if(cpssDeviceRunCheck_onEmulator())
        {
            doSleep = 1;
            loopIndex = numOfBufsFinal * 500;
        }

        /* allow another task to process it if ready , without 1 millisecond penalty */
        /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
        sleepTime = 0;

        tryMore_lbl:

#ifndef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            sleepTime +=10;
        }
#endif /*!ASIC_SIMULATION*/

        /* Wait until PP sent the packet. Or HW error if while block
                   run more than loopIndex times */
        while (loopIndex && (GDMA_DESC_GET_OWN_BIT(lastDescPtr_ofPacket) == OWNER_HW))
        {
            if(doSleep)
            {
                /* do some sleep allow the simulation process the packet */
                cpssOsTimerWkAfter(sleepTime);
            }
            loopIndex--;
        }

        if(loopIndex == 0)
        {
            if(doSleep)
            {
                if(sleepTime == 0)/* the TX was not completed ? we need to allow more retries with 'sleep (1)'*/
                {
                    loopIndex = numOfBufsFinal * 500;
                    sleepTime = 1;
                    goto tryMore_lbl;
                }
            }

            CPSS_LOG_ERROR_MAC(
                "CPU send packet : The synchronic send failed to get ownership on the descriptor (timeout passed in which the device not released it)");

            rc = GT_HW_ERROR;
            goto exitCleanly_lbl;
        }
    }

exitCleanly_lbl:
    if(synchronicSend == GT_TRUE)
    {
        if(lastDesccExtInfoPtr_ofPacket)
        {
            PRV_CPSS_INT_SCAN_LOCK();
            if(lastDesccExtInfoPtr_ofPacket->wasFreeByIsr == GT_FALSE)
            {
                /* undo the '-=' that done ~100 lines up */
                decriptorsInfoPtr->fromCpu_free_numOfDescriptors += numOfBufsFinal;
            }
            lastDesccExtInfoPtr_ofPacket->wasFreeByIsr = GT_FALSE;/*just clean it ... not really needed */
            PRV_CPSS_INT_SCAN_UNLOCK();
        }
        else
        {
            CPSS_LOG_ERROR_MAC(
                "The 'clean up' must be set with valid lastDesccExtInfoPtr_ofPacket info");
            /* keep the orig rc if already got error */
            if(rc == GT_OK)
            {
                rc = GT_BAD_STATE;
            }
        }
    }

    return rc;
}

/* notify application about a new 'userData' that relate to Tx buffers in the queue ,
    that application can free

   NOTE: the function is called under 'ISR LOCK/UNLOCK'
*/
static GT_STATUS gdmaFromCpuNotifyAppAboutFreeTxBuffers
(
    IN GT_U8                devNum,
    IN GT_U8                globalTxQueue,
    IN CPSS_BM_POOL_ID      txEndFifo_poolId,
    IN PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *currDescExtInfoPtr
)
{
    PRV_CPSS_SW_TX_FREE_DATA_STC    *txFreeDataPtr;
    PRV_CPSS_DRV_EVENT_HNDL_STC     *evHndlPtr;
    PRV_CPSS_TX_BUF_QUEUE_FIFO_STC  *fifoPtr;

    evHndlPtr   = (PRV_CPSS_DRV_EVENT_HNDL_STC*)currDescExtInfoPtr->evReqHndl;

    txFreeDataPtr = cpssBmPoolBufGet(txEndFifo_poolId);
    if(txFreeDataPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
            "failed to get buffer from txEndFifo_poolId , queue[%d]",
            globalTxQueue);
    }

    /* save the Tx End data in FIFO node */
    txFreeDataPtr->devNum   = devNum;
    txFreeDataPtr->queueNum = globalTxQueue;
    txFreeDataPtr->userData = currDescExtInfoPtr->userData;
    txFreeDataPtr->nextPtr  = NULL;

    /* insert the new Tx end node into FIFO */
    if (evHndlPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "evHndlPtr=NULL\n");
    }
    fifoPtr = (PRV_CPSS_TX_BUF_QUEUE_FIFO_STC*)evHndlPtr->extDataPtr;

    if (NULL == fifoPtr->tailPtr)
    {
        fifoPtr->tailPtr = txFreeDataPtr;
        fifoPtr->headPtr = txFreeDataPtr;

        /* notify the user process ,or        */
        /* call the application's CB function */
        /* only for first packet in fifo.     */
        prvCpssDrvEvReqNotify(evHndlPtr);
    }
    else
    {
        fifoPtr->tailPtr->nextPtr = txFreeDataPtr;
        fifoPtr->tailPtr = txFreeDataPtr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfTxBuffQueueEvIsrHandle function
* @endinternal
*
* @brief   GDMA : This routine frees all transmitted packets descriptors.
*          In addition, all user relevant data in Tx End FIFO. -- GDMA relate.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number the packet was transmitted from
* @param[in] globalTxQueue         - The Tx queue number that need to free descriptors.
*
* @retval GT_OK on success, or
*
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the CPSS does not handle the ISR for the device
*                                       so function must not be called
*                                       GT_FAIL otherwise.
*
* @note Invoked by ISR routine !!
*       the function is called internally in the CPSS from the driver as a call
*       back , see bind in function hwPpPhase1Part1(...)
*
*       may also be called also from non ISR context , but the function need to
*       be called under 'ISR LOCK/UNLOCK'
*
*   logic taken from : prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle
*
*/
GT_STATUS prvCpssDxChGdmaNetIfTxBuffQueueEvIsrHandle
(
    IN GT_U8                devNum,
    IN GT_U8                globalTxQueue
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC *decriptorsInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *currDescExtInfoPtr;
    PRV_CPSS_GDMA_DESC_STC              *currDescPtr;
    GT_U32  descIndex;
    GT_U32  ii;
    GT_U32  word0;          /* The first word of the Tx descriptor. */
    GT_U32  numOfUsedDesc;  /* number of descriptors that CPSS gave HW to use , and maybe can be freed */
    GT_U32  startOfPacketExpected;/* the descriptor to handle should hold <F> = 1 --> 'start of packet' */
    GT_U32  numOfFreedDesc_currPacket; /* number of descriptors that we need to free for current packet */

    gdmaPerQueue_fromCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[globalTxQueue];

    decriptorsInfoPtr = &gdmaPerQueue_fromCpu_Ptr->fromCpu_descriptors;

    numOfUsedDesc = decriptorsInfoPtr->fromCpu_numOfDescriptors -
                    decriptorsInfoPtr->fromCpu_free_numOfDescriptors;

    if(numOfUsedDesc == 0)
    {
        /* there is nothing to free so function must not be called */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "function should not be called when there are no 'used descriptor' on Tx queue[%d] ",
            globalTxQueue);
    }

    if(numOfUsedDesc > decriptorsInfoPtr->fromCpu_currDescriptorIndex)
    {
        /* backwards wrap around needed */
        descIndex   =  decriptorsInfoPtr->fromCpu_numOfDescriptors +
                       decriptorsInfoPtr->fromCpu_currDescriptorIndex -
                       numOfUsedDesc;
    }
    else
    {
        descIndex   =  decriptorsInfoPtr->fromCpu_currDescriptorIndex -
                       numOfUsedDesc;
    }
    currDescPtr = &decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress[descIndex];

    numOfFreedDesc_currPacket = 0;
    startOfPacketExpected = 1; /* first descriptor expected to be with <F> == 1*/

    for(ii = 0 ; ii < numOfUsedDesc ;
        ii++ , descIndex++, currDescPtr++)
    {
        if(descIndex ==
           decriptorsInfoPtr->fromCpu_numOfDescriptors)
        {
            /* wrap around */
            descIndex = 0;
            currDescPtr        = decriptorsInfoPtr->fromCpu_descQueueStartVirtualAddress;
        }

        word0 = CPSS_32BIT_LE(currDescPtr->word0);

        if((word0 & BIT_0) == OWNER_HW)
        {
            /* the descriptor (first/middle/last) is still by the HW so stop
               the iterations */
            return GT_OK;
        }

        if(startOfPacketExpected && (0 == (word0 & BIT_6)))
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR),
                "expected start of packet (<F>=1) but not get it , queue[%d]",
                globalTxQueue);
        }

        startOfPacketExpected = 0;

        numOfFreedDesc_currPacket++;

        /* check Last bit */
        if(0 == (word0 & BIT_7))
        {
            /* we are in the middle of the packet ... so continue to look for the
               end of the packet */
            continue;
        }

        /* Last bit indication --> end of packet --> save info and notify application
           about the buffers that are freed */

        /* we keep the info in all descriptors of the packet .
           not only in first descriptor , so we can use the 'last descriptor'
           of this packet */
        currDescExtInfoPtr = &decriptorsInfoPtr->fromCpu_descExtInfoArr[descIndex];

        if(currDescExtInfoPtr->evReqHndl)
        {
            /* save info and notify application about the buffers that are freed */
            rc = gdmaFromCpuNotifyAppAboutFreeTxBuffers(devNum,globalTxQueue,
                gdmaPerQueue_fromCpu_Ptr->txEndFifo_poolId,
                currDescExtInfoPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* this descriptor NOT hold 'event info' as it may belong to 'sync' sending
              so ignore it */
        }

        /* update the number of descriptors that we can consider free */
        /* only after we get 'full packet' (got <L> = 1 )*/
        decriptorsInfoPtr->fromCpu_free_numOfDescriptors += numOfFreedDesc_currPacket;
        currDescExtInfoPtr->wasFreeByIsr = GT_TRUE;

        if(decriptorsInfoPtr->fromCpu_free_numOfDescriptors >
           decriptorsInfoPtr->fromCpu_numOfDescriptors)
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "Bad management on fromCpu_free_numOfDescriptors[%d] > fromCpu_numOfDescriptors[%d] , queue[%d]",
                decriptorsInfoPtr->fromCpu_free_numOfDescriptors,
                decriptorsInfoPtr->fromCpu_numOfDescriptors,
                globalTxQueue);
        }

        /* current descriptor is 'last' so next need to be 'first' */
        startOfPacketExpected = 1;

        /* get ready for next time we will free the descriptors of full packet */
        numOfFreedDesc_currPacket = 0;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfTxQueueEnableSet function
* @endinternal
*
* @brief  GDMA :  Set (enable/disable) the specified traffic class queue for TX
*         packets FROM CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] globalTxQueue         - The Tx queue number through which these packets received.
*
* @param[in] enable                - GT_TRUE, enable queue
*                                    GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfTxQueueEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalTxQueue,
    IN  GT_BOOL                         enable
)
{
    GT_U32  gdmaUnitId,localTxQueue;

    /* convert globalTxQueue to : gdmaUnitId,localTxQueue */
    PRV_CPSS_GDMA_CONVERT_TX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalTxQueue,
        gdmaUnitId,localTxQueue);

    /* enable/disable queue , and for disable also wait for it to come down */
    return gdmaRingEnableDisable(devNum,gdmaUnitId,localTxQueue,
            enable/*enable/disable*/,
            GT_TRUE/*waitForDisable*/);
}

/**
* @internal prvCpssDxChGdmaNetIfTxQueueEnableGet function
* @endinternal
*
* @brief  GDMA :  Get status of the specified traffic class queue for TX
*         packets FROM CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] globalTxQueue            - The Tx queue number through which these packets received.
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChGdmaNetIfTxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalTxQueue,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_U32  gdmaUnitId,localTxQueue;

    /* convert globalTxQueue to : gdmaUnitId,localTxQueue */
    PRV_CPSS_GDMA_CONVERT_TX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum,globalTxQueue,
        gdmaUnitId,localTxQueue);

    return gdmaRingEnableGet(devNum,gdmaUnitId,localTxQueue,enablePtr);
}

/**
* @internal prvCpssDxChGdmaNetIfNumQueuesGet function
* @endinternal
*
* @brief  GDMA : the function return the number of queue for FROM_CPU,TO_CPU
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
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
GT_STATUS prvCpssDxChGdmaNetIfNumQueuesGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *toCpu_numOfQueuesPtr,
    OUT GT_U32                      *fromCpu_numOfQueuesPtr
)
{
    GT_U32  netIfNum;
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *allCpuPortsInfoPtr;

    /* get info about all CPU ports that need Rx/Tx queues */
    allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;

    if(toCpu_numOfQueuesPtr)
    {
        if(allCpuPortsInfoPtr->toCpuInfo.numOfValidPorts == 0)
        {
            /* nothing more to do */
            *toCpu_numOfQueuesPtr = 0;
            return GT_OK;
        }

        netIfNum = allCpuPortsInfoPtr->toCpuInfo.numOfValidPorts - 1;
        /* calc the number of queues needed for the 'TO_CPU' */
        *toCpu_numOfQueuesPtr =
            allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].firstQueue +
            allCpuPortsInfoPtr->toCpuInfo.portsInfo[netIfNum].numOfQueues;
    }

    if(fromCpu_numOfQueuesPtr)
    {
        /* get info about all CPU ports that need Rx/Tx queues */
        allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;

        if(allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts == 0)
        {
            /* nothing more to do */
            *fromCpu_numOfQueuesPtr = 0;
            return GT_OK;
        }

        netIfNum = allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts - 1;
        /* calc the number of queues needed for the 'FROM_CPU' */
        *fromCpu_numOfQueuesPtr =
            allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].firstQueue +
            allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].numOfQueues;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaNetIfEventCpuPortConvert function
* @endinternal
*
* @brief   GDMA : Function checks and gets valid mapping for port per event type
*          that relate to TO_CPU/FROM_CPU traffic
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in]  uniEvent                - the unified event , that relate to TO_CPU/FROM_CPU traffic.
* @param[in] evConvertType            - event convert type
* @param[in] IN_queue                 - queue to convert
*
* @param[out] OUT_queuePtr            - (pointer to) converted queue, according to convert event type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfEventCpuPortConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvent,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          IN_queue,
    OUT GT_U32                          *OUT_queuePtr
)
{
    devNum   = devNum;
    uniEvent = uniEvent;

    /* by default : map 1:1 */
    *OUT_queuePtr = IN_queue;

    switch (evConvertType)
    {
        case PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E:
/*      convert SW global SDMA queue 0..127 to HW global GDMA queue 0..127
 *      the 'SW' global SDMA queue is the 'SDMA queue index' used in the CPSS
 *          APIs of 'networkIf'
 *      the 'HW' global SDMA queue is the 'SDMA queue index' defined per MG 0..15.
 *      the Application may choose to work with non-consecutive MGs that will cause
 *          none 1:1 map between 'SW' and 'HW' queue index.
 *      used when mask/unmask event by application need to be converted bit in
 *          interrupt register in HW configuration.
 */
            /* by default : map 1:1 */ /* NOT IMPLEMENTED YET */
            *OUT_queuePtr = IN_queue;

            break;
        case PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E:
/*      convert HW global SDMA queue 0..127 to SW global SDMA queue 0..127
 *      see explanation above about 'SW' and 'HW' global SDMA queue meaning.
 *      used when interrupt from HW need to be converted to event of application.
 */
            /* by default : map 1:1 */ /* NOT IMPLEMENTED YET */
            *OUT_queuePtr = IN_queue;

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(evConvertType);
    }

    return GT_OK;
}

typedef struct{
    GT_U32  dpUnit;
    PRV_CPSS_DXCH_NETIF_GDMA_BIND_CPU_PORT_INFO_STC   cpuPortInfo;
}PRV_CPSS_DXCH_NETIF_GDMA_CPU_PORT_STC;

static const PRV_CPSS_DXCH_NETIF_GDMA_CPU_PORT_STC AAS_GDMA_cpuPorts[] =
{
    /* from first tile */
    {1  , {0 , 64 , 0  ,0}},  /*DP1 : TO_CPU:GDMA1,2  */
    {2  , {0 ,  0 , 0  ,32}}, /*DP2 : FROM_CPU:GDMA3  */
    /* from second tile */
    {5 , {64 , 64 , 0   ,0}},  /*DP5 : TO_CPU:GDMA5,6  */
    {6 , {0 ,  0  , 32  ,32}}, /*DP6 : FROM_CPU:GDMA7  */
};
/**
* @internal prvCpssDxChNetIfGdmaInitAfterPortMapping function
* @endinternal
*
* @brief   GDMA : set the information about all the network interfaces according
*           to 'port mapping' params
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssDxChNetIfGdmaInitAfterPortMapping
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       globalDma,
    IN GT_U32                       netIfNum
)
{
    GT_STATUS   rc;
    const PRV_CPSS_DXCH_NETIF_GDMA_CPU_PORT_STC   *devInfoPtr;
    PRV_CPSS_DXCH_NETIF_GDMA_BIND_CPU_PORT_INFO_STC cpuPortInfo;
    GT_U32  numElem;
    GT_U32  ii,dpUnit,localDmaNum;

    switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AAS_E:
            devInfoPtr = &AAS_GDMA_cpuPorts[0];
            numElem = sizeof(AAS_GDMA_cpuPorts)/sizeof(AAS_GDMA_cpuPorts[0]);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"no implemented");
    }

    if(netIfNum >= numElem)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The device not supports more than [%d] GDMA CPU ports",
            numElem);
    }

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,globalDma,/*OUT*/&dpUnit,&localDmaNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    for(ii = 0 ; ii < numElem ; ii++,devInfoPtr++)
    {
        if(devInfoPtr->dpUnit != dpUnit)
        {
            continue;
        }

        /* copy the info to remove the 'const' */
        cpuPortInfo = devInfoPtr->cpuPortInfo;

        return prvCpssDxChNetIfGdmaCpuPortBindToQueuesSet(devNum,portNum,
            &cpuPortInfo);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
        "not found globalDma[%d] as applicable for GDMA unit",
        globalDma);
}
/* SIP7 : Checks if the given desc. is not valid.    */
#define SIP7_AU_MSG_IS_NOT_VALID(auMsgPtr)                        \
    (((auMsgPtr)->fdbMsgWords[0] == 0) && ((auMsgPtr)->fdbMsgWords[1] == 0) &&   \
     ((auMsgPtr)->fdbMsgWords[2] == 0) && ((auMsgPtr)->fdbMsgWords[3] == 0))

/* SIP7 : Invalidates 4 words in the given AU desc.             */
#define SIP7_AU_MSG_RESET_MAC(auMsgPtr,startIndex) \
    (auMsgPtr)->fdbMsgWords[0+startIndex] = 0;     \
    (auMsgPtr)->fdbMsgWords[1+startIndex] = 0;     \
    (auMsgPtr)->fdbMsgWords[2+startIndex] = 0;     \
    (auMsgPtr)->fdbMsgWords[3+startIndex] = 0

/* SIP7 swap 4 words of message :
    startIndex - is the word to start swap in fdbMsgWords[]
*/
#define SIP7_AU_MSG_SWAP_4_WORDS_MAC(origAuMsgPtr,swappedAuMsg,startIndex)        \
    swappedAuMsg.fdbMsgWords[0+startIndex] = CPSS_32BIT_LE((origAuMsgPtr)->fdbMsgWords[0+startIndex]); \
    swappedAuMsg.fdbMsgWords[1+startIndex] = CPSS_32BIT_LE((origAuMsgPtr)->fdbMsgWords[1+startIndex]); \
    swappedAuMsg.fdbMsgWords[2+startIndex] = CPSS_32BIT_LE((origAuMsgPtr)->fdbMsgWords[2+startIndex]); \
    swappedAuMsg.fdbMsgWords[3+startIndex] = CPSS_32BIT_LE((origAuMsgPtr)->fdbMsgWords[3+startIndex])

/* SIP7 swap 4 words of CNC message :
    startIndex - is the word to start swap in fdbMsgWords[]
*/
#define SIP7_CNC_SWAP_4_WORDS_MAC(origCncMsgPtr,swappedCncMsg)        \
    swappedCncMsg.cncMsgWords[0] = CPSS_32BIT_LE((origCncMsgPtr)->cncMsgWords[0]); \
    swappedCncMsg.cncMsgWords[1] = CPSS_32BIT_LE((origCncMsgPtr)->cncMsgWords[1]); \
    swappedCncMsg.cncMsgWords[2] = CPSS_32BIT_LE((origCncMsgPtr)->cncMsgWords[2]); \
    swappedCncMsg.cncMsgWords[3] = CPSS_32BIT_LE((origCncMsgPtr)->cncMsgWords[3])

#define SIP7_TRACE_8_WORDS(dataPtr) \
    cpssOsPrintf("info in HEX [%8.8x][%8.8x][%8.8x][%8.8x] \n" , (dataPtr)[ 0],(dataPtr)[ 1],(dataPtr)[ 2],(dataPtr)[ 3]); \
    cpssOsPrintf("info in HEX [%8.8x][%8.8x][%8.8x][%8.8x] \n" , (dataPtr)[ 4],(dataPtr)[ 5],(dataPtr)[ 6],(dataPtr)[ 7])


/**
* @struct PRV_CPSS_CNC_MSG_SIP7_STC
 *
 * @brief Define the raw words format of the CNC messages sent
 * to the CPU. 4 words (128 bits).
 *
 * APPLICABLE DEVICES : AAS.
*/
typedef struct{
    /** @brief : SIP7 : CNC message data words 0..3
    */
    GT_U32  cncMsgWords[4];
} PRV_CPSS_CNC_MSG_SIP7_STC;

/**
* @internal gdmaClientMsgInitRing function
* @endinternal
*
* @brief   This function initializes the GDMA for the client : buffers and SGDs
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the AUQ/FUQ/CNC/IPE unit for.
* @param[in] blockPtr              - The DMA block that was allocated for this client.
* @param[in] blockSizeInBytes      - The number of bytes allocated under blockPtr
* @param[in] maxNumMsgPerQueue     - The max number of messages that the each client should hold
*                                    this will decide how many queues to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
static GT_STATUS gdmaClientMsgInitRing_perTile(
    IN GT_U8                    devNum,
    IN GT_U8                    *blockPtr,
    IN GT_U32                   maxBufSize,
    IN GT_U32                   fullSize_buffersNodesNumElements,
    IN GT_U32                   alignmentNeeded,
    IN GT_U32                   lastBufferSize,
    INOUT PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr,
    IN CPSS_SYSTEM_RECOVERY_PROCESS_ENT systemRecoveryProcess
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_GDMA_DESC_STC  currDescPtr_afterSwap;
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr;
    GT_UINTPTR  phyAddr;/*DMA address*/
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */
    GT_U32  gdmaUnitId,localRing;
    GT_U8   *sgdStartPtr;
    GT_U8   *buffersStartPtr;
    GT_U32  ii;

    buffersInfoPtr    = &clientMsgRingInfoPtr->msg_buffers;
    decriptorsInfoPtr = &clientMsgRingInfoPtr->msg_descriptors;

    buffersInfoPtr->headerOffset              = 0;
    buffersInfoPtr->buffersNodesNumElements   = fullSize_buffersNodesNumElements;
    buffersInfoPtr->perQueue_bufferNumOfBytes = maxBufSize - alignmentNeeded;
    if(lastBufferSize >= alignmentNeeded)
    {
        /* the last smaller buffer */
        buffersInfoPtr->buffersNodesNumElements += 1;
        buffersInfoPtr->perQueue_lastBufferNumOfBytes = lastBufferSize - alignmentNeeded;
    }
    else
    {
        buffersInfoPtr->perQueue_lastBufferNumOfBytes = 0;
    }

    /* start the memory , to be used by the SGDs for the MSG */
    sgdStartPtr     = blockPtr;
    /* next in the memory , to be used by the buffers for the MSG */
    buffersStartPtr = blockPtr + buffersInfoPtr->buffersNodesNumElements * PRV_CPSS_GDMA_DESC_ALIGN;

    gdmaUnitId = clientMsgRingInfoPtr->gdmaUnitId;
    localRing  = clientMsgRingInfoPtr->localRing;

    /* disable ring , and wait for it to come down */
    rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localRing,
            GT_FALSE/*disable*/,
            GT_TRUE/*waitForDisable*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* enable the PDI slave interface for this GDMA as PDI interface in the unit */
    rc = gdmaPdiSlaveEnable(devNum,gdmaUnitId,clientMsgRingInfoPtr->pdiSlaveInterface);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set the ring to mode 'MSG TO_CPU' */
    rc = gdmaInitRingType(devNum,gdmaUnitId,localRing,
        GDMA_RING_TYPE_TO_CPU_MSG_MODE_E,
        GT_TRUE/*sgdRecycleChain*/,
        GT_TRUE/*sgdExtendMode*/,
        0/*rxBufferByteCnt*/, /* last buffer size may differ from the others */
        WAIT/*lorMode*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress =
        (void*)sgdStartPtr;
    decriptorsInfoPtr->toCpu_numOfDescriptors = buffersInfoPtr->buffersNodesNumElements;

    /* array of offset from the address of descriptor to the address of it's buffer
        needed to save time to not do converting phy2Virt of the buffer

        the array allocated in runtime according to number of descriptors/buffers
    */
    clientMsgRingInfoPtr->msg_extInfoArr =
        memCalloc(buffersInfoPtr->buffersNodesNumElements *
            sizeof(PRV_CPSS_DXCH_GDMA_DB_PER_MSG_DESCRIPTORS_INFO_STC));
    if( NULL == clientMsgRingInfoPtr->msg_extInfoArr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : msg_extInfoArr");
    }

    decriptorsInfoPtr->toCpu_currDescriptorIndex = 0;

    /*
        allocate the buffersNodesArray[] according to buffersNodesNumElements and

        init the link list of :
            freeBuffers_startLinkdedList
            freeBuffers_lastLinkdedList
        with DMA info
    */
    rc = rxInitBuffersLinkList(devNum,
        buffersInfoPtr,
        NULL,               /* for dynamic mode */
        buffersStartPtr);   /* for static mode  */
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
        get from the link list of :
            freeBuffers_startLinkdedList
            freeBuffers_lastLinkdedList
        nodes to give the array of descriptors

        those nodes will mode to the link list of :
            freeNodes_startLinkdedList
            freeNodes_lastLinkdedList

        update the HW about those descriptors that are ready !!!
    */
    rc = rxInitDescriptorArr(devNum,
        gdmaUnitId,
        localRing,
        decriptorsInfoPtr,
        buffersInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < decriptorsInfoPtr->toCpu_numOfDescriptors; ii++)
    {
        currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[ii];

        currDescPtr_afterSwap.word0 = CPSS_32BIT_LE(currDescPtr->word0);
        currDescPtr_afterSwap.word1 = CPSS_32BIT_LE(currDescPtr->word1);
        currDescPtr_afterSwap.word2 = CPSS_32BIT_LE(currDescPtr->word2); /*word not in use */
        currDescPtr_afterSwap.word3 = CPSS_32BIT_LE(currDescPtr->word3);
        /* get the DMA buff address */
        GDMA_DESC_GET_BUFF_ADDR_48_BITS_FIELD(/*IN*/(&currDescPtr_afterSwap),/*OUT*/phyAddr);

        #if __WORDSIZE == 64  /* 64 bits compilation */
            phyAddr -= (((GT_UINTPTR)PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh/*0x2*/) << 32);
        #endif

        rc = perDev_cpssOsPhy2Virt(devNum,phyAddr, &buff_virtAddr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "perDev_cpssOsPhy2Virt device[%d] failed to convert buffer phyAddr[%p] to virtual addr ",
                devNum,
                phyAddr);
        }
        clientMsgRingInfoPtr->msg_extInfoArr[ii].offsetFromDescToBuff =
            buff_virtAddr - (GT_UINTPTR)currDescPtr;

        if(buffersInfoPtr->perQueue_lastBufferNumOfBytes &&
           ii == (decriptorsInfoPtr->toCpu_numOfDescriptors - 1))
        {
            clientMsgRingInfoPtr->msg_extInfoArr[ii].numBytesInBuffer =
                buffersInfoPtr->perQueue_lastBufferNumOfBytes;
        }
        else
        {
            clientMsgRingInfoPtr->msg_extInfoArr[ii].numBytesInBuffer =
                buffersInfoPtr->perQueue_bufferNumOfBytes;
        }
    }


    /* if rxsdma init is going under recovery process as PP is disabled to write CPU memory, enable Rx SDMA queues just  */
    /* after PP would be able to access CPU memory(in catch up stage)- otherwise syncronization may be lost between      */
    /* PP and descriptors                                                                                                */
    if (systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /* Enable the Rx SDMA Queue */
        rc = gdmaRingEnableDisable(devNum,gdmaUnitId,localRing,
                GT_TRUE/*enable*/,
                GT_TRUE/*dont care for 'enable*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal gdmaClientMsgInitRing function
* @endinternal
*
* @brief   This function initializes the GDMA for the client : buffers and SGDs
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the AUQ/FUQ/CNC/IPE unit for.
* @param[in] blockPtr              - The DMA block that was allocated for this client.
* @param[in] blockSizeInBytes      - The number of bytes allocated under blockPtr
* @param[in] maxNumMsgPerQueue     - The max number of messages that the each client should hold
*                                    this will decide how many queues to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
static GT_STATUS gdmaClientMsgInitRing(
    IN GT_U8                    devNum,
    IN GT_U8                    *blockPtr,
    IN GT_U32                   blockSizeInBytes,
    IN GT_U32                   maxNumMsgPerQueue,
    IN PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *tile0_clientMsgRingInfoPtr,
    IN PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *tile1_clientMsgRingInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  alignmentNeeded = PRV_CPSS_GDMA_DESC_ALIGN;/* for the SGD and for the buffers */
    GT_U32  fullSize_buffersNodesNumElements; /* number of 'full size' needed buffers */
    GT_U32  lastBufferSize; /* the size of the last smaller  buffer */
    GT_U32  msgSizeInBytes = tile0_clientMsgRingInfoPtr->msgSizeInBytes;
    GT_U32  perTile_blockSizeInBytes;
    GT_U32  diffToAlign;/* number of bytes that needed for alignment */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /*holds system recovery info*/
    GT_U32  maxBufSize;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(blockPtr == NULL || blockSizeInBytes == 0)
    {
        /* not need allocation ... OK */
        return GT_OK;
    }

    ALIGN_PTR(blockPtr,alignmentNeeded,diffToAlign);
    ALIGN_SIZE_FOR_PTR(blockSizeInBytes,diffToAlign);

    /* each buffer needs : 1 SGD + 1 buffer of size 'msgSizeInBytes'*/
    maxBufSize = maxNumMsgPerQueue * msgSizeInBytes;
    if(maxBufSize > BIT_16)
    {
        /* the buffer size limited to 16 bits , with use of 'value+1' */
        maxBufSize = BIT_16;
    }
    maxBufSize += alignmentNeeded;

    /* number of 'full size' buffers */
    fullSize_buffersNodesNumElements = blockSizeInBytes / maxBufSize;/* round down */

    /* calc the number per tile */
    fullSize_buffersNodesNumElements /= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    perTile_blockSizeInBytes = blockSizeInBytes/ PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    lastBufferSize = perTile_blockSizeInBytes - fullSize_buffersNodesNumElements * maxBufSize;
    if(lastBufferSize < (maxBufSize/5)/*20%*/ &&
       fullSize_buffersNodesNumElements > 0)
    {
        /* don't use leftover less than 20% of the other buffers */
        lastBufferSize = 0;
    }

    /* the GDMA need to have buffers >= 64 bytes */
    if(lastBufferSize == 0 && fullSize_buffersNodesNumElements == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "the given buffer size not allow any allocation of the MSG queue");
    }

    /* so we have fullSize_buffersNodesNumElements with size maxBufSize
       and single element with size lastBufferSize */


    rc = gdmaClientMsgInitRing_perTile(devNum,
        blockPtr,
        maxBufSize,
        fullSize_buffersNodesNumElements,
        alignmentNeeded,
        lastBufferSize,
        tile0_clientMsgRingInfoPtr, /* info into tile 0 */
        tempSystemRecovery_Info.systemRecoveryProcess);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1)
    {
        rc = gdmaClientMsgInitRing_perTile(devNum,
            blockPtr+perTile_blockSizeInBytes, /* start after tile 0 */
            maxBufSize,
            fullSize_buffersNodesNumElements,
            alignmentNeeded,
            lastBufferSize,
            tile1_clientMsgRingInfoPtr, /* info into tile 1 */
            tempSystemRecovery_Info.systemRecoveryProcess);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal gdmaClientMsgInit function
* @endinternal
*
* @brief   This function initializes the GDMA for clients of MSG : AUQ/FUQ/CNC/IPE , by
*           allocating the array of queues per client , implemented by , per client we hold :
*          1. SGD - to state the size of the queue (number of bytes , aligned on 'needed' bytes)
*          2. MSG buffer - the buffer that 'act' as queue , that get the messages
*           so several queues are 'link list' for the client
*
*           NOTE:
*           1. ALL tiles will get the same size of those queues .
*              example :
*                  tile 0 : 1000 , 1000 , 1000 , 300
*                  tile 1 : 1000 , 1000 , 1000 , 300
*           2. the SGDs (for all tiles) that needed for those queues , are
*              'stolen' from the start of the memory
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the AUQ/FUQ/CNC/IPE unit for.
* @param[in] clientMsgType         - The client type
* @param[in] subClientIndex        - The sub client index
*                                    Relevant to CNC and IPE.
* @param[in] blockPtr              - The DMA block that was allocated for this client.
* @param[in] blockSizeInBytes      - The number of bytes allocated under blockPtr
* @param[in] maxNumMsgPerQueue     - The max number of messages that the each client should hold
*                                    this will decide how many queues to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
static GT_STATUS gdmaClientMsgInit(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_GDMA_CLIENT_TYPE_MSG_ENT clientMsgType,
    IN GT_U32                   subClientIndex,
    IN GT_U8                    *blockPtr,
    IN GT_U32                   blockSizeInBytes,
    IN GT_U32                   maxNumMsgPerQueue
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    GT_U32 msgSizeInBytes;
    GT_U32 gdmaUnitId;
    GT_U32 jumpIndexBeteenTiles = 1;
    GT_U32 ringInfoIndex = 0;

    if(blockPtr && blockSizeInBytes)
    {
        /* the AUQ,FUQ need ZERO's to start with */
        /* the CNC not , but lets do it anyway ... */
        /* the IPE ... TBD , but lets do it anyway ... */
        cpssOsMemSet(blockPtr,0,blockSizeInBytes);
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    switch(clientMsgType)
    {
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E :
            /* actual messages are 24B or 56B but alignment is 64
                the 'NA' is 56B and other messages are 24B
            */
            msgSizeInBytes = 64;
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForAuq;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ringInfoIndex];

            clientMsgRingInfoPtr->localRing     = 0;
            clientMsgRingInfoPtr->pdiSlaveInterface = 5;

            rc = prvCpssHwPpSetRegField(devNum,
                 PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig2,
                 4,5,clientMsgRingInfoPtr->localRing);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E :
            /* messages are only 24B ... but the boundary is the same as for
              the AUQ (from GDMA point of view) --> so needed to be 64 (and not 32) */
            msgSizeInBytes = 64;
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForFuq;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ringInfoIndex];

            clientMsgRingInfoPtr->localRing     = 1;
            clientMsgRingInfoPtr->pdiSlaveInterface = 5;

            rc = prvCpssHwPpSetRegField(devNum,
                 PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig2,
                 9,5,clientMsgRingInfoPtr->localRing);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E     :
            if(subClientIndex >= SIP7_MAX_CNC_UNITS_PER_PORT_GROUP)
            {
                /* the logic below will decide how many queues needed */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                      "for CNC : subClientIndex[%d] must be < %d",
                      SIP7_MAX_CNC_UNITS_PER_PORT_GROUP);
            }

            ringInfoIndex = subClientIndex;

            jumpIndexBeteenTiles = SIP7_MAX_CNC_UNITS_PER_PORT_GROUP;/* 4 CNC units per tile */
            /* messages are 16B , also the alignment is 16 */
            msgSizeInBytes = 16;
            gdmaUnitId  = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForCnc;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[ringInfoIndex];

            clientMsgRingInfoPtr->localRing     = 2 + ringInfoIndex;/* option for 2..5 */
            clientMsgRingInfoPtr->pdiSlaveInterface = 3-ringInfoIndex;/*3..0 in reverse order*/

            rc = prvCpssHwPpSetRegField(devNum,
                 PRV_DXCH_REG1_UNIT_CNC_MAC(devNum, subClientIndex).globalRegs.CNCGlobalConfigReg,
                 26,5,clientMsgRingInfoPtr->localRing);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_4_E   :
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_8_E   :
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_16_E  :
            if(subClientIndex >= 4)
            {
                /* the logic below will decide how many queues needed */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                      "for IPE : subClientIndex[%d] must be < 4");
            }

            ringInfoIndex = subClientIndex;

            jumpIndexBeteenTiles = 4;/* 4 IPE interfaces per tile */
            msgSizeInBytes = 16;/* messages are 4/8/16B but alignment is 16 for all */
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForIpe;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaIpeInfo[ringInfoIndex];
            clientMsgRingInfoPtr->localRing     = 6 + subClientIndex;/* option for 6..9 */
            clientMsgRingInfoPtr->pdiSlaveInterface = 4;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
                  "not have register to config the 'localRing'");
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientMsgType);
    }

    /* we got the proper clientMsgRingInfoPtr , lets allocate SGDs and buffers for it */
    clientMsgRingInfoPtr->clientMsgType = clientMsgType;
    clientMsgRingInfoPtr->gdmaUnitId    = gdmaUnitId;
    clientMsgRingInfoPtr->msgSizeInBytes = msgSizeInBytes;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1)
    {
        clientMsgRingInfoPtr[jumpIndexBeteenTiles].clientMsgType  = clientMsgType;
        clientMsgRingInfoPtr[jumpIndexBeteenTiles].gdmaUnitId     = 4 + gdmaUnitId;
        clientMsgRingInfoPtr[jumpIndexBeteenTiles].msgSizeInBytes = msgSizeInBytes;
    }

    return gdmaClientMsgInitRing(devNum,blockPtr,
            blockSizeInBytes,maxNumMsgPerQueue,
            clientMsgRingInfoPtr,/*tile0*/
            &clientMsgRingInfoPtr[jumpIndexBeteenTiles]/*tile 1*/);
}

/**
* @internal prvCpssDxChGdmaFdbMsgInit function
* @endinternal
*
* @brief   This function initializes the GDMA for FDB AUQ/FUQ , by allocating the array
*         of AUQs,FUQs , implemented by , per AUQ/FUQ we hold :
*          1. SGD - to state the size of the AUQ/FUQ (number of bytes , aligned on 'needed' bytes)
*          2. MSG buffer - the buffer that 'act' as AUQ/FUQ , that get the messages
*
*           NOTE:
*           1. for AUQ/FUQ : all AUQ messages have additional 8 bytes 'padded ZERO' to align to 32 bytes
*
*           2. ALL tiles will get the same size of those queues .
*              example :
*                  tile 0 : 1000 , 1000 , 1000 , 300
*                  tile 1 : 1000 , 1000 , 1000 , 300
*           3. the SGDs (for all tiles) that needed for those queues , are
*              'stolen' from the start of the memory
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the FDB AUQ/FUQ unit for.
* @param[in] maxNum64BMsgPerAuq    - The max number of 64B messages that the each AUQ should hold
*                                    this will decide how many AUQs to hold for the device
* @param[in] maxNum32BMsgPerFuq    - The max number of 32B messages that the each FUQ should hold
*                                    this will decide how many FUQs to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaFdbMsgInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   maxNum64BMsgPerAuq,
    IN GT_U32                   maxNum32BMsgPerFuq
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC * moduleCfgPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg); /* pointer to the module configure of the PP's database*/

    if(moduleCfgPtr->fuqUseSeparate != CPSS_PP_PHASE2_UPLOAD_QUEUE_SEPARATE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "(phase2 param) fuqUseSeparate must be 'CPSS_PP_PHASE2_UPLOAD_QUEUE_SEPARATE_E' (GT_TRUE)");
    }
    if(moduleCfgPtr->useDoubleAuq == GT_TRUE)
    {
        /* the logic below will decide how many queues needed */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "(phase2 param) useDoubleAuq must be 'GT_FALSE' , the CPSS will decide how many 'sub-queues' will be for be AUQ");
    }

    /* init the AUQ */
    rc = gdmaClientMsgInit(devNum,
            PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E,
            0,/*subClientIndex - not relevant to AUQ*/
            moduleCfgPtr->auCfg.auDescBlock,
            moduleCfgPtr->auCfg.auDescBlockSize,
            maxNum64BMsgPerAuq);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* init the FUQ */
    rc = gdmaClientMsgInit(devNum,
            PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E,
            0,/*subClientIndex - not relevant to FUQ*/
            moduleCfgPtr->fuCfg.fuDescBlock,
            moduleCfgPtr->fuCfg.fuDescBlockSize,
            maxNum32BMsgPerFuq);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

extern GT_U32 cpssSimLogIsOpen(void);

/* make sure that HW is done with the SGD , so we can update it */
/* we got into race condition (in WM) before adding this check during FDB upload */
static GT_STATUS    waitForSgdMsgOwnByCpu(
    IN GT_U8                    devNum,
    IN PRV_CPSS_GDMA_DESC_STC  *currDescPtr
)
{
    GT_U32 loopIndex;
    GT_U32 doSleep;
    GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */

    devNum = devNum;/* needed for ASIC_SIMULATION */

    loopIndex = 100;
    if(0 == CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
    {
        doSleep = 0;
    }
    else
    {
        /* ASIM on single image mode should also allow WM more time to handle
           the Tx descriptor from the CPU , before declaring fail */
        doSleep = 1;

        if(cpssSimLogIsOpen())
        {
            /* if the WM log is enabled for CNC dumps ... it will take more time */
            loopIndex *= 100;
        }
    }

    /* allow another task to process it if ready , without 1 millisecond penalty */
    /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
    sleepTime = 0;

    tryMore_lbl:

    if(cpssDeviceRunCheck_onEmulator())
    {
        doSleep = 1;
        sleepTime +=5;
    }

    while(loopIndex && (CPSS_32BIT_LE(currDescPtr->word0) & 0x1) == OWNER_HW)  /*GDMA_DESC_GET_OWN_BIT*/
    {
        if(doSleep)
        {
            /* do some sleep allow the simulation process the packet */
            cpssOsTimerWkAfter(sleepTime);
        }
        loopIndex--;
    }

    if(loopIndex == 0 && doSleep && sleepTime == 0)
    {
        loopIndex = 500;/* give more time for CNC upload when cs_ver is pushing hard */
        sleepTime = 1;

        goto tryMore_lbl;
    }

    if(loopIndex == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
            "The SGD was not set with CPU_OWN (not released by the HW) \n");
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChGdmaFdbAuFuMsgBlockGet function
* @endinternal
*
* @brief   GDMA : The function return a block (array) of AU / FU messages ,
*           the max number of elements defined by the caller
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number from which AU / FU are taken
* @param[in] portGroupId              - the portGroupId - for multi-port-groups support
* @param[in] queueType                - type of message queue to retrieve entries from
* @param[in] numOfAuFuPtr             - (pointer to) max number of AU / FU messages to receive
*
* @param[out] numOfAuFuPtr            - (pointer to)actual number of AU / FU messages that
*                                      were received
* @param[out] auFuMessagesPtr         - array that holds received AU / FU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*                                      !!! NOT used when NULL !!!
* @param[out] sip7HwAuMessagesPtr    - sip7 : array that holds received AU messages in HW format
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*                                      !!! NOT used when NULL !!!
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                    waiting messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_STATE             - on bad state
*/
GT_STATUS prvCpssDxChGdmaFdbAuFuMsgBlockGet
(
    IN     GT_U8                       devNum,
    IN     GT_U32                      portGroupId,
    IN     MESSAGE_QUEUE_ENT           queueType,
    INOUT  GT_U32                      *numOfAuFuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auFuMessagesPtr,
    OUT    PRV_CPSS_AU_FU_MSG_SIP7_STC   *sip7HwAuMessagesPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_GDMA_DESC_STC                                  *currDescPtr;
    PRV_CPSS_AU_FU_MSG_SIP7_STC                                localMsg,*msgPtr;
    GT_UINTPTR offsetFromDescToBuff; /*offset from the address of descriptor to the address of it's buffer*/
    GT_U32     currOffsetInBuffer;   /*offset from the start of the buffer*/
    GT_U32     *buffMemPtr;/* pointer to current place in the buffer , to get message from */
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */
    GT_U32 gdmaUnitId,localRing;
    GT_U32 jumpIndexBeteenTiles = 1;
    GT_U32 ringInfoIndex = 0;
    GT_U32 tileId;
    GT_U32 sgdIndex;
    GT_U32 msgMaxCnt;   /* max num of messages that the caller ask for */
    GT_U32 ii;
    GT_BOOL needToSkipCurrMsg;
    GT_U32 hwEntryFormat;
    GT_U32 actualMsgSizeInBytes;
    GT_U32 next_numOfAuFu;/* the number of numOfAuFu for the recursive call */
    GT_U32 desc_word0;

    if((*numOfAuFuPtr) == 0)
    {
        return GT_OK;
    }

    tileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    switch(queueType)
    {
        case MESSAGE_QUEUE_PRIMARY_AUQ_E:
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ringInfoIndex + tileId*jumpIndexBeteenTiles];
            if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E)
            {
                /* check that the AUQ initialized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                    "The AUQ for tile[%d] was not initialized",
                    tileId);
            }
            break;

        case MESSAGE_QUEUE_PRIMARY_FUQ_E:
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ringInfoIndex + tileId*jumpIndexBeteenTiles];
            if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E)
            {
                /* check that the AUQ initialized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                    "The FUQ for tile[%d] was not initialized",
                    tileId);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(queueType);/*should not happen*/
    }

    decriptorsInfoPtr = &clientMsgRingInfoPtr->msg_descriptors;

    sgdIndex   =  decriptorsInfoPtr->toCpu_currDescriptorIndex;
    currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[sgdIndex];
    offsetFromDescToBuff = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].offsetFromDescToBuff;
    /* this is start of buffer */
    buff_virtAddr = (GT_UINTPTR)currDescPtr + offsetFromDescToBuff;
    /* this is how deep into this buffer we already gone */
    currOffsetInBuffer = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer;

    if(currOffsetInBuffer + clientMsgRingInfoPtr->msgSizeInBytes >
        clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer)
    {
        /* we should have started new SGD */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "we should have started new SGD");
    }

    /* the pointer to start of 'next message to handle'  */
    buffMemPtr = (GT_U32 *)(buff_virtAddr + currOffsetInBuffer);

    msgMaxCnt = *numOfAuFuPtr;
    *numOfAuFuPtr = 0;

    /* read number of AU descriptors */
    for (ii = 0 ; ii < msgMaxCnt; ii++)
    {
        /* point descPtr to the current AU descriptor in the queue */
        msgPtr = (PRV_CPSS_AU_FU_MSG_SIP7_STC*)buffMemPtr;

        if(SIP7_AU_MSG_IS_NOT_VALID(msgPtr))
        {
            *numOfAuFuPtr = ii;
            /* no more messages on the  */
            return /* it's not error for log */ GT_NO_MORE;
        }

        if(sip7HwAuMessagesPtr != NULL)
        {
            /* Read the Memory of AU again(Do not use register data) after the validity check
             * which ensures AU memory holds data, after DMA complete */
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), sip7HwAuMessagesPtr[ii],0);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), sip7HwAuMessagesPtr[ii],4);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), sip7HwAuMessagesPtr[ii],8);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), sip7HwAuMessagesPtr[ii],12);

            hwEntryFormat = sip7HwAuMessagesPtr[ii].fdbMsgWords[0] & 0xF;

            if(clientMsgRingInfoPtr->debug_and_statistics.enableTraceMsg)
            {
                cpssOsPrintf("case 1 : Got new FDB message type [%d] ,numOfMsgTreated[%d] \n" , hwEntryFormat ,
                    clientMsgRingInfoPtr->debug_and_statistics.numOfMsgTreated);
                SIP7_TRACE_8_WORDS(&sip7HwAuMessagesPtr[ii].fdbMsgWords[0]);
                if(hwEntryFormat == HW_SIP7_FDB_UPDATE_MSG_NA_E)
                {
                    SIP7_TRACE_8_WORDS(&sip7HwAuMessagesPtr[ii].fdbMsgWords[8]);
                }
            }
        }
        else
        {
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), localMsg,0);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), localMsg,4);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), localMsg,8);
            SIP7_AU_MSG_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr), localMsg,12);

            hwEntryFormat = localMsg.fdbMsgWords[0] & 0xF;

            if(clientMsgRingInfoPtr->debug_and_statistics.enableTraceMsg)
            {
                cpssOsPrintf("case 2 : Got new FDB message type [%d] ,numOfMsgTreated[%d] \n" , hwEntryFormat ,
                    clientMsgRingInfoPtr->debug_and_statistics.numOfMsgTreated);
                SIP7_TRACE_8_WORDS(&localMsg.fdbMsgWords[0]);
                if(hwEntryFormat == HW_SIP7_FDB_UPDATE_MSG_NA_E)
                {
                    SIP7_TRACE_8_WORDS(&localMsg.fdbMsgWords[8]);
                }
            }

            /* let the code in cpssDxChBrgFdbAu.c to parse the actual format */
            rc = internal_auDesc2UpdMsg(devNum, portGroupId ,&localMsg.fdbMsgWords[0] ,
                &auFuMessagesPtr[ii],&needToSkipCurrMsg);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(needToSkipCurrMsg)
            {
                /* the message need to be filtered for some reason , so we need to re-use this index */
                ii--;
                if(clientMsgRingInfoPtr->debug_and_statistics.enableTraceMsg)
                {
                    cpssOsPrintf("warning : the message was skipped , numOfMsgSkipped[%d]\n",
                        clientMsgRingInfoPtr->debug_and_statistics.numOfMsgSkipped);
                }
                clientMsgRingInfoPtr->debug_and_statistics.numOfMsgSkipped++;
            }
        }

        /* count this message */
        clientMsgRingInfoPtr->debug_and_statistics.numOfMsgTreated++;

        /* we need to step to state to move to next message - regardless to error */
        if(hwEntryFormat == HW_SIP7_FDB_UPDATE_MSG_NA_E)
        {
            actualMsgSizeInBytes = 64;/* 56 + 8 PADDED with ZEROs*/
        }
        else
        {
            actualMsgSizeInBytes = 32;/* 24 + 8 PADDED with ZEROs*/
        }

        SIP7_AU_MSG_RESET_MAC(msgPtr,0);
        if(actualMsgSizeInBytes > 32)
        {
            SIP7_AU_MSG_RESET_MAC(msgPtr,8);
        }

        clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer +=
            actualMsgSizeInBytes;
        currOffsetInBuffer += actualMsgSizeInBytes;

        /* jump to next message in the buffer */
        buffMemPtr += (actualMsgSizeInBytes / 4);

#ifdef CIDER_ON_EMULATOR_OR_GM /* behave on (CC-) that also still not fixed in CC+ */
        if(currOffsetInBuffer + clientMsgRingInfoPtr->msgSizeInBytes >=
#else
        if(currOffsetInBuffer + clientMsgRingInfoPtr->msgSizeInBytes >
#endif
            clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer)
        {
            /* make sure that HW is done with the SGD , so we can update it */
            /* we got into race condition (in WM) before adding this check during FDB upload */
            rc = waitForSgdMsgOwnByCpu(devNum,currDescPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            /*********************************/
            /* we should jump to next SGD that is already connected to the HW */
            /*********************************/
            clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex++;
            if(clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex >=
               clientMsgRingInfoPtr->msg_descriptors.toCpu_numOfDescriptors)
            {
                /* running in cyclic mode */
                clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex = 0;
            }

            /* rewind to start of queue */
            clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer = 0;

            /*********************************/
            /* also need to return current descriptor back to the linked list */
            /*********************************/

            desc_word0 = CPSS_32BIT_LE(currDescPtr->word0);
            /* the HW actually interested in the 'byte count' of the descriptor */
            /* we need to renew its value */
            desc_word0 &= 0xC0000000;/* clear 30 bits */
            /* update the OWN and the buffer size */
            desc_word0 |= OWNER_HW | /* <OWN> = GDMA (HW) */
                (clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer - 1) << 14;

            currDescPtr->word0 = CPSS_32BIT_LE(desc_word0);

            GT_SYNC;

            clientMsgRingInfoPtr->debug_and_statistics.numOfDescTreated ++;

            gdmaUnitId = clientMsgRingInfoPtr->gdmaUnitId;
            localRing  = clientMsgRingInfoPtr->localRing;

            /* we can update the HW about the number of waiting descriptors */
            /* add 'single descriptor' that HW can start use (regardless to <OWN> bit) */
            rc = addDescNumToHw(devNum,gdmaUnitId,localRing,1);
            if(rc != GT_OK)
            {
                return rc;
            }

            /*********************************/
            /* do recursive call to handle more messages from the next buffer */
            /*********************************/

            ii++;

            next_numOfAuFu = msgMaxCnt - ii;

            if(next_numOfAuFu)
            {
                if(sip7HwAuMessagesPtr != NULL)
                {
                    rc = prvCpssDxChGdmaFdbAuFuMsgBlockGet(devNum,portGroupId,queueType,
                        &next_numOfAuFu,
                        NULL,
                        &sip7HwAuMessagesPtr[ii]);
                }
                else
                {
                    rc = prvCpssDxChGdmaFdbAuFuMsgBlockGet(devNum,portGroupId,queueType,
                        &next_numOfAuFu,
                        &auFuMessagesPtr[ii],
                        NULL);
                }

                *numOfAuFuPtr = ii + next_numOfAuFu;

                return rc;
            }
            else /*next_numOfAuFu==0*/
            {
                *numOfAuFuPtr = ii;
                /* already served all places in the array of the caller */
                return GT_OK;
            }
        }
    }/*ii*/

    *numOfAuFuPtr = ii;

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaFdbAuqFuqMessagesNumberGet function
* @endinternal
*
* @brief  GDMA : The function scan the AU/FU queues and returns the number of
*         AU/FU messages in the queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the portGroupId - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ.
*
* @param[out] numOfMsgPtr             - (pointer to) number of AU messages in the specified queue.
* @param[out] endOfQueueReachedPtr    - (pointer to) GT_TRUE: The queue reached to the end.
*                                      GT_FALSE: else
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note : none
*
*/
GT_STATUS prvCpssDxChGdmaFdbAuqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfMsgPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_GDMA_DESC_STC                                  *currDescPtr;
    PRV_CPSS_AU_FU_MSG_SIP7_STC                                localMsg,*msgPtr;
    GT_UINTPTR offsetFromDescToBuff; /*offset from the address of descriptor to the address of it's buffer*/
    GT_U32     currOffsetInBuffer;   /*offset from the start of the buffer*/
    GT_U32     *buffMemPtr;/* pointer to current place in the buffer , to get message from */
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */
    GT_U32 jumpIndexBeteenTiles = 1;
    GT_U32 ringInfoIndex = 0;
    GT_U32 tileId;
    GT_U32 queueId,sgdIndex;
    GT_U32 hwEntryFormat;
    GT_U32 actualMsgSizeInBytes;

    *numOfMsgPtr = 0;
    *endOfQueueReachedPtr = GT_FALSE;

    tileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    switch(queueType)
    {
        case MESSAGE_QUEUE_PRIMARY_AUQ_E:
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ringInfoIndex + tileId*jumpIndexBeteenTiles];
            if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E)
            {
                /* check that the AUQ initialized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                    "The AUQ for tile[%d] was not initialized",
                    tileId);
            }
            break;

        case MESSAGE_QUEUE_PRIMARY_FUQ_E:
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ringInfoIndex + tileId*jumpIndexBeteenTiles];
            if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E)
            {
                /* check that the AUQ initialized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                    "The FUQ for tile[%d] was not initialized",
                    tileId);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(queueType);/*should not happen*/
    }

    decriptorsInfoPtr = &clientMsgRingInfoPtr->msg_descriptors;

    for(queueId = 0; queueId < decriptorsInfoPtr->toCpu_numOfDescriptors ;  queueId++)
    {
        sgdIndex = (decriptorsInfoPtr->toCpu_currDescriptorIndex + queueId) %
            decriptorsInfoPtr->toCpu_numOfDescriptors;

        currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[sgdIndex];
        offsetFromDescToBuff = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].offsetFromDescToBuff;
        /* this is start of buffer */
        buff_virtAddr = (GT_UINTPTR)currDescPtr + offsetFromDescToBuff;
        /* this is how deep into this buffer we already gone */
        currOffsetInBuffer = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer;

        /* the pointer to start of 'next message to handle'  */
        buffMemPtr = (GT_U32 *)(buff_virtAddr + currOffsetInBuffer);


        do /* loop on messages on current queue */
        {
            /* point descPtr to the current AU descriptor in the queue */
            msgPtr = (PRV_CPSS_AU_FU_MSG_SIP7_STC*)buffMemPtr;

            if(SIP7_AU_MSG_IS_NOT_VALID(msgPtr))
            {
                return GT_OK;
            }

            localMsg.fdbMsgWords[0] = CPSS_32BIT_LE(((volatile PRV_CPSS_AU_FU_MSG_SIP7_STC *)msgPtr)->fdbMsgWords[0]);
            hwEntryFormat = localMsg.fdbMsgWords[0] & 0xF;

            if(hwEntryFormat < HW_SIP7_FDB_UPDATE_MSG_NA_E ||
               hwEntryFormat > HW_SIP7_FDB_UPDATE_MSG_HR_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "unexpected HW Entry Format [%d]",
                    hwEntryFormat);
            }

            /* we need to step to state to move to next message - regardless to error */
            if(hwEntryFormat == HW_SIP7_FDB_UPDATE_MSG_NA_E)
            {
                actualMsgSizeInBytes = 64;/* 56 + 8 PADDED with ZEROs*/
            }
            else
            {
                actualMsgSizeInBytes = 32;/* 24 + 8 PADDED with ZEROs*/
            }

            (*numOfMsgPtr)++;

            currOffsetInBuffer += actualMsgSizeInBytes;

            /* jump to next message in the buffer */
            buffMemPtr += (actualMsgSizeInBytes / 4);

            if(currOffsetInBuffer + clientMsgRingInfoPtr->msgSizeInBytes >
                clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer)
            {
                /* this will break the loop on current 'queue' */
                currOffsetInBuffer = 0;
            }
        }while(currOffsetInBuffer);
    }/*queueId*/

    /* we ended all available queues , so the 'end of queue reached' */
    /* otherwise we do return GT_OK on 'non-valid' message */
    *endOfQueueReachedPtr = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaCncMsgInit function
* @endinternal
*
* @brief   This function initializes the GDMA for CNC , by allocating the array
*         of CNCs , implemented by , per CNC unit queue we hold :
*          1. SGD - to state the size of the CNC unit queue (number of bytes , aligned on 'needed' bytes)
*          2. MSG buffer - the buffer that 'act' as CNC unit queue , that get the messages
*
*           NOTE:
*           1. ALL port groups will get the same size of those queues .
*              example :
*                  pipe 0 : CNC unit 0 : 1000 , 1000 , 1000 , 300
*                  pipe 0 : CNC unit 1 :  500 ,  500 ,  500 , 200
*                   ....
*                  pipe 1 : CNC unit 0 : 1000 , 1000 , 1000 , 300
*                  pipe 1 : CNC unit 1 :  500 ,  500 ,  500 , 200
*                   ....
*           3. the SGDs (for all CNC units) that needed for those queues , are
*              'stolen' from the start of the memory
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the CNC unit for.
* @param[in] cncUnitId             - The CNC unit id (local in the port group)
* @param[in] maxNum16BMsgPerCncQueue - The max number of 16B messages that the each CNC unit queue
*                                    should hold , this will decide how many CNC unit queues to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaCncMsgInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   cncUnitId,
    IN CPSS_DMA_QUEUE_CFG_STC   *cncCfgPtr,
    IN GT_U32                   maxNum16BMsgPerCncQueue
)
{
    /* init the CNC unit queues */
    return gdmaClientMsgInit(devNum,
            PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E,
            cncUnitId,
            cncCfgPtr->dmaDescBlock,
            cncCfgPtr->dmaDescBlockSize,
            maxNum16BMsgPerCncQueue);
}

/**
* @internal prvCpssDxChGdmaCncBeforeDumpTrigger function
* @endinternal
*
* @brief  The function check is the CNC dump is allowed , and will add the number
*         of expected CNC counters to 'unreadCncCounters'.
*         The function expected to be called BEFORE the caller trigger the CNC upload in the unit.
*         The function checks that there is no unfinished previous CNC dump, that
*         not allow new trigger to start.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES: AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in] inProcessBlocksBmpArr[]- (array of) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @param[in] unitNum               - The CNC unit (0..3)
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                    continued after queue rewind but yet
*                                    not paused due to queue full and yet not finished.
*                                    The part of queue memory does not contain uploaded counters yet.
*                                    No counters retrieved.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       1. The device support parallel dump operations on all 4 CNC units.
*       2. The caller expected to trigger the CNC upload in the unit
*
*/
GT_STATUS prvCpssDxChGdmaCncBeforeDumpTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      inProcessBlocksBmpArr[/*2*/],
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  GT_U32                      unitNum
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_STC         *cncGlobalInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC    *newNode;
    GT_U32 ringInfoIndex = 0;
    GT_U32 portGroupId;
    GT_CNC_UNITS_BMP     cncUnitsBmp_perPortGroup;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;
    cncGlobalInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo;

    cncUnitsBmp_perPortGroup = 1 << unitNum;
    if(PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits > SIP7_MAX_CNC_UNITS_PER_PORT_GROUP)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "cncUnits[%d] > SIP7_MAX_CNC_UNITS_PER_PORT_GROUP[%d]",
            PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits ,
            SIP7_MAX_CNC_UNITS_PER_PORT_GROUP);
    }

    if(unitNum >= PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "not expecting unitNum[%d] >= cncUnits[%d]",
            unitNum);
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        ringInfoIndex = portGroupId * SIP7_MAX_CNC_UNITS_PER_PORT_GROUP;/*4 CNC units*/

        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[ringInfoIndex + unitNum];

        if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E)
        {
            /* check that the CNC initialized */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                "The CNC upload for unit [%d] for was not initialized (see cpssDxChCncUploadInit(...))",
                unitNum);
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    if(0xFFFF & (inProcessBlocksBmpArr[unitNum>>1] >> (16 * (unitNum & 1))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY,
            "The CNC unit[%d] (blocks[%d..%d]) not ready for new dump as still busy with previous dump(s)",
            unitNum,
            16*unitNum,
            (16*unitNum + 15));
    }

    /*****************************************/
    /* create new node to hold the dump info */
    /*****************************************/
    newNode = memCalloc(sizeof(PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC));
    if( NULL == newNode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "no memory for : newNode");
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        newNode->portGroupInfo[portGroupId].node_unreadCncCounters =
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlockNumEntries;
        newNode->portGroupInfo[portGroupId].node_cncFormat = format;

        newNode->node_cncUnitsBmp |=
            cncUnitsBmp_perPortGroup << (SIP7_MAX_CNC_UNITS_PER_PORT_GROUP * portGroupId);
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    newNode->debug_and_statistics.unitNum = unitNum;

    /***********************************/
    /* add the node to the linked list */
    /***********************************/
    if(cncGlobalInfoPtr->cncDumpListHeadPtr == NULL)
    {
        cncGlobalInfoPtr->cncDumpListHeadPtr = newNode;
        cncGlobalInfoPtr->cncDumpListTailPtr = newNode;
    }
    else
    {
        cncGlobalInfoPtr->cncDumpListTailPtr->nextCncDumpNodePtr = newNode;
        cncGlobalInfoPtr->cncDumpListTailPtr = newNode;
    }

    return GT_OK;
}

#ifdef ASIC_SIMULATION
    #define CNC_MINIMAL_SLEEP_TIME_CNS 50
#else /* HW */
    #define CNC_MINIMAL_SLEEP_TIME_CNS 1
#endif /* HW */



/**
* @internal perUnit_CncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of CNC queue full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] cncUnit                  - global cnc unit id (not per port group)
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use DMA memory that set by cpssDxChCncUploadInit.
*
*/
static GT_STATUS perUnit_CncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC *cncDumpNodePtr,
    IN     GT_U32                            cncUnit,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    OUT    CPSS_DXCH_CNC_COUNTER_STC          counterValuesPtr[]
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_GDMA_DESC_STC                                  *currDescPtr;
    PRV_CPSS_CNC_MSG_SIP7_STC                                localMsg,*msgPtr;
    GT_UINTPTR offsetFromDescToBuff; /*offset from the address of descriptor to the address of it's buffer*/
    GT_U32     currOffsetInBuffer;   /*offset from the start of the buffer*/
    GT_U32     *buffMemPtr;/* pointer to current place in the buffer , to get message from */
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */
    GT_U32 gdmaUnitId,localRing;
    GT_U32 sgdIndex;
    GT_U32 msgMaxCnt;   /* max num of messages that the caller ask for */
    GT_U32 ii;
    GT_U32 actualCncSizeInBytes;/* 8 or 16 bytes */
    GT_U32 next_numOfCncs;/* the number of numOfCncs for the recursive call */
    GT_U32 desc_word0;
    CPSS_DXCH_CNC_COUNTER_STC   currCounter;/* current counter value for multi-port-groups support */
    GT_U32  portGroupId = cncUnit / SIP7_MAX_CNC_UNITS_PER_PORT_GROUP;

    if((*numOfCounterValuesPtr) == 0)
    {
        return GT_OK;
    }

    switch(format)
    {
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E     :
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E     :
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E     :
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E     :
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E     :
        case CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E     :
            actualCncSizeInBytes = 8;
            break;
        default:
            actualCncSizeInBytes = 16;
            break;
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;
    clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[cncUnit];

    if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E)
    {
        /* check that the CNC initialized */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
            "The CNC upload for unit [%d] for was not initialized (see cpssDxChCncUploadInit(...))",
            cncUnit % SIP7_MAX_CNC_UNITS_PER_PORT_GROUP);
    }

    decriptorsInfoPtr = &clientMsgRingInfoPtr->msg_descriptors;

    sgdIndex   =  decriptorsInfoPtr->toCpu_currDescriptorIndex;
    currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[sgdIndex];
    offsetFromDescToBuff = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].offsetFromDescToBuff;
    /* this is start of buffer */
    buff_virtAddr = (GT_UINTPTR)currDescPtr + offsetFromDescToBuff;
    /* this is how deep into this buffer we already gone */
    currOffsetInBuffer = clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer;

    if(currOffsetInBuffer + actualCncSizeInBytes/*clientMsgRingInfoPtr->msgSizeInBytes*/ >
        clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer)
    {
        /* we should have started new SGD */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "we should have started new SGD");
    }

    /* the pointer to start of 'next message to handle'  */
    buffMemPtr = (GT_U32 *)(buff_virtAddr + currOffsetInBuffer);

    msgMaxCnt = *numOfCounterValuesPtr;
    *numOfCounterValuesPtr = 0;

    /* read number of AU descriptors */
    for (ii = 0 ; ii < msgMaxCnt; ii++)
    {
        if(cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters == 0)
        {
            return /* it's not error for log */ GT_NO_MORE;
        }



        /* point descPtr to the current AU descriptor in the queue */
        msgPtr = (PRV_CPSS_CNC_MSG_SIP7_STC*)buffMemPtr;

        SIP7_CNC_SWAP_4_WORDS_MAC(((volatile PRV_CPSS_CNC_MSG_SIP7_STC *)msgPtr), localMsg);

        /* parse the counter in the needed format */
        rc = prvCpssDxChCncCounterHwToSw(devNum,format, &localMsg.cncMsgWords[0], &currCounter);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* add values of currCounter into counterValuesPtr[ii] */
        /* to support multi-port-groups */
        rc = prvCpssDxChCncCountersAdd(devNum,format,
            &counterValuesPtr[ii],
            &currCounter);
        if (rc != GT_OK)
        {
            return rc;
        }

        cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters --;

        clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer +=
            actualCncSizeInBytes;
        currOffsetInBuffer += actualCncSizeInBytes;

        /* jump to next message in the buffer */
        buffMemPtr += (actualCncSizeInBytes / 4);

        if(currOffsetInBuffer + actualCncSizeInBytes/*clientMsgRingInfoPtr->msgSizeInBytes*/ >
            clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer)
        {
            /* make sure that HW is done with the SGD , so we can update it */
            /* we got into race condition (in WM) before adding this check during FDB upload */
            rc = waitForSgdMsgOwnByCpu(devNum,currDescPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            /*********************************/
            /* we should jump to next SGD that is already connected to the HW */
            /*********************************/
            clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex++;
            if(clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex >=
               clientMsgRingInfoPtr->msg_descriptors.toCpu_numOfDescriptors)
            {
                /* running in cyclic mode */
                clientMsgRingInfoPtr->msg_descriptors.toCpu_currDescriptorIndex = 0;
            }

            /* rewind to start of queue */
            clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].currOffsetInBuffer = 0;

            /*********************************/
            /* also need to return current descriptor back to the linked list */
            /*********************************/

            desc_word0 = CPSS_32BIT_LE(currDescPtr->word0);
            /* the HW actually interested in the 'byte count' of the descriptor */
            /* we need to renew its value */
            desc_word0 &= 0xC0000000;/* clear 30 bits */
            /* update the OWN and the buffer size */
            desc_word0 |= OWNER_HW | /* <OWN> = GDMA (HW) */
                (clientMsgRingInfoPtr->msg_extInfoArr[sgdIndex].numBytesInBuffer - 1) << 14;

            currDescPtr->word0 = CPSS_32BIT_LE(desc_word0);

            GT_SYNC;

            clientMsgRingInfoPtr->debug_and_statistics.numOfDescTreated ++;

            gdmaUnitId = clientMsgRingInfoPtr->gdmaUnitId;
            localRing  = clientMsgRingInfoPtr->localRing;

            /* we can update the HW about the number of waiting descriptors */
            /* add 'single descriptor' that HW can start use (regardless to <OWN> bit) */
            rc = addDescNumToHw(devNum,gdmaUnitId,localRing,1);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(clientMsgRingInfoPtr->msg_descriptors.toCpu_numOfDescriptors == 1)
            {
                /* we need to allow the HW to spill any extra counters to next buffer
                   as we only now gave the single SGD back to HW */
                cpssOsTimerWkAfter(CNC_MINIMAL_SLEEP_TIME_CNS);
            }

            /*********************************/
            /* do recursive call to handle more messages from the next buffer */
            /*********************************/

            ii++;

            next_numOfCncs = msgMaxCnt - ii;

            if(next_numOfCncs)
            {
                rc = perUnit_CncUploadedBlockGet(devNum,
                    cncDumpNodePtr,cncUnit,format,
                    &next_numOfCncs,
                    &counterValuesPtr[ii]);

                *numOfCounterValuesPtr = ii + next_numOfCncs;

                return rc;
            }
            else /*next_numOfCncs==0*/
            {
                *numOfCounterValuesPtr = ii;
                /* already served all places in the array of the caller */
                return GT_OK;
            }
        }
    }/*ii*/

    *numOfCounterValuesPtr = ii;

    return GT_OK;
}

/* return NULL if node not found */
static PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC * cncDumpNodeGet(
    IN PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC * cncDumpListHeadPtr,
    IN GT_U32   cncUnitsBmp
)
{
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC  *cncDumpNodePtr;

    cncDumpNodePtr = cncDumpListHeadPtr;
    while(cncDumpListHeadPtr)
    {
        if(cncDumpNodePtr->node_cncUnitsBmp & cncUnitsBmp)
        {
            return cncDumpNodePtr;
        }

        cncDumpNodePtr = cncDumpNodePtr->nextCncDumpNodePtr;
    }

    return cncDumpListHeadPtr;
}

/* free the unused cnc dump node */
static void cncDumpNodeRelease(
    IN PRV_CPSS_DXCH_GDMA_DB_CNC_STC      *cncGlobalInfoPtr,
    IN PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC *cncDumpNodePtr
)
{
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC  *tmpNodePtr;
    if(cncGlobalInfoPtr->cncDumpListHeadPtr == cncDumpNodePtr)
    {
        cncGlobalInfoPtr->cncDumpListHeadPtr = cncDumpNodePtr->nextCncDumpNodePtr;
        if(cncGlobalInfoPtr->cncDumpListTailPtr == cncDumpNodePtr)
        {
            cncGlobalInfoPtr->cncDumpListTailPtr = cncDumpNodePtr->nextCncDumpNodePtr;
        }
    }
    else
    {
        tmpNodePtr = cncGlobalInfoPtr->cncDumpListHeadPtr;
        while(tmpNodePtr)
        {
            if(tmpNodePtr->nextCncDumpNodePtr == cncDumpNodePtr)
            {
                tmpNodePtr->nextCncDumpNodePtr = cncDumpNodePtr->nextCncDumpNodePtr;

                if(cncDumpNodePtr == cncGlobalInfoPtr->cncDumpListTailPtr)
                {
                    cncGlobalInfoPtr->cncDumpListTailPtr = tmpNodePtr;
                }
                break;
            }

            tmpNodePtr = tmpNodePtr->nextCncDumpNodePtr;
        }
    }

    cpssOsFree(cncDumpNodePtr);
}

/**
* @internal prvCpssDxChGdmaCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES: AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Getting messages from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use DMA memory that set by cpssDxChCncUploadInit.
*
*/
GT_STATUS prvCpssDxChGdmaCncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         counterValuesPtr[]
)
{
    GT_STATUS   rc; /* return code */
    GT_U32  portGroupId,portGroupId_first;/*the port group Id - support multi-port-groups device */
    GT_U32  portGroupNumOfCounters;/* number of counters requested and actually */
                                   /* retrieved for the current port group. */
    GT_U32  minNumOfCounters;      /* the minimum number of counters read per port */
                                   /* group from all port groups.   */
    GT_STATUS   outOfSyncStatus = GT_OK;/* indication of out of synch between the multi-port groups */
    GT_U32  retryIteration,isFirstIteration;
    GT_U32  cncUnitId;
    GT_CNC_UNITS_BMP     cncUnitsBmp;
    GT_CNC_UNITS_BMP     cncUnitsBmp_perPortGroup;
    GT_U32 ii;
    PRV_CPSS_DXCH_GDMA_INFO_STC        *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_STC      *cncGlobalInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC *cncDumpNodePtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC *cncDumpNodePtr_first;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    cncUnitsBmp_perPortGroup = BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits/*value per port group*/);

    rc = GT_OK;

    minNumOfCounters = *numOfCounterValuesPtr;

    for(ii = 0; ii < minNumOfCounters ; ii++)
    {
        cpssOsMemSet(&counterValuesPtr[ii], 0, sizeof(CPSS_DXCH_CNC_COUNTER_STC));
    }

    retryIteration = 1;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;
    cncGlobalInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo;

    isFirstIteration = 1;
    portGroupId_first = 0;
    cncDumpNodePtr_first = NULL;

    /* do check that we can sum the counters of dump from the needed port groups */
    /* for single port group device , this loop will find no 'GT_BAD_PARAM'      */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        cncUnitsBmp =
            cncUnitsBmp_perPortGroup << (SIP7_MAX_CNC_UNITS_PER_PORT_GROUP * portGroupId);

        /* get the CNC dump node that was added during
            prvCpssDxChGdmaCncBeforeDumpTrigger(...) that called from
            cpssDxChCncPortGroupBlockUploadTrigger(...)
        */
        cncDumpNodePtr = cncDumpNodeGet(cncGlobalInfoPtr->cncDumpListHeadPtr,cncUnitsBmp);
        if(cncDumpNodePtr == NULL)
        {
            /* the linked list not hold info about trigger of this CNC unit */
            continue;
        }

        if(isFirstIteration)
        {
            /* save the first to compare to others */
            cncDumpNodePtr_first = cncDumpNodePtr;
            portGroupId_first = portGroupId;
        }
        else
        if(cncDumpNodePtr_first != cncDumpNodePtr)
        {
            /* different node found , so triggered on different times  */
            /* check that both hold the same number of unread counters */
            /* as the below logic going to sum the counters from those dumps */

            if(cncDumpNodePtr_first->portGroupInfo[portGroupId_first].node_cncFormat !=
               cncDumpNodePtr->portGroupInfo[portGroupId].node_cncFormat)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The CNC format [%d] in port group [%d] not match CNC format [%d] in port group [%d] ",
                    cncDumpNodePtr_first->portGroupInfo[portGroupId_first].node_cncFormat,
                    portGroupId_first,

                    cncDumpNodePtr->portGroupInfo[portGroupId].node_cncFormat,
                    portGroupId);
            }

            if(cncDumpNodePtr_first->portGroupInfo[portGroupId_first].node_unreadCncCounters !=
               cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The unreadCncCounters [%d] in port group [%d] not match unreadCncCounters [%d] in port group [%d] ",
                    cncDumpNodePtr_first->portGroupInfo[portGroupId_first].node_unreadCncCounters,
                    portGroupId_first,

                    cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters,
                    portGroupId);
            }
        }

        isFirstIteration = 0;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    if(isFirstIteration)
    {
        /* not found info about the dump for those port groups */
        *numOfCounterValuesPtr = 0;

        return /* it's not error for log */ GT_NO_MORE;
    }

    isFirstIteration = 1;

    /* sum the counters from all the port groups */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        cncUnitsBmp =
            cncUnitsBmp_perPortGroup << (SIP7_MAX_CNC_UNITS_PER_PORT_GROUP * portGroupId);

        /* get the CNC dump node that was added during
            prvCpssDxChGdmaCncBeforeDumpTrigger(...) that called from
            cpssDxChCncPortGroupBlockUploadTrigger(...)
        */
        cncDumpNodePtr = cncDumpNodeGet(cncGlobalInfoPtr->cncDumpListHeadPtr,cncUnitsBmp);
        if(cncDumpNodePtr == NULL)
        {
            /* the linked list not hold info about trigger of this CNC unit */
            /* this port group have nothing to contribute                   */
            continue;
        }

        if(cncDumpNodePtr->portGroupInfo[portGroupId].node_cncFormat != (GT_U32)format)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The CNC format [%d] not match the 'trigger dump' format [%d] in port group [%d]",
                format ,
                cncDumpNodePtr->portGroupInfo[portGroupId].node_cncFormat,
                portGroupId);
        }

        retryBlockGet_lbl:/*retry to get the block of counters */

        portGroupNumOfCounters = *numOfCounterValuesPtr;

        rc = prvCpssPpConfigBitmapFirstActiveBitGet(
            cncDumpNodePtr->node_cncUnitsBmp & cncUnitsBmp,
            &cncUnitId);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* all port groups have the same number of counters */
        rc = perUnit_CncUploadedBlockGet(
            devNum, cncDumpNodePtr , cncUnitId,
            format, &portGroupNumOfCounters, counterValuesPtr);

        if(cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters == 0)
        {
            /* this port group was fully served and the matching CNC unit can be removed from the bmp */
            cncDumpNodePtr->node_cncUnitsBmp &= ~(1<<cncUnitId);
        }

        if(cncDumpNodePtr->node_cncUnitsBmp == 0)
        {
            /* we served ALL port groups and all the matching CNC units so we can remove the node */
            cncDumpNodeRelease(cncGlobalInfoPtr,cncDumpNodePtr);
            /* set it NULL , so not used later 'by mistake' */
            cncDumpNodePtr = NULL;
        }

        if(rc == GT_NOT_READY && retryIteration && cncDumpNodePtr)
        {
            /* this port group is not ready yet , let's give it another try
               after minimal sleep */
            retryIteration = 0;
            cpssOsTimerWkAfter(CNC_MINIMAL_SLEEP_TIME_CNS);
            goto retryBlockGet_lbl;
        }

        retryIteration = 1;
        if(((GT_OK == rc) || (GT_NO_MORE == rc)) &&
            (! isFirstIteration) &&
            (portGroupNumOfCounters != minNumOfCounters))
        {
            /* the non first port group hold different number of counters
               than the previous port groups ! */
            outOfSyncStatus = GT_GET_ERROR;
        }

        isFirstIteration = 0;

        if( portGroupNumOfCounters < minNumOfCounters )
        {
            minNumOfCounters = portGroupNumOfCounters;
        }

        if( GT_OK == rc || GT_NO_MORE == rc )
        {
            /* continue to next port group in case get number of requested counters */
            /* successful or this port group has no more counters */
        }
        else
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    *numOfCounterValuesPtr = minNumOfCounters;

    if(outOfSyncStatus != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(outOfSyncStatus,
            "the port groups BMP[0x%x] are 'out of sync' about the number of available counters ",
            portGroupsBmp);
    }

    /* at this point : ( GT_OK == rc || GT_NO_MORE == rc) */
    return rc;
}

/**
* @internal gdmaMsgInfoPrint function
* @endinternal
*
* @brief   GDMA : common Prints MSG Queues info (AUQ/FUQ/CNC/IPE).
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS gdmaMsgInfoPrint
(
    IN  GT_U8                  devNum,
    IN  PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_GDMA_DB_PER_MSG_DESCRIPTORS_INFO_STC *msg_extInfoArr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  *decriptorsInfoPtr;
    PRV_CPSS_GDMA_DESC_STC  *currDescPtr;
    GT_U32  ii;
    GT_U32  word0;
    GT_U32  regValue,regAddr,value;
    GT_U32  gdmaUnitId,ringId;
    GT_U32  hw_numBytesUsedInBuffer;
    GT_UINTPTR  buff_virtAddr;/*Virtual address of the buffer */

    msg_extInfoArr  = clientMsgRingInfoPtr->msg_extInfoArr;
    decriptorsInfoPtr = &clientMsgRingInfoPtr->msg_descriptors;

    cpssOsPrintf("gdmaUnitId[%d],localRing[%d],msgSizeInBytes[%d],descNum[%d],currDesc[%d],numOfDescTreated[%d] \n",
        clientMsgRingInfoPtr->gdmaUnitId,
        clientMsgRingInfoPtr->localRing,
        clientMsgRingInfoPtr->msgSizeInBytes,
        decriptorsInfoPtr->toCpu_numOfDescriptors,
        decriptorsInfoPtr->toCpu_currDescriptorIndex,
        clientMsgRingInfoPtr->debug_and_statistics.numOfDescTreated);

    cpssOsPrintf("Descriptors info : \n");
    cpssOsPrintf("index | OWNER | offsetFromDescToBuff |  currOffsetInBuffer | numBytesInBuffer (num MSG) : \n");

    for(ii = 0 ; ii < decriptorsInfoPtr->toCpu_numOfDescriptors; ii++)
    {
        currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[ii];
        word0 = CPSS_32BIT_LE(currDescPtr->word0);

        cpssOsPrintf("%2.2d  |  [%s] |     [%ld]      |        [%d]           |      [%d] (%d) \n",
            ii,
            ((word0 & BIT_0) == OWNER_HW) ?  " HW":"CPU",
            msg_extInfoArr[ii].offsetFromDescToBuff,
            msg_extInfoArr[ii].currOffsetInBuffer,
            msg_extInfoArr[ii].numBytesInBuffer,
            msg_extInfoArr[ii].numBytesInBuffer / clientMsgRingInfoPtr->msgSizeInBytes);
    }

    cpssOsPrintf("info about desc addresses (can use cpssPpDumpCpuMemory(...) to dump memory): \n");
    cpssOsPrintf("index | : desc addr(virt) | addr start buffer (virt) | addr after buffer (virt) \n");
    for(ii = 0 ; ii < decriptorsInfoPtr->toCpu_numOfDescriptors; ii++)
    {
        currDescPtr = &decriptorsInfoPtr->toCpu_descQueueStartVirtualAddress[ii];
        /* this is start of buffer */
        buff_virtAddr = (GT_UINTPTR)currDescPtr + msg_extInfoArr[ii].offsetFromDescToBuff;

        cpssOsPrintf("%2.2d  |  [%p]      |  [%p]        |  [%p] \n",
            ii,
            currDescPtr,
            buff_virtAddr ,
            buff_virtAddr + msg_extInfoArr[ii].numBytesInBuffer);

    }
    /**************************************************/
    /* print info from the HW registers for this ring */
    /**************************************************/
    ringId     = clientMsgRingInfoPtr->localRing;
    gdmaUnitId = clientMsgRingInfoPtr->gdmaUnitId;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[ringId].wruSGDPointer;
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf(" ========== start : status from HW ============= \n");

    /*<WRU SGD Pointer Value>*/
    value = regValue & 0xFFFF;
    cpssOsPrintf("WRU SGD Pointer Value [%d] \n",
        value);

    ii = value;

    /*<SGD remaining byte count>*/
    value = regValue >> 16;
    cpssOsPrintf("SGD remaining byte count [%d]\n",
        value);

    if(ii < decriptorsInfoPtr->toCpu_numOfDescriptors)
    {
        if(value > msg_extInfoArr[ii].numBytesInBuffer)
        {
            /* ERROR */
            cpssOsPrintf("ERROR : SGD remaining byte count [%d] > numBytesInBuffer[%d] (in SGD[%d])\n",
                value,
                msg_extInfoArr[ii].numBytesInBuffer,
                ii);
        }
        else
        {
            hw_numBytesUsedInBuffer = msg_extInfoArr[ii].numBytesInBuffer - value;
            cpssOsPrintf("calculating ... SGD used byte count [%d] out of [%d] bytes (in SGD[%d])\n",
                hw_numBytesUsedInBuffer,
                msg_extInfoArr[ii].numBytesInBuffer,
                ii);
        }
    }
    else
    {
        /* ERROR */
        cpssOsPrintf("ERROR : SGD [%d] >= toCpu_numOfDescriptors[%d] \n",
            ii,
            decriptorsInfoPtr->toCpu_numOfDescriptors);
    }


    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[ringId].freeSGDsStatusRegister;
    rc = prvCpssHwPpGdmaReadReg(devNum,gdmaUnitId,regAddr,&regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*<Free SGD number>*/
    value = regValue & 0xFFFF;
    cpssOsPrintf("Free SGD number [%d] \n",
        value);
    value = (regValue >> 16) & 0xFF;
    cpssOsPrintf("SGD buffer fill level [%d] \n",
        value);

    cpssOsPrintf(" ========== end   : status from HW ============= \n");

    return GT_OK;
}

/**
* @internal gdmaCncMsgInfoPrint function
* @endinternal
*
* @brief   GDMA : CNC : Prints MSG Queues info .
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS gdmaCncMsgInfoPrint
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC           *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    GT_U32  cncUnitId;/* local CNC unit Id to the pipe */
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC  *cncDumpNodePtr;
    GT_U32  totalDumps = 0;
    GT_U32  portGroupId;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    cpssOsPrintf("CNC Info \n");

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        for(cncUnitId = 0 ; cncUnitId < SIP7_MAX_CNC_UNITS_PER_PORT_GROUP ; cncUnitId++)
        {
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[cncUnitId + portGroupId*SIP7_MAX_CNC_UNITS_PER_PORT_GROUP];
            if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E)
            {
                /* not used */
                continue;
            }

            cpssOsPrintf("CNC Info about unit [%d] in port group [%d] \n",
                cncUnitId,portGroupId);

            (void)gdmaMsgInfoPrint(devNum,clientMsgRingInfoPtr);

        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    cpssOsPrintf("======== \n");

    cncDumpNodePtr = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncDumpListHeadPtr;
    while(cncDumpNodePtr)
    {
        cpssOsPrintf("CNC dump linked list NODE [%d] : \n",
            totalDumps);
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            cpssOsPrintf("portGroupId[%d] node_unreadCncCounters[%d] , node_cncFormat[%d] \n",
                portGroupId,
                cncDumpNodePtr->portGroupInfo[portGroupId].node_unreadCncCounters,
                cncDumpNodePtr->portGroupInfo[portGroupId].node_cncFormat);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

        cpssOsPrintf("node_cncUnitsBmp[0x%x] (unitNum[%d])\n",
            cncDumpNodePtr->node_cncUnitsBmp,
            cncDumpNodePtr->debug_and_statistics.unitNum);

        cncDumpNodePtr = cncDumpNodePtr->nextCncDumpNodePtr;

        totalDumps++;
    }

    if(totalDumps == 0)
    {
        cpssOsPrintf("CNC dump linked list is empty \n");
    }

    cpssOsPrintf("======== \n");

    return GT_OK;
}

/**
* @internal gdmaFdbMsgInfoPrint function
* @endinternal
*
* @brief   GDMA : FDB (AUQ/FUQ) : Prints MSG Queues info .
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS gdmaFdbMsgInfoPrint
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC           *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    GT_U32  portGroupId;
    GT_U32  tileId;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    cpssOsPrintf("AUQ Info \n");

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        tileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[tileId];

        if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E)
        {
            /* not used */
            continue;
        }

        cpssOsPrintf("AUQ Info about tile [%d] \n",
            tileId);

        (void)gdmaMsgInfoPrint(devNum,clientMsgRingInfoPtr);

        portGroupId += PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile - 1;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    cpssOsPrintf("======== \n");

    cpssOsPrintf("FUQ Info \n");

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        tileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[tileId];

        if(clientMsgRingInfoPtr->clientMsgType != PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E)
        {
            /* not used */
            continue;
        }

        cpssOsPrintf("FUQ Info about tile [%d] \n",
            tileId);

        (void)gdmaMsgInfoPrint(devNum,clientMsgRingInfoPtr);

        portGroupId += PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile - 1;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    cpssOsPrintf("======== \n");

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaMsgInfoPrint function
* @endinternal
*
* @brief   GDMA : Prints MSG Queues info (AUQ/FUQ/CNC/IPE).
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDxChGdmaMsgInfoPrint
(
    IN  GT_U8                  devNum
)
{
    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the device not exists");
    }

    if(!PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "the device not supports GDMA");
    }

    /*AUQ,FUQ*/
    gdmaFdbMsgInfoPrint(devNum);
    /*CNC*/
    gdmaCncMsgInfoPrint(devNum);

    /*IPE - TBD */

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaFreeDb_toCpu function
* @endinternal
*
* @brief   GDMA : free all dynamic allocation that the GDMA 'TO_CPU' did during init and in runtime.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChGdmaFreeDb_toCpu
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC  *buffersInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Ptr;
    GT_U32  queue;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr)
    {
        for(queue = 0 ; queue < PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_toCpu ; queue++)
        {
            gdmaPerQueue_toCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr[queue];
            buffersInfoPtr    = &gdmaPerQueue_toCpu_Ptr->toCpu_buffers;

            FREE_PTR_MAC(buffersInfoPtr->buffersNodesArray);
        }

        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaFreeDb_fromCpu function
* @endinternal
*
* @brief   GDMA : free all dynamic allocation that the GDMA 'FROM_CPU' did during init and in runtime.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChGdmaFreeDb_fromCpu
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr;
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC *decriptorsInfoPtr;
    GT_U32  queue;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr)
    {
        for(queue = 0 ; queue < PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.totalNumQueues_fromCpu ; queue++)
        {
            gdmaPerQueue_fromCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[queue];
            decriptorsInfoPtr = &gdmaPerQueue_fromCpu_Ptr->fromCpu_descriptors;

            FREE_PTR_MAC(decriptorsInfoPtr->fromCpu_descExtInfoArr);
        }

        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaFreeDb_msg function
* @endinternal
*
* @brief   GDMA : free all dynamic allocation that the GDMA MSG did during init and in runtime.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChGdmaFreeDb_msg
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC         *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC    *cncDumpNodePtr,*old_cncDumpNodePtr;
    GT_U32  ii;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    /************/
    /* AUQ info */
    /************/
    for(ii = 0 ; ii < MAX_TILE_CNS ; ii++)
    {
        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ii];
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_extInfoArr);
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_buffers.buffersNodesArray);
    }

    /************/
    /* FUQ info */
    /************/
    for(ii = 0 ; ii < MAX_TILE_CNS ; ii++)
    {
        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ii];
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_extInfoArr);
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_buffers.buffersNodesArray);
    }

    /************/
    /* CNC info */
    /************/
    for(ii = 0 ; ii < SIP7_MAX_CNC_UNITS ; ii++)
    {
        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[ii];
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_extInfoArr);
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_buffers.buffersNodesArray);
    }

    cncDumpNodePtr = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncDumpListHeadPtr;
    while(cncDumpNodePtr)
    {
        old_cncDumpNodePtr = cncDumpNodePtr;
        cncDumpNodePtr = cncDumpNodePtr->nextCncDumpNodePtr;
        FREE_PTR_MAC(old_cncDumpNodePtr);
    }

    /************/
    /* IPE info */
    /************/
    for(ii = 0 ; ii < (4*MAX_TILE_CNS) ; ii++)
    {
        clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaIpeInfo[ii];
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_extInfoArr);
        FREE_PTR_MAC(clientMsgRingInfoPtr->msg_buffers.buffersNodesArray);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaFreeDb function
* @endinternal
*
* @brief   GDMA : free all dynamic allocation that the GDMA did during init and in runtime.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDxChGdmaFreeDb
(
    IN  GT_U8                  devNum
)
{
    GT_STATUS   rc;

    /* free the memory allocated to 'to cpu' */
    rc = prvCpssDxChGdmaFreeDb_toCpu(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* free the memory allocated to 'from cpu' */
    rc = prvCpssDxChGdmaFreeDb_fromCpu(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* free the memory allocated to 'messages' (AUQ/FUQ/CNC/IPE) */
    rc = prvCpssDxChGdmaFreeDb_msg(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaGlobalInit function
* @endinternal
*
* @brief  init the GDMA unit for global purposes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaGlobalInit
(
    IN GT_U8                    devNum
)
{
    /***************************************************************************/
    /* SWAP bytes code for legacy SDMA in sdmaSwapModeAndDeviceRevisionGet(...) */
    /***************************************************************************/
        /* SDMA LE/BE should works the same since the CPU get the data as raw
           byte */
        /* Enable byte swap, Enable word swap  */
        /*
        prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR, 6, 2, 3);
        prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR,23,2,0);
        */

    /* do not change the defaults of the PDI Slave in the GDMA_Dispatcher , as the defaults are good:
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->CNM.
        GDMA_Dispatcher.gdmaDispatcherControl;
     <PDI Slave word swap enable> , <PDI Slave byte swap enable>
    rc = prvCpssDrvHwPpSetRegField(devNum,regAddr, 3, 2, 0);

        NOTE that for 'from_CPU' we do change swap default value in register , in the
        proper GDMA unit (see function gdmaInitRingType(...)):
        PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).generalRegs.gdmaMiscConfig
    */

    /* since no need to modify defaults in the GDMA_Dispatcher , the function is empty */
    devNum = devNum;

    return GT_OK;
}

/**
* @internal prvCpssDxChGdmaRingFullGet function
* @endinternal
*
* @brief   get indication if the ring (in wait mode) is full (busy waiting).
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                 - The device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES : AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] clientMsgType          - The client type
* @param[in] subClientIndex         - The sub client index
*                                    Relevant to CNC and IPE.
* @param[out] isFullPortGroupsBmpPtr - (pointer to) BMP of per port group bit if the queue full.
*                                       a bit is 1 meaning that port group ring is full.
*                                       a bit is 0 meaning that port group ring is not full.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - HW error
*/
GT_STATUS prvCpssDxChGdmaRingFullGet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroupsBmp,
    IN PRV_CPSS_DXCH_GDMA_CLIENT_TYPE_MSG_ENT   clientMsgType,
    IN GT_U32                                   subClientIndex,
    OUT GT_PORT_GROUPS_BMP                      *isFullPortGroupsBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      fieldValue;
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    GT_U32 ringInfoIndex;
    GT_U32 jumpIndexBeteenTiles;
    GT_U32 gdmaUnitId,ringId;
    GT_U32 portGroupId;
    GT_U32 tileId;
    GT_U32 allFull;

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;
    switch(clientMsgType)
    {
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E :
            ringInfoIndex = 0;
            jumpIndexBeteenTiles = 1;
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForAuq;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ringInfoIndex];
            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_FDB_E);
            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E :
            ringInfoIndex = 0;
            jumpIndexBeteenTiles = 1;
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForFuq;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ringInfoIndex];
            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_FDB_E);
            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E     :
            ringInfoIndex = subClientIndex;
            jumpIndexBeteenTiles = SIP7_MAX_CNC_UNITS_PER_PORT_GROUP;/* 4 CNC units per tile */
            gdmaUnitId  = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForCnc;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaCncInfo.cncInfo[ringInfoIndex];
            if(subClientIndex == 0)
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_CNC_0_E);
            if(subClientIndex == 1)
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_CNC_1_E);
            if(subClientIndex == 2)
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_CNC_2_E);
            if(subClientIndex == 3)
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_CNC_3_E);
            break;
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_4_E   :
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_8_E   :
        case PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_16_E  :
            ringInfoIndex = subClientIndex;
            jumpIndexBeteenTiles = 4;/* 4 IPE interfaces per tile */
            gdmaUnitId = gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForIpe;
            clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaIpeInfo[ringInfoIndex];
            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_IPE_E);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientMsgType);
    }

    *isFullPortGroupsBmpPtr = 0;
    allFull = 1;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        tileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
        ringId = clientMsgRingInfoPtr[jumpIndexBeteenTiles*tileId].localRing;
        gdmaUnitId = clientMsgRingInfoPtr[jumpIndexBeteenTiles*tileId].gdmaUnitId;

        /* Get <busy waiting> */
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[ringId].queueEnable;
        rc = prvCpssHwPpGdmaGetRegField(devNum,gdmaUnitId,regAddr,2,1,&fieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(fieldValue)
        {
            *isFullPortGroupsBmpPtr |= 1 << portGroupId;
        }
        else
        {
            allFull = 0;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS && allFull)
    {
        *isFullPortGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChGdmaFdbMsgTraceEnable function
* @endinternal
*
* @brief   GDMA : (debug tool) enable/disable trace prints of FDB AU/FU MSG.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
* @param[in] enable                - enable/disable the trace .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDxChGdmaFdbMsgTraceEnable
(
    IN  GT_U8                  devNum,
    IN  GT_BOOL                enable
)
{
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  *clientMsgRingInfoPtr;
    PRV_CPSS_DXCH_GDMA_INFO_STC                       *gdmaInfoPtr;
    GT_U32 ringInfoIndex = 0;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the device not exists");
    }

    if(!PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "the device not supports GDMA");
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;
    /* get pointer to AUQ metadata */
    clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaAuqInfo[ringInfoIndex];
    /* save the info for AUQ */
    clientMsgRingInfoPtr->debug_and_statistics.enableTraceMsg = BOOL2BIT_MAC(enable);


    /* get pointer to FUQ metadata */
    clientMsgRingInfoPtr = &gdmaInfoPtr->gdmaClientsMsgInfo.gdmaFuqInfo[ringInfoIndex];
    /* save the info for FUQ */
    clientMsgRingInfoPtr->debug_and_statistics.enableTraceMsg = BOOL2BIT_MAC(enable);

    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfGdmaSgdChainSizeGet function
* @endinternal
*
* @brief   Get Ring/Queue SDG chain size current HW configuration
*          Reperesents number of packets-in-queue to be sent by PktGen if queue enabled
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum             PP's device number
* @param[in] gdmaUnit           gdma unit number
* @param[in] queueNum           Queue number (0..32)
* @param[out] sgdChainSizePtr   sgd_chain_size value in a HW-register
*
* @retval GT_OK
* @retval GT_BAD_PARAM        - The requested absolute or incremental value causes for
*                                 overload or underload
* @retval GT_NOT_INITIALIZED  - if the REG driver was not initialized
*
* @note This API is for TxGenerator queue but could be used for other queues as well
*/
GT_U32 prvCpssDxChNetIfGdmaSgdChainSizeGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      gdmaUnit,
    IN  GT_U32      queueNum,
    IN  GT_U32      *sgdChainSizePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      readVal;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[queueNum].sgdChainSize;
    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnit, regAddr, 0, 16, &readVal);
    *sgdChainSizePtr = readVal;
    return rc;
}

/**
* @internal prvCpssDxChNetIfGdmaSgdChainSizeSet function
* @endinternal
*
* @brief   Set Ring/Queue SDG chain size
*          The setting could be 'absolute set Num' or incremental +Num / -Num
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum             PP's device number
* @param[in] gdmaUnit           gdma unit number
* @param[in] queueNum           Queue number (0..32)
* @param[in] modeIsIncremental  GT_FALSE set 'sgdChainSize' as absolute value
*                               GT_TRUE incremental +sgdChainSize/-sgdChainSize
* @param[in] sgdChainSize       signed, value to be set into sgd_chain_size
*                               or to increment/decrement current register's value
*
* @retval GT_OK
* @retval GT_BAD_PARAM        - The requested absolute or incremental value causes for
*                                 overload or underload
* @retval GT_NOT_INITIALIZED  - if the REG driver was not initialized
*
* @note This API is for TxGenerator queue but could be used for other queues as well
*/
GT_U32 prvCpssDxChNetIfGdmaSgdChainSizeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      gdmaUnit,
    IN  GT_U32      queueNum,
    IN  GT_BOOL     modeIsIncremental,
    IN  GT_32       sgdChainSize
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      readVal, writeVal;
    PRV_CPSS_TX_DESC_LIST_STC   *txDescList;

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[queueNum]);
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[queueNum].sgdChainSize;

    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnit, regAddr, 0, 16, &readVal);
    if(rc != GT_OK)
        return rc;

    if (modeIsIncremental == GT_TRUE)
    {
        writeVal = (sgdChainSize == 0) ?
            0xFFFFFFFF/*invalid*/ :
            (GT_U32)((GT_32)readVal + sgdChainSize);
    }
    else
    {
        writeVal = (GT_U32)sgdChainSize;
    }

    if (writeVal > txDescList->maxDescNum || writeVal >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
            "Wrong SGD chain size q[%d] request=%d current=%d", queueNum, sgdChainSize, readVal);
    }

    rc = prvCpssHwPpGdmaSetRegField(devNum, gdmaUnit, regAddr, 0, 16, writeVal);

    return rc;
}

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx GDMA Generator.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] unitId                   - GDMA unit id/number.
* @param[in] txQueue                  - Tx GDMA queue to enable.
* @param[in] burstEnableReq           - GT_TRUE for packets burst generation,
*                                     - GT_FALSE for continuous packets generation
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst size.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
*
* @note If burstEnable and queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW Generator enabling the Tx GDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
* @note     No special GDMA NetIfSdmaTxGeneratorDisable() needed,
*           common cpssDxChNetIfSdmaTxQueueEnable(GT_FALSE) is used
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          unitId,
    IN  GT_U32                          txQueue,
    IN  GT_BOOL                         burstEnableReq,
    IN  GT_U32                          burstPacketsNumber
)
{
    PRV_CPSS_TX_DESC_LIST_STC   *txDescList;
    GT_U32      sgdRecycleChain; /* whether to restart the GDMA chain when ends */
    GT_BOOL     burstEnableCurr; /* burst - run chain once, don't restart */
    GT_U32      packetsInQueue;
    GT_U32      queueEnable;     /* Ring enable, set 1 after all configurations are completed */
    GT_U32      queueType;       /* Ring type field encodes the flow */
    GT_U32      regAddr, value;
    GT_BOOL     forceDownReq;    /* GT_TRUE if ring is Re-Configured and therefore
                                    should be 'disable' before 'enable' */
    GT_STATUS   rc;

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    packetsInQueue = txDescList->maxDescNum - txDescList->freeDescNum;

    if ((burstEnableReq == GT_TRUE) && (burstPacketsNumber > packetsInQueue))
    {
        /* GDMA TxGenerator can't apply (burst > packets-in-chain).
         * It passes the chain once and stops (vs Normal running cyclically till Disable).
         * No possibility to say "repeat chain N times".
         */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
    rc = prvCpssHwPpGdmaReadReg(devNum, unitId, regAddr, &value);
    if (rc != GT_OK)
        return rc;
    sgdRecycleChain = ((value & BIT_4) != 0);
    burstEnableCurr = (sgdRecycleChain == 0);
    queueType = value & 0xf;
    if (queueType != GDMA_RING_TYPE_FROM_CPU_PKT_GEN_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueEnable;
    rc = prvCpssHwPpGdmaGetRegField(devNum, unitId, regAddr, 0, 1, &queueEnable);
    if (rc != GT_OK)
        return rc;

    forceDownReq = burstEnableReq || (burstEnableReq != burstEnableCurr);

    if (burstEnableReq == GT_TRUE)
    {
        /* Force HW chain-size to burst-size */
        rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, unitId, txQueue,
                                                 GT_FALSE, burstPacketsNumber);
    }
    else /* Normal mode requested */
    if (burstEnableCurr == GT_TRUE)
    {
        /* Restore HW chain-size to packets-in-SW-queue */
        rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, unitId, txQueue,
                                                 GT_FALSE, packetsInQueue);
    }
    if (rc != GT_OK)
        return rc;

    /* Indication for explicit enabling request */
    txDescList->userQueueEnabled = GT_TRUE;

    if (forceDownReq == GT_FALSE && queueEnable == 1)
    {
        return GT_OK; /* Nothing changed */
    }
    if (forceDownReq == GT_TRUE && queueEnable == 1)
    {
        rc = prvCpssDxChGdmaNetIfTxQueueEnableSet(devNum,txQueue, GT_FALSE);
        if (rc != GT_OK)
            return rc;
        txDescList->userQueueEnabled = GT_TRUE; /* has been cleared in 'disable' */
    }
    /* Flush FIFO and sets ring to first SGD (cleared by HW when flush is complete) */
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueEnable;
    rc = prvCpssHwPpGdmaSetRegField(devNum, unitId, regAddr, 5, 1, 1);
    if (rc != GT_OK)
        return rc;

    sgdRecycleChain = (burstEnableReq == GT_FALSE);
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
    rc = prvCpssHwPpGdmaSetRegField(devNum, unitId, regAddr, 4, 1, sgdRecycleChain);
    if (rc != GT_OK)
        return rc;

    /* Enabling a queue is prohibited if the queue is empty, and will be done */
    /* later implicitly when the first packet is added to the queue. */
    if (txDescList->freeDescNum == txDescList->maxDescNum)
    {
        return GT_OK;
    }
    return prvCpssDxChGdmaNetIfTxQueueEnableSet(devNum,txQueue, GT_TRUE);
}

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorGet function
* @endinternal
*
* @brief   Get GDMA descriptor's parced information
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum           - Device number.
* @param[in] descPtr          - GDMA descriptor used for that packet
* @param[out] buffAddr        - packet buffer virtual address
* @param[out] buffLen         - packet data length
* @param[out] isValid         - VALID bit is set in descriptor
* @param[out] handledByHw     - GT_TRUE - OWN bit0 is cleared by HW (handled by HW)
*                               GT_FALSE - bit was set by SW and still not handled by HW
*
* @retval GT_OK               - on success
* @retval GT_FAIL             - cannot convert Physical descriptor's address to Virtual
*
* @note  Output 'handledByHw' is optional - may have NULL storage
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorDescriptorGet
(
    IN  GT_U8                   devNum,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr,
    OUT GT_U8                   **buffAddr,
    OUT GT_U32                  *buffLen,
    OUT GT_BOOL                 *isValid,
    OUT GT_BOOL                 *handledByHw
)
{
    PRV_CPSS_GDMA_DESC_STC  descTmp;
    GT_UINTPTR              phyAddr;
    GT_UINTPTR              virtAddr;
    GT_STATUS               rc;

    descTmp.word0 = CPSS_32BIT_LE(descPtr->word0);
    descTmp.word1 = CPSS_32BIT_LE(descPtr->word1);
    descTmp.word3 = CPSS_32BIT_LE(descPtr->word3);
    if (descTmp.word0 == 0 || descTmp.word1 == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "SW: Wrong SGD descriptor");
    }
    GDMA_DESC_GET_BUFF_ADDR_48_BITS_FIELD((&descTmp), phyAddr);
    #if __WORDSIZE == 64  /* 64 bits compilation */
        phyAddr -= (((GT_UINTPTR)PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh/*0x2*/) << 32);
    #endif
    rc = perDev_cpssOsPhy2Virt(devNum, phyAddr, &virtAddr);
    if (rc != GT_OK)
    {
        return rc;
    }
    *buffAddr = (GT_U8*)virtAddr;
    *buffLen = (descTmp.word0 >> 14) & 0xffff;
    *isValid = !!(descTmp.word0 & BIT_5);
    if (handledByHw != NULL)
    {
        *handledByHw = !(descTmp.word0 & BIT_0);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet function
* @endinternal
*
* @brief   Set GDMA descriptor (SGD) with packet buffer for TxGenerator
*          or only update the VALID bit (if packet lenght or buffAddr are NULL)
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] descPtr                  - GDMA descriptor used for that packet
* @param[in] buffAddr                 - packet buffer virtual address
* @param[in] buffLen                  - packet data length
* @param[in] validSet                 - set or no(clear) Descriptor's bit VALID
*
* @retval GT_OK               - on success
* @retval GT_FAIL             - cannot convert Virtual to Physical address
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet
(
    IN  GT_U8                   devNum,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr,
    IN  GT_U8                   *buffAddr,
    IN  GT_U32                  buffLen,
    IN  GT_BOOL                 validSet
)
{
    GT_UINTPTR  phyAddr;            /* physical address of packet-buffer to set into SGD */
    GT_U32      bufferDmaAddr_low;  /* tmp variable containing LOW part of physical buf-address */
    GT_U32      bufferDmaAddr_high; /* tmp variable containing HIGH part of physical buf-address */
    GT_U32      word0;              /* Bits 0..31 out of 128 Descriptor's bits */
    GT_U32      phyAddr_2_LSBits;   /*  2 bits to write to word0 of the descriptor */
    GT_U32      phyAddr_32_MSBits;  /* 32 bits to write to word1 of the descriptor */
    GT_U32      phyAddr_14_LSBits;  /* 14 bits to write to word3 of the descriptor */
    GT_U32      buffLenOrig;        /* Original buffer_byte_count (packet length) field value */
    GT_U32      axiAttrProfile;     /* Keep/set field "AXI ATTR Profile configuration table" */
    GT_STATUS   rc;

    word0 = CPSS_32BIT_LE(descPtr->word0);
    phyAddr_2_LSBits = word0 >> 30;
    buffLenOrig = (word0 >> 14) & 0xffff;
    axiAttrProfile = (word0 >> 8) & 0x1f;
    word0 = axiAttrProfile | OWNER_HW; /* notify the HW that descriptor is updated */

    if (buffAddr != NULL)
    {
        /* Full SGD setting */
        rc = cpssOsVirt2Phy((GT_UINTPTR)buffAddr, &phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr);

        PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(phyAddr,
            bufferDmaAddr_low,
            bufferDmaAddr_high);

        bufferDmaAddr_high += PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh;

        PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(
            /* IN */
            bufferDmaAddr_low,
            bufferDmaAddr_high,
            /* OUT */
            phyAddr_2_LSBits,
            phyAddr_32_MSBits,
            phyAddr_14_LSBits);

        descPtr->word1 = CPSS_32BIT_LE(phyAddr_32_MSBits);
        descPtr->word3 = CPSS_32BIT_LE(phyAddr_14_LSBits);
    }
    word0 |= phyAddr_2_LSBits << 30;

    /* bufLen=0 ~ do not change current len-config
     * bufLen=1 ~ set minimum '0' which is equal to 1B
     * >1 ~ (bufLen-1)
     */
    if (buffLen == 0)
    {
        word0 |= buffLenOrig << 14;
    }
    else
    {
        word0 |= ((buffLen - 1) & 0xFFFF) << 14; /* Register keeps (len -1) */
    }

    if (validSet == GT_TRUE)
    {
        word0 |= BIT_5 | BIT_6 | BIT_7; /* Valid | First | Last */
    }
    descPtr->word0 = CPSS_32BIT_LE(word0);      /* should be last set */

    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap function
* @endinternal
*
* @brief   Swap an information between 2 descriptors and set/clear Valid bit
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] descPtr1        - 1st descriptor to be swapped with 2nd
* @param[in] descValid1      - GT_TRUE/GT_FALSE set/clear VALID bit on 1st after swap
* @param[in] descPtr2        - 2nd descriptor to be swapped with 1st
* @param[in] descValid2      - GT_TRUE/GT_FALSE set/clear VALID bit on 2nd after swap
*
* @retval GT_VOID
*
* @note used for SWAP but also to invalidate a descriptor/descriptors
*/
GT_VOID prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap
(
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr1, /*current*/
    IN  GT_BOOL                 descValid1,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr2, /*last*/
    IN  GT_BOOL                 descValid2
)
{
        PRV_CPSS_GDMA_DESC_STC  desc1, desc2;

        /* .word2 is reserved/unused */
        desc1.word1 = descPtr1->word1;
        desc1.word3 = descPtr1->word3;
        desc2.word1 = descPtr2->word1;
        desc2.word3 = descPtr2->word3;

        if (descPtr1 != descPtr2)
        {   /* SWAP */
            /* .word2 is reserved/unused */
            desc1.word1 = descPtr2->word1;
            desc1.word3 = descPtr2->word3;
            desc2.word1 = descPtr1->word1;
            desc2.word3 = descPtr1->word3;

            desc1.word0 = CPSS_32BIT_LE(descPtr2->word0);
            desc1.word0 &= ~0xff;
            desc1.word0 |= OWNER_HW; /* always set to notify HW about change */
            if (descValid1 == GT_TRUE)
            {
                desc1.word0 |= BIT_5 | BIT_6 | BIT_7; /* Valid | First | Last */
            }
        }

        desc2.word0 = CPSS_32BIT_LE(descPtr1->word0); /* SWAP or just set */
        desc2.word0 &= ~0xff;
        desc2.word0 |= OWNER_HW;
        if (descValid2 == GT_TRUE)
        {
            desc2.word0 |= BIT_5 | BIT_6 | BIT_7; /* Valid | First | Last */
        }

        if (descPtr1 != descPtr2)
        {
            /* set new swapped values */
            descPtr1->word1 = desc1.word1;
            descPtr1->word3 = desc1.word3;
            descPtr2->word1 = desc2.word1;
            descPtr2->word3 = desc2.word3;
            /* SWAP change is activated on 1.word0 update ~ atomic */
            descPtr1->word0 = CPSS_32BIT_LE(desc1.word0);
        }
        descPtr2->word0 = CPSS_32BIT_LE(desc2.word0);
}

/**
* @internal gdmaTxGeneratorBuffListInit function
*/
static GT_STATUS gdmaTxGeneratorBuffListInit
(
    IN  GT_U8       devNum
)
{
    /*-- Differences between prvCpssDxChGdmaNetIfTxPacketSend and TxGenerator --
     * The TxPacketSend is using new GDMA struct-pointers
     *   PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr
     *   PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC *decriptorsInfoPtr
     *   PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *currDescExtInfoPtr
     * The TxGenerator is using generic legacy
     *   PRV_CPSS_TX_DESC_LIST_STC *txDescList.
     */
    GT_STATUS   rc;
    GT_U32      numOfNetIfs;    /* Number of CPU-interfaces (port)   */
    GT_U32      netIfNum;       /* index over CPU-interfaces         */
    GT_U32      numOfQueues;    /* Number of Queues per HW GDMA-unit */
    GT_U32      globalTxQueue;  /* Tx Queue global index in HW GDMA-unit [0..31]*/
    GT_U32      localQ;         /* local Queue index [0..7] in SW struct cfg[4][8] */
    GT_U32      cfgIfIdx;       /* first "interface" index [0..3] in SW cfg[4][8] */
    GT_U32      ii;             /* Index in SGD-chain array */
    GT_U32      generatorsTotalDesc; /* total numOf descriptors\buffers over ALL TxGen queues */
    GT_U32      gdmaUnitId;

    PRV_CPSS_GEN_PP_CONFIG_STC              *devPtr;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC         *moduleCfgPtr;
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *allCpuPortsInfoPtr;
    CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC     *cfgPtr;

    PRV_CPSS_TX_DESC_LIST_STC   *txDescList;  /* the relevant Tx desc list */
    PRV_CPSS_TX_DESC_STC        *firstTxDesc;
    PRV_CPSS_GDMA_DESC_STC      *desc;
    GT_U8                       *buff;
    GT_U32                      size;

    devPtr = PRV_CPSS_PP_MAC(devNum);
    moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);
    allCpuPortsInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.allCpuPortsInfo;

    numOfNetIfs = allCpuPortsInfoPtr->fromCpuInfo.numOfValidPorts;
    txDescList  = devPtr->intCtrl.txDescList;
    localQ = 0;
    generatorsTotalDesc = 0;

    for (netIfNum = 0; netIfNum < numOfNetIfs; netIfNum++)
    {
        numOfQueues = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].numOfQueues;
        gdmaUnitId = allCpuPortsInfoPtr->fromCpuInfo.portsInfo[netIfNum].gdmaUnitId;

        for (globalTxQueue = 0; globalTxQueue < numOfQueues; globalTxQueue++)
        {
            /* Convert Global[0..31] into legacy [4][QUEUE_MAX=8] */
            cfgIfIdx = globalTxQueue / CPSS_MAX_TX_QUEUE_CNS;
            localQ   = globalTxQueue % CPSS_MAX_TX_QUEUE_CNS;

            cfgPtr = &moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[cfgIfIdx][localQ];

            if (CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E != cfgPtr->queueMode)
            {
                continue;
            }
            /* TxGenerator queue settings */
            generatorsTotalDesc += cfgPtr->numOfTxDesc;
            PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.rateMode[cfgIfIdx][localQ] =
                                    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E;

            desc = (GT_VOID *)cfgPtr->memData.staticAlloc.buffAndDescMemPtr;
            buff = (GT_U8 *)desc + sizeof(*desc) * cfgPtr->numOfTxDesc;

            /* Fill descriptors with buffer pointers */
            for (ii = 0; ii < cfgPtr->numOfTxDesc; ii++)
            {
                rc = prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet(devNum,
                                    desc,
                                    buff,
                                    1, /* set MIN value '0' equal 1B */
                                    GT_FALSE/*dont'set valid bit*/);
                if (rc != GT_OK)
                {
                    return rc;
                }
                desc++;
                buff += cfgPtr->buffSize;
            }

            /* Not used fields:
             *   swTxDescBlock
             *   freeCpssBuff
             *   shortBuffers_number
             *   shortBuffers_arr
             *   sramTxDescStart
             *   sramTxBuffStart
             *   sramTxBuffOffset
             *   sramTxBuffEnd
             *   currSwDesc
             *   next2Feed
             *   next2Free
             */
            firstTxDesc = (PRV_CPSS_TX_DESC_STC *)cfgPtr->memData.staticAlloc.buffAndDescMemPtr;
            txDescList[globalTxQueue].firstDescPtr    = firstTxDesc;
            txDescList[globalTxQueue].freeDescPtr     = firstTxDesc;
            txDescList[globalTxQueue].scratchPadPtr   = NULL; /* not used */
            txDescList[globalTxQueue].freeDescNum = cfgPtr->numOfTxDesc;
            txDescList[globalTxQueue].maxDescNum  = cfgPtr->numOfTxDesc;
            txDescList[globalTxQueue].actualBuffSize = cfgPtr->buffSize;
            txDescList[globalTxQueue].userQueueEnabled = GT_FALSE;
            txDescList[globalTxQueue].poolId =
                PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[globalTxQueue].
                                                txEndFifo_poolId;

            /* Allocate "reverse" packet-ID (in DB),
             * to swap last packet during removal operation
             */
            if (txDescList[globalTxQueue].revPacketIdDb == NULL)
            {
                size = sizeof(GT_U32) * txDescList[globalTxQueue].maxDescNum;
                txDescList[globalTxQueue].revPacketIdDb = cpssOsMalloc(size);
                if( NULL == txDescList[globalTxQueue].revPacketIdDb)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                cpssOsMemSet(txDescList[globalTxQueue].revPacketIdDb, 0, size);
            }
            rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, gdmaUnitId, globalTxQueue,
                                                     GT_FALSE/*Not incremental, but absolute set*/,
                                                     0);
        }
    }

    /* Alloc/Init all/multiple Packet Generator queues for 'Total' of descriptors */
    size = sizeof(GT_U32) * generatorsTotalDesc;
    devPtr->intCtrl.txGeneratorPacketIdDb.generatorsTotalDesc = generatorsTotalDesc;
    if (generatorsTotalDesc == 0)
    {
        return GT_OK; /* No TxGenerators in system. All done */
    }

    devPtr->intCtrl.txGeneratorPacketIdDb.firstFree = 0;

    /* allocate packet ID cookies memory and init with 0xffffffff */
    if (devPtr->intCtrl.txGeneratorPacketIdDb.cookie == NULL)
    {
        devPtr->intCtrl.txGeneratorPacketIdDb.cookie = cpssOsMalloc(size);
        if (devPtr->intCtrl.txGeneratorPacketIdDb.cookie == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(devPtr->intCtrl.txGeneratorPacketIdDb.cookie, 0xFF, size);
    }
    /* allocate free packet ID linked list */
    if (devPtr->intCtrl.txGeneratorPacketIdDb.freeLinkedList == NULL)
    {
        devPtr->intCtrl.txGeneratorPacketIdDb.freeLinkedList = cpssOsMalloc(size);
        if (devPtr->intCtrl.txGeneratorPacketIdDb.freeLinkedList == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
    }
    /* initialize the packet ID linked list - each ID pointing to the next */
    for (ii = 0; ii < generatorsTotalDesc; ii++)
    {
        devPtr->intCtrl.txGeneratorPacketIdDb.freeLinkedList[ii] = ii + 1;
    }
    return GT_OK;
}
