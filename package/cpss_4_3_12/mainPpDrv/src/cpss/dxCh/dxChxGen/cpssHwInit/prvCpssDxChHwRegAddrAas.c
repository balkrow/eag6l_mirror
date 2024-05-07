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
* @file prvCpssDxChHwRegAddrAas.c
*
* @brief This file implement DB of units base addresses for AAS.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwAasInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwAasAddrSpace.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwCnmAddressDecoder.h>

#define UNIT___NOT_VALID PRV_CPSS_DXCH_UNIT___NOT_VALID___E

typedef struct{
    PRV_CPSS_DXCH_UNIT_ENT tile0Id;
    GT_U32                 tile0baseAddr;
    GT_U32                 sizeInBytes;
}UNIT_IDS_STC;

#define MG_SIZE (1   * _1M)
#define NUM_MG_PER_CNM     2
#define NUM_MGS_FOR_SDMA   2

#define NUM_OF_DP_UNITS               4 /*4 DP[]*/
#define NUM_OF_LMU_UNITS_PER_GOP      8 /* 8 units per every GOP/DP */
#define GOP_INDEX_CPU_NETWORK_PORT    0/* GOP index DP index = 0 */

/* per tile info on SERDESES */
#define SERDES_NUM_NETWORK_PORTS    80
#define SERDES_NUM_CPU_PORTS        1

/* There are up to 4 LED servers/units */
/*
0x92200000 + 0x1500000+a*0: where a (2-2) represents LED
0x92200000 + 0x6500000+a*0x1000000: where a (0-1) represents LED
0x92200000 + a*0: where a (3-3) represents LED CPU
*/
#define PRV_CPSS_DXCH_AAS_LED_UNIT_NUM_CNS                           4

/* There are up to 32 LED ports per one LED server */
#define PRV_CPSS_DXCH_AAS_LED_UNIT_PORTS_NUM_CNS                    32

#define GM_STEP_BETWEEN_MIB_PORTS   0x400
#define GM_STEP_BETWEEN_DPS         _64K

#define RXDMA_DIFF_0_TO_1       (AAS_ADDR_DP1_RX -AAS_ADDR_DP0_RX )
#define TXDMA_DIFF_0_TO_1       (AAS_ADDR_DP1_TXD-AAS_ADDR_DP0_TXD)
#define TXFIFO_DIFF_0_TO_1      (AAS_ADDR_DP1_TXF-AAS_ADDR_DP0_TXF)

