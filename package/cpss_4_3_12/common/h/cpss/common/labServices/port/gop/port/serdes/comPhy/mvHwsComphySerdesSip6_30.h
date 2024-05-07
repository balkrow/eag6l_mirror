/*******************************************************************************
Copyright (C) 2014-2016, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file is a placeholder of ComPhy defined file.
********************************************************************/

#ifndef MV_HWS_COMPHY_SERDES_SIP6_30_H
#define MV_HWS_COMPHY_SERDES_SIP6_30_H

#ifdef C12GP41P2V_SIP6_30

#ifdef API_C12GP41P2V_PollField
#undef API_C12GP41P2V_PollField
#endif
#define API_C12GP41P2V_PollField             mvHws_API_C12GP41P2V_PollField_Sip6_30

#ifdef API_C12GP41P2V_PollPin
#undef API_C12GP41P2V_PollPin
#endif
#define API_C12GP41P2V_PollPin               mvHws_API_C12GP41P2V_PollPin_Sip6_30

#ifdef API_C12GP41P2V_ReadField
#undef API_C12GP41P2V_ReadField
#endif
#define API_C12GP41P2V_ReadField             mvHws_API_C12GP41P2V_ReadField_Sip6_30

#ifdef API_C12GP41P2V_ReadReg
#undef API_C12GP41P2V_ReadReg
#endif
#define API_C12GP41P2V_ReadReg               mvHws_API_C12GP41P2V_ReadReg_Sip6_30

#ifdef API_C12GP41P2V_WriteField
#undef API_C12GP41P2V_WriteField
#endif
#define API_C12GP41P2V_WriteField            mvHws_API_C12GP41P2V_WriteField_Sip6_30

#ifdef API_C12GP41P2V_WriteReg
#undef API_C12GP41P2V_WriteReg
#endif
#define API_C12GP41P2V_WriteReg              mvHws_API_C12GP41P2V_WriteReg_Sip6_30

#ifdef API_C12GP41P2V_HwGetPinCfg
#undef API_C12GP41P2V_HwGetPinCfg
#endif
#define API_C12GP41P2V_HwGetPinCfg           mvHws_API_C12GP41P2V_HwGetPinCfg_Sip6_30

#ifdef API_C12GP41P2V_HwReadReg
#undef API_C12GP41P2V_HwReadReg
#endif
#define API_C12GP41P2V_HwReadReg             mvHws_API_C12GP41P2V_HwReadReg_Sip6_30

#ifdef API_C12GP41P2V_HwSetPinCfg
#undef API_C12GP41P2V_HwSetPinCfg
#endif
#define API_C12GP41P2V_HwSetPinCfg           mvHws_API_C12GP41P2V_HwSetPinCfg_Sip6_30

#ifdef API_C12GP41P2V_HwWriteReg
#undef API_C12GP41P2V_HwWriteReg
#endif
#define API_C12GP41P2V_HwWriteReg            mvHws_API_C12GP41P2V_HwWriteReg_Sip6_30

#ifdef API_C12GP41P2V_Wait
#undef API_C12GP41P2V_Wait
#endif
#define API_C12GP41P2V_Wait                  mvHws_API_C12GP41P2V_Wait_Sip6_30

#ifdef API_C12GP41P2V_PowerOffLane
#undef API_C12GP41P2V_PowerOffLane
#endif
#define API_C12GP41P2V_PowerOffLane          mvHws_API_C12GP41P2V_PowerOffLane_Sip6_30


#ifdef API_C12GP41P2V_PowerOnSeq
#undef API_C12GP41P2V_PowerOnSeq
#endif
#define API_C12GP41P2V_PowerOnSeq            mvHws_API_C12GP41P2V_PowerOnSeq_Sip6_30

#ifdef API_C12GP41P2V_CheckTraining
#undef API_C12GP41P2V_CheckTraining
#endif
#define API_C12GP41P2V_CheckTraining         mvHws_API_C12GP41P2V_CheckTraining_Sip6_30

#ifdef API_C12GP41P2V_EOMFinalize
#undef API_C12GP41P2V_EOMFinalize
#endif
#define API_C12GP41P2V_EOMFinalize           mvHws_API_C12GP41P2V_EOMFinalize_Sip6_30

#ifdef API_C12GP41P2V_EOMGetWidthHeight
#undef API_C12GP41P2V_EOMGetWidthHeight
#endif
#define API_C12GP41P2V_EOMGetWidthHeight     mvHws_API_C12GP41P2V_EOMGetWidthHeight_Sip6_30

