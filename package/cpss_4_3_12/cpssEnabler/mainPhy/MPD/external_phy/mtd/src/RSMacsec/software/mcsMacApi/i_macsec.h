/*! @file i_macsec.h
 *-----------------------------------------------------------------------------
 * @brief MACsec software internal header
 *
 * @par Copyright
 * Copyright 2018-2020 Rianta Solutions Inc. All Rights Reserved.
 *-----------------------------------------------------------------------------
 *
 * All information contained herein is, and remains the property of
 * Rianta Solutions Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Rianta
 * Solutions Inc. and its suppliers and may be covered by U.S. and
 * Foreign Patents, patents in process, and are protected by trade secret
 * or copyright law. Dissemination of this information or reproduction of
 * this material is strictly forbidden unless prior written permission is
 * obtained from Rianta Solutions Inc.
 *
 *-----------------------------------------------------------------------------
 * $Date: 2022-11-17-16:28:56
 * $Rev: 1.1.0
 *
 *---------------------------------------------------------------------------*/
#ifndef __I_MACSEC_H__
#define __I_MACSEC_H__

#include <stdbool.h>
#include <stdint.h>

#include "macsec_api.h"
#include "macsec_cfg.h"

/*!
 * @brief Allow function variables to be marked as unused
 */
#ifdef __KEIL__
#define UNUSED(VAR) ((VAR)=(VAR))
#else
#define UNUSED(VAR) ((void)(VAR))
#endif

//------------------------------------------------------------------------------
// Internal limits
//------------------------------------------------------------------------------
/*! @brief The MACsec capabilities structure
 *
 * This is the internal copy of the capabilities.
 */
typedef struct {
    MacsecCapabilities_t       external;       //!< Capabilities for the API
    struct {
        bool                    valid;          //!< Are the external capabilities valid
    } internal;                                 //!< Internal information.
} MacsecCapabilitiesEx_t;

/*! @brief Used in the SC timer register lookup in CPM
 */
typedef enum {
    CPM_TIMER_UNIT_TICK,
    CPM_SC_TIMER_RESET_ALL_GO,
    CPM_SC_TIMER_START_GO,
    CPM_SC_TIMER_TIMEOUT_THRESH,
    CPM_SC_TIMER_CONFIG,
    CPM_SC_TIMER_MEM,
} MacsecCpmScTimer_t;

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// From macsec_hw.c
//------------------------------------------------------------------------------

/*! @brief The maximum value of the register enumeration
 *
 * The enum is used as a table inedix for register lookups. Use this to check what the
 * maximum is.
 */
#define MCS_REG_INVALID  (MCS_REG_MAX + 1)

/*! @brief Read a ports configuration
 */
int MacsecPabGetPortCfg( RmsDev_t *        rmsDev_p,
                         MacsecDirection_t dir,
                         unsigned          port,
                         unsigned *        addfragsize,
                         bool *            enaFrag );

/*! @brief Set the port mode
 */
int MacsecPabSetPortMode( RmsDev_t * rmsDev_p, MacsecPortMode_t mode );

/*! @brief Configure a port.
 */
int MacsecPabSetPortCfg( RmsDev_t *        rmsDev_p,
                         MacsecDirection_t dir,
                         unsigned          port,
                         unsigned          addfragsize,
                         bool              enaFrag );

#ifdef __cplusplus
}
#endif

#endif // __I_MACSEC_H__

