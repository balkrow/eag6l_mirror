
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
* @file prvCpssDxChPortDpAas.c
*
* @brief CPSS implementation for DP and resources configuration.
*        PB GPC Packet Read, TX DMA, TX FIFO, PCA Units, EPI MIF.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortDpAas.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/* array address and size */
#define ARRAY_ADDR_AND_SIZE(_arr) _arr, (sizeof(_arr) / sizeof(_arr[0]))

/* included debug functions */
#define INC_DEBUG_FUNCTIONS

/* copied from prvCpssDxChPortTxPizzaResourceHawk.c and must be the same */
#define NOT_VALID_CNS 0xFFFFFFFF

#define DP_LOOPBACK_CHANNEL              AAS_PORTS_PER_DP_CNS    /*20*/
#define DP_CPU_CHANNEL                  (AAS_PORTS_PER_DP_CNS+1) /*21*/
#define DP_SDMA_CHANNEL                 (AAS_PORTS_PER_DP_CNS+2) /*22*/
#define DP_CPU_CHANNEL_DEFAULT_SPEED               25000
#define SFF_RESOURCE_SIZE                              4
#define SFF_RESOURCE_NUM_OF                           22
#define SFF_RESOURCE_NUM_FOR_GROUPS                   16
#define SFF_MAX_LL_NEXT_POINTERS                     128
#define BRG_UX_UNPACK_RESOURCE_SIZE                    5
#define BRG_SHM_EGR_RP_CREDITS_MUL                     4
#define BRG_SHM_INGR_LOOPBACK_CARRIER_CHANNEL         20
#define BRG_SHM_INGR_CARRIER_CHANNEL                   0
#define PCA_ARBITER_SLOTS_NUM_OF                     170
#define PCA_ARBITER_CHANNELS_NUM_OF                   22
#define PCA_ARBITER_SLOT_DEFAULT_VAL                 0x1F
#define PCA_ARBITER_MAX_RESOURCE_AS_CHANNNEL_SPEED   50000
#define PB_GPR_SHAPER_MIN_BW                         25000

/* record of array sorted by key */
typedef struct
{
    GT_U32 key; /*0xFFFFFFFF* end of Arr with default value */
    GT_U32 val;
} KEY_TO_VAL_STC;

/* speedMbPerSec -> SFF_depth for keyToValConvert*/
/* Depth mesured in resources                     */
static const KEY_TO_VAL_STC prvCpssDxChPortDpAasSffDepthArr[] =
{
    { 99999,         1},
    {100000,         2},
    {200000,         4},
    {400000,         8},
    {800000,        16},
    {0xFFFFFFFF,    16}
};
/*Brg Tx Unpack Unit Mif Credits for keyToValConvert*/
static const KEY_TO_VAL_STC prvCpssDxChPortDpAasBrgTxUnpackMifCreditsArr[] =
{
    { 49999,         11},
    { 50000,         14},
    {100000,         28},
    {200000,         56},
    {400000,        112},
    {800000,        224},
    {0xFFFFFFFF,    224}
};

/* RX DMA Minimal CutThrough packet bytecount */
static const KEY_TO_VAL_STC prvCpssDxChPortDpAasRxMinCtBytecountArr[] =
{
    {  1000,  513},
    {  2500,  385},
    { 10000,  257},
    { 0xFFFFFFFF,  256}
};

/* record of array down sorted by speed          */
/* slow channels (< 100G) mapped to one resource */
/* with the same index as channel                */
typedef struct
{
    GT_U32 speed;
    GT_U32 channel;
    GT_U32 resourcesBmp; /*0 end of Arr mark */
} FAST_CHANNEL_RESOURCES_STC;

/*****************************************************************************************************/
/* PCA Arbiter data */
/* The same resources allocation used for PCA arbiter, PB_GPR arbiter and TX_DMA arbiter */
/* Static arbiter resources - 170 slots 850G BW */
/* Resources 0-15 50G - 10 slots                */
/* Resources 16-21 25G - 5 slots                */
/* Channels 0-19 with speed 50000 and less gets one resource with its index */
/* Channel 20 - Loopback yet not supported */
/* Channels 21 - network CPU port and channel 22 - supported 25G and 50G */
static const GT_U8 prvCpssDxChPortDpAasPcaArbiterResourcesSlots[PCA_ARBITER_SLOTS_NUM_OF] =
{
  /* 0,  1,  2,  3, 4,  5, 6,  7, 8, 9, 10, 11, 12,  13,14,  15,  16 */
     0,  8,  4, 12, 2, 10, 6, 14, 1, 9,  5, 13,  3,  11, 7,  15,  20,
     0,  8,  4, 16, 2, 10, 6, 18, 1, 9,  5, 17,  3,  11, 7,  19,  21,
     0,  8,  4, 12, 2, 10, 6, 14, 1, 9,  5, 13,  3,  11, 7,  15,  20,
     0,  8,  4, 16, 2, 10, 6, 18, 1, 9,  5, 17,  3,  11, 7,  19,  21,
     0,  8,  4, 12, 2, 10, 6, 14, 1, 9,  5, 13,  3,  11, 7,  15,  20,
     0,  8,  4, 16, 2, 10, 6, 18, 1, 9,  5, 17,  3,  11, 7,  19,  21,
     0,  8,  4, 12, 2, 10, 6, 14, 1, 9,  5, 13,  3,  11, 7,  15,  20,
     0,  8,  4, 16, 2, 10, 6, 18, 1, 9,  5, 17,  3,  11, 7,  19,  21,
     0,  8,  4, 12, 2, 10, 6, 14, 1, 9,  5, 13,  3,  11, 7,  15,  20,
     0,  8,  4, 16, 2, 10, 6, 18, 1, 9,  5, 17,  3,  11, 7,  19,  21
};

static const FAST_CHANNEL_RESOURCES_STC prvCpssDxChPortDpAasPcaArbiterChannelResources[] =
{  /* speed,channel,resourcesBmp */
     {800000,  0, 0x000FFFFF} /*0-19*/
    ,{400000,  0, 0x000000FF} /*0-7*/
    ,{400000,  8, 0x000FFF00} /*8-19*/
    ,{200000,  0, 0x0000000F} /*0-3*/
    ,{200000,  4, 0x000000F0} /*4-7*/
    ,{200000,  8, 0x00000F00} /*8-11*/
    ,{200000, 12, 0x000FF000} /*12-19*/
    ,{100000,  0, 0x00000003} /*0-1*/
    ,{100000,  2, 0x0000000C} /*2-3*/
    ,{100000,  4, 0x00000030} /*4-5*/
    ,{100000,  6, 0x000000C0} /*6-7*/
    ,{100000,  8, 0x00000300} /*8-9*/
    ,{100000, 10, 0x00000C00} /*10-11*/
    ,{100000, 12, 0x00033000} /*12-13,16-17*/
    ,{100000, 14, 0x000CC000} /*14-15,18-19*/
    /* Special DMA ports */
    ,{ 50000, 20, 0x00300000} /*20*/    /* LOOPBACK 50G */
    ,{ 50000, 21, 0x00300000} /*20-21*/ /* MAC_CPU 50G */
    ,{ 50000, 22, 0x00300000} /*20-21*/ /* SDMA 50G */
    ,{ 25000, 20, 0x00100000} /*20*/    /* LOOPBACK 25G */
    ,{ 25000, 21, 0x00100000} /*20*/    /* MAC_CPU 25G */
    ,{ 25000, 22, 0x00200000} /*21*/    /* SDMA 25G */
    ,{0,  0, 0}
};

/* DP TX FIFO profiles  for keyToValConvert*/
static const KEY_TO_VAL_STC prvCpssDxChPortDpTxFifoSpeedToProfileArr[] =
{
    {  1000,        10}, /* 1G and less*/
    {  2500,         9},
    {  5000,         8},
    { 10000,         7},
    { 20000,         6},
    { 25000,         5},
    { 40000,         4},
    { 50000,         3},
    {100000,         2},
    {200000,         1},
    {400000,         0}, /* 400G and more*/
    {0xFFFFFFFF,     0}
};

/* DP TX DMA profiles  for keyToValConvert*/
static const KEY_TO_VAL_STC prvCpssDxChPortDpTxDmaSpeedToProfileArr[] =
{
    {  1000,        10}, /* 1G and less*/
    {  2500,         9},
    {  5000,         8},
    { 10000,         7},
    { 12000,         6},
    { 25000,         5},
    { 50000,         4},
    {100000,         3},
    {200000,         2},
    {400000,         1},
    {800000,         0}, /* 800G and more*/
    {0xFFFFFFFF,     0}
};

/* PB GPR profiles  for keyToValConvert*/
static const KEY_TO_VAL_STC prvCpssDxChPortDpPbGprSpeedToProfileArr[] =
{
    {  1000,         9}, /* 1G and less*/
    {  2500,         8},
    {  5000,         7},
    { 10000,         6},
    { 25000,         5},
    { 50000,         4},
    {100000,         3},
    {200000,         2},
    {400000,         1},
    {800000,         0}, /* 800G and more*/
    {0xFFFFFFFF,     0}
};

/*****************************************************************************************************/
/* MIF data */

/* AAS PCA has 4 DPs per tile                                            */
/* DP0-2 are regulal and DP3 can be reglar or FlexE                      */
/* Each DP has MIF0_8P and MIF1_8P units for 8 channels                  */
/* and MIF_4P unit for 4 channels                                        */
/* The MIF_4P unit for FlexE DP presents, but not used                   */
/* Global MIF units per tile are 1*MIF_CPU and 4*MIF_80P units for FlexE */
/* in Reg DB unit indexes are                                            */
/* per DP - MIF0_8P - 0,3,6,9. MIF1_8P - 1,4,7,10. MIF_4P - 2,5,8,11.    */
/* global - MIF_CPU - 12. MIF_80P - 13,14,15,16.                         */

/* The FlexE not supported yet */

/* Mapping cases of 8-serdes MAC clusters:
Serdes  Mode            MIF index   MAC index
0-7     R8                      8           0
0-3     R4                      0           0
4-7     R4                      4           4
0-1     R2                      0           0
2-3     R2                      2           2
4-5     R2                      4           4
6-7     R2                      6           6
0       R1                      0           0
1       R1                      1           1
2       R1                      2           2
3       R1                      3           3
4       R1                      4           4
5       R1                      5           5
6       R1                      6           6
7       R1                      7           7
1       R1 express              0           1
1       R1 preemptive           1           1
3       R1 express              2           3
3       R1 preemptive           3           3
5       R1 express              4           5
5       R1 preemptive           5           5
7       R1 express              6           7
7       R1 preemptive           7           7
*/

typedef struct
{
    GT_U32 numOfSerdes;            /* 0 - end of array mark */
    GT_U8  localMacChannelArr[16]; /* 255 - not applicable, 254 - end of list */
} MIF_CHANNEL_STC;

typedef struct
{
    GT_U32                                    dataPath;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionRole;
    GT_U32                                    globalMifNum; /* relevant only for global */
    const MIF_CHANNEL_STC                     *mifChannelArrPtr; /*NULL - end of array mark*/
} DP_MIF_UNIT_STC;

