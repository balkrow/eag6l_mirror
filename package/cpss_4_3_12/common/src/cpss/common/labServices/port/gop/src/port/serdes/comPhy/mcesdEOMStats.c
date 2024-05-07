/*******************************************************************************
Copyright (C) 2021, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
* mcesdEOMStats.c
*
* DESCRIPTION:
*       Collection of EOM Statistics functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "mcesdEOMStats.h"

#ifdef MCESD_EOM_STATS

CPSS_ADDITION_START
#define round(x) ((int)((x) + 0.5))
CPSS_ADDITION_END

/* Forward internal function prototypes used only in this module */
static MCESD_STATUS INT_EOM_STATS_BathtubAddPrecision(INOUT double *bufferPtr, IN MCESD_U32 arrayErrorIndex, IN MCESD_U32 arrayIndex, IN MCESD_U32 arrayShift, IN MCESD_U32 arrayCenter, IN MCESD_U32 arrayCount, IN MCESD_U32 sampleCount, IN MCESD_32 direction);
static MCESD_STATUS INT_EOM_STATS_LMSFit(IN double *bufferPtr, IN MCESD_U32 arrayIndex, IN MCESD_U32 arrayShift, IN MCESD_U32 minIndex, IN MCESD_U32 maxIndex, IN MCESD_32  degree, IN MCESD_32 direction, OUT double *a, OUT double *b, OUT double *c);

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
)
{
    double x_double, y_double, mean0, mean1, stdev0, stdev1;
    MCESD_U32 findHist[2] = { 0, 0 };
    MCESD_U32 vectorCount = voltageMax - voltageMin;
    MCESD_U32 i, j, adjustedVoltageCenter, findCount, segment0Min, segment0Max, segment1Min = 1, segment1Max;

    amplitudeStats->Q = -1;
    amplitudeStats->SNR = -1;
    amplitudeStats->lowerMean = -1;
    amplitudeStats->lowerStdDev = -1;
    amplitudeStats->upperMean = -1;
    amplitudeStats->upperStdDev = -1;

    /*
    * Get amplitude data @ phase 0
    * Store data in buffer[0] relative to range where 0 is data at minimum phase (going forward all indices are relative, unless stated)
    * buffer[0] = [vertical]            buffer[1] = [unused]
    */
    i = 0;
    adjustedVoltageCenter = voltageCenter - voltageMin;
    for (j = voltageMin; j <= voltageMax; j++)
    {
        /* buffer[0][i], eye[phaseCenter][j] */
        *(bufferPtr + 0 * phaseMaxCount + i) = *(eyeRawData + phaseCenter * voltageMaxCount + j);
        i++;
    }

    /*
    * Record differences in vector values from a given 0-point
    * Store data in buffer[1]
    * buffer[0] = [vertical]            buffer[1] = [vertical diff]
    */
    for (i = 0; i < vectorCount; i++)
    {
        if (i == adjustedVoltageCenter)         /* Center is 0 */
        {
            /* buffer[1][i] */
            *(bufferPtr + 1 * phaseMaxCount + i) = 0;
        }
        else if (i < adjustedVoltageCenter)     /* Subtract towards 0 */
        {
            /* buffer[1][i], buffer[0][i], buffer[0][i + 1] */
            *(bufferPtr + 1 * phaseMaxCount + i) = *(bufferPtr + 0 * phaseMaxCount + i) - *(bufferPtr + 0 * phaseMaxCount + (i + 1));
        }
        else                                    /* Subtract towards 0 */
        {
            /* buffer[1][i], buffer[0][i], buffer[0][i - 1] */
            *(bufferPtr + 1 * phaseMaxCount + i) = *(bufferPtr + 0 * phaseMaxCount + i) - *(bufferPtr + 0 * phaseMaxCount + (i - 1));
        }
    }

    /*
    * PDF (Probability Distribution Function)
    * Store data in buffer[0]
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff]
    */
    x_double = 0;   /* Total vertical differences */
    for (i = 0; i < vectorCount; i++)
    {
        /* buffer[1][i] */
        x_double += *(bufferPtr + 1 * phaseMaxCount + i);
    }
    if (0 == x_double)
        return MCESD_OK;    /* Invalid data */
    for (i = 0; i < vectorCount; i++)
    {
        /* buffer[0][i], buffer[1][i] */
        *(bufferPtr + 0 * phaseMaxCount + i) = *(bufferPtr + 1 * phaseMaxCount + i) / x_double;
    }

    /*
    * CDF (Cumulative Distribution Function)
    * Store data in buffer[1]
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    /* buffer[1][0] */
    *(bufferPtr + 1 * phaseMaxCount + 0) = 0;
    for (i = 1; i < vectorCount; i++)
    {
        /* buffer[1][i], buffer[1][i - 1], buffer[0][i] */
        *(bufferPtr + 1 * phaseMaxCount + i) = *(bufferPtr + 1 * phaseMaxCount + (i - 1)) + *(bufferPtr + 0 * phaseMaxCount + i);
    }

    /*
    * Finds ranges of equal probability in the given cdf and trims the segments to remove leading/trailing zeros
    * Ranges for the two segments are in (segment0Min, segment0Max) and (segment1Min, segment1Max)
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    /* buffer[1][vectorCount - 1], buffer[1][0] */
    x_double = *(bufferPtr + 1 * phaseMaxCount + (vectorCount - 1)) - *(bufferPtr + 1 * phaseMaxCount + 0); /* Total probability */
    /* buffer[1][0] */
    y_double = *(bufferPtr + 1 * phaseMaxCount + 0) + x_double / 4; /* Target find probability */
    findCount = 0;
    segment0Max = 0;
    for (i = 1; i < vectorCount; i++)
    {
        /* buffer[1][i] */
        if ((*(bufferPtr + 1 * phaseMaxCount + i)) > y_double)
        {
            /* Store closest index to value */
            /* buffer[1][i], buffer[1][i - 1] */
            findHist[findCount % 2] = i - (((y_double - *(bufferPtr + 1 * phaseMaxCount + i)) + (y_double - *(bufferPtr + 1 * phaseMaxCount + (i - 1)))) <= 0);

            y_double += x_double / 2;   /* Target find probability readjusted */
            findCount++;

            if (findCount > 1)
            {
                /* Average history to get bound estimate */
                segment0Max = (MCESD_U32)round((double)(findHist[0] + findHist[1]) / 2);
                segment1Min = segment0Max;
                break;
            }
        }
    }
    segment0Min = 0;
    segment1Max = vectorCount - 1;

    /*
    * Finds the mean index based on the weights at each index
    * Mean index for the two segements are in mean0 and mean1
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    x_double = 0;
    y_double = 0;
    for (i = segment0Min; i <= segment0Max; i++)    /* Segment0 */
    {
        /* buffer[0][i] */
        x_double += *(bufferPtr + 0 * phaseMaxCount + i) * i;   /* Total by weight */
        /* buffer[0][i] */
        y_double += *(bufferPtr + 0 * phaseMaxCount + i);       /* Total */
    }
    if (0 == y_double)
        return MCESD_OK;    /* Invalid data */
    mean0 = x_double / y_double;
    amplitudeStats->upperMean = adjustedVoltageCenter - mean0;
    x_double = 0;
    y_double = 0;
    for (i = segment1Min; i <= segment1Max; i++)    /* Segment1 */
    {
        /* buffer[0][i] */
        x_double += *(bufferPtr + 0 * phaseMaxCount + i) * i;   /* Total by weight */
        /* buffer[0][i] */
        y_double += *(bufferPtr + 0 * phaseMaxCount + i);       /* Total */
    }
    if (0 == y_double)
        return MCESD_OK;    /* Invalid data */
    mean1 = x_double / y_double;
    amplitudeStats->lowerMean = mean1 - adjustedVoltageCenter;

    /*
    * Search outward from the passing region, looking for the first negative probability to trim noise and real variance outliers
    * Center is closest to segment0Max and segment1Min, step direction is -1 and +1 respectively to step towards edge
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    for (i = (MCESD_U32)round(mean0); i < vectorCount; i--) /* Segment0, Sweep from mean to edge */
    {
        /* buffer[0][i] */
        if (*(bufferPtr + 0 * phaseMaxCount + i) < 0)
        {
            segment0Min = i + 1;
            break;
        }

        if (0 == i)
            break;
    }
    for (i = (MCESD_U16)round(mean1); i < vectorCount; i++) /* Segment1, Sweep from mean to edge */
    {
        /* buffer[0][i] */
        if (*(bufferPtr + 0 * phaseMaxCount + i) < 0)
        {
            segment1Max = i + 1;
            break;
        }
    }

    /*
    * Find index stdev based on weights at each index
    * Standard deviation index for the two segements are in stdev0 and stdev1
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    x_double = 0;
    y_double = 0;
    for (i = segment0Min; i <= segment0Max; i++)    /* Segment0 */
    {
        /* buffer[0][i] */
        x_double += *(bufferPtr + 0 * phaseMaxCount + i) * pow((i - mean0), 2); /* Weighted sum of deviation squared */
        /* buffer[0][i] */
        y_double += *(bufferPtr + 0 * phaseMaxCount + i);   /* Total */
    }
    if (0 == y_double)
        return MCESD_OK;    /* Invalid data */
    stdev0 = sqrt(x_double / y_double);
    amplitudeStats->upperStdDev = stdev0;
    x_double = 0;
    y_double = 0;
    for (i = segment1Min; i <= segment1Max; i++)    /* Segment1 */
    {
        /* buffer[0][i] */
        x_double += *(bufferPtr + 0 * phaseMaxCount + i) * pow((i - mean1), 2); /* Weighted sum of deviation squared */
        /* buffer[0][i] */
        y_double += *(bufferPtr + 0 * phaseMaxCount + i);   /* Total */
    }
    if (0 == y_double)
        return MCESD_OK;    /* Invalid data */
    stdev1 = sqrt(x_double / y_double);
    amplitudeStats->lowerStdDev = stdev1;

    /*
    * Calculate Q factor and SNR
    * buffer[0] = [vertical diff pdf]   buffer[1] = [vertical diff cdf]
    */
    x_double = pow((mean0 - adjustedVoltageCenter), 2) + pow((mean1 - adjustedVoltageCenter), 2);   /* SPWR */
    y_double = pow(stdev0, 2) + pow(stdev1, 2); /* NPWR */
    if (0 == y_double)
        return MCESD_OK;    /* Invalid data */

    amplitudeStats->Q = sqrt(x_double / y_double);
    amplitudeStats->SNR = 10 * log10(x_double / y_double);     /* dB */

    return MCESD_OK;
}

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
)
{
    double a_double, b_double, c_double, d_double;
    MCESD_U32 i, anchor, sweepCenter, vectorCount;
    MCESD_32 j;

    /*
    * Get dimension error data @ anchor
    * Store data in buffer[0] relative to range where 0 is data at anchor (going forward all indices are relative unless stated)
    * buffer[0] = [error]               buffer[1] = [unused]
    */
    if (estimateWidth == MCESD_TRUE)
    {
        anchor = voltageCenter;
        sweepCenter = phaseCenter;
        vectorCount = phaseMax - phaseMin + 1;

        j = 0;
        for (i = phaseMin; i <= phaseMax; i++)
        {
            /* buffer[0][j], eye[i][anchor] */
            *(bufferPtr + 0 * phaseMaxCount + j) = *(eyeRawData + i * voltageMaxCount + anchor);
            j++;
        }
        sweepCenter = sweepCenter - phaseMin;   /* adjusted */
    }
    else
    {
        anchor = phaseCenter;
        sweepCenter = voltageCenter;
        vectorCount = voltageMax - voltageMin + 1;
        for (i = voltageMin; i <= voltageMax; i++)
        {
            /* buffer[0][i], eye[anchor][i] */
            *(bufferPtr + 0 * phaseMaxCount + i) = *(eyeRawData + anchor * voltageMaxCount + i);
        }
        sweepCenter = sweepCenter - voltageMin;   /* adjusted */
    }

    /*
    * Scale error by measured precision (BER = 1 / sampleCount)
    * Store data in buffer[1]
    * buffer[0] = [error]               buffer[1] = [error rate]
    */
    for (i = 0; i < vectorCount; i++)
    {
        /* buffer[1][i], buffer[0][i] */
        *(bufferPtr + 1 * phaseMaxCount + i) = (*(bufferPtr + 0 * phaseMaxCount + i)) / sampleCount;
    }

    /*
    * Curve fit an edge of the eye to predict theoretical error-rates at all points
    * buffer[0] = [yIndex]              buffer[1] = [error rate]
    */
    if (MCESD_FAIL == INT_EOM_STATS_BathtubAddPrecision(bufferPtr, 1, 0, phaseMaxCount, sweepCenter, vectorCount, sampleCount, 1))
    {
        for (i = 0; i < estimateEyeDimCount; i++)
        {
            if (MCESD_TRUE == estimateWidth)
            {
                estimateEyeDimPtr[i].width = -1;
            }
            else
            {
                estimateEyeDimPtr[i].upperHeight = -1;
                estimateEyeDimPtr[i].lowerHeight = -1;
            }
        }
        return MCESD_FAIL;
    }
    if (MCESD_FAIL == INT_EOM_STATS_BathtubAddPrecision(bufferPtr, 1, 0, phaseMaxCount, sweepCenter, vectorCount, sampleCount, -1))
    {
        for (i = 0; i < estimateEyeDimCount; i++)
        {
            if (MCESD_TRUE == estimateWidth)
            {
                estimateEyeDimPtr[i].width = -1;
            }
            else
            {
                estimateEyeDimPtr[i].upperHeight = -1;
                estimateEyeDimPtr[i].lowerHeight = -1;
            }
        }
        return MCESD_FAIL;
    }

    /*
    * Take log of vector (log(0) = log(DBL_MIN))
    * buffer[0] = [yIndex]              buffer[1] = [log error rate]
    */
    for (i = 0; i < vectorCount; i += 1)
    {
        /* buffer[1][i] */
        if (*(bufferPtr + 1 * phaseMaxCount + i) == 0)
        {
            /* buffer[1][i] */
            *(bufferPtr + 1 * phaseMaxCount + i) = log(DBL_MIN);
        }
        else
        {
            /* buffer[1][i], buffer[1][i] */
            *(bufferPtr + 1 * phaseMaxCount + i) = log(*(bufferPtr + 1 * phaseMaxCount + i));
        }
    }

    for (i = 0; i < estimateEyeDimCount; i++)
    {
        /* Find the lowest and highest indexes in the given range where data drops below "value" and interpolates linearly between indexes */
        a_double = -1;
        b_double = -1;
        c_double = log(estimateEyeDimPtr[i].ber / 2);
        for (j = vectorCount - 2; j >= 0; j--)
        {
            /* buffer[1][j] */
            if (*(bufferPtr + 1 * phaseMaxCount + j) < c_double)
            {
                /* buffer[1][j + 1], buffer[1][j] */
                d_double = (*(bufferPtr + 1 * phaseMaxCount + (j + 1))) - (*(bufferPtr + 1 * phaseMaxCount + j));
                if (0 == d_double)
                {
                    b_double = j + 0.5;
                }
                else
                {
                    /* buffer[1][j] */
                    b_double = *(bufferPtr + 1 * phaseMaxCount + j) / d_double + j;
                }
                break;
            }
        }

        for (j = 1; j <= (MCESD_32)(vectorCount - 1); j++)
        {
            /* buffer[1][j] */
            if (*(bufferPtr + 1 * phaseMaxCount + j) < c_double)
            {
                /* buffer[1][j], buffer[1][j - 1] */
                d_double = (*(bufferPtr + 1 * phaseMaxCount + j)) - (*(bufferPtr + 1 * phaseMaxCount + (j - 1)));
                if (0 == d_double)
                {
                    a_double = j - 1 + 0.5;
                }
                else
                {
                    /* buffer[1][j - 1] */
                    a_double = *(bufferPtr + 1 * phaseMaxCount + (j - 1)) / d_double + j - 1;
                }
                break;
            }
        }
        if (MCESD_TRUE == estimateWidth)
        {
            /* Default to -1 if indicies are unchanged or out of range */
            estimateEyeDimPtr[i].width = ((-1 == b_double) || (-1 == a_double) || (b_double < a_double)) ? -1 : b_double - a_double;
        }
        else
        {
            /* Default to -1 if indicies are unchanged or out of range */
            estimateEyeDimPtr[i].upperHeight = ((-1 == a_double) || (voltageCenter < a_double)) ? -1 : voltageCenter - a_double;
            estimateEyeDimPtr[i].lowerHeight = ((-1 == b_double) || (b_double < voltageCenter)) ? -1 : b_double - voltageCenter;
        }
    }

    return MCESD_OK;
}

