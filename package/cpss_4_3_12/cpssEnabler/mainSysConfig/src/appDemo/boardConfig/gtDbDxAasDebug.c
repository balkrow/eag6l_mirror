/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxAasDebug.c
*
* @brief Debug functions for the Aas - SIP7.0- board.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/init/cpssInit.h>/* needed for : cpssPpWmDeviceInAsimEnvSet() */
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <gtOs/gtOsSharedPp.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwAasAddrSpace.h>

extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS internal_onEmulator(GT_BOOL doInitSystem);
static GT_U32 aas_emulator_deviceId = 0;/* for to use value recognized by the CPSS ! */
/* function to be called before aas_onEmulator_prepare to allow other/ZERO the 'phoenix_emulator_deviceId' */
GT_STATUS aas_emulator_deviceId_set(GT_U32   deviceId)
{
    aas_emulator_deviceId = deviceId;
    return GT_OK;
}
extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

extern GT_STATUS cnc_onEmulator_extremely_slow_emulator_set(GT_U32   timeToSleep);

extern GT_STATUS debugEmulatorTimeOutSet
(
    GT_U32 timeOutType,
    GT_U32 timeOutValue
);

GT_STATUS aas_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc =  internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 'NO KN' is doing polling in this interrupt global cause register */
    /* but this polling should not be seen during 'trace'                   */
    appDemoTrace_skipTrace_onAddress(AAS_ADDR_CNM_CNM_Interrupt + 0x100 /*0x7C4C0100*//*address*/,0/*index*/);

    /*
        allow to see Addresses that CPSS not access because forbidden in the Emulator
    */
    trace_ADDRESS_NOT_SUPPORTED_MAC_set(1);

    /* CPSS for emulator is stable. LOG is not needed for common testing. */
#ifdef CPSS_LOG_ENABLE
    /* disable CPSS LOG */
    cpssLogEnableSet (0);
    /* disable LOG for ERRORs */
    cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,0);
#endif /*CPSS_LOG_ENABLE*/

    appDemoPrintLinkChangeFlagSet(GT_TRUE);

    if(aas_emulator_deviceId)/* assign only if not ZERO */
    {
        appDemoDebugDeviceIdSet(0, aas_emulator_deviceId);
    }

#ifdef IMPL_TGF
#ifdef INCLUDE_UTF
    /* in the cnc test with fdb upload :
       1. the device get to the fdb entry after some time. so we need to help it ... to wait more than usual
       2. the CNC upload takes longer than usual.
       influence the next 2 tests :
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_cncOverflowAndCombineWithFu",1,1
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_fullQueue",1,1
    */
    cnc_onEmulator_extremely_slow_emulator_set(3000);
#endif /* INCLUDE_UTF */
#endif /* IMPL_TGF */

    {
        GT_U32 debug_sip6_TO_txq_polling_interval = 10;/*orig 200*/
        GT_U32 debug_sip6_TO_after_txq_drain      = 10;/*orig 500*/
        GT_U32 debug_sip6_TO_after_mac_disabled   = 10;/*orig 500*/

        /* use values that used before Phoenix/Hawk added USX ports */
        debugEmulatorTimeOutSet(1,debug_sip6_TO_txq_polling_interval);
        debugEmulatorTimeOutSet(3,debug_sip6_TO_after_txq_drain);
        debugEmulatorTimeOutSet(4,debug_sip6_TO_after_mac_disabled);
    }

    return GT_OK;
}
extern void pexAccessTracePreWriteCallbackBind(void (*pCb)(GT_U32, GT_U32));

typedef GT_U32 UNIT_PER_4_DP[4];

static const UNIT_PER_4_DP aasUnitsPer4DpIdsArr[] =
{
    /* PCA range */
    /*0*/
     {PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_RX_3_E}
     /*1*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_EDGE_TX_3_E}
    /*2*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_3_E}
    /*3*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_3_E}
    /*4*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_3_E}
    /*5*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_3_E}
    /*6*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E}
    /*7*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_CTSU_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_CTSU_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_CTSU_3_E}
    /*8*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E}
    /*9*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E}
    /*10*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E}
    /*11*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E}
    /*12*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E}
    /*13*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E
    ,PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E
    ,PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E
    ,PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E}
    /*14*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*15*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}

    /* DP range */
    /*16*/
    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA1_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA2_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA3_E}
     /*17*/
    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA1_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA2_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA3_E}
    /*18*/
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E}
    /*19*/
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E}
    /*20*/
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E}
    /*21*/
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_2_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_3_E
    ,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_4_E}
    /*22*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*23*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}

    /* MIF per DP */
    /*24*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_800G_0_E
    ,PRV_CPSS_DXCH_UNIT_MIF_800G_1_E
    ,PRV_CPSS_DXCH_UNIT_MIF_800G_2_E
    ,PRV_CPSS_DXCH_UNIT_MIF_800G_3_E}
    /*25*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_0_E
    ,PRV_CPSS_DXCH_UNIT_MIF_400G_1_E
    ,PRV_CPSS_DXCH_UNIT_MIF_400G_2_E
    ,PRV_CPSS_DXCH_UNIT_MIF_400G_3_E}
    /*26*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_4P_0_E
    ,PRV_CPSS_DXCH_UNIT_MIF_4P_1_E
    ,PRV_CPSS_DXCH_UNIT_MIF_4P_2_E
    ,PRV_CPSS_DXCH_UNIT_MIF_4P_3_E}
    /*27*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*28*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*28*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*29*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*30*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
    /*31*/ ,{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
};