static const MIF_CHANNEL_STC dmaInMifArr0[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {  0,   1,   2,   3,   4,   5,   6,   7, 254, 255, 255, 255, 255, 255, 255, 255}},
    {2,   {  0, 255,   2, 255,   4, 255,   6, 255, 254, 255, 255, 255, 255, 255, 255, 255}},
    {4,   {  0, 255, 255, 255,   4, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}},
    {8,   {255, 255, 255, 255, 255, 255, 255, 255,   0, 254, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaInMifArr1[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {  8,   9,  10,  11,  12,  13,  14,  15, 254, 255, 255, 255, 255, 255, 255, 255}},
    {2,   {  8, 255,  10, 255,  12, 255,  14, 255, 254, 255, 255, 255, 255, 255, 255, 255}},
    {4,   {  8, 255, 255, 255,  12, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255}},
    {8,   {255, 255, 255, 255, 255, 255, 255, 255,   8, 254, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaInMifArr_4P[] =
{/*{numOfSerdes, { localMacChannelArr[mifChannel]=macChannel|254|255} } */
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 <-mifChannel*/
    {1,   { 16,  17,  18,  19,  20, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaInMifArr_CPU[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   { 21, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};

static const MIF_CHANNEL_STC dmaExpressInMifArr0[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {  1, 255,   3, 255,   5, 255,   7, 254, 255, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaPreemptiveInMifArr0[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {255,   1, 255,   3, 255,   5, 255,   7, 254, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaExpressInMifArr1[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {  9, 255,  11, 255,  13, 255,  15, 254, 255, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const MIF_CHANNEL_STC dmaPreemptiveInMifArr1[] =
{
    /*       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15 */
    {1,   {255,   9, 255,  11, 255,  13, 255,  15, 254, 255, 255, 255, 255, 255, 255, 255}},
    {0,   {0}}
};
static const DP_MIF_UNIT_STC dpRegularMifUnitsArr[] =
{/* dpNo,         preemptionRole             globalMifNum,  mifChannelArrPtr */
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   0,  dmaInMifArr0},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   1,  dmaInMifArr1},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   2,  dmaInMifArr_4P},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   3,  dmaInMifArr0},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   4,  dmaInMifArr1},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   5,  dmaInMifArr_4P},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   6,  dmaInMifArr0},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   7,  dmaInMifArr1},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   8,  dmaInMifArr_4P},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,   9,  dmaInMifArr0},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,  10,  dmaInMifArr1},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,  11,  dmaInMifArr_4P},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E,  12,  dmaInMifArr_CPU},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    0,  dmaExpressInMifArr0},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    0,  dmaPreemptiveInMifArr0},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    1,  dmaExpressInMifArr1},
    {0, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    1,  dmaPreemptiveInMifArr1},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    3,  dmaExpressInMifArr0},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    3,  dmaPreemptiveInMifArr0},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    4,  dmaExpressInMifArr1},
    {1, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    4,  dmaPreemptiveInMifArr1},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    6,  dmaExpressInMifArr0},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    6,  dmaPreemptiveInMifArr0},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    7,  dmaExpressInMifArr1},
    {2, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    7,  dmaPreemptiveInMifArr1},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,    9,  dmaExpressInMifArr0},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,    9,  dmaPreemptiveInMifArr0},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E,   10,  dmaExpressInMifArr1},
    {3, PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E,   10,  dmaPreemptiveInMifArr1},
    {0,   0,   0,  NULL}
};

/**
* @internal keyToValConvert function
* @endinternal
*
* @brief    search in array by key the first "array_key >= parameter_key" match.
*
* @param[in] searchArr              - search Arr
* @param[in] key                    - key to search
*
* @retval - last matched found value, 0 if not found
*/
static GT_U32 keyToValConvert
(
    IN const KEY_TO_VAL_STC *searchArr,
    IN GT_U32         key
)
{
    GT_U32 ii;

    /* skip all least */
    for (ii = 0; (searchArr[ii].key < key); ii++) {};
    /* first more or equal */
    return searchArr[ii].val;
}