/* macro for MAC,PCS,ANP for a 'CPU' port */
#define CPU_100G_P4(_global,_gop,_p) \
     {PRV_CPSS_DXCH_UNIT_MAC_CPU_100G_##_global##_E   ,AAS_ADDR_GOP##_gop##_4P_MAC100_##_p   ,64  *  _1K} \
    ,{PRV_CPSS_DXCH_UNIT_PCS_CPU_100G_##_global##_E   ,AAS_ADDR_GOP##_gop##_4P_PCS100_##_p   ,128 *  _1K} \
    ,{PRV_CPSS_DXCH_UNIT_ANP1_CPU_100G_##_global##_E  ,AAS_ADDR_GOP##_gop##_4P_ANP1_##_p     ,32  *  _1K}


#define ADDR_AND_SIZE_OF_UNIT(unit) AAS_ADDR_##unit ,AAS_UNIT_SIZE_##unit

static const UNIT_IDS_STC aasUnitsIdsInTile[] =
{
    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
                                     /*tile0baseAddr*//*sizeInBytes*/
     {PRV_CPSS_DXCH_UNIT_LPM_E           ,ADDR_AND_SIZE_OF_UNIT(LPM      )}
    ,{PRV_CPSS_DXCH_UNIT_IPVX_E          ,ADDR_AND_SIZE_OF_UNIT(IPVX     )}
    ,{PRV_CPSS_DXCH_UNIT_L2I_E           ,ADDR_AND_SIZE_OF_UNIT(L2I      )}
    ,{PRV_CPSS_DXCH_UNIT_CNC_0_E         ,ADDR_AND_SIZE_OF_UNIT(CNC0     )}
    ,{PRV_CPSS_DXCH_UNIT_CNC_1_E         ,ADDR_AND_SIZE_OF_UNIT(CNC1     )}
    ,{PRV_CPSS_DXCH_UNIT_CNC_2_E         ,ADDR_AND_SIZE_OF_UNIT(CNC2     )}
    ,{PRV_CPSS_DXCH_UNIT_CNC_3_E         ,ADDR_AND_SIZE_OF_UNIT(CNC3     )}
    ,{PRV_CPSS_DXCH_UNIT_PCL_E           ,ADDR_AND_SIZE_OF_UNIT(PCL      )}
    ,{PRV_CPSS_DXCH_UNIT_EPLR_E          ,ADDR_AND_SIZE_OF_UNIT(EPLR     )}
    ,{PRV_CPSS_DXCH_UNIT_EPCL_E          ,ADDR_AND_SIZE_OF_UNIT(EPCL     )}
    ,{PRV_CPSS_DXCH_UNIT_MPCL_E          ,ADDR_AND_SIZE_OF_UNIT(MPCL     )}
    ,{PRV_CPSS_DXCH_UNIT_PREQ_E          ,ADDR_AND_SIZE_OF_UNIT(PREQ     )}
    ,{PRV_CPSS_DXCH_UNIT_ERMRK_E         ,ADDR_AND_SIZE_OF_UNIT(ERMRK    )}
    ,{PRV_CPSS_DXCH_UNIT_IOAM_E          ,ADDR_AND_SIZE_OF_UNIT(IOAM     )}
    ,{PRV_CPSS_DXCH_UNIT_MLL_E           ,ADDR_AND_SIZE_OF_UNIT(MLL      )}
    ,{PRV_CPSS_DXCH_UNIT_IPLR_E          ,ADDR_AND_SIZE_OF_UNIT(IPLR0    )}
    ,{PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,ADDR_AND_SIZE_OF_UNIT(IPLR1    )}
    ,{PRV_CPSS_DXCH_UNIT_EQ_E            ,ADDR_AND_SIZE_OF_UNIT(EQ       )}
    ,{PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,ADDR_AND_SIZE_OF_UNIT(EGF_QAG  )}
    ,{PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,ADDR_AND_SIZE_OF_UNIT(EGF_SHT  )}
    ,{PRV_CPSS_DXCH_UNIT_IPE_E           ,ADDR_AND_SIZE_OF_UNIT(IPE      )}
    ,{PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,ADDR_AND_SIZE_OF_UNIT(EGF_EFT  )}
    ,{PRV_CPSS_DXCH_UNIT_TTI_E           ,ADDR_AND_SIZE_OF_UNIT(TTI      )}
    ,{PRV_CPSS_DXCH_UNIT_SMU_E           ,ADDR_AND_SIZE_OF_UNIT(SMU      )}

    ,{PRV_CPSS_DXCH_UNIT_EREP_E          ,ADDR_AND_SIZE_OF_UNIT(EREP     )}
    ,{PRV_CPSS_DXCH_UNIT_BMA_E           ,ADDR_AND_SIZE_OF_UNIT(BMA      )}
    ,{PRV_CPSS_DXCH_UNIT_HBU_E           ,ADDR_AND_SIZE_OF_UNIT(HBU      )}

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,ADDR_AND_SIZE_OF_UNIT(DP0_SDQ   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,ADDR_AND_SIZE_OF_UNIT(DP0_PDS   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,ADDR_AND_SIZE_OF_UNIT(DP0_QFC   )}
    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E         ,ADDR_AND_SIZE_OF_UNIT(DP0_RX    )}
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,ADDR_AND_SIZE_OF_UNIT(DP0_TXF   )}
    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E         ,ADDR_AND_SIZE_OF_UNIT(DP0_TXD   )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_0)}
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E    ,ADDR_AND_SIZE_OF_UNIT(GOP0_CTSU )}

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E ,ADDR_AND_SIZE_OF_UNIT(DP1_SDQ   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E ,ADDR_AND_SIZE_OF_UNIT(DP1_PDS   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E ,ADDR_AND_SIZE_OF_UNIT(DP1_QFC   )}
    ,{PRV_CPSS_DXCH_UNIT_RXDMA1_E         ,ADDR_AND_SIZE_OF_UNIT(DP1_RX    )}
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO1_E       ,ADDR_AND_SIZE_OF_UNIT(DP1_TXF   )}
    ,{PRV_CPSS_DXCH_UNIT_TXDMA1_E         ,ADDR_AND_SIZE_OF_UNIT(DP1_TXD   )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E      ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_0)}
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_CTSU )}

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E ,ADDR_AND_SIZE_OF_UNIT(DP2_SDQ   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E ,ADDR_AND_SIZE_OF_UNIT(DP2_PDS   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E ,ADDR_AND_SIZE_OF_UNIT(DP2_QFC   )}
    ,{PRV_CPSS_DXCH_UNIT_RXDMA2_E         ,ADDR_AND_SIZE_OF_UNIT(DP2_RX    )}
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO2_E       ,ADDR_AND_SIZE_OF_UNIT(DP2_TXF   )}
    ,{PRV_CPSS_DXCH_UNIT_TXDMA2_E         ,ADDR_AND_SIZE_OF_UNIT(DP2_TXD   )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E      ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_0)}
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_CTSU )}

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E ,ADDR_AND_SIZE_OF_UNIT(DP3_SDQ   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E ,ADDR_AND_SIZE_OF_UNIT(DP3_PDS   )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E ,ADDR_AND_SIZE_OF_UNIT(DP3_QFC   )}
    ,{PRV_CPSS_DXCH_UNIT_RXDMA3_E         ,ADDR_AND_SIZE_OF_UNIT(DP3_RX    )}
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO3_E       ,ADDR_AND_SIZE_OF_UNIT(DP3_TXF   )}
    ,{PRV_CPSS_DXCH_UNIT_TXDMA3_E         ,ADDR_AND_SIZE_OF_UNIT(DP3_TXD   )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E      ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_0)}
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_CTSU )}

    ,{PRV_CPSS_DXCH_UNIT_HA_E                         ,ADDR_AND_SIZE_OF_UNIT(HA       )}
    ,{PRV_CPSS_DXCH_UNIT_TCAM_E                       ,ADDR_AND_SIZE_OF_UNIT(TCAM     )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,ADDR_AND_SIZE_OF_UNIT(PDX_PDITX)}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                   ,ADDR_AND_SIZE_OF_UNIT(PFCC     )}
    ,{PRV_CPSS_DXCH_UNIT_FDB_E                        ,ADDR_AND_SIZE_OF_UNIT(FDB      )}
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,ADDR_AND_SIZE_OF_UNIT(PSI      )}

    ,{PRV_CPSS_DXCH_UNIT_PB_COUNTER_E                 ,ADDR_AND_SIZE_OF_UNIT(PB_COUNTER)}
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,ADDR_AND_SIZE_OF_UNIT(WA0       )}
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E     ,ADDR_AND_SIZE_OF_UNIT(WA1       )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,ADDR_AND_SIZE_OF_UNIT(GCR0      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E     ,ADDR_AND_SIZE_OF_UNIT(GCR1      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E     ,ADDR_AND_SIZE_OF_UNIT(GCR2      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E     ,ADDR_AND_SIZE_OF_UNIT(GCR3      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_4_E     ,ADDR_AND_SIZE_OF_UNIT(GCR4      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,ADDR_AND_SIZE_OF_UNIT(GPW0      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E  ,ADDR_AND_SIZE_OF_UNIT(GPW1      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_2_E  ,ADDR_AND_SIZE_OF_UNIT(GPW2      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_3_E  ,ADDR_AND_SIZE_OF_UNIT(GPW3      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_4_E  ,ADDR_AND_SIZE_OF_UNIT(GPW4      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,ADDR_AND_SIZE_OF_UNIT(GPR0      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E   ,ADDR_AND_SIZE_OF_UNIT(GPR1      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E   ,ADDR_AND_SIZE_OF_UNIT(GPR2      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E   ,ADDR_AND_SIZE_OF_UNIT(GPR3      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_4_E   ,ADDR_AND_SIZE_OF_UNIT(GPR4      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,ADDR_AND_SIZE_OF_UNIT(SMB0      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E     ,ADDR_AND_SIZE_OF_UNIT(SMB1      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E     ,ADDR_AND_SIZE_OF_UNIT(SMB2      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,ADDR_AND_SIZE_OF_UNIT(NPM0      )}
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E     ,ADDR_AND_SIZE_OF_UNIT(NPM1      )}

    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  ,ADDR_AND_SIZE_OF_UNIT(GOP0_SFF             )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E                  ,ADDR_AND_SIZE_OF_UNIT(GOP1_SFF             )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E                  ,ADDR_AND_SIZE_OF_UNIT(GOP2_SFF             )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E                  ,ADDR_AND_SIZE_OF_UNIT(GOP3_SFF             )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         ,ADDR_AND_SIZE_OF_UNIT(GOP0_PCA_PIZARB      )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E         ,ADDR_AND_SIZE_OF_UNIT(GOP1_PCA_PIZARB      )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E         ,ADDR_AND_SIZE_OF_UNIT(GOP2_PCA_PIZARB      )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E         ,ADDR_AND_SIZE_OF_UNIT(GOP3_PCA_PIZARB      )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_WRP_163E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_WRP_163I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_WRP_164E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_WRP_164I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_MACSEC_WRP_163E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_MACSEC_WRP_163I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_MACSEC_WRP_164E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_MACSEC_WRP_164I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_MACSEC_WRP_163E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_MACSEC_WRP_163I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_MACSEC_WRP_164E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_MACSEC_WRP_164I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_MACSEC_WRP_163E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_MACSEC_WRP_163I )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_MACSEC_WRP_164E )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_MACSEC_WRP_164I )}

    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_163I     )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_163E     )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_164I     )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_MACSEC_164E     )}

    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_0_E          ,ADDR_AND_SIZE_OF_UNIT(GOP0_RX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_1_E          ,ADDR_AND_SIZE_OF_UNIT(GOP1_RX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_2_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_RX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_3_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_RX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_0_E          ,ADDR_AND_SIZE_OF_UNIT(GOP0_TX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_1_E          ,ADDR_AND_SIZE_OF_UNIT(GOP1_TX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_2_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_TX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_3_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_TX_EDGE         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_0_E          ,ADDR_AND_SIZE_OF_UNIT(GOP0_RX_PACK         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_1_E          ,ADDR_AND_SIZE_OF_UNIT(GOP1_RX_PACK         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_2_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_RX_PACK         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_3_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_RX_PACK         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_0_E        ,ADDR_AND_SIZE_OF_UNIT(GOP0_TX_UPACK        )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_1_E        ,ADDR_AND_SIZE_OF_UNIT(GOP1_TX_UPACK        )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_2_E        ,ADDR_AND_SIZE_OF_UNIT(GOP2_TX_UPACK        )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_3_E        ,ADDR_AND_SIZE_OF_UNIT(GOP3_TX_UPACK        )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_0_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_ING_SHM         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_1_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_ING_SHM         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_2_E           ,ADDR_AND_SIZE_OF_UNIT(GOP2_ING_SHM         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_3_E           ,ADDR_AND_SIZE_OF_UNIT(GOP3_ING_SHM         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_0_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_SHM_EGR         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_1_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_SHM_EGR         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_2_E           ,ADDR_AND_SIZE_OF_UNIT(GOP2_SHM_EGR         )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_3_E           ,ADDR_AND_SIZE_OF_UNIT(GOP3_SHM_EGR         )}

    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_01_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_1           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_02_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_2           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_03_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_3           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_04_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_4           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_05_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_5           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_06_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_6           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_07_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP0_LMU_7           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_11_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_1           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_12_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_2           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_13_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_3           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_14_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_4           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_15_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_5           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_16_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_6           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_17_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP1_LMU_7           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_21_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_1           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_22_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_2           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_23_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_3           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_24_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_4           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_25_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_5           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_26_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_6           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_27_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP2_LMU_7           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_31_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_1           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_32_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_2           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_33_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_3           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_34_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_4           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_35_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_5           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_36_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_6           )}
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_37_E                 ,ADDR_AND_SIZE_OF_UNIT(GOP3_LMU_7           )}

    ,{PRV_CPSS_DXCH_UNIT_DFX_SERVER_E    ,ADDR_AND_SIZE_OF_UNIT(SERVER                            )}

    ,{PRV_CPSS_DXCH_UNIT_TAI_E           ,ADDR_AND_SIZE_OF_UNIT( IPLR_TAI   /*dummy*/             )}
    ,{PRV_CPSS_DXCH_UNIT_TAI1_E          ,ADDR_AND_SIZE_OF_UNIT( PHA_TAI    /*dummy*/             )}
    ,{PRV_CPSS_DXCH_UNIT_TAI2_E          ,ADDR_AND_SIZE_OF_UNIT( TTI_TAI_0  /*dummy*/             )}

    ,{PRV_CPSS_DXCH_UNIT_CNM_RFU_E       ,ADDR_AND_SIZE_OF_UNIT(CNM_CnM_RFU                       )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_MPP_RFU_E   ,ADDR_AND_SIZE_OF_UNIT(CNM_MPP_RFU                       )}
    /* SMI is in the CnM section */
    ,{PRV_CPSS_DXCH_UNIT_SMI_0_E         ,ADDR_AND_SIZE_OF_UNIT(CNM_SMI_0                         )}
    ,{PRV_CPSS_DXCH_UNIT_SMI_1_E         ,ADDR_AND_SIZE_OF_UNIT(CNM_SMI_1                         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_AAC_E       ,ADDR_AND_SIZE_OF_UNIT(CNM_AAC_0                         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_ADDR_DECODER_E,   ADDR_AND_SIZE_OF_UNIT(CNM_Address_Decoder          )}
                /*  the 11 mini-units of address decoders
                    AAS_ADDR_CNM_Address_Decoder_iNIC
                    AAS_ADDR_CNM_Address_Decoder_PCIe
                    AAS_ADDR_CNM_Address_Decoder_iunit
                    AAS_ADDR_CNM_Address_Decoder_GDMA0
                    AAS_ADDR_CNM_Address_Decoder_GDMA1
                    AAS_ADDR_CNM_Address_Decoder_GDMA2
                    AAS_ADDR_CNM_Address_Decoder_GDMA3
                    AAS_ADDR_CNM_Address_Decoder_EHSM_DMA
                    AAS_ADDR_CNM_Address_Decoder_AAC_0
                    AAS_ADDR_CNM_Address_Decoder_AAC_1
                    AAS_ADDR_CNM_Address_Decoder_AMB
                */
    /* LED */
    ,{PRV_CPSS_DXCH_UNIT_LED_0_E          ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_LED_0     )}
    ,{PRV_CPSS_DXCH_UNIT_LED_1_E          ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_LED_0     )}
    ,{PRV_CPSS_DXCH_UNIT_LED_2_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_LED_0     )}
    ,{PRV_CPSS_DXCH_UNIT_LED_3_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_LED_0     )}

    /* only in GOP 2 */
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_CPU_MAC100     )}
    ,{PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_CPU_PCS100     )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_CPU_ANP1       )}
    ,{PRV_CPSS_DXCH_UNIT_SDW_CPU_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_CPU_SDW_56G_X1 )}

    ,CPU_100G_P4( 0/*global instance*/,0/*GOP*/,0/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 1/*global instance*/,0/*GOP*/,1/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 2/*global instance*/,0/*GOP*/,2/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 3/*global instance*/,0/*GOP*/,3/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 4/*global instance*/,1/*GOP*/,0/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 5/*global instance*/,1/*GOP*/,1/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 6/*global instance*/,1/*GOP*/,2/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 7/*global instance*/,1/*GOP*/,3/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 8/*global instance*/,2/*GOP*/,0/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4( 9/*global instance*/,2/*GOP*/,1/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(10/*global instance*/,2/*GOP*/,2/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(11/*global instance*/,2/*GOP*/,3/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(12/*global instance*/,3/*GOP*/,0/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(13/*global instance*/,3/*GOP*/,1/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(14/*global instance*/,3/*GOP*/,2/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */
    ,CPU_100G_P4(15/*global instance*/,3/*GOP*/,3/*local instance*/) /* macro for MAC,PCS,ANP for a 'CPU' port */

    ,{PRV_CPSS_DXCH_UNIT_SDW56G_CPU_GOP_0_E ,ADDR_AND_SIZE_OF_UNIT(GOP0_4P_SDW_56G_X4  )}
    ,{PRV_CPSS_DXCH_UNIT_SDW56G_CPU_GOP_1_E ,ADDR_AND_SIZE_OF_UNIT(GOP1_4P_SDW_56G_X4  )}
    ,{PRV_CPSS_DXCH_UNIT_SDW56G_CPU_GOP_2_E ,ADDR_AND_SIZE_OF_UNIT(GOP2_4P_SDW_56G_X4  )}
    ,{PRV_CPSS_DXCH_UNIT_SDW56G_CPU_GOP_3_E ,ADDR_AND_SIZE_OF_UNIT(GOP3_4P_SDW_56G_X4  )}

    /* MAC */
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_0_E     , ADDR_AND_SIZE_OF_UNIT(GOP0_16P_MAC800_0    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_1_E     , ADDR_AND_SIZE_OF_UNIT(GOP0_16P_MAC800_1    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_2_E     , ADDR_AND_SIZE_OF_UNIT(GOP1_16P_MAC800_0    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_3_E     , ADDR_AND_SIZE_OF_UNIT(GOP1_16P_MAC800_1    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_4_E     , ADDR_AND_SIZE_OF_UNIT(GOP2_16P_MAC800_0    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_5_E     , ADDR_AND_SIZE_OF_UNIT(GOP2_16P_MAC800_1    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_6_E     , ADDR_AND_SIZE_OF_UNIT(GOP3_16P_MAC800_0    )}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_7_E     , ADDR_AND_SIZE_OF_UNIT(GOP3_16P_MAC800_1    )}

    /* PCS */
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_0_E     , ADDR_AND_SIZE_OF_UNIT(GOP0_16P_PCS800      )}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_1_E     , ADDR_AND_SIZE_OF_UNIT(GOP1_16P_PCS800      )}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_2_E     , ADDR_AND_SIZE_OF_UNIT(GOP2_16P_PCS800      )}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_3_E     , ADDR_AND_SIZE_OF_UNIT(GOP3_16P_PCS800      )}

    /* mif */
    ,{PRV_CPSS_DXCH_UNIT_MIF_800G_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_MIF_0        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_800G_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_MIF_0        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_800G_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_MIF_0        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_800G_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_MIF_0        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_MIF_1        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_MIF_1        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_MIF_1        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_MIF_1        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_4P_0_E       ,ADDR_AND_SIZE_OF_UNIT(GOP0_4P_MIF           )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_4P_1_E       ,ADDR_AND_SIZE_OF_UNIT(GOP1_4P_MIF           )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_4P_2_E       ,ADDR_AND_SIZE_OF_UNIT(GOP2_4P_MIF           )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_4P_3_E       ,ADDR_AND_SIZE_OF_UNIT(GOP3_4P_MIF           )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E      ,ADDR_AND_SIZE_OF_UNIT(GOP1_CPU_MIF          )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_RP_0_E       ,ADDR_AND_SIZE_OF_UNIT(GOP2_80P_MIF_0        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_RP_1_E       ,ADDR_AND_SIZE_OF_UNIT(GOP2_80P_MIF_1        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_RP_2_E       ,ADDR_AND_SIZE_OF_UNIT(GOP2_80P_MIF_2        )}
    ,{PRV_CPSS_DXCH_UNIT_MIF_RP_3_E       ,ADDR_AND_SIZE_OF_UNIT(GOP2_80P_MIF_3        )}
    /*ANP*/
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_0_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_ANP8_0       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_1_E     ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_ANP8_1       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_2_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_ANP8_0       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_3_E     ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_ANP8_1       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_4_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_ANP8_0       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_5_E     ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_ANP8_1       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_6_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_ANP8_0       )}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_7_E     ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_ANP8_1       )}

    ,{PRV_CPSS_DXCH_UNIT_SDW0_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_SDW_112G_X4_0)}
    ,{PRV_CPSS_DXCH_UNIT_SDW1_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_SDW_112G_X4_1)}
    ,{PRV_CPSS_DXCH_UNIT_SDW2_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_SDW_56G_X4_0 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW3_E           ,ADDR_AND_SIZE_OF_UNIT(GOP0_16P_SDW_56G_X4_1 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW4_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_SDW_112G_X4_0)}
    ,{PRV_CPSS_DXCH_UNIT_SDW5_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_SDW_112G_X4_1)}
    ,{PRV_CPSS_DXCH_UNIT_SDW6_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_SDW_56G_X4_0 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW7_E           ,ADDR_AND_SIZE_OF_UNIT(GOP1_16P_SDW_56G_X4_1 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW8_E           ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_SDW_112G_X4_0)}
    ,{PRV_CPSS_DXCH_UNIT_SDW9_E           ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_SDW_112G_X4_1)}
    ,{PRV_CPSS_DXCH_UNIT_SDW10_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_SDW_56G_X4_0 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW11_E          ,ADDR_AND_SIZE_OF_UNIT(GOP2_16P_SDW_56G_X4_1 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW12_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_SDW_112G_X4_0)}
    ,{PRV_CPSS_DXCH_UNIT_SDW13_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_SDW_112G_X4_1)}
    ,{PRV_CPSS_DXCH_UNIT_SDW14_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_SDW_56G_X4_0 )}
    ,{PRV_CPSS_DXCH_UNIT_SDW15_E          ,ADDR_AND_SIZE_OF_UNIT(GOP3_16P_SDW_56G_X4_1 )}

    ,{PRV_CPSS_DXCH_UNIT_PHA_E           ,ADDR_AND_SIZE_OF_UNIT(PHA                    )}
    ,{PRV_CPSS_DXCH_UNIT_SHM_E           ,ADDR_AND_SIZE_OF_UNIT(SHM0                   )}
    ,{PRV_CPSS_DXCH_UNIT_IA_E            ,ADDR_AND_SIZE_OF_UNIT(IA                     )}
    ,{PRV_CPSS_DXCH_UNIT_EOAM_E          ,ADDR_AND_SIZE_OF_UNIT(EOAM                   )}
    ,{PRV_CPSS_DXCH_UNIT_PPU_E           ,ADDR_AND_SIZE_OF_UNIT(PPU1                   )}
    ,{PRV_CPSS_DXCH_UNIT_PPU_FOR_IPE_E   ,ADDR_AND_SIZE_OF_UNIT(PPU2                   )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_RUNIT_E     ,ADDR_AND_SIZE_OF_UNIT(CNM_RUNIT              )}
    ,{PRV_CPSS_DXCH_UNIT_EMX_E           ,ADDR_AND_SIZE_OF_UNIT(EMX                    )}
    ,{PRV_CPSS_DXCH_UNIT_TTI_LU_E        ,ADDR_AND_SIZE_OF_UNIT(TTI_LU                 )}
    ,{PRV_CPSS_DXCH_UNIT_EM_E            ,ADDR_AND_SIZE_OF_UNIT(EM0                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_1_E          ,ADDR_AND_SIZE_OF_UNIT(EM1                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_2_E          ,ADDR_AND_SIZE_OF_UNIT(EM2                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_3_E          ,ADDR_AND_SIZE_OF_UNIT(EM3                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_4_E          ,ADDR_AND_SIZE_OF_UNIT(EM4                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_5_E          ,ADDR_AND_SIZE_OF_UNIT(EM5                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_6_E          ,ADDR_AND_SIZE_OF_UNIT(EM6                    )}
    ,{PRV_CPSS_DXCH_UNIT_EM_7_E          ,ADDR_AND_SIZE_OF_UNIT(EM7                    )}
    ,{PRV_CPSS_DXCH_UNIT_REDUCED_EM_E    ,ADDR_AND_SIZE_OF_UNIT(EM_ILM0                )}
#ifdef GM_USED
    /*************************************************************/
    /* GM Simulation uses dummy MIB memory space (need to be     */
    /* synch with file GmProjectFunctions.cpp of GM)             */
    /*************************************************************/
    ,{PRV_CPSS_DXCH_UNIT_MIB_E,ADDR_AND_SIZE_OF_UNIT(MIB                               )}
    ,{PRV_CPSS_DXCH_UNIT_MG_E            ,ADDR_AND_SIZE_OF_UNIT(MG0                    )}
    ,{PRV_CPSS_DXCH_UNIT_MG_0_1_E        ,ADDR_AND_SIZE_OF_UNIT(MG1                    )}
#elif defined ASIC_SIMULATION
    /* A unit for WM so CPSS can sent IPC commands to WM :  needed in Sip7 as MG unit not exists */
    ,{PRV_CPSS_DXCH_UNIT__IPC_FOR_WM_ONLY__E,ADDR_AND_SIZE_OF_UNIT(WM_IPC              )}
#endif

    ,{PRV_CPSS_DXCH_UNIT_CNM_GDMA_DISPATCHER_E , ADDR_AND_SIZE_OF_UNIT(CNM_GDMA_Dispatcher)}
    ,{PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E          , ADDR_AND_SIZE_OF_UNIT(CNM_GDMA_0         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_GDMA_1_E          , ADDR_AND_SIZE_OF_UNIT(CNM_GDMA_1         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_GDMA_2_E          , ADDR_AND_SIZE_OF_UNIT(CNM_GDMA_2         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_GDMA_3_E          , ADDR_AND_SIZE_OF_UNIT(CNM_GDMA_3         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_INTERRUPTS_E      , ADDR_AND_SIZE_OF_UNIT(CNM_CNM_Interrupt  )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_XSMI_0_E          , ADDR_AND_SIZE_OF_UNIT(CNM_XSMI_0         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_XSMI_1_E          , ADDR_AND_SIZE_OF_UNIT(CNM_XSMI_1         )}
    ,{PRV_CPSS_DXCH_UNIT_CNM_XSMI_2_E          , ADDR_AND_SIZE_OF_UNIT(CNM_XSMI_2         )}

     /* must be last */
    ,{PRV_CPSS_DXCH_UNIT_LAST_E, 0,0 }
};

/*
    structure to help convert Global to Local for DMA and for GOP ports
*/
typedef struct{
    GT_U32    dpIndex;
    GT_U32    localPortInDp;
    GT_U32    globalMac;
}SPECIAL_DMA_PORTS_STC;

/* Field: localPortInDp = 20 for loopback*/
#define LOOPBACK_PORT_DMA_CHANNEL       AAS_PORTS_PER_DP_CNS

/* Field: globalMac - 2 NotApplicable values different for Loopback and GDMA/CPU */
#define GLOBAL_MAC_SPECIAL_GDMA          GT_NA
#define GLOBAL_MAC_SPECIAL_LOOPBACK     (GT_NA-1)

/* NOTE : the device hold no 'CPU reduced ports' that are muxed with the GDMAs */
static const SPECIAL_DMA_PORTS_STC aas_SpecialDma_ports[]= {
   /*80*/{ 0/*dpIndex*/, AAS_PORTS_PER_DP_CNS+1 /*21*//*localPortInDp*/, MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS/*145*/},/*CPU network port*/
   /*81*/{ 0/*dpIndex*/, AAS_PORTS_PER_DP_CNS+2 /*22*//*localPortInDp*/,GLOBAL_MAC_SPECIAL_GDMA},/*CPU GDMA 0*/
   /*82*/{ 1/*dpIndex*/, AAS_PORTS_PER_DP_CNS+2 /*22*//*localPortInDp*/,GLOBAL_MAC_SPECIAL_GDMA},/*CPU GDMA 1*/
   /*83*/{ 2/*dpIndex*/, AAS_PORTS_PER_DP_CNS+2 /*22*//*localPortInDp*/,GLOBAL_MAC_SPECIAL_GDMA},/*CPU GDMA 2*/

   /* Internal Loopback Port: per-DP, all are using same Loopback dedicated channel */
   /*84*/{ 0/*dpIndex*/, LOOPBACK_PORT_DMA_CHANNEL, GLOBAL_MAC_SPECIAL_LOOPBACK},
   /*85*/{ 1/*dpIndex*/, LOOPBACK_PORT_DMA_CHANNEL, GLOBAL_MAC_SPECIAL_LOOPBACK},
   /*86*/{ 2/*dpIndex*/, LOOPBACK_PORT_DMA_CHANNEL, GLOBAL_MAC_SPECIAL_LOOPBACK},
   /*87*/{ 3/*dpIndex*/, LOOPBACK_PORT_DMA_CHANNEL, GLOBAL_MAC_SPECIAL_LOOPBACK},
   };
static const GT_U32 num_ports_aas_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(aas_SpecialDma_ports);

/* NOTE : the device hold no 'CPU reduced ports' that are muxed with the SDMAs */
static const SPECIAL_DMA_PORTS_STC aas_2_tiles_SpecialDma_ports[]= {
   /*160*/{ 0/*global dpIndex*/, AAS_PORTS_PER_DP_CNS+1/*21*//*localPortInDp*/,289/*globalMac*/},/*CPU network port*/
   /*161*/{ 4/*global dpIndex*/, AAS_PORTS_PER_DP_CNS+1/*21*//*localPortInDp*/,293/*globalMac*/},/*CPU network port*/
   /*162*/{ 0/*global dpIndex*/, AAS_PORTS_PER_DP_CNS+2/*22*//*localPortInDp*/,GT_NA/*globalMac*/},/*CPU SDMA MG 0*/
   };
static const GT_U32 num_ports_aas_2_tiles_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(aas_2_tiles_SpecialDma_ports);

static const struct{
    GT_U32  dpIndex;
    GT_U32  gdmaUnitId_bmp_TO_CPU;
    GT_U32  gdmaUnitId_FROM_CPU;
}aas_convert_dpIndex_to_gdma_unit_id[] =
{
/*dpIndex*//*gdmaUnitId_bmp_TO_CPU*//*gdmaUnitId_FROM_CPU*/
/* for single tile */
     {0,                 0x0,                 0}
    ,{1,         BIT_1|BIT_2,                 2}
    ,{2,                 0x0,                 3}
/* for 2 tiles */
    ,{4,                 0x0,                 4}
    ,{5,         BIT_5|BIT_6,                 6}
    ,{6,                 0x0,                 7}
    /*must be last*/
    ,{GT_NA,GT_NA,GT_NA}
};

/* num of queues for TO_CPU that the device have */
static const GT_U32 aas_gdmaNumQueues_TO_CPU[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS]=
{0,32,32,0};
/* num of queues for FROM_CPU that the device have */
static const GT_U32 aas_gdmaNumQueues_FROM_CPU[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS]=
{22,0,32,32};

/*array with the base addresses of units of address decoder */
static const GT_U32 aasAddressDecoderArr[] = {
     AAS_ADDR_CNM_Address_Decoder_iNIC      /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_iNIC_E    */
    ,AAS_ADDR_CNM_Address_Decoder_PCIe      /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_PCIe_E    */
    ,AAS_ADDR_CNM_Address_Decoder_iunit     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_iunit_E   */
    ,AAS_ADDR_CNM_Address_Decoder_GDMA0     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA0_E   */
    ,AAS_ADDR_CNM_Address_Decoder_GDMA1     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA1_E   */
    ,AAS_ADDR_CNM_Address_Decoder_GDMA2     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA2_E   */
    ,AAS_ADDR_CNM_Address_Decoder_GDMA3     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA3_E   */
    ,AAS_ADDR_CNM_Address_Decoder_EHSM_DMA  /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_EHSM_DMA_E*/
    ,AAS_ADDR_CNM_Address_Decoder_AAC_0     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AAC_0_E   */
    ,AAS_ADDR_CNM_Address_Decoder_AAC_1     /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AAC_1_E   */
    ,AAS_ADDR_CNM_Address_Decoder_AMB       /* index is SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_AMB_E     */
};


/**
* @internal prvCpssAasUnitIdSizeInByteGet function
* @endinternal
*
* @brief   To get the size of the unit in bytes
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] prvUnitId                - Id of DxCh unit(one of the PRV_CPSS_DXCH_UNIT_ENT)
*
* @param[out] unitIdSize               - size of the unit in bytes
*/
static GT_STATUS prvCpssAasUnitIdSizeInByteGet
(
    IN  GT_U8                      devNum,
    IN  PRV_CPSS_DXCH_UNIT_ENT     prvUnitId,
    OUT GT_U32                    *unitIdSizePtr
)
{
    const UNIT_IDS_STC   *unitIdInPipePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr;
    GT_U32 ii;

    for(ii = 0 ; unitIdInPipePtr->tile0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,unitIdInPipePtr++)
    {
        if(unitIdInPipePtr->tile0Id == prvUnitId)
        {
            *unitIdSizePtr = unitIdInPipePtr->sizeInBytes;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "unit [%d] not supported in AAS",
        prvUnitId);
}

typedef enum{
    MAC_CAPABILITY_50G_100G_E ,/* 50G ,100G (also lower than  50G)  */
    MAC_CAPABILITY_400G_E,/* 400G (also lower than 400G)  */
    MAC_CAPABILITY_200G_E,/* 200G (also lower than 200G)  */
    MAC_CAPABILITY_1G_E ,/* 1G ,2.5G , 5G */
    MAC_CAPABILITY_CPU_E,/* cpu port*/

    MAC_CAPABILITY_800G_E,/* 800G   */

    MAC_CAPABILITY_UNKNOWN_E /* error */
}MAC_CAPABILITY_ENT;

typedef struct{
    MAC_CAPABILITY_ENT macCapability;
    GT_U32             ciderUnit;
    GT_U32             ciderIndexInUnit;
    GT_U32             ciderSubUnit;

    GT_U32             dpIndex;
    GT_U32             localPortInDp;

    GT_U32             tileId;
}MAC_INFO_STC;

static const MAC_CAPABILITY_ENT localPortsCapability_group_0_1[8] =
{
    MAC_CAPABILITY_400G_E ,     MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E,
    MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E
};
static const MAC_CAPABILITY_ENT localPortsCapability_group_2[4] =
{
    MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E , MAC_CAPABILITY_50G_100G_E
};
/*
static const MAC_CAPABILITY_ENT localPortsCapability_group_3[2] =
{
    MAC_CAPABILITY_800G_E , MAC_CAPABILITY_400G_E
};
*/
static GT_STATUS   macInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN HWS_UNIT_BASE_ADDR_TYPE_ENT unitId,
    OUT MAC_INFO_STC               *macInfoPtr,
    IN GT_BOOL                     isErrorToLog
)
{
    macInfoPtr->ciderSubUnit = 0;

    if((PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts +
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts) <= portMacNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is over the max value",
            portMacNum);
    }

    if((portMacNum >= PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts) ||
       ((portMacNum % 20) > 15))
    {

        macInfoPtr->ciderUnit        = (portMacNum /  AAS_PORTS_PER_DP_CNS) * 4 + (portMacNum %  AAS_PORTS_PER_DP_CNS - 16);
        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                if(portMacNum == aas_SpecialDma_ports[0].globalMac)
                {
                    macInfoPtr->ciderUnit = 32;
                }
                break;
            case 2:
                if(portMacNum == aas_2_tiles_SpecialDma_ports[0].globalMac)
                {
                    macInfoPtr->ciderUnit = 32;
                }
                else if(portMacNum == aas_2_tiles_SpecialDma_ports[1].globalMac)
                {
                    macInfoPtr->ciderUnit = 33;
                }
                break;
            default:
                break;
        }

        macInfoPtr->macCapability    = MAC_CAPABILITY_CPU_E;
        macInfoPtr->ciderIndexInUnit = 0;

        macInfoPtr->dpIndex          = macInfoPtr->ciderUnit;
        macInfoPtr->localPortInDp    = AAS_PORTS_PER_DP_CNS;

        if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E)
        {
            macInfoPtr->ciderUnit        = 8;
        }

        macInfoPtr->tileId           = 0;

        return GT_OK;
    }

    /*local DP within the tile */
    macInfoPtr->dpIndex       = (portMacNum / AAS_PORTS_PER_DP_CNS) % NUM_OF_DP_UNITS;
    macInfoPtr->localPortInDp = portMacNum % AAS_PORTS_PER_DP_CNS;
    macInfoPtr->tileId        = portMacNum / SERDES_NUM_NETWORK_PORTS;


    /* there are 4 'MAC' units in the GOP :
       1. octal MAC (8 ports) : 2x 200G + 2x 100G + 4x 1-50G
       2. octal MAC (8 ports) : 2x 200G + 2x 100G + 4x 1-50G
       3. single MAC(4 ports) : 4x 1-50G

       --> not know how next represented (muxed with others)
       --> as the DP have only 20+1(CPU) channels that need to fit : 8+8+4+2 + 1(CPU) =
       --> so we have 22 MACs into 20 channels.

       1. dual MAC : 800G / 2x 400G
       2. single CPU MAC (100G MAC)
    */

    /*
        MIF units:
        MIF for ports 0..7  in DP (DP0..3)
        MIF for ports 8..19 in DP (DP0..3)
        MIF for CPU port (single one in DP 0)
    */

    macInfoPtr->ciderUnit     = (macInfoPtr->dpIndex * 2) + (macInfoPtr->localPortInDp / 8);
    macInfoPtr->macCapability = MAC_CAPABILITY_UNKNOWN_E;

    switch (unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E:
            macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp % 8;

            if(macInfoPtr->localPortInDp < 16)/* one of the 2 octal MACs */
            {
                macInfoPtr->macCapability    =
                    localPortsCapability_group_0_1[macInfoPtr->localPortInDp  % 8];/*0..15*/
            }
            else/* the quad MAC */
            {
                macInfoPtr->macCapability    =
                    localPortsCapability_group_2[macInfoPtr->localPortInDp - 16];
            }

            if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E)
            {
                if(macInfoPtr->localPortInDp < 8)
                {
                    macInfoPtr->ciderUnit        = macInfoPtr->dpIndex * 2;
                    macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp;/*0.. 7*/
                }
                else
                {
                    macInfoPtr->ciderUnit        = macInfoPtr->dpIndex * 2 + 1;
                    macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp - 8;/*0..11*/
                }
            }

            break;

        case HWS_UNIT_BASE_ADDR_TYPE_MIB_E:
            if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "HWS_UNIT_BASE_ADDR_TYPE_MIB_E is only for GM");
            }
            macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp % 8;

            if(macInfoPtr->localPortInDp < 16)/* one of the 2 octal MACs */
            {
                macInfoPtr->macCapability    =
                    localPortsCapability_group_0_1[macInfoPtr->localPortInDp  % 8];/*0..15*/
            }
            else/* the quad MAC */
            {
                macInfoPtr->macCapability    =
                    localPortsCapability_group_2[macInfoPtr->localPortInDp - 16];
            }
            break;

        default:
            if(isErrorToLog == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Unit[%d] is not supported", unitId);
            }
            else
            {
                return /* not error to the LOG */ GT_NOT_SUPPORTED;
            }
    }

    return GT_OK;
}

/* offsets within the : Aas/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x0000A000
#define MAC_200G_400G_OFFSET      0x00009000
#define PORT0_100G_OFFSET         0x00001000

#define PCS_400G_OFFSET           0x00000000
#define PCS_200G_OFFSET           0x00001000
#define PCS_PORT0_100G_OFFSET     0x00002000
#define PCS_PORT1_50G_OFFSET      (PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET)
#define PCS_RS_FEC_OFFSET         0x0000a000
#define PCS_LPCS_OFFSET           0x0000b000
#define PCS_RS_FEC_STATISTICS_OFFSET 0x0000c000

#define ANP_STEP_UNIT_OFFSET      0x00200000

#define CPU_MAC_EXT_BASE_OFFSET       0x00000000
#define CPU_MAC_PORT_OFFSET           0x00003000
#define CPU_MAC_MIB_OFFSET            0x00001000


/**
* @internal aasUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS to call to cpss to use for Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] unitId                   - the unit : MIB/SERDES/GOP
* @param[in] portNum                  - the global port num (MAC number)
*
* @return - The address of the port in the unit
*/
static GT_U32 /*GT_UREG_DATA*/   aasUnitBaseAddrCalc(
    GT_U8                                   devNum,
    GT_U32/*HWS_UNIT_BASE_ADDR_TYPE_ENT*/   unitId,
    GT_U32/*GT_UOPT*/                       portNum
)
{
    GT_STATUS   rc;
    GT_U32 baseAddr;
    GT_U32 relativeAddr;
    GT_U32 dpIndex;
    MAC_INFO_STC    macInfo;
    MAC_CAPABILITY_ENT macCapability;
    GT_U32  localSDW;
    GT_U32  tileId;
    PRV_CPSS_DXCH_UNIT_ENT  dxChUnitId;

    if(unitId == HWS_UNIT_BASE_ADDR_TYPE_SERDES_E)
    {
        /* the 'portNum' is actually 'SERDES number' : 0..79 , 80 */
        if(portNum >= PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev)
        {
            return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }

        localSDW = portNum % SERDES_NUM_NETWORK_PORTS;
        tileId   = portNum / SERDES_NUM_NETWORK_PORTS;

        if(portNum >= (SERDES_NUM_NETWORK_PORTS*(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)))
        {
            /*CPU Serdes (56G unit)*/
            dxChUnitId =  PRV_CPSS_DXCH_UNIT_SDW_CPU_0_E;
            tileId     = portNum-(SERDES_NUM_NETWORK_PORTS*(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles));
        }
        else
        if((localSDW / 4) >= 13)
        {   /*PRV_CPSS_DXCH_UNIT_SDW13_E not continues with PRV_CPSS_DXCH_UNIT_SDW0_E*/
            dxChUnitId =  (PRV_CPSS_DXCH_UNIT_SDW13_E - 13) + (localSDW / 4);
        }
        else
        {
            dxChUnitId =  PRV_CPSS_DXCH_UNIT_SDW0_E + (localSDW / 4);
        }

        baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, dxChUnitId,NULL);
        baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

        return baseAddr;
    }

    rc =  macInfoGet(devNum,portNum,unitId,&macInfo,GT_FALSE);
    if(rc != GT_OK)
    {
        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    macCapability  = macInfo.macCapability;
    dpIndex        = macInfo.dpIndex;/* local DP in tile */
    tileId         = macInfo.tileId;

    switch(unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
            /* note : the 32 local ports share the same 'base' address !!! */
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E + dpIndex/*0..3*/), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:   /* 0..255 */
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            dxChUnitId  = PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit;

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, dxChUnitId, NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:   /* 0..255 */
            if(MAC_CAPABILITY_400G_E != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E, portNum);

            return (baseAddr + MAC_200G_400G_OFFSET);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = 0;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
            baseAddr = aasUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_PORT_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
            baseAddr = aasUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_EXT_BASE_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
            baseAddr = aasUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_MIB_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = 0;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = 0x2000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = MAC_MIB_OFFSET;/* shared to 8 ports */

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIB_E:
            if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            /********************************/
            /* dummy for GM device only !!! */
            /********************************/
            baseAddr     = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MIB_E, NULL);
            relativeAddr = GM_STEP_BETWEEN_DPS       * dpIndex +
                           GM_STEP_BETWEEN_MIB_PORTS * macInfo.localPortInDp;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
            if(MAC_CAPABILITY_CPU_E == macCapability)
            {
                baseAddr = aasUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E , portNum);
                if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                {
                    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                }

                return baseAddr;
            }

            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = MAC_EXT_BASE_OFFSET;/* shared to 8 ports */
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E */
            relativeAddr -= PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;


        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E    :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E   :
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(macInfo.ciderUnit < 4)
            {
                dxChUnitId  = PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit;
            }
            else
            {
                /*PRV_CPSS_DXCH_UNIT_MAC_400G_4_E not continues with PRV_CPSS_DXCH_UNIT_MAC_400G_0_E*/
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, dxChUnitId, NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;

            relativeAddr = PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E   :
            if(MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = PCS_200G_OFFSET;
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E */
            relativeAddr -= PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E   :
            if(MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = PCS_400G_OFFSET;
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E */
            relativeAddr -= PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = PCS_LPCS_OFFSET;
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E */
            relativeAddr -= PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = PCS_RS_FEC_OFFSET;
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E */
            relativeAddr -= PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E, portNum);
            /*already calculated tile offset*/

            relativeAddr = PCS_RS_FEC_STATISTICS_OFFSET;
            /* compensation for the logic in HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E */
            relativeAddr -= PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E   :
            switch (macInfo.ciderUnit)
            {
                case  0: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_800G_0_E; break;
                case  1: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_400G_0_E; break;
                case  2: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_4P_0_E;   break;
                case  3: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_800G_1_E; break;
                case  4: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_400G_1_E; break;
                case  5: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_4P_1_E;   break;
                case  6: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_800G_2_E; break;
                case  7: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_400G_2_E; break;
                case  8: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_4P_2_E;   break;
                case  9: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_800G_3_E; break;
                case 10: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_400G_3_E; break;
                case 11: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_4P_3_E;   break;
                case 12: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E;  break;
                case 13: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_RP_0_E;   break;
                case 14: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_RP_1_E;   break;
                case 15: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_RP_2_E;   break;
                case 16: dxChUnitId  = PRV_CPSS_DXCH_UNIT_MIF_RP_3_E;   break;
                default: return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, dxChUnitId, NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E   :
            if(macInfo.ciderUnit < 4)
            {
                dxChUnitId  = PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit;
            }
            else
            {
                /*PRV_CPSS_DXCH_UNIT_ANP_400G_4_E not continues with PRV_CPSS_DXCH_UNIT_ANP_400G_0_E*/
                dxChUnitId  = PRV_CPSS_DXCH_UNIT_ANP_400G_4_E - 4 + macInfo.ciderUnit;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E   :
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E, portNum);
            /*already calculated tile offset*/
            return (baseAddr +  0x4000 + macInfo.ciderIndexInUnit * 0x1000);

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + macInfo.ciderUnit), NULL);
            baseAddr += PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset * tileId;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E   :
            baseAddr   = aasUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E, portNum);
            /*already calculated tile offset*/
            return (baseAddr + 0x4000);

        default:
            break;
    }
    /* indicate to skip this unit */
    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
}

/**
* @internal prvDxChHwRegAddrAasDbInit function
* @endinternal
*
* @brief   init the base address manager of the Aas device.
*         prvDxChAasUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrAasDbInit(GT_VOID)
{
    const UNIT_IDS_STC      *currEntryPtr;
    GT_U32                  globalIndex = 0;/* index into prvDxChAasUnitsIdUnitBaseAddrArr */
    GT_U32                  ii;
    GT_U32                  errorCase = 0;
    GT_U32                  maxUnits = AAS_MAX_UNITS;
    GT_U32                  tileId , tileOffset , tileUnitOffset;

    /* bind HWS with function that calc base addresses of units */
    /* NOTE: bind was already done by 'Falcon'
       hwsFalconUnitBaseAddrCalcBind(aasUnitBaseAddrCalc); */

    for(tileId = 0 ; tileId < 2 ; tileId++)
    {
        currEntryPtr = &aasUnitsIdsInTile[0];

        tileOffset     = tileId * AAS_TILE_OFFSET_CNS;
        tileUnitOffset = tileId * PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS;
        /* add the unit in all tiles */
        for(ii = 0 ; currEntryPtr->tile0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
        {
            if(globalIndex >= maxUnits)
            {
                errorCase = 1;
                goto notEnoughUnits_lbl;
            }

            PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChAasUnitsIdUnitBaseAddrArr)[globalIndex].unitId =
                currEntryPtr->tile0Id + tileUnitOffset;
            PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChAasUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr =
                currEntryPtr->tile0baseAddr + tileOffset;
            globalIndex++;
        }
    }

    if(globalIndex >= maxUnits)
    {
        errorCase = 2;
        goto notEnoughUnits_lbl;
    }

    /* set the 'last index as invalid' */
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChAasUnitsIdUnitBaseAddrArr)[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChAasUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;

    /* avoid warning if 'CPSS_LOG_ENABLE' not defined
      (warning: variable 'errorCase' set but not used [-Wunused-but-set-variable])
    */
    if(errorCase == 0)
    {
        errorCase++;
    }

    return GT_OK;

notEnoughUnits_lbl:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :AAS_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,maxUnits , globalIndex+1);
}

/**
* @internal prvCpssDxChAasHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
static PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChAasHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    const UNIT_IDS_STC   *currEntryPtr;
    GT_U32  ii;
    GT_U32  tileId;
    GT_U32  tileOffset;
    GT_U32  regAddr_inTile0;

    tileId = regAddr / AAS_TILE_OFFSET_CNS;
    if(tileId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        /* dont error to the log as currently the prvCpssDxChAas_directInfoArr
            hold addresses of tile 1 even for single tile device.

            and the WM 'allow' to access this address as the aas_units[] support the
            addresses of the 2 tiles even for single tile
         */
        return /* not error to the log */ PRV_CPSS_DXCH_UNIT_LAST_E;

        /* bad address */
        /*CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_DXCH_UNIT_LAST_E,
            "address [0x%8.8x] can't convert to tileId",
            regAddr);*/
    }

    tileOffset      = tileId * AAS_TILE_OFFSET_CNS;
    regAddr_inTile0 = regAddr - tileOffset;

    currEntryPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr;

    /****************************************/
    /* search for addrInTile0 in the ranges */
    /****************************************/
    for(ii = 0 ; currEntryPtr->tile0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        /* add the unit in tile 0 */
        if(regAddr_inTile0 >= currEntryPtr->tile0baseAddr  &&
           regAddr_inTile0 <  (currEntryPtr->tile0baseAddr + currEntryPtr->sizeInBytes))
        {
            /* found the proper range */
            return tileOffset + currEntryPtr->tile0Id;
        }
    }

    /* not found ! */
    return PRV_CPSS_DXCH_UNIT_LAST_E;
}

