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


#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#include "macsec_ieee_api.h"
#else
#include "mcs_internals_CT.h"
#include "macsec_CT_api.h"
#endif
#include "macsec_api.h"

#ifndef __MACSECAPITST_H__
#define __MACSECAPITST_H__



#ifdef __cplusplus
extern "C" {
#endif
DLL_PUBLIC void testGetApi(RmsDev_t * device_p, bool testLockout);
DLL_PUBLIC void test_InterruptApis(RmsDev_t * device_p);

#ifdef __cplusplus
}
#endif

#endif // __MACSECAPITST_H__
