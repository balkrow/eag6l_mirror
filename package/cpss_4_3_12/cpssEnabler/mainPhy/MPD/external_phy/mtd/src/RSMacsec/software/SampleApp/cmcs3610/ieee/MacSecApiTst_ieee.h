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


#include "macsec_api.h"
#include <ieee/macsec_ieee_api.h>
#include <ieee/mcs_internals_ieee.h>

#ifndef __MACSECAPITST_IEEE_H__
#define __MACSECAPITST_IEEE_H__



#ifdef __cplusplus
extern "C" {
#endif

DLL_PUBLIC void testGetIeeeApi(RmsDev_t * device_p);

#ifdef __cplusplus
}
#endif

#endif // __MACSECAPITST_IEEE_H__
