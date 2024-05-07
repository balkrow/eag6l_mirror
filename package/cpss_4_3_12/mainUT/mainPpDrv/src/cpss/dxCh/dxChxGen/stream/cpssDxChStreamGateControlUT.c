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
* @file cpssDxChStreamGateControlUT.c
*
* @brief Unit tests for cpssDxChStreamGateControl.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>
#include <utf/private/prvUtfExtras.h>
#include <gtOs/gtOsMem.h>


/*
GT_STATUS cpssDxChStreamSgcGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcGlobalConfigSet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
        Expected: GT_OK.

    1.3 Call API with invalid exception command parameter
        Expected: GT_BAD_PARAM

    1.4 Call API with invalid cpu code parameter
        Expected: GT_BAD_PARAM

    1.5 Call API with globalParamsPtr pointer as NULL
       Expected: GT_BAD_PTR.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  globalParams;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  globalParamsGet;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&globalParams, 0, sizeof(globalParams));
    cpssOsMemSet(&globalParamsGet, 0, sizeof(globalParamsGet));

    /* Set SGC global parameters */
    globalParams.gateClosedException.command               = CPSS_PACKET_CMD_DROP_HARD_E;
    globalParams.gateClosedException.cpuCode               = CPSS_NET_USER_DEFINED_1_E;
    globalParams.intervalMaxOctetExceededException.command = CPSS_PACKET_CMD_DROP_SOFT_E;
    globalParams.intervalMaxOctetExceededException.cpuCode = CPSS_NET_USER_DEFINED_2_E;
    globalParams.sgcCountingModeL3                         = GT_TRUE;
    globalParams.sgcGlobalEnable                           = GT_TRUE;
    globalParams.sgcIgnoreTimestampUseTod                  = GT_TRUE;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcGlobalConfigSet
                                         (dev, &globalParams));

        /* 1.1 Call API with valid values.
           Expected: GT_OK.
         */
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 Read parameters back and compare them with the ones that were set.
           Expected: GT_OK.
         */
        st = cpssDxChStreamSgcGlobalConfigGet(dev, &globalParamsGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Verify set and get parameters are the same */

        /* Compare Table Set parameters */
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.intervalMaxOctetExceededException.command , globalParamsGet.intervalMaxOctetExceededException.command);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.intervalMaxOctetExceededException.cpuCode , globalParamsGet.intervalMaxOctetExceededException.cpuCode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.gateClosedException.command , globalParamsGet.gateClosedException.command);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.gateClosedException.cpuCode , globalParamsGet.gateClosedException.cpuCode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.sgcCountingModeL3           , globalParamsGet.sgcCountingModeL3);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.sgcGlobalEnable             , globalParamsGet.sgcGlobalEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.sgcGlobalEnable             , globalParamsGet.sgcGlobalEnable);
        UTF_VERIFY_EQUAL0_PARAM_MAC(globalParams.sgcIgnoreTimestampUseTod    , globalParamsGet.sgcIgnoreTimestampUseTod);

        /*
           1.3 Call API with invalid exception command parameter
           Expected: GT_BAD_PARAM
        */
        /* Check invalid gate closed exception command */
        globalParams.gateClosedException.command = CPSS_PACKET_CMD_ROUTE_E;
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Set back valid command */
        globalParams.gateClosedException.command = CPSS_PACKET_CMD_DROP_HARD_E;

        /* Check invalid Interval max exceeded exception command */
        globalParams.intervalMaxOctetExceededException.command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Set back valid command */
        globalParams.intervalMaxOctetExceededException.command = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
           1.4 Call API with invalid cpu code parameter
           Expected: GT_BAD_PARAM
        */
        /* Check invalid gate closed exception cpu code */
        globalParams.gateClosedException.cpuCode = 0xfff;
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Set back valid cpu code */
        globalParams.gateClosedException.cpuCode = CPSS_NET_USER_DEFINED_1_E;

        /* Check invalid interval max exceeded exception cpu code */
        globalParams.intervalMaxOctetExceededException.cpuCode = 0xfff;
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Set back valid cpu code */
        globalParams.intervalMaxOctetExceededException.cpuCode = CPSS_NET_USER_DEFINED_1_E;

        /*
           1.5 Call API with globalParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSgcGlobalConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcGlobalConfigSet(dev, &globalParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcGlobalConfigGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcGlobalConfigGet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Call API with globalParamsPtr pointer as NULL
       Expected: GT_BAD_PTR.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  globalParams;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&globalParams, 0, sizeof(globalParams));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcGlobalConfigGet
                                         (dev, &globalParams));

        /* 1.1 Call API with valid values.
           Expected: GT_OK.
         */
        st = cpssDxChStreamSgcGlobalConfigGet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2 Call API with globalParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSgcGlobalConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcGlobalConfigGet(dev, &globalParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcGlobalConfigGet(dev, &globalParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcGateConfigSet
(
    IN GT_U8                               devNum,
    IN GT_U32                              gateId,
    IN GT_U32                              tableSetId,
    IN GT_U32                              slotsNum,
    IN GT_U32                              imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  *gateParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcGateConfigSet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
        Expected: GT_OK.

    1.3 Call API with invalid Gate ID parameter
        Expected: GT_BAD_PARAM.

    1.4 Call API with invalid Table Set ID parameter
        Expected: GT_BAD_PARAM.

    1.5 Call API with invalid byteCountAdjust parameter
        Expected: GT_OUT_OF_RANGE.

    1.6 Call API with invalid remainingBitsFactor parameter
        Expected: GT_OUT_OF_RANGE.

    1.7 Call API with invalid ingressTimestampOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.8 Call API with invalid ingressTodOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.9 Call API with invalid ipv parameter
        Expected: GT_OUT_OF_RANGE.

    1.10 Call API with invalid intervalMaxOctetProfile parameter
         Expected: GT_OUT_OF_RANGE.

    1.11 Call API with gateParamsPtr pointer as NULL
         Expected: GT_BAD_PTR.

    1.12 Call API with invalid remainingBitsResolution parameter
         Expected: GT_OUT_OF_RANGE.

    1.13 Call API with invalid timeSlotsNum parameter
         Expected: GT_BAD_PARAM.

    1.14 Call API with invalid imxProfilesNum parameter
         Expected: GT_BAD_PARAM.

    1.15 Call API with invalid intervalMaxOctetProfile index
         Profile index to set in time slot is bigger than the number of used profiles
         Expected: GT_OUT_OF_RANGE.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       gateIdArr[3] = {0,510,511};   /* Gate ID values: {minimum, maximum, invalid} */
    GT_U32       tableSetIdArr[3] = {0,57,58}; /* Table Set ID values: {minimum, maximum, invalid} */
    GT_U32       i,j;
    GT_U32       timeSlotsNum;
    GT_U32       imxProfilesNum;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParams;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParamsGet;
    CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC     *tableSetInfoPtr;
    CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC     *tableSetInfoGetPtr;
    CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC     *timeSlotInfoPtr;
    CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC     *timeSlotInfoGetPtr;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));
    cpssOsMemSet(&gateParamsGet, 0, sizeof(gateParamsGet));

    /* Set to maximum */
    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

    /* Local pointers to structers to narrow text line */
    tableSetInfoPtr    = &(gateParams.tableSetInfo);
    tableSetInfoGetPtr = &(gateParamsGet.tableSetInfo);
    timeSlotInfoPtr    = gateParams.timeSlotInfoArr;
    timeSlotInfoGetPtr = gateParams.timeSlotInfoArr;

    /* Set Interval max and Time slot parameters */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.intervalMaxArr[i]                          = 0xffff0000 + i;
        gateParams.timeSlotInfoArr[i].timeToAdvance           = 0xf0000000 + i;
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_FALSE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x7;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_TRUE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_TRUE;
    }

    /* Set Table Set time configuration parameters. Set with maximum valid values. */
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = BIT_8-1;
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_TRUE;
    gateParams.tableSetInfo.byteCountAdjust         = BIT_7-1;
    gateParams.tableSetInfo.remainingBitsFactor     = BIT_8-1;
    gateParams.tableSetInfo.cycleTime               = 0xffffffff;
    gateParams.tableSetInfo.ingressTimestampOffset  = BIT_16-1;
    gateParams.tableSetInfo.ingressTodOffset        = BIT_16-1;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcGateConfigSet
                                         (dev, gateIdArr[0], tableSetIdArr[0], timeSlotsNum, imxProfilesNum,  &gateParams));

        /* Check edges range of Gate ID and Table Set ID */
        for (i=0;i<2;i++)
        {
            /* 1.1 Call API with valid values.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcGateConfigSet(dev,
                                                gateIdArr[i],
                                                tableSetIdArr[i],
                                                timeSlotsNum,
                                                imxProfilesNum,
                                                &gateParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Read parameters back and compare them with the ones that were set.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcGateConfigGet(dev,
                                                gateIdArr[i],
                                                tableSetIdArr[i],
                                                timeSlotsNum,
                                                imxProfilesNum,
                                                &gateParamsGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* Verify set and get parameters are the same */

            /* Compare Table Set parameters */
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->remainingBitsResolution, tableSetInfoGetPtr->remainingBitsResolution);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->byteCountAdjustPolarity, tableSetInfoGetPtr->byteCountAdjustPolarity);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->byteCountAdjust        , tableSetInfoGetPtr->byteCountAdjust);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->remainingBitsFactor    , tableSetInfoGetPtr->remainingBitsFactor);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->cycleTime              , tableSetInfoGetPtr->cycleTime);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->ingressTimestampOffset , tableSetInfoGetPtr->ingressTimestampOffset);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->ingressTodOffset       , tableSetInfoGetPtr->ingressTodOffset);

            /* Compare Time Slot parameters */
            for (j=0;j<timeSlotsNum;j++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(gateParams.intervalMaxArr[j]              ,gateParamsGet.intervalMaxArr[j]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].timeToAdvance          ,timeSlotInfoGetPtr[j].timeToAdvance);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].intervalMaxOctetProfile,timeSlotInfoGetPtr[j].intervalMaxOctetProfile);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].keepTc                 ,timeSlotInfoGetPtr[j].keepTc);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].ipv                    ,timeSlotInfoGetPtr[j].ipv);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].lengthAware            ,timeSlotInfoGetPtr[j].lengthAware);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].streamGateState        ,timeSlotInfoGetPtr[j].streamGateState);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].newSlot                ,timeSlotInfoGetPtr[j].newSlot);
            }
        }

        /* 1.3 Call API with invalid Gate ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[2],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.4 Call API with invalid Table Set ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[2],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5 Call API with invalid byteCountAdjust parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set time byteCountAdjust parameter to invlaid */
        gateParams.tableSetInfo.byteCountAdjust = BIT_7;

        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set byteCountAdjust back to valid value  */
        gateParams.tableSetInfo.byteCountAdjust -=1;

        /* 1.6 Call API with invalid remainingBitsFactor parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set remainingBitsFactor parameter to invlaid */
        gateParams.tableSetInfo.remainingBitsFactor = BIT_8;

        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.remainingBitsFactor -=1 ;

        /* 1.7 Call API with invalid ingressTimestampOffset parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set ingressTimestampOffset parameter to invlaid */
        gateParams.tableSetInfo.ingressTimestampOffset = BIT_16;

        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.ingressTimestampOffset -=1 ;

        /* 1.8 Call API with invalid ingressTodOffset parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set ingressTodOffset parameter to invlaid */
        gateParams.tableSetInfo.ingressTodOffset = BIT_16;

        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.ingressTodOffset -=1 ;

        /* 1.9 Call API with invalid ipv parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Time Slot attribute ipv parameter to invalid */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].keepTc = GT_FALSE;
        gateParams.timeSlotInfoArr[timeSlotsNum-1].ipv    = BIT_8;
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Time Slot attribute ipv parameter back to valid value */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].ipv -=1 ;

        /* 1.10 Call API with invalid intervalMaxOctetProfile parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Time Slot attribute intervalMaxOctetProfile parameter to invalid */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].intervalMaxOctetProfile = BIT_8;
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Time Slot attribute intervalMaxOctetProfile parameter back to valid value */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].intervalMaxOctetProfile -=1 ;

        /*
           1.11 Call API with gateParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.12 Call API with invalid remainingBitsResolution parameter
           Expected: GT_OUT_OF_RANGE.
         */
        gateParams.tableSetInfo.remainingBitsResolution = BIT_2;
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set remainingBitsResolution parameter back to valid value */
        gateParams.tableSetInfo.remainingBitsResolution -=1 ;

        /*
           1.13 Call API with invalid timeSlotsNum parameter
           first with timeSlotsNum > maximum
           second with timeSlotsNum '0'
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS + 1,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call API with timeSlotsNum = 0  */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            0,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.14 Call API with invalid imxProfilesNum parameter
           first with imxProfilesNum > maximum
           second with imxProfilesNum '0'
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS + 1,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call API with imxProfilesNum = 0  */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            0,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.15 Call API with invalid intervalMaxOctetProfile index
           Profile index to set in time slot is bigger than the number of used profiles
           Expected: GT_OUT_OF_RANGE.
         */
        imxProfilesNum = 10;
        gateParams.timeSlotInfoArr[5].intervalMaxOctetProfile = imxProfilesNum;
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
           1.16 Call API with invalid timeSlotsNum
           first with timeSlotsNum > maximum
           second with timeSlotsNum '0'
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS+1,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcGateConfigSet(dev,
                                        gateIdArr[0],
                                        tableSetIdArr[0],
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &gateParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcGateConfigGet
(
    IN GT_U8                                devNum,
    IN GT_U32                               gateId,
    IN GT_U32                               tableSetId,
    IN GT_U32                               slotsNum,
    IN GT_U32                               imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  *gateParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcGateConfigGet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Call API with invalid Gate ID parameter
        Expected: GT_BAD_PARAM.

    1.3 Call API with invalid Table Set ID parameter
        Expected: GT_BAD_PARAM.

    1.4 Call API with gateParamsPtr pointer as NULL
        Expected: GT_BAD_PTR.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       gateIdArr[3] = {0,510,511};   /* Gate ID values: {minimum, maximum, invalid} */
    GT_U32       tableSetIdArr[3] = {0,57,58}; /* Table Set ID values: {minimum, maximum, invalid} */
    GT_U32       i;
    GT_U32       timeSlotsNum;
    GT_U32       imxProfilesNum;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  gateParams;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));

    /* Set to maximum */
    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcGateConfigGet
                                         (dev, gateIdArr[0], tableSetIdArr[0], timeSlotsNum, imxProfilesNum, &gateParams));

        /* Check edges range of Gate ID and Table Set ID */
        for (i=0;i<2;i++)
        {
            /* 1.1 Call API with valid values.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcGateConfigGet(dev,
                                                gateIdArr[i],
                                                tableSetIdArr[i],
                                                timeSlotsNum,
                                                imxProfilesNum,
                                                &gateParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* 1.3 Call API with invalid Gate ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigGet(dev,
                                            gateIdArr[2],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.4 Call API with invalid Table Set ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigGet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[2],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.11 Call API with gateParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChStreamSgcGateConfigGet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcGateConfigGet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcGateConfigGet(dev,
                                        gateIdArr[0],
                                        tableSetIdArr[0],
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &gateParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       gateId,
    IN GT_U32                                       tableSetId,
    IN GT_U32                                       slotsNum,
    IN GT_U32                                       imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    IN CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcTimeBasedGateReConfigSet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
        Expected: GT_OK.

    1.3 Call API with invalid Gate ID parameter
        Expected: GT_BAD_PARAM.

    1.4 Call API with invalid Table Set ID parameter
        Expected: GT_BAD_PARAM.

    1.5 Call API with invalid byteCountAdjust parameter
        Expected: GT_OUT_OF_RANGE.

    1.6 Call API with invalid remainingBitsFactor parameter
        Expected: GT_OUT_OF_RANGE.

    1.7 Call API with invalid ingressTimestampOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.8 Call API with invalid ingressTodOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.9 Call API with invalid ipv parameter
        Expected: GT_OUT_OF_RANGE.

    1.10 Call API with invalid intervalMaxOctetProfile parameter
         Expected: GT_OUT_OF_RANGE.

    1.11 Call API with invalid todMsb parameter
         Expected: GT_OUT_OF_RANGE.

    1.12 Call API with invalid todLsb parameter
         Expected: GT_OUT_OF_RANGE.

    1.13 Call API with invalid configChangeTimeExtension parameter
         Expected: GT_OUT_OF_RANGE.

    1.14 Call API with gateParamsPtr pointer as NULL
         Expected: GT_BAD_PTR.

    1.15 Call API with gateReconfigTimeParams pointer as NULL
         Expected: GT_BAD_PTR.

    1.16 Call API with invalid remainingBitsResolution parameter
         Expected: GT_OUT_OF_RANGE.

    1.17 Call API with invalid timeSlotsNum parameter
         Expected: GT_BAD_PARAM.

    1.18 Call API with invalid imxProfilesNum parameter
         Expected: GT_BAD_PARAM.

    1.19 Call API with invalid intervalMaxOctetProfile index
         Profile index to set in time slot is bigger than the number of used profiles
         Expected: GT_OUT_OF_RANGE.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       gateIdArr[3] = {0,510,511};   /* Gate ID values: {minimum, maximum, invalid} */
    GT_U32       tableSetIdArr[3] = {0,57,58}; /* Table Set ID values: {minimum, maximum, invalid} */
    GT_U32       i,j;
    GT_U32       timeSlotsNum;
    GT_U32       imxProfilesNum;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParams;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParamsGet;
    CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC gateReconfigTimeParams;
    CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC gateReconfigTimeParamsGet;
    CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC     *tableSetInfoPtr;
    CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC     *tableSetInfoGetPtr;
    CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC     *timeSlotInfoPtr;
    CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC     *timeSlotInfoGetPtr;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));
    cpssOsMemSet(&gateParamsGet, 0, sizeof(gateParamsGet));
    cpssOsMemSet(&gateReconfigTimeParams, 0, sizeof(gateReconfigTimeParams));
    cpssOsMemSet(&gateReconfigTimeParamsGet, 0, sizeof(gateReconfigTimeParamsGet));

    /* Local pointers to structers to narrow text line */
    tableSetInfoPtr    = &(gateParams.tableSetInfo);
    tableSetInfoGetPtr = &(gateParamsGet.tableSetInfo);
    timeSlotInfoPtr    = gateParams.timeSlotInfoArr;
    timeSlotInfoGetPtr = gateParams.timeSlotInfoArr;

    /* Set to maximum */
    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

    /* Set Interval max and Time slot parameters */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.intervalMaxArr[i]                          = 0xffff0000 + i;
        gateParams.timeSlotInfoArr[i].timeToAdvance           = 0xf0000000 + i;
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_FALSE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x7;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_TRUE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_TRUE;
    }

    /* Set Table Set time configuration parameters. Set with maximum valid values. */
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_TRUE;
    gateParams.tableSetInfo.byteCountAdjust         = BIT_7-1;
    gateParams.tableSetInfo.remainingBitsFactor     = BIT_8-1;
    gateParams.tableSetInfo.cycleTime               = 0xffffffff;
    gateParams.tableSetInfo.ingressTimestampOffset  = BIT_16-1;
    gateParams.tableSetInfo.ingressTodOffset        = BIT_16-1;

    /* Set re-configuration parameters. Set with maximum valid values. */
    gateReconfigTimeParams.todMsb.l[0]               = 0xFFFFFFFF;
    gateReconfigTimeParams.todMsb.l[1]               = 0xFFFF;
    gateReconfigTimeParams.todLsb                    = BIT_30 - 1;
    gateReconfigTimeParams.configChangeTimeExtension = BIT_16 - 1;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcTimeBasedGateReConfigSet
                                         (dev, gateIdArr[0], tableSetIdArr[0], timeSlotsNum, imxProfilesNum, &gateParams, &gateReconfigTimeParams));

        /* Check edges range of Gate ID and Table Set ID */
        for (i=0;i<2;i++)
        {
            /* 1.1 Call API with valid values.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                           gateIdArr[i],
                                                           tableSetIdArr[i],
                                                           timeSlotsNum,
                                                           imxProfilesNum,
                                                           &gateParams,
                                                           &gateReconfigTimeParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Read parameters back and compare them with the ones that were set.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                           gateIdArr[i],
                                                           tableSetIdArr[i],
                                                           timeSlotsNum,
                                                           imxProfilesNum,
                                                           &gateParamsGet,
                                                           &gateReconfigTimeParamsGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* Verify set and get parameters are the same */

            /* Compare Table Set parameters */
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->remainingBitsResolution, tableSetInfoGetPtr->remainingBitsResolution);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->byteCountAdjustPolarity, tableSetInfoGetPtr->byteCountAdjustPolarity);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->byteCountAdjust        , tableSetInfoGetPtr->byteCountAdjust);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->remainingBitsFactor    , tableSetInfoGetPtr->remainingBitsFactor);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->cycleTime              , tableSetInfoGetPtr->cycleTime);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->ingressTimestampOffset , tableSetInfoGetPtr->ingressTimestampOffset);
            UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetInfoPtr->ingressTodOffset       , tableSetInfoGetPtr->ingressTodOffset);

            /* Compare Time Slot parameters */
            for (j=0;j<timeSlotsNum;j++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(gateParams.intervalMaxArr[j]              ,gateParamsGet.intervalMaxArr[j]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].timeToAdvance          ,timeSlotInfoGetPtr[j].timeToAdvance);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].intervalMaxOctetProfile,timeSlotInfoGetPtr[j].intervalMaxOctetProfile);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].keepTc                 ,timeSlotInfoGetPtr[j].keepTc);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].ipv                    ,timeSlotInfoGetPtr[j].ipv);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].lengthAware            ,timeSlotInfoGetPtr[j].lengthAware);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].streamGateState        ,timeSlotInfoGetPtr[j].streamGateState);
                UTF_VERIFY_EQUAL0_PARAM_MAC(timeSlotInfoPtr[j].newSlot                ,timeSlotInfoGetPtr[j].newSlot);
            }

            /* Compare re-configuration parameters */
            UTF_VERIFY_EQUAL0_PARAM_MAC(gateReconfigTimeParams.todMsb.l[0]               , gateReconfigTimeParamsGet.todMsb.l[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(gateReconfigTimeParams.todMsb.l[1]               , gateReconfigTimeParamsGet.todMsb.l[1]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(gateReconfigTimeParams.todLsb                   , gateReconfigTimeParamsGet.todLsb);
            UTF_VERIFY_EQUAL0_PARAM_MAC(gateReconfigTimeParams.configChangeTimeExtension, gateReconfigTimeParamsGet.configChangeTimeExtension);
        }

        /* 1.3 Call API with invalid Gate ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[2],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.4 Call API with invalid Table Set ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[2],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5 Call API with invalid byteCountAdjust parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set time byteCountAdjust parameter to invlaid */
        gateParams.tableSetInfo.byteCountAdjust = BIT_7;

        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set byteCountAdjust back to valid value  */
        gateParams.tableSetInfo.byteCountAdjust -=1;

        /* 1.6 Call API with invalid remainingBitsFactor parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set remainingBitsFactor parameter to invlaid */
        gateParams.tableSetInfo.remainingBitsFactor = BIT_8;

        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.remainingBitsFactor -=1 ;

        /* 1.7 Call API with invalid ingressTimestampOffset parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set ingressTimestampOffset parameter to invlaid */
        gateParams.tableSetInfo.ingressTimestampOffset = BIT_16;

        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.ingressTimestampOffset -=1 ;

        /* 1.8 Call API with invalid ingressTodOffset parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Table Set ingressTodOffset parameter to invlaid */
        gateParams.tableSetInfo.ingressTodOffset = BIT_16;

        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Table Set remainingBitsFactor back to valid value  */
        gateParams.tableSetInfo.ingressTodOffset -=1 ;

        /* 1.9 Call API with invalid ipv parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Time Slot attribute ipv parameter to invalid */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].keepTc = GT_FALSE;
        gateParams.timeSlotInfoArr[timeSlotsNum-1].ipv    = BIT_8;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Time Slot attribute ipv parameter back to valid value */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].ipv -=1 ;

        /* 1.10 Call API with invalid intervalMaxOctetProfile parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set Time Slot attribute intervalMaxOctetProfile parameter to invalid */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].intervalMaxOctetProfile = BIT_8;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set Time Slot attribute intervalMaxOctetProfile parameter back to valid value */
        gateParams.timeSlotInfoArr[timeSlotsNum-1].intervalMaxOctetProfile -=1 ;

        /* 1.11 Call API with invalid todMsb parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set re-configuration ipv parameter to invalid */
        gateReconfigTimeParams.todMsb.l[0] = 0xFFFFFFFF;
        gateReconfigTimeParams.todMsb.l[1] = 0x10000;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set re-configuration todMsb parameter back to valid value */
        gateReconfigTimeParams.todMsb.l[1] -= 1;

        /* 1.12 Call API with invalid todLsb parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set re-configuration todLsb parameter to invalid */
        gateReconfigTimeParams.todLsb = BIT_30;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set re-configuration todLsb parameter back to valid value */
        gateReconfigTimeParams.todLsb -= 1;

        /* 1.13 Call API with invalid configChangeTimeExtension parameter
           Expected: GT_OUT_OF_RANGE.
         */
        /* Set re-configuration configChangeTimeExtension parameter to invalid */
        gateReconfigTimeParams.configChangeTimeExtension = BIT_16;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set re-configuration todLsb parameter back to valid value */
        gateReconfigTimeParams.configChangeTimeExtension -= 1;

        /*
           1.14 Call API with gateParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       NULL,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.15 Call API with gateReconfigTimeParams pointer as NULL
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.16 Call API with invalid remainingBitsResolution parameter
           Expected: GT_OUT_OF_RANGE.
         */
        gateParams.tableSetInfo.remainingBitsResolution = BIT_2;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Set remainingBitsResolution parameter back to valid value */
        gateParams.tableSetInfo.remainingBitsResolution -=1;

        /*
           1.13 Call API with invalid timeSlotsNum parameter
           first with timeSlotsNum > maximum
           second with timeSlotsNum '0'
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS + 1,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call API with timeSlotsNum = 0  */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            0,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.14 Call API with invalid imxProfilesNum parameter
           first with imxProfilesNum > maximum
           second with imxProfilesNum '0'
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS + 1,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call API with imxProfilesNum = 0  */
        st = cpssDxChStreamSgcGateConfigSet(dev,
                                            gateIdArr[0],
                                            tableSetIdArr[0],
                                            timeSlotsNum,
                                            0,
                                            &gateParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.15 Call API with invalid intervalMaxOctetProfile index
           Profile index to set in time slot is bigger than the number of used profiles
           Expected: GT_OUT_OF_RANGE.
         */
        imxProfilesNum = 10;
        gateParams.timeSlotInfoArr[5].intervalMaxOctetProfile = imxProfilesNum;
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                   gateIdArr[0],
                                                   tableSetIdArr[0],
                                                   timeSlotsNum,
                                                   imxProfilesNum,
                                                   &gateParams,
                                                   &gateReconfigTimeParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       gateId,
    IN  GT_U32                                       tableSetId,
    IN  GT_U32                                       slotsNum,
    IN  GT_U32                                       imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    OUT CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcTimeBasedGateReConfigGet)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Call API with invalid Gate ID parameter
        Expected: GT_BAD_PARAM.

    1.3 Call API with invalid Table Set ID parameter
        Expected: GT_BAD_PARAM.

    1.4 Call API with gateParamsPtr pointer as NULL
        Expected: GT_BAD_PTR.

    1.5 Call API with gateReconfigTimeParams pointer as NULL
        Expected: GT_BAD_PTR.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       gateIdArr[3] = {0,510,511};   /* Gate ID values: {minimum, maximum, invalid} */
    GT_U32       tableSetIdArr[3] = {0,57,58}; /* Table Set ID values: {minimum, maximum, invalid} */
    GT_U32       i;
    GT_U32       timeSlotsNum;
    GT_U32       imxProfilesNum;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParams;
    CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC gateReconfigTimeParams;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));
    cpssOsMemSet(&gateReconfigTimeParams, 0, sizeof(gateReconfigTimeParams));

    /* Set to maximum */
    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcTimeBasedGateReConfigGet
                                         (dev, gateIdArr[0], tableSetIdArr[0], timeSlotsNum, imxProfilesNum, &gateParams, &gateReconfigTimeParams));

        /* Check edges range of Gate ID and Table Set ID */
        for (i=0;i<2;i++)
        {
            /* 1.1 Call API with valid values.
               Expected: GT_OK.
             */
            st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                           gateIdArr[i],
                                                           tableSetIdArr[i],
                                                           timeSlotsNum,
                                                           imxProfilesNum,
                                                           &gateParams,
                                                           &gateReconfigTimeParams);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* 1.2 Call API with invalid Gate ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                       gateIdArr[2],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 Call API with invalid Table Set ID parameter
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[2],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
           1.4 Call API with gateParamsPtr pointer as NULL
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       NULL,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
           1.5 Call API with gateReconfigTimeParams pointer as NULL
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                       gateIdArr[0],
                                                       tableSetIdArr[0],
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcTimeBasedGateReConfigGet(dev,
                                                   gateIdArr[0],
                                                   tableSetIdArr[0],
                                                   timeSlotsNum,
                                                   imxProfilesNum,
                                                   &gateParams,
                                                   &gateReconfigTimeParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigComplete
(
    IN GT_U8    devNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChStreamSgcTimeBasedGateReConfigComplete)
{
/*
    1.1 Call API with valid values.
        Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
        Expected: GT_OK.

    1.3 Call API with invalid Gate ID parameter
        Expected: GT_BAD_PARAM.

    1.4 Call API with invalid Table Set ID parameter
        Expected: GT_BAD_PARAM.

    1.5 Call API with invalid byteCountAdjust parameter
        Expected: GT_OUT_OF_RANGE.

    1.6 Call API with invalid remainingBitsFactor parameter
        Expected: GT_OUT_OF_RANGE.

    1.7 Call API with invalid ingressTimestampOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.8 Call API with invalid ingressTodOffset parameter
        Expected: GT_OUT_OF_RANGE.

    1.9 Call API with invalid ipv parameter
        Expected: GT_OUT_OF_RANGE.

    1.10 Call API with invalid intervalMaxOctetProfile parameter
         Expected: GT_OUT_OF_RANGE.

    1.11 Call API with invalid todMsb parameter
         Expected: GT_OUT_OF_RANGE.

    1.12 Call API with invalid todLsb parameter
         Expected: GT_OUT_OF_RANGE.

    1.13 Call API with invalid configChangeTimeExtension parameter
         Expected: GT_OUT_OF_RANGE.

    1.14 Call API with gateParamsPtr pointer as NULL
         Expected: GT_BAD_PTR.

    1.15 Call API with gateReconfigTimeParams pointer as NULL
         Expected: GT_BAD_PTR.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       gateId = 510;
    GT_U32       tableSetId = 57;
    GT_U8        tableSetIdGet;
    GT_BOOL      pendingStatusGet;
    GT_BIT       validStatusBitGet;
    GT_BOOL      isTableSetActive = GT_FALSE;
    GT_U32       i;
    GT_U32       regAddr;
    GT_U32       gclId;
    GT_U32       timeSlotsNum;
    GT_U32       imxProfilesNum;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC          gateParams;
    CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC gateReconfigTimeParams;


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));
    cpssOsMemSet(&gateReconfigTimeParams, 0, sizeof(gateReconfigTimeParams));

    /* Set to maximum */
    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

    /* Set Interval max and Time slot parameters */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.intervalMaxArr[i]                          = 0xffff0000 + i;
        gateParams.timeSlotInfoArr[i].timeToAdvance           = 0xf0000000 + i;
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_TRUE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x7;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_TRUE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_TRUE;
    }

    /* Set Table Set time configuration parameters. Set with maximum valid values. */
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_TRUE;
    gateParams.tableSetInfo.byteCountAdjust         = BIT_7-1;
    gateParams.tableSetInfo.remainingBitsFactor     = BIT_8-1;
    gateParams.tableSetInfo.cycleTime               = 1000;
    gateParams.tableSetInfo.ingressTimestampOffset  = 0;
    gateParams.tableSetInfo.ingressTodOffset        = 0;

    /* Set re-configuration parameters */
    gateReconfigTimeParams.todMsb.l[0]               = 0;
    gateReconfigTimeParams.todMsb.l[1]               = 0;
    gateReconfigTimeParams.todLsb                    = 100;
    gateReconfigTimeParams.configChangeTimeExtension = 0;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Checks TSN features for not applicable device */
        UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                         cpssDxChStreamSgcTimeBasedGateReConfigComplete
                                         (dev));

        /* Call cpssDxChStreamSgcTimeBasedGateReConfigSet to configure re-configuration parameters */
        st = cpssDxChStreamSgcTimeBasedGateReConfigSet(dev,
                                                       gateId,
                                                       tableSetId,
                                                       timeSlotsNum,
                                                       imxProfilesNum,
                                                       &gateParams,
                                                       &gateReconfigTimeParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Clear re-configuration Pending Status bit back to zero
           so cpssDxChStreamSgcTimeBasedGateReConfigComplete API can be run later
           Note: should be already done by SGC HW when re-configuration time and current time match
           Will be checked later when feature will be tested on board
         */
        regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(dev).streamGateControl.sgcReconfigPendingStatus;
        st = prvCpssHwPpSetRegField(dev, regAddr, 0, 1, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 Call API with valid values.
           Expected: GT_OK.
         */
        st = cpssDxChStreamSgcTimeBasedGateReConfigComplete(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2 Check re-configuration Pending & Valid Status bits were indeed cleared to '0'
           Expected: GT_FALSE.
         */
        st = prvCpssStreamSgcReconfigurationStatusGet(dev, &pendingStatusGet, &validStatusBitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, pendingStatusGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, validStatusBitGet);

        /* 1.3 Check re-configuration GCL ID 2 Table Set mapping is correct
           Expected: Table Set ID that was previously set in cpssDxChStreamSgcTimeBasedGateReConfigSet.
         */
        /* Extract 6 MSb of Gate ID */
        gclId = (gateId >> 3) & 0x3F;
        st = prvCpssStreamSgcGateId2TableSetConfigGet(dev, gclId, &tableSetIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* Check this Table set is the one that was set before */
        UTF_VERIFY_EQUAL0_PARAM_MAC(tableSetId, tableSetIdGet);

        /* 1.4 Check the selected Table Set was activated
           Expected: Activated
         */
        /* read it from SGC TableSet is active register */
        st = prvCpssStreamSgcTableSetStatusGet(dev, tableSetIdGet, &isTableSetActive);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, isTableSetActive);

    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChStreamSgcTimeBasedGateReConfigComplete(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamSgcTimeBasedGateReConfigComplete(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressTableSetConfigSet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       slotsNum = 3;
    GT_U32       tableSet = 11;
    GT_U32       cmpResult;
    CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC timeSlotInfoArr[3]=
    {
            {0x1,0xF,GT_FALSE},
            {0x20,0xF0,GT_FALSE},
            {0x30,0x81,GT_TRUE}
    };

    CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC timeSlotInfoArrCmp[3];

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    timeSlotInfoArr[0].gateStateBmp = 0x1;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressTableSetConfigSet
                                             (dev, tableSet, slotsNum, timeSlotInfoArr));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, slotsNum,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Read parameters back and compare them with the ones that were set.
                        Expected: GT_OK. */

            st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, slotsNum,timeSlotInfoArrCmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            cmpResult = osMemCmp(timeSlotInfoArr,timeSlotInfoArrCmp,sizeof(timeSlotInfoArrCmp));
            UTF_VERIFY_EQUAL1_PARAM_MAC(0, cmpResult, dev);

           /* 1.3 Call function with out of bound value for tableset  id.*/
            st = cpssDxChStreamEgressTableSetConfigSet(dev,30, slotsNum,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.4 Call function with out of bound value for slotsNum*/
            st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, 257,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

           /* 1.5 Call function with out of bound value for gateStateBmp*/
            timeSlotInfoArrCmp[2].gateStateBmp =0x10F;
            st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, slotsNum,timeSlotInfoArrCmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

           /* 1.6  Call function with NULL pointer*/

            st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, slotsNum,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, slotsNum,timeSlotInfoArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressTableSetConfigSet(dev,tableSet, slotsNum,timeSlotInfoArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

UTF_TEST_CASE_MAC(cpssDxChStreamEgressTableSetConfigGet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       slotsNum = 3;
    GT_U32       tableSet = 11;
    CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC timeSlotInfoArr[3];


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    timeSlotInfoArr[0].gateStateBmp = 0x1;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressTableSetConfigGet
                                             (dev, tableSet, slotsNum, timeSlotInfoArr));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, slotsNum,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


           /* 1.2 Call function with out of bound value for tableset  id.*/
            st = cpssDxChStreamEgressTableSetConfigGet(dev,30, slotsNum,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


            /* 1.3  Call function with out of bound value for slotsNum*/
            st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, 257,timeSlotInfoArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.4  Call function with NULL pointer*/
            st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, slotsNum,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, slotsNum,timeSlotInfoArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressTableSetConfigGet(dev,tableSet, slotsNum,timeSlotInfoArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortBindSet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       tableSet = 11;
    GT_PHYSICAL_PORT_NUM       portNum = 0;
    CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  bindEntry;
    osMemSet(&bindEntry,0,sizeof(CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC));
    bindEntry.gateReconfigRequestParam.bitsFactorResolution = 1;
    bindEntry.gateReconfigRequestParam.cycleTime = BIT_20;
    bindEntry.gateReconfigRequestParam.egressTodOffset = BIT_10;
    bindEntry.gateReconfigRequestParam.lastEntry = 20;
    bindEntry.gateReconfigRequestParam.lengthAware = GT_TRUE;
    bindEntry.gateReconfigRequestParam.maxAlwdBcOffset = BIT_10;
    bindEntry.gateReconfigRequestParam.remainingBitsFactor = BIT_10;

    bindEntry.gateReconfigTimeParam.configChangeTimeExtension = BIT_8;
    bindEntry.gateReconfigTimeParam.todMsb.l[0]= 0x123;
    bindEntry.gateReconfigTimeParam.todMsb.l[1]= 0x456;
    bindEntry.gateReconfigTimeParam.todLsb= BIT_10;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortBindSet
                                             (dev, portNum, tableSet, &bindEntry));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Call function with out of bound port number*/
             st = cpssDxChStreamEgressPortBindSet(dev,UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev), tableSet,&bindEntry);
             UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

           /* 1.3 Call function with out of bound value for tableset  id.*/
            st = cpssDxChStreamEgressPortBindSet(dev,portNum, 45,&bindEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* 1.4 Call function with out of bound value for bitsFactorResolution*/
            bindEntry.gateReconfigRequestParam.bitsFactorResolution = BIT_17;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            bindEntry.gateReconfigRequestParam.bitsFactorResolution = BIT_10;
             /* 1.5 Call function with out of bound value for cycleTime*/
            bindEntry.gateReconfigRequestParam.cycleTime = BIT_31;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            bindEntry.gateReconfigRequestParam.cycleTime = BIT_11;

             /* 1.6  Call function with out of bound value for egressTodOffset*/
            bindEntry.gateReconfigRequestParam.egressTodOffset = BIT_31;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            bindEntry.gateReconfigRequestParam.egressTodOffset = 1<<5;

            /* 1.7  Call function with out of bound value for lastEntry*/
            bindEntry.gateReconfigRequestParam.lastEntry = 260;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            bindEntry.gateReconfigRequestParam.lastEntry = 26;

            /* 1.8   Call function with out of bound value for maxAlwdBcOffset*/
            bindEntry.gateReconfigRequestParam.maxAlwdBcOffset = BIT_15;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

             bindEntry.gateReconfigRequestParam.maxAlwdBcOffset = 0;

            /* 1.9   Call function with out of bound value for remainingBitsFactor*/
            bindEntry.gateReconfigRequestParam.remainingBitsFactor = BIT_12;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            bindEntry.gateReconfigRequestParam.remainingBitsFactor = 1;

            /* 1.10   Call function with out of bound value for todMsb*/
            bindEntry.gateReconfigTimeParam.todMsb.l[1]= BIT_16;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            bindEntry.gateReconfigTimeParam.todMsb.l[1]= BIT_1;

            /* 1.11   Call function with out of bound value for todLsb*/
            bindEntry.gateReconfigTimeParam.todLsb = BIT_31;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            bindEntry.gateReconfigTimeParam.todLsb = BIT_2;

           /* 1.12   Call function with out of bound value for configChangeTimeExtension*/
            bindEntry.gateReconfigTimeParam.configChangeTimeExtension = BIT_16;

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

            bindEntry.gateReconfigTimeParam.configChangeTimeExtension = 0;

           /* 1.13  Call function with NULL pointer*/

            st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortBindSet(dev,portNum, tableSet,&bindEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortBindGet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_U32       tableSet;
    GT_PHYSICAL_PORT_NUM       portNum = 0;
    CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  bindEntry;


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));


    osMemSet(&bindEntry,0,sizeof(CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC));
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortBindGet
                                             (dev, portNum, &tableSet, &bindEntry));
         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressPortBindGet(dev,portNum, &tableSet,&bindEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Call function with out of bound port number*/
             st = cpssDxChStreamEgressPortBindGet(dev,UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev), &tableSet,&bindEntry);
             UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

           /* 1.3 Call function with  NULL pointer for .tableSet*/
            st = cpssDxChStreamEgressPortBindGet(dev,portNum,NULL,&bindEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

           /* 1.4  Call function with NULL pointer for bindEntry*/

            st = cpssDxChStreamEgressPortBindGet(dev,portNum, &tableSet,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortBindGet(dev,portNum, &tableSet,&bindEntry);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortBindGet(dev,portNum, &tableSet,&bindEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortBindComplete)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortBindComplete
                                             (dev));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressPortBindComplete(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortBindComplete(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortBindComplete(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortUnbindSet)
{
    /*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
    */
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM       portNum = 0;

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortUnbindSet
                                             (dev, portNum));
         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
            st = cpssDxChStreamEgressPortUnbindSet(dev,portNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2 Call function with out of bound port number*/
             st = cpssDxChStreamEgressPortUnbindSet(dev,UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev));
             UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortUnbindSet(dev,portNum);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortUnbindSet(dev,portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortQueueGateSet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM       portNum = 0;
    GT_U32 i,gate;


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortQueueGateSet
                                             (dev, portNum, 0, 0));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */  for(i=0;i<8;i++)
           {
                st = cpssDxChStreamEgressPortQueueGateSet(dev,portNum, i,i);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
           }

             /* 1.2 Read parameters back and compare them with the ones that were set.
                        Expected: GT_OK. */

           for(i=0;i<8;i++)
           {
                st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, i,&gate);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL1_PARAM_MAC(i, gate, dev);
           }


           /* 1.3 Call function with out of bound value for gate*/
            st = cpssDxChStreamEgressPortQueueGateSet(dev,portNum, 0,10);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


            /* 1.4  Call function with out of bound value for queueOffset*/
            st = cpssDxChStreamEgressPortQueueGateSet(dev,portNum, 100,0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


            /* 1.5  Call function with out of bound port number*/
            st = cpssDxChStreamEgressPortQueueGateSet(dev,UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev), 0,0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortQueueGateSet(dev,portNum, 0,0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortQueueGateSet(dev,portNum, i,i);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


UTF_TEST_CASE_MAC(cpssDxChStreamEgressPortQueueGateGet)
{
/*
    1.1 Call API with valid values.
          Expected: GT_OK.

    1.2 Read parameters back and compare them with the ones that were set                                                                        .
          Expected: GT_OK.

    2. Call API for none active devices.
    Expected: GT_NOT_APPLICABLE_DEVICE.

    3. Call API with out of bound device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS    st = GT_OK;
    GT_U8        dev;
    GT_PHYSICAL_PORT_NUM       portNum = 0;
    GT_U32 i,gate;


    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* Checks TSN features for not applicable device */
            UTF_CPSS_DXCH_TSN_SUPPORTED_MAC(dev,
                                             cpssDxChStreamEgressPortQueueGateGet
                                             (dev, portNum, 0, &gate));

         /* 1.1 Call API with valid values.
                  Expected: GT_OK.
             */
              for(i=0;i<8;i++)
              {
                   st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, i,&gate);
                   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
              }

            /* 1.2 Call function with out of bound port number*/
            st = cpssDxChStreamEgressPortQueueGateGet(dev,UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev), 0,&gate);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

           /* 1.3 Call function with  NULL pointer for .gate*/
            st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, 0,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /* 1.4 Call function with out of bound value for queue offset*/
            st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, 100,&gate);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2 For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* Prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_CPSS_PP_ALL_SIP6_30_CNS));

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
       st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, 0,&gate);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3 Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChStreamEgressPortQueueGateGet(dev,portNum, 0,&gate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}




/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChStreamControlGate suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChStreamGateControl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcGlobalConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcGateConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcGateConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcTimeBasedGateReConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcTimeBasedGateReConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamSgcTimeBasedGateReConfigComplete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortBindComplete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressTableSetConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressTableSetConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortBindSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortBindGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortUnbindSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortQueueGateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChStreamEgressPortQueueGateGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChStreamGateControl)