/* number of rx/tx DMA and txfifo 'per DP unit' */
#define AAS_NUM_PORTS_DMA   (AAS_PORTS_PER_DP_CNS + 3)/*23*/

/*check if register address is per port in RxDMA unit */
static GT_BOOL  aas_mustNotDuplicate_rxdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {

         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.cutThrough.channelCTConfig                ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelToLocalDevSourcePort ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelGeneralConfigs       ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA     [NON_FIRST_UNIT_INDEX_CNS].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg), AAS_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = RXDMA_DIFF_0_TO_1; /* offset between units */

    /* next array hold addresses in DP[0] that are global , and can't be used with addrOffset_NON_FIRST_UNIT_INDEX_CNS !!! */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalAddrArr[]=
    {
        /* next registers are accessed 'per DP index' explicitly (DP[0]) */
         {REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(sip6_rxDMA[0].debug.rxChannelIngressDropCounter)}

        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds0),  1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds1),  1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* check global */
    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    /* check per port */
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxDMA unit */
static GT_BOOL  aas_mustNotDuplicate_txdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile                 ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.channelReset                 ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase                 ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.interGapConfigitation        ), AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.rateLimiterEnable            ), AAS_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = TXDMA_DIFF_0_TO_1; /* offset between units */

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  aas_mustNotDuplicate_txfifoAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile ),  AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase ),  AAS_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.tagFIFOBase  ),  AAS_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* next array hold addresses in DP[0] that are global , and can't be used with addrOffset_NON_FIRST_UNIT_INDEX_CNS !!! */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalAddrArr[]=
    {
        /* hold bit of <enable/Disable SDMA Port> that must not be duplicated */
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[0].configs.globalConfigs.globalConfig1 ) , 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = TXFIFO_DIFF_0_TO_1; /* offset between units */

    /* check global */
    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/* check if register address is one of FUQ related ... need special treatment */
static GT_BOOL  aas_fuqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC fuqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQBaseAddr), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_generalControl), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_hostConfig    ), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,fuqRegAddrArr,regAddr);
}

/* check if register address is one of AUQ related ... need special treatment */
static GT_BOOL  aas_auqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC auqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQBaseAddr), 1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,auqRegAddrArr,regAddr);
}

/* check if register address is one 'per MG' SDMA , and allow to convert address
   of MG 0 to address of other MG  */
static GT_BOOL  aas_convertPerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         /* cover full array of : PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs */
         /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */

                    /* 4 bits per queue , no global bits */
/*per queue*/        {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxQCmdReg ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaCdp)}

                    /* 2 bits per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQCmdReg             ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txDmaCdp)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaResErrCnt)}/* the same as rxSdmaResourceErrorCountAndMode[0,1]*/
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaPcktCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaByteCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txQWrrPrioConfig)}

                    /* 1 bit per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQFixedPrioConfig    ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueConfig)}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketGeneratorConfigQueue)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketCountConfigQueue)}
                    /* the rxSdmaResourceErrorCountAndMode[0..7] not hold consecutive addresses !
                       need to split to 2 ranges [0..1] and [2..7] */
/*per queue: 0..1*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[0]) , 2}
/*per queue: 2..7*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[2]) , 6}



         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
}

/* check if register address is one MG 'per MG' for SDMA that need duplication  ... need special treatment */
static GT_BOOL  aas_allowDuplicatePerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    /* using registers from regDb */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.sdmaCfgReg)                          ,1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txSdmaWrrTokenParameters )           ,1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* using registers from regDb1 */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr_regDb1[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[0])        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[0] )        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[1])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[1] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[2])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[2] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[3])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[3] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[4])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[4] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[5])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[5] )        ,1}

        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.highAddressRemap)  }/*array of 6 consecutive addresses*/
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.windowControl)     }/*array of 6 consecutive addresses*/

        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.UnitDefaultID)         ,1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalRegAddrArr_regDb1,regAddr);
}

/* check if register address is one MG 'per tile' that need duplication  ... need special treatment */
static GT_BOOL  aas_mustDuplicatePerTile_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIManagement    ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIAddress       ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIConfiguration ), 1}*/
/*not relevant*/    /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.addrCompletion), 1}*/
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.sampledAtResetReg     ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.metalFix              ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.globalControl         ), 1}
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQBaseAddr           ), 1}*/
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQControl            ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.interruptCoalescing   ), 1}
        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.lastReadTimeStampReg  ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.extendedGlobalControl ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.generalConfigurations ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.genxsRateConfig       ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(twsiReg.serInitCtrl   ), 1}
/*not dup*/ /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.dummyReadAfterWriteReg), 1}*/

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
}
static GT_BOOL aas_mgUnitDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    INOUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT  unitId,
    INOUT PRV_CPSS_DXCH_UNIT_ENT  usedUnits[/*PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS*/],
    INOUT GT_U32                  *unitsIndexPtr,
    INOUT GT_U32                  *additionalBaseAddrPtr,
    INOUT GT_U32                  *dupIndexPtr,
    OUT   GT_BOOL                  *dupWasDonePtr,
    OUT   GT_BOOL                  *unitPerMgPtr
)
{
    GT_U32  ii;
    GT_U32  relativeRegAddr,pipe0Addr;
    GT_U32  tileId;
    GT_U32  unitsIndex    = *unitsIndexPtr;
    GT_U32  dupIndex      = *dupIndexPtr;
    GT_BOOL dupWasDone    = GT_FALSE;
    GT_BOOL unitPerMg     = GT_FALSE;
    GT_U32  tileOffset;
    GT_U32  portGroupIndex;
    GT_U32  isSdmaRegister = 0/*,isSdmaRegisterAllowDup = 0*/;
    GT_U32  /*isAuq = 0 , isFuq = 0 , */isGeneralPerTile = 0;

    portGroupId = portGroupId;

    if(GT_TRUE == aas_fuqSupport_mgAddr(devNum, regAddr))
    {
        /*isFuq = 1;*/
    }
    else
    if(GT_TRUE == aas_auqSupport_mgAddr(devNum, regAddr))
    {
        /*isAuq = 1;*/
    }
    else
    if(GT_TRUE == aas_mustDuplicatePerTile_mgAddr(devNum, regAddr))
    {
        isGeneralPerTile = 1;
    }
    else
    if (GT_TRUE == aas_convertPerMg_mgSdmaAddr(devNum, regAddr))
    {
        /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */
        isSdmaRegister = 1;
    }
    else
    if (GT_TRUE == aas_allowDuplicatePerMg_mgSdmaAddr(devNum, regAddr))
    {
        isSdmaRegister = 1;
        /*isSdmaRegisterAllowDup = 1;*/
    }
    else
    {
        return GT_FALSE;
    }

    {
        GT_U32  for2MG_pipeToMg[4]={1,2,3};
        GT_U32  for1MG_pipeToMg[1]={2};
        /* FUQ registers need special manipulations */

        /* NOTE: we assume that every access to WRITE to those register is done
           using 'port group' indication that is not 'unaware' !!!! */
        /* we allow 'read' / 'check status' on all port groups */

        /* in tile 0 : we get here with the address of 'MG_0_0' but we may need to
           convert it to address of 'MG_0_1'
           in tile 1 : need to convert to 'MG_1_0' or 'MG_1_1'
        */
        GT_U32  arrSize;
        GT_U32 *unitIdToMg;
        GT_U32  isAllMGsPerTile = isGeneralPerTile || isSdmaRegister;

        if(isAllMGsPerTile)
        {
            unitIdToMg =  for2MG_pipeToMg;
            arrSize = sizeof(for2MG_pipeToMg)/sizeof(for2MG_pipeToMg[0]);
        }
        else
        {
            unitIdToMg =  for1MG_pipeToMg;
            arrSize = sizeof(for1MG_pipeToMg)/sizeof(for1MG_pipeToMg[0]);
        }

        if(isAllMGsPerTile)
        {
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;
        }

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        /* support the tile 0 */
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex] = ii; /* MG id */
            portGroupIndex++;
        }

        /* support the tile 1 */
        for(tileId = 1 ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId ++)
        {
            /* tile offset */
            tileOffset = tileId * AAS_TILE_OFFSET_CNS;

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex] = unitIdToMg[portGroupIndex%arrSize]; /*per mg*/
                portGroupIndex++;
            }
        }

        dupWasDone  = GT_TRUE;
        unitPerMg = GT_TRUE;
    }

    *unitsIndexPtr = unitsIndex;
    *dupIndexPtr   = dupIndex;
    *dupWasDonePtr = dupWasDone;
    *unitPerMgPtr= unitPerMg;


    return GT_TRUE;
}

/* SHM : check if register address is not to be duplicated */
static GT_BOOL  aas_mustNotDuplicate_shmAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC regAddrArr[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.fdb_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.em_illegal_address ), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.arp_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_aging_illegal_address), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,regAddrArr,regAddr);
}

/* the LPM table shared between the 2 tiles ! */
static GT_BOOL  aas_mustNotDuplicate_lpmMemAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        /* the LPM table is duplicated by the HW (but not by simulation)
           within the single tile */
        /* but the SW need to duplicate the table per tile !!! */
        {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,    0/*take from tabled DB*/}

        /* must be last */
        ,{CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

/* the ARP table shared between the 2 tiles ! */
static GT_BOOL  aas_mustNotDuplicate_arpMemAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        /* the ARP table shared between 2 tables*/
        {CPSS_DXCH_SIP7_TABLE_HA_ARP_E,    0/*take from tabled DB*/}

        /* must be last */
        ,{CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

/*check if register address is in IA unit (ingress Aggregator) and need 'port group' aware (pipes)
    GT_TRUE  -     need 'port group' aware. (pipes)
    GT_FALSE - not need 'port group' aware. (need to be duplicated to ALL port groups (pipes))
*/
/*
    the 'caller' gives address in ingressAggregator[0] and a specific portGroupId (PipeId),
    and the 'engine' will calculate the address for the actual portGroupId (PipeId).

    meaning that the addresses in the ingressAggregator[x] where x!= 0 are not used !!!
*/
static GT_BOOL  aas_supportDuplicate_iaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPipeAddrArr[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholds0), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholds1), 1}
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholdsRandomization), 1}*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPPfcCounterThresholds),1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPPfcGlobalFillLevelThresholds),1}
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.GlobalTcPfcTriggerVectorsConfig),1}*/
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPTcPfcTriggerVectorsConfig[0]),1}*/
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPTcPfcTriggerVectorsConfig[1]),1}*/

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPipeAddrArr,regAddr);
}
/* check if register address is per tile*/
static GT_BOOL  aas_mustDuplicate_pdxAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        {CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,    0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}


/* check if register address is per port in TTI[0] unit AND must not be duplicated to TTI[1] */
static GT_BOOL  aas_mustNotDuplicate_ttiAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
       /* no need to duplicate those registers in DB ... the caller will always call
          specific to portGroup , and without 'multiple' port groups
          {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes) , 288}
       */
         /* must be last */
        {END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

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
);

/**
* @internal prvCpssAasDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in AAS device.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAasDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;/* index in additionalBaseAddrPtr */
    GT_U32  ii;             /*iterator*/
    GT_BOOL unitPerMg;      /*indication to use 'per MG'      units */
    GT_BOOL unitPerGdma;    /*indication to use 'per GDMA'    units */
    GT_BOOL dupWasDone;     /*indication that dup per pipe/tile/2 tiles was already done */
    PRV_CPSS_DXCH_UNIT_ENT  usedUnits[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];
    GT_U32  unitsIndex; /* index in usedUnits */
    GT_U32  tileId;/* tileId iterator */
    GT_U32  relativeRegAddr;/* relative register address to it's unit */
    GT_U32  maxSpecialNumUnit = 0;

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    unitPerMg = GT_FALSE;
    unitPerGdma = GT_FALSE;
    dupWasDone = GT_FALSE;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];
    dupIndex = 0;

    /* call direct to prvCpssDxChAasHwRegAddrToUnitIdConvert ... not need to
       get to it from prvCpssDxChHwRegAddrToUnitIdConvert(...) */
    unitId = prvCpssDxChAasHwRegAddrToUnitIdConvert(devNum, regAddr);
    /* set the 'orig' unit at index 0 */
    unitsIndex = 0;
    usedUnits[unitsIndex++] = unitId;

    if(unitId >= PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS)
    {
        /* we get here for 'non first instance' of duplicated units */
        /* we not need to duplicate the address */
        return GT_FALSE;
    }

    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_EQ_E                         :
        case PRV_CPSS_DXCH_UNIT_BMA_E                         :
        case PRV_CPSS_DXCH_UNIT_PHA_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_SHT_E                     :
        case PRV_CPSS_DXCH_UNIT_EGF_QAG_E                     :
        case PRV_CPSS_DXCH_UNIT_PCL_E                         :
        case PRV_CPSS_DXCH_UNIT_L2I_E                         :
        case PRV_CPSS_DXCH_UNIT_IPVX_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_1_E                      :
        case PRV_CPSS_DXCH_UNIT_IOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_MLL_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_EFT_E                     :
        case PRV_CPSS_DXCH_UNIT_CNC_0_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_1_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_2_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_3_E                       :
        case PRV_CPSS_DXCH_UNIT_ERMRK_E                       :
        case PRV_CPSS_DXCH_UNIT_EPCL_E                        :
        case PRV_CPSS_DXCH_UNIT_EPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_EOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_EREP_E                        :
        case PRV_CPSS_DXCH_UNIT_PREQ_E                        :
        case PRV_CPSS_DXCH_UNIT_HBU_E                         :
        case PRV_CPSS_DXCH_UNIT_CNM_GDMA_DISPATCHER_E         :
            /* ALL those are units that ALL it's config should be also on tile 1 */
            /* NOTE: if one of those units hold some special registers ....
                     take it out of this generic case */
            break;

        case PRV_CPSS_DXCH_UNIT_TCAM_E                        :
        case PRV_CPSS_DXCH_UNIT_FDB_E                         :
        case PRV_CPSS_DXCH_UNIT_EM_E                          :
        case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E                  :
            break;

        case PRV_CPSS_DXCH_UNIT_MPCL_E                        :
            break;


        /* we get here for 'non first instance' of duplicated units */
        case PRV_CPSS_DXCH_UNIT_RXDMA1_E        :
        case PRV_CPSS_DXCH_UNIT_RXDMA2_E        :
        case PRV_CPSS_DXCH_UNIT_RXDMA3_E        :
        case PRV_CPSS_DXCH_UNIT_TXDMA1_E        :
        case PRV_CPSS_DXCH_UNIT_TXDMA2_E        :
        case PRV_CPSS_DXCH_UNIT_TXDMA3_E        :
        case PRV_CPSS_DXCH_UNIT_TX_FIFO1_E      :
        case PRV_CPSS_DXCH_UNIT_TX_FIFO2_E      :
        case PRV_CPSS_DXCH_UNIT_TX_FIFO3_E      :
            /* we get here for 'non first instance' of duplicated units */
            return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_LPM_1_E                       :
        case PRV_CPSS_DXCH_UNIT_SERDES_E                      :
        case PRV_CPSS_DXCH_UNIT_GOP_E                         :
        case PRV_CPSS_DXCH_UNIT_TAI_E                         :
        case PRV_CPSS_DXCH_UNIT_TAI1_E                        :
        case PRV_CPSS_DXCH_UNIT_TAI2_E                        :
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                     :
        case PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                    :
            /* those units are not for duplication */
            return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_IA_E                          :
            if(GT_FALSE == aas_supportDuplicate_iaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            break;

        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == aas_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA3_E;

            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == aas_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA3_E;

            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == aas_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E;

            break;


        case  PRV_CPSS_DXCH_UNIT_MG_E                         :
            if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : MG units exists in GM only");
            }

            if(GT_FALSE == aas_mgUnitDuplicatedMultiPortGroupsGet(
                devNum,portGroupId,regAddr,additionalRegDupPtr,unitId,
                usedUnits,&unitsIndex,additionalBaseAddrPtr,&dupIndex,
                &dupWasDone,&unitPerMg))
            {
                return GT_FALSE;
            }

            maxSpecialNumUnit = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;

            break;

        case  PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E                 :
            if(GT_FALSE == aas_gdmaUnitDuplicatedMultiPortGroupsGet(
                devNum,portGroupId,regAddr,
                additionalRegDupPtr,additionalBaseAddrPtr,
                &dupIndex,&dupWasDone,&unitPerGdma))
            {
                return GT_FALSE;
            }

            if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            {
                /* we need to duplicate to all GDMA units in the tile */
                usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_CNM_GDMA_1_E;
                usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_CNM_GDMA_2_E;
                usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_CNM_GDMA_3_E;
            }

            maxSpecialNumUnit =  (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles *
                PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS);

            unitPerMg = unitPerGdma;

            break;

        case PRV_CPSS_DXCH_UNIT_SHM_E                         :
            if(GT_TRUE == aas_mustNotDuplicate_shmAddr(devNum, regAddr))
            {
                /* the address is not for duplication */
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_LPM_E:
            if(GT_TRUE == aas_mustNotDuplicate_lpmMemAddr(devNum, regAddr))
            {
                /* the LPM table shared between the 2 tiles ! */
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_HA_E:
            if(GT_TRUE == aas_mustNotDuplicate_arpMemAddr(devNum, regAddr))
            {
                /* the ARP table shared between the 2 tiles ! */
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                     :
            if(GT_FALSE == aas_mustDuplicate_pdxAddr(devNum, regAddr))
            {
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_TTI_E                         :
            if(GT_TRUE == aas_mustNotDuplicate_ttiAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            break;

        default:
            /* we get here for 'non first instance' of duplicated units */
            /* we not need to duplicate the address */
            return GT_FALSE;
    }

    if(unitsIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,unitsIndex);
    }

    if(dupWasDone == GT_TRUE)
    {
        /********************************/
        /* no extra duplication needed  */
        /* complex logic already applied*/
        /********************************/
    }
    else
    {
        if(unitsIndex > 1)
        {
            relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

            /****************************/
            /* handle the duplications  */
            /****************************/
            /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
            tileId = 0;
            for(ii = 1; ii < unitsIndex ; ii++)
            {
                additionalRegDupPtr->portGroupsArr[dupIndex] = tileId;
                additionalBaseAddrPtr[dupIndex++] =
                    prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                    relativeRegAddr;/* baseOfUnit + 'relative' offset */
            }
        }

        /* we get here for units in tile 0 that need duplication to tile 1 */

        /* support more than single tile */
        for(tileId = 1 ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += 1)
        {
            /* duplication of the 'orig address' that not exists in :
                additionalBaseAddrPtr[]
            */
            additionalRegDupPtr->portGroupsArr[dupIndex] = tileId;
            additionalBaseAddrPtr[dupIndex++] = regAddr + tileId * AAS_TILE_OFFSET_CNS;

            /* duplication of the address that already exists in :
                additionalBaseAddrPtr[]
            */
            /* the additionalBaseAddrPtr[0..unitsIndex-1] hold tile 0 addresses ,
                the additionalBaseAddrPtr[unitsIndex] hold already tile 1 address ,
                so we skip ii 0 (to use 'ii-1' for adding the tile offset) */
            for(ii = 1; ii < unitsIndex ; ii++)
            {
                additionalRegDupPtr->portGroupsArr[dupIndex] = tileId;
                additionalBaseAddrPtr[dupIndex++] = additionalBaseAddrPtr[ii-1] + tileId * AAS_TILE_OFFSET_CNS;
            }
        }
    }

    if(dupIndex == 0 && unitPerMg == GT_FALSE)
    {
        return GT_FALSE;
    }

    /* support multi-pipe awareness by parameter <portGroupId> */
    switch(portGroupId)
    {
        case CPSS_PORT_GROUP_UNAWARE_MODE_CNS:
            /* allow the loops on the 2 addresses (orig+additional) to be
                accessed , each one in different iteration in the loop of :
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(...)
                or
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(...)
            */

            additionalRegDupPtr->use_originalAddressPortGroup = GT_TRUE;
            additionalRegDupPtr->originalAddressPortGroup = 0; /* pipe 0 */

            additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
            *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            break;
        case 0:
             if(unitPerMg == GT_TRUE)
             {
                goto treat_mg_lbl;
             }

            /* access only to this tile */
            *portGroupsBmpPtr = 1 << portGroupId;
            /* use also addresses in additionalBaseAddrPtr[] */
            additionalRegDupPtr->use_portGroupsArr = dupIndex ? GT_TRUE : GT_FALSE;
            break;
        case 1:
             if(unitPerMg == GT_TRUE)
             {
                goto treat_mg_lbl;
             }
            /* access only to this tile */
            *portGroupsBmpPtr = 1 << portGroupId;
            /* the only valid address is the one in additionalBaseAddrPtr[..] */
            additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
            additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
            break;
        default:
            if(unitPerMg == GT_TRUE  && portGroupId < maxSpecialNumUnit)
            {
                treat_mg_lbl:
                if(portGroupId == 0)
                {
                    *portGroupsBmpPtr = BIT_0;
                    dupIndex = 0;/* access only to MG 0 --> no duplications */
                }
                else
                {
                    *portGroupsBmpPtr = (BIT_0 << portGroupId);
                    /* access only to this pipe (1/2/3/4/5/6/7) */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    /* the only valid address is the one in additionalBaseAddrPtr[0] */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                }
            }
            else
            {
                /* should not get here */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : unsupported port group id [%d] ",
                    portGroupId);
            }
            break;
    }

    if(dupIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,dupIndex);
    }

    if(unitPerMg == GT_TRUE)
    {
        /******************************************/
        /* mask the MGs BMP with existing MGs BMP */
        /******************************************/
        *portGroupsBmpPtr &= (1<<maxSpecialNumUnit)-1;

        /* state the caller not to do more mask */
        additionalRegDupPtr->skipPortGroupsBmpMask = GT_TRUE;
    }
    else
    {
        /**********************************************/
        /* mask the pipes BMP with existing pipes BMP */
        /**********************************************/
        *portGroupsBmpPtr &= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;
    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}

/**
* @internal aasOffsetFromTile0Get function
* @endinternal
*
* @brief   AAS : Get The offset (in bytes) between the address
*         of register in pipe 0 of this unit to the register in the other pipe.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] tileId     - The tileId. (in case of unit per tile --> use  = 2tileId)
*/
static GT_U32   aasOffsetFromTile0Get(
    IN GT_U32                   tileId
)
{
    return tileId * AAS_TILE_OFFSET_CNS;
}

/**
* @internal prvCpssAasOffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
static GT_U32   prvCpssAasOffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
)
{
    GT_U32  relativeInstanceInTile0;
    GT_U32  tile0UnitId;
    GT_U32  offsetFromTile0;
    GT_U32  instance0BaseAddr,instanceInTile0BaseAddr;
    GT_U32  tileId;
    static const GT_U32 numOfGdmaPerTile = PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS;/*4*/
    static const GT_U32 numOfDpPerTile   = 4;

    switch(instance0UnitId)
    {
        /************/
        /* per tile */
        /************/
        default:
            tile0UnitId = instance0UnitId;
            tileId      = instanceId;
            break;

        case  PRV_CPSS_DXCH_UNIT_MG_E               :
        if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED, "ERROR : MG units exists in GM only");
        }

        {
            /* tile 0 : MG 0,1 - serve 'tile 0' located at tile 0 memory */
            /* tile 1 : MG 2,3 - serve 'tile 1' located at tile 1 memory */
            GT_U32  internalCnmOffset = (MG_SIZE            * (instanceId % NUM_MG_PER_CNM));

            tileId = instanceId / NUM_MG_PER_CNM;
            offsetFromTile0 = aasOffsetFromTile0Get(tileId);

            return offsetFromTile0 + internalCnmOffset;
        }

        case  PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E       :
            relativeInstanceInTile0 = instanceId % numOfGdmaPerTile;
            tileId                  = instanceId / numOfGdmaPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_CNM_GDMA_1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_CNM_GDMA_2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_CNM_GDMA_3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E; break;
            }
            break;

        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_RXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXDMA_E            :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_TXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO_E          :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E   :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E   :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E; break;
            }
            break;

         case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E   :
            relativeInstanceInTile0 = instanceId % numOfDpPerTile;
            tileId                  = instanceId / numOfDpPerTile;

            switch(relativeInstanceInTile0)
            {
                case 1: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E; break;
                case 2: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E; break;
                case 3: tile0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E; break;
                default: tile0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E; break;
            }
            break;
    }

    /* we got here for units that are per DP */
    /* 1. get the offset of the unit from first instance in Pipe 0 */
    /*  1.a get address of first  instance */
    instance0BaseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,instance0UnitId,NULL);

    /*  1.b get address of needed instance (in tile 0)*/
    if(tile0UnitId != (GT_U32)instance0UnitId)
    {
        instanceInTile0BaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,tile0UnitId,NULL);
    }
    else
    {
        instanceInTile0BaseAddr = instance0BaseAddr;
    }

    /* 2. get the offset of the unit from Tile 0                   */
    if(tileId)
    {
        offsetFromTile0 = aasOffsetFromTile0Get(tileId);
    }
    else
    {
        offsetFromTile0 = 0;
    }

    return (instanceInTile0BaseAddr - instance0BaseAddr) + offsetFromTile0;
}


/**
* @internal aasSpecialPortsMapGet function
* @endinternal
*
* @brief   Aas : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS aasSpecialPortsMapGet
(
    IN GT_U8  devNum,
    OUT const SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            *specialDmaPortsPtrPtr   =           aas_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_aas_SpecialDma_ports;
            break;
        case 2:
            *specialDmaPortsPtrPtr   =           aas_2_tiles_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_aas_2_tiles_SpecialDma_ports;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "not supported number of tiles [%d]",
                PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    return GT_OK;
}

/**
* @internal prvCpssAasDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Hawk : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
static GT_STATUS prvCpssAasDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    if(globalDmaNum < numRegularPorts)
    {
        if(dpIndexPtr)
        {
            *dpIndexPtr     = globalDmaNum / AAS_PORTS_PER_DP_CNS;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr = globalDmaNum % AAS_PORTS_PER_DP_CNS;
        }
    }
    else
    {
        rc = aasSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(globalDmaNum >= (numRegularPorts + numOfSpecialDmaPorts))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global DMA number [%d] is >= [%d] (the max)",
                globalDmaNum,
                numRegularPorts + numOfSpecialDmaPorts);
        }

        if(dpIndexPtr)
        {
            *dpIndexPtr       = specialDmaPortsPtr[globalDmaNum-numRegularPorts].dpIndex;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr   = specialDmaPortsPtr[globalDmaNum-numRegularPorts].localPortInDp;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssAasDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   AAS : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index (GLOBAL DP in the device)
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
static GT_STATUS prvCpssAasDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    if(localDmaNum >= AAS_NUM_PORTS_DMA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "local DMA number [%d] >= [%d] (the max)",
            localDmaNum,AAS_NUM_PORTS_DMA);
    }

    if(localDmaNum < AAS_PORTS_PER_DP_CNS)
    {
        *globalDmaNumPtr = (AAS_PORTS_PER_DP_CNS*dpIndex) + localDmaNum;
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dpIndex].cpuPortDmaNum == GT_NA &&
        (localDmaNum == (AAS_PORTS_PER_DP_CNS+2)))
    {
        /* support GM device , that not have same amount of CPU ports */
        return /*do not log*/GT_NOT_FOUND;
    }

    rc = aasSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex       == dpIndex &&
           specialDmaPortsPtr[ii].localPortInDp == localDmaNum)
        {
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    CPSS_LOG_INFORMATION_MAC("dpIndex[%d] and localDmaNum[%d] not found as existing DMA",
    dpIndex,localDmaNum);

    return /*do not log*/GT_NOT_FOUND;
}

/**
* @internal prvCpssAasDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Aas : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
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
static GT_STATUS prvCpssAasDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    GT_U32  globalDmaNum;

    /* check validity and get 'first globalDmaNum' */
    rc = prvCpssAasDmaLocalNumInDpToGlobalNumConvert(devNum,
        dpIndex,localDmaNum,&globalDmaNum);

    if(rc != GT_OK)
    {
        /* there is error with {dpIndex,localDmaNum} ... so no 'first' and no 'next' ...*/
        return rc;
    }

    if(GT_NA == (*globalDmaNumPtr))
    {
        /* get first global DMA that match {dpIndex,localDmaNum} */
        *globalDmaNumPtr = globalDmaNum;
        return GT_OK;
    }

    /* check if there is 'next' */
    if(localDmaNum < AAS_PORTS_PER_DP_CNS)
    {
        /* regular port without muxing */

        return /* not error for the LOG */ GT_NO_MORE;
    }

    /* use the pointer as 'IN' parameter */
    globalDmaNum = *globalDmaNumPtr;

    rc = aasSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    /*************************************************/
    /* start looking from 'next' index after current */
    /*************************************************/
    ii = (globalDmaNum-numRegularPorts) + 1;

    for(/*already init*/ ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex       == dpIndex &&
           specialDmaPortsPtr[ii].localPortInDp == localDmaNum)
        {
            /****************/
            /* found 'next' */
            /****************/
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    /********************/
    /* NOT found 'next' */
    /********************/

    return /* not error for the LOG */ GT_NO_MORE;
}

/* return the BMP of local macs that may effect the local mac */
static GT_U32   macBmpEclipseSerdesBehindGet(IN GT_U32 ciderIndexInUnit)
{
    /*0,1,2,3,4,5,6,7*/
    /*0,-,2,-,4,-,6,-*/
    static GT_U32  macBmpEclipseSerdesBehind[4] = {
        BIT_0,
        BIT_0 | BIT_2,
        BIT_0 | BIT_2 | BIT_4,
        BIT_0 | BIT_2 | BIT_4 | BIT_6
    };

    return macBmpEclipseSerdesBehind[ciderIndexInUnit / 2];
}

/* return the BMP of local macs that may be effected by the local mac */
static GT_U32   macBmpEclipseSerdesAHeadGet(
    IN GT_U32 ciderIndexInUnit ,
    IN GT_U32 numOfActLanes
)
{
    GT_U32  ii;
    GT_U32  bmp = 0;
    static GT_U32   macBmpEclipseSerdesAHead[8] = {0,1,2,3,4,5,6,7};

    for(ii = ciderIndexInUnit ; (ii < (ciderIndexInUnit+numOfActLanes)) && (ii < 8) ; ii++)
    {
        bmp |= 1 << macBmpEclipseSerdesAHead[ii];
    }

    return bmp;
}
static GT_U32 startMacIn_macBmpEclipseSerdesGet(
    IN GT_U32 tileId,
    IN GT_U32 ciderIndex
)
{
    static GT_U32   startMacInUnit[]={0 , 8,16,
                                      20,28,36,
                                      40,48,56,
                                      60,68,76}; /* see also function hwsHarrierPortParamsGetLanes(...) */
    return startMacInUnit[ciderIndex] + tileId * SERDES_NUM_NETWORK_PORTS;
}

/* return the SERDES global id for the global MAC */
static GT_U32  macSerdesGet(
    IN GT_U32                      portMacNum
)
{
    return portMacNum;/*1:1*/
}

/**
* @internal lmuPhysicalPortNumToLmuIndexConvert function
* @endinternal
** @brief   LMU unit : convert the global physical port number to global LMU unit in the device and channel Id in it.
*         this is needed to access the registers in PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[lmuIndex]

*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - The PP's device number.
* @param[in] portMacNum           - the global mac port number.
* @param[out] lmuIndexPtr         - (pointer to) the global LMU unit index in the device.
* @param[out] localChannelInLmuPtr- (pointer to) the local channel that the port uses in the LMU unit.
* @return - GT_OK on success
*/
static GT_STATUS lmuPhysicalPortNumToLmuIndexConvert
(
    IN  GT_U8                 devNum,
    IN  GT_U32                portMacNum,
    OUT GT_U32                *lmuIndexPtr,
    OUT GT_U32                *localChannelInLmuPtr
)
{
    /* get global DP[] and local DMA in the global DP */
    return prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,portMacNum,
        lmuIndexPtr,localChannelInLmuPtr);
}