/**
* @internal prvCpssDxChPortAasPortLoopbackOnDpGet function
* @endinternal
*
* @brief    search global Loopback portNum on a given DataPath number.
*
* @param[in] devNum                 - device number
* @param[in] dataPath               - PCA data path (1..3)
* @param[out] portNumPtr            - resulting portNum if found
*
* @retval - GT_OK if found, else GT_NOT_FOUND
*/
GT_STATUS prvCpssDxChPortAasPortLoopbackOnDpGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      dpNum,
    OUT GT_U32      *portNumPtr
)
{
    GT_U32                          ii;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *mapInfo;

    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    for (ii = 0; ii < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; ii++)
    {
        mapInfo = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[ii];
        if (mapInfo->valid &&
           (mapInfo->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E) &&
           (mapInfo->extPortMap.globalDp == dpNum))
        {
            *portNumPtr = ii;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal channelSpeedToResourcesBitmap function
* @endinternal
*
* @brief    search in array by channel and speed the bitmap of arbiter resources.
*
* @param[in] searchArr                   - search Arr of resource bitmaps
* @param[in] maxResourceAsChannelSpeed   - maximal speed in kilo bits per second to use one
*                                          resource with channel index
* @param[in] notResourceAsChannelChanBmp - bitmap of channels excluded from
*                                          "resource with channel index" algorithm
* @param[in] channel                     - channel number
* @param[in] speedInMbps                 - speed In Mbps
*
* @retval - bitmap of arbiter resources, 0 if not found
*/
static GT_U32 channelSpeedToResourcesBitmap
(
    IN const FAST_CHANNEL_RESOURCES_STC *searchResourcesBmpArr,
    IN GT_U32         maxResourceAsChannelSpeed,
    IN GT_U32         notResourceAsChannelChanBmp,
    IN GT_U32         channel,
    IN GT_U32         speedInMbps
)
{
    GT_U32 ii;
    GT_U32 lastMatch;

    if ((speedInMbps <= maxResourceAsChannelSpeed)
        && (((1 << channel) & notResourceAsChannelChanBmp) == 0))
    {
        return (1 << channel);
    }
    lastMatch = 0xFFFFFFFF;
    for (ii = 0;
         (searchResourcesBmpArr[ii].resourcesBmp != 0);
        ii++)
    {
        if (searchResourcesBmpArr[ii].speed < speedInMbps) break;
        if (searchResourcesBmpArr[ii].channel != channel) continue;
        lastMatch = ii;
    }
    if (lastMatch == 0xFFFFFFFF)
    {
        return 0;
    }
    return searchResourcesBmpArr[lastMatch].resourcesBmp;
}


/**
* @internal prvCpssDxChPortAasMacToMif function
* @endinternal
*
* @brief    Convert Local port to MIF unit and channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] dataPath               - PCA data path (1..3)
* @param[in] localMacInDp           - local MAC number (in DP)
* @param[in] numOfSerdes            - num of serdes (as port mode)
* @param[in] preemptionRole         - Preemption Role (regular, express, preemptive)
* @param[out] mifUnitNumPtr         - (pointer to)MIF unit number (global)
* @param[out] mifLocalChannelNumPtr - (pointer to)MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_NOT FOUND             - on not mapable MAC unit
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChPortAasMacToMif
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    dataPath,
    IN  GT_U32                                    localMacInDp,
    IN  GT_U32                                    numOfSerdes,
    IN  PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionRole,
    OUT GT_U32                                    *mifUnitNumPtr, /*mif unit index in regs DB*/
    OUT GT_U32                                    *mifLocalChannelNumPtr
)
{
    const DP_MIF_UNIT_STC *mifUnitPtr;
    const MIF_CHANNEL_STC *mifChannelPtr;
    GT_U32          mifChannel;

    devNum = devNum; /* avoid compiler warning */
    if (dataPath > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (mifUnitPtr = dpRegularMifUnitsArr; (mifUnitPtr->mifChannelArrPtr != NULL); mifUnitPtr++)
    {
        if (mifUnitPtr->dataPath != dataPath) continue;
        if (mifUnitPtr->preemptionRole != preemptionRole) continue;
        for (mifChannelPtr = mifUnitPtr->mifChannelArrPtr;
              (mifChannelPtr->numOfSerdes != 0);
              mifChannelPtr++)
        {
            if (mifChannelPtr->numOfSerdes != numOfSerdes) continue;
            for (mifChannel = 0; (mifChannel < 16); mifChannel++)
            {
                if (mifChannelPtr->localMacChannelArr[mifChannel] == 254) break;
                if (mifChannelPtr->localMacChannelArr[mifChannel] == localMacInDp)
                {
                    *mifLocalChannelNumPtr = mifChannel;
                    *mifUnitNumPtr = mifUnitPtr->globalMifNum;
                    return GT_OK;
                }
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChPortAasPortInfoGet function
* @endinternal
*
* @brief   Gets port info.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
* @param[out] portInfoPtr           - (pointer to)  port info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortAasPortInfoGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     portSpeed,
    IN  GT_BOOL                                 isLoopback,
    OUT PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_SHADOW_PORT_MAP_STC detailedPortMap;
    GT_U32    speedInMbPerSec;
    GT_U32    numRegularPorts;
    GT_U32    numOfDps;
    GT_U32    regularPortsPerDp;
    GT_U32    numOfSerdes;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionRole;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E |
      CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);
    cpssOsMemSet(portInfoPtr, 0, sizeof(*portInfoPtr));

    rc = prvCpssDxChPortPhysicalPortMapGet(devNum, portNum, &detailedPortMap);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert( /* get dpNum and localDma */
        devNum, detailedPortMap.txDmaNum, &(portInfoPtr->dataPath), &(portInfoPtr->localDma));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    speedInMbPerSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    portInfoPtr->speedInMbPerSec = speedInMbPerSec;
    portInfoPtr->resourceChannel = portInfoPtr->localDma; /* save localDma BEFORE update */

    /* If(Loopback on ETH port) swap portNum to lpPortNum and update info */
    if ((detailedPortMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E) &&
        (isLoopback == GT_TRUE))
    {
        /* Get Loopback's portNum instead an original ETH portNum */
        rc = prvCpssDxChPortAasPortLoopbackOnDpGet(devNum, portInfoPtr->dataPath, &portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChPortPhysicalPortMapGet(devNum, portNum, &detailedPortMap);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        portInfoPtr->localDma = DP_LOOPBACK_CHANNEL;
    }
    /* Save AFTER update */
    portInfoPtr->portNum = portNum;
    portInfoPtr->globalDma = detailedPortMap.txDmaNum;
    portInfoPtr->isMac =
        ((detailedPortMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E) ||
         (detailedPortMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E))
            ? 0 : 1;

    if (portInfoPtr->isMac == 0)
    {
        return GT_OK;
    }

    /* ----- MAC is present and needs configurations --------------------- */
    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
    numOfDps = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
    regularPortsPerDp = numRegularPorts / numOfDps;

    /* SerdesUsageUnit is 1/256 of one serdes lane - used alse for USX using same serdes for seceral ports */
    numOfSerdes = prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert(ifMode) / 256;
    portInfoPtr->numOfSerdes = numOfSerdes;

    /*check if preemption was enabled for the port .
            The assumption is that Port manager enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum, speedInMbPerSec, &(portInfoPtr->isPreemtionEnabled));
    if(rc != GT_OK)
    {
         return rc;
    }

    if (GT_FALSE != portInfoPtr->isPreemtionEnabled)
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, speedInMbPerSec, portInfoPtr->localDma, &(portInfoPtr->localDmaPreemtive));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, portInfoPtr->dataPath, portInfoPtr->localDmaPreemtive,
            &(portInfoPtr->globalDmaPreemtive));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    portInfoPtr->globalMac = detailedPortMap.macNum;
    if (portInfoPtr->globalMac < numRegularPorts)
    {
        portInfoPtr->localMac = portInfoPtr->globalMac % regularPortsPerDp;
    }
    else
    {
        /* CPU network port or Loopback - no serdes, no global number */
        portInfoPtr->localMac =
            (detailedPortMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E) ?
                DP_LOOPBACK_CHANNEL : DP_CPU_CHANNEL;
    }

    preemptionRole = (GT_FALSE != portInfoPtr->isPreemtionEnabled)
        ? PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E
        : PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_NONE_E;
    rc = prvCpssDxChPortAasMacToMif(
        devNum, portInfoPtr->dataPath, portInfoPtr->localMac,
        numOfSerdes, preemptionRole,
        &(portInfoPtr->mifUnit), &(portInfoPtr->localMif));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE != portInfoPtr->isPreemtionEnabled)
    {
        preemptionRole = PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E;
        rc = prvCpssDxChPortAasMacToMif(
            devNum, portInfoPtr->dataPath, portInfoPtr->localMac,
            numOfSerdes, preemptionRole,
            &(portInfoPtr->mifUnit), &(portInfoPtr->localMifPreemtive));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasChannelPerGlobalPortConvert function
* @endinternal
*
* @brief    TX DMA, TX_FIFO, RX_DMA recalculate channel unit and index acccess in registers DB.
*           Used for registers reordered to be per global MAC in DB unit0.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                 - device number
* @param[in]  unitlNum               - unit number
* @param[in]  channel                - channel number
* @param[out] dbUnitNumPtr           - (pointer to) REG DB access unit number
* @param[out] dbChannelPtr           - (pointer to) REG DB access hannel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasChannelPerGlobalPortConvert
(
    IN    GT_U8      devNum,
    IN    GT_U32     unitNum,
    IN    GT_U32     channel,
    OUT   GT_U32     *dbUnitNumPtr,
    OUT   GT_U32     *dbChannelPtr
)
{
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr;
    GT_U32 dp0RegDbLocalChannels;

    /* default*/
    *dbUnitNumPtr = unitNum;
    *dbChannelPtr = channel;
    if (unitNum != 0)
    {
        return GT_OK;
    }

    dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    dp0RegDbLocalChannels =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts /
            dxDevPtr->hwInfo.multiDataPath.maxDp;
    if (channel < dp0RegDbLocalChannels)
    {
        return GT_OK;
    }

    /* take from DB unit0 */
    *dbUnitNumPtr = 0;
    /* get the global DMA of the 'special DMAs' */
    return prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
        devNum, unitNum, channel, dbChannelPtr/*globalDmaNum*/);
}

/**
 * @struct MIF_MAP_REG_FIELDS_STC
 *
 *  @brief This structure contains fields of MIF Map register.
 */
typedef struct
{
    /** channel clock enable (1) or disable (0)   */
    GT_U32                 clockEnable;
    /** RX DMA channel global number              */
    GT_U32                 rxDmaChannelNum;
    /** RX DMA channel enable (1) or disable (0)  */
    GT_U32                 rxDmaChannelEnable;
    /** RX MIF PFC enable (1) or disable (0)      */
    GT_U32                 rxMifPfcEnable;
    /** TX DMA channel global number              */
    GT_U32                 txDmaChannelNum;
    /** TX DMA channel enable (1) or disable (0)  */
    GT_U32                 txDmaChannelEnable;
    /** TX MIF PFC enable (1) or disable (0)      */
    GT_U32                 txMifPfcEnable;
    /** remote Port Id                            */
    GT_U32                 remotePortId;
} MIF_MAP_REG_FIELDS_STC;

/**
* @internal prvCpssDxChPortDpAasMifChannelMapGenericSet function
* @endinternal
*
* @brief    Set channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] fieldsPtr              - (Pointer to)MIF Map registed fields structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelMapGenericSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    MIF_MAP_REG_FIELDS_STC *fieldsPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    const GT_U32 dmaChannelMask = 0x3F;
    const GT_U32 remotePortIdMask = 0x7F;
    GT_U32 regMask;
    GT_U32 regData;
    GT_U32 data;

    regMask =
        (1 << 6)/*txEnable*/ | (1 << 14)/*rxEnable*/ | (1 << 18)/*clock*/
        | (1 << 7)/*txMifPfcEnable*/ |  (1 << 15)/*rxMifPfcEnable*/
        | dmaChannelMask/*txDmaChannelNum*/ | (dmaChannelMask << 8)/*rxDmaChannelNum*/
        | (remotePortIdMask << 24);
    regData =
        (fieldsPtr->txDmaChannelEnable << 6)/*txEnable*/ |
        (fieldsPtr->rxDmaChannelEnable << 14)/*rxEnable*/ |
        (fieldsPtr->clockEnable << 18)/*clock*/ |
        (fieldsPtr->txMifPfcEnable << 7)/*txMifPfcEnable*/ |
        (fieldsPtr->rxMifPfcEnable << 15)/*rxMifPfcEnable*/ |
        fieldsPtr->txDmaChannelNum/*txDmaChannelNum*/ |
        (fieldsPtr->rxDmaChannelNum << 8) /*rxDmaChannelNum*/ |
        (fieldsPtr->remotePortId << 24);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((data & regMask) == regData)
    {
        return GT_OK;
    }

    data = ((data & (~ regMask)) | regData);

    return prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
}

/**
* @internal prvCpssDxChPortDpAasMifChannelMapGenericGet function
* @endinternal
*
* @brief    Get channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[out] fieldsPtr             - (Pointer to)MIF Map registed fields structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelMapGenericGet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    OUT   MIF_MAP_REG_FIELDS_STC *fieldsPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    const GT_U32 dmaChannelMask = 0x3F;
    const GT_U32 remotePortIdMask = 0x7F;
    GT_U32 regData;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    fieldsPtr->clockEnable          = (regData >> 18) & 1;
    fieldsPtr->rxDmaChannelNum      = (regData >>  8) & dmaChannelMask;
    fieldsPtr->rxDmaChannelEnable   = (regData >> 14) & 1;
    fieldsPtr->txDmaChannelNum      =  regData        & dmaChannelMask;
    fieldsPtr->txDmaChannelEnable   = (regData >>  6) & 1;
    fieldsPtr->txMifPfcEnable       = (regData >>  7) & 1;
    fieldsPtr->rxMifPfcEnable       = (regData >> 15) & 1;
    fieldsPtr->remotePortId         = (regData >> 24) & remotePortIdMask;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasMifChannelMapSet function
* @endinternal
*
* @brief    Set channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] dmaChannelNum          - DMA channel global number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelMapSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_U32                 dmaChannelNum
)
{
    GT_STATUS rc;
    MIF_MAP_REG_FIELDS_STC fields;

    rc = prvCpssDxChPortDpAasMifChannelMapGenericGet(
        devNum, unitNum, channelNum, &fields);
    if (rc != GT_OK)
    {
        return rc;
    }
    fields.clockEnable        = 1;
    fields.rxDmaChannelEnable = 1;
    fields.rxDmaChannelNum    = dmaChannelNum;
    fields.rxMifPfcEnable     = 1;
    fields.txDmaChannelEnable = 1;
    fields.txDmaChannelNum    = dmaChannelNum;
    fields.txMifPfcEnable     = 1;

    return prvCpssDxChPortDpAasMifChannelMapGenericSet(
        devNum, unitNum, channelNum, &fields);
}

/**
* @internal prvCpssDxChPortDpAasMifChannelMapDisable function
* @endinternal
*
* @brief    Disable channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelMapDisable
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum
)
{
    GT_STATUS rc;
    MIF_MAP_REG_FIELDS_STC fields;

    rc = prvCpssDxChPortDpAasMifChannelMapGenericGet(
        devNum, unitNum, channelNum, &fields);
    if (rc != GT_OK)
    {
        return rc;
    }
    fields.clockEnable        = 0;
    fields.rxDmaChannelEnable = 0;
    fields.rxDmaChannelNum    = 0x3F;
    fields.rxMifPfcEnable     = 0;
    fields.txDmaChannelEnable = 0;
    fields.txDmaChannelNum    = 0x3F;
    fields.txMifPfcEnable     = 0;

    return prvCpssDxChPortDpAasMifChannelMapGenericSet(
        devNum, unitNum, channelNum, &fields);
}

/**
* @internal prvCpssDxChPortDpAasMifChannelPfcEnableGenericSet function
* @endinternal
*
* @brief    Set MIF PFC enable/disable.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] enableTx               - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx               - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelPfcEnableGenericSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_BOOL                enableTx,
    IN    GT_BOOL                enableRx
)
{
    GT_STATUS rc;
    MIF_MAP_REG_FIELDS_STC fields;

    rc = prvCpssDxChPortDpAasMifChannelMapGenericGet(
        devNum, unitNum, channelNum, &fields);
    if (rc != GT_OK)
    {
        return rc;
    }
    fields.rxMifPfcEnable = BOOL2BIT_MAC(enableRx);
    fields.txMifPfcEnable = BOOL2BIT_MAC(enableTx);

    return prvCpssDxChPortDpAasMifChannelMapGenericSet(
        devNum, unitNum, channelNum, &fields);
}

/**
* @internal prvCpssDxChPortDpAasMifChannelPfcEnableGenericGet function
* @endinternal
*
* @brief    Get MIF PFC enable/disable.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpAasMifChannelPfcEnableGenericGet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_BOOL                *enableTxPtr,
    IN    GT_BOOL                *enableRxPtr
)
{
    GT_STATUS rc;
    MIF_MAP_REG_FIELDS_STC fields;

    rc = prvCpssDxChPortDpAasMifChannelMapGenericGet(
        devNum, unitNum, channelNum, &fields);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enableRxPtr = BIT2BOOL_MAC(fields.rxMifPfcEnable);
    *enableTxPtr = BIT2BOOL_MAC(fields.txMifPfcEnable);

    return GT_OK;
}

extern GT_STATUS prvCpssDxChPortMappingRxdmaToLocalDevSrcPortNumSet
(
    IN GT_U8  devNum,
    IN GT_U32 dpIndex,
    IN GT_U32 localDmaNum,
    IN GT_U32 localDevSrcPortNum
);
/*************************************************************************************************************************************/
/*        RX_DMA          */

/**
* @internal prvCpssDxChPortDpAasRxDmaChannelConfigure function
* @endinternal
*
* @brief   Set Min Ct Bytecount, Suorce port, isPreemptive for the given RX Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] dmaUnit               - RX DMA Unit number.
* @param[in] dmaChannel            - RX DMA Unit local channel.
* @param[in] speedInMbPerSec       - Speed In Megabit Per Sec.
* @param[in] sorcePortNum          - source physical port number.
* @param[in] isPreemptive          - GT_TRUE - preemptive, GT_FALSE - other
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on wrong minCtByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvCpssDxChPortDpAasRxDmaChannelConfigure
(
    IN GT_U8                 devNum,
    IN GT_U32                dmaUnit,
    IN GT_U32                dmaChannel,
    IN GT_U32                speedInMbPerSec,
    IN GT_PHYSICAL_PORT_NUM  sorcePortNum,
    IN GT_BOOL               isPreemptive
)
{
    GT_STATUS rc;                /* return code */
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    mask;              /* Mask for selecting the written bits */
    GT_U32    dbUnitNum;
    GT_U32    dbChannel;
    GT_U32    minCtBytecount;

    minCtBytecount = keyToValConvert(
        prvCpssDxChPortDpAasRxMinCtBytecountArr, speedInMbPerSec);
    if (minCtBytecount >= BIT_14)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (sorcePortNum >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDpAasChannelPerGlobalPortConvert(
        devNum, dmaUnit, dmaChannel, &dbUnitNum, &dbChannel);
    if (rc != GT_OK)
    {
        return rc;
    }

    data = (minCtBytecount << 2);
    mask = (0x3FFF << 2);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_rxDMA[dbUnitNum].
        configs.cutThrough.channelCTConfig[dbChannel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, mask, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* the function will set the RxDmaUnit about <sorcePortNum> :
        configs.channelConfig.channelToLocalDevSourcePort[]
    , and HBU for sip7*/
    rc = prvCpssDxChPortMappingRxdmaToLocalDevSrcPortNumSet(
        devNum,dmaUnit, dmaChannel,sorcePortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    data = (BOOL2BIT_MAC(isPreemptive) << 2);
    mask = (1 << 2);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_rxDMA[dbUnitNum].
        configs.channelConfig.channelGeneralConfigs[dbChannel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, mask, data);

    return rc;
}

/*************************************************************************************************************************************/
/*              SFF          */

/**
* @internal prvCpssDxChPortDpAasSffAllocateLlResorces function
* @endinternal
*
* @brief   Allocates SFF Link List Channel resources range in bitmap of used resources.
*          Algorithm used by unit designers - mandatory for PUNCT.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] resourcesBitmapSize       - amounts of resources in bitmap (used and free)
* @param[in] resourcesBitmapPtr        - bitmap of used resources
* @param[in] numOfResourceForGroups    - amounts of first resources used for contiguous groups allocation
* @param[in] numOfResourcesAllocate    - amounts of resources to allocate
* @param[out] allocResourcesStartPtr   - (pointer to) start index of resources range
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no resources
*/
static GT_STATUS prvCpssDxChPortDpAasSffAllocateLlResorcesRange
(
    IN    GT_U32            resourcesBitmapSize,
    IN    GT_U32            *resourcesBitmapPtr,
    IN    GT_U32            numOfResourceForGroups,
    IN    GT_U32            numOfResourcesAllocate,
    OUT   GT_U32            *allocResourcesStartPtr
)
{
    GT_U32 resource;
    GT_U32 ii,jj;
    GT_U32 bound;
    GT_U32 found;

    /* attempt to use algorithm used by HW designers (PUNKT) */
    if (numOfResourcesAllocate == 1)
    {
        /* search from the end */
        for (ii = resourcesBitmapSize; (ii > 0); ii--)
        {
            resource = ii - 1;
            if ((resourcesBitmapPtr[resource / 32] & (1 << (resource % 32))) == 0)
            {
                *allocResourcesStartPtr = resource;
                return GT_OK;
            }
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* search contiguous aligned group of free resources from beginning */
    bound = numOfResourceForGroups - (numOfResourceForGroups % numOfResourcesAllocate);
    for (ii = 0; (ii < bound); ii += numOfResourcesAllocate)
    {
        found = 1;
        for (jj = 0; (jj < numOfResourcesAllocate); jj++)
        {
            resource = ii + jj;
            if ((resourcesBitmapPtr[resource / 32] & (1 << (resource % 32))) == 0)
            {
                found = 0;
                break;
            }
        }
        if (found)
        {
            /*result*/
            *allocResourcesStartPtr = ii;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChPortDpAasSffMarkLlResorcesRange function
* @endinternal
*
* @brief   Add or Remove range of resources to/from bitmap of used resources.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[out] resourcesBitmapPtr - amounts of resources to read
* @param[in] resourcesStart      - start of marked resources range
* @param[in] numOfResources      - amounts of marked resources
* @param[in] markUsed            - GT_TRUE - as used, GT_FALSE - as free
*
* @retval none
*/
void prvCpssDxChPortDpAasSffMarkLlResorcesRange
(
    OUT GT_U32               *resourcesBitmapPtr,
    IN  GT_U32               resourcesStart,
    IN  GT_U32               numOfResources,
    IN  GT_BOOL              markUsed
)
{
    GT_U32 resource;
    GT_U32 lastResource;

    lastResource = resourcesStart + numOfResources - 1;
    if (markUsed == GT_FALSE)
    {
        for (resource = resourcesStart; (resource <= lastResource); resource++)
        {
            resourcesBitmapPtr[resource / 32] &= (~ (1 << (resource % 32)));
        }
    }
    else
    {
        for (resource = resourcesStart; (resource <= lastResource); resource++)
        {
            resourcesBitmapPtr[resource / 32] |= (1 << (resource % 32));
        }
    }
}

/**
* @internal prvCpssDxChPortDpAasSffCheckLlResorcesRange function
* @endinternal
*
* @brief  Check range of resources in bitmap of used resources to be  used or free.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] resourcesBitmapPtr  - amounts of resources to read
* @param[in] resourcesStart      - start of marked resources range
* @param[in] numOfResources      - amounts of marked resources
* @param[in] markedUsedOrFree    - GT_TRUE - as used, GT_FALSE - as free
*
* @retval GT_OK         - check passed
* @retval GT_BAD_STATE  - found not supposed state
*/
GT_STATUS prvCpssDxChPortDpAasSffCheckLlResorcesRange
(
    IN  GT_U32               *resourcesBitmapPtr,
    IN  GT_U32               resourcesStart,
    IN  GT_U32               numOfResources,
    IN  GT_BOOL              markedUsedOrFree
)
{
    GT_U32 resource;
    GT_U32 lastResource;

    lastResource = resourcesStart + numOfResources - 1;
    if (markedUsedOrFree == GT_FALSE)
    {
        for (resource = resourcesStart; (resource <= lastResource); resource++)
        {
            if (resourcesBitmapPtr[resource / 32] & (1 << (resource % 32)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        for (resource = resourcesStart; (resource <= lastResource); resource++)
        {
            if ((resourcesBitmapPtr[resource / 32] & (1 << (resource % 32))) == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasSffDbChannelResorceAllocate function
* @endinternal
*
* @brief    Allocates SFF resource range in DB for channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] unitlNum             - unit number
* @param[in] channel              - channel number
* @param[in] portInfoPtr          - (pointer to)channel info
* @param[in] checkOnly            - GT_TRUE do not set/save allocation results but only check them
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - cannel resources already allocated
* @retval GT_NO_RESOURCE           - on no resources
*/
static GT_STATUS prvCpssDxChPortDpAasSffDbChannelResorceAllocate
(
    IN    GT_U8                                   devNum,
    IN    GT_U32                                  unitNum,
    IN    GT_U32                                  channel,
    IN    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC *portInfoPtr,
    IN    GT_BOOL                                 checkOnly,
    IN    GT_BOOL                                 isLoopback
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC *dpInfoPtr;
    GT_U32 numOfResources;
    GT_U32 resourcesRangeBase;
    GT_U32 lastResource;


    dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[unitNum];
    /* mark as not allocated */
    if (dpInfoPtr->sffChannelResorcesAmountArr[channel] != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    numOfResources = keyToValConvert(
        prvCpssDxChPortDpAasSffDepthArr, portInfoPtr->speedInMbPerSec);

    if (dpInfoPtr->usePunktAlgorithm == GT_TRUE && isLoopback == GT_FALSE)
    {
        rc = prvCpssDxChPortDpAasSffAllocateLlResorcesRange(
            SFF_RESOURCE_NUM_OF,
            dpInfoPtr->sffUsedLlResourcesBitmap,
            SFF_RESOURCE_NUM_FOR_GROUPS,
            numOfResources,
            &resourcesRangeBase);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* use static resource range bases (22 resources for 22 channels)) */
        /* range starts just from resource with the number of the channel  */
        /* if it faster than 100G it covers also the resourced that can be */
        /* avalable for the near channels (when it is slow)                */
        /* channels 0,4,8,12 only can be faster than 100G, but not needed  */
        /* to check it here                                                */
        resourcesRangeBase = channel;
        rc = prvCpssDxChPortDpAasSffCheckLlResorcesRange(
            dpInfoPtr->sffUsedLlResourcesBitmap,
            resourcesRangeBase, numOfResources,
            GT_FALSE /*markUsed*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (checkOnly == GT_TRUE)
    {
        /* LL elements range: PCA_SFF[unitNum].llNextPointer[0..127] */
        lastResource = resourcesRangeBase + numOfResources - 1;
        if ((lastResource * SFF_RESOURCE_SIZE + (SFF_RESOURCE_SIZE - 1)) >= SFF_MAX_LL_NEXT_POINTERS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG); \
        }
        return GT_OK;
    }

    prvCpssDxChPortDpAasSffMarkLlResorcesRange(
        dpInfoPtr->sffUsedLlResourcesBitmap,
        resourcesRangeBase, numOfResources,
        GT_TRUE /*markUsed*/);
    dpInfoPtr->sffChannelResorcesBaseArr[channel]   = (GT_U8)resourcesRangeBase;
    dpInfoPtr->sffChannelResorcesAmountArr[channel] = (GT_U8)numOfResources;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasSffDbChannelResorceFree function
* @endinternal
*
* @brief    Free SFF resource range in DB for channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] unitlNum             - unit number
* @param[in] channel              - channel number
*
* @retval - none
*/
void prvCpssDxChPortDpAasSffDbChannelResorceFree
(
    IN    GT_U8                                   devNum,
    IN    GT_U32                                  unitNum,
    IN    GT_U32                                  channel
)
{
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC *dpInfoPtr;
    GT_U32 numOfResources;
    GT_U32 resourcesRangeBase;

    dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[unitNum];
    numOfResources = dpInfoPtr->sffChannelResorcesAmountArr[channel];
    resourcesRangeBase = dpInfoPtr->sffChannelResorcesBaseArr[channel];
    if (numOfResources == 0)
    {
        return;
    }

    prvCpssDxChPortDpAasSffMarkLlResorcesRange(
        dpInfoPtr->sffUsedLlResourcesBitmap,
        resourcesRangeBase, numOfResources,
        GT_FALSE /*markUsed*/);
    dpInfoPtr->sffChannelResorcesBaseArr[channel]   = 0;
    dpInfoPtr->sffChannelResorcesAmountArr[channel] = 0;
}

/**
* @internal prvCpssDxChPortDpAasSffDbChannelResorcesPrint function
* @endinternal
*
* @brief    P:rinr SFF resources in DB for Dataparh.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] unitlNum             - unit number
* @param[in] channel              - channel number
*
* @retval - none
*/
void prvCpssDxChPortDpAasSffDbChannelResorcesPrint
(
    IN    GT_U8                                   devNum,
    IN    GT_U32                                  unitNum
)
{
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC *dpInfoPtr;
    GT_U32 numOfResources;
    GT_U32 resourcesRangeBase;
    GT_U32 channel;

    dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[unitNum];

    cpssOsPrintf(
        "SFF resources Bitmap: 0x%08X 0x%08X\n",
        dpInfoPtr->sffUsedLlResourcesBitmap[0],
        dpInfoPtr->sffUsedLlResourcesBitmap[1]);
    for (channel = 0; (channel < 22); channel++)
    {
        numOfResources = dpInfoPtr->sffChannelResorcesAmountArr[channel];
        resourcesRangeBase = dpInfoPtr->sffChannelResorcesBaseArr[channel];
        if (numOfResources != 0)
        {
            cpssOsPrintf(
                "channel %2d resourcesRangeBase %2d numOfResources %2d\n",
                channel, resourcesRangeBase, numOfResources);
        }
    }
}

/**
* @internal prvCpssDxChPortDpAasSffChannelConfigure function
* @endinternal
*
* @brief    Configure SFF Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceSize           - amounts of LLs in each resource
* @param[in] startResource          - start of resources range
* @param[in] numOfResources         - number of resorces in range
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @comment: resource is a contigous LLs group poiting each to the next, the last only can point to not-next
*           chaining the next resource or closing the LLs ring.
*/
static GT_STATUS prvCpssDxChPortDpAasSffChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceSize,
    IN    GT_U32                               startResource,
    IN    GT_U32                               numOfResources
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataEnable;
    GT_U32 regDataConfigure;
    GT_U32 regDataLl;
    GT_U32 nextLL;
    GT_U32 currResource;
    GT_U32 nextResource;
    GT_U32 lastResource;

    regDataConfigure =
        ((numOfResources * resourceSize) & 0x3F)               /*Ch %n Max Occup*/
        | (((startResource * resourceSize) & 0xFF) << 8)       /*Ch %n Cfg Wr Ptr*/
        | (((startResource * resourceSize) & 0xFF) << 16);     /*Ch %n Cfg Rd Ptr*/
    regDataEnable = 1; /* enable channel and disable statistics */

    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled channel - need power down before */
    if ((regData & 1) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* LL elements */
    lastResource = startResource + numOfResources - 1;
    currResource = startResource;
    for (nextResource = startResource + 1;
          (nextResource <= lastResource);
          nextResource++)
    {
        nextLL = (currResource * resourceSize) + (resourceSize - 1); /* Last LL in resouce */
        regAddr = regsAddrPtr->PCA_SFF[unitNum].llNextPointer[nextLL];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        regDataLl = (nextResource * resourceSize);
        if (regData != regDataLl)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataLl);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        currResource = nextResource;
    }
    /* LL ring back pointer */
    nextLL = (lastResource * resourceSize) + (resourceSize - 1); /* Last LL in resouce */
    regAddr = regsAddrPtr->PCA_SFF[unitNum].llNextPointer[nextLL];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regDataLl = (startResource * resourceSize);
    if (regData != regDataLl)
    {
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataLl);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* registers per channel */
    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataConfigure);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataEnable);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasSffChannelDown function
* @endinternal
*
* @brief    Disable SFF Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] resourceSize           - amounts of LLs in eACH resource
* @param[in] startResource          - start of resources range
* @param[out] resourcesBitmapPtr    - Array of bits indexed by resource (1 -- used, 0 - free)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @comment: resource is a contigous LLs group poiting each to the next, the last only can point to not-next
*           chaining the next resource or closing the LLs ring.
*/
static GT_STATUS prvCpssDxChPortDpAasSffChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceSize,
    IN    GT_U32                               startResource,
    IN    GT_U32                               numOfResources
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataEnable;
    GT_U32 regDataConfigure;
    GT_U32 regDataLl;
    GT_U32 nextLL;
    GT_U32 currResource;
    GT_U32 lastResource;

    regDataConfigure = 0;
    regDataEnable = 0;

    /* disable channel */
    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* LL elements */
    lastResource = startResource + numOfResources - 1;
    for (currResource = startResource;
          (currResource <= lastResource);
          currResource++)
    {
        nextLL = (currResource * resourceSize) + (resourceSize - 1); /* Last LL in resouce */
        regAddr = regsAddrPtr->PCA_SFF[unitNum].llNextPointer[nextLL];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        regDataLl = ((currResource + 1) * resourceSize);
        if (regData != regDataLl)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataLl);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* registers per channel */
    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataConfigure);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasSffStatisticsConfigure function
* @endinternal
*
* @brief    Configure SFF Statistics parameters.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] enable                 - GT_TRUE - enable, GT_FALSE - disable
* @param[in] sopNotEop              - GT_TRUE - Start of Packet, GT_FALSE - End of Packet
* @param[in] selectedChannelNum     - selected channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChPortDpAasSffStatisticsConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_BOOL                              enable,
    IN    GT_BOOL                              sopNotEop,
    IN    GT_U32                               selectedChannelNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;

    regAddr = regsAddrPtr->PCA_SFF[unitNum].globalConfig;
    regMask =  (1 << 24) | (1 << 23) | (0x3F << 16);
    regData =
        (BOOL2BIT_MAC(enable) << 24)
        | (BOOL2BIT_MAC(sopNotEop) << 23)
        | ((selectedChannelNum & 0x3F) << 16);

    rc = prvCpssDrvHwPpWriteRegBitMask(
        devNum, regAddr, regMask, regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enable channel statistics */
    regData = (enable == GT_FALSE) ? 0 : 2;
    regAddr = regsAddrPtr->PCA_SFF[unitNum].channelControlEnable[selectedChannelNum];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 2/*mask*/, regData/*value*/);
    return rc;
}

/**
* @internal prvCpssDxChPortDpAasSffUnitInit function
* @endinternal
*
* @brief    SFF Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasSffUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 val;
    GT_U32 ii;

    /* init as one big ring to update onle bacward poiners when needed */
    for (ii = 0; (ii < 84); ii++)
    {
        regAddr = regsAddrPtr->PCA_SFF[unitNum].llNextPointer[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        val = (ii < 83) ? (ii + 1) : 0;
        if (regData == val) continue;
        regData = val;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*************************************************************************************************************************************/
/* BRG_TX_UNPACK */

/**
* @internal prvCpssDxChPortDpAasBrgTxUnpackUnitInit function
* @endinternal
*
* @brief    BRG TX UNPACK Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgTxUnpackUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regMask;
    GT_U32 regData;

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].globalConfig;
    /* staistics collect enable (bit 0), Global 2Credit_eop (bit 15)*/
    regMask = 1 | (1 << 15);
    regData = 1 | (1 << 15);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgTxUnpackChannelConfigure function
* @endinternal
*
* @brief    Configure BRG TX UNPACK Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceSize           - amounts of LLs in each resource
* @param[in] startResource          - start of resources range
* @param[in] numOfResources         - number of resorces in range
* @param[in] mifCredits             - number MIF credits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgTxUnpackChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceSize,
    IN    GT_U32                               startResource,
    IN    GT_U32                               numOfResources,
    IN    GT_U32                               mifCredits
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataEnable;
    GT_U32 regMaskEnable;
    GT_U32 regDataRcrfConfigure;
    GT_U32 regDataMcrfConfigure;

    regMaskEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/
        | (1 << 7) /*ch_failover_loop_disable*/
        | (1 << 6) /*ch_enable_mif_credits_infinity*/;
    regDataEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/
        | (1 << 7) /*ch_failover_loop_disable*/
        | (0 << 6) /*ch_enable_mif_credits_infinity*/;

    regDataRcrfConfigure =
        ((startResource * resourceSize * 2) & 0x1FF)              /*Ch %n RCRF Row Base*/
        | (((numOfResources * resourceSize * 2) & 0x1FF) << 16);  /*Ch %n RCRF Cfg Number of Rows*/
    regDataMcrfConfigure =
        ((startResource * resourceSize) & 0xFF)                   /*Ch %n MCRF Row Base*/
        | (((numOfResources * resourceSize) & 0xFF) << 16);       /*Ch %n MCRF Cfg Number of Rows*/

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled channel - need power down before */
    if ((regData & 1) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    regDataEnable |= (regData & (~ regMaskEnable));

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelRcrfControlConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataRcrfConfigure);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelMcrfControlConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataMcrfConfigure);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelMifCredits[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, mifCredits);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataEnable);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgTxUnpackChannelDown function
* @endinternal
*
* @brief    Disable BRG TX UNPACK Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgTxUnpackChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regDataEnable;
    GT_U32 regMaskEnable;

    regMaskEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/;
    regDataEnable =
        0          /*channel enable*/
        | (0 << 1) /*statistics_collection_en*/;

    regAddr = regsAddrPtr->PCA_BRG_UNPACK_TX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMaskEnable, regDataEnable);

    return rc;
}

/*************************************************************************************************************************************/
/* BRG_RX_PACK */

/**
* @internal prvCpssDxChPortDpAasBrgRxPackUnitInit function
* @endinternal
*
* @brief    BRG RX PACK Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgRxPackUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regMask;
    GT_U32 regData;

    regAddr = regsAddrPtr->PCA_BRG_PACK_RX[unitNum].globalConfig;
    /* staistics collect enable (bit 0) */
    regMask = 1;
    regData = 1;
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgRxPackChannelConfigure function
* @endinternal
*
* @brief    Configure BRG RX PACK Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceSize           - amounts of LLs in each resource
* @param[in] startResource          - start of resources range
* @param[in] numOfResources         - number of resorces in range
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgRxPackChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceSize,
    IN    GT_U32                               startResource,
    IN    GT_U32                               numOfResources
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataEnable;
    GT_U32 regMaskEnable;
    GT_U32 regDataConfigure;
    GT_U32 fcThreshold;

    fcThreshold = ((numOfResources * resourceSize * 5) / 2) & 0x3FF;
    regMaskEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/
        | (0x3FF << 8) /*Ch %n Flow Control Threshold*/;
    regDataEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/
        | (fcThreshold << 8) /*ch_enable_mif_credits_infinity*/;

    regDataConfigure =
        ((startResource * resourceSize) & 0xFF)                 /*Ch %n MCRF Row Base*/
        | (((numOfResources * resourceSize) & 0xFF) << 8)        /*Ch %n MCRF Cfg Number of Rows*/
        | (((startResource * resourceSize * 2) & 0xFF) << 16)    /*Ch %n RCRF Row Base*/
        | (((numOfResources * resourceSize * 2) & 0xFF) << 24);  /*Ch %n RCRF Cfg Number of Rows*/

    regAddr = regsAddrPtr->PCA_BRG_PACK_RX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled channel - need power down before */
    if ((regData & 1) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    regDataEnable |= (regData & (~ regMaskEnable));

    regAddr = regsAddrPtr->PCA_BRG_PACK_RX[unitNum].channelControlConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataConfigure);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->PCA_BRG_PACK_RX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataEnable);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgRxPackChannelDown function
* @endinternal
*
* @brief    Disable BRG RX PACK Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgRxPackChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regDataEnable;
    GT_U32 regMaskEnable;

    regMaskEnable =
        1          /*channel enable*/
        | (1 << 1) /*statistics_collection_en*/;
    regDataEnable =
        0          /*channel enable*/
        | (0 << 1) /*statistics_collection_en*/;

    regAddr = regsAddrPtr->PCA_BRG_PACK_RX[unitNum].channelControlEnable[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMaskEnable, regDataEnable);

    return rc;
}

/*************************************************************************************************************************************/
/* BRG_SHM_EGR */

/**
* @internal prvCpssDxChPortDpAasBrgShmEgrUnitInit function
* @endinternal
*
* @brief    BRG SHM EGR Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmEgrUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regMask;
    GT_U32 regData;

    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].globalControl;
    /* FlexE (bit 10)     (loop_ch_sel)  (_CPU______) */
    regMask = (1 << 10) | (0x3F << 12) | (0x3F << 26);
    regData = (0 << 10)
        | (DP_LOOPBACK_CHANNEL << 12)
        | (DP_CPU_CHANNEL << 26);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgShmEgrChannelConfigure function
* @endinternal
*
* @brief    Configure BRG SGM EGR Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] rpDefCredits           - number of remote port default credits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmEgrChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               rpDefCredits
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataConfigure;
    GT_U32 regMaskConfigure;

    regMaskConfigure = 1 /*channel enable*/;
    regDataConfigure = 1 /*channel enable*/;

    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].channelConfig[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled channel - need power down before */
    if ((regData & 1) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    regDataConfigure |= (regData & (~ regMaskConfigure));

    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].remotePortConfig[channel];
    /* two field writes to delay between writes */
    /* Remote Port %n Credits Default Value */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 15, 8, rpDefCredits);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Remote Port %n Enable */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 23, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].channelConfig[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataConfigure);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgShmEgrChannelDown function
* @endinternal
*
* @brief    Disable BRG SHM EGR Channel.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmEgrChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;

    /* disable channel */
    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].channelConfig[channel];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = regsAddrPtr->PCA_BRG_SHM_EGR[unitNum].remotePortConfig[channel];
    /* Remote Port %n Enable */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 23, 1, 0);

    return rc;
}

/*************************************************************************************************************************************/
/* BRG_SHM_INGR */

/**
* @internal prvCpssDxChPortDpAasBrgShmIngrChannelToQueue function
* @endinternal
*
* @brief    BRG SHM INGR convert channel to queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_U32 prvCpssDxChPortDpAasBrgShmIngrChannelToQueue
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    GT_U32 queueNum;
    devNum  = devNum;
    unitNum = unitNum;

    /* queues 0-3 support 800G, other queue up to 100G                          */
    /* ports 0,4,8,12 can be confugured to speeed more than 100G - others - not */
    /* channels 0,4,8,12 mapped to queues 0,1,2,3,                              */
    /* channels 1,2,3 mapped to queues 4,8,12                                   */
    /* other channels mapped to queues with the same numbers                    */
    switch (channel)
    {
        case  0: queueNum =  0; break;
        case  4: queueNum =  1; break;
        case  8: queueNum =  2; break;
        case 12: queueNum =  3; break;
        case  1: queueNum =  4; break;
        case  2: queueNum =  8; break;
        case  3: queueNum = 12; break;
        default: queueNum = channel; break;
    }

    return queueNum;
}