/**
@brief  Curve fit an edge of the eye to predict theoretical error-rates at all points

@param[in,out]  bufferPtr - 2D buffer pointer
@param[in]      arrayErrorIndex - error rate array index in buffer
@param[in]      arrayIndex - unused array index in buffer
@param[in]      arrayShift - array shift value (2nd dimension of buffer)
@param[in]      arrayCenter - array center value
@param[in]      arrayCount - array count value
@param[in]      sampleCount - sample count value
@param[in]      direction - direction from center

@note Direction is either 1 or -1
@note Portion of eye being curve fit:
@note (direction =  1, estimateWidth =  MCESD_TRUE) = right eye
@note (direction = -1, estimateWidth =  MCESD_TRUE) =  left eye
@note (direction =  1, estimateWidth = MCESD_FALSE) = lower eye
@note (direction = -1, estimateWidth = MCESD_FALSE) = upper eye
@note Abbreviations:
@note e = errorRateIndex
@note y = yIndex (unused index)
@note x = xIndex

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS INT_EOM_STATS_BathtubAddPrecision
(
    INOUT double *bufferPtr,
    IN MCESD_U32 arrayErrorIndex,
    IN MCESD_U32 arrayIndex,
    IN MCESD_U32 arrayShift,
    IN MCESD_U32 arrayCenter,
    IN MCESD_U32 arrayCount,
    IN MCESD_U32 sampleCount,
    IN MCESD_32 direction
)
{
    double x_double, y_double, a, b, c;
    MCESD_32 i, j, innerCount;
    MCESD_U32 innerMin, innerMax;

    innerMin = 0;
    innerMax = 0;

    /*
    * Find inner eye range in this direction
    * Index innerMin is closest from center
    * buffer[e] = [error rate]          buffer[y] = [unused]
    */
    for (i = arrayCenter; i < (MCESD_32)arrayCount; i += direction)
    {
        /* buffer[e][i] */
        if ((*(bufferPtr + arrayErrorIndex * arrayShift + i)) >= (1 / (double)sampleCount))
        {
            innerMin = i;
            break;
        }
    }

    /*
    * Find end of inner eye range (Must be 3 points)
    * Index innerMax is farther from center
    * buffer[e] = [error rate]          buffer[y] = [unused]
    */
    for (i = innerMin + 2 * direction; i < (MCESD_32)arrayCount; i += direction)
    {
        /* Inner eye end = 0.01 */
        /* buffer[e][i] */
        if ((*(bufferPtr + arrayErrorIndex * arrayShift + i)) > 0.01)
        {
            innerMax = i;
            break;
        }
    }
    innerCount = (innerMax > innerMin) ? innerMax - innerMin + 1 : innerMin - innerMax + 1;

    /*
    * Copy inner eye error rate (Order data is stored is from center to edge)
    * Store data in buffer[y] relative to inner range where 0 is innerMin (going forward all indices are relative, unless stated)
    * buffer[e] = [error rate]          buffer[y] = [y coordinates]
    */
    j = innerMin;
    for (i = 0; i < innerCount; i += 1) /* Index moving from center to edge */
    {
        /* buffer[y][i], buffer[e][j] */
        *(bufferPtr + arrayIndex * arrayShift + i) = (*(bufferPtr + arrayErrorIndex * arrayShift + j));
        j += direction;
    }

    /*
    * Fix dropout that data[i] will have at least a ratio to data[i - 1]
    * buffer[e] = [error rate]          buffer[y] = [y coordinates]
    */
    /* buffer[y][0] */
    x_double = *(bufferPtr + arrayIndex * arrayShift + 0);
    for (i = 1; i < innerCount; i += 1)
    {
        y_double = x_double * 0.2;  /* Min Data */
        /* buffer[y][i] */
        x_double = *(bufferPtr + arrayIndex * arrayShift + i);  /* Temp value */
        /* Ratio = 0.2 */
        /* buffer[y][i] */
        if ((*(bufferPtr + arrayIndex * arrayShift + i)) < y_double)
        {
            /* buffer[y][i] */
            *(bufferPtr + arrayIndex * arrayShift + i) = y_double;
        }
    }

    /*
    * Take log of vector (log(0) = log(1 / sampleCount))
    * buffer[e] = [error rate]          buffer[y] = [log y coordinates]
    */
    for (i = 0; i < innerCount; i += 1)
    {
        /* buffer[y][i] */
        if ((*(bufferPtr + arrayIndex * arrayShift + i)) == 0)
        {
            /* buffer[y][i] */
            *(bufferPtr + arrayIndex * arrayShift + i) = log(1 / (double)sampleCount);
        }
        else
        {
            /* buffer[y][i] */
            *(bufferPtr + arrayIndex * arrayShift + i) = log(*(bufferPtr + arrayIndex * arrayShift + i));
        }
    }

    MCESD_ATTEMPT(INT_EOM_STATS_LMSFit(bufferPtr, arrayIndex, arrayShift, innerMin, innerMax, 2, direction, &a, &b, &c));

    /* Switch to linear if polynomial is increasing */
    if (a > 0)
    {
        MCESD_ATTEMPT(INT_EOM_STATS_LMSFit(bufferPtr, arrayIndex, arrayShift, innerMin, innerMax, 1, direction, &a, &b, &c));
    }

    if ((0 == a) & (0 == b))
        return MCESD_OK;    /* Invalid data */

    /* Update errorRate using polynomial */
    for (i = 0; i < (MCESD_32)arrayCount; i++)
    {
        /* buffer[e][i] */
        if ((*(bufferPtr + arrayErrorIndex * arrayShift + i)) < 1 / (double)sampleCount)
        {
            /* buffer[e][i] */
            *(bufferPtr + arrayErrorIndex * arrayShift + i) += exp(a * pow(i, 2) + b * i + c);
        }
    }

    return MCESD_OK;
}