/**
* @internal prvCpssAasRegDbInfoGet function
* @endinternal
*
* @brief   function to get the info to index in 'reg DB'
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum          - The PP's device number.
* @param[in] portMacNum      - the global mac port number.
* @param[out] regDbType      - the type of regDbInfo.
* @param[out] regDbInfoPtr   - (pointer to) the reg db info
* @return - GT_OK on success
*/
static GT_STATUS   prvCpssAasRegDbInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN PRV_CPSS_REG_DB_TYPE_ENT    regDbType,
    IN PRV_CPSS_REG_DB_INFO_STC   *regDbInfoPtr
)
{
    GT_STATUS       rc;
    MAC_INFO_STC    macInfo;
    HWS_UNIT_BASE_ADDR_TYPE_ENT     unitId;
    GT_U32  lmuIndex,localChannelInLmu;

    switch(regDbType)
    {
        case PRV_CPSS_REG_DB_TYPE_MTI_MIB_COUNTER_SHARED_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
        case PRV_CPSS_REG_DB_TYPE_PB_CHANNEL_ID_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_FIRST_SERDES_E:
            /* dummy - not used , just to make macInfoGet to pass without error*/
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(regDbType);
    }
    rc =  macInfoGet(devNum,portMacNum,unitId,&macInfo,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    regDbInfoPtr->macBmpEclipseSerdes = 0;/* don't care */
    switch(regDbType)
    {
        case PRV_CPSS_REG_DB_TYPE_MTI_MIB_COUNTER_SHARED_E:
            /* shared per 8 ports , and index : */
            /* we need global DP (so convert tileId to 'ciderUnit')*/
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit + 3 * macInfo.tileId * (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp/PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes);
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
            rc = lmuPhysicalPortNumToLmuIndexConvert(devNum,portMacNum,&lmuIndex,&localChannelInLmu);
            if(rc != GT_OK)
            {
                return rc;
            }
            regDbInfoPtr->regDbIndex       = lmuIndex; /* index in PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[_lmu] */
            regDbInfoPtr->ciderIndexInUnit = localChannelInLmu;
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesBehindGet(regDbInfoPtr->ciderIndexInUnit);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = startMacIn_macBmpEclipseSerdesGet(macInfo.tileId,macInfo.ciderUnit);
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesAHeadGet(regDbInfoPtr->ciderIndexInUnit ,
                regDbInfoPtr->startMacIn_macBmpEclipseSerdes/*numOfActLanes*/);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = startMacIn_macBmpEclipseSerdesGet(macInfo.tileId,macInfo.ciderUnit);
            break;
        case PRV_CPSS_REG_DB_TYPE_PB_CHANNEL_ID_E:
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit;       /* not used */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;

        case PRV_CPSS_REG_DB_TYPE_FIRST_SERDES_E:
            regDbInfoPtr->regDbIndex       = 0;       /* not used */
            regDbInfoPtr->ciderIndexInUnit = 0;       /* not used */
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = macSerdesGet(portMacNum);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(regDbType);
    }

    return GT_OK;
}

extern GT_U32 debug_force_numOfDp_get(void);

/* list of the Aas-L : common */
static const CPSS_PP_DEVICE_TYPE devs_aas_common[]=
{
    PRV_CPSS_AAS_ALL_DEVICES,
    LAST_DEV_IN_LIST_CNS /* must be last one */
 };

/* list of the Aas-L : common */
static const CPSS_PP_DEVICE_TYPE devs_aas_2_tiles_common[]=
{
    PRV_CPSS_AAS_2_TILES_ALL_DEVICES,
    LAST_DEV_IN_LIST_CNS /* must be last one */
 };

#ifndef _14K
    #define _14K    (14 * _1K)
#endif

#define LPM_MAX_NUM_OF_BLOCKS 48

#ifdef GM_USED
#define LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS _16K
#else
#define LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS _256K
#endif

#define _96K    (_64K+_32K)
/* max lines in AAS
   We have 48 big banks,
   Each pair (for example LPM bank 4 + bank 5) can be total 256K entries.
   Also we have 8 small banks with size _2K, if we use them they will be
   connected to banks 40-47 only if they are not connected to large memories */
#define AAS_MAX_LPM_CNS ((_256K/2)*LPM_MAX_NUM_OF_BLOCKS)

/* tcam line SW - 6 20_byte rules, HW addr space - 8 20_byte rules - 8 actions */
/* tcam floor - 256 lines, SW - 1.5K 20_byte rules, HW addr space - 2K 20_byte rules - 2K actions */
#define TCAM_FLOORS        24 /* SW - 24 *3K = 72K@10B = 36K@20B */
#define TCAM_ACTIONS_HW    TCAM_FLOORS * _2K
#define TCAM_ACTIONS_SW    TCAM_FLOORS * (_3K / 2)

#define NUM_TRUNK_ECMP  _8K /*Trunk members*/

/* Aas - fine tuning table  sizes according to devices flavors */
static const FINE_TUNING_STC aasFineTuningTables[]=
{
    { /* AAS : common */
        devs_aas_common,
        {
            NULL,  /*enhancedInfoPtr*/
            {
                /* bridge section                             */
                _256K,            /*fdb                        */
                _5K,            /*vidxNum                    */
                512,             /*stgNum                     */
                                 /*                           */
                /* TCAM section                               */
                0,               /* GT_U32 router;            */
                TCAM_ACTIONS_SW/*72*_1K*/,/*36K@20B*//* GT_U32 tunnelTerm;         */
                                 /*                           */
                /* tunnel section                             */
                _128K,           /*GT_U32  tunnelStart;   */
        /*_512K*/_512K,          /*GT_U32  routerArp;     */
                                 /*                          */
                /* ip section                                */
                _48K,            /*GT_U32  routerNextHop;    */
                _32K,            /*GT_U32  ecmpQos;          */
                _128K,           /*GT_U32  mllPairs;         */
                                 /*                           */
                /* PCL section                                */
                PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC,/*GT_U32 policyTcamRaws;    */

                                 /*                           */
                /* CNC section                                */
                32,  /* GT_U32 cncBlocks;          (CNC units 0,1,2,3)*/
                _8K, /* GT_U32 cncBlockNumEntries; */

                /*  Policer Section                           */
                /* NOTE: 128K is shared memory between IPLR0,1,EPLR */
                _128K,           /*policersNum                  */
                _128K,           /*egressPolicersNum            */
                                 /*                           */
                /* trunk Section                              */
                NUM_TRUNK_ECMP,/*trunk,L2Ecmp - members (using hwInfo.parametericTables.numEntriesL2Ecmp)*/
                                 /*                           */
                /* Transmit Descriptors                       */
                40*_1K,             /*GT_U32 transmitDescr;     */  CPSS_TBD_BOOKMARK_AAS
                                 /*                           */
                /* Buffer Memory                              */
                2*_1M,              /*GT_U32 bufferMemory;      */
                TX_QUEUE_NUM_8_CNS,  /* txQueuesNum - TX Queues number */
                PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(AAS_MAX_LPM_CNS),       /* GT_U32  lpmRam,  */
                AUTO_CALC_FIELD_CNS, /* pipeBWCapacityInGbps auto calculated by hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet */
                NA_TABLE_CNS,        /* bpePointToPointEChannels  */
                _512K,               /*emNum                      */
                _32K,                /* vrfIdNum                  */
                _16K,                 /* oamEntriesNum             */
                {   /*sip7_em_clients*/
                     GT_NA  /*ttiNum;       */
                    ,GT_NA  /*ipcl0Num;     */
                    ,GT_NA  /*ipcl1Num;     */
                    ,GT_NA  /*mpclNum;      */
                    ,GT_NA  /*epclNum;      */
                    ,GT_NA  /*ipeNum;       */
                    ,GT_NA  /*ttiIlmNum;    */
                },/*ended : sip7_em_clients*/
                _32K,/*sip7_hf0*/
                _32K,/*sip7_hf1*/
                _256K,/*sip7_plr_counters*/
                GT_NA,/*sip7_ipe0*/
                GT_NA,/*sip7_ipe1*/
            },

            {
                /* TR101 Feature support */
                GT_TRUE, /*tr101Supported*/

                /* VLAN translation support */
                GT_TRUE, /*vlanTranslationSupported*/

                /* Policer Ingress second stage support flag                  */
                GT_TRUE, /*iplrSecondStageSupported*/

                /* trunk CRC hash support flag */
                GT_TRUE, /*trunkCrcHashSupported*/

                /* TM support flag */
                GT_FALSE,  /* TmSupported */

                /* TSN features support */
                GT_TRUE    /* tsnSupported */
            }
        }
    },
    {
        devs_aas_2_tiles_common,
        {
            NULL,  /*enhancedInfoPtr*/
            {
                /* bridge section                             */
                _256K,            /*fdb                        */
                _5K,            /*vidxNum                    */
                512,             /*stgNum                     */
                                 /*                           */
                /* TCAM section                               */
                0,               /* GT_U32 router;            */
                TCAM_ACTIONS_SW/*72*_1K*/,/*36K@20B*//* GT_U32 tunnelTerm;         */
                                 /*                           */
                /* tunnel section                             */
                _128K,           /*GT_U32  tunnelStart;   */
        /*_512K*/AUTO_CALC_FIELD_CNS,/*GT_U32  routerArp;     */
                                 /*                          */
                /* ip section                                */
                _48K,            /*GT_U32  routerNextHop;    */
                _32K,            /*GT_U32  ecmpQos;          */
                _128K,           /*GT_U32  mllPairs;         */
                                 /*                           */
                /* PCL section                                */
                PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC,/*GT_U32 policyTcamRaws;    */

                                 /*                           */
                /* CNC section                                */
                CNC_AUTO_CALC,  /* GT_U32 cncBlocks;          */
                2*16*_8K,/*2 units , 16 blocks in each, 8K per block */ /* GT_U32 cncBlockNumEntries; */

                /*  Policer Section                           */
                /* NOTE: 128K is shared memory between IPLR0,1,EPLR */
                _128K,           /*policersNum                  */
                _128K,           /*egressPolicersNum            */
                                 /*                           */
                /* trunk Section                              */
                NUM_TRUNK_ECMP,/*trunk,L2Ecmp - members (using hwInfo.parametericTables.numEntriesL2Ecmp)*/
                                 /*                           */
                /* Transmit Descriptors                       */
                40*_1K,             /*GT_U32 transmitDescr;     */  CPSS_TBD_BOOKMARK_AAS
                                 /*                           */
                /* Buffer Memory                              */
                2*_1M,              /*GT_U32 bufferMemory;      */
                TX_QUEUE_NUM_8_CNS,  /* txQueuesNum - TX Queues number */
                PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(AAS_MAX_LPM_CNS),       /* GT_U32  lpmRam,  */
                AUTO_CALC_FIELD_CNS, /* pipeBWCapacityInGbps auto calculated by hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet */
                NA_TABLE_CNS,        /* bpePointToPointEChannels  */
                _1M,                 /*emNum                      */
                _32K,                /* vrfIdNum                  */
                _16K,                /* oamEntriesNum             */
                {   /*sip7_em_clients*/
                     GT_NA  /*ttiNum;       */
                    ,GT_NA  /*ipcl0Num;     */
                    ,GT_NA  /*ipcl1Num;     */
                    ,GT_NA  /*mpclNum;      */
                    ,GT_NA  /*epclNum;      */
                    ,GT_NA  /*ipeNum;       */
                    ,GT_NA  /*ttiIlmNum;    */
                },/*ended : sip7_em_clients*/
                _64K,/*sip7_hf0*/
                _64K,/*sip7_hf1*/
                _256K,/*sip7_plr_counters*/
                GT_NA,/*sip7_ipe0*/
                GT_NA,/*sip7_ipe1*/
            },

            {
                /* TR101 Feature support */
                GT_TRUE, /*tr101Supported*/

                /* VLAN translation support */
                GT_TRUE, /*vlanTranslationSupported*/

                /* Policer Ingress second stage support flag                  */
                GT_TRUE, /*iplrSecondStageSupported*/

                /* trunk CRC hash support flag */
                GT_TRUE, /*trunkCrcHashSupported*/

                /* TM support flag */
                GT_FALSE,  /* TmSupported */

                /* TSN features support */
                GT_TRUE    /* tsnSupported */
            }
        }
    }
};
static const GT_U32 aasFineTuningTables_size = sizeof(aasFineTuningTables)/sizeof(aasFineTuningTables[0]);


/* in sip7 the registers in fdb unit starts at 0x02000000*/
#define AAS_FDB_REG_RELATIVE_BASE   0x02000000

/* Common macro for Direct Info entry PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC */
#define PRV_DIRECT_INFO_MAC(_baseAddr, _step/*in bytes*/, _nextWord, _notUsed_tableId) \
    {_baseAddr, _step, _nextWord}

/* Macro for AAS Direct Info record in Table Info (refer PRV_CPSS_DXCH_TABLES_INFO_STC) */

#define AAS_DIRECT_MAC(_tblSize,_entrySize, _tableId) \
{ _tableId, {_tblSize, _entrySize, \
    PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL,\
    PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL}}


/* Macro for AAS Indirect Info record in Table Info */
#define AAS_INDIRECT_MAC(_tblSize,_entrySize, _indirectTblIdx, _tableId) \
{ _tableId, {_tblSize, _entrySize, \
    PRV_CPSS_DXCH_INDIRECT_ACCESS_E, &prvCpssDxChAas_indirectInfoArr[_indirectTblIdx],\
    PRV_CPSS_DXCH_INDIRECT_ACCESS_E, &prvCpssDxChAas_indirectInfoArr[_indirectTblIdx]}}

/* indirect table info for AAS  */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC prvCpssDxChAas_indirectInfoArr[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  (CPSS_DXCH_TABLE_FDB_E --> used as direct) */
     {AAS_ADDR_FDB + AAS_FDB_REG_RELATIVE_BASE + 0x00000130,
     AAS_ADDR_FDB + AAS_FDB_REG_RELATIVE_BASE + 0x00000134,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    ,{AAS_ADDR_IOAM + 0x000000D0,
     AAS_ADDR_IOAM + 0x000000D4,
     0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    ,{AAS_ADDR_EOAM + 0x000000D0,
     AAS_ADDR_EOAM + 0x000000D4,
     0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM0 + 0x00002000,
     AAS_ADDR_EM0 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_1_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM1 + 0x00002000,
     AAS_ADDR_EM1 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_2_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM2 + 0x00002000,
     AAS_ADDR_EM2 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_3_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM3 + 0x00002000,
     AAS_ADDR_EM3 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_4_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM4 + 0x00002000,
     AAS_ADDR_EM4 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_5_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM5 + 0x00002000,
     AAS_ADDR_EM5 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_5_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM6 + 0x00002000,
     AAS_ADDR_EM6 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

   /* PRV_CPSS_SIP7_EXACT_MATCH_6_TABLE_INDIRECT_E */
   ,{AAS_ADDR_EM7 + 0x00002000,
     AAS_ADDR_EM7 + 0x00002004,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}

};


/*
    NOTE: using CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC instead of NUM_OF_DP_UNITS
        for the TXQ
*/
#define TXQ_TABLE_INDEX_TILE_OFFSET(_tileId/*tile*/,_dp/*local DP*/) \
            (_tileId/*0..1*/ * CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC + _dp/*0..3*/)

#define TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(_tileId/*tile*/,_dp/*local DP*/) \
            (_tileId/*0..1*/ * NUM_OF_DP_UNITS + _dp/*0..3*/)


#define PRV_DIRECT_INFO_TILE_1_TXQ_MAC(_tileId/*tile*/,_dp/*local DP*/) \
    /*PDS0*/                                                                                                                                                                  \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x00000000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000c0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x00080000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000e6000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000e4000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000e0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000e2000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000ec000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x000e8000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_PDS+0x00100000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
     /*SDQ0*/ \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x00020000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x000C0000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E    + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x000D0000,   4, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E   + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x000d8000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E     + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x000d8200,   4, 4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E  + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x000d8600,   4, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E  + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)), \
 PRV_DIRECT_INFO_MAC(AAS_TILE_OFFSET_CNS*_tileId+AAS_ADDR_DP##_dp##_SDQ+0x00060000,   8, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E         + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp))


#define PRV_DIRECT_INFO_MISSING_IN_DP1_TXQ_MAC \
    /*PDS0*/                                                                                                                                                                  \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS+0x00010000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E          + 1)

#define PRV_DIRECT_INFO_MISSING_IN_DP2_TXQ_MAC \
    /*PDS0*/                                                                                                                                                                  \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS+0x00010000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E          + 2)


/* missing in DP 3 (got info only for DP[0..2] before adding this macro */
#define PRV_DIRECT_INFO_MISSING_IN_DP3_TXQ_MAC \
    /*PDS0*/                                                                                                                                                                  \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x00040000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E       + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x00080000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x00010000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000e6000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000e4000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000e0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000e2000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000ec000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E          + 3), \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000e8000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E          + 3), \
   /*SDQ0*/                                                                                                                                \
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x00100000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 3)


#define NUM_BITS_VPORTS  18                 /* 18 bit --> 256K vPorts */
#define NUM_VPORTS  (1<<NUM_BITS_VPORTS)    /* _256K */
#define NUM_EPORTS  (16*_1K)
#define NUM_EVIDS   (64*_1K)
#define NUM_4K_VLANS   (4*_1K)
#define NUM_PHY_PORTS  _1K
#define NUM_DEF_EPORTS  (_1K)
#define NUM_IP2ME   (256)
#define NUM_TRUNKS  (_1K)
#define NUM_STGS    (512)
#define NUM_PORT_ISOLATION  (4*_1K + 1*_1K)
#define NUM_VIDXS    (5*_1K)
#define NUM_SSTS     (4*_1K)
#define NUM_MY_PHYSICAL_PORT    304
#define NUM_ARPS    (1024*_1K)
#define NUM_TS      (128*_1K)
#define CNC_QUEUE_PORT_MAPPING_MEMORY_NUM   NUM_PHY_PORTS
#define TXQ_PDX_QUEUE_GROUP_MAP_NUM     (1024)
#define CNC_NUM_IN_BLOCK    _16K/*of 64 bits*/
#define CNC_NUM_WORDS        2 /* 2 words = 64 bits */
#define SIP7_CNC_64_OFFSET  0x00200000

/* the '*2' needed as we work with FDB of 256K in MHT8 , that need 'memory space' of 512K entries */
/* see the address manipulation in : prvCpssDxChIndexAsPortNumConvert(...) for CPSS_DXCH_TABLE_FDB_E */
#define NUM_FDB  (256*_1K)*2
#define NUM_EM   (256*_1K)
#define NUM_OAM     _64K
#define NUM_VRFS    _32K
#define NUM_L2_ECMP _32K
#define NUM_L3_ECMP _32K
#define NUM_L2_LTT_MLL  98304 /* 96K */
#define NUM_MLL_PAIRS   _128K
#define NUM_PLR_IPFIX_OR_COUNTERS _256K
#define NUM_PLR_METERS _128K
#define NUM_MIRROR_PORTS    1024 /*???*/
#define NUM_ANALYZERS    63
#define NUM_CIRCUITS     _32K
#define NUM_REDUCED_EM   _8K
/* EQ HECMP. */ /* Cider not ready yet */ CPSS_TBD_BOOKMARK
#define NUM_HECMP_LTT  (32*_1K)
#define NUM_HECMP_ECMP (16*_1K)
#define HECMP_BASE_ADDR (AAS_ADDR_EQ + 0x00600000)

/* Direct tables info */
const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC  prvCpssDxChAas_directInfoArr[] =
{
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00010000,   4, 4, CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a0000,   4, 4, CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a0400,   4, 4, CPSS_DXCH_TABLE_CPU_CODE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL+0x00020000,   8, 4, CPSS_DXCH_TABLE_PCL_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a2000,   4, 4, CPSS_DXCH_TABLE_QOS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00400000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_TABLE_MULTICAST_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00080000,   4, 4, CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x00008000,   4, 4, CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x03000000, 256, 4, CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00008000,  32, 4, CPSS_DXCH3_TABLE_MAC2ME_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00050000,   4, 4, CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00040000,   4, 4, CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00020000, 128, 4, CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00040000,   8, 4, CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00100000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00200000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00400000,  32, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x01000000,  32, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00030000,   8, 4, CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00002000,  32, 4, CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00300000,  16, 4, CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00000000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00220000,  2*(NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_STG_INGRESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00380000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_STG_EGRESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00300000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00390000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x002B0000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00100000,  16, 4, CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00210000,  32, 4, CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00240000,   4, 4, CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00400000,  16, 4, CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00210000,   8, 4, CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00202000,   8, 4, CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00200000,   4, 4, CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E),
 PRV_DIRECT_INFO_MAC(0/*no such table*/,   0, 0, CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000e0000,   8, 4, CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00022000,  16, 4, CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000c0000,   8, 4, CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00140000,   8, 4, CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00180000,   4, 4, CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x01400000,   8, 4, CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x0000E400,   4, 4, CPSS_DXCH_SIP5_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000A3000,   4, 4, CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000d0000,   4, 4, CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00001000,   4, 4, CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_LPM+0x00000000,  32, 4, CPSS_DXCH_SIP5_TABLE_LPM_MEM_E),
 PRV_DIRECT_INFO_MAC(0,   0, 0, CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E),
 PRV_DIRECT_INFO_MAC(0,   0, 0, CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E),
 PRV_DIRECT_INFO_MAC(0,   0, 0, CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_EFT + 0x00040000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x00280000,   8, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x00000000,   8, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x002A8000,   8, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x002AD000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x001A0000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x002A0000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00100000,  16, 4, CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x000F0000,  4, 4, CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00008000,  16, 4, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00014000,  16, 4, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00020000,   4, 4, CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x0000C000, 128, 4, CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x000D0000,   8, 4, CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x000C0000, 128, 4, CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00030000,   8, 4, CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MLL+0x00000000,  128, 4, CPSS_DXCH_SIP5_TABLE_IP_MLL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MLL+0x00000000,  128, 4, CPSS_DXCH_SIP5_TABLE_L2_MLL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MLL+0x00400000,   4, 4, CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E),
 PRV_DIRECT_INFO_MAC(0/*no such table*/,   0, 0, CPSS_DXCH_SIP5_TABLE_ADJACENCY_E), /* roni need to delete ?????*/
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00800000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00040000,   8, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00000000,   0, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00800000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00040000,   8, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00800000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00200000,  16, 4, CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00100000,  16, 4, CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00400000,  32, 4, CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00020000,   4, 4, CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00060000,  16, 4, CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00010000,   4, 4, CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00004000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00010000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00018000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00020000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00028000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00030000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00038000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00040000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IOAM+0x00060000,   8, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00002000,   4, 4, CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00074000, 128, 4, CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00077000,  32, 4, CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00072000,  16, 4, CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00070000,  64, 4, CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00040000,  64, 4, CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00044000,  64, 4, CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TCAM+0x00400000,  64, 4, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00005000,   8, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00100000,   4, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00200000,  64, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00300000,  64, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00003000,   4, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_ERMRK+0x00003300,   4, 4, CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TCAM+0x00000000,  16, 4, CPSS_DXCH_SIP5_TABLE_TCAM_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00280000, (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_L2I+0x00240000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x00080000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x001C0000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_SHT + 0x000C0000,   (NUM_PHY_PORTS/32)*4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_BMA+0x0005A000,   4, 4, CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_BMA+0x00000000,   4, 4, CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00030000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00030000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00030000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00008000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00008000,   4, 4, CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00008000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00020000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00040000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00060000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00080000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000a0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 4),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000c0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 5),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000e0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 6),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00100000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 7),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00120000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 8),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00140000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 9),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00160000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 10),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00180000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 11),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001a0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 12),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001c0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 13),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001e0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 14),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00200000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 15),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00100000,   8, 4, CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00100000,   8, 4, CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00100000,   8, 4, CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00900000,   4, 4, CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00900000,   4, 4, CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00900000,   4, 4, CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x002AC000,   4, 4, CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x002A6000,   4, 4, CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00160000,  32, 4, CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00280000,   4, 4, CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00002000,   4, 4, CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG + 0x002A0000,   4, 4, CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00300000,   16, 4, CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x0004A000,   4, 4, CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x0004A000,   4, 4, CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x0004A000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00048000,   4, 4, CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00048000,   4, 4, CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00048000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00001000,  32, 4, CPSS_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x0016000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x0014000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x0013000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x0012000,   4, 4, CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PDX_PDITX+0x00000000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x00100000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x00100000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x00100000,   8, 4, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x0000000,   12, 4, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x0000000,   12, 4, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x0000000,   12, 4, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000c0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000c0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000c0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x00080000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000e6000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000e6000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000e6000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000e4000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000e4000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000e4000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000e0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000e0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000e0000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000e2000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000e2000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000e2000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000ec000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000ec000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000ec000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000e8000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS +0x000e8000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E + 1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS +0x000e8000,   4, 4, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E + 2),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x00180000,   8,4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x000C0000,   8,4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x000D0000,   4,4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x000d8000,   8,4,CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PFCC+0x00001000,   4, 4, CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00060000,   4, 4, CPSS_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_TTI + 0x00300000,   4, 4, CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00003000,   4, 4, CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00040000,  16, 4, CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ + 0x00010000,  64, 4, CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ + 0x00001400,  16, 4, CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00002000,   4, 4, CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a4000,  32, 4, CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a8000,  32, 4, CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000ac000,  32, 4, CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000b0000,  32, 4, CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00280000,  8, 4, CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_LPM+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_PBR_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_LPM+0x00800000,   4, 4, CPSS_DXCH_SIP6_TABLE_LPM_AGING_E),
             /* Indexes here 207,208; 31 LMU Tables STATS and CONFIG added at the end starting from index */
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_0+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_0+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPCL+0x00100000, 16, 4, CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR0+0x00020000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPLR1+0x00020000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EPLR+0x00020000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000a1000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00004000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00005000,   4, 4, CPSS_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x000d8200,   4,4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00052000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PCL + 0x00054000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00004000,   8, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00020000,  32, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x00000000,   8, 4, CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x00062000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS +0x00063000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ +0x00060000,  8,  4,CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ +0x000d8600,  4,  4,CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00020000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00030000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00060000,  32, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00040000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00050000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00001600,   4, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ+0x00001700,   4, 4, CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00041000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00044000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_OCTET_COUNTERS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00042000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00080000,  16, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_SMU+0x00050000,   4, 4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_TO_ADVANCE_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x00180000,   8,4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x000C0000,   8, 4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x000D0000,   4, 4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x000D8000,   8, 4,CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x000d8200,   4, 4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x00060000,   8, 4,CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x000d8600,   4, 4,CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+1),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x00180000,   8, 4,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x000C0000,   8, 4, 2+CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x000D0000,   4, 4, 2+CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x000D8000,   8, 4, 2+CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x000d8200,   4, 4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x00060000,   8, 4,CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x000d8600,   4, 4,CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+2),


 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x00180000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x000C0000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x000D0000,   4, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x000D8000,   8, 4, CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x000d8200,   4, 4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x00060000,   8, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x000d8600,   4, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+3),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x007F0000,   4, 4, CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x007F8000,  32, 4, CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x007F4000,   4, 4, CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x007F6000,   4, 4, CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x00040000,  16, 4, CPSS_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PHA+0x007C0000,   4, 4, CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E),
/* those of 'extrnal DRAM' 'DP[8]' */
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+4),
 PRV_DIRECT_INFO_MAC(0,   0, 4, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+4),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x04000000,  64, 4, CPSS_DXCH_SIP7_TABLE_HA_ARP_E),

 PRV_DIRECT_INFO_MISSING_IN_DP1_TXQ_MAC,
 PRV_DIRECT_INFO_MISSING_IN_DP2_TXQ_MAC,
 PRV_DIRECT_INFO_MISSING_IN_DP3_TXQ_MAC,

 PRV_DIRECT_INFO_TILE_1_TXQ_MAC(1/*tile*/,0/*local DP*/),
 PRV_DIRECT_INFO_TILE_1_TXQ_MAC(1/*tile*/,1/*local DP*/),
 PRV_DIRECT_INFO_TILE_1_TXQ_MAC(1/*tile*/,2/*local DP*/),
 PRV_DIRECT_INFO_TILE_1_TXQ_MAC(1/*tile*/,3/*local DP*/),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00020000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 16),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00040000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 17),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00060000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 18),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00080000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 19),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000a0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 20),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000c0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 21),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x000e0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 22),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00100000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 23),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00120000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 24),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00140000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 25),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00160000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 26),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00180000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 27),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001a0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 28),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001c0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 29),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x001e0000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 30),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_CNC0+0x00200000+SIP7_CNC_64_OFFSET,   CNC_NUM_WORDS*4, 4, CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 31),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_MPCL+0x00010000,   4, 4, CPSS_DXCH_SIP7_TABLE_MIDWAY_PCL_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MPCL+0x00003000,   4, 4, CPSS_DXCH_SIP7_TABLE_MIDWAY_PCL_SOURCE_PORT_CONFIG_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_LPM+0x00F80000,   16, 4, CPSS_DXCH_SIP7_TABLE_VRF_COMMON_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS+0x000f0000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS+0x000f0000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS+0x000f0000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000f0000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+3),

 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_TABLE_INDEX_TILE_OFFSET(1,3)),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_PSI+0x00000000                       ,4, 4, CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E),
 PRV_DIRECT_INFO_MAC(0 ,4, 4, CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E+1),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00400000, 8, 4, CPSS_DXCH_SIP7_TABLE_VPORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x00300000, 4, 4, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_STATUS_VECTOR_E),
 PRV_DIRECT_INFO_MAC(HECMP_BASE_ADDR, 8, 4, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_LTT_MEM_E),
 PRV_DIRECT_INFO_MAC(HECMP_BASE_ADDR + 0x100000, 32, 4, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_ECMP_MEM_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00200000,  8, 4, CPSS_DXCH_SIP7_TABLE_HA_SRC_EPORT_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x000E0000,  4, 4, CPSS_DXCH_SIP7_TABLE_HA_SRC_EPORT_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00018000,  4, 4, CPSS_DXCH_SIP7_TABLE_HA_SRC_PHYSICAL_PORT_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00300000, 32, 4, CPSS_DXCH_SIP7_TABLE_HA_TARGET_CIRCUITS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x0000D000,  8, 4, CPSS_DXCH_SIP7_TABLE_HA_ANALYZERS_TARGET_CIRCUITS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00400000, 16, 4, CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x00010000, 16, 4, CPSS_DXCH_SIP7_TABLE_HA_QOS_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x02000000,128, 4, CPSS_DXCH_SIP7_TABLE_HA_HEADER_FIELDS_0_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_HA+0x01000000,128, 4, CPSS_DXCH_SIP7_TABLE_HA_HEADER_FIELDS_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPVX+0x00070000,16,  4, CPSS_DXCH_SIP7_TABLE_IPVX_ROUTER_L3NHE_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EM_ILM0+0x00000010,   32, 4, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_REDUCED_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_PDS+0x000f8000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_PDS+0x000f8000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_PDS+0x000f8000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_PDS+0x000f8000,8,4,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+3),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 PRV_DIRECT_INFO_MAC(0,   8, 4, CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),
#if /*CIDER_ON_EMULATOR_OR_GM */ defined GM_USED/* on GM/Emulator Cider 'CC-' */
 /* On GM the direct access not work */
#else
 PRV_DIRECT_INFO_MAC(AAS_ADDR_FDB+0x00000000,32/*32 bytes for 133 bits*/,4,CPSS_DXCH_TABLE_FDB_E),
#endif
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PREQ + 0x00004000,  4, 4, CPSS_DXCH_SIP7_PREQ_PORT_PROFILE_BIND_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PPU2 + 0x00003000, 64, 4, CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_0_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PPU2 + 0x00005000, 64, 4, CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PPU2 + 0x00007000, 64, 4, CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_2_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_PPU2 + 0x00009000, 64, 4, CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_3_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x00100000, 16, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_ADDRESS_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x00200000, 8, 4, CPSS_DXCH_SIP7_TABLE_IPE_ICT_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x00110000, 64, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_DATA_CMD_PROFILE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x00300000, 256, 4, CPSS_DXCH_SIP7_TABLE_IPE_DPO_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x00400000, 256, 4, CPSS_DXCH_SIP7_TABLE_IPE_DPI_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x02000000, 16, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_T0_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_IPE + 0x03000000, 16, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_T1_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_QAG+0x00200000,4,4,CPSS_DXCH_SIP7_EGF_QAG_QUEUE_MAP_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EGF_EFT + 0x00000000, 4, 4, CPSS_DXCH_SIP7_EGF_EFT_EVLAN_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MLL+0x00000000,  128, 4, CPSS_DXCH_SIP7_TABLE_IP_MLL_BIER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_MLL+0x00000000,  128, 4, CPSS_DXCH_SIP7_TABLE_L2_MLL_BIER_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_1+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+1   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_1+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_2+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+2   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_2+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_3+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+3   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_3+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+3),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_4+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+4   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_4+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+4),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_5+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+5   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_5+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+5),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_6+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+6   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_6+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+6),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_7+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+7   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP0_LMU_7+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+7),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_0+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+8   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_0+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+8),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_1+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+9   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_1+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+9),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_2+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+10  ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_2+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+10),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_3+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+11   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_3+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+11),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_4+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+12   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_4+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+12),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_5+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+13   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_5+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+13),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_6+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+14   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_6+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+14),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_7+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+15   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP1_LMU_7+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+15),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_0+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+16   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_0+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+16),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_1+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+17   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_1+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+17),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_2+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+18   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_2+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+18),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_3+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+19   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_3+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+19),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_4+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+20   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_4+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+20),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_5+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+21   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_5+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+21),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_6+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+22   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_6+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+22),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_7+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+23   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP2_LMU_7+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+23),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_0+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+24   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_0+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+24),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_1+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+25   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_1+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+25),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_2+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+26   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_2+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+26),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_3+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+27   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_3+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+27),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_4+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+28   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_4+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+28),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_5+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+29   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_5+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+29),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_6+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+30   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_6+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+30),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_7+0x00000000,  32, 4, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+31   ),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_GOP3_LMU_7+0x00004000,  16, 4, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+31),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_EQ + 0x000F0000,   4, 4,   CPSS_DXCH_SIP7_TABLE_EQ_TRUNK_ECMP_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00004000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00010000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00018000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00020000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00028000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00030000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00038000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00040000,   4, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_EOAM+0x00060000,   8, 4, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x00180000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x00180000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x00180000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x00180000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+3),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP0_SDQ+0x00180000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP1_SDQ+0x00180000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP2_SDQ+0x00180000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP3_SDQ+0x00180000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_HBU + 0x00000800 , 4,4, CPSS_DXCH_SIP7_TABLE_HBU_PORT_MAPPING_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_SDQ+0x00200000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_SDQ+0x00200000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_SDQ+0x00200000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_SDQ+0x00200000,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+3),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP0_SDQ+0x00200000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP1_SDQ+0x00200000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP2_SDQ+0x00200000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 PRV_DIRECT_INFO_MAC(0/*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP3_SDQ+0x00200000*/,8,4,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E + TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),

 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP0_QFC +0x00008000,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP1_QFC +0x00008000,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+1),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP2_QFC +0x00008000,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+2),
 PRV_DIRECT_INFO_MAC(AAS_ADDR_DP3_QFC +0x00008000,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+3),

 PRV_DIRECT_INFO_MAC(0 /*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP0_QFC +0x00008000*/,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 PRV_DIRECT_INFO_MAC(0 /*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP1_QFC +0x00008000*/,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 PRV_DIRECT_INFO_MAC(0 /*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP2_QFC +0x00008000*/,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 PRV_DIRECT_INFO_MAC(0 /*AAS_TILE_OFFSET_CNS+AAS_ADDR_DP3_QFC +0x00008000*/,4,4,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3))


};

#define PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP1_TXQ_MAC()  \
 AAS_DIRECT_MAC(   8944,   1,  CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E + 1)

#define PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP2_TXQ_MAC()  \
 AAS_DIRECT_MAC(   8944,   1,  CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E + 2)

#define PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP3_TXQ_MAC()                    \
    /**/                                                                        \
 AAS_DIRECT_MAC(   4080,   3, CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E + 3),      \
 AAS_DIRECT_MAC(   3452,   1, CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E + 3),         \
 AAS_DIRECT_MAC(   8944,   1, CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E + 3),         \
 AAS_DIRECT_MAC(    2048,   1, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E + 3),         \
 AAS_DIRECT_MAC(    2048,   1, CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E + 3),         \
 AAS_DIRECT_MAC(    2048,   1, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E + 3),         \
 AAS_DIRECT_MAC(    2048,   1, CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E + 3),         \
 AAS_DIRECT_MAC(    2080,   1, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E + 3),         \
 AAS_DIRECT_MAC(    2080,   1, CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E + 3),         \
   /*SDQ0*/                                                                             \
 AAS_DIRECT_MAC(   2048,   2,  CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 3)

#define PRV_DIRECT_TABLE_SIZE_INFO_TILE_1_TXQ_MAC(_tileId/*tile*/,_dp/*local DP*/)         \
    /*PDS0*/                                                                                                                   \
 AAS_DIRECT_MAC(   0,   3,  CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E    + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   0,   1,  CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   0,   1,  CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E       + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),      \
 AAS_DIRECT_MAC(   0,   2, CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E  + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
  /*SDQ0*/                                                                                                                          \
 AAS_DIRECT_MAC(   0,  2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E          + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,  2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E    + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,  1, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E   + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,    2,  CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E     + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E  + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E   + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp)),   \
 AAS_DIRECT_MAC(   0,   2,  CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E         + TXQ_TABLE_INDEX_TILE_OFFSET(_tileId,_dp))


/* Number of entries in Direct tables info array */
const GT_U32 prvCpssDxChAas_directInfoArrSize = sizeof(prvCpssDxChAas_directInfoArr)/sizeof(prvCpssDxChAas_directInfoArr[0]);


/* Tables info array  - NOTE : MUST be at the same order as prvCpssDxChAas_directInfoArr*/
const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC prvCpssDxChAas_tablesInfoArr[] =
{
 AAS_DIRECT_MAC(  16*NUM_DEF_EPORTS,   1, CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E),
 AAS_DIRECT_MAC(     32,   1,    CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E),
 AAS_DIRECT_MAC(    256,   1,    CPSS_DXCH_TABLE_CPU_CODE_E),
 AAS_DIRECT_MAC(   6*_1K,  2,    CPSS_DXCH_TABLE_PCL_CONFIG_E),
 AAS_DIRECT_MAC(   1024,   1,    CPSS_DXCH_TABLE_QOS_PROFILE_E),
 AAS_DIRECT_MAC(   NUM_VIDXS,   (NUM_PHY_PORTS/32),    CPSS_DXCH_TABLE_MULTICAST_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,   CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E),
 AAS_DIRECT_MAC(   _4K + NUM_PHY_PORTS/*1K*/,   1,   CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E),
 AAS_DIRECT_MAC(    NUM_TS/8, 50,   CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E),
 AAS_DIRECT_MAC(     256,   6,   CPSS_DXCH3_TABLE_MAC2ME_E),
 AAS_DIRECT_MAC(   NUM_4K_VLANS,   1,   CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,  CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E),
 AAS_DIRECT_MAC(     16,  23,   CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E),
 AAS_DIRECT_MAC(     80,   2,   CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E),
 AAS_DIRECT_MAC(   NUM_PORT_ISOLATION,   (NUM_PHY_PORTS/32),  CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E),
 AAS_DIRECT_MAC(   NUM_PORT_ISOLATION,   (NUM_PHY_PORTS/32),  CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   7,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E),
 AAS_DIRECT_MAC(   NUM_PLR_IPFIX_OR_COUNTERS,   8,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E),
 AAS_DIRECT_MAC(   6*_1K,   2,  CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E),
 AAS_DIRECT_MAC(     32,   5,  CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   4,  CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E),
 AAS_DIRECT_MAC(   NUM_4K_VLANS,   (NUM_PHY_PORTS/32), CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E),
 AAS_DIRECT_MAC(   NUM_STGS,   2*(NUM_PHY_PORTS/32),   CPSS_DXCH_LION_TABLE_STG_INGRESS_E),
 AAS_DIRECT_MAC(   NUM_STGS,   (NUM_PHY_PORTS/32),  CPSS_DXCH_LION_TABLE_STG_EGRESS_E),
 AAS_DIRECT_MAC(   NUM_SSTS,   (NUM_PHY_PORTS/32),  CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E),
 AAS_DIRECT_MAC(    256,   (NUM_PHY_PORTS/32), CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E),
 AAS_DIRECT_MAC(    256,   (NUM_PHY_PORTS/32),   CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E),
 AAS_DIRECT_MAC(   NUM_PHY_PORTS,   3,  CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E),
 AAS_DIRECT_MAC(   NUM_DEF_EPORTS,   6,  CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E),
 AAS_DIRECT_MAC(   NUM_EPORTS*2,   1,   CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   3,   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E),
 AAS_DIRECT_MAC(    NUM_EPORTS/8,   2,   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E),
 AAS_DIRECT_MAC(    NUM_TRUNKS/8,   2,   CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E),
 AAS_DIRECT_MAC(   NUM_PHY_PORTS,   1,   CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E),
 AAS_DIRECT_MAC(   0/*no such table*/,   1,   CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E),
 AAS_DIRECT_MAC(   NUM_EPORTS/4,   2,   CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E),
 AAS_DIRECT_MAC(   NUM_MIRROR_PORTS/8,  3,  CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E),
 AAS_DIRECT_MAC(   NUM_TRUNKS,   2,   CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   2,   CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E),
 AAS_DIRECT_MAC(   NUM_L2_ECMP,   1,  CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   2,   CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E),
 AAS_DIRECT_MAC(    256,   1,         CPSS_DXCH_SIP5_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E),
 AAS_DIRECT_MAC(   NUM_EPORTS/32,   1,  CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E),
 AAS_DIRECT_MAC(   NUM_EPORTS/2,   1,  CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E),
 AAS_DIRECT_MAC(    512,   1,  CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E),
 AAS_DIRECT_MAC( (LPM_MAX_NUM_OF_BLOCKS*LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS), 5, CPSS_DXCH_SIP5_TABLE_LPM_MEM_E),
 AAS_DIRECT_MAC(   0,   1,   CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E),
 AAS_DIRECT_MAC(   0,   1,   CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E),
 AAS_DIRECT_MAC(   0,   1,   CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   2,    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E),
 AAS_DIRECT_MAC(  (NUM_PHY_PORTS/16)*NUM_4K_VLANS,   2,  CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   2,    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   1,   CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E),
 AAS_DIRECT_MAC(   _8K,   1,   CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   4,    CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E),
 AAS_DIRECT_MAC(   NUM_EPORTS,   1,   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   3,   CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   4,   CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E),
 AAS_DIRECT_MAC(   1024,   1,   CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E),
 AAS_DIRECT_MAC(     16,  23,   CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E),
 AAS_DIRECT_MAC(    5*NUM_PHY_PORTS,   2, CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E),
 AAS_DIRECT_MAC(      32,  18,   CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E),
 AAS_DIRECT_MAC(    4096,   2,   CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E),
/*   each line hold 4 ip/l2 mll pairs entries */
 AAS_DIRECT_MAC(   NUM_MLL_PAIRS/4,   24,   CPSS_DXCH_SIP5_TABLE_IP_MLL_E),
 AAS_DIRECT_MAC(   NUM_MLL_PAIRS/4,   24,   CPSS_DXCH_SIP5_TABLE_L2_MLL_E),
 AAS_DIRECT_MAC(   NUM_L2_LTT_MLL,   1, CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E),
 AAS_DIRECT_MAC(    0/*no such table*/,   1,   CPSS_DXCH_SIP5_TABLE_ADJACENCY_E),
 AAS_DIRECT_MAC(   8192,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E),
 AAS_DIRECT_MAC(   1024,   2,   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E),
/*not exists*/ AAS_DIRECT_MAC(   0 ,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E),
 AAS_DIRECT_MAC(   8192,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E),
 AAS_DIRECT_MAC(   1024,   2,   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E),
 AAS_DIRECT_MAC(   8192,   1,   CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E),
 AAS_DIRECT_MAC(   NUM_EPORTS/16,   4,   CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   3,   CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E),
 AAS_DIRECT_MAC(   49152,  5,  CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E),
 AAS_DIRECT_MAC(   49152/32,  1,   CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E),
 AAS_DIRECT_MAC(    192,   4,   CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E),
 AAS_DIRECT_MAC(    256,   1,   CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E),
 AAS_DIRECT_MAC(    256,   2,   CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E),
 AAS_DIRECT_MAC(    128,   1,   CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E),
 AAS_DIRECT_MAC(     72,  20,   CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E),
 AAS_DIRECT_MAC(     72,   5,   CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E),
 AAS_DIRECT_MAC(     72,   3,   CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E),
 AAS_DIRECT_MAC(     72,  12,   CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E),
 AAS_DIRECT_MAC(     20,  12,   CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E),
 AAS_DIRECT_MAC(    112,  16,   CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E),
 AAS_DIRECT_MAC(  TCAM_ACTIONS_HW,  11/*for 350 bits*/,  CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E),
 AAS_DIRECT_MAC(    128,   2,   CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E),
 AAS_DIRECT_MAC(   2048,   1,   CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,  11, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,  11, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E),
 AAS_DIRECT_MAC(    192,   1,  CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E),
 AAS_DIRECT_MAC(    192,   1,  CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E),
 AAS_DIRECT_MAC(  TCAM_ACTIONS_HW*2/*X,Y*/,   3,  CPSS_DXCH_SIP5_TABLE_TCAM_E),
 AAS_DIRECT_MAC(   NUM_4K_VLANS,  (NUM_PHY_PORTS/32), CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,  CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E),
 AAS_DIRECT_MAC(   NUM_TRUNKS,   (NUM_PHY_PORTS/32), CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E),
 AAS_DIRECT_MAC(   1344,   1, CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 1),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 2),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 3),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 4),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 5),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 6),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 7),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 8),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 9),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 10),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 11),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 12),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 13),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 14),
 AAS_DIRECT_MAC(   CNC_NUM_IN_BLOCK,  CNC_NUM_WORDS,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 15),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   2,  CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   2,  CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   2,  CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   1,  CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   1,  CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E),
 AAS_DIRECT_MAC(   NUM_PLR_METERS,   1,  CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   8,  CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E),
 AAS_DIRECT_MAC(     64,   1,  CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E),
 AAS_DIRECT_MAC(    512,   1,  CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E),
 AAS_DIRECT_MAC(   1024,   1,  CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E),
 AAS_DIRECT_MAC(   49152,  3,  CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E),
 AAS_DIRECT_MAC(     16,   5,  CPSS_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E),
 AAS_DIRECT_MAC(   TXQ_PDX_QUEUE_GROUP_MAP_NUM,   1,  CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E),
 AAS_DIRECT_MAC(   2048,   2,  CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E),
 AAS_DIRECT_MAC(   2048,   2,  CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 1),
 AAS_DIRECT_MAC(   2048,   2,  CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E + 2),
 AAS_DIRECT_MAC(   24552,   3,  CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E),
 AAS_DIRECT_MAC(   24552,   3,  CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E + 1),
 AAS_DIRECT_MAC(   24552,   3,  CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E + 2),
 AAS_DIRECT_MAC(   15868,   1,  CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E),
 AAS_DIRECT_MAC(   15868,   1,  CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E + 1),
 AAS_DIRECT_MAC(   15868,   1,  CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E + 2),
 AAS_DIRECT_MAC(   63472,   1,  CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E + 1),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E + 2),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E + 1),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E + 2),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E + 1),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E + 2),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E + 1),
 AAS_DIRECT_MAC(    2048,   1,  CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E + 2),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E + 1),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E + 2),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E + 1),
 AAS_DIRECT_MAC(    2080,   1,  CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E + 2),
 AAS_DIRECT_MAC(    4096,  2,  CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E),
 AAS_DIRECT_MAC(    4096,  2,  CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E),
 AAS_DIRECT_MAC(    4096,  1,  CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E),
 AAS_DIRECT_MAC(    23,    2,  CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E),
 AAS_DIRECT_MAC(   210,   1,  CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E),
 AAS_DIRECT_MAC(   NUM_PHY_PORTS,   1, CPSS_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E),
 AAS_DIRECT_MAC(   NUM_EVIDS,   1,  CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E),
 AAS_DIRECT_MAC(   CNC_QUEUE_PORT_MAPPING_MEMORY_NUM,   1, CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E),
 AAS_DIRECT_MAC(    768,   3,  CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E),
 AAS_DIRECT_MAC(   1024,  11,  CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E),
 AAS_DIRECT_MAC(     32,   4, CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E),
 AAS_DIRECT_MAC(    512,   5,  CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E),
 AAS_DIRECT_MAC(    512,   5,  CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E),
 AAS_DIRECT_MAC(    512,   5,  CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E),
 AAS_DIRECT_MAC(    512,   5,  CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E),
 AAS_DIRECT_MAC(    16384, 2,  CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E),
 AAS_DIRECT_MAC(   1639,   5,  CPSS_DXCH_SIP6_TABLE_PBR_E),
 AAS_DIRECT_MAC(   983040,   1,  CPSS_DXCH_SIP6_TABLE_LPM_AGING_E),
 AAS_DIRECT_MAC(    512,   8, CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E),
 AAS_DIRECT_MAC(    512,   3, CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E),
 AAS_DIRECT_MAC(  16*_1K,  3, CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E),
 AAS_DIRECT_MAC(     NUM_PLR_IPFIX_OR_COUNTERS/32,   1,  CPSS_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1, CPSS_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1, CPSS_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E),
 AAS_DIRECT_MAC(    NUM_PHY_PORTS,   1, CPSS_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E),
 AAS_DIRECT_MAC(    22,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E),
 AAS_DIRECT_MAC(    112,   1,  CPSS_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E),
 AAS_DIRECT_MAC(    112,   1,  CPSS_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E),
 AAS_DIRECT_MAC(   2048,   2,  CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E),
 AAS_DIRECT_MAC(   2048,   5,  CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E),
 AAS_DIRECT_MAC(  25064,   2,  CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC_E),
 AAS_DIRECT_MAC(    456,   1,  CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER_E),
 AAS_DIRECT_MAC(    456,   1,  CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER_E),
 AAS_DIRECT_MAC(   3072,   2,  CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E),
 AAS_DIRECT_MAC(     23,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E),
 AAS_DIRECT_MAC(   2048,   3,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E),
 AAS_DIRECT_MAC(   1024,   3,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E),
 AAS_DIRECT_MAC(   1024,   5,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E),
 AAS_DIRECT_MAC(   1024,   3,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E),
 AAS_DIRECT_MAC(   1024,   4,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E),
 AAS_DIRECT_MAC(     32,   1,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E),
 AAS_DIRECT_MAC(     32,   1,  CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E),
 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E),
 AAS_DIRECT_MAC(    464,   1,  CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_OCTET_COUNTERS_E),
 AAS_DIRECT_MAC(     58,   4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E),
 AAS_DIRECT_MAC(  14848,   4, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E),
 AAS_DIRECT_MAC(  14848,   1, CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_TO_ADVANCE_E),

 AAS_DIRECT_MAC(    4096,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+1),
 AAS_DIRECT_MAC(    4096,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+1),
 AAS_DIRECT_MAC(    4096,   1, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+1),
 AAS_DIRECT_MAC(    23,    2, CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+1),
 AAS_DIRECT_MAC(    23,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+1),
 AAS_DIRECT_MAC(   3072,   2,  CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+1),
 AAS_DIRECT_MAC(     23,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+1),

 AAS_DIRECT_MAC(    4096,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+2),
 AAS_DIRECT_MAC(    4096,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+2),
 AAS_DIRECT_MAC(    4096,   1, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+2),
 AAS_DIRECT_MAC(    23,    2,  CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+2),
 AAS_DIRECT_MAC(    23,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+2),
 AAS_DIRECT_MAC(   3072,   2,  CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+2),
 AAS_DIRECT_MAC(     23,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+2),

 AAS_DIRECT_MAC(    4096,   2,CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+3),
 AAS_DIRECT_MAC(    4096,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+3),
 AAS_DIRECT_MAC(    4096,   1, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+3),
 AAS_DIRECT_MAC(    23,    2,  CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+3),
 AAS_DIRECT_MAC(    23,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+3),
 AAS_DIRECT_MAC(   3072,   2,  CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+3),
 AAS_DIRECT_MAC(     23,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+3),

 AAS_DIRECT_MAC(    256,   1,  CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E),
 AAS_DIRECT_MAC(    256,   5,  CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E),
 AAS_DIRECT_MAC(   NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E),
 AAS_DIRECT_MAC(   NUM_PHY_PORTS,   1,  CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E),
 AAS_DIRECT_MAC(   1024,   4,  CPSS_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E),

 AAS_DIRECT_MAC( 0,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+4),
 AAS_DIRECT_MAC( 0,   2, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+4),
 AAS_DIRECT_MAC( 0,   1, CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+4),
 AAS_DIRECT_MAC( 0,    2, CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+4),
 AAS_DIRECT_MAC( 0,    1 , CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E+4),
 AAS_DIRECT_MAC( 0,   2, CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E+4),
 AAS_DIRECT_MAC( 0,   1, CPSS_DXCH_SIP6_30_TXQ_SDQ_PORT_QBV_STATE_E+4),

 AAS_DIRECT_MAC(    NUM_ARPS/8, 12,  CPSS_DXCH_SIP7_TABLE_HA_ARP_E),

 PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP1_TXQ_MAC(),/* 1 index */

 PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP2_TXQ_MAC(),/* 1 index */

 PRV_DIRECT_TABLE_SIZE_INFO_MISSING_IN_DP3_TXQ_MAC(),/*10 indexes*/

 PRV_DIRECT_TABLE_SIZE_INFO_TILE_1_TXQ_MAC(1/*tile*/,0/*local DP*/), /*18 indexes*/
 PRV_DIRECT_TABLE_SIZE_INFO_TILE_1_TXQ_MAC(1/*tile*/,1/*local DP*/), /*18 indexes*/
 PRV_DIRECT_TABLE_SIZE_INFO_TILE_1_TXQ_MAC(1/*tile*/,2/*local DP*/), /*18 indexes*/
 PRV_DIRECT_TABLE_SIZE_INFO_TILE_1_TXQ_MAC(1/*tile*/,3/*local DP*/), /*18 indexes*/

 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 16),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 17),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 18),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 19),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 20),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 21),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 22),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 23),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 24),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 25),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 26),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 27),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 28),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 29),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 30),
 AAS_DIRECT_MAC(CNC_NUM_IN_BLOCK,   CNC_NUM_WORDS ,  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 31),

 AAS_DIRECT_MAC(_4K + NUM_PHY_PORTS/*1K*/, 1, CPSS_DXCH_SIP7_TABLE_MIDWAY_PCL_CONFIG_E),
 AAS_DIRECT_MAC(NUM_PHY_PORTS/*1K*/,       1,  CPSS_DXCH_SIP7_TABLE_MIDWAY_PCL_SOURCE_PORT_CONFIG_E),

 AAS_DIRECT_MAC(NUM_VRFS,   3,  CPSS_DXCH_SIP7_TABLE_VRF_COMMON_E),

 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+1),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+2),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+3),
 /*AAS_DIRECT_MAC(4096,2,384,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+4)*/

 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),
 AAS_DIRECT_MAC(131072,1,CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E),
 AAS_DIRECT_MAC(0,1,CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E+1),
 AAS_DIRECT_MAC(NUM_VPORTS, 2, CPSS_DXCH_SIP7_TABLE_VPORT_MAPPING_E),
 AAS_DIRECT_MAC(1024,  1,   CPSS_DXCH_SIP7_TABLE_EQ_HECMP_STATUS_VECTOR_E),
 AAS_DIRECT_MAC(NUM_HECMP_LTT,  2,  CPSS_DXCH_SIP7_TABLE_EQ_HECMP_LTT_MEM_E),
 AAS_DIRECT_MAC(NUM_HECMP_ECMP, 5,   CPSS_DXCH_SIP7_TABLE_EQ_HECMP_ECMP_MEM_E),
 AAS_DIRECT_MAC(NUM_EPORTS,    2,  CPSS_DXCH_SIP7_TABLE_HA_SRC_EPORT_1_E),
 AAS_DIRECT_MAC(NUM_EPORTS,    1,  CPSS_DXCH_SIP7_TABLE_HA_SRC_EPORT_2_E),
 AAS_DIRECT_MAC(NUM_PHY_PORTS, 1,  CPSS_DXCH_SIP7_TABLE_HA_SRC_PHYSICAL_PORT_2_E),
 AAS_DIRECT_MAC(NUM_CIRCUITS,  8,  CPSS_DXCH_SIP7_TABLE_HA_TARGET_CIRCUITS_E),
 AAS_DIRECT_MAC(NUM_ANALYZERS, 2,  CPSS_DXCH_SIP7_TABLE_HA_ANALYZERS_TARGET_CIRCUITS_E),
 AAS_DIRECT_MAC(16384,         3,  CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E),
 AAS_DIRECT_MAC(576,           3,  CPSS_DXCH_SIP7_TABLE_HA_QOS_MAPPING_E),
 AAS_DIRECT_MAC(131072,       25,  CPSS_DXCH_SIP7_TABLE_HA_HEADER_FIELDS_0_E),
 AAS_DIRECT_MAC(131072,       25,  CPSS_DXCH_SIP7_TABLE_HA_HEADER_FIELDS_1_E),
 AAS_DIRECT_MAC(256, 3,  CPSS_DXCH_SIP7_TABLE_IPVX_ROUTER_L3NHE_PROFILE_E),
 AAS_DIRECT_MAC(NUM_REDUCED_EM, 5,  CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_REDUCED_E),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+1),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+2),
 AAS_DIRECT_MAC(4096,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+3),
 AAS_DIRECT_MAC(0,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 AAS_DIRECT_MAC(0,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 AAS_DIRECT_MAC(0,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 AAS_DIRECT_MAC(0,2,CPSS_DXCH_SIP7_TXQ_PDS_FLEXE_CHANNEL_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),

#if /*CIDER_ON_EMULATOR_OR_GM */ defined GM_USED/* on GM/Emulator Cider 'CC-' */
 /* On GM the direct access not work */
#else
 AAS_DIRECT_MAC(NUM_FDB,5/*133 bits*/,CPSS_DXCH_TABLE_FDB_E),
#endif
 AAS_DIRECT_MAC(1024,   1, CPSS_DXCH_SIP7_PREQ_PORT_PROFILE_BIND_E),
 AAS_DIRECT_MAC(16, 12,  CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_0_E),
 AAS_DIRECT_MAC(16, 12,  CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_1_E),
 AAS_DIRECT_MAC(16, 12,  CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_2_E),
 AAS_DIRECT_MAC(16, 12,  CPSS_DXCH_SIP7_TABLE_PPU_FOR_IPE_ACTION_TABLE_3_E),
 AAS_DIRECT_MAC(128, 3,  CPSS_DXCH_SIP7_TABLE_IPE_STATE_ADDRESS_PROFILE_E),
 AAS_DIRECT_MAC(65536, 2, CPSS_DXCH_SIP7_TABLE_IPE_ICT_E),
 AAS_DIRECT_MAC(128, 13,  CPSS_DXCH_SIP7_TABLE_IPE_STATE_DATA_CMD_PROFILE_E),
 AAS_DIRECT_MAC(32, 33, CPSS_DXCH_SIP7_TABLE_IPE_DPO_E),
 AAS_DIRECT_MAC(256, 38, CPSS_DXCH_SIP7_TABLE_IPE_DPI_E),
 AAS_DIRECT_MAC(1048576, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_T0_E),
 AAS_DIRECT_MAC(1048576, 4, CPSS_DXCH_SIP7_TABLE_IPE_STATE_T1_E),
 AAS_DIRECT_MAC(131072,1,CPSS_DXCH_SIP7_EGF_QAG_QUEUE_MAP_TABLE_E),
 AAS_DIRECT_MAC(NUM_EVIDS, 1,  CPSS_DXCH_SIP7_EGF_EFT_EVLAN_E),
/*   each line hold a single ip/l2 mll bier pairs entry */
 AAS_DIRECT_MAC(   NUM_MLL_PAIRS/4,   24,   CPSS_DXCH_SIP7_TABLE_IP_MLL_BIER_E),
 AAS_DIRECT_MAC(   NUM_MLL_PAIRS/4,   24,   CPSS_DXCH_SIP7_TABLE_L2_MLL_BIER_E),

 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+1),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+1),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+2),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+2),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+3),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+3),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+4),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+4),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+5),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+5),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+6),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+6),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+7),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+7),

 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+8),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+8),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+9),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+9),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+10),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+10),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+11),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+11),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+12),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+12),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+13),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+13),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+14),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+14),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+15),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+15),

 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+16),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+16),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+17),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+17),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+18),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+18),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+19),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+19),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+20),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+20),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+21),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+21),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+22),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+22),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+23),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+23),

 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+24),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+24),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+25),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+25),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+26),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+26),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+27),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+27),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+28),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+28),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+29),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+29),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+30),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+30),
 AAS_DIRECT_MAC(    512,   8,  CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E+31),
 AAS_DIRECT_MAC(    512,   3,  CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E+31),
 AAS_DIRECT_MAC(NUM_TRUNK_ECMP,1, CPSS_DXCH_SIP7_TABLE_EQ_TRUNK_ECMP_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E),
 AAS_DIRECT_MAC(   2048,   1,  CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E),
 AAS_DIRECT_MAC(   256,   2, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+1),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+2),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+3),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),
 AAS_DIRECT_MAC(    512,  1,CPSS_DXCH_SIP7_TABLE_HBU_PORT_MAPPING_E),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+1),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+2),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+3),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 AAS_DIRECT_MAC(   4096,  2,CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+1),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+2),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+3),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,0)),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,1)),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,2)),
 AAS_DIRECT_MAC(   8192,  1,CPSS_DXCH_SIP7_TXQ_QFC_LOGICAL_Q_TO_PROFILE_MAPPING_TABLE_E+ TXQ_SIP_7_TABLE_INDEX_TILE_OFFSET(1,3)),

 /****************************************************/
/* keep the 'indirect tables ' as LAST in the array */
/****************************************************/
#if /*CIDER_ON_EMULATOR_OR_GM */ defined GM_USED/* on GM/Emulator Cider 'CC-' */
 /* On GM the direct access not work */
 AAS_INDIRECT_MAC(  NUM_FDB,   5, 0, CPSS_DXCH_TABLE_FDB_E), /*--> moved to be 'direct table' */
#endif
 AAS_INDIRECT_MAC(  NUM_OAM,   4,  1, CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E),
 AAS_INDIRECT_MAC(  NUM_OAM,   4,  2, CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  3, CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  4, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_1_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  5, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_2_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  6, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_3_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  7, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_4_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  8, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_5_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5,  9, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_6_E),
 AAS_INDIRECT_MAC(   NUM_EM,   5, 10, CPSS_DXCH_SIP7_TABLE_EXACT_MATCH_7_E)
};

/* Number of entries in Tables info array */
const GT_U32 prvCpssDxChAas_tablesInfoArrSize = sizeof(prvCpssDxChAas_tablesInfoArr)/sizeof(prvCpssDxChAas_tablesInfoArr[0]);

/* list the TAI '0' units in the Aas */
static const PRV_CPSS_DXCH_UNIT_ENT  aasTai0UnitsArray[] =
{
    PRV_CPSS_DXCH_UNIT_TAI_E
};

GT_STATUS prvCpssAasNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
)
{
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum].prvCpssHwsUnitBaseAddrCalcFunc)        = aasUnitBaseAddrCalc;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum].prvCpssRegDbInfoGetFunc)               = prvCpssAasRegDbInfoGet;
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssAasDmaGlobalNumToLocalNumInDpConvert;
    DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(devNum)= prvCpssAasDmaLocalNumInDpToGlobalNumConvert;
    PRV_HW_REG_TO_UNIT_ID(devNum) = prvCpssDxChAasHwRegAddrToUnitIdConvert;
    PRV_DMA_DP_LOCAL_TO_GLOBAL_NUM_CONVERT_GET_NEXT(devNum)= prvCpssAasDmaLocalNumInDpToGlobalNumConvert_getNext;
    PRV_OFFSET_FROM_FIRST_INSTANCE_GET(devNum)= prvCpssAasOffsetFromFirstInstanceGet;
    PRV_CPSS_UNIT_ID_SIZE_IN_BYTE_GET(devNum)= prvCpssAasUnitIdSizeInByteGet;

    return GT_OK;
}