static const GT_U32 aasUnitsIdsGlobalArr[] =
{
    /* 0*/ PRV_CPSS_DXCH_UNIT_PB_COUNTER_E
    /* 1*/,PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E
    /* 2*/,PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E
    /* 3*/,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_4_E
    /* 4*/,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E
    /* 5*/,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_4_E
    /* 6*/,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E
    /* 7*/,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E
    /* 8*/,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E
    /* 9*/,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E
    /*10*/,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E
    /*11*/,PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E
    /*12*/,PRV_CPSS_DXCH_UNIT_MIF_RP_0_E
    /*13*/,PRV_CPSS_DXCH_UNIT_MIF_RP_1_E
    /*14*/,PRV_CPSS_DXCH_UNIT_MIF_RP_2_E
    /*15*/,PRV_CPSS_DXCH_UNIT_MIF_RP_3_E
    /*16*/,0xFFFFFFFF
    /*17*/,0xFFFFFFFF
    /*18*/,0xFFFFFFFF
    /*19*/,0xFFFFFFFF
    /*20*/,0xFFFFFFFF
    /*21*/,0xFFFFFFFF
    /*22*/,0xFFFFFFFF
    /*23*/,0xFFFFFFFF
    /*24*/,0xFFFFFFFF
    /*25*/,0xFFFFFFFF
    /*26*/,0xFFFFFFFF
    /*27*/,0xFFFFFFFF
    /*28*/,0xFFFFFFFF
    /*29*/,0xFFFFFFFF
    /*30*/,0xFFFFFFFF
    /*31*/,0xFFFFFFFF
};

/* unitIdIntile_n = unitIdIntile_0 + (n * PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS) */
/* supported by prvCpssDxChHwUnitBaseAddrGet                                             */

static void appDemoHwPpDrvRegTracingAasDpTracingRegAddressesBoundsSet
(
    IN GT_U8  devNum,
    IN GT_U32 perDpUnitsBitmap,
    IN GT_U32 dpsBitmap, /* 4 DPs per tile, for 2 tiles - 8 DPs */
    IN GT_U32 perTileUnitsBitmap,
    IN GT_U32 tilesBitmap /* related to perTileUnitsBitmap only */
)
{
    GT_STATUS                rc;
    GT_U32                   baseArr[33]; /* 32 possible units bits and end-mark with size==0 */
    GT_U32                   sizeArr[33];
    GT_U32                   dp;
    GT_U32                   tile;
    GT_U32                   bitIdx;
    GT_U32                   unitsNum;
    PRV_CPSS_DXCH_UNIT_ENT   unitId;
    GT_BOOL                  error;

    appDemoHwPpDrvRegTracingRegAddressesRangeAddModeSet(
        APP_DEMO_HW_PP_TRACING_ADDR_ADD_MODE_APPEND_E);

    /* units per DP */
    for (dp = 0; (dp < 32); dp++)
    {
        if ((dpsBitmap & (1 << dp)) == 0) continue;
        cpssOsMemSet(baseArr, 0, sizeof(baseArr));
        cpssOsMemSet(sizeArr, 0, sizeof(sizeArr));
        unitsNum = 0;
        for (bitIdx = 0; (bitIdx < 32); bitIdx++)
        {
            if ((perDpUnitsBitmap & (1 << bitIdx)) == 0) continue;
            unitId =
                aasUnitsPer4DpIdsArr[bitIdx][dp % 4]
                + (PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS * (dp / 4));
            baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
                devNum, unitId, &error);
            if (error != GT_FALSE)
            {
                cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
                return;
            }
            rc = prvCpssSip6UnitIdSizeInByteGet(
                devNum, unitId, &(sizeArr[unitsNum]));
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
                return;
            }
            unitsNum ++;
        }
        appDemoHwPpDrvRegTracingRegAddressesBoundsFromArraySet(
            33, baseArr, sizeArr);
    }

    /* units per tile */
    for (tile = 0; (tile < 32); tile++)
    {
        if ((tilesBitmap & (1 << tile)) == 0) continue;
        cpssOsMemSet(baseArr, 0, sizeof(baseArr));
        cpssOsMemSet(sizeArr, 0, sizeof(sizeArr));
        unitsNum = 0;
        for (bitIdx = 0; (bitIdx < 32); bitIdx++)
        {
            if ((perTileUnitsBitmap & (1 << bitIdx)) == 0) continue;
            unitId =
                aasUnitsIdsGlobalArr[bitIdx]
                + (PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS * tile);
            baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
                devNum, unitId, &error);
            if (error != GT_FALSE)
            {
                cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
                return;
            }
            rc = prvCpssSip6UnitIdSizeInByteGet(
                devNum, unitId, &(sizeArr[unitsNum]));
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
                return;
            }
            unitsNum ++;
        }
        appDemoHwPpDrvRegTracingRegAddressesBoundsFromArraySet(
            33, baseArr, sizeArr);
    }
}

