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
* @file cpssDxChNetIfGdma.h
*
* @brief Include DxCh GDMA network interface API functions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChNetIfGdmah
#define __cpssDxChNetIfGdmah

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>

/** @brief : A constant for the max number of 'TO_CPU' CPU ports    */
#define CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_PORTS_CNS       2 /* in AAS : one per tile */
/** @brief : A constant for the max number of 'FROM_CPU' CPU ports  */
#define CPSS_DXCH_NETIF_GDMA_FROM_CPU_MAX_PORTS_CNS     6 /* in AAS : 3 per tile */
/** @brief : A constant for the max number of GDMA units that needed per 'TO_CPU' CPU port  */
#define CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_GDMA_UNITS_CNS  2 /* in AAS : 2 GDMA units can serve the 'TO_CPU' CPU port */

/**
* @struct CPSS_DXCH_NETIF_GDMA_TO_CPU_GDMA_UNIT_INFO_STC
 *
 * @brief A Structure with info about the GDMA unit serving 'TO_CPU'.
 *
*/
typedef struct{
    /**@brief : the GDMA unit Id for this CPU Port */
    GT_U32  gdmaUnitId;
    /**@brief : the (local) first queue within the GDMA unit for this CPU Port */
    GT_U32  gdmaFirstQueue;
    /**@brief : the number of queues in this GDMA unit  */
    GT_U32  gdmaNumOfQueues;
}CPSS_DXCH_NETIF_GDMA_TO_CPU_GDMA_UNIT_INFO_STC;

/**
* @struct CPSS_DXCH_NETIF_GDMA_TO_CPU_PORT_INFO_STC
 *
 * @brief A Structure with info about a 'TO_CPU' CPU port.
 *
*/
typedef struct{
    /**@brief : the physical port number of the port */
    GT_PHYSICAL_PORT_NUM  cpuPortNumber;
    /**@brief : the (global) first queue for this CPU Port */
    GT_U32  firstQueue;
    /**@brief : the number of queues for this CPU Port */
    GT_U32  numOfQueues;
    /**@brief : the number of GDMA units that serve this port */
    GT_U32  numOfGdmaUnits;
    /**@brief : info about the GDMA units serving 'TO_CPU'
     *  as there can be several
    */
    CPSS_DXCH_NETIF_GDMA_TO_CPU_GDMA_UNIT_INFO_STC    gdmaUnitInfoArr[CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_GDMA_UNITS_CNS];
}CPSS_DXCH_NETIF_GDMA_TO_CPU_PORT_INFO_STC;

/**
* @struct CPSS_DXCH_NETIF_GDMA_FROM_CPU_PORT_INFO_STC
 *
 * @brief A Structure with info about a 'FROM_CPU' CPU port.
 *
*/
typedef struct{
    /**@brief : the physical port number of the port */
    GT_PHYSICAL_PORT_NUM  cpuPortNumber;
    /**@brief : the (global) first queue for this CPU Port */
    GT_U32  firstQueue;
    /**@brief : the number of queues for this CPU Port */
    GT_U32  numOfQueues;
    /**@brief : the GDMA unit Id for this CPU Port */
    GT_U32  gdmaUnitId;
    /**@brief : the (local) first queue within the GDMA unit for this CPU Port */
    GT_U32  gdmaFirstQueue;
}CPSS_DXCH_NETIF_GDMA_FROM_CPU_PORT_INFO_STC;


/**
* @struct CPSS_DXCH_NETIF_GDMA_TO_CPU_INFO_STC
 *
 * @brief A Structure with info about a CPU port(s) , that used for 'TO_CPU'
 *
*/
typedef struct{
    /**@brief : number of valid indexes in portsInfo */
    GT_U32  numOfValidPorts;

    /**@brief : array of info about each port */
    CPSS_DXCH_NETIF_GDMA_TO_CPU_PORT_INFO_STC portsInfo[CPSS_DXCH_NETIF_GDMA_TO_CPU_MAX_PORTS_CNS];
}CPSS_DXCH_NETIF_GDMA_TO_CPU_INFO_STC;

/**
* @struct CPSS_DXCH_NETIF_GDMA_FROM_CPU_INFO_STC
 *
 * @brief A Structure with info about a CPU port(s) , that used for 'FROM_CPU'
 *
*/
typedef struct{
    /**@brief : number of valid indexes in portsInfo */
    GT_U32  numOfValidPorts;

    /**@brief : array of info about each port */
    CPSS_DXCH_NETIF_GDMA_FROM_CPU_PORT_INFO_STC portsInfo[CPSS_DXCH_NETIF_GDMA_FROM_CPU_MAX_PORTS_CNS];
}CPSS_DXCH_NETIF_GDMA_FROM_CPU_INFO_STC;

/**
* @struct CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC
 *
 * @brief A Structure with info about ALL CPU ports for 'TO_CPU' , 'FROM_CPU'
 *
*/
typedef struct{
    /**@brief : info about all CPU ports that used for 'TO_CPU' */
    CPSS_DXCH_NETIF_GDMA_TO_CPU_INFO_STC   toCpuInfo;

    /**@brief : info about all CPU ports that used for 'FROM_CPU' */
    CPSS_DXCH_NETIF_GDMA_FROM_CPU_INFO_STC fromCpuInfo;
}CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC;


/**
* @internal cpssDxChNetIfGdmaCpuPortsInfoGet function
* @endinternal
*
* @brief  The function returns information about ALL the CPU ports that used for :
*       'TO_CPU' and the 'FROM_CPU'
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number.
*
* @param[out] infoPtr              - (pointer to) info about the number ports and queues for 'TO_CPU','FROM_CPU'
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChNetIfGdmaCpuPortsInfoGet
(
    IN GT_U8                        devNum,
    OUT CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC *infoPtr
);


#ifdef __cplusplus
}
#endif

#endif  /* __cpssDxChNetIfGdmah */

