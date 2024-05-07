/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChRegsVer1h.h
*
* DESCRIPTION:
*       This file includes the declaration of the structure to hold the
*       addresses of eArch PP registers for each different PP type.
*
*
* FILE REVISION NUMBER:
*       $Revision: 63 $
*
*******************************************************************************/
#ifndef __prvCpssDxChRegsVer1h
#define __prvCpssDxChRegsVer1h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegs.h>

/* number of Tiles that the device support */
#define MAX_TILE_CNS  4
/* number of DP (data path) per tile that the device support */
#define MAX_DP_PER_TILE_CNS  8
/* Falcon device tile offset */
#define FALCON_TILE_OFFSET_CNS 0x20000000
/* AAS device tile offset */
#define AAS_TILE_OFFSET_CNS    0x80000000

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdma.h>


/* Falcon control pipe offset */
#define FALCON_CONTROL_PIPE_OFFSET_CNS 0x08000000

/* number of DP (data path) that the device support */
#define MAX_DP_CNS  (MAX_TILE_CNS * MAX_DP_PER_TILE_CNS)/* total 32 */

/* number of pipes per tile that the device support */
#define MAX_PIPES_PER_TILE_CNS  2
/* number of pipes that the device support */
#define MAX_PIPES_CNS  (MAX_TILE_CNS * MAX_PIPES_PER_TILE_CNS)/* total 8 */

/* added for Ironman */
#define SIP6_30_MAX_MIF_CNS 4
#define SIP6_30_MAX_MIF_CHANNELS_PER_MIF_CNS 17
/* AAS PCA has 4 DPs per tile                                            */
/* DP0-2 are regulal and DP3 can be regular or FlexE                     */
/* Each DP has MIF0_8P and MIF1_8P units for 8 channels                  */
/* and MIF_4P unit for 4 channels                                        */
/* The MIF_4P unit for FlexE DP presents, but not used                   */
/* Global MIF units per tile are 1*MIF_CPU and 4*MIF_80P units for FlexE */
/* in Reg DB unit indexes are                                            */
/* per DP - MIF0_8P - 0,3,6,9. MIF1_8P - 1,4,7,10. MIF_4P - 2,5,8,11.    */
/* global - MIF_CPU - 12. MIF_80P - 13,14,15,16.                         */
#define SIP7_MAX_MIF_CNS 17
#define SIP7_MAX_MIF_CHANNELS_PER_MIF_CNS 16
/* MAX MIF Values */
#define ALL_DEVS_MAX_MIF_CNS 17
#define ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS 17

/* Falcon : port per DP --without-- the CPU port (network/SDMA) */
#define FALCON_PORTS_PER_DP_CNS    8

/* number of ravens in Falcon per one tile */
#define FALCON_RAVENS_PER_TILE     4

/* maximal number of LED units for SIP_6 device */
#define SIP_6_MAX_LED_UNIT_NUM_CNC (MAX_TILE_CNS * FALCON_RAVENS_PER_TILE)

/* 9 TAI units per tile */
#define FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS        9

/* falcon maximum speed profiles for tx configuration*/
#define MAX_SPEED_PROFILE_NUM_FALCON_CNS  12
#define MAX_SPEED_PROFILE_NUM_HAWK_CNS    12
#define MAX_SPEED_PROFILE_NUM_IRONMAN_CNS 15

#define MAX_SPEED_PROFILE_NUM_CNS  32

/* hawk : port per DP --without-- the CPU port (network/SDMA) */
#define HAWK_PORTS_PER_DP_CNS    26

/* phoenix : single DP include ALL channel (54 network ports + 2 SDMA) */
#define PHOENIX_PORTS_PER_DP_CNS    (54+2)

/* Harrier : port per DP --without-- the CPU port (network/SDMA) */
#define HARRIER_PORTS_PER_DP_CNS    16

/* AAS : port per DP --without-- the CPU port , GDMA/LP (network/SDMA) */
/*       the CPU port is 21'th channel      */
/*       the GDMA/ LB port is 22'th channel */
#define AAS_PORTS_PER_DP_CNS    20

/* number of LMUs in Raven */
#define LMU_NUMBER_PER_RAVEN       2

/* number of LMUs in tile */
#define LMU_NUMBER_PER_TILE        FALCON_RAVENS_PER_TILE * 2

/* Check if 'offsetof' is defined by compiler and if not, define it */
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ( (GT_U32) ( (GT_UINTPTR) (&((TYPE *)0)->MEMBER) ) )
#endif

/* Falcon : 1024 registers for per port register */
/* BC3 : 512 registers for per port register */
#define PORTS_NUM_CNS       1024
#define PORTS_BMP_NUM_CNS   ((PORTS_NUM_CNS + 31) / 32)

/*SIP 6 maximal local port  number*/                  /* 9 in Falcon , 27 in Hawk , 56 in Phoenix , 57 in Ironman*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC  57

#define CPSS_DXCH_SIP_7_MAX_DP_Q_NUM_DEV_LESS_MAC  4096
#define CPSS_DXCH_SIP_6_MAX_DP_Q_NUM_DEV_LESS_MAC  512

/*SIP 6 maximal  number of uburst profiles*/
#define CPSS_DXCH_SIP_6_MAX_UBURST_PROFILE_NUM_MAC                    32
/*SIP 7 maximal  number of uburst profiles*/
#define CPSS_DXCH_SIP_7_MAX_UBURST_PROFILE_NUM_MAC                    128


#define CPSS_DXCH_MAX_UBURST_PROFILE_NUM_DEV_LESS_MAC  CPSS_DXCH_SIP_7_MAX_UBURST_PROFILE_NUM_MAC




#define PRV_QFC_GROUP_SIZE_CNS       3
#define PRV_QFC_MAX_GROUP_NUM_CNS    (CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC/PRV_QFC_GROUP_SIZE_CNS)
/* Ironman H ports for the HSR/PRP */
#define H_PORTS_NUM_CNS     10
/* Ironman number for LRE for the HSR/PRP */
#define LRE_NUM_CNS     4
/* Ironman number for PNT (Proxy node table) entries */
#define PNT_NUM_CNS     128
/* Ironman number of stream SNG entries */
#define STREAM_SNG_NUM_CNS  _2K
/* Ironman number of stream SRF entries */
#define STREAM_PREQ_SRF_NUM 1024

/* sip7 with 63 analyzers */
#define MAX_NUM_ANALYZER_PORTS  63

/* per PPG : in Falcon: 9 in Hawk : 10 */
#define MAX_PPN_IN_PPG_CNS  10

/* per PPG : in IPE seahawk: 12 */
#define MAX_PPN_IN_IPE_PPG_CNS  12
/* in Falcon: 3 (per pipe) in Hawk : 4 */
#define MAX_PPG_CNS          4
/* Hawk - amount of datapaths - PCA unit group per datapath */
#define MAX_PCA_IN_TILE_CNS   4
#define MAX_PCA_CNS          (MAX_PCA_IN_TILE_CNS * MAX_TILE_CNS/*support AAS*/)
/* Hawk - 32 channels per PCA unit Phoenix - 64 */
#define MAX_PCA_CHANNELS_CNS 64
/* Number of channels in Packet Generator of PCA unit */
#define MAX_PCA_PG_CHANNELS_CNS 32
/* Number of Packet Generator instances in PCA unit */
#define MAX_PCA_PG_INSTANCE_CNS 32
/* Number of bytes in Packet Generator Header */
#define MAX_PCA_PG_HEADER_BYTES_CNS 28
/* Number of registers for Packet Generator Header */
#define MAX_PCA_PG_HEADER_REGS_CNS ((MAX_PCA_PG_HEADER_BYTES_CNS+3)/4)
/* AAS max number of SHM_INGR_QUEUES for FlexE*/
#define MAX_PCA_BRG_SHM_INGR_QUEUES_CNS 168
/* AAS max number of SHM_INGR_CHANNELS for FlexE*/
#define MAX_PCA_BRG_SHM_INGR_CHANNELS_CNS 86

/* Ironman - maximal number of header words in Packet Generator of PCA unit */
#define IRONMAN_MAX_PCA_PG_HEADER_REGS_CNS      8
/* Ironman - number of profiles in Packet Generator of PCA unit */
#define IRONMAN_MAX_PCA_PG_PROFILES_CNS         4
/* Ironman - number of Packet Generator instances in PCA unit */
#define IRONMAN_MAX_PCA_PG_INSTANCE_CNS         2
/* Ironman Number of bytes in Packet Generator Header */
#define IRONMAN_MAX_PCA_PG_HEADER_BYTES_CNS    32

/* AAS max number of range classifications entries */
#define AAS_MAX_RANGE_CLASSIFICATION_NUM_CNS    128

/* In Hawk TTI: max number of tunnel header length profiles */
#define MAX_TUNNEL_HEADER_LENGTH_PROFILE_NUM_CNS 8

/* In Hawk PPU: max number of PPU profiles */
#define MAX_PPU_PROFILE_NUM_CNS 16

/* In Hawk PPU: max number of K_stg units */
#define MAX_PPU_KSTG_NUM_CNS 3

/* In SeaHawk IPE PPU: max number of K_stg units */
#define MAX_PPU_FOR_IPE_KSTG_NUM_CNS 4

/* In Hawk PPU: max number of SP_BUS profiles */
#define MAX_PPU_SP_BUS_PROFILE_NUM_CNS 4

/* In Hawk PPU: max number of interrupt counters */
#define MAX_PPU_INT_COUNTER_NUM_CNS 8

/* In Hawk PPU: max number of error profiles */
#define MAX_PPU_ERROR_PROFILE_NUM_CNS 2

/* In Hawk PPU: max number of key generation profiles */
#define MAX_PPU_KEY_GEN_PROFILE_NUM_CNS 16

/* In Hawk PPU: max number of tcam rules */
#define MAX_PPU_TCAM_RULES_NUM_CNS 63

/* In Hawk PPU: max number of tcam rules */
#define MAX_IPE_PPU_TCAM_RULES_NUM_CNS 15

/* In Hawk PPU: max number of dau protected windows */
#define MAX_PPU_DAU_PROT_WIN_NUM_CNS 5

/* in Legacy: 2 (per pipe) in Hawk : 4 */
#define MAX_CNC_BLOCKS_CNC  4

#define MAX_TCP_CMP_CNS 64 /* Hawk - 64 , Falcon - 8 */
#define MAX_UDP_CMP_CNS 64 /* Hawk - 64 , Falcon - 8 */

#define MAX_EMX_CHANNEL_NUM_CNS 8
#define MAX_EMX_PROFILE_NUM_CNS 16
#define MAX_EMX_UNIT_NUM_CNS    4
#define MAX_EMX_REDUCED_ACTION_BYTES_CNS    45

#define MAX_TTI_LU_SERIAL_EM_PROFILE_NUM_CNS 128
#define MAX_TTI_LU_SERIAL_LOOKUP_NUM_CNS 3
#define MAX_TTI_LU_PARALLEL_LOOKUP_NUM_CNS 4
#define MAX_TTI_LU_ILM_QOS_ACTION_ENTRY_NUM_CNS 64
#define MAX_TTI_LU_ILM_TTL_ACTION_ENTRY_NUM_CNS 16

#define PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS 16


#define MAX_EM_UNIT_NUM_CNS    8

/* maximal number of OAM Group Status registers. AAS - 64 */
#define MAX_OAM_GROUP_STATUS_CNS 64

/* short name to TTI unit registers */
#define PRV_DXCH_REG1_UNIT_TTI_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TTI

/* short name to TTI_LU unit registers */
#define PRV_DXCH_REG1_UNIT_TTI_LU_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TTI_LU

/* short name to L2I unit registers */
#define PRV_DXCH_REG1_UNIT_L2I_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->L2I

/* short name to FDB unit registers */
#define PRV_DXCH_REG1_UNIT_FDB_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->FDB

/* short name to EM unit registers */
#define PRV_DXCH_REG1_UNIT_EM_MAC(_devNum, _unitNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EM[_unitNum]

/* short name to PHA unit registers */
#define PRV_DXCH_REG1_UNIT_PHA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PHA

/* short name to EQ unit registers */
#define PRV_DXCH_REG1_UNIT_EQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EQ

/* short name to LPM unit registers */
#define PRV_DXCH_REG1_UNIT_LPM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LPM

/* short name to EGF_eft unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_eft

/* short name to EGF_qag unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_qag

/* short name to EGF_sht unit registers */
#define PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EGF_sht

/* short name to HA unit registers */
#define PRV_DXCH_REG1_UNIT_HA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->HA

/* short name to ETS unit registers */
#define PRV_DXCH_REG1_UNIT_ETS_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->HA.ETS_temp

/* short name to MLL unit registers */
#define PRV_DXCH_REG1_UNIT_MLL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MLL

/* short name to PCL unit (IPCL) registers */
#define PRV_DXCH_REG1_UNIT_PCL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PCL

/* short name to Midway PCL unit (MPCL) registers */
#define PRV_DXCH_REG1_UNIT_MPCL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MPCL

/* short name to EPCL unit (EPCL) registers */
#define PRV_DXCH_REG1_UNIT_EPCL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->EPCL

/* short name to RXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_RXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->rxDMA[0]

/* short name to sip6_RXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_rxDMA[0]

/* short name to sip6_RXDMA[index] unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_RXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_rxDMA[index]

/* short name to AAC unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNM.AAC

/* short name to MPPSelectors unit registers */
#define PRV_DXCH_REG1_UNIT_SIP6_CNM_MPP_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNM.MPP_RFU

/* short name to SMU unit registers */
#define PRV_DXCH_REG1_UNIT_SMU_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->SMU

/* Falcon : LMU unit macro   */
/* _tile - tile number */
/* _raven - raven number */
/* _lmu - LMU number */
#define PRV_DXCH_FALCON_LMU_NUM_MAC(_tile, _raven, _lmu) \
    (_tile) * LMU_NUMBER_PER_TILE + (_raven) * LMU_NUMBER_PER_RAVEN + (_lmu)


/* note: lmu index is calculated using PRV_CPSS_REG_DB_TYPE_MTI_LMU_E */
#define PRV_DXCH_REG1_UNIT_LMU_MAC(_devNum, _lmu)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[_lmu]

#define MPF_PER_RAVEN           2
#define MPFS_PER_MPF            10
#define MPFS_TOTAL_PER_RAVEN    MPF_PER_RAVEN * MPFS_PER_MPF
#define MPFS_TOTAL_PER_FALCON   MPFS_TOTAL_PER_RAVEN * FALCON_RAVENS_PER_TILE
/* Falcon : MPFS unit macro   */
/* _tile - tile number */
/* _raven - raven number */
/* _mpf - MPF number */
/* _mpfs - MPFS number */
#define PRV_DXCH_FALCON_MPFS_NUM_MAC(_tile, _raven, _mpf, _mpfs) \
    (_tile) * MPFS_TOTAL_PER_FALCON + (_raven) * MPFS_TOTAL_PER_RAVEN  + (_mpf) * MPFS_PER_MPF + _mpfs
#define PRV_DXCH_REG1_UNIT_MPFS_MAC(_devNum, _mpfs)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MPFS[_mpfs]

#define PRV_DXCH_FALCON_MPFS_CPU_MAC(_tile, _raven) \
    (_tile) * FALCON_RAVENS_PER_TILE + (_raven)
#define PRV_DXCH_REG1_UNIT_MPFS_CPU_MAC(_devNum, _mpfs_cpu)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MPFS_CPU[_mpfs_cpu]


/* SIP6 LED unit macro   */
/* _tile - tile number */
/* _raven - raven number */
#define PRV_DXCH_LED_NUM_MAC(_tile, _raven) \
    (_tile) * FALCON_RAVENS_PER_TILE + (_raven)
#define PRV_DXCH_REG1_UNIT_LED_MAC(_devNum, _tile, _raven)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_LED[PRV_DXCH_LED_NUM_MAC(_tile, _raven)]

/* short name to IA unit registers */
#define PRV_DXCH_REG1_UNIT_IA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ingressAggregator[0]

/* short name to RXDMA specific unit registers */
#define PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->rxDMA[index]

/* short name to IA specific unit registers */
#define PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ingressAggregator[index]

/* short name to TXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_TXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txDMA[0]

#define PRV_DXCH_REG1_UNIT_SIP6_TXDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txDMA[0]

/* short name to TXDMA specific unit registers */
#define PRV_DXCH_REG1_UNIT_TXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txDMA[index]

#define PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txDMA[index]

/* short name to TX_FIFO unit registers */
#define PRV_DXCH_REG1_UNIT_TX_FIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txFIFO[0]

#define PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txFIFO[0]

/* short name to TX_FIFO specific unit registers */
#define PRV_DXCH_REG1_UNIT_TX_FIFO_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->txFIFO[index]

#define PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->sip6_txFIFO[index]

/* short name to SIP_ETH_TXFIFO unit registers */
#define PRV_DXCH_REG1_UNIT_ETH_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->SIP_ETH_TXFIFO

/* short name to SIP_ILKN_TXFIFO unit registers */
#define PRV_DXCH_REG1_UNIT_ILKN_TXFIFO_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->SIP_ILKN_TXFIFO


/* short name to ERMRK unit registers */
#define PRV_DXCH_REG1_UNIT_ERMRK_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->ERMRK

/* short name to OAM unit registers :
    _stage - 0 --> ingress    (CPSS_DXCH_OAM_STAGE_TYPE_INGRESS_E)
    _stage - 1 --> egress     (CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E)
*/
#define PRV_DXCH_REG1_UNIT_OAM_MAC(_devNum,_stage)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->OAMUnit[_stage]

/* TAI subunit                                                  */
/* _gop - Group Of Ports (port number can be converted to _gop) */
/* _id - TAI id in the GOP (0 or 1)                             */
#define PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(_devNum, _gop, _id)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.TAI[_gop][_id]

/* PTP (TSU) subunit - 1/10/100/200/400G ports */
/* _port - port number */
#define PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(_devNum, _port)   \
    ((PRV_CPSS_PORT_MTI_400_E != PRV_CPSS_DXCH_PORT_TYPE_MAC(_devNum, _port)) ?      \
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.PTP[_port][0] :               \
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.PTP[_port][1])

/* CG subunit */
/* CG Port */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT

/* CG Port MAC */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC

/* CG Port MAC MIBS */
#define PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(_devNum, _port)   \
    &(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[0])

/* CG Converters */
#define PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_CONVERTERS

/* CG_PORT wrapper - get the register address - cg port mac address0 register */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACaddress0;

/* CG_PORT wrapper - get the register address - cg port mac frame length */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACFrmLength;

/* CG_PORT wrapper - get the register address - cg port mac command config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACCommandConfig;

/* CG_PORT wrapper - get the register address - cg port mac command config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_TX_IPG_LENGTH_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACTxIpgLength;

/* CG_PORT wrapper - get the register address - cg port mac Rx crc opt */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACRxCrcOpt;

/* CG_PORT wrapper - get the register address - cg port mac status */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatus;

/* CG_PORT wrapper - get the register address - cg port mac StatN Config */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatNConfig;

/* CG_PORT wrapper - get the register address - cg port mac MIBS */
#define PRV_CPSS_DXCH_REG1_CG_PORT_MAC_MIB_REG_ADDR_ENTRIES_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(devNum, portNum);

/* CG_CONVERTERS wrapper - get the register address - cg port mac control register0 */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAControl0;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters status */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersStatus;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters resets */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_RESETS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersResets

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status2 */
#define PRV_CPSS_DXCH_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC(devNum,portNum,regAddrPtr)\
    if(((portNum) == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)) \
        *(regAddrPtr) = PRV_CPSS_SW_PTR_ENTRY_UNUSED; \
    else                                                                        \
        *(regAddrPtr) = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus2

/* FCA subunit   */
/* _port - port/mac number */
#define PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.FCA[_port]

#define PRV_DXCH_REG_UNIT_GOP_FCA_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->macRegs.perPortRegs[_port].FCA

#define PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->macRegs.cpuPortRegs.FCA

/* MTI_EXT subunit           */
/* _port - global mac number */
#define PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_port].MTI_EXT

/* MTI_USX_EXT subunit           */
/* _port - global mac number */
#define PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_port].MTI_USX_EXT

/* MTI_CPU_EXT subunit           */
/* _port - global mac number */
#define PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(_devNum, _port)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_port].MTI_CPU_EXT

/* short name to PLR unit (PLR) registers */
#define PRV_DXCH_REG1_UNIT_ILKN_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.ILKN

/* short name to PLR unit (PLR) registers */
#define PRV_DXCH_REG1_UNIT_PLR_MAC(_devNum,_stage)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PLR[_stage]

/* short name to CNC unit registers */
#define PRV_DXCH_REG1_UNIT_CNC_MAC(_devNum, _instance)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNC[_instance]


/* short name to TXQ_DQ unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[0]

/* short name to TXQ_DQ specific unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[index]


/* short name to TXQ_LL unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_LL_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.ll

/* short name to TXQ_PFC unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.pfc

/* short name to TXQ_QCN unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_QCN_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.qcn

/* short name to TXQ_BMX unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_BMX_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.bmx

/* short name to TXQ_Q unit registers */
#define PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TXQ.queue

/* short name to PREQ unit registers */
#define PRV_DXCH_REG1_UNIT_PREQ_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PREQ


/* short name to TM DROP unit registers */
#define PRV_DXCH_REG1_UNIT_TM_DROP_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TMDROP

/* short name to LMS unit registers */
#define PRV_DXCH_REG1_UNIT_LMS_MAC(_devNum,_instance)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.LMS[_instance]

/* short name to MG unit registers */
#define PRV_DXCH_REG1_UNIT_MG_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->MG

/* short name to TM QMAP unit registers */
#define PRV_DXCH_REG1_UNIT_TM_QMAP_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TMQMAP

/* short name to TM_INGR_GLUE unit registers */
#define PRV_DXCH_REG1_UNIT_TM_INGR_GLUE_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_INGR_GLUE

/* short name to TM_EGR_GLUE unit registers */
#define PRV_DXCH_REG1_UNIT_TM_EGR_GLUE_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_EGR_GLUE

/* short name to TM_FCU unit registers */
#define PRV_DXCH_REG1_UNIT_TM_FCU_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->TM_FCU

/* short name to BM unit registers */
#define PRV_DXCH_REG1_UNIT_BM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->BM

/* short name to PPU unit registers - Sip 6.10 only */
#define PRV_DXCH_REG1_UNIT_PPU_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->PPU

/* short name to CNM units registers  */
#define PRV_DXCH_REG1_UNIT_CNM_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNM

typedef struct PRV_CPSS_DXCH_PP_TXQ_PIZZA_ARBITER_STCT  /* BC2 / BobK --- 85 registers, */
{                                                       /* Aldrin     --- 96 registres  */
    GT_U32    pizzaArbiterConfigReg[96];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_TXQ_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[85];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STCT  /* BobK   RxDmaGlue - 37  TxDma-Glue --- 37 */
{                                                           /* Aldrin RxDmaGlue - 78  TxDma-Glue --- 37 */
    GT_U32    pizzaArbiterConfigReg[78];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC;

typedef struct PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT /* falcon */
{
    GT_U32    pizzaArbiterConfigReg[256];
    GT_U32    pizzaArbiterCtrlReg;
}PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT;

/* The max number of words per channel is 32 */
#define  PRV_CPSS_DXCH_AAC_MAX_NUM_OF_TRANSACTION_WORDS       32  /* Up to 32 Outstanding accesses */

/**
* @enum PRV_CPSS_DXCH_FDB_MANAGER_AAC_CHANNELS_ENT
 *
 * @brief This enum defines the AAC channels used for FDB
*/
typedef enum {
    /* FDB AAC Channel 1  */
    PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E = 1,
    /* FDB AAC Channel 2  */
    PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_SECONDARY_E,
    /* DLB and PHA AAC Channel */
    PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E,
    /* EM AAC Channel 1 */
    PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_PRIMARY_E,
    /* EM AAC Channel 2 */
    PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_SECONDARY_E,
    /* LPM AAC Channel 1 */
    PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_1_E,
    /* LPM AAC Channel 2 */
    PRV_CPSS_DXCH_AAC_CHANNEL_LPM_MANAGER_2_E,

    /* Must be last*/
    PRV_CPSS_DXCH_AAC_CHANNEL_LAST_E
}PRV_CPSS_DXCH_AAC_CHANNELS_ENT;

/*--------------------------------------
 * /Cider/EBU/BobK/BobK {Current}/Switching Core/MPPM/Units/<MPPM_IP> MPPM IP Units/<Pizza_arb> SIP_COMMON_MODULES_IP Units/Pizza Arbiter/Pizza Arbiter Configuration Register %n
 *    Register CPU 0x47000200 Pizza Arbiter Control Register
 *    Register CPU 0x47000204 Pizza Arbiter Status
 *    Register CPU 0x47000208 Pizza Arbiter Configuration Register %n (0x47000208 + n*0x4 : where n (0-92) represents reg number)
 *    Register CPU 0x47000400 Pizza Arbiter debug register (Internal)
 *-----------------------------------------*/
typedef struct PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[93];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STC;

#define MAX_MG_UNITS    16  /* 8 per 2 tiles */

/* maximum XSMI units in CNM */
#define MAX_CNM_XSMI_UNITS_CNS 3

/* size of Data in 32 bit words for SHM indirect access */
#define SHM_SIP6_DATA_SIZE_CNS 4
#define SHM_SIP7_DATA_SIZE_CNS 5
#define SHM_DATA_SIZE_CNS 5

/* FALCON 12.8T need 256+16+16 DMAs :
    256 network ports
    16   reduced network ports
    16  SDMA CPU ports
*/
#define PRV_CPSS_MAX_DMA_NUM_CNS  (PRV_CPSS_MAX_MAC_PORTS_NUM_CNS + MAX_MG_UNITS)

/*
 * Typedef: struct PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC
 *
 * Description: A struct including PP registers addresses for register access
 *              by different types of PPs.
 *
 * Fields: The struct includes fields by the names of PP registers, each
 *         field represents the respective register address.
 *
 * Notes:
 *          format of structure is similar to PRV_CPSS_DXCH_PP_REGS_ADDR_STC
 *
 */

/* This registers DB struct is used in several generic algorithm like registers
   dump. These algorithms require that this struct and PRV_CPSS_BOOKMARK_STC
   struct have packed members.
   Use 4 byte alignment to guaranty that nextPtr will be right after size in
   both this structure and in the PRV_CPSS_BOOKMARK_STC.
*/
#pragma pack(push, 4)
typedef struct
{
    struct /*TTI*/{

        struct /*VLANAssignment*/{

            GT_U32    ingrTPIDConfig[4]/*VLAN_Ethertype_index*/;
            GT_U32    ingrTagProfileTPIDSelect[2][2]/*Tag*//*Profile group*/;
            GT_U32    ingrTPIDTagType;
            GT_U32    ingrTPIDEnhClasification[8];      /* TPID Index */
            GT_U32    ingrTPIDEnhClasificationExt[8];   /* TPID Index */
            GT_U32    ingrTPIDEnhClasificationExt2[8];  /* TPID Index */
            GT_U32    ingrTPIDEnhClasificationExt3[8];  /* TPID Index */
        }VLANAssignment;

        struct /*userDefinedBytes*/{

            GT_U32    IPCLTCPPortRangeComparatorConfig[MAX_TCP_CMP_CNS]/*TCPCmp*/;
            GT_U32    IPCLUDPPortRangeComparatorConfig[MAX_UDP_CMP_CNS]/*UDPCmp*/;

        }userDefinedBytes;

        struct /*TTIUnitGlobalConfigs*/{

            GT_U32    TTIUnitGlobalConfig;
            GT_U32    TTIUnitGlobalConfigForCCPorts;
            GT_U32    TTIUnitGlobalConfigExt;
            GT_U32    TTIUnitGlobalConfigExt2;
            GT_U32    TTIUnitGlobalConfigExt3;
            GT_U32    TTIUnitGlobalConfigExt4;/* new for sip7 */
            GT_U32    TTIUnitGlobalConfigExt5;/* new for sip7 */
            /* SIP 6.30 */
            GT_U32    hsrPrpGlobalConfig0;
            /* SIP7 */
            GT_U32    TTIEliQosAttributes;
            GT_U32    TTIExplicitNullQosAttributes;

        }TTIUnitGlobalConfigs;

        struct /*TTIEngine*/{

            GT_U32    TTIEngineInterruptCause;
            GT_U32    TTIEngineInterruptMask;
            GT_U32    TTIEngineConfig;
            GT_U32    TTIPCLIDConfig0;
            GT_U32    TTIPCLIDConfig1;
            GT_U32    TTILookupIPExceptionCommands;
            GT_U32    TTIPktTypeKeys[8]/*packet types*/;
            GT_U32    TTILookupIPExceptionCodes0;
            GT_U32    TTIKeySegmentMode0;
            GT_U32    TTIKeySegmentMode1;
            GT_U32    TTIKeyType;
            GT_U32    TTILookupMPLSExceptionCommands;
            GT_U32    TTILookupIPExceptionCodes1;
            GT_U32    TTIPacketTypeExactMatchProfileId[10]/*packet types pairs*/;/* from sip6 */
            GT_U32    TTIExactMatchProfileId1Mapping[128]/*remote ports*/;/* from sip6_10 */
            GT_U32    TTIExactMatchProfileId2Mapping[128]/*remote ports*/;/* from sip6_10 */

        }TTIEngine;

        struct /*trunkHashSimpleHash*/{

            GT_U32    trunkHashConfigReg0;
            GT_U32    trunkHashConfigReg1;
            GT_U32    trunkHashConfigReg2;

        }trunkHashSimpleHash;

        struct /*sohoCcfc*/{

            GT_U32    sohoCcfcGlobal;
            GT_U32    sohoCcfcSpeedToTimer[4]/*speed_index*/;

        }sohoCcfc;

        struct /*PTP*/{

            GT_U32    _1588V1DomainDomainNumber[4][4]/*i*//*Domain*/;
            GT_U32    PTPExceptionCheckingModeDomain[5]/*Domain*/;
            GT_U32    piggyBackTSConfigs[8]/*group of ports*/;
            GT_U32    PTPConfigs;
            GT_U32    PTPOverUDPDestinationPorts;
            GT_U32    PTPExceptionsAndCPUCodeConfig;
            GT_U32    PTPEtherTypes;
            GT_U32    PTPExceptionsCntr;
            GT_U32    _1588V2DomainDomainNumber;
            GT_U32    timestampEtherTypes;
            GT_U32    timestampConfigs;

        }PTP;

        struct /*pseudoWire*/{

            GT_U32    MPLSOAMChannelTypeProfile[15]/*channel type*/;
            GT_U32    PWE3Config;
            GT_U32    PWTagModeExceptionConfig;

        }pseudoWire;

        struct /*protocolMatch*/{

            GT_U32    protocolsConfig[6]/*Protocols*/;
            GT_U32    protocolsEncapsulationConfig0;
            GT_U32    protocolsEncapsulationConfig1;

        }protocolMatch;

        struct /*portMap*/{

            GT_U32    portGroupMap[32]/*8 ports*/;
            GT_U32    portListMap[43]/*6 ports*/;

        }portMap;

        struct /*phyPortAndEPortAttributes*/{

            GT_U32    myPhysicalPortAttributes [304]/* index - myPhysicalPort */;
            GT_U32    myPhysicalPortAttributes2[304]/* same registers in second pipe TTI unit */;/* NOTE: NOT used by sip6 CPSS code !!! */
            GT_U32    remotePhysicalPortAssignmentGlobalConfig;

             /*SIP6 registers*/
            GT_U32    remotePhysicalPortAssignmentGlobalConfigExt;
        }phyPortAndEPortAttributes;

        struct /*MRUException*/{

            GT_U32    MRUSize[4]/*2 profiles*/;
            GT_U32    MRUCommandAndCode;

        }MRUException;

        struct /*MAC2ME*/{

            GT_U32    MAC2MEHighMask[256]/*Number of MAC*/;
            GT_U32    MAC2MELow[256]/*Number of MAC*/;
            GT_U32    MAC2MEHigh[256]/*Number of MAC*/;
            GT_U32    MAC2MELowMask[256]/*Number of MAC*/;
            GT_U32    MAC2MESrcInterface[256]/*Mac2Me entry*/;
            GT_U32    MAC2MESrcInterfaceSel[256]/*Mac2Me Entry*/;

        }MAC2ME;

        struct /*IP2ME*/{
            GT_U32    IP2MEDip0[8];
            GT_U32    IP2MEDip1[8];
            GT_U32    IP2MEDip2[8];
            GT_U32    IP2MEDip3[8];
            GT_U32    IP2MEConf0[8];
            GT_U32    IP2MEConf1[8];

        }IP2ME;

        struct /*IPv6SolicitedNode*/{

            GT_U32    IPv6SolicitedNodeMcAddrConfigReg0;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg1;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg2;
            GT_U32    IPv6SolicitedNodeMcAddrConfigReg3;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg0;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg1;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg2;
            GT_U32    IPv6SolicitedNodeMcAddrMaskReg3;

        }IPv6SolicitedNode;

        struct /*FCOE*/{

            GT_U32    FCoEGlobalConfig;
            GT_U32    FCoEExceptionConfigs;
            GT_U32    FCoEExceptionCntrs0;
            GT_U32    FCoEExceptionCntrs1;
            GT_U32    FCoEExceptionCntrs2;

        }FCOE;

        struct /*duplicationConfigs*/{

            GT_U32    duplicationConfigs0;
            GT_U32    duplicationConfigs1;
            GT_U32    MPLSMCDALow;
            GT_U32    MPLSMCDAHigh;
            GT_U32    MPLSMCDALowMask;
            GT_U32    MPLSMCDAHighMask;

        }duplicationConfigs;

        GT_U32    TTIKRuleMatchCntr[12]/*TTICounter*/;
        GT_U32    TTIUserDefinedEtherTypes[4]/*index*/;
        GT_U32    TTIIPv6GREEtherType;
        GT_U32    TTIIPv4GREEthertype;
        GT_U32    oamLabelGlobalConfiguration0;
        GT_U32    oamLabelGlobalConfiguration1;
        GT_U32    specialEtherTypes;
        GT_U32    sourceIDAssignment0;
        GT_U32    sourceIDAssignment1;
        GT_U32    receivedFlowCtrlPktsCntr;
        GT_U32    preRouteExceptionMasks;
        GT_U32    PCLUserDefinedEtherTypes[4]/*index*/;
        GT_U32    MPLSEtherTypes;
        GT_U32    IPv6ExtensionValue;
        GT_U32    IPMinimumOffset;
        GT_U32    IPLengthChecks;
        GT_U32    ECCTwoErrorsCntr;
        GT_U32    ECCOneErrorCntr;
        GT_U32    DSAConfig;
        GT_U32    droppedFlowCtrlPktsCntr;
        GT_U32    CNAndFCConfig;
        GT_U32    CFMEtherType;
        GT_U32    TTIMetalFix;
        GT_U32    ttiCutThroughConfiguration;
        GT_U32    cutThroughByteCountExtractFailCntr;
        GT_U32    cutThroughHdrIntegrityExceptionCfg1;
        GT_U32    cutThroughHdrIntegrityExceptionCfg2;

        /*SIP6 registers*/
        GT_U32    ECNDroppedConfiguration;

        /* SIP 6.10 registers */
        GT_U32    ppuProfile[MAX_PPU_PROFILE_NUM_CNS];
        GT_U32    tunnelHeaderLengthProfile[MAX_TUNNEL_HEADER_LENGTH_PROFILE_NUM_CNS];
        GT_U32    packetTypeTcamProfileId[20];
        GT_U32    tcamProfileIdMapping[128];

        /*SIP7 registers*/
        GT_U32    ttiRangeCheckGlobalConfig0;
        GT_U32    ttiRangeCheckGlobalConfig1;
        GT_U32    pclRangeCheckGlobalConfig0;
        GT_U32    pclRangeCheckGlobalConfig1;
        GT_U32    exp4qosDebugCounter;
        GT_U32    exp4egressDebugCounter;

    }TTI;


    struct /*L2I*/{

        struct /*VLANRangesConfig*/{

            GT_U32    ingrVLANRangeConfig;

        }VLANRangesConfig;

        struct /*VLANMRUProfilesConfig*/{

            GT_U32    VLANMRUProfileConfig[4]/*Register*/;

        }VLANMRUProfilesConfig;

        struct /*UDPBcMirrorTrapUDPRelayConfig*/{

            GT_U32    UDPBcDestinationPortConfigTable[12]/*Port*/;

        }UDPBcMirrorTrapUDPRelayConfig;

        struct /*securityBreachStatus*/{

            GT_U32    securityBreachStatus0;
            GT_U32    securityBreachStatus1;
            GT_U32    securityBreachStatus2;
            GT_U32    securityBreachStatus3;

        }securityBreachStatus;

        struct /*MACBasedQoSTable*/{

            GT_U32    MACQoSTableEntry[7]/*QoS Profile*/;

        }MACBasedQoSTable;

        struct /*layer2BridgeMIBCntrs*/{

            GT_U32    setVLANIngrFilteredPktCount[2]/*SetNum*/;
            GT_U32    cntrsSetConfig0[2]/*Set*/;
            GT_U32    cntrsSetConfig1[2]/*Set*/;
            GT_U32    setIncomingPktCount[2]/*SetNum*/;
            GT_U32    setSecurityFilteredPktCount[2]/*SetNum*/;
            GT_U32    setBridgeFilteredPktCount[2]/*SetNum*/;
            GT_U32    MACAddrCount0;
            GT_U32    MACAddrCount1;
            GT_U32    MACAddrCount2;
            GT_U32    hostOutgoingMcPktCount;
            GT_U32    hostOutgoingPktsCount;
            GT_U32    hostIncomingPktsCount;
            GT_U32    hostOutgoingBcPktCount;
            GT_U32    matrixSourceDestinationPktCount;

        }layer2BridgeMIBCntrs;

        struct /*IPv6McBridgingBytesSelection*/{

            GT_U32    IPv6MCBridgingBytesSelectionConfig;

        }IPv6McBridgingBytesSelection;

        struct /*ingrPortsRateLimitConfig*/{

            GT_U32    portSpeedOverrideEnable[8]/*Register*/;
            GT_U32    MIISpeedGranularity[8]/*Register*/;
            GT_U32    GMIISpeedGranularity[8]/*Register*/;
            GT_U32    XGSpeedGranularity[8]/*Register*/;
            GT_U32    CGSpeedGranularity[8]/*Register*/;
            GT_U32    ingrRateLimitConfig0;
            GT_U32    ingrRateLimitConfig3;
            GT_U32    ingrRateLimitConfig2;
            GT_U32    ingrRateLimitConfig1;
            GT_U32    ingrRateLimitDropCntr310;
            GT_U32    ingrRateLimitDropCntr3932;

        }ingrPortsRateLimitConfig;

        struct /*globalEportConifguration*/{

            GT_U32    globalEPortValue;/* in SIP5 but not SIP6 */
            GT_U32    globalEPortMask; /* in SIP5 but not SIP6 */
            GT_U32    globalEportMinValue; /* in SIP6 */
            GT_U32    globalEportMaxValue; /* in SIP6 */
        }globalEportConifguration;

        struct /*ctrlTrafficToCPUConfig*/{

            GT_U32    IPv6McLinkLocalCPUCodeIndex[8][2]/*CPU Code Index_n*//*CPU Code Index_t*/;
            GT_U32    IPv6McLinkLocalReged[8]/*Register*/;
            GT_U32    IPv6McLinkLocalConfig[32]/*Register*/;
            GT_U32    IPv6ICMPMsgTypeConfig[2]/*ICMP_Msg_Type*/;
            GT_U32    IPv6ICMPCommand;
            GT_U32    IPv4McLinkLocalReged[8]/*Register*/;
            GT_U32    IPv4McLinkLocalCPUCodeIndex[8][2]/*Register*//*Register_t*/;
            GT_U32    IPv4McLinkLocalConfig[32]/*Register*/;
            GT_U32    IEEEReservedMcReged[8]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcCPUIndex[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig7[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig6[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig5[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig4[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig3[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig2[16]/*Register*/;
/*base from IEEEReservedMcConfig0*/            GT_U32    IEEEReservedMcConfig1[16]/*Register*/;
            GT_U32    IEEEReservedMcConfig0[16]/*Register*/;

        }ctrlTrafficToCPUConfig;

        struct /*bridgeEngineInterrupts*/{

            GT_U32    bridgeInterruptCause;   /*prvCpssDrvPpIntDefDxChLion2Init*/
            GT_U32    bridgeInterruptMask;    /*prvCpssDrvPpIntDefDxChLion2Init*/

        }bridgeEngineInterrupts;

        struct /*bridgeEngineConfig*/{

            GT_U32    bridgeGlobalConfig0;
            GT_U32    bridgeGlobalConfig1;
            GT_U32    bridgeGlobalConfig3;
            GT_U32    bridgeCommandConfig0;
            GT_U32    bridgeCommandConfig1;
            GT_U32    bridgeCommandConfig2;
            GT_U32    bridgeCommandConfig3;
            GT_U32    bridgeSecurityBreachDropCntrCfg0;
            GT_U32    bridgeSecurityBreachDropCntrCfg1;
            GT_U32    bridgecpuPortConfig;

            GT_U32    bridgeGlobalConfig4;
            GT_U32    bridgeGlobalConfig5;
            GT_U32    bridgeMacSpoofProtectionConfig0;
            GT_U32    bridgeMacSpoofProtectionConfig1;

        }bridgeEngineConfig;

        struct /*bridgeDropCntrAndSecurityBreachDropCntrs*/{

            GT_U32    globalSecurityBreachFilterCntr;
            GT_U32    portVLANSecurityBreachDropCntr;
            GT_U32    bridgeFilterCntr;

        }bridgeDropCntrAndSecurityBreachDropCntrs;

        struct /*bridgeAccessMatrix*/{

            GT_U32    bridgeAccessMatrixLine[8]/*SA Access Level*/;
            GT_U32    bridgeAccessMatrixDefault;

        }bridgeAccessMatrix;

        struct /*eVlanMirroringToAnalyser*/{

                GT_U32 eVlanMirroringToAnalyserEnable[PORTS_BMP_NUM_CNS];

        }eVlanMirroringToAnalyser;
        GT_U32    l2iBadAddressLatchReg;

        /* SIP 6.30 */
        struct{
            GT_U32    hsrPrpGlobalConfig0;
            GT_U32    hsrPrpGlobalConfig1;
            GT_U32    hsrPrpGlobalConfig2;
            GT_U32    hsrPrpGlobalConfig3;
            GT_U32    hsrPrpSourcePortConfig0[H_PORTS_NUM_CNS];
            GT_U32    hsrPrpSourcePortConfig1[H_PORTS_NUM_CNS];
            GT_U32    hsrPrpPntCounter;
            GT_U32    hsrPrpPntCounterThreshold;
            GT_U32    hsrPrpPntDiscardPacketsLreACounter[LRE_NUM_CNS];
            GT_U32    hsrPrpPntDiscardPacketsLreBCounter[LRE_NUM_CNS];
            GT_U32    hsrPrpLreInstanceTable[LRE_NUM_CNS];
            GT_U32    hsrPrpPntReady      [PNT_NUM_CNS];
            GT_U32    hsrPrpPntValid      [PNT_NUM_CNS];
            GT_U32    hsrPrpPntEntryPart0 [PNT_NUM_CNS];
            GT_U32    hsrPrpPntEntryPart1 [PNT_NUM_CNS];
            GT_U32    hsrPrpPntEntryPart2 [PNT_NUM_CNS];
        }hsrPrp;

    }L2I;

    struct /*EM*/ /* from sip6 */{

        struct /*EMGlobalConfiguration*/{

            GT_U32    EMGlobalConfiguration1;
            GT_U32    EMGlobalConfiguration2;/* not relevant to sip6_10*/
            GT_U32    EMCRCHashInitConfiguration;

        }EMGlobalConfiguration;

        struct /*EMIndirectAccess*/{

            GT_U32    EMIndirectAccessControl;
            GT_U32    EMIndirectAccessData[5]/*word*/;
            GT_U32    EMIndirectAccessAccess;/* not relevant to sip6_10*/

        }EMIndirectAccess;

        struct /*EMInterrupt*/{

                GT_U32    EMInterruptCauseReg;
                GT_U32    EMInterruptMaskReg;

        }EMInterrupt;

        struct /*EMHashResults*/{

                GT_U32    EMMultiHashCRCResultReg[8]/*reg_num*/;

        }EMhashResults;

        struct /*EMSchedulerConfiguration*/{

                GT_U32    EMSchedulerSWRRControl;/* not relevant to sip6_10*/
                GT_U32    EMSchedulerSWRRConfig; /* not relevant to sip6_10*/

                GT_U32    EMSchedulerSWRRArbiterWeights; /* new in sip6_10 instead of emSchedulerSWRRConfig*/
                GT_U32    EMSchedulerSWRRArbiterPriority;/* new in sip6_10 instead of emSchedulerSWRRControl*/

        }EMSchedulerConfiguration;

        struct /*EMAutoLearning*/{

            GT_U32    EMProfileConfiguration1[16];
            GT_U32    EMProfileConfiguration2[16];
            GT_U32    EMProfileConfiguration3[16];
            GT_U32    EMProfileConfiguration4[16];
            GT_U32    EMFlowIdAllocationConfiguration1;
            GT_U32    EMFlowIdAllocationConfiguration2;
            GT_U32    EMFlowIdAllocationStatus1;
            GT_U32    EMFlowIdAllocationStatus2;
            GT_U32    EMFlowIdAllocationStatus3;
            GT_U32    EMInterruptCauseRegister;
            GT_U32    EMInterruptMaskRegister;
            GT_U32    EMAutoLearningCollisionStatus1;
            GT_U32    EMAutoLearningCollisionStatus_n[12];
            GT_U32    EMSemaphore;
            GT_U32    EMFlowIdFailCounter;
            GT_U32    EMIndexFailCounter;
            GT_U32    EMRateLimiterFailCounter;
            GT_U32    EMRateLimitConfiguration;
            GT_U32    EMRateLimitBurstCounter;
            GT_U32    EMRateLimitPacketsCounter;

        }EMAutoLearning;/* sip6_10 only*/

    }EM[MAX_EM_UNIT_NUM_CNS];

    struct /*REDUCED_EM*/ /* from sip7 */{

        struct /*EMGlobalConfiguration*/{

            GT_U32    EMGlobalConfiguration1;
            GT_U32    EMCRCHashInitConfiguration;

        }EMGlobalConfiguration;

    }REDUCED_EM;

    struct /*FDB*/{

        struct /*FDBCore*/{

            struct /*sourceAddrNotLearntCntr*/{

                GT_U32    learnedEntryDiscardsCount;

            }sourceAddrNotLearntCntr;

            struct /*FDB*/{

                GT_U32    MTCPipeDispatcher;

            }FDB;

            struct /*FDBInterrupt*/{

                GT_U32    FDBInterruptCauseReg;
                GT_U32    FDBInterruptMaskReg;

            }FDBInterrupt;

            struct /*FDBInternal*/{

                GT_U32    metalFix;
                GT_U32    FDBIdleState;

            }FDBInternal;

            struct /*FDBIndirectAccess*/{

                GT_U32    FDBIndirectAccessData[5]/*word*/;
                GT_U32    FDBIndirectAccessCtrl;
                GT_U32    FDBIndirectAccessAccess;

            }FDBIndirectAccess;

            struct /*FDBHashResults*/{

                GT_U32    FDBMultiHashCRCResultReg[8]/*reg_num*/;
                GT_U32    FDBNonMulti_hash_crc_result;
                GT_U32    FDBXorHash;

            }FDBHashResults;

            struct /*FDBGlobalConfig*/{

                GT_U32    FDBGlobalConfig;
                GT_U32    FDBGlobalConfig1;
                GT_U32    FDBCRCHashInitConfig;
                GT_U32    bridgeAccessLevelConfig;
                GT_U32    FDBAgingWindowSize;

                GT_U32    FDBGlobalConfig2;/*sip6*/
            }FDBGlobalConfig;

            struct /*FDBCntrsUpdate*/{

                GT_U32    FDBCntrsUpdateCtrl;

            }FDBCntrsUpdate;

            struct /*FDBBankCntrs*/{

                GT_U32    FDBBankCntr[16]/*BankNum*/;
                GT_U32    FDBBankRank0;
                GT_U32    FDBBankRank1;
                GT_U32    FDBBankRank2;
                GT_U32    FDBBankRank3;
                GT_U32    FDBBankRank4;
                GT_U32    FDBBankRank5;
                GT_U32    FDBBankRank6;
                GT_U32    FDBBankRank7;
                GT_U32    FDBBankRank8;
                GT_U32    FDBBankRank9;
                GT_U32    FDBBankRank13;
                GT_U32    FDBBankRank12;
                GT_U32    FDBBankRank11;
                GT_U32    FDBBankRank10;
                GT_U32    FDBBankRank14;
                GT_U32    FDBBankRank15;
                GT_U32    FDBBankCntrsCtrl;

            }FDBBankCntrs;

            struct /*FDBAddrUpdateMsgs*/{

                struct /*AUMsgToCPU*/{

                    GT_U32    AUMsgToCPU;
                    GT_U32    AUFIFOToCPUConfig;

                }AUMsgToCPU;

                struct /*AUMsgFromCPU*/{

                    GT_U32    AUMsgFromCPUWord[6]/*word*/;
                    GT_U32    AUMsgFromCPUCtrl;

                }AUMsgFromCPU;

            }FDBAddrUpdateMsgs;

            struct /*FDBAction*/{

                GT_U32    FDBActionGeneral;
                GT_U32    FDBAction0;
                GT_U32    FDBAction1;
                GT_U32    FDBAction2;
                GT_U32    FDBAction3;
                GT_U32    FDBAction4;
                GT_U32    FDBActionOldParameters;
                GT_U32    FDBActionNewParameters;

            }FDBAction;

            struct /*AUMsgRateLimitingCounter*/{

                GT_U32    AUMsgRateLimCnt;

            } AUMsgRateLimitingCounter;

            struct /*FDBScedulerConfiguration*/{

                GT_U32    FDBScedulerSWRRControl;
                GT_U32    FDBScedulerSWRRConfig;

            } FDBScedulerConfiguration;

            /* SIP 6.0 */
            struct {

                GT_U32    FdbFifoThresholds;

            } Debug;

            /* SIP 6.30 */
            struct{
                GT_U32    hsrPrpGlobalConfig;
                GT_U32    ddeExpirationTimeConfig;
                GT_U32    hsrPrpTimer;
                GT_U32    hsrPrpCountersFirst [H_PORTS_NUM_CNS];
                GT_U32    hsrPrpCountersSecond[H_PORTS_NUM_CNS];
                GT_U32    hsrPrpCountersThird [H_PORTS_NUM_CNS];
                GT_U32    hsrPrpOverridenNonExpiredCounter;
                GT_U32    hsrPrpAging;
            }hsrPrp;

        }FDBCore;

    }FDB;

    struct /*EQ*/{

        struct /*toCpuPktRateLimiters*/{

            GT_U32    CPUCodeRateLimiterDropCntr;
            GT_U32    toCpuRateLimiterPktCntr[255]/*Limiter*/;

        }toCpuPktRateLimiters;

        struct /*SCTRateLimitersInterrupt*/{

            GT_U32    CPUCodeRateLimitersInterruptCause[32]/*reg*/;
            GT_U32    CPUCodeRateLimitersInterruptMask[32]/*reg*/;
            GT_U32    CPUCodeRateLimitersInterruptSummary;
            GT_U32    CPUCodeRateLimitersInterruptSummaryMask;

        }SCTRateLimitersInterrupt;

        struct /*preEgrInterrupt*/{

            GT_U32    preEgrInterruptSummary;
            GT_U32    preEgrInterruptSummaryMask;

        }preEgrInterrupt;

        struct /*preEgrEngineGlobalConfig*/{

            GT_U32    duplicationOfPktsToCPUConfig[PORTS_BMP_NUM_CNS]/*port*/;
            GT_U32    MTUTableEntry[4]/*port*/;
            GT_U32    preEgrEngineGlobalConfig;
            GT_U32    preEgrEngineGlobalConfig2; /* new in sip 6 */
            GT_U32    MTUGlobalConfig;
            GT_U32    lossyDropConfig;
            GT_U32    cscdTrunkHashBitSelectionConfig;/* obsolete in sip7 */
            GT_U32    targetEPortMTUExceededCntr;
            GT_U32    incomingWrrArbiterWeights;
            GT_U32    dupFifoWrrArbiterWeights;
            GT_U32    scalableSgtSupport; /*new in SIP 6.10*/

        }preEgrEngineGlobalConfig;

        struct /*mirrToAnalyzerPortConfigs*/{

            GT_U32    mirrorInterfaceParameterReg[MAX_NUM_ANALYZER_PORTS]/*Parameter Number*/;
            GT_U32    analyzerPortGlobalConfig;
            GT_U32    ingrStatisticMirrToAnalyzerPortConfig;    /* not used in sip 6 */
            GT_U32    ingrAndEgrMonitoringToAnalyzerQoSConfig;

            GT_U32    ingrStatisticMirrToAnalyzerPortRatioConfig[MAX_NUM_ANALYZER_PORTS]/*analyzer*/;  /* new in sip 6 */
            GT_U32    ingrStatisticMirrToAnalyzerPortEnableConfig[MAX_NUM_ANALYZER_PORTS]/*analyzer*/; /* new in sip 6 */
        }mirrToAnalyzerPortConfigs;

        struct /*L2ECMP*/{

            GT_U32    ePortECMPEPortValue;      /* not used in sip 6 */
            GT_U32    ePortECMPEPortMask;       /* not used in sip 6 */
            GT_U32    ePortECMPLTTIndexBaseEPort;
            GT_U32    ePortECMPLFSRConfig;
            GT_U32    ePortECMPLFSRSeed;
            GT_U32    ePortECMPHashBitSelectionConfig;
            GT_U32    trunkLFSRConfig;
            GT_U32    trunkLFSRSeed;
            GT_U32    trunkHashBitSelectionConfig;

            GT_U32    ePortECMPEPortMinimum; /* new in sip 6 */
            GT_U32    ePortECMPEPortMaximum; /* new in sip 6 */

            GT_U32    dlbLTTIndexBaseEport; /* new in sip 6 */
            GT_U32    dlbEportMinValue;     /* new in sip 6 */
            GT_U32    dlbEportMaxValue;     /* new in sip 6 */
            GT_U32    ePortDLBCurrentTime;  /* new in sip 6 */
            GT_U32    dlbMinimumFlowletIpg; /* new in sip 6 */
            GT_U32    globalEportDLBConfig; /* new in sip 6 */
            GT_U32    coarseGrainDetectorHashBitSelectionConfig; /* new in sip 6 */
            /* new in sip6.10 */
            GT_U32    L2ECMP_config;
            GT_U32    L2ECMP_Hash_Bit_Selection[16];
        }L2ECMP;

        struct /*ingrSTCInterrupt*/{

            GT_U32    ingrSTCInterruptCause[32]/*Port*/;
            GT_U32    ingrSTCInterruptMask[32]/*Port*/;
            GT_U32    ingrSTCInterruptSummary;
            GT_U32    ingrSTCInterruptSummaryMask;
            GT_U32    ingrSTCInterruptSummary1;
            GT_U32    ingrSTCInterruptSummary1Mask;

        }ingrSTCInterrupt;/* not used in sip 6 */

        struct /*ingrSTCConfig*/{

            GT_U32    ingrSTCConfig;

        }ingrSTCConfig;/* not used in sip 6 */

        struct /*ingrForwardingRestrictions*/{

            GT_U32    ingrForwardingRestrictionsDroppedPktsCntr;
            GT_U32    ingrForwardingRestrictionsDropCode;

        }ingrForwardingRestrictions;

        struct /*ingrDropCntr*/{

            GT_U32    ingrDropCntrConfig;
            GT_U32    ingrDropCntr;

        }ingrDropCntr;

        struct /*ingrDropCodesCntr*/{

            GT_U32    dropCodesCntrConfig;
            GT_U32    dropCodesCntr;

        }ingrDropCodesCntr;

        struct /*ingrDropCodeLatching*/{

            GT_U32    ingrDropCodeLatching;
            GT_U32    lastDropCode;

        }ingrDropCodeLatching;/* new in sip 6 */

        struct /*ePort*/{

            GT_U32    ePortGlobalConfig;
            GT_U32    protectionLOCWriteMask;
            GT_U32    protectionSwitchingRXExceptionConfig;

        }ePort;

        struct /*criticalECCCntrs*/{

            GT_U32    criticalECCConfig;          /* internal in sip 6 */
            GT_U32    BMClearCriticalECCErrorCntr;
            GT_U32    BMClearCriticalECCErrorParams;
            GT_U32    BMIncCriticalECCErrorCntr;
            GT_U32    BMIncCriticalECCErrorParams;
            GT_U32    BMCriticalECCInterruptCause;
            GT_U32    BMCriticalECCInterruptMask;

        }criticalECCCntrs;

        struct /*CPUTargetDeviceConfig*/{

            GT_U32    CPUTargetDeviceConfigReg0;
            GT_U32    CPUTargetDeviceConfigReg1;
            GT_U32    CPUTargetDeviceConfigReg2;
            GT_U32    CPUTargetDeviceConfigReg3;

        }CPUTargetDeviceConfig;

        struct /*applicationSpecificCPUCodes*/{

            GT_U32    IPProtocolCPUCodeEntry[4]/*Protocol*/;
            GT_U32    TCPUDPDestPortRangeCPUCodeEntryWord0[16]/*Entry*/;
            GT_U32    TCPUDPDestPortRangeCPUCodeEntryWord1[16]/*Entry*/;
            GT_U32    IPProtocolCPUCodeValidConfig;

        }applicationSpecificCPUCodes;

        /* sip7 : GCF (generic classification field) */
        struct{
            GT_U32 stagePathInfo[3];/* hold fields :      length  start bit
                        StageX Path Selection Enable      1 bit:   0
                        StageX Path Selection GCF lsb     7 bits   1
                        StageX Path Selection GCF size    3 bits   8
                        StageX P/B Path Selection Enable  1 bit    11
                        StageX P/B Path Selection GCF lsb 7 bits   12
                                */
            GT_U32  mcReplicationToRemoteDev; /* Enable bit exists   - 1bit
                                                 Bit location in GCF - 7 bits */

            GT_U32  isIngressDevice; /* Enable bit          - 1 bit
                                        Bit location in GCF - 7 bits */

        }GCF;CPSS_TBD_BOOKMARK_AAS/* not in Cider yet */

        /* sip7 : E2PHY */
        struct{
            GT_U32  circuitsBaseAddress;/*
                 0..15 : circuitsBaseAddress_In_E2Phy
                16..31 : circuitsBaseAddress_Analyzer_In_E2PHY
                */
            GT_U32  mllToOtherDevice;/*
                0..2  : mllToOtherDevicePktCmd
                3..10 : mllToOtherDeviceCpuCode
                */
            GT_U32  hEcmpOutOfSync;/*
                0..2  : hEcmpOutOfSyncPktCmd
                3..10 : hEcmpOutOfSyncCpuCode
                */
        }E2PHY;CPSS_TBD_BOOKMARK_AAS/* not in Cider yet */
    }EQ;


    struct /*IPE*/{

        struct /*IPE_Configurations*/{
            GT_U32 ipeGlobalConfigurations;
            GT_U32 ipeThreadIdExtract;
            GT_U32 ipePpuProfileN[256];
            GT_U32 ipeRandomNumberGeneratorSeedMsbs;
            GT_U32 ipeRandomNumberGeneratorSeedLsbs;
            GT_U32 ipeBypassFifoConfigurations;
            GT_U32 ictProfileLsbsN[128];
            GT_U32 ictProfileMsbsN[128];
            GT_U32 ipeHashSeed;
            GT_U32 ipeHashKeyMaskLsbs;
            GT_U32 ipeHashKeyMaskMsbs;
            GT_U32 ipeHashSaltN[15];
            GT_U32 tcamLookupLatencyFifoConfigurations;
            GT_U32 ipeGdmaPdiBufferThresholds;
            GT_U32 tcamLookupSuperkeyReductionTableN[8];
        }IPE_Configurations;

        struct /*IPE_status*/{
            GT_U32 ipeIllegalAddress;
            GT_U32 ipeBypassBufferStatus;
            GT_U32 ipeIdleState;
            GT_U32 tcamLookupLatencyFifoStatus;
        }IPE_status;

        struct /*IPE_Interrupts*/{
            GT_U32 ipeGlobalInterruptMask;
            GT_U32 ipeGlobalInterruptCause;
            GT_U32 ipeInterruptSumCause;
            GT_U32 ipeInterruptSumMask;
        }IPE_Interrupts;

        struct /*IPE_Misc*/{
            GT_U32 ipeMetalFix;
        }IPE_Misc;

        struct /*PPA*/{
            struct /*ppa_regs*/{
                GT_U32 ppaControl;
                GT_U32 ppaInterruptSumCause;
                GT_U32 ppaInterruptSumMask;
                GT_U32 ppaInternalErrorCause;
                GT_U32 ppaInternalErrorMask;
                GT_U32 ppaDebugBus;
                GT_U32 profilingImemStallCounter;
                GT_U32 ppaMetalFix;
                GT_U32 profilingSharedDmemStallCounter;
                GT_U32 profilingTotalPpnExecutionCyclesCounter;
                GT_U32 ppaBadAddressLatch;
            }ppa_regs;
         }PPA;
          struct /*PPG[3]*/{
            struct /*ppg_regs*/{
                GT_U32    PPGInterruptSumCause;
                GT_U32    PPGInterruptSumMask;
                GT_U32    PPGInternalErrorCause;
                GT_U32    PPGInternalErrorMask;
                GT_U32    PPGBadAddrLatch;
            }ppg_regs;

            struct /*PPN[11]*/{
                struct /*ppn_regs*/{
                    GT_U32 fwDeviceName;     /* hold 4 char of 'device name' like : FLCN (Falcon) or AC5P (Hawk) */
                    GT_U32 fwVersion;        /* hold 4 U8 of : year,month,version,debugValue (18 06 00 00 , means 2018-June ver00 debug00)*/
                    GT_U32 fwImageId;        /* fw image ID */
                    GT_U32 ppnInternalErrorCause;
                    GT_U32 ppnInternalErrorMask;
                    GT_U32 packetHeaderAccessOffset;
                    GT_U32 ppnDoorbell;
                    GT_U32 todTWord0;
                    GT_U32 todTWord1;
                    GT_U32 todTWord2;
                    GT_U32 ppnBadAddressLatch;
                }ppn_regs;
            }PPN[MAX_PPN_IN_IPE_PPG_CNS];

            /* manually added */
            GT_U32      PPG_IMEM_base_addr;/* base address of this memory (4 words per entry) */
        }PPG[MAX_PPG_CNS];
    }IPE;


    struct /*LPM*/{
        GT_U32    globalConfig;
        GT_U32    lpmMemBankId;
        GT_U32    directAccessMode;
        GT_U32    exceptionStatus;
        GT_U32    LPMInterruptsCause;
        GT_U32    LPMInterruptsMask;
        GT_U32    LPMGeneralIntCause;
        GT_U32    LPMGeneralIntMask;
    }LPM;

    struct /*TCAM*/{

        struct /*tcamInterrupts*/{

            GT_U32    TCAMInterruptsSummaryMask;
            GT_U32    TCAMInterruptsSummaryCause;
            GT_U32    TCAMInterruptMask;
            GT_U32    TCAMInterruptCause;
            GT_U32    tcamParityErrorAddr;

        }tcamInterrupts;

        struct /*tcamArrayConf*/{

            GT_U32    tcamActiveFloors;
            GT_U32    tcamArraySpeed;

        }tcamArrayConf;

        struct /*parityDaemonRegs*/{

            GT_U32    parityDaemonCtrl;
            GT_U32    parityDaemonWriteAdressStart;
            GT_U32    parityDaemonWriteAdressEnd;
            GT_U32    parityDaemonWriteAdressJump;
            GT_U32    parityDaemonReadAdressStart;
            GT_U32    parityDaemonReadAdressEnd;
            GT_U32    parityDaemonReadAdressJump;
            GT_U32    parityDaemonDefaultWriteDataX0;
            GT_U32    parityDaemonDefaultWriteDataX1;
            GT_U32    parityDaemonDefaultWriteDataY0;
            GT_U32    parityDaemonDefaultWriteDataX2;
            GT_U32    parityDaemonDefaultWriteDataY1;
            GT_U32    parityDaemonDefaultWriteDataY2;
            GT_U32    parityDaemonStatus;

        }parityDaemonRegs;

        struct /*mgAccess*/{

            GT_U32    mgKeySizeAndTypeGroup[5]/*Tcam Group Num*/;
            GT_U32    mgHitGroupHitNum[6][4]/*Tcam Group Num*//*Hit Number*/;
            GT_U32    mgKeyGroupChunk[5][30]/*Tcam Group Num*//*Chunk Of The Key*/;
            GT_U32    mgCompRequest;
            GT_U32    mgKeyTypeUpdateReq;
            GT_U32    keyTypeData;

        }mgAccess;

        GT_U32    groupClientEnable[5]          /*Tcam Group num*/;
        GT_U32    tcamHitNumAndGroupSelFloor[24]/*Floor num, AAS can support 24 */;
        GT_U32    tcamGlobalConfig;
        GT_U32    tcamAnswersFloorSample;

        struct /*exactMatchProfilesTable*/{

            GT_U32    exactMatchPortMapping[2]/*Pipe*/;
            GT_U32    exactMatchProfileTableControl[16]/*Entry*/;
            GT_U32    exactMatchProfileTableMaskData[16][12]/*Entry/Word*/;
            GT_U32    exactMatchProfileTableDefaultActionData[16][8]/*Entry/Word*/;

        }exactMatchProfilesTable;

        GT_U32    exactMatchActionAssignment[16][32]/*entryIndex/Byte*/;

        /* different keys in parallel sublookups data */
        /* sip6_10 [0] for all client groups, sip7 - instance for each client group */
        GT_U32 subKey0Mapping[6][64]; /* for sip6_10 key size only, sip7 - as keys 1-3 */
        GT_U32 subKey1_3Mapping[5][64][3]; /* not present for client group 5 */
        GT_U32 sharedMapping[6][64];
        GT_U32 keyMuxCfgTable[6][48][6]; /*[client group][line][word]*/

    }TCAM;

    struct /*EGF_eft*/{

        struct /*MCFIFO*/{

            struct /*MCFIFOConfigs*/{

                GT_U32    portToMcFIFOAttributionReg[64]/*reg_offset*/;
                GT_U32    portToHemisphereMapReg[PORTS_BMP_NUM_CNS]/*reg offset*/;
                GT_U32    MCFIFOGlobalConfigs;
                GT_U32    MCFIFOArbiterWeights0;
                GT_U32    lossyDropAndPfcUcThreshold;
                GT_U32    priorityMulticast;

            }MCFIFOConfigs;

        }MCFIFO;

        struct /*global*/{

            GT_U32    cpuPortMapOfReg[128]/*reg_offset*/;
            GT_U32    cpuPortDist;
            GT_U32    portIsolationLookup0;
            GT_U32    portIsolationLookup1;
            GT_U32    clearedUCDist;       /* Cleared Unicast Distribution */
            GT_U32    EFTInterruptsCause;
            GT_U32    EFTInterruptsMask;
            GT_U32    ECCConf;
            GT_U32    ECCStatus;

            /* new in sip 6.0 */
            GT_U32    bypassEGFTargetEPortAttributes[PORTS_BMP_NUM_CNS]/*Source Port div 32*/;
        }global;

        struct /*egrFilterConfigs*/{

            GT_U32    enFCTriggerByCNFrameOnPort[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    enCNFrameTxOnPort[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unregedBCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unknownUCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    unregedMCFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    OAMLoopbackFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    virtual2PhysicalPortRemap[64]/*offset*/;
            GT_U32    physicalPortLinkStatusMask[PORTS_BMP_NUM_CNS]/*reg offset*/;
            GT_U32    egrFiltersGlobal;
            GT_U32    egrFiltersEnable;
            GT_U32    globalHashSelection;/*sip7*/

        }egrFilterConfigs;

        struct /*deviceMapConfigs*/{

            GT_U32    devMapTableAddrConstructProfile[8]/*profile_num*/;
            GT_U32    localSrcPortMapOwnDevEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    localTrgPortMapOwnDevEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    portAddrConstructMode[256]/*local physical source port*/;

        }deviceMapConfigs;

        struct /*counters*/{

            GT_U32    eftDescEcc1ErrorCntr;

        }counters;

        struct /*egress MIB counters*/
        {
            GT_U32    egrMIBCntrsPortSetConfig[2]/*Set*/;
            GT_U32    egrMIBCntrsSetConfig[2]/*Set*/;
            GT_U32    egrMIBMcFIFODroppedPktsCntr[2]/*Set*/;
            GT_U32    egrMIBBridgeEgrFilteredPktCntr[2]/*Set*/;
            GT_U32    egrMIBMcFilteredPktCntr[2]/*Set*/;
        } egrMIBCntrs;/* sip6 only */

        struct /*CNC eVlan Configurations*/
        {
            GT_U32    vlanPassDropConfigs;
        } cncEvlanConfigs;/* sip6 only */

    }EGF_eft;

    struct /*EGF_qag*/{

        struct /*uniMultiArb*/{

            GT_U32    uniMultiCtrl;
            GT_U32    uniWeights;
            GT_U32    multiWeights;
            GT_U32    multiShaperCtrl;

        }uniMultiArb;

        struct /*global*/{

            GT_U32    QAGInterruptsCause;
            GT_U32    QAGInterruptsMask;
            GT_U32    QAGMetalFix;

        }global;

        struct /*distributor*/{

            GT_U32    useVlanTag1ForTagStateReg[PORTS_BMP_NUM_CNS]/*reg_offset*/;
            GT_U32    toAnalyzerLoopback[MAX_NUM_ANALYZER_PORTS + 1]/*Reg Offset*/;
            GT_U32    TCRemap[32]/*remapped_tc*/;
            GT_U32    stackRemapEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    perTrgPhyPortLbProfile[16]/*Reg Offset*/;
            GT_U32    perSrcPhyPortLbProfile[16]/*Reg Offset*/;
            GT_U32    loopbackForwardingGlobalConfig;
            GT_U32    loopbackEnableConfig[4];
            GT_U32    evlanMirrEnable[PORTS_BMP_NUM_CNS]/*Reg Offset*/;
            GT_U32    DPRemap[16]/*remapped_dp*/;
            GT_U32    distributorGeneralConfigs;

            /* new in sip 6.0 */
            GT_U32    TCLoopbackConfig;
            GT_U32    eVIDXActivityStatusWriteMask;
            GT_U32    voQAssignmentGeneralConfig0;
            GT_U32    pktHashForVoQAssignment0;
            GT_U32    pktHashForVoQAssignment1;
            GT_U32    voQAssignmentGeneralConfig1;
            GT_U32    cutThroughConfig;

        }distributor;

    }EGF_qag;

    struct /*EGF_sht*/{

        struct /*global*/{

            GT_U32    eportStpState[PORTS_BMP_NUM_CNS]/*Port div 8*/;
            GT_U32    eportEvlanFilteringEnable[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTInterruptsCause;
            GT_U32    UCSrcIDFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTEgrFiltersEnable;
            GT_U32    MCLocalEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    ignorePhySrcMcFilterEn[PORTS_BMP_NUM_CNS]/*Reg Offset*/;
            GT_U32    eportVlanEgrFiltering;
            GT_U32    eportStpStateMode[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportOddOnlySrcIdFilterEnable[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportRoutedForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportPortIsolationMode[2*PORTS_BMP_NUM_CNS]/*Port div 16*/;
            GT_U32    SHTGlobalConfigs;
            GT_U32    eportIPMCRoutedFilterEn[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    eportFromCPUForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    SHTInterruptsMask;
            GT_U32    eportBridgedForwardRestricted[PORTS_BMP_NUM_CNS]/*Port div 32*/;
            GT_U32    meshIdConfigs;
            GT_U32    ePortMeshId[PORTS_NUM_CNS/8];
            GT_U32    dropOnEportVid1MismatchEn[PORTS_BMP_NUM_CNS]; /* port div 32 */
            GT_U32    eportAssociatedVid1[PORTS_NUM_CNS/2];             /* port div 2 */

        }global;

    }EGF_sht;


    struct /*HA*/{

        GT_U32    invalidPTPHeaderCntr;
        GT_U32    TPIDConfigReg[8]/*entry*/;
        GT_U32    routerMACSABaseReg1;
        GT_U32    PWCtrlWord[15]/*PW word*/;
        GT_U32    PWETreeEnable;
        GT_U32    invalidPTPOutgoingPiggybackCntr;
        GT_U32    PTPTimestampTagEthertype;
        GT_U32    PTPAndTimestampingExceptionConfig;
        GT_U32    MPLSEtherType[4];
        GT_U32    MPLSChannelTypeProfileReg7;
        GT_U32    MPLSChannelTypeProfileReg[7]/*Reg*/;
        GT_U32    IPLengthOffset;
        GT_U32    egrUDPPortRangeComparatorConfig[MAX_UDP_CMP_CNS]/*UDPCmp*/;
        GT_U32    invalidPTPIncomingPiggybackCntr;
        GT_U32    egrPolicyDIPSolicitationData1;
        GT_U32    ingrAnalyzerVLANTagConfig;
        GT_U32    IEtherType;
        GT_U32    haGlobalConfig1;
        GT_U32    haGlobalConfig;
        GT_U32    HAInterruptMask;
        GT_U32    HAInterruptCause;
        GT_U32    ethernetOverGREProtocolType;
        GT_U32    routerMACSABaseReg0;
        GT_U32    egrTCPPortRangeComparatorConfig[MAX_TCP_CMP_CNS]/*TCPCmp*/;
        GT_U32    egrPolicyDIPSolicitationMask3;
        GT_U32    egrPolicyDIPSolicitationMask2;
        GT_U32    egrPolicyDIPSolicitationMask1;
        GT_U32    egrPolicyDIPSolicitationMask0;
        GT_U32    egrPolicyDIPSolicitationData3;
        GT_U32    egrPolicyDIPSolicitationData2;
        GT_U32    tunnelStartFragmentIDConfig;
        GT_U32    egrPolicyDIPSolicitationData0;
        GT_U32    egrAnalyzerVLANTagConfig;
        GT_U32    dataECCStatus;
        GT_U32    CPIDReg1;
        GT_U32    CPIDReg0;
        GT_U32    congestionNotificationConfig;
        GT_U32    CNMHeaderConfig;
        GT_U32    BPEConfigReg2;
        GT_U32    BPEConfigReg1;
        GT_U32    haBadAddrLatchReg;/*manual added for Sip5_10 : Latch bad address in case of regfile bad address indication.*/
        GT_U32    NATConfig;        /*manual added for Sip5_10*/
        GT_U32    NATExceptionDropCounter;/*manual added for Sip5_10*/
        GT_U32    haGlobalConfig2;/*manual added for Sip5_25*/
        GT_U32    HASerConfig;

        CPSS_TBD_BOOKMARK
        /* next manually added to stop us the
            PRV_CPSS_DXCH_PP_REGS_ADDR_STC::haRegs */
        struct ETS_temp_STCT{
            GT_U32  ptpGlobalConfig; /* Egress PTP Global Register */
            GT_U32  ptpEnableTimeStamp; /* Enable Time Stamping Register */
            GT_U32  ptpGlobalFifoCurEntry[2]; /* Global FIFO Current Entry */
            GT_U32  ptpTodCntrNanoSeconds; /* Egress TOD Counter - Nanoseconds */
            GT_U32  ptpTodCntrSeconds[2]; /* Egress TOD Counter - Seconds */
            GT_U32  ptpTodCntrShadowNanoSeconds; /* Egress TOD Counter Shadow - Nanoseconds */
            GT_U32  ptpTodCntrShadowSeconds[2]; /* Egress TOD Counter Shadow - Seconds */
        }ETS_temp;

        /* SIP6 Registers */
        GT_U32    PTPExceptionCPUCodeConfig;
        GT_U32    QCNModifications;
        GT_U32    innerL3OffsetTooLong;

        /* SIP6.30 Registers */
        GT_U32    egrTPIDEnhClasificationExt1[8];  /* TPID Index */ /*in addition to :TPIDConfigReg[8]*/
        GT_U32    egrTPIDEnhClasificationExt2[8];  /* TPID Index */

        /* sip7 registers */
        GT_U32 numberOfCircuitsInFixedTable;
        GT_U32 circuitsLocationInTheSbms;
        GT_U32 ipv6TsFlowLabelEntropyOnly;
        GT_U32 ipv6TsFlowLabelEntropyWithMetadata0;
        GT_U32 ipv6TsFlowLabelEntropyWithMetadata1;
        GT_U32 meshIdConfiguration;
        GT_U32 srv6Configuration;
        GT_U32 epclRangeCheck0Configuration;
        GT_U32 epclRangeCheck1Configuration;
        GT_U32 edsa20bConfiguration;
        GT_U32 gcfFlowId_r[16];/* 16 flows id*/
        GT_U32 globalExpMappingRow_r_Reg_i[8][4];
        GT_U32 gcfAssignmentProfileTable_p_Nibble_n_Orig[16][32];/* 16 prifiles 32 nibbles*/
        GT_U32 gcfAssignmentProfileTable_p_Nibble_n_Mirrored[16][32];/* 16 prifiles 32 nibbles*/
        GT_U32 ingressMplsReservedTsPtr_r[2];
        GT_U32 cncBcMode;
        GT_U32 ingressMplsReservedCircuitEntry;

    }HA;

    struct /*MLL*/{

        struct /*multiTargetRateShape*/{

            GT_U32    multiTargetRateShapeConfig;
            GT_U32    multiTargetRateShapeConfig2;
            GT_U32    multiTargetMTUReg;
            GT_U32    tokenBucketBaseLine;

        }multiTargetRateShape;

        struct /*mcLinkedListMLLTables*/{

            GT_U32    qoSProfileToMultiTargetTCQueuesReg[64]/*Index*/;
            GT_U32    extQoSModeMCQueueSelectionReg[2]/*Index*/;

        }mcLinkedListMLLTables;

        struct /*multiTargetVsUcSDWRRAndStrictPriorityScheduler*/{

            GT_U32    mcUcSDWRRAndStrictPriorityConfig;
            GT_U32    mcPriorityTypeConfiguration;/* new in sip7 */

        }multiTargetVsUcSDWRRAndStrictPriorityScheduler;

        struct /*multiTargetTCQueuesAndArbitrationConfig*/{

            GT_U32    multitargetTCQueuesGlobalConfig;
            GT_U32    multitargetTCQueuesStrictPriorityEnableConfig; /* not used in sip 6 */
            GT_U32    multitargetTCQueuesWeightConfig;

            GT_U32    multitargetTCQueuesPriorityConfig;/* new in sip 6 */

        }multiTargetTCQueuesAndArbitrationConfig;

        struct /*multiTargetEPortMap*/{

            GT_U32    multiTargetEPortValue;  /* not used in sip 6 */
            GT_U32    multiTargetEPortMask;   /* not used in sip 6 */
            GT_U32    multiTargetEPortBase;
            GT_U32    ePortToEVIDXBase;

            GT_U32    multiTargetGlobalEPortMinValue;/* new in sip 6 */
            GT_U32    multiTargetGlobalEPortMaxValue;/* new in sip 6 */

        }multiTargetEPortMap;

        struct /*MLLOutInterfaceCntrs*/{

            GT_U32    L2MLLOutInterfaceCntrConfig1[2]/*Counter*/;
            GT_U32    L2MLLOutInterfaceCntrConfig[2]/*Counter*/;
            GT_U32    L2MLLValidProcessedEntriesCntr[3]/*Counter*/;
            GT_U32    L2MLLTTLExceptionCntr;
            GT_U32    L2MLLSkippedEntriesCntr;
            GT_U32    L2MLLSilentDropCntr;
            GT_U32    L2MLLOutMcPktsCntr[2]/*Counter*/;
            GT_U32    MLLSilentDropCntr;
            GT_U32    MLLMCFIFODropCntr;
            GT_U32    MLLMCQueueDropCnter[4]/*Counter*/;
            GT_U32    IPMLLSkippedEntriesCntr;
            GT_U32    IPMLLOutMcPktsCntr[2]/*Counter*/;
            GT_U32    IPMLLOutInterfaceCntrConfig1[2]/*Counter*/;
            GT_U32    IPMLLOutInterfaceCntrConfig[2]/*Counter*/;

        }MLLOutInterfaceCntrs;

        struct /*MLLGlobalCtrl*/{

            GT_U32    MLLGlobalCtrl;
            GT_U32    MLLLookupTriggerConfig;
            GT_U32    globalTTLExceptionConfig;
            GT_U32    globalEPortRangeConfig; /* not used in sip 6 */
            GT_U32    globalEPortRangeMask;   /* not used in sip 6 */
            GT_U32    IPMLLTableResourceSharingConfig;
            GT_U32    L2MLLTableResourceSharingConfig;
            GT_U32    MLLInterruptCauseReg;
            GT_U32    MLLInterruptMaskReg;
            GT_U32    MLLReplicatedTrafficCtrl;
            GT_U32    sourceBasedL2MLLFiltering;
            GT_U32    MLLEccStatus;
            GT_U32    MLLMCSourceID;

            GT_U32    L2mllGlobalEPortMinValue; /* new in sip 6 */
            GT_U32    L2mllGlobalEPortMaxValue; /* new in sip 6 */

            GT_U32    etreeConfiguration;           /* new in sip7 */
            GT_U32    ipMllGlobalGcfConfigValue[4]; /* new in sip7 */
            GT_U32    ipMllGlobalGcfConfigMask[4];  /* new in sip7 */
            GT_U32    bierConfiguration;            /* new in sip7 */

        }MLLGlobalCtrl;

        struct /*multi_target_vport_mapping*/{
            GT_U32 multiTargetVportRangeBase;
            GT_U32 multiTargetVportRangeSize;
            GT_U32 multiTargetVportEvidx;
            struct{
                GT_U32  multiTargetVPortSubRangeSize;
                struct{
                    GT_U32  backupCfgValue;
                    GT_U32  backupCfgMask;
                }backupChunks[4];
            }subRangeArr[4];
            GT_U32 multiTargetVportIngressDevice;
        }multiTargetVportMapping;/* new in sip7 */

    }MLL;

    struct /*PCL*/{

        struct /*OAMConfigs*/{

            GT_U32    channelTypeOAMOpcode[15]/*channel type*/;
            GT_U32    OAMRDIConfigs;
            GT_U32    OAMMEGLevelModeConfigs;
            GT_U32    OAMRFlagConfigs;
            GT_U32    RBitAssignmentEnable;

        }OAMConfigs;

        struct /*HASH*/{

            GT_U32    pearsonHashTable[16]/*index div 4*/;
            GT_U32    CRCHashConfig;
            GT_U32    CRC32Salt[20]/*n*/;
            GT_U32    CRC32Seed;
            GT_U32    CRC32SeedHash1;
            GT_U32    CRC32SaltHash1[20];

        }HASH;

        GT_U32    ingrPolicyGlobalConfig;
        GT_U32    PCLUnitInterruptCause;
        GT_U32    PCLUnitInterruptMask;
        GT_U32    pclBadAddressLatchReg;
        GT_U32    policyEngineConfig;
        GT_U32    policyEngineUserDefinedBytesConfig;
        GT_U32    CFMEtherType;
        GT_U32    lookupSourceIDMask[3]/*lookup num*/;
        GT_U32    policyEngineUserDefinedBytesExtConfig;
        GT_U32    TCAMKeyTypeMap[2]/*reg num*/;
        GT_U32    L2L3IngrVLANCountingEnable[PORTS_BMP_NUM_CNS]/*port div 32*/;
        GT_U32    countingModeConfig;
        GT_U32    IPCLReservedBitMask[3] /* lookup Num */;

        /* SIP6 Registers */
        GT_U32    CRCHashModes;
        GT_U32    exactMatchProfileIdMapEntry[256]/*entry num*/;

       /*sip6_30*/
       GT_U32    maxSduSizeProfile[4]/*profile num*/;
       GT_U32    maxSduException;
       GT_U32    flowIdActionAssignmentModeConfig[3]/*lookup num*/;
       GT_U32    policyGenericActionConfig[3]         /*lookup num*/;
       GT_U32    policyGlobalConfig[3]   /*lookup num*/;
       GT_U32    globalConfig_ext_1;

       /* SIP7 */
       GT_U32    policyGlobalConfig_ext1[3] /*lookup num*/;
       GT_U32    actionModifiedFieldsSelect[3][2]; /*lookup num, reg idx */

    }PCL;

    struct /*EPCL*/{

        GT_U32    egrPolicyGlobalConfig;
        GT_U32    EPCLOAMPktDetection;
        GT_U32    EPCLOAMMEGLevelMode;
        GT_U32    EPCLOAMRDI;
        GT_U32    EPCLInterruptCause;
        GT_U32    EPCLInterruptMask;
        GT_U32    EPCLChannelTypeToOAMOpcodeMap[5]/*Set*/;
        GT_U32    EPCLReservedBitMask;
        GT_U32    TCAMKeyTypeMap;

        GT_U32    EPCLBadAddrLatchReg;
        GT_U32    oamRBbitConfigReg;
        GT_U32    EPCLFifoConfiguration;

        /* SIP6 Registers */
        GT_U32    PHAMetadataMask1;
        GT_U32    PHAMetadataMask2;
        GT_U32    PHAMetadataMask3;
        GT_U32    PHAMetadataMask4;
        GT_U32    PHAThreadNumMask;
        GT_U32    latencyMonitoringSamplingConfig[8];
        GT_U32    latencyMonitoringProfile2SamplingProfile[512]; /* sip7[512] vs [64] */
        GT_U32    latencyMonitoringControl;

        /* SIP6_10 register */
        GT_U32    queueOffsetLatencyProfileConfig[8];
        /* SIP6_30 register */
        GT_U32    egressMaxSduSizeProfile[4]/*profile num*/;
        GT_U32    egressMaxSduException;
        GT_U32    EPCL_UDB_replacement[6];/*register per UDB size (10/20..60)*/
        GT_U32    egressPolicyGlobalConfig_ext_1;
        GT_U32    egressPolicyFlowIdActionAssignmentModeConfig;
        /* SIP7 register */
        GT_U32    epclPolicyLookupGlobalConfig;   /* TBD - not found in cider, copied from MPCL */

    }EPCL;

    struct /*MPCL*/{

        struct /*OAMConfigs*/{
            GT_U32    mpclChannelTypeOAMOpcode[15]/*channel type*/;
            GT_U32    mpclOAMRDIConfigs;
            GT_U32    mpclOAMMEGLevelModeConfigs;
            GT_U32    mpclOAMRFlagConfigs;
            GT_U32    mpclRBitAssignmentEnable;
        } mpclOAMConfigs;

        GT_U32    mpclPolicyGlobalConfig;
        GT_U32    mpclUnitInterruptCause;
        GT_U32    mpclUnitInterruptMask;
        GT_U32    mpclPolicyEngineConfig;
        GT_U32    mpclBadAddressLatchReg;
        GT_U32    mpclPolicyLookupGlobalConfig;
        GT_U32    mpclPolicyGlobalConfigExt1;
        GT_U32    mpclCountingModeConfig;
        GT_U32    mpclMaxSduSizeProfile[4]/*profile num*/;
        GT_U32    mpclMaxSduException;
        GT_U32    mpclGenClassMaskGlobalConfig0;
        GT_U32    mpclGenClassMaskGlobalConfig1;
        GT_U32    mpclGenClassLsbGlobalConfig;
        GT_U32    mpclPolicyGenericActionConfig;
        GT_U32    mpclFlowIdActionAssignmentModeConfig;
        GT_U32    mpclIpeKeySelection[4];
        GT_U32    mpclActionModifiedFieldsSelect[2]; /*reg idx */
        GT_U32    mpclExactMatchProfileIdMapEntry[256]/*entry num*/;
        GT_U32    mpclRangeClassGlobalConfig;
        GT_U32    mpclRangeProfilerConfig;
        GT_U32    mpclRangeClass0[64];
        GT_U32    mpclRangeClass1[64];
    } MPCL;

    struct /*rxDMA*/{

        struct /*singleChannelDMAConfigs*/{

            GT_U32    SCDMAConfig0[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    SCDMAConfig1[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    SCDMACTPktIndentification[PRV_CPSS_MAX_DMA_NUM_CNS]/*port*/;

        }singleChannelDMAConfigs;

        struct /*globalRxDMAConfigs*/{

            struct /*rejectConfigs*/{

                GT_U32    rejectPktCommand;
                GT_U32    rejectCPUCode;

            }rejectConfigs;

            struct /*globalConfigCutThrough*/{

                GT_U32    CTEtherTypeIdentification;
                GT_U32    CTUPEnable;
                GT_U32    CTGeneralConfiguration;

            }globalConfigCutThrough;

            struct /*buffersLimitProfiles*/{

                GT_U32    portsBuffersLimitProfileXONXOFFConfig[8];
                GT_U32    portsBuffersLimitProfileRxlimitConfig[8];
            }buffersLimitProfiles;

            struct /*preIngrPrioritizationConfStatus*/{

                GT_U32    VLANUPPriorityArrayProfile[4]/*profile*/;
                GT_U32    MPLSEtherTypeConf[2]/*MPLS*/;
                GT_U32    VIDConf[4]/*index*/;
                GT_U32    genericEtherType[4]/*index*/;
                GT_U32    qoSDSAPriorityArrayProfileSegment[4][8]/*profile*//*segment*/;
                GT_U32    preingressPrioritizationEnable;           /* removed from sip5.20 (moved to IA unit) , returned in sip6 !*/
                GT_U32    PIPPriorityDropGlobalCntrsClear;          /* removed from sip6 */
                GT_U32    PIPVeryHighPriorityDropGlobalCntrLsb;     /* removed from sip6 */
                GT_U32    PIPVeryHighPriorityDropGlobalCntrMsb;     /* removed from sip6 */
                GT_U32    PIPPrioThresholds;                        /* removed from sip5.20 (moved to IA unit) */
                GT_U32    PIPMedPriorityDropGlobalCntrMsb;          /* removed from sip6 */
                GT_U32    PIPMedPriorityDropGlobalCntrLsb;          /* removed from sip6 */
                GT_U32    PIPLowPriorityDropGlobalCntrMsb;          /* removed from sip6 */
                GT_U32    VLANEtherTypeConf[4]/*index*/;            /* in sip6 called 'OS TPIDs' */
                GT_U32    MACDA2MsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPHighPriorityDropGlobalCntrLsb;         /* removed from sip6 */
                GT_U32    MPLSEXPPriorityArrayProfile[4]/*profile*/;
                GT_U32    UPDSATagArrayProfile[4]/*profile*/;
                GT_U32    MACDAPriorityArrayProfile[4]/*profile*/;
                GT_U32    MACDAMask4LsbConf[4]/*mask*/;
                GT_U32    MACDAMask2MsbConf[4]/*mask*/;
                GT_U32    MACDA4LsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPLowPriorityDropGlobalCntrLSb;          /* removed from sip6 */
                GT_U32    IPv6TCPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv6EtherTypeConf;
                GT_U32    IPv4ToSPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv4EtherTypeConf;
                GT_U32    PIPHighPriorityDropGlobalCntrMsb;         /* removed from sip6 */
                GT_U32    PIPDropGlobalCounterTrigger;              /* removed from sip6 */
                GT_U32    etherTypePriorityArrayProfile[4]/*profile*/;

                GT_U32    profileTPID[4]/*Profile*/;   /* new in sip6*/
                GT_U32    latencySensitiveEtherTypeConf[4]/*index*/;/* new in sip6*/
                GT_U32    channelPIPConfigReg[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;/* new in sip6 - instead of SCDMA_n_PIP_Config */
                GT_U32    preMacDaSizeReg[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;/* new in sip6.10 */

                GT_U32    contextId_PIPPrioThresholds0;  /* new in sip6*/
                GT_U32    contextId_PIPPrioThresholds1;  /* new in sip6*/

            }preIngrPrioritizationConfStatus;

        }globalRxDMAConfigs;

        struct /* SCDMA Debug*/{
            GT_U32 portBuffAllocCnt[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
        }scdmaDebug;

        struct /*SIP_COMMON_MODULES*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }SIP_COMMON_MODULES;

        struct /*interrupts*/{

            GT_U32    rxDMASCDMAInterruptMask [PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    rxDMASCDMAInterruptCause[PRV_CPSS_MAX_DMA_NUM_CNS]/*Port*/;
            GT_U32    rxDMAInterrupt0Cause;
            GT_U32    rxDMAInterrupt0Mask;
            GT_U32    rxDMAInterruptSummary0Cause;
            GT_U32    rxDMAInterruptSummary0Mask;
            GT_U32    rxDMAInterruptSummary1Cause;
            GT_U32    rxDMAInterruptSummary1Mask;
            GT_U32    rxDMAInterruptSummary2Cause;
            GT_U32    rxDMAInterruptSummary2Mask;
            GT_U32    rxDMAInterruptSummary3Cause;
            GT_U32    rxDMAInterruptSummary3Mask;
            GT_U32    rxDMAInterruptSummaryCause;
            GT_U32    rxDMAInterruptSummaryMask;

        }interrupts;

        struct /* debug*/{

            struct /* counters_and_counters_statuses */{

                GT_U32    currentNumOfPacketsStoredInMppm;

            }countersAndCountersStatuses;

        }debug;

        struct /*singleChannelDMAPip*/ {
            GT_U32    SCDMA_n_PIP_Config      [PRV_CPSS_MAX_DMA_NUM_CNS];
            GT_U32    SCDMA_n_Drop_Pkt_Counter[PRV_CPSS_MAX_DMA_NUM_CNS];
        }singleChannelDMAPip;

    }rxDMA[MAX_DP_CNS];

    struct /*ingressAggregator*/ {
        GT_U32  PIPPrioThresholds0;
        GT_U32  PIPPrioThresholds1;
        GT_U32  PIPEnable;

        /* new in SIP6 */
        GT_U32  PIPLsThreshold;
        GT_U32  PIPPrioThresholdsRandomization;

        struct {
            GT_U32  PIPPfcCounterThresholds;
            GT_U32  PIPPfcGlobalFillLevelThresholds;
            GT_U32  GlobalTcPfcTriggerVectorsConfig;
            GT_U32  PIPTcPfcTriggerVectorsConfig[2];
        }pfcTriggering;/* new in SIP6 */

    }ingressAggregator[MAX_PIPES_PER_TILE_CNS];/* in BC3,Falcon - per 'pipe' */

    struct /*txDMA*/{

        struct /*txDMAThresholdsConfigs*/{

            GT_U32    RXDMAUpdatesFIFOsThresholds[1]/*rxdma*/;
            GT_U32    clearFIFOThreshold;
            GT_U32    headerReorderFifoThreshold;
            GT_U32    payloadReorderFifoThreshold;

        }txDMAThresholdsConfigs;

        struct /*txDMAPerSCDMAConfigs*/{

            GT_U32    burstLimiterSCDMA              [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    FIFOsThresholdsConfigsSCDMAReg1[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOCntrsConfigsSCDMA        [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs                   [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs1                  [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs2                  [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA reg2*/;
            GT_U32    SCDMAMetalFixReg               [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMARateLimitResidueVectorThr [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
        }txDMAPerSCDMAConfigs;

        struct /*txDMAInterrupts*/{

            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg1;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg1;
            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg2;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg3;
            GT_U32    SCDMAIDFIFOOverrunInterruptMaskReg3;
            GT_U32    RXDMAUpdatesFIFOsOverrunInterruptCauseReg1;
            GT_U32    RXDMAUpdatesFIFOsOverrunInterruptMaskReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg1;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptMaskReg2;
            GT_U32    SCDMAWaitingForNextReplyInterruptCauseReg3;
            GT_U32    SCDMAIDFIFOOverrunInterruptCauseReg3;
            GT_U32    txDMAGeneralCauseReg1;
            GT_U32    txDMAGeneralMaskReg1;
            GT_U32    txDMASummaryCauseReg;
            GT_U32    txDMASummaryMaskReg;

        }txDMAInterrupts;

        struct /*txDMAGlobalConfigs*/{

            GT_U32    txDMAGlobalConfig1;
            GT_U32    txDMAGlobalConfig3;
            GT_U32    ingrCoreIDToMPPMMap;
            GT_U32    TXDMAMetalFixReg;
            GT_U32    srcSCDMASpeedTbl[10];
            GT_U32    localDevSrcPort2DMANumberMap[128];
        }txDMAGlobalConfigs;

        struct /*txDMADescCriticalFieldsECCConfigs*/{

            GT_U32    dataECCConfig;
            GT_U32    dataECCStatus;

        }txDMADescCriticalFieldsECCConfigs;

        struct /*txDMADebug*/{

            struct /*informativeDebug*/{

                GT_U32    nextBCFIFOFillLevel[1]/*core*/;
                GT_U32    nextBCFIFOMaxFillLevel[1]/*core*/;
                GT_U32    SCDMAStatusReg1          [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2          [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOFillLevel   [PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOMaxFillLevel[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    descOverrunReg1;
                GT_U32    descOverrunReg2;
                GT_U32    descOverrunReg3;
                GT_U32    clearFIFOFillLevel;
                GT_U32    clearFIFOMaxFillLevel;
                GT_U32    headerReorderFIFOFillLevel;
                GT_U32    headerReorderFIFOMaxFillLevel;
                GT_U32    payloadReorderFIFOFillLevel;
                GT_U32    payloadReorderFIFOMaxFillLevel;
                GT_U32    descIDFIFOFillLevel;
                GT_U32    descIDFIFOMaxFillLevel;
                GT_U32    debugBusCtrlReg;
                GT_U32    txDMADebugBus;
                GT_U32    incomingDescsCntrLsb;
                GT_U32    incomingDescsCntrMsb;
                GT_U32    outgoingDescsCntrLsb;
                GT_U32    outgoingDescsCntrMsb;
                GT_U32    outgoingClearRequestsCntrLsb;
                GT_U32    outgoingClearRequestsCntrMsb;
                GT_U32    MPPMReadRequestsCntr;
                GT_U32    MPPMReadAnswersCntrLsb;
                GT_U32    MPPMReadAnswersCntrMsb;
                GT_U32    outgoingPayloadsCntrLsb;
                GT_U32    outgoingPayloadsCntrMsb;
                GT_U32    outgoingPayloadsEOPCntrLsb;
                GT_U32    outgoingPayloadsEOPCntrMsb;
                GT_U32    outgoingHeadersEOPCntrLsb;
                GT_U32    outgoingHeadersEOPCntrMsb;
                GT_U32    outgoingCTBCUpdatesCntr;
                GT_U32    HADescIDDBFreeIDFIFOFillLevel;
                GT_U32    internalDescIDDBFreeIDFIFOFillLevel;
                GT_U32    headerReorderMemIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadReorderMemIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txDMADebug;

        struct /*txDMAPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txDMAPizzaArb;

    }txDMA[MAX_DP_CNS];

    struct /*txFIFO*/{

        struct /*txFIFOShiftersConfig*/{

            GT_U32    SCDMAShiftersConf[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;

        }txFIFOShiftersConfig;

        struct /*txFIFOLinkedLists*/{

            struct /*payloadLinkedList*/{

                struct /*payloadLinkListRAM*/{

                    GT_U32    payloadLinkListAddr;

                }payloadLinkListRAM;

            }payloadLinkedList;

            struct /*headerLinkedList*/{

                struct /*headerLinkListRAM*/{

                    GT_U32    headerLinkListAddr;

                }headerLinkListRAM;

            }headerLinkedList;

        }txFIFOLinkedLists;

        struct /*txFIFOInterrupts*/{

            GT_U32    txFIFOGeneralCauseReg1;
            GT_U32    txFIFOGeneralMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg3;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg3;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg1;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg1;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg2;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg2;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg3;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg3;
            GT_U32    txFIFOSummaryCauseReg;
            GT_U32    txFIFOSummaryMaskReg;

        }txFIFOInterrupts;

        struct /*txFIFOGlobalConfig*/{

            GT_U32    SCDMAPayloadThreshold[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOMetalFixReg;

        }txFIFOGlobalConfig;

        struct /*txFIFOEnd2EndECCConfigs*/{

            GT_U32    dataECCConfig[2]/*ecc*/;
            GT_U32    dataECCStatus[2]/*ecc*/;

        }txFIFOEnd2EndECCConfigs;

        struct /*txFIFODebug*/{

            struct /*informativeDebug*/{

                GT_U32    SCDMAStatusReg6[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg1[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg3[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg4[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg5[PRV_CPSS_MAX_DMA_NUM_CNS]/*SCDMA*/;
                GT_U32    debugBusCtrlReg;
                GT_U32    txFIFODebugBus;
                GT_U32    LLEOPCntrLsb;
                GT_U32    LLEOPCntrMsb;
                GT_U32    incomingHeadersCntrLsb;
                GT_U32    incomingHeadersCntrMsb;
                GT_U32    headerIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadIDDBFreeIDFIFOFillLevel;
                GT_U32    CTBCIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txFIFODebug;

        struct /*txFIFOPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txFIFOPizzaArb;

    }txFIFO[MAX_DP_CNS];

    struct /*SIP_ETH_TXFIFO*/{

        struct /*txFIFOShiftersConfig*/{

            GT_U32    SCDMAShiftersConf[73]/*scdma*/;

        }txFIFOShiftersConfig;

        struct /*txFIFOPizzaArb*/{
            PRV_CPSS_DXCH_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txFIFOPizzaArb;

        struct /*txFIFOLinkedLists*/{

            struct /*payloadLinkedList*/{

                struct /*payloadLinkListRAM*/{

                    GT_U32    payloadLinkListAddr;

                }payloadLinkListRAM;

            }payloadLinkedList;

            struct /*headerLinkedList*/{

                struct /*headerLinkListRAM*/{

                    GT_U32    headerLinkListAddr;

                }headerLinkListRAM;

            }headerLinkedList;

        }txFIFOLinkedLists;

        struct /*txFIFOInterrupts*/{

            GT_U32    txFIFOGeneralCauseReg1;
            GT_U32    txFIFOGeneralMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg3;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg3;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg1;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg1;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg2;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg2;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg3;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg3;
            GT_U32    txFIFOSummaryCauseReg;
            GT_U32    txFIFOSummaryMaskReg;

        }txFIFOInterrupts;

        struct /*txFIFOGlobalConfig*/{

            GT_U32    SCDMAPayloadThreshold[73]/*scdma*/;
            GT_U32    txFIFOGlobalConfigs;
            GT_U32    TXFIFOMetalFixReg;

        }txFIFOGlobalConfig;

        struct /*txFIFOEnd2EndECCConfigs*/{

            GT_U32    dataECCStatus[1]/*ecc*/;
            GT_U32    dataECCConfig[1]/*ecc*/;
            GT_U32    eccErrorCntr;
            GT_U32    ECCFailedAddr;
            GT_U32    eccErrorType;
            GT_U32    lockErronousEvent;

        }txFIFOEnd2EndECCConfigs;

        struct /*txFIFODebug*/{

            struct /*informativeDebug*/{

                GT_U32    SCDMAStatusReg6[73]/*scdma*/;
                GT_U32    SCDMAStatusReg1[73]/*scdma*/;
                GT_U32    SCDMAStatusReg5[73]/*scdma*/;
                GT_U32    SCDMAStatusReg4[73]/*scdma*/;
                GT_U32    SCDMAStatusReg3[73]/*scdma*/;
                GT_U32    SCDMAStatusReg2[73]/*scdma*/;
                GT_U32    debugBusCtrlReg;
                GT_U32    txFIFODebugBus;
                GT_U32    LLEOPCntr;
                GT_U32    incomingHeadersCntr;
                GT_U32    headerIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadIDDBFreeIDFIFOFillLevel;
                GT_U32    CTBCIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

            struct /*activeDebug*/{

                GT_U32    debugSCDMA[73]/*scdma*/;
                GT_U32    globalActiveDebugReg;
                GT_U32    globalActiveDebugConfReg;
                GT_U32    debugSCDMAConfReg;

            }activeDebug;

        }txFIFODebug;

    }SIP_ETH_TXFIFO[2];

    struct /*SIP_ILKN_TXFIFO*/{

        struct /*ilknTxFIFOInterrupts*/{

            GT_U32    ilknTxfifoGeneralInterruptCause;
            GT_U32    ilknTxfifoGeneralInterruptMask;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg1;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg1;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg1;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg1;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg2;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg2;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg2;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg2;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptCauseReg3;
            GT_U32    streamIDDataMemFullIlknTxfifoInterruptMaskReg3;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptCauseReg3;
            GT_U32    streamIDDescMemFullIlknTxfifoInterruptMaskReg3;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg1;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg1;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg2;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg2;
            GT_U32    ilknTxfifoUnderrunInterruptCauseReg3;
            GT_U32    ilknTxfifoUnderrunInterruptMaskReg3;
            GT_U32    ilknTxfifoSummaryCauseReg;
            GT_U32    ilknTxfifoSummaryMaskReg;

        }ilknTxFIFOInterrupts;

        struct /*ilknTxFIFOGlobalConfig*/{

            GT_U32    portDynamicMemoryAlocation[64]/*port_num*/;
            GT_U32    portTransmittingThreshold[64]/*port num*/;
            GT_U32    portChannelEnable[64]/*port_num*/;
            GT_U32    globalConfig;

        }ilknTxFIFOGlobalConfig;

        struct /*ilknTxFIFODebug*/{

            GT_U32    dataFifoFillLevelForPort[64]/*port_num*/;

        }ilknTxFIFODebug;

        struct /*ilknTxFIFOArbiter*/{

            GT_U32    pizzaSlicePortMap[1064]/*slice num*/;
            GT_U32    pizzaArbiterCtrlReg;
            GT_U32    pizzaArbiterStatusReg;

        }ilknTxFIFOArbiter;

    }SIP_ILKN_TXFIFO;

    struct /*ERMRK*/{

        GT_U32    ERMRKGlobalConfig;
        GT_U32    ERMRKInterruptCause;
        GT_U32    ERMRKInterruptMask;
        GT_U32    ERMRKPTPConfig;
        GT_U32    timestampingUDPChecksumMode;
        GT_U32    timestampQueueMsgType;
        GT_U32    timestampQueueEntryID;
        GT_U32    PTPNTPOffset;
        GT_U32    ERMRKUP2UPMapTable[12]/*set*/;
        GT_U32    ERMRKPTPInvalidTimestampCntr;
        GT_U32    ERMRKDP2CFITable;
        GT_U32    ERMRKEXP2EXPMapTable[12]/*set*/;
        GT_U32    timestampQueueEntryWord0[4]/*queue*/;
        GT_U32    timestampQueueEntryWord1[4]/*queue*/;
        GT_U32    timestampQueueEntryWord2[2]/*queue*/;
        GT_U32    timestampQueueEntryIDClear;
        GT_U32    ECCErrorInjection;
        GT_U32    ERMRKBadAddrLatch;
        GT_U32    ERMRKSerConfig;
        GT_U32    cumulativeScaleRateTaiReg0[4];
        GT_U32    cumulativeScaleRateTaiReg1[4];

    }ERMRK;

    struct /*BM*/{

        struct /*BMInterrupts*/{

            GT_U32    BMGeneralCauseReg1;
            GT_U32    BMGeneralMaskReg1;
            GT_U32    BMGeneralCauseReg2;
            GT_U32    BMGeneralMaskReg2;
            GT_U32    BMSummaryCauseReg;
            GT_U32    BMSummaryMaskReg;

        }BMInterrupts;

        struct /*BMGlobalConfigs*/{

            GT_U32    BMBufferTermination[5]/*reg*/;
            GT_U32    BMGlobalConfig1;
            GT_U32    BMGlobalConfig2;
            GT_U32    BMGlobalConfig3;
            GT_U32    BMPauseConfig;
            GT_U32    BMBufferLimitConfig1;
            GT_U32    BMBufferLimitConfig2;
            GT_U32    BMMetalFixReg;

        }BMGlobalConfigs;

        struct /*BMDebug*/{

            GT_U32    rxDMANextUpdatesFIFOMaxFillLevel[6]/*core*/;
            GT_U32    rxDMANextUpdatesFIFOFillLevel[6]/*core*/;
            GT_U32    SCDMAInProgRegs[148]/*scdma*/;/*148 in bobk , 114 in bc2*/

        }BMDebug;

        struct /*BMCntrs*/{

            GT_U32    BMGlobalBufferCntr;
            GT_U32    rxDMAsAllocationsCntr;
            GT_U32    BMFreeBuffersInsideBMCntr;
            GT_U32    BMRxDMAPendingAllocationCntr;
            GT_U32    BMCTClearsInsideBMCntr;
            GT_U32    incomingCTClearsCntr;
            GT_U32    incomingRegularClearsCntr;
            GT_U32    nextTableAccessCntr;
            GT_U32    nextTableWriteAccessCntr;
            GT_U32    BMTerminatedBuffersCntr;
            GT_U32    BMLostClearRequestsCntr;

        }BMCntrs;

    }BM;

    struct /*OAMUnit*/{

        GT_U32    OAMGlobalCtrl;
        GT_U32    agingEntryGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    summaryExceptionGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    txPeriodExceptionGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    sourceInterfaceMismatchCntr;
        GT_U32    OAMGlobalCtrl1;
        GT_U32    RDIStatusChangeExceptionGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    RDIStatusChangeCntr;
        GT_U32    OAMUnitInterruptMask;
        GT_U32    OAMUnitInterruptCause;
        GT_U32    OAMTableAccessData[4]/*Reg*/;
        GT_U32    OAMTableAccessCtrl;
        GT_U32    OAMLossMeasurementOpcodes[4]/*reg*/;
        GT_U32    OAMLMCountedOpcodes[2]/*reg*/;
        GT_U32    OAMKeepAliveOpcodes[4]/*reg*/;
        GT_U32    OAMInvalidKeepaliveExceptionConfigs;
        GT_U32    agingPeriodEntryHigh[8]/*entry*/;
        GT_U32    dualEndedLM;
        GT_U32    OAMExceptionConfigs2;
        GT_U32    OAMExceptionConfigs1;
        GT_U32    OAMExceptionConfigs;
        GT_U32    OAMDelayMeasurementOpcodes[4]/*reg*/;
        GT_U32    MEGLevelGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    MEGLevelExceptionCntr;
        GT_U32    sourceInterfaceGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    invalidKeepaliveHashCntr;
        GT_U32    invalidKeepaliveGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    hashBitsSelection;
        GT_U32    excessKeepaliveGroupStatus[MAX_OAM_GROUP_STATUS_CNS]/*reg*/;
        GT_U32    excessKeepaliveCntr;
        GT_U32    keepaliveAgingCntr;
        GT_U32    agingPeriodEntryLow[8]/*entry*/;
        GT_U32    txPeriodExceptionCntr;
        GT_U32    summaryExceptionCntr;
        GT_U32    exceptionSupressionConfig1;
        GT_U32    exceptionSupressionConfig2;
        GT_U32    recoveryPacketCommandConfig1;
        GT_U32    recoveryPacketCommandConfig2;
        GT_U32    recoveryPacketCommandConfig3;

    }OAMUnit[2];

    /* PCA units group: BRG, PZ_ARBITER, SFF, LMU, CTSU, MACSEC_EXT. */
    /* Exising LMU unit registers and tables adapted to use as is.   */
    /* New CTSU unit per datapath should be used instead of per port */
    /* GOP.PTP unit in PTP CPSS code.                                */
    /* All other PCA units are new and not affect existing CPSS code.*/

    struct /*PCA_BRG*/ {
        GT_U32 brgInterruptCause;
        GT_U32 brgInterruptMask;
        GT_U32 brgInterruptSummaryCause;
        GT_U32 brgInterruptSummaryMask;
        GT_U32 rxChannel[MAX_PCA_CHANNELS_CNS];
        GT_U32 rxChannelInterruptCause[MAX_PCA_CHANNELS_CNS]; /*0-15 and 16-31*/
        GT_U32 rxChannelInterruptMask[MAX_PCA_CHANNELS_CNS];  /*0-15 and 16-31*/
        GT_U32 rxChannel_0_15_InterruptSummaryCause;
        GT_U32 rxChannel_16_31_InterruptSummaryCause;
        GT_U32 rxChannel_32_47_InterruptSummaryCause;
        GT_U32 rxChannel_48_63_InterruptSummaryCause;
        GT_U32 rxChannel_0_15_InterruptSummaryMask;
        GT_U32 rxChannel_16_31_InterruptSummaryMask;
        GT_U32 rxChannel_32_47_InterruptSummaryMask;
        GT_U32 rxChannel_48_63_InterruptSummaryMask;
        GT_U32 txChannel[MAX_PCA_CHANNELS_CNS];
        GT_U32 txChannelStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 txChannelStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 txChannelCreditsStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 txChannelInterruptCause[MAX_PCA_CHANNELS_CNS]; /*0-15 and 16-31*/
        GT_U32 txChannelInterruptMask[MAX_PCA_CHANNELS_CNS];  /*0-15 and 16-31*/
        GT_U32 txChannel_0_15_InterruptSummaryCause;
        GT_U32 txChannel_16_31_InterruptSummaryCause;
        GT_U32 txChannel_32_47_InterruptSummaryCause;
        GT_U32 txChannel_48_63_InterruptSummaryCause;
        GT_U32 txChannel_0_15_InterruptSummaryMask;
        GT_U32 txChannel_16_31_InterruptSummaryMask;
        GT_U32 txChannel_32_47_InterruptSummaryMask;
        GT_U32 txChannel_48_63_InterruptSummaryMask;
        GT_U32 rxSdbSopCounter;
        GT_U32 rxSdbEopCounter;
        GT_U32 rxSliceSopCounter;
        GT_U32 rxSliceEopCounter;
        GT_U32 rxShortPacketsCounter;
        GT_U32 rxCounterCfgChannel;
        GT_U32 rxSdbSopCounterCfgChannel;
        GT_U32 rxSdbEopCounterCfgChannel;
        GT_U32 rxSliceSopCounterCfgChannel;
        GT_U32 rxSliceEopCounterCfgChannel;
        GT_U32 rxShortPacketsCounterCfgChannel;
        GT_U32 rxRotatedFifoStatus;
        GT_U32 rxRotatedFifoStatistics;
        GT_U32 rxCdcFifoStatus;
        GT_U32 rxCdcFifoStatistics;
        GT_U32 rxGlogalInterruptCause;
        GT_U32 rxGlobalInterruptMask;
        GT_U32 rxPaddingControl;
        GT_U32 txSdbSopCounter;
        GT_U32 txSdbEopCounter;
        GT_U32 txSliceSopCounter;
        GT_U32 txSliceEopCounter;
        GT_U32 txShortPacketsCounter;
        GT_U32 txPch_1_ByteCounter;
        GT_U32 txPch_8_BytesCounter;
        GT_U32 txCounterCfgChannel;
        GT_U32 txSdbSopCounterCfgChannel;
        GT_U32 txSdbEopCounterCfgChannel;
        GT_U32 txSliceSopCounterCfgChannel;
        GT_U32 txSliceEopCounterCfgChannel;
        GT_U32 txShortPacketsCounterCfgChannel;
        GT_U32 txPch_1_ByteCounterCfgChannel;
        GT_U32 txPch_8_BytesCounterCfgChannel;
        GT_U32 txCdcFifoStatus;
        GT_U32 txCdcFifoStatistics;
        GT_U32 txCreditCdcFifoStatus;
        GT_U32 txCreditCdcFifoStatistics;
        GT_U32 txCreditRotatedFifoStatus;
        GT_U32 txCreditRotatedFifoStatistics;
        GT_U32 txAccomulatorStatus;
        GT_U32 txAccomulatorStatistics;
        GT_U32 txGlogalInterruptCause;
        GT_U32 txGlobalInterruptMask;
        GT_U32 txPaddingControl;

        /* Packet Generator registers */
        GT_U32 dbgFrmGenCfg[MAX_PCA_PG_CHANNELS_CNS];
        GT_U32 dbgFrmGenCfgStat[MAX_PCA_PG_CHANNELS_CNS];
        GT_U32 dbgFrmGenTbl[MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 dbgFrmGenTblLen[MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 dbgFrmGenTblHeader[MAX_PCA_PG_INSTANCE_CNS][MAX_PCA_PG_HEADER_REGS_CNS];
        GT_U32 dbgFrmGenTblPayload[MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 cfgInitTx[MAX_PCA_PG_CHANNELS_CNS];
        GT_U32 cfgInitRx[MAX_PCA_PG_CHANNELS_CNS];
        GT_U32 pgsModesCfg;
        GT_U32 pgsModesCfg1;
    } PCA_BRG[MAX_PCA_CNS];

    struct {
        /* Ironman PG TX/RX registers */
        GT_U32 pgGlobalCfg;
        GT_U32 pgProfileHeaderCfg[IRONMAN_MAX_PCA_PG_PROFILES_CNS][IRONMAN_MAX_PCA_PG_HEADER_REGS_CNS];
        GT_U32 pgProfileGapsCfg[IRONMAN_MAX_PCA_PG_PROFILES_CNS];
        GT_U32 pgProfilePayloadCfg[IRONMAN_MAX_PCA_PG_PROFILES_CNS];
        GT_U32 pgChannelControl[IRONMAN_MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 pgChannelPacketNumber[IRONMAN_MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 pgChannelCounter[IRONMAN_MAX_PCA_PG_INSTANCE_CNS];
        GT_U32 pgChannelRateControl[IRONMAN_MAX_PCA_PG_INSTANCE_CNS];
    } PCA_PGC[2/*0-RX, 1-TX*/];

    struct /*PCA_PZ_ARBITER*/ {
        GT_U32 swTag[MAX_PCA_CHANNELS_CNS]; /* hawk, phoenix, harrier */
        GT_U32 configuration[320];
        GT_U32 configurationBank1[256]; /*AAS*/
        GT_U32 control;
        GT_U32 badAddr;
        GT_U32 badAddrInterruptCause;
        GT_U32 badAddrInterruptMask;
        GT_U32 channelEnable[MAX_PCA_CHANNELS_CNS]; /* ironman,AAS */
        GT_U32 channelEnableBank1[MAX_PCA_CHANNELS_CNS]; /* AAS */
        GT_U32 arbiterBankSelect;
    } PCA_PZ_ARBITER[MAX_PCA_CNS][2/*0-ingress,1-egress*/];

    struct /*PCA_SFF*/ {
        GT_U32 channelControlEnable[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelControlConfig[MAX_PCA_CHANNELS_CNS];
        GT_U32 llNextPointer[256];
        GT_U32 channelInterruptCause[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInterruptMask[MAX_PCA_CHANNELS_CNS];
        GT_U32 globalConfig;
        GT_U32 spareConfig;
        GT_U32 globalInterruptCause;
        GT_U32 globalInterruptMask;
        GT_U32 unitInterruptSummaryCause;
        GT_U32 unitInterruptSummaryMask;
        GT_U32 channel_InterruptSummaryCause; /*AAS*/
        GT_U32 channel_InterruptSummaryMask;  /*AAS*/
        GT_U32 channel_0_15_InterruptSummaryCause;
        GT_U32 channel_0_15_InterruptSummaryMask;
        GT_U32 channel_16_31_InterruptSummaryCause;
        GT_U32 channel_16_31_InterruptSummaryMask;
        GT_U32 channel_32_47_InterruptSummaryCause;
        GT_U32 channel_32_47_InterruptSummaryMask;
        GT_U32 channel_48_63_InterruptSummaryCause;
        GT_U32 channel_48_63_InterruptSummaryMask;
        GT_U32 channelTokenOccupStatist[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelOccupStatist[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelLLAttribStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEventStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelTokenCorrectStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 globalStatus;
        GT_U32 selChIdAndGlobalPacketInStatus;
        GT_U32 selChIdStatistSopOrEop;
        GT_U32 selChIdStatistExp24;
        GT_U32 selChIdStatistExp32;
        GT_U32 selChIdStatistExp34; /*AAS*/
        GT_U32 selChIdStatistExp35; /*AAS*/
        GT_U32 selChIdStatistExp36; /*AAS*/
        GT_U32 selChIdStatistExp37; /*AAS*/
        GT_U32 selChIdStatistExpOn;
        GT_U32 selChIdStatistExpGo;
        GT_U32 selChIdStatistThdToken0Halt;
        GT_U32 selChIdStatistThdToken1Halt;
        GT_U32 selChIdStatistNoToken0Halt;
        GT_U32 selChIdStatistNoToken1Halt;
        GT_U32 selChIdStatistPauseOn;
        GT_U32 selChIdStatistChannelUp;
        GT_U32 selChIdStatistPtpOn;
        GT_U32 selChIdStatistPrecisePtpOn;
        GT_U32 selChIdTokenCorrection10BStatist;
        GT_U32 selChIdTokenCorrection24BStatist;
        GT_U32 selChIdTokenCorrection32BStatist;
        GT_U32 selChIdTokenCorrection34BStatist; /*AAS*/
        GT_U32 selChIdTokenCorrection35BStatist; /*AAS*/
        GT_U32 selChIdTokenCorrection36BStatist; /*AAS*/
        GT_U32 selChIdTokenCorrection37BStatist; /*AAS*/
        GT_U32 selChIdSkipGoStatist;
        GT_U32 selChIdCreditReturnStatist;
    } PCA_SFF[MAX_PCA_CNS];

    struct /* PCA_BRG_SHM_INGR */ { /*AAS*/
        /* regulal unit - 26 channels, FlexE unit - 86 channels */
        /* regulal unit - 26 queues, FlexE unit - 168 queues */
        GT_U32 queueControl[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueFillLevelThreshold[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueStatus[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueFillLevelCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queuePacketSizeCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueGoodPacketCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueBadPacketCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueDiscardedPacketCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueTruncatedPacketCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 queueOverThresholdCount[MAX_PCA_BRG_SHM_INGR_QUEUES_CNS];
        GT_U32 channelInterruptCause[MAX_PCA_BRG_SHM_INGR_CHANNELS_CNS];
        GT_U32 channelInterruptMask[MAX_PCA_BRG_SHM_INGR_CHANNELS_CNS];
        GT_U32 globalRxRemotePortConfig;
        GT_U32 globalLoopbackRemotePortConfig;
        GT_U32 globalStatus;
        GT_U32 globalLoopbackCodeConfig[16];
        GT_U32 interruptShmBadAddrCause;
        GT_U32 interruptShmBadAddrMask;
        GT_U32 interruptShmSummaryCause;
        GT_U32 interruptShmSummaryMask;
        GT_U32 interruptChannelInterruptSummaryCause[3];
        GT_U32 interruptChannelInterruptSummaryMask[3];
    } PCA_BRG_SHM_INGR[MAX_PCA_CNS];

    struct /* PCA_BRG_SHM_EGR */ { /*AAS*/
        GT_U32 channelConfig[MAX_PCA_CHANNELS_CNS];
        GT_U32 remotePortConfig[82];
        GT_U32 group0QueueInterruptCause[27];
        GT_U32 group0QueueInterruptMask[27];
        GT_U32 group1QueueInterruptCause[27];
        GT_U32 group1QueueInterruptMask[27];
        GT_U32 group2QueueInterruptCause[27];
        GT_U32 group2QueueInterruptMask[27];
        GT_U32 group0QueueInterruptSummaryCause;
        GT_U32 group1QueueInterruptSummaryCause;
        GT_U32 group2QueueInterruptSummaryCause;
        GT_U32 group0QueueInterruptSummaryMask;
        GT_U32 group1QueueInterruptSummaryMask;
        GT_U32 group2QueueInterruptSummaryMask;
        GT_U32 remotePortOccupancyStatistics[82];
        GT_U32 globalControl;
        GT_U32 loopbackTimestampInsertionEnable;
        GT_U32 globalInterruptCause;
        GT_U32 globalInterruptMask;
        GT_U32 globalInterruptSummaryCause;
        GT_U32 globalInterruptSummaryMask;
        GT_U32 illegialAddrStatus;
        GT_U32 llSelectedQueuePointerStatus;
        GT_U32 llSelectedQueueOccupancyStatus;
        GT_U32 llFreeColumnPointerStatus;
        GT_U32 llFreeColumnOccupancyStatus;
        GT_U32 packetsOnTheFlyStatus;
        GT_U32 selectedQueueCreditStatus;
        GT_U32 llGlobalStatus;
    } PCA_BRG_SHM_EGR[MAX_PCA_CNS];

    struct /* PCA_BRG_UNPACK_TX */ { /*AAS*/
        GT_U32 channelControlEnable[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfControlConfig[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfControlConfig[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMifCredits[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInerruptCause[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInerruptMask[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfOccupancyStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfOccupancyStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMifCreditStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelBlockPointerStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfRowPointerStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfRowPointerStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfOccupancyStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfCreditSkipOccupancyStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 globalConfig;
        GT_U32 globalPchLoopsConfig;
        GT_U32 spareConfig;
        GT_U32 globalInerruptCause;
        GT_U32 globalInerruptMask;
        GT_U32 illegialApbAddressStatus;
        GT_U32 selectedChannelPacketsInPipeStatus;
        GT_U32 selectedChannelInputSopOrEopStatistics;
        GT_U32 selectedChannelOutputSopOrEopStatistics;
        GT_U32 selectedChannelMcrfValidOrIdleSlicesStatistics;
        GT_U32 selectedChannelMcrfReadBlocksStatistics;
        GT_U32 selectedChannelMcrfWriteBlocksStatistics;
        GT_U32 selectedChannelRcrfReadFlagsStatistics;
        GT_U32 selectedChannelRcrfWriteFlagsStatistics;
        GT_U32 selectedChannelSetLoopPacketsStatistics;
        GT_U32 selectedChannelClearedSetLoopPacketsStatistics;
        GT_U32 selectedChannelTxTrafficHaltStatistics;
        GT_U32 selectedChannelPtpDispatcOnStatistics;
    } PCA_BRG_UNPACK_TX[MAX_PCA_CNS];

    struct /* PCA_BRG_PACK_RX */ { /*AAS*/
        GT_U32 channelControlEnable[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelControlConfig[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInerruptCause[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInerruptMask[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfOccupancyStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfOccupancyStatistics[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelBlockPointerStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRowPointerStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelMcrfOccupancyStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelRcrfOccupancyStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 globalConfig;
        GT_U32 spareConfig;
        GT_U32 globalInerruptCause;
        GT_U32 globalInerruptMask;
        GT_U32 illegialApbAddressStatus;
        GT_U32 grpSummaryInerruptCause;
        GT_U32 grpSummaryInerruptMask;
        GT_U32 unitSummaryInerruptCause;
        GT_U32 unitSummaryInerruptMask;
        GT_U32 selectedChannelPacketsInPipeStatus;
        GT_U32 selectedChannelInputSopOrEopStatistics;
        GT_U32 selectedChannelOutputSopOrEopStatistics;
        GT_U32 selectedChannelMcrfValidOrIdleSlicesStatistics;
        GT_U32 selectedChannelNumberOfShortPacketsStatistics;
        GT_U32 selectedChannelMcrfReadBlocksStatistics;
        GT_U32 selectedChannelMcrfWriteBlocksStatistics;
        GT_U32 selectedChannelRcrfReadFlagsStatistics;
        GT_U32 selectedChannelRcrfWriteFlagsStatistics;
    } PCA_BRG_PACK_RX[MAX_PCA_CNS];

    struct /*PCA_CTSU*/ {
        GT_U32 channelTsControl[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelTsPrecision[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressPipeDelay[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressPipeDelayFuturistic[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelTsQueue0EntryClear[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelTsQueue1EntryClear[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInterruptCause[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelInterruptMask[MAX_PCA_CHANNELS_CNS];
        GT_U32 channel_0_15_InterruptSummaryCause;
        GT_U32 channel_16_31_InterruptSummaryCause;
        GT_U32 channel_0_15_InterruptSummaryMask;
        GT_U32 channel_16_31_InterruptSummaryMask;
        GT_U32 channelEgressTsQueue0MaxOccupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue0MinOccupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue1MaxOccupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue1MinOccupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue0Occupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue1Occupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelPipelineFrameOccupancy[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelTsProcessingStatus[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue0EntryLow[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue0EntryHigh[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue1EntryLow[MAX_PCA_CHANNELS_CNS];
        GT_U32 channelEgressTsQueue1EntryHigh[MAX_PCA_CHANNELS_CNS];
        GT_U32 ctsuControl;
        GT_U32 ptpNtpOffset;
        GT_U32 pchControl;
        GT_U32 pchStaticLow;
        GT_U32 pchStaticHigh;
        GT_U32 pchCaptureControl;
        GT_U32 ctsuStatusClear;
        GT_U32 tsMaskingProfile[8];
        GT_U32 ctsuGlobalInterruptCause;
        GT_U32 ctsuGlobalInterruptMask;
        GT_U32 ctsuGlobalInterruptSummaryCause;
        GT_U32 ctsuGlobalInterrupSummarytMask;
        GT_U32 tsFrameCounter[64];
        GT_U32 tsFrameCounterControl[64];
        GT_U32 pchCaptureLowStatus;
        GT_U32 pchCaptureHighStatus;
        GT_U32 pchCaptureGeneralStatus;
        GT_U32 pchParsingErrorLowStatus;
        GT_U32 pchParsingErrorHighStatus;
        GT_U32 pchParsingErrorGeneralStatus;
        GT_U32 fieldDetectionErrorCommandStatus;
        GT_U32 fieldDetectionErrorGeneralStatus;
        GT_U32 lookAheadDataErrorLowStatus;
        GT_U32 lookAheadDataErrorHighStatus;
        GT_U32 lookAheadDataErrorGeneralStatus;
        GT_U32 cfUpdateAddSecGeneralStatus;
        GT_U32 globalPipelineFrameOccupancy;
    } PCA_CTSU[MAX_PCA_CNS];

    struct /*PCA_MACSEC_EXT*/ {
        GT_U32 errorHandlerControl;
        GT_U32 attributesCapture;
        GT_U32 packetErrorControl;
        GT_U32 dataCapture[32];
        GT_U32 msWrapperMisc;
        GT_U32 msPktIntegrChkSwReset; /*AAS*/
        GT_U32 msExtTcamCfg; /*AAS*/
        GT_U32 msTransApbTermBadAddr; /*AAS*/
        GT_U32 msChannelConfig[MAX_PCA_CHANNELS_CNS]; /*Ironman, AAS*/
        GT_U32 msInterruptCause;
        GT_U32 msInterruptMask;
        GT_U32 msChannelInterrupts0Cause;
        GT_U32 msChannelInterrupts1Cause;
        GT_U32 msChannelInterrupts2Cause;
        GT_U32 msChannelInterrupts3Cause;
        GT_U32 msChannelInterrupts0Mask;
        GT_U32 msChannelInterrupts1Mask;
        GT_U32 msChannelInterrupts2Mask;
        GT_U32 msChannelInterrupts3Mask;
        GT_U32 msWrongTxNrxInterrupts0Cause; /*AAS*/
        GT_U32 msWrongTxNrxInterrupts0Mask; /*AAS*/
        GT_U32 msWrongTxNrxInterrupts1Cause; /*AAS*/
        GT_U32 msWrongTxNrxInterrupts1Mask; /*AAS*/
        GT_U32 msInterruptSummaryCause; /*AAS*/
        GT_U32 msInterruptSummaryMask; /*AAS*/
        GT_U32 msWrapperStatus;
        struct /*sdbDbgMonitor*/ {
            GT_U32 sdbDbgMonitorControl;
            GT_U32 packetLength;
            GT_U32 maxInnerPacketGap;
            GT_U32 sopEopError;
            GT_U32 numOfPackets;
            GT_U32 numOfErrorPackets;
            GT_U32 maxPacketGap;
            GT_U32 minPacketGap;
        } sdbDbgMonitor[2/*0-preMS,1-postMS*/];
    } PCA_MACSEC_EXT[MAX_PCA_CNS][2/*0-163,1-164*/][2/*0-egress,1-ingress*/];

    struct /*GOP*/{

        struct {

            GT_U32    TAIInterruptCause;
            GT_U32    TAIInterruptMask;
            GT_U32    TAICtrlReg0;
            GT_U32    TAICtrlReg1;
            GT_U32    timeCntrFunctionConfig0;
            GT_U32    timeCntrFunctionConfig1;
            GT_U32    timeCntrFunctionConfig2;
            GT_U32    frequencyAdjustTimeWindow;
            GT_U32    TODStepNanoConfigHigh;
            GT_U32    TODStepNanoConfigLow;
            GT_U32    TODStepFracConfigHigh;
            GT_U32    TODStepFracConfigLow;
            GT_U32    timeAdjustmentPropagationDelayConfigHigh;
            GT_U32    timeAdjustmentPropagationDelayConfigLow;
            GT_U32    triggerGenerationControl;     /* SIP_6 */
            GT_U32    clockControl;                 /* SIP_6 */
            GT_U32    triggerGenerationTODSecHigh;
            GT_U32    triggerGenerationTODSecMed;
            GT_U32    triggerGenerationTODSecLow;
            GT_U32    triggerGenerationTODNanoHigh;
            GT_U32    triggerGenerationTODNanoLow;
            GT_U32    triggerGenerationTODFracHigh;
            GT_U32    triggerGenerationTODFracLow;
            GT_U32    timeLoadValueSecHigh;
            GT_U32    timeLoadValueSecMed;
            GT_U32    timeLoadValueSecLow;
            GT_U32    timeLoadValueNanoHigh;
            GT_U32    timeLoadValueNanoLow;
            GT_U32    timeLoadValueFracHigh;
            GT_U32    timeLoadValueFracLow;
            GT_U32    timeCaptureValue0SecHigh;
            GT_U32    timeCaptureValue0SecMed;
            GT_U32    timeCaptureValue0SecLow;
            GT_U32    timeCaptureValue0NanoHigh;
            GT_U32    timeCaptureValue0NanoLow;
            GT_U32    timeCaptureValue0FracHigh;
            GT_U32    timeCaptureValue0FracLow;
            GT_U32    timeCaptureValue1SecHigh;
            GT_U32    timeCaptureValue1SecMed;
            GT_U32    timeCaptureValue1SecLow;
            GT_U32    timeCaptureValue1NanoHigh;
            GT_U32    timeCaptureValue1NanoLow;
            GT_U32    timeCaptureValue1FracHigh;
            GT_U32    timeCaptureValue1FracLow;
            GT_U32    timeCaptureStatus;
            GT_U32    timeUpdateCntr;
            GT_U32    PClkClockCycleConfigLow;
            GT_U32    PClkClockCycleConfigHigh;
            GT_U32    incomingClockInCountingConfigLow;
            GT_U32    incomingClockCounter; /* SIP_6 */
            GT_U32    incomingClockInCountingEnable;
            GT_U32    generateFunctionMaskSecMed;
            GT_U32    generateFunctionMaskSecLow;
            GT_U32    generateFunctionMaskSecHigh;
            GT_U32    generateFunctionMaskNanoLow;
            GT_U32    generateFunctionMaskNanoHigh;
            GT_U32    generateFunctionMaskFracLow;
            GT_U32    generateFunctionMaskFracHigh;
            GT_U32    externalClockPropagationDelayConfigLow;
            GT_U32    externalClockPropagationDelayConfigHigh;
            GT_U32    driftThresholdConfigLow;
            GT_U32    driftThresholdConfigHigh;
            GT_U32    driftAdjustmentConfigLow;
            GT_U32    driftAdjustmentConfigHigh;
            GT_U32    clockCycleConfigLow;
            GT_U32    clockCycleConfigHigh;
            GT_U32    clockCycleNanoSeconds; /* SIP_6 */
            GT_U32    clockCycleSeconds;     /* SIP_6 */
            GT_U32    captureTriggerCntr;
            GT_U32    clockReceptionEdge;   /* SIP_6 */
            GT_U32    clockGenerationEdge;  /* SIP_6 */
            GT_U32    serRxControl;
            GT_U32    serTxControl;
            GT_U32    frameLatency;
            GT_U32    manchesterControl;
            GT_U32    clockCycleHighLevelLength;   /* SIP_6 */
            GT_U32    TAIDriftCompensationControl; /* SIP_6.30 */
            GT_U32    fractionalNanosecondDriftAdjustment; /* SIP_6.30 */

        } TAI[36][5];/* FALCON: 4 eagle(5 Pair each - 1 Master pair + 2 EPCL pairs + 4 single TXQ) + 16 Raven(1 pair each), 20+16=36*/
                    /* IRONMAN: 5 master TAIs no slaves; uses TAI[0][0..4] only */

        /* SIP6_10 uses shared CTSU units moved into PCA unit group from GOP */
        /* see PCA_CTSU substructure                                         */
        /* the PTP structure below not relevant to SIP6_10 devices           */
        struct /*PTP[73]--Network PTPs[72] + ILKN PTP--*/{

            GT_U32    PTPInterruptCause;
            GT_U32    PTPInterruptMask;
            GT_U32    PTPGeneralCtrl;
            GT_U32    PTPTXTimestampQueue0Reg0;
            GT_U32    PTPTXTimestampQueue0Reg1;
            GT_U32    PTPTXTimestampQueue0Reg2;
            GT_U32    PTPTXTimestampQueue1Reg0;
            GT_U32    PTPTXTimestampQueue1Reg1;
            GT_U32    PTPTXTimestampQueue1Reg2;
            GT_U32    totalPTPPktsCntr;
            GT_U32    PTPv1PktCntr;
            GT_U32    PTPv2PktCntr;
            GT_U32    Y1731PktCntr;
            GT_U32    NTPTsPktCntr;
            GT_U32    NTPReceivePktCntr;
            GT_U32    NTPTransmitPktCntr;
            GT_U32    WAMPPktCntr;
            GT_U32    addCorrectedTimeActionPktCntr;
            GT_U32    NTPPTPOffsetHigh;
            GT_U32    noneActionPktCntr;
            GT_U32    forwardActionPktCntr;
            GT_U32    dropActionPktCntr;
            GT_U32    captureIngrTimeActionPktCntr;
            GT_U32    captureAddTimeActionPktCntr;
            GT_U32    captureAddIngrTimeActionPktCntr;
            GT_U32    captureAddCorrectedTimeActionPktCntr;
            GT_U32    captureActionPktCntr;
            GT_U32    addTimeActionPktCntr;
            GT_U32    addIngrTimeActionPktCntr;
            GT_U32    NTPPTPOffsetLow;
            GT_U32    txPipeStatusDelay;
            GT_U32    egressTimestampQueue;        /* SIP_6 */
            GT_U32    timestampMaskingProfile[8];  /* SIP_6 */
            GT_U32    timestampFrameCntr[4];          /* SIP_6 */
            GT_U32    timestampFrameCntrControl[4];   /* SIP_6 */

        }PTP[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS][2]/* Network PTPs + ILKN PTP; [0] - 1/10G/100G ports, [1] - 200G/400G ports - for SIP_6 and above */;

        struct /*CG*/{
            struct /*CG_CONVERTERS*/{
                GT_U32 CGMAControl0;
                GT_U32 CGMAControl1;
                GT_U32 CGMAConvertersStatus;
                GT_U32 CGMAConvertersResets;
                GT_U32 CGMAConvertersIpStatus;
                GT_U32 CGMAConvertersIpStatus2;
                GT_U32 CGMAConvertersFcControl0;
            }CG_CONVERTERS;
            struct /*CG_PORT*/{
                struct /*CG_PORT_MAC*/{
                    GT_U32 CGPORTMACCommandConfig;
                    GT_U32 CGPORTMACaddress0;
                    GT_U32 CGPORTMACaddress1;
                    GT_U32 CGPORTMACFrmLength;
                    GT_U32 CGPORTMACRxCrcOpt;
                    GT_U32 CGPORTMACStatus;
                    GT_U32 CGPORTMACStatNConfig;
                    GT_U32 CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_LAST_E];
                    GT_U32 CGPORTMACTxIpgLength;
                }CG_PORT_MAC;
            }CG_PORT;

        }CG[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct /*MTI*/{
            struct /*MTI100*/{
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
                GT_U32 xifMode;
                GT_U32 txIpgLength;
                GT_U32 crcMode;
                /*BR*/
                struct
                {
                  GT_U32 brCntrl;
                  GT_U32 brStatus;
                  GT_U32 brTxFragCount;
                  GT_U32 brRxFragCount;
                  GT_U32 brTxHoldCount;
                  GT_U32 brRxSmdErrCount;
                  GT_U32 brRxAssyErrCount;
                  GT_U32 brRxAssyOkCount;
                  GT_U32 brRxVerifyCount;
                  GT_U32 brRxResponseCount;
                  GT_U32 brTxVerifCount;
                }BR;

                struct
                {
                    GT_U32 emac_commandConfig;
                    GT_U32 emac_macAddr0;
                    GT_U32 emac_macAddr1;
                    GT_U32 emac_frmLength;
                    GT_U32 emac_cl01PauseQuanta;
                    GT_U32 emac_cl23PauseQuanta;
                    GT_U32 emac_cl45PauseQuanta;
                    GT_U32 emac_cl67PauseQuanta;
                    GT_U32 emac_cl01QuantaThresh;
                    GT_U32 emac_cl23QuantaThresh;
                    GT_U32 emac_cl45QuantaThresh;
                    GT_U32 emac_cl67QuantaThresh;

                }EMAC;

            }MTI100_MAC;

            struct /*MTI400*/{
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
                GT_U32 xifMode;
                GT_U32 txIpgLength;
            }MTI400_MAC;

            struct /*MTI CPU*/{
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
                GT_U32 xifMode;
                GT_U32 txIpgLength;
                GT_U32 crcMode;
                GT_U32 usxPchControl;

                /*BR*/
                struct
                {
                  GT_U32 brCntrl;
                  GT_U32 brStatus;
                  GT_U32 brTxFragCount;
                  GT_U32 brRxFragCount;
                  GT_U32 brTxHoldCount;
                  GT_U32 brRxSmdErrCount;
                  GT_U32 brRxAssyErrCount;
                  GT_U32 brRxAssyOkCount;
                  GT_U32 brRxVerifyCount;
                  GT_U32 brRxResponseCount;
                  GT_U32 brTxVerifCount;
                  GT_U32 brBwCtrl;
                  GT_U32 brBwSar;
                  GT_U32 brBwStatus;
                  GT_U32 brAvbControl;
                }BR;

                struct
                {
                    GT_U32 emac_commandConfig;
                    GT_U32 emac_macAddr0;
                    GT_U32 emac_macAddr1;
                    GT_U32 emac_frmLength;
                    GT_U32 emac_cl01PauseQuanta;
                    GT_U32 emac_cl01QuantaThresh;
                    GT_U32 emac_rxPauseStatus;
                    GT_U32 tsuControl;
                }EMAC;

            }MTI_CPU_MAC;

            struct /*MTI_EXT*/{
                GT_U32 portStatus;
                GT_U32 portControl;
                GT_U32 portPeerDelay;
                GT_U32 segPortPeerDelay;
                GT_U32 segPortControl; /* used for 200G/400G ports */
                GT_U32 segPauseOverride;/* used for 200G/400G ports */
                GT_U32 segXoffOverride;/* used for 200G/400G ports */
                GT_U32 pauseOverride;
                GT_U32 xoffOverride;
                GT_U32 portXoffStatus;
                GT_U32 segPortXoffStatus;/* used for 200G/400G ports */
                GT_U32 tsuControl0;
                GT_U32 tsuControl2;
                GT_U32 tsuControl3;
                GT_U32 portTsdControl0;
                GT_U32 segPortTsdControl0; /* used for 200G/400G ports */
                GT_U32 portTsdControl1;
                GT_U32 portTsdControl2;
                GT_U32 portPmacControl;
            }MTI_EXT;

            struct {
                GT_U32 portControl;
                GT_U32 portPeerDelay;
                GT_U32 pauseOverride;
                GT_U32 xoffOverride;
                GT_U32 portXoffStatus;
                GT_U32 tsuControl0;
                GT_U32 tsuControl2;
                GT_U32 portTsdControl0;
                GT_U32 portTsdControl1;
                GT_U32 portTsdControl2;
                GT_U32 portTsxControl2;
                GT_U32 portTsxControl3;
                GT_U32 portTsxControl4;
                GT_U32 portEmacUsxPchTxControl1;
                GT_U32 portPmacUsxPchTxControl1;
                GT_U32 portEmacUsxPchRxControl;
                GT_U32 portPmacUsxPchRxControl;
                GT_U32 portEmacUsxPchBadCrcCounter;
                GT_U32 tsqTimestamp;
                GT_U32 tsqSignature;
                GT_U32 tsqClearControl;
                GT_U32 pcsExtPortControl; /*Ironman PCS EXT control register*/
            }MTI_CPU_EXT;


            struct /*MTI_USX_EXT*/{
                GT_U32 portStatus;
                GT_U32 portControl;
                GT_U32 portPeerDelay;
                GT_U32 pauseOverride;
                GT_U32 xoffOverride;
                GT_U32 portXoffStatus;
                GT_U32 portTsxControl2;
                GT_U32 portTsxControl3;
                GT_U32 portTsxControl4;
                GT_U32 portTsdControl0;
                GT_U32 portTsdControl1;
                GT_U32 portTsdControl2;
                GT_U32 globalUsxPchControl;
                GT_U32 portUsxPchLastTimestamp;
                GT_U32 portUsxPchSignatureControl;
                GT_U32 portUsxPchControl;
                GT_U32 portUsxPchCrcErrorCounter;
                GT_U32 portEmacUsxPchTxControl1;
                GT_U32 portPmacUsxPchTxControl1;
                GT_U32 portEmacUsxPchRxControl;
                GT_U32 portPmacUsxPchRxControl;
                GT_U32 portEmacUsxPchBadCrcCounter;
                GT_U32 tsqTimestamp;
                GT_U32 tsqSignature;
                GT_U32 tsqClearControl;
                GT_U32 pcsExtPortControl; /*Ironman PCS EXT control register*/
            }MTI_USX_EXT;

            struct {
                GT_U32 commandConfig;
                GT_U32 frmLength;
                GT_U32 macAddr0;
                GT_U32 macAddr1;
                GT_U32 status;
                GT_U32 cl01PauseQuanta;
                GT_U32 cl23PauseQuanta;
                GT_U32 cl45PauseQuanta;
                GT_U32 cl67PauseQuanta;
                GT_U32 cl01QuantaThresh;
                GT_U32 cl23QuantaThresh;
                GT_U32 cl45QuantaThresh;
                GT_U32 cl67QuantaThresh;
                GT_U32 rxPauseStatus;
                GT_U32 xifMode;
                GT_U32 txIpgLength;
                GT_U32 crcMode;
                GT_U32 usxPchControl;

                /*BR*/
                struct
                {
                  GT_U32 brCntrl;
                  GT_U32 brStatus;
                  GT_U32 brTxFragCount;
                  GT_U32 brRxFragCount;
                  GT_U32 brTxHoldCount;
                  GT_U32 brRxSmdErrCount;
                  GT_U32 brRxAssyErrCount;
                  GT_U32 brRxAssyOkCount;
                  GT_U32 brRxVerifyCount;
                  GT_U32 brRxResponseCount;
                  GT_U32 brTxVerifCount;
                  GT_U32 brBwCtrl;
                  GT_U32 brBwSar;
                  GT_U32 brBwStatus;
                  GT_U32 brAvbControl;
                }BR;

                struct
                {
                    GT_U32 emac_commandConfig;
                    GT_U32 emac_macAddr0;
                    GT_U32 emac_macAddr1;
                    GT_U32 emac_frmLength;
                    GT_U32 emac_cl01PauseQuanta;
                    GT_U32 emac_cl23PauseQuanta;
                    GT_U32 emac_cl45PauseQuanta;
                    GT_U32 emac_cl67PauseQuanta;
                    GT_U32 emac_cl01QuantaThresh;
                    GT_U32 emac_cl23QuantaThresh;
                    GT_U32 emac_cl45QuantaThresh;
                    GT_U32 emac_cl67QuantaThresh;
                    GT_U32 emac_rxPauseStatus;
                    GT_U32 tsuControl;
                }EMAC;

            }MTI_USX_MAC;

            struct /*MTI_USX_LPCS*/{

                GT_U32 control;
                GT_U32 partnerAbility;
                GT_U32 portIfMode;
            }MTI_USX_LPCS;
        }MTI[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct /*MTI*/{
            GT_U32 dataHi;
            GT_U32 status;
            GT_U32 config;
            GT_U32 control;
            GT_U32 clearValueLo;
            GT_U32 clearValueHi;
            GT_U32 dataHiCdc;
            GT_U32 MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
        }MTI_STATISTICS[MAX_DP_CNS];

        struct /*MTI*/{
            GT_U32 dataHi;
            GT_U32 status;
            GT_U32 config;
            GT_U32 control;
            GT_U32 clearValueLo;
            GT_U32 clearValueHi;
            GT_U32 dataHiCdc;
            GT_U32 MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
        }MTI_USX_STATISTICS[MAX_DP_CNS];

        struct /*MTI CPU*/{
            GT_U32 dataHi;
            GT_U32 status;
            GT_U32 config;
            GT_U32 control;
            GT_U32 clearValueLo;
            GT_U32 clearValueHi;
            GT_U32 dataHiCdc;
            GT_U32 MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
        }MTI_CPU_STATISTICS[MAX_TILE_CNS * FALCON_RAVENS_PER_TILE];

         struct /*MSDB*/{
            GT_U32 fcuControl;
            /*8 channels*/
            GT_U32 fcuChannelTXControl[9];
            GT_U32 fcuChannelRXControl[9];
            GT_U32 fcuRXTimer;
        }MSDB[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS];

        struct LMS_ST/*LMS*/{   /* Bobcat2 A0 */

            struct /*LMS0*/{

                struct /*LMS0Group0*/{

                    struct /*SMIConfig*/{

                        GT_U32    PHYAddrReg0forPorts0Through5;
                        GT_U32    PHYAutoNegConfigReg0;
                        GT_U32    SMI0Management;
                        GT_U32    LMS0MiscConfigs;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP0MIBsInterruptCause;
                        GT_U32    portsGOP0MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACSourceAddr*/{

                        GT_U32    sourceAddrMiddle;
                        GT_U32    sourceAddrHigh;

                    }portsMACSourceAddr;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts0Through5;

                    }portsMACMIBCntrs;

                    struct /*portsAndMIBCntrsInterruptSummary*/{

                        GT_U32    MIBCntrsInterruptSummary;

                    }portsAndMIBCntrsInterruptSummary;

                    struct /*periodicFCRegs*/{

                        GT_U32    gigPortsPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface0CtrlReg0forPorts0Through11AndStackPort;
                        GT_U32    LEDInterface0Group01ConfigRegforPorts0Through11;
                        GT_U32    LEDInterface0Class01ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Class4ManipulationRegforPorts0Through11;
                        GT_U32    stackLEDInterface0Class04ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface0Class59ManipulationRegforStackPorts;
                        GT_U32    LEDInterface0FlexlinkPortsDebugSelectRegforStackPort;
                        GT_U32    LEDInterface0FlexlinkPortsDebugSelectReg1forStackPort;

                    }LEDInterfaceConfig;

                }LMS0Group0;

                struct /*LMS0Group1*/{

                    struct /*SMIConfig*/{

                        GT_U32    flowCtrlAdvertiseForFiberMediaSelectedConfigReg0forPorts0Through11;
                        GT_U32    PHYAddrReg1forPorts6Through11;
                        GT_U32    PHYAutoNegConfigReg1;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    triSpeedPortsGOP1MIBsInterruptCause;
                        GT_U32    triSpeedPortsGOP1MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts6Through11;

                    }portsMACMIBCntrs;

                    struct /*portsAndMIBCntrsInterruptSummary*/{

                        GT_U32    portsMIBCntrsInterruptSummaryMask;
                        GT_U32    portsInterruptSummaryMask;

                    }portsAndMIBCntrsInterruptSummary;

                    struct /*periodicFCRegs*/{

                        GT_U32    twoAndHalfGigPortsPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface0CtrlReg1AndClass6ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Group23ConfigRegforPorts0Through11;
                        GT_U32    LEDInterface0Class23ManipulationRegforPorts0Through11;
                        GT_U32    LEDInterface0Class5ManipulationRegforPorts0Through11;
                        GT_U32    stackLEDInterface0Class1011ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface0Group01ConfigRegforStackPort;

                    }LEDInterfaceConfig;

                    GT_U32    stackPortsModeReg;

                }LMS0Group1;

            }LMS0;

            struct /*LMS1*/{

                struct /*LMS1Group0*/{

                    struct /*SMIConfig*/{

                        GT_U32    PHYAddrReg2forPorts12Through17;
                        GT_U32    PHYAutoNegConfigReg2;
                        GT_U32    SMI1Management;
                        GT_U32    LMS1MiscConfigs;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP2MIBsInterruptCause;
                        GT_U32    portsGOP2MIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts12Through17;

                    }portsMACMIBCntrs;

                    struct /*portsInterruptSummary*/{

                        GT_U32    portsInterruptSummary;

                    }portsInterruptSummary;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface1CtrlReg0forPorts12Through23AndStackPort;
                        GT_U32    LEDInterface1Group01ConfigRegforPorts12Through23;
                        GT_U32    LEDInterface1Class01ManipulationRegforPorts12Through23;
                        GT_U32    LEDInterface1Class4ManipulationRegforPorts12Through23;
                        GT_U32    stackLEDInterface1Class04ManipulationRegforStackPorts;
                        GT_U32    stackLEDInterface1Class59ManipulationRegforStackPorts;
                        GT_U32    LEDInterface1FlexlinkPortsDebugSelectRegforStackPort;
                        GT_U32    LEDInterface1FlexlinkPortsDebugSelectReg1forStackPort;

                    }LEDInterfaceConfig;

                }LMS1Group0;

                struct /*LMS1Group1*/{

                    struct /*SMIConfig*/{

                        GT_U32    flowCtrlAdvertiseForFiberMediaSelectedConfigReg1forPorts12Through23;
                        GT_U32    PHYAddrReg3forPorts18Through23;
                        GT_U32    PHYAutoNegConfigReg3;

                    }SMIConfig;

                    struct /*portsMIBCntrsInterrupts*/{

                        GT_U32    portsGOP3AndStackMIBsInterruptCause;
                        GT_U32    portsGOP3AndStackMIBsInterruptMask;

                    }portsMIBCntrsInterrupts;

                    struct /*portsMACMIBCntrs*/{

                        GT_U32    MIBCntrsCtrlReg0forPorts18Through23;

                    }portsMACMIBCntrs;

                    struct /*periodicFCRegs*/{

                        GT_U32    stackPorts1GPeriodicFCCntrMaxValueReg;

                    }periodicFCRegs;

                    struct /*LEDInterfaceConfig*/{

                        GT_U32    LEDInterface1CtrlReg1AndClass6ManipulationforPorts12Through23;
                        GT_U32    LEDInterface1Group23ConfigRegforPorts12Through23;
                        GT_U32    LEDInterface1Class23ManipulationRegforPorts12Through23;
                        GT_U32    LEDInterface1Class5ManipulationRegforPorts12Through23;
                        GT_U32    stackLEDInterface1Class1011ManipulationRegforStackPort;
                        GT_U32    stackLEDInterface1Group01ConfigRegforStackPort;

                    }LEDInterfaceConfig;

                }LMS1Group1;

            }LMS1;

        }LMS[3];
        struct LED_ST    /* Bobcat2 B0 */
        {
            GT_U32 LEDControl;
            GT_U32 blinkGlobalControl;
            GT_U32 classGlobalControl;
            GT_U32 classesAndGroupConfig;
            GT_U32 classForcedData[6];
            GT_U32 portIndicationSelect[32];   /* BC2 -12 BobK  12 Aldrin 32 BC3 - 13 */
            GT_U32 portTypeConfig[2];          /* BC2   1 BobK   1 Aldrin  2 BC3   1  */
            GT_U32 portIndicationPolarity[8];  /* BC2 - 3 BobK   3 Aldrin  8 BC3 - 4  */
            GT_U32 classRawData[6];
        }LED[5];
        struct SMI_ST     /* Bobcat2 B0 */
        {
            GT_U32 SMIManagement;
            GT_U32 SMIMiscConfiguration;
            GT_U32 PHYAutoNegotiationConfig;
            GT_U32 PHYAddress[16];
        }SMI[4];

        PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC FCA[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS]/*ports 0..71*/;

        struct /*PR*/{

            struct /*PRMIB*/{

                GT_U32    MIBChannelSelect;
                GT_U32    MIBGeneralConfigs;
                GT_U32    MIBWraparoundInterruptCause0;
                GT_U32    MIBWraparoundInterruptMask0;
                GT_U32    MIBWraparoundInterruptCause1;
                GT_U32    MIBWraparoundInterruptMask1;
                GT_U32    MIBWraparoundInterruptCause2;
                GT_U32    MIBWraparoundInterruptMask2;

                GT_U32    MIBCountersBase;
            }PRMIB;

            struct /*PRInterrupts*/{

                GT_U32    rxFullInterruptCause0;
                GT_U32    rxFullInterruptMask0;
                GT_U32    rxFullInterruptCause1;
                GT_U32    rxFullInterruptMask1;
                GT_U32    rxFullInterruptCause2;
                GT_U32    rxFullInterruptMask2;
                GT_U32    noSOPInterruptCause0;
                GT_U32    noSOPInterruptMask0;
                GT_U32    noSOPInterruptCause1;
                GT_U32    noSOPInterruptMask2;
                GT_U32    noSOPInterruptCause2;
                GT_U32    noSOPInterruptMask1;
                GT_U32    fillOverrunInterruptCause0;
                GT_U32    fillOverrunInterruptMask0;
                GT_U32    fillOverrunInterruptCause1;
                GT_U32    fillOverrunInterruptMask1;
                GT_U32    noEOPInterruptMask2;
                GT_U32    fillOverrunInterruptMask2;
                GT_U32    noEOPInterruptMask0;
                GT_U32    fillUnderrunInterruptMask0;
                GT_U32    fillUnderrunInterruptCause1;
                GT_U32    fillUnderrunInterruptMask1;
                GT_U32    fillUnderrunInterruptCause2;
                GT_U32    fillUnderrunInterruptMask2;
                GT_U32    PRGeneralCauseReg;
                GT_U32    PRGeneralMaskReg;
                GT_U32    noEOPInterruptCause0;
                GT_U32    fillUnderrunInterruptCause0;
                GT_U32    noEOPInterruptCause1;
                GT_U32    noEOPInterruptMask1;
                GT_U32    noEOPInterruptCause2;
                GT_U32    fillOverrunInterruptCause2;
                GT_U32    PRSummaryCauseReg;
                GT_U32    PRSummaryMaskReg;

            }PRInterrupts;

            struct /*PRFunctional*/{

                GT_U32    channelConfig[64]/*Channel*/;
                GT_U32    minMaxPkts;
                GT_U32    general;

            }PRFunctional;

        }PR;

        struct /* Interlaken Register File */
        {
            GT_U32    ILKN_COUNT_BLK_TYPE_ERR_H[8]/*index*/;
            GT_U32    ILKN_COUNT_BLK_TYPE_ERR_L[16]/*index*/;
            GT_U32    ILKN_STAT_EN;
            GT_U32    ILKN_RX_PKT_GEN;
            GT_U32    ILKN_REMOTE_LANE_STATUS;
            GT_U32    ILKN_PTR_RST;
            GT_U32    ILKN_MAC_CFG_0;
            GT_U32    ILKN_PCS_PARAM;
            GT_U32    ILKN_MAC_CFG_1;
            GT_U32    ILKN_MAC_CFG_2;
            GT_U32    ILKN_MAC_CFG_3;
            GT_U32    ILKN_MAC_CFG_4;
            GT_U32    ILKN_MAC_CFG_5;
            GT_U32    ILKN_PCS_DEBUG_1;
            GT_U32    ILKN_PCS_DEBUG_0;
            GT_U32    ILKN_PCS_CFG_2;
            GT_U32    ILKN_PCS_CFG_1;
            GT_U32    ILKN_PCS_CFG_0;
            GT_U32    ILKN_OVF_7;
            GT_U32    ILKN_OVF_6;
            GT_U32    ILKN_MAC_RL_CFG_1;
            GT_U32    ILKN_OVF_4;
            GT_U32    ILKN_MAC_CFG_12;
            GT_U32    ILKN_CH_FC_CFG_1;
            GT_U32    ILKN_OVF_1;
            GT_U32    ILKN_OVF_0;
            GT_U32    ILKN_MAC_RL_CFG_2;
            GT_U32    ILKN_INT_CLR;
            GT_U32    ILKN_MAC_RL_CFG_0;
            GT_U32    ILKN_MAC_CBFC_BM_0_1;
            GT_U32    ILKN_MAC_CFG_9;
            GT_U32    ILKN_MAC_CFG_8;
            GT_U32    ILKN_MAC_CFG_7;
            GT_U32    ILKN_MAC_CFG_6;
            GT_U32    ILKN_MAC_CBFC_BM_2_0;
            GT_U32    ILKN_MAC_CBFC_BM_2_1;
            GT_U32    ILKN_MAC_CBFC_BM_2_2;
            GT_U32    ILKN_MAC_CBFC_BM_3_0;
            GT_U32    ILKN_MAC_CBFC_BM_3_1;
            GT_U32    ILKN_MAC_CFG_11;
            GT_U32    ILKN_MAC_CFG_10;
            GT_U32    ILKN_WORD_LOCK_STATUS;
            GT_U32    ILKN_SYNC_LOCK_STATUS;
            GT_U32    ILKN_MAC_CBFC_BM_CNTL;
            GT_U32    ILKN_MAC_CBFC_BM_3_2;
            GT_U32    ILKN_ERR_GEN_STATUS;
            GT_U32    ILKN_PCS_DEBUG_DATA;
            GT_U32    ILKN_MAC_FC_STATUS;
            GT_U32    ILKN_PROT_ERR_STATUS;
            GT_U32    ILKN_LINK_STATUS;
            GT_U32    ILKN_MAC_CBFC_BM_1_2;
            GT_U32    ILKN_MAC_CBFC_BM_1_1;
            GT_U32    ILKN_MAC_CBFC_BM_1_0;
            GT_U32    ILKN_MAC_CBFC_BM_0_2;
            GT_U32    ILKN_EN;
            GT_U32    ILKN_MAC_CBFC_BM_0_0;
            GT_U32    ILKN_LL_FC_STATUS;
            GT_U32    ILKN_FIFO_RST;
            GT_U32    ILKN_LANE_SWAP;
            GT_U32    ILKN_PCS_DEBUG_2;
            GT_U32    ILKN_PCS_DEBUG_3;
            GT_U32    ILKN_PCS_DEBUG_4;
            GT_U32    ILKN_PCS_DEBUG_5;
            GT_U32    ILKN_COUNT_WORD_SYNC_ERR_L[16]/*index*/;
            GT_U32    ILKN_COUNT_WORD_SYNC_ERR_H[8]/*index*/;
            GT_U32    ILKN_COUNT_TX_PKT_LOW;
            GT_U32    ILKN_COUNT_TX_PKT_HIGH;
            GT_U32    ILKN_COUNT_TX_BYTE_LOW;
            GT_U32    ILKN_COUNT_TX_BYTE_HIGH;
            GT_U32    ILKN_COUNT_TX_BAD_PKT_LOW;
            GT_U32    ILKN_COUNT_TX_BAD_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_PKT_LOW;
            GT_U32    ILKN_COUNT_RX_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_CRC_ERROR_LOW;
            GT_U32    ILKN_COUNT_RX_CRC_ERROR_HIGH;
            GT_U32    ILKN_COUNT_RX_BYTE_LOW;
            GT_U32    ILKN_COUNT_RX_BYTE_HIGH;
            GT_U32    ILKN_COUNT_RX_BURST_SIZE_ERROR_L;
            GT_U32    ILKN_COUNT_RX_BURST_SIZE_ERROR_H;
            GT_U32    ILKN_COUNT_RX_BAD_PKT_LOW;
            GT_U32    ILKN_COUNT_RX_BAD_PKT_HIGH;
            GT_U32    ILKN_COUNT_RX_ALIGNMENT_FAILURE;
            GT_U32    ILKN_COUNT_RX_ALIGNMENT_ERROR;
            GT_U32    ILKN_COUNT_DIAG_CRC_ERR_L[16]/*index*/;
            GT_U32    ILKN_COUNT_DIAG_CRC_ERR_H[8]/*index*/;
            GT_U32    ILKN_OVF_2;
            GT_U32    ILKN_OVF_3;
            GT_U32    ILKN_CH_FC_CFG_2;
            GT_U32    ILKN_OVF_5;
            GT_U32    ILKN_CH_FC_CFG_0;
            GT_U32    ILKN_ALIGN_STATUS;
        }ILKN;

        struct /* Interlaken Wrapper Register File */
        {
            GT_U32    sdFcLedConvertersControl0;
            GT_U32    txdmaConverterControl0;
            GT_U32    rxdmaConverterPacketSizeRestriction;

        }ILKN_WRP;

        /* MIF unit -- added for Ironman */
        struct /*MIF*/
        {
            struct
            {
                struct
                {
                    GT_U32 linkStatusFilter;
                } config;
                struct
                {
                    GT_U32 badAddrIntCause;
                    GT_U32 badAddrIntMask;
                    GT_U32 channelSummaryIntCause;
                    GT_U32 channelSummaryIntMask;
                    GT_U32 summaryIntCause;
                    GT_U32 summaryIntMask;
                } interrupt;
            } global;
            struct
            {
                struct
                {
                    GT_U32 txStatus[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 rxStatus[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                } status;
                struct
                {
                    GT_U32 txGoodPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 rxGoodPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 txBadPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 rxBadPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 txDiscardedPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 rxDiscardedPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 txLinkFailCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 sdbDiscardedShortPacketCount[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                } statistics;
                struct
                {
                    GT_U32 mapping[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 rxFifoReadyThershold[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 txCredit[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 channelFifoConfig[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 statusFsmControl[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                } config;
                struct
                {
                    GT_U32 channelViolationIntCause[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                    GT_U32 channelViolationIntMask[ALL_DEVS_MAX_MIF_CHANNELS_PER_MIF_CNS];
                } interrupt;
            } channel;
        } MIF[ALL_DEVS_MAX_MIF_CNS];
    }GOP;

    struct /*IPvX*/{
        struct /*routerManagementCntrsSets*/{

            GT_U32    routerManagementInUcNonRoutedExceptionPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcTrappedMirroredPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementInUcPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInUcNonRoutedNonException[4]/*set number*/;
            GT_U32    routerManagementInMcNonRoutedNonExceptionPktsCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcNonRoutedExceptionPktsCntrSet[4]/*set nmber*/;
            GT_U32    routerManagementInUcTrappedMirroredPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementInMcRPFFailCntrSet[4]/*set number*/;
            GT_U32    routerManagementOutUcPktCntrSet[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig0[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig1[4]/*set number*/;
            GT_U32    routerManagementCntrsSetConfig2[4]/*set number*/;
            GT_U32    routerManagementIncomingPktCntr;
            GT_U32    routerManagementOutgoingPktCntr;

        }routerManagementCntrsSets;

        struct /*routerGlobalCtrl*/{

            GT_U32    routerGlobalCtrl0;
            GT_U32    routerMTUConfigReg[4]/*MTU Index*/;
            GT_U32    routerGlobalCtrl1;
            GT_U32    routerGlobalCtrl2;
            GT_U32    routerGlobalCtrl3;
            GT_U32    genericClassificationFieldDipLookupNheFetched; /* new sip7 register */
            GT_U32    genericClassificationFieldDipLookupPrefixMatch;/* new sip7 register */
            GT_U32    genericClassificationFieldSipLookupPrefixMatch;/* new sip7 register */
            GT_U32    routerUcGlobalCfgValue[4];/* new sip7 register */
            GT_U32    routerUcGlobalCfgMask[4];/* new sip7 register */
        }routerGlobalCtrl;

        struct /*routerDropCntr*/{

            GT_U32    routerDropCntr;
            GT_U32    routerDropCntr1; /* new sip7 register */
            GT_U32    routerDropCntrConfig;

        }routerDropCntr;

        struct /*routerBridgedPktsExceptionCntr*/{

            GT_U32    routerBridgedPktExceptionsCntr;

        }routerBridgedPktsExceptionCntr;

        struct /*IPv6Scope*/{

            GT_U32    IPv6UcScopeTableReg[4]/*RegNum*/;
            GT_U32    IPv6McScopeTableReg[4]/*RegNum*/;
            GT_U32    IPv6UcScopePrefix[4];
            GT_U32    IPv6UcScopeLevel[4];

        }IPv6Scope;

        struct /*IPv6GlobalCtrl*/{

            GT_U32    IPv6CtrlReg0;
            GT_U32    IPv6CtrlReg1;

        }IPv6GlobalCtrl;

        struct /*IPv4GlobalCtrl*/{

            GT_U32    IPv4CtrlReg0;
            GT_U32    IPv4CtrlReg1;

        }IPv4GlobalCtrl;

        struct /*internalRouter*/{

            GT_U32    routerInterruptCause;
            GT_U32    routerInterruptMask;

        }internalRouter;

        struct /*FCoEGlobalCtrl*/{

            GT_U32    routerFCoEGlobalConfig;

        }FCoEGlobalCtrl;

        struct /*ECMPRoutingConfig*/{

            GT_U32    ECMPConfig;
            GT_U32    ECMPSeed;

        }ECMPRoutingConfig;

        struct /*CPUCodes*/{

            GT_U32    CPUCodes0;
            GT_U32    CPUCodes1;
            GT_U32    CPUCodes2;
            GT_U32    CPUCodes3;
            GT_U32    CPUCodes4;
            GT_U32    CPUCodes5;
            GT_U32    CPUCodes6;
            GT_U32    CPUCodes7;
            GT_U32    CPUCodes8;
            GT_U32    CPUCodes9;
            GT_U32    CPUCodes10;
            GT_U32    CPUCodes11;

        }CPUCodes;

        struct /*FdbIpLookup*/{
            GT_U32    FdbIpv4RouteLookupMask;
            GT_U32    FdbIpv6RouteLookupMask[4];
        }FdbIpLookup;

        struct /*FdbRouteLookup*/{
            GT_U32    FdbHostLookup;
        }FdbRouteLookup;

        GT_U32  routerFifoThreshold1Reg;/*Router FIFOs threshold 1*/

    }IPvX;

    struct /*PLR*/{

        GT_U32    policerInitialDP[64]/*QoSProfile*/;/* not used in sip6.0*/
/*treated bits 0-3,5-18,21-22, 24-28,31*/        GT_U32    policerCtrl0;
/*treated bits 5,7-9*/        GT_U32    policerCtrl1;
        GT_U32    globalBillingCntrIndexingModeConfig0;
        GT_U32    policerScanCtrl;
        GT_U32    policerPortMeteringReg[16]/*register*/;/* not used in sip6.0*/
        GT_U32    policerPerPortRemarkTableID[32]/*Port*/;
        GT_U32    policerOAM;
        GT_U32    policerMRU;
        GT_U32    IPFIXSecondsMSbTimerStampUpload;
        GT_U32    policerCountingBaseAddr;
        GT_U32    policerMeteringBaseAddr;
        GT_U32    policerIPFIXPktCountWAThresholdLSB;
        GT_U32    policerIPFIXPktCountWAThresholdMSB;
        GT_U32    policerIPFIXDroppedPktCountWAThreshold;
        GT_U32    policerIPFIXByteCountWAThresholdMSB;
        GT_U32    policerIPFIXByteCountWAThresholdLSB;
        GT_U32    policerInterruptMask;
        GT_U32    policerInterruptCause;
        GT_U32    policerTableAccessData[9]/*Reg*/;
        GT_U32    policerError;
        GT_U32    policerErrorCntr;
        GT_U32    meteringAndCountingRAMSize0;
        GT_U32    meteringAndCountingRAMSize1;
        GT_U32    meteringAndCountingRAMSize2;
        GT_U32    policerTableAccessCtrl;
        GT_U32    policerMeteringRefreshScanRateLimit;
        GT_U32    policerMeteringRefreshScanAddrRange0;
        GT_U32    policerMeteringRefreshScanAddrRange1;
        GT_U32    IPFIXSecondsLSbTimerStampUpload;
        GT_U32    policerShadow[2]/*RegNum*/;
        GT_U32    ipfixSampleEntriesLog_n[4/*n*/];
        GT_U32    IPFIXNanoTimerStampUpload;
        GT_U32    IPFIXCtrl;
/*treated bits 0-1,5-7*/        GT_U32    hierarchicalPolicerCtrl;
        GT_U32    globalBillingCntrIndexingModeConfig1;
       GT_U32    portAndPktTypeTranslationTable[512 ]/*port<perPort>*/;/* not used in sip6.0*/
        GT_U32      policerBadAddressLatch;
        GT_U32      policerManagementCntrsTbl;  /* PLR Management Counters */
        GT_U32      policerCountingTbl;         /* PLR Counting Table */
        GT_U32      policerTimerTbl;            /* PLR Timer Table */
        GT_U32      ipfixWrapAroundAlertTbl;    /* PLR IPFIX wrap around alert */
        GT_U32      ipfixAgingAlertTbl;         /* PLR IPFIX aging alert */
        GT_U32      meteringAddressingModeCfg0;     /* Metering Addressing Mode Configuration 0 */
        GT_U32      meteringAddressingModeCfg1;     /* Metering Addressing Mode Configuration 1 */
        GT_U32      mef10_3_bucket0MaxRateArr[128]; /* MEF 10.3 Bucket0 Max Rate */
        GT_U32      mef10_3_bucket1MaxRateArr[128]; /* MEF 10.3 Bucket1 Max Rate */
        GT_U32      qosProfile2PriorityMapArr[128]; /* Qos Profile to Priority Map */ /* not used in sip6.0*/

        GT_U32      debug_LFSRSeed[2]; /* debug register */
        GT_U32      debug_PrngControl1;/* debug register */
        GT_U32      debug_PrngControl2;/* debug register */

        struct {
            /* new in sip 5.20*/
            GT_U32    countingConfigTableThreshold[7]/*thresh_num*/;
            GT_U32    countingConfigTableSegment[8]/*table_segment*/;

            GT_U32    statisticalCountingConfig0;
            GT_U32    statisticalCountingConfig1;
            GT_U32    meteringAnalyzerIndexes;

            /* new in sip7 */
            GT_U32 globalBillingCntrIndexingModeConfig2;
            GT_U32 policerGlobalConfigurations;
            GT_U32 timeRate_n[6/*n*/];
        }globalConfig;

        /* SIP 6_10 */
        GT_U32      IPFIXFirstNPacketsConfig0;
        GT_U32      IPFIXFirstNPacketsConfig1; /* PHA metadata counter mask */
    }PLR[3];

    struct /*CNC*/
    {
        struct /*perBlockRegs*/
        {
            struct /*wraparound*/
            {
                GT_U32    CNCBlockWraparoundStatusReg[16][4]/*Block*//*Entry*/;
            }wraparound;

            struct /*rangeCount*/
            {
                GT_U32    CNCBlockRangeCountEnable[2][16][3]/*group of ranges*//*block*//*client:sip6.30*/;
            }rangeCount;

            struct /*clientEnable*/
            {
                GT_U32    CNCBlockConfigReg0[16][3]/*Block*//*client:sip6.30*/;
            }clientEnable;

            struct /*entryMode*/
            {
                GT_U32    CNCBlocksCounterEntryModeRegister[2]/*Block/8*/;
            }entryMode;

            struct /*memory*/
            {
                GT_U32    baseAddress[16]/*Block memory base address*/;
            }memory;
        }perBlockRegs;

        struct /*globalRegs*/
        {
            GT_U32    CNCGlobalConfigReg;
            GT_U32    CNCFastDumpTriggerReg;
            GT_U32    CNCClearByReadValueRegWord0;
            GT_U32    CNCClearByReadValueRegWord1;
            GT_U32    CNCInterruptSummaryCauseReg;
            GT_U32    CNCInterruptSummaryMaskReg;
            GT_U32    wraparoundFunctionalInterruptCauseReg;
            GT_U32    wraparoundFunctionalInterruptMaskReg;
            GT_U32    rateLimitFunctionalInterruptCauseReg;
            GT_U32    rateLimitFunctionalInterruptMaskReg;
            GT_U32    miscFunctionalInterruptCauseReg;
            GT_U32    miscFunctionalInterruptMaskReg;
        }globalRegs;

    }CNC[MAX_CNC_BLOCKS_CNC]/*CNC number*/;

    struct /*TXQ*/
    {
        struct
        {
            GT_U32 pdq2sdq_Map[108];/*in Falcon Port_Queue_Offset*/
            GT_U32 Credit_Value;
            GT_U32 PSI_Interrupt_Cause;
            GT_U32 PSI_Interrupt_Mask;
            GT_U32 PSI_Last_Address_Violation;
            GT_U32 PSI_Metal_Fix_Register;
            GT_U32 QmrSchPLenBursts;
            GT_U32 QmrSchBurstsSent;
            GT_U32 MSG_FIFO_Max_Peak;
            GT_U32 MSG_FIFO_Min_Peak;
            GT_U32 MSG_FIFO_Fill_Level;
            GT_U32 Queue_Status_Read_Request;
            GT_U32 Queue_Status_Read_Reply;
            GT_U32 Debug_CFG_Register;
            /*hawk*/
            GT_U32 SDQ_Map_Error_Capture;
            GT_U32 PDQ_Map_Error_Capture;
            /*phoenix*/
            GT_U32 SNK_Access_Error;
            GT_U32 Psi_Idle;
            /*AAS*/
            GT_U32 bp_pdq_sdq;
            GT_U32 debug_invq2q_read;
        }
        psi_regs;

         struct
         {
            GT_U32 pdx_metal_fix;
            GT_U32 DB_Read_Control;
            GT_U32 QCX_Read_qu_db;
            GT_U32 QCX_Read_Port_DB;
            GT_U32 QCX_Read_Reorder_Fifos;
            GT_U32 pdx2pdx_interface_mapping;
            GT_U32 Interrupt_Cause;
            GT_U32 Interrupt_Mask;
            GT_U32 Address_Violation;
            GT_U32 micro_Memory_Traffic_Counter;
            GT_U32 max_bank_fill_levels;
            GT_U32 drop_Counter;
            GT_U32 Drop_Counter_Mask;
            GT_U32 MIN_Global_Burst_Fifo_Available_Entries;

            /*hawk*/
            GT_U32 Preemption_Enable[32];
            GT_U32 PDX_Functional_Interrupt_Cause;
            GT_U32 PDX_Functional_Interrupt_Mask;
            GT_U32 PDX_Interrupt_Summary_Cause;
            GT_U32 PDX_Interrupt_Summary_Mask;
            GT_U32 PDX_Debug_Interrupt_Cause;
            GT_U32 PDX_Debug_Interrupt_Mask;
            GT_U32 pfcc_burst_fifo_thr[4];
            GT_U32 global_burst_fifo_thr[4];
            GT_U32 global_burst_fifo_available_entries[4];
            GT_U32 QCX_Wrong_Qgrpmap_Addr;
            GT_U32 QCX_Wrong_PDS_Target;
            GT_U32 Drop_Counter[4];
            GT_U32 PDS_Drop_Counter[5];

            /*aas*/
            GT_U32 CP_RXs_outstanding;
            GT_U32 QCX_Wrong_Port_Target;
            GT_U32 Micro_memory_bank_fill_level[8];
            GT_U32 QCX_Wrong_Trg_Port_Map_Addr[5];
            GT_U32 DX_Wrong_PDS_Target[10];
            GT_U32 DX_Wrong_PORT_Target[5];
            GT_U32 rx_port;
            GT_U32 rx_queue;
            GT_U32 epb_fifo_thr;
            GT_U32 Telemetry_Decay_Counter_Threshold;
            GT_U32 Renaming_Enable;
            GT_U32 CP_Renaming_Disable_Error_Queue[2];
            GT_U32 dp_core_mapping_register_pds_id;
            GT_U32 dp_core_mapping_register_pdx_id;
            GT_U32 pds_msg_mapping_register_pds_id;
            GT_U32 pds_msg_mapping_register_pdx_id;
            GT_U32 qcx_dp_core_to_db_map;
         }
        pdx;/*pdx_itx*/

    struct
    {
        GT_U32 global_pfcc_cfg;
        GT_U32 pool_CFG;
        GT_U32 PFCC_metal_fix;
        GT_U32 TC_to_pool_CFG;
        GT_U32 PFCC_Interrupt_Cause;
        GT_U32 PFCC_Interrupt_Mask;
        GT_U32 Address_Violation;
        GT_U32 Global_status_counters[8];
        GT_U32 Pool_counter[4];
        GT_U32 PFCC_Cycle_Counter;
        GT_U32 Aggregation_Request_Gen_SM_State;
        GT_U32 Source_Port_Requested_For_Read;
        GT_U32 Source_Port_Counter_value[8];
        GT_U32 Source_Port_Counter;
        GT_U32 Global_HR_status_counters[8];
        GT_U32 Pool_HR_Counter_Max_Peak[4];
        GT_U32 Global_TC_HR_Counter_Max_Peak[8];
        GT_U32 pb_occupancy_status;
        GT_U32 Pool_HR_counter_status[4];
        /*ironman*/
        GT_U32 Total_virtual_buffer_limit_CFG;
        GT_U32 Total_Virtual_Buffer_Counter;
        GT_U32 Total_Virtual_Buffer_Counter_Max;
    }
    pfcc;

    struct
    {
        GT_U32 pdx_pac_metal_fix;
        GT_U32 port_desc_counter[36];
        GT_U32 Interrupt_Cause;
        GT_U32 Interrupt_Mask;
        GT_U32 Address_Violation;
        GT_U32 pizza_arbiter_configuration_register_pds[4][33];
        GT_U32 pizza_Arbiter_Control_Register_pds[4];
    }
    pdx_pac[2];


      struct
      {
        GT_U32 Max_PDS_size_limit_for_pdx;
        GT_U32 Queue_PDS_Profile[CPSS_DXCH_SIP_6_MAX_DP_Q_NUM_DEV_LESS_MAC];
        GT_U32 PDS_Metal_Fix;
        GT_U32 Global_PDS_CFG;
        GT_U32 FIFOs_Limits;
        GT_U32 FIFOs_DQ_Disable;
        GT_U32 Max_PDS_size_limit_for_PB;
        GT_U32 Max_Num_Of_Long_Queues;
        GT_U32 Tail_Size_for_PB_Wr;
        GT_U32 PID_Empty_Limit_for_PDX;
        GT_U32 PID_Empty_Limit_for_PB;
        GT_U32 NEXT_Empty_Limit_for_PDX;
        GT_U32 NEXT_Empty_Limit_for_PB;
        GT_U32 PB_Full_Limit;
        GT_U32 Queue_Length_Adjust_Profile[CPSS_DXCH_SIP_6_MAX_DP_Q_NUM_DEV_LESS_MAC];
        GT_U32 PB_Read_Req_FIFO_Limit;
        GT_U32 PDS_Interrupt_Debug_Cause;
        GT_U32 PDS_Interrupt_Debug_Mask;
        GT_U32 PDS_Last_Address_Violation;
        GT_U32 PDS_Interrupt_Summary_Mask;
        GT_U32 PDS_Interrupt_Summary_Cause;
        GT_U32 PDS_Interrupt_Functional_Mask;
        GT_U32 PDS_Interrupt_Functional_Cause;
        GT_U32 Total_PDS_Counter;
        GT_U32 PDS_Cache_Counter;
        GT_U32 Idle_Register;
        GT_U32 Queue_Fragment_On_The_Air_Counter[2080];
        GT_U32 Free_Next_FIFO_Max_Peak;
        GT_U32 Free_Next_FIFO_Min_Peak;
        GT_U32 Free_PID_FIFO_Max_Peak;
        GT_U32 Free_PID_FIFO_Min_Peak;
        GT_U32 Data_Read_Order_FIFO_Max_Peak;
        GT_U32 Head_Answer_FIFO_Max_Peak;
        GT_U32 PB_Read_Line_Buff_FIFO_Max_Peak;
        GT_U32 PB_Read_Req_FIFO_Max_Peak;
        GT_U32 PB_Write_Req_FIFO_Max_Peak;
        GT_U32 PB_Write_Reply_FIFO_Max_Peak;
        GT_U32 PB_Read_FIFO_State_Return_FIFO_Max_Peak;
        GT_U32 Tail_Ans_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_Data_FIFO_Max_Peak;
        GT_U32 PB_FIFO_State_Read_Order_FIFO_Max_Peak;
        GT_U32 Frag_On_The_Air_Cntr_Queue_Status;
        GT_U32 Datas_BMX_Addr_Out_Of_Range_Port;
        GT_U32 PB_Write_FIFO_State_Return_FIFO_Max_Peak;
        GT_U32 PB_Read_Write_Order_FIFO_Max_Peak;
        GT_U32 FIFO_State_Latency_Max_Peak;
        GT_U32 FIFO_State_Latency_Min_Peak;
        GT_U32 Long_Queue_Counter;
        GT_U32 Long_Queue_Count_Max_Peak;
        GT_U32 PDS_Cache_Count_Max_Peak;
        GT_U32 Total_PDS_Count_Max_Peak;
        GT_U32 Active_Queue_Counter;
        GT_U32 Active_Queue_Count_Max_Peak;
        GT_U32 Profile_Long_Queue_Limit[256];
        GT_U32 Profile_Head_Empty_Limit[256];
        GT_U32 Profile_Dequeue_Length_Adjust[256];
        GT_U32 Profile_Long_Queue_Enable[256];
        /*phoenix*/
        GT_U32 pds_debug_configurations;
        GT_U32 pds_enable_debug_counters;
        GT_U32 pds_debug_cycles_to_count_lsb;
        GT_U32 pds_debug_cycles_to_count_msb;
        GT_U32 pds_desc_in_count_lsb;
        GT_U32 pds_desc_in_count_type_ref;
        GT_U32 pds_desc_in_count_type_mask;
        GT_U32 pds_desc_in_count_msb;
        GT_U32 pds_desc_in_stress_count_lsb;
        GT_U32 pds_desc_in_stress_count_type_ref;
        GT_U32 pds_desc_in_stress_count_type_mask;
        GT_U32 pds_desc_in_stress_count_msb;
        GT_U32 pds_desc_out_count_lsb;
        GT_U32 pds_desc_out_count_type_ref;
        GT_U32 pds_desc_out_count_type_mask;
        GT_U32 pds_desc_out_count_msb;
        GT_U32 pds_sdq_select_count_lsb;
        GT_U32 pds_sdq_select_count_type_ref;
        GT_U32 pds_sdq_select_count_type_mask;
        GT_U32 pds_sdq_select_count_msb;
        GT_U32 pds_pb_write_req_count_lsb;
        GT_U32 pds_pb_write_req_count_type_ref;
        GT_U32 pds_pb_write_req_count_type_mask;
        GT_U32 pds_pb_write_req_count_msb;
        GT_U32 pds_pb_read_req_count_lsb;
        GT_U32 pds_pb_read_req_count_type_ref;
        GT_U32 pds_pb_read_req_count_type_mask;
        GT_U32 pds_pb_read_req_count_msb;

        /*ironman*/
        GT_U32 L2_CutThrough_Byte_Count_Enqueue;
        GT_U32 PTP_Size_Adjust;
        GT_U32 queue_counter_select[16];
        GT_U32 BMX_Addr_Out_Of_Range_Port;
        GT_U32 PDS_BC_LL_Counter;
        GT_U32 PDS_BC_LL_Counter_Max_Peak;

        /*aas*/
        GT_U32 message_fifo_spare_entries;
        GT_U32 PDS_ID;
        GT_U32 RX_Frag_Attribute_FIFO_Limit;
        GT_U32 RxQueue_ID;
        GT_U32 Message_FIFO_Fill_Level;
        GT_U32 Message_FIFO_Max_Peak;
        GT_U32 TxQueue_Total_Frag_Ready_Counter;
        GT_U32 RxQueue_Total_Frag_Counter;
        GT_U32 TxQueue_Total_Frag_Ready_Counter_Max_Peak;
        GT_U32 RxQueue_Total_Frag_Counter_Max_Peak;
        GT_U32 RxQueue_Tail_Counter;
        GT_U32 RxQueue_Tail_Counter_Max_Peak;
        GT_U32 Dram_Queues_Active;
        GT_U32 PB_Wr_Line_Buff_Data_Ready_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_EPB_Queue_FIFO_Max_Peak;
        GT_U32 PB_Wr_Line_Buff_Desc_Ctrl_FIFO_Max_Peak;
        GT_U32 Frag_Size_FIFO_Max_Peak;
        GT_U32 Profile_Mailbox_ID[256];
        GT_U32 profile_epb_spillover_control[256];
        GT_U32 Mailbox_Read_Pointer[32];
        GT_U32 Mailbox_Write_Pointer[32];
        GT_U32 Mailbox_Priority[32];
        GT_U32 Mailbox_Target_Mailbox[32];
        GT_U32 Mailbox_Frag_Ready_Counter[32];
        GT_U32 Mailbox_RX_Queue_Frag_Counter[32];
        GT_U32 Mailbox_Copy_Back_Requests_Counter[32];
        GT_U32 Mailbox_Copy_Back_Requests_Limit[32];
        GT_U32 Mailbox_NoDealloc[32];
        GT_U32 Mailbox_Dram_PDS_Queues[32];
        GT_U32 PB_Wr_Line_Buff_Ctrl_SOP_FIFO_Max_Peak;
        GT_U32 Frag_Head_Forward_Update_FIFO_Max_Peak;
        GT_U32 Mailbox_Frag_Size[32];
        /*aas - Version 7.6.10*/
        GT_U32 Spillover_Frag_Size_Limits;
        GT_U32 Port_BC_FIFO_Size;
        GT_U32 SDQ_Selection_Delay_Line_Size;
        GT_U32 Frag_Target_Size;
        GT_U32 Desc_In_Frag_Coefficient;
        GT_U32 PDS_Panic_Spillover;
        GT_U32 SDQ_Selection_Delay_Line_Enable;
        GT_U32 PDS_Frag_Counter;
        GT_U32 PDS_Frag_Count_Max_Peak;
        GT_U32 Profile_Dynamic_Thresholds_Enable[256];
        GT_U32 Port_Average_BC[23];
        GT_U32 Port_Head_Count[23];
        GT_U32 Port_Copy_Back_Request_Count[23];
        GT_U32 Port_PB_Available_Threshold[23];
        GT_U32 Port_PB_Size_Thresholds[23];
        GT_U32 Port_PDS_Available_Threshold[23];
        GT_U32 Port_PDS_Size_Thresholds[23];
        GT_U32 Port_Average_BC_Remainder[23];
        GT_U32 Port_Frag_Spillover_Threshold[23];
        GT_U32 Port_Head_Max_Factor[23];
         /*aas - Version 7.6.12*/
        GT_U32 TxQueue_ID;
        GT_U32 TX_Frag_Attribute_FIFO_Limit;
        GT_U32 Bypass_Enable;
        GT_U32 Bypass_FIFO_Limit;
        GT_U32 Renaming_Enable;
        GT_U32 Free_Queue_Limit;
        GT_U32 TX_Frag_Attribute_FIFO_Fill_Level;
        GT_U32 TX_Frag_Attribute_FIFO_Max_Peak;
        GT_U32 Bypass_FIFO_Fill_Level;
        GT_U32 Bypass_FIFO_Max_Peak;
        GT_U32 Free_Queue_FIFO_Max_Peak;
        GT_U32 Free_Queue_FIFO_Min_Peak;
        GT_U32 Dram_Frag_Count[2];
        GT_U32 Dram_Frag_Count_Max_Peak[2];
    }pds[CPSS_DXCH_SIP6_PDS_UNITS_PER_TILE_MAC];

        struct
        {
            GT_U32 global_config;
            GT_U32 SDQ_Metal_Fix;
            GT_U32 SDQ_Interrupt_Functional_Cause;
            GT_U32 sdq_interrupt_functional_mask;
            GT_U32 SDQ_Interrupt_Summary_Mask;
            GT_U32 SDQ_Interrupt_Summary_Cause;
            GT_U32 SDQ_Interrupt_Debug_Mask;
            GT_U32 SDQ_Interrupt_Debug_Cause;
            GT_U32 queue_config[CPSS_DXCH_SIP_7_MAX_DP_Q_NUM_DEV_LESS_MAC];
            GT_U32 Port_Range_Low[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Range_High[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Back_Pressure_Low_Threshold[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Back_Pressure_High_Threshold[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Config[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 QCN_Config;
            GT_U32 PFC_Control[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Pause_TC[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Sdq_Idle;
            GT_U32 Illegal_Bad_Address;
            GT_U32 QCN_Message_Drop_Counter;
            GT_U32 Queue_Dryer_FIFO_Max_Peak;
            GT_U32 PDQ_Message_Output_FIFO_Max_Peak;
            GT_U32 QCN_Message_Drop_Counter_Control;
            GT_U32 QCN_Message_FIFO_Max_Peak;
            GT_U32 QCN_Message_FIFO_Fill_Level;
            GT_U32 Elig_Func_Error_Capture;
            GT_U32 Select_Func_Error_Capture;
            GT_U32 Sel_List_Range_Low[2][CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Sel_List_Range_High[2][CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Sel_List_Enable[2][CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            /*phoenix*/
            GT_U32 debug_config;
            GT_U32 enable_debug_counters;
            GT_U32 debug_cycles_to_count_lsb;
            GT_U32 debug_cycles_to_count_msb;
            GT_U32 sel_if_cnt_lsb;
            GT_U32 Select_IF_Count_Type_Ref;
            GT_U32 Select_IF_Count_Type_Mask;
            GT_U32 sel_if_cnt_msb;
            GT_U32 pds2sdq_deq_if_cnt_lsb;
            GT_U32 PDS2SDQ_DEQ_IF_Count_Type_Ref;
            GT_U32 PDS2SDQ_DEQ_IF_Count_Type_Mask;
            GT_U32 pds2sdq_deq_if_cnt_msb;
            GT_U32 pdq_sdq_if_cnt_lsb;
            GT_U32 PDQ_SDQ_IF_Count_Type_Ref;
            GT_U32 PDQ_SDQ_IF_Count_Type_Mask;
            GT_U32 pdq_sdq_if_cnt_msb;
            GT_U32 pds2sdq_enq_if_cnt_lsb;
            GT_U32 PDS2SDQ_ENQ_IF_Count_Type_Ref;
            GT_U32 PDS2SDQ_ENQ_IF_Count_Type_Mask;
            GT_U32 pds2sdq_enq_if_cnt_msb;
            /*ironman*/
            GT_U32 Port_QBV_TableSet[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 QBV_Config[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Egress_TOD_Offset[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_TableSet_Cycle_Time[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_TableSet_Cycle_Start[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Last_TOD_SEC[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Config_Change_Time_TOD_LSB;
            GT_U32 Config_Change_Time_TOD_MSB_Low;
            GT_U32 Config_Change_Time_TOD_MSB_High;
            GT_U32 Config_Change_Time_Ext;
            GT_U32 Config_Change_Time_Req;
            GT_U32 Config_Change_Time_Cycle_Time;
            GT_U32 Config_Change_Time_Egress_TOD_Offset;
            GT_U32 Debug_Cycle_Time_Sync_Port[2];
            GT_U32 Debug_Cycle_Time_Sync_offset[2];
            GT_U32 Debug_Cycle_Time_Sync_Duration[2];
            GT_U32 Port_Length_Awr_Attr[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Max_Alwd_BC_Offset[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 Port_Remain_Per_NS_Resolution[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
            GT_U32 TOD_Nanosec;
            GT_U32 TOD_Sec;
            GT_U32 TOD_MSB_Low;
            GT_U32 TOD_MSB_High;
            GT_U32 Select_IF_Count_Type_Ref1;
            GT_U32 Select_IF_Count_Type_Mask1;
            GT_U32 Select_IF_Count_Type_Ref0;
            GT_U32 Select_IF_Count_Type_Mask0;
            /*AAS*/
            GT_U32 Profile_Queue_TC[256];
            GT_U32 Profile_Q_Semi_Elig_Threshold[256];
            GT_U32 Profile_QCN_Div_Factor[256];
            GT_U32 Profile_Queue_Prio[256];
            GT_U32 profile_queue_gate[256];
            GT_U32 Profile_Queue_AVB_en[256];
            GT_U32 Profile_Q_Aging_Credit_Threshold[256];
            GT_U32 Profile_Q_High_Credit_Threshold[256];
            GT_U32 Profile_Q_Low_Credit_Threshold[256];
            GT_U32 Profile_Q_Neg_Credit_Threshold[256];
        }
        sdq[CPSS_DXCH_SIP6_SDQ_UNITS_PER_TILE_MAC];

        struct
        {       GT_U32 uburst_map[CPSS_DXCH_SIP_7_MAX_DP_Q_NUM_DEV_LESS_MAC];
                GT_U32 uburst_profile_threshold_0[CPSS_DXCH_MAX_UBURST_PROFILE_NUM_DEV_LESS_MAC];
                GT_U32 uburst_profile_threshold_1[CPSS_DXCH_MAX_UBURST_PROFILE_NUM_DEV_LESS_MAC];
                GT_U32 port_tc_pfc_threshold[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC*8];
                GT_U32 port_pfc_threshold[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
                GT_U32 global_tc_pfc_threshold[8];
                GT_U32 global_pb_limit;
                GT_U32 Port_PFC_Hysteresis_Conf[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
                GT_U32 TC_PFC_Hysteresis_Conf[8];
                GT_U32 QFC_Interrupt_functional_Cause;
                GT_U32 QFC_Interrupt_functional_Mask;
                GT_U32 Address_Violation;
                GT_U32 qfc_interrupt_debug_cause;
                GT_U32 QFC_Interrupt_Debug_Mask;
                GT_U32 QFC_Interrupt_Summary_Cause;
                GT_U32 QFC_Interrupt_Summary_Mask;
                GT_U32 local_port_source_port_config[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
                GT_U32 qfc_metal_fix;
                GT_U32 Global_PFC_conf;
                GT_U32 Port_TC_Available_buffers_Conf[8];
                GT_U32 Global_TC_Available_buffers_Conf[8];
                GT_U32 Global_counters[8];
                GT_U32 PFC_messages_Drop_counter;
                GT_U32 global_PFC_status;
                GT_U32 Uburst_Event_FIFO_Max_Peak;
                GT_U32 Uburst_Event_FIFO_Min_Peak;
                GT_U32 PFC_Messages_counter;
                GT_U32 Xoff_Size_Indirect_Read_Access;
                GT_U32 QCN2PFC_Counter;
                GT_U32 QCN2PFC_Counter_Control;
                GT_U32 HR_Counters_Indirect_Read_Access;
                GT_U32 Port_TC_HR_Counter_Min_Max_Conf;
                GT_U32 HR_Counter_Max_Peak;
                GT_U32 HR_Counter_Min_Peak;
                GT_U32 Xoff_Size_Indirect_Read_Access_Data;
                GT_U32 Global_HR_counters[8];
                GT_U32 port_tc_pfc_guaranteed_threshold[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC*8];
                GT_U32 port_pfc_guaranteed_threshold[23];
                GT_U32 global_tc_pfc_guaranteed_threshold[8];
                GT_U32 pb_available_buffers_conf;
                GT_U32 HR_Counters_Indirect_Read_Access_Data;

                /*Added manually*/
                GT_U32 Uburst_Event_FIFO;
                GT_U32 Counter_Table_0_Indirect_Read_Address;/*Queue Length Counters, address 0-399 (number of target queue)*/
                GT_U32 Counter_Table_0_Indirect_Read_Data;/*Queue Length Counters data*/
                GT_U32 Counter_Table_1_Indirect_Read_Address;/*Port Length Counters, address 0-8 (number of target port)*/
                GT_U32 Counter_Table_1_Indirect_Read_Data;/*Port Length Counters data*/
                GT_U32 Counter_Table_2_Indirect_Read_Address;/*PFC Counters, address 0-2063 (number of {port,tc} counter, IE port 1 TC 3 would be 8*1+3=11)*/
                GT_U32 Counter_Table_2_Indirect_Read_Data;/*PFC Counters data*/
                GT_U32 Group_Ports_PFC_Status[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];/*The status of the source port PFC ON/OFF vector*/
                GT_U32 Group_Port_TC_PFC_Status[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];/*The Status of the port.tc PFC ON/OFF vector*/
                GT_U32 Port_HR_Counters_Threshold;
                GT_U32 HR_Crossed_Threshold_Reg_Cause[PRV_QFC_MAX_GROUP_NUM_CNS];
                GT_U32 HR_Crossed_Threshold_Reg_Mask[PRV_QFC_MAX_GROUP_NUM_CNS];
                /*AAS*/
                GT_U32 port_rate_config[23];
                GT_U32 period_timer_config[4];
                GT_U32 Rate_average_profile[CPSS_DXCH_MAX_UBURST_PROFILE_NUM_DEV_LESS_MAC];
            }
    qfc[CPSS_DXCH_SIP6_QFC_UNITS_PER_TILE_MAC];


        struct /*dq*/
        {
#define MAX_DQ_NUM_PORTS_CNS        SIP_5_20_DQ_NUM_PORTS_CNS
#define SIP_5_20_DQ_NUM_PORTS_CNS   96/* 96 ports per DQ (BC3 with 6 DQs) */
#define SIP_5_DQ_NUM_PORTS_CNS      72/* 72 ports per DQ (BC2,bobk with single DQ) */
            struct /*statisticalAndCPUTrigEgrMirrToAnalyzerPort*/
            {
                                            /* 512/8 = 64 (the BC3 hold 72) */
                GT_U32    egrAnalyzerEnableIndex[PORTS_NUM_CNS/8]/*physical Port/8 -> 8 ports per register */;
                GT_U32    STCStatisticalTxSniffConfig;
                GT_U32    egrCTMTrigger;
                GT_U32    CTMAnalyzerIndex;
                GT_U32    STCAnalyzerIndex;

            }statisticalAndCPUTrigEgrMirrToAnalyzerPort;

            struct /*shaper*/
            {

                GT_U32    portTokenBucketMode[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                GT_U32    portDequeueEnable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                GT_U32    tokenBucketUpdateRate;
                GT_U32    tokenBucketBaseLine;
                GT_U32    CPUTokenBucketMTU;
                GT_U32    portsTokenBucketMTU;

            }shaper;

            struct /*scheduler*/
            {

                struct /*schedulerConfig*/
                {

                    GT_U32    conditionalMaskForPort[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    schedulerConfig;
                    GT_U32    pktLengthForTailDropDequeue;
                    GT_U32    highSpeedPorts[2];

                }schedulerConfig;

                struct /*priorityArbiterWeights*/
                {

                    GT_U32    profileSDWRRGroup[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg0[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg1[16]/*Profile*/;
                    GT_U32    profileSDWRREnable[16]/*Profile*/;
                    GT_U32    portSchedulerProfile[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;

                }priorityArbiterWeights;

                struct /*portShaper*/
                {

                    GT_U32    portRequestMaskSelector[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    portRequestMask;
                    GT_U32    fastPortShaperThreshold;

                }portShaper;

                struct /*portArbiterConfig*/
                {
                    /* BC2 / BobK --- 85 registers, */
                    /* Aldrin     --- 96 registres  */
                    GT_U32    portsArbiterPortWorkConservingEnable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-port*/;
                    GT_U32    portsArbiterMap[96]/*Slice_Group*/;
                    GT_U32    portsArbiterConfig;
                    GT_U32    portsArbiterStatus;

                }portArbiterConfig;

            }scheduler;

            struct /*global*/
            {

                struct /*memoryParityError*/
                {

                    GT_U32    tokenBucketPriorityParityErrorCntr;
                    GT_U32    parityErrorBadAddr;

                }memoryParityError;

                struct /*interrupt*/
                {

                    GT_U32    flushDoneInterruptCause[4]/*pipe*/;
                    GT_U32    flushDoneInterruptMask[4]/*pipe*/;
                    GT_U32    egrSTCInterruptCause[4]/*pipe*/;
                    GT_U32    egrSTCInterruptMask[4]/*pipe*/;
                    GT_U32    txQDQInterruptSummaryCause;
                    GT_U32    txQDQInterruptSummaryMask;
                    GT_U32    txQDQMemoryErrorInterruptCause;
                    GT_U32    txQDQMemoryErrorInterruptMask;
                    GT_U32    txQDQGeneralInterruptCause;
                    GT_U32    txQDQGeneralInterruptMask;

                }interrupt;

                struct /*globalDQConfig*/
                {

                    GT_U32    portToDMAMapTable[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    profileByteCountModification[16]/*Profile*/;
                    GT_U32    portBCConstantValue[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;
                    GT_U32    globalDequeueConfig;
                    GT_U32    BCForCutThrough;
                    GT_U32    creditCountersReset[3*6];

                }globalDQConfig;

                struct /*flushTrig*/
                {

                    GT_U32    portTxQFlushTrigger[MAX_DQ_NUM_PORTS_CNS*6]/*DQ-Port*/;

                }flushTrig;

                struct /*ECCCntrs*/
                {

                    GT_U32    DQIncECCErrorCntr;
                    GT_U32    DQClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/
                {

                    GT_U32    egrMirroredDroppedCntr;
                    GT_U32    STCDroppedCntr;

                }dropCntrs;

                struct /*Credit_Counters*/
                {
                    GT_U32    txdmaPortCreditCounter[PRV_CPSS_MAX_DMA_NUM_CNS];
                }creditCounters;

            }global;

            struct /*flowCtrlResponse*/
            {

                GT_U32    schedulerProfilePFCTimerToPriorityQueueMapEnable[16]/*profile*/;
                GT_U32    schedulerProfilePriorityQueuePFCFactor[16][8]/*Profile*//*TC*/;
                GT_U32    schedulerProfileLLFCXOFFValue[16]/*profile*/;
                GT_U32    portToTxQPortMap[256]/*Source Port*/;
                GT_U32    PFCTimerToPriorityQueueMap[8]/*TC*/;
                GT_U32    flowCtrlResponseConfig;
                GT_U32    ttiToPfcResponseTableEntry;

            }flowCtrlResponse;

        }dq[6];

        struct /*ll*/
        {

            struct /*global*/{

                struct /*portsLinkEnableStatus*/{

                    GT_U32    portsLinkEnableStatus[18];

                }portsLinkEnableStatus;

                struct /*interrupts*/{

                    GT_U32    LLInterruptCause;
                    GT_U32    LLInterruptMask;

                }interrupts;

                struct /*globalLLConfig*/{

                    GT_U32    profilePriorityQueueToPoolAssociation[16]/*profile*/;
                    GT_U32    portProfile[72]/*Port*/;
                    GT_U32    highSpeedPort[8]/*port*/;

                }globalLLConfig;

                struct /*debugBus*/{

                    GT_U32    llMetalFix;

                }debugBus;

            }global;

        }ll;

        struct /*pfc*/
        {

            struct /*PFCInterrupts*/{

                GT_U32    PFCPortGroupGlobalOverUnderflowInterruptCause;
                GT_U32    PFCPortGroupGlobalOverUnderflowInterruptMask;
                GT_U32    PFCInterruptSummaryCause;
                GT_U32    PFCInterruptSummaryMask;
                GT_U32    PFCParityInterruptCause;
                GT_U32    PFCCntrsOverUnderFlowInterruptMask;
                GT_U32    PFCParityInterruptMask;
                GT_U32    PFCFIFOsOverrunsInterruptCause;
                GT_U32    PFCFIFOsOverrunsInterruptMask;
                GT_U32    PFCMsgsSetsOverrunsInterruptCause;
                GT_U32    PFCMsgsSetsOverrunsInterruptMask;
                GT_U32    PFCCntrsOverUnderFlowInterruptCause;

            }PFCInterrupts;

            GT_U32    LLFCTargetPortRemap[144]/*Port div 4*/;
            GT_U32    portFCMode[16]/*Port div 32*/;
            GT_U32    PFCSourcePortToPFCIndexMap[512]/*Global Port*/;
            GT_U32    PFCPortProfile[64]/*Port div 8*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[1]/*port group*/;
            GT_U32    PFCPortGroupEqFIFOOverrunCntr[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsStatus[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsParityErrorsCntr[1]/*port group*/;
            GT_U32    PFCGroupOfPortsTCXonThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCXoffThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCDropThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCCntr[1][8]/*port group*//*tc*/;
            GT_U32    PFCTriggerGlobalConfig;
         /* GT_U32    FCModeProfileTCXOffThresholds[64] - new registers. CPSS use CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E with the same format and number of entries */
         /* GT_U32    FCModeProfileTCXOnThresholds[64] - new registers. CPSS use CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E with the same format and number of entries */
            struct /*DBA - dynamic buffer allocation*/
            {
                GT_U32 PFCProfileTCAlphaThresholds[8][8]/*profile*//*TC*/;
                GT_U32 PFCAvailableBuffers;
                GT_U32 PFCDynamicBufferAllocationDisable;
            }dba;
        }pfc;

        struct /*qcn*/
        {

            GT_U32    CCFCSpeedIndex[72]/*Reg offset*/;
            GT_U32    CNSampleTbl[8]/*qFb 3 msb*/;
            GT_U32    profilePriorityQueueCNThreshold[16][8]/*profile*//*priority*/;
            GT_U32    feedbackMIN;
            GT_U32    feedbackMAX;
            GT_U32    CNGlobalConfig;
            GT_U32    enFCTriggerByCNFrameOnPort[16]/*Port div 32*/;
            GT_U32    enCNFrameTxOnPort[16]/*Port div 32*/;
            GT_U32    feedbackCalcConfigs;
            GT_U32    txqECCConf;
            GT_U32    descECCSingleErrorCntr;
            GT_U32    descECCDoubleErrorCntr;
            GT_U32    CNTargetAssignment;
            GT_U32    CNDropCntr;
            GT_U32    QCNInterruptCause;
            GT_U32    QCNInterruptMask;
            GT_U32    CNDescAttributes;
            GT_U32    CNBufferFIFOParityErrorsCntr;
            GT_U32    CNBufferFIFOOverrunsCntr;
            GT_U32    ECCStatus;
            GT_U32    cnAvailableBuffers;

        }qcn;

        struct /*queue*/
        {
            struct /*tailDrop*/{

                struct /*tailDropLimits*/{

                    GT_U32    poolBufferLimits[8]/*Priority*/;
                    GT_U32    poolDescLimits[8]/*Priority*/;
                    GT_U32    profilePortDescLimits[16]/*profile*/;
                    GT_U32    profilePortBufferLimits[16]/*profile*/;
                    GT_U32    globalDescsLimit;
                    GT_U32    globalBufferLimit;

                }tailDropLimits;

                struct /*tailDropCntrs*/{

                    GT_U32    portBuffersCntr[576]/*Port*/; /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portDescCntr[576]/*Port*/;    /* for sip5.20 and above - 576, else 72 */
                    GT_U32    priorityDescCntr[8]/*TC*/;
                    GT_U32    priorityBuffersCntr[8]/*TC*/;
                    GT_U32    totalDescCntr;
                    GT_U32    totalBuffersCntr;

                }tailDropCntrs;

                struct /*tailDropConfig*/{

                    GT_U32    portTailDropCNProfile[576]/*Port*/;   /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portEnqueueEnable[576]/*Port*/;       /* for sip5.20 and above - 576, else 72 */
                    GT_U32    profileTailDropConfig[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*Profile*/;
                    GT_U32    profilePriorityQueueToPoolAssociation[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*profile*/;
                    GT_U32    profilePriorityQueueEnablePoolUsage[8]/*Priority*/;
                    GT_U32    profilePoolWRTDEn[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*profile*/;
                    GT_U32    profileQueueWRTDEn[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*profile*/;
                    GT_U32    profilePortWRTDEn[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*profile*/;
                    GT_U32    profileMaxQueueWRTDEn[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*profile*/;
                    GT_U32    profileECNMarkEn[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*Profile*/;
                    GT_U32    profileCNMTriggeringEnable[PRV_CPSS_DXCH_SIP_5_PORT_TX_DROP_PROFILE_MAX_NUM_CNS]/*Profile*/;
                    GT_U32    globalTailDropConfig;
                    GT_U32    pktLengthForTailDropEnqueue;
                    GT_U32    maxQueueWRTDMasks;
                    GT_U32    portWRTDMasks;
                    GT_U32    queueWRTDMasks;
                    GT_U32    poolWRTDMasks;
                    GT_U32    dynamicAvailableBuffers;

                }tailDropConfig;

                struct /*resourceHistogram*/{

                    struct /*resourceHistogramLimits*/{

                        GT_U32    resourceHistogramLimitReg1;
                        GT_U32    resourceHistogramLimitReg2;

                    }resourceHistogramLimits;

                    struct /*resourceHistogramCntrs*/{

                        GT_U32    resourceHistogramCntr[4]/*counter*/;

                    }resourceHistogramCntrs;

                }resourceHistogram;

                struct /*mcFilterLimits*/{

                    GT_U32    mirroredPktsToAnalyzerPortDescsLimit;
                    GT_U32    ingrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    egrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    mcDescsLimit;
                    GT_U32    mcBuffersLimit;

                }mcFilterLimits;

                struct /*muliticastFilterCntrs*/{

                    GT_U32    snifferDescsCntr;
                    GT_U32    ingrSnifferBuffersCntr;
                    GT_U32    egrSnifferBuffersCntr;
                    GT_U32    mcDescsCntr;
                    GT_U32    mcBuffersCntr;

                }muliticastFilterCntrs;

                struct /*FCUAndQueueStatisticsLimits*/{

                    GT_U32    XONLimit[8]/*SharedLimit*/;
                    GT_U32    XOFFLimit[8]/*SharedLimit*/;
                    GT_U32    queueXONLimitProfile[16]/*Profile*/;
                    GT_U32    queueXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    portXONLimitProfile[16]/*Profile*/;
                    GT_U32    portXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    globalXONLimit;
                    GT_U32    globalXOFFLimit;
                    GT_U32    FCUMode;

                }FCUAndQueueStatisticsLimits;

            }tailDrop;

            struct /*global*/{
                GT_U32  portBufferStat;
                GT_U32  queueBufferStat;

                struct /*interrupt*/{

                    struct /*queueStatistics*/{

                        GT_U32    lowCrossingStatusReg[18]/*Index*/;
                        GT_U32    highCrossingInterruptMaskPort[72]/*Port*/;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts61To31;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts30To0;
                        GT_U32    lowCrossingInterruptSummaryCausePorts71To62;
                        GT_U32    lowCrossingInterruptSummaryCausePorts61To31;
                        GT_U32    lowCrossingInterruptSummaryCausePorts30To0;
                        GT_U32    lowCrossingInterruptMaskPort[72]/*Port*/;
                        GT_U32    lowCrossingInterruptCausePort[72]/*Port*/;
                        GT_U32    highCrossingStatusReg[18]/*Index*/;
                        GT_U32    highCrossingInterruptSummaryMaskPorts71To62;
                        GT_U32    lowCrossingInterruptSummaryMaskPorts71To62;
                        GT_U32    highCrossingInterruptSummaryCausePorts30To0;
                        GT_U32    highCrossingInterruptSummaryMaskPorts30To0;
                        GT_U32    highCrossingInterruptSummaryCausePorts61To31;
                        GT_U32    highCrossingInterruptSummaryMaskPorts61To31;
                        GT_U32    highCrossingInterruptSummaryCausePorts71To62;
                        GT_U32    highCrossingInterruptCausePort[72]/*Port*/;

                    }queueStatistics;

                    GT_U32    transmitQueueInterruptSummaryCause;
                    GT_U32    transmitQueueInterruptSummaryMask;
                    GT_U32    generalInterrupt1Cause;
                    GT_U32    generalInterrupt1Mask;
                    GT_U32    portDescFullInterruptCause0;
                    GT_U32    portDescFullInterruptMask0;
                    GT_U32    portDescFullInterruptCause1;
                    GT_U32    portDescFullInterruptMask1;
                    GT_U32    portDescFullInterruptCause2;
                    GT_U32    portDescFullInterruptMask2;
                    GT_U32    portBufferFullInterruptCause0;
                    GT_U32    portBufferFullInterruptMask0;
                    GT_U32    portBufferFullInterruptCause1;
                    GT_U32    portBufferFullInterruptMask1;
                    GT_U32    portBufferFullInterruptCause2;
                    GT_U32    portBufferFullInterruptMask2;

                }interrupt;

                struct /*ECCCntrs*/{

                    GT_U32    TDClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/{

                    GT_U32    clearPktsDroppedCntrPipe[4]/*Pipe*/;

                }dropCntrs;

                struct /*Debug Bus */{
                    GT_U32    queueMetalFix;
                }debugBus;

            }global;

            struct /*peripheralAccess*/{

                struct /*peripheralAccessMisc*/{

                    GT_U32    portGroupDescToEQCntr[4]/*port group*/;
                    GT_U32    peripheralAccessMisc;
                    GT_U32    QCNIncArbiterCtrl;

                }peripheralAccessMisc;

                struct /*egrMIBCntrs*/{

                    GT_U32    txQMIBCntrsPortSetConfig[2]/*Set*/;
                    GT_U32    txQMIBCntrsSetConfig[2]/*Set*/;
                    GT_U32    setTailDroppedPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingUcPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingMcPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingBcPktCntr[2]/*Set*/;
                    GT_U32    setMcFIFO7_4DroppedPktsCntr[2]/*Set*/;
                    GT_U32    setMcFIFO3_0DroppedPktsCntr[2]/*Set*/;
                    GT_U32    setCtrlPktCntr[2]/*Set*/;
                    GT_U32    setBridgeEgrFilteredPktCntr[2]/*Set*/;

                }egrMIBCntrs;

                struct /*CNCModes*/{

                    GT_U32    CNCModes;

                }CNCModes;

            }peripheralAccess;

        }queue;

        struct /*bmx*/
        {

            GT_U32    almostFullDebugRegister;
            GT_U32    fillLevelDebugRegister;   /*Fill Level Debug Register*/

        }bmx;

    }TXQ;

    struct /*TMDROP*/
    {
        GT_U32    portsInterface[8]/*32 ports*/;
        GT_U32    TMDROPTMTCMap8To87[2]/*8 TM TC*/;
        GT_U32    globalConfigs;
        GT_U32    TMDROPInterruptCause;

        GT_U32    TMDROPInterruptMask;
    }TMDROP;

    struct /*TMQMAP*/
    {
        GT_U32    TC2TCMap8To16[6]/*8 TC2TC addresses*/;
        GT_U32    TMQMAPGlobalConfig;
        GT_U32    TMQMAPInterruptCause;
        GT_U32    TMQMAPInterruptMask;

    }TMQMAP;

    struct /*TM_INGR_GLUE*/{

        GT_U32    globalConfig;
        GT_U32    debug;
        GT_U32    TMIngrGlueInterruptCause;
        GT_U32    TMIngrGlueInterruptMask;

    }TM_INGR_GLUE;

    struct /*TM_EGR_GLUE*/
    {
        GT_U32    statisticalAgingTimeCntrMsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingTimeCntrLsb[100]/*statistical aging set*/;
        GT_U32    TMEgrGlueInterruptMask;
        GT_U32    TMEgrGlueInterruptCause;
        GT_U32    statisticalMinAgeTime[100]/*statistical aging set*/;
        GT_U32    statisticalMinAgeTimeShadow;
        GT_U32    statisticalMaxAgeTime[100]/*statistical aging set*/;
        GT_U32    statisticalMaxAgeTimeShadow;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingTimeCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingTimeCntrsMsbShadow;
        GT_U32    statisticalAgingTimeCntrsLsbShadow;
        GT_U32    TMEgrGlueSummaryCause;
        GT_U32    TMEgrGlueSummaryMask;
        GT_U32    statisticalAgingQueueID[100]/*statistical aging set*/;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingPktCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingPktCntr[100]/*statistical aging set*/;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingOctetCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingOctetCntrsMsbShadow;
        GT_U32    statisticalAgingOctetCntrsLsbShadow;
        GT_U32    statisticalAgingOctetCntrMsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingOctetCntrLsb[100]/*statistical aging set*/;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask0;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask1;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask2;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptMask3;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause0;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause1;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause2;
        GT_U32    statisticalAgingAgedOutPktCntrsWraparoundInterruptCause3;
        GT_U32    statisticalAgingAgedOutPktCntrsShadow;
        GT_U32    statisticalAgingAgedOutPktCntr[100]/*statistical aging set*/;
        GT_U32    queueTimerConfig;
        GT_U32    queueTimer;
        GT_U32    portInterface0;
        GT_U32    portInterface1;
        GT_U32    portInterface2;
        GT_U32    perPortAgedOutPktCntr[192]/*TmPort*/;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask0;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask1;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask2;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask3;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask4;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask5;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptMask6;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause0;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause1;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause2;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause3;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause4;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause5;
        GT_U32    perPortAgedOutPktCntrsWraparoundInterruptCause6;
        GT_U32    globalConfig;
        GT_U32    ECCStatus;
        GT_U32    dropPortAllOutgoingPkts[192]/*TmPort*/;
        GT_U32    dropAgedPktCntrConfig;
        GT_U32    dropAgedPktCntr;
        GT_U32    debug;

    }TM_EGR_GLUE;

    struct /*TM_FCU*/
    {
        GT_U32    TMEgrFlowCtrlCntrs[64]/*word*/;
        GT_U32    TMEgrFlowCtrlXOFFThresholds[64]/*word*/;
        GT_U32    TMEgrFlowCtrlXONThresholds[64]/*word*/;
        GT_U32    TMFCUGlobalConfigs;

    }TM_FCU;

    /* Added manually */
    struct /* MG */{

        struct /*XSMI*/{

            GT_U32 XSMIManagement;
            GT_U32 XSMIAddress;
            GT_U32 XSMIConfiguration;
            GT_U32 XSMIInterruptCause;
            GT_U32 XSMIInterruptMask;

        }XSMI[16];
        struct /*addressDecoding*/
        {
            GT_U32 baseAddress[6];
            GT_U32 windowSize[6];
            GT_U32 highAddressRemap[6];
            GT_U32 windowControl[6];

            GT_U32 UnitDefaultID;
        }addressDecoding;
        struct /*confiProcessor*/
        {
            GT_U32 serviceCpuGlobalConfiguration;
            GT_U32 memoryBase;
        }confiProcessor;
        struct /*tableAccessControl*/
        {
            GT_U32 mgcamControl;
            GT_U32 mgcamEngineStatus;
            GT_U32 mgcamEngineData;    /* CPSS Use MGCAM entry #0*/
            GT_U32 mgcamEngineAddress; /* CPSS Use MGCAM entry #0*/
        }tableAccessControl;
    }MG;

    struct PRV_CPSS_DXCH_MAC_PACKET_GEN_VER1_CONFIG_STC
    {
        GT_U32 macDa[3];
        GT_U32 macSa[3];
        GT_U32 etherType;
        GT_U32 vlanTag;
        GT_U32 packetLength;
        GT_U32 packetCount;
        GT_U32 ifg;
        GT_U32 macDaIncrementLimit;
        GT_U32 controlReg0;
        GT_U32 readAndWriteData;
        GT_U32 readAndWriteControl;
        GT_U32 dataPattern[32];
    }packGenConfig[74]; /* one packet generator per port for sip_5_15 (exclude sip_5_20); */
                        /* for other devices one packet generators per port group         */

    struct PRV_CPSS_DXCH_RX_DMA_GLUE_STCT
    {
        PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC pizzaArbiter;
    }RX_DMA_GLUE;

    struct PRV_CPSS_DXCH_TX_DMA_GLUE_STCT
    {
        PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC pizzaArbiter;
    }TX_DMA_GLUE;

    struct PRV_CPSS_DXCH_MPPM_STCT
    {
        PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STC pizzaArbiter;
        struct {
            GT_U32 statusFailedSyndrome;
            GT_U32 lastFailingBuffer;
            GT_U32 lastFailingSegment;
        } dataIntegrity;

    }MPPM;

    struct PRV_CPSS_DXCH_BMA_STCT
    {
        /* MCCNT Parity Error Counter1 Register */
        GT_U32  mcCntParityErrorCounter1;
    } BMA;

    struct PRV_CPSS_DXCH_EREP_STCT
    {
        struct
        {
            GT_U32 pchHeaderEnableBase; /*1024 registers*/
            GT_U32 replicationsArbiterConfigs;
            GT_U32 qcnConfigs0;
            GT_U32 qcnConfigs1;
            GT_U32 dropCodeConfigs;
            GT_U32 hbuArbiterConfigs;
            GT_U32 erepGlobalConfigs;
            GT_U32 erepGlobalConfigs2;
            GT_U32 dropCodeStatus;
            GT_U32 egressStatisticalMirroringToAnalyzerRatio[MAX_NUM_ANALYZER_PORTS];
        } configurations;
        struct
        {
            GT_U32 qcnOutgoingCounter;
            GT_U32 sniffOutgoingCounter;
            GT_U32 trapOutgoingCounter;
            GT_U32 mirrorOutgoingCounter;
            GT_U32 qcnFifoFullDroppedPacketsCounter;
            GT_U32 sniffFifoFullDroppedPacketsCounter;
            GT_U32 trapFifoFullDroppedPacketsCounter;
            GT_U32 mirrorFifoFullDroppedPacketsCounter;
            GT_U32 erepHeaderExceedCounter;
            GT_U32 incNotReadyCounter;
            GT_U32 mirrorReplicationCancelledDueIncBackpressure;
            GT_U32 sniffReplicationCancelledDueIncBackpressure;
            GT_U32 trapReplicationCancelledDueIncBackpressure;
            GT_U32 hbuPacketsOutgoingForwardCounter;
            GT_U32 egressAnalyzerCounter[MAX_NUM_ANALYZER_PORTS];
        } counters;
        struct
        {
            GT_U32 erepInterruptsCause;
            GT_U32 erepInterruptsMask;
        } interrupts;
    } EREP;

    struct PRV_CPSS_DXCH_PRE_Q_STCT
    {
        GT_U32 globalConfig;
        GT_U32 global_tail_drop_limit_high;
        GT_U32 poolTailDropLimit[SIP7_SHARED_TAIL_DROP_POOLS_NUM_CNS];
        GT_U32 mcAvailableBuffers;
        GT_U32 mcTDConfigurationLowPrio;
        GT_U32 mcTDConfigurationHighPrio;
        GT_U32 tdPoolAvailableBuffers[SIP7_SHARED_TAIL_DROP_POOLS_NUM_CNS];
        GT_U32 globalTailDropConfig;
        GT_U32 queueMaxLimitConfig;
        GT_U32 globalQcnConfig;
        GT_U32 qcnPoolAvailableBuffers[SIP7_SHARED_TAIL_DROP_POOLS_NUM_CNS];
        GT_U32 fbMin;
        GT_U32 fbMax;
        GT_U32 qcnTrigger[32];
        GT_U32 mirroringGlobalConfig;
        GT_U32 portMapping[128];
        GT_U32 queueOffsetProfileMapping[32];
        /*start sip 7 specific*/
        GT_U32 global_tail_drop_limit_low;
        GT_U32 Global_PDX_Burst_Fifo_Limit_High;
        GT_U32 Global_PDX_Burst_Fifo_Limit_Low;
        GT_U32 Pool_Tail_Drop_Limit_LSB_Mask;
        GT_U32 MC_TD_LSBits_Mask;
        /*end of sip 7 specific*/

        struct /*egrMIBCntrs*/
        {
            GT_U32    egrMIBCntrsPortSetConfig[2]/*Set*/;
            GT_U32    egrMIBCntrsSetConfig[2]/*Set*/;
            GT_U32    egrMIBTailDroppedPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingUcPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingMcPktCntr[2]/*Set*/;
            GT_U32    egrMIBOutgoingBcPktCntr[2]/*Set*/;
            GT_U32    egrMIBCtrlPktCntr[2]/*Set*/;
            /*start sip 7 specific*/
            GT_U32 Set_Outgoing_Unicast_Packet_Counter_Msb[2];
            GT_U32 Set_Outgoing_Multicast_Packet_Counter_Msb[2];
            GT_U32 Set_Outgoing_Broadcast_Packet_Counter_Msb[2];
            GT_U32 Set_Tail_Dropped_Packet_Counter_Msb[2];
            GT_U32 Set_Control_Packet_Counter_Msb[2];
            /*end of sip 7 specific*/

        } egrMIBCntrs;
        struct
        {
            GT_U32 desc_fifo_full_cnt;
            GT_U32 mirrored_pkts_cnt;
            GT_U32 query_overide_bits_31_t_0;
            GT_U32 query_fifo_max_peak;
            GT_U32 egress_packet_cnt;
            GT_U32 query_fifo_full_cnt;
            GT_U32 mark_ecn_pkts_cnt;
            GT_U32 query_overide_bits_63_t_32;
            GT_U32 qcn_pkts_cnt;
            GT_U32 drop_reason_cnt_config;
            GT_U32 drop_reason_port_configuration;
            GT_U32 query_overide_bits_111_t_96;
            GT_U32 mirrored_pkt_config;
            GT_U32 query_overide_bits_95_t_64;
            GT_U32 drop_reason_cnt;
            GT_U32 query_fifo_min_peak;
            GT_U32 ingress_packet_cnt;
            GT_U32 ingress_pkt_config;
            GT_U32 mark_ecn_pkt_config;
            GT_U32 debug_cntr_en;
            GT_U32 quires_cnt;
            GT_U32 qcn_pkt_config;
            GT_U32 query_overide_en;
        }
        preq_debug_registers;
        struct /*FRE*/
        {
            GT_U32    freSrfGlobalConfig0;
            GT_U32    freSrfGlobalConfig1;
            GT_U32    freGlobalConfig    ;
            GT_U32    LatentErrorPeriod  ;
            GT_U32    RestartPeriod      ;
            GT_U32    ArbiterPriority    ;
            GT_U32    ArbiterWeights     ;
            GT_U32    DaemonsRangeControl;
            GT_U32    ErrorDetected[STREAM_PREQ_SRF_NUM/32];/*32 registers*/
            GT_U32    TakeAny      [STREAM_PREQ_SRF_NUM/32];/*32 registers*/
        }FRE;/* sip6.30 */

    } PREQ; /* sip6 only */

    struct /*sip6_rxDMA*/{

        struct /*status*/{

            GT_U32    pktDropReport;

        }status;

        struct /*configs*/{

            struct /*rejectConfigs*/{

                GT_U32    rejectPktCommand;
                GT_U32    rejectCPUCode;

            }rejectConfigs;

            struct /*dropThresholds*/{

                GT_U32    lowPrioDropThreshold0;
                GT_U32    lowPrioDropThreshold1;
                /* used preingressPrioritizationEnable in rxDMA structure for sip5 */

            }dropThresholds;

            struct /*dataAccumulator*/{

                GT_U32    PBFillLevelThresholds;
                GT_U32    maxTailBC;
                GT_U32    sdmaPortConfig[2];        /* sip6.10 - 1 sdma, sip6.15 - 2 sdma */
                GT_U32    pchEopTimer;              /* sip6.10 */
                GT_U32    egressHeaderBackPressure; /* sip6.10 */
                GT_U32    maxRxIdsNumber;           /* sip6.10 */
                GT_U32    ct2sfHeadersStressMargin; /* sip6.10 */

            }dataAccumulator;

            struct /*cutThrough*/{

                GT_U32    CTGenericConfigs[4]/*Generic*/;
                GT_U32    channelCTConfig[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    CTGeneralConfig;
                GT_U32    CTErrorConfig;
                GT_U32    CTUPEnable;

            }cutThrough;

            struct /*channelConfig*/{

                GT_U32    channelToLocalDevSourcePort[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    PCHConfig[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    channelGeneralConfigs[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    channelCascadePort[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;
                GT_U32    preemptedConfigs[PRV_CPSS_MAX_DMA_NUM_CNS]/*Channel*/;

            }channelConfig;

            struct /*backPressureCfg*/ /*AAS*/
            {
                GT_U32 backPressFreeContextIdFifoThreshold;
                GT_U32 backPressPbGlobalFillLevelThreshold;
                GT_U32 backPressAccumHeadThreshold;
                GT_U32 backPressAccumTailThreshold;
                GT_U32 backPressIaConfigThreshold;
                GT_U32 backPressEnqueueDesriptor1Threshold;
                GT_U32 backPressEnqueueDesriptor2Threshold;
                GT_U32 backPressContextIdHeadPktCountThreshold;
                GT_U32 backPressPbManagerThreshold;
            } backPressureCfg;

        }configs;

        struct /*DFX*/{

            GT_U32    ECCErrorInjection;
            GT_U32    ECCErrorMode;

        }DFX;

        struct /*debug*/{

            /* interrupt caused for debug print only */
            GT_U32  rx_dma_interrupt0_cause;
            GT_U32  rx_dma_interrupt1_cause;

            /* dp_rx_debug_configurations */
            GT_U32  dp_rx_enable_debug_counters;
            GT_U32  dp_rx_debug_configurations;
            GT_U32  dp_rx_debug_cycles_to_count;
            GT_U32  dp_rx_debug_cycles_to_count_high;

            struct /*debug_status*/{
                GT_U32  dp_rx_general_status;
                GT_U32  dp_rx_debug_bus;
            }debug_status;

            struct /*fifos_status*/{
                GT_U32  rx_ingress_fifo_status;
                GT_U32  rx_ing_async_fifo_status;
                GT_U32  rx_egr_context_id_ctrl_fifos_status0;
                GT_U32  rx_egr_context_id_ctrl_fifos_status1;
                GT_U32  rx_egr_context_id_ctrl_fifos_status2;
                GT_U32  rx_egr_context_id_ctrl_fifos_status3;
                GT_U32  rx_egr_context_id_ctrl_fifos_status4;
                GT_U32  rx_egr_context_id_ctrl_fifos_status5;
                GT_U32  rx_egr_pb_manager_fifos_status0;
                GT_U32  rx_egr_pb_manager_fifos_status1;
                GT_U32  rx_egr_async_fifos_status0;
                GT_U32  rx_egr_async_fifos_status1;
            }fifos_status;

            struct /*ingr_stress_err_counters*/{
                GT_U32  rx_ingr_stress_cnt; /* low */
                GT_U32  rx_ingr_stress_cnt_high;
                GT_U32  rx_ingr_stress_cnt_type_ref;
                GT_U32  rx_ingr_stress_cnt_type_mask;
            }ingr_stress_err_cnt;

            struct /*rx_accum_mem_cnt*/{
                GT_U32  accum_count; /* low */
                GT_U32  accum_count_high;
                GT_U32  accum_count_type_ref;
                GT_U32  accum_count_type_mask;
                GT_U32  accum_count_max_fill_level[4];
            }rx_accum_mem_cnt;

            struct /*RX Ingress Drop Count*/{ /* not for AAS */
                GT_U32  rx_ingress_drop_count;
                GT_U32  rx_ingress_drop_count_high;
                GT_U32  rx_ingress_drop_count_type_ref;
                GT_U32  rx_ingress_drop_count_type_mask;
            }rxIngressDropCounter[5];

            struct /*RX Ingress Drop Count*/{ /* for AAS */
                GT_U32  rx_channel_ingress_drop_count;
                GT_U32  rx_channel_ingress_drop_count_high;
                GT_U32  rx_channel_ingress_drop_count_type_ref;
                GT_U32  rx_channel_ingress_drop_count_type_mask;
            }rxChannelIngressDropCounter[64/*channel*/][4/*prio*/];

            struct /*rx_to_cp_cnt*/{
                GT_U32  rx_to_cp_count;
                GT_U32  rx_to_cp_count_high;
                GT_U32  rx_to_cp_count_type_ref;
                GT_U32  rx_to_cp_count_type_mask;
            }rx_to_cp_cnt;

            struct /*cp_to_rx_cnt*/{
                GT_U32  cp_to_rx_count;
                GT_U32  cp_to_rx_count_high;
                GT_U32  cp_to_rx_count_type_ref;
                GT_U32  cp_to_rx_count_type_mask;
            }cp_to_rx_cnt[2];

            struct /*RX Egress Drop Count*/{ /* Not For AAS */
                GT_U32  rx_egress_drop_count;
                GT_U32  rx_egress_drop_count_high;
                GT_U32  rx_egress_drop_count_type_ref;
                GT_U32  rx_egress_drop_count_type_mask;
            }rxEgressDropCounter[2];

            struct /*RX Egress Drop Count*/{ /* For AAS */
                GT_U32  rx_channel_egress_drop_count;
                GT_U32  rx_channel_egress_drop_count_high;
                GT_U32  rx_channel_egress_drop_count_type_ref;
                GT_U32  rx_channel_egress_drop_count_type_mask;
            }rxChannelEgressDropCounter[64/*channel*/];

            struct /*rx_to_pb_cnt*/{
                GT_U32  rx_to_pb_count;
                GT_U32  rx_to_pb_count_high;
                GT_U32  rx_to_pb_count_type_ref;
                GT_U32  rx_to_pb_count_type_mask;
            }rx_to_pb_cnt;

            struct /*pb_to_rx_cnt*/{
                GT_U32  pb_to_rx_count;
                GT_U32  pb_to_rx_count_high;
                GT_U32  pb_to_rx_count_type_ref;
                GT_U32  pb_to_rx_count_type_mask;
                GT_U32  pb_to_rx_packet_credit_count;
            }pb_to_rx_cnt;

            struct /*rx_to_pdx_cnt*/{
                GT_U32  rx_to_pdx_count;
                GT_U32  rx_to_pdx_count_high;
                GT_U32  rx_to_pdx_count_type_ref;
                GT_U32  rx_to_pdx_count_type_mask;
                GT_U32  rx_to_pdx_count_enq_desc_fifo_max;
            }rx_to_pdx_cnt;

            struct /*pdx_to_rx_cnt*/{
                GT_U32  pdx_to_rx_count;
                GT_U32  pdx_to_rx_count_high;
                GT_U32  pdx_to_rx_count_type_ref;
                GT_U32  pdx_to_rx_count_type_mask;
            }pdx_to_rx_cnt;

        }debug;

        struct /*interrupts*/{

            GT_U32    rxDMAInterrupt0Cause;
            GT_U32    rxDMAInterrupt0Mask;
            GT_U32    rxDMAInterrupt1Cause;
            GT_U32    rxDMAInterrupt1Mask;
            GT_U32    rxDMAInterrupt2Cause;
            GT_U32    rxDMAInterrupt2Mask;
            GT_U32    rxDMAInterruptSummaryCause;
            GT_U32    rxDMAInterruptSummaryMask;

        }interrupts;
    }sip6_rxDMA[MAX_DP_CNS]/*RXDMA_UNIT*/;

    struct /*LMU*/ {
        GT_U32 channelEnable;
        GT_U32 decimatorSelect;
        GT_U32 averageCoefficient;
        GT_U32 statClear;
        GT_U32 profileStatisticsReadData[6];
        GT_U32 latencyOverThreshold_cause[17];
        GT_U32 latencyOverThreshold_mask[17];
        GT_U32 channelFifoFull_cause;
        GT_U32 channelFifoFull_mask;
        GT_U32 lmuGlobalInterrupt_cause;
        GT_U32 lmuGlobalInterrupt_mask;
        GT_U32 summary_cause;
        GT_U32 summary_mask;
        GT_U32 badAccessAddress;
    } LMU[4 * 8 * MAX_TILE_CNS]; /* MAX=(PCAs * LMUs * numOfTiles) */

    struct /*MPFS*/ {
        GT_U32 PAUControl;
        GT_U32 PAUStatus;
        GT_U32 SAUControl;
        GT_U32 LEDControl;
        GT_U32 illegalAccessAddess;
        GT_U32 MPFSClockControl;
    }MPFS[MAX_TILE_CNS * FALCON_RAVENS_PER_TILE * MPF_PER_RAVEN * MPFS_PER_MPF];

    struct /*MPFS - CPU port*/ {
        GT_U32 PAUControl;
        GT_U32 PAUStatus;
        GT_U32 SAUControl;
        GT_U32 LEDControl;
        GT_U32 illegalAccessAddess;
        GT_U32 MPFSClockControl;
    }MPFS_CPU[MAX_TILE_CNS * FALCON_RAVENS_PER_TILE];

    struct /* SIP6 LED */
    {
        GT_U32 LEDControl;
        GT_U32 blinkGlobalControl;
        GT_U32 classGlobalControl;
        GT_U32 classesAndGroupConfig;
        GT_U32 classForcedData[6];
        GT_U32 portIndicationSelect[32];
        GT_U32 portTypeConfig[2];
        GT_U32 portIndicationPolarity[8];
        GT_U32 classRawData[6];
        GT_U32 LEDInterruptCause;
        GT_U32 LEDInterruptMask;
        GT_U32 LEDIllegalAddress;
        GT_U32 LEDChainStreamControl0;
        GT_U32 LEDChainStreamControl1;
        GT_U32 LEDChainClockControl;
        GT_U32 blinkDivision0;
        GT_U32 blinkDivision1;
        GT_U32 pulseStretchDivision;
        GT_U32 parallelLedControl;
        GT_U32 parallelLedBitControl[8];
        GT_U32 ledControl2;
        GT_U32 reorderConfig[64];
    }sip6_LED[SIP_6_MAX_LED_UNIT_NUM_CNC];

    struct /* Packet Buffer DIP7 only (SIP6)*/
    {
        struct /* PB_CENTER */
        {
            GT_U32 npmRandomizerEnable;
            GT_U32 tileId;
            GT_U32 tileMode;
            GT_U32 mswCollapceCfg;
            GT_U32 pbOutOfReset;
            GT_U32 mswDebug0; /* for SIP6_10 - MSW Dequeue Disable 0 */
            GT_U32 mswDebug1; /* for SIP6_10 - MSW Dequeue Disable 0 */
            struct /*interrupts*/
            {
                GT_U32 pbCenterCause;
                GT_U32 pbCenterMask;
                GT_U32 npmSumCause;
                GT_U32 npmSumMask;
                GT_U32 gpcSumCause;
                GT_U32 gpcSumMask;
                GT_U32 smbSumCause;
                GT_U32 smbSumMask;
                GT_U32 wrArbiterSumCause;
                GT_U32 wrArbiterSumMask;
                GT_U32 pbSumCause;
                GT_U32 pbSumMask;
            }interrupts;
            struct /*debugRing*/
            {
                GT_U32 drSelect;
                GT_U32 drMatchValue;
                GT_U32 drMatchMask;
                GT_U32 drMatchCount;
                GT_U32 drMatchMin;
                GT_U32 drMatchMax;
                GT_U32 drTraceCfg;
                GT_U32 drTraceLastWritePtr;
                GT_U32 drTraceArray[16];
            }debugRing;
        } pbCenter;
        struct /* GPC_GRP_PACKET_WRITE */
        {
            /* each related to 4 GPC_PACKET_WRITE */
            GT_U32 npmRandomizerEnable;
        } gpcGrpPacketWrite[MAX_DP_PER_TILE_CNS / 4];
        struct /* GPC_PACKET_WRITE */
        {
            GT_U32 cfgEnable;
            GT_U32 cfgEnableChannel;
            GT_U32 cfgEnableChannelHigh;
            GT_U32 configStatistics;
            GT_U32 cfgFreeListBuffer;
            GT_U32 cfgNextPointerBuffer;
            GT_U32 cfgCellPointerBuffer;
            GT_U32 cfgCaFreeListFifo; /* SIP_6_30 only */
            GT_U32 cfgNpmWriteCmdFifo; /* SIP_6_30 only */
            GT_U32 statisticsCounters[24];
            GT_U32 cfgSmbRateLimiter;
            GT_U32 cfgNpmRateLimiter;
            GT_U32 interruptCause;
            GT_U32 interruptMask;
            GT_U32 debugClear;
            GT_U32 debugClearChannel;
            GT_U32 debugClearChannelHigh;
            GT_U32 debugStatus;
        } gpcPacketWrite[MAX_DP_PER_TILE_CNS];
        struct /* GPC_GRP_PACKET_READ */
        {
            /* each related to 2 GPC_PACKET_READ */
            GT_U32 badAddress;
        } gpcGrpPacketRead[MAX_DP_PER_TILE_CNS / 2];
        struct /* GPC_PACKET_READ */
        {
            GT_U32 channelEnable[57];               /* 10 Falcon, 28 Hawk, 57 Phoenix  */
            GT_U32 channelReset[57];                /* 10 Falcon, 28 Hawk, 57 Phoenix  */
            GT_U32 channelShaper[10];               /* Falcon only */
            GT_U32 channelProfile[57];              /* 10 Falcon, 28 Hawk, 57 Phoenix  */
            GT_U32 profileMergeCellCredits[12];     /* Hawk only */
            GT_U32 profileMergeLevel012Credits[12]; /* Hawk only */
            GT_U32 profileReorderCellCredits[12];   /* Hawk only */
            GT_U32 profileReorderH2tCredits[12];    /* Hawk only */
            GT_U32 profileReorderLvl01Credits[12];  /* Hawk only */
            GT_U32 profileReorderLvl2Credits[12];   /* Hawk only */
            GT_U32 profileShaper[12];               /* Hawk only */
            GT_U32 packetCountEnable;               /* Falcon and Hawk */
            GT_U32 packetCountReset;                /* Falcon and Hawk */
            GT_U32 fifoSegmentStart[10];            /* Falcon only */
            GT_U32 fifoSegmentNext[20];             /* Falcon only */
            GT_U32 cfgUpdateTrigger;                /* Falcon only */
            GT_U32 arbiterPointHighSpeedChannel0En; /* Falcon only */
            GT_U32 arbiterPointGroup1[5];           /* Falcon only */
            GT_U32 arbiterPointGroup2[5];           /* Falcon only */
            GT_U32 arbiterCalendarSlot[181];        /* 92 Hawk, 128 Phoenix, 181 AAS */
            GT_U32 arbiterCalendarMax;              /* adapted to Hawk */
            GT_U32 globalConfig;                    /* Ironman only */
            struct /*debug*/
            {
                struct /*latency statistics - both Falcon and Hawk */
                {
                    GT_U32 lsEnable;
                    GT_U32 lsCapture;
                    GT_U32 lsMinTheshold[2];
                    GT_U32 lsMaxTheshold[2];
                    GT_U32 lsTotalCount[2];
                    GT_U32 lsThesholdCount[2];
                    GT_U32 lsMinLatency[2];
                    GT_U32 lsMaxLatency[2];
                }latency;
                struct /*credits - both Falcon and Hawk */
                {
                    GT_U32 creditCntControl;
                    GT_U32 reorderChannelCreditCnt0;
                    GT_U32 reorderChannelCreditCnt1;
                    GT_U32 reorderCommandCreditCnt0;
                    GT_U32 reorderCommandCreditCnt1;
                    GT_U32 commandQueueCreditCnt0;
                    GT_U32 commandQueueCreditCnt1;
                    GT_U32 mergeQueueCreditCnt; /* Falcon only */
                    GT_U32 txRequestCnt;
                    GT_U32 cellReadCreditCnt;
                    GT_U32 readNpmRequestMswCreditCnt;
                    GT_U32 countNpmRequestCreditCnt;
                }credits;
                GT_U32 fifoStatus0Channel[57];     /* 10 Falcon, 28 Hawk, 57 Phoenix  */
                GT_U32 fifoStatus1Channel[57];     /* 10 Falcon, 28 Hawk, 57 Phoenix  */
                GT_U32 fillStatus0;                /* Falcon only */
                GT_U32 fillStatus1;                /* Falcon only */
                GT_U32 reorderFreeListFillLevel0;  /* Hawk only */
                GT_U32 reorderFreeListFillLevel1;  /* Hawk only */
                GT_U32 pcReorderFreeListFillLevel; /* Hawk only */
                GT_U32 pktCntFillStatus;           /* Falcon only */
                GT_U32 cellCntChannelMask;         /* Falcon and Hawk */
                GT_U32 cellCntChannelMask1;        /* Hawk only */
                GT_U32 descriptorCellCnt;          /* Falcon and Hawk */
                GT_U32 requestCellCnt;             /* Falcon and Hawk */
                GT_U32 mergeFifoFillLevel;         /* Hawk only */
                GT_U32 pcMergeFifoFillLevel;       /* Hawk only */
            }debug;
            struct /*interrupts*/
            {
                GT_U32 channelCause[57];  /* 10 Falcon, 28 Hawk, 57 Phoenix  */
                GT_U32 channelMask[57];   /* 10 Falcon, 28 Hawk, 57 Phoenix  */
                GT_U32 packetCountCause;
                GT_U32 packetCountMask;
                GT_U32 miscCause;
                GT_U32 miscMask;
                GT_U32 packetCountCreditCause;
                GT_U32 packetCountCreditMask;
                GT_U32 packetReadCreditCause;
                GT_U32 packetReadCreditMask;
                GT_U32 channelsSummary0Cause;
                GT_U32 channelsSummary0Mask;
                GT_U32 channelsSummary1Cause;
                GT_U32 channelsSummary1Mask;
                GT_U32 summaryCause;
                GT_U32 summaryMask;
            }interrupts;
        } gpcPacketRead[MAX_DP_PER_TILE_CNS];

        struct /* GPC_GRP_CELL_READ */
        {
            /* each related to 2 GPC_CELL_READ */
            GT_U32 badRegister;
        } gpcGrpCellRead[MAX_DP_PER_TILE_CNS / 2];

        struct /* GPC_CELL_READ */
        {
            GT_U32 initialization;
            GT_U32 interruptCause;
            GT_U32 interruptMask;
            GT_U32 fifoStatus;
            GT_U32 fillStatus;
            GT_U32 miscStatus;
            struct /*latency statistics*/
            {
                GT_U32 lsEnable;
                GT_U32 lsCapture;
                GT_U32 lsMinTheshold[2];
                GT_U32 lsMaxTheshold[2];
                GT_U32 lsTotalCount[2];
                GT_U32 lsThesholdCount[2];
                GT_U32 lsMinLatency[2];
                GT_U32 lsMaxLatency[2];
            }latency;
        } gpcCellRead[MAX_DP_PER_TILE_CNS];

        struct /* NPM_MC */
        { /* not ready*/
            GT_U32 cfgEnable;
            GT_U32 cfgUnit;
            GT_U32 cfgRefs;
            GT_U32 clearRefs[4];
            GT_U32 cfgList;
            GT_U32 insList[4];
            struct /*interrupts*/
            {
                GT_U32 summaryCause;
                GT_U32 summaryMask;
                GT_U32 genericCause;
                GT_U32 genericMask;
                GT_U32 unitErrorCause;
                GT_U32 unitErrorMask;
                GT_U32 reinsertCause;
                GT_U32 reinsertMask;
                GT_U32 reinsertFailCause;
                GT_U32 reinsertFailMask;
                GT_U32 unitRefsCause;
                GT_U32 unitRefsMask;
                GT_U32 unitDataCause;
                GT_U32 unitDataMask;
                GT_U32 refsErrorCause;
                GT_U32 refsErrorMask;
                GT_U32 agingErrorCause;
                GT_U32 agingErrorMask;
                GT_U32 refsFifoCause;
                GT_U32 refsFifoMask;
                GT_U32 listErrorCause;
                GT_U32 listErrorMask;
            }interrupts;
            struct /*ramAccess*/
            {
                GT_U32 refCntRamControl;
                GT_U32 refCntRamData0;
                GT_U32 refCntRamData1;
                GT_U32 nextPointerRamControl;
                GT_U32 nextPointerRamData;
                GT_U32 listFifoBufferRamControl;
                GT_U32 listFifoBufferRamData;
            }ramAccess;
        } npmMc[3];
        struct /* SMB_MC */
        {
            GT_U32 cfgInit;
            GT_U32 cfgEnable;
            GT_U32 cfgRefs;
            GT_U32 cfgMsws;
            GT_U32 cfgRbwNoTrustPer; /* Ironman only */
            GT_U32 refsCtrl;
            GT_U32 cfgList;
            GT_U32 dbgRefCntCtrl;
            GT_U32 dbgRefCntDataLow;
            GT_U32 dbgRefCntDataHigh;
            GT_U32 dbgPayloadCtrl;
            GT_U32 dbgPayloadData[8];
            GT_U32 flFifoRamCtrl;
            GT_U32 flFifoRamData;
            GT_U32 dbgCellAlign[4];
            GT_U32 insList[4];
            GT_U32 clrRefs[4];
            GT_U32 interruptMiscCause;
            GT_U32 interruptMiscMask;
            GT_U32 interruptAgeCause;
            GT_U32 interruptAgeMask;
            GT_U32 interruptFifoCause;
            GT_U32 interruptFifoMask;
            GT_U32 interruptRwbCause;
            GT_U32 interruptRwbMask;
            GT_U32 interruptMemoriesCause;
            GT_U32 interruptMemoriesMask;
            GT_U32 interruptMemories2Cause;
            GT_U32 interruptMemories2Mask;
            GT_U32 interruptSummaryCause;
            GT_U32 interruptSummaryMask;
        } smbMc[6];
        struct /* SMB_WRITE_ARBITER */
        {
            GT_U32 cfgEnable;
            GT_U32 cfgReady;
            GT_U32 dbgLpmFailLow;
            GT_U32 dbgLpmFailHigh;
            GT_U32 dbgLpmFailCounter;
            GT_U32 dbgLpmRepeatCounter[8];
            GT_U32 interruptMiscCause;
            GT_U32 interruptMiscMask;
        } smbWriteArbiter[4]; /* Hawk has 4 units */
        struct /* PB_COUNTER */
        { /* not ready */
            GT_U32 enablePbc;
            GT_U32 enableMasterMode;
            GT_U32 enableInterTileTransact;
            GT_U32 cfgTile;
            GT_U32 cfgMode;
            GT_U32 resizeSmbCells;
            GT_U32 resizeSmbMcCells;
            GT_U32 resizeSmbSumCells;
            GT_U32 resizeNpmWords;
            GT_U32 resizeNpmMcWords;
            GT_U32 resizeNpmSumWords;
            GT_U32 cfgFillThreshold;
            /* Counters of currently allocated cells/next-pointers             */
            /* Locals means in current tile, otherwize - global - in all tiles */
            /* Diff means difference between maximal and minimal value         */
            /* OvertimeMax and  OvertimeMin means Maximal or Minimal from the  */
            /* previous reading of the given counter                           */
            GT_U32 dbgSmbLocalMax;
            GT_U32 dbgSmbLocalMin;
            GT_U32 dbgSmbLocalSum;
            GT_U32 dbgSmbLocalMcSum;
            GT_U32 dbgNpmLocalMax;
            GT_U32 dbgNpmLocalMin;
            GT_U32 dbgNpmLocalSum;
            GT_U32 dbgNpmLocalMcSum;
            GT_U32 dbgSmbMax;
            GT_U32 dbgSmbMin;
            GT_U32 dbgSmbSum;
            GT_U32 dbgSmbMcSum;
            GT_U32 dbgSmbUcSum;
            GT_U32 dbgNpmMax;
            GT_U32 dbgNpmMin;
            GT_U32 dbgNpmSum;
            GT_U32 dbgNpmMcSum;
            GT_U32 dbgNpmUcSum;
            GT_U32 dbgSmbDiff;
            GT_U32 dbgNpmDiff;
            GT_U32 dbgSmbOvertimeMaxSum;
            GT_U32 dbgSmbOvertimeMaxMcSum;
            GT_U32 dbgSmbOvertimeMaxUcSum;
            GT_U32 dbgSmbOvertimeMinSum;
            GT_U32 dbgSmbOvertimeMinMcSum;
            GT_U32 dbgSmbOvertimeMinUcSum;
            GT_U32 dbgSmbOvertimeMaxDiff;
            GT_U32 dbgSmbOvertimeMinDiff;
            GT_U32 dbgNpmOvertimeMaxSum;
            GT_U32 dbgNpmOvertimeMaxMcSum;
            GT_U32 dbgNpmOvertimeMaxUcSum;
            GT_U32 dbgNpmOvertimeMinSum;
            GT_U32 dbgNpmOvertimeMinMcSum;
            GT_U32 dbgNpmOvertimeMinUcSum;
            GT_U32 dbgNpmOvertimeMaxDiff;
            GT_U32 dbgNpmOvertimeMinDiff;
            GT_U32 interruptCause;
            GT_U32 interruptMask;
        } pbCounter;
    } sip6_packetBuffer;

     struct /*sip6_txDMA*/{

         struct /*configs*/{

             struct /*globalConfigs*/{

                GT_U32    txDMAGlobalConfig;
                PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT     pizzaArbiter;
                GT_U32    pchBytes; /*not for SIP7*/
                GT_U32    minPacketSize; /*SIP7*/

             }globalConfigs;

             struct /*speedProfileConfigurations*/{

                 GT_U32    descFIFODepth[MAX_SPEED_PROFILE_NUM_CNS]; /* removed in sip6.15 */
                 GT_U32    sdqMaxCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    sdqThresholdBytes[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxWordCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxCellsCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    maxDescCredits[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    interPacketRateLimiter[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    interCellRateLimiter[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    interWordRateLimiter[MAX_SPEED_PROFILE_NUM_CNS]; /*SIP7*/
                 GT_U32    interPacketRateMaxTokens[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    rateLimitSpeedUp[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    rateLimitGranularityFix[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    dpWipThreshold[MAX_SPEED_PROFILE_NUM_CNS];

             }speedProfileConfigs;

             struct /*channelConfigurations*/{

                  GT_U32    speedProfile[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    channelReset[PRV_CPSS_MAX_DMA_NUM_CNS]; /*not for AAS*/
                  GT_U32    descFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS]; /* removed in sip6.15 */
                  GT_U32    interGapConfigitation[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    rateLimiterEnable[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    minPacketSize[PRV_CPSS_MAX_DMA_NUM_CNS]; /*Ironman*//*not for AAS*/
                  GT_U32    channelGlobalConfig[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/
                  GT_U32    channelInterPacketRateLimiter[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/
                  GT_U32    channelInerPacketRateMaxTokens[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/
                  GT_U32    channelRateLimitSpeedUp[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/
                  GT_U32    channelRateLimitGranularityFix[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/
                  GT_U32    channelDpWip[PRV_CPSS_MAX_DMA_NUM_CNS]; /*AAS*/

             }channelConfigs;

         }configs;

         struct /*TDSConfigs*/{

              GT_U32    globalConfig;
              GT_U32    channelReset;
              GT_U32    cellRateLimiterConfig;
              GT_U32    cellRateLimiter;
              GT_U32    maxTdsCellCredits;
              GT_U32    maxTdsDescriptorCredits;

          }TDSConfigs;

          struct /*txdStatus*/{
              struct /*globalStatus*/{
                  struct /*rateLimitEventCnt*/{
                      GT_U32    noEmptyTags;
                      GT_U32    noPbCellCredits;
                      GT_U32    noPbDescriptorCredits;
                      GT_U32    noTxfResourcesForPacket;
                      GT_U32    noTxfResourcesForRequest;
                      GT_U32    noInnerPacketTokens;
                      GT_U32    noInterPacketTokens;
                      GT_U32    noInterCellTokens;
                      GT_U32    noDescriptors;
                  }rateLimitEventCnt;
                  struct /*eventCnt*/{
                      GT_U32    descriptorRequests;
                      GT_U32    pdsDescriptors;
                      GT_U32    txfDescriptors;
                      GT_U32    cellRequests;
                      GT_U32    txfWordResponses;
                      GT_U32    tdsDescriptorCredits;
                      GT_U32    tdsCellCredits;
                      GT_U32    tdsWordResponses;
                      GT_U32    tdsDescriptorRequests;
                      GT_U32    tdsCellRequests;
                  }eventCnt;
                  GT_U32    txdIsIdle;
                  GT_U32    maxPbCellLatency;
              }globalStatus;
              struct /*channelStatus*/{
                  GT_U32    sdqCredit[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    sdqCreditHigh[PRV_CPSS_MAX_DMA_NUM_CNS];/*AAS*/
                  GT_U32    txfDescriptorCredits[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    txfCellCredits[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    txfWordCredits[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    pbCredits[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    txdInternalState[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    txdFaultEvents[PRV_CPSS_MAX_DMA_NUM_CNS];
              }channelStatus;
          }txdStatus;
          struct /*debug*/{
              GT_U32    statusBadAddr;
              GT_U32    txdDebugConfig;
              GT_U32    debugBus;
              GT_U32    enableDebugCounters;
              GT_U32    creditsCountersOverride;   /* sip6.15 */
              GT_U32    writeCreditsCounters;      /* sip6.15 */
              GT_U32    enableOrDisableInterface; /*AAS*/
              GT_U32    descriptorOverrideValue[4];
              GT_U32    descriptorOverrideMask[4];
          }debug;

          struct /*interrupts*/ {
              GT_U32 interruptCause;
              GT_U32 interruptMask;
          }interrupts;

     }sip6_txDMA[MAX_DP_CNS]/*TXDMA_UNIT*/;

     struct /*sip6_txFIFO*/{

         struct /*configs*/{

                 struct /*globalConfigs*/{

                 GT_U32    globalConfig1;
                 GT_U32    packetsMaximalReadLatencyFromPacketPuffer; /* Falcon only */
                 GT_U32    sdmaPacketsMaximalReadLatencyFromPacketPuffer;
                 GT_U32    wordBufferCmn8wr4rdBufferConfig1;
                 GT_U32    wordBufferCmn8wr4rdBufferConfig2;
                 PRV_CPSS_DXCH_PP_DP_TX_PIZZA_ARBITER_STCT     pizzaArbiter;

             }globalConfigs;

             struct /*speedProfileConfs*/{

                 GT_U32    descFIFODepth[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    tagFIFODepth[MAX_SPEED_PROFILE_NUM_CNS];
                 GT_U32    packetsMaximalReadLatencyFromPacketPuffer[MAX_SPEED_PROFILE_NUM_CNS]; /* Hawk */
                 GT_U32    txfPcaCredit[MAX_SPEED_PROFILE_NUM_CNS]; /*AAS*/
                 GT_U32    txfDataFifoCredit[MAX_SPEED_PROFILE_NUM_CNS]; /*AAS*/

             }speedProfileConfigs;

              struct /*ChannelConfigs*/{

                  GT_U32    speedProfile[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    descFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    tagFIFOBase[PRV_CPSS_MAX_DMA_NUM_CNS];
                  GT_U32    ptpDispatch[PRV_CPSS_MAX_DMA_NUM_CNS]; /*Ironman*/
                  GT_U32    channelGenConfig[PRV_CPSS_MAX_DMA_NUM_CNS];/*AAS*/

              }channelConfigs;

          }configs;

          struct /*interrupts*/ {
              GT_U32 interruptCause;
              GT_U32 interruptMask;
          }interrupts;

           struct /*debug*/{

               GT_U32   statusBadAddr;
               GT_U32   dpTxfEnableDebugCounters;
               GT_U32   dpTxfDebugConfig;
               GT_U32   dpTxfDebugThresholdsConfig;
               struct /*debugStatus*/{
                   GT_U32   dpTxfGeneralStatus;
                   GT_U32   dpTxfDescriptorFifoOverflowInfo;
                   GT_U32   dpTxfDebugBus;
                   GT_U32   dpTxfTagInfoFromTab;
               }debugStatus;
               struct /*tfxCounters*/{
                   GT_U32   descriptorReadByReadCtrl;
                   GT_U32   cellsSplitFromDescriptors;
                   GT_U32   wordsGenerateInPrefetchFifo;
                   GT_U32   sopReadCtrlDeriveToWb;
                   GT_U32   cyclesFromFirstDescriptor;
                   GT_U32   alignerInputPackets;
                   GT_U32   alignerOutputPackets;
                   GT_U32   alignerOutputByteCount;
               }tfxCounters;
               struct /*tfxWordBufeer*/{
                   GT_U32   cmn8wr4rdBufferStatus;
               }tfxWordBufeer;
               struct /*tfxReadControl*/{
                   GT_U32   creditCounter;
                   GT_U32   minCreditCounter;
                   GT_U32   maxCreditCounter;
                   GT_U32   fifoPointers;
                   GT_U32   wordFifoPointers;
               }tfxReadControl;
               struct /*tfxAligner*/{
                   GT_U32   alignerSdmaStatus;
                   GT_U32   alignerResidueCtrl;
               }tfxAligner;
           }debug;

     }sip6_txFIFO[MAX_DP_CNS]/*TXFIFO_UNIT*/;

    struct /*SHM*/{

        GT_U32    block_port_select[130]/*SBM number (per tile)*/;
        GT_U32    shm_engines_size_mode;    /* NA for sip7 */
        GT_U32    bad_address_latch;
        GT_U32    shm_error_sum_cause;
        GT_U32    shm_error_sum_mask;
        GT_U32    lpm_illegal_address;
        GT_U32    fdb_illegal_address;
        GT_U32    em_illegal_address;
        GT_U32    arp_illegal_address;
        GT_U32    lpm_aging_illegal_address;

        /* new sip7 registers*/
        GT_U32    ts_illegal_address;
        GT_U32    cnc_illegal_address;
        GT_U32    plr_met_illegal_address;
        GT_U32    plr_count_illegal_address;
        GT_U32    ipe_illegal_address;
        GT_U32    hf0_illegal_address;
        GT_U32    hf1_illegal_address;

        GT_U32    ipe0_state_select;
        GT_U32    ipe1_state_select;

        GT_U32    debug_access_conf;
        GT_U32    debug_access_write_data[SHM_DATA_SIZE_CNS];
        GT_U32    debug_access_read_data[SHM_DATA_SIZE_CNS];
        GT_U32    debug_access_read_status;

    }SHM;/*sip6 shared memories (FDB/EM/ARP-TS/LPM) */

    struct{
        struct /*pha_regs*/{

            struct /*PPAThreadConfigs*/{

                GT_U32    nullProcessingInstructionPointer;

            }PPAThreadConfigs;

            struct /*generalRegs*/{

                GT_U32    PHACtrl;
                GT_U32    PHAStatus;

            }generalRegs;

            struct /*errorsAndInterrupts*/{

                GT_U32    headerModificationViolationConfig;
                GT_U32    tablesReadErrorConfig;
                GT_U32    PPAClockDownErrorConfig;
                GT_U32    headerModificationViolationAttributes;
                GT_U32    PHABadAddrLatch;
                GT_U32    PHAInternalErrorCause;
                GT_U32    PHAInternalErrorMask;
                GT_U32    PHAInterruptSumCause;
                GT_U32    PHAInterruptSumMask;

            }errorsAndInterrupts;

            struct /*debugCounters*/{
                GT_U32    statisticalProcessingDebugCounterConfig;
                GT_U32    statisticalProcessingDebugCounter;
            }debugCounters;

        }pha_regs;

        struct /*PPA*/{

            struct /*ppa_regs*/{

                GT_U32    PPACtrl;
                GT_U32    PPAInterruptSumCause;
                GT_U32    PPAInterruptSumMask;
                GT_U32    PPAInternalErrorCause;
                GT_U32    PPAInternalErrorMask;
                GT_U32    PPABadAddrLatch;

            }ppa_regs;

        }PPA;

        struct /*PPG[3]*/{
            struct /*ppg_regs*/{

                GT_U32    PPGInterruptSumCause;
                GT_U32    PPGInterruptSumMask;
                GT_U32    PPGInternalErrorCause;
                GT_U32    PPGInternalErrorMask;
                GT_U32    PPGBadAddrLatch;

            }ppg_regs;

            struct /*PPN[9]*/{
                struct /*ppn_regs*/{
                    GT_U32    fwDeviceName;     /* hold 4 char of 'device name' like : FLCN (Falcon) or AC5P (Hawk) */
                    GT_U32    fwVersion;        /* hold 4 U8 of : year,month,version,debugValue (18 06 00 00 , means 2018-June ver00 debug00)*/
                    GT_U32    fwImageId;        /* fw image ID */
                    GT_U32    TODWord0[2]       /*tod IF*/;
                    GT_U32    TODWord1[2]       /*TOD IF*/;
                    GT_U32    TODWord2[2]       /*TOD IF*/;
                    GT_U32    PPNDoorbell;
                    GT_U32    pktHeaderAccessOffset;
                    GT_U32    PPNInternalErrorCause;
                    GT_U32    PPNInternalErrorMask;
                    GT_U32    PPNBadAddrLatch;

                }ppn_regs;
            }PPN[MAX_PPN_IN_PPG_CNS];

            /* manually added */
            GT_U32      PPG_IMEM_base_addr;/* base address of this memory (4 words per entry) */
        }PPG[MAX_PPG_CNS];

    }PHA;/* sip6 PHA unit */

    struct /* PPU - SIP6.10 */ {
        struct /*general regs*/{
            GT_U32  ppuGlobalConfig;
            GT_U32  ppuInterruptCause;
            GT_U32  ppuInterruptMask;
            GT_U32  ppuLastAddrViolation;
        }generalRegs;

        struct /*debug regs*/{
            GT_U32  ppuDebugCounterIn;
            GT_U32  ppuDebugCounterOut;
            GT_U32  ppuDebugCounterKstg0;
            GT_U32  ppuDebugCounterKstg1;
            GT_U32  ppuDebugCounterKstg2;
            GT_U32  ppuDebugCounterDau;
            GT_U32  ppuDebugCounterLoopback;
            GT_U32  ppuMetalFixRegister;
            GT_U32  ppuIdleState;
            GT_U32  ppuDebug;
            GT_U32  ppuThresholds;
        }debugRegs;

        struct /* SP Bus default profile %p word %w */{
            GT_U32 word[5];
        }spBusDefaultProfile[MAX_PPU_SP_BUS_PROFILE_NUM_CNS];

        struct /* Error Profile %p field %b */ {
            GT_U32 field[4];
        }errProfile[MAX_PPU_ERROR_PROFILE_NUM_CNS];

        /* Interrupt Counter %i */
        GT_U32  interruptCounter[MAX_PPU_INT_COUNTER_NUM_CNS];

        struct /* Key gen profile k_stage %k profile %i byte %b */{
            struct {
                GT_U32  byte[7];
            }keyGenProfile[MAX_PPU_KEY_GEN_PROFILE_NUM_CNS];

            struct /* TCAM regs */{
                GT_U32 keyLsb;
                GT_U32 keyMsb;
                GT_U32 maskLsb;
                GT_U32 maskMsb;
                GT_U32 validate;
            }tcamKey[MAX_PPU_TCAM_RULES_NUM_CNS];
        }kstg[MAX_PPU_KSTG_NUM_CNS];

        struct /*dau regs*/{
            GT_U32  dauProtWin[MAX_PPU_DAU_PROT_WIN_NUM_CNS];
            GT_U32  dauProtWinLatchData;
        }dau;

    }PPU; /* PPU - SIP6.10 Only */

    struct /* PPU_FOR_IPE - SIP7 */ {
        struct /*general regs*/{
            GT_U32  ppuGlobalConfig;
            GT_U32  ppuInterruptCause;
            GT_U32  ppuInterruptMask;
            GT_U32  ppuLastAddrViolation;
        }generalRegs;

        struct /*debug regs*/{
            GT_U32  ppuDebugCounterIn;
            GT_U32  ppuDebugCounterOut;
            GT_U32  ppuDebugCounterKstg[MAX_PPU_FOR_IPE_KSTG_NUM_CNS];
            GT_U32  ppuDebugCounterLoopback;
            GT_U32  ppuMetalFixRegister;
            GT_U32  ppuIdleState;
            GT_U32  ppuDebug;
            GT_U32  ppuThresholds;
        }debugRegs;

        struct /* SP Bus default profile %p word %w */{
            GT_U32 word[5];
        }spBusDefaultProfile[MAX_PPU_SP_BUS_PROFILE_NUM_CNS];

        struct /* Error Profile %p field %b */ {
            GT_U32 field[4];
        }errProfileSp[MAX_PPU_ERROR_PROFILE_NUM_CNS];

        /* Interrupt Counter %i */
        GT_U32  interruptCounter[MAX_PPU_INT_COUNTER_NUM_CNS];

        struct /* Key gen profile k_stage %k profile %i byte %b */{
            struct {
                GT_U32  byte[7];
            }keyGenProfile[MAX_PPU_KEY_GEN_PROFILE_NUM_CNS];

            struct /* TCAM regs */{
                GT_U32 keyLsb;
                GT_U32 keyMsb;
                GT_U32 maskLsb;
                GT_U32 maskMsb;
                GT_U32 validate;
            }tcamKey[MAX_IPE_PPU_TCAM_RULES_NUM_CNS];
        }kstg[MAX_PPU_FOR_IPE_KSTG_NUM_CNS];

    }PPU_FOR_IPE; /* IPE PPU - SIP7 Only */

    struct /* CNM */
    {
        struct /* AAC */
        {
            GT_U32    AACControl;
            GT_U32    AACEnginesStatus;
            GT_U32    AACInvalidtransactionaddress;
            GT_U32    AACErrorStatus;
            GT_U32    AACInterruptsCause;
            GT_U32    AACInterruptsMask;
            GT_U32    AACBadAddrLatchReg;
            GT_U32    AACEngineControl[8];
            GT_U32    AACEngineData[8];
            GT_U32    AACEngineAddress[8];
            GT_U32    AACEngineMCInternalOffset[8];
            GT_U32    AACEngineMCExternalOffset[8];
            GT_U32    AACEngineMCControl[8];
        }AAC; /* AAC unit */

        struct /* MPP_RFU */
        {
            GT_U32   MPPControl0_7;
            GT_U32   MPPControl8_15;
            GT_U32   MPPControl16_23;
            GT_U32   MPPControl24_31;
            GT_U32   MPPControl32_39;
            GT_U32   MPPControl40_47;
            GT_U32   MPPControl48_55;
            GT_U32   MPPControl56_63;

        }MPP_RFU; /* MPP_RFU unit */

        /*  Relevant for AC5X and Ironman and AAS
            see :
            SIP6_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT
            SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_ENT
        */
        struct /*ADDR_DECODER*/
        {
            struct /*WIN_CTRL*/
            {
                GT_U32   winAttrAndSizeHigh;
                GT_U32   winSizeLow;
                GT_U32   winBaseAndRemapHigh;
                GT_U32   winBaseLow;
                GT_U32   winRemapLow;
                GT_U32   winAxiAttr;
            } WIN_CTRL[16];
        } ADDR_DECODER[MAX_UNITS_ADDR_DECODER];

        struct /*GDMA*/ {
            struct/*General Regs*/ {
                GT_U32 gdmaBuffersSize;
                GT_U32 gdmaMiscConfig;
                GT_U32 axiTotalOutstandingControlReg;
                GT_U32 gdmaSWResetReg;
                GT_U32 gdmaPayloadBufferThreshold;
            }generalRegs;

            struct/*Sys Counter Regs*/ {
                GT_U32 sc_cr;
                GT_U32 sc_cvlr;
                GT_U32 sc_cvhr;
                GT_U32 sc_cvlr_st;
                GT_U32 sc_cvhr_st;
                /*GT_U32 timerClockDividerControl;*/
            }sysCounterRegs;

            struct/*SGD and AXI Control Regs*/ {
                GT_U32 sgdChainBaseAddressLow;
                GT_U32 sgdChainBaseAddressHigh;
                GT_U32 sgdBufferAllocationControl;
                GT_U32 payloadBufferAddressHigh;
                GT_U32 addGDMASGDOwn;
                GT_U32 axiMaxAttrTable;
                GT_U32 axiSGDAttributes;
                GT_U32 sgdChainSize;
            }sgdAxiControlRegs[PRV_CPSS_NUM_OF_GDMA_RINGS_CNS];

            struct/*GDMA Timer Regs*/ {
                GT_U32 timerValue;
                GT_U32 timerJitterCfg;
            }gdmaTimerRegs[PRV_CPSS_NUM_OF_GDMA_RINGS_CNS];

            struct/*Queue Status and Control Regs*/ {
                GT_U32 queueEnable;
                GT_U32 rxBufferByteCount;
                GT_U32 queueControl;
                GT_U32 rxStatCounter;
                GT_U32 wruSGDPointer;
                GT_U32 freeSGDsStatusRegister;
                GT_U32 RX_Drop_counter[2];
                GT_U32 stat_counter[2];
                GT_U32 stat_byte_counter[2];
            }queueStatusAndControlRegs[PRV_CPSS_NUM_OF_GDMA_RINGS_CNS];

            struct/*Multicast Profile Regs*/ {
                GT_U32 multicastInternalOffset;
                GT_U32 multicastExternalOffset;
            }multicastProfileRegs[PRV_CPSS_MAX_GDMA_MC_PROFILE_NUM_CNS];

            struct/*PDI Slave Regs*/ {
                GT_U32 pdiSlaveControl;
                GT_U32 pdiSlaveArbiterControl;
            }pdiSlaveRegs[PRV_CPSS_MAX_GDMA_PDI_SLAVE_NUM_CNS];

            struct/*PDI Master Regs*/ {
                GT_U32 pdiMasterControl;
            }pdiMasterRegs;

            struct/*PDI to PDI Queue Profile Regs*/ {
                GT_U32 qProfileControlRegister;
                GT_U32 header[PRV_CPSS_MAX_GDMA_PDI_QUEUE_PROFILE_HEADER_NUM_CNS];
            }pdiToPdiQueueProfileRegs[PRV_CPSS_MAX_GDMA_PDI_QUEUE_PROFILE_NUM_CNS];

            struct/*Interrupt and Debug Regs*/ {
                GT_U32 gdmaRFMetalFixRegister;
                GT_U32 gdmaMiscInterruptCause;
                GT_U32 gdmaMiscInterruptMask;
                GT_U32 badAddrLatchReg;
                GT_U32 payloadBufferMaxFillLevel;
                GT_U32 gdmaQueueStatusRegister;
                GT_U32 gdmaStatusRegister;
                GT_U32 currentSGD0;
                GT_U32 currentSGD1;
                GT_U32 currentSGD2;
                GT_U32 currentSGD3;
                GT_U32 debugControlRegister;
                GT_U32 gdmaQInterruptCause[PRV_CPSS_NUM_OF_GDMA_RINGS_CNS];
                GT_U32 gdmaQInterruptMask[PRV_CPSS_NUM_OF_GDMA_RINGS_CNS];
            }interruptAndDebugRegs;

        }GDMA;/* single instance : regardless to number of GDMA units in the device
        see : prvCpssHwPpGdmaWriteReg(...) to allow write to all GDMA units */

        struct /*GDMA_Dispatcher*/ {
            GT_U32 gdmaDispatcherControl          ;
            GT_U32 dsaFieldOffsets                ;
            GT_U32 packetDebugSuffix              ;
            GT_U32 gdmaDispatcherInterruptCause   ;
            GT_U32 gdmaDispatcherInterruptMask    ;
            GT_U32 badAddrLatchReg                ;
            GT_U32 dispatcherStatusReg            ;
        }GDMA_Dispatcher;

        struct/*interrupts*/{
            struct/*interrupts*/{
                GT_U32  interruptCoalescingConfig;
                GT_U32  globalInterruptCounterInitializationValue;
            }tree[16];
        }RFU_INTERRUPTS;

        /* XSMI logic was moved to CNM in SIP7 */
        struct /*XSMI*/{

            GT_U32 XSMIManagement;
            GT_U32 XSMIAddress;
            GT_U32 XSMIConfiguration;
            GT_U32 XSMIInterruptCause;
            GT_U32 XSMIInterruptMask;

        }XSMI[MAX_CNM_XSMI_UNITS_CNS];

    }CNM; /* sip6 CNM unit */

    struct /* SMU : new in sip6.30 */
    {
        struct{
            GT_U32 smuBadAddrLatchReg;
            GT_U32 smuInterruptCause;
            GT_U32 smuInterruptMask;
        }smuMiscellaneous;

        struct{
            GT_U32 todNanoSec;
            GT_U32 todSecLsb;
            GT_U32 todSecMsb;
        }debug;

        struct{
            GT_U32 irfSngGlobalConfig;
            GT_U32 irfSngCountersFieldsConfig;
            GT_U32 irfAgeBit[STREAM_SNG_NUM_CNS/32];/*64 registers*/
        }irfSng;

        struct{
            GT_U32 sgcGlobalConfig;
            GT_U32 sgcGlobalScanConfig;
            GT_U32 sgcReconfigTodMsbHigh;
            GT_U32 sgcReconfigTodMsbLow;
            GT_U32 sgcReconfigTodLsb;
            GT_U32 sgcReconfigTableSetConfig;
            GT_U32 sgcReconfigPendingStatus;
            GT_U32 sgcReconfigValidStatus;
            GT_U32 sgcTableSetIsActive[2];
            GT_U32 sgcGateId2TableSetConfig[13];
        }streamGateControl;

    }SMU;

    struct /* EMX : new in sip7 */
    {
        GT_U32 emChannel[MAX_EMX_CHANNEL_NUM_CNS];
        GT_U32 emxInterruptCause;
        GT_U32 emxInterruptMask;

        struct{
            GT_U32 ilmExactMatchProfileTableConfig[MAX_EMX_PROFILE_NUM_CNS];
            GT_U32 ilmExactMatchProfileTableMaskData[MAX_EMX_PROFILE_NUM_CNS][2];
            GT_U32 ilmExactMatchProfileTableDefaultActionData[MAX_EMX_PROFILE_NUM_CNS][12];
            GT_U32 ilmExactMatchProfileTableReducedActionData[MAX_EMX_PROFILE_NUM_CNS][MAX_EMX_REDUCED_ACTION_BYTES_CNS];
        }ilmExactMatchProfileTable;

        struct{
            GT_U32 exactMatchProfileTableConfig[MAX_EMX_PROFILE_NUM_CNS];
            GT_U32 exactMatchProfilesTableMaskData[MAX_EMX_PROFILE_NUM_CNS][14];
            GT_U32 exactMatchProfileTableDefaultActionData[MAX_EMX_PROFILE_NUM_CNS][12];
            GT_U32 exactMatchProfileTable10BSelect[MAX_EMX_PROFILE_NUM_CNS][6];
            GT_U32 exactMatchProfileTableReducedActionData[MAX_EMX_PROFILE_NUM_CNS][MAX_EMX_REDUCED_ACTION_BYTES_CNS];
        }exactMatchProfilesTable[MAX_EMX_UNIT_NUM_CNS];
    }EMX;

    struct /* TTI LU : new in sip7 */
    {
        struct{
            GT_U32 ttiLuGlobalConfig;
            GT_U32 ttiInterruptCause;
            GT_U32 ttiInterruptMask;
            GT_U32 ttiLastAddrViolation;
        }generalRegs;

        struct{
            GT_U32 ilmLuSerialEmProfile[MAX_TTI_LU_SERIAL_EM_PROFILE_NUM_CNS];
        }ilmLuConfig[MAX_TTI_LU_SERIAL_LOOKUP_NUM_CNS];

        struct{
            GT_U32 ttiLuTtSrcIdAssignment[4];
            GT_U32 ttiLuIlmSrcIdAssignment[4];
            GT_U32 ttiLuMplsExceptionCmds;
            GT_U32 ttiLuIpExceptionCmds;
            GT_U32 ttiLuGenericActionConfig;
            GT_U32 ttiLuGenericClassifParallelLu[MAX_TTI_LU_PARALLEL_LOOKUP_NUM_CNS];
            GT_U32 ttiLuGenericClassifSerialEmLu[MAX_TTI_LU_SERIAL_LOOKUP_NUM_CNS];
            GT_U32 ttiLuTunnelHeaderLenProfile[8];
            struct{
                GT_U32 qosEntry[MAX_TTI_LU_ILM_QOS_ACTION_ENTRY_NUM_CNS];
            }ttiLuIlmLookupIdxQosActionResolution[MAX_TTI_LU_SERIAL_LOOKUP_NUM_CNS];

            struct{
                GT_U32 ttlEntry[MAX_TTI_LU_ILM_TTL_ACTION_ENTRY_NUM_CNS];
            }ttiLuIlmLookupIdxTtlActionResolution[MAX_TTI_LU_SERIAL_LOOKUP_NUM_CNS];

            struct{
                GT_U32 qosEntry[MAX_TTI_LU_ILM_QOS_ACTION_ENTRY_NUM_CNS];
            }ttiLuParallelLookupIdxQosActionResolution[MAX_TTI_LU_PARALLEL_LOOKUP_NUM_CNS];

            struct{
                GT_U32 ttlEntry[MAX_TTI_LU_ILM_TTL_ACTION_ENTRY_NUM_CNS];
            }ttiLuParallelLookupIdxTtlActionResolution[MAX_TTI_LU_PARALLEL_LOOKUP_NUM_CNS];
        }ttiLuActionConfig;

        struct{
            GT_U32    ttiLuRangeClassification0[128];
            GT_U32    ttiLuRangeClassification1[128];
        }ttiLuGenericRangesConfig;

    }TTI_LU;

}PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC;
/* restore alignment setting */
#pragma pack(pop)

/**
* @internal prvCpssDxChNonSip5HwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers structure for pre eArch devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChNonSip5HwRegAddrVer1Init
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChHwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrVer1Init
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChHwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller registers
*         struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller (DFX) registers
*         struct for xCat3 devices.
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
);

/**
* @internal prvCpssDxChCgPortDbInvalidate function
* @endinternal
*
* @brief   Invalidate or init CG port register database
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - CG MAC port number
* @param[in] cgUnitInvalidate         - invalidate status
*                                      GT_TRUE - invalidate DB for given MAC port
*                                      GT_FALSE - init DB for given MAC port
*                                       None
*/
GT_STATUS prvCpssDxChCgPortDbInvalidate
(
    IN  GT_U8 devNum,
    IN  GT_U32 portNum,
    IN  GT_BOOL cgUnitInvalidate
);

/**
* @internal debug_unitAccessEnableCheckCallbackSet function
* @endinternal
*
* @brief   This function sets callback to filter access to units address space.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] funcPtr      - (pointer to)
*
* @retval none
*/
GT_VOID debug_unitAccessEnableCheckCallbackSet
(
    IN GT_U32 (*funcPtr)(GT_U8 devNum, GT_U32 unitId)
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChRegsVer1h */

