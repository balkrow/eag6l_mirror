/*! 
 * @file macsec_hw.h
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

#ifndef __MACSEC_HW_H__
#define __MACSEC_HW_H__

#define VARIANT VARIANT_cmcs3610

#ifndef VARIANT
#error "Need to specify a code variant!"
#endif
#if (VARIANT != VARIANT_cmcs3610 )
#error "Included cmcs3610 HW definitions in some other variant"
#endif
#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#else
#include "mcs_internals_CT.h"
#endif
/*!
 * @brief number of ports
 *
 * @ingroup base_types
 */
#define MACSEC_PORT_COUNT                   1

/*!
 * @brief Number of channel per port
 *
 * @ingroup base_types
 */
#define MACSEC_CHANNELS_PER_PORT            2   // Pre-empt and express

/*!
 * @brief BBE does not have configurable credits.
 *
 * @ingroup base_types
 */
#define MACSEC_PLATFORM_BBE_CREDIT_CFG      0

/*!
 * @brief Platform support for PEX interrupts
 */
#define MACSEC_PLATFORM_SUPPORTS_PEX_INTERRUPTS     1

/*!
 * @brief Number of dfifo channels
 *
 * Oddly MCS3510 has one DFIFO per port, MCS100 has 2.
 */
#define MACSEC_DFIFO_COUNT              (MACSEC_PORT_COUNT)

/*!
 * @brief Port per direction parameters
 *
 * Port parameters configurable for ingress and egress independently.
 *
 * @sa MCS_REG_PAB_RX_PORT_CFG_0
 * @sa MCS_REG_PAB_TX_PORT_CFG_0
 *
 * @ingroup port_if_types
 */
typedef struct {
    bool                    valid;                  //!< Table entry is valid
    unsigned                addfragsize;            //!< 2 bits. Preempt does not occur until 64 * (1 + addFragSize)
    bool                    enaFrag;                //!< Enable fragmentation
} PortDirParam_t;

/*! Port mode.
 *
 * Setting for the data rate of the port. (PORT_CFG->MODE bits).
 *
 * NOTE: the ports are 10G combined. Only port 0 may be set for 10G and
 * only ports 0 and 2 can be set for 50G.
 *
 * @sa macsec_set_port_config, macsec_get_configuration
 *
 * @ingroup base_types
 * */
typedef enum {
    MACSEC_PORT_MODE_1X10,             //!< 10,  0,  0,  0

    MACSEC_PORT_MODE_COUNT,             //!< Number of valid modes
    MACSEC_PORT_MODE_INVALID            //!< Returned if ingress != egress
} MacsecPortMode_t;

#ifdef __cplusplus
extern "C" {
#endif

    //TODO: These should be common move them to i_macsec.h


#ifdef __cplusplus
}
#endif

#endif // __MACSEC_HW_H__