/**
* @internal prvCpssDxChPortDpAasBrgShmIngrUnitInit function
* @endinternal
*
* @brief    BRG SHM INGR Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmIngrUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;
    /*for loop_code_global_config<<%i>> register*/
    /*enable packet decryption bypass when the packet loop matches register index.
      0 - packet decryption bypass is disabled.
      1- packet decryption bypass is enabled according to the incoming packet loop code.
      */
    const GT_U32 decrypt_bypass_en = 0;
    /*enable packet truncation when packet size exceed the value written in the packet truncate threshold
    and the packet loop matches register index.
    0 - packet truncation is disabled .1- packet truncation is enabled
    */
    const GT_U32 truncate_en = 0;
    /*maximal packet length (in SDB words) for the current loop code,
    if truncation is enabled for this loop code.
    */
    const GT_U32 pkt_trunc_thres = 0;

    regAddr = regsAddrPtr->PCA_BRG_SHM_INGR[unitNum].globalRxRemotePortConfig;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, BRG_SHM_INGR_CARRIER_CHANNEL);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = regsAddrPtr->PCA_BRG_SHM_INGR[unitNum].globalLoopbackRemotePortConfig;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, BRG_SHM_INGR_LOOPBACK_CARRIER_CHANNEL);
    if (rc != GT_OK)
    {
        return rc;
    }

    regData = (decrypt_bypass_en << 31) | (truncate_en << 30) | pkt_trunc_thres;
    for (ii = 0; (ii < 6); ii++)
    {
        regAddr = regsAddrPtr->PCA_BRG_SHM_INGR[unitNum].globalLoopbackCodeConfig[ii];
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasBrgShmIngrQueueConfigure function
* @endinternal
*
* @brief    Configure BRG SGM INGR Queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] queueNum               - number of queue related to channel
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmIngrQueueConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               queueNum,
    IN    GT_U32                               channel
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regDataConfigure;
    GT_U32 regMaskConfigure;

    regMaskConfigure =
        1            /*queue enable          */ |
        (0x3F <<  1) /*channel ID            */ |
        (1    << 24) /*flexE mode            */ ;
    regDataConfigure =
        1                     /*queue enable          */ |
        (channel <<  1)       /*channel ID            */ |
        (0    << 24)          /*flexE mode            */ ;

    regAddr = regsAddrPtr->PCA_BRG_SHM_INGR[unitNum].queueControl[queueNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled channel - need power down before */
    if ((regData & 1) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    regDataConfigure |= (regData & (~ regMaskConfigure));

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regDataConfigure);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasBrgShmIngrQueueDown function
* @endinternal
*
* @brief    Disable BRG SGM INGR Queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] queueNum               - number of queue related to channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasBrgShmIngrQueueDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               queueNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;

    regAddr = regsAddrPtr->PCA_BRG_SHM_INGR[unitNum].queueControl[queueNum];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 0); /*disable queue*/

    return rc;
}

