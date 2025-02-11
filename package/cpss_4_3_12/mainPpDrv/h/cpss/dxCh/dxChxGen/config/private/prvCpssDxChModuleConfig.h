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
* @file prvCpssDxChModuleConfig.h
*
* @brief This file includes the declaration of the Module configuration
* parameters such as tables addresses , num entries , entries size ,
* entries offset
*
* @version   15
********************************************************************************
*/
#ifndef __prvCpssDxChModuleConfigh
#define __prvCpssDxChModuleConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

/**
* @struct PRV_CPSS_DXCH_MODULE_CONFIG_STC
 *
 * @brief Includes configuration parameters for initializing port group modules.
*/
typedef struct
{
    /*
     * typedef: struct auCfg
     *
     * Description: Configuration structure for the Au management.
     *              Includes the blocks from which the AU descriptors should be
     *              allocated.
     *              --Note about 'per port group' :
     *                  those parameters not used beyond the init stage (phase2,ppLogicalInit)
     *                  so no need to hold here 'per port group' info.
     *                  only the global parameters from the application saved here.
     *                  instead used in runtime : PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]
     * Fields:
     *  auDescBlock     - The block from which the Au descriptors should be
     *                    allocated from (a none-cached memory block).
     *  auDescBlockSize - Size of auDescBlock (in Bytes).
     *
     */
    struct
    {
        GT_U8   *auDescBlock;
        GT_U32  auDescBlockSize;
    }auCfg;

    /*
     * typedef: struct fuCfg
     *
     * Description: Configuration structure for the Fu management.
     *              Includes the blocks from which the FU descriptors should be
     *              allocated.
     *              --Note about 'per port group' :
     *                  those parameters not used beyond the init stage (phase2,ppLogicalInit)
     *                  so no need to hold here 'per port group' info.
     *                  only the global parameters from the application saved here.
     *                  instead used in runtime : PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId]
     *
     * Fields:
     *  fuDescBlock      - The block from which the Fu descriptors should be
     *                     allocated from (an uncached memory block).
     *  fuDescBlockSize  - Size of fuDescBlock (in Bytes).
     *
     */
    struct
    {
        GT_U8   *fuDescBlock;
        GT_U32  fuDescBlockSize;
    }fuCfg;

    /*    fuqUseSeparate   FDB/CNC upload queue type
    */
    CPSS_PP_PHASE2_UPLOAD_QUEUE_TYPE_ENT   fuqUseSeparate;

    /*
     * typedef: struct netIfCfg
     *
     * Description: Configuration structure for Network interface management.
     *                    Relevant only if useMultiNetIfSdma == GT_FALSE.
     *
     * Fields:
     *  txDescBlock     - Pointer to a block of memory to be used for allocating
     *                    Tx descriptor structures.
     *  txDescBlockSize - The raw size in bytes of txDescBlock memory.
     *  rxDescBlock     - Pointer to a block memory to be used for allocating Rx
     *                    description structures.
     *  rxDescBlockSize - The raw size in byte of rxDescBlock.
     *  rxBufInfo       - Rx buffers allocation information.
     *
     */
    struct
    {
        GT_U32  *txDescBlock;
        GT_U32   txDescBlockSize;
        GT_U32  *rxDescBlock;
        GT_U32   rxDescBlockSize;
        CPSS_RX_BUF_INFO_STC rxBufInfo;
    }netIfCfg;

    /* useMultiNetIfSdma - Enable Multi-Group network interface configurations.*/
    GT_BOOL useMultiNetIfSdma;

    /*    multiNetIfCfg - Multi-Group related network interface configuration
     *                    parameters.
     *                    Relevant only if useMultiNetIfSdma == GT_TRUE.
     */
    CPSS_MULTI_NET_IF_CFG_STC   multiNetIfCfg;

    /*
     * typedef: struct ppEventsCfg[]
     *
     * Description: 'Per port group' Configuration structure for the interrupts unit.
     *
     * Fields:
     *  intVecNum       - The interrupt vector number this PP is connected to.
     *  intMask         - The interrupt mask to enable PP interrupts.
     */
    struct
    {
        GT_U32      intVecNum;
        GT_UINTPTR  intMask;
    }ppEventsCfg[CPSS_MAX_PORT_GROUPS_CNS];

    /*
     * struct : ip
     *
     * Description: ip values
     *
     * Fields:
     *      routingMode - PP routing mode: Policy based routing Or TCAM based Routing
     *      sharedTableMode    - pp shared table mode
     *      maxNumOfPbrEntries - max number of LPM leafs that can be allocated
     *                           in the RAM for policy based routing
     *                           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *      numOfBigBanks   - number of LPM shared memory banks used for LPM.
     *                           (APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman)
     *      numOfSmallBanks - number of LPM dedicated memory banks used for LPM.
     *                           (APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman)
     *
     *      lpmSharedMemoryBankNumber - number of banks dedicated for LPM in shared memory
     *                                  (APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman)
     *      lpmBankSize               - lpm bank size (APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman)
     *      numOfBigPbrBanks          - number of big banks taken by PBR
     *      numOfSmallPbrBanks        - number of small banks taken by PBR
     *      pbrBankSize               - max size of each PBR bank
     *      numberOfPbrLines          - number of lines that hold all PBR entries
     *      lastBankIdConfigured      - the value of the last bank configured.
     *                                  sip7 new lpm memory read/write method is
     *                                  to set the bankIndex we want to read/write from/to.
     *                                  this parameter is the last bankId set to hw.
     *                                  by defalut the value will be 0xFFF,
     *                                  meaning the parameter was not set yet.
     *
    */
    struct
    {
        CPSS_DXCH_CFG_ROUTING_MODE_ENT routingMode;
        CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT sharedTableMode;
        GT_U32                         maxNumOfPbrEntries;
        GT_U32                         numOfBigBanks;
        GT_U32                         numOfSmallBanks;
        GT_U32                         lpmSharedMemoryBankNumber;
        GT_U32                         lpmBankSize;
        GT_U32                         numOfBigPbrBanks;
        GT_U32                         numOfSmallPbrBanks;
        GT_U32                         pbrBankSize;
        GT_U32                         numberOfPbrLines;
        GT_U32                         lastBankIdConfigured;

    }ip;

    /*    useDoubleAuq   - Support configuration of two AUQ memory regions.
     *                     GT_TRUE - CPSS manages two AU Queues with the same size:
     *                     auqCfg->auDescBlockSize / 2.
     *                     GT_FALSE - CPSS manages single AU Queue with size:
     *                     auqCfg->auDescBlockSize.
     */
    GT_BOOL useDoubleAuq;

    /* indication that cpssDxChCfgPpLogicalInit function was called and successfully finished */
    /* was by sip6 devices for indication that the shared tables partition was done */
    GT_BOOL wasDone_cpssDxChCfgPpLogicalInit;

    /* resources used for configuration of PP address windows            */
    /* used to reserve address windows for Firmware usage                */
    /* and to protect windows used for already configured address ranges */
    struct {
        /* Reseved windows - set before first range configuration */
        /* bitmap of Reserved MG Unit Address Windows indexes */
        GT_U32 dmaMG_AddressWindowsReservedBmp;
        /* bitmap of Reserved Output Address Translation table Indexes */
        GT_U32 dmaOATU_AddressWindowsReservedBmp;

        /* first range configuration switches 0 to 1 */
        GT_U32 initDone;

        /* set at first range configuration to check 2G decoder window */
        GT_U32 firstRangeBaseLow;

        /* set at first range configuration    */
        /* updated at each range configuration */
        /* bitmap Free of MG Unit Address Windows indexes */
        GT_U32 dmaMG_AddressWindowsFreeBmp;
        /* bitmap Free of Output Address Translation table Indexes */
        GT_U32 dmaOATU_AddressWindowsFreeBmp;
    } addrWindowsResources;

}PRV_CPSS_DXCH_MODULE_CONFIG_STC;

/**
* @internal prvCpssDxChTtiDbTablesInit function
* @endinternal
*
* @brief   init TTI tables internal DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS  prvCpssDxChTtiDbTablesInit
(
    IN GT_U8   devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChModuleConfigh */

