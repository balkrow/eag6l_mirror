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
* @file prvCpssDxChPortDynamicPAUnitDrv.h
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter
*
* @version   4
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_UNIT_DRV_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PA_UNIT_DRV_H

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define PA_INVALID_PORT_CNS  0xFFFF


typedef struct
{
    GT_U32 offset;
    GT_U32 len;
}PRV_CPSS_DXCH_PA_REG_FIELDC_STC;


typedef struct
{
    GT_U32                           baseAddressPtrOffs;
    PRV_CPSS_DXCH_PA_REG_FIELDC_STC  fldSliceNum2run;
    PRV_CPSS_DXCH_PA_REG_FIELDC_STC  fldMapLoadEn;
    PRV_CPSS_DXCH_PA_REG_FIELDC_STC  fldWorkConservModeEn;
}PRV_CPSS_DXCH_PA_CTRL_REG_STC;

typedef struct
{
    GT_U32                           baseAddressPtrOffs;
    PRV_CPSS_DXCH_PA_REG_FIELDC_STC  fldSlice2PortMap;
    PRV_CPSS_DXCH_PA_REG_FIELDC_STC  fldSliceEn;
    GT_U32                           totalReg;
    GT_U32                           numberFldsPerReg;
    GT_U32                           totalFlds;
}PRV_CPSS_DXCH_PA_MAP_REG_STC;


typedef struct
{
    GT_U32 ctrlRegSliceNum2Run_Offs;
    GT_U32 ctrlRegSliceNum2Run_Len ;

    GT_U32 ctrlRegMapLoadEn_Offs    ;
    GT_U32 ctrlRegMapLoadEn_Len     ;

    GT_U32 ctrlRegWorkConservEn_Offs;
    GT_U32 ctrlRegWorkConservEn_Len;
}PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC;

typedef struct
{
    GT_U32 paMapRegNum    ;
    GT_U32 paMapFldPerReg ;
    GT_U32 paMapTotalFlds ;

    GT_U32 paMapRegWorkSliceMapOffs ;
    GT_U32 paMapRegWorkSliceMapLen  ;
    GT_U32 paMapRegWorkSliceEnOffs ;
    GT_U32 paMapRegWorkSliceEnLen  ;
}PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC;

typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT                         unitType;
    GT_U32                                        ctrlRegBaseAddrOffs;
    const PRV_CPSS_DXCH_PA_UNIT_CTRL_REG_FLD_DESCR_STC *ctrlRegFldDefPtr;
    GT_U32                                        mapRegBaseAddrOffs;
    const PRV_CPSS_DXCH_PA_UNIT_MAP_REG_FLD_DESCR_STC  *mapRegFldDefPtr;
}PRV_CPSS_DXCH_PA_UNIT_DEF_STC;

typedef struct PRV_CPSS_DXCH_BC2_PA_UNIT_STCT
{
    CPSS_DXCH_PA_UNIT_ENT unit;
    PRV_CPSS_DXCH_PA_CTRL_REG_STC ctrlReg;
    PRV_CPSS_DXCH_PA_MAP_REG_STC  mapReg;
}PRV_CPSS_DXCH_BC2_PA_UNIT_STC;

typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT           family;
    CPSS_PP_SUB_FAMILY_TYPE_ENT       subfamily;
    const PRV_CPSS_DXCH_PA_UNIT_DEF_STC    * const *unitDefListPtr;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC     *unitsDescrAllPtr;
}PRV_CPSS_DXCH_DEV_X_UNITDEFLIST;

typedef struct PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC
{
    GT_BOOL                         isInitilaized;
    PRV_CPSS_DXCH_DEV_X_UNITDEFLIST * dev_x_unitDefList;
}PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC;

extern const PRV_CPSS_DXCH_PA_UNIT_DEF_STC * const prv_paUnitDef_All_bc2[];
extern const PRV_CPSS_DXCH_PA_UNIT_DEF_STC * const prv_paUnitDef_All_bobk[];
extern const PRV_CPSS_DXCH_PA_UNIT_DEF_STC * const prv_paUnitDef_All_aldrin[];
extern const PRV_CPSS_DXCH_PA_UNIT_DEF_STC * const prv_paUnitDef_All_bc3[];
extern const PRV_CPSS_DXCH_PA_UNIT_DEF_STC * const prv_paUnitDef_All_aldrin2[];

/* RXDMA TXDMA TX-FIFO ETH-TX_FIFO */

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_SLICE_NUM_2_RUN_LEN_CNS   9

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_MAP_LOAD_EN_OFFS_CNS      14
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_MAP_LOAD_EN_LEN_CNS       1

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_WORK_CONSERV_EN_OFFS_CNS  15
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_WORK_CONSERV_EN_LEN_CNS   1

/* TXQ */
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_LEN_CNS   9

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_WORK_CONSERV_EN_OFFS_CNS  9
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_WORK_CONSERV_EN_LEN_CNS   1

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_MAP_LOAD_EN_OFFS_CNS      12
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_TXQ_MAP_LOAD_EN_LEN_CNS       1


/* Ilkn-Tx-Fifo */
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_SLICE_NUM_2_RUN_LEN_CNS   11

#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_MAP_LOAD_EN_OFFS_CNS      11
#define PRV_CPSS_DXCH_BC2_PA_CTRLREG_ILKN_FX_FIFO_MAP_LOAD_EN_LEN_CNS       1


/* BobK tx-Dma-Glue rx-Dma-Glue */
#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS   8

#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS      9
#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS       1

#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS 10
#define PRV_CPSS_DXCH_BobK_PA_CTRLREG_RX_TX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS   1


/* Aldrin rx-Dma-Glue */
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS   8

#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS      9
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS       1

#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS 11
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_RX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS   1

/* Aldrin tx-Dma-Glue */
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_SLICE_NUM_2_RUN_LEN_CNS   8

#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_MAP_LOAD_EN_OFFS_CNS      9
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_MAP_LOAD_EN_LEN_CNS       1

#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_WORK_CONSERV_EN_OFFS_CNS 10
#define PRV_CPSS_DXCH_Aldrin_PA_CTRLREG_TX_DMA_GLUE_WORK_CONSERV_EN_LEN_CNS   1



/*-------------------------*/
/* MAP register definition */
/*-------------------------*/
/* RXDMA TXDMA TXQ TX-FIFO ETH-TX-FIFO */
#define PRV_CPSS_DXCH_BC2_PA_MAP_REGNUM_CNS                    85
#define PRV_CPSS_DXCH_BC2_PA_MAP_FLD_PER_REG_CNS               4
#define PRV_CPSS_DXCH_BC2_PA_MAP_TOTAL_FLDS_REG_CNS            340

#define PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICEMAP_LEN_CNS      7

#define PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_OFFS_CNS     7
#define PRV_CPSS_DXCH_BC2_PA_MAPREG_WORK_SLICE_EN_LEN_CNS      1

/* TXQ */
#define PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_REGNUM_CNS             96
#define PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_FLD_PER_REG_CNS        4
#define PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_TOTAL_FLDS_REG_CNS     384


/* Ilkn-Tx-FIFO  */
#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_REGNUM_CNS                    1064
#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_FLD_PER_REG_CNS               1
#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAP_TOTAL_FLDS_REG_CNS            1064

#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICEMAP_LEN_CNS      6

#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICE_EN_OFFS_CNS     6
#define PRV_CPSS_DXCH_BC2_PA_ILKN_TX_FIFO_MAPREG_WORK_SLICE_EN_LEN_CNS      1

/*-----------------------------------------*
 * bobk rx-dma-glue tx-dma-glue            *
 *-----------------------------------------*/
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_REGNUM_CNS                    37
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_FLD_PER_REG_CNS               5
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS            185

#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS      1

#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS     1
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS      1


/*-----------------------------------------*
 * BobK rx-tx-dma-glue                     *
 *-----------------------------------------*/
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_REGNUM_CNS                    37
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_FLD_PER_REG_CNS               5
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS            185

#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS      1

#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS     1
#define PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS      1

/*-----------------------------------------*
 * Aldrin rx-dma-glue                   *
 *-----------------------------------------*/
#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_REGNUM_CNS                    78
#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_FLD_PER_REG_CNS               5
#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS            390

#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS      2

#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS     2
#define PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS      1

/*-----------------------------------------*
 * Aldrin tx-dma-glue                   *
 *-----------------------------------------*/
#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_REGNUM_CNS                    37
#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_FLD_PER_REG_CNS               5
#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_TOTAL_FLDS_REG_CNS            185

#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICEMAP_LEN_CNS      2

#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_OFFS_CNS     2
#define PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAPREG_WORK_SLICE_EN_LEN_CNS      1



/* MPPM  */
#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_REGNUM_CNS                    93
#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_FLD_PER_REG_CNS               4
#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_TOTAL_FLDS_REG_CNS            372

#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICEMAP_LEN_CNS      7

#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICE_EN_OFFS_CNS     7
#define PRV_CPSS_DXCH_BobK_PA_MPPM_MAPREG_WORK_SLICE_EN_LEN_CNS      1


GT_STATUS bobcat2PizzaArbiterUnitDrvGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT unitId,
    OUT CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC * unitStatePtr
);

GT_STATUS bobcat2PizzaArbiterUnitDrvSlicesMapGet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PA_UNIT_ENT                           unitId,
    IN  GT_U32                                         slice2PortMapArrSize,
    OUT GT_U32                                         *sliceNumPtr,
    OUT GT_U32                                         *slice2PortMapArrPtr /* can be NULL */
);

GT_STATUS bobcat2PizzaArbiterUnitDrvSlicesMapMaxSizeGet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PA_UNIT_ENT                           unitId,
    OUT GT_U32                                         *sliceMapMaxSizePtr
);

GT_STATUS bobcat2PizzaArbiterUnitDrvSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_U32   sliceNum,
    IN GT_U32  *slice2PortMapArr
);

/*------------------------------------*/
/* Work conserving mode per TxQ port  */
/*------------------------------------*/
#define PRV_CPSS_DXCH_BC2B0_PORT_WORK_PORTN_CNS       72
#define PRV_CPSS_DXCH_BC3_PORT_WORK_PORTN_CNS         96

#define PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_OFFS_CNS 0
#define PRV_CPSS_DXCH_BC2B0_PORT_WORK_CONSERV_LEN_CNS  1

GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet
(
    IN GT_U8                     devNum,
    IN GT_U32                    txqPort,
    IN GT_BOOL                   status
);

GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    txqPort,
    OUT GT_BOOL                  *statusPtr
);


/*------------------------------------*/
/* Work conserving mode per unit      */
/*------------------------------------*/
GT_STATUS bobcat2PizzaArbiterUnitDrvWorkConservingModeSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_BOOL               status
);

GT_STATUS bobcat2PizzaArbiterUnitDrvWorkConservingModeGet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_BOOL              *statusPtr
);

/**
* @internal bobcat2PizzaArbiterUnitDrvIsWorkConservingModeSupported function
* @endinternal
*
* @brief   Check if unit supports work conserving mode configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] unitId                   - unit ID
*
* @param[out] isSupportedPtr           - GT_FALSE - unit does not support configuration
*                                      - GT_TRUE - unit supports configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, unitId
* @retval GT_FAIL                  - on error
*/
GT_STATUS bobcat2PizzaArbiterUnitDrvIsWorkConservingModeSupported
(
    IN  GT_U8                 devNum,
    IN  CPSS_DXCH_PA_UNIT_ENT unitId,
    OUT GT_BOOL              *isSupportedPtr
);


GT_STATUS bobcat2PizzaArbiterUnitDrvInit
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_BOOL               initHw
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