#ifdef API_C12GP41P2V_EOMInit
#undef API_C12GP41P2V_EOMInit
#endif
#define API_C12GP41P2V_EOMInit               mvHws_API_C12GP41P2V_EOMInit_Sip6_30

#ifdef API_C12GP41P2V_EOMMeasPoint
#undef API_C12GP41P2V_EOMMeasPoint
#endif
#define API_C12GP41P2V_EOMMeasPoint          mvHws_API_C12GP41P2V_EOMMeasPoint_Sip6_30

#ifdef API_C12GP41P2V_ExecuteTraining
#undef API_C12GP41P2V_ExecuteTraining
#endif
#define API_C12GP41P2V_ExecuteTraining       mvHws_API_C12GP41P2V_ExecuteTraining_Sip6_30

#ifdef API_C12GP41P2V_GetAlign90
#undef API_C12GP41P2V_GetAlign90
#endif
#define API_C12GP41P2V_GetAlign90            mvHws_API_C12GP41P2V_GetAlign90_Sip6_30

#ifdef API_C12GP41P2V_GetCDRLock
#undef API_C12GP41P2V_GetCDRLock
#endif
#define API_C12GP41P2V_GetCDRLock            mvHws_API_C12GP41P2V_GetCDRLock_Sip6_30

#ifdef API_C12GP41P2V_GetCDRParam
#undef API_C12GP41P2V_GetCDRParam
#endif
#define API_C12GP41P2V_GetCDRParam           mvHws_API_C12GP41P2V_GetCDRParam_Sip6_30

#ifdef API_C12GP41P2V_GetComparatorStats
#undef API_C12GP41P2V_GetComparatorStats
#endif
#define API_C12GP41P2V_GetComparatorStats    mvHws_API_C12GP41P2V_GetComparatorStats_Sip6_30


#ifdef API_C12GP41P2V_GetCTLEParam
#undef API_C12GP41P2V_GetCTLEParam
#endif
#define API_C12GP41P2V_GetCTLEParam          mvHws_API_C12GP41P2V_GetCTLEParam_Sip6_30

#ifdef API_C12GP41P2V_GetDataBusWidth
#undef API_C12GP41P2V_GetDataBusWidth
#endif
#define API_C12GP41P2V_GetDataBusWidth       mvHws_API_C12GP41P2V_GetDataBusWidth_Sip6_30

#ifdef API_C12GP41P2V_GetDataPath
#undef API_C12GP41P2V_GetDataPath
#endif
#define API_C12GP41P2V_GetDataPath           mvHws_API_C12GP41P2V_GetDataPath_Sip6_30

#ifdef API_C12GP41P2V_GetDfeEnable
#undef API_C12GP41P2V_GetDfeEnable
#endif
#define API_C12GP41P2V_GetDfeEnable          mvHws_API_C12GP41P2V_GetDfeEnable_Sip6_30

#ifdef API_C12GP41P2V_GetDfeTap
#undef API_C12GP41P2V_GetDfeTap
#endif
#define API_C12GP41P2V_GetDfeTap             mvHws_API_C12GP41P2V_GetDfeTap_Sip6_30

#ifdef API_C12GP41P2V_GetFreezeDfeUpdates
#undef API_C12GP41P2V_GetFreezeDfeUpdates
#endif
#define API_C12GP41P2V_GetFreezeDfeUpdates   mvHws_API_C12GP41P2V_GetFreezeDfeUpdates_Sip6_30

#ifdef API_C12GP41P2V_GetPhyMode
#undef API_C12GP41P2V_GetPhyMode
#endif
#define API_C12GP41P2V_GetPhyMode            mvHws_API_C12GP41P2V_GetPhyMode_Sip6_30

#ifdef API_C12GP41P2V_GetPLLLock
#undef API_C12GP41P2V_GetPLLLock
#endif
#define API_C12GP41P2V_GetPLLLock            mvHws_API_C12GP41P2V_GetPLLLock_Sip6_30

#ifdef API_C12GP41P2V_GetPowerIvRef
#undef API_C12GP41P2V_GetPowerIvRef
#endif
#define API_C12GP41P2V_GetPowerIvRef         mvHws_API_C12GP41P2V_GetPowerIvRef_Sip6_30

#ifdef API_C12GP41P2V_GetPowerPLL
#undef API_C12GP41P2V_GetPowerPLL
#endif
#define API_C12GP41P2V_GetPowerPLL           mvHws_API_C12GP41P2V_GetPowerPLL_Sip6_30

#ifdef API_C12GP41P2V_GetPowerRx
#undef API_C12GP41P2V_GetPowerRx
#endif
#define API_C12GP41P2V_GetPowerRx            mvHws_API_C12GP41P2V_GetPowerRx_Sip6_30

#ifdef API_C12GP41P2V_GetPowerTx
#undef API_C12GP41P2V_GetPowerTx
#endif
#define API_C12GP41P2V_GetPowerTx            mvHws_API_C12GP41P2V_GetPowerTx_Sip6_30

#ifdef API_C12GP41P2V_GetRefFreq
#undef API_C12GP41P2V_GetRefFreq
#endif
#define API_C12GP41P2V_GetRefFreq            mvHws_API_C12GP41P2V_GetRefFreq_Sip6_30

#ifdef API_C12GP41P2V_GetSlewRateEnable
#undef API_C12GP41P2V_GetSlewRateEnable
#endif
#define API_C12GP41P2V_GetSlewRateEnable     mvHws_API_C12GP41P2V_GetSlewRateEnable_Sip6_30

#ifdef API_C12GP41P2V_GetSlewRateParam
#undef API_C12GP41P2V_GetSlewRateParam
#endif
#define API_C12GP41P2V_GetSlewRateParam      mvHws_API_C12GP41P2V_GetSlewRateParam_Sip6_30

#ifdef API_C12GP41P2V_GetSquelchDetect
#undef API_C12GP41P2V_GetSquelchDetect
#endif
#define API_C12GP41P2V_GetSquelchDetect      mvHws_API_C12GP41P2V_GetSquelchDetect_Sip6_30

#ifdef API_C12GP41P2V_GetSquelchThreshold
#undef API_C12GP41P2V_GetSquelchThreshold
#endif
#define API_C12GP41P2V_GetSquelchThreshold   mvHws_API_C12GP41P2V_GetSquelchThreshold_Sip6_30

#ifdef API_C12GP41P2V_GetTrainingTimeout
#undef API_C12GP41P2V_GetTrainingTimeout
#endif
#define API_C12GP41P2V_GetTrainingTimeout    mvHws_API_C12GP41P2V_GetTrainingTimeout_Sip6_30

#ifdef API_C12GP41P2V_GetTxEqParam
#undef API_C12GP41P2V_GetTxEqParam
#endif
#define API_C12GP41P2V_GetTxEqParam          mvHws_API_C12GP41P2V_GetTxEqParam_Sip6_30

#ifdef API_C12GP41P2V_GetTxOutputEnable
#undef API_C12GP41P2V_GetTxOutputEnable
#endif
#define API_C12GP41P2V_GetTxOutputEnable     mvHws_API_C12GP41P2V_GetTxOutputEnable_Sip6_30

#ifdef API_C12GP41P2V_GetTxRxBitRate
#undef API_C12GP41P2V_GetTxRxBitRate
#endif
#define API_C12GP41P2V_GetTxRxBitRate        mvHws_API_C12GP41P2V_GetTxRxBitRate_Sip6_30

#ifdef API_C12GP41P2V_GetTxRxPattern
#undef API_C12GP41P2V_GetTxRxPattern
#endif
#define API_C12GP41P2V_GetTxRxPattern        mvHws_API_C12GP41P2V_GetTxRxPattern_Sip6_30

#ifdef API_C12GP41P2V_GetTxRxPolarity
#undef API_C12GP41P2V_GetTxRxPolarity
#endif
#define API_C12GP41P2V_GetTxRxPolarity       mvHws_API_C12GP41P2V_GetTxRxPolarity_Sip6_30

#ifdef API_C12GP41P2V_GetTxRxReady
#undef API_C12GP41P2V_GetTxRxReady
#endif
#define API_C12GP41P2V_GetTxRxReady          mvHws_API_C12GP41P2V_GetTxRxReady_Sip6_30

#ifdef API_C12GP41P2V_ResetComparatorStats
#undef API_C12GP41P2V_ResetComparatorStats
#endif
#define API_C12GP41P2V_ResetComparatorStats  mvHws_API_C12GP41P2V_ResetComparatorStats_Sip6_30

#ifdef API_C12GP41P2V_RxInit
#undef API_C12GP41P2V_RxInit
#endif
#define API_C12GP41P2V_RxInit                mvHws_API_C12GP41P2V_RxInit_Sip6_30