static GT_STATUS gdmaAasInitParamsSet
(
    IN  GT_U8   devNum
)
{
    PRV_CPSS_DXCH_GDMA_INFO_STC *gdmaInfoPtr;

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* the GM device not supports the GDMA at all   */
        /* keep using the legacy MG and SDMA config     */
        PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) = GT_FALSE;

        return GT_OK;
    }

    gdmaInfoPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo;

    PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) = GT_TRUE;
    /*GDMA0,GDMA3 not pointed by the gdma Dispatcher ! */
    gdmaInfoPtr->gdmaDispatcherToUnitmap[0] = 1;/*GDMA1*/
    gdmaInfoPtr->gdmaDispatcherToUnitmap[1] = 2;/*GDMA2*/
    gdmaInfoPtr->gdmaDispatcherToUnitmap[2] = GT_NA;
    gdmaInfoPtr->gdmaDispatcherToUnitmap[3] = GT_NA;

    /* next flags will be remove , one by one as the implementation progress */
    gdmaInfoPtr->supportGdmaFor_AUQ       = GT_TRUE;
    gdmaInfoPtr->supportGdmaFor_FUQ_CNC   = GT_TRUE;

    /* values in tile 0 (for tile 1) values are '+ 4' */
    gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForAuq = 0;
    gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForFuq = 0;
    gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForCnc = 0;
    gdmaInfoPtr->gdmaClientsMsgInfo.gdmaUnitIdForIpe = 0;

    return GT_OK;
}

/**
* @internal prvCpssAasInitParamsSet function
* @endinternal
*
* @brief  Aas  : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      Aas.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssAasInitParamsSet
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  ii,jj,kk,mm,index;
    GT_U32  bmp;
    GT_U32  numMgUnitsPerTile = NUM_MGS_FOR_SDMA;
    GT_U32  numMgUnits = numMgUnitsPerTile * devPtr->multiPipe.numOfTiles;
    GT_U32  ePorts;/*number of eports*/
    GT_U32  numBitsVPorts;/*number of bits for vPorts*/
    GT_U32  auqPortGroupIndex = 0,fuqPortGroupIndex = 0;
    PRV_CPSS_DXCH_GDMA_CPU_PORT_NETIF_INFO_STC  *gdmaPortInfoPtr;

    rc = gdmaAasInitParamsSet(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits =
        PRV_CPSS_PP_MAC(devNum)->isGmDevice ? numMgUnits :
        0;/* No MG units */

    for(ii = 0 ; ii < devPtr->multiPipe.numOfTiles; ii++)
    {
        /* AUQ/FUQ per tile (same as 'per pipe') */

        /* each serve tile */
        PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp |= BIT_0 << auqPortGroupIndex;
        PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp |= BIT_0 << fuqPortGroupIndex;

        /* the port groups bmp are 'per pipe' so every pipe we have AUQ */
        auqPortGroupIndex ++;
        /* the port groups bmp are 'per pipe' so every pipe we have FUQ */
        fuqPortGroupIndex ++;
    }

    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgCoreClock         = 333333333;

    devPtr->multiPipe.tileOffset        = AAS_TILE_OFFSET_CNS;
    devPtr->multiPipe.mirroredTilesBmp  = 0;/* no mirroring of tiles */

    devPtr->multiPipe.numOfPipesPerTile = 1;/* 1 pipe per tile */
    devPtr->multiPipe.numOfPipes        =
        devPtr->multiPipe.numOfTiles *
        devPtr->multiPipe.numOfPipesPerTile;

    /* number of GOP ports per pipe , NOT including the 'CPU Port' . */
    devPtr->multiPipe.numOfPortsPerPipe = AAS_PORTS_PER_DP_CNS * NUM_OF_DP_UNITS;/* 4 DP[] per pipe */

    dxDevPtr->hwInfo.multiDataPath.supportMultiDataPath = 1;
    dxDevPtr->hwInfo.multiDataPath.rxNumPortsPerDp = AAS_PORTS_PER_DP_CNS+3; /* must be 23 by cider            */
    dxDevPtr->hwInfo.multiDataPath.txNumPortsPerDp = AAS_PORTS_PER_DP_CNS+3; /* add something and SDMA and EPB */

    dxDevPtr->hwInfo.multiDataPath.maxDp = devPtr->multiPipe.numOfPipes * NUM_OF_DP_UNITS;/* 4 DP[] per pipe */

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts =
        dxDevPtr->hwInfo.multiDataPath.maxDp * AAS_PORTS_PER_DP_CNS;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts / 8;/* segmented port per 8 MACs */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts = devPtr->multiPipe.numOfTiles;

    PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev =
        devPtr->multiPipe.numOfTiles *
        (SERDES_NUM_NETWORK_PORTS /*80*/+SERDES_NUM_CPU_PORTS/*1*/);

    /* led info */
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum = devPtr->multiPipe.numOfTiles *PRV_CPSS_DXCH_AAS_LED_UNIT_NUM_CNS;
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts   = devPtr->multiPipe.numOfTiles *PRV_CPSS_DXCH_AAS_LED_UNIT_PORTS_NUM_CNS;

    /* get info for 'SDMA CPU' port numbers */
    rc = aasSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS ; ii++)
    {
        dxDevPtr->gdmaInfo.gdmaNumQueues_TO_CPU[ii]    = aas_gdmaNumQueues_TO_CPU[ii];
        dxDevPtr->gdmaInfo.gdmaNumQueues_FROM_CPU[ii]  = aas_gdmaNumQueues_FROM_CPU[ii];
    }

    index = 0;
    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].globalMac == GLOBAL_MAC_SPECIAL_GDMA)  /* CPU GDMA (or SDMA for GM) */
        {
            /* this is valid 'SDMA CPU' port */
            if(PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
            {
                if(index >= (CPSS_DXCH_NETIF_GDMA_FROM_CPU_MAX_PORTS_CNS)/devPtr->multiPipe.numOfTiles)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "too many GDMA ports in specialDmaPortsPtr[]");
                }
            }
            else /* case for GM */
            {
                if(index >= NUM_MG_PER_CNM)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "too many SDMA ports in specialDmaPortsPtr[]");
                }
            }
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum =
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts + ii;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].valid  = GT_TRUE;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].usedAsCpuPort = GT_FALSE;/* the 'port mapping' should bind it */
            /* parameter called 'dqNum' but should be considered as 'DP index' */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dqNum  = specialDmaPortsPtr[ii].dpIndex;
            /* set local local DMA of the CPU ports */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].localDmaNum = AAS_PORTS_PER_DP_CNS + 2;

            if(!PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
            {
                /* only GM device uses SDMA , and single MG unit is enough */
                break;
            }

            gdmaPortInfoPtr = &dxDevPtr->gdmaInfo.cpuPortsArr[index];

            for(jj = 0 ; aas_convert_dpIndex_to_gdma_unit_id[jj].dpIndex != GT_NA ; jj++)
            {
                if(aas_convert_dpIndex_to_gdma_unit_id[jj].dpIndex ==
                   specialDmaPortsPtr[ii].dpIndex)
                {
                    bmp = aas_convert_dpIndex_to_gdma_unit_id[jj].gdmaUnitId_bmp_TO_CPU;
                    if(aas_convert_dpIndex_to_gdma_unit_id[jj].gdmaUnitId_FROM_CPU < 32)
                    {
                        bmp |= (1<<aas_convert_dpIndex_to_gdma_unit_id[jj].gdmaUnitId_FROM_CPU);
                    }
                    gdmaPortInfoPtr->numGdmaUnitsUsed = prvCpssPpConfigBitmapNumBitsGet(bmp);
                    /* to be init in cpssDxChGdmaCpuPortBindToQueuesSet(...) */
                    gdmaPortInfoPtr->index_gdmaUnitInfo_FROM_CPU = GT_NA;
                    mm = 0;
                    for(kk = 0 ; kk < PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS ; kk++)
                    {
                        if(bmp < (GT_U32)(1 << kk))
                        {
                            break;
                        }
                        if(0 == (bmp & (1 << kk)))
                        {
                            continue;
                        }

                        gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaUnitId = kk;

                        if(kk == aas_convert_dpIndex_to_gdma_unit_id[jj].gdmaUnitId_FROM_CPU)
                        {
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_FROM_CPU.gdmaLocalFirstQueue = 0;
                            /* to be init in cpssDxChGdmaCpuPortBindToQueuesSet(...) */
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_FROM_CPU.gdmaGlobalQueue = 0;
                            /* to be init in cpssDxChGdmaCpuPortBindToQueuesSet(...) */
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_FROM_CPU.numOfQueues     = 0;
                        }

                        if((1<<kk) & aas_convert_dpIndex_to_gdma_unit_id[jj].gdmaUnitId_bmp_TO_CPU)
                        {
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_TO_CPU.gdmaLocalFirstQueue = 0;
                            /* to be init in cpssDxChGdmaCpuPortBindToQueuesSet(...) */
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_TO_CPU.gdmaGlobalQueue = 0;
                            /* to be init in cpssDxChGdmaCpuPortBindToQueuesSet(...) */
                            gdmaPortInfoPtr->gdmaUnitInfo[mm].gdmaQueueInfo_TO_CPU.numOfQueues     = 0;
                        }

                        mm++;
                    }
                    break;
                }
            }

            index++;
        }
    }

    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = ii * AAS_PORTS_PER_DP_CNS;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = AAS_PORTS_PER_DP_CNS + 3;/* support CPU SDMA port + CPU/LP port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].loopbackPortDmaNum = LOOPBACK_PORT_DMA_CHANNEL;
    }
    if(PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        /* only DP[0..2] with cpu port DMA capability */
        dxDevPtr->hwInfo.multiDataPath.info[0].cpuPortDmaNum      = AAS_PORTS_PER_DP_CNS + 2;
        dxDevPtr->hwInfo.multiDataPath.info[1].cpuPortDmaNum      = AAS_PORTS_PER_DP_CNS + 2;
        dxDevPtr->hwInfo.multiDataPath.info[2].cpuPortDmaNum      = AAS_PORTS_PER_DP_CNS + 2;
        dxDevPtr->hwInfo.multiDataPath.info[3].cpuPortDmaNum      = GT_NA;
    }
    else
    {
        /* only DP[0] with cpu port DMA capability */
        dxDevPtr->hwInfo.multiDataPath.info[0].cpuPortDmaNum      = AAS_PORTS_PER_DP_CNS + 2;
        dxDevPtr->hwInfo.multiDataPath.info[1].cpuPortDmaNum      = GT_NA;
        dxDevPtr->hwInfo.multiDataPath.info[2].cpuPortDmaNum      = GT_NA;
        dxDevPtr->hwInfo.multiDataPath.info[3].cpuPortDmaNum      = GT_NA;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks = LPM_MAX_NUM_OF_BLOCKS;
    #ifdef GM_USED
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 16*_1K;/* each 2 blocks share 256K lines. at the moment we set it 16K for each bank */
    #else
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 128*_1K;/* each 2 blocks share 256K lines. at the moment we set it half for each bank */
    #endif
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap = LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = _2K;/* falcon : 640 lines , in non shared block */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  = 8;/* falcon : up to 20 blocks from non shared (to fill to total of 30) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = LPM_MAX_NUM_OF_BLOCKS;/* AAS : 48 blocks supported */
    /* next fields set only during prvCpssDxChSharedMemoryAasConnectClientsToMemory(...)
       that initialize the SHM clients
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip7_lpm_numSbmsPerBlock[0..39] */

    /* PHA info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg = 4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn = 10;

     /* IPE ppa info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ipePpaInfo.numOfPpg = 4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ipePpaInfo.numOfPpn = 12;


    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts = 23 * NUM_OF_DP_UNITS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues  = 400;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups = TXQ_PDX_QUEUE_GROUP_MAP_NUM;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxPizzaSize =  180;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxNumOfSpeedProfiles = 16;

    /* SBM info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm = 130 * devPtr->multiPipe.numOfTiles;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm = 0;/* not used*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.bmpSbmSupportMultipleArp = 0;/* not used*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm = _16K;/*64K in 4 SBMs*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort = 8;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numOfTcamProfiles      = 0 /* use default --> means 64 */;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.limitedNumOfParrallelLookups = 0;/* use default --> means 4 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.limitedNumOfParrallelLookups = 0; /* use default --> means 4 */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.numDefaultEports = NUM_DEF_EPORTS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_myPhysicalPortAttributes_numEntries = NUM_MY_PHYSICAL_PORT;/*per tile*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries = 256;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_ip2me_numEntries = 256;


    /* limit the number of PCLs to 2 instead of 3 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass = GT_TRUE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.limitedNumOfParrallelLookups = 0; /* use default --> means 4 */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId = 6;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors = TCAM_FLOORS;/* TBD to fit unit space*//* 24 *3K = 72K@10B = 36K@20B */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups = 6;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits =
        NUM_OF_DP_UNITS * NUM_OF_LMU_UNITS_PER_GOP * devPtr->multiPipe.numOfTiles;/* total num of LMUs */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.preqInfo.portMappingNum = 128;/*keep like other devices*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum = NUM_PLR_IPFIX_OR_COUNTERS;

    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits  = 4;
    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks = 16;/*16 per block --> total 32 blocks*/
    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries = _16K;/*16K of 64bits counters: legacy was 1K*/
    /* next fields set only during prvCpssDxChSharedMemoryAasConnectClientsToMemory(...)
       that initialize the SHM clients */
    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_0_1_usedBlocksBmp = 0;
    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_2_3_usedBlocksBmp = 0;
    for(ii = 0; ii < 64; ii++)
    {

        PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).sip7_cnc_mapToActualBlocksUsed[ii] =
            PRV_CPSS_PP_MAC(devNum)->isGmDevice ?
                ii : /* the GM not supports the SHARED memory so give it 1:1 mapping */
                GT_NA;/* initialize with 'un-mapped' value */
    }

    ePorts = NUM_EPORTS;
    numBitsVPorts = NUM_BITS_VPORTS;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEportLttEcmp =  0;/* not limited (so according to eports)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL2Ecmp       =  NUM_L2_ECMP; /* Hawk  8K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL3Ecmp       =  NUM_L3_ECMP; /* Hawk 12K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTrunkEcmp    =  NUM_TRUNK_ECMP; /* new : Seahawk : 8K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIpNh         =  _8K; /* Hawk 16K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt       =  NUM_L2_LTT_MLL; /* Hawk 16K ,Phoenix 12K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllPairs     =  NUM_MLL_PAIRS; /* Hawk  8K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering =  NUM_PLR_METERS; /* Hawk  4K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrMetering =
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering;/* IPLR and EPLR are 'shared' */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrIpfix    =  PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum; /* Hawk 64K ,Phoenix 16K*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrIpfix    =  PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum; /* Hawk 64K ,Phoenix 16K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam          =  NUM_OAM; /* Hawk  2K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTxqQueue     =    0; /* not relevant to sip6 devices */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesProtectionLoc=  _16K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb          = NUM_FDB; /* Hawk 256K , in shared tables (in phoenix no shared tables)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm           = NUM_EM; /* Hawk 256K , in shared tables (in phoenix no shared tables)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart  = NUM_TS; /* Hawk 64K , in shared tables (in phoenix no shared tables)*//* used only by ENTRY_TYPE_TUNNEL_START_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesPortIsolation= NUM_PORT_ISOLATION; /* used only by CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E/CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx         = NUM_VIDXS; /* used only by CPSS_DXCH_TABLE_MULTICAST_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStream       = _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf    = _1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist= _1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIpclConfig = _4K + _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEpclConfig = _4K + NUM_PHY_PORTS/*1K*/;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.egfInfo.numBitsPerPort = 10;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.egfInfo.shtRegisterOffset = 0x00560000;

    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort            = ePorts;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).phyPort          = NUM_PHY_PORTS;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vid              = NUM_EVIDS;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vidx             = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).stgId            = NUM_STGS;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).l2LttMll         = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).trunkId          = NUM_TRUNKS;

    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort    = 10;/* 10 bits */
    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).ePort      = 14;/* 14 bits represents 16K ePorts */
    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).vid        = 16;/* support NUM_EVIDS 64K */
    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).fid        = 16;/* support NUM_EVIDS 64K */
    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).vPort      = numBitsVPorts;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp =
        (1 << dxDevPtr->hwInfo.multiDataPath.maxDp) - 1;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.myFamilyFineTuningPtr  = aasFineTuningTables;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.myFamilyFineTuningSize = aasFineTuningTables_size;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr        = aasUnitsIdsInTile;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits = 1; /* max number of GOPs; in Aas only single GOP 0 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.taiUnitsPtr   = aasTai0UnitsArray;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais = NUM_ELEMENTS_IN_ARR_MAC(aasTai0UnitsArray);/*PRV_CPSS_NUM_OF_TAI_IN_AAS_CNS*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr = NULL;        /* CPSS_TBD_BOOKMARK_AAS */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr = NULL;    /* CPSS_TBD_BOOKMARK_AAS */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz = 1000000; /* 1G for TAI_CLK */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.fracNanoSeconds = 0.125;

    /* a single IA unit exists */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pipInfo.isIaUnitNotSupported  = GT_FALSE;

    /* PPU unit supported */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ppuInfo.notSupported = GT_FALSE;

    /* USX not supported */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.notSupportUsxPorts = GT_TRUE;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.notImplementedMacSec = GT_TRUE;

    PRV_CPSS_PP_MAC(devNum)->l2NatSupported = GT_TRUE;

    /* state that the devices supports the preemption */
    /* not implemented yet (per port) , but the 'MAC' hold 2 sets of registers  */
    rc = prvCpssSip6TxqCapabilityAdd(devNum,PRV_CPSS_DXCH_TXQ_SIP_6_FEATURE_PREEMPTION_E);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    {
        static const GT_U32  localMacsWithPreemption[] = {1,3,4,5,7,9,11,13};
        GT_U32 globalMac;

        for(ii = 0 ;
            ii < dxDevPtr->hwInfo.multiDataPath.maxDp ;
            ii++)
        {
            /* Full preemption up to 25G up to 8x per DP .
               Packet boundary up to 800G */
            for(index = 0; index < 8; index++)
            {
                globalMac = AAS_PORTS_PER_DP_CNS * ii + localMacsWithPreemption[index] ;
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[globalMac].preemptionSupported = GT_FALSE; /* not implemented yet */
            }
        }
    }

    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxPipesBmp = 0x0F;/* dummy - TBD */

    PRV_CPSS_PP_MAC(devNum)->diagInfo.notSupportPacketGenerator = GT_TRUE;

    prvCpssAasNonSharedHwInfoFuncPointersSet(devNum);

    if(devPtr->multiPipe.numOfPipes >= 2)
    {
        devPtr->portGroupsInfo.activePortGroupsBmp        = BIT_MASK_0_31_MAC(devPtr->multiPipe.numOfPipes);
        devPtr->portGroupsInfo.firstActivePortGroup       = 0;
        devPtr->portGroupsInfo.lastActivePortGroup        = devPtr->multiPipe.numOfPipes - 1;
        devPtr->portGroupsInfo.isMultiPortGroupDevice     = GT_TRUE;
        devPtr->portGroupsInfo.numOfPortGroups            = devPtr->multiPipe.numOfPipes;

        /* this needed to allow code to do :
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC
            and
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC
        */
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.addrDecoderNeeded = GT_TRUE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.addressDecoderArrPtr = aasAddressDecoderArr;
    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.addressDecoderUnitsBmp = BIT_11 - 1;/*11 units (that fit to aasAddressDecoderArr)*/
    /* @brief : the ADDR_DECODER need to 'PCIe window' to match addresses that
     *   the GDMA unit use to access the DRAM (over PCIe)
     *   this window in Seahawk is expected to be 0x2_0000_0000 (size 4G)
     *   in order to not collide with other transactions
     *
     *   but the LINUX may give DRAM addresses in range of : 0x4_0000_0000
     *   Therefore the GDMA unit will be given to work with addresses in 0x2_0000_0000
     *   and the ADDR_DECODER will remap those to 0x4_0000_0000 addresses
     *   and the oATU will 'keep' the mapping as is , meaning : 0x4_0000_0000 to 0x4_0000_0000
     *
     *  so for Seahawk : fake_dmaBaseHigh = 0x2 (representing 0x2_0000_0000)
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.fake_dmaBaseHigh = 0x2;
    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.CNM_WINDOW_INDEX_PCIe = SIP7_CNM_WINDOW_INDEX_PCIe_E;
    PRV_CPSS_DXCH_PP_MAC(devNum)->addressDecoderInfo.cnmAddrDecoderUnitsBmp =
        1 << SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA0_E |
        1 << SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA1_E |
        1 << SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA2_E |
        1 << SIP7_CNM_ADDRESS_DECODER_UNIT_TYPE_GDMA3_E ;

    PRV_CPSS_DXCH_PP_MAC(devNum)->oAtuInfo.oAtuNeeded = GT_TRUE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->oAtuInfo.oAtuWindowsBmp = 0xFF; CPSS_TBD_BOOKMARK_AAS

    return GT_OK;
}