/*************************************************************************************************************************************/
/* PCA ARBITER */

/**
* @internal prvCpssDxChPortDpAasPcaArbiterUnitInit function
* @endinternal
*
* @brief    PCA Arbiter Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasPcaArbiterUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;

    regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].arbiterBankSelect;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (regData != 0)
    {
        /* select bank0 */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].control;
    regData = PCA_ARBITER_SLOTS_NUM_OF; /* arbiter disable*/
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regData = (1 << 12) | PCA_ARBITER_SLOTS_NUM_OF; /* arbiter enable*/
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (ii = 0; (ii < PCA_ARBITER_CHANNELS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].channelEnable[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (regData != 0)
        {
            /* disable channel */
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].configuration[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (regData != PCA_ARBITER_SLOT_DEFAULT_VAL)
        {
            /* slot default value */
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, PCA_ARBITER_SLOT_DEFAULT_VAL);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasPcaArbiterChannelDown function
* @endinternal
*
* @brief    PCA Arbiter Channel Down.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasPcaArbiterChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;

    regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].channelEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (regData != 0)
    {
        /* disable channel */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].configuration[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (regData == channel)
        {
            /* slot default value */
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, PCA_ARBITER_SLOT_DEFAULT_VAL);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasPcaArbiterChannelConfigure function
* @endinternal
*
* @brief    PCA Arbiter Channel Configure.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceChannel        - channel number used for lookup on PcaArbiterChannelResources[]
* @param[in] speedInMbps            - speed In Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasPcaArbiterChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceChannel,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 resourcesBmp;
    GT_U32 ii;

    regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].channelEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (regData != 0)
    {
        /* enableed channel */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if (speedInMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    resourcesBmp = channelSpeedToResourcesBitmap(
        prvCpssDxChPortDpAasPcaArbiterChannelResources,
        PCA_ARBITER_MAX_RESOURCE_AS_CHANNNEL_SPEED,
        ((1 << DP_LOOPBACK_CHANNEL) | (1 << DP_CPU_CHANNEL)),/*notResourceAsChannel*/
        resourceChannel, speedInMbps);
    if (resourcesBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        if ((resourcesBmp & (1 << prvCpssDxChPortDpAasPcaArbiterResourcesSlots[ii])) == 0) continue;
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].configuration[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (regData != PCA_ARBITER_SLOT_DEFAULT_VAL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        /* slot map to channel */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, channel);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* enable channel */
    regAddr = regsAddrPtr->PCA_PZ_ARBITER[unitNum][0].channelEnable[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 1);

    return rc;
}

/*************************************************************************************************************************************/
/* TX FIFO */

/**
* @internal prvCpssDxChPortDpAasTxFifoUnitInit function
* @endinternal
*
* @brief    TX FIFO Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxFifoUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    /* placeholder - will be updated */
    devNum = devNum;
    unitNum = unitNum;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasTxFifoChannelConfigure function
* @endinternal
*
* @brief    TX FIFO Channel Configure.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] speedInMbps            - speed In Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxFifoChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 dbUnitNum;
    GT_U32 dbChannel;

    rc = prvCpssDxChPortDpAasChannelPerGlobalPortConvert(
        devNum, unitNum, channel, &dbUnitNum, &dbChannel);
    if (rc != GT_OK)
    {
        return rc;
    }

    regData = keyToValConvert(
        prvCpssDxChPortDpTxFifoSpeedToProfileArr, speedInMbps);
    regAddr = regsAddrPtr->sip6_txFIFO[dbUnitNum].configs.channelConfigs.speedProfile[dbChannel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasTxFifoChannelDown function
* @endinternal
*
* @brief    TX FIFO Channel Down.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxFifoChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    /* placeholder code, will be changed */
    devNum = devNum;
    unitNum = unitNum;
    channel = channel;
    return GT_OK;
}

/*************************************************************************************************************************************/
/* TX DMA */

/**
* @internal prvCpssDxChPortDpAasTxDmaUnitInit function
* @endinternal
*
* @brief    TX DMA Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxDmaUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 val;
    GT_U32 ii;

    /* set arbiter size */
    regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
    /* disabled with correct size */
    val = (PCA_ARBITER_SLOTS_NUM_OF - 1);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled with correct size */
    val = (1 << 31) | (PCA_ARBITER_SLOTS_NUM_OF - 1);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* disable all arbiter slots*/
    val = 0x3F;
    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Fix wrong register default value */
    regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.minPacketSize;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 68);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasSpeedResourceCheck function
* @endinternal
*
* @brief    Check DMA Channel available for requested speed.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] resourcePortNum        - resource physical port
* @param[in] ifMode                 - Interface mode
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - not enough resources or busy
*/
GT_STATUS prvCpssDxChPortDpAasSpeedResourceCheck
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    GT_PHYSICAL_PORT_NUM                 resourcePortNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
)
{
    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC portInfo; /* resource's info */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC     *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 unitNum, channel, speedInMbps;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 resourcesBmp;
    GT_U32 ii;
    GT_STATUS rc;

    rc = prvCpssDxChPortAasPortInfoGet(
        devNum, resourcePortNum, ifMode, portSpeed, isLoopback, &portInfo);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (portNum != portInfo.portNum)
    {
        /* Given-config and config-found-from resourcePortNum are not on same DP */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    unitNum = portInfo.dataPath;
    channel = portInfo.localDma;
    speedInMbps = portInfo.speedInMbPerSec;

    if (channel >= SFF_RESOURCE_NUM_OF)
    { /* PCA_SFF.x, PCA_BRG_UNPACK_TX.x, PCA_BRG_PACK_RX.channelControlEnable[0..21] */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (speedInMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDpAasSffDbChannelResorceAllocate(      /*checkOnly*/
        devNum, portInfo.dataPath, portInfo.localDma, &portInfo, GT_TRUE, isLoopback);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Arbiter check */
    resourcesBmp = channelSpeedToResourcesBitmap(
        prvCpssDxChPortDpAasPcaArbiterChannelResources,
        PCA_ARBITER_MAX_RESOURCE_AS_CHANNNEL_SPEED,
        ((1 << DP_LOOPBACK_CHANNEL) | (1 << DP_CPU_CHANNEL) | (1 << DP_SDMA_CHANNEL)),/*notResourceAsChannel*/
        portInfo.resourceChannel, speedInMbps);
    if (resourcesBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        if ((resourcesBmp & (1 << prvCpssDxChPortDpAasPcaArbiterResourcesSlots[ii])) == 0) continue;
        regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if ((regData & (1 << 31)) != 0)
        {
            regData &= ~(1 << 31);
            if (regData != channel)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChPortDpAasTxDmaChannelConfigure function
* @endinternal
*
* @brief    TX DMA Channel Configure.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceChannel        - channel number used for lookup on PcaArbiterChannelResources[]
* @param[in] speedInMbps            - speed In Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxDmaChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceChannel,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 resourcesBmp;
    GT_U32 dbUnitNum;
    GT_U32 dbChannel;
    GT_U32 val;
    GT_U32 ii;

    rc = prvCpssDxChPortDpAasChannelPerGlobalPortConvert(
        devNum, unitNum, channel, &dbUnitNum, &dbChannel);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*speed profile*/
    regData = keyToValConvert(
        prvCpssDxChPortDpTxDmaSpeedToProfileArr, speedInMbps);
    regAddr = regsAddrPtr->sip6_txDMA[dbUnitNum].configs.channelConfigs.speedProfile[dbChannel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* inter packet rate limiter */
    /*($ceil(20923.0769 * real'(rate) / 850000)*/ /* in designers code */
    val = (speedInMbps * 40) / 1000; /* By cider */
    regData = (val << 16);
    regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.channelConfigs.channelInterPacketRateLimiter[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0xFFFF0000, regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    val = 24 * 256; /* By cider */
    regData = (val << 16);
    regAddr = regsAddrPtr->sip6_txDMA[dbUnitNum].configs.channelConfigs.interGapConfigitation[dbChannel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 0xFFFF0000, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* arbiter */
    if (speedInMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    resourcesBmp = channelSpeedToResourcesBitmap(
        prvCpssDxChPortDpAasPcaArbiterChannelResources,
        PCA_ARBITER_MAX_RESOURCE_AS_CHANNNEL_SPEED,
        ((1 << DP_LOOPBACK_CHANNEL) | (1 << DP_CPU_CHANNEL) | (1 << DP_SDMA_CHANNEL)),/*notResourceAsChannel*/
        resourceChannel, speedInMbps);
    if (resourcesBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        if ((resourcesBmp & (1 << prvCpssDxChPortDpAasPcaArbiterResourcesSlots[ii])) == 0) continue;
        regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & (1 << 31)) != 0)
        {
            /* valid slot */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        regData = ((1 << 31) | channel);
        /* slot map to channel */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasTxDmaChannelDown function
* @endinternal
*
* @brief    TX DMA Channel Down.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasTxDmaChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;
    GT_U32 dbUnitNum;
    GT_U32 dbChannel;

    rc = prvCpssDxChPortDpAasChannelPerGlobalPortConvert(
        devNum, unitNum, channel, &dbUnitNum, &dbChannel);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* arbiter */
    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->sip6_txDMA[unitNum].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (((regData & (1 << 31)) == 0) || ((regData & 0x3F) != channel)) continue;
        regData = 0x3F;
        /* slot invalid */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*reset*/
    regAddr = regsAddrPtr->sip6_txDMA[dbUnitNum].configs.channelConfigs.channelGlobalConfig[dbChannel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = regsAddrPtr->sip6_txDMA[dbUnitNum].configs.channelConfigs.channelGlobalConfig[dbChannel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 1, 0);

    return rc;
}

/*************************************************************************************************************************************/
/* PB GPR */

/**
* @internal prvCpssDxChPortDpAasPbGprUnitInit function
* @endinternal
*
* @brief    PB GPR Unit init.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpAasPbGprUnitInit
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 val;
    GT_U32 ii;

    /* set arbiter size */
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].arbiterCalendarMax;
    /* disabled with correct size */
    val = (PCA_ARBITER_SLOTS_NUM_OF - 1);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* enabled with correct size */
    val = (1 << 31) | (PCA_ARBITER_SLOTS_NUM_OF - 1);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* disable all arbiter slots*/
    val = 0x1F;
    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].arbiterCalendarSlot[ii];
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, val);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasPbGprChannelConfigure function
* @endinternal
*
* @brief    PB GPR Channel Configure.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
* @param[in] resourceChannel        - channel number used for lookup on PcaArbiterChannelResources[]
* @param[in] speedInMbps            - speed In Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpPbGprChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel,
    IN    GT_U32                               resourceChannel,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 resourcesBmp;
    GT_U32 val;
    GT_U32 ii;

    /*speed profile*/
    regData = keyToValConvert(
        prvCpssDxChPortDpPbGprSpeedToProfileArr, speedInMbps);
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].channelProfile[channel];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* arbiter */
    if (speedInMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    resourcesBmp = channelSpeedToResourcesBitmap(
        prvCpssDxChPortDpAasPcaArbiterChannelResources,
        PCA_ARBITER_MAX_RESOURCE_AS_CHANNNEL_SPEED,
        ((1 << DP_LOOPBACK_CHANNEL) | (1 << DP_CPU_CHANNEL) | (1 << DP_SDMA_CHANNEL)),/*notResourceAsChannel*/
        resourceChannel, speedInMbps);
    if (resourcesBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        if ((resourcesBmp & (1 << prvCpssDxChPortDpAasPcaArbiterResourcesSlots[ii])) == 0) continue;
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].arbiterCalendarSlot[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & (1 << 31)) != 0)
        {
            /* valid slot */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        regData = ((1 << 31) | channel);
        /* slot map to channel */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*enable: bit0 - enable, bits 2-3 - flags of shaper */
    val = (speedInMbps <= PB_GPR_SHAPER_MIN_BW) ? 1 : 7;
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].channelEnable[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 7, val);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* reset */
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].channelReset[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 1, 0);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasPbGprChannelDown function
* @endinternal
*
* @brief    PB GPR Channel Down.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitlNum               - unit number
* @param[in] channel                - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpPbGprChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               unitNum,
    IN    GT_U32                               channel
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;

    /* arbiter */
    for (ii = 0; (ii < PCA_ARBITER_SLOTS_NUM_OF); ii++)
    {
        regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].arbiterCalendarSlot[ii];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (((regData & (1 << 31)) == 0) || ((regData & 0x1F) != channel)) continue;
        regData = 0x1F;
        /* slot map to channel */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*disable: bit0 - enable, bits 2-3 - flags of shaper */
    regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitNum].channelEnable[channel];
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, 1, 0);

    return rc;
}

/*************************************************************************************************************************************/
/* function called from other files */
/*************************************************************************************************************************************/
/**
* @internal prvCpssDxChPortDpAasDevicePunktBehaviorEnableSet function
* @endinternal
*
* @brief    Enable/Disable AAS DP PUNKT Behavior.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number
* @param[in]  enable                - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - not initialyzed device
*/
GT_STATUS prvCpssDxChPortDpAasDevicePunktBehaviorEnableSet
(
    IN    GT_U8                                devNum,
    IN    GT_BOOL                              enable
)
{
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC *dpInfoPtr;
    GT_U32 dataPath;
    GT_U32 numOfDataPath;

    numOfDataPath = PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles * 4;

    for (dataPath = 0; (dataPath < numOfDataPath); dataPath++)
    {
        /* allocate per datapath DB - needed for resources shadow */
        dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[dataPath];
        if (dpInfoPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
        dpInfoPtr->usePunktAlgorithm = enable;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasDeviceInit function
* @endinternal
*
* @brief    Initialyze AAS DP units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpAasDeviceInit
(
    IN    GT_U8                                devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC *dpInfoPtr;
    GT_U32 dataPath;
    GT_U32 numOfDataPath;
    GT_U32 globalDmaNum;

    numOfDataPath = PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles * 4;

    for (dataPath = 0; (dataPath < numOfDataPath); dataPath++)
    {
        /* allocate per datapath DB - needed for resources shadow */
        dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[dataPath];
        if (dpInfoPtr == NULL)
        {
            dpInfoPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_SIP7_DP_INFO_STC));
            if (dpInfoPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[dataPath] = dpInfoPtr;
        }
        cpssOsMemSet(dpInfoPtr, 0, sizeof(PRV_CPSS_DXCH_SIP7_DP_INFO_STC));
    }
    for (dataPath = 0; (dataPath < numOfDataPath); dataPath++)
    {
        /*egress*/
        rc = prvCpssDxChPortDpAasSffUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgTxUnpackUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgShmEgrUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasPcaArbiterUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasTxFifoUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasTxDmaUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasPbGprUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        /*ingress*/
        rc = prvCpssDxChPortDpAasBrgRxPackUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgShmIngrUnitInit(devNum, dataPath);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Configure SDMA on all DPs */
    for (dataPath = 0; (dataPath < numOfDataPath); dataPath++)
    {
        /* bypass not existng CPU ports */
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, dataPath, DP_SDMA_CHANNEL, &globalDmaNum);
        if (rc != GT_OK)
        {
            continue;
        }

        rc = prvCpssDxChPortDpAasTxFifoChannelConfigure(
            devNum, dataPath, DP_SDMA_CHANNEL, DP_CPU_CHANNEL_DEFAULT_SPEED);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasTxDmaChannelConfigure(
            devNum, dataPath, DP_SDMA_CHANNEL, DP_SDMA_CHANNEL, DP_CPU_CHANNEL_DEFAULT_SPEED);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpPbGprChannelConfigure(
            devNum, dataPath, DP_SDMA_CHANNEL, DP_SDMA_CHANNEL, DP_CPU_CHANNEL_DEFAULT_SPEED);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* RX_DMA registers for CPU port */
        rc = prvCpssDxChPortDpAasRxDmaChannelConfigure(
            devNum, dataPath, DP_SDMA_CHANNEL,
            DP_CPU_CHANNEL_DEFAULT_SPEED,
            63/*default CPU PORT*/,
            GT_FALSE /*isPreemptive*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasPortConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC portInfo;
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC          *dpInfoPtr;
    GT_U32                                   mifCredits;
    GT_U32                                   shmEgrRpDefCredits;
    GT_U32                                   shmIngrQueue;

    rc = prvCpssDxChPortAasPortInfoGet(
        devNum, portNum, ifMode, portSpeed, isLoopback, &portInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[portInfo.dataPath];

    rc = prvCpssDxChPortDpAasSffDbChannelResorceAllocate(
        devNum, portInfo.dataPath, portInfo.localDma, &portInfo, GT_FALSE, isLoopback);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* ------- egress ---------------------------------------------------*/
    shmEgrRpDefCredits =
        dpInfoPtr->sffChannelResorcesAmountArr[portInfo.resourceChannel] * BRG_SHM_EGR_RP_CREDITS_MUL;

    if (portInfo.isMac == 0)
    {
        mifCredits = 0;
    }
    else
    {
        mifCredits = keyToValConvert(
            prvCpssDxChPortDpAasBrgTxUnpackMifCreditsArr, portInfo.speedInMbPerSec);

        rc = prvCpssDxChPortDpAasMifChannelMapSet(
            devNum, portInfo.mifUnit, portInfo.localMif, portInfo.localDma);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        goto continueAfterBrgConfig_lbl;
    }

    rc = prvCpssDxChPortDpAasSffChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma,
        SFF_RESOURCE_SIZE,
        dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDma],
        dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDma]);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpAasBrgTxUnpackChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma,
        BRG_UX_UNPACK_RESOURCE_SIZE,
        dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDma],
        dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDma],
        mifCredits);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpAasBrgShmEgrChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma, shmEgrRpDefCredits);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpAasPcaArbiterChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma, portInfo.resourceChannel, portInfo.speedInMbPerSec);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpAasTxFifoChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma, portInfo.speedInMbPerSec);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpAasTxDmaChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma, portInfo.resourceChannel, portInfo.speedInMbPerSec);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChPortDpPbGprChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma, portInfo.resourceChannel, portInfo.speedInMbPerSec);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* ------- ingress ---------------------------------------------------*/
    rc = prvCpssDxChPortDpAasBrgRxPackChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma,
        BRG_UX_UNPACK_RESOURCE_SIZE,
        dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDma],
        dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDma]);
    if (rc != GT_OK)
    {
        return rc;
    }
    shmIngrQueue = prvCpssDxChPortDpAasBrgShmIngrChannelToQueue(
        devNum, portInfo.dataPath, portInfo.resourceChannel);
    rc = prvCpssDxChPortDpAasBrgShmIngrQueueConfigure(
        devNum, portInfo.dataPath, shmIngrQueue,
        portInfo.localDma);
    if (rc != GT_OK)
    {
        return rc;
    }

continueAfterBrgConfig_lbl:/* GM need to skip some code */

    rc = prvCpssDxChPortDpAasRxDmaChannelConfigure(
        devNum, portInfo.dataPath, portInfo.localDma,
        portInfo.speedInMbPerSec, portInfo.portNum, GT_FALSE /*isPreemptive*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portInfo.isPreemtionEnabled != GT_FALSE)
    {
        rc = prvCpssDxChPortDpAasSffDbChannelResorceAllocate(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, &portInfo, GT_FALSE, isLoopback);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* ------- egress ---------------------------------------------------*/
        rc = prvCpssDxChPortDpAasMifChannelMapSet(
            devNum, portInfo.mifUnit, portInfo.localMifPreemtive, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortDpAasSffChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive,
            SFF_RESOURCE_SIZE,
            dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDmaPreemtive],
            dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDmaPreemtive]);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgTxUnpackChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive,
            BRG_UX_UNPACK_RESOURCE_SIZE,
            dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDmaPreemtive],
            dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDmaPreemtive],
            mifCredits);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgShmEgrChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, shmEgrRpDefCredits);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasPcaArbiterChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, portInfo.localDmaPreemtive, portInfo.speedInMbPerSec);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasTxFifoChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, portInfo.speedInMbPerSec);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasTxDmaChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, portInfo.localDmaPreemtive, portInfo.speedInMbPerSec);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpPbGprChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive, portInfo.localDmaPreemtive, portInfo.speedInMbPerSec);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* ------- ingress ---------------------------------------------------*/
        rc = prvCpssDxChPortDpAasBrgRxPackChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive,
            BRG_UX_UNPACK_RESOURCE_SIZE,
            dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDmaPreemtive],
            dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDmaPreemtive]);
        if (rc != GT_OK)
        {
            return rc;
        }
        shmIngrQueue = prvCpssDxChPortDpAasBrgShmIngrChannelToQueue(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        rc = prvCpssDxChPortDpAasBrgShmIngrQueueConfigure(
            devNum, portInfo.dataPath, shmIngrQueue,
            portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasRxDmaChannelConfigure(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive,
            portInfo.speedInMbPerSec, portInfo.portNum, GT_TRUE /*isPreemptive*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpAasPortDownForce function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*           Called also at configure fail - when ifMode and portSpeed not in CPSS DB.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - port interface mode
* @param[in] portSpeed              - port speed enum
* @param[in] isLoopback             - loopback mode for this port/speed is requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortDownForce
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    IN    GT_BOOL                              isLoopback
)
{
    GT_STATUS                               rc;
    GT_STATUS                               rcTotal;
    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC portInfo;
    PRV_CPSS_DXCH_SIP7_DP_INFO_STC          *dpInfoPtr;
    GT_U32                                  shmIngrQueue;

    rc = prvCpssDxChPortAasPortInfoGet(
        devNum, portNum, ifMode, portSpeed, isLoopback, &portInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    dpInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip7_dpInfoPtr[portInfo.dataPath];

    rcTotal = GT_OK;

    /*egress*/
    rc = prvCpssDxChPortDpAasMifChannelMapDisable(
        devNum, portInfo.mifUnit, portInfo.localMif);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }

    if (dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDma] != 0)
    {
        rc = prvCpssDxChPortDpAasSffChannelDown(
            devNum, portInfo.dataPath, portInfo.localDma,
            SFF_RESOURCE_SIZE,
            dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDma],
            dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDma]);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
    }
    rc = prvCpssDxChPortDpAasBrgTxUnpackChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    rc = prvCpssDxChPortDpAasBrgShmEgrChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    rc = prvCpssDxChPortDpAasPcaArbiterChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    rc = prvCpssDxChPortDpAasTxFifoChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    rc = prvCpssDxChPortDpAasTxDmaChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    rc = prvCpssDxChPortDpPbGprChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }

    /*ingress*/
    rc = prvCpssDxChPortDpAasBrgRxPackChannelDown(
        devNum, portInfo.dataPath, portInfo.localDma);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    shmIngrQueue = prvCpssDxChPortDpAasBrgShmIngrChannelToQueue(
        devNum, portInfo.dataPath, portInfo.resourceChannel);
    rc = prvCpssDxChPortDpAasBrgShmIngrQueueDown(
        devNum, portInfo.dataPath, shmIngrQueue);
    if (rc != GT_OK)
    {
        rcTotal = rc;
    }
    prvCpssDxChPortDpAasSffDbChannelResorceFree(
        devNum, portInfo.dataPath, portInfo.localDma);

    if (portInfo.isPreemtionEnabled != GT_FALSE)
    {
        /*egress*/
        rc = prvCpssDxChPortDpAasMifChannelMapDisable(
            devNum, portInfo.mifUnit, portInfo.localMifPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }

        rc = prvCpssDxChPortDpAasSffChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive,
            SFF_RESOURCE_SIZE,
            dpInfoPtr->sffChannelResorcesBaseArr[portInfo.localDmaPreemtive],
            dpInfoPtr->sffChannelResorcesAmountArr[portInfo.localDmaPreemtive]);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        rc = prvCpssDxChPortDpAasBrgTxUnpackChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpAasBrgShmEgrChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        rc = prvCpssDxChPortDpAasPcaArbiterChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        rc = prvCpssDxChPortDpAasTxDmaChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        rc = prvCpssDxChPortDpPbGprChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }

        /*ingress*/
        rc = prvCpssDxChPortDpAasBrgRxPackChannelDown(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        shmIngrQueue = prvCpssDxChPortDpAasBrgShmIngrChannelToQueue(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
        rc = prvCpssDxChPortDpAasBrgShmIngrQueueDown(
            devNum, portInfo.dataPath, shmIngrQueue);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
        prvCpssDxChPortDpAasSffDbChannelResorceFree(
            devNum, portInfo.dataPath, portInfo.localDmaPreemtive);
    }

    return rcTotal;
}

/**
* @internal prvCpssDxChPortDpAasPortDown function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_U32                                  globalMac;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     portSpeed;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(
        devNum, portNum, globalMac);
    ifMode     = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, globalMac);
    portSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, globalMac);

    return prvCpssDxChPortDpAasPortDownForce(
        devNum, portNum, ifMode, portSpeed, GT_FALSE/*isLoopback*/);
}

/* MIF utils */

/**
* @internal prvCpssDxChPortDpAasPortMifPfcEnableSet function
* @endinternal
*
* @brief  Set MIF PFC enable/disable
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                - device number
* @param[in] globalMac             - global MAC number
* @param[in] enableTx              - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx              - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpAasPortMifPfcEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enableTx,
    IN  GT_BOOL                 enableRx
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC portInfo;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     portSpeed;
    GT_U32                                  globalMac;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(
        devNum, portNum, globalMac);
    ifMode     = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, globalMac);
    portSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, globalMac);

    rc = prvCpssDxChPortAasPortInfoGet(
        devNum, portNum, ifMode, portSpeed, GT_FALSE/*isLoopback*/, &portInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpAasMifChannelPfcEnableGenericSet(
        devNum, portInfo.mifUnit, portInfo.localMif, enableTx, enableRx);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasPortMifPfcEnableGet function
* @endinternal
*
* @brief  Get MIF PFC enable/disable
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                - device number
* @param[in] globalMac             - global MAC number
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpAasPortMifPfcEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enableTxPtr,
    OUT GT_BOOL                 *enableRxPtr
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_PORT_DP_AAS_PORT_INFO_STC portInfo;
    CPSS_PORT_INTERFACE_MODE_ENT            ifMode;
    CPSS_PORT_SPEED_ENT                     portSpeed;
    GT_U32                                  globalMac;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(
        devNum, portNum, globalMac);
    ifMode     = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, globalMac);
    portSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, globalMac);

    rc = prvCpssDxChPortAasPortInfoGet(
        devNum, portNum, ifMode, portSpeed, GT_FALSE/*isLoopback*/, &portInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpAasMifChannelPfcEnableGenericGet(
        devNum, portInfo.mifUnit, portInfo.localMif, enableTxPtr, enableRxPtr);

    return rc;
}

/**
* @internal prvCpssDxChPortDpAasMifUnitChannelDump function
* @endinternal
*
* @brief    Dump MIF unit channel data.
*
* @note   APPLICABLE DEVICES:     AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpAasMifUnitChannelDump
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32    regAddr;
    GT_U32    regData;
    GT_STATUS rc;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("mapping: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.rxFifoReadyThershold[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxFifoReadyThershold: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.txCredit[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txCredit: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.statusFsmControl[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("statusFsmControl: 0x%08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.status.txStatus[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txStatus: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.status.rxStatus[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxStatus: 0x%08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txGoodPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txGoodPacketCount: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxGoodPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxGoodPacketCount: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txBadPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txBadPacketCount: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxBadPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxBadPacketCount: 0x%08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txDiscardedPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txDiscardedPacketCount: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxDiscardedPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxDiscardedPacketCount: 0x%08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txLinkFailCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txLinkFailCount: 0x%08X ", regData);

    cpssOsPrintf("\n");
    return GT_OK;
}
