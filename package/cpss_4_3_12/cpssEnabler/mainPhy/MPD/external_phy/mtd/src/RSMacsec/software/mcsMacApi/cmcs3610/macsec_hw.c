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
// * $Rev: 
// * $Date: 
// ******************************************************************************


#include <errno.h>

#include "i_macsec.h"
#include "macsec_api.h"
#include "macsec_hw.h"
#include "sim_log.h"

#if !(VARIANT == VARIANT_cmcs3610)
#error "Trying to compile macsec_hw.c for wrong target."
#endif

int MacsecPabSetPortMode( RmsDev_t * rmsDev_p, MacsecPortMode_t mode )
{
    if( rmsDev_p ) {
        rmsDev_p = rmsDev_p;
    }
    mode = mode;
    return 0;
}

int MacsecPabSetPortCfg( RmsDev_t *        rmsDev_p,
                         MacsecDirection_t dir,
                         unsigned          port,
                         unsigned          addfragsize,
                         bool              enaFrag )
{
    UNUSED(rmsDev_p);
    UNUSED(dir);
    UNUSED(port);
    UNUSED(addfragsize);
    UNUSED(enaFrag);

    return -ENOSYS;
}

int MacsecPabGetPortCfg( RmsDev_t *        rmsDev_p,
                         MacsecDirection_t dir,
                         unsigned          port,
                         unsigned *        addfragsize,
                         bool *            enaFrag )
{
    UNUSED(rmsDev_p);
    UNUSED(dir);
    UNUSED(port);
    UNUSED(addfragsize);
    UNUSED(enaFrag);

    return -ENOSYS;
}

bool MacsecPexHasInterrupts( void )
{
    return true;
}

int MacsecBbeSetFifoCreditCfg( RmsDev_t * rmsDev_p, MacsecDirection_t dir, unsigned index, unsigned credits )
{
    UNUSED(rmsDev_p);
    UNUSED(dir);
    UNUSED(index);
    UNUSED(credits);

    return -ENOSYS;
}

int MacsecBbeGetFifoCreditCfg( RmsDev_t * rmsDev_p, MacsecDirection_t dir, unsigned index, unsigned * credits )
{
    UNUSED(rmsDev_p);
    UNUSED(dir);
    UNUSED(index);
    UNUSED(credits);

    return -ENOSYS;
}

int MacsecMtSetPortCfg( RmsDev_t * rmsDev_p, unsigned port, unsigned parseDepth )
{
    Ra01RsMcsTopPortConfig_t portCfg;

    portCfg.parseDepth = (uint8_t)parseDepth;

    return Ra01AccRsMcsTopPortConfig( rmsDev_p, port, &portCfg , RA01_WRITE_OP);
}

int MacsecMtGetPortCfg( RmsDev_t * rmsDev_p, unsigned port, unsigned * parseDepth )
{
    int                   ret = 0;
    Ra01RsMcsTopPortConfig_t portCfg;

    if( parseDepth == NULL ) { return -EINVAL; }

    ret = Ra01AccRsMcsTopPortConfig( rmsDev_p, port, &portCfg , RA01_READ_OP);
    DBGRETURN(ret);

    *parseDepth = (unsigned)portCfg.parseDepth;

    return ret;
}