static GT_BOOL AasPreWriteTrace_FirstCall = GT_TRUE;
static GT_U8   AasPreWriteTrace_devNum = 0;
static GT_U32  AasPreWriteTrace_perDpUnitsBitmap = 0;
static GT_U32  AasPreWriteTrace_dpsBitmap = 0;
static GT_U32  AasPreWriteTrace_perTileUnitsBitmap = 0;
static GT_U32  AasPreWriteTrace_tilesBitmap = 0;

static void AasPreWriteTrace(GT_U32 address, GT_U32 data)
{
    if ((AasPreWriteTrace_dpsBitmap == 0)
    && (AasPreWriteTrace_tilesBitmap == 0))
    {
        /* no units bound - ready to new bind */
        AasPreWriteTrace_FirstCall = GT_TRUE;
        return;
    }
    if (AasPreWriteTrace_FirstCall != GT_FALSE)
    {
        if (PRV_CPSS_PP_MAC(AasPreWriteTrace_devNum) == 0)
        {
            /* no DB yet */
            return;
        }
        if (PRV_CPSS_DXCH_PP_MAC(AasPreWriteTrace_devNum)->accessTableInfoSize == 0)
        {
            /* no table DB yet */
            return;
        }
        /* bind at first call when unit addresses DB already configured */
        AasPreWriteTrace_FirstCall = GT_FALSE;
        appDemoHwPpDrvRegTracingAasDpTracingRegAddressesBoundsSet(
            AasPreWriteTrace_devNum,
            AasPreWriteTrace_perDpUnitsBitmap,
            AasPreWriteTrace_dpsBitmap,
            AasPreWriteTrace_perTileUnitsBitmap,
            AasPreWriteTrace_tilesBitmap);
    }
    if (appDemoHwPpDrvRegTracingIsTraced(address) == 0) return;

    osPrintf("0x%08X 0x%08X\n", address, data);
}

void appDemoHwPpDrvRegTracingAasDpPreWriteEnable
(
    IN GT_U8  devNum,
    IN GT_U32 perDpUnitsBitmap,
    IN GT_U32 dpsBitmap, /* 4 DPs per tile, for 2 tiles - 8 DPs */
    IN GT_U32 perTileUnitsBitmap,
    IN GT_U32 tilesBitmap /* related to perTileUnitsBitmap only */
)
{
    AasPreWriteTrace_devNum             = devNum;
    AasPreWriteTrace_perDpUnitsBitmap   = perDpUnitsBitmap;
    AasPreWriteTrace_dpsBitmap          = dpsBitmap;
    AasPreWriteTrace_perTileUnitsBitmap = perTileUnitsBitmap;
    AasPreWriteTrace_tilesBitmap        = tilesBitmap;
    pexAccessTracePreWriteCallbackBind(&AasPreWriteTrace);
}

/*====== registers detailed print ======*/

