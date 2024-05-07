/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChBrgHEcmp.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChBrgHEcmpUT, that provides
*       Hierarchical ECMP facility CPSS DxCh implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgHEcmp.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
GT_STATUS cpssDxChBrgHEcmpTargetVPortMappingEntrySet
(
    IN GT_U8 devNum,
    IN GT_U32 index,
    IN CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpTargetVPortMappingEntrySet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      index = 0;
    CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC entry;
    CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC entryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    cpssOsMemSet(&entry, 0, sizeof(entry));

    entry.nextStage         = CPSS_DXCH_BRG_HECMP_STAGE_3_E;
    entry.nextStagePtr      = 3;
    entry.tablePtrTableId   = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_HFT2_E;
    entry.tablePtrBaseAddr  = 0x100;
    entry.tablePtrEntrySize = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_DOUBLE_E;
    entry.profile           = 5;

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (index = 0; index < CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS; index++)
        {
            cpssOsMemSet(&entryGet, 0, sizeof(entryGet));

            /* check for proper table entry write */
            rc = cpssDxChBrgHEcmpTargetVPortMappingEntrySet(dev, index, &entry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, &entryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.nextStage        , entryGet.nextStage        , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.nextStagePtr     , entryGet.nextStagePtr     , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.tablePtrTableId  , entryGet.tablePtrTableId  , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.tablePtrBaseAddr , entryGet.tablePtrBaseAddr , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.tablePtrEntrySize, entryGet.tablePtrEntrySize, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(entry.profile          , entryGet.profile          , dev);
        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntrySet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        index = CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS;
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntrySet(dev, index, &entry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    index = 0;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntrySet(dev, index, &entry);;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpTargetVPortMappingEntrySet(dev, index, &entry);;
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChBrgHEcmpTargetVPortMappingEntryGet
(
    IN GT_U8 devNum,
    IN GT_U32 index,
    OUT CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpTargetVPortMappingEntryGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      index = 0;
    CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC entryGet;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    cpssOsMemSet(&entryGet, 0, sizeof(entryGet));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (index = 0; index < CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS; index++)
        {
            /* check for proper table entry read */
            rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, &entryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        index = CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS;
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, &entryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    index = 0;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, &entryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpTargetVPortMappingEntryGet(dev, index, &entryGet);;
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChBrgHEcmpLttEntrySet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 index,
    IN CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpLttEntrySet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage;
    GT_U32                              index;
    CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   entry;
    CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   entryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    cpssOsMemSet(&entry, 0, sizeof(entry));

    entry.ecmpStartIndex          = 50;
    entry.ecmpNumOfPaths          = 3;
    entry.ecmpEnable              = GT_TRUE;
    entry.ecmpRandomPathEnable    = GT_TRUE;
    entry.hashBitSelectionProfile = 3;
    entry.dualModeEnable          = GT_TRUE;

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            for (index = 0; index < CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS; index++)
            {
                cpssOsMemSet(&entryGet, 0, sizeof(entryGet));

                /* check for proper register setting */
                rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, &entry);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

                rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.ecmpStartIndex         , entryGet.ecmpStartIndex          , dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.ecmpNumOfPaths         , entryGet.ecmpNumOfPaths          , dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.ecmpEnable             , entryGet.ecmpEnable              , dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.ecmpRandomPathEnable   , entryGet.ecmpRandomPathEnable    , dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.hashBitSelectionProfile, entryGet.hashBitSelectionProfile , dev);
                UTF_VERIFY_EQUAL1_PARAM_MAC(entry.dualModeEnable         , entryGet.dualModeEnable          , dev);
            }

            index = CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS;
            rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, &entry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        index = 0;
        stage = CPSS_DXCH_BRG_HECMP_STAGE_3_E + 1;
        rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, &entry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);

        /* check for null pointer */
        index = 0;
        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpLttEntrySet(dev, stage, index, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpLttEntryGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 index,
    OUT CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpLttEntryGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage;
    GT_U32                              index;
    CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   entryGet;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            for (index = 0; index < CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS; index++)
            {
                cpssOsMemSet(&entryGet, 0, sizeof(entryGet));

                /* check for proper register setting */
                rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
            }

            index = CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS;
            rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        index = 0;
        stage = CPSS_DXCH_BRG_HECMP_STAGE_3_E + 1;
        rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);

        /* check for null pointer */
        index = 0;
        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpLttEntryGet(dev, stage, index, &entryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpEcmpPairSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 index,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT writeForm,
    IN CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC *pairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpEcmpPairSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage;
    GT_U32                              index;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT  writeForm;
    CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC   pairEntry;
    CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC   pairEntryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    cpssOsMemSet(&pairEntry, 0, sizeof(pairEntry));

    pairEntry.entry0.nextStage          = CPSS_DXCH_BRG_HECMP_STAGE_2_E;
    pairEntry.entry0.nextStagePtr       = 25;
    pairEntry.entry0.tablePtrTableId    = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_HFT1_E;
    pairEntry.entry0.tablePtrBaseAddr   = 0x200;
    pairEntry.entry0.tablePtrEntrySize  = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_OCTUPLE_E;
    pairEntry.entry0.memberId           = 3;

    pairEntry.entry1.nextStage          = CPSS_DXCH_BRG_HECMP_E2PHY_E;
    pairEntry.entry1.nextStagePtr       = 25;
    pairEntry.entry1.tablePtrTableId    = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_TS_E;
    pairEntry.entry1.tablePtrBaseAddr   = 0x200;
    pairEntry.entry1.tablePtrEntrySize  = CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_QUADRUPLE_E;
    pairEntry.entry1.memberId           = 4;

    pairEntry.statusVectorPtr           = 0x1011;

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            for (index = 0; index < CPSS_DXCH_HECMP_ECMP_TABLE_SIZE_CNS; index++)
            {
                for (writeForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
                      writeForm <= CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E; writeForm++)
                {
                    cpssOsMemSet(&pairEntryGet, 0, sizeof(pairEntryGet));

                    /* check for proper register setting */
                    rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, &pairEntry);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

                    rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, writeForm, &pairEntryGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

                    UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.statusVectorPtr, pairEntryGet.statusVectorPtr, index);

                    if (writeForm != CPSS_DXCH_PAIR_READ_WRITE_SECOND_ONLY_E)
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.nextStage    , pairEntryGet.entry0.nextStage    , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.nextStagePtr     , pairEntryGet.entry0.nextStagePtr     , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.tablePtrTableId  , pairEntryGet.entry0.tablePtrTableId  , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.tablePtrBaseAddr , pairEntryGet.entry0.tablePtrBaseAddr , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.tablePtrEntrySize, pairEntryGet.entry0.tablePtrEntrySize, dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry0.memberId         , pairEntryGet.entry0.memberId         , dev);
                    }

                    if (writeForm != CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.nextStage    , pairEntryGet.entry1.nextStage    , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.nextStagePtr     , pairEntryGet.entry1.nextStagePtr     , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.tablePtrTableId  , pairEntryGet.entry1.tablePtrTableId  , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.tablePtrBaseAddr , pairEntryGet.entry1.tablePtrBaseAddr , dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.tablePtrEntrySize, pairEntryGet.entry1.tablePtrEntrySize, dev);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(pairEntry.entry1.memberId         , pairEntryGet.entry1.memberId         , dev);
                    }
                }
            }

            rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, &pairEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        index = 0;
        rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, &pairEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    writeForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, &pairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpEcmpPairSet(dev, stage, index, writeForm, &pairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpEcmpPairGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 index,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT readForm,
    OUT CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpEcmpPairGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage;
    GT_U32                              index;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT  readForm;
    CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC   pairEntryGet;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            for (index = 0; index < CPSS_DXCH_HECMP_ECMP_TABLE_SIZE_CNS; index++)
            {
                for (readForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
                      readForm <= CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E; readForm++)
                {
                    cpssOsMemSet(&pairEntryGet, 0, sizeof(pairEntryGet));

                    /* check for proper register setting */
                    rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, &pairEntryGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
                }
            }
            rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, &pairEntryGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        index = 0;
        rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, &pairEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    readForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, &pairEntryGet);;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpEcmpPairGet(dev, stage, index, readForm, &pairEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpStatusVectorSet
(
    IN GT_U8 devNum,
    IN GT_U32 index,
    IN GT_U32 statusVector
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpStatusVectorSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      index = 0;
    GT_U32      statusVector = 0;
    GT_U32      statusVectorGet = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    statusVector = 0x980;

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (index = 0; index < CPSS_DXCH_HECMP_STATUS_VECTOR_SIZE_IN_WORDS; index++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpStatusVectorSet(dev, index, statusVector);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, &statusVectorGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(statusVector, statusVectorGet, dev);
        }

        rc = cpssDxChBrgHEcmpStatusVectorSet(dev, index, statusVector);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    index = 0;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpStatusVectorSet(dev, index, statusVector);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpStatusVectorSet(dev, index, statusVector);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpStatusVectorGet
(
    IN GT_U8 devNum,
    IN GT_U32 index,
    OUT GT_U32 *statusVectorPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpStatusVectorGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      index;
    GT_U32      statusVectorGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (index = 0; index < CPSS_DXCH_HECMP_STATUS_VECTOR_SIZE_IN_WORDS; index++)
        {
            rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, &statusVectorGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, &statusVectorGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        index = 0;
        rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    index = 0;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, &statusVectorGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpStatusVectorGet(dev, index, &statusVectorGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfEnableSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPathSelectGcfEnableSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable;
    GT_BOOL     enableGet;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            enable = GT_TRUE;
            rc = cpssDxChBrgHEcmpPathSelectGcfEnableSet(dev,stage, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

            enable = GT_FALSE;
            rc = cpssDxChBrgHEcmpPathSelectGcfEnableSet(dev,stage, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
        }

        enable = GT_TRUE;
        rc = cpssDxChBrgHEcmpPathSelectGcfEnableSet(dev,stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPathSelectGcfEnableSet(dev,stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPathSelectGcfEnableSet(dev,stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfEnableGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPathSelectGcfEnableGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enableGet;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 location,
    IN GT_U32 size
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;
    GT_U32      location, size;
    GT_U32      locationGet, sizeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        location = 24;
        size = 2;
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,&sizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(location, locationGet, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(size, sizeGet, dev);
        }
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        location = 0xFFFF;
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        location = 24;
        size = 20;
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);
    size = 2;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(dev,stage,location,size);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 *locationPtr,
    IN GT_U32 *sizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;
    GT_U32      locationGet, sizeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,&sizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,&sizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,NULL,&sizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,&sizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(dev,stage,&locationGet,&sizeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPOrBSelectGcfEnableSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPOrBSelectGcfEnableSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable;
    GT_BOOL     enableGet;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            enable = GT_TRUE;
            rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(dev,stage, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

            enable = GT_FALSE;
            rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(dev,stage, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPathSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
        }

        enable = GT_TRUE;
        rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(dev,stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(dev,stage, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(dev,stage, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPOrBSelectGcfEnableGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    OUT GT_BOOL *enablePtr

)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPOrBSelectGcfEnableGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enableGet;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(dev,stage, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPorBSelectGcfLocationSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 location
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPorBSelectGcfLocationSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;
    GT_U32      location;
    GT_U32      locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        location = 24;
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpPorBSelectGcfLocationSet(dev,stage,location);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,&locationGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(location, locationGet, dev);
        }
        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationSet(dev,stage,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        location = 0xFFFF;
        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationSet(dev,stage,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    location = 24;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationSet(dev,stage,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPorBSelectGcfLocationSet(dev,stage,location);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpPorBSelectGcfLocationGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_STAGE_ENT stage,
    IN GT_U32 *locationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpPorBSelectGcfLocationGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_STAGE_ENT stage;
    GT_U32      locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E; stage <= CPSS_DXCH_BRG_HECMP_STAGE_3_E; stage++)
        {
            rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,&locationGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        stage = CPSS_DXCH_BRG_HECMP_STAGE_1_E;
        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpPorBSelectGcfLocationGet(dev,stage,&locationGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet
(
    IN GT_U8 devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable, enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        enable = GT_TRUE;
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

        enable = GT_FALSE;
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet(dev,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(dev,&enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet
(
    IN GT_U8 devNum,
    IN GT_U32 location
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      location, locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        location = 24;
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet(dev,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(location, locationGet, dev);

        location = 0xFFFF;
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet(dev,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    location = 24;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet(dev,location);;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet(dev,location);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet
(
    IN GT_U8 devNum,
    IN GT_U32 *locationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(dev,&locationGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet
(
    IN GT_U8 devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enable, enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        enable = GT_TRUE;
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

        enable = GT_FALSE;
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet(dev,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet(dev,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,&enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(dev,&enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet
(
    IN GT_U8 devNum,
    IN GT_U32 location
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      location, locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        location = 24;
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet(dev,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(location, locationGet, dev);

        location = 0xFFFF;
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet(dev,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    location = 24;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet(dev,location);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet(dev,location);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet
(
    IN GT_U8 devNum,
    IN GT_U32 *locationPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      locationGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(dev,&locationGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(dev,&locationGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpExceptionPktCmdSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN CPSS_PACKET_CMD_ENT command
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpExceptionPktCmdSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type;
    CPSS_PACKET_CMD_ENT command, commandGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        for (type = CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E;
              type <= CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E; type ++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpExceptionPktCmdSet(dev, type, command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, &commandGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(command, commandGet, dev);
        }
        rc = cpssDxChBrgHEcmpExceptionPktCmdSet(dev, type, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        command = CPSS_PACKET_CMD_ROUTE_E;
        rc = cpssDxChBrgHEcmpExceptionPktCmdSet(dev, type, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpExceptionPktCmdSet(dev, type, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpExceptionPktCmdSet(dev, type, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpExceptionPktCmdGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_PACKET_CMD_ENT *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpExceptionPktCmdGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type;
    CPSS_PACKET_CMD_ENT commandGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        for (type = CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E;
              type <= CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E; type ++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, &commandGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }
        rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, &commandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpExceptionPktCmdGet(dev, type, &commandGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpExceptionCpuCodeSet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpExceptionCpuCodeSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode, cpuCodeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpuCode = (CPSS_NET_RX_CPU_CODE_ENT) 0x23;
        for (type = CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E;
              type <= CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E; type ++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpExceptionCpuCodeSet(dev, type, cpuCode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, &cpuCodeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(cpuCode, cpuCodeGet, dev);
        }
        rc = cpssDxChBrgHEcmpExceptionCpuCodeSet(dev, type, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        cpuCode = (CPSS_NET_RX_CPU_CODE_ENT) 300;
        rc = cpssDxChBrgHEcmpExceptionCpuCodeSet(dev, type, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT) 0x23;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpExceptionCpuCodeSet(dev, type, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpExceptionCpuCodeSet(dev, type, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChBrgHEcmpExceptionCpuCodeGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgHEcmpExceptionCpuCodeGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type;
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        for (type = CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E;
              type <= CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E; type ++)
        {
            /* check for proper register setting */
            rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, &cpuCodeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }
        rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP7_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, &cpuCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChBrgHEcmpExceptionCpuCodeGet(dev, type, &cpuCodeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgHEcmp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgHEcmp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpTargetVPortMappingEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpTargetVPortMappingEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpLttEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpLttEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpEcmpPairSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpEcmpPairGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpStatusVectorSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpStatusVectorGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPathSelectGcfEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPathSelectGcfEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPOrBSelectGcfEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPOrBSelectGcfEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPorBSelectGcfLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpPorBSelectGcfLocationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpExceptionPktCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpExceptionPktCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgHEcmpExceptionCpuCodeGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgHEcmp)