/**
@brief  Finds the closest polynomial of given degree

@param[in]  bufferPtr - 2D buffer pointer
@param[in]  arrayIndex - log y coordinates array index in buffer
@param[in]  arrayShift - array shift value (2nd dimension of buffer)
@param[in]  minIndex - minimum index of log y coordinates array
@param[in]  maxIndex - maximum index of log y coordinates array
@param[in]  degree - target polynomial degree
@param[in]  direction - direction from center

@param[out] a - quadratic coefficient
@param[out] b - linear coefficient
@param[out] c - constant term

@note Supported degrees are 1 and 2
@note Direction is either 1 or -1

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS INT_EOM_STATS_LMSFit
(
    IN double *bufferPtr,
    IN MCESD_U32 arrayIndex,
    IN MCESD_U32 arrayShift,
    IN MCESD_U32 minIndex,
    IN MCESD_U32 maxIndex,
    IN MCESD_32  degree,
    IN MCESD_32 direction,
    OUT double *a,
    OUT double *b,
    OUT double *c
)
{
    double reduction[3][4];             /* reduction matrix - rowSize = MAX_DEGREE + 1, colSize = MAX_DEGREE + 2 */
    double xLMS[5] = { 0 };             /* Size = 2 * MAX_DEGREE + 1 */
    double yLMS[3] = { 0 };             /* Size = MAX_DEGREE + 1 */
    double coefficients[3] = { 0 };     /* Size = MAX_DEGREE + 1 */
    double x_double;