#ifdef API_C12GP41P2V_SetAlign90
#undef API_C12GP41P2V_SetAlign90
#endif
#define API_C12GP41P2V_SetAlign90            mvHws_API_C12GP41P2V_SetAlign90_Sip6_30

#ifdef API_C12GP41P2V_SetCDRParam
#undef API_C12GP41P2V_SetCDRParam
#endif
#define API_C12GP41P2V_SetCDRParam           mvHws_API_C12GP41P2V_SetCDRParam_Sip6_30

#ifdef API_C12GP41P2V_SetCTLEParam
#undef API_C12GP41P2V_SetCTLEParam
#endif
#define API_C12GP41P2V_SetCTLEParam          mvHws_API_C12GP41P2V_SetCTLEParam_Sip6_30

#ifdef API_C12GP41P2V_SetDataBusWidth
#undef API_C12GP41P2V_SetDataBusWidth
#endif
#define API_C12GP41P2V_SetDataBusWidth       mvHws_API_C12GP41P2V_SetDataBusWidth_Sip6_30

#ifdef API_C12GP41P2V_SetDataPath
#undef API_C12GP41P2V_SetDataPath
#endif
#define API_C12GP41P2V_SetDataPath           mvHws_API_C12GP41P2V_SetDataPath_Sip6_30

#ifdef API_C12GP41P2V_SetDfeEnable
#undef API_C12GP41P2V_SetDfeEnable
#endif
#define API_C12GP41P2V_SetDfeEnable          mvHws_API_C12GP41P2V_SetDfeEnable_Sip6_30

#ifdef API_C12GP41P2V_SetFreezeDfeUpdates
#undef API_C12GP41P2V_SetFreezeDfeUpdates
#endif
#define API_C12GP41P2V_SetFreezeDfeUpdates   mvHws_API_C12GP41P2V_SetFreezeDfeUpdates_Sip6_30

#ifdef API_C12GP41P2V_SetPhyMode
#undef API_C12GP41P2V_SetPhyMode
#endif
#define API_C12GP41P2V_SetPhyMode            mvHws_API_C12GP41P2V_SetPhyMode_Sip6_30

#ifdef API_C12GP41P2V_SetPowerIvRef
#undef API_C12GP41P2V_SetPowerIvRef
#endif
#define API_C12GP41P2V_SetPowerIvRef         mvHws_API_C12GP41P2V_SetPowerIvRef_Sip6_30

#ifdef API_C12GP41P2V_SetPowerPLL
#undef API_C12GP41P2V_SetPowerPLL
#endif
#define API_C12GP41P2V_SetPowerPLL           mvHws_API_C12GP41P2V_SetPowerPLL_Sip6_30

#ifdef API_C12GP41P2V_SetPowerRx
#undef API_C12GP41P2V_SetPowerRx
#endif
#define API_C12GP41P2V_SetPowerRx            mvHws_API_C12GP41P2V_SetPowerRx_Sip6_30

#ifdef API_C12GP41P2V_SetPowerTx
#undef API_C12GP41P2V_SetPowerTx
#endif
#define API_C12GP41P2V_SetPowerTx            mvHws_API_C12GP41P2V_SetPowerTx_Sip6_30

#ifdef API_C12GP41P2V_SetRefFreq
#undef API_C12GP41P2V_SetRefFreq
#endif
#define API_C12GP41P2V_SetRefFreq            mvHws_API_C12GP41P2V_SetRefFreq_Sip6_30

#ifdef API_C12GP41P2V_SetSlewRateEnable
#undef API_C12GP41P2V_SetSlewRateEnable
#endif
#define API_C12GP41P2V_SetSlewRateEnable     mvHws_API_C12GP41P2V_SetSlewRateEnable_Sip6_30

#ifdef API_C12GP41P2V_SetSlewRateParam
#undef API_C12GP41P2V_SetSlewRateParam
#endif
#define API_C12GP41P2V_SetSlewRateParam      mvHws_API_C12GP41P2V_SetSlewRateParam_Sip6_30

#ifdef API_C12GP41P2V_SetSquelchThreshold
#undef API_C12GP41P2V_SetSquelchThreshold
#endif
#define API_C12GP41P2V_SetSquelchThreshold   mvHws_API_C12GP41P2V_SetSquelchThreshold_Sip6_30

#ifdef API_C12GP41P2V_SetTrainingTimeout
#undef API_C12GP41P2V_SetTrainingTimeout
#endif
#define API_C12GP41P2V_SetTrainingTimeout    mvHws_API_C12GP41P2V_SetTrainingTimeout_Sip6_30