static const APP_DEMO_REG_PRINT_FIELD_STC MIF_Channel_Map_reg_fields_arr[] =
{
     { 0, 6, 0, "Tx channel id number"}
    ,{ 6, 1, 0, "MIF Tx enable"}
    ,{ 7, 1, 0, "MIF Tx pfc en"}
    ,{ 8, 6, 0, "Rx channel id number"}
    ,{14, 1, 0, "MIF Rx enable"}
    ,{15, 1, 0, "MIF Rx PFC enable"}
    ,{18, 1, 0, "clock enable"}
    ,{19, 1, 0, "channel loopback enable"}
    ,{21, 1, 0, "Rx graceful disable mode"}
    ,{22, 1, 0, "Tx graceful disable mode"}
    ,{23, 1, 0, "channel unibus loopback enable"}
    ,{23, 7, 1, "remote port ID number"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC MIF_800G_reg_data_arr[] =
{
     {0x0A00,  4, 9, "Mif Channel Mapping", MIF_Channel_Map_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT MIF_800G_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_MIF_800G_0_E,
    PRV_CPSS_DXCH_UNIT_MIF_800G_1_E,
    PRV_CPSS_DXCH_UNIT_MIF_800G_2_E,
    PRV_CPSS_DXCH_UNIT_MIF_800G_3_E,
    (PRV_CPSS_DXCH_UNIT_MIF_800G_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_800G_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_800G_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_800G_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC MIF_800G_unit =
{
    MIF_800G_unit_id_arr,
    "MIF_800G",
    MIF_800G_reg_data_arr
};

static const PRV_CPSS_DXCH_UNIT_ENT MIF_400G_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_1_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_2_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_3_E,
    (PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_400G_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_400G_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_400G_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC MIF_400G_unit =
{
    MIF_400G_unit_id_arr,
    "MIF_400G",
    MIF_800G_reg_data_arr /* the same for 400G */
};

static const APP_DEMO_REG_PRINT_DATA_STC MIF_4P_reg_data_arr[] =
{
     {0x0A00,  4, 5, "Mif Channel Mapping", MIF_Channel_Map_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT MIF_4P_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_MIF_4P_0_E,
    PRV_CPSS_DXCH_UNIT_MIF_4P_1_E,
    PRV_CPSS_DXCH_UNIT_MIF_4P_2_E,
    PRV_CPSS_DXCH_UNIT_MIF_4P_3_E,
    (PRV_CPSS_DXCH_UNIT_MIF_4P_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_4P_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_4P_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_MIF_4P_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC MIF_4P_unit =
{
    MIF_4P_unit_id_arr,
    "MIF_4P",
    MIF_4P_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_SFF_Channel_Ctrl_En_reg_fields_arr[] =
{
     { 0, 1, 0, "Channel Enable"}
    ,{ 1, 1, 0, "statistics Enable"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_SFF_Channel_Ctrl_Cfg_reg_fields_arr[] =
{
     { 0, 6, 0, "Channel Max Occup"}
    ,{ 8, 7, 0, "Channel Cfg Wr Ptr"}
    ,{16, 7, 0, "Channel Cfg Rd Ptr"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_SFF_LL_Next_Ptr_reg_fields_arr[] =
{
     { 0, 7, 0, "LL Next Ptr"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_SFF_Global_Cfg_reg_fields_arr[] =
{
     {16, 6, 0, "Stat Channel ID Select"}
    ,{23, 7, 0, "Stat for SOP not EOP"}
    ,{24, 7, 0, "Stat Collection Enable"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_SFF_reg_data_arr[] =
{
     {0x0600,  4, 22, "SFF Channel Enable", PCA_SFF_Channel_Ctrl_En_reg_fields_arr}
    ,{0x0800,  4, 22, "SFF Channel Enable", PCA_SFF_Channel_Ctrl_Cfg_reg_fields_arr}
    ,{0x0A00,  4,128, "SFF LL Next Ptr"   , PCA_SFF_LL_Next_Ptr_reg_fields_arr}
    ,{0x0000,  0,  1, "SFF Globa Cfg"     , PCA_SFF_Global_Cfg_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_SFF_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_SFF_unit =
{
    PCA_SFF_unit_id_arr,
    "PCA_SFF",
    PCA_SFF_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_UNPACK_TX_Global_Cfg_reg_fields_arr[] =
{
     { 0, 1, 0, "Stat Collection Enable"}
    ,{ 1, 1, 0, "Stat MCRF Valid not IDLE Slices"}
    ,{ 2, 1, 0, "\nStat input Slice SOP not EOP"}
    ,{ 3, 1, 0, "Stat output Slice SOP not EOP"}
    ,{ 4, 1, 0, "Stat wr side occupancy not rd side"}
    ,{ 6, 2, 0, "Stat selected TX traffic Halt source"}
    ,{ 8, 6, 0, "\nStat Channel ID Select"}
    ,{15, 1, 0, "Global_2_Credit_eop"}
    ,{16, 6, 1, "Global RP_CHID"}
    ,{24, 7, 1, "Global debug RPID"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_UNPACK_TX_Channel_Ctrl_En_reg_fields_arr[] =
{
     { 0, 1, 0, "Enable"}
    ,{ 1, 1, 0, "Statistics Enable"}
    ,{ 2, 1, 0, "SW Halt"}
    ,{ 3, 1, 0, "PTP Control Enable"}
    ,{ 4, 1, 0, "Enable PTP Timeout"}
    ,{ 5, 1, 0, "Enable PTP Tx-FIFO Empty"}
    ,{ 6, 1, 0, "Enable MIF-credits Infinity"}
    ,{ 7, 1, 0, "Failover Loop Disable"}
    ,{18,14, 1, "Wait Slots"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_UNPACK_TX_Channel_Ctrl_Cfg_reg_fields_arr[] =
{
     { 0, 7, 0, "MCRF Row Base"}
    ,{ 8, 7, 0, "MCRF Cfg Number of Rows"}
    ,{16, 8, 0, "RCRF Row Base"}
    ,{24, 8, 0, "RCRF Cfg Number of Rows"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_UNPACK_TX_Channel_Mif_credits_reg_fields_arr[] =
{
     { 0,14, 0, "Max MIF Credits"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_BRG_UNPACK_TX_reg_data_arr[] =
{
     {0x0000,  0,  1, "UNPACK_TX Global Cfg"          , PCA_BRG_UNPACK_TX_Global_Cfg_reg_fields_arr}
    ,{0x0600,  4, 22, "UNPACK_TX Channel Enable"      , PCA_BRG_UNPACK_TX_Channel_Ctrl_En_reg_fields_arr}
    ,{0x0800,  4, 22, "UNPACK_TX Channel Config"      , PCA_BRG_UNPACK_TX_Channel_Ctrl_Cfg_reg_fields_arr}
    ,{0x1000,  4, 22, "UNPACK_TX Channel Mif Credits" , PCA_BRG_UNPACK_TX_Channel_Mif_credits_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_BRG_UNPACK_TX_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_UNPACK_TX_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_BRG_UNPACK_TX_unit =
{
    PCA_BRG_UNPACK_TX_unit_id_arr,
    "PCA_BRG_UNPACK_TX",
    PCA_BRG_UNPACK_TX_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_PACK_RX_Global_Cfg_reg_fields_arr[] =
{
     { 0, 1, 0, "Stat Collection Enable"}
    ,{ 1, 1, 0, "Stat MCRF Valid not IDLE Slices"}
    ,{ 2, 1, 0, "\nStat input SDB SOP not EOP"}
    ,{ 3, 1, 0, "Stat output Slice SOP not EOP"}
    ,{ 4, 1, 0, "Stat wr side occupancy not rd side"}
    ,{ 7, 1, 0, "\nShort Packet interrupt Enable"}
    ,{ 8, 6, 0, "Stat Channel ID Select"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_PACK_RX_Channel_Ctrl_En_reg_fields_arr[] =
{
     { 0, 1, 0, "Enable"}
    ,{ 1, 1, 0, "Statistics Enable"}
    ,{ 3, 1, 0, "Flow Control Reaction Enable"}
    ,{ 8,10, 0, "Flow Control Threshold"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_PACK_RX_Channel_Ctrl_Cfg_reg_fields_arr[] =
{
     { 0, 7, 0, "MCRF Row Base"}
    ,{ 8, 7, 0, "MCRF Cfg Number of Rows"}
    ,{16, 8, 0, "RCRF Row Base"}
    ,{24, 8, 0, "RCRF Cfg Number of Rows"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_BRG_PACK_RX_reg_data_arr[] =
{
     {0x0000,  0,  1, "PACK_RX Global Cfg"          , PCA_BRG_PACK_RX_Global_Cfg_reg_fields_arr}
    ,{0x0600,  4, 22, "PACK_RX Channel Enable"      , PCA_BRG_PACK_RX_Channel_Ctrl_En_reg_fields_arr}
    ,{0x0800,  4, 22, "PACK_RX Channel Config"      , PCA_BRG_PACK_RX_Channel_Ctrl_Cfg_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_BRG_PACK_RX_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_PACK_RX_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_BRG_PACK_RX_unit =
{
    PCA_BRG_PACK_RX_unit_id_arr,
    "PCA_BRG_PACK_RX",
    PCA_BRG_PACK_RX_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_EGR_Global_Control_reg_fields_arr[] =
{
     { 0, 1, 0, "SHM_EGR Bypass"}
    ,{ 1, 1, 0, "Global Loop Force"}
    ,{ 2, 1, 0, "\nLL Software Reset"}
    ,{ 3, 1, 0, "Statistics Collection Enable"}
    ,{ 4, 6, 0, "FlexE Channel Select"}
    ,{10, 1, 0, "\nFlexE Mode"}
    ,{12, 6, 0, "Loop Channel Select"}
    ,{18, 8, 0, "Status Queue Select"}
    ,{26, 6, 0, "CPU Channel Select"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_EGR_Channel_Config_reg_fields_arr[] =
{
     { 0, 1, 0, "Channel Enable"}
    ,{ 1, 1, 0, "Channel Stop"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_EGR_Remote_Port_Config_reg_fields_arr[] =
{
     { 0, 8, 0, "Remote Port FC Threshold"}
    ,{ 8, 8, 0, "Remote Port Credits Default Value"}
    ,{16, 1, 0, "Remote Port Enable"}
    ,{17, 1, 0, "Remote Port Statistics Enable"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_BRG_SHM_EGR_reg_data_arr[] =
{
     {0x1000,  0,  1, "Global Control"      , PCA_BRG_SHM_EGR_Global_Control_reg_fields_arr}
    ,{0x0000,  4, 22, "Channel Config"      , PCA_BRG_SHM_EGR_Channel_Config_reg_fields_arr}
    ,{0x0140,  4, 82, "Remote Port Config"  , PCA_BRG_SHM_EGR_Remote_Port_Config_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_BRG_SHM_EGR_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_TX_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_BRG_SHM_EGR_unit =
{
    PCA_BRG_SHM_EGR_unit_id_arr,
    "PCA_BRG_SHM_EGR",
    PCA_BRG_SHM_EGR_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_INGR_Global_Remote_Port_Cfg_reg_fields_arr[] =
{
     { 0, 6, 0, "Rx carrier channel ID"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_INGR_Global_Loopback_Remote_Port_Cfg_reg_fields_arr[] =
{
     { 0, 6, 0, "loopback carrier channel ID"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_INGR_Global_Loop_Code_Cfg_reg_fields_arr[] =
{
     { 0, 8, 0, "loop code packet truncate threshold"}
    ,{30, 1, 0, "packet truncate enable loop code"}
    ,{31, 1, 0, "packet decryption bypass enable loop code"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_BRG_SHM_INGR_Queue_Control_reg_fields_arr[] =
{
     { 0, 1, 0, "queue enable"}
    ,{ 1, 6, 0, "channel ID"}
    ,{ 8, 7, 0, "remote port channel ID"}
    ,{16, 8, 0, "queue number"}
    ,{24, 1, 0, "flexe mode"}
    ,{25, 1, 0, "ignore queue threshold"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_BRG_SHM_INGR_reg_data_arr[] =
{
     {0x0060,  0,  1, "Remote Port Global Config"           , PCA_BRG_SHM_INGR_Global_Remote_Port_Cfg_reg_fields_arr}
    ,{0x0064,  0,  1, "Loopback Remote Port Global Config"  , PCA_BRG_SHM_INGR_Global_Loopback_Remote_Port_Cfg_reg_fields_arr}
    ,{0x0080,  4,  6, "Loop Code Global Config"             , PCA_BRG_SHM_INGR_Global_Loop_Code_Cfg_reg_fields_arr}
    ,{0x0100,  4, 26, "Queue Control"                       , PCA_BRG_SHM_INGR_Queue_Control_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_BRG_SHM_INGR_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_BRG_SHM_RX_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_BRG_SHM_INGR_unit =
{
    PCA_BRG_SHM_INGR_unit_id_arr,
    "PCA_BRG_SHM_INGR",
    PCA_BRG_SHM_INGR_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PCA_PZ_ARBITER_Global_Arbiter_Bank_Select_reg_fields_arr[] =
{
     { 0, 1, 0, "pz_bank_sel"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_PZ_ARBITER_Global_Cycle_Control_reg_fields_arr[] =
{
     { 0, 8, 0, "pz cycle length"}
    ,{12, 1, 0, "pz cycle en"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_PZ_ARBITER_Bank0_Channel_Enable_reg_fields_arr[] =
{
     { 0, 1, 0, "pz bnk0 chid en"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PCA_PZ_ARBITER_Bank0_Arbiter_Config_reg_fields_arr[] =
{
     { 0, 5, 0, "Bank0 CHID Slot Map"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PCA_PZ_ARBITER_reg_data_arr[] =
{
     {0x0A14,  0,  1, "Pizza Arbiter Bank Select"           , PCA_PZ_ARBITER_Global_Arbiter_Bank_Select_reg_fields_arr}
    ,{0x0A0C,  0,  1, "Pizza Arbiter Cycle Control"         , PCA_PZ_ARBITER_Global_Cycle_Control_reg_fields_arr}
    ,{0x0000,  4, 22, "Pizza Arbiter CHID Enable Bank0"     , PCA_PZ_ARBITER_Bank0_Channel_Enable_reg_fields_arr}
    ,{0x0200,  4,256, "Pizza Arbiter Configuration Bank0"   , PCA_PZ_ARBITER_Bank0_Arbiter_Config_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PCA_PZ_ARBITER_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E,
    (PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PCA_PZ_ARBITER_unit =
{
    PCA_PZ_ARBITER_unit_id_arr,
    "PCA_PZ_ARBITER",
    PCA_PZ_ARBITER_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_FIFO_Channel_Speed_Profile_reg_fields_arr[] =
{
     { 0, 4, 0, "TXF Speed Profile"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC DP_TX_FIFO_reg_data_arr[] =
{
     {0x1000,  4, 23, "Channel Speed Profile"   , DP_TX_FIFO_Channel_Speed_Profile_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT DP_TX_FIFO_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_TX_FIFO_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,
    PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,
    (PRV_CPSS_DXCH_UNIT_TX_FIFO_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TX_FIFO1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TX_FIFO2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TX_FIFO3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC DP_TX_FIFO_unit =
{
    DP_TX_FIFO_unit_id_arr,
    "DP_TX_FIFO",
    DP_TX_FIFO_reg_data_arr
};


static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Arbiter_Control_reg_fields_arr[] =
{
     { 0, 8, 0, "Max Slice"}
    ,{31, 1, 0, "Load New Pizza"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Arbiter_Config_reg_fields_arr[] =
{
     { 0, 6, 0, "Slot Map"}
    ,{31, 1, 0, "Slot Valid"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Channel_Speed_Profile_reg_fields_arr[] =
{
     { 0, 4, 0, "TXD Speed Profile"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Channel_Inter_Packet_Rate_Limiter_reg_fields_arr[] =
{
     { 0,16, 0, "Inter Packet Threshold"}
    ,{16,16, 0, "Idle Rate Increment"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Channel_Inter_Packet_Gap_Config_reg_fields_arr[] =
{
     {16,16, 0, "Inter Packet Gap"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_TX_DMA_Channel_Txd_Global_Config_reg_fields_arr[] =
{
     { 0, 1, 0, "TXD Channel Reset"}
    ,{ 1, 1, 0, "Padding enable"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC DP_TX_DMA_reg_data_arr[] =
{
     {0x0040,  0,  1, "Pizza Arbiter Control"               , DP_TX_DMA_Arbiter_Control_reg_fields_arr}
    ,{0x0044,  4,180, "Pizza Arbiter Configuration"         , DP_TX_DMA_Arbiter_Config_reg_fields_arr}
    ,{0x1000,  4, 23, "Channel Speed Profile"               , DP_TX_DMA_Channel_Speed_Profile_reg_fields_arr}
    ,{0x1400,  4, 23, "Channel Inter Packet Rate Limiter"   , DP_TX_DMA_Channel_Inter_Packet_Rate_Limiter_reg_fields_arr}
    ,{0x1300,  4, 23, "Channel Inter Packet Gap Config"     , DP_TX_DMA_Channel_Inter_Packet_Gap_Config_reg_fields_arr}
    ,{0x1100,  4, 23, "Channel TXD global config"           , DP_TX_DMA_Channel_Txd_Global_Config_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT DP_TX_DMA_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_TXDMA_E,
    PRV_CPSS_DXCH_UNIT_TXDMA1_E,
    PRV_CPSS_DXCH_UNIT_TXDMA2_E,
    PRV_CPSS_DXCH_UNIT_TXDMA3_E,
    (PRV_CPSS_DXCH_UNIT_TXDMA_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TXDMA1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TXDMA2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_TXDMA3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC DP_TX_DMA_unit =
{
    DP_TX_DMA_unit_id_arr,
    "DP_TX_DMA",
    DP_TX_DMA_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC PB_GPR_Arbiter_Control_reg_fields_arr[] =
{
     { 0, 8, 0, "Max Slice"}
    ,{31, 1, 0, "Load New Pizza"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PB_GPR_Arbiter_Config_reg_fields_arr[] =
{
     { 0, 5, 0, "Slot Map"}
    ,{31, 1, 0, "Slot Valid"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PB_GPR_Channel_Speed_Profile_reg_fields_arr[] =
{
     { 0, 4, 0, "Speed Profile"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PB_GPR_Channel_Enable_reg_fields_arr[] =
{
     { 0, 1, 0, "Channel Enable"}
    ,{ 1, 1, 0, "Channel Shaper Merge Enable"}
    ,{ 2, 1, 0, "Channel Shaper Reorder Enable"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC PB_GPR_Channel_Reset_reg_fields_arr[] =
{
     { 0, 1, 0, "Channel Reset"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC PB_GPR_reg_data_arr[] =
{
     {0x0100,  0,  1, "Pizza Arbiter Control"        , PB_GPR_Arbiter_Control_reg_fields_arr}
    ,{0x0200,  4,181, "Pizza Arbiter Configuration"  , PB_GPR_Arbiter_Config_reg_fields_arr}
    ,{0x0700,  4, 24, "Channel Speed Profile"        , PB_GPR_Channel_Speed_Profile_reg_fields_arr}
    ,{0x0500,  4, 24, "Channel Enable"               , PB_GPR_Channel_Enable_reg_fields_arr}
    ,{0x0600,  4, 24, "Channel Reset"                , PB_GPR_Channel_Reset_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT PB_GPR_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E,
    (PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC PB_GPR_unit =
{
    PB_GPR_unit_id_arr,
    "PB_GPR",
    PB_GPR_reg_data_arr
};

static const APP_DEMO_REG_PRINT_FIELD_STC DP_RX_DMA_Channel_General_Config_reg_fields_arr[] =
{
     { 0, 1, 0, "Cascade Enable"}
    ,{ 1, 1, 0, "ingress Packet Include PCH"}
    ,{ 2, 1, 0, "ingress_packet_preempted"}
    ,{ 3, 1, 0, "ingress_packet_from_epb"}
    ,{ 4, 2, 0, "Recalc CRC"}
    ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_RX_DMA_Channel_To_Local_Dev_Source_Port_reg_fields_arr[] =
{
      { 0,10, 0, "Port Number"}
     ,{0,0,0,NULL}
};
static const APP_DEMO_REG_PRINT_FIELD_STC DP_RX_DMA_Channel_Cut_Through_Config_reg_fields_arr[] =
{
     { 0, 1, 0, "Cut Through Enable"}
    ,{ 1, 1, 0, "Cut Through Untagged Enable"}
    ,{ 2,14, 0, "Minimal CT Byte Count"}
    ,{16,12, 0, "CT max header reduction byte count"}
    ,{0,0,0,NULL}
};

static const APP_DEMO_REG_PRINT_DATA_STC DP_RX_DMA_reg_data_arr[] =
{
     {0x0800,  4, 23, "Channel General Configurations"      , DP_RX_DMA_Channel_General_Config_reg_fields_arr}
    ,{0x0B00,  4, 23, "Channel to Local Dev Source Port"    , DP_RX_DMA_Channel_To_Local_Dev_Source_Port_reg_fields_arr}
    ,{0x0120,  4, 23, "Channel CutThrough Configuration"    , DP_RX_DMA_Channel_Cut_Through_Config_reg_fields_arr}
    ,{0,0,0,NULL,NULL}
};

static const PRV_CPSS_DXCH_UNIT_ENT DP_RX_DMA_unit_id_arr[] =
{
    PRV_CPSS_DXCH_UNIT_RXDMA_E,
    PRV_CPSS_DXCH_UNIT_RXDMA1_E,
    PRV_CPSS_DXCH_UNIT_RXDMA2_E,
    PRV_CPSS_DXCH_UNIT_RXDMA3_E,
    (PRV_CPSS_DXCH_UNIT_RXDMA_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_RXDMA1_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_RXDMA2_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E),
    (PRV_CPSS_DXCH_UNIT_RXDMA3_E + PRV_CPSS_DXCH_UNIT_LAST____IN_TILE_0____E)
};

static const APP_DEMO_REG_PRINT_UNIT_STC DP_RX_DMA_unit =
{
    DP_RX_DMA_unit_id_arr,
    "DP_RX_DMA",
    DP_RX_DMA_reg_data_arr
};

/**
* @internal aas_appDemoDebugPcaUnitsRegsDetaledPrint function
* @endinternal
*
* @brief   Print PCA units registers with fields.
*
* @param[in] devNum                   - physical device number
* @param[in] unitInstance             - number of unit instance
*
* @retval - NONE
*/
GT_VOID aas_appDemoDebugPcaUnitsRegsDetaledPrint
(
    IN  GT_U8  devNum,
    IN  GT_U32  unitInstance
)
{
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &MIF_800G_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &MIF_400G_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &MIF_4P_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_SFF_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_BRG_UNPACK_TX_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_BRG_PACK_RX_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_BRG_SHM_EGR_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_BRG_SHM_INGR_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PCA_PZ_ARBITER_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
}

/**
* @internal aas_appDemoDebugDpUnitsRegsDetaledPrint function
* @endinternal
*
* @brief   Print Datapath units registers with fields.
*
* @param[in] devNum                   - physical device number
* @param[in] unitInstance             - number of unit instance
*
* @retval - NONE
*/
GT_VOID aas_appDemoDebugDpUnitsRegsDetaledPrint
(
    IN  GT_U8  devNum,
    IN  GT_U32  unitInstance
)
{
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &DP_TX_FIFO_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &DP_TX_DMA_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &PB_GPR_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
    appDemoDebugPrvUnitRegsDetaledPrint(
        devNum, &DP_RX_DMA_unit, unitInstance);
    cpssOsPrintf("______________________________________________\n");
}
