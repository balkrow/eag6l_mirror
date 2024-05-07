/*******************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
* mcesdEOMStats.h
*
* DESCRIPTION:
*       Collection of EOM statistics functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef MCESD_EOM_STATS_H
#define MCESD_EOM_STATS_H

#include "mcesdTop.h"
#include "mcesdApiTypes.h"

#ifdef MCESD_EOM_STATS

/* Estimated Eye Dimensions Structure */
typedef struct
{
    double ber;         /* User provided BER target */
    double upperHeight; /* When EOMGetStats() returns, upperHeight will be populated */
    double lowerHeight; /* When EOMGetStats() returns, lowerHeight will be populated */
    double width;       /* When EOMGetStats() returns, width will be populated */
} S_EOM_STATS_EYE_DIM, *S_EOM_STATS_EYE_DIM_PTR;

/* Estimated Eye Amplitudes Structure */
typedef struct
{
    double Q;           /* Q factor */
    double SNR;         /* Signal-to-noise ratio */
    double upperMean;   /* Upper mean amplitude */
    double lowerMean;   /* Lower mean amplitude */
    double upperStdDev; /* Upper standard deviation */
    double lowerStdDev; /* Lower standard deviation */
} S_EOM_STATS_EYE_AMP, *S_EOM_STATS_EYE_AMP_PTR;

/* Internal functions intended to be called by other modules, but not directly by user */
/**
@brief  Get EOM Amplitude Statistics

@param[in]  eyeRawData - 2D array
@param[in]  bufferPtr - scratch buffer
@param[in]  phaseCenter - absolute phase center in eyeRawData
@param[in]  phaseMaxCount - 2D array phase dimension count
@param[in]  voltageMin - absolute voltage minimum in eyeRawData
@param[in]  voltageMax - absolute voltage maximum in eyeRawData
@param[in]  voltageCenter - absolute voltage center in eyeRawData
@param[in]  voltageMaxCount - 2D array voltage dimension count

@param[out] amplitudeStats - S_EOM_STATS_EYE_AMP containing Q, SNR, mean and stddev

@note Compilation with the MCESD_EOM_STATS is required
@note Compilation requires linking with math libraries ('-lm') or standard that provides math libraries ('-std=C99')
@note Expected 2D array dimensions to be [phase][voltage]
@note Expected scratch buffer dimensions to be at least [2][phaseMaxCount]
@note For calculating amplitude stats, only values in phase center and voltage range are used
@note Q and SNR default to -1 when data is invalid

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS INT_EOM_STATS_GetAmplitudeStats
(
    IN MCESD_32 *eyeRawData,
    IN double *bufferPtr,
    IN MCESD_U32 phaseCenter,
    IN MCESD_U16 phaseMaxCount,
    IN MCESD_U32 voltageMin,
    IN MCESD_U32 voltageMax,
    IN MCESD_U32 voltageCenter,
    IN MCESD_U32 voltageMaxCount,
    OUT S_EOM_STATS_EYE_AMP* amplitudeStats
);

/**
@brief  Estimate eye dimensions

@param[in]      eyeRawData - 2D array
@param[in]      bufferPtr - scratch buffer
@param[in]      phaseMin - absolute phase minimum in eyeRawData
@param[in]      phaseMax - absolute phase maximum in eyeRawData
@param[in]      phaseCenter - absolute phase center in eyeRawData
@param[in]      phaseMaxCount - 2D array phase dimension count
@param[in]      voltageMin - absolute voltage minimum in eyeRawData
@param[in]      voltageMax - absolute voltage maximum in eyeRawData
@param[in]      voltageCenter - absolute voltage center in eyeRawData
@param[in]      voltageMaxCount - 2D array voltage dimension count
@param[in]      sampleCount - sample count
@param[in,out]  estimateEyeDimPtr - array of S_EOM_STATS_EYE_DIM of requested estimated eye width/height
@param[in]      estimateEyeDimCount - count in array
@param[in]      estimateWidth - TRUE to estimate width, FALSE to estimate height

@note Compilation with the MCESD_EOM_STATS is required
@note Compilation requires linking with math libraries ('-lm') or standard that provides math libraries ('-std=C99')
@note Expected 2D array dimensions to be [phase][voltage]
@note Expected scratch buffer dimensions to be at least [2][phaseMaxCount]
@note Array of estimate eye dimensions expects BER attribute to be populated
@note Format of BER attribute is actual BER for example (1e-9)
@note Dimensions default to -1 when predictions are out of bounds
@note For estimating width, only values in phase range and voltage center are used (data from full UI expected)
@note For estimating height, only values in phase center and voltage range are used (data from full UI expected)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS INT_EOM_STATS_EstimateDimension
(
    IN MCESD_32 *eyeRawData,
    IN double *bufferPtr,
    IN MCESD_U32 phaseMin,
    IN MCESD_U32 phaseMax,
    IN MCESD_U32 phaseCenter,
    IN MCESD_U16 phaseMaxCount,
    IN MCESD_U32 voltageMin,
    IN MCESD_U32 voltageMax,
    IN MCESD_U32 voltageCenter,
    IN MCESD_U16 voltageMaxCount,
    IN MCESD_U32 sampleCount,
    INOUT S_EOM_STATS_EYE_DIM *estimateEyeDimPtr,
    IN MCESD_U32 estimateEyeDimCount,
    IN MCESD_BOOL estimateWidth
);
#endif

#endif /* defined MCESD_EOM_STATS_H */