#ifdef API_C12GP41P2V_SetTxEqParam
#undef API_C12GP41P2V_SetTxEqParam
#endif
#define API_C12GP41P2V_SetTxEqParam          mvHws_API_C12GP41P2V_SetTxEqParam_Sip6_30

#ifdef API_C12GP41P2V_SetTxOutputEnable
#undef API_C12GP41P2V_SetTxOutputEnable
#endif
#define API_C12GP41P2V_SetTxOutputEnable     mvHws_API_C12GP41P2V_SetTxOutputEnable_Sip6_30

#ifdef API_C12GP41P2V_SetTxRxBitRate
#undef API_C12GP41P2V_SetTxRxBitRate
#endif
#define API_C12GP41P2V_SetTxRxBitRate        mvHws_API_C12GP41P2V_SetTxRxBitRate_Sip6_30

#ifdef API_C12GP41P2V_SetTxRxPattern
#undef API_C12GP41P2V_SetTxRxPattern
#endif
#define API_C12GP41P2V_SetTxRxPattern        mvHws_API_C12GP41P2V_SetTxRxPattern_Sip6_30

#ifdef API_C12GP41P2V_SetTxRxPolarity
#undef API_C12GP41P2V_SetTxRxPolarity
#endif
#define API_C12GP41P2V_SetTxRxPolarity       mvHws_API_C12GP41P2V_SetTxRxPolarity_Sip6_30

#ifdef API_C12GP41P2V_StartPhyTest
#undef API_C12GP41P2V_StartPhyTest
#endif
#define API_C12GP41P2V_StartPhyTest          mvHws_API_C12GP41P2V_StartPhyTest_Sip6_30

#ifdef API_C12GP41P2V_StartTraining
#undef API_C12GP41P2V_StartTraining
#endif
#define API_C12GP41P2V_StartTraining         mvHws_API_C12GP41P2V_StartTraining_Sip6_30

#ifdef API_C12GP41P2V_StopPhyTest
#undef API_C12GP41P2V_StopPhyTest
#endif
#define API_C12GP41P2V_StopPhyTest           mvHws_API_C12GP41P2V_StopPhyTest_Sip6_30

#ifdef API_C12GP41P2V_StopTraining
#undef API_C12GP41P2V_StopTraining
#endif
#define API_C12GP41P2V_StopTraining          mvHws_API_C12GP41P2V_StopTraining_Sip6_30

#ifdef API_C12GP41P2V_TxInjectError
#undef API_C12GP41P2V_TxInjectError
#endif
#define API_C12GP41P2V_TxInjectError         mvHws_API_C12GP41P2V_TxInjectError_Sip6_30

#ifdef API_C12GP41P2V_EOMPlotEyeData
#undef API_C12GP41P2V_EOMPlotEyeData
#endif
#define API_C12GP41P2V_EOMPlotEyeData        mvHws_API_C12GP41P2V_EOMPlotEyeData_Sip6_30

#ifdef API_C12GP41P2V_EOMGetEyeData
#undef API_C12GP41P2V_EOMGetEyeData
#endif
#define API_C12GP41P2V_EOMGetEyeData         mvHws_API_C12GP41P2V_EOMGetEyeData_Sip6_30

#ifdef API_C12GP41P2V_GetDRO
#undef API_C12GP41P2V_GetDRO
#endif
#define API_C12GP41P2V_GetDRO                mvHws_API_C12GP41P2V_GetDRO_Sip6_30

#ifdef API_C12GP41P2V_EOM1UIStepCount
#undef API_C12GP41P2V_EOM1UIStepCount
#endif
#define API_C12GP41P2V_EOM1UIStepCount       mvHws_API_C12GP41P2V_EOM1UIStepCount_Sip6_30

#ifdef API_C12GP41P2V_EOMConvertWidthHeight
#undef API_C12GP41P2V_EOMConvertWidthHeight
#endif
#define API_C12GP41P2V_EOMConvertWidthHeight mvHws_API_C12GP41P2V_EOMConvertWidthHeight_Sip6_30

#ifdef API_C12GP41P2V_EyeDataDimensions
#undef API_C12GP41P2V_EyeDataDimensions
#endif
#define API_C12GP41P2V_EyeDataDimensions mvHws_API_C12GP41P2V_EyeDataDimensions_Sip6_30

#endif /*C12GP41P2V_SIP6_30*/

#endif /* defined MV_HWS_COMPHY_SERDES_H */
