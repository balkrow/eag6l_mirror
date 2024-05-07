/*! 
 * @file macsec_caps.c
 */

// *******************************************************************************
// *
// * MARVELL CONFIDENTIAL
// * ___________________
// *
// * Copyright (C) 2022 Marvell.
// * All Rights Reserved.
// *
// * NOTICE: All information contained herein is, and remains the property of
// * Marvell Asia Pte Ltd and its suppliers, if any. The intellectual and
// * technical concepts contained herein are proprietary to Marvell Asia Pte Ltd
// * and its suppliers and may be covered by U.S. and Foreign Patents, patents
// * in process, and are protected by trade secret or copyright law.
// * Dissemination of this information or reproduction of this material is strictly
// * forbidden unless prior written permission is obtained from Marvell Asia Pte Ltd.
// *
// ******************************************************************************
// * $Rev: 1.1.0
// * $Date: 2022-11-17-16:28:56
// ******************************************************************************

#include <errno.h>

#include "i_macsec.h"
#include "macsec_api.h"
#include "macsec_cfg.h"
#include "sim_log.h"

static const MacsecCapabilitiesEx_t g_capabilities = {
    .external = {
        .ingressMaxSecy                 = INGRESS_SECY_POLICY_TABLE_SIZE,
        .ingressFlowIdTcamTableSize     = INGRESS_FLOW_ID_TCAM_TABLE_SIZE,
        .ingressSecyPolicyTableSize     = INGRESS_SECY_POLICY_TABLE_SIZE,
        .ingressScCamLookupKeyTableSize = INGRESS_SC_CAM_LOOKUP_TABLE_SIZE,
        .ingressAnPerSc                 = INGRESS_AN_PER_SC,
        .ingressScAnToSaMapTableSize    = INGRESS_SA_INDEX_TABLE_SIZE,
        .ingressSaPolicyTableSize       = INGRESS_SA_POLICY_TABLE_SIZE,

        .egressMaxSecy                  = EGRESS_SECY_POLICY_TABLE_SIZE,
        .egressFlowIdTcamTableSize      = EGRESS_FLOW_ID_TCAM_TABLE_SIZE,
        .egressSecyPolicyTableSize      = EGRESS_SECY_POLICY_TABLE_SIZE,
        .egressSaPolicyTableSize        = EGRESS_SA_POLICY_TABLE_SIZE,
        .egressSecyToSaMapTableSize     = EGRESS_SECY_TO_SA_MAP_TABLE_SIZE,

        .version = {
            .major   = MACSEC_SWAPI_VERSION_MAJOR,
            .minor   = MACSEC_SWAPI_VERSION_MINOR,
            .release = MACSEC_SWAPI_VERSION_RELEASE,
        },
    },

    .internal.valid = true,
};

int MacsecGetCapabilities( MacsecCapabilities_t * capabilities )
{
    if( capabilities == NULL ) {
        return -EINVAL;
    }
    if( g_capabilities.internal.valid ) {
        capabilities->ingressMaxSecy = g_capabilities.external.ingressMaxSecy;
        capabilities->ingressFlowIdTcamTableSize = g_capabilities.external.ingressFlowIdTcamTableSize;
        capabilities->ingressSecyPolicyTableSize = g_capabilities.external.ingressSecyPolicyTableSize;
        capabilities->ingressScCamLookupKeyTableSize = g_capabilities.external.ingressScCamLookupKeyTableSize;
        capabilities->ingressAnPerSc = g_capabilities.external.ingressAnPerSc ;
        capabilities->ingressScAnToSaMapTableSize = g_capabilities.external.ingressScAnToSaMapTableSize;
        capabilities->ingressSaPolicyTableSize = g_capabilities.external.ingressSaPolicyTableSize;

        capabilities->egressMaxSecy = g_capabilities.external.egressMaxSecy;
        capabilities->egressFlowIdTcamTableSize = g_capabilities.external.egressFlowIdTcamTableSize;
        capabilities->egressSecyPolicyTableSize = g_capabilities.external.egressSecyPolicyTableSize;
        capabilities->egressSaPolicyTableSize = g_capabilities.external.egressSaPolicyTableSize;
        capabilities->egressSecyToSaMapTableSize = g_capabilities.external.egressSecyToSaMapTableSize;

        capabilities->version.major = g_capabilities.external.version.major;
        capabilities->version.minor = g_capabilities.external.version.minor;
        capabilities->version.release = g_capabilities.external.version.release;

    } else {
        return -ENOSYS;
    }

    return 0;
}