CPSS_ADDITION_START
    volatile MCESD_32 i, j, k; /* use volatile to solve false compilation error in GCC 4.8.2 */
CPSS_ADDITION_END
    MCESD_32 innerCount = (maxIndex > minIndex) ? maxIndex - minIndex + 1 : minIndex - maxIndex + 1;

    *a = 0;
    *b = 0;
    *c = 0;

    if ((degree != 2) && (degree != 1))
        return MCESD_FAIL;                      /* Invalid degree */

    if ((direction != 1) && (direction != -1))
        return MCESD_FAIL;                      /* Invalid direction*/

    /* Precomputations for reduction matrix */
    for (i = 0; i < 2 * degree + 1; i++)        /* Exponent index */
    {
        for (j = 0; j < innerCount; j++)        /* Inner buffer region index */
        {
            xLMS[i] += pow(minIndex + j * direction, i);
        }
    }

    for (i = 0; i < degree + 1; i++)            /* Exponent index */
    {
        for (j = 0; j < innerCount; j++)        /* Inner buffer region index */
        {
            /* buffer[arrayIndex][j] */
            yLMS[i] += pow(minIndex + j * direction, i) * (*(bufferPtr + arrayIndex * arrayShift + j));
        }
    }

    /* Populate reduction matrix */
    for (i = 0; i < degree + 1; i++)            /* Row index */
    {
        for (j = 0; j < degree + 1; j++)        /* Column index */
        {
            reduction[i][j] = xLMS[i + j];
        }
        reduction[i][degree + 1] = yLMS[i];
    }

    /* Matrix reduction via gaussian elimination */
    for (i = 0; i < degree; i++)                /* Diagonal index */
    {
        k = i;                                  /* Largest row index */
        for (j = i + 1; j < degree + 1; j++)    /* Row dimension to find largest row */
        {
            if (abs((MCESD_32)reduction[k][i]) < abs((MCESD_32)reduction[j][i]))
            {
                k = j;
            }
        }
        for (j = 0; j < degree + 2; j++)        /* Row dimension to swap rows */
        {
            x_double = reduction[i][j];         /* Temp value */
            reduction[i][j] = reduction[k][j];
            reduction[k][j] = x_double;
        }

        for (j = i + 1; j < degree + 1; j++)    /* Row dimension to zero below the diagonal */
        {
            x_double = reduction[j][i] / reduction[i][i];
            for (k = 0; k < degree + 2; k++)
            {
                reduction[j][k] -= x_double * reduction[i][k];
            }
        }
    }

    /* Solve system of equations */
    for (i = degree; i >= 0; i--)
    {
        coefficients[i] = reduction[i][degree + 1];
        for (j = i + 1; j < degree + 1; j++)
        {
            coefficients[i] -= reduction[i][j] * coefficients[j];
        }
        if (0 == reduction[i][i])
            return MCESD_OK;    /* Invalid data */
        coefficients[i] /= reduction[i][i];
    }

    *a = coefficients[2];
    *b = coefficients[1];
    *c = coefficients[0];

    return MCESD_OK;
}
#endif
